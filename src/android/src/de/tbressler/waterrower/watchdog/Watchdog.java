package de.tbressler.waterrower.watchdog;

import java.time.Duration;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;

import static java.util.Objects.requireNonNull;
import static java.util.concurrent.TimeUnit.MILLISECONDS;

/**
 * An abstract watchdog.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class Watchdog {

    /* The wakeup interval for the watchdog. */
    private final Duration interval;

    /* Repeat the watchdog periodically. */
    private final boolean doRepeat;

    /* The executor service. */
    private final ScheduledExecutorService executorService;

    /* True if watchdog is stopped. */
    private final AtomicBoolean isStopped = new AtomicBoolean(true);

    /* The listener that should be notified on timeout. */
    private ITimeoutListener timeoutListener;


    /**
     * A watchdog.
     *
     * @param interval The interval, must not be null.
     * @param repeat True if the watchdog task should be repeated periodically.
     * @param executorService The scheduled executor service, must not be null.
     */
    public Watchdog(Duration interval, boolean repeat, ScheduledExecutorService executorService) {
        this.interval = requireNonNull(interval);
        this.doRepeat = repeat;
        this.executorService = requireNonNull(executorService);
    }


    /**
     * Starts the watchdog.
     */
    public void start() {
        isStopped.set(false);
        scheduleWatchdogTask();
    }

    /* Schedule the watchdog task for execution. */
    private void scheduleWatchdogTask() {
        executorService.schedule(this::executeWatchdogTask, interval.toMillis(), MILLISECONDS);
    }

    /* Execute the watchdog task. */
    private void executeWatchdogTask() {

        // Check if already stopped.
        if (isStopped.get())
            return;

        wakeUpAndCheck();

        // Start the next period if the task should
        // be executed periodically.
        if (doRepeat && !isStopped.get())
            scheduleWatchdogTask();
    }


    /**
     * The task that should be executed, when the watchdog wakes up. Please call
     * #fireOnTimeout(...) if a timeout was detected.
     */
    protected abstract void wakeUpAndCheck();


    /**
     * Notifies the listener about a timeout.
     *
     * @param reason The reason for the timeout, must not be null.
     */
    protected void fireOnTimeout(TimeoutReason reason) {
        if (timeoutListener == null)
            return;
        timeoutListener.onTimeout(requireNonNull(reason));
    }


    /**
     * Sets a timeout listener.
     *
     * @param listener The timeout listener or null.
     */
    public void setTimeoutListener(ITimeoutListener listener) {
        this.timeoutListener = listener;
    }


    /**
     * Stops the watchdog.
     */
    public void stop() {
        isStopped.set(true);
    }

}
