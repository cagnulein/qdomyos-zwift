package de.tbressler.waterrower.workout;

import static com.google.common.base.MoreObjects.toStringHelper;
import static java.util.Objects.requireNonNull;

/**
 * A workout interval (a part of a workout).
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class WorkoutInterval {

    /* The rest interval. */
    private final int restInterval;

    /* The distance or duration (depending on the unit). */
    private final int value;

    /* The unit of the workout distance/duration. */
    private final WorkoutUnit unit;


    /**
     * A workout interval.
     *
     * @param value The distance (in meters/strokes) or duration (in seconds) of the workout. When unit = METERS,
     *                 MILES or KMS: this value is in Meters, the display value for miles is a conversion and valid
     *                 values are 0x0001 to 0xFA00. When unit = STROKES this value is the number of strokes and valid
     *                 values are 0x0001 to 0x1388. When unit = SECONDS this value is in seconds. Valid values are 0x0001
     *                 to 0x4650. This value is limited to 5 Hours, which is 18,000 seconds.
     * @param unit The unit of the workout distance/duration, must not be null.
     */
    public WorkoutInterval(int value, WorkoutUnit unit) {
        this(0, value, unit);
    }


    /**
     * A workout interval.
     *
     * @param restInterval The rest interval (in seconds) or 0 if no rest interval must be
     *                     set. Usually for single workouts or the first interval of an interval
     *                     workout. Valid values are 0x0000 to 0x0E10.
     * @param value The distance (in meters/strokes) or duration (in seconds) of the workout. When unit = METERS,
     *                 MILES or KMS: this value is in Meters, the display value for miles is a conversion and valid
     *                 values are 0x0001 to 0xFA00. When unit = STROKES this value is the number of strokes and valid
     *                 values are 0x0001 to 0x1388. When unit = SECONDS this value is in seconds. Valid values are 0x0001
     *                 to 0x4650. This value is limited to 5 Hours, which is 18,000 seconds.
     * @param unit The unit of the workout distance/duration, must not be null.
     */
    public WorkoutInterval(int restInterval, int value, WorkoutUnit unit) {
        this.restInterval = checkRestInterval(restInterval);
        this.unit = requireNonNull(unit);
        this.value = checkValue(value, unit);
    }

    /* Checks if the rest interval is in range. */
    private int checkRestInterval(int restInterval) {
        if ((restInterval < 0x0000) || (restInterval > 0x0E10))
            throw new IllegalArgumentException("The rest interval must be between 0x0000 and 0x0E10!");
        return restInterval;
    }

    /* Checks if distance or duration is in range. */
    private int checkValue(int distance, WorkoutUnit unit) {
        switch(unit) {
            case METERS:
            case MILES:
            case KMS:
                // When unit = METERS, MILES or KMS: this value is in Meters, the display value for
                // miles is a conversion and valid values are 0x0001 to 0xFA00.
                checkRange(distance, 0x0001, 0xFA00, "The distance of the workout must be between 0x0001 and 0xFA00!");
                break;
            case STROKES:
                // When unit = STROKES this value is the number of strokes and valid values are
                // 0x0001 to 0x1388.
                checkRange(distance, 0x0001, 0x1388, "The distance of the workout must be between 0x0001 and 0x1388!");
                break;
            case SECONDS:
                // When unit = SECONDS this value is in seconds. Valid values are 0x0001 to 0x4650. This value is limited
                // to 5 Hours, which is 18,000 seconds.
                checkRange(distance, 0x0001, 0x4650, "The duration of the workout must be between 0x0001 and 0x4650!");
                break;
        }
        return distance;
    }

    /* Checks if the range is correct. */
    private void checkRange(int distance, int min, int max, String msg) {
        if ((distance < min) || (distance > max))
            throw new IllegalArgumentException(msg);
    }


    /**
     * The rest interval in seconds.
     *
     * Can be 0 if no rest interval is specified. Usually this is the case if this is the first
     * interval of an interval workout.
     *
     * @return The rest interval.
     */
    public int getRestInterval() {
        return restInterval;
    }


    /**
     * Returns the distance or duration of the workout interval.
     *
     * @return The distance or duration.
     */
    public int getValue() {
        return value;
    }


    /**
     * Returns the unit of the workout distance/duration.
     *
     * @return The unit of the workout distance/duration.
     */
    public WorkoutUnit getUnit() {
        return unit;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("restInterval", restInterval)
                .add("value", value)
                .add("unit", unit)
                .toString();
    }

}
