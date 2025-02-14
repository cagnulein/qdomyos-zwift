package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
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
public class ErrorMessage extends AbstractMessage {

    @Override
    public String toString() {
        return toStringHelper(this).toString();
    }

}
