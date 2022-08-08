#pragma once
#include "bike.h"
#include "qsettings.h"
#include "trixterxdreamv1client.h"
#include "trixterxdreamv1serial.h"
#include "trixterxdreamv1settings.h"

class trixterxdreamv1bike : public bike
{
    Q_OBJECT

private:
    /**
     * @brief SettingsUpdateTimerIntervalMilliseconds The object will check for a settings update at this interval.
     */
    constexpr static int SettingsUpdateTimerIntervalMilliseconds = 10000;

    /**
     * @brief client An object that processes incoming data to CSCS, heart rate and steering data
     */
    trixterxdreamv1client client;

    /**
     * @brief port An object that monitors a serial port to read incoming data, and to write
     * resistance level requests.
     */
    trixterxdreamv1serial * port = nullptr;

    /**
     * @brief resistanceTimerId The id for identifying the resistance timer in void timerEvent(QEvent*).
     */
    int resistanceTimerId = 0;

    /**
     * @brief settingsUpdateTimerId The id for identifying the settings update timer in void timerEvent(QEVent*).
     */
    int settingsUpdateTimerId = 0;

    /**
     * @brief noHeartService Suppress heart rate readings, QZ level setting.
     */
    bool noHeartService;

    /**
     * @brief noHeartRate Value from app settings combined with QZ's noHeartService value.
     */
    bool noHeartRate;

    /**
     * @brief noVirtualDevice Suppress virtual device.
     */
    bool noVirtualDevice;

    /**
     * @brief noWriteResistance Suppress sending resistance to device.
     */
    bool noWriteResistance;

    /**
     * @brief noSteering Suppress steering readings
     */
    bool noSteering;

    /**
     * @brief resistanceLevel The last requested resistance level.
     */
    uint8_t resistanceLevel = 0;

    /**
     * @brief wheelCircumference The simulated circumference of the bike's wheels, for converting
     * angular velocity to a speed. Units: kilometers.
     */
    double wheelCircumference;

    /**
     * @brief t0 The start time in milliseconds. Used to reduce te size of time values processed.
     */
    qint64 t0=0;

    /**
     * @brief packetsProcessed The number of packets processed.
     */
    uint32_t packetsProcessed=0;

    /**
     * @brief lastPacketProcessedTime The last time (from getTime()) a packet was processed.
     */
    uint32_t lastPacketProcessedTime=0;

    /**
     * @brief appSettings The application settings.
     */
    trixterxdreamv1settings * appSettings;

    /**
     * @brief lastAppSettingsVersion The last app settings version that was used to configure the object.
     */
    uint32_t lastAppSettingsVersion=0;

    /**
     * @brief steeringMap Stores the mapping between incoming steering values and the steering angles expected by the application.
     */
    std::vector<double> steeringMap;

    /**
     * @brief getTime Gets the time in miliseconds since this object was created.
     */
    static uint32_t getTime();

    /**
     * @brief updateClient Passes the array of bytes into the client one by one.
     * @param bytes The incoming bytes.
     * @param client The client object that interprets the incoming bytes into data packets.
     * @return True if the state of the client changed due to the input.
     */
    static bool updateClient(const QByteArray &bytes, trixterxdreamv1client * client);

    /**
     * @brief Called by the data source (serial port) when a new block of data arrives.
     */
    void update(const QByteArray& bytes);

    /**
     * @brief updateResistance Called by the resistanceTimer to send the resistance request to the
     * device.
     */
    void updateResistance();

    /**
     * @brief calculateSteering Calculates the mapping between steering values coming from the device, and
     * the steering angles sent to the application. Uses the values in the appSettings field.
     */
    void calculateSteeringMap();
protected:

    /**
     * @brief timerEvent Processes timer events, e.g. for resistance.
     * @param event
     */
    void timerEvent(QTimerEvent *event) override;

    /**
     * @brief disconnectPort Disconnect the serial port and resistance timer.
     */
    void disconnectPort();
public Q_SLOTS:
    /**
     * @brief changeResistance Called to change the requested resistance level.
     * @param resistanceLevel The resistance level to request (0..250)
     */
    void changeResistance(int8_t resistanceLevel) override;


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
    constexpr static double DefaultWheelDiameter = 26*0.0254;

    /**
     * @brief DisconnectionTimeout The number of milliseconds of no packets processed required before
     * this object will be considered disconnected from the device.
     */
    constexpr static int32_t DisconnectionTimeout = 50;

    /**
     * @brief bluetoothDeviceInfo A QBluetoothDeviceInfo object for functions that need it.
     */
    const QBluetoothDeviceInfo bluetoothDeviceInfo { QBluetoothUuid {QStringLiteral("774f25bd-6636-4cdc-9398-839de026be1d")}, "Trixter X-Dream V1 Bike", 0};

    /**
     * @brief trixterxdreamv1bike Constructor
     * @param noWriteResistance Option to avoid sending resistance to the device.
     * @param noHeartService Option to avoid using the heart rate reading.
     * @param noVirtualDevice Option to avoid using a virtual device.
     */
    trixterxdreamv1bike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice);

    ~trixterxdreamv1bike();

    /**
     * @brief connect Attempt to connect to the specified port.
     * @param portName The name of the serial port to connect to.
     */
    bool connect(QString portName);

    /**
     * @brief connected Indicates if a valid packet was received from the device within the DisconnectionTimeout.
     */
    bool connected() override;

    /**
     * @brief set_wheelDiameter Set the simulated wheel diameter to be used for converting angular velocity to speed. Units: meters
     * @param value
     */
    void set_wheelDiameter(double value);

    /**
     * @brief get_appSettings Gets the settings object for this device type.
     */
    const trixterxdreamv1settings * get_appSettings() { return this->appSettings; }

    /**
     * @brief maxResistance The maximum resistance supported.
     * @return
     */
    uint8_t maxResistance() override { return trixterxdreamv1client::MaxResistance; }

    /**
     * @brief tryCreate Attempt to create an object to interact with an existing Trixter X-Dream V1 bike on a specific serial port,
     * or if the port is unspecified, any serial port.
     * @param noWriteResistance Option to avoid sending resistance to the device.
     * @param noHeartService Option to avoid using the heart rate reading.
     * @param noVirtualDevice Option to avoid using a virtual device.
     * @param portName (Optional) The specific port to search.
     * @return nullptr if no device is found, an object if a device is found and connected.
     */
    static trixterxdreamv1bike * tryCreate(bool noWriteResistance, bool noHeartService, bool noVirtualDevice, const QString& portName = nullptr);

    /**
     * @brief tryCreate Attempt to create an object to interact with an existing Trixter X-Dream V1 bike on a specific serial port,
     * or if the port is unspecified, any serial port.
     * @param port (Optional) The specific port to search.
     * @return
     */
    static trixterxdreamv1bike * tryCreate(const QString& portName = nullptr);

};
