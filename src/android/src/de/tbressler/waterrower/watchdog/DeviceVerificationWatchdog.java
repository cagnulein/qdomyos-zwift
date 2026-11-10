package de.tbressler.waterrower.watchdog;

import de.tbressler.waterrower.log.Log;

import java.time.Duration;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;

import static de.tbressler.waterrower.watchdog.TimeoutReason.DEVICE_NOT_CONFIRMED_TIMEOUT;

/**
 * A watchdog which checks after the given amount of time, if the device is confirmed.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class DeviceVerificationWatchdog extends Watchdog {

    /* True if the device is confirmed. */
    private final AtomicBoolean deviceConfirmed = new AtomicBoolean(false);


    /**
     * A watchdog which checks after the given amount of time, if the device is confirmed.
     *
     * @param duration The duration when to check if the device is confirmed, must not be null.
     * @param executorService The executor service, must not be null.
     */
    public DeviceVerificationWatchdog(Duration duration, ScheduledExecutorService executorService) {
        super(duration, false, executorService);
    }


    /**
     * Set the device as confirmed or unconfirmed.
     *
     * @param isConfirmed True if the device is confirmed.
     */
    public void setDeviceConfirmed(boolean isConfirmed) {
        deviceConfirmed.set(isConfirmed);
    }

    /**
     * Returns true if the device is confirmed.
     *
     * @return True if the device is confirmed.
     */
    public boolean isDeviceConfirmed() {
        return deviceConfirmed.get();
    }


    @Override
    protected final void wakeUpAndCheck() {
        Log.debug("Checking if device type is confirmed.");
        if (!isDeviceConfirmed()) {
            Log.warn("The device type was not confirmed yet!");
            fireOnTimeout(DEVICE_NOT_CONFIRMED_TIMEOUT);
        }
    }


    @Override
    public void start() {
        setDeviceConfirmed(false);
        super.start();
    }


    @Override
    public void stop() {
        super.stop();
    }

}
