package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.in.PulseCountMessage;
import de.tbressler.waterrower.log.Log;

import static de.tbressler.waterrower.utils.MessageUtils.achToInt;

/**
 * Interpreter for:
 *
 * Pulse Count in the last 25mS (S4/S5 -> PC).
 *
 * This packet is auto transmitted by the rowing computer.
 *
 * "XX" is an ACH value representing the number of pulse’s counted during the last 25mS
 * period; this value can range from 1 to 50 typically. (Zero values will not be transmitted).
 * Please refer to "WaterRower Series 4 Rowing Algorithm.doc" for in depth details on how to use
 * this data. At this time the constant values are:
 *
 * pins_per_xxcm 32 ; number of pin edges allowed to equal xxcm (dec)
 * distance_xxcm 35 ; number of cm per flagged xxcm no. of pins (dec)
 *
 * This packet has the third highest priority of transmission on the USB.
 *
 * [P] + XX + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class PulseCountMessageInterpreter extends AbstractMessageInterpreter<PulseCountMessage> {

    @Override
    public String getMessageIdentifier() {
        return "P";
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof PulseCountMessage);
    }

    @Override
    public PulseCountMessage decode(String msg) {

        // Workaround: Discard ping messages, because
        // they also start with a 'P'.
        if (msg.startsWith("PING"))
            return null;

        if (msg.length() < 3)
            return null;

        try {

            String pulsesCount = msg.substring(1, 3);
            return new PulseCountMessage(achToInt(pulsesCount));

        } catch (NumberFormatException e) {
            Log.error("Couldn't parse ACH value from message!", e);
            return null;
        }
    }

    @Override
    public String encode(PulseCountMessage msg) {
        throw new IllegalStateException("This type of message can not be send to the WaterRower S4/S5 monitor.");
    }

}
