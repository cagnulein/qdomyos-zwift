package de.tbressler.waterrower.io.msg.out;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
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
public class StartCommunicationMessage extends AbstractMessage {

    @Override
    public String toString() {
        return toStringHelper(this).toString();
    }

}
