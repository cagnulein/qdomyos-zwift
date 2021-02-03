package org.cagnulen.qdomyoszwift;

import android.content.Intent;

private ChannelService.ChannelServiceComm mChannelService;
private boolean mChannelServiceBound = false;

public class MyActivity extends org.qtproject.qt5.android.bindings.QtActivity {
 @Override
 public void onCreate(android.os.Bundle savedInstanceState){
	 super.onCreate(savedInstanceState);
	 this.getWindow().addFlags(android.view.WindowManager.LayoutParams.FLAG_KEEP_SCREEN_ON);
 }

 public void antStart() {
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
	mChannelServiceBound = getActivity().bindService(new Intent(getActivity(), ChannelService.class), mChannelServiceConnection , Context.BIND_AUTO_CREATE);

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
		getActivity().unbindService(mChannelServiceConnection);

		mChannelServiceBound = false;
		}

	Log.v(TAG, "...doUnbindChannelService");
	}

 public void setCadenceSpeedPower(float speed, float power, int cadence)
 {
	 mChannelService.setSpeed(speed / 10.0);
	 mChannelService.setPower(power / 10);
	 mChannelService.setCadence(cadence);
 }
}
