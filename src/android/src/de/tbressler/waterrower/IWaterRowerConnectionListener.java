package de.tbressler.waterrower;

import de.tbressler.waterrower.model.ErrorCode;
import de.tbressler.waterrower.model.ModelInformation;

/**
 * Listener interface for the connection to the WaterRower monitor.
 *
 * @author Tobias Bressler
 * @version 1.0
 */
public interface IWaterRowerConnectionListener {

    /**
     * Will be called, if a supported WaterRower monitor was connected.
     *
     * @param modelInformation Model information (e.g. monitor type and firmware version).
     */
    void onConnected(ModelInformation modelInformation);

    /**
     * Will be called, if the WaterRower monitor was disconnected.
     */
    void onDisconnected();

    /**
     * Will be called, if an error occurred while communicating with WaterRower monitor.
     *
     * @param errorCode The error code.
     */
    void onError(ErrorCode errorCode);

}