package de.tbressler.waterrower.model;

/**
 * Simple error codes which identifies the type of error.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public enum ErrorCode {

    /* The device is not supported. */
    DEVICE_NOT_SUPPORTED,

    /* IO or connection error. */
    COMMUNICATION_FAILED,

    /* Communication timed out (no ping received). */
    TIMEOUT,

    /* The WaterRower monitor sent a error message. */
    ERROR_MESSAGE_RECEIVED,

}