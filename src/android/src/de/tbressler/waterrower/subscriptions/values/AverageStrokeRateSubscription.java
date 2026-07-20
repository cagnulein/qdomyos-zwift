package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.SINGLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.STROKE_AVERAGE;
import static de.tbressler.waterrower.subscriptions.Priority.HIGH;

/**
 * Subscription for the average stroke rate (strokes/min) of a whole stroke which is displayed in
 * the stroke rate window of the Performance Monitor.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class AverageStrokeRateSubscription extends AbstractMemorySubscription {

    /* The last stroke rate received. */
    private int lastStrokeRate = -1;


    /**
     * Subscription for the average stroke rate (strokes/min) of a whole stroke which is displayed in
     * the stroke rate window of the Performance Monitor.
     */
    public AverageStrokeRateSubscription() {
        this(HIGH);
    }

    /**
     * Subscription for the average stroke rate (strokes/min) of a whole stroke which is displayed in
     * the stroke rate window of the Performance Monitor.
     *
     * @param priority The priority (recommended HIGH).
     */
    public AverageStrokeRateSubscription(Priority priority) {
        super(priority, SINGLE_MEMORY, STROKE_AVERAGE);
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {

        int strokeRate = msg.getValue1();

        // If the received duration is the same as before,
        // don't send an update.
        if (lastStrokeRate == strokeRate)
            return;
        lastStrokeRate = strokeRate;

        onStrokeRateUpdated(calculateAverageStrokeRate(strokeRate));
    }

    /* Calculate the average stroke rate. */
    private double calculateAverageStrokeRate(int strokeRate) {
        if (strokeRate == 0)
            return 0D;
        return (60000D / (((double) strokeRate) * 25D));
    }


    /**
     * Is called if the value for the average stroke rate was updated.
     *
     * @param strokeRate The new stroke rate (in strokes per minute).
     */
    abstract protected void onStrokeRateUpdated(double strokeRate);

}
