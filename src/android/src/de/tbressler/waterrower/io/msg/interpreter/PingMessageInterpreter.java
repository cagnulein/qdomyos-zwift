package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.in.PingMessage;

/**
 * Interpreter for:
 *
 * Ping (S4/S5 -> PC).
 *
 * Sent once a second while NO rowing is occurring to indicate to the PC the rowing monitor is
 * still operational but stopped.
 *
 * [P][ING] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class PingMessageInterpreter extends AbstractMessageInterpreter<PingMessage> {

    /* Single instance of an acknowledgment message. */
    private final static PingMessage PING_MESSAGE = new PingMessage();


    @Override
    public String getMessageIdentifier() {
        return "PING";
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof PingMessage);
    }

    @Override
    public PingMessage decode(String msg) {
        return PING_MESSAGE;
    }

    @Override
    public String encode(PingMessage msg) {
        throw new IllegalStateException("This type of message can not be send to the WaterRower S4/S5 monitor.");
    }

}
