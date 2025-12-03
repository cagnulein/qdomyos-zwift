package de.tbressler.waterrower.io.utils;

import io.netty.buffer.ByteBuf;

/**
 * Helper class for byte, byte buffers and byte arrays.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ByteUtils {

    /* Private constructor. */
    private ByteUtils()  {}

    /**
     * Returns the data of the buffer as String for debug purposes.
     *
     * @param buffer The byte buffer.
     * @return The data of the buffer as String.
     */
    public static String bufferToString(ByteBuf buffer) {
        return "ByteBuf[index=" + buffer.readerIndex() + ",bytes=" + buffer.readableBytes() + "]";
    }

}
