package org.cagnulen.qdomyoszwift;

import android.content.Context;
import android.os.Build;
import android.util.Log;

import java.lang.reflect.InvocationHandler;
import java.lang.reflect.Method;
import java.lang.reflect.Proxy;

public class AgeSignalsHelper {
    private static final String TAG = "AgeSignalsHelper";
    private static final int MIN_SUPPORTED_SDK = 23;
    private static boolean requested = false;

    public static synchronized void requestAgeSignals(Context context) {
        if (requested) {
            return;
        }

        requested = true;

        if (context == null) {
            Log.d(TAG, "Skipping Play Age Signals: context is null");
            return;
        }

        if (Build.VERSION.SDK_INT < MIN_SUPPORTED_SDK) {
            Log.d(TAG, "Skipping Play Age Signals: unsupported Android API " + Build.VERSION.SDK_INT);
            return;
        }

        try {
            final Class<?> factoryClass = Class.forName("com.google.android.play.agesignals.AgeSignalsManagerFactory");
            final Class<?> requestClass = Class.forName("com.google.android.play.agesignals.AgeSignalsRequest");

            Object manager = factoryClass
                    .getMethod("create", Context.class)
                    .invoke(null, context.getApplicationContext());
            Object requestBuilder = requestClass.getMethod("builder").invoke(null);
            Object request = requestBuilder.getClass().getMethod("build").invoke(requestBuilder);
            Object task = manager.getClass().getMethod("checkAgeSignals", requestClass).invoke(manager, request);

            attachSuccessListener(task);
            attachFailureListener(task);
            Log.d(TAG, "Play Age Signals request started");
        } catch (ClassNotFoundException e) {
            Log.d(TAG, "Play Age Signals library is not available in this build");
        } catch (Throwable t) {
            Log.w(TAG, "Play Age Signals request was skipped", t);
        }
    }

    private static void attachSuccessListener(Object task) throws Exception {
        Class<?> listenerClass = Class.forName("com.google.android.gms.tasks.OnSuccessListener");
        Object listener = Proxy.newProxyInstance(
                listenerClass.getClassLoader(),
                new Class<?>[] { listenerClass },
                new InvocationHandler() {
                    @Override
                    public Object invoke(Object proxy, Method method, Object[] args) {
                        if ("onSuccess".equals(method.getName()) && args != null && args.length > 0) {
                            logResult(args[0]);
                        }
                        return null;
                    }
                });

        task.getClass().getMethod("addOnSuccessListener", listenerClass).invoke(task, listener);
    }

    private static void attachFailureListener(Object task) throws Exception {
        Class<?> listenerClass = Class.forName("com.google.android.gms.tasks.OnFailureListener");
        Object listener = Proxy.newProxyInstance(
                listenerClass.getClassLoader(),
                new Class<?>[] { listenerClass },
                new InvocationHandler() {
                    @Override
                    public Object invoke(Object proxy, Method method, Object[] args) {
                        if ("onFailure".equals(method.getName()) && args != null && args.length > 0) {
                            logFailure(args[0]);
                        }
                        return null;
                    }
                });

        task.getClass().getMethod("addOnFailureListener", listenerClass).invoke(task, listener);
    }

    private static void logResult(Object result) {
        try {
            Object userStatus = invokeNoArg(result, "userStatus");
            Object ageLower = invokeNoArg(result, "ageLower");
            Object ageUpper = invokeNoArg(result, "ageUpper");
            Object approvalDate = invokeNoArg(result, "mostRecentApprovalDate");
            Object installId = invokeNoArg(result, "installId");

            Log.d(TAG, "Play Age Signals result: userStatus=" + userStatus
                    + ", ageLower=" + ageLower
                    + ", ageUpper=" + ageUpper
                    + ", mostRecentApprovalDate=" + approvalDate
                    + ", installIdPresent=" + (installId != null));
        } catch (Throwable t) {
            Log.w(TAG, "Play Age Signals result received but could not be logged", t);
        }
    }

    private static void logFailure(Object failure) {
        if (!(failure instanceof Throwable)) {
            Log.w(TAG, "Play Age Signals request failed: " + failure);
            return;
        }

        Throwable throwable = (Throwable) failure;
        Object errorCode = null;
        try {
            errorCode = invokeNoArg(throwable, "getErrorCode");
        } catch (Throwable ignored) {
        }

        if (errorCode == null) {
            Log.w(TAG, "Play Age Signals request failed", throwable);
        } else {
            Log.w(TAG, "Play Age Signals request failed with errorCode=" + errorCode, throwable);
        }
    }

    private static Object invokeNoArg(Object target, String methodName) throws Exception {
        return target.getClass().getMethod(methodName).invoke(target);
    }
}
