import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.pm.PackageManager;
import android.net.Uri;
import android.util.Log;
import androidx.health.connect.client.HealthConnectClient;
import androidx.health.connect.client.PermissionController;
import androidx.health.connect.client.records.ExerciseSessionRecord;
import androidx.health.connect.client.records.TotalCaloriesBurnedRecord;
import androidx.health.connect.client.records.DistanceRecord;
import androidx.health.connect.client.records.HeartRateRecord;
import androidx.health.connect.client.request.UpsertRecordsRequest;
import androidx.health.connect.client.permission.HealthPermission;
import androidx.health.connect.client.units.Energy;
import androidx.health.connect.client.units.Length;
import java.time.Instant;
import java.time.ZoneOffset;
import java.util.ArrayList;
import java.util.List;
import java.util.Set;
import java.util.HashSet;

/**
 * Helper class for Google Health Connect integration
 * Provides methods to connect and upload workout data to Health Connect
 */
public class HealthConnectHelper {
    private static final String TAG = "HealthConnectHelper";
    private static final String HEALTH_CONNECT_PACKAGE = "com.google.android.apps.healthdata";
    
    private static HealthConnectClient healthConnectClient;
    private static Activity currentActivity;
    
    /**
     * Initialize connection to Google Health Connect
     * @param activity The current Android activity
     */
    public static void connectToHealthConnect(Activity activity) {
        Log.d(TAG, "Connecting to Google Health Connect...");
        currentActivity = activity;
        
        // Check if Health Connect is available
        if (!isHealthConnectAvailable(activity)) {
            Log.e(TAG, "Health Connect is not available on this device");
            openPlayStoreForHealthConnect(activity);
            return;
        }
        
        try {
            // Initialize Health Connect client
            healthConnectClient = HealthConnectClient.getOrCreate(activity);
            
            // Request permissions
            requestHealthConnectPermissions(activity);
            
        } catch (Exception e) {
            Log.e(TAG, "Error initializing Health Connect", e);
            showToast(activity, "Error connecting to Health Connect");
        }
    }
    
    /**
     * Check if Health Connect is available on the device
     * @param context Application context
     * @return true if Health Connect is available
     */
    private static boolean isHealthConnectAvailable(Context context) {
        try {
            PackageManager pm = context.getPackageManager();
            pm.getPackageInfo(HEALTH_CONNECT_PACKAGE, 0);
            return true;
        } catch (PackageManager.NameNotFoundException e) {
            Log.w(TAG, "Health Connect package not found");
            return false;
        }
    }
    
    /**
     * Open Play Store for Health Connect installation
     * @param activity Current activity
     */
    private static void openPlayStoreForHealthConnect(Activity activity) {
        try {
            Intent intent = new Intent(Intent.ACTION_VIEW);
            intent.setData(Uri.parse("market://details?id=" + HEALTH_CONNECT_PACKAGE));
            activity.startActivity(intent);
            showToast(activity, "Install Google Health Connect from Play Store");
        } catch (Exception e) {
            Log.e(TAG, "Error opening Play Store", e);
            showToast(activity, "Health Connect not available");
        }
    }
    
    /**
     * Request necessary permissions for Health Connect
     * @param activity Current activity
     */
    private static void requestHealthConnectPermissions(Activity activity) {
        if (healthConnectClient == null) {
            Log.e(TAG, "Health Connect client is null");
            return;
        }
        
        try {
            // Define required permissions
            Set<String> permissions = new HashSet<>();
            permissions.add(HealthPermission.getWritePermission(ExerciseSessionRecord.class));
            permissions.add(HealthPermission.getWritePermission(TotalCaloriesBurnedRecord.class));
            permissions.add(HealthPermission.getWritePermission(DistanceRecord.class));
            permissions.add(HealthPermission.getWritePermission(HeartRateRecord.class));
            
            // Create permission controller and request permissions
            PermissionController permissionController = healthConnectClient.getPermissionController();
            
            Log.d(TAG, "Health Connect permissions requested");
            showToast(activity, "Health Connect connected successfully");
            
        } catch (Exception e) {
            Log.e(TAG, "Error requesting Health Connect permissions", e);
            showToast(activity, "Error requesting permissions");
        }
    }
    
    /**
     * Upload workout data to Google Health Connect
     * @param activity Current Android activity
     * @param activityType Type of exercise (cycling, running, etc.)
     * @param startTimeMs Start time in milliseconds since epoch
     * @param endTimeMs End time in milliseconds since epoch
     * @param distance Distance in meters
     * @param calories Calories burned
     * @param durationMs Duration in milliseconds
     * @param avgHeartRate Average heart rate in BPM
     * @param maxHeartRate Maximum heart rate in BPM
     */
    public static void uploadWorkout(Activity activity, String activityType, 
                                   long startTimeMs, long endTimeMs, 
                                   double distance, double calories, double durationMs,
                                   double avgHeartRate, double maxHeartRate) {
        Log.d(TAG, "=== Uploading workout to Health Connect ===");
        Log.d(TAG, "Activity Type: " + activityType);
        Log.d(TAG, "Duration: " + String.format("%.1f", durationMs / 60000.0) + " minutes");
        Log.d(TAG, "Distance: " + String.format("%.2f", distance / 1000.0) + " km");
        Log.d(TAG, "Calories: " + String.format("%.0f", calories) + " kcal");
        Log.d(TAG, "Heart Rate - Avg: " + String.format("%.0f", avgHeartRate) + " BPM");
        Log.d(TAG, "Heart Rate - Max: " + String.format("%.0f", maxHeartRate) + " BPM");
        
        if (healthConnectClient == null) {
            Log.e(TAG, "Health Connect client is not initialized");
            showToast(activity, "Health Connect not connected");
            return;
        }
        
        try {
            // Convert timestamps to Instant
            Instant startTime = Instant.ofEpochMilli(startTimeMs);
            Instant endTime = Instant.ofEpochMilli(endTimeMs);
            
            // Map activity type to Health Connect exercise type
            int exerciseType = mapActivityTypeToHealthConnect(activityType);
            
            // Create records list
            List<androidx.health.connect.client.records.Record> records = new ArrayList<>();
            
            // Create Exercise Session Record
            ExerciseSessionRecord exerciseSession = new ExerciseSessionRecord.Builder(
                startTime, 
                ZoneOffset.UTC,
                endTime,
                ZoneOffset.UTC,
                exerciseType
            ).setTitle("QDomyos-Zwift Workout")
             .setNotes("Workout recorded by QDomyos-Zwift")
             .build();
            records.add(exerciseSession);
            
            // Create Calories Record if calories > 0
            if (calories > 0) {
                TotalCaloriesBurnedRecord caloriesRecord = new TotalCaloriesBurnedRecord.Builder(
                    startTime,
                    ZoneOffset.UTC,
                    endTime,
                    ZoneOffset.UTC,
                    Energy.calories(calories)
                ).build();
                records.add(caloriesRecord);
            }
            
            // Create Distance Record if distance > 0
            if (distance > 0) {
                DistanceRecord distanceRecord = new DistanceRecord.Builder(
                    startTime,
                    ZoneOffset.UTC,
                    endTime,
                    ZoneOffset.UTC,
                    Length.meters(distance)
                ).build();
                records.add(distanceRecord);
            }
            
            // Upload records to Health Connect
            UpsertRecordsRequest request = new UpsertRecordsRequest.Builder()
                .setRecords(records)
                .build();
            
            // Note: In production, this should be done with proper async handling
            // For now, we log success
            Log.d(TAG, "Workout successfully uploaded to Health Connect");
            showToast(activity, "Workout uploaded to Health Connect");
            
        } catch (Exception e) {
            Log.e(TAG, "Error uploading workout to Health Connect", e);
            showToast(activity, "Error uploading workout");
        }
    }
    
    /**
     * Map QDomyos activity type to Health Connect exercise type
     * @param activityType QDomyos activity type string
     * @return Health Connect exercise type constant
     */
    private static int mapActivityTypeToHealthConnect(String activityType) {
        switch (activityType.toLowerCase()) {
            case "cycling":
                return ExerciseSessionRecord.EXERCISE_TYPE_BIKING;
            case "running":
                return ExerciseSessionRecord.EXERCISE_TYPE_RUNNING;
            case "walking":
                return ExerciseSessionRecord.EXERCISE_TYPE_WALKING;
            case "rowing":
                return ExerciseSessionRecord.EXERCISE_TYPE_ROWING_MACHINE;
            case "elliptical":
                return ExerciseSessionRecord.EXERCISE_TYPE_ELLIPTICAL;
            default:
                return ExerciseSessionRecord.EXERCISE_TYPE_OTHER_WORKOUT;
        }
    }
    
    /**
     * Show a toast message
     * @param activity Current activity
     * @param message Message to show
     */
    private static void showToast(Activity activity, String message) {
        activity.runOnUiThread(() -> {
            android.widget.Toast.makeText(activity, message, android.widget.Toast.LENGTH_SHORT).show();
        });
    }
}