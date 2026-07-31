package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.model.StrokeType;

import static com.google.common.base.MoreObjects.toStringHelper;
import static java.util.Objects.requireNonNull;

/**
 * Stroke start/end (S4/S5 -> PC).
 *
 * This packet is auto transmitted by the rowing computer.
 *
 * Start of stroke:
 *
 * Start of stroke pull to show when the rowing computer determined acceleration occurring in
 * the paddle. This packet has the highest priority of transmission on the USB.
 *
 * [S][S] + 0x0D0A
 *
 * End of stroke:
 *
 * End of stroke pull to show when the rowing computer determined deceleration occurring in
 * the paddle. (Now entered the relax phase). This packet has the second highest priority of
 * transmission on the USB.
 *
 * [S][E] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class StrokeMessage extends AbstractMessage {


    /* The type of stroke. */
    private final StrokeType strokeType;


    /**
     * Stroke start/end message.
     *
     * @param stroke The type of stroke, must not be null.
     */
    public StrokeMessage(StrokeType stroke) {
        this.strokeType = requireNonNull(stroke);
    }


    /**
     * Returns the type of stroke.
     *
     * @return The type of stroke, never null.
     */
    public StrokeType getStrokeType() {
        return strokeType;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("strokeType", strokeType)
                .toString();
    }

}
