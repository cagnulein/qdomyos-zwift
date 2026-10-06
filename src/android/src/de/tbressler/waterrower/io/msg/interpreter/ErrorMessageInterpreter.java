package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.in.ErrorMessage;

/**
 * Interpreter for:
 *
 * Unknown packet / error (S4/S5 -> PC).
 *
 * The last received packet from the PC was of an unknown time and caused a general ERROR reply
 * to be issued.
 *
 * [E][RROR] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ErrorMessageInterpreter extends AbstractMessageInterpreter<ErrorMessage> {

    /* Single instance of an error message. */
    private final static ErrorMessage ERROR_MESSAGE = new ErrorMessage();


    @Override
    public String getMessageIdentifier() {
        return "ERROR";
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof ErrorMessage);
    }

    @Override
    public ErrorMessage decode(String msg) {
        return ERROR_MESSAGE;
    }

    @Override
    public String encode(ErrorMessage msg) {
        throw new IllegalStateException("This type of message can not be send to the WaterRower.");
    }

}
