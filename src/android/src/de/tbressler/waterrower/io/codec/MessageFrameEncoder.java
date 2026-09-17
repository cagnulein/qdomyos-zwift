package de.tbressler.waterrower.io.codec;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.log.Log;
import io.netty.buffer.ByteBuf;
import io.netty.channel.ChannelHandlerContext;
import io.netty.handler.codec.MessageToByteEncoder;

import static java.nio.charset.StandardCharsets.US_ASCII;
import static java.util.Objects.requireNonNull;

/**
 * Encodes messages (msg > byte).
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class MessageFrameEncoder extends MessageToByteEncoder {

    /* The message parser. */
    private final MessageParser parser;


    /**
     * Constructor.
     *
     * @param parser The message parser, must not be null.
     */
    public MessageFrameEncoder(MessageParser parser) {
        this.parser = requireNonNull(parser);
    }

    @Override
    protected void encode(ChannelHandlerContext ctx, Object msg, ByteBuf out) throws Exception {
        if (!(msg instanceof AbstractMessage)) {
            Exception e = new IllegalArgumentException("This type of message can not be send! " +
                    "Only messages of type >"+AbstractMessage.class.getSimpleName()+"< can be send.");
            Log.error("Message couldn't be send to serial device!", e);
            throw e;
        }

        // Parse the message:
        byte[] byteArray = parser.encode((AbstractMessage) msg);
        if (byteArray == null) {
            Log.warn("Message couldn't been encoded! Skipped message.");
            return;
        }

        // Write bytes to channel.
        out.writeBytes(byteArray);
        out.writeByte(0x0D);
        out.writeByte(0x0A);

        Log.debug("Message buffer encoded and written:\n" +
                " As String: >" + new String(byteArray, US_ASCII) + "<");

        ctx.writeAndFlush(out);
    }

}
