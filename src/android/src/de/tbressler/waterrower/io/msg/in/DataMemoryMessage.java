package de.tbressler.waterrower.io.msg.in;

import de.tbressler.waterrower.io.msg.out.ReadMemoryMessage;

import static com.google.common.base.MoreObjects.toStringHelper;
import static de.tbressler.waterrower.io.msg.Memory.*;
import static de.tbressler.waterrower.utils.MessageUtils.intToAch;

/**
 * Value from single, double or triple memory locations (S4/S5 -> PC).
 *
 * The read packets will retrieve values from the rowing-computer memory, these locations are
 * raw data which maybe a decimal, hexadecimal, binary or BCD format, each will be returned in
 * ACH format in the packet. Correct conversion and usage will be needed for the PC application
 * to use the values.
 *
 * Value from single memory location:
 *
 * Returns the single byte of data Y1 from location XXX for the users application.
 *
 * [I][DS] + XXX + Y1 + 0x0D0A
 *
 * Value from double memory locations:
 *
 * Returns two bytes of data starting from the second location first (Y2) then location XXX (Y1).
 * This is for reading 16bit values which have (H)igh and (L)ow pair in one go.
 *
 * [I][DD] + XXX + Y2 + Y1 + 0x0D0A
 *
 * Value from triple memory locations:
 *
 * Returns three bytes of data starting from the third location first (Y3) then (Y2) to location
 * XXX (Y1). This is for reading 24bit values like a clock, which has Hours, Minutes & Seconds.
 *
 * [I][DT] + XXX + Y3 + Y2 + Y1 + 0x0D0A
 *
 * XXX is in ACH format and has a maximum range of 0x000 to 0xFFF, however not all locations are
 * available (see Memory Map), errors will be replied for out of spec memory reads.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class DataMemoryMessage extends ReadMemoryMessage {

    /* The single byte of data Y1 (0 .. 255) from memory location. */
    private final int value1;

    /* The single byte of data Y2 (0 .. 255) from memory location. */
    private final int value2;

    /* The single byte of data Y3 (0 .. 255) from memory location. */
    private final int value3;


    /**
     * This message returns the single byte Y1 of data from single memory location for the users
     * application.
     *
     * @param location The memory location (0 .. 4095), please refer to memory map of the Water
     *                 Rower monitor.
     * @param value1 The single byte of data Y1 (0 .. 255) from memory location.
     */
    public DataMemoryMessage(int location, int value1) {
        super(SINGLE_MEMORY, location);
        this.value3 = -1;
        this.value2 = -1;
        this.value1 = assertValueRange(value1);
    }

    /**
     * This message returns the bytes Y1 and Y2 of data from double memory location for the users
     * application.
     *
     * @param location The memory location (0 .. 4095), please refer to memory map of the Water
     *                 Rower monitor.
     * @param value2 The single byte of data Y2 (0 .. 255) from memory location.
     * @param value1 The single byte of data Y1 (0 .. 255) from memory location.
     */
    public DataMemoryMessage(int location, int value2, int value1) {
        super(DOUBLE_MEMORY, location);
        this.value3 = -1;
        this.value2 = assertValueRange(value2);
        this.value1 = assertValueRange(value1);
    }

    /**
     * This message returns the bytes Y1, Y2 and Y3 of data from triple memory location for the
     * users application.
     *
     * @param location The memory location (0 .. 4095), please refer to memory map of the Water
     *                 Rower monitor.
     * @param value3 The single byte of data Y3 (0 .. 255) from memory location.
     * @param value2 The single byte of data Y2 (0 .. 255) from memory location.
     * @param value1 The single byte of data Y1 (0 .. 255) from memory location.
     */
    public DataMemoryMessage(int location, int value3, int value2, int value1) {
        super(TRIPLE_MEMORY, location);
        this.value3 = assertValueRange(value3);
        this.value2 = assertValueRange(value2);
        this.value1 = assertValueRange(value1);
    }

    /* Throws IllegalArgumentException if value is out of range. */
    private int assertValueRange(int value) {
        if ((value < 0) || (value > 255))
            throw new IllegalArgumentException("The value must be between 0 and 255!");
        return value;
    }


    /**
     * Returns the single byte of data Y3 from the memory location.
     *
     * @return The single byte of data (0 .. 255) from memory location. The value is -1 for
     * SINGLE_MEMORY or DOUBLE_MEMORY messages.
     */
    public int getValue3() {
        return value3;
    }

    /**
     * Returns the single byte of data Y3 from the memory location (as ACH string).
     *
     * @return The single byte of data (00 .. FF) from memory location.
     */
    public String getValue3AsACH() {
        return intToAch(value3, 2);
    }


    /**
     * Returns the single byte of data Y2 from the memory location.
     *
     * @return The single byte of data (0 .. 255) from memory location. The value is -1 for
     * SINGLE_MEMORY messages.
     */
    public int getValue2() {
        return value2;
    }

    /**
     * Returns the single byte of data Y2 from the memory location (as ACH string).
     *
     * @return The single byte of data (00 .. FF) from memory location.
     */
    public String getValue2AsACH() {
        return intToAch(value2, 2);
    }


    /**
     * Returns the single byte of data Y1 from the memory location.
     *
     * @return The single byte of data (0 .. 255) from memory location.
     */
    public int getValue1() {
        return value1;
    }

    /**
     * Returns the single byte of data Y1 from the memory location (as ACH string).
     *
     * @return The single byte of data (00 .. FF) from memory location.
     */
    public String getValue1AsACH() {
        return intToAch(value1, 2);
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("memory", getMemory())
                .add("location", getLocation())
                .add("value3", value3)
                .add("value2", value2)
                .add("value1", value1)
                .toString();
    }

}
