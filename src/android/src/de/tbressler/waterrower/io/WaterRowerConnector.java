package de.tbressler.waterrower.io;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.transport.SerialDeviceAddress;
import de.tbressler.waterrower.log.Log;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import static java.util.Objects.requireNonNull;

/**
 * Handles the connection to the WaterRower.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class WaterRowerConnector {

    /* ms to wait after a message was send, in order to give the WaterRower time to
       process the message. */
    private final static int MINIMUM_SEND_INTERVAL = 30;


    /* The serial communication service. */
    private final CommunicationService communicationService;

    /* The lock to synchronize connect and disconnect. */
    private final ReentrantLock lock = new ReentrantLock(true);

    /* Listeners for the serial communication. */
    private final List<IConnectionListener> listeners = new ArrayList<>();


    /**
     * Handles the connection to the WaterRower.
     *
     * @param communicationService The communication service, must not be null.
     */
    public WaterRowerConnector(CommunicationService communicationService) {
        this.communicationService = requireNonNull(communicationService);
    }


    /**
     * Connect to the rowing computer.
     *
     * @param address The serial port, must not be null.
     *
     * @throws IOException If connect fails.
     */
    public void connect(SerialDeviceAddress address) throws IOException {
        requireNonNull(address);

        lock.lock();

        try {

            if (isConnected())
                throw new IOException("Service is already connected! Can not connect.");

            Log.debug("Opening serial channel at '" + address.value() + "' connection.");
            communicationService.open(address);

        } finally {
            lock.unlock();
        }
    }


    /* Returns true if connected. */
    public boolean isConnected() {
        return communicationService.isConnected();
    }


    /**
     * Disconnects from the rowing computer.
     *
     * @throws IOException If disconnect fails.
     */
    public void disconnect() throws IOException {

        lock.lock();

        try {

            if (!isConnected())
                throw new IOException("Service is not connected! Can not disconnect.");

            Log.debug("Closing serial channel.");
            communicationService.close();

        } finally {
            lock.unlock();
        }
    }


    /**
     * Sends a single message.
     *
     * @param msg The message to be sent, must not be null.
     */
    public void send(AbstractMessage msg) throws IOException {
        requireNonNull(msg);

        lock.lock();

        if (!isConnected())
            throw new IOException("Not connected! Can not send message to WaterRower.");

        try {

            communicationService.send(msg);
            Thread.sleep(MINIMUM_SEND_INTERVAL); // Wait, this gives the rowing computer time to process.

        } catch (InterruptedException e) {
            Log.error("Error while sending message!", e);
        } finally {
            lock.unlock();
        }
    }


    /**
     * Sends multiple messages at once.
     *
     * @param messages The messages to be sent, must not be null.
     */
    public void send(List<AbstractMessage> messages) throws IOException {
        requireNonNull(messages);

        lock.lock();

        if (!isConnected())
            throw new IOException("Not connected! Can not send message to WaterRower.");

        try {

            for(AbstractMessage msg : messages) {
                send(msg);
            }

        } finally {
            lock.unlock();
        }
    }


    /**
     * Adds the connection listener.
     *
     * @param listener The listener, must not be null.
     */
    public void addConnectionListener(IConnectionListener listener) {
        listeners.add(requireNonNull(listener));
        communicationService.addConnectionListener(listener);
    }

    /**
     * Removes the connection listener.
     *
     * @param listener The listener, must not be null.
     */
    public void removeConnectionListener(IConnectionListener listener) {
        listeners.remove(requireNonNull(listener));
        communicationService.removeConnectionListener(listener);
    }

}
