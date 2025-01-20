package de.tbressler.waterrower.subscriptions.workouts;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.model.MemoryLocation;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.DOUBLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.*;
import static de.tbressler.waterrower.subscriptions.Priority.LOW;
import static de.tbressler.waterrower.utils.MessageUtils.intFromHighAndLow;
import static java.util.Objects.requireNonNull;

/**
 * Subscription for the workout interval values.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class WorkoutIntervalValueSubscription extends AbstractMemorySubscription {

    /**
     * The interval type (e.g. row or rest).
     */
    public enum IntervalType {

        /* Rest interval. */
        REST_INTERVAL,

        /* Row interval. */
        ROW_INTERVAL
    }


    /* The interval type. */
    private final IntervalType intervalType;

    /* The interval index. */
    private final int intervalIndex;


    /* The last value received. */
    private int lastValue = -1;


    /**
     * Subscription for the workout interval values.
     *
     * @param intervalType The interval type (e.g. row or rest), must not be null.
     * @param intervalIndex The index of the workout interval. Must be between 1 and 9 for
     *                      interval type REST_INTERVAL and between 1 and 8 for interval type
     *                      ROW_INTERVAL.
     */
    public WorkoutIntervalValueSubscription(IntervalType intervalType, int intervalIndex) {
        this(LOW, intervalType, intervalIndex);
    }

    /**
     * Subscription for the workout interval values.
     *
     * @param priority The priority (recommended LOW).
     * @param intervalType The interval type (e.g. row or rest), must not be null.
     * @param intervalIndex The index of the workout interval. Must be between 1 and 9 for
     *                      interval type REST_INTERVAL and between 1 and 8 for interval type
     *                      ROW_INTERVAL.
     */
    public WorkoutIntervalValueSubscription(Priority priority, IntervalType intervalType, int intervalIndex) {
        super(priority, DOUBLE_MEMORY, getMemoryLocation(intervalType, intervalIndex));
        this.intervalType = intervalType;
        this.intervalIndex = intervalIndex;
    }

    /* Returns the memory location for the given interval type and interval index. */
    private static MemoryLocation getMemoryLocation(IntervalType intervalType, int intervalIndex) {
        switch (requireNonNull(intervalType)) {
            case ROW_INTERVAL:
                if (intervalIndex == 1)  return WORKOUT_WORK1_L;
                else if (intervalIndex == 2)  return WORKOUT_WORK2_L;
                else if (intervalIndex == 3)  return WORKOUT_WORK3_L;
                else if (intervalIndex == 4)  return WORKOUT_WORK4_L;
                else if (intervalIndex == 5)  return WORKOUT_WORK5_L;
                else if (intervalIndex == 6)  return WORKOUT_WORK6_L;
                else if (intervalIndex == 7)  return WORKOUT_WORK7_L;
                else if (intervalIndex == 8)  return WORKOUT_WORK8_L;
                else if (intervalIndex == 9)  return WORKOUT_WORK9_L;
                else throw new IllegalArgumentException("Interval index is out of range! Index must be between 1 and 9.");
            case REST_INTERVAL:
                if (intervalIndex == 1)  return WORKOUT_REST1_L;
                else if (intervalIndex == 2)  return WORKOUT_REST2_L;
                else if (intervalIndex == 3)  return WORKOUT_REST3_L;
                else if (intervalIndex == 4)  return WORKOUT_REST4_L;
                else if (intervalIndex == 5)  return WORKOUT_REST5_L;
                else if (intervalIndex == 6)  return WORKOUT_REST6_L;
                else if (intervalIndex == 7)  return WORKOUT_REST7_L;
                else if (intervalIndex == 8)  return WORKOUT_REST8_L;
                else throw new IllegalArgumentException("Interval index is out of range! Index must be between 1 and 8.");
            default:
                throw new IllegalStateException("Unhandled interval type!");
        }
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {
        int value = intFromHighAndLow(msg.getValue2(), msg.getValue1());

        // If the received value is the same as before,
        // don't send an update.
        if (lastValue == value)
            return;
        lastValue = value;

        onWorkoutIntervalUpdated(intervalType, intervalIndex, value);
    }


    /**
     * Is called if the value for the workout interval was updated.
     *
     * @param intervalType The interval type (e.g. row or rest), never null.
     * @param intervalIndex The index of the workout interval.
     * @param value The new value (the unit depends on the interval and workout type).
     */
    abstract protected void onWorkoutIntervalUpdated(IntervalType intervalType, int intervalIndex, int value);

}
