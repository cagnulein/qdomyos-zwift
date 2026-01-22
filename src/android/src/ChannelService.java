/*
 * Copyright 2012 Dynastream Innovations Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may not
 * use this file except in compliance with the License. You may obtain a copy of
 * the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations under
 * the License.
 */
package org.cagnulen.qdomyoszwift;

import com.dsi.ant.AntService;
import com.dsi.ant.channel.*;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.ExtendedAssignment;
import com.dsi.ant.message.LibConfig;
import com.dsi.ant.message.fromant.*;
import com.dsi.ant.message.ipc.AntMessageParcel;

import android.app.Service;
import android.content.BroadcastReceiver;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.content.ServiceConnection;
import android.os.Binder;
import android.os.IBinder;
import android.os.RemoteException;
import org.cagnulen.qdomyoszwift.QLog;
import android.util.SparseArray;
import android.os.Build;
import androidx.core.content.ContextCompat;

import java.util.ArrayList;

public class ChannelService extends Service {
    private static final String TAG = "ChannelService";

    private boolean mAntRadioServiceBound;
    private AntService mAntRadioService = null;
    private AntChannelProvider mAntChannelProvider = null;
    private boolean mAllowAddChannel = false;

    public static native void nativeSetResistance(int resistance);
    public static native void nativeSetPower(int power);
    public static native void nativeSetInclination(double inclination);

    HeartChannelController heartChannelController = null;
    PowerChannelController powerChannelController = null;
    SpeedChannelController speedChannelController = null;
    SDMChannelController sdmChannelController = null;
    BikeChannelController bikeChannelController = null; // Added BikeChannelController reference
    BikeTransmitterController bikeTransmitterController = null; // Added BikeTransmitterController reference

    private ServiceConnection mAntRadioServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            QLog.v(TAG, "onServiceConnected");
            // Must pass in the received IBinder object to correctly construct an AntService object
            mAntRadioService = new AntService(service);

            try {
                // Getting a channel provider in order to acquire channels
                mAntChannelProvider = mAntRadioService.getChannelProvider();

                // Initial check for number of channels available
                boolean mChannelAvailable = mAntChannelProvider.getNumChannelsAvailable() > 0;
                // Initial check for if legacy interface is in use. If the
                // legacy interface is in use, applications can free the ANT
                // radio by attempting to acquire a channel.
                boolean legacyInterfaceInUse = mAntChannelProvider.isLegacyInterfaceInUse();

                QLog.v(TAG, "onServiceConnected mChannelAvailable=" + mChannelAvailable + " legacyInterfaceInUse=" + legacyInterfaceInUse);

                // If there are channels OR legacy interface in use, allow adding channels
                if (mChannelAvailable || legacyInterfaceInUse) {
                    mAllowAddChannel = true;
                    QLog.i(TAG, "ANT+ channels are available - proceeding to open channels");
                } else {
                    // If no channels available AND legacy interface is not in use, disallow adding channels
                    mAllowAddChannel = false;
                    QLog.w(TAG, "No ANT+ channels available on this device!");
                    QLog.w(TAG, "Possible causes:");
                    QLog.w(TAG, "  1. Device does not have ANT+ hardware");
                    QLog.w(TAG, "  2. ANT+ Radio Service is not installed or not running");
                    QLog.w(TAG, "  3. All channels are in use by other applications");
                    QLog.w(TAG, "  4. ANT+ permissions are not granted");
                    QLog.w(TAG, "Waiting for channels to become available...");
                }

                try {
                    openAllChannels();
                } catch (ChannelNotAvailableException exception) {
                    QLog.e(TAG, "Channel not available!! Exception: " + exception.getMessage());
                }
            } catch (RemoteException e) {
                // TODO Auto-generated catch block
                e.printStackTrace();
            }
        }

        @Override
        public void onServiceDisconnected(ComponentName name) {
            die("Binder Died");

            mAntChannelProvider = null;
            mAntRadioService = null;

            mAllowAddChannel = false;
        }

    };

    /**
     * The interface used to communicate with the ChannelService
     */
    public class ChannelServiceComm extends Binder {

        void setSpeed(double speed) {
            if (null != speedChannelController) {
                speedChannelController.speed = speed;
            }
            if (null != sdmChannelController) {
                sdmChannelController.speed = speed;
            }
            // Update bike transmitter with speed data (only if not treadmill)
            if (!Ant.treadmill && null != bikeTransmitterController) {
                bikeTransmitterController.setSpeedKph(speed);
            }
        }

        void setPower(int power) {
            if (null != powerChannelController) {
                powerChannelController.power = power;
            }
            // Update bike transmitter with power data (only if not treadmill)
            if (!Ant.treadmill && null != bikeTransmitterController) {
                bikeTransmitterController.setPower(power);
            }
        }

        void setCadence(int cadence) {
            if (null != powerChannelController) {
                powerChannelController.cadence = cadence;
            }
            if (null != speedChannelController) {
                speedChannelController.cadence = cadence;
            }
            if (null != sdmChannelController) {
                sdmChannelController.cadence = cadence;
            }
            // Update bike transmitter with cadence data (only if not treadmill)
            if (!Ant.treadmill && null != bikeTransmitterController) {
                bikeTransmitterController.setCadence(cadence);
            }
        }

        int getHeart() {
            if (null != heartChannelController) {
                QLog.v(TAG, "getHeart");
                return heartChannelController.heart;
            }
            if (null != bikeChannelController) {
                return bikeChannelController.getHeartRate();
            }
            return 0;
        }

        // Added getters for bike channel data
        int getBikeCadence() {
            if (null != bikeChannelController) {
                return bikeChannelController.getCadence();
            }
            return 0;
        }

        int getBikePower() {
            if (null != bikeChannelController) {
                return bikeChannelController.getPower();
            }
            return 0;
        }

        double getBikeSpeed() {
            if (null != bikeChannelController) {
                return bikeChannelController.getSpeedKph();
            }
            return 0.0;
        }

        long getBikeDistance() {
            if (null != bikeChannelController) {
                return bikeChannelController.getDistance();
            }
            return 0;
        }

        boolean isBikeConnected() {
            return (bikeChannelController != null && bikeChannelController.isConnected());
        }

        // ========== BIKE TRANSMITTER METHODS ==========

        /**
         * Start the bike transmitter (only available if not treadmill)
         */
        boolean startBikeTransmitter() {
            QLog.v(TAG, "ChannelServiceComm.startBikeTransmitter");
            
            if (Ant.treadmill) {
                QLog.w(TAG, "Bike transmitter not available in treadmill mode");
                return false;
            }
            
            if (bikeTransmitterController != null) {
                return bikeTransmitterController.startTransmission();
            }
            QLog.w(TAG, "Bike transmitter controller is null");
            return false;
        }

        /**
         * Stop the bike transmitter
         */
        void stopBikeTransmitter() {
            QLog.v(TAG, "ChannelServiceComm.stopBikeTransmitter");
            
            if (bikeTransmitterController != null) {
                bikeTransmitterController.stopTransmission();
            }
        }

        /**
         * Check if bike transmitter is active (only if not treadmill)
         */
        boolean isBikeTransmitterActive() {
            if (Ant.treadmill) {
                return false;
            }
            return (bikeTransmitterController != null && bikeTransmitterController.isTransmitting());
        }

        /**
         * Update bike transmitter with extended metrics (only if not treadmill)
         */
        void updateBikeTransmitterExtendedMetrics(long distanceMeters, int heartRate, 
                                                 double elapsedTimeSeconds, int resistance, 
                                                 double inclination) {
            if (!Ant.treadmill && bikeTransmitterController != null) {
                bikeTransmitterController.setDistance(distanceMeters);
                bikeTransmitterController.setHeartRate(heartRate);
                bikeTransmitterController.setElapsedTime(elapsedTimeSeconds);
                bikeTransmitterController.setResistance(resistance);
                bikeTransmitterController.setInclination(inclination);
            }
        }

        /**
         * Get the last requested resistance from ANT+ controller (only if not treadmill)
         */
        int getRequestedResistanceFromAnt() {
            if (!Ant.treadmill && bikeTransmitterController != null) {
                return bikeTransmitterController.getRequestedResistance();
            }
            return -1;
        }

        /**
         * Get the last requested power from ANT+ controller (only if not treadmill)
         */
        int getRequestedPowerFromAnt() {
            if (!Ant.treadmill && bikeTransmitterController != null) {
                return bikeTransmitterController.getRequestedPower();
            }
            return -1;
        }

        /**
         * Get the last requested inclination from ANT+ controller (only if not treadmill)
         */
        double getRequestedInclinationFromAnt() {
            if (!Ant.treadmill && bikeTransmitterController != null) {
                return bikeTransmitterController.getRequestedInclination();
            }
            return -100.0;
        }

        /**
         * Clear any pending control requests (only if not treadmill)
         */
        void clearAntControlRequests() {
            if (!Ant.treadmill && bikeTransmitterController != null) {
                bikeTransmitterController.clearControlRequests();
            }
        }

        /**
         * Get transmission info for debugging (only if not treadmill)
         */
        String getBikeTransmitterInfo() {
            if (Ant.treadmill) {
                return "Bike transmitter disabled in treadmill mode";
            }
            if (bikeTransmitterController != null) {
                return bikeTransmitterController.getTransmissionInfo();
            }
            return "Bike transmitter not initialized";
        }

        /**
         * Closes all channels currently added.
         */
        void clearAllChannels() {
            closeAllChannels();
        }
    }

    public void openAllChannels() throws ChannelNotAvailableException {
        // Check if we're allowed to add channels before attempting to acquire any
        if (!mAllowAddChannel) {
            QLog.w(TAG, "openAllChannels: Cannot open channels - no ANT+ channels available on this device");
            QLog.w(TAG, "openAllChannels: Waiting for ANT+ channels to become available...");
            return;
        }

        if (Ant.heartRequest && heartChannelController == null)
            heartChannelController = new HeartChannelController(Ant.antHeartDeviceNumber);

        if (Ant.speedRequest) {
            if(Ant.treadmill && sdmChannelController == null) {
                sdmChannelController = new SDMChannelController(acquireChannel());
            } else if(powerChannelController == null) {
                powerChannelController = new PowerChannelController(acquireChannel());
                speedChannelController = new SpeedChannelController(acquireChannel());
            }
        }

        // Add initialization for BikeChannelController (receiver)
        if (Ant.bikeRequest && bikeChannelController == null) {
            bikeChannelController = new BikeChannelController(Ant.technoGymGroupCycle, Ant.antBikeDeviceNumber);
        }

        // Add initialization for BikeTransmitterController (transmitter) - only when NOT treadmill
        if (!Ant.treadmill && bikeTransmitterController == null) {
            QLog.i(TAG, "Initializing BikeTransmitterController (bike mode)");
            try {
                // Acquire channel like other controllers
                AntChannel transmitterChannel = acquireChannel();
                if (transmitterChannel != null) {
                    QLog.i(TAG, "ANT+ channel acquired successfully for BikeTransmitterController");
                    bikeTransmitterController = new BikeTransmitterController(transmitterChannel);
                    
                    // Set up control command listener to handle requests from ANT+ devices
                    bikeTransmitterController.setControlCommandListener(new BikeTransmitterController.ControlCommandListener() {
                        @Override
                        public void onResistanceChangeRequested(int resistance) {
                            QLog.d(TAG, "ChannelService: ANT+ Resistance change requested: " + resistance);
                            // Send broadcast intent to notify the main application
                            Intent intent = new Intent("org.cagnulen.qdomyoszwift.ANT_RESISTANCE_CHANGE");
                            intent.putExtra("resistance", resistance);
                            nativeSetResistance(resistance);
                            sendBroadcast(intent);
                        }

                        @Override
                        public void onPowerChangeRequested(int power) {
                            QLog.d(TAG, "ChannelService: ANT+ Power change requested: " + power + "W");
                            // Send broadcast intent to notify the main application
                            Intent intent = new Intent("org.cagnulen.qdomyoszwift.ANT_POWER_CHANGE");
                            intent.putExtra("power", power);
                            nativeSetPower(power);
                            sendBroadcast(intent);
                        }

                        @Override
                        public void onInclinationChangeRequested(double inclination) {
                            QLog.d(TAG, "ChannelService: ANT+ Inclination change requested: " + inclination + "%");
                            // Send broadcast intent to notify the main application
                            Intent intent = new Intent("org.cagnulen.qdomyoszwift.ANT_INCLINATION_CHANGE");
                            intent.putExtra("inclination", inclination);
                            nativeSetInclination(inclination);
                            sendBroadcast(intent);
                        }
                    });
                    
                    QLog.i(TAG, "BikeTransmitterController initialized successfully (bike mode)");
                    
                    // Start the bike transmitter immediately after initialization
                    boolean transmissionStarted = bikeTransmitterController.startTransmission();
                    if (transmissionStarted) {
                        QLog.i(TAG, "BikeTransmitterController transmission started automatically");
                    } else {
                        QLog.w(TAG, "Failed to start BikeTransmitterController transmission");
                    }
                } else {
                    QLog.e(TAG, "Failed to acquire channel for BikeTransmitterController");
                }
                
            } catch (Exception e) {
                QLog.e(TAG, "Failed to initialize BikeTransmitterController: " + e.getMessage());
                bikeTransmitterController = null;
            }
        }
    }

    private void closeAllChannels() {
        if (heartChannelController != null)
            heartChannelController.close();
        if (powerChannelController != null)
            powerChannelController.close();
        if (speedChannelController != null)
            speedChannelController.close();
        if (sdmChannelController != null)
            sdmChannelController.close();
        if (bikeChannelController != null)  // Added closing bikeChannelController
            bikeChannelController.close();
        if (bikeTransmitterController != null) {  // Added closing bikeTransmitterController
            bikeTransmitterController.close();  // Use close() method like other controllers
        }

        heartChannelController = null;
        powerChannelController = null;
        speedChannelController = null;
        sdmChannelController = null;
        bikeChannelController = null;  // Added nullifying bikeChannelController
        bikeTransmitterController = null;  // Added nullifying bikeTransmitterController
    }

    AntChannel acquireChannel() throws ChannelNotAvailableException {
        AntChannel mAntChannel = null;
        if (null != mAntChannelProvider) {
            try {
                /*
                 * If applications require a channel with specific capabilities
                 * (event buffering, background scanning etc.), a Capabilities
                 * object should be created and then the specific capabilities
                 * required set to true. Applications can specify both required
                 * and desired Capabilities with both being passed in
                 * acquireChannel(context, PredefinedNetwork,
                 * requiredCapabilities, desiredCapabilities).
                 */
                if (Ant.garminKey == false)
                    mAntChannel = mAntChannelProvider.acquireChannel(this, PredefinedNetwork.ANT_PLUS_1);
                else {
                    NetworkKey mNK = new NetworkKey(new byte[]{(byte) 0xb9, (byte) 0xa5, (byte) 0x21, (byte) 0xfb,
                            (byte) 0xbd, (byte) 0x72, (byte) 0xc3, (byte) 0x45});
                    QLog.v(TAG, mNK.toString());
                    mAntChannel = mAntChannelProvider.acquireChannelOnPrivateNetwork(this, mNK);
                }
            } catch (RemoteException e) {
                QLog.v(TAG, "ACP Remote Ex");
            } catch (UnsupportedFeatureException e) {
                QLog.v(TAG, "ACP UnsupportedFeature Ex");
            }
        }
        return mAntChannel;
    }

    @Override
    public IBinder onBind(Intent arg0) {
        return new ChannelServiceComm();
    }

    /**
     * Receives AntChannelProvider state changes being sent from ANT Radio Service
     */
    private final BroadcastReceiver mChannelProviderStateChangedReceiver = new BroadcastReceiver() {
        @Override
        public void onReceive(Context context, Intent intent) {
            QLog.d(TAG, "onReceive");
            if (AntChannelProvider.ACTION_CHANNEL_PROVIDER_STATE_CHANGED.equals(intent.getAction())) {
                boolean update = false;
                // Retrieving the data contained in the intent
                int numChannels = intent.getIntExtra(AntChannelProvider.NUM_CHANNELS_AVAILABLE, 0);
                boolean legacyInterfaceInUse = intent.getBooleanExtra(AntChannelProvider.LEGACY_INTERFACE_IN_USE, false);

                QLog.d(TAG, "onReceive" + mAllowAddChannel + " " +  numChannels + " " + legacyInterfaceInUse);

                if (mAllowAddChannel) {
                    // Was a acquire channel allowed
                    // If no channels available AND legacy interface is not in use, disallow acquiring of channels
                    if (0 == numChannels && !legacyInterfaceInUse) {
                        mAllowAddChannel = false;
                        update = true;
                        closeAllChannels();
                    }
                } else {
                    // Acquire channels not allowed
                    // If there are channels OR legacy interface in use, allow acquiring of channels
                    if (numChannels > 0 || legacyInterfaceInUse) {
                        mAllowAddChannel = true;
                        update = true;
                        QLog.i(TAG, "ANT+ channels are now available! (numChannels=" + numChannels + ") - Opening channels...");
                        try {
                            openAllChannels();
                        } catch (ChannelNotAvailableException exception) {
                            QLog.e(TAG, "Channel not available!! Exception: " + exception.getMessage());
                        }
                    }
                }
            }
        }
    };

    private void doBindAntRadioService() {
        if (BuildConfig.DEBUG) QLog.v(TAG, "doBindAntRadioService");

        ContextCompat.registerReceiver(
            this,
            mChannelProviderStateChangedReceiver,
            new IntentFilter(AntChannelProvider.ACTION_CHANNEL_PROVIDER_STATE_CHANGED),
            ContextCompat.RECEIVER_EXPORTED
        );

        // Creating the intent and calling context.bindService() is handled by
        // the static bindService() method in AntService
        mAntRadioServiceBound = AntService.bindService(this, mAntRadioServiceConnection);
    }

    private void doUnbindAntRadioService() {
        if (BuildConfig.DEBUG) QLog.v(TAG, "doUnbindAntRadioService");

        // Stop listing for channel available intents
        try {
            unregisterReceiver(mChannelProviderStateChangedReceiver);
        } catch (IllegalArgumentException exception) {
            if (BuildConfig.DEBUG)
                QLog.d(TAG, "Attempting to unregister a never registered Channel Provider State Changed receiver.");
        }

        if (mAntRadioServiceBound) {
            try {
                unbindService(mAntRadioServiceConnection);
            } catch (IllegalArgumentException e) {
                // Not bound, that's what we want anyway
            }

            mAntRadioServiceBound = false;
        }
    }

    @Override
    public void onCreate() {
        super.onCreate();

        mAntRadioServiceBound = false;

        doBindAntRadioService();

    }

    @Override
    public void onDestroy() {
        closeAllChannels();

        doUnbindAntRadioService();
        mAntChannelProvider = null;

        super.onDestroy();
    }

    static void die(String error) {
        QLog.e(TAG, "DIE: " + error);
    }

}