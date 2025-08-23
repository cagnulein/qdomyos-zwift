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
    
    // Peloton service constants (from Grupetto v1 develop - callback-based)
    private static final String SERVICE_ACTION = "com.onepeloton.affernetservice.IV1Interface";
    private static final String SERVICE_PACKAGE = "com.onepeloton.affernetservice";
    private static final String SERVICE_INTENT = "com.onepeloton.affernetservice.AffernetService";
    
    // Using callback-based sensors from Grupetto v1 develop
    // No transaction codes needed here - handled by PelotonCallbackSensor
    
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
        
        Intent intent = new Intent(SERVICE_INTENT);
        intent.setAction(SERVICE_ACTION);
        intent.setPackage(SERVICE_PACKAGE);
        
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
         * Reads sensor value from the Peloton service using Grupetto protocol
         */
        public float readValue() throws RemoteException {
            if (binder == null) {
                throw new RemoteException("Service binder is null");
            }
            
            Parcel data = Parcel.obtain();
            Parcel reply = Parcel.obtain();
            
            try {
                // Prepare the transaction (from BikePlusSensor.kt line 73-74)
                data.writeInterfaceToken(SERVICE_ACTION);
                
                // Execute the binder transaction (transaction code 14)
                boolean result = binder.transact(TRANSACTION_GET_BIKE_DATA, data, reply, 0);
                
                if (!result) {
                    throw new RemoteException("Binder transaction failed");
                }
                
                // Check for exceptions
                reply.readException();
                
                // Skip the first integer (from BikePlusSensor.kt line 77)
                reply.readInt();
                
                // Read BikeData from parcel (line 78)
                BikeData bikeData = BikeData.CREATOR.createFromParcel(reply);
                
                // Extract sensor-specific value and apply mapping
                float rawValue = extractSensorValue(bikeData);
                return mapValue(rawValue);
                
            } finally {
                data.recycle();
                reply.recycle();
            }
        }
        
        /**
         * Extract the appropriate sensor value from BikeData
         */
        protected abstract float extractSensorValue(BikeData bikeData);
        
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
            super(binder, 0); // Not used in new protocol
        }
        
        @Override
        protected float extractSensorValue(BikeData bikeData) {
            return (float) bikeData.getPower();
        }
        
        @Override
        protected float mapValue(float rawValue) {
            // Power values are already in correct units from BikeData
            // Filter out spurious readings
            if (rawValue < 0 || rawValue > 1000) {
                QLog.w(TAG, "Filtering spurious power reading: " + rawValue);
                return 0.0f;
            }
            
            return rawValue;
        }
    }
    
    /**
     * RPM (cadence) sensor implementation based on Grupetto's RpmSensor.kt
     */
    public static class RpmSensor extends PelotonSensor {
        
        public RpmSensor(IBinder binder) {
            super(binder, 0); // Not used in new protocol
        }
        
        @Override
        protected float extractSensorValue(BikeData bikeData) {
            return (float) bikeData.getRPM();
        }
        
        @Override
        protected float mapValue(float rawValue) {
            // RPM values are already in correct units from BikeData
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
            super(binder, 0); // Not used in new protocol
        }
        
        @Override
        protected float extractSensorValue(BikeData bikeData) {
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
    
    public boolean isConnected() {
        return isConnected;
    }
    
    public IBinder getServiceBinder() {
        return serviceBinder;
    }
}