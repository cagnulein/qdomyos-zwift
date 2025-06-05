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
import org.cagnulen.qdomyoszwift.QLog;

import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.AcknowledgedDataMessage;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;
import android.os.RemoteException;

import java.util.Locale;
import java.util.Timer;
import java.util.TimerTask;
import java.util.concurrent.TimeUnit;
import java.util.Random;

/**
 * ANT+ Bike Transmitter Controller
 * Follows exactly the same pattern as PowerChannelController but for Fitness Equipment
 */
public class BikeTransmitterController {
    public static final int FITNESS_EQUIPMENT_SENSOR_ID = 0x9e3d4b67; // Different from power sensor
    // The device type and transmission type to be part of the channel ID message
    private static final int CHANNEL_FITNESS_EQUIPMENT_DEVICE_TYPE = 17; // Fitness Equipment
    private static final int CHANNEL_FITNESS_EQUIPMENT_TRANSMISSION_TYPE = 5;
    // The period and frequency values the channel will be configured to
    private static final int CHANNEL_FITNESS_EQUIPMENT_PERIOD = 8192; // 4 Hz for FE
    private static final int CHANNEL_FITNESS_EQUIPMENT_FREQUENCY = 57;
    private static final String TAG = BikeTransmitterController.class.getSimpleName();
    
    // ANT+ Data Page IDs for Fitness Equipment
    private static final byte DATA_PAGE_GENERAL_FE = 0x10;
    private static final byte DATA_PAGE_BIKE_DATA = 0x19;
    private static final byte DATA_PAGE_TRAINER_DATA = 0x1A;
    private static final byte DATA_PAGE_GENERAL_SETTINGS = 0x11;
    
    private static Random randGen = new Random();
    
    // Current bike metrics to transmit
    int currentCadence = 0;           // Current cadence in RPM
    int currentPower = 0;             // Current power in watts
    double currentSpeedKph = 0.0;     // Current speed in km/h
    long totalDistance = 0;           // Total distance in meters
    int currentHeartRate = 0;         // Heart rate in BPM
    double elapsedTimeSeconds = 0.0;  // Elapsed time in seconds
    int currentResistance = 0;        // Current resistance level (0-100)
    double currentInclination = 0.0;  // Current inclination in percentage
    
    // Control commands received from ANT+ devices
    private int requestedResistance = -1;     // Requested resistance from controller
    private int requestedPower = -1;          // Requested power from controller
    private double requestedInclination = -100; // Requested inclination from controller
    
    private AntChannel mAntChannel;
    private ChannelEventCallback mChannelEventCallback = new ChannelEventCallback();
    private boolean mIsOpen;
    
    // Callbacks for control commands
    public interface ControlCommandListener {
        void onResistanceChangeRequested(int resistance);
        void onPowerChangeRequested(int power);
        void onInclinationChangeRequested(double inclination);
    }
    
    private ControlCommandListener controlListener = null;

    public BikeTransmitterController(AntChannel antChannel) {
        mAntChannel = antChannel;
        openChannel();
    }
    
    /**
     * Set the listener for control commands received from ANT+ devices
     */
    public void setControlCommandListener(ControlCommandListener listener) {
        this.controlListener = listener;
    }

    boolean openChannel() {
        if (null != mAntChannel) {
            if (mIsOpen) {
                QLog.w(TAG, "Channel was already open");
            } else {
                // Channel ID message contains device number, type and transmission type
                ChannelId channelId = new ChannelId(FITNESS_EQUIPMENT_SENSOR_ID & 0xFFFF,
                        CHANNEL_FITNESS_EQUIPMENT_DEVICE_TYPE, CHANNEL_FITNESS_EQUIPMENT_TRANSMISSION_TYPE);

                try {
                    // Setting the channel event handler so that we can receive messages from ANT
                    mAntChannel.setChannelEventHandler(mChannelEventCallback);

                    // Performs channel assignment by assigning the type to the channel
                    mAntChannel.assign(ChannelType.BIDIRECTIONAL_MASTER);

                    // Configures the channel ID, messaging period and rf frequency after assigning,
                    // then opening the channel.
                    mAntChannel.setChannelId(channelId);
                    mAntChannel.setPeriod(CHANNEL_FITNESS_EQUIPMENT_PERIOD);
                    mAntChannel.setRfFrequency(CHANNEL_FITNESS_EQUIPMENT_FREQUENCY);
                    mAntChannel.open();
                    mIsOpen = true;

                    QLog.d(TAG, "Opened fitness equipment channel with device number: " + FITNESS_EQUIPMENT_SENSOR_ID);

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

    public boolean startTransmission() {
        return openChannel();
    }

    public void stopTransmission() {
        close();
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
    }

    public void close() {
        if (null != mAntChannel) {
            mIsOpen = false;
            // Releasing the channel to make it available for others.
            // After releasing, the AntChannel instance cannot be reused.
            mAntChannel.release();
            mAntChannel = null;
        }
        QLog.e(TAG, "Fitness Equipment Channel Closed");
    }

    // Setter methods for updating bike metrics from the main application
    public void setCadence(int cadence) {
        this.currentCadence = Math.max(0, cadence);
    }

    public void setPower(int power) {
        this.currentPower = Math.max(0, power);
    }

    public void setSpeedKph(double speedKph) {
        this.currentSpeedKph = Math.max(0, speedKph);
    }

    public void setDistance(long distance) {
        this.totalDistance = Math.max(0, distance);
    }

    public void setHeartRate(int heartRate) {
        this.currentHeartRate = Math.max(0, Math.min(255, heartRate));
    }

    public void setElapsedTime(double timeSeconds) {
        this.elapsedTimeSeconds = Math.max(0, timeSeconds);
    }

    public void setResistance(int resistance) {
        this.currentResistance = Math.max(0, Math.min(100, resistance));
    }

    public void setInclination(double inclination) {
        this.currentInclination = Math.max(-100, Math.min(100, inclination));
    }

    // Getter methods for the last requested control values
    public int getRequestedResistance() {
        return requestedResistance;
    }

    public int getRequestedPower() {
        return requestedPower;
    }

    public double getRequestedInclination() {
        return requestedInclination;
    }

    public void clearControlRequests() {
        requestedResistance = -1;
        requestedPower = -1;
        requestedInclination = -100;
    }

    public boolean isTransmitting() {
        return mIsOpen;
    }

    public String getTransmissionInfo() {
        if (!mIsOpen) {
            return "Transmission: STOPPED";
        }
        
        return String.format("Transmission: ACTIVE - Cadence: %drpm, Power: %dW, " +
                           "Speed: %.1fkm/h, Resistance: %d, Inclination: %.1f%%",
                           currentCadence, currentPower, currentSpeedKph,
                           currentResistance, currentInclination);
    }

    /**
     * Implements the Channel Event Handler Interface following PowerChannelController pattern
     */
    public class ChannelEventCallback implements IAntChannelEventHandler {

        int cnt = 0;
        int eventCount = 0;
        int cumulativeDistance = 0;
        Timer carousalTimer = null;

        @Override
        public void onChannelDeath() {
            // Display channel death message when channel dies
            QLog.e(TAG, "Fitness Equipment Channel Death");
        }

        @Override
        public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
            QLog.d(TAG, "Rx: " + antParcel);
            QLog.d(TAG, "Message Type: " + messageType);
            byte[] payload = new byte[8];

            // Start unsolicited transmission timer like PowerChannelController
            if(carousalTimer == null) {
               carousalTimer = new Timer(); // At this line a new Thread will be created
               carousalTimer.scheduleAtFixedRate(new TimerTask() {
                   @Override
                   public void run() {
                       QLog.d(TAG, "Tx Unsolicited Fitness Equipment Data");
                       byte[] payload = new byte[8];
                       eventCount = (eventCount + 1) & 0xFF;
                       cumulativeDistance = (cumulativeDistance + (int)(currentSpeedKph / 3.6)) & 0xFFFF; // rough distance calc
                       
                       // Build General FE Data Page (0x10)
                       buildGeneralFEDataPage(payload);

                       if (mIsOpen) {
                           try {
                               // Setting the data to be broadcast on the next channel period
                               mAntChannel.setBroadcastData(payload);
                           } catch (RemoteException e) {
                               channelError(e);
                           }
                       }
                   }
               }, 0, 250); // Every 250ms for 4Hz
           }

            // Switching on message type to handle different types of messages
            switch (messageType) {
                case BROADCAST_DATA:
                    // Rx Data
                    break;
                case ACKNOWLEDGED_DATA:
                    // Handle control commands
                    payload = new AcknowledgedDataMessage(antParcel).getPayload();
                    QLog.d(TAG, "AcknowledgedDataMessage: " + payload);
                    handleControlCommand(payload);
                    break;
                case CHANNEL_EVENT:
                    // Constructing channel event message from parcel
                    ChannelEventMessage eventMessage = new ChannelEventMessage(antParcel);
                    EventCode code = eventMessage.getEventCode();
                    QLog.d(TAG, "Event Code: " + code);

                    // Switching on event code to handle the different types of channel events
                    switch (code) {
                        case TX:
                            cnt += 1;

                            // Cycle through different data pages like PowerChannelController
                            if (cnt % 16 == 1) {
                                // General FE Data Page (0x10)
                                buildGeneralFEDataPage(payload);
                            } else if (cnt % 16 == 5) {
                                // Bike Data Page (0x19)
                                buildBikeDataPage(payload);
                            } else if (cnt % 16 == 9) {
                                // Trainer Data Page (0x1A)
                                buildTrainerDataPage(payload);
                            } else if (cnt % 16 == 13) {
                                // General Settings Page (0x11)
                                buildGeneralSettingsPage(payload);
                            } else {
                                // Default General FE Data Page (0x10)
                                buildGeneralFEDataPage(payload);
                            }

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
                            cnt += 1;
                            break;
                        case RX_SEARCH_TIMEOUT:
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
        
        /**
         * Build General Fitness Equipment Data Page (0x10)
         */
        private void buildGeneralFEDataPage(byte[] payload) {
            payload[0] = DATA_PAGE_GENERAL_FE;
            payload[1] = 0x19; // Equipment type: Bike
            
            // Elapsed time (0.25 second resolution)
            int elapsedTime025s = (int) (elapsedTimeSeconds * 4);
            payload[2] = (byte) (elapsedTime025s & 0xFF);
            
            // Distance (meters)
            payload[3] = (byte) (totalDistance & 0xFF);
            payload[4] = (byte) ((totalDistance >> 8) & 0xFF);
            
            // Speed (0.001 m/s resolution)
            int speedMms = (int) (currentSpeedKph / 3.6 * 1000);
            payload[5] = (byte) (speedMms & 0xFF);
            payload[6] = (byte) ((speedMms >> 8) & 0xFF);
            
            // Heart rate
            payload[7] = (byte) currentHeartRate;
        }
        
        /**
         * Build Bike Data Page (0x19)
         */
        private void buildBikeDataPage(byte[] payload) {
            payload[0] = DATA_PAGE_BIKE_DATA;
            payload[1] = (byte) 0xFF; // Reserved
            payload[2] = (byte) 0xFF; // Reserved
            payload[3] = (byte) 0xFF; // Reserved
            
            // Instantaneous cadence
            payload[4] = (byte) currentCadence;
            
            // Instantaneous power (watts)
            payload[5] = (byte) (currentPower & 0xFF);
            payload[6] = (byte) ((currentPower >> 8) & 0xFF);
            
            payload[7] = (byte) 0xFF; // Reserved
        }
        
        /**
         * Build Trainer Data Page (0x1A)
         */
        private void buildTrainerDataPage(byte[] payload) {
            payload[0] = DATA_PAGE_TRAINER_DATA;
            
            // Event count (increments on parameter changes)
            payload[1] = (byte) eventCount;
            
            // Instantaneous cadence
            payload[2] = (byte) currentCadence;
            
            // Instantaneous power (watts)
            payload[3] = (byte) (currentPower & 0xFF);
            payload[4] = (byte) ((currentPower >> 8) & 0xFF);
            
            // Resistance level (0.5% resolution)
            int resistance05 = (int) (currentResistance * 2);
            payload[5] = (byte) (resistance05 & 0xFF);
            
            // Target power (watts) - echo back any requested power
            payload[6] = (byte) (requestedPower != -1 ? (requestedPower & 0xFF) : 0xFF);
            payload[7] = (byte) (requestedPower != -1 ? ((requestedPower >> 8) & 0xFF) : 0xFF);
        }
        
        /**
         * Build General Settings Page (0x11)
         */
        private void buildGeneralSettingsPage(byte[] payload) {
            payload[0] = DATA_PAGE_GENERAL_SETTINGS;
            payload[1] = (byte) 0xFF; // Reserved
            payload[2] = (byte) 0xFF; // Reserved
            payload[3] = (byte) 0xFF; // Reserved
            payload[4] = (byte) 0xFF; // Reserved
            payload[5] = (byte) 0xFF; // Reserved
            payload[6] = (byte) 0xFF; // Reserved
            payload[7] = (byte) 0xFF; // Reserved
        }
        
        /**
         * Handle incoming control commands
         */
        private void handleControlCommand(byte[] data) {
            if (data.length < 8) return;
            
            byte pageNumber = data[0];
            QLog.d(TAG, "Received control command page: 0x" + String.format("%02X", pageNumber));
            
            // Handle control command pages
            switch (pageNumber) {
                case 0x30: // Basic Resistance
                    handleBasicResistanceCommand(data);
                    break;
                case 0x31: // Target Power
                    handleTargetPowerCommand(data);
                    break;
                case 0x33: // Track Resistance
                    handleTrackResistanceCommand(data);
                    break;
            }
        }
        
        private void handleBasicResistanceCommand(byte[] data) {
            int resistance = data[7] & 0xFF; // Resistance in 0.5% increments
            double resistancePercent = resistance * 0.5;
            
            QLog.d(TAG, "Received basic resistance command: " + resistancePercent + "%");
            
            if (resistancePercent != requestedResistance && controlListener != null) {
                requestedResistance = (int) resistancePercent;
                controlListener.onResistanceChangeRequested(requestedResistance);
            }
        }
        
        private void handleTargetPowerCommand(byte[] data) {
            int targetPower = ((data[7] & 0xFF) << 8) | (data[6] & 0xFF);
            
            QLog.d(TAG, "Received target power command: " + targetPower + "W");
            
            if (targetPower != requestedPower && controlListener != null) {
                requestedPower = targetPower;
                controlListener.onPowerChangeRequested(targetPower);
            }
        }
        
        private void handleTrackResistanceCommand(byte[] data) {
            // Grade is in 0.01% increments, signed 16-bit
            int gradeRaw = ((data[6] & 0xFF) << 8) | (data[5] & 0xFF);
            if (gradeRaw > 32767) gradeRaw -= 65536; // Convert to signed
            double grade = gradeRaw * 0.01;
            
            QLog.d(TAG, "Received track resistance command: " + grade + "% grade");
            
            if (Math.abs(grade - requestedInclination) > 0.1 && controlListener != null) {
                requestedInclination = grade;
                controlListener.onInclinationChangeRequested(grade);
            }
        }
    }
}