package de.tbressler.waterrower.io.msg.out;

import de.tbressler.waterrower.io.msg.InformationRequestMessage;
import de.tbressler.waterrower.io.msg.Memory;

import static com.google.common.base.MoreObjects.toStringHelper;
import static java.util.Objects.requireNonNull;

/**
 * Read memory locations (PC -> S4/S5).
 *
 * Read a single memory location:
 *
 * Requests the contents of a single location XXX, this will return a single byte in hex format.
 *
 * [I][RS] + XXX + 0x0D0A
 *
 * Read double memory locations:
 *
 * Requests the contents of two location starting from XXX, this will return two bytes in hex format.
 *
 * [I][RD] + XXX + 0x0D0A
 *
 * Read triple memory locations:
 *
 * Requests the contents of three locations starting from XXX, this will return three bytes in hex format.
 *
 * [I][RT] + XXX + 0x0D0A
 *
 * XXX is in ACH format and has a maximum range of 0x000 to 0xFFF, however not all locations are
 * available (see Memory Map), errors will be replied for out of spec memory reads.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ReadMemoryMessage extends InformationRequestMessage {


    /* The memory location (0 .. 4095). */
    private final int location;

    /* Defines if you want to read single, double or triple memory locations. */
    private final Memory memory;


    /**
     * This message requests the contents of a single location XXX, this will return a single
     * byte in hex format.
     *
     * @param memory Define if you want to read single, double or triple memory locations, must
     *               not be null.
     * @param location The memory location (0 .. 4095), please refer to memory map of the Water
 *                 Rower monitor.
     */
    public ReadMemoryMessage(Memory memory, int location) {
        if ((location < 0) || (location > 4095))
            throw new IllegalArgumentException("The value for the memory location must be between 0 and 4095!");
        this.memory = requireNonNull(memory);
        this.location = location;
    }


    /**
     * Returns the memory location (0 .. 4095), please refer to memory map of the WaterRower
     * monitor.
     *
     * @return The memory location (0 .. 4095).
     */
    public int getLocation() {
        return location;
    }


    /**
     * Returns if this message reads from single, double or triple memory locations.
     *
     * @return Single, double or triple memory locations.
     */
    public Memory getMemory() {
        return memory;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("memory", memory)
                .add("location", location)
                .toString();
    }

}
