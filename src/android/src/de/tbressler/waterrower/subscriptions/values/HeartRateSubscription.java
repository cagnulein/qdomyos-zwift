package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.SINGLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.ZONE_HR_VAL;
import static de.tbressler.waterrower.subscriptions.Priority.MEDIUM;

/**
 * Subscription for the heart rate value (in beats per minute).
 *
 * TODO Because of the absence of the optional heart rate (pulse) device this value couldn't be tested so far.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class HeartRateSubscription extends AbstractMemorySubscription {

    /* The last heart rate received. */
    private int lastHeartRate = -1;


    /**
     * Subscription for the heart rate value (in beats per minute).
     */
    public HeartRateSubscription() {
        this(MEDIUM);
    }

    /**
     * Subscription for the heart rate value (in beats per minute).
     *
     * @param priority The priority (recommended MEDIUM).
     */
    public HeartRateSubscription(Priority priority) {
        super(priority, SINGLE_MEMORY, ZONE_HR_VAL);
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {

        int heartRate = msg.getValue1();

        // If the received heart rate is the same as before,
        // don't send an update.
        if (lastHeartRate == heartRate)
            return;
        lastHeartRate = heartRate;

        onHeartRateUpdated(heartRate);
    }


    /**
     * Is called if the value for the heart rate was updated.
     *
     * @param heartRate The new heart rate (in bpm).
     */
    abstract protected void onHeartRateUpdated(int heartRate);

}
