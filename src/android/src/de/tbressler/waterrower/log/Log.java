package de.tbressler.waterrower.log;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Simple singleton for logging.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class Log {

    /* The logger. */
    private static final Logger logger = LoggerFactory.getLogger("WaterRowerLibrary");


    /* Private constructor. */
    private Log() {}


    /** Logs debug messages. */
    public static void debug(String msg) {
        logger.debug(msg);
    }

    /** Logs debug messages. */
    public static void info(String msg) {
        logger.info(msg);
    }

    /** Logs warning messages. */
    public static void warn(String msg) {
        logger.warn(msg);
    }

    /** Logs error messages. */
    public static void error(String msg, Throwable t) {
        logger.error(msg, t);
    }

}
