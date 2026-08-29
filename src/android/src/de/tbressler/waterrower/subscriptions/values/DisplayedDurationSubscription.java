package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import java.time.Duration;

import static de.tbressler.waterrower.io.msg.Memory.TRIPLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.DISPLAY_SEC;
import static de.tbressler.waterrower.subscriptions.Priority.HIGH;
import static java.lang.Integer.parseInt;
import static java.time.Duration.ofSeconds;

/**
 * Subscription for the displayed duration on the duration window of the Performance Monitor.
 *
 * The duration window displays the time covered (or time to be covered in a duration workout)
 * in units of hours, minutes, seconds and decimal seconds.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class DisplayedDurationSubscription extends AbstractMemorySubscription {

    /* The last duration received. */
    private Duration lastDuration = null;


    /**
     * Subscription to the displayed duration value.
     */
    public DisplayedDurationSubscription() {
        this(HIGH);
    }

    /**
     * Subscription to the displayed duration value.
     *
     * @param priority The priority (recommended HIGH).
     */
    public DisplayedDurationSubscription(Priority priority) {
        super(priority, TRIPLE_MEMORY, DISPLAY_SEC);
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {

        int sec = parseInt(msg.getValue1AsACH());
        int min = parseInt(msg.getValue2AsACH());
        int hrs = parseInt(msg.getValue3AsACH());

        Duration duration = ofSeconds(sec)
                .plusMinutes(min)
                .plusHours(hrs);

        // If the received duration is the same as before,
        // don't send an update.
        if (duration.equals(lastDuration))
            return;
        lastDuration = duration;

        // Notify update.
        onDurationUpdated(duration);
    }


    /**
     * Is called if the value for the displayed duration was updated.
     *
     * @param duration The new duration, never null.
     */
    abstract protected void onDurationUpdated(Duration duration);

}
