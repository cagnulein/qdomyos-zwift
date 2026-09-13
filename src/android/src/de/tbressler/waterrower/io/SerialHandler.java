package de.tbressler.waterrower.io;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.log.Log;
import io.netty.channel.ChannelHandler.Sharable;
import io.netty.channel.ChannelHandlerContext;
import io.netty.channel.ChannelInboundHandlerAdapter;
import io.netty.util.ReferenceCountUtil;

/**
 * Handler for different events on the serial connection (e.g. connect, disconnect).
 *
 * @author Tobias Bressler
 * @version 1.0
 */
@Sharable
public abstract class SerialHandler extends ChannelInboundHandlerAdapter {

    @Override
    public void channelRead(ChannelHandlerContext ctx, Object msg) {
        try {

            if (!(msg instanceof AbstractMessage)) {
                Log.warn("Invalid message received! Message skipped.");
                return;
            }

            Log.debug("Message received: " + msg);

            // Notify that a message was received.
            onMessageReceived((AbstractMessage) msg);

        } finally {
            ReferenceCountUtil.release(msg);
        }
    }

    /**
     * Is called if a message was received.
     *
     * @param message The message.
     */
    abstract protected void onMessageReceived(AbstractMessage message);


    @Override
    public void channelActive(ChannelHandlerContext ctx) throws Exception {
        super.channelActive(ctx);
        onConnected();
    }

    /**
     * Is called if connection was established.
     */
    abstract protected void onConnected();


    @Override
    public void channelInactive(ChannelHandlerContext ctx) throws Exception {
        super.channelInactive(ctx);
        onDisconnected();
    }

    /**
     * Is called if connection was closed.
     */
    protected abstract void onDisconnected();


    @Override
    public void exceptionCaught(ChannelHandlerContext ctx, Throwable cause) {
        Log.error("Unexpected exception caught in serial handler!", cause);
        ctx.close();
        Log.debug("Connection to serial port closed.");
        onError();
    }

    /**
     * Is called if an connection error occurred.
     */
    protected abstract void onError();

}
