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
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikePowerPcc;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikePowerPcc.IRawPowerOnlyDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikePowerPcc.ICalculatedPowerReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikeSpeedDistancePcc;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikeSpeedDistancePcc.CalculatedSpeedReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikeSpeedDistancePcc.CalculatedAccumulatedDistanceReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikeSpeedDistancePcc.IRawSpeedAndDistanceDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikeCadencePcc;
import com.dsi.ant.plugins.antplus.pcc.AntPlusBikeCadencePcc.ICalculatedCadenceReceiver;
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
    private AntPlusBikePowerPcc powerPcc = null;
    private PccReleaseHandle<AntPlusBikePowerPcc> powerReleaseHandle = null;
    private AntPlusBikeSpeedDistancePcc speedCadencePcc = null;
    private PccReleaseHandle<AntPlusBikeSpeedDistancePcc> speedCadenceReleaseHandle = null;
    private AntPlusBikeCadencePcc cadencePcc = null;
    private PccReleaseHandle<AntPlusBikeCadencePcc> cadenceReleaseHandle = null;
    private boolean isConnected = false;
    private boolean isPowerConnected = false;
    private boolean isSpeedCadenceConnected = false;

    // Bike data fields - from fitness equipment
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
    
    // Bike data fields - from dedicated sensors
    public int powerSensorPower = 0;     // Power from dedicated power sensor
    public int speedSensorCadence = 0;   // Cadence from speed/cadence sensor
    public BigDecimal speedSensorSpeed = new BigDecimal(0); // Speed from speed/cadence sensor
    public long speedSensorDistance = 0; // Distance from speed/cadence sensor

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
        //openChannel();
        openPowerSensorChannel();
        //openSpeedCadenceSensorChannel();
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

    public boolean openPowerSensorChannel() {
        // Request access to first available power sensor device
        powerReleaseHandle = AntPlusBikePowerPcc.requestAccess((Activity)context, 0, 0,
            new IPluginAccessResultReceiver<AntPlusBikePowerPcc>() {
                @Override
                public void onResultReceived(AntPlusBikePowerPcc result, RequestAccessResult resultCode, DeviceState initialDeviceState) {
                    switch(resultCode) {
                        case SUCCESS:
                            powerPcc = result;
                            isPowerConnected = true;
                            QLog.d(TAG, "Connected to power sensor: " + result.getDeviceName());
                            subscribeToPowerSensorEvents();
                            break;
                        case CHANNEL_NOT_AVAILABLE:
                            QLog.e(TAG, "Power Sensor Channel Not Available");
                            break;
                        case ADAPTER_NOT_DETECTED:
                            QLog.e(TAG, "ANT Adapter Not Available for Power Sensor");
                            break;
                        case BAD_PARAMS:
                            QLog.e(TAG, "Bad request parameters for Power Sensor");
                            break;
                        case OTHER_FAILURE:
                            QLog.e(TAG, "Power Sensor RequestAccess failed");
                            break;
                        case DEPENDENCY_NOT_INSTALLED:
                            QLog.e(TAG, "Dependency not installed for Power Sensor");
                            break;
                        case USER_CANCELLED:
                            QLog.e(TAG, "User cancelled Power Sensor");
                            break;
                        default:
                            QLog.e(TAG, "Unrecognized power sensor result: " + resultCode);
                            break;
                    }
                }
            },
            new IDeviceStateChangeReceiver() {
                @Override
                public void onDeviceStateChange(DeviceState newDeviceState) {
                    QLog.d(TAG, "Power Sensor State Changed to: " + newDeviceState);
                    if (newDeviceState == DeviceState.DEAD) {
                        isPowerConnected = false;
                    }
                }
            }
        );
        return isPowerConnected;
    }

    public boolean openSpeedCadenceSensorChannel() {
        // Request access to first available speed/cadence sensor device
        speedCadenceReleaseHandle = AntPlusBikeSpeedDistancePcc.requestAccess((Activity)context, context,
            new IPluginAccessResultReceiver<AntPlusBikeSpeedDistancePcc>() {
                @Override
                public void onResultReceived(AntPlusBikeSpeedDistancePcc result, RequestAccessResult resultCode, DeviceState initialDeviceState) {
                    switch(resultCode) {
                        case SUCCESS:
                            speedCadencePcc = result;
                            isSpeedCadenceConnected = true;
                            QLog.d(TAG, "Connected to speed/cadence sensor: " + result.getDeviceName());
                            subscribeToSpeedCadenceSensorEvents();
                            break;
                        case CHANNEL_NOT_AVAILABLE:
                            QLog.e(TAG, "Speed/Cadence Sensor Channel Not Available");
                            break;
                        case ADAPTER_NOT_DETECTED:
                            QLog.e(TAG, "ANT Adapter Not Available for Speed/Cadence Sensor");
                            break;
                        case BAD_PARAMS:
                            QLog.e(TAG, "Bad request parameters for Speed/Cadence Sensor");
                            break;
                        case OTHER_FAILURE:
                            QLog.e(TAG, "Speed/Cadence Sensor RequestAccess failed");
                            break;
                        case DEPENDENCY_NOT_INSTALLED:
                            QLog.e(TAG, "Dependency not installed for Speed/Cadence Sensor");
                            break;
                        case USER_CANCELLED:
                            QLog.e(TAG, "User cancelled Speed/Cadence Sensor");
                            break;
                        default:
                            QLog.e(TAG, "Unrecognized speed/cadence sensor result: " + resultCode);
                            break;
                    }
                }
            },
            new IDeviceStateChangeReceiver() {
                @Override
                public void onDeviceStateChange(DeviceState newDeviceState) {
                    QLog.d(TAG, "Speed/Cadence Sensor State Changed to: " + newDeviceState);
                    if (newDeviceState == DeviceState.DEAD) {
                        isSpeedCadenceConnected = false;
                    }
                }
            }
        );
        return isSpeedCadenceConnected;
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

    private void subscribeToPowerSensorEvents() {
        if (powerPcc != null) {
            // Subscribe to raw power-only data events
            powerPcc.subscribeRawPowerOnlyDataEvent(new IRawPowerOnlyDataReceiver() {
                @Override
                public void onNewRawPowerOnlyData(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                long powerOnlyUpdateEventCount, int instantaneousPower,
                                                long accumulatedPower) {
                    if (instantaneousPower != -1) {
                        powerSensorPower = instantaneousPower;
                        QLog.d(TAG, "Power Sensor Data - Power: " + powerSensorPower + "W");
                    }
                }
            });

            // Also subscribe to calculated power events
            powerPcc.subscribeCalculatedPowerEvent(new ICalculatedPowerReceiver() {
                @Override
                public void onNewCalculatedPower(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                AntPlusBikePowerPcc.DataSource dataSource,
                                                BigDecimal calculatedPower) {
                    if (calculatedPower != null && calculatedPower.intValue() != -1) {
                        powerSensorPower = calculatedPower.intValue();
                        QLog.d(TAG, "Power Sensor Calculated Data - Power: " + powerSensorPower + "W");
                    }
                }
            });
        }
    }

    private void subscribeToSpeedCadenceSensorEvents() {
        if (speedCadencePcc != null) {
            // 2.095m circumference = average 700cx23mm road tire
            BigDecimal wheelCircumference = new BigDecimal("2.095");
            
            // Subscribe to calculated speed events
            speedCadencePcc.subscribeCalculatedSpeedEvent(new CalculatedSpeedReceiver(wheelCircumference) {
                @Override
                public void onNewCalculatedSpeed(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                BigDecimal calculatedSpeed) {
                    if (calculatedSpeed != null && calculatedSpeed.doubleValue() > 0) {
                        speedSensorSpeed = calculatedSpeed;
                        QLog.d(TAG, "Speed Sensor Data - Speed: " + speedSensorSpeed + "m/s");
                    }
                }
            });

            // Subscribe to calculated distance events
            speedCadencePcc.subscribeCalculatedAccumulatedDistanceEvent(new CalculatedAccumulatedDistanceReceiver(wheelCircumference) {
                @Override
                public void onNewCalculatedAccumulatedDistance(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                             BigDecimal calculatedAccumulatedDistance) {
                    if (calculatedAccumulatedDistance != null && calculatedAccumulatedDistance.longValue() > 0) {
                        speedSensorDistance = calculatedAccumulatedDistance.longValue();
                        QLog.d(TAG, "Speed Sensor Data - Distance: " + speedSensorDistance + "m");
                    }
                }
            });

            // Subscribe to raw speed and distance data
            speedCadencePcc.subscribeRawSpeedAndDistanceDataEvent(new IRawSpeedAndDistanceDataReceiver() {
                @Override
                public void onNewRawSpeedAndDistanceData(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                        BigDecimal timestampOfLastEvent, long cumulativeRevolutions) {
                    QLog.d(TAG, "Speed/Distance Raw Data - Revs: " + cumulativeRevolutions + ", Time: " + timestampOfLastEvent);
                }
            });

            // Check if this is a combined speed/cadence sensor
            if (speedCadencePcc.isSpeedAndCadenceCombinedSensor()) {
                // Connect to cadence functionality
                cadenceReleaseHandle = AntPlusBikeCadencePcc.requestAccess(
                    (Activity)context, speedCadencePcc.getAntDeviceNumber(), 0, true,
                    new IPluginAccessResultReceiver<AntPlusBikeCadencePcc>() {
                        @Override
                        public void onResultReceived(AntPlusBikeCadencePcc result, RequestAccessResult resultCode, DeviceState initialDeviceState) {
                            if (resultCode == RequestAccessResult.SUCCESS) {
                                cadencePcc = result;
                                cadencePcc.subscribeCalculatedCadenceEvent(new ICalculatedCadenceReceiver() {
                                    @Override
                                    public void onNewCalculatedCadence(long estTimestamp, EnumSet<EventFlag> eventFlags,
                                                                      BigDecimal calculatedCadence) {
                                        if (calculatedCadence != null && calculatedCadence.intValue() > 0) {
                                            speedSensorCadence = calculatedCadence.intValue();
                                            QLog.d(TAG, "Cadence Sensor Data - Cadence: " + speedSensorCadence + "rpm");
                                        }
                                    }
                                });
                            }
                        }
                    },
                    new IDeviceStateChangeReceiver() {
                        @Override
                        public void onDeviceStateChange(DeviceState newDeviceState) {
                            QLog.d(TAG, "Cadence Sensor State Changed to: " + newDeviceState);
                        }
                    }
                );
            }
        }
    }

    public void close() {
        if (releaseHandle != null) {
            releaseHandle.close();
            releaseHandle = null;
        }
        if (powerReleaseHandle != null) {
            powerReleaseHandle.close();
            powerReleaseHandle = null;
        }
        if (speedCadenceReleaseHandle != null) {
            speedCadenceReleaseHandle.close();
            speedCadenceReleaseHandle = null;
        }
        if (cadenceReleaseHandle != null) {
            cadenceReleaseHandle.close();
            cadenceReleaseHandle = null;
        }
        fePcc = null;
        powerPcc = null;
        speedCadencePcc = null;
        cadencePcc = null;
        isConnected = false;
        isPowerConnected = false;
        isSpeedCadenceConnected = false;
        QLog.d(TAG, "All Channels Closed");
    }

    // Getter methods for bike data with sensor reconciliation
    public int getCadence() {
        // Priority: 1) Fitness Equipment, 2) Speed/Cadence Sensor, 3) Power Sensor
        if (isConnected && cadence > 0) {
            return cadence; // From fitness equipment
        } else if (isSpeedCadenceConnected && speedSensorCadence > 0) {
            return speedSensorCadence; // From dedicated speed/cadence sensor
        } else if (isPowerConnected && speedSensorCadence > 0) {
            return speedSensorCadence; // From power sensor (if it provides cadence)
        }
        return 0;
    }

    public int getPower() {
        // Priority: 1) Dedicated Power Sensor, 2) Fitness Equipment
        if (isPowerConnected && powerSensorPower > 0) {
            return powerSensorPower; // From dedicated power sensor (most accurate)
        } else if (isConnected && power > 0) {
            return power; // From fitness equipment
        }
        return 0;
    }

    public double getSpeedKph() {
        // Convert from m/s to km/h
        return getSpeedMps() * 3.6;
    }

    public double getSpeedMps() {
        // Priority: 1) Speed/Cadence Sensor, 2) Fitness Equipment
        if (isSpeedCadenceConnected && speedSensorSpeed.doubleValue() > 0) {
            return speedSensorSpeed.doubleValue(); // From dedicated speed sensor (most accurate)
        } else if (isConnected && speed.doubleValue() > 0) {
            return speed.doubleValue(); // From fitness equipment
        }
        return 0.0;
    }

    public long getDistance() {
        // Priority: 1) Speed/Cadence Sensor, 2) Fitness Equipment
        if (isSpeedCadenceConnected && speedSensorDistance > 0) {
            return speedSensorDistance; // From dedicated speed sensor (most accurate)
        } else if (isConnected && distance > 0) {
            return distance; // From fitness equipment
        }
        return 0;
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

    // Additional connection status methods
    public boolean isPowerSensorConnected() {
        return isPowerConnected;
    }

    public boolean isSpeedCadenceSensorConnected() {
        return isSpeedCadenceConnected;
    }

    public boolean isAnyDeviceConnected() {
        return isConnected || isPowerConnected || isSpeedCadenceConnected;
    }

    // Raw sensor data getters (for debugging/advanced use)
    public int getRawFitnessEquipmentPower() {
        return power;
    }

    public int getRawPowerSensorPower() {
        return powerSensorPower;
    }

    public int getRawFitnessEquipmentCadence() {
        return cadence;
    }

    public int getRawSpeedSensorCadence() {
        return speedSensorCadence;
    }

    public double getRawFitnessEquipmentSpeed() {
        return speed.doubleValue();
    }

    public double getRawSpeedSensorSpeed() {
        return speedSensorSpeed.doubleValue();
    }

    public long getRawFitnessEquipmentDistance() {
        return distance;
    }

    public long getRawSpeedSensorDistance() {
        return speedSensorDistance;
    }
}
