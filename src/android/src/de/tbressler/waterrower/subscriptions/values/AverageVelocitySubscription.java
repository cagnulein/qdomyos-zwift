package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.DOUBLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.M_S_LOW_AVERAGE;
import static de.tbressler.waterrower.subscriptions.Priority.HIGH;
import static de.tbressler.waterrower.utils.MessageUtils.intFromHighAndLow;

/**
 * Subscription for the displayed average velocity (in meters per second) on the intensity window
 * of the Performance Monitor.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class AverageVelocitySubscription extends AbstractMemorySubscription {

    /* The last velocity received (in cm/s). */
    private int lastVelocity = -1;


    /**
     * Subscription for the displayed average velocity on the intensity window
     * of the Performance Monitor.
     */
    public AverageVelocitySubscription() {
        this(HIGH);
    }

    /**
     * Subscription for the displayed average velocity on the intensity window
     * of the Performance Monitor.
     *
     * @param priority The priority (recommended HIGH).
     */
    public AverageVelocitySubscription(Priority priority) {
        super(priority, DOUBLE_MEMORY, M_S_LOW_AVERAGE);
    }


    @Override
    protected void handle(DataMemoryMessage msg) {

        int velocity = intFromHighAndLow(msg.getValue2(), msg.getValue1());

        // If the received velocity is the same as before,
        // don't send an update.
        if (lastVelocity == velocity)
            return;
        lastVelocity = velocity;

        double value = ((double) velocity) / 100D;

        onVelocityUpdated(value);
    }


    /**
     * Is called if the value for the average velocity was updated.
     *
     * @param velocity The new value (in meters per second).
     */
    abstract protected void onVelocityUpdated(double velocity);

}
