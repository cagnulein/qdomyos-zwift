#include "homeform.h"
#include <QQmlContext>
#include <QTime>
#include <QSettings>
#include <QQmlFile>
#include "gpx.h"

DataObject::DataObject(QString name, QString icon, QString value, bool writable, QString id, int valueFontSize, int labelFontSize)
{
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_writable = writable;
    m_id = id;
    m_valueFontSize = valueFontSize;
    m_labelFontSize = labelFontSize;

    emit plusNameChanged(plusName());
    emit minusNameChanged(minusName());
}

void DataObject::setValue(QString v) {m_value = v; emit valueChanged(m_value);}
void DataObject::setValueFontSize(int value) {m_valueFontSize = value; emit valueFontSizeChanged(m_valueFontSize);}
void DataObject::setLabelFontSize(int value) {m_labelFontSize = value; emit labelFontSizeChanged(m_labelFontSize);}
void DataObject::setVisible(bool visible) {m_visible = visible; emit visibleChanged(m_visible);}

homeform::homeform(QQmlApplicationEngine* engine, bluetooth* bl)
{       
    QSettings settings;
    bool miles = settings.value("miles_unit", false).toBool();
    QString unit = "km";
    if(miles)
        unit = "mi";

#ifdef Q_OS_IOS
    const int labelFontSize = 14;
    const int valueElapsedFontSize = 36;
#elif defined Q_OS_ANDROID
    const int labelFontSize = 16;
    const int valueElapsedFontSize = 36;
#else
    const int labelFontSize = 10;
    const int valueElapsedFontSize = 30;
#endif
    speed = new DataObject("Speed (" + unit + "/h)", "icons/icons/speed.png", "0.0", true, "speed", 48, labelFontSize);
    inclination = new DataObject("Inclination (%)", "icons/icons/inclination.png", "0.0", true, "inclination", 48, labelFontSize);
    cadence = new DataObject("Cadence (bpm)", "icons/icons/cadence.png", "0", false, "cadence", 48, labelFontSize);
    elevation = new DataObject("Elev. Gain (m)", "icons/icons/elevationgain.png", "0", false, "elevation", 48, labelFontSize);
    calories = new DataObject("Calories (KCal)", "icons/icons/kcal.png", "0", false, "calories", 48, labelFontSize);
    odometer = new DataObject("Odometer (" + unit + ")", "icons/icons/odometer.png", "0.0", false, "odometer", 48, labelFontSize);
    pace = new DataObject("Pace (m/km)", "icons/icons/pace.png", "0:00", false, "pace", 48, labelFontSize);
    resistance = new DataObject("Resistance (%)", "icons/icons/resistance.png", "0", true, "resistance", 48, labelFontSize);
    watt = new DataObject("Watt", "icons/icons/watt.png", "0", false, "watt", 48, labelFontSize);
    heart = new DataObject("Heart (bpm)", "icons/icons/heart_red.png", "0", false, "heart", 48, labelFontSize);
    fan = new DataObject("Fan Speed", "icons/icons/fan.png", "0", true, "fan", 48, labelFontSize);
    jouls = new DataObject("KJouls", "icons/icons/joul.png", "0", false, "joul", 48, labelFontSize);
    elapsed = new DataObject("Elapsed", "icons/icons/clock.png", "0:00:00", false, "elapsed", valueElapsedFontSize, labelFontSize);

    this->bluetoothManager = bl;
    this->engine = engine;
    connect(bluetoothManager, SIGNAL(deviceFound(QString)), this, SLOT(deviceFound(QString)));
    connect(bluetoothManager, SIGNAL(deviceConnected()), this, SLOT(deviceConnected()));
    connect(bluetoothManager, SIGNAL(deviceConnected()), this, SLOT(trainProgramSignals()));
    engine->rootContext()->setContextProperty("rootItem", (QObject *)this);

    this->trainProgram = new trainprogram(QList<trainrow>(), bl);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1000);

    QObject *rootObject = engine->rootObjects().first();
    QObject *home = rootObject->findChild<QObject*>("home");
    QObject *stack = rootObject;
    QObject::connect(home, SIGNAL(start_clicked()),
        this, SLOT(Start()));
    QObject::connect(home, SIGNAL(stop_clicked()),
        this, SLOT(Stop()));
    QObject::connect(stack, SIGNAL(trainprogram_open_clicked(QUrl)),
        this, SLOT(trainprogram_open_clicked(QUrl)));
    QObject::connect(stack, SIGNAL(gpx_open_clicked(QUrl)),
        this, SLOT(gpx_open_clicked(QUrl)));
}

void homeform::trainProgramSignals()
{
     if(bluetoothManager->device())
     {
         disconnect(trainProgram, SIGNAL(start()), bluetoothManager->device(), SLOT(start()));
         disconnect(trainProgram, SIGNAL(stop()), bluetoothManager->device(), SLOT(stop()));
         disconnect(trainProgram, SIGNAL(changeSpeed(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeed(double)));
         disconnect(trainProgram, SIGNAL(changeInclination(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeInclination(double)));
         disconnect(trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeedAndInclination(double, double)));
         disconnect(trainProgram, SIGNAL(changeResistance(double)), ((bike*)bluetoothManager->device()), SLOT(changeResistance(double)));
         disconnect(((treadmill*)bluetoothManager->device()), SIGNAL(tapeStarted()), trainProgram, SLOT(onTapeStarted()));
         disconnect(((bike*)bluetoothManager->device()), SIGNAL(bikeStarted()), trainProgram, SLOT(onTapeStarted()));

         connect(trainProgram, SIGNAL(start()), bluetoothManager->device(), SLOT(start()));
         connect(trainProgram, SIGNAL(stop()), bluetoothManager->device(), SLOT(stop()));
         connect(trainProgram, SIGNAL(changeSpeed(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeed(double)));
         connect(trainProgram, SIGNAL(changeInclination(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeInclination(double)));
         connect(trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeedAndInclination(double, double)));
         connect(trainProgram, SIGNAL(changeResistance(double)), ((bike*)bluetoothManager->device()), SLOT(changeResistance(double)));
         connect(((treadmill*)bluetoothManager->device()), SIGNAL(tapeStarted()), trainProgram, SLOT(onTapeStarted()));
         connect(((bike*)bluetoothManager->device()), SIGNAL(bikeStarted()), trainProgram, SLOT(onTapeStarted()));

         qDebug() << "trainProgram associated to a device";
     }
     else
     {
         qDebug() << "trainProgram NOT associated to a device";
     }
}

void homeform::deviceConnected()
{
    if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
    {
        dataList = {
            speed,
            inclination,
            elevation,
            elapsed,
            calories,
            odometer,
            pace,
            watt,
            jouls,
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
            elapsed,
            calories,
            odometer,
            resistance,
            watt,
            jouls,
            heart,
            fan
        };

    }

    engine->rootContext()->setContextProperty("appModel", QVariant::fromValue(dataList));

    QObject *rootObject = engine->rootObjects().first();
    QObject *home = rootObject->findChild<QObject*>("home");
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
    trainProgram->restart();
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

        QSettings settings;
        bool miles = settings.value("miles_unit", false).toBool();
        double unit_conversion = 1.0;
        if(miles)
            unit_conversion = 0.621371;

        emit signalChanged(signal());

        speed->setValue(QString::number(bluetoothManager->device()->currentSpeed() * unit_conversion, 'f', 1));
        heart->setValue(QString::number(bluetoothManager->device()->currentHeart()));
        odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
        calories->setValue(QString::number(bluetoothManager->device()->calories(), 'f', 0));
        fan->setValue(QString::number(bluetoothManager->device()->fanSpeed()));
        jouls->setValue(QString::number(bluetoothManager->device()->jouls() / 1000.0, 'f', 1));
        elapsed->setValue(bluetoothManager->device()->elapsedTime().toString("h:mm:ss"));

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
            watts = ((treadmill*)bluetoothManager->device())->watts(settings.value("weight", 75.0).toFloat());
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
    static bool toggle = false;
    if(!this->bluetoothManager->device())
    {
        // toggling the bluetooth icon
        toggle = !toggle;
        return toggle;
    }
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

void homeform::trainprogram_open_clicked(QUrl fileName)
{
    qDebug() << "trainprogram_open_clicked" << fileName;
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();
    if(!file.fileName().isEmpty())
    {
        {
               if(trainProgram)
                     delete trainProgram;
                trainProgram = trainprogram::load(file.fileName(), bluetoothManager);
        }

        trainProgramSignals();
    }
}

void homeform::gpx_open_clicked(QUrl fileName)
{
    qDebug() << "gpx_open_clicked" << fileName;
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();
    if(!file.fileName().isEmpty())
    {
        {
               if(trainProgram)
                     delete trainProgram;
            gpx g;
            QList<trainrow> list;
            foreach(gpx_altitude_point_for_treadmill p, g.open(file.fileName()))
            {
                trainrow r;
                r.speed = p.speed;
                r.duration = QTime(0,0,0,0);
                r.duration = r.duration.addSecs(p.seconds);
                r.inclination = p.inclination;
                r.forcespeed = true;
                list.append(r);
            }
                trainProgram = new trainprogram(list, bluetoothManager);
        }

        trainProgramSignals();
    }
}
