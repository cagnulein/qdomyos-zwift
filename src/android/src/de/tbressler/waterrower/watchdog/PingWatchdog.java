package de.tbressler.waterrower.watchdog;

import de.tbressler.waterrower.log.Log;

import java.time.Duration;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicLong;

import static de.tbressler.waterrower.watchdog.TimeoutReason.PING_TIMEOUT;
import static java.lang.System.currentTimeMillis;

/**
 * A watchdog which checks if a message (e.g. a ping) was received in a specified amount of time.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class PingWatchdog extends Watchdog {

    /* Maximum duration (in ms) between messages. If the duration between messages
     * exceeds, the method onTimeout() is called. */
    private final long maxPingDuration;

    /* Last time a ping was received. */
    private final AtomicLong lastReceivedPing = new AtomicLong(0);


    /**
     * A watchdog which checks if a message (e.g. a ping) was received in a specified amount of time.
     *
     * @param duration Maximum duration between messages, must not be null.
     * @param executorService The executor service, must not be null.
     */
    public PingWatchdog(Duration duration, ScheduledExecutorService executorService) {
        super(duration, true, executorService);
        this.maxPingDuration = duration.toMillis();
    }


    /**
     * Notifies the watchdog about a received message (e.g. a ping).
     */
    public void pingReceived() {
        lastReceivedPing.set(currentTimeMillis());
    }


    @Override
    protected void wakeUpAndCheck() {
        Log.debug("Checking if a message (e.g. ping) was received in the last "+maxPingDuration+" ms.");
        if (currentTimeMillis() - lastReceivedPing.get() > maxPingDuration) {
            Log.warn("No message (e.g. ping) received in the last "+maxPingDuration+" ms.");
            fireOnTimeout(PING_TIMEOUT);
        }
    }


    @Override
    public void start() {
        pingReceived();
        super.start();
    }


    @Override
    public void stop() {
        super.stop();
    }

}
