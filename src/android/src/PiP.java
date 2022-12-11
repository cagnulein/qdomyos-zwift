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
import android.app.PictureInPictureParams;
import android.util.Rational;
import android.provider.Settings;
import android.view.Display;
import android.graphics.Point;

public class PiP {

    public static void enterPiP(Activity a) {
        Display d = a.getWindowManager()
                    .getDefaultDisplay();
        Point p = new Point();
        d.getSize(p);
        int width = p.x;
        int height = p.y;

        Rational ratio
            = new Rational(width, height);
        PictureInPictureParams.Builder
            pip_Builder
            = new PictureInPictureParams
                  .Builder();
        pip_Builder.setAspectRatio(ratio).build();
        Log.v("QZ", "Pip");
        a.enterPictureInPictureMode(pip_Builder.build());
    }
}
