package de.tbressler.waterrower.io.msg.out;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
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
public class ResetMessage extends AbstractMessage {

    @Override
    public String toString() {
        return toStringHelper(this).toString();
    }

}
