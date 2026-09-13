package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.TRIPLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.TOTAL_DIS_DEC;
import static de.tbressler.waterrower.subscriptions.Priority.MEDIUM;
import static de.tbressler.waterrower.utils.MessageUtils.intFromHighAndLow;

/**
 * Subscription for the total distance values of the Performance Monitor.
 *
 * The value represents the total distance meter counter - this value will be reset to zero when the Performance
 * Monitor is switched off.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class TotalDistanceSubscription extends AbstractMemorySubscription {

    /* The last distance received. */
    private double lastDistance = -1D;


    /**
     * Subscription to the displayed distance values.
     */
    public TotalDistanceSubscription() {
        this(MEDIUM);
    }

    /**
     * Subscription to the displayed distance values.
     *
     * @param priority The priority (recommended MEDIUM).
     */
    public TotalDistanceSubscription(Priority priority) {
        super(priority, TRIPLE_MEMORY, TOTAL_DIS_DEC);
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
     * Is called if the value for the total distance was updated.
     *
     * @param distance The new distance (in meter).
     */
    abstract protected void onDistanceUpdated(double distance);

}
