package org.cagnulen.qdomyoszwift;

import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.IBinder;

import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CountDownLatch;

import org.cagnulen.qdomyoszwift.QLog;

/**
 * Service binder for connecting to Peloton Grupetto v1 callback-based service
 * Based on: https://github.com/selalipop/grupetto/pull/10
 * More efficient than polling - receives data only when it changes
 */
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
     * Based on Grupetto's v1 Binder.kt implementation
     */
    public CompletableFuture<IBinder> getBinder() {
        CompletableFuture<IBinder> future = new CompletableFuture<>();
        CountDownLatch connectionLatch = new CountDownLatch(1);
        
        ServiceConnection serviceConnection = new ServiceConnection() {
            @Override
            public void onServiceConnected(ComponentName name, IBinder service) {
                QLog.i(TAG, "V1 service connected: " + name.getClassName());
                serviceBinder = service;
                isConnected = true;
                future.complete(service);
                connectionLatch.countDown();
            }
            
            @Override
            public void onServiceDisconnected(ComponentName name) {
                QLog.w(TAG, "V1 service disconnected: " + name.getClassName());
                serviceBinder = null;
                isConnected = false;
            }
            
            @Override
            public void onBindingDied(ComponentName name) {
                QLog.e(TAG, "V1 service binding died: " + name.getClassName());
                serviceBinder = null;
                isConnected = false;
                if (!future.isDone()) {
                    future.completeExceptionally(new RuntimeException("V1 service binding died"));
                }
            }
            
            @Override
            public void onNullBinding(ComponentName name) {
                QLog.i(TAG, "V1 service null binding: " + name.getClassName());
                if (!future.isDone()) {
                    future.completeExceptionally(new RuntimeException("V1 service null binding"));
                }
            }
        };
        
        Intent intent = new Intent(SERVICE_INTENT);
        intent.setAction(SERVICE_ACTION);
        intent.setPackage(SERVICE_PACKAGE);
        
        boolean bound = context.bindService(intent, serviceConnection, Context.BIND_AUTO_CREATE);
        
        if (!bound) {
            QLog.e(TAG, "Failed to bind to Peloton V1 sensor service");
            future.completeExceptionally(new RuntimeException("Failed to bind to V1 service"));
            return future;
        }
        
        QLog.i(TAG, "Binding to Peloton V1 sensor service...");
        return future;
    }
    
    public boolean isConnected() {
        return isConnected;
    }
    
    public IBinder getServiceBinder() {
        return serviceBinder;
    }
}