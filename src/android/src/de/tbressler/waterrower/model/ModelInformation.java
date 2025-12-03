package de.tbressler.waterrower.model;

import static com.google.common.base.MoreObjects.toStringHelper;
import static java.util.Objects.requireNonNull;

/**
 * Model information (details from the rowing computer), e.g. Monitor type / model and firmware version.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public class ModelInformation {

    /* The type of monitor (e.g. S4 or S5). */
    private final MonitorType monitorType;

    /* The firmware version of the monitor */
    private final String firmwareVersion;


    /**
     * Current model information.
     *
     * @param monitorType The type of monitor (e.g. S4 or S5), must not be null.
     * @param firmwareVersion The firmware version of the monitor, must not be null.
     */
    public ModelInformation(MonitorType monitorType, String firmwareVersion) {
        this.monitorType = requireNonNull(monitorType);
        this.firmwareVersion = requireNonNull(firmwareVersion);
    }


    /**
     * Returns the type of monitor (e.g. S4 or S5).
     *
     * @return The type of monitor, never null.
     */
    public MonitorType getMonitorType() {
        return monitorType;
    }


    /**
     * The firmware version of the monitor.
     *
     * @return The firmware version of the monitor, never null.
     */
    public String getFirmwareVersion() {
        return firmwareVersion;
    }


    @Override
    public String toString() {
        return toStringHelper(this)
                .add("monitorType", monitorType)
                .add("firmwareVersion", firmwareVersion)
                .toString();
    }

}
