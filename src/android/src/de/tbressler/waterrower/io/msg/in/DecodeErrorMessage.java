package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
 * Decode error message.
 */
public class DecodeErrorMessage extends AbstractMessage {

    private final String message;

    public DecodeErrorMessage(String message) {
        this.message = message;
    }

    public String getMessage() {
        return this.message;
    }

    @Override
    public String toString() {
        return toStringHelper(this).toString();
    }

}
