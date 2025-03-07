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
import android.util.Log;
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

    HeartChannelController heartChannelController = null;
    PowerChannelController powerChannelController = null;
    SpeedChannelController speedChannelController = null;
    SDMChannelController sdmChannelController = null;
    BikeChannelController bikeChannelController = null; // Added BikeChannelController reference

    private ServiceConnection mAntRadioServiceConnection = new ServiceConnection() {
        @Override
        public void onServiceConnected(ComponentName name, IBinder service) {
            Log.v(TAG, "onServiceConnected");
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

                Log.v(TAG, "onServiceConnected mChannelAvailable=" + mChannelAvailable + " legacyInterfaceInUse=" + legacyInterfaceInUse);

                // If there are channels OR legacy interface in use, allow adding channels
                if (mChannelAvailable || legacyInterfaceInUse) {
                    mAllowAddChannel = true;
                } else {
                    // If no channels available AND legacy interface is not in use, disallow adding channels
                    mAllowAddChannel = false;
                }

                try {
                    openAllChannels();
                } catch (ChannelNotAvailableException exception) {
                    Log.e(TAG, "Channel not available!!");
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
        }

        void setPower(int power) {
            if (null != powerChannelController) {
                powerChannelController.power = power;
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
        }

        int getHeart() {
            if (null != heartChannelController) {
                Log.v(TAG, "getHeart");
                return heartChannelController.heart;
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

        /**
         * Closes all channels currently added.
         */
        void clearAllChannels() {
            closeAllChannels();
        }
    }

    public void openAllChannels() throws ChannelNotAvailableException {
        if (Ant.heartRequest && heartChannelController == null)
            heartChannelController = new HeartChannelController();

        if (Ant.speedRequest) {
            if(Ant.treadmill && sdmChannelController == null) {
                sdmChannelController = new SDMChannelController(acquireChannel());
            } else if(powerChannelController == null) {
                powerChannelController = new PowerChannelController(acquireChannel());
                speedChannelController = new SpeedChannelController(acquireChannel());
            }
        }

        // Add initialization for BikeChannelController
        if (Ant.bikeRequest && bikeChannelController == null) {
            bikeChannelController = new BikeChannelController();
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

        heartChannelController = null;
        powerChannelController = null;
        speedChannelController = null;
        sdmChannelController = null;
        bikeChannelController = null;  // Added nullifying bikeChannelController
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
                    Log.v(TAG, mNK.toString());
                    mAntChannel = mAntChannelProvider.acquireChannelOnPrivateNetwork(this, mNK);
                }
            } catch (RemoteException e) {
                Log.v(TAG, "ACP Remote Ex");
            } catch (UnsupportedFeatureException e) {
                Log.v(TAG, "ACP UnsupportedFeature Ex");
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
            Log.d(TAG, "onReceive");
            if (AntChannelProvider.ACTION_CHANNEL_PROVIDER_STATE_CHANGED.equals(intent.getAction())) {
                boolean update = false;
                // Retrieving the data contained in the intent
                int numChannels = intent.getIntExtra(AntChannelProvider.NUM_CHANNELS_AVAILABLE, 0);
                boolean legacyInterfaceInUse = intent.getBooleanExtra(AntChannelProvider.LEGACY_INTERFACE_IN_USE, false);

                Log.d(TAG, "onReceive" + mAllowAddChannel + " " +  numChannels + " " + legacyInterfaceInUse);

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
                        try {
                            openAllChannels();
                        } catch (ChannelNotAvailableException exception) {
                            Log.e(TAG, "Channel not available!!");
                        }
                    }
                }
            }
        }
    };

    private void doBindAntRadioService() {
        if (BuildConfig.DEBUG) Log.v(TAG, "doBindAntRadioService");

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
        if (BuildConfig.DEBUG) Log.v(TAG, "doUnbindAntRadioService");

        // Stop listing for channel available intents
        try {
            unregisterReceiver(mChannelProviderStateChangedReceiver);
        } catch (IllegalArgumentException exception) {
            if (BuildConfig.DEBUG)
                Log.d(TAG, "Attempting to unregister a never registered Channel Provider State Changed receiver.");
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
        Log.e(TAG, "DIE: " + error);
    }

}
