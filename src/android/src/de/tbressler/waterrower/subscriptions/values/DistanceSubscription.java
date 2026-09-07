package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.TRIPLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.MS_DISTANCE_DEC;
import static de.tbressler.waterrower.subscriptions.Priority.HIGH;
import static de.tbressler.waterrower.utils.MessageUtils.intFromHighAndLow;

/**
 * Subscription for the current distance.
 *
 * The value will be set to 0 by the Performance Monitor when a new row interval begins or
 * the user performs a RESET.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class DistanceSubscription extends AbstractMemorySubscription {

    /* The last distance received. */
    private double lastDistance = -1D;


    /**
     * Subscription to the distance values.
     */
    public DistanceSubscription() {
        this(HIGH);
    }

    /**
     * Subscription to the distance values.
     *
     * @param priority The priority (recommended HIGH).
     */
    public DistanceSubscription(Priority priority) {
        super(priority, TRIPLE_MEMORY, MS_DISTANCE_DEC);
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {

        double distance = intFromHighAndLow(msg.getValue3(), msg.getValue2()) + (((double) msg.getValue1()) / 100D);

        // If the received distance is the same as before,
        // don't send an update.
        if (lastDistance == distance)
            return;
        lastDistance = distance;

        // Notify update.
        onDistanceUpdated(distance);
    }

    /**
     * Is called if the value for the current distance was updated.
     *
     * @param distance The new distance (in meter).
     */
    abstract protected void onDistanceUpdated(double distance);

}
