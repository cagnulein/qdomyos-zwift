#include "homeform.h"
#include <QQmlContext>
#include <QTime>

DataObject::DataObject(QString name, QString icon, QString value, bool writable, QString id)
{
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_writable = writable;
    m_id = id;

    emit plusNameChanged(plusName());
    emit minusNameChanged(minusName());
}

void DataObject::setValue(QString v) {m_value = v; emit valueChanged(m_value);}
void DataObject::setVisible(bool visible) {m_visible = visible; emit visibleChanged(m_visible);}

homeform::homeform(QQmlApplicationEngine* engine, bluetooth* bl)
{       
    this->bluetoothManager = bl;
    this->engine = engine;
    connect(bluetoothManager, SIGNAL(deviceFound(QString)), this, SLOT(deviceFound(QString)));
    connect(bluetoothManager, SIGNAL(deviceConnected()), this, SLOT(deviceConnected()));
    engine->rootContext()->setContextProperty("rootItem", (QObject *)this);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1000);
}

void homeform::deviceConnected()
{
    if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
    {
        dataList = {
            speed,
            inclination,
            elevation,
            calories,
            odometer,
            pace,
            watt,
            heart,
            fan
        };

    }
    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
    {
        dataList = {
            speed,
            cadence,
            elevation,
            calories,
            odometer,
            resistance,
            watt,
            heart,
            fan
        };

    }

    engine->rootContext()->setContextProperty("appModel", QVariant::fromValue(dataList));

    QObject *rootObject = engine->rootObjects().first();
    QObject *home = rootObject->findChild<QObject*>("home");
    QObject::connect(home, SIGNAL(start_clicked()),
        this, SLOT(Start()));
    QObject::connect(home, SIGNAL(stop_clicked()),
        this, SLOT(Stop()));
    QObject::connect(home, SIGNAL(plus_clicked(QString)),
        this, SLOT(Plus(QString)));
    QObject::connect(home, SIGNAL(minus_clicked(QString)),
        this, SLOT(Minus(QString)));
}

void homeform::deviceFound(QString name)
{
    if(!name.trimmed().length()) return;
    m_info = name + " founded";
    emit infoChanged(m_info);
}

void homeform::Plus(QString name)
{
    if(name.contains("speed"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                ((treadmill*)bluetoothManager->device())->changeSpeed(((treadmill*)bluetoothManager->device())->currentSpeed() + 0.5);
            }
        }
    }
    else if(name.contains("inclination"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                ((treadmill*)bluetoothManager->device())->changeInclination(((treadmill*)bluetoothManager->device())->currentInclination() + 0.5);
            }
        }
    }
    else if(name.contains("resistance"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
            {
                ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance() + 1);
            }
        }
    }
    else if(name.contains("fan"))
    {
        if(bluetoothManager->device())
             bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 1);
    }
    else
    {
        qDebug() << name << "not handled";
    }
}

void homeform::Minus(QString name)
{
    if(name.contains("speed"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                ((treadmill*)bluetoothManager->device())->changeSpeed(((treadmill*)bluetoothManager->device())->currentSpeed() - 0.5);
            }
        }
    }
    else if(name.contains("inclination"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                ((treadmill*)bluetoothManager->device())->changeInclination(((treadmill*)bluetoothManager->device())->currentInclination() - 0.5);
            }
        }
    }
    else if(name.contains("resistance"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
            {
                ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance() - 1);
            }
        }
    }
    else if(name.contains("fan"))
    {
        if(bluetoothManager->device())
             bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 1);
    }
    else
    {
        qDebug() << name << "not handled";
    }
}

void homeform::Start()
{
    //trainProgram->restart();
    if(bluetoothManager->device())
        bluetoothManager->device()->start();
}

void homeform::Stop()
{
    if(bluetoothManager->device())
        bluetoothManager->device()->stop();
}

QString homeform::signal()
{
    if(!bluetoothManager)
        return "icons/icons/signal-1.png";

    if(!bluetoothManager->device())
        return "icons/icons/signal-1.png";

    int16_t rssi = bluetoothManager->device()->bluetoothDevice.rssi();
    if(rssi > -40)
        return "icons/icons/signal-3.png";
    else if(rssi > -60)
        return "icons/icons/signal-2.png";

    return "icons/icons/signal-1.png";
}

void homeform::update()
{
    if(bluetoothManager->device())
    {
        double inclination = 0;
        double resistance = 0;
        double watts = 0;
        double pace = 0;

        emit signalChanged(signal());

        speed->setValue(QString::number(bluetoothManager->device()->currentSpeed(), 'f', 2));
        heart->setValue(QString::number(bluetoothManager->device()->currentHeart()));
        odometer->setValue(QString::number(bluetoothManager->device()->odometer(), 'f', 2));
        calories->setValue(QString::number(bluetoothManager->device()->calories(), 'f', 0));
        fan->setValue(QString::number(bluetoothManager->device()->fanSpeed()));

        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            if(bluetoothManager->device()->currentSpeed())
            {
                pace = 10000 / (((treadmill*)bluetoothManager->device())->currentPace().second() + (((treadmill*)bluetoothManager->device())->currentPace().minute() * 60));
                if(pace < 0) pace = 0;
            }
            else
            {
                pace = 0;
            }
            watts = ((treadmill*)bluetoothManager->device())->watts(/*weight->text().toFloat()*/); // TODO: add weight to settings
            inclination = ((treadmill*)bluetoothManager->device())->currentInclination();
            this->pace->setValue(((treadmill*)bluetoothManager->device())->currentPace().toString("m:ss"));
            watt->setValue(QString::number(watts, 'f', 0));
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            elevation->setValue(QString::number(((treadmill*)bluetoothManager->device())->elevationGain(), 'f', 1));
        }
        else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
        {
            resistance = ((bike*)bluetoothManager->device())->currentResistance();
            watts = ((bike*)bluetoothManager->device())->watts();
            watt->setValue(QString::number(watts));
            this->resistance->setValue(QString::number(resistance));
            this->cadence->setValue(QString::number(((bike*)bluetoothManager->device())->currentCadence()));
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
                      bluetoothManager->device()->currentSpeed(),
                      inclination,
                      bluetoothManager->device()->odometer(),
                      watts,
                      resistance,
                      bluetoothManager->device()->currentHeart(),
                      pace);

        Session.append(s);
    }

    emit changeOfdevice();
    emit changeOfzwift();
}

bool homeform::getDevice()
{
    if(!this->bluetoothManager->device())
        return false;
    return this->bluetoothManager->device()->connected();
}

bool homeform::getZwift()
{
    if(!this->bluetoothManager->device())
        return false;
    if(!this->bluetoothManager->device()->VirtualDevice())
        return false;
    if(this->bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
            ((virtualtreadmill*)((treadmill*)bluetoothManager->device())->VirtualDevice())->connected())
    {
        return true;
    }
    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE &&
            ((virtualbike*)((bike*)bluetoothManager->device())->VirtualDevice())->connected())
    {
        return true;
    }
    return false;
}
