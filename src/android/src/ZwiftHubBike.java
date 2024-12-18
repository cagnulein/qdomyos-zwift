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
import com.garmin.android.connectiq.ConnectIQ;
import com.garmin.android.connectiq.ConnectIQAdbStrategy;
import com.garmin.android.connectiq.IQApp;
import com.garmin.android.connectiq.IQDevice;
import com.garmin.android.connectiq.exception.InvalidStateException;
import com.garmin.android.connectiq.exception.ServiceUnavailableException;
import android.content.BroadcastReceiver;
import android.content.ContextWrapper;
import android.content.IntentFilter;
import android.widget.Toast;

import org.jetbrains.annotations.Nullable;

import com.google.protobuf.InvalidProtocolBufferException;

import java.util.HashMap;
import java.util.List;

public class ZwiftHubBike {

    private static Context context;

    private static final String TAG = "ZwiftHubBike: ";

    public static byte[] inclinationCommand(double inclination) throws InvalidProtocolBufferException {
        ZwiftHub.SimulationParam.Builder simulation = ZwiftHub.SimulationParam.newBuilder();
        simulation.setInclineX100((int)(inclination * 100.0));

        ZwiftHub.HubCommand.Builder command = ZwiftHub.HubCommand.newBuilder();
        command.setSimulation(simulation.build());

        byte[] data = command.build().toByteArray();
        byte[] fullData = new byte[data.length + 1];
        fullData[0] = 0x04;
        System.arraycopy(data, 0, fullData, 1, data.length);

        return fullData;
    }

    public static byte[] setGearCommand(int gears) throws InvalidProtocolBufferException {
        ZwiftHub.PhysicalParam.Builder physical = ZwiftHub.PhysicalParam.newBuilder();
        physical.setGearRatioX10000(gears);

        ZwiftHub.HubCommand.Builder command = ZwiftHub.HubCommand.newBuilder();
        command.setPhysical(physical.build());

        byte[] data = command.build().toByteArray();
        byte[] fullData = new byte[data.length + 1];
        fullData[0] = 0x04;
        System.arraycopy(data, 0, fullData, 1, data.length);

        return fullData;
    }
}
