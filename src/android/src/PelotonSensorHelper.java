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

/**
 * Simplified Peloton sensor helper class combining the approaches from 
 * Grupetto's sensor interfaces with our existing architecture
 */
public class PelotonSensorHelper {
    
    private static final String TAG = "PelotonSensorHelper";
    
    // Singleton instance for static access
    private static PelotonSensorHelper instance = null;
    private static Context staticContext = null;
    
    // Update intervals
    private static final int SENSOR_UPDATE_INTERVAL_MS = 200;
    
    // Threading components
    private Handler mainHandler;
    private ExecutorService executorService;
    private Runnable sensorUpdateRunnable;
    
    // Sensor components
    private PelotonSensorBinder sensorBinder;
    private PelotonSensorBinder.PowerSensor powerSensor;
    private PelotonSensorBinder.RpmSensor rpmSensor;
    private PelotonSensorBinder.ResistanceSensor resistanceSensor;
    
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
        
        // Initialize individual sensors
        powerSensor = new PelotonSensorBinder.PowerSensor(serviceBinder);
        rpmSensor = new PelotonSensorBinder.RpmSensor(serviceBinder);
        resistanceSensor = new PelotonSensorBinder.ResistanceSensor(serviceBinder);
        
        isInitialized = true;
        QLog.i(TAG, "Peloton sensor initialization completed");
    }
    
    private void startSensorUpdatesInstance() {
        if (isUpdating || !isInitialized) {
            QLog.w(TAG, "Cannot start sensor updates - not ready");
            return;
        }
        
        isUpdating = true;
        
        sensorUpdateRunnable = new Runnable() {
            @Override
            public void run() {
                if (!isUpdating || !isInitialized) return;
                
                executorService.execute(() -> {
                    readAllSensorValues();
                    
                    if (isUpdating && isInitialized) {
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
    
    private void readAllSensorValues() {
        try {
            // Read power
            if (powerSensor != null) {
                float newPower = powerSensor.readValue();
                currentPower = newPower;
                
                // Calculate speed from power using Peloton V1 formula (from Grupetto)
                currentSpeed = calculateSpeedFromPelotonV1Power(newPower);
            }
            
            // Read cadence (RPM)
            if (rpmSensor != null) {
                currentCadence = rpmSensor.readValue();
            }
            
            // Read resistance (with built-in filtering)
            if (resistanceSensor != null) {
                currentResistance = resistanceSensor.readValue();
            }
            
        } catch (Exception e) {
            QLog.w(TAG, "Error reading sensor values", e);
            // Don't throw exception, just log and continue
        }
    }
    
    /**
     * Calculate speed from power using Peloton V1 bike formula
     * Based on Grupetto's exact implementation from Peloton.kt
     * Source: https://ihaque.org/posts/2020/12/25/pelomon-part-ib-computing-speed/
     */
    private float calculateSpeedFromPelotonV1Power(float power) {
        if (power < 0.1f) {
            return 0.0f;
        }
        
        // Use exact formula from Grupetto Peloton.kt (returns mph)
        double pwrSqrt = Math.sqrt(power);
        float speedMph;
        if (power < 26f) {
            speedMph = (float)(0.057f - (0.172f * pwrSqrt) + (0.759f * Math.pow(pwrSqrt, 2)) - (0.079f * Math.pow(pwrSqrt, 3)));
        } else {
            speedMph = (float)(-1.635f + (2.325f * pwrSqrt) - (0.064f * Math.pow(pwrSqrt, 2)) + (0.001f * Math.pow(pwrSqrt, 3)));
        }
        
        // Convert from mph to km/h
        return speedMph * 1.60934f;
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