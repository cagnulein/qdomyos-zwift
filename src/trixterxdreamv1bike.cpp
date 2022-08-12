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
    this->t0 = getTime();

    // create the port object and connect it
    auto thisObject = this;
    this->port = new trixterxdreamv1serial(this);
    this->port->set_receiveBytes([thisObject](const QByteArray& bytes)->void{thisObject->update(bytes);});

    // References to objects for callbacks
    auto device=this->port;

    // tell the client where to get the time
    this->client.set_GetTime(getTime);

    // tell the client how to send data to the device
    if(!noWriteResistance)
        this->client.set_WriteBytes([device](uint8_t * bytes, int length)->void{ device->write(QByteArray((const char *)bytes, length), "");});

    // Set up a stopwatch to time the connection operations
    QElapsedTimer stopWatch;
    stopWatch.start();

    // open the port. This should be at 115200 bits per second.
    if(!this->port->open(portName, QSerialPort::Baud115200)) {
        qDebug() << "Failed to open port, determined after " << stopWatch.elapsed() << "milliseconds";
        return false;
    }

    // wait for up to 500ms for some packets to arrive
    for(uint32_t start = getTime(), t=start, limit=start+500; t<limit; t=getTime()) {
        if(this->connected()) {
            qDebug() << "Connected after " << stopWatch.elapsed() << "milliseconds";
            break;
        }
        QThread::msleep(20);
    }

    if(!this->connected())
    {
        qDebug() << "Failed to connect to device, after " << stopWatch.elapsed() << "milliseconds";
        this->disconnectPort();
        return false;
    }

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

    this->configureVirtualBike();

    return true;
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
        qDebug() << "Killing settings update timer";
        this->killTimer(this->settingsUpdateTimerId);
        this->settingsUpdateTimerId = 0;
    }
}

void trixterxdreamv1bike::configureVirtualBike(){
// ******************************************* virtual bike init *************************************

    bool haveVirtualBike = this->virtualBike!=nullptr;

    #ifdef Q_OS_IOS
    #ifndef IO_UNDER_QT
    haveVirtualBike &= !h;
    #endif
    #endif

    if(!haveVirtualBike){
        QSettings settings;
        bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();

    #ifdef Q_OS_IOS
    #ifndef IO_UNDER_QT
        bool cadence = settings.value("bike_cadence_sensor", false).toBool();
        bool ios_peloton_workaround = settings.value("ios_peloton_workaround", true).toBool();
        if (ios_peloton_workaround && cadence) {
            qDebug() << "ios_peloton_workaround activated!";
            h = new lockscreen();
            h->virtualbike_ios();
        } else
    #endif
    #endif
        if (virtual_device_enabled) {
            qDebug() << QStringLiteral("creating virtual bike interface...");
            this->virtualBike = new virtualbike(this, noWriteResistance, noHeartService);
        }
    }

// ********************************************************************************************************
}


bool trixterxdreamv1bike::connected() {
    QMutexLocker locker(&this->updateMutex);
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
    QMutexLocker locker(&this->updateMutex);

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
            QTimer::singleShot(10ms, this, &trixterxdreamv1bike::calculateSteeringMap);

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


    trixterxdreamv1settings::steeringCalibrationInfo info = this->appSettings->get_steeringCalibration();

    vector<double> newMap;

    // Map the calibration values from [-info.max,+info.max] to [0, 2*info.max]
    double mid = info.max, max = 2*mid;

    double l = mid+info.left;
    double cl = mid+info.centerLeft;
    double cr = mid+info.centerRight;
    double r = mid+info.right;

    double scale = max / trixterxdreamv1client::MaxSteering;
    double scaleLeft = mid / (cl-l);
    double scaleRight = mid / (r-cr);

    for(int i=0; i<=trixterxdreamv1client::MaxSteering; i++) {
        double mappedValue = i *scale;

        if(mappedValue>=cl && mappedValue<=cr) {
            mappedValue = mid;
        } else if (mappedValue<=l) {
            mappedValue = 0;
        } else if (mappedValue>=r) {
            mappedValue = max;
        } else if(mappedValue<cl) {
            mappedValue = mid-(cl-mappedValue) * scaleLeft;
        } else {
            mappedValue = mid+(mappedValue-cr) * scaleRight;
        }
        mappedValue = std::max(0.0, std::min(max, mappedValue));
        newMap.push_back(mappedValue-mid);
    }

    QMutexLocker locker(&this->updateMutex);
    this->steeringMap=newMap;

}

void trixterxdreamv1bike::changeResistance(int8_t resistanceLevel) {
    // ignore the resistance if this option was selected
    if(this->noWriteResistance)
        return;

    QMutexLocker locker(&this->updateMutex);

    // Clip the incoming values
    if(resistanceLevel<0) resistanceLevel = 0;
    if(resistanceLevel>maxResistance()) resistanceLevel = maxResistance();

    bike::changeResistance(resistanceLevel);

    // store the new resistance level. This might be the same as lastRequestedResistance(),Value
    // but it doesn't involve a function call and a cast to get the value.
    this->resistanceLevel = resistanceLevel;

    // store the resistance level as a metric for the UI
    constexpr double pelotonScaleFactor = 100.0 / trixterxdreamv1client::MaxResistance;
    this->Resistance.setValue(resistanceLevel);
    this->m_pelotonResistance.setValue(round(pelotonScaleFactor * resistanceLevel));

}

void trixterxdreamv1bike::updateResistance() {
    QMutexLocker locker(&this->updateMutex);
    this->client.SendResistance(this->resistanceLevel);
}

trixterxdreamv1bike::~trixterxdreamv1bike() {
    if(this->port) delete this->port;
    if(this->appSettings) delete this->appSettings;
    if(this->virtualBike) delete this->virtualBike;
}

void *trixterxdreamv1bike::VirtualDevice() {
    return this->virtualBike;
}

void trixterxdreamv1bike::set_wheelDiameter(double value) {
    QMutexLocker locker(&this->updateMutex);

    // clip the value
    value = std::min(MaxWheelDiameter, std::max(value, MinWheelDiameter));

    // stored as km to avoid dividing by 1000 every time it's used
    this->wheelCircumference = value * M_PI / 1000.0;
}


int trixterxdreamv1bike::pelotonToBikeResistance(int pelotonResistance) {
    pelotonResistance = std::max(0, std::min(100, pelotonResistance));
    return round(0.01*pelotonResistance*trixterxdreamv1client::MaxResistance);
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
                 << "," << "system location:" << port.systemLocation()
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


