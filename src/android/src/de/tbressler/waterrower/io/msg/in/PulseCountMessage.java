package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.AbstractMessage;

import static com.google.common.base.MoreObjects.toStringHelper;

/**
 * Pulse Count in the last 25mS (S4/S5 -> PC).
 *
 * This packet is auto transmitted by the rowing computer.
 *
 * "XX" is an ACH value representing the number of pulse’s counted during the last 25mS
 * period; this value can range from 1 to 50 typically. (Zero values will not be transmitted).
 * Please refer to "WaterRower Series 4 Rowing Algorithm.doc" for in depth details on how to use
 * this data. At this time the constant values are:
 *
 * pins_per_xxcm 32 ; number of pin edges allowed to equal xxcm (dec)
 * distance_xxcm 35 ; number of cm per flagged xxcm no. of pins (dec)
 *
 * This packet has the third highest priority of transmission on the USB.
 *
 * [P] + XX + 0x0D0A
 *
 * It seems the Pulse counts roughly (and linearly) corresponds to the distance. By experimenting you can
 * find out your coef so that the distance on your S4 equals transforms pulses: `distanceOnS4 = pulseCount * coef`.
 * On my Waterrower this number was 0.011.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class PulseCountMessage extends AbstractMessage {

    /* The number of pulse’s counted. */
    private final int pulsesCounted;


    /**
     * Message for Pulse Count in the last 25mS.
     *
     * @param pulsesCounted The number of pulse’s counted during the last 25mS period.
     */
    public PulseCountMessage(int pulsesCounted) {
        this.pulsesCounted = pulsesCounted;
    }


    /**
     * Returns the number of pulse’s counted during the last 25mS period.
     *
     * @return The number of pulse’s counted.
     */
    public int getPulsesCounted() {
        return pulsesCounted;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("pulsesCounted", pulsesCounted)
                .toString();
    }

}
