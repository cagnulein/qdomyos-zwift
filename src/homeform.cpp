#include "homeform.h"
#include <QQmlContext>
#include <QTime>
#include <QSettings>
#include <QQmlFile>
#include <QStandardPaths>
#include "gpx.h"
#include "qfit.h"

DataObject::DataObject(QString name, QString icon, QString value, bool writable, QString id, int valueFontSize, int labelFontSize, QString valueFontColor, QString secondLine)
{
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_secondLine = secondLine;
    m_writable = writable;
    m_id = id;
    m_valueFontSize = valueFontSize;
    m_valueFontColor = valueFontColor;
    m_labelFontSize = labelFontSize;

    emit plusNameChanged(plusName());
    emit minusNameChanged(minusName());
}

void DataObject::setValue(QString v) {m_value = v; emit valueChanged(m_value);}
void DataObject::setSecondLine(QString value) {m_secondLine = value; emit secondLineChanged(m_secondLine);}
void DataObject::setValueFontSize(int value) {m_valueFontSize = value; emit valueFontSizeChanged(m_valueFontSize);}
void DataObject::setValueFontColor(QString value) {m_valueFontColor = value; emit valueFontColorChanged(m_valueFontColor);}
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
    peloton_resistance = new DataObject("Peloton Res. (%)", "icons/icons/resistance.png", "0", false, "peloton_resistance", 48, labelFontSize);
    watt = new DataObject("Watt", "icons/icons/watt.png", "0", false, "watt", 48, labelFontSize);
    avgWatt = new DataObject("AVG Watt", "icons/icons/watt.png", "0", false, "avgWatt", 48, labelFontSize);
    ftp = new DataObject("FTP Zone", "icons/icons/watt.png", "0", false, "ftp", 48, labelFontSize);
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
    QObject::connect(stack, SIGNAL(gpx_save_clicked()),
        this, SLOT(gpx_save_clicked()));
    QObject::connect(stack, SIGNAL(fit_save_clicked()),
        this, SLOT(fit_save_clicked()));
    QObject::connect(stack, SIGNAL(refresh_bluetooth_devices_clicked()),
        this, SLOT(refresh_bluetooth_devices_clicked()));
}

void homeform::refresh_bluetooth_devices_clicked()
{
    bluetoothManager->onlyDiscover = true;
    bluetoothManager->restart();
}

homeform::~homeform()
{
    gpx_save_clicked();
    fit_save_clicked();
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
    QSettings settings;

    if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
    {
        if(settings.value("tile_speed_enabled", true).toBool())
                dataList.append(speed);

        if(settings.value("tile_inclination_enabled", true).toBool())
            dataList.append(inclination);

        if(settings.value("tile_elevation_enabled", true).toBool())
            dataList.append(elevation);

        if(settings.value("tile_elapsed_enabled", true).toBool())
            dataList.append(elapsed);

        if(settings.value("tile_calories_enabled", true).toBool())
            dataList.append(calories);

        if(settings.value("tile_odometer_enabled", true).toBool())
            dataList.append(odometer);

        if(settings.value("tile_pace_enabled", true).toBool())
            dataList.append(pace);

        if(settings.value("tile_watt_enabled", true).toBool())
            dataList.append(watt);

        if(settings.value("tile_avgwatt_enabled", true).toBool())
            dataList.append(avgWatt);

        if(settings.value("tile_ftp_enabled", true).toBool())
            dataList.append(ftp);

        if(settings.value("tile_jouls_enabled", true).toBool())
            dataList.append(jouls);

        if(settings.value("tile_heart_enabled", true).toBool())
            dataList.append(heart);

        if(settings.value("tile_fan_enabled", true).toBool())
            dataList.append(fan);
    }
    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE || bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
    {
        if(settings.value("tile_speed_enabled", true).toBool())
            dataList.append(speed);

        if(settings.value("tile_cadence_enabled", true).toBool())
            dataList.append(cadence);

        if(settings.value("tile_elevation_enabled", true).toBool())
            dataList.append(elevation);

        if(settings.value("tile_elapsed_enabled", true).toBool())
            dataList.append(elapsed);

        if(settings.value("tile_calories_enabled", true).toBool())
            dataList.append(calories);

        if(settings.value("tile_odometer_enabled", true).toBool())
            dataList.append(odometer);

        if(settings.value("tile_resistance_enabled", true).toBool())
            dataList.append(resistance);

        if(settings.value("tile_peloton_resistance_enabled", true).toBool())
            dataList.append(peloton_resistance);        

        if(settings.value("tile_watt_enabled", true).toBool())
            dataList.append(watt);

        if(settings.value("tile_avgwatt_enabled", true).toBool())
            dataList.append(avgWatt);

        if(settings.value("tile_ftp_enabled", true).toBool())
            dataList.append(ftp);

        if(settings.value("tile_jouls_enabled", true).toBool())
            dataList.append(jouls);

        if(settings.value("tile_heart_enabled", true).toBool())
            dataList.append(heart);

        if(settings.value("tile_fan_enabled", true).toBool())
            dataList.append(fan);
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
    m_info = name + " found";
    emit infoChanged(m_info);    
    emit bluetoothDevicesChanged(bluetoothDevices());
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
            else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            {
                ((elliptical*)bluetoothManager->device())->changeResistance(((elliptical*)bluetoothManager->device())->currentResistance() + 1);
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
            else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            {
                ((elliptical*)bluetoothManager->device())->changeResistance(((elliptical*)bluetoothManager->device())->currentResistance() - 1);
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
        uint8_t cadence = 0;

        QSettings settings;
        bool miles = settings.value("miles_unit", false).toBool();
        double ftpSetting = settings.value("ftp", 200.0).toDouble();
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
        avgWatt->setValue(QString::number(bluetoothManager->device()->avgWatt(), 'f', 0));        

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
            cadence = ((bike*)bluetoothManager->device())->currentCadence();
            resistance = ((bike*)bluetoothManager->device())->currentResistance();
            watts = ((bike*)bluetoothManager->device())->watts();
            watt->setValue(QString::number(watts));
            this->peloton_resistance->setValue(QString::number(((bike*)bluetoothManager->device())->pelotonResistance(), 'f', 0));
            this->resistance->setValue(QString::number(resistance));
            this->cadence->setValue(QString::number(cadence));
        }
        else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
        {
            cadence = ((elliptical*)bluetoothManager->device())->currentCadence();
            resistance = ((elliptical*)bluetoothManager->device())->currentResistance();
            watts = ((elliptical*)bluetoothManager->device())->watts();
            watt->setValue(QString::number(watts));
            this->peloton_resistance->setValue(QString::number(((elliptical*)bluetoothManager->device())->pelotonResistance(), 'f', 0));
            this->resistance->setValue(QString::number(resistance));
            this->cadence->setValue(QString::number(cadence));
        }

        double ftpPerc = 0;
        double ftpZone = 1;
        if(ftpSetting > 0)
            ftpPerc = (watts / ftpSetting) * 100.0;
        if(ftpPerc < 55)
        {
            ftpZone = 1;
            ftp->setValueFontColor("white");
        }
        else if(ftpPerc < 76)
        {
            ftpZone = 2;
            ftp->setValueFontColor("limegreen");
        }
        else if(ftpPerc < 91)
        {
            ftpZone = 3;
            ftp->setValueFontColor("gold");
        }
        else if(ftpPerc < 106)
        {
            ftpZone = 4;
            ftp->setValueFontColor("orange");
        }
        else if(ftpPerc < 121)
        {
            ftpZone = 5;
            ftp->setValueFontColor("darkorange");
        }
        else if(ftpPerc < 151)
        {
            ftpZone = 6;
            ftp->setValueFontColor("orangered");
        }
        else
        {
            ftpZone = 7;
            ftp->setValueFontColor("red");
        }
        ftp->setValue("Z" + QString::number(ftpZone, 'f', 0));
        ftp->setSecondLine(QString::number(ftpPerc, 'f', 0) + "%");
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
                      pace, cadence, bluetoothManager->device()->calories(),
                      bluetoothManager->device()->elevationGain());

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
    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL &&
            ((virtualtreadmill*)((elliptical*)bluetoothManager->device())->VirtualDevice())->connected())
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

void homeform::gpx_save_clicked()
{
    QString path = "";
#if defined(Q_OS_ANDROID) || defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif

    if(bluetoothManager->device())
        gpx::save(path + QDateTime::currentDateTime().toString().replace(":", "_") + ".gpx", Session,  bluetoothManager->device()->deviceType());
}

void homeform::fit_save_clicked()
{
    QString path = "";
#if defined(Q_OS_ANDROID) || defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif

    if(bluetoothManager->device())
        qfit::save(path + QDateTime::currentDateTime().toString().replace(":", "_") + ".fit", Session, bluetoothManager->device()->deviceType());
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

QStringList homeform::bluetoothDevices()
{
    QStringList r;
    r.append("Disabled");
    foreach(QBluetoothDeviceInfo b, bluetoothManager->devices)
    {
        if(b.name().trimmed().length())
            r.append(b.name());
    }
    return r;
}
