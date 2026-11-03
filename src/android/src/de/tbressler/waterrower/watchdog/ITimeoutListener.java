package de.tbressler.waterrower.watchdog;

/**
 * Interface for watchdog.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public interface ITimeoutListener {

    /**
     * Called if a timeout occurs at a watchdog.
     *
     * @param reason The reason for the timeout.
     */
    void onTimeout(TimeoutReason reason);

}
