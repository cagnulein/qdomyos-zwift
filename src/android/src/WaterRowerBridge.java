package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.util.Log;

import de.tbressler.waterrower.WaterRower;
import de.tbressler.waterrower.IWaterRowerConnectionListener;
import de.tbressler.waterrower.discovery.WaterRowerAutoDiscovery;
import de.tbressler.waterrower.io.transport.SerialDeviceAddress;
import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.model.ErrorCode;
import de.tbressler.waterrower.model.ModelInformation;
import de.tbressler.waterrower.subscriptions.values.*;

import java.io.IOException;
import java.util.List;
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
            try {
                // Subscribe to stroke rate  
                waterRower.subscribe(new StrokeSubscription() {
                    @Override
                    public void onStrokeReceived(int strokeRate) {
                        lastStrokeRate = strokeRate;
                        lastDataUpdate = System.currentTimeMillis();
                        Log.d(TAG, "Stroke rate: " + strokeRate);
                    }
                    
                    @Override
                    public void handle(AbstractMessage message) {
                        // Handle stroke messages
                    }
                });
                
                // Subscribe to distance
                waterRower.subscribe(new DistanceSubscription() {
                    @Override
                    public void onDistanceReceived(int distance) {
                        lastDistance = distance;
                        lastDataUpdate = System.currentTimeMillis();
                        Log.d(TAG, "Distance: " + distance);
                    }
                    
                    @Override
                    public void handle(AbstractMessage message) {
                        // Handle distance messages
                    }
                });
                
                // Subscribe to total velocity (pace)
                waterRower.subscribe(new TotalVelocitySubscription() {
                    @Override
                    public void onVelocityReceived(int velocity) {
                        if (velocity > 0) {
                            lastPace = 500.0 / velocity; // Convert to seconds per 500m
                        }
                        lastDataUpdate = System.currentTimeMillis();
                        Log.d(TAG, "Velocity: " + velocity + ", Pace: " + lastPace);
                    }
                    
                    @Override
                    public void handle(AbstractMessage message) {
                        // Handle velocity messages
                    }
                });
                
                // Subscribe to watts
                waterRower.subscribe(new WattsSubscription() {
                    @Override
                    public void onWattsReceived(int watts) {
                        lastWatts = watts;
                        lastDataUpdate = System.currentTimeMillis();
                        Log.d(TAG, "Watts: " + watts);
                    }
                    
                    @Override
                    public void handle(AbstractMessage message) {
                        // Handle watts messages
                    }
                });
                
                // Subscribe to calories
                waterRower.subscribe(new TotalCaloriesSubscription() {
                    @Override
                    public void onCaloriesReceived(int calories) {
                        lastCalories = calories;
                        lastDataUpdate = System.currentTimeMillis();
                        Log.d(TAG, "Calories: " + calories);
                    }
                    
                    @Override
                    public void handle(AbstractMessage message) {
                        // Handle calories messages
                    }
                });
                
            } catch (IOException e) {
                Log.e(TAG, "Failed to subscribe to WaterRower metrics", e);
            }
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
            // Auto-discover WaterRower devices
            WaterRowerAutoDiscovery discovery = new WaterRowerAutoDiscovery();
            List<SerialDeviceAddress> devices = discovery.discover();
            
            if (devices.isEmpty()) {
                Log.w(TAG, "No WaterRower devices found");
                return "NO_DEVICE_FOUND";
            }
            
            Log.d(TAG, "Found " + devices.size() + " WaterRower device(s)");
            
            // Connect to the first available device
            SerialDeviceAddress deviceAddress = devices.get(0);
            Log.d(TAG, "Connecting to device: " + deviceAddress.getAddress());
            
            waterRower = new WaterRower();
            waterRower.addConnectionListener(connectionListener);
            
            // Connect with timeout
            CompletableFuture<Void> connectFuture = CompletableFuture.runAsync(() -> {
                try {
                    waterRower.connect(deviceAddress);
                } catch (IOException e) {
                    throw new RuntimeException(e);
                }
            });
            
            try {
                connectFuture.get(10, TimeUnit.SECONDS);
                Log.d(TAG, "WaterRower connection initiated");
                return "SUCCESS";
            } catch (Exception e) {
                Log.e(TAG, "Connection timeout or failed", e);
                return "CONNECTION_FAILED";
            }
            
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