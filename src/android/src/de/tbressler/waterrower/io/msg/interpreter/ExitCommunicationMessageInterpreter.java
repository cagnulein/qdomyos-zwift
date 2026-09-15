package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.out.ExitCommunicationMessage;

/**
 * Interpreter for:
 *
 * Application is exiting (PC -> S4/S5).
 *
 * Any application wishing to normally terminate (close) is required to send this packet to stop
 * the automatic packets being sent to the PC.
 *
 * [E][XIT] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ExitCommunicationMessageInterpreter extends AbstractMessageInterpreter<ExitCommunicationMessage> {

    @Override
    public String getMessageIdentifier() {
        return null;
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof ExitCommunicationMessage);
    }

    @Override
    public ExitCommunicationMessage decode(String msg) {
        throw new IllegalStateException("This type of message should not be send by WaterRower S4/S5 monitor to the PC.");
    }

    @Override
    public String encode(ExitCommunicationMessage msg) {
        return "EXIT";
    }

}
