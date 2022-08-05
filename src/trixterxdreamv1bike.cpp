#include "trixterxdreamv1bike.h"
#include "qcoreevent.h"
#include "trixterxdreamv1serial.h"
#include <cmath>
#include <qmath.h>
#include <QSerialPortInfo>
#include <QTimer>

using namespace std;


class trixterxdreamv1bike::serialPortMonitor : public trixterxdreamv1serial
{
    protected:
    trixterxdreamv1bike * bike = nullptr;

    void receive(const QByteArray &bytes) override
    {
        this->bike->update(bytes);
    }

public:
    explicit serialPortMonitor(trixterxdreamv1bike * bike) {
        this->bike = bike;
    }
};


trixterxdreamv1bike::trixterxdreamv1bike(bool noWriteResistance, bool noHeartService, bool noVirtualDevice, bool noSteering)
{
    // Set the wheel diameter for speed and distance calculations
    this->set_wheelDiameter(DefaultWheelDiameter);

    // QZ things from expected constructor
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->noVirtualDevice = noVirtualDevice;
    this->noSteering = noSteering;
}

bool trixterxdreamv1bike::connect(QString portName)
{
    if(this->port) delete this->port;

    // Get the current time in milliseconds since ancient times.
    // This will be subtracted from further readings from getTime() to get an easier to look at number.
    this->t0 = this->getTime();

    // create the port object and connect it
    this->port = new serialPortMonitor(this);

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

    return this->connected();
}

bool trixterxdreamv1bike::connected()
{
    return (this->getTime()-this->lastPacketProcessedTime) < DisconnectionTimeout;
}


uint32_t trixterxdreamv1bike::getTime()
{
    auto currentDateTime = QDateTime::currentDateTime();
    auto ms = currentDateTime.toMSecsSinceEpoch();
    return static_cast<uint32_t>(ms);
}

void trixterxdreamv1bike::timerEvent(QTimerEvent *event)
{
    if(event->timerId()==this->resistanceTimerId)
    {
        event->accept();
        this->updateResistance();
    }
}

bool trixterxdreamv1bike::updateClient(const QByteArray& bytes, trixterxdreamv1client * client)
{
    bool stateChanged = false;

    for(int i=0; i<bytes.length();i++)
        stateChanged |= client->ReceiveChar(bytes[i]);

    return stateChanged;
}

void trixterxdreamv1bike::update(const QByteArray &bytes)
{
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
    if(!this->noHeartService)
        this->Heart.setValue(state.HeartRate);

    // set the speed in km/h
    constexpr double minutesPerHour = 60.0;
    this->Speed.setValue(state.FlywheelRPM * minutesPerHour * this->wheelCircumference);

    // set the distance in km
    this->Distance.setValue(state.CumulativeWheelRevolutions * this->wheelCircumference);

    // set the cadence in revolutions per minute
    this->Cadence.setValue(state.CrankRPM);

    // set the crank revolutions
    this->CrankRevs = state.CumulativeCrankRevolutions;

    // Set the steering
    constexpr double steeringScale = 90.0 / trixterxdreamv1client::MaxSteering;
    if(!this->noSteering)
        this->m_steeringAngle.setValue(round(steeringScale * state.Steering - 45.0));

    // set the elapsed time
    this->elapsed = (currentTime - this->t0) * 0.001;
}

void trixterxdreamv1bike::changeResistance(int8_t resistanceLevel)
{
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

void trixterxdreamv1bike::updateResistance()
{
    this->client.SendResistance(this->resistanceLevel);
}

trixterxdreamv1bike::~trixterxdreamv1bike()
{

}

void trixterxdreamv1bike::set_wheelDiameter(double value)
{
    // clip the value
    value = std::min(MaxWheelDiameter, std::max(value, MinWheelDiameter));

    // stored as km to avoid dividing by 1000 every time it's used
    this->wheelCircumference = value * M_PI / 1000.0;
}

trixterxdreamv1bike * trixterxdreamv1bike::tryCreate(bool noWriteResistance, bool noHeartService, bool noVirtualDevice, bool noSteering, const QString &portName)
{
    // first check if there's a port specified
    if(portName!=nullptr && !portName.isEmpty())
    {
        trixterxdreamv1bike * result = new trixterxdreamv1bike(noWriteResistance, noHeartService, noVirtualDevice, noSteering);
        if(result->connect(portName))
            return result;
        delete result;
        return nullptr;
    }

    // Find the available ports and return the first success
    auto availablePorts = trixterxdreamv1serial::availablePorts();

    for(int i=0; i<availablePorts.length(); i++)
    {
        trixterxdreamv1bike * result = tryCreate(noWriteResistance, noHeartService, noVirtualDevice, noSteering, availablePorts[i].portName());
        if(result) return result;
        delete result;
    }

    return nullptr;
}

trixterxdreamv1bike * trixterxdreamv1bike::tryCreate(const QString& portName)
{
    return tryCreate(false, false, false, false, portName);
}


