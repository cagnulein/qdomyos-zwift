package org.cagnulen.qdomyoszwift;

import android.content.Context;
import org.cagnulen.qdomyoszwift.QLog;
import android.app.Activity;

// ANT+ Plugin imports for transmitting
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.IFitnessEquipmentStateReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.IBikeDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.IGeneralFitnessEquipmentDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.ITrainerDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.EquipmentState;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.EquipmentType;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.HeartRateDataSource;
import com.dsi.ant.plugins.antplus.pcc.defines.DeviceState;
import com.dsi.ant.plugins.antplus.pcc.defines.EventFlag;
import com.dsi.ant.plugins.antplus.pcc.defines.RequestAccessResult;
import com.dsi.ant.plugins.antplus.pccbase.AntPluginPcc.IDeviceStateChangeReceiver;
import com.dsi.ant.plugins.antplus.pccbase.AntPluginPcc.IPluginAccessResultReceiver;
import com.dsi.ant.plugins.antplus.pccbase.PccReleaseHandle;

// ANT+ low-level API imports for transmitting data
import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.fromant.AcknowledgedDataMessage;
import com.dsi.ant.message.fromant.BroadcastDataMessage;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

// Java imports
import java.math.BigDecimal;
import java.util.EnumSet;
import java.util.Timer;
import java.util.TimerTask;

/**
 * ANT+ FTMS Transmitter Controller
 * This class simulates a fitness equipment (bike) and transmits data via ANT+
 * while also receiving control commands from connected ANT+ devices
 */
public class BikeTransmitterController {
    private static final String TAG = BikeTransmitterController.class.getSimpleName();

    // ANT+ Fitness Equipment constants
    private static final int DEVICE_TYPE_FITNESS_EQUIPMENT = 17;
    private static final int TRANSMISSION_TYPE = 5;
    private static final int CHANNEL_PERIOD = 8192; // 32768/4 = 8192 (4Hz)
    private static final int RADIO_FREQUENCY = 57; // 2457 MHz
    
    // ANT+ Data Page IDs for Fitness Equipment
    private static final byte DATA_PAGE_GENERAL_FE = 0x10;
    private static final byte DATA_PAGE_GENERAL_SETTINGS = 0x11;
    private static final byte DATA_PAGE_GENERAL_METABOLIC = 0x12;
    private static final byte DATA_PAGE_BIKE_DATA = 0x19;
    private static final byte DATA_PAGE_TRAINER_DATA = 0x1A;

    private Context context;
    private AntChannel antChannel = null;
    private boolean isTransmitting = false;
    private Timer transmissionTimer = null;
    private int deviceNumber = 12345; // Should be unique device number
    
    // Current bike metrics to transmit
    private int currentCadence = 0;           // Current cadence in RPM
    private int currentPower = 0;             // Current power in watts
    private BigDecimal currentSpeed = new BigDecimal(0); // Current speed in m/s
    private long totalDistance = 0;           // Total distance in meters
    private long totalCalories = 0;           // Total calories burned
    private int currentHeartRate = 0;         // Heart rate in BPM
    private BigDecimal elapsedTime = new BigDecimal(0); // Elapsed time in seconds
    private int currentResistance = 0;        // Current resistance level (0-100)
    private double currentInclination = 0.0;  // Current inclination in percentage
    
    // Control commands received from ANT+ devices
    private int requestedResistance = -1;     // Requested resistance from controller
    private int requestedPower = -1;          // Requested power from controller
    private double requestedInclination = -100; // Requested inclination from controller
    
    // Callbacks for control commands
    public interface ControlCommandListener {
        void onResistanceChangeRequested(int resistance);
        void onPowerChangeRequested(int power);
        void onInclinationChangeRequested(double inclination);
    }
    
    private ControlCommandListener controlListener = null;
    
    // ANT+ Channel event handler
    private final IAntChannelEventHandler mChannelEventHandler = new IAntChannelEventHandler() {
        @Override
        public void onChannelDeath() {
            QLog.d(TAG, "ANT+ Channel Death");
            isTransmitting = false;
        }

        @Override
        public void onReceiveMessage(MessageFromAntType messageType, AntMessageParcel antParcel) {
            switch (messageType) {
                case ACKNOWLEDGED_DATA:
                    handleAcknowledgedData(new AcknowledgedDataMessage(antParcel));
                    break;
                case BROADCAST_DATA:
                    handleBroadcastData(new BroadcastDataMessage(antParcel));
                    break;
                case CHANNEL_EVENT:
                    handleChannelEvent(new ChannelEventMessage(antParcel));
                    break;
                default:
                    break;
            }
        }
    };

    public BikeTransmitterController() {
        this.context = Ant.activity;
    }
    
    /**
     * Set the listener for control commands received from ANT+ devices
     */
    public void setControlCommandListener(ControlCommandListener listener) {
        this.controlListener = listener;
    }

    /**
     * Start transmitting bike data via ANT+
     */
    public boolean startTransmission() {
        try {
            // Request ANT+ channel
            antChannel = AntChannel.requestChannel(context, mChannelEventHandler);
            
            if (antChannel == null) {
                QLog.e(TAG, "Failed to get ANT+ channel");
                return false;
            }
            
            // Configure channel for fitness equipment transmission
            ChannelId channelId = new ChannelId(deviceNumber, DEVICE_TYPE_FITNESS_EQUIPMENT, TRANSMISSION_TYPE);
            
            antChannel.assign(ChannelType.BIDIRECTIONAL_MASTER);
            antChannel.setChannelId(channelId);
            antChannel.setPeriod(CHANNEL_PERIOD);
            antChannel.setRfFrequency(RADIO_FREQUENCY);
            
            // Open the channel
            antChannel.open();
            
            isTransmitting = true;
            
            // Start periodic data transmission (every 250ms for smooth updates)
            startPeriodicTransmission();
            
            QLog.d(TAG, "ANT+ bike transmission started successfully on channel " + antChannel.getChannelNumber());
            return true;
            
        } catch (AntCommandFailedException e) {
            QLog.e(TAG, "Failed to start ANT+ transmission: " + e.getMessage());
            return false;
        } catch (Exception e) {
            QLog.e(TAG, "Unexpected error starting ANT+ transmission: " + e.getMessage());
            return false;
        }
    }

    /**
     * Stop transmitting bike data
     */
    public void stopTransmission() {
        isTransmitting = false;
        
        if (transmissionTimer != null) {
            transmissionTimer.cancel();
            transmissionTimer = null;
        }
        
        if (antChannel != null) {
            try {
                antChannel.close();
                antChannel.release();
            } catch (AntCommandFailedException e) {
                QLog.w(TAG, "Error closing ANT+ channel: " + e.getMessage());
            }
            antChannel = null;
        }
        
        QLog.d(TAG, "ANT+ bike transmission stopped");
    }

    /**
     * Start periodic transmission of bike data
     */
    private void startPeriodicTransmission() {
        if (transmissionTimer != null) {
            transmissionTimer.cancel();
        }
        
        transmissionTimer = new Timer();
        transmissionTimer.scheduleAtFixedRate(new TimerTask() {
            @Override
            public void run() {
                if (isTransmitting && bikeSimulator != null) {
                    transmitCurrentData();
                }
            }
        }, 0, 250); // Transmit every 250ms
    }

    /**
     * Transmit current bike data via ANT+
     */
    private void transmitCurrentData() {
        if (!isTransmitting || antChannel == null) {
            return;
        }
        
        try {
            // Alternate between different data pages for complete information
            long currentTime = System.currentTimeMillis();
            int pageIndex = (int) ((currentTime / 250) % 4); // Switch page every 250ms
            
            byte[] dataPage = new byte[8];
            
            switch (pageIndex) {
                case 0:
                    // General Fitness Equipment Data Page (0x10)
                    buildGeneralFEDataPage(dataPage);
                    break;
                case 1:
                    // Bike Data Page (0x19)
                    buildBikeDataPage(dataPage);
                    break;
                case 2:
                    // Trainer Data Page (0x1A)
                    buildTrainerDataPage(dataPage);
                    break;
                case 3:
                    // General Metabolic Data Page (0x12)
                    buildMetabolicDataPage(dataPage);
                    break;
            }
            
            // Send the data page
            antChannel.broadcastData(dataPage);
            
            QLog.v(TAG, "Transmitted ANT+ data page: " + pageIndex);
                       
        } catch (AntCommandFailedException e) {
            QLog.e(TAG, "Error transmitting ANT+ data: " + e.getMessage());
        } catch (Exception e) {
            QLog.e(TAG, "Unexpected error transmitting ANT+ data: " + e.getMessage());
        }
    }
    
    /**
     * Build General Fitness Equipment Data Page (0x10)
     */
    private void buildGeneralFEDataPage(byte[] dataPage) {
        dataPage[0] = DATA_PAGE_GENERAL_FE;
        dataPage[1] = 0x19; // Equipment type: Bike
        
        // Elapsed time (0.25 second resolution)
        int elapsedTime025s = (int) (elapsedTime.doubleValue() * 4);
        dataPage[2] = (byte) (elapsedTime025s & 0xFF);
        
        // Distance (meters)
        int distanceMeters = (int) totalDistance;
        dataPage[3] = (byte) (distanceMeters & 0xFF);
        dataPage[4] = (byte) ((distanceMeters >> 8) & 0xFF);
        
        // Speed (0.001 m/s resolution)
        int speedMms = (int) (currentSpeed.doubleValue() * 1000);
        dataPage[5] = (byte) (speedMms & 0xFF);
        dataPage[6] = (byte) ((speedMms >> 8) & 0xFF);
        
        // Heart rate
        dataPage[7] = (byte) currentHeartRate;
    }
    
    /**
     * Build Bike Data Page (0x19)
     */
    private void buildBikeDataPage(byte[] dataPage) {
        dataPage[0] = DATA_PAGE_BIKE_DATA;
        dataPage[1] = 0xFF; // Reserved
        dataPage[2] = 0xFF; // Reserved
        dataPage[3] = 0xFF; // Reserved
        
        // Instantaneous cadence
        dataPage[4] = (byte) currentCadence;
        
        // Instantaneous power (watts)
        dataPage[5] = (byte) (currentPower & 0xFF);
        dataPage[6] = (byte) ((currentPower >> 8) & 0xFF);
        
        dataPage[7] = 0xFF; // Reserved
    }
    
    /**
     * Build Trainer Data Page (0x1A)
     */
    private void buildTrainerDataPage(byte[] dataPage) {
        dataPage[0] = DATA_PAGE_TRAINER_DATA;
        
        // Event count (increments on parameter changes)
        dataPage[1] = 0x00;
        
        // Instantaneous cadence
        dataPage[2] = (byte) currentCadence;
        
        // Instantaneous power (watts)
        dataPage[3] = (byte) (currentPower & 0xFF);
        dataPage[4] = (byte) ((currentPower >> 8) & 0xFF);
        
        // Resistance level (0.5% resolution)
        int resistance05 = (int) (currentResistance * 2);
        dataPage[5] = (byte) (resistance05 & 0xFF);
        
        // Target power (watts) - echo back any requested power
        dataPage[6] = (byte) (requestedPower & 0xFF);
        dataPage[7] = (byte) ((requestedPower >> 8) & 0xFF);
    }
    
    /**
     * Build General Metabolic Data Page (0x12)
     */
    private void buildMetabolicDataPage(byte[] dataPage) {
        dataPage[0] = DATA_PAGE_GENERAL_METABOLIC;
        dataPage[1] = 0xFF; // Reserved
        
        // Metabolic equivalent (METs) - 0.1 MET resolution
        // Estimate METs based on power and weight (assuming 75kg rider)
        double estimatedMETs = (currentPower * 0.014) + 1.0; // Rough estimation
        int mets01 = (int) (estimatedMETs * 10);
        dataPage[2] = (byte) (mets01 & 0xFF);
        
        // Calories per hour
        int caloriesPerHour = (int) (estimatedMETs * 75); // 75kg rider assumption
        dataPage[3] = (byte) (caloriesPerHour & 0xFF);
        dataPage[4] = (byte) ((caloriesPerHour >> 8) & 0xFF);
        
        // Calories accumulated
        dataPage[5] = (byte) (totalCalories & 0xFF);
        dataPage[6] = (byte) ((totalCalories >> 8) & 0xFF);
        
        dataPage[7] = 0xFF; // Reserved
    }
    
    /**
     * Handle incoming acknowledged data (control commands)
     */
    private void handleAcknowledgedData(AcknowledgedDataMessage message) {
        byte[] data = message.getMessageContent();
        if (data.length < 8) return;
        
        byte pageNumber = data[0];
        QLog.d(TAG, "Received acknowledged data page: 0x" + String.format("%02X", pageNumber));
        
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
    
    /**
     * Handle basic resistance command (page 0x30)
     */
    private void handleBasicResistanceCommand(byte[] data) {
        int resistance = data[7] & 0xFF; // Resistance in 0.5% increments
        double resistancePercent = resistance * 0.5;
        
        QLog.d(TAG, "Received basic resistance command: " + resistancePercent + "%");
        
        if (resistancePercent != requestedInclination && controlListener != null) {
            requestedResistance = (int) resistancePercent;
            controlListener.onResistanceChangeRequested(requestedResistance);
        }
    }
    
    /**
     * Handle target power command (page 0x31)
     */
    private void handleTargetPowerCommand(byte[] data) {
        int targetPower = ((data[7] & 0xFF) << 8) | (data[6] & 0xFF);
        
        QLog.d(TAG, "Received target power command: " + targetPower + "W");
        
        if (targetPower != requestedPower && controlListener != null) {
            requestedPower = targetPower;
            controlListener.onPowerChangeRequested(targetPower);
        }
    }
    
    /**
     * Handle track resistance command (page 0x33)
     */
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
    
    /**
     * Handle broadcast data (not typically used for control)
     */
    private void handleBroadcastData(BroadcastDataMessage message) {
        // Usually not used for receiving control commands
    }
    
    /**
     * Handle channel events
     */
    private void handleChannelEvent(ChannelEventMessage message) {
        QLog.d(TAG, "Channel event: " + message.getEventCode());
    }

    // Setter methods for updating bike metrics from the main application
    
    /**
     * Update the current cadence value
     * @param cadence Cadence in RPM
     */
    public void setCadence(int cadence) {
        this.currentCadence = Math.max(0, cadence);
    }

    /**
     * Update the current power value
     * @param power Power in watts
     */
    public void setPower(int power) {
        this.currentPower = Math.max(0, power);
    }

    /**
     * Update the current speed value
     * @param speedKph Speed in km/h
     */
    public void setSpeedKph(double speedKph) {
        // Convert km/h to m/s
        this.currentSpeed = new BigDecimal(speedKph / 3.6);
    }

    /**
     * Update the current speed value
     * @param speedMps Speed in m/s
     */
    public void setSpeedMps(double speedMps) {
        this.currentSpeed = new BigDecimal(Math.max(0, speedMps));
    }

    /**
     * Update the total distance
     * @param distance Total distance in meters
     */
    public void setDistance(long distance) {
        this.totalDistance = Math.max(0, distance);
    }

    /**
     * Update the total calories
     * @param calories Total calories burned
     */
    public void setCalories(long calories) {
        this.totalCalories = Math.max(0, calories);
    }

    /**
     * Update the current heart rate
     * @param heartRate Heart rate in BPM
     */
    public void setHeartRate(int heartRate) {
        this.currentHeartRate = Math.max(0, Math.min(255, heartRate));
    }

    /**
     * Update the elapsed time
     * @param timeSeconds Elapsed time in seconds
     */
    public void setElapsedTime(double timeSeconds) {
        this.elapsedTime = new BigDecimal(Math.max(0, timeSeconds));
    }

    /**
     * Update the current resistance level
     * @param resistance Resistance level (typically 0-100)
     */
    public void setResistance(int resistance) {
        this.currentResistance = Math.max(0, Math.min(100, resistance));
    }

    /**
     * Update the current inclination
     * @param inclination Inclination in percentage (-100 to +100)
     */
    public void setInclination(double inclination) {
        this.currentInclination = Math.max(-100, Math.min(100, inclination));
    }

    // Getter methods for the last requested control values
    
    /**
     * Get the last requested resistance from ANT+ controller
     * @return Requested resistance level, or -1 if none requested
     */
    public int getRequestedResistance() {
        return requestedResistance;
    }

    /**
     * Get the last requested power from ANT+ controller
     * @return Requested power in watts, or -1 if none requested
     */
    public int getRequestedPower() {
        return requestedPower;
    }

    /**
     * Get the last requested inclination from ANT+ controller
     * @return Requested inclination in percentage, or -100 if none requested
     */
    public double getRequestedInclination() {
        return requestedInclination;
    }

    /**
     * Clear any pending control requests
     */
    public void clearControlRequests() {
        requestedResistance = -1;
        requestedPower = -1;
        requestedInclination = -100;
    }

    /**
     * Check if the transmitter is currently active
     * @return True if transmitting, false otherwise
     */
    public boolean isTransmitting() {
        return isTransmitting;
    }

    /**
     * Get current transmission state info for debugging
     * @return String with current state information
     */
    public String getTransmissionInfo() {
        if (!isTransmitting) {
            return "Transmission: STOPPED";
        }
        
        return String.format("Transmission: ACTIVE - Cadence: %drpm, Power: %dW, " +
                           "Speed: %.1fkm/h, Resistance: %d, Inclination: %.1f%%",
                           currentCadence, currentPower, currentSpeed.doubleValue() * 3.6,
                           currentResistance, currentInclination);
    }
}