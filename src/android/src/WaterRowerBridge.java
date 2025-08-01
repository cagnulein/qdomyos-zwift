package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.util.Log;

import de.tbressler.waterrower.WaterRower;
import de.tbressler.waterrower.IWaterRowerConnectionListener;
import de.tbressler.waterrower.discovery.WaterRowerAutoDiscovery;
import de.tbressler.waterrower.io.transport.SerialDeviceAddress;
import de.tbressler.waterrower.model.ErrorCode;
import de.tbressler.waterrower.model.ModelInformation;
import de.tbressler.waterrower.model.StrokeType;
import de.tbressler.waterrower.subscriptions.values.*;

import java.io.IOException;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.TimeUnit;

public class WaterRowerBridge {
    private static final String TAG = "WaterRowerBridge";
    private static WaterRower waterRower;
    private static boolean isConnected = false;
    private static double lastStrokeRate = 0;
    private static double lastDistance = 0;
    private static double lastPace = 0;
    private static double lastWatts = 0;
    private static double lastCalories = 0;
    private static long lastDataUpdate = 0;
    
    private static final IWaterRowerConnectionListener connectionListener = new IWaterRowerConnectionListener() {
        @Override
        public void onConnected(ModelInformation modelInformation) {
            Log.d(TAG, "WaterRower connected: " + modelInformation.getMonitorType());
            isConnected = true;
            
            // Subscribe to rowing metrics
            // Subscribe to stroke events  
            waterRower.subscribe(new StrokeSubscription() {
                @Override
                protected void onStroke(StrokeType strokeType) {
                    // Track stroke events - could calculate stroke rate here
                    lastDataUpdate = System.currentTimeMillis();
                    Log.d(TAG, "Stroke: " + strokeType);
                }
            });
            
            // Subscribe to distance
            waterRower.subscribe(new DistanceSubscription() {
                @Override
                protected void onDistanceUpdated(double distance) {
                    lastDistance = distance;
                    lastDataUpdate = System.currentTimeMillis();
                    Log.d(TAG, "Distance: " + distance);
                }
            });
            
            // Subscribe to total velocity (pace)
            waterRower.subscribe(new TotalVelocitySubscription() {
                @Override
                protected void onVelocityUpdated(double velocity) {
                    if (velocity > 0) {
                        lastPace = 500.0 / velocity; // Convert to seconds per 500m
                    }
                    lastDataUpdate = System.currentTimeMillis();
                    Log.d(TAG, "Velocity: " + velocity + ", Pace: " + lastPace);
                }
            });
            
            // Subscribe to watts
            waterRower.subscribe(new WattsSubscription() {
                @Override
                protected void onWattsUpdated(int watts) {
                    lastWatts = watts;
                    lastDataUpdate = System.currentTimeMillis();
                    Log.d(TAG, "Watts: " + watts);
                }
            });
            
            // Subscribe to calories
            waterRower.subscribe(new TotalCaloriesSubscription() {
                @Override
                protected void onCaloriesUpdated(int calories) {
                    lastCalories = calories;
                    lastDataUpdate = System.currentTimeMillis();
                    Log.d(TAG, "Calories: " + calories);
                }
            });
        }
        
        @Override
        public void onDisconnected() {
            Log.d(TAG, "WaterRower disconnected");
            isConnected = false;
        }
        
        @Override
        public void onError(ErrorCode errorCode) {
            Log.e(TAG, "WaterRower error: " + errorCode);
            isConnected = false;
        }
    };
    
    public static String initialize(Context context) {
        Log.d(TAG, "Initializing WaterRower USB connection");
        
        if (waterRower != null) {
            shutdown();
        }
        
        try {
            // Create WaterRower instance and set up auto-discovery
            waterRower = new WaterRower();
            waterRower.addConnectionListener(connectionListener);
            
            // Set up auto-discovery
            WaterRowerAutoDiscovery discovery = new WaterRowerAutoDiscovery(waterRower);
            
            // Start auto-discovery (this will automatically connect when a device is found)
            discovery.start();
            
            Log.d(TAG, "WaterRower auto-discovery started");
            return "SUCCESS";
            
        } catch (Exception e) {
            Log.e(TAG, "Failed to initialize WaterRower", e);
            return "INITIALIZATION_FAILED: " + e.getMessage();
        }
    }
    
    public static void shutdown() {
        Log.d(TAG, "Shutting down WaterRower connection");
        
        if (waterRower != null) {
            try {
                waterRower.disconnect();
            } catch (IOException e) {
                Log.e(TAG, "Error disconnecting WaterRower", e);
            }
            waterRower = null;
        }
        
        isConnected = false;
        lastStrokeRate = 0;
        lastDistance = 0;
        lastPace = 0;
        lastWatts = 0;
        lastCalories = 0;
        lastDataUpdate = 0;
    }
    
    public static boolean isConnected() {
        return isConnected;
    }
    
    public static String getStrokeData() {
        if (!isConnected || waterRower == null) {
            return "NO_DATA";
        }
        
        // Check if data is recent (within last 5 seconds)
        long currentTime = System.currentTimeMillis();
        if (currentTime - lastDataUpdate > 5000) {
            return "NO_DATA";
        }
        
        // Return data in CSV format: strokeRate,distance,pace,watts,calories
        return String.format("%.1f,%.1f,%.2f,%.1f,%.1f", 
                            lastStrokeRate, lastDistance, lastPace, lastWatts, lastCalories);
    }
    
    public static double getStrokeRate() {
        return lastStrokeRate;
    }
    
    public static double getDistance() {
        return lastDistance;
    }
    
    public static double getPace() {
        return lastPace;
    }
    
    public static double getWatts() {
        return lastWatts;
    }
    
    public static double getCalories() {
        return lastCalories;
    }
}