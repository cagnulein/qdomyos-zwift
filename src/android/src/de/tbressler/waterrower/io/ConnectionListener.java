package de.tbressler.waterrower.io;

import de.tbressler.waterrower.io.msg.AbstractMessage;

/**
 * Simple implementation of the interface IConnectionListener.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ConnectionListener implements IConnectionListener {

    @Override
    public void onConnected() {}

    @Override
    public void onMessageReceived(AbstractMessage msg) {}

    @Override
    public void onDisconnected() {}

    @Override
    public void onError() {}

}
