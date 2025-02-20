package de.tbressler.waterrower.subscriptions;

import de.tbressler.waterrower.io.ConnectionListener;
import de.tbressler.waterrower.io.IConnectionListener;
import de.tbressler.waterrower.io.WaterRowerConnector;
import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.log.Log;

import java.io.IOException;
import java.time.Duration;
import java.util.List;
import java.util.concurrent.ArrayBlockingQueue;
import java.util.concurrent.BlockingQueue;
import java.util.concurrent.CopyOnWriteArrayList;
import java.util.concurrent.ScheduledExecutorService;
import java.util.concurrent.atomic.AtomicBoolean;
import java.util.concurrent.atomic.AtomicLong;

import static de.tbressler.waterrower.subscriptions.Priority.*;
import static java.util.Objects.requireNonNull;
import static java.util.concurrent.TimeUnit.MILLISECONDS;

/**
 * The implementation of the subscription polling service.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class SubscriptionPollingService implements ISubscriptionPollingService {

    /* The maximum size of messages in the queue. */
    private static final int MESSAGE_QUEUE_SIZE = 20;


    /* The interval between two poll messages (in ms). */
    private final long interval;

    /* List of subscriptions. */
    private final List<ISubscription> subscriptions = new CopyOnWriteArrayList<>();

    /* The connector to the WaterRower. */
    private final WaterRowerConnector connector;


    /* The executor service for polling of subscriptions. */
    private final ScheduledExecutorService executorService;

    /* True if subscription polling is active. */
    private final AtomicBoolean isActive = new AtomicBoolean(false);


    /* Counter for the polling cycles. */
    private final AtomicLong pollCycle = new AtomicLong(0);

    /* Message queue for the current polling cycle. */
    private final BlockingQueue<AbstractMessage> messageQueue = new ArrayBlockingQueue<>(MESSAGE_QUEUE_SIZE, true);


    /* Listener for the connection to the WaterRower, which handles the received messages*/
    private final IConnectionListener listener = new ConnectionListener() {
        @Override
        public void onMessageReceived(AbstractMessage msg) {

            // If not active skip execution.
            if (!isActive.get())
                return;

            for(ISubscription subscription : subscriptions) {
                subscription.handle(msg);
            }
        }
    };


    /**
     * The subscription polling manager.
     *
     * @param connector The connector to the WaterRower, must not be null.
     * @param executorService The executor service for the subscription polling, must not be null.
     * @param interval The interval between messages, must not be null.
     *                 Recommended = 200 ms.
     */
    public SubscriptionPollingService(WaterRowerConnector connector, ScheduledExecutorService executorService, Duration interval) {
        this.interval = requireNonNull(interval).toMillis();
        this.connector = requireNonNull(connector);
        this.connector.addConnectionListener(listener);
        this.executorService = requireNonNull(executorService);
    }


    /**
     * Start the subscription polling service.
     */
    @Override
    public void start() {

        Log.debug("Start subscription polling service.");

        isActive.set(true);

        collectMessagesForNextPollingInterval();

        scheduleSendMessageTask();
    }


    /* Collect messages from the current subscriptions for polling. */
    private void collectMessagesForNextPollingInterval() {

        // If not active skip execution.
        if (!isActive.get())
            return;

        Log.debug("Schedule polling for "+subscriptions.size()+" subscription(s)...");

        long cycle = pollCycle.getAndIncrement();
        boolean pollMedium = cycle % 2 == 0;
        boolean pollLow = cycle % 5 == 0;

        for (ISubscription subscription : subscriptions) {

            if ((subscription.getPriority() == NO_POLLING)
               || (subscription.getPriority() == MEDIUM && !pollMedium)
               || (subscription.getPriority() == LOW && !pollLow))
                continue;

            AbstractMessage msg = subscription.poll();

            boolean addedToQueue = messageQueue.offer(msg);

            // If the message couldn't be added to queue, show a warning. This is the case
            // when too many subscriptions are subscribed. The size of the queue = MESSAGE_QUEUE_SIZE.
            if (!addedToQueue) {
                Log.warn("Can not add more messages, the message queue is full! Skipping remaining messages in current cycle.");
                return;
            }
        }
    }


    /* Schedule the send task for execution. */
    private void scheduleSendMessageTask() {
        executorService.schedule(this::sendNextMessage, interval, MILLISECONDS);
    }

    /* Send the first message from the message queue to the WaterRower. */
    private void sendNextMessage() {

        try {

            // If not active skip execution.
            if (!isActive.get())
                return;

            AbstractMessage msg = messageQueue.poll();

            if (msg != null) {
                Log.debug("Send scheduled polling message >" + msg.toString() + "<");
                connector.send(msg);
            }

        } catch (IOException e) {
            Log.error("Couldn't send polling message due to an error!", e);
        }

        if (messageQueue.isEmpty())
            collectMessagesForNextPollingInterval();

        scheduleSendMessageTask();
    }


    /**
     * Stop the subscription polling service.
     */
    @Override
    public void stop() {

        Log.debug("Stop subscription polling service.");

        isActive.set(false);
    }


    /**
     * Subscribe to data/events. This will start the polling for the given data.
     *
     * @param subscription The subscription and callback, must not be null.
     */
    @Override
    public void subscribe(ISubscription subscription) {
        subscriptions.add(requireNonNull(subscription));
        Log.debug("Added subscription: " + subscription);
    }

    /**
     * Unsubscribe from data/events. This will stop the polling for the given data.
     *
     * @param subscription The subscription, must not be null.
     */
    @Override
    public void unsubscribe(ISubscription subscription) {
        subscriptions.remove(requireNonNull(subscription));
        Log.debug("Removed subscription: " + subscription);
    }

}
