package de.tbressler.waterrower.subscriptions.workouts;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.DOUBLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.WORKOUT_STROKEL;
import static de.tbressler.waterrower.subscriptions.Priority.LOW;
import static de.tbressler.waterrower.utils.MessageUtils.intFromHighAndLow;

/**
 * Subscription for values of the total workout strokes.
 * The stroke value is updated by the WaterRower after each workout interval.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class TotalWorkoutStrokesSubscription extends AbstractMemorySubscription {

    /* The last value received. */
    private int lastValue = -1;


    /**
     * Subscription for values of the total workout strokes.
     * The stroke value is updated by the WaterRower after each workout interval.
     */
    public TotalWorkoutStrokesSubscription() {
        this(LOW);
    }

    /**
     * Subscription for values of the total workout strokes.
     * The stroke value is updated by the WaterRower after each workout interval.
     *
     * @param priority The priority (recommended LOW).
     */
    public TotalWorkoutStrokesSubscription(Priority priority) {
        super(priority, DOUBLE_MEMORY, WORKOUT_STROKEL);
    }


    @Override
    protected void handle(DataMemoryMessage msg) {

        int value = intFromHighAndLow(msg.getValue2(), msg.getValue1());

        // If the received value is the same as before,
        // don't send an update.
        if (lastValue == value)
            return;
        lastValue = value;

        onStrokesUpdated(value);
    }


    /**
     * Is called if the value of the total workout strokes was updated.
     *
     * @param strokes The new value for strokes.
     */
    abstract protected void onStrokesUpdated(int strokes);

}
