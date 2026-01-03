package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.os.Handler;
import android.os.Looper;
import android.os.IBinder;
import android.util.Log;

import java.util.concurrent.ExecutorService;
import java.util.concurrent.Executors;
import java.util.concurrent.TimeUnit;

import org.cagnulen.qdomyoszwift.QLog;
import org.cagnulen.qdomyoszwift.PelotonSensorBinder;
import org.cagnulen.qdomyoszwift.PelotonCallbackSensor;

/**
 * Peloton sensor helper class using callback-based approach from Grupetto v1 develop
 * Based on: https://github.com/selalipop/grupetto/pull/10
 * More efficient than polling - receives data only when it changes
 */
public class PelotonSensorHelper {
    
    private static final String TAG = "PelotonSensorHelper";
    
    // Singleton instance for static access
    private static PelotonSensorHelper instance = null;
    private static Context staticContext = null;
    
    // Threading components (reduced need with callback approach)
    private Handler mainHandler;
    private ExecutorService executorService;
    
    // Sensor components (callback-based from Grupetto v1)
    private PelotonSensorBinder sensorBinder;
    private PelotonCallbackSensor.PowerSensor powerSensor;
    private PelotonCallbackSensor.RpmSensor rpmSensor;
    private PelotonCallbackSensor.ResistanceSensor resistanceSensor;
    
    // Control flags and current values
    private volatile boolean isInitialized = false;
    private volatile boolean isUpdating = false;
    private volatile float currentPower = 0.0f;
    private volatile float currentCadence = 0.0f;
    private volatile float currentResistance = 0.0f;
    private volatile float currentSpeed = 0.0f;
    
    // Context for accessing system services
    private Context context;
    
    public PelotonSensorHelper(Context context) {
        this.context = context;
        this.mainHandler = new Handler(Looper.getMainLooper());
        this.executorService = Executors.newSingleThreadExecutor();
        this.sensorBinder = new PelotonSensorBinder(context);
    }
    
    private void initializeInstance() throws Exception {
        QLog.i(TAG, "Initializing Peloton sensor connection...");
        
        // Get binder to Peloton service (async operation)
        IBinder serviceBinder = sensorBinder.getBinder().get(10, TimeUnit.SECONDS);
        
        if (serviceBinder == null) {
            throw new Exception("Failed to get service binder");
        }
        
        // Initialize individual callback-based sensors
        powerSensor = new PelotonCallbackSensor.PowerSensor(serviceBinder);
        rpmSensor = new PelotonCallbackSensor.RpmSensor(serviceBinder);
        resistanceSensor = new PelotonCallbackSensor.ResistanceSensor(serviceBinder);
        
        // Set up callbacks to receive sensor data
        powerSensor.setCallback(new PelotonCallbackSensor.SensorDataCallback() {
            @Override
            public void onSensorDataReceived(float value) {
                currentPower = value;
                currentSpeed = calculateSpeedFromPelotonV1Power(value);
            }
            
            @Override
            public void onSensorError(long errorCode) {
                QLog.w(TAG, "Power sensor error: " + errorCode);
            }
        });
        
        rpmSensor.setCallback(new PelotonCallbackSensor.SensorDataCallback() {
            @Override
            public void onSensorDataReceived(float value) {
                currentCadence = value;
            }
            
            @Override
            public void onSensorError(long errorCode) {
                QLog.w(TAG, "RPM sensor error: " + errorCode);
            }
        });
        
        resistanceSensor.setCallback(new PelotonCallbackSensor.SensorDataCallback() {
            @Override
            public void onSensorDataReceived(float value) {
                currentResistance = value;
            }
            
            @Override
            public void onSensorError(long errorCode) {
                QLog.w(TAG, "Resistance sensor error: " + errorCode);
            }
        });
        
        isInitialized = true;
        QLog.i(TAG, "Peloton sensor initialization completed");
    }
    
    private void startSensorUpdatesInstance() {
        if (isUpdating || !isInitialized) {
            QLog.w(TAG, "Cannot start sensor updates - not ready");
            return;
        }
        
        isUpdating = true;
        
        try {
            // Start callback-based sensors (no polling needed)
            if (powerSensor != null) powerSensor.start();
            if (rpmSensor != null) rpmSensor.start();
            if (resistanceSensor != null) resistanceSensor.start();
            
            QLog.i(TAG, "Started callback-based sensor updates");
        } catch (Exception e) {
            QLog.e(TAG, "Failed to start sensor updates", e);
            isUpdating = false;
        }
    }
    
    private void stopSensorUpdatesInstance() {
        isUpdating = false;
        
        // Stop callback-based sensors
        if (powerSensor != null) powerSensor.stop();
        if (rpmSensor != null) rpmSensor.stop();
        if (resistanceSensor != null) resistanceSensor.stop();
        
        QLog.i(TAG, "Stopped callback-based sensor updates");
    }
    
    // Sensor values are now updated via callbacks - no polling needed
    
    /**
     * Calculate speed from power using Peloton V1 bike formula
     * Based on Grupetto's SensorInterface.kt implementation
     */
    private float calculateSpeedFromPelotonV1Power(float power) {
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
        
        // Clean up sensors
        powerSensor = null;
        rpmSensor = null;
        resistanceSensor = null;
        sensorBinder = null;
        
        isInitialized = false;
    }
    
    // Static wrapper methods for JNI calls
    public static void initialize() {
        try {
            if (staticContext == null) {
                QLog.e(TAG, "Context not set. Call setContext() first.");
                return;
            }
            
            if (instance == null) {
                instance = new PelotonSensorHelper(staticContext);
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
            QLog.e(TAG, "Helper not initialized. Call initialize() first.");
        }
    }
    
    public static void stopSensorUpdates() {
        if (instance != null) {
            instance.stopSensorUpdatesInstance();
        } else {
            QLog.e(TAG, "Helper not initialized. Call initialize() first.");
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
        if (instance != null && instance.sensorBinder != null) {
            return instance.sensorBinder.isConnected();
        }
        return false;
    }
    
    public static boolean isInitialized() {
        if (instance != null) {
            return instance.isInitialized;
        }
        return false;
    }
    
    public static void shutdown() {
        if (instance != null) {
            instance.shutdownInstance();
            instance = null;
        }
    }
}