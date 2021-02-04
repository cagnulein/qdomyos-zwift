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
import android.util.Log;
import android.content.Intent;

public class Ant {

 private ChannelService.ChannelServiceComm mChannelService;
 private boolean mChannelServiceBound = false;
 private final String TAG = "Ant";
 private Activity activity = null;

 public void antStart(Activity a) {
	 Log.v(TAG, "antStart");
	 activity = a;
	 if(a != null)
	    Log.v(TAG, "antStart activity is valid");
	 else
	 {
		 Log.v(TAG, "antStart activity is invalid");
		 return;
	 }
    if(!mChannelServiceBound) doBindChannelService();
 }

 private ServiceConnection mChannelServiceConnection = new ServiceConnection()
 {
	@Override
	public void onServiceConnected(ComponentName name, IBinder serviceBinder)
	{
		Log.v(TAG, "mChannelServiceConnection.onServiceConnected...");

		mChannelService = (ChannelService.ChannelServiceComm) serviceBinder;


		Log.v(TAG, "...mChannelServiceConnection.onServiceConnected");
		}

	@Override
	public void onServiceDisconnected(ComponentName arg0)
	{
		Log.v(TAG, "mChannelServiceConnection.onServiceDisconnected...");

		// Clearing and disabling when disconnecting from ChannelService
		mChannelService = null;

		Log.v(TAG, "...mChannelServiceConnection.onServiceDisconnected");
		}
	};

 private void doBindChannelService()
 {
	Log.v(TAG, "doBindChannelService...");

	// Binds to ChannelService. ChannelService binds and manages connection between the
	// app and the ANT Radio Service
	mChannelServiceBound = activity.bindService(new Intent(activity, ChannelService.class), mChannelServiceConnection , Context.BIND_AUTO_CREATE);

	if(!mChannelServiceBound)   //If the bind returns false, run the unbind method to update the GUI
	doUnbindChannelService();

	Log.i(TAG, "  Channel Service binding = "+ mChannelServiceBound);

	Log.v(TAG, "...doBindChannelService");
	}

 private void doUnbindChannelService()
 {
	Log.v(TAG, "doUnbindChannelService...");

	if(mChannelServiceBound)
	{
		activity.unbindService(mChannelServiceConnection);

		mChannelServiceBound = false;
		}

	Log.v(TAG, "...doUnbindChannelService");
	}

 public void setCadenceSpeedPower(float speed, int power, int cadence)
 {
	 mChannelService.setSpeed(speed / 10.0);
	 mChannelService.setPower(power / 10);
	 mChannelService.setCadence(cadence);
 }
}
