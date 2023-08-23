package org.cagnulen.qdomyoszwift;

import android.app.ActivityManager;
import android.app.Activity;
import android.content.Context;
import android.content.DialogInterface;
import android.content.Intent;
import android.net.Uri;
import android.os.Build;
import android.os.Bundle;
import android.provider.Settings;
import android.text.Editable;
import android.text.TextWatcher;
import android.view.View;
import android.widget.Button;
import android.widget.EditText;
import android.widget.Toast;
import android.os.Looper;
import android.os.Handler;
import android.util.Log;
import android.content.BroadcastReceiver;
import android.content.ContextWrapper;
import android.content.IntentFilter;
import android.widget.Toast;

import org.jetbrains.annotations.Nullable;

import java.util.HashMap;
import java.util.List;

import androidx.activity.result.ActivityResultLauncher;
import androidx.activity.result.contract.ActivityResultContracts;
import androidx.health.connect.client.HealthConnectClient;
import androidx.health.connect.client.PermissionController;
import androidx.health.connect.client.permission.HealthPermission;
import androidx.health.connect.client.records.*;
import androidx.health.connect.client.request.*;
import androidx.health.connect.client.time.*;

import java.time.Instant;
import java.util.HashSet;
import java.util.Set;

public class HealthConnect {

	private static final Set<HealthPermission> PERMISSIONS = new HashSet<>();
	    private ActivityResultLauncher<Set<HealthPermission>> requestPermissions;
		 private static HealthConnectClient healthConnectClient;

		 private static void checkAndRun(Context context) {
			  int availabilityStatus = HealthConnectClient.sdkStatus(context, "com.google.android.apps.healthdata");
			  if (availabilityStatus == HealthConnectClient.SDK_UNAVAILABLE) {
				   return; // early return as there is no viable integration
					}
				if (availabilityStatus == HealthConnectClient.SDK_UNAVAILABLE_PROVIDER_UPDATE_REQUIRED) {
					String uriString = "market://details?id=com.google.android.apps.healthdata&url=healthconnect%3A%2F%2Fonboarding";
					Intent intent = new Intent(Intent.ACTION_VIEW);
					intent.setPackage("com.android.vending");
					intent.setData(Uri.parse(uriString));
					intent.putExtra("overlay", true);
					intent.putExtra("callerId", context.getPackageName());
					context.startActivity(intent);
					return;
					}

				healthConnectClient = HealthConnectClient.getOrCreate(context);
			  // Issue operations with healthConnectClient

			  PERMISSIONS.add(HealthPermission.getReadPermission(StepsRecord.class));
			  PERMISSIONS.add(HealthPermission.getWritePermission(StepsRecord.class));
			  PERMISSIONS.add(HealthPermission.getReadPermission(HeartRateRecord.class));
			  PERMISSIONS.add(HealthPermission.getWritePermission(HeartRateRecord.class));

			  checkPermissionsAndRun(healthConnectClient);
			}

		 private void handlePermissionsResult(Boolean granted) {
			  if (granted) {
				   // Permissions successfully granted
					} else {
					// Lack of required permissions
					}
		 }

	 private static void checkPermissionsAndRun() {
		  Set<HealthPermission> granted = permissionController.getGrantedPermissions();
		  if (granted.containsAll(PERMISSIONS)) {
			   // Permissions already granted; proceed with inserting or reading data
				} else {
				Intent intent = permissionController.createPermissionRequestIntent(PERMISSIONS);
				startActivityForResult(intent, PERMISSION_REQUEST_CODE);
				}
	 }

    public void onActivityResult(int requestCode, int resultCode, Intent data) {
		  if (requestCode == PERMISSION_REQUEST_CODE) {
			   if (resultCode == Activity.RESULT_OK) {
					 // Permissions successfully granted
					} else {
					 // Lack of required permissions
					}
			}
	 }

       private static void readStepsByTimeRange(HealthConnectClient healthConnectClient, Instant startTime, Instant endTime) {
			  try {
				   ReadRecordsRequest request = new ReadRecordsRequest(
					        StepsRecord.class,
							  new TimeRangeFilter(startTime, endTime)
							);
					ReadRecordsRequest.Response response = healthConnectClient.readRecords(request);
					for (StepsRecord stepRecord : response.getRecords()) {
						 // Process each step record
						}
				} catch (Exception e) {
				   // Run error handling here.
					}
		 }

	public static void init(Context c) {
		checkAndRun(c);
		readStepsByTimeRange(healthConnectClient, Instant.now(), Instant.now());
	}
}
