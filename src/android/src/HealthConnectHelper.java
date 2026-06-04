package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.os.Build;
import android.util.Log;

import java.lang.reflect.Field;
import java.lang.reflect.Method;

public class HealthConnectHelper {
    private static final String TAG = "HealthConnectHelper";
    private static final int MIN_SUPPORTED_SDK = 26;
    private static final String HEALTH_CONNECT_PROVIDER = "com.google.android.apps.healthdata";
    private static boolean initialized = false;

    public static synchronized void initialize(Context context) {
        if (initialized) {
            return;
        }

        initialized = true;

        if (context == null) {
            Log.d(TAG, "Skipping Health Connect init: context is null");
            return;
        }

        if (Build.VERSION.SDK_INT < MIN_SUPPORTED_SDK) {
            Log.d(TAG, "Skipping Health Connect init: unsupported Android API " + Build.VERSION.SDK_INT);
            return;
        }

        try {
            Class<?> clientClass = Class.forName("androidx.health.connect.client.HealthConnectClient");
            int sdkStatus = getSdkStatus(clientClass, context.getApplicationContext());
            int availableStatus = getIntField(clientClass, "SDK_AVAILABLE", 3);

            Log.d(TAG, "Health Connect SDK status: " + sdkStatus);

            if (sdkStatus != availableStatus) {
                return;
            }

            Object client = getOrCreate(clientClass, context.getApplicationContext());
            Log.d(TAG, "Health Connect client initialized: " + (client != null));
        } catch (ClassNotFoundException e) {
            Log.d(TAG, "Health Connect library is not available in this build");
        } catch (Throwable t) {
            Log.w(TAG, "Health Connect init was skipped", t);
        }
    }

    private static int getSdkStatus(Class<?> clientClass, Context context) throws Exception {
        try {
            Method method = clientClass.getMethod("getSdkStatus", Context.class);
            return (Integer) method.invoke(null, context);
        } catch (NoSuchMethodException ignored) {
            Method method = clientClass.getMethod("getSdkStatus", Context.class, String.class);
            return (Integer) method.invoke(null, context, HEALTH_CONNECT_PROVIDER);
        }
    }

    private static Object getOrCreate(Class<?> clientClass, Context context) throws Exception {
        try {
            Method method = clientClass.getMethod("getOrCreate", Context.class);
            return method.invoke(null, context);
        } catch (NoSuchMethodException ignored) {
            Method method = clientClass.getMethod("getOrCreate", Context.class, String.class);
            return method.invoke(null, context, HEALTH_CONNECT_PROVIDER);
        }
    }

    private static int getIntField(Class<?> targetClass, String fieldName, int defaultValue) {
        try {
            Field field = targetClass.getField(fieldName);
            return field.getInt(null);
        } catch (Throwable ignored) {
            return defaultValue;
        }
    }
}
