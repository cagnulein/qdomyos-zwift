package de.tbressler.waterrower.io.msg.interpreter;

import de.tbressler.waterrower.io.msg.AbstractMessage;
import de.tbressler.waterrower.io.msg.AbstractMessageInterpreter;
import de.tbressler.waterrower.io.msg.InformationRequestMessage;
import de.tbressler.waterrower.io.msg.in.DataMemoryMessage;
import de.tbressler.waterrower.io.msg.in.ModelInformationMessage;
import de.tbressler.waterrower.io.msg.out.ReadMemoryMessage;
import de.tbressler.waterrower.io.msg.out.RequestModelInformationMessage;
import de.tbressler.waterrower.log.Log;
import de.tbressler.waterrower.model.ModelInformation;
import de.tbressler.waterrower.model.MonitorType;

import static de.tbressler.waterrower.model.MonitorType.*;
import static de.tbressler.waterrower.utils.MessageUtils.achToInt;
import static de.tbressler.waterrower.utils.MessageUtils.intToAch;

/**
 * Interpreter for:
 *
 * Request Model Information (PC -> S4/S5).
 *
 * Request details from the rowing computer on what it is and firmware version.
 *
 * [I][V?] + 0x0D0A
 *
 * Current Model Information (S4/S5 -> PC).
 *
 * Details of what unit is attached:
 * - Model - Sent as 4 or 5 to indicate if it is a Series 4 or series 5 rowing computer.
 * - Version high - 02 as an example for version 2.00 MSB of the firmware version.
 * - Version low - 00 as an example for version 2.00 LSB of the firmware version.
 *
 * [I][V] + [Model] + [Version High] + [Version Low] + 0x0D0A
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
 * @author Tobias Bressler
 * @version 1.0
 */
public class InformationRequestMessageInterpreter extends AbstractMessageInterpreter<InformationRequestMessage> {

    @Override
    public String getMessageIdentifier() {
        return "I";
    }

    @Override
    public boolean isSupported(AbstractMessage msg) {
        return (msg instanceof InformationRequestMessage);
    }

    @Override
    public InformationRequestMessage decode(String msg) {

        if (msg.startsWith("IV")) {
            return decodeModelInformationMessage(msg);
        } else if (msg.startsWith("IDS")) {
            return decodeSingleMemoryLocation(msg);
        } else if (msg.startsWith("IDD")) {
            return decodeDoubleMemoryLocation(msg);
        } else if (msg.startsWith("IDT")) {
            return decodeTripleMemoryLocation(msg);
        }

        Log.warn("Message couldn't be decoded!\n" +
                " Message was: >" + msg + "<");

        return null;
    }

    /* Parse current model information. */
    private ModelInformationMessage decodeModelInformationMessage(String msg) {

        MonitorType monitorType = parseMonitorType(msg);
        String firmwareVersion = msg.substring(3, 5) + "." + msg.substring(5, 7);

        return new ModelInformationMessage(new ModelInformation(monitorType, firmwareVersion));
    }

    /* Parses and returns the monitor type from the given message. */
    private MonitorType parseMonitorType(String payload) {
        switch (payload.charAt(2)) {
            case '4':
                return WATER_ROWER_S4;
            case '5':
                return WATER_ROWER_S5;
        }
        return UNKNOWN_MONITOR_TYPE;
    }

    /* Parse value from single memory location. */
    private DataMemoryMessage decodeSingleMemoryLocation(String msg) {

        int location = achToInt(msg.substring(3, 6));
        int value1 = achToInt(msg.substring(6, 8));

        return new DataMemoryMessage(location, value1);
    }

    /* Parse values from double memory locations. */
    private DataMemoryMessage decodeDoubleMemoryLocation(String msg) {

        int location = achToInt(msg.substring(3, 6));
        int value2 = achToInt(msg.substring(6, 8));
        int value1 = achToInt(msg.substring(8, 10));

        return new DataMemoryMessage(location, value2, value1);
    }

    /* Parse values from triple memory locations. */
    private DataMemoryMessage decodeTripleMemoryLocation(String msg) {

        int location = achToInt(msg.substring(3, 6));
        int value3 = achToInt(msg.substring(6, 8));
        int value2 = achToInt(msg.substring(8, 10));
        int value1 = achToInt(msg.substring(10, 12));

        return new DataMemoryMessage(location, value3, value2, value1);
    }


    @Override
    public String encode(InformationRequestMessage msg) {

        if (msg instanceof RequestModelInformationMessage) {
            return "IV?";
        } else if (msg instanceof ReadMemoryMessage) {
            return encodeReadMemoryMessage((ReadMemoryMessage) msg);
        }

        Log.warn("Message couldn't be encoded!\n" +
                " Message was: " + msg);

        return null;
    }

    /* Encodes messages of type ReadMemoryMessage. */
    private String encodeReadMemoryMessage(ReadMemoryMessage msg) {
        String result = "IR";

        switch (msg.getMemory()) {
            case SINGLE_MEMORY:
                result += "S";
                break;
            case DOUBLE_MEMORY:
                result += "D";
                break;
            case TRIPLE_MEMORY:
                result += "T";
                break;
            default:
                Log.warn("Message contains invalid values!\n" +
                        " Message was: "+msg.toString());
                return null;
        }

        result += intToAch(msg.getLocation(), 3);

        return result;
    }

}
