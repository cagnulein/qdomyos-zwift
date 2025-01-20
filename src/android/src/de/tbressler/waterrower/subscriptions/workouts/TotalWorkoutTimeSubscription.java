package de.tbressler.waterrower.subscriptions.workouts;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import java.time.Duration;

import static de.tbressler.waterrower.io.msg.Memory.DOUBLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.WORKOUT_TIMEL;
import static de.tbressler.waterrower.subscriptions.Priority.LOW;
import static de.tbressler.waterrower.utils.MessageUtils.intFromHighAndLow;

/**
 * Subscription for values of the total workout times.
 * The time is updated by the WaterRower after each workout interval.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class TotalWorkoutTimeSubscription extends AbstractMemorySubscription {

    /* The last value received. */
    private int lastValue = -1;


    /**
     * Subscription for values of the total workout times.
     * The time is updated by the WaterRower after each workout interval.
     */
    public TotalWorkoutTimeSubscription() {
        this(LOW);
    }

    /**
     * Subscription for values of the total workout times.
     * The time is updated by the WaterRower after each workout interval.
     *
     * @param priority The priority (recommended LOW).
     */
    public TotalWorkoutTimeSubscription(Priority priority) {
        super(priority, DOUBLE_MEMORY, WORKOUT_TIMEL);
    }


    @Override
    protected void handle(DataMemoryMessage msg) {

        int value = intFromHighAndLow(msg.getValue2(), msg.getValue1());

        // If the received value is the same as before,
        // don't send an update.
        if (lastValue == value)
            return;
        lastValue = value;

        onTimeUpdated(Duration.ofSeconds(value));
    }


    /**
     * Is called if the total workout time value was updated.
     *
     * @param time The new workout time.
     */
    abstract protected void onTimeUpdated(Duration time);

}
