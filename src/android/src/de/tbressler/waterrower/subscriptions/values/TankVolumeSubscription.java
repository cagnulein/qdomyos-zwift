package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.SINGLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.TANK_VOLUME;
import static de.tbressler.waterrower.subscriptions.Priority.LOW;

/**
 * Subscription for the tank volume value (in liters).
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class TankVolumeSubscription extends AbstractMemorySubscription {

    /* The last tank volume received. */
    int lastTankVolume = -1;


    /**
     * Subscription for the tank volume value.
     */
    public TankVolumeSubscription() {
        this(LOW);
    }

    /**
     * Subscription for the tank volume value.
     *
     * @param priority The priority (recommended LOW).
     */
    public TankVolumeSubscription(Priority priority) {
        super(priority, SINGLE_MEMORY, TANK_VOLUME);
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {
        int tankVolume = msg.getValue1();

        if (lastTankVolume == tankVolume)
            return;
        lastTankVolume = tankVolume;

        double value = ((double) tankVolume) / 10D;

        onTankVolumeUpdated(value);
    }


    /**
     * Is called if the value for the tank volume was updated.
     *
     * @param tankVolume The volume of water in the tank (in liters).
     */
    abstract protected void onTankVolumeUpdated(double tankVolume);

}
