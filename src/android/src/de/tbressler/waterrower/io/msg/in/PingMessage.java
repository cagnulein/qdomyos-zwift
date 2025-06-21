package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
 * Ping (S4/S5 -> PC).
 *
 * Sent once a second while NO rowing is occurring to indicate to the PC the rowing monitor is
 * still operational but stopped.
 *
 * [P][ING] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class PingMessage extends AbstractMessage {

    @Override
    public String toString() {
        return toStringHelper(this).toString();
    }

}
