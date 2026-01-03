package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.content.ComponentName;
import android.os.IBinder;
import android.os.RemoteException;
import android.os.Handler;
import android.os.Looper;
import android.util.Log;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;

import org.cagnulen.qdomyoszwift.QLog;

public class PelotonSensorService {
    
    private static final String TAG = "PelotonSensorService";
    
    // Singleton instance for static access
    private static PelotonSensorService instance = null;
    private static Context staticContext = null;
    
    // Peloton service action and permissions
    private static final String PELOTON_SENSOR_ACTION = "android.intent.action.peloton.SensorData";
    private static final String PELOTON_SENSOR_PERMISSION = "onepeloton.permission.ACCESS_SENSOR_SERVICE";
    
    // Update interval for sensor reading
    private static final int SENSOR_UPDATE_INTERVAL_MS = 200;
    
    // Threading components
    private Handler mainHandler;
    private ExecutorService executorService;
    private Runnable sensorUpdateRunnable;
    
    // Service connection components
    private IBinder sensorBinder = null;
    private boolean isServiceConnected = false;
    private boolean isUpdating = false;
    
    // Sensor components (similar to Grupetto's implementation)
    private PelotonPowerSensor powerSensor;
    private PelotonRpmSensor rpmSensor;
    private PelotonResistanceSensor resistanceSensor;
    
    // Current sensor values
    private volatile float currentPower = 0.0f;
    private volatile float currentCadence = 0.0f;
    private volatile float currentResistance = 0.0f;
    private volatile float currentSpeed = 0.0f;
    
    // Context for service binding
    private Context context;
    
    public PelotonSensorService(Context context) {
        this.context = context;
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.executorService = Executors.newSingleThreadExecutor();
    }
    
    private void initializeInstance() throws Exception {
        QLog.i(TAG, "Initializing Peloton sensor service connection...");
        
        // Check if required permission is available
        if (context.checkSelfPermission(PELOTON_SENSOR_PERMISSION) != 
            android.content.pm.PackageManager.PERMISSION_GRANTED) {
            throw new Exception("Missing required permission: " + PELOTON_SENSOR_PERMISSION);
        }
        
        // Connect to Peloton sensor service
        connectToSensorService();
    }
    
    private void connectToSensorService() throws Exception {
        QLog.i(TAG, "Attempting to connect to Peloton sensor service...");
        
        CompletableFuture<IBinder> binderFuture = new CompletableFuture<>();
        CountDownLatch connectionLatch = new CountDownLatch(1);
        
        ServiceConnection serviceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                QLog.i(TAG, "Connected to Peloton sensor service");
                sensorBinder = service;
                isServiceConnected = true;
                binderFuture.complete(service);
                connectionLatch.countDown();
                
                // Initialize sensor components
                try {
                    initializeSensors();
                } catch (Exception e) {
                    QLog.e(TAG, "Failed to initialize sensors", e);
                }
            }
            
            @Override
            public void onServiceDisconnected(ComponentName name) {
                QLog.w(TAG, "Disconnected from Peloton sensor service");
                sensorBinder = null;
                isServiceConnected = false;
                isUpdating = false;
            }
            
            @Override
            public void onBindingDied(ComponentName name) {
                QLog.e(TAG, "Peloton sensor service binding died");
                sensorBinder = null;
                isServiceConnected = false;
                isUpdating = false;
            }
        };
        
        Intent intent = new Intent();
        intent.setAction(PELOTON_SENSOR_ACTION);
        
        boolean bound = context.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
        
        if (!bound) {
            throw new Exception("Failed to bind to Peloton sensor service");
        }
        
        // Wait for connection with timeout
        try {
            if (!connectionLatch.await(10, TimeUnit.SECONDS)) {
                throw new Exception("Timeout waiting for Peloton sensor service connection");
            }
        } catch (InterruptedException e) {
            throw new Exception("Interrupted while waiting for service connection", e);
        }
    }
    
    private void initializeSensors() throws Exception {
        if (sensorBinder == null) {
            throw new Exception("Service binder not available");
        }
        
        // Initialize individual sensor components (similar to Grupetto approach)
        powerSensor = new PelotonPowerSensor(sensorBinder);
        rpmSensor = new PelotonRpmSensor(sensorBinder);
        resistanceSensor = new PelotonResistanceSensor(sensorBinder);
        
        QLog.i(TAG, "All sensors initialized successfully");
    }
    
    private void startSensorUpdatesInstance() {
        if (isUpdating || !isServiceConnected) {
            QLog.w(TAG, "Cannot start sensor updates - service not ready");
            return;
        }
        
        isUpdating = true;
        
        sensorUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                if (!isUpdating || !isServiceConnected) return;
                
                executorService.execute(() -> {
                    try {
                        // Read all sensor values
                        if (powerSensor != null) {
                            currentPower = powerSensor.readValue();
                        }
                        if (rpmSensor != null) {
                            currentCadence = rpmSensor.readValue();
                        }
                        if (resistanceSensor != null) {
                            currentResistance = resistanceSensor.readValue();
                        }
                        
                        // Calculate speed from power (similar to Grupetto approach)
                        currentSpeed = calculateSpeedFromPower(currentPower);
                        
                    } catch (Exception e) {
                        QLog.w(TAG, "Error reading sensor values", e);
                    }
                    
                    if (isUpdating && isServiceConnected) {
                        mainHandler.postDelayed(sensorUpdateRunnable, SENSOR_UPDATE_INTERVAL_MS);
                    }
                });
            }
        };
        
        mainHandler.post(sensorUpdateRunnable);
        QLog.i(TAG, "Started periodic sensor updates");
    }
    
    private void stopSensorUpdatesInstance() {
        isUpdating = false;
        
        if (sensorUpdateRunnable != null) {
            mainHandler.removeCallbacks(sensorUpdateRunnable);
        }
        
        QLog.i(TAG, "Stopped periodic sensor updates");
    }
    
    private float calculateSpeedFromPower(float power) {
        if (power < 0.1f) {
            return 0.0f;
        }
        
        // Use exact formula from Grupetto Peloton.kt
        double pwrSqrt = Math.sqrt(power);
        if (power < 26f) {
            return (float)(0.057f - (0.172f * pwrSqrt) + (0.759f * Math.pow(pwrSqrt, 2)) - (0.079f * Math.pow(pwrSqrt, 3)));
        } else {
            return (float)(-1.635f + (2.325f * pwrSqrt) - (0.064f * Math.pow(pwrSqrt, 2)) + (0.001f * Math.pow(pwrSqrt, 3)));
        }
    }
    
    private void shutdownInstance() {
        stopSensorUpdates();
        
        if (executorService != null) {
            executorService.shutdown();
            try {
                if (!executorService.awaitTermination(2, TimeUnit.SECONDS)) {
                    executorService.shutdownNow();
                }
            } catch (InterruptedException e) {
                QLog.e(TAG, "Error shutting down executor service", e);
                executorService.shutdownNow();
            }
        }
        
        // Cleanup sensors
        powerSensor = null;
        rpmSensor = null;
        resistanceSensor = null;
        
        // Unbind from service
        if (isServiceConnected && context != null) {
            try {
                // Note: In real implementation, we'd need to properly unbind
                // context.unbindService(serviceConnection);
            } catch (Exception e) {
                QLog.e(TAG, "Error unbinding service", e);
            }
        }
        
        isServiceConnected = false;
        sensorBinder = null;
    }
    
    // Static wrapper methods for JNI calls (similar to GrpcTreadmillService)
    public static void initialize() {
        try {
            if (staticContext == null) {
                QLog.e(TAG, "Context not set. Call setContext() first.");
                return;
            }
            
            if (instance == null) {
                instance = new PelotonSensorService(staticContext);
            }
            
            instance.initializeInstance();
            QLog.i(TAG, "Static initialize completed");
        } catch (Exception e) {
            QLog.e(TAG, "Static initialize failed", e);
        }
    }
    
    public static void setContext(Context context) {
        staticContext = context;
    }
    
    public static void startSensorUpdates() {
        if (instance != null) {
            instance.startSensorUpdatesInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    public static void stopSensorUpdates() {
        if (instance != null) {
            instance.stopSensorUpdatesInstance();
        } else {
            QLog.e(TAG, "Service not initialized. Call initialize() first.");
        }
    }
    
    // Getter methods for current sensor values
    public static float getCurrentPower() {
        if (instance != null) {
            return instance.currentPower;
        }
        return 0.0f;
    }
    
    public static float getCurrentCadence() {
        if (instance != null) {
            return instance.currentCadence;
        }
        return 0.0f;
    }
    
    public static float getCurrentResistance() {
        if (instance != null) {
            return instance.currentResistance;
        }
        return 0.0f;
    }
    
    public static float getCurrentSpeed() {
        if (instance != null) {
            return instance.currentSpeed;
        }
        return 0.0f;
    }
    
    public static boolean isConnected() {
        if (instance != null) {
            return instance.isServiceConnected;
        }
        return false;
    }
    
    public static void shutdown() {
        if (instance != null) {
            instance.shutdownInstance();
            instance = null;
        }
    }
    
    // Inner classes for individual sensors (simplified versions based on Grupetto)
    private static class PelotonPowerSensor {
        private IBinder binder;
        
        public PelotonPowerSensor(IBinder binder) {
            this.binder = binder;
        }
        
        public float readValue() throws RemoteException {
            // Implementation would call into Peloton service via binder
            // This is a simplified version - actual implementation would need
            // proper AIDL interface definitions
            
            // For now, return mock data or attempt basic binder calls
            // In real implementation, this would use proper service calls
            return 0.0f; // Placeholder
        }
    }
    
    private static class PelotonRpmSensor {
        private IBinder binder;
        
        public PelotonRpmSensor(IBinder binder) {
            this.binder = binder;
        }
        
        public float readValue() throws RemoteException {
            // Implementation would call into Peloton service via binder
            return 0.0f; // Placeholder
        }
    }
    
    private static class PelotonResistanceSensor {
        private IBinder binder;
        
        public PelotonResistanceSensor(IBinder binder) {
            this.binder = binder;
        }
        
        public float readValue() throws RemoteException {
            // Implementation would call into Peloton service via binder
            return 0.0f; // Placeholder
        }
    }
}