package de.tbressler.waterrower.subscriptions;

/**
 * Interface for the subscription polling service.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public interface ISubscriptionPollingService {

    /**
     * Start the subscription polling service.
     */
    void start();


    /**
     * Subscribe to data/events. This will start the polling for the given data.
     *
     * @param subscription The subscription and callback, must not be null.
     */
    void subscribe(ISubscription subscription);


    /**
     * Unsubscribe from data/events. This will stop the polling for the given data.
     *
     * @param subscription The subscription, must not be null.
     */
    void unsubscribe(ISubscription subscription);


    /**
     * Stop the subscription polling service.
     */
    void stop();

}
