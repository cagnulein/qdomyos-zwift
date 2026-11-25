#!/usr/bin/env python3
"""Host JVM regression test for the Qt Bluetooth permission helper.

The Android API doubles model the installed package's merged permission
metadata. This tests the Java decision, not Android split APK installation or BLE.
"""
from pathlib import Path
import subprocess
import tempfile

ROOT = Path(__file__).resolve().parent
JAVA = ROOT / "qtconnectivity/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth/QtBluetoothUtility.java"
PATCH = ROOT / "0006-Android-BluetoothScan-neverForLocation.patch"

# Keep the source backport and standalone JAR source in sync.
patch_java = PATCH.read_text().split("+++ b/src/android/bluetooth/src/org/qtproject/qt5/android/bluetooth/QtBluetoothUtility.java\n", 1)[1]
patch_lines = patch_java.splitlines()[1:]
assert "\n".join(line[1:] for line in patch_lines if line.startswith("+")) + "\n" == JAVA.read_text()

STUBS = {
    "android/os/Build.java": """
package android.os;
public class Build { public static class VERSION { public static int SDK_INT; } }
""",
    "android/util/Log.java": """
package android.util;
public class Log {
    public static int d(String tag, String msg) { return 0; }
    public static int w(String tag, String msg) { return 0; }
}
""",
    "android/content/pm/PackageInfo.java": """
package android.content.pm;
public class PackageInfo {
    public String[] requestedPermissions;
    public int[] requestedPermissionsFlags;
}
""",
    "android/content/pm/PackageManager.java": """
package android.content.pm;
public class PackageManager {
    public static final int GET_PERMISSIONS = 4096;
    public PackageInfo info;
    public boolean fail;
    public int calls;
    public PackageInfo getPackageInfo(String name, int flags) throws Exception {
        ++calls;
        if (flags != GET_PERMISSIONS) throw new AssertionError("GET_PERMISSIONS required");
        if (fail) throw new Exception("simulated query failure");
        return info;
    }
}
""",
    "android/content/Context.java": """
package android.content;
import android.content.pm.PackageManager;
public class Context {
    public final PackageManager pm = new PackageManager();
    public PackageManager getPackageManager() { return pm; }
    public String getPackageName() { return "test.qz"; }
}
""",
    "org/qtproject/qt5/android/bluetooth/Test.java": """
package org.qtproject.qt5.android.bluetooth;
import android.content.Context;
import android.content.pm.PackageInfo;
import android.os.Build;

public class Test {
    public static void main(String[] args) {
        Build.VERSION.SDK_INT = Integer.parseInt(args[0]);
        Context context = new Context();
        PackageInfo info = new PackageInfo();
        context.pm.info = info;
        String mode = args[1];
        String scan = "android.permission.BLUETOOTH_SCAN";
        info.requestedPermissions = new String[]{"other.permission", scan};
        info.requestedPermissionsFlags = new int[]{0, 0};
        boolean expected = true;
        if (mode.equals("never")) {
            info.requestedPermissionsFlags[1] = 0x10000 | 2;
            expected = Build.VERSION.SDK_INT < 31;
        } else if (mode.equals("missing")) {
            info.requestedPermissions = new String[]{"other.permission"};
            info.requestedPermissionsFlags = new int[]{0x10000};
        } else if (mode.equals("null")) {
            info.requestedPermissionsFlags = null;
        } else if (mode.equals("short")) {
            info.requestedPermissionsFlags = new int[]{0};
        } else if (mode.equals("exception")) {
            context.pm.fail = true;
            expected = Build.VERSION.SDK_INT < 31;
        }
        boolean actual = QtBluetoothUtility.bluetoothScanRequiresLocation(context);
        if (actual != expected) throw new AssertionError("SDK " + args[0] + " " + mode);
        int expectedCalls = Build.VERSION.SDK_INT < 31 ? 0 : 1;
        if (context.pm.calls != expectedCalls) throw new AssertionError("Unexpected query count");
        if (QtBluetoothUtility.bluetoothScanRequiresLocation(context) != expected
                || context.pm.calls != expectedCalls) throw new AssertionError("Cache regression");
    }
}
""",
}

with tempfile.TemporaryDirectory() as tmp:
    directory = Path(tmp)
    sources = [str(JAVA)]
    for path, content in STUBS.items():
        file = directory / path
        file.parent.mkdir(parents=True, exist_ok=True)
        file.write_text(content)
        sources.append(str(file))
    classes = directory / "classes"
    subprocess.run(["javac", "-source", "8", "-target", "8", "-d", str(classes), *sources], check=True)
    for sdk in (21, 23, 30, 31, 36, 37):
        for mode in ("never", "plain", "missing", "null", "short", "exception"):
            subprocess.run(["java", "-cp", str(classes), "org.qtproject.qt5.android.bluetooth.Test",
                            str(sdk), mode], check=True)
print("Qt Bluetooth permission regression: 36 cases passed")
