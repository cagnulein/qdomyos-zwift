#include "trixterxdreamv1bike.h"
#include "trixterxdreamv1serial.h"
#include "trixterxdreamv1settings.h"
#include "qcoreevent.h"

#include <cmath>
#include <qmath.h>
#include <QSerialPortInfo>
#include <QTimer>

using namespace std;

double trixterxdreamv1bike::powerSurface[260][3] =
{
           {30, 0, 4.68},
           {30, 10, 5.77},
           {30, 20, 6.86},
           {30, 30, 7.95},
           {30, 40, 9.04},
           {30, 50, 10.13},
           {30, 60, 13.75},
           {30, 70, 17.38},
           {30, 80, 21},
           {30, 90, 24.63},
           {30, 100, 28.25},
           {30, 110, 32.42},
           {30, 120, 36.59},
           {30, 130, 40.77},
           {30, 140, 44.94},
           {30, 150, 49.11},
           {30, 160, 52.31},
           {30, 170, 55.5},
           {30, 180, 58.7},
           {30, 190, 61.89},
           {30, 200, 65.09},
           {30, 210, 66.87},
           {30, 220, 68.65},
           {30, 230, 70.42},
           {30, 240, 72.2},
           {30, 250, 73.98},
           {40, 0, 5.79},
           {40, 10, 6.96},
           {40, 20, 8.13},
           {40, 30, 9.29},
           {40, 40, 10.46},
           {40, 50, 11.63},
           {40, 60, 17.27},
           {40, 70, 22.91},
           {40, 80, 28.54},
           {40, 90, 34.18},
           {40, 100, 39.82},
           {40, 110, 47.69},
           {40, 120, 55.55},
           {40, 130, 63.42},
           {40, 140, 71.28},
           {40, 150, 79.15},
           {40, 160, 84.57},
           {40, 170, 89.99},
           {40, 180, 95.4},
           {40, 190, 100.82},
           {40, 200, 106.24},
           {40, 210, 109.37},
           {40, 220, 112.5},
           {40, 230, 115.62},
           {40, 240, 118.75},
           {40, 250, 121.88},
           {50, 0, 7.09},
           {50, 10, 8.73},
           {50, 20, 10.37},
           {50, 30, 12.01},
           {50, 40, 13.65},
           {50, 50, 15.29},
           {50, 60, 23.02},
           {50, 70, 30.76},
           {50, 80, 38.49},
           {50, 90, 46.23},
           {50, 100, 53.96},
           {50, 110, 65.73},
           {50, 120, 77.5},
           {50, 130, 89.28},
           {50, 140, 101.05},
           {50, 150, 112.82},
           {50, 160, 121.57},
           {50, 170, 130.32},
           {50, 180, 139.07},
           {50, 190, 147.82},
           {50, 200, 156.57},
           {50, 210, 161.46},
           {50, 220, 166.34},
           {50, 230, 171.23},
           {50, 240, 176.11},
           {50, 250, 181},
           {60, 0, 7.85},
           {60, 10, 9.64},
           {60, 20, 11.43},
           {60, 30, 13.21},
           {60, 40, 15},
           {60, 50, 16.79},
           {60, 60, 26.91},
           {60, 70, 37.04},
           {60, 80, 47.16},
           {60, 90, 57.29},
           {60, 100, 67.41},
           {60, 110, 83.71},
           {60, 120, 100.01},
           {60, 130, 116.32},
           {60, 140, 132.62},
           {60, 150, 148.92},
           {60, 160, 161.5},
           {60, 170, 174.09},
           {60, 180, 186.67},
           {60, 190, 199.26},
           {60, 200, 211.84},
           {60, 210, 219.79},
           {60, 220, 227.75},
           {60, 230, 235.7},
           {60, 240, 243.66},
           {60, 250, 251.61},
           {70, 0, 7.02},
           {70, 10, 9.56},
           {70, 20, 12.1},
           {70, 30, 14.64},
           {70, 40, 17.18},
           {70, 50, 19.72},
           {70, 60, 31.66},
           {70, 70, 43.6},
           {70, 80, 55.55},
           {70, 90, 67.49},
           {70, 100, 79.43},
           {70, 110, 99.66},
           {70, 120, 119.89},
           {70, 130, 140.13},
           {70, 140, 160.36},
           {70, 150, 180.59},
           {70, 160, 199.42},
           {70, 170, 218.25},
           {70, 180, 237.09},
           {70, 190, 255.92},
           {70, 200, 274.75},
           {70, 210, 285.29},
           {70, 220, 295.84},
           {70, 230, 306.38},
           {70, 240, 316.93},
           {70, 250, 327.47},
           {80, 0, 11.21},
           {80, 10, 14.45},
           {80, 20, 17.69},
           {80, 30, 20.94},
           {80, 40, 24.18},
           {80, 50, 27.42},
           {80, 60, 42.48},
           {80, 70, 57.54},
           {80, 80, 72.61},
           {80, 90, 87.67},
           {80, 100, 102.73},
           {80, 110, 128.44},
           {80, 120, 154.15},
           {80, 130, 179.85},
           {80, 140, 205.56},
           {80, 150, 231.27},
           {80, 160, 253.8},
           {80, 170, 276.33},
           {80, 180, 298.85},
           {80, 190, 321.38},
           {80, 200, 343.91},
           {80, 210, 359.11},
           {80, 220, 374.3},
           {80, 230, 389.5},
           {80, 240, 404.69},
           {80, 250, 419.89},
           {90, 0, 13.95},
           {90, 10, 17.98},
           {90, 20, 22.01},
           {90, 30, 26.03},
           {90, 40, 30.06},
           {90, 50, 34.09},
           {90, 60, 52.98},
           {90, 70, 71.87},
           {90, 80, 90.77},
           {90, 90, 109.66},
           {90, 100, 128.55},
           {90, 110, 159.29},
           {90, 120, 190.03},
           {90, 130, 220.76},
           {90, 140, 251.5},
           {90, 150, 282.24},
           {90, 160, 312.59},
           {90, 170, 342.94},
           {90, 180, 373.3},
           {90, 190, 403.65},
           {90, 200, 434},
           {90, 210, 449.53},
           {90, 220, 465.06},
           {90, 230, 480.6},
           {90, 240, 496.13},
           {90, 250, 511.66},
           {100, 0, 10.66},
           {100, 10, 14.86},
           {100, 20, 19.06},
           {100, 30, 23.27},
           {100, 40, 27.47},
           {100, 50, 31.67},
           {100, 60, 50.99},
           {100, 70, 70.32},
           {100, 80, 89.64},
           {100, 90, 108.97},
           {100, 100, 128.29},
           {100, 110, 162.32},
           {100, 120, 196.34},
           {100, 130, 230.37},
           {100, 140, 264.39},
           {100, 150, 298.42},
           {100, 160, 337.06},
           {100, 170, 375.71},
           {100, 180, 414.35},
           {100, 190, 453},
           {100, 200, 491.64},
           {100, 210, 512.57},
           {100, 220, 533.5},
           {100, 230, 554.44},
           {100, 240, 575.37},
           {100, 250, 596.3},
           {110, 0, 10.32},
           {110, 10, 15.17},
           {110, 20, 20.03},
           {110, 30, 24.88},
           {110, 40, 29.74},
           {110, 50, 34.59},
           {110, 60, 56.34},
           {110, 70, 78.09},
           {110, 80, 99.85},
           {110, 90, 121.6},
           {110, 100, 143.35},
           {110, 110, 182.15},
           {110, 120, 220.95},
           {110, 130, 259.76},
           {110, 140, 298.56},
           {110, 150, 337.36},
           {110, 160, 382.87},
           {110, 170, 428.38},
           {110, 180, 473.9},
           {110, 190, 519.41},
           {110, 200, 564.92},
           {110, 210, 591.47},
           {110, 220, 618.03},
           {110, 230, 644.58},
           {110, 240, 671.14},
           {110, 250, 697.69},
           {120, 0, 12.28},
           {120, 10, 18.05},
           {120, 20, 23.84},
           {120, 30, 29.61},
           {120, 40, 35.39},
           {120, 50, 41.16},
           {120, 60, 67.05},
           {120, 70, 92.93},
           {120, 80, 118.83},
           {120, 90, 144.71},
           {120, 100, 170.6},
           {120, 110, 216.77},
           {120, 120, 262.95},
           {120, 130, 309.14},
           {120, 140, 355.31},
           {120, 150, 401.49},
           {120, 160, 455.65},
           {120, 170, 509.81},
           {120, 180, 563.98},
           {120, 190, 618.14},
           {120, 200, 672.3},
           {120, 210, 703.9},
           {120, 220, 735.51},
           {120, 230, 767.1},
           {120, 240, 798.71},
           {120, 250, 830.31}};


trixterxdreamv1bike::trixterxdreamv1bike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice) {
    // Initialize metrics
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);

    this->useResistancePercentage = false;

    // Set the fake bluetooth device info
    this->bluetoothDevice =
            QBluetoothDeviceInfo(QBluetoothUuid {QStringLiteral("774f25bd-6636-4cdc-9398-839de026be1d")}, "Trixter X-Dream V1 Bike", 0);

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

    // wait for up to the configured connection timeout for some packets to arrive
    for(uint32_t start = getTime(), t=start, limit=start+this->appSettings->get_connectionTimeoutMilliseconds(); t<limit; t=getTime()) {
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
        qDebug() << "Killing resistance timer";
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

uint16_t trixterxdreamv1bike::powerFromResistanceRequest(resistance_t requestedResistance)
{
    requestedResistance = this->adjustedResistance(requestedResistance, true);
    return this->calculatePower((int)this->Cadence.value(), requestedResistance);
}

resistance_t trixterxdreamv1bike::resistanceFromPowerRequest(uint16_t power)
{
    int c = std::max(0, std::min(9, (int)(0.1*(this->Cadence.value()-30) +0.5)));

    int16_t result = -1;

    double * ps = powerSurface[c*26];
    for(int i=0; result<0 && i<26; i++, ps++)
        if(ps[2]>=power)
            result = ps[1];
    if(result<0)
        result = (ps-1)[1];

    result = this->adjustedResistance(result, false);
    result = std::min((int16_t)trixterxdreamv1client::MaxResistance, std::max((int16_t)0, result));
    return result;
}

double trixterxdreamv1bike::calculatePower(int cadenceRPM, int resistance) {
    if(cadenceRPM<30)
        return 0.0;

    int c = std::max(0, std::min(9, (int)(0.1*(cadenceRPM-30) +0.5)));
    int r = 0.1*std::max(0, std::min(250, resistance))+0.5;

    double *ps = powerSurface[c*26+r];

    if(ps[0]!=c*10+30 || ps[1]!=r*10)
    {
        qDebug() << "Unexpected (r,c) values: (" << r << "," << c << ")";
    }

    return ps[2];
}

resistance_t trixterxdreamv1bike::adjustedResistance(resistance_t input, bool toDevice) {
    if(this->useResistancePercentage){
        if(toDevice)
            return trixterxdreamv1client::MaxResistance * input / 100;
        else
            return 100 * input / trixterxdreamv1client::MaxResistance ;
    }
    return input;
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

    // Determine if the user is pressing the button to stop.
    this->stopping = (state.Buttons & trixterxdreamv1client::buttons::Red) != 0;

    constexpr double brakeScale = 125.0/(trixterxdreamv1client::MaxBrake-trixterxdreamv1client::MinBrake);
    uint8_t b1 = 125 - (state.Brake1 - trixterxdreamv1client::MinBrake) * brakeScale;
    uint8_t b2 = 125 - (state.Brake2 - trixterxdreamv1client::MinBrake) * brakeScale;
    this->brakeLevel = b1 + b2;

    // update the metrics
    this->LastCrankEventTime = state.LastEventTime;

    // set the speed in km/h
    constexpr double minutesPerHour = 60.0;
    this->Speed.setValue(state.FlywheelRPM * minutesPerHour * this->wheelCircumference);

    // set the distance in km
    this->Distance.setValue(state.CumulativeWheelRevolutions * this->wheelCircumference);

    // set the cadence in revolutions per minute
    this->Cadence.setValue(state.CrankRPM);

    // check if there's a request for a resistance level
    if(this->requestResistance!=-1) {
        qDebug() << "requestResistance=" << this->requestResistance;
        this->changeResistance(this->requestResistance);
        this->requestResistance = -1;
    }

    // update the power output
    this->update_metrics(true, this->brakeLevel + this->brakeLevel + this->calculatePower(state.CrankRPM, this->resistanceLevel));

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
    bool steeringAngleChanged = false;
    if(!this->noSteering) {
        double newValue = this->steeringMap[state.Steering];
        steeringAngleChanged = this->m_steeringAngle.value()!=newValue;
        if(steeringAngleChanged)
            this->m_steeringAngle.setValue(newValue);
    }

    // set the elapsed time
    this->elapsed = (currentTime - this->t0) * 0.001;

    locker.unlock();

    if(steeringAngleChanged)
        emit this->steeringAngleChanged(this->m_steeringAngle.value());
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

void trixterxdreamv1bike::changeResistance(resistance_t resistanceLevel) {
    // ignore the resistance if this option was selected
    if(this->noWriteResistance)
        return;

    QMutexLocker locker(&this->updateMutex);

    // Clip the incoming values
    if(resistanceLevel<0) resistanceLevel = 0;
    if(resistanceLevel>maxResistance()) resistanceLevel = maxResistance();

    bike::changeResistance(resistanceLevel);

    // store the resistance level as a metric for the UI
    constexpr double pelotonScaleFactor = 100.0 / trixterxdreamv1client::MaxResistance;

    this->Resistance.setValue(resistanceLevel);
    if(this->useResistancePercentage)
        this->m_pelotonResistance.setValue(resistanceLevel);
    else
        this->m_pelotonResistance.setValue(round(pelotonScaleFactor * resistanceLevel));

    // store the new resistance level. This might be the same as lastRequestedResistance(),Value
    // but it doesn't involve a function call and a cast to get the value.
    this->resistanceLevel = this->adjustedResistance(resistanceLevel, true);
}

void trixterxdreamv1bike::updateResistance() {
    QMutexLocker locker(&this->updateMutex);
    resistance_t actualResistance = this->stopping ? (trixterxdreamv1client::MaxResistance): this->resistanceLevel;
    this->client.SendResistance(actualResistance);
}

trixterxdreamv1bike::~trixterxdreamv1bike() {
    if(this->port) delete this->port;
    if(this->appSettings) delete this->appSettings;

    // NOTE: bluetooth::restart() deletes this object, then deletes the bike object
    //if(this->virtualBike) delete this->virtualBike;
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


resistance_t trixterxdreamv1bike::pelotonToBikeResistance(int pelotonResistance) {
    pelotonResistance = std::max(0, std::min(100, pelotonResistance));

    if(this->useResistancePercentage)
        return pelotonResistance;

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


