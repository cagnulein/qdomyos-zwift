package de.tbressler.waterrower.io;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.transport.SerialChannel;
import de.tbressler.waterrower.io.transport.SerialDeviceAddress;
import de.tbressler.waterrower.log.Log;
import io.netty.bootstrap.Bootstrap;
import io.netty.channel.Channel;
import io.netty.channel.ChannelFuture;
import io.netty.channel.oio.OioEventLoopGroup;

import java.io.IOException;
import java.util.ArrayList;
import java.util.List;
import java.util.concurrent.locks.ReentrantLock;

import static java.util.Objects.requireNonNull;

/**
 * A communication service that manages the serial connection.
 * It can receive and send serial messages.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class CommunicationService {

    /* The bootstrap. */
    private final Bootstrap bootstrap;

    /* The current channel or null. */
    private Channel currentChannel;

    /* A lock for synchronized access to open/close/read/write on channel. */
    private final ReentrantLock lock = new ReentrantLock(true);

    /* Listeners for serial connections. */
    private final List<IConnectionListener> connectionListeners = new ArrayList<>();


    /* Handler for the communication channel. */
    private final SerialHandler serialHandler = new SerialHandler() {

        @Override
        protected void onConnected() {
            fireOnConnected();
        }

        @Override
        protected void onMessageReceived(AbstractMessage message) {
            fireOnMessageReceived(message);
        }

        @Override
        protected void onDisconnected() {
            fireOnDisconnected();
        }

        @Override
        protected void onError() {
            closeWithoutExceptions();
            fireOnError();
        }

    };


    /**
     * A communication service that manages the serial connection.
     * It can receive and send serial messages.
     *
     * @param bootstrap The bootstrap, not null.
     * @param channelInitializer The channel initializer, not null.
     */
    public CommunicationService(Bootstrap bootstrap, ChannelInitializer channelInitializer) {
        requireNonNull(bootstrap);
        requireNonNull(channelInitializer);

        this.bootstrap = bootstrap;
        this.bootstrap.group(new OioEventLoopGroup());
        this.bootstrap.channel(SerialChannel.class);

        channelInitializer.setSerialHandler(serialHandler);

        this.bootstrap.handler(channelInitializer);
    }


    /**
     * Opens the connection to the given serial port.
     *
     * @param address The serial port, must not be null.
     * @throws IOException if opening of the channel fails.
     */
    public void open(SerialDeviceAddress address) throws IOException {
        requireNonNull(address);

        lock.lock();

        try {

            checkIfChannelIsClose();

            Log.debug("Opening channel at serial port '" + address.value() + "'.");

            ChannelFuture future = bootstrap.connect(address).syncUninterruptibly();
            if (!future.isSuccess()) {
                fireOnError();
                throw new IOException("Serial channel couldn't be opened!");
            }

            Log.debug("Serial channel was successfully opened.");

            currentChannel = future.channel();

        } catch (Exception e) {
            throw new IOException("Can not connect to '"+address.value()+"'!", e);
        } finally {
            lock.unlock();
        }
    }

    /* Throws IOException if channel is already open. */
    private void checkIfChannelIsClose() throws IOException {
        if (currentChannel != null)
            throw new IOException("Serial channel is already open!");
    }


    /**
     * Returns true if the communication service is connected.
     *
     * @return True if connected otherwise false.
     */
    public boolean isConnected() {

        lock.lock();

        try {

            return (currentChannel != null);

        } finally {
            lock.unlock();
        }
    }


    /**
     * Sends the given message.
     *
     * @param msg The message to be send, must not be null.
     */
    public void send(AbstractMessage msg) throws IOException {
        requireNonNull(msg);

        lock.lock();

        try {

            checkIfChannelIsOpen();

            Log.debug("Sending message '" + msg.toString() + "'.");

            currentChannel.writeAndFlush(msg);

        } catch (Exception e) {
            throw new IOException("Can not send message '"+msg+"'!", e);
        } finally {
            lock.unlock();
        }
    }


    /**
     * Closes the current connection.
     *
     * @throws IOException if closing fails.
     */
    public void close() throws IOException {

        try {

            checkIfChannelIsOpen();

            Log.debug("Closing serial channel.");

            ChannelFuture future = currentChannel.close().syncUninterruptibly();
            if (!future.isSuccess())
                throw new IOException("Serial channel couldn't be closed!");

            Log.debug("Serial channel was successfully closed.");

        } catch (Exception e) {
            throw new IOException("Can not disconnect!", e);
        } finally {
            currentChannel = null;
        }
    }

    /* Throws IOException if channel is already closed. */
    private void checkIfChannelIsOpen() throws IOException {
        if ((currentChannel == null) || (!currentChannel.isOpen()))
            throw new IOException("Serial channel is not open!");
    }

    /* Close the channel and suppress exceptions. */
    private void closeWithoutExceptions() {
        try {
            Log.debug("Try to close channel.");
            close();
        } catch (IOException e) {
            Log.warn("Channel can not be closed! " + e.getMessage());
        }
    }


    /**
     * Add a connection listener.
     *
     * @param listener The listener.
     */
    public void addConnectionListener(IConnectionListener listener) {
        requireNonNull(listener);
        connectionListeners.add(listener);
    }

    /* Notify all listeners about a successful connection. */
    private void fireOnConnected() {
        connectionListeners.forEach(IConnectionListener::onConnected);
    }

    /* Notify all listeners about an error. */
    private void fireOnError() {
        connectionListeners.forEach(IConnectionListener::onError);
    }

    /* Notify all listeners about a disconnect. */
    private void fireOnDisconnected() {
        connectionListeners.forEach(IConnectionListener::onDisconnected);
    }

    /* Notify all listeners about a received message. */
    private void fireOnMessageReceived(AbstractMessage msg) {
        for (IConnectionListener listener : connectionListeners)
            listener.onMessageReceived(msg);
    }

    /**
     * Remove a connection listener.
     *
     * @param listener The listener.
     */
    public void removeConnectionListener(IConnectionListener listener) {
        requireNonNull(listener);
        connectionListeners.remove(listener);
    }

}
