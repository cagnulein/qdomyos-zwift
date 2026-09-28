package de.tbressler.waterrower.io.msg.out;

import de.tbressler.waterrower.io.msg.InformationRequestMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
 * Request Model Information (PC -> S4/S5).
 *
 * Request details from the rowing computer on what it is and firmware version.
 *
 * [I][V?] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class RequestModelInformationMessage extends InformationRequestMessage {

    @Override
    public String toString() {
        return toStringHelper(this).toString();
    }

}
