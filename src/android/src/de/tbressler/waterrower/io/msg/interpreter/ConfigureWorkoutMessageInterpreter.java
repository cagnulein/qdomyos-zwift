package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.out.ConfigureWorkoutMessage;
import de.tbressler.waterrower.log.Log;

import static de.tbressler.waterrower.utils.MessageUtils.intToAch;

/**
 * Interpreter for: ConfigureWorkoutMessage
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ConfigureWorkoutMessageInterpreter extends AbstractMessageInterpreter<ConfigureWorkoutMessage> {

    @Override
    public String getMessageIdentifier() {
        return null;
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof ConfigureWorkoutMessage);
    }

    @Override
    public ConfigureWorkoutMessage decode(String msg) {
        throw new IllegalStateException("This type of message should not be send by WaterRower S4/S5 monitor to the PC.");
    }

    @Override
    public String encode(ConfigureWorkoutMessage msg) {

        switch(msg.getMessageType()) {
            case SINGLE_WORKOUT:
                return addUnitAndDistance(msg, "WS");
            case START_INTERVAL_WORKOUT:
                return addUnitAndDistance(msg, "WI");
            case ADD_INTERVAL_WORKOUT:
            case END_INTERVAL_WORKOUT:
                return "WIN" + intToAch(msg.getRestInterval(), 4) + intToAch(msg.getDistance(), 4);
        }

        Log.warn("Message couldn't be encoded!\n" +
                " Message was: " + msg);

        return null;
    }

    /* Add unit and distance to the given string. */
    private String addUnitAndDistance(ConfigureWorkoutMessage msg, String output) {

        switch (msg.getWorkoutUnit()) {
            case METERS:
                output += "I1";
                break;
            case MILES:
                output += "I2";
                break;
            case KMS:
                output += "I3";
                break;
            case STROKES:
                output += "I4";
                break;
            case SECONDS:
                output += "U";
                break;
        }

        output += intToAch(msg.getDistance(), 4);

        return output;
    }

}
