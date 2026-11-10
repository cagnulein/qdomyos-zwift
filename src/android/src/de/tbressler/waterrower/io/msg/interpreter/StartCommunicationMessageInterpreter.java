package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.out.StartCommunicationMessage;

/**
 * Interpreter for:
 *
 * Application starting communication's (PC -> S4/S5).
 *
 * This is the very first packet sent by an application once the COM port is opened, this will
 * tell the rowing computer to reply with its hardware type packet.
 *
 * [U][SB] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class StartCommunicationMessageInterpreter extends AbstractMessageInterpreter<StartCommunicationMessage> {

    @Override
    public String getMessageIdentifier() {
        return null;
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof StartCommunicationMessage);
    }

    @Override
    public StartCommunicationMessage decode(String msg) {
        throw new IllegalStateException("This type of message should not be send by WaterRower S4/S5 monitor to the PC.");
    }

    @Override
    public String encode(StartCommunicationMessage msg) {
        return "USB";
    }

}
