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
package org.surfsite.iconsole;

import android.os.RemoteException;
import android.util.Log;

import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

import java.util.Random;

public class SpeedChannelController {
    // The device type and transmission type to be part of the channel ID message
    private static final int CHANNEL_SPEED_DEVICE_TYPE = 0x7B;
    private static final int CHANNEL_SPEED_TRANSMISSION_TYPE = 1;

    // The period and frequency values the channel will be configured to
    private static final int CHANNEL_SPEED_PERIOD = 8118; // 1 Hz
    private static final int CHANNEL_SPEED_FREQUENCY = 57;

    private static final String TAG = SpeedChannelController.class.getSimpleName();
    public static final int SPEED_SENSOR_ID = 0x9e3d4b65;

    private static Random randGen = new Random();

    private AntChannel mAntChannel;

    private ChannelEventCallback mChannelEventCallback = new ChannelEventCallback();


    private boolean mIsOpen;
    double speed = 0.0;

    public SpeedChannelController(AntChannel antChannel) {
        mAntChannel = antChannel;
        openChannel();
    }

    boolean openChannel() {
        if (null != mAntChannel) {
            if (mIsOpen) {
                Log.w(TAG, "Channel was already open");
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

                    Log.d(TAG, "Opened channel with device number: " + SPEED_SENSOR_ID);
                } catch (RemoteException e) {
                    channelError(e);
                } catch (AntCommandFailedException e) {
                    // This will release, and therefore unassign if required
                    channelError("Open failed", e);
                }
            }
        } else {
            Log.w(TAG, "No channel available");
        }

        return mIsOpen;
    }

    void channelError(RemoteException e) {
        String logString = "Remote service communication failed.";

        Log.e(TAG, logString);
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

        Log.e(TAG, logString.toString());

        mAntChannel.release();

        Log.e(TAG, "ANT Command Failed");
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

        Log.e(TAG, "Channel Closed");
    }

    /**
     * Implements the Channel Event Handler Interface so that messages can be
     * received and channel death events can be handled.
     */
    public class ChannelEventCallback implements IAntChannelEventHandler {
        int revCounts = 0;
        int ucMessageCount = 0;
        byte ucPageChange = 0;
        byte ucExtMesgType = 1;
        long lastTime = 0;
        double way;
        int rev;
        double remWay;
        double wheel = 0.1;

        @Override
        public void onChannelDeath() {
            // Display channel death message when channel dies
            Log.e(TAG, "Channel Death");
        }

        @Override
        public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
            Log.d(TAG, "Rx: " + antParcel);
            Log.d(TAG, "Message Type: " + messageType);

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
                    Log.d(TAG, "Event Code: " + code);

                    // Switching on event code to handle the different types of channel events
                    switch (code) {
                        case TX:
                            long unixTime = System.currentTimeMillis() / 1000L;

                            if (lastTime != 0) {
                                way = speed * (unixTime - lastTime) / 3.6 + remWay;
                                rev = (int)(way / wheel + 0.5);
                                remWay = way - rev * wheel;
                                revCounts += rev;
                            }
                            lastTime = unixTime;

                            ucPageChange += 0x20;
                            ucPageChange &= 0xF0;
                            ucMessageCount += 1;
                            byte[] payload = new byte[8];

                            if (ucMessageCount >= 65) {
                                if (ucExtMesgType >= 4)
                                    ucExtMesgType = 1;

                                if (ucExtMesgType == 1) {
                                    int halfunixTime = (int) (unixTime / 2L);
                                    payload[0] = (byte) ((byte) 0x01 | (byte) (ucPageChange & (byte) 0x80));
                                    payload[1] = (byte) (halfunixTime & 0xFF);
                                    payload[2] = (byte) ((halfunixTime >> 8) & 0xFF);
                                    payload[3] = (byte) ((halfunixTime >> 16) & 0xFF);
                                }
                                else if (ucExtMesgType == 2) {
                                    payload[0] = (byte) ((byte) 0x02 | (byte) (ucPageChange & (byte) 0x80));
                                    payload[1] = (byte) 0xFF;
                                    payload[2] = (byte) ((SPEED_SENSOR_ID >> 16) & 0xFF);
                                    payload[3] = (byte) ((SPEED_SENSOR_ID >> 24) & 0xFF);
                                }
                                else if (ucExtMesgType == 3) {
                                    payload[0] = (byte) ((byte) 0x03 | (byte) (ucPageChange & (byte) 0x80));
                                    payload[1] = (byte) 0x01;
                                    payload[2] = (byte) 0x01;
                                    payload[3] = (byte) 0x01;
                                }
                                if (ucMessageCount >= 68) {
                                    ucMessageCount = 0;
                                    ucExtMesgType += 1;
                                }
                            } else {
                                payload[0] = (byte) (ucPageChange & 0x80);
                                payload[1] = (byte) 0xFF;
                                payload[2] = (byte) 0xFF;
                                payload[3] = (byte) 0xFF;
                            }

                            int unixTime1024 = (int) (unixTime * 1024);
                            payload[4] = (byte) (unixTime1024 & 0xFF);
                            payload[5] = (byte) ((unixTime1024 >> 8) & 0xFF);
                            payload[6] = (byte) (revCounts & 0xFF);
                            payload[7] = (byte) ((revCounts >> 8) & 0xFF);

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
                            ucPageChange += 0x20;
                            ucPageChange &= 0xF0;
                            ucMessageCount += 1;
                            break;
                        case RX_SEARCH_TIMEOUT:
                            // TODO May want to keep searching
                            Log.e(TAG, "No Device Found");
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
