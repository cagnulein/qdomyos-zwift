#pragma once
#include "bike.h"
#include "trixterxdreamv1client.h"
#include "trixterxdreamv1serial.h"

class trixterxdreamv1bike : public bike
{
    Q_OBJECT
private:
    /**
     * @brief client An object that processes incoming data to CSCS, heart rate and steering data
     */
    trixterxdreamv1client client;

    /**
     * @brief port An object that monitors a serial port to read incoming data, and to write
     * resistance level requests.
     */
    trixterxdreamv1serial port;

    /**
     * @brief resistanceTimer A timer to push the currently requested resistance level to the device.
     */
    QTimer * resistanceTimer;

    /**
     * @brief noHeartService Suppress heart rate readings.
     */
    bool noHeartService;

    /**
     * @brief noVirtualDevice Suppress virtual device.
     */
    bool noVirtualDevice;

    /**
     * @brief noWriteResistance Suppress sending resistance to device.
     */
    bool noWriteResistance;

    /**
     * @brief noSteering Suppress steering readings.
     */
    bool noSteering;

    /**
     * @brief resistanceLevel The last requested resistance level.
     */
    uint8_t resistanceLevel;

    /**
     * @brief wheelCircumference The simulated circumference of the bike's wheels, for converting
     * angular velocity to a speed. Units: kilometers.
     */
    double wheelCircumference;

    /**
     * @brief t0 The start time in milliseconds. Used to reduce te size of time values processed.
     */
    qint64 t0;

    /**
     * @brief packetsProcessed The number of packets processed.
     */
    uint32_t packetsProcessed;

    /**
     * @brief getTime Gets the time in miliseconds since this object was created.
     * @return The number of milliseconds since this object was created.
     */
    uint32_t getTime();
    
    /**
     * @brief Temporary method to contain what happens when a new block of data comes in
     * from the data source (serial port).
     */
    void update(QByteArray bytes);

    /**
     * @brief updateResistance Called by the resistanceTimer to send the resistence request to the
     * device.
     */
    void updateResistance();

    /**
     * @brief updateClient Passes the array of bytes into the client one by one.
     * @param bytes The incoming bytes.
     * @param client The client object that interprets the incoming bytes into data packets.
     * @return True if the state of the client changed due to the input.
     */
    static bool updateClient(QByteArray bytes, trixterxdreamv1client * client);

    /**
     * @brief testPort Tries to open a port and looks for valid data packets.
     * @param portName The name of the serial port.
     * @return True if valid data packets were obtained from the port.
     */
    static bool testPort(const QString &portName);
protected:
    virtual BLUETOOTH_TYPE devicetype() { return BIKE; }

public Q_SLOTS:
    /**
     * @brief changeResistance Called to change the requested resistance level.
     * @param resistanceLevel The resistance level to request (0..250)
     */
    virtual void changeResistance(int8_t resistanceLevel);

public:

    /**
     * @brief MaxWheelDiameter The maximum supported wheel diameter. Unit: meters
     */
    constexpr static double MaxWheelDiameter = 2.0;

    /**
     * @brief MinWheelDiameter The minimum supported wheel diameter. Unit: meters
     */
    constexpr static double MinWheelDiameter = 0.1;

    /**
     * @brief DefaultWheelDiameter The default wheel diameter. Unit: meters
     */
    constexpr static double DefaultWheelDiamter = 26*0.0254;


    /**
     * @brief trixterxdreamv1bike Constructor
     * @param portName The name of the serial port to connect to.
     * @param noWriteResistance Option to avoid sending resistance to the device.
     * @param noHeartService Option to avoid using the heart rate reading.
     * @param noVirtualDevice Option to avoid using a virtual device.
     * @param noSteering Option to avoid using the steering reading.
     */
    trixterxdreamv1bike(QString portName, bool noWriteResistance, bool noHeartService, bool noVirtualDevice, bool noSteering);

    ~trixterxdreamv1bike();

    /**
     * @brief set_wheelDiameter Set the simulated wheel diameter to be used for converting angular velocity to speed. Units: meters
     * @param value
     */
    void set_wheelDiameter(double value);

    /**
     * @brief maxResistance The maximum resistance supported.
     * @return
     */
    virtual uint8_t maxResistance() { return trixterxdreamv1client::MaxResistance; }

    /**
     * @brief findPort Looks for an X-Dream V1 bike on known serial ports and returns the port name if it finds one.
     */
    static QString findPort();

};
