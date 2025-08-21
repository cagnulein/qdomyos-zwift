package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.Parcel;
import android.util.Log;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;
import java.util.concurrent.TimeUnit;

import org.cagnulen.qdomyoszwift.QLog;

public class PelotonSensorBinder {
    
    private static final String TAG = "PelotonSensorBinder";
    
    // Peloton service constants (based on Grupetto analysis)
    private static final String PELOTON_SENSOR_ACTION = "android.intent.action.peloton.SensorData";
    
    // Binder transaction codes (these would need to be discovered from actual Peloton service)
    private static final int TRANSACTION_GET_POWER_REPEATING = 1;
    private static final int TRANSACTION_GET_RPM_REPEATING = 2;
    private static final int TRANSACTION_GET_RESISTANCE_REPEATING = 3;
    
    private Context context;
    private IBinder serviceBinder = null;
    private boolean isConnected = false;
    
    public PelotonSensorBinder(Context context) {
        this.context = context;
    }
    
    /**
     * Asynchronously connects to the Peloton sensor service
     * Based on Grupetto's Binder.kt implementation
     */
    public CompletableFuture<IBinder> getBinder() {
        CompletableFuture<IBinder> future = new CompletableFuture<>();
        CountDownLatch connectionLatch = new CountDownLatch(1);
        
        ServiceConnection serviceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                QLog.i(TAG, "Service connected: " + name.getClassName());
                serviceBinder = service;
                isConnected = true;
                future.complete(service);
                connectionLatch.countDown();
            }
            
            @Override
            public void onServiceDisconnected(ComponentName name) {
                QLog.w(TAG, "Service disconnected: " + name.getClassName());
                serviceBinder = null;
                isConnected = false;
            }
            
            @Override
            public void onBindingDied(ComponentName name) {
                QLog.e(TAG, "Service binding died: " + name.getClassName());
                serviceBinder = null;
                isConnected = false;
                if (!future.isDone()) {
                    future.completeExceptionally(new RuntimeException("Service binding died"));
                }
            }
        };
        
        Intent intent = new Intent();
        intent.setAction(PELOTON_SENSOR_ACTION);
        
        boolean bound = context.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
        
        if (!bound) {
            QLog.e(TAG, "Failed to bind to Peloton sensor service");
            future.completeExceptionally(new RuntimeException("Failed to bind to service"));
            return future;
        }
        
        QLog.i(TAG, "Binding to Peloton sensor service...");
        return future;
    }
    
    /**
     * Base sensor class similar to Grupetto's Sensor.kt
     */
    public abstract static class PelotonSensor {
        protected IBinder binder;
        protected int transactionCode;
        
        public PelotonSensor(IBinder binder, int transactionCode) {
            this.binder = binder;
            this.transactionCode = transactionCode;
        }
        
        /**
         * Reads sensor value from the Peloton service
         */
        public float readValue() throws RemoteException {
            if (binder == null) {
                throw new RemoteException("Service binder is null");
            }
            
            Parcel data = Parcel.obtain();
            Parcel reply = Parcel.obtain();
            
            try {
                // Prepare the transaction
                data.writeInterfaceToken("com.onepeloton.callisto.sensors.ISensorService");
                
                // Execute the binder transaction
                boolean result = binder.transact(transactionCode, data, reply, 0);
                
                if (!result) {
                    throw new RemoteException("Binder transaction failed");
                }
                
                // Check for exceptions
                reply.readException();
                
                // Read the sensor value
                float rawValue = reply.readFloat();
                
                // Apply sensor-specific value mapping
                return mapValue(rawValue);
                
            } finally {
                data.recycle();
                reply.recycle();
            }
        }
        
        /**
         * Maps raw sensor value to application-specific value
         * Override in subclasses for sensor-specific processing
         */
        protected abstract float mapValue(float rawValue);
    }
    
    /**
     * Power sensor implementation based on Grupetto's PowerSensor.kt
     */
    public static class PowerSensor extends PelotonSensor {
        
        public PowerSensor(IBinder binder) {
            super(binder, TRANSACTION_GET_POWER_REPEATING);
        }
        
        @Override
        protected float mapValue(float rawValue) {
            // Based on Grupetto: divide by 100 to normalize power values
            // Also handle spurious readings at low power
            float normalizedValue = rawValue / 100.0f;
            
            // Filter out spurious low readings (Grupetto comment about low power spikes)
            if (normalizedValue < 0 || normalizedValue > 1000) {
                QLog.w(TAG, "Filtering spurious power reading: " + normalizedValue);
                return 0.0f;
            }
            
            return normalizedValue;
        }
    }
    
    /**
     * RPM (cadence) sensor implementation based on Grupetto's RpmSensor.kt
     */
    public static class RpmSensor extends PelotonSensor {
        
        public RpmSensor(IBinder binder) {
            super(binder, TRANSACTION_GET_RPM_REPEATING);
        }
        
        @Override
        protected float mapValue(float rawValue) {
            // Based on Grupetto: return value directly without transformation
            return rawValue;
        }
    }
    
    /**
     * Resistance sensor implementation based on Grupetto's ResistanceSensor.kt
     * Includes moving window filtering for ADC spike mitigation
     */
    public static class ResistanceSensor extends PelotonSensor {
        
        // Moving window for resistance filtering (based on Grupetto approach)
        private static final int FILTER_WINDOW_SIZE = 3;
        private float[] resistanceWindow = new float[FILTER_WINDOW_SIZE];
        private int windowIndex = 0;
        private boolean windowFilled = false;
        
        public ResistanceSensor(IBinder binder) {
            super(binder, TRANSACTION_GET_RESISTANCE_REPEATING);
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
    
    public boolean isConnected() {
        return isConnected;
    }
    
    public IBinder getServiceBinder() {
        return serviceBinder;
    }
}