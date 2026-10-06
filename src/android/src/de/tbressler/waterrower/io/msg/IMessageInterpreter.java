package de.tbressler.waterrower.io.msg;


/**
 * Interface for message interpreters, which decode or encode incoming and outgoing messages.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public interface IMessageInterpreter<T extends AbstractMessage> {

    /**
     * Returns the identifier of the message type which this interpreter can decode. Can be null if no incoming
     * messages of this type are expected.
     *
     * @return The identifier char or null.
     */
    String getMessageIdentifier();

    /**
     * Returns true if the message type is supported by this interpreter.
     *
     * @return The message.
     */
    boolean isSupported(AbstractMessage msg);

    /**
     * Decodes the given ASCII string to a message object. If the message can not be decoded the method returns null.
     *
     * @param msg The message as ASCII string.
     * @return The message object or null.
     */
    T decode(String msg);

    /**
     * Encodes the given message object to a ASCII string. If the message can not be encoded the method returns null.
     *
     * @param msg The message object.
     * @return The message as ASCII string.
     */
    String encode(T msg);

}
