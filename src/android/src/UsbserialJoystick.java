package org.cagnulen.qdomyoszwift;

import android.app.PendingIntent;
import android.content.BroadcastReceiver;
import android.content.Context;
import android.content.Intent;
import android.content.IntentFilter;
import android.hardware.usb.UsbDevice;
import android.hardware.usb.UsbDeviceConnection;
import android.hardware.usb.UsbManager;
import org.cagnulen.qdomyoszwift.QLog;
import android.os.Build;
import androidx.core.content.ContextCompat;

import com.hoho.android.usbserial.driver.UsbSerialDriver;
import com.hoho.android.usbserial.driver.UsbSerialPort;
import com.hoho.android.usbserial.driver.UsbSerialProber;

import java.io.IOException;
import java.util.List;

/**
 * USB Serial Joystick handler for Kettler joystick
 * Reads modem control lines (DCD, CTS, DSR, RI) for button detection
 */
public class UsbserialJoystick {

    private static UsbSerialPort joystickPort = null;
    private static int deviceIndex = -1;

    /**
     * Discover and open the joystick device
     * @param context Android context
     * @param skipDeviceIndex Index of device to skip (the main bike device)
     * @return true if joystick found and opened
     */
    public static boolean discover(Context context, int skipDeviceIndex) {
        QLog.d("QZ", "UsbserialJoystick: Discovering joystick device, skipping index " + skipDeviceIndex);

        UsbManager manager = (UsbManager) context.getSystemService(Context.USB_SERVICE);
        List<UsbSerialDriver> availableDrivers = UsbSerialProber.getDefaultProber().findAllDrivers(manager);

        if (availableDrivers.isEmpty()) {
            QLog.d("QZ", "UsbserialJoystick: No USB serial devices found");
            return false;
        }

        QLog.d("QZ", "UsbserialJoystick: Found " + availableDrivers.size() + " USB serial devices");

        // Try each device and identify which is the joystick (doesn't respond to Kettler commands)
        for (int i = 0; i < availableDrivers.size(); i++) {
            UsbSerialDriver driver = availableDrivers.get(i);
            QLog.d("QZ", "UsbserialJoystick: Trying device at index " + i);

            // Request permissions if needed
            if (!manager.hasPermission(driver.getDevice())) {
                QLog.d("QZ", "UsbserialJoystick: Requesting USB permission for device " + i);
                final Boolean[] granted = {null};
                BroadcastReceiver usbReceiver = new BroadcastReceiver() {
                    @Override
                    public void onReceive(Context context, Intent intent) {
                        granted[0] = intent.getBooleanExtra(UsbManager.EXTRA_PERMISSION_GRANTED, false);
                    }
                };
                int flags = Build.VERSION.SDK_INT >= Build.VERSION_CODES.M ? PendingIntent.FLAG_IMMUTABLE : 0;
                PendingIntent permissionIntent = PendingIntent.getBroadcast(context, 0,
                    new Intent("org.cagnulen.qdomyoszwift.USB_PERMISSION_JOYSTICK"), flags);
                IntentFilter filter = new IntentFilter("org.cagnulen.qdomyoszwift.USB_PERMISSION_JOYSTICK");
                ContextCompat.registerReceiver(
                    context,
                    usbReceiver,
                    filter,
                    ContextCompat.RECEIVER_EXPORTED
                );
                manager.requestPermission(driver.getDevice(), permissionIntent);

                // Wait for permission (max 5 seconds)
                for (int j = 0; j < 5000; j++) {
                    if (granted[0] != null) break;
                    try {
                        Thread.sleep(1);
                    } catch (InterruptedException e) {
                        // Ignore
                    }
                }

                QLog.d("QZ", "UsbserialJoystick: USB permission " + granted[0]);
                if (granted[0] == null || !granted[0]) {
                    continue; // Try next device
                }
            }

            // Try to open this device
            UsbDeviceConnection connection = manager.openDevice(driver.getDevice());
            if (connection == null) {
                QLog.d("QZ", "UsbserialJoystick: Failed to open device " + i);
                continue;
            }

            UsbSerialPort testPort = driver.getPorts().get(0);
            try {
                testPort.open(connection);
                testPort.setParameters(9600, 8, UsbSerialPort.STOPBITS_1, UsbSerialPort.PARITY_NONE);

                // Test if we can read control lines
                boolean cd = testPort.getCD();
                boolean cts = testPort.getCTS();
                boolean dsr = testPort.getDSR();
                boolean ri = testPort.getRI();

                QLog.d("QZ", "UsbserialJoystick: Device " + i + " control lines: CD=" + cd +
                       " CTS=" + cts + " DSR=" + dsr + " RI=" + ri);

                // Test if this device responds to Kettler commands (then it's the bike, not joystick)
                // Send a simple command and check for response
                String testCommand = "VE\r\n";
                testPort.write(testCommand.getBytes(java.nio.charset.StandardCharsets.US_ASCII), 500);

                // Wait a bit for response
                try {
                    Thread.sleep(100);
                } catch (InterruptedException e) {
                    // Ignore
                }

                byte[] buffer = new byte[256];
                int bytesRead = 0;
                try {
                    bytesRead = testPort.read(buffer, 200);
                } catch (Exception e) {
                    // Timeout or no data - this is expected for joystick
                }

                if (bytesRead > 0) {
                    // Device responded - this is likely the bike, not the joystick
                    QLog.d("QZ", "UsbserialJoystick: Device " + i + " responded to command (" +
                           bytesRead + " bytes), skipping (likely bike device)");
                    testPort.close();
                    continue;
                }

                QLog.d("QZ", "UsbserialJoystick: Device " + i + " did not respond - this is the joystick");

                // This is our joystick device (supports control lines but doesn't respond to commands)
                joystickPort = testPort;
                deviceIndex = i;
                QLog.d("QZ", "UsbserialJoystick: Successfully opened joystick on device " + i);
                return true;

            } catch (IOException e) {
                QLog.d("QZ", "UsbserialJoystick: Failed to configure device " + i + ": " + e.getMessage());
                try {
                    testPort.close();
                } catch (IOException e2) {
                    // Ignore
                }
            } catch (UnsupportedOperationException e) {
                QLog.d("QZ", "UsbserialJoystick: Device " + i + " doesn't support control lines");
                try {
                    testPort.close();
                } catch (IOException e2) {
                    // Ignore
                }
            }
        }

        QLog.d("QZ", "UsbserialJoystick: No suitable joystick device found");
        return false;
    }

    /**
     * Get the state of modem control lines
     * Returns an array of 4 booleans: [DCD, CTS, DSR, RI]
     */
    public static boolean[] getControlLines() {
        boolean[] lines = new boolean[4];

        if (joystickPort == null) {
            return lines;
        }

        try {
            lines[0] = joystickPort.getCD();  // DCD - Left arrow
            lines[1] = joystickPort.getCTS(); // CTS - Right arrow
            lines[2] = joystickPort.getDSR(); // DSR - Down arrow
            lines[3] = joystickPort.getRI();  // RI (RING) - Up arrow
        } catch (IOException e) {
            QLog.d("QZ", "UsbserialJoystick: Error reading control lines: " + e.getMessage());
        } catch (UnsupportedOperationException e) {
            QLog.d("QZ", "UsbserialJoystick: Control lines not supported");
        }

        return lines;
    }

    /**
     * Close the joystick port
     */
    public static void close() {
        if (joystickPort != null) {
            try {
                joystickPort.close();
                QLog.d("QZ", "UsbserialJoystick: Port closed");
            } catch (IOException e) {
                QLog.d("QZ", "UsbserialJoystick: Error closing port: " + e.getMessage());
            }
            joystickPort = null;
            deviceIndex = -1;
        }
    }

    /**
     * Check if joystick is connected
     */
    public static boolean isConnected() {
        return joystickPort != null;
    }
}
