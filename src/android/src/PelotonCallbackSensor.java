package org.cagnulen.qdomyoszwift;

import android.os.IBinder;
import android.os.Parcel;
import android.os.RemoteException;

import org.cagnulen.qdomyoszwift.QLog;

/**
 * Callback-based sensor implementation from Grupetto v1 develop
 * Based on: https://github.com/selalipop/grupetto/pull/10
 * More efficient than polling - receives data only when it changes
 */
public abstract class PelotonCallbackSensor {
    
    private static final String TAG = "PelotonCallbackSensor";
    
    // Transaction codes from Grupetto v1 CallbackSensor.kt
    private static final int TRANSACTION_REGISTER_CALLBACK = 1;
    private static final int TRANSACTION_UNREGISTER_CALLBACK = 2;
    
    // Interface descriptors from Grupetto v1
    private static final String IV1_INTERFACE = "com.onepeloton.affernetservice.IV1Interface";
    private static final String IV1_CALLBACK_INTERFACE = "com.onepeloton.affernetservice.IV1Callback";
    
    private IBinder binder;
    private boolean isRegistered = false;
    private PelotonCallbackBinder callbackBinder;
    
    // Callback interface for receiving sensor data
    public interface SensorDataCallback {
        void onSensorDataReceived(float value);
        void onSensorError(long errorCode);
    }
    
    private SensorDataCallback callback;
    
    public PelotonCallbackSensor(IBinder binder) {
        this.binder = binder;
        this.callbackBinder = new PelotonCallbackBinder();
    }
    
    public void setCallback(SensorDataCallback callback) {
        this.callback = callback;
    }
    
    public void start() throws RemoteException {
        if (isRegistered) {
            QLog.w(TAG, "Sensor already started");
            return;
        }
        
        registerCallback();
        isRegistered = true;
        QLog.d(TAG, "Callback sensor started successfully");
    }
    
    public void stop() {
        if (!isRegistered) {
            return;
        }
        
        try {
            unregisterCallback();
            isRegistered = false;
            QLog.d(TAG, "Callback sensor stopped successfully");
        } catch (Exception e) {
            QLog.e(TAG, "Failed to stop callback sensor", e);
        }
    }
    
    private void registerCallback() throws RemoteException {
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        
        try {
            data.writeInterfaceToken(IV1_INTERFACE);
            data.writeStrongBinder(callbackBinder);
            data.writeString("QDomyos-Zwift"); // Identifier like Grupetto
            
            QLog.d(TAG, "Registering callback with interface: " + IV1_INTERFACE);
            boolean success = binder.transact(TRANSACTION_REGISTER_CALLBACK, data, reply, 0);
            if (success) {
                reply.readException();
                QLog.i(TAG, "Successfully registered callback");
            } else {
                throw new RemoteException("Failed to register callback");
            }
        } finally {
            data.recycle();
            reply.recycle();
        }
    }
    
    private void unregisterCallback() throws RemoteException {
        Parcel data = Parcel.obtain();
        Parcel reply = Parcel.obtain();
        
        try {
            data.writeInterfaceToken(IV1_INTERFACE);
            data.writeStrongBinder(callbackBinder);
            data.writeString("QDomyos-Zwift"); // Identifier like Grupetto
            
            boolean success = binder.transact(TRANSACTION_UNREGISTER_CALLBACK, data, reply, 0);
            if (success) {
                reply.readException();
                QLog.d(TAG, "Successfully unregistered callback");
            }
        } catch (Exception e) {
            QLog.w(TAG, "Error unregistering callback", e);
        } finally {
            data.recycle();
            reply.recycle();
        }
    }
    
    /**
     * Extract the specific sensor value from BikeData
     * Override in subclasses for different sensor types
     */
    protected abstract float extractValue(BikeData bikeData);
    
    /**
     * Apply sensor-specific value mapping
     * Override in subclasses if needed
     */
    protected float mapValue(float rawValue) {
        return rawValue;
    }
    
    /**
     * Binder implementation for receiving callbacks from Peloton service
     */
    private class PelotonCallbackBinder extends android.os.Binder {
        
        @Override
        protected boolean onTransact(int code, Parcel data, Parcel reply, int flags) throws RemoteException {
            QLog.d(TAG, "Callback onTransact called with code: " + code);
            
            switch (code) {
                case 1: // onSensorDataChange
                    try {
                        data.enforceInterface(IV1_CALLBACK_INTERFACE);
                        QLog.d(TAG, "Interface enforced successfully");
                        
                        int hasData = data.readInt();
                        QLog.d(TAG, "Has data flag: " + hasData);
                        
                        if (hasData != 0) {
                            QLog.d(TAG, "Creating BikeData from parcel");
                            BikeData bikeData = BikeData.CREATOR.createFromParcel(data);
                            
                            float rawValue = extractValue(bikeData);
                            float mappedValue = mapValue(rawValue);
                            
                            if (callback != null) {
                                callback.onSensorDataReceived(mappedValue);
                            }
                            
                            QLog.i(TAG, "Received sensor data: " + mappedValue);
                        } else {
                            QLog.d(TAG, "No bike data received");
                        }
                        return true;
                    } catch (Exception e) {
                        QLog.e(TAG, "Error processing sensor data", e);
                        return false;
                    }
                    
                case 2: // onSensorError
                    try {
                        data.enforceInterface(IV1_CALLBACK_INTERFACE);
                        long errorCode = data.readLong();
                        QLog.w(TAG, "Sensor error: " + errorCode);
                        
                        if (callback != null) {
                            callback.onSensorError(errorCode);
                        }
                        return true;
                    } catch (Exception e) {
                        QLog.e(TAG, "Error processing sensor error", e);
                        return false;
                    }
                    
                case 3: // onCalibrationStatus
                    try {
                        data.enforceInterface(IV1_CALLBACK_INTERFACE);
                        int status = data.readInt();
                        boolean success = data.readInt() != 0;
                        long errorCode = data.readLong();
                        QLog.d(TAG, "Calibration status: status=" + status + " success=" + success + " error=" + errorCode);
                        return true;
                    } catch (Exception e) {
                        QLog.e(TAG, "Error processing calibration status", e);
                        return false;
                    }
                    
                default:
                    QLog.d(TAG, "Unknown transaction code: " + code + ", calling super");
                    return super.onTransact(code, data, reply, flags);
            }
        }
    }
    
    /**
     * Power sensor implementation
     */
    public static class PowerSensor extends PelotonCallbackSensor {
        
        public PowerSensor(IBinder binder) {
            super(binder);
        }
        
        @Override
        protected float extractValue(BikeData bikeData) {
            return (float) bikeData.getPower();
        }
        
        @Override
        protected float mapValue(float rawValue) {
            // From Grupetto v1: divide by 100 to normalize power values
            float normalizedValue = rawValue / 100.0f;
            
            // Filter out spurious readings
            if (normalizedValue < 0 || normalizedValue > 1000) {
                QLog.w(TAG, "Filtering spurious power reading: " + normalizedValue);
                return 0.0f;
            }
            
            return normalizedValue;
        }
    }
    
    /**
     * RPM sensor implementation
     */
    public static class RpmSensor extends PelotonCallbackSensor {
        
        public RpmSensor(IBinder binder) {
            super(binder);
        }
        
        @Override
        protected float extractValue(BikeData bikeData) {
            return (float) bikeData.getRPM();
        }
    }
    
    /**
     * Resistance sensor implementation with moving window filtering
     */
    public static class ResistanceSensor extends PelotonCallbackSensor {
        
        // Moving window for resistance filtering (from Grupetto approach)
        private static final int FILTER_WINDOW_SIZE = 3;
        private float[] resistanceWindow = new float[FILTER_WINDOW_SIZE];
        private int windowIndex = 0;
        private boolean windowFilled = false;
        
        public ResistanceSensor(IBinder binder) {
            super(binder);
        }
        
        @Override
        protected float extractValue(BikeData bikeData) {
            return (float) bikeData.getTargetResistance();
        }
        
        @Override
        protected float mapValue(float rawValue) {
            // Add value to moving window
            resistanceWindow[windowIndex] = rawValue;
            windowIndex = (windowIndex + 1) % FILTER_WINDOW_SIZE;
            
            if (!windowFilled && windowIndex == 0) {
                windowFilled = true;
            }
            
            // If window not full yet, return current value
            if (!windowFilled) {
                return rawValue;
            }
            
            // Return minimum value from window (Grupetto strategy for spike filtering)
            float minValue = resistanceWindow[0];
            for (int i = 1; i < FILTER_WINDOW_SIZE; i++) {
                if (resistanceWindow[i] < minValue) {
                    minValue = resistanceWindow[i];
                }
            }
            
            return minValue;
        }
    }
}