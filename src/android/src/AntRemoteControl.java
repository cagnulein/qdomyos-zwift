/*
 * ANT+ Remote Control implementation for QDomyos-Zwift
 * Based on Golden Cheetah RemoteControl implementation
 * Maps ANT+ remote control commands to workout controls
 */
package org.cagnulen.qdomyoszwift;

import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.AcknowledgedDataMessage;
import com.dsi.ant.message.fromant.BroadcastDataMessage;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

import android.os.RemoteException;
import android.util.Log;
import org.cagnulen.qdomyoszwift.QLog;

/**
 * ANT+ Remote Control Channel Controller
 * Handles ANT+ Control Device Profile (Device Type 0x10)
 */
public class AntRemoteControl {
    private static final String TAG = "AntRemoteControl";

    // ANT+ Control Device Profile constants
    private static final int CONTROL_DEVICE_TYPE = 0x10;
    private static final int CONTROL_TRANSMISSION_TYPE = 0x05;
    private static final short CONTROL_PERIOD = 8192; // 4 Hz
    private static final int CONTROL_FREQUENCY = 57; // 2457 MHz

    // ANT+ Control Generic Command Page
    private static final byte ANT_CONTROL_GENERIC_CMD_PAGE = 0x49;

    // ANT+ Generic Commands (from Golden Cheetah)
    private static final int ANT_CONTROL_GENERIC_CMD_MENU_UP = 0x00;
    private static final int ANT_CONTROL_GENERIC_CMD_MENU_DOWN = 0x01;
    private static final int ANT_CONTROL_GENERIC_CMD_MENU_SELECT = 0x02;
    private static final int ANT_CONTROL_GENERIC_CMD_MENU_BACK = 0x03;
    private static final int ANT_CONTROL_GENERIC_CMD_HOME = 0x04;
    private static final int ANT_CONTROL_GENERIC_CMD_START = 0x20;
    private static final int ANT_CONTROL_GENERIC_CMD_STOP = 0x21;
    private static final int ANT_CONTROL_GENERIC_CMD_RESET = 0x22;
    private static final int ANT_CONTROL_GENERIC_CMD_LENGTH = 0x23;
    private static final int ANT_CONTROL_GENERIC_CMD_LAP = 0x24;
    private static final int ANT_CONTROL_GENERIC_CMD_USER_1 = 0x8000;
    private static final int ANT_CONTROL_GENERIC_CMD_USER_2 = 0x8001;
    private static final int ANT_CONTROL_GENERIC_CMD_USER_3 = 0x8002;

    private AntChannel mAntChannel;
    private ChannelController mChannelController;
    private boolean isChannelOpen = false;
    private int deviceNumber = 0; // 0 means wildcard - accept any remote

    // Native methods for communicating with Qt layer
    public static native void nativeOnRemoteCommand(int command);
    public static native void nativeGearUp();
    public static native void nativeGearDown();

    /**
     * Channel Controller for handling ANT+ events
     */
    private class ChannelController implements IAntChannelEventHandler {

        @Override
        public void onChannelDeath() {
            QLog.w(TAG, "onChannelDeath: Remote Control Channel Death - cleaning up");
            isChannelOpen = false;
        }

        @Override
        public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
            QLog.v(TAG, "onReceiveMessage: messageType=" + messageType + ", parcel=" + antParcel);

            switch(messageType) {
                case ACKNOWLEDGED_DATA:
                    QLog.d(TAG, "onReceiveMessage: Received ACKNOWLEDGED_DATA");
                    AcknowledgedDataMessage ackMsg = new AcknowledgedDataMessage(antParcel);
                    byte[] ackPayload = ackMsg.getPayload();
                    QLog.v(TAG, "onReceiveMessage: ACKNOWLEDGED_DATA payload length=" + (ackPayload != null ? ackPayload.length : 0));
                    handleDataMessage(ackPayload);
                    break;
                case BROADCAST_DATA:
                    QLog.d(TAG, "onReceiveMessage: Received BROADCAST_DATA");
                    BroadcastDataMessage broadcastMsg = new BroadcastDataMessage(antParcel);
                    byte[] broadcastPayload = broadcastMsg.getPayload();
                    QLog.v(TAG, "onReceiveMessage: BROADCAST_DATA payload length=" + (broadcastPayload != null ? broadcastPayload.length : 0));
                    handleDataMessage(broadcastPayload);
                    break;
                case CHANNEL_EVENT:
                    ChannelEventMessage eventMessage = new ChannelEventMessage(antParcel);
                    QLog.d(TAG, "onReceiveMessage: CHANNEL_EVENT - eventCode=" + eventMessage.getEventCode());

                    switch(eventMessage.getEventCode()) {
                        case EventCode.CHANNEL_IN_WRONG_STATE:
                            QLog.w(TAG, "onReceiveMessage: CHANNEL_IN_WRONG_STATE error");
                            break;
                        case EventCode.CHANNEL_COLLISION:
                            QLog.w(TAG, "onReceiveMessage: CHANNEL_COLLISION error");
                            break;
                        case EventCode.TRANSFER_TX_FAILED:
                            QLog.w(TAG, "onReceiveMessage: TRANSFER_TX_FAILED error");
                            break;
                        case EventCode.RX_SEARCH_TIMEOUT:
                            QLog.i(TAG, "onReceiveMessage: RX_SEARCH_TIMEOUT - no remote control found");
                            break;
                        case EventCode.CHANNEL_CLOSED:
                            QLog.i(TAG, "onReceiveMessage: CHANNEL_CLOSED");
                            isChannelOpen = false;
                            break;
                        default:
                            QLog.v(TAG, "onReceiveMessage: Other channel event=" + eventMessage.getEventCode());
                            break;
                    }
                    break;
                default:
                    QLog.d(TAG, "onReceiveMessage: Unhandled messageType=" + messageType + ", parcel=" + antParcel);
                    break;
            }
        }

        /**
         * Handle incoming data messages from ANT+ remote control
         */
        private void handleDataMessage(byte[] payload) {
            QLog.v(TAG, "handleDataMessage: called with payload=" + (payload != null ? "length=" + payload.length : "null"));

            if (payload == null) {
                QLog.w(TAG, "handleDataMessage: payload is null, ignoring");
                return;
            }

            if (payload.length < 8) {
                QLog.w(TAG, "handleDataMessage: payload too short (length=" + payload.length + "), expected 8 bytes");
                return;
            }

            // Log raw payload for debugging
            StringBuilder payloadHex = new StringBuilder();
            for (int i = 0; i < payload.length; i++) {
                payloadHex.append(String.format("%02X ", payload[i]));
            }
            QLog.v(TAG, "handleDataMessage: raw payload: " + payloadHex.toString());

            // Check if this is a Generic Command Page
            byte pageNumber = payload[0];
            QLog.d(TAG, "handleDataMessage: pageNumber=0x" + Integer.toHexString(pageNumber & 0xFF) +
                   " (expected=0x" + Integer.toHexString(ANT_CONTROL_GENERIC_CMD_PAGE & 0xFF) + ")");

            if (pageNumber != ANT_CONTROL_GENERIC_CMD_PAGE) {
                QLog.w(TAG, "handleDataMessage: not a Generic Command Page, ignoring (pageNumber=0x" +
                       Integer.toHexString(pageNumber & 0xFF) + ")");
                return;
            }

            // Extract command from payload
            // Command is in bytes 1-2 (little endian)
            int command = ((payload[2] & 0xFF) << 8) | (payload[1] & 0xFF);

            QLog.i(TAG, "handleDataMessage: extracted ANT+ Remote Command=0x" + Integer.toHexString(command) +
                   " from bytes[1]=0x" + Integer.toHexString(payload[1] & 0xFF) +
                   ", bytes[2]=0x" + Integer.toHexString(payload[2] & 0xFF));

            // Map commands to actions
            handleRemoteCommand(command);
        }

        /**
         * Handle remote control commands and map them to QDomyos-Zwift actions
         */
        private void handleRemoteCommand(int command) {
            QLog.d(TAG, "handleRemoteCommand: processing command=0x" + Integer.toHexString(command));

            switch(command) {
                case ANT_CONTROL_GENERIC_CMD_MENU_UP:
                    QLog.i(TAG, "handleRemoteCommand: MENU_UP -> Gear Up (like Zwift Click)");
                    try {
                        nativeGearUp();
                        QLog.d(TAG, "handleRemoteCommand: nativeGearUp() called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeGearUp()", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_MENU_DOWN:
                    QLog.i(TAG, "handleRemoteCommand: MENU_DOWN -> Gear Down (like Zwift Click)");
                    try {
                        nativeGearDown();
                        QLog.d(TAG, "handleRemoteCommand: nativeGearDown() called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeGearDown()", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_MENU_SELECT:
                    QLog.i(TAG, "handleRemoteCommand: MENU_SELECT -> Select action");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(SELECT) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(SELECT)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_MENU_BACK:
                    QLog.i(TAG, "handleRemoteCommand: MENU_BACK -> Back action");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(BACK) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(BACK)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_HOME:
                    QLog.i(TAG, "handleRemoteCommand: HOME -> Home action");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(HOME) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(HOME)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_START:
                    QLog.i(TAG, "handleRemoteCommand: START -> Start workout");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(START) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(START)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_STOP:
                    QLog.i(TAG, "handleRemoteCommand: STOP -> Stop workout");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(STOP) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(STOP)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_LAP:
                    QLog.i(TAG, "handleRemoteCommand: LAP -> Lap marker");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(LAP) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(LAP)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_RESET:
                    QLog.i(TAG, "handleRemoteCommand: RESET -> Reset action");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(RESET) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(RESET)", e);
                    }
                    break;

                case ANT_CONTROL_GENERIC_CMD_USER_1:
                case ANT_CONTROL_GENERIC_CMD_USER_2:
                case ANT_CONTROL_GENERIC_CMD_USER_3:
                    QLog.i(TAG, "handleRemoteCommand: USER_" + (command - ANT_CONTROL_GENERIC_CMD_USER_1 + 1) + " -> User action");
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(USER) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(USER)", e);
                    }
                    break;

                default:
                    QLog.w(TAG, "handleRemoteCommand: Unknown/unmapped command=0x" + Integer.toHexString(command));
                    try {
                        nativeOnRemoteCommand(command);
                        QLog.d(TAG, "handleRemoteCommand: nativeOnRemoteCommand(UNKNOWN) called successfully");
                    } catch (Exception e) {
                        QLog.e(TAG, "handleRemoteCommand: Error calling nativeOnRemoteCommand(UNKNOWN)", e);
                    }
                    break;
            }
        }
    }

    /**
     * Constructor
     */
    public AntRemoteControl() {
        QLog.i(TAG, "AntRemoteControl: constructor - initializing ANT+ Remote Control");
        mChannelController = new ChannelController();
        QLog.d(TAG, "AntRemoteControl: constructor completed, channel controller created");
    }

    /**
     * Open ANT+ remote control channel
     */
    public boolean openChannel(AntChannel antChannel) {
        QLog.i(TAG, "openChannel: request to open ANT+ Remote Control channel");

        if (isChannelOpen) {
            QLog.w(TAG, "openChannel: Remote control channel already open, ignoring request");
            return false;
        }

        if (antChannel == null) {
            QLog.e(TAG, "openChannel: antChannel is null, cannot proceed");
            return false;
        }

        mAntChannel = antChannel;
        QLog.d(TAG, "openChannel: antChannel assigned");

        try {
            // Configure the channel
            ChannelId channelId = new ChannelId(deviceNumber, CONTROL_DEVICE_TYPE, CONTROL_TRANSMISSION_TYPE);
            QLog.d(TAG, "openChannel: created ChannelId - deviceNumber=" + deviceNumber +
                   ", deviceType=0x" + Integer.toHexString(CONTROL_DEVICE_TYPE) +
                   ", transmissionType=0x" + Integer.toHexString(CONTROL_TRANSMISSION_TYPE));

            QLog.i(TAG, "openChannel: configuring ANT+ Remote Control channel" +
                   " (deviceNumber=" + deviceNumber + ", frequency=" + CONTROL_FREQUENCY +
                   "MHz, period=" + CONTROL_PERIOD + ")");

            // Assign the channel with slave configuration
            QLog.d(TAG, "openChannel: assigning channel as SLAVE_RECEIVE_ONLY");
            mAntChannel.assign(ChannelType.SLAVE_RECEIVE_ONLY);

            // Set the channel ID
            QLog.d(TAG, "openChannel: setting channel ID");
            mAntChannel.setChannelId(channelId);

            // Set the period
            QLog.d(TAG, "openChannel: setting period=" + CONTROL_PERIOD);
            mAntChannel.setPeriod(CONTROL_PERIOD);

            // Set the RF frequency
            QLog.d(TAG, "openChannel: setting RF frequency=" + CONTROL_FREQUENCY);
            mAntChannel.setRfFrequency(CONTROL_FREQUENCY);

            // Register event handler
            QLog.d(TAG, "openChannel: registering channel event handler");
            mAntChannel.setChannelEventHandler(mChannelController);

            // Open the channel
            QLog.d(TAG, "openChannel: opening the channel");
            mAntChannel.open();

            isChannelOpen = true;
            QLog.i(TAG, "openChannel: ANT+ Remote Control channel opened successfully, now listening for commands");

            return true;

        } catch (RemoteException e) {
            QLog.e(TAG, "openChannel: RemoteException while opening ANT+ Remote Control channel", e);
            isChannelOpen = false;
            return false;
        } catch (AntCommandFailedException e) {
            QLog.e(TAG, "openChannel: AntCommandFailedException while opening ANT+ Remote Control channel", e);
            isChannelOpen = false;
            return false;
        }
    }

    /**
     * Close ANT+ remote control channel
     */
    public void closeChannel() {
        QLog.i(TAG, "closeChannel: request to close ANT+ Remote Control channel");

        if (!isChannelOpen && mAntChannel == null) {
            QLog.d(TAG, "closeChannel: channel not open and null, nothing to do");
            return;
        }

        if (mAntChannel == null) {
            QLog.w(TAG, "closeChannel: channel marked as open but mAntChannel is null, clearing flag");
            isChannelOpen = false;
            return;
        }

        try {
            QLog.d(TAG, "closeChannel: closing ANT+ Remote Control channel");
            mAntChannel.close();
            QLog.d(TAG, "closeChannel: channel closed, releasing resources");
            mAntChannel.release();
            QLog.i(TAG, "closeChannel: ANT+ Remote Control channel closed and released successfully");
        } catch (RemoteException e) {
            QLog.e(TAG, "closeChannel: RemoteException while closing ANT+ Remote Control channel", e);
        } catch (AntCommandFailedException e) {
            QLog.e(TAG, "closeChannel: AntCommandFailedException while closing ANT+ Remote Control channel", e);
        }

        isChannelOpen = false;
        mAntChannel = null;
        QLog.d(TAG, "closeChannel: cleanup completed");
    }

    /**
     * Check if channel is open
     */
    public boolean isChannelOpen() {
        boolean channelOpen = isChannelOpen;
        QLog.v(TAG, "isChannelOpen: returning " + channelOpen);
        return channelOpen;
    }

    /**
     * Set the device number to search for (0 = wildcard)
     */
    public void setDeviceNumber(int deviceNumber) {
        QLog.d(TAG, "setDeviceNumber: changing deviceNumber from " + this.deviceNumber + " to " + deviceNumber);
        if (deviceNumber == 0) {
            QLog.i(TAG, "setDeviceNumber: using wildcard (0) to accept any remote control");
        } else {
            QLog.i(TAG, "setDeviceNumber: will only accept remote control with device number " + deviceNumber);
        }
        this.deviceNumber = deviceNumber;
    }

    /**
     * Get the current device number
     */
    public int getDeviceNumber() {
        QLog.v(TAG, "getDeviceNumber: returning " + deviceNumber);
        return deviceNumber;
    }
}