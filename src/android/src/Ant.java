package org.cagnulen.qdomyoszwift;
import android.annotation.SuppressLint;
import android.app.ActionBar;
import android.app.Activity;
import android.bluetooth.BluetoothAdapter;
import android.bluetooth.BluetoothDevice;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.view.LayoutInflater;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.view.ViewGroup;
import android.widget.Button;
import android.widget.NumberPicker;
import android.widget.TextView;
import android.widget.Toast;
import org.cagnulen.qdomyoszwift.QLog;
import android.content.Intent;

public class Ant {
 private ChannelService.ChannelServiceComm mChannelService = null;
 private boolean mChannelServiceBound = false;
 private final String TAG = "Ant";
 public static Activity activity = null;
 static boolean speedRequest = false;
 static boolean heartRequest = false;
 static boolean bikeRequest = false; // Added bike request flag
 static boolean garminKey = false;
 static boolean treadmill = false;

 // Updated antStart method with BikeRequest parameter at the end
 public void antStart(Activity a, boolean SpeedRequest, boolean HeartRequest, boolean GarminKey, boolean Treadmill, boolean BikeRequest) {
     QLog.v(TAG, "antStart");
     speedRequest = SpeedRequest;
     heartRequest = HeartRequest;
     treadmill = Treadmill;
     garminKey = GarminKey;
     bikeRequest = BikeRequest; // Set bike request flag
     activity = a;
     if(a != null)
        QLog.v(TAG, "antStart activity is valid");
     else
     {
         QLog.v(TAG, "antStart activity is invalid");
         return;
     }

    if(!mChannelServiceBound) doBindChannelService();
 }

 private ServiceConnection mChannelServiceConnection = new ServiceConnection()
 {
    @Override
    public void onServiceConnected(ComponentName name, IBinder serviceBinder)
    {
        QLog.v(TAG, "mChannelServiceConnection.onServiceConnected...");
        mChannelService = (ChannelService.ChannelServiceComm) serviceBinder;
        QLog.v(TAG, "...mChannelServiceConnection.onServiceConnected");
    }

    @Override
    public void onServiceDisconnected(ComponentName arg0)
    {
        QLog.v(TAG, "mChannelServiceConnection.onServiceDisconnected...");
        // Clearing and disabling when disconnecting from ChannelService
        mChannelService = null;
        QLog.v(TAG, "...mChannelServiceConnection.onServiceDisconnected");
    }
 };

 private void doBindChannelService()
 {
    QLog.v(TAG, "doBindChannelService...");
    // Binds to ChannelService. ChannelService binds and manages connection between the
    // app and the ANT Radio Service
    mChannelServiceBound = activity.bindService(new Intent(activity, ChannelService.class), mChannelServiceConnection, Context.BIND_AUTO_CREATE);
    if(!mChannelServiceBound)   //If the bind returns false, run the unbind method to update the GUI
        doUnbindChannelService();
    QLog.i(TAG, "  Channel Service binding = "+ mChannelServiceBound);
    QLog.v(TAG, "...doBindChannelService");
 }

 public void doUnbindChannelService()
 {
    QLog.v(TAG, "doUnbindChannelService...");
    if(mChannelServiceBound)
    {
        activity.unbindService(mChannelServiceConnection);
        mChannelServiceBound = false;
    }
    QLog.v(TAG, "...doUnbindChannelService");
 }

 public void setCadenceSpeedPower(float speed, int power, int cadence)
 {
     if(mChannelService == null)
        return;
     QLog.v(TAG, "setCadenceSpeedPower " + speed + " " + power + " " + cadence);
     mChannelService.setSpeed(speed);
     mChannelService.setPower(power);
     mChannelService.setCadence(cadence);
 }

 public int getHeart()
 {
    if(mChannelService == null)
       return 0;
    QLog.v(TAG, "getHeart");
    return mChannelService.getHeart();
 }

 // Added bike-related getter methods
 public int getBikeCadence() {
    if(mChannelService == null)
       return 0;
    QLog.v(TAG, "getBikeCadence");
    return mChannelService.getBikeCadence();
 }

 public int getBikePower() {
    if(mChannelService == null)
       return 0;
    QLog.v(TAG, "getBikePower");
    return mChannelService.getBikePower();
 }

 public double getBikeSpeed() {
    if(mChannelService == null)
       return 0.0;
    QLog.v(TAG, "getBikeSpeed");
    return mChannelService.getBikeSpeed();
 }

 public long getBikeDistance() {
    if(mChannelService == null)
       return 0;
    QLog.v(TAG, "getBikeDistance");
    return mChannelService.getBikeDistance();
 }

 public boolean isBikeConnected() {
    if(mChannelService == null)
       return false;
    QLog.v(TAG, "isBikeConnected");
    return mChannelService.isBikeConnected();
 }
}
