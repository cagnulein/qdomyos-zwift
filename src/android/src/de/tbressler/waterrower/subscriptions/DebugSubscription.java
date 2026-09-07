package de.tbressler.waterrower.subscriptions;

import de.tbressler.waterrower.io.msg.Memory;
import de.tbressler.waterrower.model.MemoryLocation;

/**
 * For test purposes only!
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public abstract class DebugSubscription extends AbstractMemorySubscription {

    /**
     * For test purposes only!
     */
    public DebugSubscription(Priority priority, Memory memory, MemoryLocation location) {
        super(priority, memory, location);
    }

}
