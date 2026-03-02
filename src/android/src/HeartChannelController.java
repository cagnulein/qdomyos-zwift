package org.cagnulen.qdomyoszwift;

import android.content.Context;
import org.cagnulen.qdomyoszwift.QLog;
import android.app.Activity;

// ANT+ Plugin imports
import com.dsi.ant.plugins.antplus.pcc.AntPlusHeartRatePcc;
import com.dsi.ant.plugins.antplus.pcc.AntPlusHeartRatePcc.DataState;
import com.dsi.ant.plugins.antplus.pcc.AntPlusHeartRatePcc.IHeartRateDataReceiver;
import com.dsi.ant.plugins.antplus.pcc.defines.DeviceState;
import com.dsi.ant.plugins.antplus.pcc.defines.EventFlag;
import com.dsi.ant.plugins.antplus.pcc.defines.RequestAccessResult;
import com.dsi.ant.plugins.antplus.pccbase.AntPluginPcc.IDeviceStateChangeReceiver;
import com.dsi.ant.plugins.antplus.pccbase.AntPluginPcc.IPluginAccessResultReceiver;
import com.dsi.ant.plugins.antplus.pccbase.PccReleaseHandle;

// Basic ANT imports for legacy support
import com.dsi.ant.channel.AntChannel;
import com.dsi.ant.channel.AntCommandFailedException;
import com.dsi.ant.channel.IAntChannelEventHandler;
import com.dsi.ant.message.ChannelId;
import com.dsi.ant.message.ChannelType;
import com.dsi.ant.message.EventCode;
import com.dsi.ant.message.fromant.AcknowledgedDataMessage;
import com.dsi.ant.message.fromant.BroadcastDataMessage;
import com.dsi.ant.message.fromant.ChannelEventMessage;
import com.dsi.ant.message.fromant.MessageFromAntType;
import com.dsi.ant.message.ipc.AntMessageParcel;

// Java imports
import java.math.BigDecimal;
import java.util.EnumSet;
import java.util.Random;

public class HeartChannelController {
    private static final String TAG = HeartChannelController.class.getSimpleName();
    
    private Context context;
    private AntPlusHeartRatePcc hrPcc = null;
    private PccReleaseHandle<AntPlusHeartRatePcc> releaseHandle = null;
    private boolean isConnected = false;
    public int heart = 0; // Public to be accessible from ChannelService

    public HeartChannelController(int antHeartDeviceNumber) {
        this.context = Ant.activity;
        openChannel(antHeartDeviceNumber);
    }

    public boolean openChannel(int deviceNumber) {
        // Request access to heart rate device (deviceNumber = 0 means first available)
        releaseHandle = AntPlusHeartRatePcc.requestAccess((Activity)context, deviceNumber, 0,
            new IPluginAccessResultReceiver<AntPlusHeartRatePcc>() {
                @Override
                public void onResultReceived(AntPlusHeartRatePcc result, RequestAccessResult resultCode, DeviceState initialDeviceState) {
                    switch(resultCode) {
                        case SUCCESS:
                            hrPcc = result;
                            isConnected = true;
                            QLog.d(TAG, "Connected to heart rate monitor: " + result.getDeviceName() + " (Device #" + deviceNumber + ")");
                            subscribeToHrEvents();
                            break;
                        case CHANNEL_NOT_AVAILABLE:
                            QLog.e(TAG, "Channel Not Available");
                            break;
                        case ADAPTER_NOT_DETECTED:
                            QLog.e(TAG, "ANT Adapter Not Available");
                            break;
                        case BAD_PARAMS:
                            QLog.e(TAG, "Bad request parameters");
                            break;
                        case OTHER_FAILURE:
                            QLog.e(TAG, "RequestAccess failed");
                            break;
                        case DEPENDENCY_NOT_INSTALLED:
                            QLog.e(TAG, "Dependency not installed");
                            break;
                        default:
                            QLog.e(TAG, "Unrecognized result: " + resultCode);
                            break;
                    }
                }
            },
            new IDeviceStateChangeReceiver() {
                @Override
                public void onDeviceStateChange(DeviceState newDeviceState) {
                    QLog.d(TAG, "Device State Changed to: " + newDeviceState);
                    if (newDeviceState == DeviceState.DEAD) {
                        isConnected = false;
                    }
                }
            }
        );

        return isConnected;
    }

    private void subscribeToHrEvents() {
        if (hrPcc != null) {
            hrPcc.subscribeHeartRateDataEvent(new IHeartRateDataReceiver() {
                @Override
                public void onNewHeartRateData(long estTimestamp, EnumSet<EventFlag> eventFlags,
                    int computedHeartRate, long heartBeatCount,
                    BigDecimal heartBeatEventTime, DataState dataState) {
                    
                    heart = computedHeartRate;
                    QLog.d(TAG, "Heart Rate: " + heart);
                }
            });
        }
    }

    public void close() {
        if (releaseHandle != null) {
            releaseHandle.close();
            releaseHandle = null;
        }
        hrPcc = null;
        isConnected = false;
        QLog.d(TAG, "Channel Closed");
    }

    public int getHeartRate() {
        return heart;
    }

    public boolean isConnected() {
        return isConnected;
    }
}
