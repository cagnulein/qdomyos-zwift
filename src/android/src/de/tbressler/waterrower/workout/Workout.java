package de.tbressler.waterrower.workout;

import java.util.ArrayList;
import java.util.List;

import static com.google.common.base.MoreObjects.toStringHelper;
import static java.util.Objects.requireNonNull;

/**
 * A workout configuration.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class Workout {

    /* True if this workout is a single workout. */
    private boolean isSingleWorkout = true;

    /* A list of workout intervals. */
    private final List<WorkoutInterval> intervals = new ArrayList<>();

    /* The unit for the distance/duration of the single or interval workout. */
    private final WorkoutUnit unit;


    /**
     * A workout configuration.
     *
     * @param value The distance (in meters/strokes) or duration (in seconds) of the workout. When unit = METERS,
     *                 MILES or KMS: this value is in Meters, the display value for miles is a conversion and valid
     *                 values are 0x0001 to 0xFA00. When unit = STROKES this value is the number of strokes and valid
     *                 values are 0x0001 to 0x1388. When unit = SECONDS this value is in seconds. Valid values are 0x0001
     *                 to 0x4650. This value is limited to 5 Hours, which is 18,000 seconds.
     * @param unit The unit for the distance/duration of the single or interval workout.
     */
    public Workout(int value, WorkoutUnit unit) {
        this.unit = requireNonNull(unit);
        intervals.add(new WorkoutInterval(value, unit));
    }


    /**
     * Adds an interval to the workout. 8 additional intervals can be added.
     *
     * @param restInterval The rest interval (in seconds). Valid values are 0x0001 to 0x0E10.
     * @param value The distance/duration of the interval, using the same workout unit from the
     *                 first interval (constructor). When unit = METERS, MILES or KMS: this value
     *                 is in Meters, the display value for miles is a conversion and valid values
     *                 are 0x0001 to 0xFA00. When unit = STROKES this value is the number of
     *                 strokes and valid values are 0x0001 to 0x1388. When unit = SECONDS this
     *                 value is in seconds. Valid values are 0x0001 to 0x4650. This value is
     *                 limited to 5 Hours, which is 18,000 seconds.
     */
    public void addInterval(int restInterval, int value) {
        if (intervals.size() > 8)
            throw new IllegalStateException("Only 8 additional intervals allowed!");
        if (restInterval < 1)
            throw new IllegalArgumentException("Rest interval must be greater than 0!");
        isSingleWorkout = false;
        intervals.add(new WorkoutInterval(restInterval, value, unit));
    }


    /**
     * Returns true if this is a single workout (only one interval).
     *
     * @return True if this is a single workout.
     */
    public boolean isSingleWorkout() {
        return isSingleWorkout;
    }


    /**
     * Returns true if this is an interval workout (more than one interval).
     *
     * @return True if this is an interval workout.
     */
    public boolean isIntervalWorkout() {
        return !isSingleWorkout;
    }


    /**
     * Returns the unit of the workout distance/duration.
     *
     * @return The unit of the workout distance/duration.
     */
    public WorkoutUnit getUnit() {
        return unit;
    }


    /**
     * Returns the intervals of the workout.
     *
     * @return The intervals of the workout.
     */
    public List<WorkoutInterval> getWorkoutIntervals() {
        return intervals;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("unit", unit)
                .add("intervals(size)", intervals.size())
                .add("isSingleWorkout", isSingleWorkout)
                .toString();
    }

}