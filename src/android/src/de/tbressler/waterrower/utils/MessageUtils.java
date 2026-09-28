package de.tbressler.waterrower.utils;

import static java.lang.Integer.parseInt;
import static java.lang.Integer.toHexString;
import static java.lang.String.valueOf;
import static java.util.Objects.requireNonNull;

/**
 * Utils for conversion of ASCII data.
 * The class supports:
 * - ACD (ASCII coded decimal)
 * - ACH (ASCII coded hexadecimal)
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class MessageUtils {

    /* Private constructor. */
    private MessageUtils() {}


    /**
     * Returns the ACD (ASCII coded decimal) as int value.
     *
     * @param ascii The ACD value, must not be null.
     * @return The integer value.
     *
     * @throws NumberFormatException If not a decimal value.
     */
    public static int acdToInt(String ascii) throws NumberFormatException {
        return parseInt(requireNonNull(ascii));
    }


    /**
     * Returns the int value as ACD (ASCII coded decimal).
     *
     * @param value The int value.
     * @param chars The number of chars.
     * @return The int value as ACD.
     *
     * @throws NumberFormatException If the int has more characters than the number of chars given.
     */
    public static String intToAcd(int value, int chars) throws NumberFormatException {
        String ascii = valueOf(value);
        return addLeadingZeros(chars, ascii);
    }


    /**
     * Returns the ACH (ASCII coded hexadecimal) as int value.
     *
     * @param ascii The ACH value, must not be null.
     * @return The integer value.
     *
     * @throws NumberFormatException If not a hexadecimal value.
     */
    public static int achToInt(String ascii) throws NumberFormatException {
        return parseInt(requireNonNull(ascii), 16);
    }


    /**
     * Returns the int value as ACH (ASCII coded hexadecimal).
     *
     * @param value The int value.
     * @param chars The number of chars.
     * @return The int value as ACH.
     *
     * @throws NumberFormatException If the int has more characters than the number of chars given.
     */
    public static String intToAch(int value, int chars) throws NumberFormatException {
        String ascii = toHexString(value).toUpperCase();
        return addLeadingZeros(chars, ascii);
    }


    /* Add leading zeros to string. */
    private static String addLeadingZeros(int chars, String ascii) {
        int numberOfLeadingZeros = chars - ascii.length();
        if (numberOfLeadingZeros < 0)
            throw new NumberFormatException("Number has more than "+chars+" characters!");
        for(int i=0; i<numberOfLeadingZeros; i++)
            ascii = "0"+ascii;
        return ascii;
    }


    /**
     * Returns true if the bit at the given index is 1.
     *
     * @param value The value, must be in range 0x00 to 0xFF.
     * @param index The index, must be in range 0 to 7.
     * @return True if the bit at the index is 1.
     */
    public static boolean getBooleanFromByte(int value, int index) {
        if ((index > 7) || (index < 0))
            throw new IllegalArgumentException("The index is out of range! Only values between 0 and 7 allowed.");
        if ((value < 0) || (value > 0xFF))
            throw new IllegalArgumentException("The value is is out of range! Only values between 0x00 and 0xFF allowed.!");

        int mask = 0x01;
        for (int i=0; i < index; i++)
            mask = mask << 1;

        return ((mask & value) > 0);
    }


    /**
     * Returns the integer from the two byte values.
     *
     * @param high The high byte.
     * @param low The low byte.
     * @return The integer as combination of high and low bytes.
     */
    public static int intFromHighAndLow(int high, int low) {
        if ((high < 0x00) || (high > 0xFF) || (low < 0x00) || (low > 0xFF))
            throw new IllegalArgumentException("The low or high value is out of range!");
        return (high << 8) + low;
    }

    /**
     * Returns the integer from the two byte values.
     *
     * @param up The up byte.
     * @param high The high byte.
     * @param low The low byte.
     * @return The integer as combination of up, high and low bytes.
     */
    public static int intFromUpHighAndLow(int up, int high, int low) {
        if ((up < 0x00) || (up > 0xFF) || (high < 0x00) || (high > 0xFF) || (low < 0x00) || (low > 0xFF))
            throw new IllegalArgumentException("The low, high or up value is out of range!");
        return ((up << 16) + (high << 8) + low);
    }

}
