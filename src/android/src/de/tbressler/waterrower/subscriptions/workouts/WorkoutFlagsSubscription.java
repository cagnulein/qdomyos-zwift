package de.tbressler.waterrower.subscriptions.workouts;

import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.model.WorkoutFlags;
import de.tbressler.waterrower.subscriptions.AbstractMemorySubscription;
import de.tbressler.waterrower.subscriptions.Priority;

import static de.tbressler.waterrower.io.msg.Memory.SINGLE_MEMORY;
import static de.tbressler.waterrower.model.MemoryLocation.FEXTENDED;
import static de.tbressler.waterrower.subscriptions.Priority.HIGH;

/**
 * Subscription for working and workout control flags (FEXTENDED).
 *
 * The received message contains the following flags:
 * 0 = fzone_hr: working in heartrate zone
 * 1 = fzone_int: working in intensity zone
 * 2 = fzone_sr: working in strokerate zone
 * 3 = fprognostics: prognostics active
 * 4 = fworkout_dis: workout distance mode
 * 5 = fworkout_dur: workout duration mode
 * 6 = fworkout_dis_i: workout distance interval mode
 * 7 = fworkout_dur_i: workout duration interval mode
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class WorkoutFlagsSubscription extends AbstractMemorySubscription {

    /* The last received workout flags. */
    private WorkoutFlags lastWorkoutFlags;


    /**
     * Subscription for working and workout control flags (FEXTENDED).
     */
    public WorkoutFlagsSubscription() {
        this(HIGH);
    }

    /**
     * Subscription for working and workout control flags (FEXTENDED).
     *
     * @param priority The priority (recommended HIGH).
     */
    public WorkoutFlagsSubscription(Priority priority) {
        super(priority, SINGLE_MEMORY, FEXTENDED);
    }


    @Override
    public final void handle(DataMemoryMessage msg) {

        WorkoutFlags flags = new WorkoutFlags(msg.getValue1());

        // If the received workout flags are the same as before,
        // don't send an update.
        if (flags.equals(lastWorkoutFlags))
            return;
        lastWorkoutFlags = flags;

        onWorkoutFlagsUpdated(flags);
    }


    /**
     * Is called, when an update of the workout mode flags was received.
     *
     * @param flags The flags of the workout mode, never null.
     */
    abstract protected void onWorkoutFlagsUpdated(WorkoutFlags flags);

}
