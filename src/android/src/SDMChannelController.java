/*
 * Copyright 2012 Dynastream Innovations Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
package org.cagnulen.qdomyoszwift;

import android.os.RemoteException;
import android.os.SystemClock;
import org.cagnulen.qdomyoszwift.QLog;

import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;


import java.util.Random;

public class SDMChannelController {
    // The device type and transmission type to be part of the channel ID message
    private static final int CHANNEL_SPEED_DEVICE_TYPE = 0x7C;
    private static final int CHANNEL_SPEED_TRANSMISSION_TYPE = 1;

    // The period and frequency values the channel will be configured to
    private static final int CHANNEL_SPEED_PERIOD = 8134; // 1 Hz
    private static final int CHANNEL_SPEED_FREQUENCY = 57;

    private static final String TAG = SDMChannelController.class.getSimpleName();
    public static final int SPEED_SENSOR_ID = 0x9e3d4b99;

    private static final double MILLISECOND_TO_1_1024_CONVERSION = 0.9765625;

    private AntChannel mAntChannel;

    private ChannelEventCallback mChannelEventCallback = new ChannelEventCallback();

    private boolean mIsOpen;
    double speed = 0.0;
    int cadence = 0;
    byte stride_count = 0;

    public SDMChannelController(AntChannel antChannel) {
        mAntChannel = antChannel;
        openChannel();
    }

    boolean openChannel() {
        if (null != mAntChannel) {
            if (mIsOpen) {
                QLog.w(TAG, "Channel was already open");
            } else {
                // Channel ID message contains device number, type and transmission type. In
                // order for master (TX) channels and slave (RX) channels to connect, they
                // must have the same channel ID, or wildcard (0) is used.
                ChannelId channelId = new ChannelId(SPEED_SENSOR_ID & 0xFFFF,
                        CHANNEL_SPEED_DEVICE_TYPE, CHANNEL_SPEED_TRANSMISSION_TYPE);

                try {
                    // Setting the channel event handler so that we can receive messages from ANT
                    mAntChannel.setChannelEventHandler(mChannelEventCallback);

                    // Performs channel assignment by assigning the type to the channel. Additional
                    // features (such as, background scanning and frequency agility) can be enabled
                    // by passing an ExtendedAssignment object to assign(ChannelType, ExtendedAssignment).
                    mAntChannel.assign(ChannelType.BIDIRECTIONAL_MASTER);

                    /*
                     * Configures the channel ID, messaging period and rf frequency after assigning,
                     * then opening the channel.
                     *
                     * For any additional ANT features such as proximity search or background scanning, refer to
                     * the ANT Protocol Doc found at:
                     * http://www.thisisant.com/resources/ant-message-protocol-and-usage/
                     */
                    mAntChannel.setChannelId(channelId);
                    mAntChannel.setPeriod(CHANNEL_SPEED_PERIOD);
                    mAntChannel.setRfFrequency(CHANNEL_SPEED_FREQUENCY);
                    mAntChannel.open();
                    mIsOpen = true;

                    QLog.d(TAG, "Opened channel with device number: " + SPEED_SENSOR_ID);
                } catch (RemoteException e) {
                    channelError(e);
                } catch (AntCommandFailedException e) {
                    // This will release, and therefore unassign if required
                    channelError("Open failed", e);
                }
            }
        } else {
            QLog.w(TAG, "No channel available");
        }

        return mIsOpen;
    }

    void channelError(RemoteException e) {
        String logString = "Remote service communication failed.";

        QLog.e(TAG, logString);
    }

    void channelError(String error, AntCommandFailedException e) {
        StringBuilder logString;

        if (e.getResponseMessage() != null) {
            String initiatingMessageId = "0x" + Integer.toHexString(
                    e.getResponseMessage().getInitiatingMessageId());
            String rawResponseCode = "0x" + Integer.toHexString(
                    e.getResponseMessage().getRawResponseCode());

            logString = new StringBuilder(error)
                    .append(". Command ")
                    .append(initiatingMessageId)
                    .append(" failed with code ")
                    .append(rawResponseCode);
        } else {
            String attemptedMessageId = "0x" + Integer.toHexString(
                    e.getAttemptedMessageType().getMessageId());
            String failureReason = e.getFailureReason().toString();

            logString = new StringBuilder(error)
                    .append(". Command ")
                    .append(attemptedMessageId)
                    .append(" failed with reason ")
                    .append(failureReason);
        }

        QLog.e(TAG, logString.toString());

        mAntChannel.release();

        QLog.e(TAG, "ANT Command Failed");
    }

    public void close() {
        // TODO kill all our resources
        if (null != mAntChannel) {
            mIsOpen = false;

            // Releasing the channel to make it available for others.
            // After releasing, the AntChannel instance cannot be reused.
            mAntChannel.release();
            mAntChannel = null;
        }

        QLog.e(TAG, "Channel Closed");
    }

    /**
     * Implements the Channel Event Handler Interface so that messages can be
     * received and channel death events can be handled.
     */
    public class ChannelEventCallback implements IAntChannelEventHandler {
        long lastTime = 0;
        double totalWay = 0.0;
        double totalRotations = 0.0;
        long lastSpeedEventTime = 0;
        long lastCadenceEventTime = 0;
        long elapsedMillis = 0;
        int rotations;
        int rev;
        double wheel = 0.1;
        Timer carousalTimer = null;

        @Override
        public void onChannelDeath() {
            // Display channel death message when channel dies
            QLog.e(TAG, "Channel Death");
        }

        @Override
        public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
            QLog.d(TAG, "Rx: " + antParcel);
            QLog.d(TAG, "Message Type: " + messageType);

            if(carousalTimer == null) {
               carousalTimer = new Timer(); // At this line a new Thread will be created
               carousalTimer.scheduleAtFixedRate(new TimerTask() {
                   @Override
                   public void run() {
                       QLog.d(TAG, "Tx Unsollicited");
                       long realtimeMillis = SystemClock.elapsedRealtime();
                       double speedM_s = speed / 3.6;
                       long deltaTime = (realtimeMillis - lastTime);
                       lastTime = realtimeMillis;

                       byte[] payload = new byte[8];

                       payload[0] = (byte) 0x01;
                       payload[1] = (byte) (((lastTime % 256000) / 5) & 0xFF);
                       payload[2] = (byte) ((lastTime % 256000) / 1000);
                       payload[3] = (byte) 0x00;
                       int speedFixed = (int) Math.round(speedM_s * 256.0);
                       payload[4] = (byte) ((speedFixed >> 8) & 0x0F);  // upper 4 bits of 12-bit value
                       payload[5] = (byte) (speedFixed & 0xFF);         // lower 8 bits
                       payload[6] = (byte) stride_count++;
                       payload[7] = (byte) ((double)deltaTime * 0.03125);

                       if (mIsOpen) {
                           try {
                               // Setting the data to be broadcast on the next channel period
                               mAntChannel.setBroadcastData(payload);
                           } catch (RemoteException e) {
                               channelError(e);
                           }
                       }
                   }
               }, 0, 250); // delay
           }

            // Switching on message type to handle different types of messages
            switch (messageType) {
                // If data message, construct from parcel and update channel data
                case BROADCAST_DATA:
                    // Rx Data
                    //updateData(new BroadcastDataMessage(antParcel).getPayload());
                    break;
                case ACKNOWLEDGED_DATA:
                    // Rx Data
                    //updateData(new AcknowledgedDataMessage(antParcel).getPayload());
                    break;
                case CHANNEL_EVENT:
                    // Constructing channel event message from parcel
                    ChannelEventMessage eventMessage = new ChannelEventMessage(antParcel);
                    EventCode code = eventMessage.getEventCode();
                    QLog.d(TAG, "Event Code: " + code);

                    // Switching on event code to handle the different types of channel events
                    switch (code) {
                        case TX:
                            long realtimeMillis = SystemClock.elapsedRealtime();
                            double speedM_s = speed / 3.6;
                            long deltaTime = (realtimeMillis - lastTime);
                            // in case the treadmill doesn't provide cadence, I have to force it. ANT+ requires cadence
                            lastTime = realtimeMillis;
    
                            byte[] payload = new byte[8];
    
                            payload[0] = (byte) 0x01;
                            payload[1] = (byte) (((lastTime % 256000) / 5) & 0xFF);
                            payload[2] = (byte) ((lastTime % 256000) / 1000);
                            payload[3] = (byte) 0x00;
                            int speedFixed = (int) Math.round(speedM_s * 256.0);
                            payload[4] = (byte) ((speedFixed >> 8) & 0x0F);  // upper 4 bits of 12-bit value
                            payload[5] = (byte) (speedFixed & 0xFF);         // lower 8 bits
                            payload[6] = (byte) stride_count++;
                            payload[7] = (byte) ((double)deltaTime * 0.03125);

                            if (mIsOpen) {
                                try {
                                    // Setting the data to be broadcast on the next channel period
                                    mAntChannel.setBroadcastData(payload);
                                } catch (RemoteException e) {
                                    channelError(e);
                                }
                            }
                            break;
                        case CHANNEL_COLLISION:
                            break;
                        case RX_SEARCH_TIMEOUT:
                            // TODO May want to keep searching
                            QLog.e(TAG, "No Device Found");
                            break;
                        case CHANNEL_CLOSED:
                        case RX_FAIL:
                        case RX_FAIL_GO_TO_SEARCH:
                        case TRANSFER_RX_FAILED:
                        case TRANSFER_TX_COMPLETED:
                        case TRANSFER_TX_FAILED:
                        case TRANSFER_TX_START:
                        case UNKNOWN:
                            // TODO More complex communication will need to handle these events
                            break;
                    }
                    break;
                case ANT_VERSION:
                case BURST_TRANSFER_DATA:
                case CAPABILITIES:
                case CHANNEL_ID:
                case CHANNEL_RESPONSE:
                case CHANNEL_STATUS:
                case SERIAL_NUMBER:
                case OTHER:
                    // TODO More complex communication will need to handle these message types
                    break;
            }
        }
    }
}
