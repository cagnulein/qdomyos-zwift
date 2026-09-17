package de.tbressler.waterrower.subscriptions;

import de.tbressler.waterrower.io.msg.AbstractMessage;

/**
 * A generic interface for subscriptions.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public interface ISubscription {

    /**
     * Returns the priority of the subscription. The priority determines how
     * often a subscription will be polled.
     *
     * @return The priority, never null.
     */
    Priority getPriority();

    /**
     * Returns the message that must be send to the WaterRower S4/S5 monitor to send the current value
     * for the subscription.
     *
     * @return The poll message or null, if not message must be send.
     */
    AbstractMessage poll();

    /**
     * Handles the received message from the WaterRower S4/S5 monitor.
     *
     * @param msg The message from the WaterRower S4/S5 monitor.
     */
    void handle(AbstractMessage msg);

}