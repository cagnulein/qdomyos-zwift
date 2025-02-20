package de.tbressler.waterrower.subscriptions.values;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;
import de.tbressler.waterrower.utils.MessageUtils;

import java.time.Duration;

import static de.tbressler.waterrower.io.msg.Memory.TRIPLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.CLOCK_DOWN_DEC;
import static de.tbressler.waterrower.subscriptions.Priority.HIGH;
import static java.time.Duration.ofSeconds;

/**
 * Subscription for clock count down values.
 *
 * This value is only set if a count down is running. The count down is also transmitted with the
 * DisplayedDurationSubscription when the count down is active.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class ClockCountDownSubscription extends AbstractMemorySubscription {

    /* The last clock count down received. */
    private Duration lastClockCountDown = null;


    /**
     * Subscription to the distance value.
     */
    public ClockCountDownSubscription() {
        this(HIGH);
    }

    /**
     * Subscription to the distance value.
     *
     * @param priority The priority (recommended HIGH).
     */
    public ClockCountDownSubscription(Priority priority) {
        super(priority, TRIPLE_MEMORY, CLOCK_DOWN_DEC);
    }


    @Override
    protected final void handle(DataMemoryMessage msg) {

        int millis = msg.getValue1();
        int sec = MessageUtils.intFromHighAndLow(msg.getValue3(), msg.getValue2());

        Duration duration = ofSeconds(sec).plusMillis(millis * 100);

        // If the received duration is the same as before,
        // don't send an update.
        if (duration.equals(lastClockCountDown))
            return;
        lastClockCountDown = duration;

        onClockCountDownUpdated(duration);
    }


    /**
     * Is called if the value for the clock count-down was updated.
     *
     * @param duration The new clock count-down, never null.
     */
    abstract protected void onClockCountDownUpdated(Duration duration);

}
