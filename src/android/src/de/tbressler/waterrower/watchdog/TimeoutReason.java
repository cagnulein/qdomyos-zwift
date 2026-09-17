package de.tbressler.waterrower.watchdog;

/**
 * The reason for a timeout at a watchdog.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public enum TimeoutReason {

    /* No message received in given interval. */
    PING_TIMEOUT,

    /* No confirmation of the device type received. */
    DEVICE_NOT_CONFIRMED_TIMEOUT

}
