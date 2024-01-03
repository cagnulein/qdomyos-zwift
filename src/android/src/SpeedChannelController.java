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
import android.util.Log;

import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelType;

import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;

public class SpeedChannelController extends AntChannelController {
    public static final int SPEED_SENSOR_ID = 0x9e3d4b33;

    // The device type and transmission type to be part of the channel ID message
    private static final int CHANNEL_SPEED_DEVICE_TYPE = 0x79;
    private static final int CHANNEL_SPEED_TRANSMISSION_TYPE = 1;

    // The period and frequency values the channel will be configured to
    private static final int CHANNEL_SPEED_PERIOD = 8086; // 1 Hz
    private static final int CHANNEL_SPEED_FREQUENCY = 57;

    private static final String TAG = SpeedChannelController.class.getSimpleName();

	private static final IAntChannelEventHandler CALLBACK = new ChannelEventCallback();

    private static final double MILLISECOND_TO_1_1024_CONVERSION = 0.9765625;

    double speed = 0.0;
    int cadence = 0;

    public SpeedChannelController(AntChannel antChannel) {
	    super(antChannel, SPEED_SENSOR_ID, CHANNEL_SPEED_DEVICE_TYPE, CHANNEL_SPEED_TRANSMISSION_TYPE, CHANNEL_SPEED_PERIOD, CHANNEL_SPEED_FREQUENCY, ChannelType.BIDIRECTIONAL_MASTER, TAG, CALLBACK);
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
            Log.e(TAG, "Channel Death");
        }

        @Override
        public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
            Log.d(TAG, "Rx: " + antParcel);
            Log.d(TAG, "Message Type: " + messageType);

            if(carousalTimer == null) {
               carousalTimer = new Timer(); // At this line a new Thread will be created
               carousalTimer.scheduleAtFixedRate(new TimerTask() {
                   @Override
                   public void run() {
                       Log.d(TAG, "Tx Unsollicited");
                       long realtimeMillis = SystemClock.elapsedRealtime();

                       if (lastTime != 0) {
                           elapsedMillis = realtimeMillis - lastTime;
                           totalWay += speed * elapsedMillis / 3_600L;
                           totalRotations += (double) cadence * elapsedMillis / 60_000L;
                           rev = (int) (totalWay / wheel);
                           rotations = (int) totalRotations;
                           lastCadenceEventTime = realtimeMillis - (long) ((totalRotations - rotations) / cadence * 60_000);
                           lastSpeedEventTime = realtimeMillis - (long) ((totalWay - (rev * wheel)) / speed * 3_600);
                       }
                       lastTime = realtimeMillis;

                       byte[] payload = new byte[8];

                       int lastCadenceEventTime1024 = (int) ((double) lastCadenceEventTime / MILLISECOND_TO_1_1024_CONVERSION);
                       int lastSpeedEventTime1024 = (int) ((double) lastSpeedEventTime / MILLISECOND_TO_1_1024_CONVERSION);
                       payload[0] = (byte) (lastCadenceEventTime1024 & 0xFF);
                       payload[1] = (byte) ((lastCadenceEventTime1024 >> 8) & 0xFF);
                       payload[2] = (byte) (rotations & 0xFF);
                       payload[3] = (byte) ((rotations >> 8) & 0xFF);
                       payload[4] = (byte) (lastSpeedEventTime1024 & 0xFF);
                       payload[5] = (byte) ((lastSpeedEventTime1024 >> 8) & 0xFF);
                       payload[6] = (byte) (rev & 0xFF);
                       payload[7] = (byte) ((rev >> 8) & 0xFF);

                       if (IsOpen()) {
                           try {
                               // Setting the data to be broadcast on the next channel period
                               AntChannel.setBroadcastData(payload);
                           } catch (RemoteException e) {
                               channelError(e);
                           }
                       }
                   }
               }, 0, 500); // delay
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
                    Log.d(TAG, "Event Code: " + code);

                    // Switching on event code to handle the different types of channel events
                    switch (code) {
                        case TX:
                            long realtimeMillis = SystemClock.elapsedRealtime();

                            if (lastTime != 0) {
                                elapsedMillis = realtimeMillis - lastTime;
                                totalWay += speed * elapsedMillis / 3_600L;
                                totalRotations += (double) cadence * elapsedMillis / 60_000L;
                                rev = (int) (totalWay / wheel);
                                rotations = (int) totalRotations;
                                lastCadenceEventTime = realtimeMillis - (long) ((totalRotations - rotations) / cadence * 60_000);
                                lastSpeedEventTime = realtimeMillis - (long) ((totalWay - (rev * wheel)) / speed * 3_600);
                            }
                            lastTime = realtimeMillis;

                            byte[] payload = new byte[8];

                            int lastCadenceEventTime1024 = (int) ((double) lastCadenceEventTime / MILLISECOND_TO_1_1024_CONVERSION);
                            int lastSpeedEventTime1024 = (int) ((double) lastSpeedEventTime / MILLISECOND_TO_1_1024_CONVERSION);
                            payload[0] = (byte) (lastCadenceEventTime1024 & 0xFF);
                            payload[1] = (byte) ((lastCadenceEventTime1024 >> 8) & 0xFF);
                            payload[2] = (byte) (rotations & 0xFF);
                            payload[3] = (byte) ((rotations >> 8) & 0xFF);
                            payload[4] = (byte) (lastSpeedEventTime1024 & 0xFF);
                            payload[5] = (byte) ((lastSpeedEventTime1024 >> 8) & 0xFF);
                            payload[6] = (byte) (rev & 0xFF);
                            payload[7] = (byte) ((rev >> 8) & 0xFF);

                            if (IsOpen()) {
                                try {
                                    // Setting the data to be broadcast on the next channel period
                                    AntChannel.setBroadcastData(payload);
                                } catch (RemoteException e) {
                                    channelError(e);
                                }
                            }
                            break;
                        case CHANNEL_COLLISION:
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
