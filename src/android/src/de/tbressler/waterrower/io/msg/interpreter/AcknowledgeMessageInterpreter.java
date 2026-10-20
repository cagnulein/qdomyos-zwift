package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.in.AcknowledgeMessage;

/**
 * Interpreter for:
 *
 * Packet Accepted (S4/S5 -> PC).
 *
 * This packet will only be sent where no other reply to a PC would otherwise be given. If a
 * packet response is required to the PC then that will take the place of the OK packet.
 *
 * [O][K] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class AcknowledgeMessageInterpreter extends AbstractMessageInterpreter<AcknowledgeMessage> {

    /* Single instance of an acknowledge message. */
    private final static AcknowledgeMessage ACKNOWLEDGE_MESSAGE = new AcknowledgeMessage();


    @Override
    public String getMessageIdentifier() {
        return "OK";
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof AcknowledgeMessage);
    }

    @Override
    public AcknowledgeMessage decode(String msg) {
        return ACKNOWLEDGE_MESSAGE;
    }

    @Override
    public String encode(AcknowledgeMessage msg) {
        throw new IllegalStateException("This type of message can not be send to the WaterRower S4/S5 monitor.");
    }

}
