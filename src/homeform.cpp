#include "homeform.h"
#include <QQmlContext>
#include <QTime>

DataObject::DataObject(QString name, QString icon, QString value, bool writable)
{
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_writable = writable;
}

void DataObject::setValue(QString v) {m_value = v; emit valueChanged(m_value);}

homeform::homeform(QQmlApplicationEngine* engine, bluetooth* bl)
{       
    this->Bluetooth = bl;
    engine->rootContext()->setContextProperty("rootItem", (QObject *)this);

    dataList = {
        speed,
        inclination,
        cadence,
        elevation,
        calories,
        odometer,
        pace,
        resistance,
        watt,
        heart,
        fan
    };

    engine->rootContext()->setContextProperty("appModel", QVariant::fromValue(dataList));

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1000);
}

void homeform::update()
{
    if(Bluetooth->device())
    {
        double inclination = 0;
        double resistance = 0;
        double watts = 0;
        double pace = 0;

        speed->setValue(QString::number(Bluetooth->device()->currentSpeed(), 'f', 2));
        heart->setValue(QString::number(Bluetooth->device()->currentHeart()));
        odometer->setValue(QString::number(Bluetooth->device()->odometer(), 'f', 2));
        calories->setValue(QString::number(Bluetooth->device()->calories(), 'f', 0));
        fan->setValue(QString::number(Bluetooth->device()->fanSpeed()));

        if(Bluetooth->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            pace = 10000 / (((treadmill*)Bluetooth->device())->currentPace().second() + (((treadmill*)Bluetooth->device())->currentPace().minute() * 60));
            if(pace < 0) pace = 0;
            watts = ((treadmill*)Bluetooth->device())->watts(/*weight->text().toFloat()*/); // TODO: add weight to settings
            inclination = ((treadmill*)Bluetooth->device())->currentInclination();
            this->pace->setValue(((treadmill*)Bluetooth->device())->currentPace().toString("m:ss"));
            watt->setValue(QString::number(watts, 'f', 0));
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            elevation->setValue(QString::number(((treadmill*)Bluetooth->device())->elevationGain(), 'f', 1));
        }
        else if(Bluetooth->device()->deviceType() == bluetoothdevice::BIKE)
        {
            resistance = ((bike*)Bluetooth->device())->currentResistance();
            watts = ((bike*)Bluetooth->device())->watts();
            watt->setValue(QString::number(watts));
            this->resistance->setValue(QString::number(resistance));
        }
/*
        if(trainProgram)
        {
            trainProgramElapsedTime->setText(trainProgram->totalElapsedTime().toString("hh:mm:ss"));
            trainProgramCurrentRowElapsedTime->setText(trainProgram->currentRowElapsedTime().toString("hh:mm:ss"));
            trainProgramDuration->setText(trainProgram->duration().toString("hh:mm:ss"));

            double distance = trainProgram->totalDistance();
            if(distance > 0)
            {
                trainProgramTotalDistance->setText(QString::number(distance));
            }
            else
                trainProgramTotalDistance->setText("N/A");
        }
*/

        SessionLine s(
                      Bluetooth->device()->currentSpeed(),
                      inclination,
                      Bluetooth->device()->odometer(),
                      watts,
                      resistance,
                      Bluetooth->device()->currentHeart(),
                      pace);

        Session.append(s);
    }

    emit changeOfdevice();
    emit changeOfzwift();
}

bool homeform::getDevice()
{
    if(!this->Bluetooth->device())
        return false;
    return this->Bluetooth->device()->connected();
}

bool homeform::getZwift()
{
    if(!this->Bluetooth->device())
        return false;
    if(!this->Bluetooth->device()->VirtualDevice())
        return false;
    if(this->Bluetooth->device()->deviceType() == bluetoothdevice::TREADMILL &&
            ((virtualtreadmill*)((treadmill*)Bluetooth->device())->VirtualDevice())->connected())
    {
        return true;
    }
    else if(Bluetooth->device()->deviceType() == bluetoothdevice::BIKE &&
            ((virtualbike*)((bike*)Bluetooth->device())->VirtualDevice())->connected())
    {
        return true;
    }
    return false;
}
