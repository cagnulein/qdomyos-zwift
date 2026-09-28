package de.tbressler.waterrower.io.codec;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.IMessageInterpreter;
import de.tbressler.waterrower.io.msg.in.DecodeErrorMessage;
import de.tbressler.waterrower.io.msg.interpreter.*;
import de.tbressler.waterrower.log.Log;

import java.util.ArrayList;
import java.util.List;

import static java.nio.charset.StandardCharsets.US_ASCII;
import static java.util.Objects.requireNonNull;

/**
 * Decodes and encodes messages received from or sent to the WaterRower S4/S5 monitor.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class MessageParser {

    /* List of message interpreters. */
    private List<IMessageInterpreter> interpreters = new ArrayList<>();


    /**
     * Decodes and encodes messages received from or sent to the WaterRower S4/S5 monitor.
     */
    public MessageParser() {
        createAndAddMessageInterpreters();
    }

    /* Add all message interpreters to this parser. */
    private void createAndAddMessageInterpreters() {
        interpreters.add(new InformationRequestMessageInterpreter());
        interpreters.add(new PulseCountMessageInterpreter());
        interpreters.add(new StrokeMessageInterpreter());
        interpreters.add(new PingMessageInterpreter());
        interpreters.add(new AcknowledgeMessageInterpreter());
        interpreters.add(new ErrorMessageInterpreter());
        interpreters.add(new HardwareTypeMessageInterpreter());
        interpreters.add(new ResetMessageInterpreter());
        interpreters.add(new ConfigureWorkoutMessageInterpreter());
        interpreters.add(new StartCommunicationMessageInterpreter());
        interpreters.add(new ExitCommunicationMessageInterpreter());
    }

    /* For testing purposes only! Returns all interpreters. */
    List<IMessageInterpreter> getInterpreters() {
        return interpreters;
    }

    /**
     * Decodes and encodes messages received from or sent to the WaterRower S4/S5 monitor.
     * Mainly used for test purposes!
     *
     * @param interpreters The interpreters for the different messages, must not be null.
     */
    MessageParser(List<IMessageInterpreter> interpreters) {
        this.interpreters = requireNonNull(interpreters);
    }


    /**
     * Decodes the given byte array to a message object. Returns a DecodeErrorMessage if the message
     * couldn't be decoded.
     *
     * @param bytes The byte array.
     * @return The message object or a DecodeErrorMessage.
     */
    public AbstractMessage decode(byte[] bytes) {

        Log.debug("Parsing message to object.");

        String msg = new String(bytes, US_ASCII);

        String msgIdentifier;
        for (IMessageInterpreter interpreter : interpreters) {

            // Check message identifiers:
            msgIdentifier = interpreter.getMessageIdentifier();
            if (msgIdentifier == null)
                continue;
            if (!msg.startsWith(msgIdentifier))
                continue;

            // Decode message to an object:
            AbstractMessage decodedMsg = interpreter.decode(msg);

            if (decodedMsg != null)
                return decodedMsg;
        }

        return new DecodeErrorMessage(msg);
    }


    /**
     * Encodes the given message to a byte array. Returns null if the message
     * couldn't be encoded.
     *
     * @param msg The message.
     * @return The byte array or null.
     */
    @SuppressWarnings("unchecked")
    public byte[] encode(AbstractMessage msg) {

        Log.debug("Parsing message '"+msg.toString()+"' to bytes.");

        for (IMessageInterpreter interpreter : interpreters) {

            // Check if message type matches:
            if (!interpreter.isSupported(msg))
                continue;

            // Encode object to message:
            String encodedMsg = interpreter.encode(msg);

            if (encodedMsg != null)
                return encodedMsg.getBytes(US_ASCII);
        }

        Log.warn("Message couldn't be encoded! Unknown message type '"+msg.getClass().getName()+"'.");

        return null;
    }

}
