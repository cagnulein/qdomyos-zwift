#include "homeform.h"
#include <QQmlContext>
#include <QTime>
#include <QSettings>
#include <QQmlFile>
#include <QApplication>
#include <QStandardPaths>
#include <QAbstractOAuth2>
#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QDesktopServices>
#include <QJsonDocument>
#include <QUrlQuery>
#include <QHttpMultiPart>
#include <QFileInfo>
#include <QRandomGenerator>
#include "keepawakehelper.h"
#include "gpx.h"
#include "qfit.h"
#include "material.h"

#ifdef Q_OS_ANDROID
#include <QtAndroid>
#include <QAndroidJniEnvironment>
#endif

#ifndef IO_UNDER_QT
#include "secret.h"
#endif

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
    const int valueTimeFontSize = 26;
#elif defined Q_OS_ANDROID
    const int labelFontSize = 16;
    const int valueElapsedFontSize = 36;
    const int valueTimeFontSize = 26;
#else
    const int labelFontSize = 10;
    const int valueElapsedFontSize = 30;
    const int valueTimeFontSize = 22;
#endif
    speed = new DataObject("Speed (" + unit + "/h)", "icons/icons/speed.png", "0.0", true, "speed", 48, labelFontSize);
    inclination = new DataObject("Inclination (%)", "icons/icons/inclination.png", "0.0", true, "inclination", 48, labelFontSize);
    cadence = new DataObject("Cadence (rpm)", "icons/icons/cadence.png", "0", false, "cadence", 48, labelFontSize);
    elevation = new DataObject("Elev. Gain (m)", "icons/icons/elevationgain.png", "0", false, "elevation", 48, labelFontSize);
    calories = new DataObject("Calories (KCal)", "icons/icons/kcal.png", "0", false, "calories", 48, labelFontSize);
    odometer = new DataObject("Odometer (" + unit + ")", "icons/icons/odometer.png", "0.0", false, "odometer", 48, labelFontSize);
    pace = new DataObject("Pace (m/km)", "icons/icons/pace.png", "0:00", false, "pace", 48, labelFontSize);
    resistance = new DataObject("Resistance (%)", "icons/icons/resistance.png", "0", true, "resistance", 48, labelFontSize);
    peloton_resistance = new DataObject("Peloton R(%)", "icons/icons/resistance.png", "0", false, "peloton_resistance", 48, labelFontSize);
    target_resistance = new DataObject("Target R(%)", "icons/icons/resistance.png", "0", true, "target_resistance", 48, labelFontSize);
    target_peloton_resistance = new DataObject("T.Peloton R(%)", "icons/icons/resistance.png", "0", false, "target_peloton_resistance", 48, labelFontSize);
    target_cadence = new DataObject("T.Cadence(rpm)", "icons/icons/cadence.png", "0", false, "target_cadence", 48, labelFontSize);
    target_power = new DataObject("T.Power(W)", "icons/icons/watt.png", "0", false, "target_power", 48, labelFontSize);
    watt = new DataObject("Watt", "icons/icons/watt.png", "0", false, "watt", 48, labelFontSize);
    avgWatt = new DataObject("AVG Watt", "icons/icons/watt.png", "0", false, "avgWatt", 48, labelFontSize);
    ftp = new DataObject("FTP Zone", "icons/icons/watt.png", "0", false, "ftp", 48, labelFontSize);
    heart = new DataObject("Heart (bpm)", "icons/icons/heart_red.png", "0", false, "heart", 48, labelFontSize);
    fan = new DataObject("Fan Speed", "icons/icons/fan.png", "0", true, "fan", 48, labelFontSize);
    jouls = new DataObject("KJouls", "icons/icons/joul.png", "0", false, "joul", 48, labelFontSize);
    elapsed = new DataObject("Elapsed", "icons/icons/clock.png", "0:00:00", false, "elapsed", valueElapsedFontSize, labelFontSize);
    moving_time = new DataObject("Moving T.", "icons/icons/clock.png", "0:00:00", false, "moving_time", valueElapsedFontSize, labelFontSize);
    datetime = new DataObject("Clock", "icons/icons/clock.png", QTime::currentTime().toString("hh:mm:ss"), false, "time", valueTimeFontSize, labelFontSize);
    lapElapsed = new DataObject("Lap Elapsed", "icons/icons/clock.png", "0:00:00", false, "lapElapsed", valueElapsedFontSize, labelFontSize);

    if(!settings.value("top_bar_enabled", true).toBool())
    {
        m_topBarHeight = 0;
        emit topBarHeightChanged(m_topBarHeight);
        m_info = "";
        emit infoChanged(m_info);
    }

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

    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &homeform::backup);
    backupTimer->start(60000);

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
    QObject::connect(stack, SIGNAL(strava_connect_clicked()),
        this, SLOT(strava_connect_clicked()));
    QObject::connect(stack, SIGNAL(refresh_bluetooth_devices_clicked()),
        this, SLOT(refresh_bluetooth_devices_clicked()));
    QObject::connect(home, SIGNAL(lap_clicked()),
        this, SLOT(Lap()));
    QObject::connect(home, SIGNAL(peloton_start_workout()),
        this, SLOT(peloton_start_workout()));


    if(settings.value("top_bar_enabled", true).toBool())
    {
        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
        emit stopColorChanged(stopColor());        
    }

    emit tile_orderChanged(tile_order());

    pelotonHandler = new peloton(bl);
    connect(pelotonHandler, SIGNAL(workoutStarted(QString, QString)), this, SLOT(pelotonWorkoutStarted(QString, QString)));
    connect(pelotonHandler, SIGNAL(workoutChanged(QString, QString)), this, SLOT(pelotonWorkoutChanged(QString, QString)));
    connect(pelotonHandler, SIGNAL(loginState(bool)), this, SLOT(pelotonLoginState(bool)));

    //populate the UI
#if 0
#warning("disable me!")
    {
        if(settings.value("tile_speed_enabled", true).toBool())
            dataList.append(speed);

        if(settings.value("tile_cadence_enabled", true).toBool())
            dataList.append(cadence);

        if(settings.value("tile_elevation_enabled", true).toBool())
            dataList.append(elevation);

        if(settings.value("tile_elapsed_enabled", true).toBool())
            dataList.append(elapsed);

        if(settings.value("tile_moving_time_enabled", true).toBool())
            dataList.append(moving_time);

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

    QObject::connect(home, SIGNAL(plus_clicked(QString)),
        this, SLOT(Plus(QString)));
    QObject::connect(home, SIGNAL(minus_clicked(QString)),
        this, SLOT(Minus(QString)));
#endif
}

void homeform::peloton_start_workout()
{
    qDebug() << "peloton_start_workout!";
    if(pelotonHandler && pelotonHandler->trainrows.length())
    {
        if(trainProgram)
        {
            trainProgram->stop();
            delete trainProgram;
            trainProgram = 0;
        }
        trainProgram = new trainprogram(pelotonHandler->trainrows, bluetoothManager);
        trainProgramSignals();
        trainProgram->restart();
    }
}

void homeform::pelotonLoginState(bool ok)
{
    m_pelotonLoginState = (ok?1:0);
    emit pelotonLoginChanged(m_pelotonLoginState);
}

void homeform::pelotonWorkoutStarted(QString name, QString instructor)
{
    m_pelotonAskStart = true;
    emit(changePelotonAskStart(pelotonAskStart()));
}

void homeform::pelotonWorkoutChanged(QString name, QString instructor)
{
    QSettings settings;

    stravaPelotonActivityName = name;
    stravaPelotonInstructorName = instructor;

    if(!settings.value("top_bar_enabled", true).toBool()) return;
    m_info = name;
    emit infoChanged(m_info);
}

void homeform::backup()
{
    static uint8_t index = 0;
    qDebug() << "saving fit file backup...";

    QString path = "";
#if defined(Q_OS_ANDROID)
    path = getAndroidDataAppDir() + "/";
#elif defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif

    if(bluetoothManager->device())
    {
        QString filename = path + QString::number(index) + backupFitFileName;
        QFile::remove(filename);
        qfit::save(filename, Session, bluetoothManager->device()->deviceType());

        index++;
        if(index > 1)
            index = 0;
    }
}

QString homeform::stopColor()
{
    return "#00000000";
}

QString homeform::startColor()
{
    static uint8_t startColorToggle = 0;
    if(paused || stopped)
    {
        if(startColorToggle)
        {
            startColorToggle = 0;
            return "red";
        }
        else
        {
            startColorToggle = 1;
            return "#00000000";
        }
    }
    return "#00000000";
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

void homeform::aboutToQuit()
{
    /*if(bluetoothManager->device())
        bluetoothManager->device()->disconnectBluetooth();*/
}

void homeform::trainProgramSignals()
{
     if(bluetoothManager->device())
     {
         disconnect(trainProgram, SIGNAL(start()), bluetoothManager->device(), SLOT(start()));
         disconnect(trainProgram, SIGNAL(stop()), bluetoothManager->device(), SLOT(stop()));
         disconnect(trainProgram, SIGNAL(changeSpeed(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeed(double)));
         disconnect(trainProgram, SIGNAL(changeInclination(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeInclination(double)));
         disconnect(trainProgram, SIGNAL(changeFanSpeed(uint8_t)), ((treadmill*)bluetoothManager->device()), SLOT(changeFanSpeed(uint8_t)));
         disconnect(trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeedAndInclination(double, double)));
         disconnect(trainProgram, SIGNAL(changeResistance(int8_t)), ((bike*)bluetoothManager->device()), SLOT(changeResistance(int8_t)));
         disconnect(trainProgram, SIGNAL(changeRequestedPelotonResistance(int8_t)), ((bike*)bluetoothManager->device()), SLOT(changeRequestedPelotonResistance(int8_t)));
         disconnect(trainProgram, SIGNAL(changeCadence(int16_t)), ((bike*)bluetoothManager->device()), SLOT(changeCadence(int16_t)));
         disconnect(trainProgram, SIGNAL(changePower(int32_t)), ((bike*)bluetoothManager->device()), SLOT(changePower(int32_t)));
         disconnect(((treadmill*)bluetoothManager->device()), SIGNAL(tapeStarted()), trainProgram, SLOT(onTapeStarted()));
         disconnect(((bike*)bluetoothManager->device()), SIGNAL(bikeStarted()), trainProgram, SLOT(onTapeStarted()));

         connect(trainProgram, SIGNAL(start()), bluetoothManager->device(), SLOT(start()));
         connect(trainProgram, SIGNAL(stop()), bluetoothManager->device(), SLOT(stop()));
         connect(trainProgram, SIGNAL(changeSpeed(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeed(double)));
         connect(trainProgram, SIGNAL(changeFanSpeed(uint8_t)), ((treadmill*)bluetoothManager->device()), SLOT(changeFanSpeed(uint8_t)));
         connect(trainProgram, SIGNAL(changeInclination(double)), ((treadmill*)bluetoothManager->device()), SLOT(changeInclination(double)));
         connect(trainProgram, SIGNAL(changeSpeedAndInclination(double, double)), ((treadmill*)bluetoothManager->device()), SLOT(changeSpeedAndInclination(double, double)));
         connect(trainProgram, SIGNAL(changeResistance(int8_t)), ((bike*)bluetoothManager->device()), SLOT(changeResistance(int8_t)));
         connect(trainProgram, SIGNAL(changeRequestedPelotonResistance(int8_t)), ((bike*)bluetoothManager->device()), SLOT(changeRequestedPelotonResistance(int8_t)));
         connect(trainProgram, SIGNAL(changeCadence(int16_t)), ((bike*)bluetoothManager->device()), SLOT(changeCadence(int16_t)));
         connect(trainProgram, SIGNAL(changePower(int32_t)), ((bike*)bluetoothManager->device()), SLOT(changePower(int32_t)));
         connect(((treadmill*)bluetoothManager->device()), SIGNAL(tapeStarted()), trainProgram, SLOT(onTapeStarted()));
         connect(((bike*)bluetoothManager->device()), SIGNAL(bikeStarted()), trainProgram, SLOT(onTapeStarted()));

         qDebug() << "trainProgram associated to a device";
     }
     else
     {
         qDebug() << "trainProgram NOT associated to a device";
     }
}

QStringList homeform::tile_order()
{
    QStringList r;
    for(int i = 0; i < 23; i++)
        r.append(QString::number(i));
    return r;
}

void homeform::deviceConnected()
{
    // if the device reconnects in the same session, the tiles shouldn't be created again
    static bool first = false;
    if(first) return;
    first = true;

    m_labelHelp = false;
    changeLabelHelp(m_labelHelp);

    QSettings settings;

    if(settings.value("pause_on_start", false).toBool() && bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL)
    {
        Start();
    }

    if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
    {
        for(int i=0; i<100; i++)
        {
            if(settings.value("tile_speed_enabled", true).toBool() && settings.value("tile_speed_order", 0).toInt() == i)
                dataList.append(speed);

            if(settings.value("tile_inclination_enabled", true).toBool() && settings.value("tile_inclination_order", 0).toInt() == i)
                dataList.append(inclination);

            if(settings.value("tile_elevation_enabled", true).toBool() && settings.value("tile_elevation_order", 0).toInt() == i)
                dataList.append(elevation);

            if(settings.value("tile_elapsed_enabled", true).toBool() && settings.value("tile_elapsed_order", 0).toInt() == i)
                dataList.append(elapsed);

            if(settings.value("tile_moving_time_enabled", false).toBool() && settings.value("tile_moving_time_order", 19).toInt() == i)
                dataList.append(moving_time);

            if(settings.value("tile_calories_enabled", true).toBool() && settings.value("tile_calories_order", 0).toInt() == i)
                dataList.append(calories);

            if(settings.value("tile_odometer_enabled", true).toBool() && settings.value("tile_odometer_order", 0).toInt() == i)
                dataList.append(odometer);

            if(settings.value("tile_pace_enabled", true).toBool() && settings.value("tile_pace_order", 0).toInt() == i)
                dataList.append(pace);

            if(settings.value("tile_watt_enabled", true).toBool() && settings.value("tile_watt_order", 0).toInt() == i)
                dataList.append(watt);

            if(settings.value("tile_avgwatt_enabled", true).toBool() && settings.value("tile_avgwatt_order", 0).toInt() == i)
                dataList.append(avgWatt);

            if(settings.value("tile_ftp_enabled", true).toBool() && settings.value("tile_ftp_order", 0).toInt() == i)
                dataList.append(ftp);

            if(settings.value("tile_jouls_enabled", true).toBool() && settings.value("tile_jouls_order", 0).toInt() == i)
                dataList.append(jouls);

            if(settings.value("tile_heart_enabled", true).toBool() && settings.value("tile_heart_order", 0).toInt() == i)
                dataList.append(heart);

            if(settings.value("tile_fan_enabled", true).toBool() && settings.value("tile_fan_order", 0).toInt() == i)
                dataList.append(fan);

            if(settings.value("tile_datetime_enabled", true).toBool() && settings.value("tile_datetime_order", 0).toInt() == i)
                dataList.append(datetime);

            if(settings.value("tile_lapelapsed_enabled", false).toBool() && settings.value("tile_lapelapsed_order", 18).toInt() == i)
                dataList.append(lapElapsed);
        }
    }
    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
    {
        for(int i=0; i<100; i++)
        {
            if(settings.value("tile_speed_enabled", true).toBool() && settings.value("tile_speed_order", 0).toInt() == i)
                dataList.append(speed);

            if(settings.value("tile_cadence_enabled", true).toBool() && settings.value("tile_cadence_order", 0).toInt() == i)
                dataList.append(cadence);

            if(settings.value("tile_elevation_enabled", true).toBool() && settings.value("tile_elevation_order", 0).toInt() == i)
                dataList.append(elevation);

            if(settings.value("tile_elapsed_enabled", true).toBool() && settings.value("tile_elapsed_order", 0).toInt() == i)
                dataList.append(elapsed);

            if(settings.value("tile_moving_time_enabled", false).toBool() && settings.value("tile_moving_time_order", 19).toInt() == i)
                dataList.append(moving_time);

            if(settings.value("tile_calories_enabled", true).toBool() && settings.value("tile_calories_order", 0).toInt() == i)
                dataList.append(calories);

            if(settings.value("tile_odometer_enabled", true).toBool() && settings.value("tile_odometer_order", 0).toInt() == i)
                dataList.append(odometer);

            if(settings.value("tile_resistance_enabled", true).toBool() && settings.value("tile_resistance_order", 0).toInt() == i)
                dataList.append(resistance);

            if(settings.value("tile_peloton_resistance_enabled", true).toBool() && settings.value("tile_peloton_resistance_order", 0).toInt() == i)
                dataList.append(peloton_resistance);

            if(settings.value("tile_watt_enabled", true).toBool() && settings.value("tile_watt_order", 0).toInt() == i)
                dataList.append(watt);

            if(settings.value("tile_avgwatt_enabled", true).toBool() && settings.value("tile_avgwatt_order", 0).toInt() == i)
                dataList.append(avgWatt);

            if(settings.value("tile_ftp_enabled", true).toBool() && settings.value("tile_ftp_order", 0).toInt() == i)
                dataList.append(ftp);

            if(settings.value("tile_jouls_enabled", true).toBool() && settings.value("tile_jouls_order", 0).toInt() == i)
                dataList.append(jouls);

            if(settings.value("tile_heart_enabled", true).toBool() && settings.value("tile_heart_order", 0).toInt() == i)
                dataList.append(heart);

            if(settings.value("tile_fan_enabled", true).toBool() && settings.value("tile_fan_order", 0).toInt() == i)
                dataList.append(fan);

            if(settings.value("tile_datetime_enabled", true).toBool() && settings.value("tile_datetime_order", 0).toInt() == i)
                dataList.append(datetime);

            if(settings.value("tile_target_resistance_enabled", true).toBool() && settings.value("tile_target_resistance_order", 0).toInt() == i)
                dataList.append(target_resistance);

            if(settings.value("tile_target_peloton_resistance_enabled", false).toBool() && settings.value("tile_target_peloton_resistance_order", 21).toInt() == i)
                dataList.append(target_peloton_resistance);

            if(settings.value("tile_target_cadence_enabled", false).toBool() && settings.value("tile_target_cadence_order", 19).toInt() == i)
                dataList.append(target_cadence);

            if(settings.value("tile_target_power_enabled", false).toBool() && settings.value("tile_target_power_order", 20).toInt() == i)
                dataList.append(target_power);

            if(settings.value("tile_lapelapsed_enabled", false).toBool() && settings.value("tile_lapelapsed_order", 18).toInt() == i)
                dataList.append(lapElapsed);
        }
    }
    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
    {
        for(int i=0; i<100; i++)
        {
            if(settings.value("tile_speed_enabled", true).toBool() && settings.value("tile_speed_order", 0).toInt() == i)
                dataList.append(speed);

            if(settings.value("tile_cadence_enabled", true).toBool() && settings.value("tile_cadence_order", 0).toInt() == i)
                dataList.append(cadence);

            if(settings.value("tile_inclination_enabled", true).toBool() && settings.value("tile_inclination_order", 0).toInt() == i)
                dataList.append(inclination);

            if(settings.value("tile_elevation_enabled", true).toBool() && settings.value("tile_elevation_order", 0).toInt() == i)
                dataList.append(elevation);

            if(settings.value("tile_elapsed_enabled", true).toBool() && settings.value("tile_elapsed_order", 0).toInt() == i)
                dataList.append(elapsed);

            if(settings.value("tile_moving_time_enabled", false).toBool() && settings.value("tile_moving_time_order", 19).toInt() == i)
                dataList.append(moving_time);

            if(settings.value("tile_calories_enabled", true).toBool() && settings.value("tile_calories_order", 0).toInt() == i)
                dataList.append(calories);

            if(settings.value("tile_odometer_enabled", true).toBool() && settings.value("tile_odometer_order", 0).toInt() == i)
                dataList.append(odometer);

            if(settings.value("tile_resistance_enabled", true).toBool() && settings.value("tile_resistance_order", 0).toInt() == i)
                dataList.append(resistance);

            if(settings.value("tile_peloton_resistance_enabled", true).toBool() && settings.value("tile_peloton_resistance_order", 0).toInt() == i)
                dataList.append(peloton_resistance);

            if(settings.value("tile_watt_enabled", true).toBool() && settings.value("tile_watt_order", 0).toInt() == i)
                dataList.append(watt);

            if(settings.value("tile_avgwatt_enabled", true).toBool() && settings.value("tile_avgwatt_order", 0).toInt() == i)
                dataList.append(avgWatt);

            if(settings.value("tile_ftp_enabled", true).toBool() && settings.value("tile_ftp_order", 0).toInt() == i)
                dataList.append(ftp);

            if(settings.value("tile_jouls_enabled", true).toBool() && settings.value("tile_jouls_order", 0).toInt() == i)
                dataList.append(jouls);

            if(settings.value("tile_heart_enabled", true).toBool() && settings.value("tile_heart_order", 0).toInt() == i)
                dataList.append(heart);

            if(settings.value("tile_fan_enabled", true).toBool() && settings.value("tile_fan_order", 0).toInt() == i)
                dataList.append(fan);

            if(settings.value("tile_datetime_enabled", true).toBool() && settings.value("tile_datetime_order", 0).toInt() == i)
                dataList.append(datetime);

            if(settings.value("tile_target_resistance_enabled", true).toBool() && settings.value("tile_target_resistance_order", 0).toInt() == i)
                dataList.append(target_resistance);

            if(settings.value("tile_lapelapsed_enabled", false).toBool() && settings.value("tile_lapelapsed_order", 18).toInt() == i)
                dataList.append(lapElapsed);
        }
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

    emit bluetoothDevicesChanged(bluetoothDevices());

    QSettings settings;
    if(!settings.value("top_bar_enabled", true).toBool()) return;    
    m_info = name + " found";
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
                ((treadmill*)bluetoothManager->device())->changeSpeed(((treadmill*)bluetoothManager->device())->currentSpeed().value() + 0.5);
            }
        }
    }
    else if(name.contains("inclination"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                ((treadmill*)bluetoothManager->device())->changeInclination(((treadmill*)bluetoothManager->device())->currentInclination().value() + 0.5);
            }
            else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            {
                ((elliptical*)bluetoothManager->device())->changeInclination(((elliptical*)bluetoothManager->device())->currentInclination().value() + 0.5);
            }
        }
    }
    else if(name.contains("target_resistance"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE || bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            {
                bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() + 0.03);
                if(bluetoothManager->device()->difficult() == 0)
                    bluetoothManager->device()->setDifficult(0.03);

                if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                {
                    ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance().value());
                }
                else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
                {
                    ((elliptical*)bluetoothManager->device())->changeResistance(((elliptical*)bluetoothManager->device())->currentResistance());
                }
            }
        }
    }
    else if(name.contains("resistance"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
            {
                ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance().value() + 1);
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
                ((treadmill*)bluetoothManager->device())->changeSpeed(((treadmill*)bluetoothManager->device())->currentSpeed().value() - 0.5);
            }
        }
    }
    else if(name.contains("inclination"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
            {
                ((treadmill*)bluetoothManager->device())->changeInclination(((treadmill*)bluetoothManager->device())->currentInclination().value() - 0.5);
            }
            else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            {
                ((elliptical*)bluetoothManager->device())->changeInclination(((elliptical*)bluetoothManager->device())->currentInclination().value() - 0.5);
            }
        }
    }
    else if(name.contains("target_resistance"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE || bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
            {
                bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() - 0.03);
                if(bluetoothManager->device()->difficult() == 0)
                    bluetoothManager->device()->setDifficult(-0.03);

                if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                {
                    ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance().value());
                }
                else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
                {
                    ((elliptical*)bluetoothManager->device())->changeResistance(((elliptical*)bluetoothManager->device())->currentResistance());
                }
            }
        }
    }
    else if(name.contains("resistance"))
    {
        if(bluetoothManager->device())
        {
            if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
            {
                ((bike*)bluetoothManager->device())->changeResistance(((bike*)bluetoothManager->device())->currentResistance().value() - 1);
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
    qDebug() << "Start pressed - paused" << paused << "stopped" << stopped;

    if(!paused && !stopped)
    {
        paused = true;
        if(bluetoothManager->device())
            bluetoothManager->device()->stop();
    }
    else
    {
        trainProgram->restart();
        if(bluetoothManager->device())
            bluetoothManager->device()->start();

        if(stopped)
        {
            if(bluetoothManager->device())
                bluetoothManager->device()->clearStats();
            Session.clear();
            chartImagesFilenames.clear();

            stravaPelotonActivityName = "";
            stravaPelotonInstructorName = "";
        }

        paused = false;
        stopped = false;
    }

    QSettings settings;
    if(settings.value("top_bar_enabled", true).toBool())
    {
        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit stopColorChanged(stopColor());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
    }

    if(bluetoothManager->device())
        bluetoothManager->device()->setPaused(paused | stopped);
}

void homeform::Stop()
{
    qDebug() << "Stop pressed - paused" << paused << "stopped" << stopped;

    if(bluetoothManager->device())
        bluetoothManager->device()->stop();

    paused = false;
    stopped = true;    

    fit_save_clicked();

    if(bluetoothManager->device())
        bluetoothManager->device()->setPaused(paused | stopped);

    QSettings settings;
    if(settings.value("top_bar_enabled", true).toBool())
    {
        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit stopColorChanged(stopColor());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
    }

    if(trainProgram)
        trainProgram->rows.clear();
}

void homeform::Lap()
{
    qDebug() << "lap pressed";
    if(bluetoothManager)
    {
        if(bluetoothManager->device())
        {
            bluetoothManager->device()->setLap();
            lapTrigger = true;
        }
    }
}

bool homeform::labelHelp()
{
    return m_labelHelp;
}

QString homeform::stopText()
{
    QSettings settings;
    if(settings.value("top_bar_enabled", true).toBool())
        return "Stop";
    return "";
}

QString homeform::stopIcon()
{
    return "icons/icons/stop.png";
}


QString homeform::startText()
{
    QSettings settings;
    if(settings.value("top_bar_enabled", true).toBool())
    {
        if(paused || stopped)
            return "Start";
        else
            return "Pause";
    }
    return "";
}

QString homeform::startIcon()
{
    QSettings settings;
    if(settings.value("top_bar_enabled", true).toBool())
    {
        if(paused || stopped)
            return "icons/icons/start.png";
        else
            return "icons/icons/pause.png";
    }
    return "";
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
    QSettings settings;
    uint8_t currentHRZone = 1;

    if((paused || stopped) && settings.value("top_bar_enabled", true).toBool())
    {
        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
        emit stopColorChanged(stopColor());
    }

    if(bluetoothManager->device())
    {
        double inclination = 0;
        double resistance = 0;
        double watts = 0;
        double pace = 0;
        double peloton_resistance = 0;
        uint8_t cadence = 0;

        bool miles = settings.value("miles_unit", false).toBool();
        double ftpSetting = settings.value("ftp", 200.0).toDouble();
        double unit_conversion = 1.0;
        if(miles)
            unit_conversion = 0.621371;

        emit signalChanged(signal());

        speed->setValue(QString::number(bluetoothManager->device()->currentSpeed().value() * unit_conversion, 'f', 1));
        speed->setSecondLine("AVG: " + QString::number((bluetoothManager->device())->currentSpeed().average() * unit_conversion, 'f', 1) + " MAX: " + QString::number((bluetoothManager->device())->currentSpeed().max() * unit_conversion, 'f', 1));
        heart->setValue(QString::number(bluetoothManager->device()->currentHeart().value(), 'f', 0));        
        odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
        calories->setValue(QString::number(bluetoothManager->device()->calories(), 'f', 0));
        fan->setValue(QString::number(bluetoothManager->device()->fanSpeed()));
        jouls->setValue(QString::number(bluetoothManager->device()->jouls().value() / 1000.0, 'f', 1));
        elapsed->setValue(bluetoothManager->device()->elapsedTime().toString("h:mm:ss"));
        moving_time->setValue(bluetoothManager->device()->movingTime().toString("h:mm:ss"));
        lapElapsed->setValue(bluetoothManager->device()->lapElapsedTime().toString("h:mm:ss"));
        avgWatt->setValue(QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0));
        datetime->setValue(QTime::currentTime().toString("hh:mm:ss"));
        watts = bluetoothManager->device()->wattsMetric().value();
        watt->setValue(QString::number(watts));

        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            if(bluetoothManager->device()->currentSpeed().value())
            {
                pace = 10000 / (((treadmill*)bluetoothManager->device())->currentPace().second() + (((treadmill*)bluetoothManager->device())->currentPace().minute() * 60));
                if(pace < 0) pace = 0;
            }
            else
            {
                pace = 0;
            }
            inclination = ((treadmill*)bluetoothManager->device())->currentInclination().value();
            this->pace->setValue(((treadmill*)bluetoothManager->device())->currentPace().toString("m:ss"));
            this->pace->setSecondLine("AVG: " + ((treadmill*)bluetoothManager->device())->averagePace().toString("m:ss") + " MAX: " + ((treadmill*)bluetoothManager->device())->maxPace().toString("m:ss"));
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine("AVG: " + QString::number(((treadmill*)bluetoothManager->device())->currentInclination().average(), 'f', 1) + " MAX: " + QString::number(((treadmill*)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((treadmill*)bluetoothManager->device())->elevationGain(), 'f', 1));

            if(bluetoothManager->device()->currentSpeed().value() < 9)
            {
                speed->setValueFontColor("white");
                this->pace->setValueFontColor("white");
            }
            else if(bluetoothManager->device()->currentSpeed().value() < 10)
            {
                speed->setValueFontColor("limegreen");
                this->pace->setValueFontColor("limegreen");
            }
            else if(bluetoothManager->device()->currentSpeed().value() < 11)
            {
                speed->setValueFontColor("gold");
                this->pace->setValueFontColor("gold");
            }
            else if(bluetoothManager->device()->currentSpeed().value() < 12)
            {
                speed->setValueFontColor("orange");
                this->pace->setValueFontColor("orange");
            }
            else if(bluetoothManager->device()->currentSpeed().value() < 13)
            {
                speed->setValueFontColor("darkorange");
                this->pace->setValueFontColor("darkorange");
            }
            else if(bluetoothManager->device()->currentSpeed().value() < 14)
            {
                speed->setValueFontColor("orangered");
                this->pace->setValueFontColor("orangered");
            }
            else
            {
                speed->setValueFontColor("red");
                this->pace->setValueFontColor("red");
            }
        }
        else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
        {
            cadence = ((bike*)bluetoothManager->device())->currentCadence().value();
            resistance = ((bike*)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((bike*)bluetoothManager->device())->pelotonResistance().value();
            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(QString::number(((bike*)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(((bike*)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->target_cadence->setValue(QString::number(((bike*)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
            this->target_power->setValue(QString::number(((bike*)bluetoothManager->device())->lastRequestedPower().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance, 'f', 0));
            this->cadence->setValue(QString::number(cadence));

            this->cadence->setSecondLine("AVG: " + QString::number(((bike*)bluetoothManager->device())->currentCadence().average(), 'f', 0) + " MAX: " + QString::number(((bike*)bluetoothManager->device())->currentCadence().max(), 'f', 0));
            this->resistance->setSecondLine("AVG: " + QString::number(((bike*)bluetoothManager->device())->currentResistance().average(), 'f', 0) + " MAX: " + QString::number(((bike*)bluetoothManager->device())->currentResistance().max(), 'f', 0));
            this->peloton_resistance->setSecondLine("AVG: " + QString::number(((bike*)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) + " MAX: " + QString::number(((bike*)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(QString::number(bluetoothManager->device()->difficult() * 100.0,'f', 0) + "% @0%=" + QString::number((bluetoothManager->device()->difficult() * settings.value("bike_resistance_gain_f", 1.0).toDouble()) + settings.value("bike_resistance_offset", 4.0).toDouble(),'f', 0));
        }
        else if(bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
        {
            cadence = ((elliptical*)bluetoothManager->device())->currentCadence();
            resistance = ((elliptical*)bluetoothManager->device())->currentResistance();
            //this->peloton_resistance->setValue(QString::number(((elliptical*)bluetoothManager->device())->pelotonResistance(), 'f', 0));
            this->resistance->setValue(QString::number(resistance));
            this->cadence->setValue(QString::number(cadence));
            inclination = ((elliptical*)bluetoothManager->device())->currentInclination().value();
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine("AVG: " + QString::number(((elliptical*)bluetoothManager->device())->currentInclination().average(), 'f', 1) + " MAX: " + QString::number(((elliptical*)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((elliptical*)bluetoothManager->device())->elevationGain(), 'f', 1));
        }
        watt->setSecondLine("AVG: " + QString::number((bluetoothManager->device())->wattsMetric().average(), 'f', 0) + " MAX: " + QString::number((bluetoothManager->device())->wattsMetric().max(), 'f', 0));

        double ftpPerc = 0;
        double ftpZone = 1;
        QString ftpMinW = "0";
        QString ftpMaxW = "0";
        if(ftpSetting > 0)
            ftpPerc = (watts / ftpSetting) * 100.0;
        if(ftpPerc < 55)
        {
            ftpMinW = QString::number(0, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.54, 'f', 0);
            ftpZone = 1;
            ftp->setValueFontColor("white");
        }
        else if(ftpPerc < 76)
        {
            ftpMinW = QString::number(ftpSetting * 0.55, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.75, 'f', 0);
            ftpZone = 2;
            ftp->setValueFontColor("limegreen");
        }
        else if(ftpPerc < 91)
        {
            ftpMinW = QString::number(ftpSetting * 0.76, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.90, 'f', 0);
            ftpZone = 3;
            ftp->setValueFontColor("gold");
        }
        else if(ftpPerc < 106)
        {
            ftpMinW = QString::number(ftpSetting * 0.91, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.05, 'f', 0);
            ftpZone = 4;
            ftp->setValueFontColor("orange");
        }
        else if(ftpPerc < 121)
        {
            ftpMinW = QString::number(ftpSetting * 1.06, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.20, 'f', 0);
            ftpZone = 5;
            ftp->setValueFontColor("darkorange");
        }
        else if(ftpPerc < 151)
        {
            ftpMinW = QString::number(ftpSetting * 1.21, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.50, 'f', 0);
            ftpZone = 6;
            ftp->setValueFontColor("orangered");
        }
        else
        {
            ftpMinW = QString::number(ftpSetting * 1.51, 'f', 0);
            ftpMaxW = "∞";
            ftpZone = 7;
            ftp->setValueFontColor("red");
        }
        ftp->setValue("Z" + QString::number(ftpZone, 'f', 0));
        ftp->setSecondLine(ftpMinW + "-" + ftpMaxW + "W " + QString::number(ftpPerc, 'f', 0) + "%");

        QString Z;
        double maxHeartRate = 220.0 - settings.value("age", 35).toDouble();
        if(maxHeartRate == 0) maxHeartRate = 190.0;
        double percHeartRate = (bluetoothManager->device()->currentHeart().value() * 100) / maxHeartRate;

        if(percHeartRate < settings.value("heart_rate_zone1", 70.0).toDouble())
        {
            Z = "Z1";
            currentHRZone = 1;
            heart->setValueFontColor("lightsteelblue");
        }
        else if(percHeartRate < settings.value("heart_rate_zone2", 80.0).toDouble())
        {
            Z = "Z2";
            currentHRZone = 2;
            heart->setValueFontColor("green");
        }
        else if(percHeartRate < settings.value("heart_rate_zone3", 90.0).toDouble())
        {
            Z = "Z3";
            currentHRZone = 3;
            heart->setValueFontColor("yellow");
        }
        else if(percHeartRate < settings.value("heart_rate_zone4", 100.0).toDouble())
        {
            Z = "Z4";
            currentHRZone = 4;
            heart->setValueFontColor("orange");
        }
        else
        {
            Z = "Z5";
            currentHRZone = 5;
            heart->setValueFontColor("red");
        }
        heart->setSecondLine(Z + " AVG: " + QString::number((bluetoothManager->device())->currentHeart().average(), 'f', 0) + " MAX: " + QString::number((bluetoothManager->device())->currentHeart().max(), 'f', 0));

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

#ifdef Q_OS_ANDROID
        if(settings.value("ant_cadence", false).toBool() && KeepAwakeHelper::antObject(false))
        {
            KeepAwakeHelper::antObject(false)->callMethod<void>("setCadenceSpeedPower","(FII)V", (float)bluetoothManager->device()->currentSpeed().value(), (int)watts, (int)cadence);
        }
#endif

        if(settings.value("trainprogram_random", false).toBool())
        {
            if(!paused && !stopped)
            {
                static QRandomGenerator r;
                static uint32_t last_seconds = 0;
                uint32_t seconds = bluetoothManager->device()->elapsedTime().second() + (bluetoothManager->device()->elapsedTime().minute() * 60) + (bluetoothManager->device()->elapsedTime().hour() * 3600);
                if((seconds / 60) < settings.value("trainprogram_total", 60).toUInt())
                {
                    qDebug() << "trainprogram random seconds " + QString::number(seconds) + " last_change " + last_seconds + " period " + settings.value("trainprogram_period_seconds", 60).toUInt();
                    if(last_seconds == 0 || ((seconds - last_seconds) >= settings.value("trainprogram_period_seconds", 60).toUInt()))
                    {
                        bool done = false;

                        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL && ((treadmill*)bluetoothManager->device())->currentSpeed().value() > 0.0f)
                        {
                            double speed = settings.value("trainprogram_speed_min", 8).toUInt();
                            double incline = settings.value("trainprogram_incline_min", 0).toUInt();
                            if(!speed) speed = 1.0;
                            if(settings.value("trainprogram_speed_min", 8).toUInt() != 0 && settings.value("trainprogram_speed_min", 8).toUInt() < settings.value("trainprogram_speed_max", 16).toUInt())
                            {
                                speed = (double)r.bounded(settings.value("trainprogram_speed_min", 8).toUInt() * 10, settings.value("trainprogram_speed_max", 16).toUInt() * 10) / 10.0;
                            }
                            if(settings.value("trainprogram_incline_min", 0).toUInt() < settings.value("trainprogram_incline_max", 15).toUInt())
                            {
                                incline = (double)r.bounded(settings.value("trainprogram_incline_min", 0).toUInt() * 10, settings.value("trainprogram_incline_max", 15).toUInt() * 10) / 10.0;
                            }
                            ((treadmill*)bluetoothManager->device())->changeSpeedAndInclination(speed, incline);
                            done = true;
                        }
                        else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                        {
                            double resistance = settings.value("trainprogram_resistance_min", 1).toUInt();
                            if(settings.value("trainprogram_resistance_min", 1).toUInt() < settings.value("trainprogram_resistance_max", 32).toUInt())
                            {
                                resistance = (double)r.bounded(settings.value("trainprogram_resistance_min", 1).toUInt(), settings.value("trainprogram_resistance_max", 32).toUInt());
                            }
                            ((bike*)bluetoothManager->device())->changeResistance(resistance);

                            done = true;
                        }

                        if(done)
                        {
                            if(last_seconds == 0)
                            {
                                r.seed(QDateTime::currentDateTime().currentMSecsSinceEpoch());
                                last_seconds = 1; // in order to avoid to re-enter here again if the user doesn't ride
                            }
                            else
                                last_seconds = seconds;
                        }
                    }
                }
                else if(bluetoothManager->device()->currentSpeed().value() > 0)
                {
                    if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
                    {
                        ((treadmill*)bluetoothManager->device())->changeSpeedAndInclination(0,0);
                    }
                    else if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                    {
                        ((bike*)bluetoothManager->device())->changeResistance(1);
                    }
                }
            }
        }
        else if(!settings.value("treadmill_pid_heart_zone", "Disabled").toString().contains("Disabled") &&
                bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            static uint32_t last_seconds_pid_heart_zone = 0;
            uint32_t seconds = bluetoothManager->device()->elapsedTime().second() + (bluetoothManager->device()->elapsedTime().minute() * 60) + (bluetoothManager->device()->elapsedTime().hour() * 3600);

            if(last_seconds_pid_heart_zone == 0 || ((seconds - last_seconds_pid_heart_zone) >= 10))
            {
                last_seconds_pid_heart_zone = seconds;
                uint8_t zone = settings.value("treadmill_pid_heart_zone", "Disabled").toString().toUInt();
                if(!stopped && !paused &&
                        bluetoothManager->device()->currentHeart().value() &&
                        ((treadmill*)bluetoothManager->device())->currentSpeed().value() > 0.0f)
                {
                    if(zone < currentHRZone)
                    {
                        ((treadmill*)bluetoothManager->device())->changeSpeedAndInclination(
                                    ((treadmill*)bluetoothManager->device())->currentSpeed().value() - 0.2,
                                    ((treadmill*)bluetoothManager->device())->currentInclination().value());
                    }
                    else if(zone > currentHRZone)
                    {
                        ((treadmill*)bluetoothManager->device())->changeSpeedAndInclination(
                                    ((treadmill*)bluetoothManager->device())->currentSpeed().value() + 0.2,
                                    ((treadmill*)bluetoothManager->device())->currentInclination().value());
                    }
                }
            }
        }

        if(!stopped && !paused)
        {
            SessionLine s(
                        bluetoothManager->device()->currentSpeed().value(),
                        inclination,
                        bluetoothManager->device()->odometer(),
                        watts,
                        resistance,
                        peloton_resistance,
                        (uint8_t)bluetoothManager->device()->currentHeart().value(),
                        pace, cadence, bluetoothManager->device()->calories(),
                        bluetoothManager->device()->elevationGain(),
                        bluetoothManager->device()->elapsedTime().second() + (bluetoothManager->device()->elapsedTime().minute() * 60) + (bluetoothManager->device()->elapsedTime().hour() * 3600),
                        lapTrigger);

            Session.append(s);

            if(lapTrigger)
                lapTrigger = false;
        }
    }

    emit changeOfdevice();
    emit changeOflap();
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

bool homeform::getLap()
{
    if(!this->bluetoothManager->device())
        return false;
    return true;
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
#if defined(Q_OS_ANDROID)
    path = getAndroidDataAppDir() + "/";
#elif defined(Q_OS_MACOS) || defined(Q_OS_OSX)
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
#if defined(Q_OS_ANDROID)
    path = getAndroidDataAppDir() + "/";
#elif defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#endif

    if(bluetoothManager->device())
    {
        QString filename = path + QDateTime::currentDateTime().toString().replace(":", "_") + ".fit";
        qfit::save(filename, Session, bluetoothManager->device()->deviceType());
        lastFitFileSaved = filename;

        QSettings settings;
        if(settings.value("strava_accesstoken", "").toString().length())
        {
            QFile f(filename);
            f.open(QFile::OpenModeFlag::ReadOnly);
            QByteArray fitfile = f.readAll();
            strava_upload_file(fitfile,filename);
            f.close();
        }        
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

struct OAuth2Parameter
{
    QString responseType = "code";
    QString approval_prompt = "force";

    inline bool isEmpty() const{
        return responseType.isEmpty() && approval_prompt.isEmpty();
    }
    QString toString() const{
        QString msg;
        QTextStream out(&msg);
        out << "OAuth2Parameter{\n"
            << "responseType: " << this->responseType << "\n"
            << "approval_prompt: " << this->approval_prompt << "\n";
        return msg;
    }
};

QAbstractOAuth::ModifyParametersFunction homeform::buildModifyParametersFunction(QUrl clientIdentifier,QUrl clientIdentifierSharedKey){
    return [clientIdentifier,clientIdentifierSharedKey]
            (QAbstractOAuth::Stage stage, QVariantMap *parameters){
        if(stage == QAbstractOAuth::Stage::RequestingAuthorization){
            parameters->insert("responseType","code"); /* Request refresh token*/
            parameters->insert("approval_prompt","force"); /* force user check scope again */
            QByteArray code = parameters->value("code").toByteArray();
            (*parameters)["code"] = QUrl::fromPercentEncoding(code);
        }
        if(stage == QAbstractOAuth::Stage::RefreshingAccessToken){
            parameters->insert("client_id",clientIdentifier);
            parameters->insert("client_secret",clientIdentifierSharedKey);
        }
    };
}

#define STRAVA_CLIENT_ID "7976"

void homeform::strava_refreshtoken()
{
    QSettings settings;
    QUrlQuery params;

    if(!settings.value("strava_refreshtoken").toString().length())
    {
        strava_connect();
        return;
    }

    QNetworkRequest request(QUrl("https://www.strava.com/oauth/token?"));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // set params
    QString data;
    data += "client_id=" STRAVA_CLIENT_ID;
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
    data += "&client_secret=";
    data += STRINGIFY(STRAVA_SECRET_KEY);
#endif
    data += "&refresh_token=" + settings.value("strava_refreshtoken").toString();
    data += "&grant_type=refresh_token";

    // make request
    if(manager)
    {
        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    QNetworkReply* reply = manager->post(request, data.toLatin1());

    // blocking request
    QEventLoop loop;
    connect(reply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << "HTTP response code: " << statusCode;

    // oops, no dice
    if (reply->error() != 0) {
        qDebug() << "Got error" << reply->errorString().toStdString().c_str();
        return;
    }

    // lets extract the access token, and possibly a new refresh token
    QByteArray r = reply->readAll();
    qDebug() << "Got response:" << r.data();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(r, &parseError);

    // failed to parse result !?
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << tr("JSON parser error") << parseError.errorString();
    }

    QString access_token = document["access_token"].toString();
    QString refresh_token = document["refresh_token"].toString();

    settings.setValue("strava_accesstoken", access_token);
    settings.setValue("strava_refreshtoken", refresh_token);
    settings.setValue("strava_lastrefresh", QDateTime::currentDateTime());
}

bool homeform::strava_upload_file(QByteArray &data, QString remotename)
{
    strava_refreshtoken();

    QSettings settings;
    QString token = settings.value("strava_accesstoken").toString();

    // The V3 API doc said "https://api.strava.com" but it is not working yet
    QUrl url = QUrl( "https://www.strava.com/api/v3/uploads" );
    QNetworkRequest request = QNetworkRequest(url);

    //QString boundary = QString::number(qrand() * (90000000000) / (RAND_MAX + 1) + 10000000000, 16);
    QString boundary = QVariant(qrand()).toString() +
        QVariant(qrand()).toString() + QVariant(qrand()).toString();

    // MULTIPART *****************

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->setBoundary(boundary.toLatin1());

    QHttpPart accessTokenPart;
    accessTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                              QVariant("form-data; name=\"access_token\""));
    accessTokenPart.setBody(token.toLatin1());
    multiPart->append(accessTokenPart);

    QHttpPart activityTypePart;
    activityTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"activity_type\""));

    // Map some known sports and default to ride for anything else
    if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
      activityTypePart.setBody("run");
    else
      activityTypePart.setBody("ride");
    multiPart->append(activityTypePart);

    QHttpPart activityNamePart;
    activityNamePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"name\""));

    // use metadata config if the user selected it
    QString activityName = " " + settings.value("strava_suffix", "#qdomyos-zwift").toString() ;
    if(stravaPelotonActivityName.length())
    {
        activityName = stravaPelotonActivityName + " - " + stravaPelotonInstructorName + activityName;
    }
    else
    {
        if(bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
        {
            activityName = "Run" + activityName;
        }
        else
        {
            activityName = "Ride" + activityName;
        }
    }
    activityNamePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain;charset=utf-8"));
    activityNamePart.setBody(activityName.toUtf8());
    if (activityName != "")
        multiPart->append(activityNamePart);

    QHttpPart activityDescriptionPart;
    activityDescriptionPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"description\""));
    QString activityDescription = "";
    activityDescriptionPart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("text/plain;charset=utf-8"));
    activityDescriptionPart.setBody(activityDescription.toUtf8());
    if (activityDescription != "")
        multiPart->append(activityDescriptionPart);

    // upload file data
    QString filename = QFileInfo(remotename).baseName();

    QHttpPart dataTypePart;
    dataTypePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"data_type\""));
    dataTypePart.setBody("fit");
    multiPart->append(dataTypePart);

    QHttpPart externalIdPart;
    externalIdPart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"external_id\""));
    externalIdPart.setBody(filename.toStdString().c_str());
    multiPart->append(externalIdPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader, QVariant("form-data; name=\"file\"; filename=\""+remotename+"\"; type=\"application/octet-stream\""));
    filePart.setBody(data);
    multiPart->append(filePart);

    // this must be performed asyncronously and call made
    // to notifyWriteCompleted(QString remotename, QString message) when done
    if(manager)
    {
        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    replyStrava = manager->post(request, multiPart);

    // catch finished signal
    connect(replyStrava, SIGNAL(finished()), this, SLOT(writeFileCompleted()));
    connect(replyStrava, SIGNAL(errorOccurred(QNetworkReply::NetworkError)), this, SLOT(errorOccurredUploadStrava(QNetworkReply::NetworkError)));
    return true;
}

void homeform::errorOccurredUploadStrava(QNetworkReply::NetworkError code)
{
    qDebug() << "strava upload error!" << code;
}

void homeform::writeFileCompleted()
{
    qDebug() << "strava upload completed!";

    QNetworkReply *reply = static_cast<QNetworkReply*>(QObject::sender());

    QString response = reply->readAll();
    QString uploadError="invalid response or parser error";

    qDebug() << "reply:" << response;
}

void homeform::onStravaGranted()
{
    QSettings settings;
    settings.setValue("strava_accesstoken", strava->token());
    settings.setValue("strava_refreshtoken", strava->refreshToken());
    settings.setValue("strava_lastrefresh", QDateTime::currentDateTime());
    qDebug() << "strava authenticathed" << strava->token() << strava->refreshToken();
    strava_refreshtoken();
    setGeneralPopupVisible(true);
}

void homeform::onStravaAuthorizeWithBrowser(const QUrl &url)
{
    //ui->textBrowser->append(tr("Open with browser:") + url.toString());
    QDesktopServices::openUrl(url);
}

void homeform::replyDataReceived(QByteArray v)
{
    qDebug() << v;

    QByteArray data;
    QSettings settings;
    QString s(v);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(s.toUtf8());
    settings.setValue("strava_accesstoken", jsonResponse["access_token"]);
    settings.setValue("strava_refreshtoken", jsonResponse["refresh_token"]);
    settings.setValue("strava_expires", jsonResponse["expires_at"]);

    qDebug() << jsonResponse["access_token"] << jsonResponse["refresh_token"] << jsonResponse["expires_at"];

    QString urlstr = QString("https://www.strava.com/oauth/token?");
    QUrlQuery params;
    params.addQueryItem("client_id", STRAVA_CLIENT_ID);
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
    params.addQueryItem("client_secret", STRINGIFY(STRAVA_SECRET_KEY));
#endif

    params.addQueryItem("code", strava_code);
    data.append(params.query(QUrl::FullyEncoded));

    // trade-in the temporary access code retrieved by the Call-Back URL for the finale token
    QUrl url = QUrl(urlstr);

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader,"application/x-www-form-urlencoded");

    // now get the final token - but ignore errors
    if(manager)
    {
        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    //connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this, SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> & )));
    //connect(manager, SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestFinished(QNetworkReply*)));
    manager->post(request, data);
}

void homeform::onSslErrors(QNetworkReply *reply, const QList<QSslError>& error)
{
    reply->ignoreSslErrors();
    qDebug() << "homeform::onSslErrors" << error;
}

void homeform::networkRequestFinished(QNetworkReply *reply)
{
    QSettings settings;

    // we can handle SSL handshake errors, if we got here then some kind of protocol was agreed
    if (reply->error() == QNetworkReply::NoError || reply->error() == QNetworkReply::SslHandshakeFailedError) {

        QByteArray payload = reply->readAll(); // JSON
        QString refresh_token;
        QString access_token;

        // parse the response and extract the tokens, pretty much the same for all services
        // although polar choose to also pass a user id, which is needed for future calls
        QJsonParseError parseError;
        QJsonDocument document = QJsonDocument::fromJson(payload, &parseError);
        if (parseError.error == QJsonParseError::NoError) {
            refresh_token = document["refresh_token"].toString();
            access_token = document["access_token"].toString();
        }

        settings.setValue("strava_accesstoken", access_token);
        settings.setValue("strava_refreshtoken", refresh_token);
        settings.setValue("strava_lastrefresh", QDateTime::currentDateTime());

        qDebug() << access_token << refresh_token;

    } else {

            // general error getting response
            QString error = QString(tr("Error retrieving access token, %1 (%2)")).arg(reply->errorString()).arg(reply->error());
            qDebug() << error << reply->url() << reply->readAll();
    }
}

void homeform::callbackReceived(const QVariantMap &values)
{
    qDebug() << "homeform::callbackReceived" << values;
    if(values.value("code").toString().length())
    {
        strava_code = values.value("code").toString();
        qDebug() << strava_code;
    }
}

QOAuth2AuthorizationCodeFlow* homeform::strava_connect()
{
    if(manager)
    {
        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    OAuth2Parameter parameter;
    auto strava = new QOAuth2AuthorizationCodeFlow(manager, this);
    strava->setScope("activity:read_all,activity:write");
    strava->setClientIdentifier(STRAVA_CLIENT_ID);
    strava->setAuthorizationUrl(QUrl("https://www.strava.com/oauth/authorize"));
    strava->setAccessTokenUrl(QUrl("https://www.strava.com/oauth/token"));
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
    strava->setClientIdentifierSharedKey(STRINGIFY(STRAVA_SECRET_KEY));
#elif defined (WIN32)
#pragma message("DEFINE STRAVA_SECRET_KEY!!!")
#else
#warning "DEFINE STRAVA_SECRET_KEY!!!"
#endif
    strava->setModifyParametersFunction(buildModifyParametersFunction(QUrl(""), QUrl("")));
    auto replyHandler = new QOAuthHttpServerReplyHandler(QHostAddress("127.0.0.1"), 8091, this);
    connect(replyHandler,&QOAuthHttpServerReplyHandler::replyDataReceived, this,
            &homeform::replyDataReceived);
    connect(replyHandler,&QOAuthHttpServerReplyHandler::callbackReceived, this,
            &homeform::callbackReceived);
    strava->setReplyHandler(replyHandler);

    return strava;
}

void homeform::strava_connect_clicked()
{
    QLoggingCategory::setFilterRules("qt.networkauth.*=true");
    strava = strava_connect();
    connect(strava,&QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            this,&homeform::onStravaAuthorizeWithBrowser);
    connect(strava,&QOAuth2AuthorizationCodeFlow::granted,
            this,&homeform::onStravaGranted);
    strava->grant();
    //qDebug() << QAbstractOAuth2::post("https://www.strava.com/oauth/authorize?client_id=7976&scope=activity:read_all,activity:write&redirect_uri=http://127.0.0.1&response_type=code&approval_prompt=force");
}

bool homeform::generalPopupVisible()
{
    return m_generalPopupVisible;
}

void homeform::setGeneralPopupVisible(bool value)
{
    m_generalPopupVisible = value;
    generalPopupVisibleChanged(m_generalPopupVisible);
}

void homeform::smtpError(SmtpClient::SmtpError e)
{
    qDebug() << "SMTP ERROR" << e;
}

void homeform::sendMail()
{
    QSettings settings;

    bool miles = settings.value("miles_unit", false).toBool();
    double unit_conversion = 1.0;
    if(miles)
        unit_conversion = 0.621371;

    // TODO: add a condition to avoid sending mail when the user look at the chart while is riding
    if(settings.value("user_email","").toString().length() == 0 || !bluetoothManager->device())
        return;

#ifdef SMTP_SERVER
#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
    SmtpClient smtp(STRINGIFY(SMTP_SERVER), 587, SmtpClient::TlsConnection);
    connect(&smtp, SIGNAL(smtpError(SmtpClient::SmtpError)), this, SLOT(smtpError(SmtpClient::SmtpError)));
#else
#warning "stmp server is unset!"
    SmtpClient smtp("",25, SmtpClient::TlsConnection);
    return;
#endif

    // We need to set the username (your email address) and the password
    // for smtp authentification.
#ifdef SMTP_PASSWORD
#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
    smtp.setUser(STRINGIFY(SMTP_USERNAME));
#else
    #warning "smtp username is unset!"
    return;
#endif
#ifdef SMTP_PASSWORD
#define _STR(x) #x
#define STRINGIFY(x)  _STR(x)
    smtp.setPassword(STRINGIFY(SMTP_PASSWORD));
#else
#warning "smtp password is unset!"
    return;
#endif

    // Now we create a MimeMessage object. This will be the email.

    MimeMessage message;

    message.setSender(new EmailAddress("no-reply@qzapp.it", "QZ"));
    message.addRecipient(new EmailAddress(settings.value("user_email","").toString(), settings.value("user_email","").toString()));
    if(Session.length())
    {
        QString title = Session.first().time.toString();
        if(stravaPelotonActivityName.length())
            title += " " + stravaPelotonActivityName + " - " + stravaPelotonInstructorName;
        message.setSubject(title);
    }
    else
        message.setSubject("Test");

    // Now add some text to the email.
    // First we create a MimeText object.

    MimeText text;

    QString textMessage = "Great workout!\n\n";

    if(pelotonHandler)
    {
        if(pelotonHandler->current_ride_id.length())
        {
            textMessage += stravaPelotonActivityName + " - " + stravaPelotonInstructorName + " https://members.onepeloton.com/classes/cycling?modal=classDetailsModal&classId=" + pelotonHandler->current_ride_id;
        }
    }

    textMessage += "Average Speed: " + QString::number(bluetoothManager->device()->currentSpeed().average() * unit_conversion, 'f', 1) + "\n";
    textMessage += "Max Speed: " + QString::number(bluetoothManager->device()->currentSpeed().max() * unit_conversion, 'f', 1) + "\n";
    textMessage += "Calories burned: " + QString::number(bluetoothManager->device()->calories(), 'f', 0) + "\n";
    textMessage += "Distance: " + QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 1) + "\n";
    textMessage += "Average Watt: " + QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0) + "\n";
    textMessage += "Max Watt: " + QString::number(bluetoothManager->device()->wattsMetric().max(), 'f', 0) + "\n";
    textMessage += "Average Heart Rate: " + QString::number(bluetoothManager->device()->currentHeart().average(), 'f', 0) + "\n";
    textMessage += "Max Heart Rate: " + QString::number(bluetoothManager->device()->currentHeart().max(), 'f', 0) + "\n";
    textMessage += "Total Output: " + QString::number(bluetoothManager->device()->jouls().max() / 1000.0, 'f', 0) + "\n";
    textMessage += "Elapsed Time: " + bluetoothManager->device()->elapsedTime().toString() + "\n";
    textMessage += "Moving Time: " + bluetoothManager->device()->movingTime().toString() + "\n";
    if(bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
    {
        textMessage += "Average Cadence: " + QString::number(((bike*)bluetoothManager->device())->currentCadence().average(), 'f', 0) + "\n";
        textMessage += "Max Cadence: " + QString::number(((bike*)bluetoothManager->device())->currentCadence().max(), 'f', 0) + "\n";
        textMessage += "Average Resistance: " + QString::number(((bike*)bluetoothManager->device())->currentResistance().average(), 'f', 0) + "\n";
        textMessage += "Max Resistance: " + QString::number(((bike*)bluetoothManager->device())->currentResistance().max(), 'f', 0) + "\n";
        textMessage += "Average Peloton Resistance: " + QString::number(((bike*)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) + "\n";
        textMessage += "Max Peloton Resistance: " + QString::number(((bike*)bluetoothManager->device())->pelotonResistance().max(), 'f', 0) + "\n";
    }
    textMessage += "\n\nQZ version: " + QApplication::applicationVersion();
    text.setText(textMessage);
    message.addPart(&text);

    foreach(QString f, chartImagesFilenames)
    {
        // Create a MimeInlineFile object for each image
        MimeInlineFile* image = new MimeInlineFile((new QFile(f)));

        // An unique content id must be setted
        image->setContentId(f);
        image->setContentType("image/jpg");
        message.addPart(image);
    }

    if(lastFitFileSaved.length())
    {
        // Create a MimeInlineFile object for each image
        MimeInlineFile* fit = new MimeInlineFile((new QFile(lastFitFileSaved)));

        // An unique content id must be setted
        fit->setContentId(lastFitFileSaved);
        fit->setContentType("application/octet-stream");
        message.addPart(fit);
    }

    smtp.connectToHost();
    smtp.login();
    smtp.sendMail(message);
    smtp.quit();

    // delete image variable TODO
}

#if defined(Q_OS_ANDROID)
QString homeform::getAndroidDataAppDir() {
    static QString path = "";

    if(path.length())
        return path;

    QAndroidJniObject filesArr = QtAndroid::androidActivity().callObjectMethod("getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;", nullptr);
    jobjectArray dataArray = filesArr.object<jobjectArray>();
    QString out;
    if (dataArray) {
        QAndroidJniEnvironment env;
        jsize dataSize = env->GetArrayLength(dataArray);
        if (dataSize) {
            QAndroidJniObject mediaPath;
            QAndroidJniObject file;
            for (int i = 0; i<dataSize; i++) {
                file = env->GetObjectArrayElement(dataArray, i);
                jboolean val = QAndroidJniObject::callStaticMethod<jboolean>("android/os/Environment", "isExternalStorageRemovable", "(Ljava/io/File;)Z", file.object());
                mediaPath = file.callObjectMethod( "getAbsolutePath", "()Ljava/lang/String;" );
                out = mediaPath.toString();
                if (!val)
                    break;
            }
        }
    }
    path = out;
    return out;
}
#endif
