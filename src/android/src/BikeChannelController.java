package org.cagnulen.qdomyoszwift;

import android.content.Context;
import org.cagnulen.qdomyoszwift.QLog;
import android.app.Activity;

// ANT+ Plugin imports
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.IFitnessEquipmentStateReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.IBikeDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.IGeneralFitnessEquipmentDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.EquipmentState;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.EquipmentType;
import com.dsi.ant.plugins.antplus.pcc.AntPlusFitnessEquipmentPcc.HeartRateDataSource;
import com.dsi.ant.plugins.antplus.pcc.defines.DeviceState;
import com.dsi.ant.plugins.antplus.pcc.defines.EventFlag;
import com.dsi.ant.plugins.antplus.pcc.defines.RequestAccessResult;
import com.dsi.ant.plugins.antplus.pccbase.AntPluginPcc.IDeviceStateChangeReceiver;
import com.dsi.ant.plugins.antplus.pccbase.AntPluginPcc.IPluginAccessResultReceiver;
import com.dsi.ant.plugins.antplus.pccbase.PccReleaseHandle;

// Java imports
import java.math.BigDecimal;
import java.util.EnumSet;

public class BikeChannelController {
    private static final String TAG = BikeChannelController.class.getSimpleName();

    private Context context;
    private AntPlusFitnessEquipmentPcc fePcc = null;
    private PccReleaseHandle<AntPlusFitnessEquipmentPcc> releaseHandle = null;
    private boolean isConnected = false;

    // Bike data fields
    public int cadence = 0;           // Current cadence in RPM
    public int power = 0;             // Current power in watts
    public BigDecimal speed = new BigDecimal(0); // Current speed in m/s
    public long distance = 0;         // Total distance in meters
    public long calories = 0;         // Total calories burned
    public EquipmentType equipmentType = EquipmentType.UNKNOWN;
    public EquipmentState equipmentState = EquipmentState.ASLEEP_OFF;
    public int heartRate = 0;         // Heart rate from equipment
    public HeartRateDataSource heartRateSource = HeartRateDataSource.UNKNOWN;
    public BigDecimal elapsedTime = new BigDecimal(0); // Elapsed time in seconds

    // Fitness equipment state receiver
    private final IFitnessEquipmentStateReceiver mFitnessEquipmentStateReceiver =
        new IFitnessEquipmentStateReceiver() {
            @Override
            public void onNewFitnessEquipmentState(long estTimestamp,
                                                 EnumSet<EventFlag> eventFlags,
                                                 EquipmentType type,
                                                 EquipmentState state) {
                equipmentType = type;
                equipmentState = state;
                QLog.d(TAG, "Equipment type: " + type + ", State: " + state);

                // Only subscribe to bike specific data if this is actually a bike
                if (type == EquipmentType.BIKE && !isSubscribedToBikeData) {
                    subscribeToBikeSpecificData();
                    isSubscribedToBikeData = true;
                }
            }
        };

    public BikeChannelController() {
        this.context = Ant.activity;
        openChannel();
    }

    public boolean openChannel() {
        // Request access to first available fitness equipment device
        // Using requestNewOpenAccess from the sample code
        releaseHandle = AntPlusFitnessEquipmentPcc.requestNewOpenAccess(
            (Activity)context,
            context,
            new IPluginAccessResultReceiver<AntPlusFitnessEquipmentPcc>() {
                @Override
                public void onResultReceived(AntPlusFitnessEquipmentPcc result, RequestAccessResult resultCode, DeviceState initialDeviceState) {
                    switch(resultCode) {
                        case SUCCESS:
                            fePcc = result;
                            isConnected = true;
                            QLog.d(TAG, "Connected to fitness equipment: " + result.getDeviceName());
                            subscribeToBikeEvents();
                            break;
                        case CHANNEL_NOT_AVAILABLE:
                            QLog.e(TAG, "Channel Not Available");
                            break;
                        case ADAPTER_NOT_DETECTED:
                            QLog.e(TAG, "ANT Adapter Not Available");
                            break;
                        case BAD_PARAMS:
                            QLog.e(TAG, "Bad request parameters");
                            break;
                        case OTHER_FAILURE:
                            QLog.e(TAG, "RequestAccess failed");
                            break;
                        case DEPENDENCY_NOT_INSTALLED:
                            QLog.e(TAG, "Dependency not installed");
                            break;
                        case USER_CANCELLED:
                            QLog.e(TAG, "User cancelled");
                            break;
                        default:
                            QLog.e(TAG, "Unrecognized result: " + resultCode);
                            break;
                    }
                }
            },
            new IDeviceStateChangeReceiver() {
                @Override
                public void onDeviceStateChange(DeviceState newDeviceState) {
                    QLog.d(TAG, "Device State Changed to: " + newDeviceState);
                    if (newDeviceState == DeviceState.DEAD) {
                        isConnected = false;
                    }
                }
            },
            mFitnessEquipmentStateReceiver
        );

        return isConnected;
    }

    private void subscribeToBikeEvents() {
        if (fePcc != null) {
            // General fitness equipment data
            fePcc.subscribeGeneralFitnessEquipmentDataEvent(new IGeneralFitnessEquipmentDataReceiver() {
                @Override
                public void onNewGeneralFitnessEquipmentData(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                           BigDecimal elapsedTime, long cumulativeDistance,
                                                           BigDecimal instantaneousSpeed, boolean virtualInstantaneousSpeed,
                                                           int instantaneousHeartRate, HeartRateDataSource source) {

                    if (elapsedTime != null && elapsedTime.intValue() != -1) {
                        BikeChannelController.this.elapsedTime = elapsedTime;
                    }

                    if (cumulativeDistance != -1) {
                        distance = cumulativeDistance;
                    }

                    if (instantaneousSpeed != null && instantaneousSpeed.intValue() != -1) {
                        speed = instantaneousSpeed;
                    }

                    if (instantaneousHeartRate != -1) {
                        heartRate = instantaneousHeartRate;
                        heartRateSource = source;
                    }

                    QLog.d(TAG, "General Data - Time: " + elapsedTime + "s, Distance: " +
                          distance + "m, Speed: " + speed + "m/s, HR: " + heartRate + "bpm");
                }
            });
        }
    }

    private boolean isSubscribedToBikeData = false;

    private void subscribeToBikeSpecificData() {
        if (fePcc != null) {
            // Subscribe to bike specific data
            fePcc.getBikeMethods().subscribeBikeDataEvent(new IBikeDataReceiver() {
                @Override
                public void onNewBikeData(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                        int instantaneousCadence, int instantaneousPower) {

                    if (instantaneousCadence != -1) {
                        cadence = instantaneousCadence;
                    }

                    if (instantaneousPower != -1) {
                        power = instantaneousPower;
                    }

                    QLog.d(TAG, "Bike Data - Cadence: " + cadence + "rpm, Power: " + power + "W");
                }
            });
        }
    }

    public void close() {
        if (releaseHandle != null) {
            releaseHandle.close();
            releaseHandle = null;
        }
        fePcc = null;
        isConnected = false;
        QLog.d(TAG, "Channel Closed");
    }

    // Getter methods for bike data
    public int getCadence() {
        return cadence;
    }

    public int getPower() {
        return power;
    }

    public double getSpeedKph() {
        // Convert from m/s to km/h
        return speed.doubleValue() * 3.6;
    }

    public double getSpeedMps() {
        return speed.doubleValue();
    }

    public long getDistance() {
        return distance;
    }

    public long getCalories() {
        return calories;
    }

    public int getHeartRate() {
        return heartRate;
    }

    public BigDecimal getElapsedTime() {
        return elapsedTime;
    }

    public EquipmentState getEquipmentState() {
        return equipmentState;
    }

    public EquipmentType getEquipmentType() {
        return equipmentType;
    }

    public boolean isConnected() {
        return isConnected;
    }
}
