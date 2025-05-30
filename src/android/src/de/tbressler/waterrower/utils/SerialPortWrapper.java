package de.tbressler.waterrower.utils;

import com.fazecast.jSerialComm.SerialPort;

import java.util.Arrays;
import java.util.List;

import static java.util.Collections.emptyList;
import static java.util.stream.Collectors.toList;

/**
 * A wrapper for the serial port class of jSerialComm.
 * This wrapper is used in the unit tests.
 *
 * @author Tobias Breßler
 * @version 1.0
 */
public class SerialPortWrapper {

    /**
     * Returns the list of available ports.
     *
     * @return A list of the available ports, never null.
     */
    public List<AvailablePort> getAvailablePorts() {
        SerialPort[] serialPorts = SerialPort.getCommPorts();
        if (serialPorts.length == 0)
            return emptyList();
        return Arrays.stream(serialPorts).map((port) -> new AvailablePort(port)).collect(toList());
    }

}
