#include "trixterxdreamv1bike.h"
#include "trixterxdreamv1serial.h"
#include "trixterxdreamv1settings.h"
#include "qcoreevent.h"

#include <cmath>
#include <qmath.h>
#include <QSerialPortInfo>
#include <QTimer>

using namespace std;

trixterxdreamv1bike::trixterxdreamv1bike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    // Set the wheel diameter for speed and distance calculations
    this->set_wheelDiameter(DefaultWheelDiameter);

    // Create the settings object and load from QSettings.
    this->appSettings = new trixterxdreamv1settings();

    // QZ things from expected constructor
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    this->noSteering = !appSettings->get_steeringEnabled();

    // Calculate the steering mapping
    this->calculateSteeringMap();
}

bool trixterxdreamv1bike::connect(QString portName) {
    // In case already connected, disconnect.
    this->disconnectPort();

    // Get the current time in milliseconds since ancient times.
    // This will be subtracted from further readings from getTime() to get an easier to look at number.
    this->t0 = this->getTime();

    auto thisObject = this;

    // create the port object and connect it
    this->port = new trixterxdreamv1serial(this);
    this->port->set_receiveBytes([thisObject](const QByteArray& bytes)->void{thisObject->update(bytes);});

    // References to objects for callbacks
    auto device=this->port;

    // tell the client where to get the time
    this->client.set_GetTime(getTime);

    // tell the client how to send data to the device
    if(!noWriteResistance)
        this->client.set_WriteBytes([device](uint8_t * bytes, int length)->void{ device->write(QByteArray((const char *)bytes, length), "");});

    // open the port. This should be at 115200 bits per second.
    this->port->open(portName, QSerialPort::Baud115200, 1000);

    // wait for some packets to arrive
    QThread::msleep(500);

    if(!this->noWriteResistance)
    {
        this->resistanceTimerId = this->startTimer(trixterxdreamv1client::ResistancePulseIntervalMilliseconds, Qt::PreciseTimer);
        if(this->resistanceTimerId==0)
        {
            qDebug() << "Failed to start resistance timer";
            throw "Failed to start resistance timer";
        }
    }

    this->settingsUpdateTimerId = this->startTimer(SettingsUpdateTimerIntervalMilliseconds, Qt::VeryCoarseTimer);
    if(this->settingsUpdateTimerId==0)
    {
        qDebug() << "Failed to start settings update timer. Too bad.";
    }

    if(!this->connected())
    {
        qDebug() << "Failed to connect to device";
        this->disconnectPort();
        return false;
    }

    return this->connected();
}

void trixterxdreamv1bike::disconnectPort() {
    if(this->port) {
        qDebug() << "Disconnecting from serial port";
        delete this->port;
        this->port = nullptr;
    }
    if(this->resistanceTimerId) {
        qDebug() << "Kiling resistance timer";
        this->killTimer(this->resistanceTimerId);
        this->resistanceTimerId = 0;
    }
    if(this->settingsUpdateTimerId) {
        qDebug() << "Kiling settings update timer";
        this->killTimer(this->settingsUpdateTimerId);
        this->settingsUpdateTimerId = 0;
    }
}

bool trixterxdreamv1bike::connected() {
    return (this->getTime()-this->lastPacketProcessedTime) < DisconnectionTimeout;
}


uint32_t trixterxdreamv1bike::getTime() {
    auto currentDateTime = QDateTime::currentDateTime();
    auto ms = currentDateTime.toMSecsSinceEpoch();
    return static_cast<uint32_t>(ms);
}

void trixterxdreamv1bike::timerEvent(QTimerEvent *event) {
    int timerId = event->timerId();

    if(timerId==this->resistanceTimerId){
        event->accept();
        this->updateResistance();
    } else if(timerId==this->settingsUpdateTimerId) {
        event->accept();
        this->appSettings->Load();
    }
}

bool trixterxdreamv1bike::updateClient(const QByteArray& bytes, trixterxdreamv1client * client) {
    bool stateChanged = false;

    for(int i=0; i<bytes.length();i++)
        stateChanged |= client->ReceiveChar(bytes[i]);

    return stateChanged;
}

void trixterxdreamv1bike::update(const QByteArray &bytes) {
    // send the bytes to the client and return if there's no change of state
    if(!updateClient(bytes, &this->client))
        return;

    // Take the most recent state read
    auto state = this->client.getLastState();
    auto currentTime = getTime();

    // update the packet count
    this->packetsProcessed++;
    this->lastPacketProcessedTime = currentTime;

    // update the metrics
    this->LastCrankEventTime = state.LastEventTime;

    // set the speed in km/h
    constexpr double minutesPerHour = 60.0;
    this->Speed.setValue(state.FlywheelRPM * minutesPerHour * this->wheelCircumference);

    // set the distance in km
    this->Distance.setValue(state.CumulativeWheelRevolutions * this->wheelCircumference);

    // set the cadence in revolutions per minute
    this->Cadence.setValue(state.CrankRPM);

    // set the crank revolutions
    this->CrankRevs = state.CumulativeCrankRevolutions;

    // check if the settings have been updated and adjust accordingly
    if(this->appSettings->get_version()!=this->lastAppSettingsVersion) {

        this->noHeartRate = this->noHeartService || !this->appSettings->get_heartRateEnabled();
        if(this->noHeartRate)
            this->Heart.setValue(0.0);

        this->noSteering = !this->appSettings->get_steeringEnabled();
        if(this->noSteering)
            this->m_steeringAngle.setValue(0.0);
        else
            this->calculateSteeringMap();

        this->lastAppSettingsVersion = this->appSettings->get_version();
    }

    // update the heart rate
    if(!this->noHeartRate)
        this->Heart.setValue(state.HeartRate);

    // Set the steering
    if(!this->noSteering) {
        this->m_steeringAngle.setValue(this->steeringMap[state.Steering]);
    }

    // set the elapsed time
    this->elapsed = (currentTime - this->t0) * 0.001;
}

void trixterxdreamv1bike::calculateSteeringMap() {

    constexpr double maxSteeringAngle = 45.0;

    this->steeringMap.clear();

    int halfDeadZone = this->appSettings->get_steeringDeadZoneWidth()/2;
    int deadZoneLeft = this->appSettings->get_steeringCenter()-halfDeadZone;
    int deadZoneRight = this->appSettings->get_steeringCenter()+halfDeadZone;
    double sensitivityLeft = 0.01 * this->appSettings->get_steeringSensitivityLeft();
    double sensitivityRight = 0.01 * this->appSettings->get_steeringSensitivityRight();
    double scaleLeft = sensitivityLeft * maxSteeringAngle / deadZoneLeft;
    double scaleRight = sensitivityRight * maxSteeringAngle / (trixterxdreamv1client::MaxSteering - deadZoneRight);

    for(int i=0; i<=trixterxdreamv1client::MaxSteering; i++) {
        double mappedValue;
        if(i>=deadZoneLeft && i<=deadZoneRight) {
            mappedValue = 0.0;
        } else if (i<deadZoneLeft) {
            mappedValue = -std::max(0.0, std::min(maxSteeringAngle, scaleLeft*(deadZoneLeft-i)));
        } else {
            mappedValue = std::max(0.0, std::min(maxSteeringAngle, scaleRight * (i-deadZoneRight)));
        }
        this->steeringMap.push_back(mappedValue);
    }

}

void trixterxdreamv1bike::changeResistance(int8_t resistanceLevel) {
    // ignore the resistance if this option was selected
    if(this->noWriteResistance)
        return;

    // Clip the incoming values
    if(resistanceLevel<0) resistanceLevel = 0;
    if(resistanceLevel>maxResistance()) resistanceLevel = maxResistance();

    bike::changeResistance(resistanceLevel);

    // store the new resistance level. This might be the same as lastRequestedResistance(),Value
    // but it doesn't involve a function call and a cast to get the value.
    this->resistanceLevel = resistanceLevel;

    // store the resistance level as a metric for the UI
    this->Resistance.setValue(resistanceLevel);
}

void trixterxdreamv1bike::updateResistance() {
    this->client.SendResistance(this->resistanceLevel);
}

trixterxdreamv1bike::~trixterxdreamv1bike() {
    if(this->port) delete this->port;
    if(this->appSettings) delete this->appSettings;
}

void trixterxdreamv1bike::set_wheelDiameter(double value) {
    // clip the value
    value = std::min(MaxWheelDiameter, std::max(value, MinWheelDiameter));

    // stored as km to avoid dividing by 1000 every time it's used
    this->wheelCircumference = value * M_PI / 1000.0;
}

trixterxdreamv1bike * trixterxdreamv1bike::tryCreate(bool noWriteResistance, bool noHeartService, bool noVirtualDevice, const QString &portName) {
    // first check if there's a port specified
    if(portName!=nullptr && !portName.isEmpty())
    {
        qDebug() << "Looking for Trixter X-Dream V1 device on port: " << portName;
        trixterxdreamv1bike * result = new trixterxdreamv1bike(noWriteResistance, noHeartService, noVirtualDevice);
        try {
            if(result->connect(portName)) {
                qDebug() << "Found Trixter X-Dream V1 device on port: " << portName;
                return result;
            }
            delete result;
        } catch(...) {
            qDebug() << "Error thrown looking for Trixter X-Dream V1 device on port: " << portName;

            // make absolutely sure the object is delete otherwise the serial port it opened will remain blocked.
            if(result) {
                qDebug() << "Deleting object that was not able to connect";
                delete result;
            }
            throw;
        }
        qDebug() << "No Trixter X-Dream V1 device found on port: " << portName;
        return nullptr;
    }

    // Find the available ports and return the first success
    auto availablePorts = trixterxdreamv1serial::availablePorts();

    for(int i=0; i<availablePorts.length(); i++)
    {
        auto port = availablePorts[i];

#if defined(Q_OS_LINUX)
        if(!port.portName().startsWith("ttyUSB"))
        {
            qDebug() << "Skipping port: " << port.portName() << " because it doesn't start with ttyUSB";
            continue;
        }
#endif

        qDebug() << "Found portName:" << port.portName()
                 << "," << "description:" << port.description()
                 << "," << "vender identifier:" << port.vendorIdentifier()
                 << "," << "manufacturer:" << port.manufacturer()
                 << "," << "product identifier:" << port.productIdentifier()
                 << "," << "isBusy:" << port.isBusy()
                 << "," << "isNull:" << port.isNull()
                 << "," << "serialNumber:" << port.serialNumber();

        trixterxdreamv1bike * result = tryCreate(noWriteResistance, noHeartService, noVirtualDevice, port.portName());
        if(result)
            return result;
    }

    return nullptr;
}

trixterxdreamv1bike * trixterxdreamv1bike::tryCreate(const QString& portName) {
    return tryCreate(false, false, false, portName);
}


