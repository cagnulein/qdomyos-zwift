package org.cagnulen.qdomyoszwift;

import android.bluetooth.le.ScanCallback;
import android.bluetooth.le.ScanResult;
import org.cagnulen.qdomyoszwift.qlog as Log;

import java.util.List;

public class NativeScanCallback extends ScanCallback {
    private static final String TAG  = "NativeScanCallback";
    public native void newScanResult(ScanRecordResult srr);
    public native void scanError(int code);
    @Override
    public void onScanResult(int callbackType, ScanResult result) {
        Log.i(TAG, "Res " + result);
        newScanResult(new ScanRecordResult(result));
    }

    @Override
    public void onBatchScanResults(List<ScanResult> results) {
        for (ScanResult result: results)
            onScanResult(0, result);
    }

    @Override
    public void onScanFailed(int errorCode) {
        Log.i(TAG, "onScanFailed "+errorCode);
        scanError(errorCode);
    }
}
