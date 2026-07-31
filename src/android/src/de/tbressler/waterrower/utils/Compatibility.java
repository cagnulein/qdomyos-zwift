package de.tbressler.waterrower.utils;

import de.tbressler.waterrower.model.ModelInformation;
import de.tbressler.waterrower.model.MonitorType;

import static java.util.Objects.requireNonNull;

/**
 * Helper class to check compatibility of model/monitor type and firmware version with this library.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class Compatibility {

    /* Private constructor. */
    private Compatibility() {}


    /**
     * Returns true if monitor type and firmware are supported by this library.
     *
     * @param modelInformation The model information from the device, must not be null.
     * @return True if monitor type and firmware are supported by this library.
     */
    public static boolean isSupportedWaterRower(ModelInformation modelInformation) {
        requireNonNull(modelInformation);

        if (!isModelTypeSupported(modelInformation.getMonitorType()))
            return false;
        return isFirmwareVersionSupported(modelInformation.getFirmwareVersion());
    }

    /* Returns true if monitor type is supported. */
    private static boolean isModelTypeSupported(MonitorType monitorType) {
        switch (monitorType) {
            case WATER_ROWER_S4:
            case WATER_ROWER_S5:
                return true;
            default:
                return false;
        }
    }

    /* Returns true if firmware version is supported. */
    private static boolean isFirmwareVersionSupported(String firmwareVersion) {
        return firmwareVersion.startsWith("02.");
    }

}
