package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
 * Hardware Type (S4/S5 -> PC).
 *
 * The WaterRower will reply with this packet when it receives a "USB" packet and will then
 * proceed to send other packets accordingly until it switch’s off or the application issues an
 * exit packet.
 *
 * [_][WR_] + 0x0D0A
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class HardwareTypeMessage extends AbstractMessage {

    /* True if the connected device is a WaterRower. */
    private final boolean isWaterRower;


    /**
     * The WaterRower will reply with this packet when it receives a "USB" packet and will then
     * proceed to send other packets accordingly until it switch’s off or the application issues an
     * exit packet.
     *
     * @param isWaterRower True if the connected device is a WaterRower.
     */
    public HardwareTypeMessage(boolean isWaterRower) {
        this.isWaterRower = isWaterRower;
    }


    /**
     * Returns true if the hardware type is a "WaterRower".
     *
     * @return True if the hardware is a "WaterRower".
     */
    public boolean isWaterRower() {
        return isWaterRower;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("isWaterRower", isWaterRower)
                .toString();
    }

}
