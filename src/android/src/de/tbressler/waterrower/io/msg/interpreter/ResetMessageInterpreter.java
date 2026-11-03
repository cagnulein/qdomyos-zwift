package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.out.ResetMessage;

/**
 * Interpreter for:
 *
 * Request the rowing computer to reset (PC -> S4/S5).
 *
 * Request the rowing computer to perform a reset; this will be identical to the user performing
 * this with the power button. Used prior to configuring the rowing computer from a PC.
 * Interactive mode will be disabled on a reset.
 *
 * [R][ESET] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ResetMessageInterpreter extends AbstractMessageInterpreter<ResetMessage> {

    @Override
    public String getMessageIdentifier() {
        return null;
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof ResetMessage);
    }

    @Override
    public ResetMessage decode(String msg) {
        throw new IllegalStateException("This type of message should not be send by WaterRower S4/S5 monitor to the PC.");
    }

    @Override
    public String encode(ResetMessage msg) {
        return "RESET";
    }

}
