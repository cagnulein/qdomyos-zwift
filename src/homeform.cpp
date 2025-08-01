#include "homeform.h"
#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif
#include "localipaddress.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <jni.h>
#include <QJniObject>
#endif
#include "material.h"
#include "qfit.h"
#include "simplecrypt.h"
#include "templateinfosenderbuilder.h"
#include "zwiftworkout.h"

#include <QAbstractOAuth2>
#include <QApplication>
#include <QByteArray>
#include <QDesktopServices>
#include <QDir>
#include <QDirIterator>
#include <QFileInfo>
#include <QGeoCoordinate>
#include <QHttpMultiPart>
#include <QImageWriter>
#include <QJsonDocument>
#include <QNetworkAccessManager>
#include <QNetworkInterface>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QProcess>
#include <QQmlContext>
#include <QQmlFile>

#include <QRandomGenerator>
#include <QSettings>
#include <QStandardPaths>
#include <QTime>
#include <QUrlQuery>
#include <chrono>

homeform *homeform::m_singleton = 0;
using namespace std::chrono_literals;

#ifndef STRAVA_CLIENT_ID
#define STRAVA_CLIENT_ID 7976
#if defined(WIN32)
#pragma message("DEFINE STRAVA_CLIENT_ID!!!")
#else
#pragma message "DEFINE STRAVA_CLIENT_ID!!!"
#endif
#endif
#define STRAVA_CLIENT_ID_S STRINGIFY(STRAVA_CLIENT_ID)

DataObject::DataObject(const QString &name, const QString &icon, const QString &value, bool writable, const QString &id,
                       int valueFontSize, int labelFontSize, const QString &valueFontColor, const QString &secondLine,
                       const int gridId, bool largeButton, QString largeButtonLabel, QString largeButtonColor) {
    m_name = name;
    m_icon = icon;
    m_value = value;
    m_secondLine = secondLine;
    m_writable = writable;
    m_id = id;
    m_valueFontSize = valueFontSize;
    m_valueFontColor = valueFontColor;
    m_labelFontSize = labelFontSize;
    m_gridId = gridId;
    m_largeButton = largeButton;
    m_largeButtonLabel = largeButtonLabel;
    m_largeButtonColor = largeButtonColor;

    emit plusNameChanged(plusName());   // NOTE: clazy-incorrecrt-emit
    emit minusNameChanged(minusName()); // NOTE: clazy-incorrecrt-emit
    emit identificatorChanged(identificator());
    emit largeButtonChanged(this->largeButton());
    emit largeButtonLabelChanged(this->largeButtonLabel());
    emit largeButtonColorChanged(this->largeButtonColor());
}

void DataObject::setName(const QString &v) {
    m_name = v;
    emit nameChanged(m_name);
}
void DataObject::setValue(const QString &v) {
    m_value = v;
    emit valueChanged(m_value);
}
void DataObject::setSecondLine(const QString &value) {
    m_secondLine = value;
    emit secondLineChanged(m_secondLine);
}
void DataObject::setValueFontSize(int value) {
    m_valueFontSize = value;
    emit valueFontSizeChanged(m_valueFontSize);
}
void DataObject::setValueFontColor(const QString &value) {
    m_valueFontColor = value;
    emit valueFontColorChanged(m_valueFontColor);
}
void DataObject::setLargeButtonColor(const QString &color) {
    m_largeButtonColor = color;
    emit largeButtonColorChanged(m_largeButtonColor);
}
void DataObject::setLabelFontSize(int value) {
    m_labelFontSize = value;
    emit labelFontSizeChanged(m_labelFontSize);
}
void DataObject::setGridId(int id) {
    m_gridId = id;
    emit gridIdChanged(m_gridId);
}
void DataObject::setVisible(bool visible) {
    m_visible = visible;
    emit visibleChanged(m_visible);
}

homeform::homeform(QQmlApplicationEngine *engine, bluetooth *bl) {
    m_singleton = this;
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    QString unit = QStringLiteral("km");
    QString meters = QStringLiteral("m");
    QString weightLossUnit = QStringLiteral("Kg");
    QString cm = QStringLiteral("cm");
    if (miles) {
        unit = QStringLiteral("mi");
        weightLossUnit = QStringLiteral("Oz");
        meters = QStringLiteral("ft");
        cm = QStringLiteral("in");
    }

#ifdef Q_OS_ANDROID
    QJniObject context = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    m_locationServices = QJniObject::callStaticMethod<jboolean>("org/cagnulen/qdomyoszwift/LocationHelper", "start",
                                              "(Landroid/content/Context;)Z", context.object());
    if(m_locationServices) {
        QSettings settings;
        // so if someone pressed the skip message but now he forgot to enable GPS it will prompt out
        settings.setValue(QZSettings::skipLocationServicesDialog, QZSettings::default_skipLocationServicesDialog);
    }
#endif

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

    stravaAuthWebVisible = false;
    stravaWebVisibleChanged(stravaAuthWebVisible);

    QString innerId = QStringLiteral("inner");
    QString sKey = QStringLiteral("template_") + innerId + QStringLiteral("_" TEMPLATE_PRIVATE_WEBSERVER_ID "_");

    QString path = homeform::getWritableAppDir() + QStringLiteral("QZTemplates");
    this->userTemplateManager = TemplateInfoSenderBuilder::getInstance(
        QStringLiteral("user"), QStringList({path, QStringLiteral(":/templates/")}), this);

    settings.setValue(sKey + QStringLiteral("enabled"), true);
    settings.setValue(sKey + QStringLiteral("type"), TEMPLATE_TYPE_WEBSERVER);
    settings.setValue(sKey + QStringLiteral("port"), 0);
    this->innerTemplateManager =
        TemplateInfoSenderBuilder::getInstance(innerId, QStringList({QStringLiteral(":/inner_templates/")}), this);

    speed = new DataObject(QStringLiteral("Speed (") + unit + QStringLiteral("/h)"),
                           QStringLiteral("icons/icons/speed.png"), QStringLiteral("0.0"), true,
                           QStringLiteral("speed"), 48, labelFontSize);
    inclination = new DataObject(QStringLiteral("Inclination (%)"), QStringLiteral("icons/icons/inclination.png"),
                                 QStringLiteral("0.0"), true, QStringLiteral("inclination"), 48, labelFontSize);
    cadence = new DataObject(QStringLiteral("Cadence (rpm)"), QStringLiteral("icons/icons/cadence.png"),
                             QStringLiteral("0"), false, QStringLiteral("cadence"), 48, labelFontSize);
    elevation = new DataObject(QStringLiteral("Elev. Gain (") + meters + QStringLiteral(")"),
                               QStringLiteral("icons/icons/elevationgain.png"), QStringLiteral("0"), false,
                               QStringLiteral("elevation"), 48, labelFontSize);
    calories = new DataObject(QStringLiteral("Calories (KCal)"), QStringLiteral("icons/icons/kcal.png"),
                              QStringLiteral("0"), false, QStringLiteral("calories"), 48, labelFontSize);
    odometer = new DataObject(QStringLiteral("Odometer (") + unit + QStringLiteral(")"),
                              QStringLiteral("icons/icons/odometer.png"), QStringLiteral("0.0"), false,
                              QStringLiteral("odometer"), 48, labelFontSize);
    pace =
        new DataObject(QStringLiteral("Pace (m/") + unit + QStringLiteral(")"), QStringLiteral("icons/icons/pace.png"),
                       QStringLiteral("0:00"), false, QStringLiteral("pace"), 48, labelFontSize);

    target_pace =
        new DataObject(QStringLiteral("T.Pace(m/") + unit + QStringLiteral(")"), QStringLiteral("icons/icons/pace.png"),
                       QStringLiteral("0:00"), false, QStringLiteral("pace"), 48, labelFontSize);

    pace_last500m = new DataObject(QStringLiteral("Pace 500m (m/") + unit + QStringLiteral(")"),
                                   QStringLiteral("icons/icons/pace.png"), QStringLiteral("0:00"), false,
                                   QStringLiteral("pace"), 48, labelFontSize);

    resistance = new DataObject(QStringLiteral("Resistance"), QStringLiteral("icons/icons/resistance.png"),
                                QStringLiteral("0"), true, QStringLiteral("resistance"), 48, labelFontSize);
    peloton_resistance =
        new DataObject(QStringLiteral("Peloton R(%)"), QStringLiteral("icons/icons/resistance.png"),
                       QStringLiteral("0"), true, QStringLiteral("peloton_resistance"), 48, labelFontSize);
    target_resistance =
        new DataObject(QStringLiteral("Target R."), QStringLiteral("icons/icons/resistance.png"), QStringLiteral("0"),
                       true, QStringLiteral("target_resistance"), 48, labelFontSize);
    target_peloton_resistance =
        new DataObject(QStringLiteral("T.Peloton R(%)"), QStringLiteral("icons/icons/resistance.png"),
                       QStringLiteral("0"), false, QStringLiteral("target_peloton_resistance"), 48, labelFontSize);
    target_cadence = new DataObject(QStringLiteral("T.Cadence(rpm)"), QStringLiteral("icons/icons/cadence.png"),
                                    QStringLiteral("0"), false, QStringLiteral("target_cadence"), 48, labelFontSize);
    target_power = new DataObject(QStringLiteral("T.Power(W)"), QStringLiteral("icons/icons/watt.png"),
                                  QStringLiteral("0"), true, QStringLiteral("target_power"), 48, labelFontSize);
    target_zone = new DataObject(QStringLiteral("T.Zone"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("1"),
                                 false, QStringLiteral("target_zone"), 48, labelFontSize);
    target_speed = new DataObject(QStringLiteral("T.Speed (") + unit + QStringLiteral("/h)"),
                                  QStringLiteral("icons/icons/speed.png"), QStringLiteral("0.0"), true,
                                  QStringLiteral("target_speed"), 48, labelFontSize);
    target_incline =
        new DataObject(QStringLiteral("T.Incline (%)"), QStringLiteral("icons/icons/inclination.png"),
                       QStringLiteral("0.0"), true, QStringLiteral("target_inclination"), 48, labelFontSize);

    watt = new DataObject(QStringLiteral("Watt"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"), false,
                          QStringLiteral("watt"), 48, labelFontSize);
    weightLoss = new DataObject(QStringLiteral("Weight Loss(") + weightLossUnit + QStringLiteral(")"),
                                QStringLiteral("icons/icons/kcal.png"), QStringLiteral("0"), false,
                                QStringLiteral("weight_loss"), 48, labelFontSize);
    avgWatt = new DataObject(QStringLiteral("AVG Watt"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"),
                             false, QStringLiteral("avgWatt"), 48, labelFontSize);
    avgWattLap = new DataObject(QStringLiteral("AVG Watt Lap"), QStringLiteral("icons/icons/watt.png"),
                                QStringLiteral("0"), false, QStringLiteral("avgWattLap"), 48, labelFontSize);
    wattKg = new DataObject(QStringLiteral("Watt/Kg"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"),
                            false, QStringLiteral("watt_kg"), 48, labelFontSize);
    ftp = new DataObject(QStringLiteral("FTP Zone"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"), false,
                         QStringLiteral("ftp"), 48, labelFontSize);
    heart = new DataObject(QStringLiteral("Heart (bpm)"), QStringLiteral("icons/icons/heart_red.png"),
                           QStringLiteral("0"), false, QStringLiteral("heart"), 48, labelFontSize);
    fan = new DataObject(QStringLiteral("Fan Speed"), QStringLiteral("icons/icons/fan.png"), QStringLiteral("0"), true,
                         QStringLiteral("fan"), 48, labelFontSize);
    jouls = new DataObject(QStringLiteral("KJouls"), QStringLiteral("icons/icons/joul.png"), QStringLiteral("0"), false,
                           QStringLiteral("joul"), 48, labelFontSize);
    elapsed =
        new DataObject(QStringLiteral("Elapsed"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"),
                       false, QStringLiteral("elapsed"), valueElapsedFontSize, labelFontSize);
    moving_time =
        new DataObject(QStringLiteral("Moving T."), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"),
                       false, QStringLiteral("moving_time"), valueElapsedFontSize, labelFontSize);
    datetime = new DataObject(QStringLiteral("Clock"), QStringLiteral("icons/icons/clock.png"),
                              QTime::currentTime().toString(QStringLiteral("hh:mm:ss")), false,
                              QStringLiteral("datetime"), valueTimeFontSize, labelFontSize);
    lapElapsed = new DataObject(QStringLiteral("Lap Elapsed"), QStringLiteral("icons/icons/clock.png"),
                                QStringLiteral("0:00:00"), false, QStringLiteral("lapElapsed"), valueElapsedFontSize,
                                labelFontSize);
    remaningTimeTrainingProgramCurrentRow = new DataObject(
        QStringLiteral("Time to Next"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0:00:00"), true,
        QStringLiteral("remainingtimetrainprogramrow"), valueElapsedFontSize, labelFontSize);

    nextRows =
        new DataObject(QStringLiteral("Next Rows"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("N/A"),
                       false, QStringLiteral("nextrows"), valueElapsedFontSize, labelFontSize);

    mets = new DataObject(QStringLiteral("METS"), QStringLiteral("icons/icons/watt.png"), QStringLiteral("0"), false,
                          QStringLiteral("mets"), 48, labelFontSize);
    targetMets = new DataObject(QStringLiteral("Target METS"), QStringLiteral("icons/icons/watt.png"),
                                QStringLiteral("0"), false, QStringLiteral("targetmets"), 48, labelFontSize);
    rss = new DataObject(QStringLiteral("RSS"), QStringLiteral("icons/icons/watt.png"),
                                QStringLiteral("0"), false, QStringLiteral("rss"), 48, labelFontSize);                                
    steeringAngle = new DataObject(QStringLiteral("Steering"), QStringLiteral("icons/icons/cadence.png"),
                                   QStringLiteral("0"), false, QStringLiteral("steeringangle"), 48, labelFontSize);
    peloton_offset =
        new DataObject(QStringLiteral("Peloton Offset"), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0"),
                       true, QStringLiteral("peloton_offset"), valueElapsedFontSize, labelFontSize);
    peloton_remaining =
        new DataObject(QStringLiteral("Peloton Rem."), QStringLiteral("icons/icons/clock.png"), QStringLiteral("0"),
                       true, QStringLiteral("peloton_remaining"), valueElapsedFontSize, labelFontSize);
    strokesCount = new DataObject(QStringLiteral("Strokes Count"), QStringLiteral("icons/icons/cadence.png"),
                                  QStringLiteral("0"), false, QStringLiteral("strokes_count"), 48, labelFontSize);
    strokesLength = new DataObject(QStringLiteral("Strokes Length"), QStringLiteral("icons/icons/cadence.png"),
                                   QStringLiteral("0"), false, QStringLiteral("strokes_length"), 48, labelFontSize);
    gears = new DataObject(QStringLiteral("Gears"), QStringLiteral("icons/icons/elevationgain.png"),
                           QStringLiteral("0"), true, QStringLiteral("gears"), 48, labelFontSize);
    biggearsPlus = new DataObject(QStringLiteral("GearsPlus"), QStringLiteral("icons/icons/elevationgain.png"),
                                  QStringLiteral("0"), true, QStringLiteral("biggearsplus"), 48, labelFontSize, QStringLiteral("white"), QLatin1String(""), 0, true, "Gear +", QStringLiteral("red"));
    biggearsMinus = new DataObject(QStringLiteral("GearsMinus"), QStringLiteral("icons/icons/elevationgain.png"),
                                  QStringLiteral("0"), true, QStringLiteral("biggearsminus"), 48, labelFontSize, QStringLiteral("white"), QLatin1String(""), 0, true, "Gear -", QStringLiteral("green"));
    pidHR = new DataObject(QStringLiteral("PID Heart"), QStringLiteral("icons/icons/heart_red.png"),
                           QStringLiteral("0"), true, QStringLiteral("pid_hr"), 48, labelFontSize);
    extIncline = new DataObject(QStringLiteral("Ext.Inclin.(%)"), QStringLiteral("icons/icons/inclination.png"),
                                QStringLiteral("0.0"), true, QStringLiteral("external_inclination"), 48, labelFontSize);
    instantaneousStrideLengthCM =
        new DataObject(QStringLiteral("Stride L.(") + cm + ")", QStringLiteral("icons/icons/inclination.png"),
                       QStringLiteral("0"), false, QStringLiteral("stride_length"), 48, labelFontSize);
    groundContactMS = new DataObject(QStringLiteral("Ground C.(ms)"), QStringLiteral("icons/icons/inclination.png"),
                                     QStringLiteral("0"), false, QStringLiteral("ground_contact"), 48, labelFontSize);
    verticalOscillationMM =
        new DataObject(QStringLiteral("Vert.Osc.(mm)"), QStringLiteral("icons/icons/inclination.png"),
                       QStringLiteral("0"), false, QStringLiteral("vertical_oscillation"), 48, labelFontSize);

    stepCount =
        new DataObject(QStringLiteral("Step Count"), QStringLiteral("icons/icons/pace.png"),
                       QStringLiteral("0"), false, QStringLiteral("step_count"), 48, labelFontSize);

    ergMode = new DataObject(
        "", "", "", false, QStringLiteral("erg_mode"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true, QStringLiteral("ERG MODE"), "#696969");

    preset_resistance_1 = new DataObject(
        "", "", "", false, QStringLiteral("preset_resistance_1"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_resistance_1_label, QZSettings::default_tile_preset_resistance_1_label)
            .toString(),
        settings.value(QZSettings::tile_preset_resistance_1_color, QZSettings::default_tile_preset_resistance_1_color)
            .toString());
    preset_resistance_2 = new DataObject(
        "", "", "", false, QStringLiteral("preset_resistance_2"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_resistance_2_label, QZSettings::default_tile_preset_resistance_2_label)
            .toString(),
        settings.value(QZSettings::tile_preset_resistance_2_color, QZSettings::default_tile_preset_resistance_2_color)
            .toString());
    preset_resistance_3 = new DataObject(
        "", "", "", false, QStringLiteral("preset_resistance_3"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_resistance_3_label, QZSettings::default_tile_preset_resistance_3_label)
            .toString(),
        settings.value(QZSettings::tile_preset_resistance_3_color, QZSettings::default_tile_preset_resistance_3_color)
            .toString());
    preset_resistance_4 = new DataObject(
        "", "", "", false, QStringLiteral("preset_resistance_4"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_resistance_4_label, QZSettings::default_tile_preset_resistance_4_label)
            .toString(),
        settings.value(QZSettings::tile_preset_resistance_4_color, QZSettings::default_tile_preset_resistance_4_color)
            .toString());
    preset_resistance_5 = new DataObject(
        "", "", "", false, QStringLiteral("preset_resistance_5"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_resistance_5_label, QZSettings::default_tile_preset_resistance_5_label)
            .toString(),
        settings.value(QZSettings::tile_preset_resistance_5_color, QZSettings::default_tile_preset_resistance_5_color)
            .toString());
    preset_speed_1 = new DataObject(
        "", "", "", false, QStringLiteral("preset_speed_1"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_speed_1_label, QZSettings::default_tile_preset_speed_1_label).toString(),
        settings.value(QZSettings::tile_preset_speed_1_color, QZSettings::default_tile_preset_speed_1_color)
            .toString());
    preset_speed_2 = new DataObject(
        "", "", "", false, QStringLiteral("preset_speed_2"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_speed_2_label, QZSettings::default_tile_preset_speed_2_label).toString(),
        settings.value(QZSettings::tile_preset_speed_2_color, QZSettings::default_tile_preset_speed_2_color)
            .toString());
    preset_speed_3 = new DataObject(
        "", "", "", false, QStringLiteral("preset_speed_3"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_speed_3_label, QZSettings::default_tile_preset_speed_3_label).toString(),
        settings.value(QZSettings::tile_preset_speed_3_color, QZSettings::default_tile_preset_speed_3_color)
            .toString());
    preset_speed_4 = new DataObject(
        "", "", "", false, QStringLiteral("preset_speed_4"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_speed_4_label, QZSettings::default_tile_preset_speed_4_label).toString(),
        settings.value(QZSettings::tile_preset_speed_4_color, QZSettings::default_tile_preset_speed_4_color)
            .toString());
    preset_speed_5 = new DataObject(
        "", "", "", false, QStringLiteral("preset_speed_5"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_speed_5_label, QZSettings::default_tile_preset_speed_5_label).toString(),
        settings.value(QZSettings::tile_preset_speed_5_color, QZSettings::default_tile_preset_speed_5_color)
            .toString());
    preset_inclination_1 = new DataObject(
        "", "", "", false, QStringLiteral("preset_inclination_1"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_inclination_1_label, QZSettings::default_tile_preset_inclination_1_label)
            .toString(),
        settings.value(QZSettings::tile_preset_inclination_1_color, QZSettings::default_tile_preset_inclination_1_color)
            .toString());
    preset_inclination_2 = new DataObject(
        "", "", "", false, QStringLiteral("preset_inclination_2"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_inclination_2_label, QZSettings::default_tile_preset_inclination_2_label)
            .toString(),
        settings.value(QZSettings::tile_preset_inclination_2_color, QZSettings::default_tile_preset_inclination_2_color)
            .toString());
    preset_inclination_3 = new DataObject(
        "", "", "", false, QStringLiteral("preset_inclination_3"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_inclination_3_label, QZSettings::default_tile_preset_inclination_3_label)
            .toString(),
        settings.value(QZSettings::tile_preset_inclination_3_color, QZSettings::default_tile_preset_inclination_3_color)
            .toString());
    preset_inclination_4 = new DataObject(
        "", "", "", false, QStringLiteral("preset_inclination_4"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_inclination_4_label, QZSettings::default_tile_preset_inclination_4_label)
            .toString(),
        settings.value(QZSettings::tile_preset_inclination_4_color, QZSettings::default_tile_preset_inclination_4_color)
            .toString());
    preset_inclination_5 = new DataObject(
        "", "", "", false, QStringLiteral("preset_inclination_5"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_inclination_5_label, QZSettings::default_tile_preset_inclination_5_label)
            .toString(),
        settings.value(QZSettings::tile_preset_inclination_5_color, QZSettings::default_tile_preset_inclination_5_color)
            .toString());
    preset_powerzone_1 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_1"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_1_label, QZSettings::default_tile_preset_powerzone_1_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_1_color, QZSettings::default_tile_preset_powerzone_1_color).toString());

    preset_powerzone_2 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_2"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_2_label, QZSettings::default_tile_preset_powerzone_2_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_2_color, QZSettings::default_tile_preset_powerzone_2_color).toString());

    preset_powerzone_3 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_3"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_3_label, QZSettings::default_tile_preset_powerzone_3_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_3_color, QZSettings::default_tile_preset_powerzone_3_color).toString());

    preset_powerzone_4 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_4"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_4_label, QZSettings::default_tile_preset_powerzone_4_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_4_color, QZSettings::default_tile_preset_powerzone_4_color).toString());

    preset_powerzone_5 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_5"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_5_label, QZSettings::default_tile_preset_powerzone_5_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_5_color, QZSettings::default_tile_preset_powerzone_5_color).toString());

    preset_powerzone_6 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_6"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_6_label, QZSettings::default_tile_preset_powerzone_6_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_6_color, QZSettings::default_tile_preset_powerzone_6_color).toString());

    preset_powerzone_7 = new DataObject(
        "", "", "", false, QStringLiteral("preset_powerzone_7"), 48, labelFontSize, QStringLiteral("white"),
        QLatin1String(""), 0, true,
        settings.value(QZSettings::tile_preset_powerzone_7_label, QZSettings::default_tile_preset_powerzone_7_label).toString(),
        settings.value(QZSettings::tile_preset_powerzone_7_color, QZSettings::default_tile_preset_powerzone_7_color).toString());

    tile_hr_time_in_zone_1 = new DataObject(QStringLiteral("HR Zone 1+"), QStringLiteral("icons/icons/heart_red.png"),
                                            QStringLiteral("0:00:00"), false, QStringLiteral("tile_hr_time_in_zone_1"), valueElapsedFontSize, labelFontSize);

    tile_hr_time_in_zone_2 = new DataObject(QStringLiteral("HR Zone 2+"), QStringLiteral("icons/icons/heart_red.png"),
                                            QStringLiteral("0:00:00"), false, QStringLiteral("tile_hr_time_in_zone_2"), valueElapsedFontSize, labelFontSize);

    tile_hr_time_in_zone_3 = new DataObject(QStringLiteral("HR Zone 3+"), QStringLiteral("icons/icons/heart_red.png"),
                                            QStringLiteral("0:00:00"), false, QStringLiteral("tile_hr_time_in_zone_3"), valueElapsedFontSize, labelFontSize);

    tile_hr_time_in_zone_4 = new DataObject(QStringLiteral("HR Zone 4+"), QStringLiteral("icons/icons/heart_red.png"),
                                            QStringLiteral("0:00:00"), false, QStringLiteral("tile_hr_time_in_zone_4"), valueElapsedFontSize, labelFontSize);

    tile_hr_time_in_zone_5 = new DataObject(QStringLiteral("HR Zone 5"), QStringLiteral("icons/icons/heart_red.png"),
                                            QStringLiteral("0:00:00"), false, QStringLiteral("tile_hr_time_in_zone_5"), valueElapsedFontSize, labelFontSize);

    tile_heat_time_in_zone_1 = new DataObject(QStringLiteral("Heat Zone 1"), QStringLiteral("icons/icons/fan.png"),
                                              QStringLiteral("0:00:00"), false, QStringLiteral("tile_heat_time_in_zone_1"), valueElapsedFontSize, labelFontSize);

    tile_heat_time_in_zone_2 = new DataObject(QStringLiteral("Heat Zone 2"), QStringLiteral("icons/icons/fan.png"),
                                              QStringLiteral("0:00:00"), false, QStringLiteral("tile_heat_time_in_zone_2"), valueElapsedFontSize, labelFontSize);

    tile_heat_time_in_zone_3 = new DataObject(QStringLiteral("Heat Zone 3"), QStringLiteral("icons/icons/fan.png"),
                                              QStringLiteral("0:00:00"), false, QStringLiteral("tile_heat_time_in_zone_3"), valueElapsedFontSize, labelFontSize);

    tile_heat_time_in_zone_4 = new DataObject(QStringLiteral("Heat Zone 4"), QStringLiteral("icons/icons/fan.png"),
                                              QStringLiteral("0:00:00"), false, QStringLiteral("tile_heat_time_in_zone_4"), valueElapsedFontSize, labelFontSize);

    coreTemperature = new DataObject(QStringLiteral("Core Temp"), QStringLiteral("icons/icons/heart_red.png"),
                                  QStringLiteral("0"), false, QStringLiteral("coretemperature"), 48, labelFontSize, QStringLiteral("white"), QLatin1String(""));


    if (!settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {

        m_topBarHeight = 0;
        emit topBarHeightChanged(m_topBarHeight); // NOTE: clazy-incorrecrt-emit
        m_info = QLatin1String("");
        emit infoChanged(m_info); // NOTE: clazy-incorrecrt-emit
    }

    stravaPelotonActivityName = QLatin1String("");
    stravaPelotonInstructorName = QLatin1String("");
    activityDescription = QLatin1String("");
    stravaWorkoutName = QLatin1String("");
    movieFileName = QUrl("");

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS)) || (defined(Q_OS_ANDROID) && defined(LICENSE))
#ifndef STEAM_STORE
    connect(engine, &QQmlApplicationEngine::quit, &QGuiApplication::quit);
    connect(&tLicense, &QTimer::timeout, this, &homeform::licenseTimeout);
    tLicense.start(600000);
    licenseRequest();
#endif
#endif

    this->bluetoothManager = bl;
    this->engine = engine;
    connect(bluetoothManager, &bluetooth::bluetoothDeviceConnected, this, &homeform::bluetoothDeviceConnected);
    connect(bluetoothManager, &bluetooth::bluetoothDeviceDisconnected, this, &homeform::bluetoothDeviceDisconnected);
    connect(bluetoothManager, &bluetooth::deviceFound, this, &homeform::deviceFound);
    connect(bluetoothManager, &bluetooth::deviceConnected, this, &homeform::deviceConnected);
    connect(bluetoothManager, &bluetooth::ftmsAccessoryConnected, this, &homeform::ftmsAccessoryConnected);
    connect(bluetoothManager, &bluetooth::deviceConnected, this, &homeform::trainProgramSignals);
    connect(this, &homeform::workoutNameChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutNameChanged);
    connect(this, &homeform::workoutStartDateChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutStartDate);
    connect(this, &homeform::instructorNameChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::onInstructorName);
    connect(this, &homeform::workoutEventStateChanged, this->userTemplateManager,
            &TemplateInfoSenderBuilder::workoutEventStateChanged);
    connect(this->userTemplateManager, &TemplateInfoSenderBuilder::activityDescriptionChanged, this,
            &homeform::setActivityDescription);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::chartSaved, this, &homeform::chartSaved);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::lap, this, &homeform::Lap);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::floatingClose, this, &homeform::floatingOpen);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::autoResistance, this,
            &homeform::toggleAutoResistance);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::pelotonOffset_Plus, this,
            &homeform::pelotonOffset_Plus);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::pelotonOffset_Minus, this,
            &homeform::pelotonOffset_Minus);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::gears_Plus, this, &homeform::gearUp);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::gears_Minus, this, &homeform::gearDown);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::pelotonOffset, this, &homeform::pelotonOffset);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::pelotonAskStart, this, &homeform::pelotonAskStart);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::peloton_start_workout, this,
            &homeform::peloton_start_workout);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::peloton_abort_workout, this,
            &homeform::peloton_abort_workout);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::Start, this, &homeform::StartRequested);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::Pause, this, &homeform::Start);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::Stop, this, &homeform::StopRequested);
    connect(this, &homeform::workoutNameChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutNameChanged);
    connect(this, &homeform::workoutStartDateChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::onWorkoutStartDate);
    connect(this, &homeform::instructorNameChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::onInstructorName);
    connect(this, &homeform::workoutEventStateChanged, this->innerTemplateManager,
            &TemplateInfoSenderBuilder::workoutEventStateChanged);
    connect(this->innerTemplateManager, &TemplateInfoSenderBuilder::activityDescriptionChanged, this,
            &homeform::setActivityDescription);
    engine->rootContext()->setContextProperty(QStringLiteral("rootItem"), (QObject *)this);
    connect(this, &homeform::restoreDefaultWheelDiameter, this, &homeform::handleRestoreDefaultWheelDiameter);

    const QUrl url(QStringLiteral("qrc:/main.qml"));
    engine->load(url);


    this->trainProgram = new trainprogram(QList<trainrow>(), bl);

    timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &homeform::update);
    timer->start(1s);

    backupTimer = new QTimer(this);
    connect(backupTimer, &QTimer::timeout, this, &homeform::backup);
    backupTimer->start(1min);

    QObject *stack = nullptr;

    if(engine->rootObjects().count() > 0) {
        QObject *rootObject = engine->rootObjects().constFirst();
        QObject *home = rootObject->findChild<QObject *>(QStringLiteral("home"));
        stack = rootObject;

        engine->rootContext()->setContextProperty("pathController", &pathController);
        QObject::connect(home, SIGNAL(start_clicked()), this, SLOT(Start()));
        QObject::connect(home, SIGNAL(stop_clicked()), this, SLOT(Stop()));
        QObject::connect(stack, SIGNAL(trainprogram_open_clicked(QUrl)), this, SLOT(trainprogram_open_clicked(QUrl)));
        QObject::connect(stack, SIGNAL(trainprogram_open_other_folder(QUrl)), this, SLOT(trainprogram_open_other_folder(QUrl)));
        QObject::connect(stack, SIGNAL(gpx_open_other_folder(QUrl)), this, SLOT(gpx_open_other_folder(QUrl)));
        QObject::connect(stack, SIGNAL(profile_open_clicked(QUrl)), this, SLOT(profile_open_clicked(QUrl)));
        QObject::connect(stack, SIGNAL(trainprogram_preview(QUrl)), this, SLOT(trainprogram_preview(QUrl)));
        QObject::connect(stack, SIGNAL(gpxpreview_open_clicked(QUrl)), this, SLOT(gpxpreview_open_clicked(QUrl)));
        QObject::connect(stack, SIGNAL(trainprogram_zwo_loaded(QString)), this, SLOT(trainprogram_zwo_loaded(QString)));
        QObject::connect(stack, SIGNAL(gpx_open_clicked(QUrl)), this, SLOT(gpx_open_clicked(QUrl)));
        QObject::connect(stack, SIGNAL(gpx_save_clicked()), this, SLOT(gpx_save_clicked()));
        QObject::connect(stack, SIGNAL(fit_save_clicked()), this, SLOT(fit_save_clicked()));
        QObject::connect(stack, SIGNAL(strava_connect_clicked()), this, SLOT(strava_connect_clicked()));
        QObject::connect(stack, SIGNAL(refresh_bluetooth_devices_clicked()), this,
                         SLOT(refresh_bluetooth_devices_clicked()));
        QObject::connect(home, SIGNAL(lap_clicked()), this, SLOT(Lap()));
        QObject::connect(home, SIGNAL(peloton_start_workout()), this, SLOT(peloton_start_workout()));
        QObject::connect(home, SIGNAL(peloton_abort_workout()), this, SLOT(peloton_abort_workout()));
        QObject::connect(stack, SIGNAL(loadSettings(QUrl)), this, SLOT(loadSettings(QUrl)));
        QObject::connect(stack, SIGNAL(saveSettings(QUrl)), this, SLOT(saveSettings(QUrl)));
        QObject::connect(stack, SIGNAL(deleteSettings(QUrl)), this, SLOT(deleteSettings(QUrl)));
        QObject::connect(stack, SIGNAL(restoreSettings()), this, SLOT(restoreSettings()));
        QObject::connect(stack, SIGNAL(saveProfile(QString)), this, SLOT(saveProfile(QString)));
        QObject::connect(stack, SIGNAL(restart()), this, SLOT(restart()));

        QObject::connect(stack, SIGNAL(volumeUp()), this, SLOT(volumeUp()));
        QObject::connect(stack, SIGNAL(volumeDown()), this, SLOT(volumeDown()));
        QObject::connect(stack, SIGNAL(keyMediaPrevious()), this, SLOT(keyMediaPrevious()));
        QObject::connect(stack, SIGNAL(keyMediaNext()), this, SLOT(keyMediaNext()));
        QObject::connect(stack, SIGNAL(floatingOpen()), this, SLOT(floatingOpen()));
        QObject::connect(stack, SIGNAL(openFloatingWindowBrowser()), this, SLOT(openFloatingWindowBrowser()));
        QObject::connect(stack, SIGNAL(strava_upload_file_prepare()), this, SLOT(strava_upload_file_prepare()));

        qDebug() << "homeform constructor events linked";
    } else {
        qDebug() << "error on QML engine UI";
    }

#if !defined(Q_OS_ANDROID) && !defined(Q_OS_IOS)
    QObject::connect(engine, &QQmlApplicationEngine::quit, &QGuiApplication::quit);
#endif

    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
        emit stopIconChanged(stopIcon());     // NOTE: clazy-incorrecrt-emit
        emit stopTextChanged(stopText());     // NOTE: clazy-incorrecrt-emit
        emit startIconChanged(startIcon());   // NOTE: clazy-incorrecrt-emit
        emit startTextChanged(startText());   // NOTE: clazy-incorrecrt-emit
        emit startColorChanged(startColor()); // NOTE: clazy-incorrecrt-emit
        emit stopColorChanged(stopColor());   // NOTE: clazy-incorrecrt-emit
    }

    emit tile_orderChanged(tile_order()); // NOTE: clazy-incorrecrt-emit

    pelotonHandler = new peloton(bl);
    connect(pelotonHandler, &peloton::workoutStarted, this, &homeform::pelotonWorkoutStarted);
    connect(pelotonHandler, &peloton::workoutChanged, this, &homeform::pelotonWorkoutChanged);
    connect(pelotonHandler, &peloton::loginState, this, &homeform::pelotonLoginState);
    connect(pelotonHandler, &peloton::pzpLoginState, this, &homeform::pzpLoginState);
    connect(pelotonHandler, &peloton::pelotonAuthUrlChanged, this, &homeform::pelotonAuthUrlChanged);
    connect(pelotonHandler, &peloton::pelotonWebVisibleChanged, this, &homeform::pelotonWebVisibleChanged);
    if(stack)
        connect(stack, SIGNAL(peloton_connect_clicked()), pelotonHandler, SLOT(peloton_connect_clicked()));

    // copying bundles zwo files in the right path if necessary
    QDirIterator itZwo(":/zwo/");
    QDir().mkdir(getWritableAppDir() + "training/");
    while (itZwo.hasNext()) {
        qDebug() << itZwo.next() << itZwo.fileName();
        if (!QFile(getWritableAppDir() + "training/" + itZwo.fileName()).exists()) {
            QFile::copy(":/zwo/" + itZwo.fileName(), getWritableAppDir() + "training/" + itZwo.fileName());
        }
    }

    QDirIterator itGpx(":/gpx/");
    QDir().mkdir(getWritableAppDir() + "gpx/");
    while (itGpx.hasNext()) {
        qDebug() << itGpx.next() << itGpx.fileName();
        if (!QFile(getWritableAppDir() + "gpx/" + itGpx.fileName()).exists()) {
            QFile::copy(":/gpx/" + itGpx.fileName(), getWritableAppDir() + "gpx/" + itGpx.fileName());
        }
    }    
#ifdef Q_OS_ANDROID

    QString bluetoothName = getBluetoothName();
    qDebug() << "getBluetoothName()" << bluetoothName;

    QRegularExpression regex("^[A-Za-z0-9 ]+$");
    if(bluetoothName.length() > 9 || !regex.match(bluetoothName).hasMatch()) {
        setToastRequested("Bluetooth name too long, change it to a 4 letters one in the android settings and use only A-Z or 0-9 characters");
    }
    
    // Android 14 restrics access to /Android/data folder
    bool android_documents_folder = settings.value(QZSettings::android_documents_folder, QZSettings::default_android_documents_folder).toBool();
    if (android_documents_folder || QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 14)) {
        QDirIterator itAndroid(getAndroidDataAppDir(), QDirIterator::Subdirectories);
        QDir().mkdir(getWritableAppDir());
        QDir().mkdir(getProfileDir());
        while (itAndroid.hasNext()) {
            qDebug() << itAndroid.filePath() << itAndroid.fileName() << itAndroid.filePath().replace(itAndroid.path(), "");
            if (!QFile(getWritableAppDir() + itAndroid.next().replace(itAndroid.path(), "")).exists()) {
                if(QFile::copy(itAndroid.filePath(), getWritableAppDir() + itAndroid.filePath().replace(itAndroid.path(), "")))
                       QFile::remove(itAndroid.filePath());
            }
        }
    }
#endif

    m_speech.setLocale(QLocale::English);

#if defined(Q_OS_LINUX) && !defined(Q_OS_ANDROID)
    QBluetoothDeviceInfo b;
    deviceConnected(b);
#endif

    if (settings.value(QZSettings::peloton_bike_ocr, QZSettings::default_peloton_bike_ocr).toBool()) {
        QBluetoothDeviceInfo b;
        deviceConnected(b);
    }

#ifndef Q_OS_IOS
    iphone_browser = new QMdnsEngine::Browser(&iphone_server, "_qz_iphone._tcp.local.", &iphone_cache);

    QObject::connect(iphone_browser, &QMdnsEngine::Browser::serviceAdded, [](const QMdnsEngine::Service &service) {
        homeform::singleton()->iphone_service = service;
        qDebug() << service.name() << service.hostname() << service.port() << "discovered!";

        if (homeform::singleton()->iphone_resolver)
            delete homeform::singleton()->iphone_resolver;
        homeform::singleton()->iphone_resolver = new QMdnsEngine::Resolver(
            &homeform::singleton()->iphone_server, service.hostname(), &homeform::singleton()->iphone_cache);
        QObject::connect(homeform::singleton()->iphone_resolver, &QMdnsEngine::Resolver::resolved,
                         [](const QHostAddress &address) {
                             qDebug() << "resolved to" << address;
                             if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                                 (homeform::singleton()->iphone_socket == nullptr ||
                                  !homeform::singleton()->iphone_address.isEqual(address))) {
                                 if (homeform::singleton()->iphone_socket)
                                     delete homeform::singleton()->iphone_socket;
                                 homeform::singleton()->iphone_socket = new QTcpSocket();
                                 QObject::connect(homeform::singleton()->iphone_socket, &QTcpSocket::connected,
                                                  []() { qDebug() << "iphone_socket connected!"; });
                                 QObject::connect(homeform::singleton()->iphone_socket, &QTcpSocket::readyRead, []() {
                                     QString rec = homeform::singleton()->iphone_socket->readAll();
                                     qDebug() << "iphone_socket received << " << rec;
                                     QStringList fields = rec.split("#");
                                     foreach (QString f, fields) {
                                         if (f.contains("HR")) {
                                             QStringList values = f.split("=");
                                             if (values.length() > 1) {
                                                 emit homeform::singleton()->heartRate(values[1].toDouble());
                                             }
                                         }
                                     }
                                 });

                                 homeform::singleton()->iphone_address = address;
                                 homeform::singleton()->iphone_socket->connectToHost(
                                     address, homeform::singleton()->iphone_service.port());
                             }
                         });
    });

    QObject::connect(iphone_browser, &QMdnsEngine::Browser::serviceUpdated, [](const QMdnsEngine::Service &service) {
        homeform::singleton()->iphone_service = service;
        qDebug() << service.name() << service.hostname() << service.port() << "updated!";

        if (homeform::singleton()->iphone_resolver)
            delete homeform::singleton()->iphone_resolver;
        homeform::singleton()->iphone_resolver = new QMdnsEngine::Resolver(
            &homeform::singleton()->iphone_server, service.hostname(), &homeform::singleton()->iphone_cache);
        QObject::connect(homeform::singleton()->iphone_resolver, &QMdnsEngine::Resolver::resolved,
                         [](const QHostAddress &address) {
                             if (address.protocol() == QAbstractSocket::IPv4Protocol &&
                                 (homeform::singleton()->iphone_socket == nullptr ||
                                  !homeform::singleton()->iphone_address.isEqual(address))) {
                                 if (homeform::singleton()->iphone_socket)
                                     delete homeform::singleton()->iphone_socket;
                                 qDebug() << "resolved to" << address;
                                 homeform::singleton()->iphone_socket = new QTcpSocket();
                                 QObject::connect(homeform::singleton()->iphone_socket, &QTcpSocket::connected,
                                                  []() { qDebug() << "iphone_socket connected!"; });
                                 QObject::connect(homeform::singleton()->iphone_socket, &QTcpSocket::readyRead, []() {
                                     QString rec = homeform::singleton()->iphone_socket->readAll();
                                     qDebug() << "iphone_socket received << " << rec;
                                     QStringList fields = rec.split("#");
                                     foreach (QString f, fields) {
                                         if (f.contains("HR")) {
                                             QStringList values = f.split("=");
                                             if (values.length() > 1) {
                                                 emit homeform::singleton()->heartRate(values[1].toDouble());
                                             }
                                         }
                                     }
                                 });
                                 homeform::singleton()->iphone_address = address;
                                 homeform::singleton()->iphone_socket->connectToHost(
                                     address, homeform::singleton()->iphone_service.port());
                             }
                         });
    });
#endif

    if (QSslSocket::supportsSsl()) {
        qDebug() << "SSL supported";
    } else {
        qDebug() << "SSL non supported";
    }
    
#ifdef Q_OS_ANDROID
    QJniObject javaPath = QJniObject::fromString(getWritableAppDir());
    QJniObject context2 = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/Shortcuts", "createShortcutsForFiles",
                                                "(Ljava/lang/String;Landroid/content/Context;)V", javaPath.object<jstring>(), context2.object());

    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/MediaButtonReceiver",
                                              "registerReceiver",
                                              "(Landroid/content/Context;)V",
                                              context2.object());
#endif    

    bluetoothManager->homeformLoaded = true;
}

#ifdef Q_OS_ANDROID
extern "C" {
JNIEXPORT void JNICALL
  Java_org_cagnulen_qdomyoszwift_MediaButtonReceiver_nativeOnMediaButtonEvent(JNIEnv *env, jobject obj, jint prev, jint current, jint max) {
    qDebug() << "Media button event: current =" << current << "max =" << max << "prev =" << prev;
    static QDateTime volumeLastChange = QDateTime::currentDateTime();
    QSettings settings;
    bool gears_volume_debouncing = settings.value(QZSettings::gears_volume_debouncing, QZSettings::default_gears_volume_debouncing).toBool();

    if (!settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
      qDebug() << "volume_change_gears disabled!"; 
      return;
    }
  
    if(gears_volume_debouncing && volumeLastChange.msecsTo(QDateTime::currentDateTime()) < 500) {
      qDebug() << "volume debouncing"; 
      return;
    }
  
    if(prev > current)
      homeform::singleton()->Minus(QStringLiteral("gears"));
    else
      homeform::singleton()->Plus(QStringLiteral("gears"));      

    volumeLastChange = QDateTime::currentDateTime();
  }
}
#endif

void homeform::setActivityDescription(QString desc) { activityDescription = desc; }

void homeform::chartSaved(QString fileName) {
    if (!stopped)
        return;
    chartImagesFilenames.append(fileName);
    if (chartImagesFilenames.length() >= 9) {
        sendMail();
        qDebug() << "removing chart images";
        for (const QString &f : qAsConst(chartImagesFilenames)) {
            QFile::remove(f);
        }
        chartImagesFilenames.clear();
    }
}

void homeform::keyMediaPrevious() {
    qDebug() << QStringLiteral("keyMediaPrevious");
    QSettings settings;
    if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
        Minus(QStringLiteral("gears"));
    }
}

void homeform::keyMediaNext() {
    qDebug() << QStringLiteral("keyMediaNext");
    QSettings settings;
    if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
        Plus(QStringLiteral("gears"));
    }
}

void homeform::volumeUp() {
    qDebug() << QStringLiteral("volumeUp");
    QSettings settings;
    if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        Plus(QStringLiteral("speed"));
    } else if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
        Plus(QStringLiteral("gears"));
    }
}

void homeform::volumeDown() {
    qDebug() << QStringLiteral("volumeDown");
    QSettings settings;
    if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        Minus(QStringLiteral("speed"));
    } else if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
        Minus(QStringLiteral("gears"));
    }
}

void homeform::floatingOpen() {
#ifdef Q_OS_ANDROID
    if (!floating_open) {

        QSettings settings;
        // Get the floating window type setting (0 = classic, 1 = horizontal)
        int floatingWindowType = settings.value(QZSettings::floatingwindow_type, QZSettings::default_floatingwindow_type).toInt();
        
        // Determine which HTML file to use based on the setting
        QString htmlFile = (floatingWindowType == 0) ? "floating.htm" : "hfloating.htm";
        
        QJniObject javaHtmlFile = QJniObject::fromString(htmlFile);
        
        QJniObject context3 = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
        QJniObject::callStaticMethod<void>(
            "org/cagnulen/qdomyoszwift/FloatingHandler", "show", "(Landroid/content/Context;IIIILjava/lang/String;)V",
            context3.object(), 
            settings.value("template_inner_QZWS_port", 6666).toInt(),
            settings.value(QZSettings::floating_width, QZSettings::default_floating_width).toInt(),
            settings.value(QZSettings::floating_height, QZSettings::default_floating_height).toInt(),
            settings.value(QZSettings::floating_transparency, QZSettings::default_floating_transparency).toInt(),
            javaHtmlFile.object<jstring>());
    } else {
        QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/FloatingHandler", "hide", "()V");
    }
    floating_open = !floating_open;
#endif
}

void homeform::openFloatingWindowBrowser() {
    QSettings settings;
    QHostAddress a;
    foreach (QNetworkInterface netInterface, QNetworkInterface::allInterfaces()) {
        // Return only the first non-loopback MAC Address
        QString addr = netInterface.hardwareAddress();
        if (!(netInterface.flags() & QNetworkInterface::IsLoopBack) && !addr.isEmpty()) {
            const auto entries = netInterface.addressEntries();
            for (const QNetworkAddressEntry &newEntry : entries) {
                qDebug() << newEntry.ip().toIPv4Address();
                if (!newEntry.ip().isLoopback()) {
                    a = newEntry.ip();
                    break;
                }
            }
        }
    }
    QString url = "http://" + localipaddress::getIP(a).toString() + ":" +
                  QString::number(settings.value("template_inner_QZWS_port", 6666).toInt()) + "/floating/floating.htm";
    QDesktopServices::openUrl(url);
}

void homeform::peloton_abort_workout() {
    m_pelotonAskStart = false;
    emit changePelotonAskStart(pelotonAskStart());
    qDebug() << QStringLiteral("peloton_abort_workout!");
    pelotonAbortedName = pelotonAskedName;
    pelotonAbortedInstructor = pelotonAskedInstructor;
}

void homeform::peloton_start_workout() {

    QSettings settings;
    stravaPelotonActivityName = pelotonAskedName;
    stravaPelotonInstructorName = pelotonAskedInstructor;
    if (pelotonHandler) {
        if (pelotonHandler->current_workout_type.toLower().startsWith("meditation") ||
            pelotonHandler->current_workout_type.toLower().startsWith("cardio") ||
            pelotonHandler->current_workout_type.toLower().startsWith("circuit") ||
            pelotonHandler->current_workout_type.toLower().startsWith("strength") ||
            pelotonHandler->current_workout_type.toLower().startsWith("stretching") ||
            pelotonHandler->current_workout_type.toLower().startsWith("yoga"))
            stravaPelotonWorkoutType = FIT_SPORT_GENERIC;
        else if (pelotonHandler->current_workout_type.toLower().startsWith("walking"))
            stravaPelotonWorkoutType = FIT_SPORT_WALKING;
        else if (pelotonHandler->current_workout_type.toLower().startsWith("running"))
            stravaPelotonWorkoutType = FIT_SPORT_RUNNING;
        else
            stravaPelotonWorkoutType = FIT_SPORT_INVALID;

        pelotonHandler->downloadImage();
    } else {
        stravaPelotonWorkoutType = FIT_SPORT_INVALID;
    }
    emit workoutNameChanged(workoutName());
    emit instructorNameChanged(instructorName());

    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
        m_info = stravaPelotonActivityName;
        emit infoChanged(m_info);
    }

    m_pelotonAskStart = false;
    emit changePelotonAskStart(pelotonAskStart());
    qDebug() << QStringLiteral("peloton_start_workout!");
    if (pelotonHandler && !pelotonHandler->trainrows.isEmpty()) {
        if (trainProgram) {
            // useless, cause a treadmill to stop
            // emit trainProgram->stop(false);

            delete trainProgram;
            trainProgram = nullptr;
        }
        trainProgram = new trainprogram(pelotonHandler->trainrows, bluetoothManager);
        if (!stravaPelotonActivityName.isEmpty() && !stravaPelotonInstructorName.isEmpty()) {
            QString path = getWritableAppDir() + "training/" + workoutNameBasedOnBluetoothDevice() + "/" +
                           stravaPelotonInstructorName + "/";
            QDir().mkpath(path);
            lastTrainProgramFileSaved =
                path + stravaPelotonActivityName.replace("/", "-") + " - " + stravaPelotonInstructorName + ".xml";
            trainProgram->save(lastTrainProgramFileSaved);
        }
        trainProgramSignals();
        trainProgram->restart();
    }
}

void homeform::pzpLoginState(bool ok) {

    m_pzpLoginState = (ok ? 1 : 0);
    emit pzpLoginChanged(m_pzpLoginState);
}

void homeform::pelotonLoginState(bool ok) {

    m_pelotonLoginState = (ok ? 1 : 0);
    emit pelotonLoginChanged(m_pelotonLoginState);
    if (!ok) {
        setToastRequested("Peloton Login Error!");        
    }
}

void homeform::zwiftLoginState(bool ok) {

    m_zwiftLoginState = (ok ? 1 : 0);
    emit zwiftLoginChanged(m_zwiftLoginState);
    if (!ok) {
        setToastRequested("Zwift Login Error!");
    }
}


void homeform::pelotonWorkoutStarted(const QString &name, const QString &instructor) {
    pelotonAskedName = name;
    pelotonAskedInstructor = instructor;

    if (!pelotonAskedName.compare(pelotonAbortedName) && !pelotonAskedInstructor.compare(pelotonAbortedInstructor)) {
        qDebug() << QStringLiteral("Peloton class aborted before");
        return;
    }
    pelotonAbortedName.clear();
    pelotonAbortedInstructor.clear();

    if (pelotonHandler) {
        switch (pelotonHandler->currentApi()) {
        case peloton::homefitnessbuddy_api:
            m_pelotonProvider = QStringLiteral("Metrics are provided from https://www.homefitnessbuddy.com");
            break;
        case peloton::powerzonepack_api:
            m_pelotonProvider = QStringLiteral("Metrics are provided from https://pzpack.com");
            break;
        case peloton::no_metrics:
            m_pelotonProvider = QStringLiteral("No metrics are provided for this class");
            break;
        default:
            m_pelotonProvider = QStringLiteral("Metrics are provided from https://onepeloton.com");
            break;
        }
    }
    emit changePelotonProvider(pelotonProvider());
    int peloton_start_offset = pelotonHandler->getIntroOffset();
    qDebug() << "peloton_start_time" << pelotonHandler->start_time << "current epoch" << QDateTime::currentSecsSinceEpoch() << qAbs(pelotonHandler->start_time - QDateTime::currentSecsSinceEpoch()) << peloton_start_offset;
    QSettings settings;
    bool peloton_auto_start_with_intro = settings.value(QZSettings::peloton_auto_start_with_intro, QZSettings::default_peloton_auto_start_with_intro).toBool();
    bool peloton_auto_start_without_intro = settings.value(QZSettings::peloton_auto_start_without_intro, QZSettings::default_peloton_auto_start_without_intro).toBool();
    if(qAbs(pelotonHandler->start_time - QDateTime::currentSecsSinceEpoch()) < 180 && (peloton_auto_start_with_intro || peloton_auto_start_without_intro)) {
        // auto start is possible!        
        int timer = 0;        

        if(peloton_auto_start_with_intro) {
            setToastRequested(QStringLiteral("Peloton workout auto started! It will start automatically after the intro! ") + name + QStringLiteral(" - ") + instructor);
            timer = (pelotonHandler->start_time - QDateTime::currentSecsSinceEpoch()) + (peloton_start_offset + 4);  // + 64; // // 4 average time to buffer and 60 to the intro
        } else {
            setToastRequested(QStringLiteral("Peloton workout auto started skipping the intro! ") + name + QStringLiteral(" - ") + instructor);
            timer = (pelotonHandler->start_time - QDateTime::currentSecsSinceEpoch()) + 6;  // 6 average time to push skip intro and wait the 3 seconds of the intro
        }
        if(timer <= 0)
            peloton_start_workout();
        else {
            QTimer::singleShot(timer * 1000, this, [this]() {
                peloton_start_workout();
            });
        }
    } else {
        m_pelotonAskStart = true;
        emit changePelotonAskStart(pelotonAskStart());
    }
}

void homeform::pelotonWorkoutChanged(const QString &name, const QString &instructor) {

}

QString homeform::getWritableAppDir() {
    QString path = QLatin1String("");
#if defined(Q_OS_ANDROID)
    QSettings settings;
    bool android_documents_folder = settings.value(QZSettings::android_documents_folder, QZSettings::default_android_documents_folder).toBool();
    if (android_documents_folder || QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 14)) {
        path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/QZ/";
        QDir().mkdir(path);
        // Create .nomedia file to prevent gallery indexing
        QFile nomediaFile(path + ".nomedia");
        if (!nomediaFile.exists()) {
            nomediaFile.open(QIODevice::WriteOnly);
            nomediaFile.close();
        }
    } else {
        path = getAndroidDataAppDir() + "/";
    }
#elif defined(Q_OS_MACOS) || defined(Q_OS_OSX)
    path = QStandardPaths::writableLocation(QStandardPaths::DownloadLocation) + "/";
#elif defined(Q_OS_IOS)
    path = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/";
#elif defined(Q_OS_WINDOWS)
    path = QDir::currentPath() + "/";
#endif
    return path;
}

void homeform::backup() {

    static uint8_t index = 0;
    qDebug() << QStringLiteral("saving fit file backup...");

    QString path = getWritableAppDir();
    bluetoothdevice *dev = bluetoothManager->device();
    if (dev) {

        QString filename = path + QString::number(index) + backupFitFileName;
        QFile::remove(filename);
        qfit::save(filename, Session, dev->deviceType(),
                   qobject_cast<m3ibike *>(dev) ? QFIT_PROCESS_DISTANCENOISE : QFIT_PROCESS_NONE,
                   stravaPelotonWorkoutType, dev->bluetoothDevice.name());

        index++;
        if (index > 1) {
            index = 0;
        }
    }
}

QString homeform::stopColor() { return QStringLiteral("#00000000"); }

QString homeform::startColor() {
    static uint8_t startColorToggle = 0;
    if (paused || stopped) {
        if (startColorToggle) {

            startColorToggle = 0;
            return QStringLiteral("red");
        } else {

            startColorToggle = 1;
            return QStringLiteral("#00000000");
        }
    }
    return QStringLiteral("#00000000");
}

void homeform::refresh_bluetooth_devices_clicked() {

    bluetoothManager->onlyDiscover = true;
    bluetoothManager->restart();
}

homeform::~homeform() {
    gpx_save_clicked();
    fit_save_clicked();
}

void homeform::aboutToQuit() {
    qDebug() << "homeform::aboutToQuit()";

#ifdef Q_OS_ANDROID
    // closing floating window
    if (floating_open)
        floatingOpen();
    QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/NotificationClient", "hide", "()V");
#endif

    QSettings settings;
    if (settings.value(QZSettings::fit_file_saved_on_quit, QZSettings::default_fit_file_saved_on_quit).toBool()) {
        qDebug() << "fit_file_saved_on_quit true";
        fit_save_clicked();
    }

    if (bluetoothManager->device())
        bluetoothManager->device()->disconnectBluetooth();
}

void homeform::trainProgramSignals() {
    if (bluetoothManager->device()) {
        disconnect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        disconnect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        disconnect(trainProgram, &trainprogram::stop, this, &homeform::StopFromTrainProgram);
        disconnect(trainProgram, &trainprogram::lap, this, &homeform::Lap);
        disconnect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeed);
        disconnect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeInclination);
        disconnect(trainProgram, &trainprogram::changeNextInclination300Meters, bluetoothManager->device(),
                   &bluetoothdevice::changeNextInclination300Meters);
        disconnect(trainProgram, &trainprogram::changeInclination, ((bike *)bluetoothManager->device()),
                   &bike::changeInclination);
        disconnect(trainProgram, &trainprogram::changeFanSpeed, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeFanSpeed);
        disconnect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                   &treadmill::changeSpeedAndInclination);
        disconnect(trainProgram, &trainprogram::changeResistance, ((bike *)bluetoothManager->device()),
                   &bike::changeResistance);
        disconnect(trainProgram, &trainprogram::changeRequestedPelotonResistance, ((bike *)bluetoothManager->device()),
                   &bike::changeRequestedPelotonResistance);
        disconnect(trainProgram, &trainprogram::changeCadence, ((bike *)bluetoothManager->device()),
                   &bike::changeCadence);
        disconnect(trainProgram, &trainprogram::changePower, ((treadmill *)bluetoothManager->device()), &treadmill::changePower);
        disconnect(trainProgram, &trainprogram::changePower, ((bike *)bluetoothManager->device()), &bike::changePower);
        disconnect(trainProgram, &trainprogram::changePower, ((rower *)bluetoothManager->device()),
                   &rower::changePower);
        disconnect(trainProgram, &trainprogram::changeSpeed, ((rower *)bluetoothManager->device()),
                   &rower::changeSpeed);
        disconnect(trainProgram, &trainprogram::changeCadence, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changeCadence);
        disconnect(trainProgram, &trainprogram::changePower, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changePower);
        disconnect(trainProgram, &trainprogram::changeInclination, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changeInclination);
        disconnect(trainProgram, &trainprogram::changeResistance, ((elliptical *)bluetoothManager->device()),
                   &elliptical::changeResistance);
        disconnect(trainProgram, &trainprogram::changeRequestedPelotonResistance,
                   ((elliptical *)bluetoothManager->device()), &elliptical::changeRequestedPelotonResistance);
        disconnect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                   &trainprogram::onTapeStarted);
        disconnect(((bike *)bluetoothManager->device()), &bike::bikeStarted, trainProgram,
                   &trainprogram::onTapeStarted);
        disconnect(trainProgram, &trainprogram::changeGeoPosition, bluetoothManager->device(),
                   &bluetoothdevice::changeGeoPosition);
        disconnect(this, &homeform::workoutEventStateChanged, bluetoothManager->device(),
                   &bluetoothdevice::workoutEventStateChanged);
        disconnect(trainProgram, &trainprogram::changeTimestamp, this, &homeform::changeTimestamp);
        disconnect(trainProgram, &trainprogram::toastRequest, this, &homeform::onToastRequested);
        disconnect(trainProgram, &trainprogram::zwiftLoginState, this, &homeform::zwiftLoginState);

        connect(trainProgram, &trainprogram::start, bluetoothManager->device(), &bluetoothdevice::start);
        connect(trainProgram, &trainprogram::stop, bluetoothManager->device(), &bluetoothdevice::stop);
        connect(trainProgram, &trainprogram::stop, this, &homeform::StopFromTrainProgram);
        connect(trainProgram, &trainprogram::lap, this, &homeform::Lap);
        connect(trainProgram, &trainprogram::toastRequest, this, &homeform::onToastRequested);
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            connect(trainProgram, &trainprogram::changeSpeed, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeSpeed);
            connect(trainProgram, &trainprogram::changeFanSpeed, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeFanSpeed);
            connect(trainProgram, &trainprogram::changeInclination, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeInclination);
            connect(trainProgram, &trainprogram::changeSpeedAndInclination, ((treadmill *)bluetoothManager->device()),
                    &treadmill::changeSpeedAndInclination);
            connect(((treadmill *)bluetoothManager->device()), &treadmill::tapeStarted, trainProgram,
                    &trainprogram::onTapeStarted);
            connect(trainProgram, &trainprogram::changePower, ((treadmill *)bluetoothManager->device()), &treadmill::changePower);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            connect(trainProgram, &trainprogram::changeCadence, ((bike *)bluetoothManager->device()),
                    &bike::changeCadence);
            connect(trainProgram, &trainprogram::changePower, ((bike *)bluetoothManager->device()), &bike::changePower);
            connect(trainProgram, &trainprogram::changeInclination, ((bike *)bluetoothManager->device()),
                    &bike::changeInclination);
            connect(trainProgram, &trainprogram::changeResistance, ((bike *)bluetoothManager->device()),
                    &bike::changeResistance);
            connect(trainProgram, &trainprogram::changeRequestedPelotonResistance, ((bike *)bluetoothManager->device()),
                    &bike::changeRequestedPelotonResistance);
            connect(((bike *)bluetoothManager->device()), &bike::bikeStarted, trainProgram,
                    &trainprogram::onTapeStarted);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
            connect(trainProgram, &trainprogram::changeCadence, ((elliptical *)bluetoothManager->device()),
                    &elliptical::changeCadence);
            connect(trainProgram, &trainprogram::changePower, ((elliptical *)bluetoothManager->device()),
                    &elliptical::changePower);
            connect(trainProgram, &trainprogram::changeInclination, ((elliptical *)bluetoothManager->device()),
                    &elliptical::changeInclination);
            connect(trainProgram, &trainprogram::changeResistance, ((elliptical *)bluetoothManager->device()),
                    &elliptical::changeResistance);
            connect(trainProgram, &trainprogram::changeRequestedPelotonResistance,
                    ((elliptical *)bluetoothManager->device()), &elliptical::changeRequestedPelotonResistance);
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            connect(trainProgram, &trainprogram::changePower, ((rower *)bluetoothManager->device()),
                    &rower::changePower);
            connect(trainProgram, &trainprogram::changeResistance, ((rower *)bluetoothManager->device()),
                    &rower::changeResistance);
            connect(trainProgram, &trainprogram::changeCadence, ((rower *)bluetoothManager->device()),
                    &rower::changeCadence);
            connect(trainProgram, &trainprogram::changeSpeed, ((rower *)bluetoothManager->device()),
                    &rower::changeSpeed);
        }
        connect(trainProgram, &trainprogram::changeNextInclination300Meters, bluetoothManager->device(),
                &bluetoothdevice::changeNextInclination300Meters);
        connect(trainProgram, &trainprogram::changeGeoPosition, bluetoothManager->device(),
                &bluetoothdevice::changeGeoPosition);
        connect(trainProgram, &trainprogram::changeTimestamp, this, &homeform::changeTimestamp);
        connect(this, &homeform::workoutEventStateChanged, bluetoothManager->device(),
                &bluetoothdevice::workoutEventStateChanged);
        connect(trainProgram, &trainprogram::zwiftLoginState, this, &homeform::zwiftLoginState);

        if (trainProgram) {
            setChartIconVisible(trainProgram->powerzoneWorkout());
            if (chartFooterVisible()) {
                if (trainProgram->powerzoneWorkout()) {
                    // reloading
                    setChartFooterVisible(false);
                    setChartFooterVisible(true);
                } else {
                    setChartFooterVisible(false);
                }
            }
        }

        qDebug() << QStringLiteral("trainProgram associated to a device");
    } else {
        qDebug() << QStringLiteral("trainProgram NOT associated to a device");
    }
}

void homeform::onToastRequested(QString message) {
    setToastRequested(message);
}

QStringList homeform::tile_order() {

    QStringList r;
    r.reserve(72);
    for (int i = 0; i < 73; i++) {
        r.append(QString::number(i));
    }
    return r;
}

// these events are coming from the SS2K, so when the auto resistance is off, this event shouldn't be processed
void homeform::gearUp() {
    if (autoResistance())
        Plus(QStringLiteral("gears"));
}

void homeform::gearDown() {
    if (autoResistance())
        Minus(QStringLiteral("gears"));
}

void homeform::ftmsAccessoryConnected(smartspin2k *d) {
    connect(this, &homeform::autoResistanceChanged, d, &smartspin2k::autoResistanceChanged);
    connect(d, &smartspin2k::gearUp, this, &homeform::gearUp);
    connect(d, &smartspin2k::gearDown, this, &homeform::gearDown);
}

void homeform::sortTiles() {

    QSettings settings;
    bool pelotoncadence =
        settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();

    if (!bluetoothManager || !bluetoothManager->device())
        return;

    dataList.clear();

    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QZSettings::tile_speed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_speed_order, 0).toInt() == i) {

                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QZSettings::tile_inclination_enabled, true).toBool() &&
                settings.value(QZSettings::tile_inclination_order, 0).toInt() == i) {
                inclination->setGridId(i);
                dataList.append(inclination);
            }

            if (settings.value(QZSettings::tile_elevation_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elevation_order, 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QZSettings::tile_elapsed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elapsed_order, 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QZSettings::tile_moving_time_enabled, false).toBool() &&
                settings.value(QZSettings::tile_moving_time_order, 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QZSettings::tile_peloton_offset_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_offset_order, 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QZSettings::tile_peloton_remaining_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_remaining_order, 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QZSettings::tile_calories_enabled, true).toBool() &&
                settings.value(QZSettings::tile_calories_order, 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QZSettings::tile_odometer_enabled, true).toBool() &&
                settings.value(QZSettings::tile_odometer_order, 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QZSettings::tile_pace_enabled, true).toBool() &&
                settings.value(QZSettings::tile_pace_order, 0).toInt() == i) {
                pace->setGridId(i);
                dataList.append(pace);
            }

            if (settings.value(QZSettings::tile_watt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_watt_order, 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QZSettings::tile_weight_loss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_weight_loss_order, 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QZSettings::tile_avgwatt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avgwatt_order, 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QZSettings::tile_avg_watt_lap_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avg_watt_lap_order, 0).toInt() == i) {
                avgWattLap->setGridId(i);
                dataList.append(avgWattLap);
            }

            if (settings.value(QZSettings::tile_ftp_enabled, true).toBool() &&
                settings.value(QZSettings::tile_ftp_order, 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QZSettings::tile_jouls_enabled, true).toBool() &&
                settings.value(QZSettings::tile_jouls_order, 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QZSettings::tile_heart_enabled, true).toBool() &&
                settings.value(QZSettings::tile_heart_order, 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QZSettings::tile_fan_enabled, true).toBool() &&
                settings.value(QZSettings::tile_fan_order, 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QZSettings::tile_datetime_enabled, true).toBool() &&
                settings.value(QZSettings::tile_datetime_order, 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QZSettings::tile_lapelapsed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_lapelapsed_order, 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QZSettings::tile_watt_kg_enabled, false).toBool() &&
                settings.value(QZSettings::tile_watt_kg_order, 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QZSettings::tile_remainingtimetrainprogramrow_enabled, false).toBool() &&
                settings.value(QZSettings::tile_remainingtimetrainprogramrow_order, 27).toInt() == i) {

                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QZSettings::tile_nextrowstrainprogram_enabled, false).toBool() &&
                settings.value(QZSettings::tile_nextrowstrainprogram_order, 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QZSettings::tile_mets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_mets_order, 28).toInt() == i) {

                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QZSettings::tile_targetmets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_targetmets_order, 29).toInt() == i) {

                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QZSettings::tile_target_speed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_speed_order, 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }

            if (settings.value(QZSettings::tile_target_incline_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_incline_order, 29).toInt() == i) {
                target_incline->setGridId(i);
                dataList.append(target_incline);
            }

            if (settings.value(QZSettings::tile_cadence_enabled, false).toBool() &&
                settings.value(QZSettings::tile_cadence_order, 30).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QZSettings::tile_pid_hr_enabled, false).toBool() &&
                settings.value(QZSettings::tile_pid_hr_order, 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QZSettings::tile_instantaneous_stride_length_enabled, false).toBool() &&
                settings.value(QZSettings::tile_instantaneous_stride_length_order, 32).toInt() == i) {
                instantaneousStrideLengthCM->setGridId(i);
                dataList.append(instantaneousStrideLengthCM);
            }

            if (settings.value(QZSettings::tile_ground_contact_enabled, false).toBool() &&
                settings.value(QZSettings::tile_ground_contact_order, 33).toInt() == i) {
                groundContactMS->setGridId(i);
                dataList.append(groundContactMS);
            }

            if (settings.value(QZSettings::tile_vertical_oscillation_enabled, false).toBool() &&
                settings.value(QZSettings::tile_vertical_oscillation_order, 34).toInt() == i) {
                verticalOscillationMM->setGridId(i);
                dataList.append(verticalOscillationMM);
            }

            if (settings.value(QZSettings::tile_preset_speed_1_enabled, QZSettings::default_tile_preset_speed_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_1_order, QZSettings::default_tile_preset_speed_1_order)
                        .toInt() == i) {
                preset_speed_1->setGridId(i);
                dataList.append(preset_speed_1);
            }
            if (settings.value(QZSettings::tile_preset_speed_2_enabled, QZSettings::default_tile_preset_speed_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_2_order, QZSettings::default_tile_preset_speed_2_order)
                        .toInt() == i) {
                preset_speed_2->setGridId(i);
                dataList.append(preset_speed_2);
            }
            if (settings.value(QZSettings::tile_preset_speed_3_enabled, QZSettings::default_tile_preset_speed_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_3_order, QZSettings::default_tile_preset_speed_3_order)
                        .toInt() == i) {
                preset_speed_3->setGridId(i);
                dataList.append(preset_speed_3);
            }
            if (settings.value(QZSettings::tile_preset_speed_4_enabled, QZSettings::default_tile_preset_speed_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_4_order, QZSettings::default_tile_preset_speed_4_order)
                        .toInt() == i) {
                preset_speed_4->setGridId(i);
                dataList.append(preset_speed_4);
            }
            if (settings.value(QZSettings::tile_preset_speed_5_enabled, QZSettings::default_tile_preset_speed_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_5_order, QZSettings::default_tile_preset_speed_5_order)
                        .toInt() == i) {
                preset_speed_5->setGridId(i);
                dataList.append(preset_speed_5);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_1_enabled,
                           QZSettings::default_tile_preset_inclination_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_1_order,
                               QZSettings::default_tile_preset_inclination_1_order)
                        .toInt() == i) {
                preset_inclination_1->setGridId(i);
                dataList.append(preset_inclination_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_2_enabled,
                           QZSettings::default_tile_preset_inclination_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_2_order,
                               QZSettings::default_tile_preset_inclination_2_order)
                        .toInt() == i) {
                preset_inclination_2->setGridId(i);
                dataList.append(preset_inclination_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_3_enabled,
                           QZSettings::default_tile_preset_inclination_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_3_order,
                               QZSettings::default_tile_preset_inclination_3_order)
                        .toInt() == i) {
                preset_inclination_3->setGridId(i);
                dataList.append(preset_inclination_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_4_enabled,
                           QZSettings::default_tile_preset_inclination_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_4_order,
                               QZSettings::default_tile_preset_inclination_4_order)
                        .toInt() == i) {
                preset_inclination_4->setGridId(i);
                dataList.append(preset_inclination_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_5_enabled,
                           QZSettings::default_tile_preset_inclination_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_5_order,
                               QZSettings::default_tile_preset_inclination_5_order)
                        .toInt() == i) {
                preset_inclination_5->setGridId(i);
                dataList.append(preset_inclination_5);
            }

            if (settings.value(QZSettings::tile_target_pace_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_pace_order, 50).toInt() == i) {
                target_pace->setGridId(i);
                dataList.append(target_pace);
            }

            if (settings.value(QZSettings::tile_step_count_enabled, QZSettings::default_tile_step_count_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_step_count_order, QZSettings::default_tile_step_count_order)
                        .toInt() == i) {

                stepCount->setGridId(i);
                dataList.append(stepCount);
            }            

            if (settings.value(QZSettings::tile_rss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_rss_order, 53).toInt() == i) {
                rss->setGridId(i);
                dataList.append(rss);
            }

            if (settings.value(QZSettings::tile_target_power_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_power_order, 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QZSettings::tile_target_zone_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_zone_order, 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_1_order, 0).toInt() == i) {
                tile_hr_time_in_zone_1->setGridId(i);
                dataList.append(tile_hr_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_2_order, 0).toInt() == i) {
                tile_hr_time_in_zone_2->setGridId(i);
                dataList.append(tile_hr_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_3_order, 0).toInt() == i) {
                tile_hr_time_in_zone_3->setGridId(i);
                dataList.append(tile_hr_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_4_order, 0).toInt() == i) {
                tile_hr_time_in_zone_4->setGridId(i);
                dataList.append(tile_hr_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_5_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_5_order, 0).toInt() == i) {
                tile_hr_time_in_zone_5->setGridId(i);
                dataList.append(tile_hr_time_in_zone_5);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_1_order, 0).toInt() == i) {
                tile_heat_time_in_zone_1->setGridId(i);
                dataList.append(tile_heat_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_2_order, 0).toInt() == i) {
                tile_heat_time_in_zone_2->setGridId(i);
                dataList.append(tile_heat_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_3_order, 0).toInt() == i) {
                tile_heat_time_in_zone_3->setGridId(i);
                dataList.append(tile_heat_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_4_order, 0).toInt() == i) {
                tile_heat_time_in_zone_4->setGridId(i);
                dataList.append(tile_heat_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled).toBool() &&
                settings.value(QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order).toInt() == i) {
                coreTemperature->setGridId(i);
                dataList.append(coreTemperature);
            }
        }
    } else     if (bluetoothManager->device()->deviceType() == bluetoothdevice::STAIRCLIMBER) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QZSettings::tile_speed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_speed_order, 0).toInt() == i) {

                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QZSettings::tile_inclination_enabled, true).toBool() &&
                settings.value(QZSettings::tile_inclination_order, 0).toInt() == i) {
                inclination->setGridId(i);
                dataList.append(inclination);
            }

            if (settings.value(QZSettings::tile_elevation_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elevation_order, 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QZSettings::tile_elapsed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elapsed_order, 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QZSettings::tile_moving_time_enabled, false).toBool() &&
                settings.value(QZSettings::tile_moving_time_order, 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QZSettings::tile_peloton_offset_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_offset_order, 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QZSettings::tile_peloton_remaining_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_remaining_order, 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QZSettings::tile_calories_enabled, true).toBool() &&
                settings.value(QZSettings::tile_calories_order, 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QZSettings::tile_odometer_enabled, true).toBool() &&
                settings.value(QZSettings::tile_odometer_order, 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QZSettings::tile_pace_enabled, true).toBool() &&
                settings.value(QZSettings::tile_pace_order, 0).toInt() == i) {
                pace->setGridId(i);
                dataList.append(pace);
            }

            if (settings.value(QZSettings::tile_watt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_watt_order, 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QZSettings::tile_weight_loss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_weight_loss_order, 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QZSettings::tile_avgwatt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avgwatt_order, 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QZSettings::tile_avg_watt_lap_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avg_watt_lap_order, 0).toInt() == i) {
                avgWattLap->setGridId(i);
                dataList.append(avgWattLap);
            }

            if (settings.value(QZSettings::tile_ftp_enabled, true).toBool() &&
                settings.value(QZSettings::tile_ftp_order, 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QZSettings::tile_jouls_enabled, true).toBool() &&
                settings.value(QZSettings::tile_jouls_order, 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QZSettings::tile_heart_enabled, true).toBool() &&
                settings.value(QZSettings::tile_heart_order, 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QZSettings::tile_fan_enabled, true).toBool() &&
                settings.value(QZSettings::tile_fan_order, 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QZSettings::tile_datetime_enabled, true).toBool() &&
                settings.value(QZSettings::tile_datetime_order, 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QZSettings::tile_lapelapsed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_lapelapsed_order, 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QZSettings::tile_watt_kg_enabled, false).toBool() &&
                settings.value(QZSettings::tile_watt_kg_order, 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QZSettings::tile_remainingtimetrainprogramrow_enabled, false).toBool() &&
                settings.value(QZSettings::tile_remainingtimetrainprogramrow_order, 27).toInt() == i) {

                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QZSettings::tile_nextrowstrainprogram_enabled, false).toBool() &&
                settings.value(QZSettings::tile_nextrowstrainprogram_order, 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QZSettings::tile_mets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_mets_order, 28).toInt() == i) {

                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QZSettings::tile_targetmets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_targetmets_order, 29).toInt() == i) {

                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QZSettings::tile_target_speed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_speed_order, 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }

            if (settings.value(QZSettings::tile_target_incline_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_incline_order, 29).toInt() == i) {
                target_incline->setGridId(i);
                dataList.append(target_incline);
            }

            if (settings.value(QZSettings::tile_cadence_enabled, false).toBool() &&
                settings.value(QZSettings::tile_cadence_order, 30).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QZSettings::tile_pid_hr_enabled, false).toBool() &&
                settings.value(QZSettings::tile_pid_hr_order, 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QZSettings::tile_instantaneous_stride_length_enabled, false).toBool() &&
                settings.value(QZSettings::tile_instantaneous_stride_length_order, 32).toInt() == i) {
                instantaneousStrideLengthCM->setGridId(i);
                dataList.append(instantaneousStrideLengthCM);
            }

            if (settings.value(QZSettings::tile_ground_contact_enabled, false).toBool() &&
                settings.value(QZSettings::tile_ground_contact_order, 33).toInt() == i) {
                groundContactMS->setGridId(i);
                dataList.append(groundContactMS);
            }

            if (settings.value(QZSettings::tile_vertical_oscillation_enabled, false).toBool() &&
                settings.value(QZSettings::tile_vertical_oscillation_order, 34).toInt() == i) {
                verticalOscillationMM->setGridId(i);
                dataList.append(verticalOscillationMM);
            }

            if (settings.value(QZSettings::tile_preset_speed_1_enabled, QZSettings::default_tile_preset_speed_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_1_order, QZSettings::default_tile_preset_speed_1_order)
                        .toInt() == i) {
                preset_speed_1->setGridId(i);
                dataList.append(preset_speed_1);
            }
            if (settings.value(QZSettings::tile_preset_speed_2_enabled, QZSettings::default_tile_preset_speed_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_2_order, QZSettings::default_tile_preset_speed_2_order)
                        .toInt() == i) {
                preset_speed_2->setGridId(i);
                dataList.append(preset_speed_2);
            }
            if (settings.value(QZSettings::tile_preset_speed_3_enabled, QZSettings::default_tile_preset_speed_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_3_order, QZSettings::default_tile_preset_speed_3_order)
                        .toInt() == i) {
                preset_speed_3->setGridId(i);
                dataList.append(preset_speed_3);
            }
            if (settings.value(QZSettings::tile_preset_speed_4_enabled, QZSettings::default_tile_preset_speed_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_4_order, QZSettings::default_tile_preset_speed_4_order)
                        .toInt() == i) {
                preset_speed_4->setGridId(i);
                dataList.append(preset_speed_4);
            }
            if (settings.value(QZSettings::tile_preset_speed_5_enabled, QZSettings::default_tile_preset_speed_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_speed_5_order, QZSettings::default_tile_preset_speed_5_order)
                        .toInt() == i) {
                preset_speed_5->setGridId(i);
                dataList.append(preset_speed_5);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_1_enabled,
                           QZSettings::default_tile_preset_inclination_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_1_order,
                               QZSettings::default_tile_preset_inclination_1_order)
                        .toInt() == i) {
                preset_inclination_1->setGridId(i);
                dataList.append(preset_inclination_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_2_enabled,
                           QZSettings::default_tile_preset_inclination_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_2_order,
                               QZSettings::default_tile_preset_inclination_2_order)
                        .toInt() == i) {
                preset_inclination_2->setGridId(i);
                dataList.append(preset_inclination_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_3_enabled,
                           QZSettings::default_tile_preset_inclination_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_3_order,
                               QZSettings::default_tile_preset_inclination_3_order)
                        .toInt() == i) {
                preset_inclination_3->setGridId(i);
                dataList.append(preset_inclination_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_4_enabled,
                           QZSettings::default_tile_preset_inclination_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_4_order,
                               QZSettings::default_tile_preset_inclination_4_order)
                        .toInt() == i) {
                preset_inclination_4->setGridId(i);
                dataList.append(preset_inclination_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_5_enabled,
                           QZSettings::default_tile_preset_inclination_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_5_order,
                               QZSettings::default_tile_preset_inclination_5_order)
                        .toInt() == i) {
                preset_inclination_5->setGridId(i);
                dataList.append(preset_inclination_5);
            }

            if (settings.value(QZSettings::tile_target_pace_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_pace_order, 50).toInt() == i) {
                target_pace->setGridId(i);
                dataList.append(target_pace);
            }

            if (settings.value(QZSettings::tile_step_count_enabled, QZSettings::default_tile_step_count_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_step_count_order, QZSettings::default_tile_step_count_order)
                        .toInt() == i) {

                stepCount->setGridId(i);
                dataList.append(stepCount);
            }

            if (settings.value(QZSettings::tile_rss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_rss_order, 53).toInt() == i) {
                rss->setGridId(i);
                dataList.append(rss);
            }

            if (settings.value(QZSettings::tile_target_power_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_power_order, 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QZSettings::tile_target_zone_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_zone_order, 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_1_order, 0).toInt() == i) {
                tile_hr_time_in_zone_1->setGridId(i);
                dataList.append(tile_hr_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_2_order, 0).toInt() == i) {
                tile_hr_time_in_zone_2->setGridId(i);
                dataList.append(tile_hr_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_3_order, 0).toInt() == i) {
                tile_hr_time_in_zone_3->setGridId(i);
                dataList.append(tile_hr_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_4_order, 0).toInt() == i) {
                tile_hr_time_in_zone_4->setGridId(i);
                dataList.append(tile_hr_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_5_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_5_order, 0).toInt() == i) {
                tile_hr_time_in_zone_5->setGridId(i);
                dataList.append(tile_hr_time_in_zone_5);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_1_order, 0).toInt() == i) {
                tile_heat_time_in_zone_1->setGridId(i);
                dataList.append(tile_heat_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_2_order, 0).toInt() == i) {
                tile_heat_time_in_zone_2->setGridId(i);
                dataList.append(tile_heat_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_3_order, 0).toInt() == i) {
                tile_heat_time_in_zone_3->setGridId(i);
                dataList.append(tile_heat_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_4_order, 0).toInt() == i) {
                tile_heat_time_in_zone_4->setGridId(i);
                dataList.append(tile_heat_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled).toBool() &&
                settings.value(QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order).toInt() == i) {
                coreTemperature->setGridId(i);
                dataList.append(coreTemperature);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QZSettings::tile_speed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_speed_order, 0).toInt() == i) {

                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QZSettings::tile_cadence_enabled, true).toBool() &&
                settings.value(QZSettings::tile_cadence_order, 0).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QZSettings::tile_elevation_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elevation_order, 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QZSettings::tile_elapsed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elapsed_order, 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QZSettings::tile_moving_time_enabled, false).toBool() &&
                settings.value(QZSettings::tile_moving_time_order, 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QZSettings::tile_peloton_offset_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_offset_order, 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QZSettings::tile_peloton_remaining_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_remaining_order, 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QZSettings::tile_calories_enabled, true).toBool() &&
                settings.value(QZSettings::tile_calories_order, 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QZSettings::tile_odometer_enabled, true).toBool() &&
                settings.value(QZSettings::tile_odometer_order, 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QZSettings::tile_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_resistance_order, 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QZSettings::tile_peloton_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_peloton_resistance_order, 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QZSettings::tile_watt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_watt_order, 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QZSettings::tile_weight_loss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_weight_loss_order, 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QZSettings::tile_avgwatt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avgwatt_order, 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QZSettings::tile_avg_watt_lap_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avg_watt_lap_order, 0).toInt() == i) {
                avgWattLap->setGridId(i);
                dataList.append(avgWattLap);
            }

            if (settings.value(QZSettings::tile_ftp_enabled, true).toBool() &&
                settings.value(QZSettings::tile_ftp_order, 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QZSettings::tile_jouls_enabled, true).toBool() &&
                settings.value(QZSettings::tile_jouls_order, 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QZSettings::tile_heart_enabled, true).toBool() &&
                settings.value(QZSettings::tile_heart_order, 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QZSettings::tile_fan_enabled, true).toBool() &&
                settings.value(QZSettings::tile_fan_order, 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QZSettings::tile_datetime_enabled, true).toBool() &&
                settings.value(QZSettings::tile_datetime_order, 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QZSettings::tile_target_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_target_resistance_order, 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QZSettings::tile_target_peloton_resistance_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_peloton_resistance_order, 21).toInt() == i) {
                target_peloton_resistance->setGridId(i);
                dataList.append(target_peloton_resistance);
            }

            if (settings.value(QZSettings::tile_target_cadence_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_cadence_order, 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QZSettings::tile_target_power_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_power_order, 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QZSettings::tile_target_zone_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_zone_order, 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }

            if (settings.value(QZSettings::tile_lapelapsed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_lapelapsed_order, 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QZSettings::tile_watt_kg_enabled, false).toBool() &&
                settings.value(QZSettings::tile_watt_kg_order, 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }
            if (settings.value(QZSettings::tile_gears_enabled, false).toBool() &&
                settings.value(QZSettings::tile_gears_order, 25).toInt() == i) {
                gears->setGridId(i);
                dataList.append(gears);
            }

            if (settings.value(QZSettings::tile_remainingtimetrainprogramrow_enabled, false).toBool() &&
                settings.value(QZSettings::tile_remainingtimetrainprogramrow_order, 27).toInt() == i) {

                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QZSettings::tile_nextrowstrainprogram_enabled, false).toBool() &&
                settings.value(QZSettings::tile_nextrowstrainprogram_order, 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QZSettings::tile_mets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_mets_order, 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QZSettings::tile_targetmets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_targetmets_order, 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }
            // the proform studio is the only bike managed with an inclination properties.
            // In order to don't break the tiles layout to all the bikes users, i enable this
            // only if this bike is selected
            // since i'm adding the inclination from zwift in this tile, in order to preserve the
            // layour for legacy users, i'm not showing this one if the peloton cadence sensor setting
            // is enabled (assuming that if someone has it, he doesn't want an inclination tile)
            if (!pelotoncadence) {
                if (settings.value(QZSettings::tile_inclination_enabled, true).toBool() &&
                    settings.value(QZSettings::tile_inclination_order, 29).toInt() == i) {
                    inclination->setGridId(i);
                    dataList.append(inclination);
                }
            }
            if (settings.value(QZSettings::tile_steering_angle_enabled, false).toBool() &&
                settings.value(QZSettings::tile_steering_angle_order, 30).toInt() == i) {

                steeringAngle->setGridId(i);
                dataList.append(steeringAngle);
            }

            if (settings.value(QZSettings::tile_pid_hr_enabled, false).toBool() &&
                settings.value(QZSettings::tile_pid_hr_order, 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QZSettings::tile_ext_incline_enabled, false).toBool() &&
                settings.value(QZSettings::tile_ext_incline_order, 32).toInt() == i) {
                extIncline->setGridId(i);
                dataList.append(extIncline);
            }

            if (settings
                    .value(QZSettings::tile_preset_inclination_1_enabled,
                           QZSettings::default_tile_preset_inclination_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_1_order,
                               QZSettings::default_tile_preset_inclination_1_order)
                        .toInt() == i) {
                preset_inclination_1->setGridId(i);
                dataList.append(preset_inclination_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_2_enabled,
                           QZSettings::default_tile_preset_inclination_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_2_order,
                               QZSettings::default_tile_preset_inclination_2_order)
                        .toInt() == i) {
                preset_inclination_2->setGridId(i);
                dataList.append(preset_inclination_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_3_enabled,
                           QZSettings::default_tile_preset_inclination_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_3_order,
                               QZSettings::default_tile_preset_inclination_3_order)
                        .toInt() == i) {
                preset_inclination_3->setGridId(i);
                dataList.append(preset_inclination_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_4_enabled,
                           QZSettings::default_tile_preset_inclination_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_4_order,
                               QZSettings::default_tile_preset_inclination_4_order)
                        .toInt() == i) {
                preset_inclination_4->setGridId(i);
                dataList.append(preset_inclination_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_5_enabled,
                           QZSettings::default_tile_preset_inclination_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_5_order,
                               QZSettings::default_tile_preset_inclination_5_order)
                        .toInt() == i) {
                preset_inclination_5->setGridId(i);
                dataList.append(preset_inclination_5);
            }

            if (settings
                    .value(QZSettings::tile_preset_resistance_1_enabled,
                           QZSettings::default_tile_preset_resistance_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_1_order,
                               QZSettings::default_tile_preset_resistance_1_order)
                        .toInt() == i) {
                preset_resistance_1->setGridId(i);
                dataList.append(preset_resistance_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_2_enabled,
                           QZSettings::default_tile_preset_resistance_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_2_order,
                               QZSettings::default_tile_preset_resistance_2_order)
                        .toInt() == i) {
                preset_resistance_2->setGridId(i);
                dataList.append(preset_resistance_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_3_enabled,
                           QZSettings::default_tile_preset_resistance_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_3_order,
                               QZSettings::default_tile_preset_resistance_3_order)
                        .toInt() == i) {
                preset_resistance_3->setGridId(i);
                dataList.append(preset_resistance_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_4_enabled,
                           QZSettings::default_tile_preset_resistance_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_4_order,
                               QZSettings::default_tile_preset_resistance_4_order)
                        .toInt() == i) {
                preset_resistance_4->setGridId(i);
                dataList.append(preset_resistance_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_5_enabled,
                           QZSettings::default_tile_preset_resistance_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_5_order,
                               QZSettings::default_tile_preset_resistance_5_order)
                        .toInt() == i) {
                preset_resistance_5->setGridId(i);
                dataList.append(preset_resistance_5);
            }
            if (settings.value(QZSettings::tile_erg_mode_enabled, QZSettings::default_tile_erg_mode_enabled).toBool() &&
                settings.value(QZSettings::tile_erg_mode_order, QZSettings::default_tile_erg_mode_order).toInt() == i) {
                ergMode->setGridId(i);
                dataList.append(ergMode);
            }

            if (settings.value(QZSettings::tile_biggears_enabled, false).toBool() &&
                settings.value(QZSettings::tile_biggears_order, 54).toInt() == i + (settings.value(QZSettings::tile_biggears_swap, QZSettings::default_tile_biggears_swap).toBool() ? 1 : 0)) {
                biggearsPlus->setGridId(i);
                dataList.append(biggearsPlus);
            }

            if (settings.value(QZSettings::tile_biggears_enabled, false).toBool() &&
                settings.value(QZSettings::tile_biggears_order, 54).toInt() == i + (settings.value(QZSettings::tile_biggears_swap, QZSettings::default_tile_biggears_swap).toBool() ? 0 : 1)) {
                biggearsMinus->setGridId(i);
                dataList.append(biggearsMinus);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_1_enabled, QZSettings::default_tile_preset_powerzone_1_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_1_order, QZSettings::default_tile_preset_powerzone_1_order).toInt() == i) {
                preset_powerzone_1->setGridId(i);
                dataList.append(preset_powerzone_1);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_2_enabled, QZSettings::default_tile_preset_powerzone_2_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_2_order, QZSettings::default_tile_preset_powerzone_2_order).toInt() == i) {
                preset_powerzone_2->setGridId(i);
                dataList.append(preset_powerzone_2);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_3_enabled, QZSettings::default_tile_preset_powerzone_3_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_3_order, QZSettings::default_tile_preset_powerzone_3_order).toInt() == i) {
                preset_powerzone_3->setGridId(i);
                dataList.append(preset_powerzone_3);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_4_enabled, QZSettings::default_tile_preset_powerzone_4_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_4_order, QZSettings::default_tile_preset_powerzone_4_order).toInt() == i) {
                preset_powerzone_4->setGridId(i);
                dataList.append(preset_powerzone_4);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_5_enabled, QZSettings::default_tile_preset_powerzone_5_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_5_order, QZSettings::default_tile_preset_powerzone_5_order).toInt() == i) {
                preset_powerzone_5->setGridId(i);
                dataList.append(preset_powerzone_5);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_6_enabled, QZSettings::default_tile_preset_powerzone_6_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_6_order, QZSettings::default_tile_preset_powerzone_6_order).toInt() == i) {
                preset_powerzone_6->setGridId(i);
                dataList.append(preset_powerzone_6);
            }

            if (settings.value(QZSettings::tile_preset_powerzone_7_enabled, QZSettings::default_tile_preset_powerzone_7_enabled).toBool() &&
                settings.value(QZSettings::tile_preset_powerzone_7_order, QZSettings::default_tile_preset_powerzone_7_order).toInt() == i) {
                preset_powerzone_7->setGridId(i);
                dataList.append(preset_powerzone_7);
            }


            if (settings.value(QZSettings::tile_hr_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_1_order, 0).toInt() == i) {
                tile_hr_time_in_zone_1->setGridId(i);
                dataList.append(tile_hr_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_2_order, 0).toInt() == i) {
                tile_hr_time_in_zone_2->setGridId(i);
                dataList.append(tile_hr_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_3_order, 0).toInt() == i) {
                tile_hr_time_in_zone_3->setGridId(i);
                dataList.append(tile_hr_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_4_order, 0).toInt() == i) {
                tile_hr_time_in_zone_4->setGridId(i);
                dataList.append(tile_hr_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_5_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_5_order, 0).toInt() == i) {
                tile_hr_time_in_zone_5->setGridId(i);
                dataList.append(tile_hr_time_in_zone_5);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_1_order, 0).toInt() == i) {
                tile_heat_time_in_zone_1->setGridId(i);
                dataList.append(tile_heat_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_2_order, 0).toInt() == i) {
                tile_heat_time_in_zone_2->setGridId(i);
                dataList.append(tile_heat_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_3_order, 0).toInt() == i) {
                tile_heat_time_in_zone_3->setGridId(i);
                dataList.append(tile_heat_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_4_order, 0).toInt() == i) {
                tile_heat_time_in_zone_4->setGridId(i);
                dataList.append(tile_heat_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled).toBool() &&
                settings.value(QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order).toInt() == i) {
                coreTemperature->setGridId(i);
                dataList.append(coreTemperature);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QZSettings::tile_speed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_speed_order, 0).toInt() == i) {
                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QZSettings::tile_cadence_enabled, true).toBool() &&
                settings.value(QZSettings::tile_cadence_order, 0).toInt() == i) {
                cadence->setGridId(i);
                cadence->setName("Stroke Rate");
                dataList.append(cadence);
            }

            if (settings.value(QZSettings::tile_elevation_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elevation_order, 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QZSettings::tile_elapsed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elapsed_order, 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QZSettings::tile_moving_time_enabled, false).toBool() &&
                settings.value(QZSettings::tile_moving_time_order, 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QZSettings::tile_peloton_offset_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_offset_order, 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QZSettings::tile_peloton_remaining_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_remaining_order, 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QZSettings::tile_calories_enabled, true).toBool() &&
                settings.value(QZSettings::tile_calories_order, 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QZSettings::tile_odometer_enabled, true).toBool() &&
                settings.value(QZSettings::tile_odometer_order, 0).toInt() == i) {
                odometer->setGridId(i);
                odometer->setName("Odometer (m)");
                dataList.append(odometer);
            }

            if (settings.value(QZSettings::tile_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_resistance_order, 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QZSettings::tile_peloton_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_peloton_resistance_order, 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QZSettings::tile_watt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_watt_order, 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QZSettings::tile_weight_loss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_weight_loss_order, 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QZSettings::tile_avgwatt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avgwatt_order, 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QZSettings::tile_avg_watt_lap_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avg_watt_lap_order, 0).toInt() == i) {
                avgWattLap->setGridId(i);
                dataList.append(avgWattLap);
            }

            if (settings.value(QZSettings::tile_ftp_enabled, true).toBool() &&
                settings.value(QZSettings::tile_ftp_order, 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QZSettings::tile_jouls_enabled, true).toBool() &&
                settings.value(QZSettings::tile_jouls_order, 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QZSettings::tile_heart_enabled, true).toBool() &&
                settings.value(QZSettings::tile_heart_order, 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QZSettings::tile_fan_enabled, true).toBool() &&
                settings.value(QZSettings::tile_fan_order, 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QZSettings::tile_datetime_enabled, true).toBool() &&
                settings.value(QZSettings::tile_datetime_order, 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QZSettings::tile_target_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_target_resistance_order, 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QZSettings::tile_target_peloton_resistance_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_peloton_resistance_order, 21).toInt() == i) {
                target_peloton_resistance->setGridId(i);
                dataList.append(target_peloton_resistance);
            }

            if (settings.value(QZSettings::tile_target_cadence_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_cadence_order, 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QZSettings::tile_target_power_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_power_order, 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QZSettings::tile_lapelapsed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_lapelapsed_order, 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QZSettings::tile_strokes_length_enabled, false).toBool() &&
                settings.value(QZSettings::tile_strokes_length_order, 21).toInt() == i) {
                strokesLength->setGridId(i);
                dataList.append(strokesLength);
            }

            if (settings.value(QZSettings::tile_strokes_count_enabled, false).toBool() &&
                settings.value(QZSettings::tile_strokes_count_order, 22).toInt() == i) {
                strokesCount->setGridId(i);
                dataList.append(strokesCount);
            }

            if (settings.value(QZSettings::tile_pace_enabled, true).toBool() &&
                settings.value(QZSettings::tile_pace_order, 0).toInt() == i) {
                pace->setGridId(i);
                pace->setName("Pace (m/500m)");
                dataList.append(pace);
            }

            if (settings.value(QZSettings::tile_watt_kg_enabled, false).toBool() &&
                settings.value(QZSettings::tile_watt_kg_order, 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QZSettings::tile_remainingtimetrainprogramrow_enabled, false).toBool() &&
                settings.value(QZSettings::tile_remainingtimetrainprogramrow_order, 27).toInt() == i) {
                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QZSettings::tile_nextrowstrainprogram_enabled, false).toBool() &&
                settings.value(QZSettings::tile_nextrowstrainprogram_order, 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QZSettings::tile_mets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_mets_order, 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QZSettings::tile_targetmets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_targetmets_order, 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QZSettings::tile_pid_hr_enabled, false).toBool() &&
                settings.value(QZSettings::tile_pid_hr_order, 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QZSettings::tile_target_zone_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_zone_order, 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }

            if (settings.value(QZSettings::tile_pace_last500m_enabled, QZSettings::default_tile_pace_last500m_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_pace_last500m_order, QZSettings::default_tile_pace_last500m_order)
                        .toInt() == i) {

                pace_last500m->setGridId(i);
                dataList.append(pace_last500m);
            }

            if (settings.value(QZSettings::tile_target_speed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_speed_order, 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }

            if (settings.value(QZSettings::tile_target_pace_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_pace_order, 50).toInt() == i) {
                target_pace->setGridId(i);
                target_pace->setName("T.Pace(m/500m)");
                dataList.append(target_pace);
            }

            if (settings
                    .value(QZSettings::tile_preset_resistance_1_enabled,
                           QZSettings::default_tile_preset_resistance_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_1_order,
                               QZSettings::default_tile_preset_resistance_1_order)
                        .toInt() == i) {
                preset_resistance_1->setGridId(i);
                dataList.append(preset_resistance_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_2_enabled,
                           QZSettings::default_tile_preset_resistance_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_2_order,
                               QZSettings::default_tile_preset_resistance_2_order)
                        .toInt() == i) {
                preset_resistance_2->setGridId(i);
                dataList.append(preset_resistance_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_3_enabled,
                           QZSettings::default_tile_preset_resistance_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_3_order,
                               QZSettings::default_tile_preset_resistance_3_order)
                        .toInt() == i) {
                preset_resistance_3->setGridId(i);
                dataList.append(preset_resistance_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_4_enabled,
                           QZSettings::default_tile_preset_resistance_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_4_order,
                               QZSettings::default_tile_preset_resistance_4_order)
                        .toInt() == i) {
                preset_resistance_4->setGridId(i);
                dataList.append(preset_resistance_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_5_enabled,
                           QZSettings::default_tile_preset_resistance_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_5_order,
                               QZSettings::default_tile_preset_resistance_5_order)
                        .toInt() == i) {
                preset_resistance_5->setGridId(i);
                dataList.append(preset_resistance_5);
            }
            if (settings.value(QZSettings::tile_gears_enabled, false).toBool() &&
                settings.value(QZSettings::tile_gears_order, 51).toInt() == i) {
                gears->setGridId(i);
                dataList.append(gears);
            }


            if (settings.value(QZSettings::tile_hr_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_1_order, 0).toInt() == i) {
                tile_hr_time_in_zone_1->setGridId(i);
                dataList.append(tile_hr_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_2_order, 0).toInt() == i) {
                tile_hr_time_in_zone_2->setGridId(i);
                dataList.append(tile_hr_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_3_order, 0).toInt() == i) {
                tile_hr_time_in_zone_3->setGridId(i);
                dataList.append(tile_hr_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_4_order, 0).toInt() == i) {
                tile_hr_time_in_zone_4->setGridId(i);
                dataList.append(tile_hr_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_5_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_5_order, 0).toInt() == i) {
                tile_hr_time_in_zone_5->setGridId(i);
                dataList.append(tile_hr_time_in_zone_5);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_1_order, 0).toInt() == i) {
                tile_heat_time_in_zone_1->setGridId(i);
                dataList.append(tile_heat_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_2_order, 0).toInt() == i) {
                tile_heat_time_in_zone_2->setGridId(i);
                dataList.append(tile_heat_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_3_order, 0).toInt() == i) {
                tile_heat_time_in_zone_3->setGridId(i);
                dataList.append(tile_heat_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_4_order, 0).toInt() == i) {
                tile_heat_time_in_zone_4->setGridId(i);
                dataList.append(tile_heat_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled).toBool() &&
                settings.value(QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order).toInt() == i) {
                coreTemperature->setGridId(i);
                dataList.append(coreTemperature);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::JUMPROPE) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QZSettings::tile_speed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_speed_order, 0).toInt() == i) {
                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QZSettings::tile_cadence_enabled, true).toBool() &&
                settings.value(QZSettings::tile_cadence_order, 0).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QZSettings::tile_elevation_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elevation_order, 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QZSettings::tile_elapsed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elapsed_order, 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QZSettings::tile_moving_time_enabled, false).toBool() &&
                settings.value(QZSettings::tile_moving_time_order, 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QZSettings::tile_peloton_offset_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_offset_order, 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QZSettings::tile_peloton_remaining_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_remaining_order, 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QZSettings::tile_inclination_enabled, true).toBool() &&
                settings.value(QZSettings::tile_inclination_order, 29).toInt() == i) {
                inclination->setGridId(i);
                inclination->setName("Sequence");
                dataList.append(inclination);
            }

            if (settings.value(QZSettings::tile_calories_enabled, true).toBool() &&
                settings.value(QZSettings::tile_calories_order, 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QZSettings::tile_odometer_enabled, true).toBool() &&
                settings.value(QZSettings::tile_odometer_order, 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QZSettings::tile_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_resistance_order, 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QZSettings::tile_peloton_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_peloton_resistance_order, 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QZSettings::tile_watt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_watt_order, 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QZSettings::tile_weight_loss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_weight_loss_order, 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QZSettings::tile_avgwatt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avgwatt_order, 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QZSettings::tile_avg_watt_lap_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avg_watt_lap_order, 0).toInt() == i) {
                avgWattLap->setGridId(i);
                dataList.append(avgWattLap);
            }

            if (settings.value(QZSettings::tile_ftp_enabled, true).toBool() &&
                settings.value(QZSettings::tile_ftp_order, 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QZSettings::tile_jouls_enabled, true).toBool() &&
                settings.value(QZSettings::tile_jouls_order, 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QZSettings::tile_heart_enabled, true).toBool() &&
                settings.value(QZSettings::tile_heart_order, 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QZSettings::tile_fan_enabled, true).toBool() &&
                settings.value(QZSettings::tile_fan_order, 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QZSettings::tile_datetime_enabled, true).toBool() &&
                settings.value(QZSettings::tile_datetime_order, 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QZSettings::tile_target_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_target_resistance_order, 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QZSettings::tile_target_peloton_resistance_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_peloton_resistance_order, 21).toInt() == i) {
                target_peloton_resistance->setGridId(i);
                dataList.append(target_peloton_resistance);
            }

            if (settings.value(QZSettings::tile_target_cadence_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_cadence_order, 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QZSettings::tile_target_power_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_power_order, 20).toInt() == i) {
                target_power->setGridId(i);
                dataList.append(target_power);
            }

            if (settings.value(QZSettings::tile_lapelapsed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_lapelapsed_order, 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QZSettings::tile_strokes_length_enabled, false).toBool() &&
                settings.value(QZSettings::tile_strokes_length_order, 21).toInt() == i) {
                strokesLength->setGridId(i);
                dataList.append(strokesLength);
            }

            if (settings.value(QZSettings::tile_strokes_count_enabled, false).toBool() &&
                settings.value(QZSettings::tile_strokes_count_order, 22).toInt() == i) {
                strokesCount->setGridId(i);
                dataList.append(strokesCount);
            }

            if (settings.value(QZSettings::tile_pace_enabled, true).toBool() &&
                settings.value(QZSettings::tile_pace_order, 0).toInt() == i) {
                pace->setGridId(i);
                dataList.append(pace);
            }

            if (settings.value(QZSettings::tile_watt_kg_enabled, false).toBool() &&
                settings.value(QZSettings::tile_watt_kg_order, 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QZSettings::tile_step_count_enabled, QZSettings::default_tile_step_count_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_step_count_order, QZSettings::default_tile_step_count_order)
                        .toInt() == i) {

                stepCount->setGridId(i);
                stepCount->setName("Jumps Count");
                dataList.append(stepCount);
            }

            if (settings.value(QZSettings::tile_remainingtimetrainprogramrow_enabled, false).toBool() &&
                settings.value(QZSettings::tile_remainingtimetrainprogramrow_order, 27).toInt() == i) {
                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QZSettings::tile_nextrowstrainprogram_enabled, false).toBool() &&
                settings.value(QZSettings::tile_nextrowstrainprogram_order, 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QZSettings::tile_mets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_mets_order, 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QZSettings::tile_targetmets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_targetmets_order, 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QZSettings::tile_pid_hr_enabled, false).toBool() &&
                settings.value(QZSettings::tile_pid_hr_order, 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QZSettings::tile_target_zone_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_zone_order, 24).toInt() == i) {
                target_zone->setGridId(i);
                dataList.append(target_zone);
            }

            if (settings.value(QZSettings::tile_target_speed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_speed_order, 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }

            if (settings.value(QZSettings::tile_target_pace_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_pace_order, 50).toInt() == i) {
                target_pace->setGridId(i);
                dataList.append(target_pace);
            }

            if (settings
                    .value(QZSettings::tile_preset_resistance_1_enabled,
                           QZSettings::default_tile_preset_resistance_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_1_order,
                               QZSettings::default_tile_preset_resistance_1_order)
                        .toInt() == i) {
                preset_resistance_1->setGridId(i);
                dataList.append(preset_resistance_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_2_enabled,
                           QZSettings::default_tile_preset_resistance_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_2_order,
                               QZSettings::default_tile_preset_resistance_2_order)
                        .toInt() == i) {
                preset_resistance_2->setGridId(i);
                dataList.append(preset_resistance_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_3_enabled,
                           QZSettings::default_tile_preset_resistance_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_3_order,
                               QZSettings::default_tile_preset_resistance_3_order)
                        .toInt() == i) {
                preset_resistance_3->setGridId(i);
                dataList.append(preset_resistance_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_4_enabled,
                           QZSettings::default_tile_preset_resistance_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_4_order,
                               QZSettings::default_tile_preset_resistance_4_order)
                        .toInt() == i) {
                preset_resistance_4->setGridId(i);
                dataList.append(preset_resistance_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_5_enabled,
                           QZSettings::default_tile_preset_resistance_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_5_order,
                               QZSettings::default_tile_preset_resistance_5_order)
                        .toInt() == i) {
                preset_resistance_5->setGridId(i);
                dataList.append(preset_resistance_5);
            }
            if (settings.value(QZSettings::tile_gears_enabled, false).toBool() &&
                settings.value(QZSettings::tile_gears_order, 51).toInt() == i) {
                gears->setGridId(i);
                dataList.append(gears);
            }


            if (settings.value(QZSettings::tile_hr_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_1_order, 0).toInt() == i) {
                tile_hr_time_in_zone_1->setGridId(i);
                dataList.append(tile_hr_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_2_order, 0).toInt() == i) {
                tile_hr_time_in_zone_2->setGridId(i);
                dataList.append(tile_hr_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_3_order, 0).toInt() == i) {
                tile_hr_time_in_zone_3->setGridId(i);
                dataList.append(tile_hr_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_4_order, 0).toInt() == i) {
                tile_hr_time_in_zone_4->setGridId(i);
                dataList.append(tile_hr_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_5_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_5_order, 0).toInt() == i) {
                tile_hr_time_in_zone_5->setGridId(i);
                dataList.append(tile_hr_time_in_zone_5);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_1_order, 0).toInt() == i) {
                tile_heat_time_in_zone_1->setGridId(i);
                dataList.append(tile_heat_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_2_order, 0).toInt() == i) {
                tile_heat_time_in_zone_2->setGridId(i);
                dataList.append(tile_heat_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_3_order, 0).toInt() == i) {
                tile_heat_time_in_zone_3->setGridId(i);
                dataList.append(tile_heat_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_4_order, 0).toInt() == i) {
                tile_heat_time_in_zone_4->setGridId(i);
                dataList.append(tile_heat_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled).toBool() &&
                settings.value(QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order).toInt() == i) {
                coreTemperature->setGridId(i);
                dataList.append(coreTemperature);
            }
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
        for (int i = 0; i < 100; i++) {
            if (settings.value(QZSettings::tile_speed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_speed_order, 0).toInt() == i) {
                speed->setGridId(i);
                dataList.append(speed);
            }

            if (settings.value(QZSettings::tile_cadence_enabled, true).toBool() &&
                settings.value(QZSettings::tile_cadence_order, 0).toInt() == i) {
                cadence->setGridId(i);
                dataList.append(cadence);
            }

            if (settings.value(QZSettings::tile_inclination_enabled, true).toBool() &&
                settings.value(QZSettings::tile_inclination_order, 0).toInt() == i) {
                inclination->setGridId(i);
                dataList.append(inclination);
            }

            if (settings.value(QZSettings::tile_elevation_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elevation_order, 0).toInt() == i) {
                elevation->setGridId(i);
                dataList.append(elevation);
            }

            if (settings.value(QZSettings::tile_elapsed_enabled, true).toBool() &&
                settings.value(QZSettings::tile_elapsed_order, 0).toInt() == i) {
                elapsed->setGridId(i);
                dataList.append(elapsed);
            }

            if (settings.value(QZSettings::tile_moving_time_enabled, false).toBool() &&
                settings.value(QZSettings::tile_moving_time_order, 19).toInt() == i) {
                moving_time->setGridId(i);
                dataList.append(moving_time);
            }

            if (settings.value(QZSettings::tile_peloton_offset_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_offset_order, 20).toInt() == i) {
                peloton_offset->setGridId(i);
                dataList.append(peloton_offset);
            }

            if (settings.value(QZSettings::tile_peloton_remaining_enabled, false).toBool() &&
                settings.value(QZSettings::tile_peloton_remaining_order, 20).toInt() == i) {
                peloton_remaining->setGridId(i);
                dataList.append(peloton_remaining);
            }

            if (settings.value(QZSettings::tile_calories_enabled, true).toBool() &&
                settings.value(QZSettings::tile_calories_order, 0).toInt() == i) {
                calories->setGridId(i);
                dataList.append(calories);
            }

            if (settings.value(QZSettings::tile_odometer_enabled, true).toBool() &&
                settings.value(QZSettings::tile_odometer_order, 0).toInt() == i) {
                odometer->setGridId(i);
                dataList.append(odometer);
            }

            if (settings.value(QZSettings::tile_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_resistance_order, 0).toInt() == i) {
                resistance->setGridId(i);
                dataList.append(resistance);
            }

            if (settings.value(QZSettings::tile_peloton_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_peloton_resistance_order, 0).toInt() == i) {
                peloton_resistance->setGridId(i);
                dataList.append(peloton_resistance);
            }

            if (settings.value(QZSettings::tile_watt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_watt_order, 0).toInt() == i) {
                watt->setGridId(i);
                dataList.append(watt);
            }

            if (settings.value(QZSettings::tile_weight_loss_enabled, false).toBool() &&
                settings.value(QZSettings::tile_weight_loss_order, 24).toInt() == i) {
                weightLoss->setGridId(i);
                dataList.append(weightLoss);
            }

            if (settings.value(QZSettings::tile_avgwatt_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avgwatt_order, 0).toInt() == i) {
                avgWatt->setGridId(i);
                dataList.append(avgWatt);
            }

            if (settings.value(QZSettings::tile_avg_watt_lap_enabled, true).toBool() &&
                settings.value(QZSettings::tile_avg_watt_lap_order, 0).toInt() == i) {
                avgWattLap->setGridId(i);
                dataList.append(avgWattLap);
            }

            if (settings.value(QZSettings::tile_ftp_enabled, true).toBool() &&
                settings.value(QZSettings::tile_ftp_order, 0).toInt() == i) {
                ftp->setGridId(i);
                dataList.append(ftp);
            }

            if (settings.value(QZSettings::tile_jouls_enabled, true).toBool() &&
                settings.value(QZSettings::tile_jouls_order, 0).toInt() == i) {
                jouls->setGridId(i);
                dataList.append(jouls);
            }

            if (settings.value(QZSettings::tile_heart_enabled, true).toBool() &&
                settings.value(QZSettings::tile_heart_order, 0).toInt() == i) {
                heart->setGridId(i);
                dataList.append(heart);
            }

            if (settings.value(QZSettings::tile_fan_enabled, true).toBool() &&
                settings.value(QZSettings::tile_fan_order, 0).toInt() == i) {
                fan->setGridId(i);
                dataList.append(fan);
            }

            if (settings.value(QZSettings::tile_datetime_enabled, true).toBool() &&
                settings.value(QZSettings::tile_datetime_order, 0).toInt() == i) {
                datetime->setGridId(i);
                dataList.append(datetime);
            }

            if (settings.value(QZSettings::tile_target_resistance_enabled, true).toBool() &&
                settings.value(QZSettings::tile_target_resistance_order, 0).toInt() == i) {
                target_resistance->setGridId(i);
                dataList.append(target_resistance);
            }

            if (settings.value(QZSettings::tile_lapelapsed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_lapelapsed_order, 18).toInt() == i) {
                lapElapsed->setGridId(i);
                dataList.append(lapElapsed);
            }

            if (settings.value(QZSettings::tile_watt_kg_enabled, false).toBool() &&
                settings.value(QZSettings::tile_watt_kg_order, 24).toInt() == i) {
                wattKg->setGridId(i);
                dataList.append(wattKg);
            }

            if (settings.value(QZSettings::tile_remainingtimetrainprogramrow_enabled, false).toBool() &&
                settings.value(QZSettings::tile_remainingtimetrainprogramrow_order, 27).toInt() == i) {
                remaningTimeTrainingProgramCurrentRow->setGridId(i);
                dataList.append(remaningTimeTrainingProgramCurrentRow);
            }

            if (settings.value(QZSettings::tile_nextrowstrainprogram_enabled, false).toBool() &&
                settings.value(QZSettings::tile_nextrowstrainprogram_order, 31).toInt() == i) {

                nextRows->setGridId(i);
                dataList.append(nextRows);
            }

            if (settings.value(QZSettings::tile_mets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_mets_order, 28).toInt() == i) {
                mets->setGridId(i);
                dataList.append(mets);
            }
            if (settings.value(QZSettings::tile_targetmets_enabled, false).toBool() &&
                settings.value(QZSettings::tile_targetmets_order, 29).toInt() == i) {
                targetMets->setGridId(i);
                dataList.append(targetMets);
            }

            if (settings.value(QZSettings::tile_pid_hr_enabled, false).toBool() &&
                settings.value(QZSettings::tile_pid_hr_order, 31).toInt() == i) {
                pidHR->setGridId(i);
                dataList.append(pidHR);
            }

            if (settings.value(QZSettings::tile_target_cadence_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_cadence_order, 19).toInt() == i) {
                target_cadence->setGridId(i);
                dataList.append(target_cadence);
            }

            if (settings.value(QZSettings::tile_target_speed_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_speed_order, 28).toInt() == i) {
                target_speed->setGridId(i);
                dataList.append(target_speed);
            }

            if (settings
                    .value(QZSettings::tile_preset_inclination_1_enabled,
                           QZSettings::default_tile_preset_inclination_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_1_order,
                               QZSettings::default_tile_preset_inclination_1_order)
                        .toInt() == i) {
                preset_inclination_1->setGridId(i);
                dataList.append(preset_inclination_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_2_enabled,
                           QZSettings::default_tile_preset_inclination_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_2_order,
                               QZSettings::default_tile_preset_inclination_2_order)
                        .toInt() == i) {
                preset_inclination_2->setGridId(i);
                dataList.append(preset_inclination_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_3_enabled,
                           QZSettings::default_tile_preset_inclination_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_3_order,
                               QZSettings::default_tile_preset_inclination_3_order)
                        .toInt() == i) {
                preset_inclination_3->setGridId(i);
                dataList.append(preset_inclination_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_4_enabled,
                           QZSettings::default_tile_preset_inclination_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_4_order,
                               QZSettings::default_tile_preset_inclination_4_order)
                        .toInt() == i) {
                preset_inclination_4->setGridId(i);
                dataList.append(preset_inclination_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_inclination_5_enabled,
                           QZSettings::default_tile_preset_inclination_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_inclination_5_order,
                               QZSettings::default_tile_preset_inclination_5_order)
                        .toInt() == i) {
                preset_inclination_5->setGridId(i);
                dataList.append(preset_inclination_5);
            }

            if (settings
                    .value(QZSettings::tile_preset_resistance_1_enabled,
                           QZSettings::default_tile_preset_resistance_1_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_1_order,
                               QZSettings::default_tile_preset_resistance_1_order)
                        .toInt() == i) {
                preset_resistance_1->setGridId(i);
                dataList.append(preset_resistance_1);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_2_enabled,
                           QZSettings::default_tile_preset_resistance_2_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_2_order,
                               QZSettings::default_tile_preset_resistance_2_order)
                        .toInt() == i) {
                preset_resistance_2->setGridId(i);
                dataList.append(preset_resistance_2);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_3_enabled,
                           QZSettings::default_tile_preset_resistance_3_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_3_order,
                               QZSettings::default_tile_preset_resistance_3_order)
                        .toInt() == i) {
                preset_resistance_3->setGridId(i);
                dataList.append(preset_resistance_3);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_4_enabled,
                           QZSettings::default_tile_preset_resistance_4_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_4_order,
                               QZSettings::default_tile_preset_resistance_4_order)
                        .toInt() == i) {
                preset_resistance_4->setGridId(i);
                dataList.append(preset_resistance_4);
            }
            if (settings
                    .value(QZSettings::tile_preset_resistance_5_enabled,
                           QZSettings::default_tile_preset_resistance_5_enabled)
                    .toBool() &&
                settings.value(QZSettings::tile_preset_resistance_5_order,
                               QZSettings::default_tile_preset_resistance_5_order)
                        .toInt() == i) {
                preset_resistance_5->setGridId(i);
                dataList.append(preset_resistance_5);
            }
            if (settings.value(QZSettings::tile_gears_enabled, false).toBool() &&
                settings.value(QZSettings::tile_gears_order, 25).toInt() == i) {
                gears->setGridId(i);
                dataList.append(gears);
            }

            if (settings.value(QZSettings::tile_target_pace_enabled, false).toBool() &&
                settings.value(QZSettings::tile_target_pace_order, 50).toInt() == i) {
                target_pace->setGridId(i);
                dataList.append(target_pace);
            }

            if (settings.value(QZSettings::tile_pace_enabled, true).toBool() &&
                settings.value(QZSettings::tile_pace_order, 51).toInt() == i) {
                pace->setGridId(i);
                dataList.append(pace);
            }


            if (settings.value(QZSettings::tile_hr_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_1_order, 0).toInt() == i) {
                tile_hr_time_in_zone_1->setGridId(i);
                dataList.append(tile_hr_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_2_order, 0).toInt() == i) {
                tile_hr_time_in_zone_2->setGridId(i);
                dataList.append(tile_hr_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_3_order, 0).toInt() == i) {
                tile_hr_time_in_zone_3->setGridId(i);
                dataList.append(tile_hr_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_4_order, 0).toInt() == i) {
                tile_hr_time_in_zone_4->setGridId(i);
                dataList.append(tile_hr_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_hr_time_in_zone_5_enabled, false).toBool() &&
                settings.value(QZSettings::tile_hr_time_in_zone_5_order, 0).toInt() == i) {
                tile_hr_time_in_zone_5->setGridId(i);
                dataList.append(tile_hr_time_in_zone_5);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_1_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_1_order, 0).toInt() == i) {
                tile_heat_time_in_zone_1->setGridId(i);
                dataList.append(tile_heat_time_in_zone_1);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_2_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_2_order, 0).toInt() == i) {
                tile_heat_time_in_zone_2->setGridId(i);
                dataList.append(tile_heat_time_in_zone_2);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_3_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_3_order, 0).toInt() == i) {
                tile_heat_time_in_zone_3->setGridId(i);
                dataList.append(tile_heat_time_in_zone_3);
            }

            if (settings.value(QZSettings::tile_heat_time_in_zone_4_enabled, false).toBool() &&
                settings.value(QZSettings::tile_heat_time_in_zone_4_order, 0).toInt() == i) {
                tile_heat_time_in_zone_4->setGridId(i);
                dataList.append(tile_heat_time_in_zone_4);
            }

            if (settings.value(QZSettings::tile_coretemperature_enabled, QZSettings::default_tile_coretemperature_enabled).toBool() &&
                settings.value(QZSettings::tile_coretemperature_order, QZSettings::default_tile_coretemperature_order).toInt() == i) {
                coreTemperature->setGridId(i);
                dataList.append(coreTemperature);
            }
        }
    }

    engine->rootContext()->setContextProperty(QStringLiteral("appModel"), QVariant::fromValue(dataList));
}

DataObject *homeform::tileFromName(QString name) {
    foreach (QObject *d, dataList) {
        if (!((DataObject *)d)->name().compare(name)) {
            return (DataObject *)d;
        }
    }
    return nullptr;
}

void homeform::moveTile(QString name, int newIndex, int oldIndex) {
    QSettings settings;
    DataObject *current = tileFromName(name);
    if (current) {
        qDebug() << "moveTile" << name << newIndex << oldIndex;

        foreach (QString s, settings.allKeys()) {
            if (s.contains(QStringLiteral("tile_")) && s.contains(QStringLiteral("_order"))) {

                qDebug() << s << settings.value(s);
            }
        }

        int i = 0;
        foreach (QObject *d, dataList) {
            if (i == newIndex) {
                settings.setValue("tile_" + current->m_id.toLower() + "_order", i);
                i++;
            }
            QString n = ((DataObject *)d)->m_id;
            if (((DataObject *)d)->name().compare(name)) {
                settings.setValue("tile_" + n.toLower() + "_order", i);
                i++;
            }
        }

        foreach (QString s, settings.allKeys()) {
            if (s.contains(QStringLiteral("tile_")) && s.contains(QStringLiteral("_order"))) {

                qDebug() << s << settings.value(s);
            }
        }

        // sortTiles();
        // dataList.move(oldIndex, newIndex);
        // very dirty, but i needed a way to synchronize QML with C++
        QTimer::singleShot(100, this, &homeform::sortTilesTimeout);
    }
}

void homeform::sortTilesTimeout() { sortTiles(); }

void homeform::deviceConnected(QBluetoothDeviceInfo b) {

    qDebug() << "deviceConnected" << bluetoothManager << engine;
    if (bluetoothManager)
        qDebug() << bluetoothManager->device();

    if (bluetoothManager->device() == nullptr)
        return;

    // if the device reconnects in the same session, the tiles shouldn't be created again
    static bool first = false;
    if (first) {
        return;
    }
    first = true;

    if (b.isValid())
        deviceFound(b.name());

    m_labelHelp = false;
    emit changeLabelHelp(m_labelHelp);

    QSettings settings;

    if (settings.value(QZSettings::pause_on_start, QZSettings::default_pause_on_start).toBool() &&
        bluetoothManager->device()->deviceType() != bluetoothdevice::TREADMILL) {
        Start();
        stopped = true; // when you will press start while you did some kms in pause mode from the beginning, the stats must be resetted
    } else if (settings.value(QZSettings::pause_on_start_treadmill, QZSettings::default_pause_on_start_treadmill)
                   .toBool() &&
               bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        Start_inner(false); // because if you sent the start command to a treadmill it could start the tape
        stopped = true; // when you will press start while you did some kms in pause mode from the beginning, the stats must be resetted
    }

    sortTiles();

    QObject *rootObject = engine->rootObjects().constFirst();
    QObject *home = rootObject->findChild<QObject *>(QStringLiteral("home"));
    QObject::connect(home, SIGNAL(plus_clicked(QString)), this, SLOT(Plus(QString)));
    QObject::connect(home, SIGNAL(minus_clicked(QString)), this, SLOT(Minus(QString)));
    QObject::connect(home, SIGNAL(largeButton_clicked(QString)), this, SLOT(LargeButton(QString)));

    emit workoutNameChanged(workoutName());
    emit instructorNameChanged(instructorName());

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::floating_startup, QZSettings::default_floating_startup).toBool()) {
        floatingOpen();
    }

    if (!settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name)
             .toString()
             .compare(QZSettings::default_heart_rate_belt_name) &&
        !settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool()) {
        QJniObject context4 = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
        QJniObject::callStaticMethod<void>("org/cagnulen/qdomyoszwift/WearableController", "start",
                                                  "(Landroid/content/Context;)V", context4.object());
    }
#endif

    if (settings.value(QZSettings::gears_restore_value, QZSettings::default_gears_restore_value).toBool() ||
        settings.value(QZSettings::restore_specific_gear, QZSettings::default_restore_specific_gear).toBool()) {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            ((bike *)bluetoothManager->device())
                ->setGears(settings.value(QZSettings::gears_current_value, QZSettings::default_gears_current_value)
                               .toDouble());
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
            ((elliptical *)bluetoothManager->device())
                ->setGears(settings.value(QZSettings::gears_current_value, QZSettings::default_gears_current_value)
                               .toDouble());
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            ((rower *)bluetoothManager->device())
                ->setGears(settings.value(QZSettings::gears_current_value, QZSettings::default_gears_current_value)
                               .toDouble());
        }

    }
}

void homeform::deviceFound(const QString &name) {
    if (name.trimmed().isEmpty()) {
        return;
    }

    emit bluetoothDevicesChanged(bluetoothDevices());

    if (m_labelHelp == false) {
        return;
    }

    QSettings settings;
    if (!settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
        return;
    }
    m_info = name + QStringLiteral(" found");
    emit infoChanged(m_info);
}

void homeform::LargeButton(const QString &name) {
    QSettings settings;
    qDebug() << QStringLiteral("LargeButton") << name;
    if (!bluetoothManager || !bluetoothManager->device())
        return;

    if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE || 
        bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL ||
        bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        if (name.startsWith(QStringLiteral("preset_powerzone_"))) {
            double ftp = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
            int zoneNum = name.right(1).toInt(); // Gets last digit from preset_powerzone_X
            QString zoneSetting = QString("tile_preset_powerzone_%1_value").arg(zoneNum);
            double zoneValue = settings.value(zoneSetting, zoneNum).toDouble();

            // Calculate target watts based on FTP and zone value
            // Each zone represents a percentage of FTP
            double targetWatts;
            if (zoneValue <= 1.9) {
                targetWatts = ftp * 0.55 * (zoneValue);
            } else if (zoneValue <= 2.9) {
                targetWatts = ftp * 0.75 * (zoneValue - 1);
            } else if (zoneValue <= 3.9) {
                targetWatts = ftp * 0.90 * (zoneValue - 2);
            } else if (zoneValue <= 4.9) {
                targetWatts = ftp * 1.05 * (zoneValue - 3);
            } else if (zoneValue <= 5.9) {
                targetWatts = ftp * 1.20 * (zoneValue - 4);
            } else if (zoneValue <= 6.9) {
                targetWatts = ftp * 1.50 * (zoneValue - 5);
            } else {
                targetWatts = ftp * 1.70 * (zoneValue - 6);
            }
            bluetoothManager->device()->changePower(targetWatts);
        } else if (name.contains(QStringLiteral("erg_mode"))) {
            settings.setValue(QZSettings::zwift_erg, !settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool());
        } else if (name.contains(QStringLiteral("preset_resistance_1"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_1_value,
                                                                    QZSettings::default_tile_preset_resistance_1_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_2"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_2_value,
                                                                    QZSettings::default_tile_preset_resistance_2_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_3"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_3_value,
                                                                    QZSettings::default_tile_preset_resistance_3_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_4"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_4_value,
                                                                    QZSettings::default_tile_preset_resistance_4_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_5"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_5_value,
                                                                    QZSettings::default_tile_preset_resistance_5_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_1"))) {
            ((bike *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_1_value,
                                               QZSettings::default_tile_preset_inclination_1_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_1_value,
                                               QZSettings::default_tile_preset_inclination_1_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_2"))) {
            ((bike *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_2_value,
                                               QZSettings::default_tile_preset_inclination_2_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_2_value,
                                               QZSettings::default_tile_preset_inclination_2_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_3"))) {
            ((bike *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_3_value,
                                               QZSettings::default_tile_preset_inclination_3_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_3_value,
                                               QZSettings::default_tile_preset_inclination_3_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_4"))) {
            ((bike *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_4_value,
                                               QZSettings::default_tile_preset_inclination_4_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_4_value,
                                               QZSettings::default_tile_preset_inclination_4_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_5"))) {
            ((bike *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_5_value,
                                               QZSettings::default_tile_preset_inclination_5_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_5_value,
                                               QZSettings::default_tile_preset_inclination_5_value)
                                        .toDouble());
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
        if (name.contains(QStringLiteral("preset_speed_1"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(
                    settings.value(QZSettings::tile_preset_speed_1_value, QZSettings::default_tile_preset_speed_1_value)
                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_speed_2"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(
                    settings.value(QZSettings::tile_preset_speed_2_value, QZSettings::default_tile_preset_speed_2_value)
                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_speed_3"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(
                    settings.value(QZSettings::tile_preset_speed_3_value, QZSettings::default_tile_preset_speed_3_value)
                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_speed_4"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(
                    settings.value(QZSettings::tile_preset_speed_4_value, QZSettings::default_tile_preset_speed_4_value)
                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_speed_5"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeSpeed(
                    settings.value(QZSettings::tile_preset_speed_5_value, QZSettings::default_tile_preset_speed_5_value)
                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_1"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_1_value,
                                               QZSettings::default_tile_preset_inclination_1_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_1_value,
                                               QZSettings::default_tile_preset_inclination_1_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_2"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_2_value,
                                               QZSettings::default_tile_preset_inclination_2_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_2_value,
                                               QZSettings::default_tile_preset_inclination_2_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_3"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_3_value,
                                               QZSettings::default_tile_preset_inclination_3_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_3_value,
                                               QZSettings::default_tile_preset_inclination_3_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_4"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_4_value,
                                               QZSettings::default_tile_preset_inclination_4_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_4_value,
                                               QZSettings::default_tile_preset_inclination_4_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_5"))) {
            ((treadmill *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_5_value,
                                               QZSettings::default_tile_preset_inclination_5_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_5_value,
                                               QZSettings::default_tile_preset_inclination_5_value)
                                        .toDouble());
        }
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
        if (name.contains(QStringLiteral("preset_resistance_1"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_1_value,
                                                                    QZSettings::default_tile_preset_resistance_1_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_2"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_2_value,
                                                                    QZSettings::default_tile_preset_resistance_2_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_3"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_3_value,
                                                                    QZSettings::default_tile_preset_resistance_3_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_4"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_4_value,
                                                                    QZSettings::default_tile_preset_resistance_4_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_resistance_5"))) {
            bluetoothManager->device()->changeResistance(settings
                                                             .value(QZSettings::tile_preset_resistance_5_value,
                                                                    QZSettings::default_tile_preset_resistance_5_value)
                                                             .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_1"))) {
            ((elliptical *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_1_value,
                                               QZSettings::default_tile_preset_inclination_1_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_1_value,
                                               QZSettings::default_tile_preset_inclination_1_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_2"))) {
            ((elliptical *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_2_value,
                                               QZSettings::default_tile_preset_inclination_2_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_2_value,
                                               QZSettings::default_tile_preset_inclination_2_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_3"))) {
            ((elliptical *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_3_value,
                                               QZSettings::default_tile_preset_inclination_3_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_3_value,
                                               QZSettings::default_tile_preset_inclination_3_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_4"))) {
            ((elliptical *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_4_value,
                                               QZSettings::default_tile_preset_inclination_4_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_4_value,
                                               QZSettings::default_tile_preset_inclination_4_value)
                                        .toDouble());
        } else if (name.contains(QStringLiteral("preset_inclination_5"))) {
            ((elliptical *)bluetoothManager->device())
                ->changeInclination(settings
                                        .value(QZSettings::tile_preset_inclination_5_value,
                                               QZSettings::default_tile_preset_inclination_5_value)
                                        .toDouble(),
                                    settings
                                        .value(QZSettings::tile_preset_inclination_5_value,
                                               QZSettings::default_tile_preset_inclination_5_value)
                                        .toDouble());
        }
    }

    if(name.contains(QStringLiteral("biggearsplus"))) {
        gearUp();
    } else if(name.contains(QStringLiteral("biggearsminus"))) {
        gearDown();
    }
}

void homeform::Plus(const QString &name) {
    QSettings settings;

    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    qDebug() << QStringLiteral("Plus") << name;
    if (name.contains(QStringLiteral("target_speed")) || name.contains(QStringLiteral("target_pace"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                bool treadmill_difficulty_gain_or_offset =
                    settings
                        .value(QZSettings::treadmill_difficulty_gain_or_offset,
                               QZSettings::default_treadmill_difficulty_gain_or_offset)
                        .toBool();

                if (!treadmill_difficulty_gain_or_offset) {
                    bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() + 0.03);
                    if (bluetoothManager->device()->difficult() == 0) {
                        bluetoothManager->device()->setDifficult(0.03);
                    }
                } else {
                    bluetoothManager->device()->setDifficultOffset(bluetoothManager->device()->difficultOffset() + 0.1);
                    if (bluetoothManager->device()->difficultOffset() == 0) {
                        bluetoothManager->device()->setDifficultOffset(0.1);
                    }
                }

                ((treadmill *)bluetoothManager->device())
                    ->changeSpeed(((treadmill *)bluetoothManager->device())->lastRawSpeedRequested());
            }
        }
    } else if (name.contains(QStringLiteral("target_inclination"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                bool treadmill_difficulty_gain_or_offset =
                    settings
                        .value(QZSettings::treadmill_difficulty_gain_or_offset,
                               QZSettings::default_treadmill_difficulty_gain_or_offset)
                        .toBool();

                if (!treadmill_difficulty_gain_or_offset) {
                    bluetoothManager->device()->setInclinationDifficult(
                        bluetoothManager->device()->inclinationDifficult() + 0.03);
                    if (bluetoothManager->device()->inclinationDifficult() == 0) {
                        bluetoothManager->device()->setInclinationDifficult(0.03);
                    }
                } else {
                    bluetoothManager->device()->setInclinationDifficultOffset(
                        bluetoothManager->device()->inclinationDifficultOffset() + 0.5);
                    if (bluetoothManager->device()->inclinationDifficultOffset() == 0) {
                        bluetoothManager->device()->setInclinationDifficultOffset(0.5);
                    }
                }

                ((treadmill *)bluetoothManager->device())
                    ->changeInclination(((treadmill *)bluetoothManager->device())->lastRawInclinationRequested(),
                                        ((treadmill *)bluetoothManager->device())->lastRawInclinationRequested());
            }
        }
    } else if (name.contains(QStringLiteral("speed"))) {
        if (bluetoothManager->device() && bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            // round up to the next .5 increment (.0 or .5)
            double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
            double requestedspeed = ((treadmill *)bluetoothManager->device())->requestedSpeed();
            double targetspeed = ((treadmill *)bluetoothManager->device())->currentTargetSpeed();
            qDebug() << QStringLiteral("Current Speed") << speed << QStringLiteral("Current Requested Speed")
                     << requestedspeed << QStringLiteral("Current Target Speed") << targetspeed;
            if (targetspeed != -1)
                speed = targetspeed;
            if (requestedspeed != -1)
                speed = requestedspeed;
            double minStepSpeed = ((treadmill *)bluetoothManager->device())->minStepSpeed();
            double step =
                settings.value(QZSettings::treadmill_step_speed, QZSettings::default_treadmill_step_speed).toDouble();
            if (!miles)
                step = ((double)qRound(step * 10.0)) / 10.0;
            if (step > minStepSpeed)
                minStepSpeed = step;
            int rest = 0;
            if (!miles)
                rest = (minStepSpeed * 10.0) - (((int)(speed * 10.0)) % (uint8_t)(minStepSpeed * 10.0));
            if (rest == 5 || rest == 0)
                speed = speed + minStepSpeed;
            else
                speed = speed + (((double)rest) / 10.0);

            ((treadmill *)bluetoothManager->device())->changeSpeed(speed);
        }
    } else if (name.contains(QStringLiteral("external_inclination"))) {
        double elite_rizer_gain =
            settings.value(QZSettings::elite_rizer_gain, QZSettings::default_elite_rizer_gain).toDouble();
        elite_rizer_gain = elite_rizer_gain + 0.1;
        settings.setValue(QZSettings::elite_rizer_gain, elite_rizer_gain);
    } else if (name.contains(QStringLiteral("inclination"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                if (step < ((treadmill *)bluetoothManager->device())->minStepInclination())
                    step = ((treadmill *)bluetoothManager->device())->minStepInclination();
                double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() + step;
                ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                if (step < ((elliptical *)bluetoothManager->device())->minStepInclination())
                    step = ((elliptical *)bluetoothManager->device())->minStepInclination();
                double perc = ((elliptical *)bluetoothManager->device())->currentInclination().value() + step;
                ((elliptical *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                ((bike *)bluetoothManager->device())
                    ->changeInclination(((bike *)bluetoothManager->device())->currentInclination().value() + step,
                                        ((bike *)bluetoothManager->device())->currentInclination().value() + step);
            }
        }
    } else if (name.contains(QStringLiteral("pid_hr"))) {
        if (bluetoothManager->device()) {
            QSettings settings;
            QString zoneS =
                settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                    .toString();
            uint8_t zone =
                settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                    .toString()
                    .toUInt();

            if (!zoneS.compare(QStringLiteral("Disabled")))
                zone = 0;

            if (zone < 5) {
                zone++;
                settings.setValue(QZSettings::treadmill_pid_heart_zone, QString::number(zone));
            }

            if(trainProgram)
                trainProgram->overrideZoneHRForCurrentRow(zone);
        }
    } else if (name.contains("gears")) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->setGears(((bike *)bluetoothManager->device())->gears() +
                               settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble());
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                ((elliptical *)bluetoothManager->device())
                    ->setGears(((elliptical *)bluetoothManager->device())->gears() +
                               settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble());
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                ((rower *)bluetoothManager->device())
                    ->setGears(((rower *)bluetoothManager->device())->gears() +
                               settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble());
            }
        }
    } else if (name.contains(QStringLiteral("target_resistance"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() + 0.03);
                if (bluetoothManager->device()->difficult() == 0) {
                    bluetoothManager->device()->setDifficult(0.03);
                }

                if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                    ((bike *)bluetoothManager->device())
                        ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                    ((rower *)bluetoothManager->device())
                        ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                    ((elliptical *)bluetoothManager->device())
                        ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value());
                }
            }
        }
    } else if (name.contains(QStringLiteral("resistance")) || name.contains(QStringLiteral("peloton_resistance"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value() + 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                ((rower *)bluetoothManager->device())
                    ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value() + 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                ((elliptical *)bluetoothManager->device())
                    ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value() + 1);
            }
        }
    } else if (name.contains(QStringLiteral("target_power"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                m_overridePower = true;
                ((bike *)bluetoothManager->device())
                    ->changePower(((bike *)bluetoothManager->device())->lastRequestedPower().value() + 10);
                if (trainProgram) {
                    trainProgram->overridePowerForCurrentRow(
                        ((bike *)bluetoothManager->device())->lastRequestedPower().value());
                }
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                m_overridePower = true;
                ((treadmill *)bluetoothManager->device())
                    ->changePower(((treadmill *)bluetoothManager->device())->lastRequestedPower().value() + 10);
                if (trainProgram) {
                    trainProgram->overridePowerForCurrentRow(
                        ((treadmill *)bluetoothManager->device())->lastRequestedPower().value());
                }
            }
        }
    } else if (name.contains(QStringLiteral("fan"))) {
        QSettings settings;
        if (bluetoothManager->device()) {
            if (settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable)
                    .toBool() &&
                settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                    .toString()
                    .compare(QStringLiteral("Manual"))) {
                fanOverride += 10;
            } else if (settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable)
                           .toBool()) {
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 10);
            } else
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() + 1);
        }
    } else if (name.contains(QStringLiteral("remainingtimetrainprogramrow"))) {
        if (bluetoothManager->device() && trainProgram) {
            trainProgram->increaseElapsedTime(QTime(0, 0, 0).secsTo(trainProgram->currentRowRemainingTime()));
        }
    } else if (name.contains(QStringLiteral("peloton_offset")) || name.contains(QStringLiteral("peloton_remaining"))) {
        if (bluetoothManager->device() && trainProgram) {
            trainProgram->increaseElapsedTime(1);
        }
    } else {
        qDebug() << name << QStringLiteral("not handled");

        qDebug() << "Plus" << name;
    }
}

void homeform::pelotonOffset_Plus() { Plus(QStringLiteral("peloton_offset")); }

void homeform::pelotonOffset_Minus() { Minus(QStringLiteral("peloton_offset")); }

void homeform::bluetoothDeviceConnected(bluetoothdevice *b) {
    this->innerTemplateManager->start(b);
    this->userTemplateManager->start(b);
#ifndef Q_OS_IOS
    // heart rate received from apple watch while QZ is running on a different device via TCP socket (iphone_socket)
    connect(this, SIGNAL(heartRate(uint8_t)), b, SLOT(heartRate(uint8_t)));
#endif
}

void homeform::bluetoothDeviceDisconnected() {
    this->innerTemplateManager->stop();
    this->userTemplateManager->stop();
}

void homeform::Minus(const QString &name) {
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    qDebug() << QStringLiteral("Minus") << name;
    if (name.contains(QStringLiteral("target_speed")) || name.contains(QStringLiteral("target_pace"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                bool treadmill_difficulty_gain_or_offset =
                    settings
                        .value(QZSettings::treadmill_difficulty_gain_or_offset,
                               QZSettings::default_treadmill_difficulty_gain_or_offset)
                        .toBool();

                if (!treadmill_difficulty_gain_or_offset) {
                    bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() - 0.03);
                    if (bluetoothManager->device()->difficult() == 0) {
                        bluetoothManager->device()->setDifficult(-0.03);
                    }
                } else {
                    bluetoothManager->device()->setDifficultOffset(bluetoothManager->device()->difficultOffset() - 0.1);
                    if (bluetoothManager->device()->difficultOffset() == 0) {
                        bluetoothManager->device()->setDifficultOffset(-0.1);
                    }
                }

                ((treadmill *)bluetoothManager->device())
                    ->changeSpeed(((treadmill *)bluetoothManager->device())->lastRawSpeedRequested());
            }
        }
    } else if (name.contains(QStringLiteral("target_inclination"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                bool treadmill_difficulty_gain_or_offset =
                    settings
                        .value(QZSettings::treadmill_difficulty_gain_or_offset,
                               QZSettings::default_treadmill_difficulty_gain_or_offset)
                        .toBool();

                if (!treadmill_difficulty_gain_or_offset) {
                    bluetoothManager->device()->setInclinationDifficult(
                        bluetoothManager->device()->inclinationDifficult() - 0.03);
                    if (bluetoothManager->device()->inclinationDifficult() == 0) {
                        bluetoothManager->device()->setInclinationDifficult(-0.03);
                    }
                } else {
                    bluetoothManager->device()->setInclinationDifficultOffset(
                        bluetoothManager->device()->inclinationDifficultOffset() - 0.5);
                    if (bluetoothManager->device()->inclinationDifficultOffset() == 0) {
                        bluetoothManager->device()->setInclinationDifficult(-0.5);
                    }
                }

                ((treadmill *)bluetoothManager->device())
                    ->changeInclination(((treadmill *)bluetoothManager->device())->lastRawInclinationRequested(),
                                        ((treadmill *)bluetoothManager->device())->lastRawInclinationRequested());
            }
        }
    } else if (name.contains(QStringLiteral("speed"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                // round up to the next .5 increment (.0 or .5)
                double speed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                double requestedspeed = ((treadmill *)bluetoothManager->device())->requestedSpeed();
                double targetspeed = ((treadmill *)bluetoothManager->device())->currentTargetSpeed();
                qDebug() << QStringLiteral("Current Speed") << speed << QStringLiteral("Current Requested Speed")
                         << requestedspeed << QStringLiteral("Current Target Speed") << targetspeed;
                if (targetspeed != -1)
                    speed = targetspeed;
                if (requestedspeed != -1 && requestedspeed < speed)
                    speed = requestedspeed;
                double minStepSpeed = ((treadmill *)bluetoothManager->device())->minStepSpeed();
                double step = settings.value(QZSettings::treadmill_step_speed, QZSettings::default_treadmill_step_speed)
                                  .toDouble();
                if (!miles)
                    step = ((double)qRound(step * 10.0)) / 10.0;
                if (step > minStepSpeed)
                    minStepSpeed = step;
                int rest = 0;
                if (!miles)
                    rest = (minStepSpeed * 10.0) - (((int)(speed * 10.0)) % (uint8_t)(minStepSpeed * 10.0));
                if (rest == 5 || rest == 0)
                    speed = speed - minStepSpeed;
                else
                    speed = speed - (((double)rest) / 10.0);
                ((treadmill *)bluetoothManager->device())->changeSpeed(speed);
            }
        }
    } else if (name.contains(QStringLiteral("external_inclination"))) {
        double elite_rizer_gain =
            settings.value(QZSettings::elite_rizer_gain, QZSettings::default_elite_rizer_gain).toDouble();
        if (elite_rizer_gain)
            elite_rizer_gain = elite_rizer_gain - 0.1;
        settings.setValue(QZSettings::elite_rizer_gain, elite_rizer_gain);
    } else if (name.contains(QStringLiteral("inclination"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                if (step < ((treadmill *)bluetoothManager->device())->minStepInclination())
                    step = ((treadmill *)bluetoothManager->device())->minStepInclination();
                double perc = ((treadmill *)bluetoothManager->device())->currentInclination().value() - step;
                ((treadmill *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                if (step < ((elliptical *)bluetoothManager->device())->minStepInclination())
                    step = ((elliptical *)bluetoothManager->device())->minStepInclination();
                double perc = ((elliptical *)bluetoothManager->device())->currentInclination().value() - step;
                ((elliptical *)bluetoothManager->device())->changeInclination(perc, perc);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                double step =
                    settings.value(QZSettings::treadmill_step_incline, QZSettings::default_treadmill_step_incline)
                        .toDouble();
                ((bike *)bluetoothManager->device())
                    ->changeInclination(((bike *)bluetoothManager->device())->currentInclination().value() - step,
                                        ((bike *)bluetoothManager->device())->currentInclination().value() - step);
            }
        }
    } else if (name.contains(QStringLiteral("pid_hr"))) {
        if (bluetoothManager->device()) {
            QSettings settings;
            uint8_t zone =
                settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                    .toString()
                    .toUInt();
            if (zone > 1) {
                zone--;
                settings.setValue(QZSettings::treadmill_pid_heart_zone, QString::number(zone));
            } else {
                settings.setValue(QZSettings::treadmill_pid_heart_zone, QStringLiteral("Disabled"));
            }

            if(trainProgram)
                trainProgram->overrideZoneHRForCurrentRow(zone);
        }
    } else if (name.contains(QStringLiteral("gears"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->setGears(((bike *)bluetoothManager->device())->gears() -
                               settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble());
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                ((elliptical *)bluetoothManager->device())
                    ->setGears(((elliptical *)bluetoothManager->device())->gears() -
                               settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble());
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                ((rower *)bluetoothManager->device())
                    ->setGears(((rower *)bluetoothManager->device())->gears() -
                               settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble());
            }
        }
    } else if (name.contains(QStringLiteral("target_resistance"))) {
        if (bluetoothManager->device()) {

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL ||
                bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                bluetoothManager->device()->setDifficult(bluetoothManager->device()->difficult() - 0.03);
                if (bluetoothManager->device()->difficult() == 0) {
                    bluetoothManager->device()->setDifficult(-0.03);
                }

                if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                    ((bike *)bluetoothManager->device())
                        ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                    ((rower *)bluetoothManager->device())
                        ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value());
                } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                    ((elliptical *)bluetoothManager->device())
                        ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value());
                }
            }
        }
    } else if (name.contains(QStringLiteral("resistance")) || name.contains(QStringLiteral("peloton_resistance"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                ((bike *)bluetoothManager->device())
                    ->changeResistance(((bike *)bluetoothManager->device())->currentResistance().value() - 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                ((rower *)bluetoothManager->device())
                    ->changeResistance(((rower *)bluetoothManager->device())->currentResistance().value() - 1);
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                ((elliptical *)bluetoothManager->device())
                    ->changeResistance(((elliptical *)bluetoothManager->device())->currentResistance().value() - 1);
            }
        }
    } else if (name.contains(QStringLiteral("target_power"))) {
        if (bluetoothManager->device()) {
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                m_overridePower = true;
                ((bike *)bluetoothManager->device())
                    ->changePower(((bike *)bluetoothManager->device())->lastRequestedPower().value() - 10);
                if (trainProgram) {
                    trainProgram->overridePowerForCurrentRow(
                        ((bike *)bluetoothManager->device())->lastRequestedPower().value());
                }
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
                m_overridePower = true;
                ((treadmill *)bluetoothManager->device())
                    ->changePower(((treadmill *)bluetoothManager->device())->lastRequestedPower().value() - 10);
                if (trainProgram) {
                    trainProgram->overridePowerForCurrentRow(
                        ((treadmill *)bluetoothManager->device())->lastRequestedPower().value());
                }
            }
        }
    } else if (name.contains(QStringLiteral("fan"))) {

        if (bluetoothManager->device()) {
            QSettings settings;
            if (settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable)
                    .toBool() &&
                settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                    .toString()
                    .compare(QStringLiteral("Manual"))) {
                fanOverride -= 10;
            } else if (settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable)
                           .toBool()) {
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 10);
            } else
                bluetoothManager->device()->changeFanSpeed(bluetoothManager->device()->fanSpeed() - 1);
        }
    } else if (name.contains(QStringLiteral("remainingtimetrainprogramrow"))) {
        if (bluetoothManager->device() && trainProgram) {
            // Offset:
            // 1. To account for current tick
            // 2. To bounce back to previous
            trainProgram->decreaseElapsedTime(QTime(0, 0, 0).secsTo(trainProgram->currentRowElapsedTime()) + 2);
        }
    } else if (name.contains(QStringLiteral("peloton_offset")) || name.contains(QStringLiteral("peloton_remaining"))) {
        if (bluetoothManager->device() && trainProgram) {
            trainProgram->decreaseElapsedTime(1);
        }
    } else {
        qDebug() << name << QStringLiteral("not handled");
        qDebug() << "Minus" << name;
    }
}

void homeform::Start() { Start_inner(true); }

void homeform::Start_inner(bool send_event_to_device) {
    QSettings settings;
    qDebug() << QStringLiteral("Start pressed - paused") << paused << QStringLiteral("stopped") << stopped;

    m_overridePower = false;

    if (settings.value(QZSettings::tts_enabled, QZSettings::default_tts_enabled).toBool())
        m_speech.say("Start pressed");

    if (!paused && !stopped) {
        paused = true;
        if (bluetoothManager->device() && send_event_to_device) {
            bluetoothManager->device()->stop(paused);
        }
        emit workoutEventStateChanged(bluetoothdevice::PAUSED);
        // Pause Video if running and visible
        if ((trainProgram) && (videoVisible() == true)) {
            QObject *rootObject = engine->rootObjects().constFirst();
            auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
            auto videoPlaybackHalfPlayer = qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
            videoPlaybackHalfPlayer->pause();
        }
    } else {

        if (bluetoothManager->device() && send_event_to_device) {
            bluetoothManager->device()->start();
        }

        if (stopped) {
            trainProgram->restart();
            if (bluetoothManager->device()) {

                bluetoothManager->device()->clearStats();
            }
            Session.clear();
            chartImagesFilenames.clear();

#ifdef Q_OS_IOS
            // due to #857
            if (!settings
                     .value(QZSettings::peloton_companion_workout_ocr,
                            QZSettings::default_companion_peloton_workout_ocr)
                     .toBool())
                this->innerTemplateManager->start(bluetoothManager->device());
#endif

            if (!pelotonHandler || (pelotonHandler && !pelotonHandler->isWorkoutInProgress())) {
                stravaPelotonActivityName = QLatin1String("");
                stravaPelotonInstructorName = QLatin1String("");
                movieFileName = QLatin1String("");
                stravaWorkoutName = QLatin1String("");
                stravaPelotonWorkoutType = FIT_SPORT_INVALID;
                emit workoutNameChanged(workoutName());
                emit instructorNameChanged(instructorName());
            }
            emit workoutEventStateChanged(bluetoothdevice::STARTED);
        } else {
            // if loading a training program (gpx or xml) directly from the startup of QZ, there is no way to start
            // the program otherwise
            if (!trainProgram->isStarted()) {
                qDebug() << QStringLiteral("starting training program from a resume");
                trainProgram->restart();
            }
            emit workoutEventStateChanged(bluetoothdevice::RESUMED);
            // Resume Video if visible
            if ((trainProgram) && (videoVisible() == true)) {
                QObject *rootObject = engine->rootObjects().constFirst();
                auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
                auto videoPlaybackHalfPlayer =
                    qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
                videoPlaybackHalfPlayer->play();
            }
        }

        paused = false;
        stopped = false;
    }

    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {

        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit stopColorChanged(stopColor());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
    }

    if (bluetoothManager->device()) {
        bluetoothManager->device()->setPaused(paused | stopped);
    }
}

void homeform::StartRequested() {
    Start();
    m_stopRequested = false;
    m_startRequested = true;
    emit stopRequestedChanged(m_stopRequested);
    emit startRequestedChanged(m_startRequested);
}

void homeform::StopRequested() {
    m_startRequested = false;
    m_stopRequested = true;
    emit startRequestedChanged(m_startRequested);
    emit stopRequestedChanged(m_stopRequested);
}

void homeform::StopFromTrainProgram(bool paused) {
    Stop();
}

void homeform::Stop() {
    QSettings settings;

    m_startRequested = false;

    qDebug() << QStringLiteral("Stop pressed - paused") << paused << QStringLiteral("stopped") << stopped;

    if (stopped) {
        qDebug() << QStringLiteral("Stop pressed - already pressed, ignoring...");
        return;
    }

#ifdef Q_OS_IOS
    // due to #857
    if (!settings.value(QZSettings::peloton_companion_workout_ocr, QZSettings::default_companion_peloton_workout_ocr)
             .toBool())
        this->innerTemplateManager->reinit();
#endif

    if (settings.value(QZSettings::tts_enabled, QZSettings::default_tts_enabled).toBool())
        m_speech.say("Stop pressed");

    if (bluetoothManager->device()) {

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            QTime zero(0, 0, 0, 0);
            if (bluetoothManager->device()->currentSpeed().value() == 0.0 &&
                zero.secsTo(bluetoothManager->device()->elapsedTime()) == 0) {
                qDebug() << QStringLiteral("Stop pressed - nothing to do. Elapsed time is 0 and current speed is 0");
                return;
            }
        }

        bluetoothManager->device()->stop(false);
    }

    paused = false;
    stopped = true;

    emit workoutEventStateChanged(bluetoothdevice::STOPPED);

    fit_save_clicked();

    if (bluetoothManager->device()) {
        bluetoothManager->device()->setPaused(paused | stopped);
    }

    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {

        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit stopColorChanged(stopColor());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());

        // clearing the label on top because if it was running a training program, with stop the program will be terminated
        m_info = workoutName();
        emit infoChanged(m_info);
    }

    if (trainProgram) {
        trainProgram->clearRows();
    }
}

void homeform::Lap() {
    qDebug() << QStringLiteral("lap pressed");
    if (bluetoothManager) {
        if (bluetoothManager->device()) {

            bluetoothManager->device()->setLap();
            lapTrigger = true;
        }
    }
}

bool homeform::labelHelp() { return m_labelHelp; }

QString homeform::stopText() {

    QSettings settings;
    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
        return QStringLiteral("Stop");
    }
    return QLatin1String("");
}

QString homeform::stopIcon() { return QStringLiteral("icons/icons/stop.png"); }

QString homeform::startText() {

    QSettings settings;
    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
        if (paused || stopped) {
            return QStringLiteral("Start");
        } else {
            return QStringLiteral("Pause");
        }
    }
    return QLatin1String("");
}

QString homeform::startIcon() {

    QSettings settings;
    if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
        if (paused || stopped) {
            return QStringLiteral("icons/icons/start.png");
        } else {
            return QStringLiteral("icons/icons/pause.png");
        }
    }
    return QLatin1String("");
}

void homeform::updateGearsValue() {
    QSettings settings;
    bool gears_zwift_ratio = settings.value(QZSettings::gears_zwift_ratio, QZSettings::default_gears_zwift_ratio).toBool();
    bool zwift_gear_ui_aligned = settings.value(QZSettings::zwift_gear_ui_aligned, QZSettings::default_zwift_gear_ui_aligned).toBool();
    double gear = ((bike *)bluetoothManager->device())->gears();
    double maxGearDefault = ((bike *)bluetoothManager->device())->defaultMaxGears();
    double maxGear = ((bike *)bluetoothManager->device())->maxGears();
    if(zwift_gear_ui_aligned && bluetoothManager && bluetoothManager->device() && ((bike *)bluetoothManager->device())->VirtualBike())
        gear = ((bike *)bluetoothManager->device())->VirtualBike()->currentGear();
    if (settings.value(QZSettings::gears_gain, QZSettings::default_gears_gain).toDouble() == 1.0 || gears_zwift_ratio || maxGear < maxGearDefault) {
        this->gears->setValue(QString::number(gear));
        this->gears->setSecondLine(wheelCircumference::gearsInfo(gear));
    } else {
        this->gears->setValue(QString::number(gear, 'f', 1));
        this->gears->setSecondLine(QStringLiteral(""));
    }
}

QString homeform::signal() {
    if (!bluetoothManager) {
        return QStringLiteral("icons/icons/signal-1.png");
    }

    if (!bluetoothManager->device()) {
        return QStringLiteral("icons/icons/signal-1.png");
    }

    int16_t rssi = bluetoothManager->device()->bluetoothDevice.rssi();
    if (rssi > -40) {
        return QStringLiteral("icons/icons/signal-3.png");
    } else if (rssi > -60) {
        return QStringLiteral("icons/icons/signal-2.png");
    }

    return QStringLiteral("icons/icons/signal-1.png");
}

void homeform::handleRestoreDefaultWheelDiameter() {
    if (bluetoothManager && bluetoothManager->device() &&
        bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

        // Controlla se il dispositivo è un wahookickrsnapbike
        wahookickrsnapbike* kickrBike = dynamic_cast<wahookickrsnapbike*>(bluetoothManager->device());
        if (kickrBike) {
            kickrBike->restoreDefaultWheelDiameter();
        }
    }
}

void homeform::update() {

    QSettings settings;
    double currentHRZone = 1;
    double ftpZone = 1;

    qDebug() << "homeform::update fired!";

    if (settings.status() != QSettings::NoError) {
        qDebug() << "!!!!QSETTINGS ERROR!" << settings.status();
    }

    if ((paused || stopped) &&
        settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {

        emit stopIconChanged(stopIcon());
        emit stopTextChanged(stopText());
        emit startIconChanged(startIcon());
        emit startTextChanged(startText());
        emit startColorChanged(startColor());
        emit stopColorChanged(stopColor());
    }

    if (bluetoothManager->device()) {

        double inclination = 0;
        double resistance = 0;
        double watts = 0;
        double pace = 0;
        double peloton_resistance = 0;
        uint8_t cadence = 0;
        uint32_t totalStrokes = 0;
        double avgStrokesRate = 0;
        double maxStrokesRate = 0;
        double avgStrokesLength = 0;
        double strideLength = 0;
        double groundContact = 0;
        double verticalOscillation = 0;
        double stepCount = 0;

        bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
        double ftpSetting = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
        double unit_conversion = 1.0;
        double meter_feet_conversion = 1.0;
        double cm_inches_conversion = 1.0;        
        uint8_t treadmill_pid_heart_zone =
            settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                .toString()
                .toUInt();
        QString treadmill_pid_heart_zone_string =
            settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                .toString();
        if (!treadmill_pid_heart_zone_string.compare(QStringLiteral("Disabled")))
            treadmill_pid_heart_zone = 0;
        if (trainProgram && trainProgram->currentRow().zoneHR >= 0)
            treadmill_pid_heart_zone = trainProgram->currentRow().zoneHR;

        if (miles) {
            unit_conversion = 0.621371;
            meter_feet_conversion = 3.28084;
            cm_inches_conversion = 0.393701;
        }

        // Get the time spent in each zone
        uint32_t seconds_zone1 = bluetoothManager->device()->secondsForHeartZone(0);
        uint32_t seconds_zone2 = bluetoothManager->device()->secondsForHeartZone(1);
        uint32_t seconds_zone3 = bluetoothManager->device()->secondsForHeartZone(2);
        uint32_t seconds_zone4 = bluetoothManager->device()->secondsForHeartZone(3);
        uint32_t seconds_zone5 = bluetoothManager->device()->secondsForHeartZone(4);

        // Check if individual mode is enabled
        bool individual_mode = settings.value(QZSettings::tile_hr_time_in_zone_individual_mode, QZSettings::default_tile_hr_time_in_zone_individual_mode).toBool();

        uint32_t display_zone1, display_zone2, display_zone3, display_zone4, display_zone5;
        
        if (individual_mode) {
            // Individual mode: show only time in specific zone
            display_zone1 = seconds_zone1;
            display_zone2 = seconds_zone2;
            display_zone3 = seconds_zone3;
            display_zone4 = seconds_zone4;
            display_zone5 = seconds_zone5;
        } else {
            // Progressive mode: show cumulative time (time in this zone or higher)
            display_zone1 = seconds_zone1 + seconds_zone2 + seconds_zone3 + seconds_zone4 + seconds_zone5;
            display_zone2 = seconds_zone2 + seconds_zone3 + seconds_zone4 + seconds_zone5;
            display_zone3 = seconds_zone3 + seconds_zone4 + seconds_zone5;
            display_zone4 = seconds_zone4 + seconds_zone5;
            display_zone5 = seconds_zone5;
        }

        // Update labels based on mode
        if (individual_mode) {
            // Individual mode: show specific zone labels
            tile_hr_time_in_zone_1->setName(QStringLiteral("HR Zone 1"));
            tile_hr_time_in_zone_2->setName(QStringLiteral("HR Zone 2"));
            tile_hr_time_in_zone_3->setName(QStringLiteral("HR Zone 3"));
            tile_hr_time_in_zone_4->setName(QStringLiteral("HR Zone 4"));
            tile_hr_time_in_zone_5->setName(QStringLiteral("HR Zone 5"));
        } else {
            // Progressive mode: show cumulative zone labels
            tile_hr_time_in_zone_1->setName(QStringLiteral("HR Zone 1+"));
            tile_hr_time_in_zone_2->setName(QStringLiteral("HR Zone 2+"));
            tile_hr_time_in_zone_3->setName(QStringLiteral("HR Zone 3+"));
            tile_hr_time_in_zone_4->setName(QStringLiteral("HR Zone 4+"));
            tile_hr_time_in_zone_5->setName(QStringLiteral("HR Zone 5"));
        }

        // Update the UI for each tile
        tile_hr_time_in_zone_1->setValue(QTime(0, 0, 0).addSecs(display_zone1).toString("h:mm:ss"));
        tile_hr_time_in_zone_2->setValue(QTime(0, 0, 0).addSecs(display_zone2).toString("h:mm:ss"));
        tile_hr_time_in_zone_3->setValue(QTime(0, 0, 0).addSecs(display_zone3).toString("h:mm:ss"));
        tile_hr_time_in_zone_4->setValue(QTime(0, 0, 0).addSecs(display_zone4).toString("h:mm:ss"));
        tile_hr_time_in_zone_5->setValue(QTime(0, 0, 0).addSecs(display_zone5).toString("h:mm:ss"));

               // Set colors based on the zone
        tile_hr_time_in_zone_1->setValueFontColor(QStringLiteral("lightsteelblue"));
        tile_hr_time_in_zone_2->setValueFontColor(QStringLiteral("green"));
        tile_hr_time_in_zone_3->setValueFontColor(QStringLiteral("yellow"));
        tile_hr_time_in_zone_4->setValueFontColor(QStringLiteral("orange"));
        tile_hr_time_in_zone_5->setValueFontColor(QStringLiteral("red"));

        // Get the time spent in each heat zone
        uint32_t heat_seconds_zone1 = bluetoothManager->device()->secondsForHeatZone(0);
        uint32_t heat_seconds_zone2 = bluetoothManager->device()->secondsForHeatZone(1);
        uint32_t heat_seconds_zone3 = bluetoothManager->device()->secondsForHeatZone(2);
        uint32_t heat_seconds_zone4 = bluetoothManager->device()->secondsForHeatZone(3);

        // Calculate cumulative times (time in this heat zone or higher)
        //uint32_t heat_seconds_zone1_plus = heat_seconds_zone1 + heat_seconds_zone2 + heat_seconds_zone3 + heat_seconds_zone4;
        //uint32_t heat_seconds_zone2_plus = heat_seconds_zone2 + heat_seconds_zone3 + heat_seconds_zone4;
        //uint32_t heat_seconds_zone3_plus = heat_seconds_zone3 + heat_seconds_zone4;
        //uint32_t heat_seconds_zone4_plus = heat_seconds_zone4; // Zone 4 is already just zone 4

        // Update the UI for each heat tile
        tile_heat_time_in_zone_1->setValue(QTime(0, 0, 0).addSecs(heat_seconds_zone1).toString("h:mm:ss"));
        tile_heat_time_in_zone_2->setValue(QTime(0, 0, 0).addSecs(heat_seconds_zone2).toString("h:mm:ss"));
        tile_heat_time_in_zone_3->setValue(QTime(0, 0, 0).addSecs(heat_seconds_zone3).toString("h:mm:ss"));
        tile_heat_time_in_zone_4->setValue(QTime(0, 0, 0).addSecs(heat_seconds_zone4).toString("h:mm:ss"));

        // Set colors based on the heat zone
        tile_heat_time_in_zone_1->setValueFontColor(QStringLiteral("lightblue"));
        tile_heat_time_in_zone_2->setValueFontColor(QStringLiteral("yellow"));
        tile_heat_time_in_zone_3->setValueFontColor(QStringLiteral("orange"));
        tile_heat_time_in_zone_4->setValueFontColor(QStringLiteral("red"));

        emit signalChanged(signal());
        emit currentSpeedChanged(bluetoothManager->device()->currentSpeed().value());
        speed->setValue(QString::number(bluetoothManager->device()->currentSpeed().value() * unit_conversion, 'f', 1));
        speed->setSecondLine(
            QStringLiteral("AVG: ") +
            QString::number((bluetoothManager->device())->currentSpeed().average() * unit_conversion, 'f', 1) +
            QStringLiteral(" MAX: ") +
            QString::number((bluetoothManager->device())->currentSpeed().max() * unit_conversion, 'f', 1));
        heart->setValue(QString::number(bluetoothManager->device()->currentHeart().value(), 'f', 0));

        calories->setValue(QString::number(bluetoothManager->device()->calories().value(), 'f', 0));
        calories->setSecondLine(QString::number(bluetoothManager->device()->calories().rate1s() * 60.0, 'f', 1) +
                                " /min");
        if (!settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable).toBool())
            fan->setValue(QString::number(bluetoothManager->device()->fanSpeed()));
        else
            fan->setValue(QString::number(qRound(((double)bluetoothManager->device()->fanSpeed()) / 10.0) * 10.0));
        jouls->setValue(QString::number(bluetoothManager->device()->jouls().value() / 1000.0, 'f', 1));
        jouls->setSecondLine(QString::number(bluetoothManager->device()->jouls().rate1s() / 1000.0 * 60.0, 'f', 1) +
                             " /min");
        elapsed->setValue(bluetoothManager->device()->elapsedTime().toString(QStringLiteral("h:mm:ss")));
        moving_time->setValue(bluetoothManager->device()->movingTime().toString(QStringLiteral("h:mm:ss")));

        coreTemperature->setValue(QString::number(bluetoothManager->device()->CoreBodyTemperature.value(), 'f', 1) + "C");
        coreTemperature->setSecondLine(QString::number(bluetoothManager->device()->SkinTemperature.value(), 'f', 1) + "C HSI:" + QString::number(bluetoothManager->device()->HeatStrainIndex.value(), 'f', 1));
        
        // Update heat zone based on Heat Strain Index
        bluetoothManager->device()->setHeatZone(bluetoothManager->device()->HeatStrainIndex.value());

        if (trainProgram) {
            // sync the video with the zwo workout file
            if (videoVisible() == true && !bluetoothManager->device()->currentCordinate().isValid()) {
                QObject *rootObject = engine->rootObjects().constFirst();
                auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
                auto videoPlaybackHalfPlayer =
                    qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
                double videoTimeStampSeconds = (double)videoPlaybackHalfPlayer->position() / 1000.0;
                QTime videoCurrent = QTime(0, 0, videoTimeStampSeconds);
                int delta = trainProgram->totalElapsedTime().secsTo(videoCurrent);
                if (qAbs(delta) > 1) {
                    videoPlaybackHalfPlayer->setPosition(QTime(0, 0, 0).secsTo(trainProgram->totalElapsedTime()) *
                                                         1000.0);
                }
            }

            peloton_offset->setValue(QString::number(trainProgram->offsetElapsedTime()) + QStringLiteral(" sec."));
            peloton_remaining->setValue(trainProgram->remainingTime().toString("h:mm:ss"));
            peloton_remaining->setSecondLine(QString::number(trainProgram->offsetElapsedTime()) +
                                             QStringLiteral(" sec."));
            remaningTimeTrainingProgramCurrentRow->setValue(
                trainProgram->currentRowRemainingTime().toString(QStringLiteral("h:mm:ss")));
            remaningTimeTrainingProgramCurrentRow->setSecondLine(
                trainProgram->currentRowElapsedTime().toString(QStringLiteral("h:mm:ss")));
            targetMets->setValue(QString::number(trainProgram->currentTargetMets(), 'f', 1));
            trainrow next = trainProgram->getRowFromCurrent(1);
            trainrow next_1 = trainProgram->getRowFromCurrent(2);
            if (next.duration.second() != 0 || next.duration.minute() != 0 || next.duration.hour() != 0 || next.distance != -1) {
                QString duration = next.duration.toString(QStringLiteral("mm:ss"));
                if(next.distance != -1) {
                    duration = QString::number(next.distance, 'f' , 1);
                }
                if (next.requested_peloton_resistance != -1) {
                    nextRows->setValue(QStringLiteral("PR") + QString::number(next.requested_peloton_resistance));
                    nextRows->setSecondLine(duration);
                } else if (next.resistance != -1) {
                    nextRows->setValue(QStringLiteral("R") + QString::number(next.resistance));
                    nextRows->setSecondLine(duration);
                } else if (next.zoneHR != -1) {
                    nextRows->setValue(QStringLiteral("HR") + QString::number(next.zoneHR));
                    nextRows->setSecondLine(duration);
                } else if (next.HRmin != -1 && next.HRmax != -1) {
                    nextRows->setValue(QStringLiteral("HR") + QString::number(next.HRmin) + QStringLiteral("-") +
                                       QString::number(next.HRmax));
                    nextRows->setSecondLine(duration);
                } else if (next.speed != -1 && next.inclination != -200) {
                    nextRows->setValue(QStringLiteral("S") + QString::number(next.speed, 'f' , 1) + QStringLiteral("I") +
                                       QString::number(next.inclination, 'f' , 1));
                    nextRows->setSecondLine(duration);
                } else if (next.speed != -1) {
                    nextRows->setValue(QStringLiteral("S") + QString::number(next.speed, 'f' , 1));
                    nextRows->setSecondLine(duration);
                } else if (next.inclination != -200) {
                    nextRows->setValue(QStringLiteral("I") + QString::number(next.inclination, 'f' , 1));
                    nextRows->setSecondLine(duration);
                } else if (next.power != -1) {
                    double ftpPerc = (next.power / ftpSetting) * 100.0;
                    uint8_t ftpZone = 1;
                    if (ftpPerc < 56) {
                        ftpZone = 1;
                    } else if (ftpPerc < 76) {
                        ftpZone = 2;
                    } else if (ftpPerc < 91) {
                        ftpZone = 3;
                    } else if (ftpPerc < 106) {
                        ftpZone = 4;
                    } else if (ftpPerc < 121) {
                        ftpZone = 5;
                    } else if (ftpPerc < 151) {
                        ftpZone = 6;
                    } else {
                        ftpZone = 7;
                    }
                    nextRows->setValue(QStringLiteral("Z") + QString::number(ftpZone) + QStringLiteral(" ") +
                                       duration);
                    if (next_1.duration.second() != 0 || next_1.duration.minute() != 0 || next_1.duration.hour() != 0 || next_1.distance != -1) {
                        QString duration_1 = next_1.duration.toString(QStringLiteral("mm:ss"));
                        if(next_1.distance != -1) {
                            duration_1 = QString::number(next_1.distance, 'f' , 1);
                        }
                        if (next_1.requested_peloton_resistance != -1)
                            nextRows->setSecondLine(
                                QStringLiteral("PR") + QString::number(next_1.requested_peloton_resistance) +
                                QStringLiteral(" ") + duration_1);
                        else if (next_1.resistance != -1)
                            nextRows->setSecondLine(QStringLiteral("R") + QString::number(next_1.resistance) +
                                                    QStringLiteral(" ") +
                                                    duration_1);
                        else if (next_1.power != -1) {
                            double ftpPerc = (next_1.power / ftpSetting) * 100.0;
                            uint8_t ftpZone = 1;
                            if (ftpPerc < 56) {
                                ftpZone = 1;
                            } else if (ftpPerc < 76) {
                                ftpZone = 2;
                            } else if (ftpPerc < 91) {
                                ftpZone = 3;
                            } else if (ftpPerc < 106) {
                                ftpZone = 4;
                            } else if (ftpPerc < 121) {
                                ftpZone = 5;
                            } else if (ftpPerc < 151) {
                                ftpZone = 6;
                            } else {
                                ftpZone = 7;
                            }
                            nextRows->setSecondLine(QStringLiteral("Z") + QString::number(ftpZone) +
                                                    QStringLiteral(" ") +
                                                    duration_1);
                        }
                    } else {
                        nextRows->setSecondLine(QStringLiteral("N/A"));
                    }
                }
            } else {
                nextRows->setValue(QStringLiteral("N/A"));
            }
        }
        mets->setValue(QString::number(bluetoothManager->device()->currentMETS().value(), 'f', 1));
        mets->setSecondLine(
            QStringLiteral("AVG: ") + QString::number(bluetoothManager->device()->currentMETS().average(), 'f', 1) +
            QStringLiteral("MAX: ") + QString::number(bluetoothManager->device()->currentMETS().max(), 'f', 1));
        lapElapsed->setValue(bluetoothManager->device()->lapElapsedTime().toString(QStringLiteral("h:mm:ss")));
        lapElapsed->setSecondLine(QString::number(bluetoothManager->device()->lapOdometer() * unit_conversion, 'f', 2));
        avgWatt->setValue(QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0));
        avgWattLap->setValue(QString::number(bluetoothManager->device()->wattsMetric().lapAverage(), 'f', 0));
        wattKg->setValue(QString::number(bluetoothManager->device()->wattKg().value(), 'f', 1));
        wattKg->setSecondLine(
            QStringLiteral("AVG: ") + QString::number(bluetoothManager->device()->wattKg().average(), 'f', 1) +
            QStringLiteral("MAX: ") + QString::number(bluetoothManager->device()->wattKg().max(), 'f', 1));
        QLocale locale = QLocale::system();

        // Format the time based on the locale
        QString timeFormat = locale.timeFormat(QLocale::ShortFormat);
        bool usesAMPMFormat = timeFormat.toUpper().contains("A");
        QDateTime currentTime = QDateTime::currentDateTime();

        QString formattedTime;
        if (usesAMPMFormat) {
            // The locale uses 12-hour format with AM/PM
            formattedTime = currentTime.toString("h:mm:ss AP");
        } else {
            // The locale uses 24-hour format
            formattedTime = currentTime.toString("H:mm:ss");
        }
        datetime->setValue(formattedTime);
        watts = bluetoothManager->device()->wattsMetricforUI();
        watt->setValue(QString::number(watts, 'f', 0));
        weightLoss->setValue(QString::number(miles ? bluetoothManager->device()->weightLoss() * 35.274
                                                   : bluetoothManager->device()->weightLoss(),
                                             'f', 2));

        cadence = bluetoothManager->device()->currentCadence().value();
        this->cadence->setValue(QString::number(cadence));
        this->cadence->setSecondLine(
            QStringLiteral("AVG: ") +
            QString::number(((bike *)bluetoothManager->device())->currentCadence().average(), 'f', 0) +
            QStringLiteral(" MAX: ") +
            QString::number(((bike *)bluetoothManager->device())->currentCadence().max(), 'f', 0));

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        if (settings.value(QZSettings::volume_change_gears, QZSettings::default_volume_change_gears).toBool()) {
            lockscreen h;
            static double volumeLast = -1;
            double currentVolume = h.getVolume() * 10.0;
            qDebug() << "volume" << volumeLast << currentVolume;
            QSettings settings;
            bool gears_volume_debouncing = settings.value(QZSettings::gears_volume_debouncing, QZSettings::default_gears_volume_debouncing).toBool();
            if (volumeLast == -1)
                qDebug() << "volume init";
            else if (volumeLast > currentVolume) {
                double diff = volumeLast - currentVolume;
                for (int i = 0; i < diff; i++) {
                    Minus(QStringLiteral("gears"));
                    if(gears_volume_debouncing) {
                        i = diff;
                        break;
                    }
                }
            } else if (volumeLast < currentVolume) {
                double diff = currentVolume - volumeLast;
                for (int i = 0; i < diff; i++) {
                    Plus(QStringLiteral("gears"));
                    if(gears_volume_debouncing) {
                        i = diff;
                        break;
                    }
                }
            }
            volumeLast = currentVolume;
        }
#endif
#endif

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            double _rss = ((treadmill *)bluetoothManager->device())->runningStressScore();
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            if (bluetoothManager->device()->currentSpeed().value()) {
                pace = 10000 / (((treadmill *)bluetoothManager->device())->currentPace().second() +
                                (((treadmill *)bluetoothManager->device())->currentPace().minute() * 60));
                if (pace < 0) {
                    pace = 0;
                }
            } else {

                pace = 0;
            }
            strideLength =
                ((treadmill *)bluetoothManager->device())->currentStrideLength().value() * cm_inches_conversion;
            groundContact = ((treadmill *)bluetoothManager->device())->currentGroundContact().value();
            verticalOscillation = ((treadmill *)bluetoothManager->device())->currentVerticalOscillation().value();
            stepCount = ((treadmill *)bluetoothManager->device())->currentStepCount().value();
            inclination = ((treadmill *)bluetoothManager->device())->currentInclination().value();
            if (((treadmill *)bluetoothManager->device())->currentSpeed().value() > 2)
                this->pace->setValue(
                    ((treadmill *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            else
                this->pace->setValue("N/A");
            this->pace->setSecondLine(
                QStringLiteral("AVG: ") +
                ((treadmill *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                QStringLiteral(" MAX: ") +
                ((treadmill *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
            this->target_power->setValue(
                QString::number(((treadmill *)bluetoothManager->device())->lastRequestedPower().value(), 'f', 0));
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((treadmill *)bluetoothManager->device())->elevationGain().value() *
                                                    meter_feet_conversion,
                                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(
                QString::number(((treadmill *)bluetoothManager->device())->elevationGain().rate1s() * 60.0 *
                                    meter_feet_conversion,
                                'f', (miles ? 0 : 1)) +
                " /min");

            this->stepCount->setValue(QString::number(
                ((treadmill *)bluetoothManager->device())->currentStepCount().value(), 'f', 0));
            this->rss->setValue(QString::number(_rss, 'f', 0));

            this->instantaneousStrideLengthCM->setValue(QString::number(strideLength, 'f', 0));
            this->instantaneousStrideLengthCM->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentStrideLength().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentStrideLength().max(), 'f', 0));

            this->groundContactMS->setValue(QString::number(groundContact, 'f', 0));
            this->groundContactMS->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentGroundContact().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentGroundContact().max(), 'f', 0));

            this->verticalOscillationMM->setValue(QString::number(verticalOscillation, 'f', 0));
            this->verticalOscillationMM->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentVerticalOscillation().average(), 'f',
                                0) +
                QStringLiteral(" MAX: ") +
                QString::number(((treadmill *)bluetoothManager->device())->currentVerticalOscillation().max(), 'f', 0));

            // if there is no training program, the color is based on presets
            if (!trainProgram || trainProgram->currentRow().speed == -1 || trainProgram->currentRow().upper_speed == -1) {
                if (bluetoothManager->device()->currentSpeed().value() < 9) {
                    speed->setValueFontColor(QStringLiteral("white"));
                    this->pace->setValueFontColor(QStringLiteral("white"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 10) {
                    speed->setValueFontColor(QStringLiteral("limegreen"));
                    this->pace->setValueFontColor(QStringLiteral("limegreen"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 11) {
                    speed->setValueFontColor(QStringLiteral("gold"));
                    this->pace->setValueFontColor(QStringLiteral("gold"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 12) {
                    speed->setValueFontColor(QStringLiteral("orange"));
                    this->pace->setValueFontColor(QStringLiteral("orange"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 13) {
                    speed->setValueFontColor(QStringLiteral("darkorange"));
                    this->pace->setValueFontColor(QStringLiteral("darkorange"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 14) {
                    speed->setValueFontColor(QStringLiteral("orangered"));
                    this->pace->setValueFontColor(QStringLiteral("orangered"));
                } else {
                    speed->setValueFontColor(QStringLiteral("red"));
                    this->pace->setValueFontColor(QStringLiteral("red"));
                }
            } else {
                // Round speeds to 1 decimal place before comparison to avoid overly strict matching
                double currentSpeed = round(bluetoothManager->device()->currentSpeed().value() * 10.0) / 10.0;
                double upperSpeed = round(trainProgram->currentRow().upper_speed * 10.0) / 10.0;
                double lowerSpeed = round(trainProgram->currentRow().lower_speed * 10.0) / 10.0;

                // Check if speed is in target zone (green)
                if (currentSpeed <= upperSpeed && currentSpeed >= lowerSpeed) {
                    this->target_zone->setValueFontColor(QStringLiteral("limegreen"));
                    this->pace->setValueFontColor(QStringLiteral("limegreen"));
                }
                // Check if speed is close to target zone (orange)
                else if (currentSpeed <= (upperSpeed + 0.2) && currentSpeed >= (lowerSpeed - 0.2)) {
                    this->target_zone->setValueFontColor(QStringLiteral("orange"));
                    this->pace->setValueFontColor(QStringLiteral("orange"));
                }
                // Speed is out of range (red)
                else {
                    this->target_zone->setValueFontColor(QStringLiteral("red"));
                    this->pace->setValueFontColor(QStringLiteral("red"));
                }
            }

            // Use different zone names for walking vs running workouts
            bool isWalkingWorkout = pelotonHandler && pelotonHandler->current_workout_type.toLower().startsWith("walking");
            
            switch (trainProgram->currentRow().pace_intensity) {
            case 0:
                this->target_zone->setValue(tr("Rec."));
                break;
            case 1:
                this->target_zone->setValue(tr("Easy"));
                break;
            case 2:
                if (isWalkingWorkout) {
                    this->target_zone->setValue(tr("Brisk"));
                } else {
                    this->target_zone->setValue(tr("Moder."));
                }
                break;
            case 3:
                if (isWalkingWorkout) {
                    this->target_zone->setValue(tr("Power"));
                } else {
                    this->target_zone->setValue(tr("Chall."));
                }
                break;
            case 4:
                if (isWalkingWorkout) {
                    this->target_zone->setValue(tr("Max"));
                } else {
                    this->target_zone->setValue(tr("Hard"));
                }
                break;
            case 5:
                this->target_zone->setValue(tr("V.Hard"));
                break;
            case 6:
                this->target_zone->setValue(tr("Max"));
                break;
            default:
                this->target_zone->setValue(tr("N/A"));
                break;
            }

            if (trainProgram) {
                // in order to see the target pace of a peloton workout even if the speed force for treadmill is disabled
                this->target_pace->setValue(
                            ((treadmill *)bluetoothManager->device())->speedToPace(trainProgram->currentRow().speed).toString(QStringLiteral("m:ss")));
                this->target_pace->setSecondLine(((treadmill *)bluetoothManager->device())
                                                     ->speedToPace(trainProgram->currentRow().lower_speed)
                                                     .toString(QStringLiteral("m:ss")) +
                                                 " - " +
                                                 ((treadmill *)bluetoothManager->device())
                                                     ->speedToPace(trainProgram->currentRow().upper_speed)
                                                     .toString(QStringLiteral("m:ss")));
            } else {
                this->target_pace->setValue(
                    ((treadmill *)bluetoothManager->device())->lastRequestedPace().toString(QStringLiteral("m:ss")));
            }
            this->target_speed->setValue(QString::number(
                ((treadmill *)bluetoothManager->device())->lastRequestedSpeed().value() * unit_conversion, 'f', 1));
            this->target_speed->setSecondLine(QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) +
                                              QStringLiteral("% @0%=") +
                                              QString::number(bluetoothManager->device()->difficult(), 'f', 0));
            this->target_incline->setValue(
                QString::number(((treadmill *)bluetoothManager->device())->lastRequestedInclination().value(), 'f', 1));
            this->target_incline->setSecondLine(
                QString::number(bluetoothManager->device()->inclinationDifficult() * 100.0, 'f', 0) +
                QStringLiteral("% @0%=") + QString::number(bluetoothManager->device()->inclinationDifficult(), 'f', 0));

            // originally born for #470. When the treadmill reaches the 0 speed it enters in the pause mode
            // so this logic should care about sync the treadmill state to the UI state
            if (((treadmill *)bluetoothManager->device())->autoPauseWhenSpeedIsZero() &&
                bluetoothManager->device()->currentSpeed().value() == 0 && paused == false && stopped == false) {
                qDebug() << QStringLiteral("autoPauseWhenSpeedIsZero!");
                Start_inner(false);
            } else if (((treadmill *)bluetoothManager->device())->autoStartWhenSpeedIsGreaterThenZero() &&
                       bluetoothManager->device()->currentSpeed().value() > 0 && (paused == true || stopped == true)) {
                qDebug() << QStringLiteral("autoStartWhenSpeedIsGreaterThenZero!");
                Start_inner(false);
            }
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::STAIRCLIMBER) {
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            stepCount = ((stairclimber *)bluetoothManager->device())->currentStepCount().value();
            inclination = ((stairclimber *)bluetoothManager->device())->currentInclination().value();
            if (((stairclimber *)bluetoothManager->device())->currentSpeed().value() > 2)
                this->pace->setValue(
                    ((stairclimber *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            else
                this->pace->setValue("N/A");
            this->pace->setSecondLine(
                QStringLiteral("AVG: ") +
                ((stairclimber *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                QStringLiteral(" MAX: ") +
                ((stairclimber *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((stairclimber *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((stairclimber *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((stairclimber *)bluetoothManager->device())->elevationGain().value() *
                                                    meter_feet_conversion,
                                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(
                QString::number(((stairclimber *)bluetoothManager->device())->elevationGain().rate1s() * 60.0 *
                                    meter_feet_conversion,
                                'f', (miles ? 0 : 1)) +
                " /min");

            this->stepCount->setValue(QString::number(
                ((stairclimber *)bluetoothManager->device())->currentStepCount().value(), 'f', 0));

                   // if there is no training program, the color is based on presets
            if (!trainProgram || trainProgram->currentRow().speed == -1 || trainProgram->currentRow().upper_speed == -1) {
                if (bluetoothManager->device()->currentSpeed().value() < 9) {
                    speed->setValueFontColor(QStringLiteral("white"));
                    this->pace->setValueFontColor(QStringLiteral("white"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 10) {
                    speed->setValueFontColor(QStringLiteral("limegreen"));
                    this->pace->setValueFontColor(QStringLiteral("limegreen"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 11) {
                    speed->setValueFontColor(QStringLiteral("gold"));
                    this->pace->setValueFontColor(QStringLiteral("gold"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 12) {
                    speed->setValueFontColor(QStringLiteral("orange"));
                    this->pace->setValueFontColor(QStringLiteral("orange"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 13) {
                    speed->setValueFontColor(QStringLiteral("darkorange"));
                    this->pace->setValueFontColor(QStringLiteral("darkorange"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 14) {
                    speed->setValueFontColor(QStringLiteral("orangered"));
                    this->pace->setValueFontColor(QStringLiteral("orangered"));
                } else {
                    speed->setValueFontColor(QStringLiteral("red"));
                    this->pace->setValueFontColor(QStringLiteral("red"));
                }
            } else {
                // Round speeds to 1 decimal place before comparison to avoid overly strict matching
                double currentSpeed = round(bluetoothManager->device()->currentSpeed().value() * 10.0) / 10.0;
                double upperSpeed = round(trainProgram->currentRow().upper_speed * 10.0) / 10.0;
                double lowerSpeed = round(trainProgram->currentRow().lower_speed * 10.0) / 10.0;

                       // Check if speed is in target zone (green)
                if (currentSpeed <= upperSpeed && currentSpeed >= lowerSpeed) {
                    this->target_zone->setValueFontColor(QStringLiteral("limegreen"));
                    this->pace->setValueFontColor(QStringLiteral("limegreen"));
                }
                // Check if speed is close to target zone (orange)
                else if (currentSpeed <= (upperSpeed + 0.2) && currentSpeed >= (lowerSpeed - 0.2)) {
                    this->target_zone->setValueFontColor(QStringLiteral("orange"));
                    this->pace->setValueFontColor(QStringLiteral("orange"));
                }
                // Speed is out of range (red)
                else {
                    this->target_zone->setValueFontColor(QStringLiteral("red"));
                    this->pace->setValueFontColor(QStringLiteral("red"));
                }
            }

            switch (trainProgram->currentRow().pace_intensity) {
            case 0:
                this->target_zone->setValue(tr("Rec."));
                break;
            case 1:
                this->target_zone->setValue(tr("Easy"));
                break;
            case 2:
                this->target_zone->setValue(tr("Moder."));
                break;
            case 3:
                this->target_zone->setValue(tr("Chall."));
                break;
            case 4:
                this->target_zone->setValue(tr("Hard"));
                break;
            case 5:
                this->target_zone->setValue(tr("V.Hard"));
                break;
            case 6:
                this->target_zone->setValue(tr("Max"));
                break;
            default:
                this->target_zone->setValue(tr("N/A"));
                break;
            }
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

            bool pelotoncadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();

            if (!pelotoncadence) {
                inclination = ((bike *)bluetoothManager->device())->currentInclination().value();
                this->inclination->setValue(QString::number(inclination, 'f', 1));
                this->inclination->setSecondLine(
                    QStringLiteral("AVG: ") +
                    QString::number(((bike *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                    QStringLiteral(" MAX: ") +
                    QString::number(((bike *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            }
            if (bluetoothManager->externalInclination())
                extIncline->setValue(
                    QString::number(bluetoothManager->externalInclination()->currentInclination().value(), 'f', 1));
            double elite_rizer_gain =
                settings.value(QZSettings::elite_rizer_gain, QZSettings::default_elite_rizer_gain).toDouble();
            ergMode->setLargeButtonColor(settings.value(QZSettings::zwift_erg, QZSettings::default_zwift_erg).toBool() ? "#008000" :"#8B0000");
            extIncline->setSecondLine(QStringLiteral("Gain: ") + QString::number(elite_rizer_gain, 'f', 1));
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            resistance = ((bike *)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((bike *)bluetoothManager->device())->pelotonResistance().value();
            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(
                QString::number(((bike *)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(
                ((bike *)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->target_cadence->setValue(
                QString::number(((bike *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
            this->target_power->setValue(
                QString::number(((bike *)bluetoothManager->device())->lastRequestedPower().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance, 'f', 0));
            updateGearsValue();

            this->resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((bike *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((bike *)bluetoothManager->device())->currentResistance().max(), 'f', 0));
            this->peloton_resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((bike *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((bike *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(
                QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) + QStringLiteral("% @0%=") +
                QString::number(
                    bluetoothManager->device()->difficult() *
                        settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                            .toDouble() +
                        settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset)
                            .toDouble(),
                    'f', 0));

            elevation->setValue(
                QString::number(((bike *)bluetoothManager->device())->elevationGain().value() * meter_feet_conversion,
                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(QString::number(((bike *)bluetoothManager->device())->elevationGain().rate1s() *
                                                         60.0 * meter_feet_conversion,
                                                     'f', (miles ? 0 : 1)) +
                                     " /min");

            this->steeringAngle->setValue(
                QString::number(((bike *)bluetoothManager->device())->currentSteeringAngle().value(), 'f', 1));

            if ((!trainProgram || (trainProgram && !trainProgram->isStarted())) &&
                !((bike *)bluetoothManager->device())->ergModeSupportedAvailableBySoftware() &&
                ((bike *)bluetoothManager->device())->lastRequestedPower().value() > 0 && m_overridePower) {
                qDebug() << QStringLiteral("using target power tile for ERG workout manually");
                ((bike *)bluetoothManager->device())
                    ->changePower(((bike *)bluetoothManager->device())->lastRequestedPower().value());
            }

        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            if (bluetoothManager->device()->currentSpeed().value()) {
                pace = 10000 / (((rower *)bluetoothManager->device())->currentPace().second() +
                                (((rower *)bluetoothManager->device())->currentPace().minute() * 60));
                if (pace < 0) {
                    pace = 0;
                }
            } else {

                pace = 0;
            }

            this->gears->setValue(QString::number(((rower *)bluetoothManager->device())->gears()));
            this->pace_last500m->setValue(
                ((rower *)bluetoothManager->device())->lastPace500m().toString(QStringLiteral("m:ss")));

            this->pace->setValue(((rower *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            this->pace->setSecondLine(
                QStringLiteral("AVG: ") +
                ((rower *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                QStringLiteral(" MAX: ") +
                ((rower *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
            this->target_pace->setValue(
                ((rower *)bluetoothManager->device())->lastRequestedPace().toString(QStringLiteral("m:ss")));
            if (trainProgram) {
                this->target_pace->setSecondLine(((rower *)bluetoothManager->device())
                                                     ->speedToPace(trainProgram->currentRow().lower_speed)
                                                     .toString(QStringLiteral("m:ss")) +
                                                 " - " +
                                                 ((rower *)bluetoothManager->device())
                                                     ->speedToPace(trainProgram->currentRow().upper_speed)
                                                     .toString(QStringLiteral("m:ss")));

                if (((rower *)bluetoothManager->device())->lastRequestedCadence().value() > 0) {
                    if (bluetoothManager->device()->currentSpeed().value() <= trainProgram->currentRow().upper_speed &&
                        bluetoothManager->device()->currentSpeed().value() >= trainProgram->currentRow().lower_speed) {
                        this->target_zone->setValueFontColor(QStringLiteral("limegreen"));
                        this->pace->setValueFontColor(QStringLiteral("limegreen"));
                    } else if (bluetoothManager->device()->currentSpeed().value() <=
                                   (trainProgram->currentRow().upper_speed + 0.2) &&
                               bluetoothManager->device()->currentSpeed().value() >=
                                   (trainProgram->currentRow().lower_speed - 0.2)) {
                        this->target_zone->setValueFontColor(QStringLiteral("orange"));
                        this->pace->setValueFontColor(QStringLiteral("orange"));
                    } else {
                        this->target_zone->setValueFontColor(QStringLiteral("red"));
                        this->pace->setValueFontColor(QStringLiteral("red"));
                    }
                } else {
                    this->target_zone->setValueFontColor(QStringLiteral("white"));
                    this->pace->setValueFontColor(QStringLiteral("white"));
                }
                switch (trainProgram->currentRow().pace_intensity) {
                case 0:
                    this->target_zone->setValue(tr("Rec."));
                    break;
                case 1:
                    this->target_zone->setValue(tr("Easy"));
                    break;
                case 2:
                    this->target_zone->setValue(tr("Moder."));
                    break;
                case 3:
                    this->target_zone->setValue(tr("Chall."));
                    break;
                case 4:
                    this->target_zone->setValue(tr("Max"));
                    break;
                default:
                    this->target_zone->setValue(tr("N/A"));
                    break;
                }
            }
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * 1000.0, 'f', 0));
            resistance = ((rower *)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((rower *)bluetoothManager->device())->pelotonResistance().value();
            totalStrokes = ((rower *)bluetoothManager->device())->currentStrokesCount().value();
            avgStrokesRate = ((rower *)bluetoothManager->device())->currentCadence().average();
            maxStrokesRate = ((rower *)bluetoothManager->device())->currentCadence().max();
            avgStrokesLength = ((rower *)bluetoothManager->device())->currentStrokesLength().average();
            this->strokesCount->setValue(
                QString::number(((rower *)bluetoothManager->device())->currentStrokesCount().value(), 'f', 0));
            this->strokesLength->setValue(
                QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().value(), 'f', 1));

            this->target_speed->setValue(QString::number(
                ((rower *)bluetoothManager->device())->lastRequestedSpeed().value() * unit_conversion, 'f', 1));

            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(
                QString::number(((rower *)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(
                ((rower *)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->target_cadence->setValue(
                QString::number(((rower *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
            this->target_power->setValue(
                QString::number(((rower *)bluetoothManager->device())->lastRequestedPower().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance, 'f', 0));

            this->resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((rower *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((rower *)bluetoothManager->device())->currentResistance().max(), 'f', 0));
            this->peloton_resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((rower *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((rower *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(
                QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) + QStringLiteral("% @0%=") +
                QString::number(
                    bluetoothManager->device()->difficult() *
                        settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                            .toDouble() *
                        settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset)
                            .toDouble(),
                    'f', 0));
            this->strokesLength->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().max(), 'f', 1));

            // if there is no training program, the color is based on presets
            if (!trainProgram || trainProgram->currentRow().speed == -1) {
                if (bluetoothManager->device()->currentSpeed().value() < 8) {
                    speed->setValueFontColor(QStringLiteral("white"));
                    this->pace->setValueFontColor(QStringLiteral("white"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 10) {
                    speed->setValueFontColor(QStringLiteral("limegreen"));
                    this->pace->setValueFontColor(QStringLiteral("limegreen"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 11) {
                    speed->setValueFontColor(QStringLiteral("gold"));
                    this->pace->setValueFontColor(QStringLiteral("gold"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 12) {
                    speed->setValueFontColor(QStringLiteral("orange"));
                    this->pace->setValueFontColor(QStringLiteral("orange"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 13) {
                    speed->setValueFontColor(QStringLiteral("darkorange"));
                    this->pace->setValueFontColor(QStringLiteral("darkorange"));
                } else if (bluetoothManager->device()->currentSpeed().value() < 14) {
                    speed->setValueFontColor(QStringLiteral("orangered"));
                    this->pace->setValueFontColor(QStringLiteral("orangered"));
                } else {
                    speed->setValueFontColor(QStringLiteral("red"));
                    this->pace->setValueFontColor(QStringLiteral("red"));
                }
            }

        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::JUMPROPE) {
                odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
                if (bluetoothManager->device()->currentSpeed().value()) {
                    pace = 10000 / (((treadmill *)bluetoothManager->device())->currentPace().second() +
                                    (((treadmill *)bluetoothManager->device())->currentPace().minute() * 60));
                    if (pace < 0) {
                        pace = 0;
                    }
                } else {

                    pace = 0;
                }
                stepCount = ((jumprope *)bluetoothManager->device())->JumpsCount.value();
                inclination = ((jumprope *)bluetoothManager->device())->JumpsSequence.value();
                if (((jumprope *)bluetoothManager->device())->currentSpeed().value() > 2)
                    this->pace->setValue(
                        ((jumprope *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
                else
                    this->pace->setValue("N/A");
                this->pace->setSecondLine(
                    QStringLiteral("AVG: ") +
                    ((jumprope *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                    QStringLiteral(" MAX: ") +
                    ((jumprope *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
                this->inclination->setValue(QString::number(inclination, 'f', 0));
                this->inclination->setSecondLine("");
                this->stepCount->setValue(QString::number(stepCount, 'f', 0));

                // Sequence of jumps resetted and number of jumps > 0, so i have to start a new lap
                if(inclination == 0 && ((jumprope *)bluetoothManager->device())->JumpsCount.lapValue() > 0)
                    lapTrigger = true;

        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {

            if (((elliptical *)bluetoothManager->device())->currentSpeed().value() > 2)
                this->pace->setValue(
                    ((elliptical *)bluetoothManager->device())->currentPace().toString(QStringLiteral("m:ss")));
            else
                this->pace->setValue("N/A");
            this->pace->setSecondLine(
                QStringLiteral("AVG: ") +
                ((elliptical *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                QStringLiteral(" MAX: ") +
                ((elliptical *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")));
            odometer->setValue(QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 2));
            resistance = ((elliptical *)bluetoothManager->device())->currentResistance().value();
            peloton_resistance = ((elliptical *)bluetoothManager->device())->pelotonResistance().value();
            this->peloton_resistance->setValue(QString::number(peloton_resistance, 'f', 0));
            this->target_resistance->setValue(
                QString::number(((elliptical *)bluetoothManager->device())->lastRequestedResistance().value(), 'f', 0));
            this->target_peloton_resistance->setValue(QString::number(
                ((elliptical *)bluetoothManager->device())->lastRequestedPelotonResistance().value(), 'f', 0));
            this->resistance->setValue(QString::number(resistance));
            this->peloton_resistance->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((elliptical *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                QStringLiteral(" MAX: ") +
                QString::number(((elliptical *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0));
            this->target_resistance->setSecondLine(
                QString::number(bluetoothManager->device()->difficult() * 100.0, 'f', 0) + QStringLiteral("% @0%=") +
                QString::number(
                    bluetoothManager->device()->difficult() *
                        settings.value(QZSettings::bike_resistance_gain_f, QZSettings::default_bike_resistance_gain_f)
                            .toDouble() *
                        settings.value(QZSettings::bike_resistance_offset, QZSettings::default_bike_resistance_offset)
                            .toDouble(),
                    'f', 0));
            inclination = ((elliptical *)bluetoothManager->device())->currentInclination().value();
            this->inclination->setValue(QString::number(inclination, 'f', 1));
            this->inclination->setSecondLine(
                QStringLiteral("AVG: ") +
                QString::number(((elliptical *)bluetoothManager->device())->currentInclination().average(), 'f', 1) +
                QStringLiteral(" MAX: ") +
                QString::number(((elliptical *)bluetoothManager->device())->currentInclination().max(), 'f', 1));
            elevation->setValue(QString::number(((elliptical *)bluetoothManager->device())->elevationGain().value() *
                                                    meter_feet_conversion,
                                                'f', (miles ? 0 : 1)));
            elevation->setSecondLine(
                QString::number(((elliptical *)bluetoothManager->device())->elevationGain().rate1s() * 60.0 *
                                    meter_feet_conversion,
                                'f', (miles ? 0 : 1)) +
                " /min");
            this->gears->setValue(QString::number(((elliptical *)bluetoothManager->device())->gears()));
            this->target_speed->setValue(QString::number(
                ((elliptical *)bluetoothManager->device())->lastRequestedSpeed().value() * unit_conversion, 'f', 1));

            this->target_cadence->setValue(
                QString::number(((elliptical *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
        }
        watt->setSecondLine(
            QStringLiteral("AVG: ") + QString::number((bluetoothManager->device())->wattsMetric().average(), 'f', 0) +
            QStringLiteral(" MAX: ") + QString::number((bluetoothManager->device())->wattsMetric().max(), 'f', 0));

        if (trainProgram) {
            int8_t lower_requested_peloton_resistance = trainProgram->currentRow().lower_requested_peloton_resistance;
            int8_t upper_requested_peloton_resistance = trainProgram->currentRow().upper_requested_peloton_resistance;
            double lower_requested_peloton_resistance_to_bike_resistance = 0;
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                lower_requested_peloton_resistance_to_bike_resistance =
                    ((bike *)bluetoothManager->device())->pelotonToBikeResistance(lower_requested_peloton_resistance);
            else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING)
                lower_requested_peloton_resistance_to_bike_resistance =
                    ((rower *)bluetoothManager->device())->pelotonToBikeResistance(lower_requested_peloton_resistance);
            else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL)
                lower_requested_peloton_resistance_to_bike_resistance =
                    ((elliptical *)bluetoothManager->device())
                        ->pelotonToEllipticalResistance(lower_requested_peloton_resistance);

            if (lower_requested_peloton_resistance != -1) {
                this->target_peloton_resistance->setSecondLine(
                    QStringLiteral("MIN: ") + QString::number(lower_requested_peloton_resistance, 'f', 0) +
                    QStringLiteral(" MAX: ") + QString::number(upper_requested_peloton_resistance, 'f', 0));
            } else {
                this->target_peloton_resistance->setSecondLine(QLatin1String(""));
            }

            if (settings
                    .value(QZSettings::tile_peloton_resistance_color_enabled,
                           QZSettings::default_tile_peloton_resistance_color_enabled)
                    .toBool()) {
                if (lower_requested_peloton_resistance == -1) {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("white"));
                } else if (resistance < lower_requested_peloton_resistance_to_bike_resistance) {
                    // we need to compare the real resistance and not the peloton resistance because most of the bikes
                    // have a 1:3 conversion so this compare will be always true even if the actual resistance is the
                    // same #1608
                    this->peloton_resistance->setValueFontColor(QStringLiteral("red"));
                } else if (((int8_t)qRound(peloton_resistance)) <= upper_requested_peloton_resistance) {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("limegreen"));
                } else {
                    this->peloton_resistance->setValueFontColor(QStringLiteral("orange"));
                }
            }

            int16_t lower_cadence = trainProgram->currentRow().lower_cadence;
            int16_t upper_cadence = trainProgram->currentRow().upper_cadence;
            if (lower_cadence != -1) {
                this->target_cadence->setSecondLine(QStringLiteral("MIN: ") + QString::number(lower_cadence, 'f', 0) +
                                                    QStringLiteral(" MAX: ") + QString::number(upper_cadence, 'f', 0));
            } else {
                this->target_cadence->setSecondLine(QLatin1String(""));
            }

            if (settings.value(QZSettings::tile_cadence_color_enabled, QZSettings::default_tile_cadence_color_enabled)
                    .toBool()) {
                if (lower_cadence == -1) {
                    this->cadence->setValueFontColor(QStringLiteral("white"));
                } else if (cadence < lower_cadence) {
                    this->cadence->setValueFontColor(QStringLiteral("red"));
                } else if (cadence <= upper_cadence) {
                    this->cadence->setValueFontColor(QStringLiteral("limegreen"));
                } else {
                    this->cadence->setValueFontColor(QStringLiteral("orange"));
                }
            }
        }

        double ftpPerc = 0;
        QString ftpMinW = QStringLiteral("0");
        QString ftpMaxW = QStringLiteral("0");
        double requestedPerc = 0;
        double requestedZone = 1;
        QString requestedMinW = QStringLiteral("0");
        QString requestedMaxW = QStringLiteral("0");

        if (ftpSetting > 0) {
            ftpPerc = (watts / ftpSetting) * 100.0;
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                requestedPerc =
                    (((bike *)bluetoothManager->device())->lastRequestedPower().value() / ftpSetting) * 100.0;
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                requestedPerc =
                    (((rower *)bluetoothManager->device())->lastRequestedPower().value() / ftpSetting) * 100.0;
            }
        }
        if (ftpPerc < 56) {
            ftpMinW = QString::number(0, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.55, 'f', 0);
            ftpZone = 1;
            ftpZone += (ftpPerc / 56);
            if (ftpZone >= 1.95) { // double precision could cause unwanted approximation
                ftpZone = 1.9;
            }
            ftp->setValueFontColor(QStringLiteral("white"));
            watt->setValueFontColor(QStringLiteral("white"));
        } else if (ftpPerc < 76) {

            ftpMinW = QString::number((ftpSetting * 0.55) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.75, 'f', 0);
            ftpZone = 2;
            ftpZone += ((ftpPerc - 56) / 20);
            if (ftpZone >= 2.95) { // double precision could cause unwanted approximation
                ftpZone = 2.9;
            }
            ftp->setValueFontColor(QStringLiteral("limegreen"));
            watt->setValueFontColor(QStringLiteral("limegreen"));
        } else if (ftpPerc < 91) {

            ftpMinW = QString::number((ftpSetting * 0.75) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 0.90, 'f', 0);
            ftpZone = 3;
            ftpZone += ((ftpPerc - 76) / 15);
            if (ftpZone >= 3.95) { // double precision could cause unwanted approximation
                ftpZone = 3.9;
            }
            ftp->setValueFontColor(QStringLiteral("gold"));
            watt->setValueFontColor(QStringLiteral("gold"));
        } else if (ftpPerc < 106) {

            ftpMinW = QString::number((ftpSetting * 0.90) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.05, 'f', 0);
            ftpZone = 4;
            ftpZone += ((ftpPerc - 91) / 15);
            if (ftpZone >= 4.95) { // double precision could cause unwanted approximation
                ftpZone = 4.9;
            }
            ftp->setValueFontColor(QStringLiteral("orange"));
            watt->setValueFontColor(QStringLiteral("orange"));
        } else if (ftpPerc < 121) {

            ftpMinW = QString::number((ftpSetting * 1.05) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.20, 'f', 0);
            ftpZone = 5;
            ftpZone += ((ftpPerc - 106) / 15);
            if (ftpZone >= 5.95) { // double precision could cause unwanted approximation
                ftpZone = 5.9;
            }
            ftp->setValueFontColor(QStringLiteral("darkorange"));
            watt->setValueFontColor(QStringLiteral("darkorange"));
        } else if (ftpPerc < 151) {

            ftpMinW = QString::number((ftpSetting * 1.20) + 1, 'f', 0);
            ftpMaxW = QString::number(ftpSetting * 1.50, 'f', 0);
            ftpZone = 6;
            ftpZone += ((ftpPerc - 121) / 30);
            if (ftpZone >= 6.95) { // double precision could cause unwanted approximation
                ftpZone = 6.9;
            }
            ftp->setValueFontColor(QStringLiteral("orangered"));
            watt->setValueFontColor(QStringLiteral("orangered"));
        } else {

            ftpMinW = QString::number((ftpSetting * 1.50) + 1, 'f', 0);
            ftpMaxW = QStringLiteral("∞");
            ftpZone = 7;

            ftp->setValueFontColor(QStringLiteral("red"));
            watt->setValueFontColor(QStringLiteral("red"));
        }
        bluetoothManager->device()->setPowerZone(ftpZone);
        ftp->setValue(QStringLiteral("Z") + QString::number(ftpZone, 'f', 1));
        ftp->setSecondLine(ftpMinW + QStringLiteral("-") + ftpMaxW + QStringLiteral("W ") +
                           QString::number(ftpPerc, 'f', 0) + QStringLiteral("%"));

        if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE ||
            (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING &&
             (!trainProgram || trainProgram->currentRow().pace_intensity == -1))) {
            if (requestedPerc < 56) {

                requestedMinW = QString::number(0, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 0.55, 'f', 0);
                requestedZone = 1;
                requestedZone += (requestedPerc / 56);
                if (requestedZone >= 2) { // double precision could cause unwanted approximation
                    requestedZone = 1.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("white"));
            } else if (requestedPerc < 76) {

                requestedMinW = QString::number((ftpSetting * 0.55) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 0.75, 'f', 0);
                requestedZone = 2;
                requestedZone += ((requestedPerc - 56) / 20);
                if (requestedZone >= 3) { // double precision could cause unwanted approximation
                    requestedZone = 2.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("limegreen"));
            } else if (requestedPerc < 91) {

                requestedMinW = QString::number((ftpSetting * 0.75) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 0.90, 'f', 0);
                requestedZone = 3;
                requestedZone += ((requestedPerc - 76) / 15);
                if (requestedZone >= 4) { // double precision could cause unwanted approximation
                    requestedZone = 3.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("gold"));
            } else if (requestedPerc < 106) {

                requestedMinW = QString::number((ftpSetting * 0.90) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 1.05, 'f', 0);
                requestedZone = 4;
                requestedZone += ((requestedPerc - 91) / 15);
                if (requestedZone >= 5) { // double precision could cause unwanted approximation
                    requestedZone = 4.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("orange"));
            } else if (requestedPerc < 121) {

                requestedMinW = QString::number((ftpSetting * 1.05) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 1.20, 'f', 0);
                requestedZone = 5;
                requestedZone += ((requestedPerc - 106) / 15);
                if (requestedZone >= 6) { // double precision could cause unwanted approximation
                    requestedZone = 5.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("darkorange"));
            } else if (requestedPerc < 151) {

                requestedMinW = QString::number((ftpSetting * 1.20) + 1, 'f', 0);
                requestedMaxW = QString::number(ftpSetting * 1.50, 'f', 0);
                requestedZone = 6;
                requestedZone += ((requestedPerc - 121) / 30);
                if (requestedZone >= 7) { // double precision could cause unwanted approximation
                    requestedZone = 6.9999;
                }
                target_zone->setValueFontColor(QStringLiteral("orangered"));
            } else {

                requestedMinW = QString::number((ftpSetting * 1.50) + 1, 'f', 0);
                requestedMaxW = QStringLiteral("∞");
                requestedZone = 7;

                target_zone->setValueFontColor(QStringLiteral("red"));
            }
            bluetoothManager->device()->setTargetPowerZone(requestedZone);
            target_zone->setValue(QStringLiteral("Z") + QString::number(requestedZone, 'f', 1));
            target_zone->setSecondLine(requestedMinW + QStringLiteral("-") + requestedMaxW + QStringLiteral("W ") +
                                       QString::number(requestedPerc, 'f', 0) + QStringLiteral("%"));
        }

        QString Z;
        double maxHeartRate = heartRateMax();
        double percHeartRate = (bluetoothManager->device()->currentHeart().value() * 100) / maxHeartRate;
        double hrCurrentZoneRangeMin = 0;
        double hrCurrentZoneRangeMax = maxHeartRate;

        if (percHeartRate <
            settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble()) {
            currentHRZone = 1;
            currentHRZone +=
                (percHeartRate /
                 settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble());
            if (currentHRZone >= 2) { // double precision could cause unwanted approximation
                currentHRZone = 1.9999;
            }
            hrCurrentZoneRangeMax =
                ((settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble() *
                  maxHeartRate) /
                 100) -
                1;
            heart->setValueFontColor(QStringLiteral("lightsteelblue"));
        } else if (percHeartRate <
                   settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble()) {
            currentHRZone = 2;
            currentHRZone +=
                ((percHeartRate -
                  settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble()) /
                 (settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble() -
                  settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble()));
            if (currentHRZone >= 3) { // double precision could cause unwanted approximation
                currentHRZone = 2.9999;
            }
            hrCurrentZoneRangeMin =
                (settings.value(QZSettings::heart_rate_zone1, QZSettings::default_heart_rate_zone1).toDouble() *
                 maxHeartRate) /
                100;
            hrCurrentZoneRangeMax =
                ((settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble() *
                  maxHeartRate) /
                 100) -
                1;
            heart->setValueFontColor(QStringLiteral("green"));
        } else if (percHeartRate <
                   settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble()) {
            currentHRZone = 3;
            currentHRZone +=
                ((percHeartRate -
                  settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble()) /
                 (settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble() -
                  settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble()));
            if (currentHRZone >= 4) { // double precision could cause unwanted approximation
                currentHRZone = 3.9999;
            }
            hrCurrentZoneRangeMin =
                (settings.value(QZSettings::heart_rate_zone2, QZSettings::default_heart_rate_zone2).toDouble() *
                 maxHeartRate) /
                100;
            hrCurrentZoneRangeMax =
                ((settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble() *
                  maxHeartRate) /
                 100) -
                1;
            heart->setValueFontColor(QStringLiteral("yellow"));
        } else if (percHeartRate <
                   settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4).toDouble()) {
            currentHRZone = 4;
            currentHRZone +=
                ((percHeartRate -
                  settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble()) /
                 (settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4).toDouble() -
                  settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble()));
            if (currentHRZone >= 5) { // double precision could cause unwanted approximation
                currentHRZone = 4.9999;
            }
            hrCurrentZoneRangeMin =
                (settings.value(QZSettings::heart_rate_zone3, QZSettings::default_heart_rate_zone3).toDouble() *
                 maxHeartRate) /
                100;
            hrCurrentZoneRangeMax =
                ((settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4).toDouble() *
                  maxHeartRate) /
                 100) -
                1;
            heart->setValueFontColor(QStringLiteral("orange"));
        } else {
            currentHRZone = 5;
            heart->setValueFontColor(QStringLiteral("red"));
            hrCurrentZoneRangeMin =
                (settings.value(QZSettings::heart_rate_zone4, QZSettings::default_heart_rate_zone4).toDouble() *
                 maxHeartRate) /
                100;
        }
        pidHR->setValue(QString::number(treadmill_pid_heart_zone));
        pidHR->setSecondLine(QString::number(hrCurrentZoneRangeMin) + "-" + QString::number(hrCurrentZoneRangeMax));
        switch (treadmill_pid_heart_zone) {
        case 5:
            pidHR->setValueFontColor(QStringLiteral("red"));
            break;
        case 4:
            pidHR->setValueFontColor(QStringLiteral("orange"));
            break;
        case 3:
            pidHR->setValueFontColor(QStringLiteral("yellow"));
            break;
        case 2:
            pidHR->setValueFontColor(QStringLiteral("green"));
            break;
        case 1:
            pidHR->setValueFontColor(QStringLiteral("lightsteelblue"));
            break;
        default:
        case 0:
            pidHR->setValueFontColor(QStringLiteral("white"));
            break;
        }
        bluetoothManager->device()->setHeartZone(currentHRZone);
        Z = QStringLiteral("Z") + QString::number(currentHRZone, 'f', 1);
        heart->setSecondLine(Z + QStringLiteral(" AVG: ") +
                             QString::number((bluetoothManager->device())->currentHeart().average(), 'f', 0) +
                             QStringLiteral(" MAX: ") +
                             QString::number((bluetoothManager->device())->currentHeart().max(), 'f', 0));

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

        qDebug() << "homeform::update tiles updated!";

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_cadence, QZSettings::default_ant_cadence).toBool() &&
            KeepAwakeHelper::antObject(false)) {
            double v = bluetoothManager->device()->currentSpeed().value();
            v *= settings.value(QZSettings::ant_speed_gain, QZSettings::default_ant_speed_gain).toDouble();
            v += settings.value(QZSettings::ant_speed_offset, QZSettings::default_ant_speed_offset).toDouble();
            KeepAwakeHelper::antObject(false)->callMethod<void>("setCadenceSpeedPower", "(FII)V", (float)v, (int)watts,
                                                                (int)cadence);

            long distanceMeters = (long)(bluetoothManager->device()->odometer() * 1000.0);
            
            // Get heart rate
            int heartRate = (int)bluetoothManager->device()->currentHeart().value();
            
            // Calculate elapsed time in seconds
            double elapsedTimeSeconds = (double)(bluetoothManager->device()->elapsedTime().second() +
                (bluetoothManager->device()->elapsedTime().minute() * 60) +
                (bluetoothManager->device()->elapsedTime().hour() * 3600));
            
            // Get resistance and inclination values
            int resistance = 0;
            double inclination = 0.0;
            
            if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                resistance = (int)((bike*)bluetoothManager->device())->currentResistance().value();
                inclination = ((bike*)bluetoothManager->device())->currentInclination().value();
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
                resistance = (int)((elliptical*)bluetoothManager->device())->currentResistance().value();
                inclination = ((elliptical*)bluetoothManager->device())->currentInclination().value();
            } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                resistance = (int)((rower*)bluetoothManager->device())->currentResistance().value();
            }
            
            // Call the extended metrics update via JNI
            KeepAwakeHelper::antObject(false)->callMethod<void>("updateBikeTransmitterExtendedMetrics", 
                "(JIDID)V", 
                distanceMeters, 
                heartRate, 
                elapsedTimeSeconds, 
                resistance, 
                inclination);                                      
        }
#endif

        if (settings.value(QZSettings::trainprogram_random, QZSettings::default_trainprogram_random).toBool()) {
            if (!paused && !stopped) {

                static QRandomGenerator r;
                r.seed(QDateTime::currentDateTime().toMSecsSinceEpoch());
                static uint32_t last_seconds = 0;
                uint32_t seconds = bluetoothManager->device()->elapsedTime().second() +
                                   (bluetoothManager->device()->elapsedTime().minute() * 60) +
                                   (bluetoothManager->device()->elapsedTime().hour() * 3600);
                if ((seconds / 60) <
                    settings.value(QZSettings::trainprogram_total, QZSettings::default_trainprogram_total).toUInt()) {
                    qDebug() << QStringLiteral("trainprogram random seconds ") << QString::number(seconds) <<
                                    QStringLiteral(" last_change ") << last_seconds << QStringLiteral(" period ") <<
                                    settings
                                        .value(QZSettings::trainprogram_period_seconds,
                                               QZSettings::default_trainprogram_period_seconds)
                                        .toUInt();
                    if (last_seconds == 0 ||
                        ((seconds - last_seconds) >= settings
                                                         .value(QZSettings::trainprogram_period_seconds,
                                                                QZSettings::default_trainprogram_period_seconds)
                                                         .toUInt())) {
                        bool done = false;

                        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
                            ((treadmill *)bluetoothManager->device())->currentSpeed().value() > 0.0f) {
                            double speed = settings
                                               .value(QZSettings::trainprogram_speed_min,
                                                      QZSettings::default_trainprogram_speed_min)
                                               .toDouble();
                            double incline = settings
                                                 .value(QZSettings::trainprogram_incline_min,
                                                        QZSettings::default_trainprogram_incline_min)
                                                 .toDouble();
                            if (!speed) {
                                speed = 1.0;
                            }
                            if (settings.value(QZSettings::trainprogram_speed_min,
                                               QZSettings::default_trainprogram_speed_min)
                                        .toDouble() != 0 &&
                                settings.value(QZSettings::trainprogram_speed_min,
                                               QZSettings::default_trainprogram_speed_min)
                                        .toDouble() < settings
                                                          .value(QZSettings::trainprogram_speed_max,
                                                                 QZSettings::default_trainprogram_speed_max)
                                                          .toDouble()) {
                                speed =
                                    (double)r.bounded((uint32_t)(settings
                                                                     .value(QZSettings::trainprogram_speed_min,
                                                                            QZSettings::default_trainprogram_speed_min)
                                                                     .toDouble() *
                                                                 10.0),
                                                      (uint32_t)(settings
                                                                     .value(QZSettings::trainprogram_speed_max,
                                                                            QZSettings::default_trainprogram_speed_max)
                                                                     .toDouble() *
                                                                 10.0)) /
                                    10.0;
                            }
                            if (settings
                                    .value(QZSettings::trainprogram_incline_min,
                                           QZSettings::default_trainprogram_incline_min)
                                    .toDouble() < settings
                                                      .value(QZSettings::trainprogram_incline_max,
                                                             QZSettings::default_trainprogram_incline_max)
                                                      .toDouble()) {
                                incline = (double)r.bounded(
                                              (uint32_t)(settings
                                                             .value(QZSettings::trainprogram_incline_min,
                                                                    QZSettings::default_trainprogram_incline_min)
                                                             .toDouble() *
                                                         10.0),
                                              (uint32_t)(settings
                                                             .value(QZSettings::trainprogram_incline_max,
                                                                    QZSettings::default_trainprogram_incline_max)
                                                             .toDouble() *
                                                         10.0)) /
                                          10.0;
                            }
                            ((treadmill *)bluetoothManager->device())->changeSpeedAndInclination(speed, incline);
                            done = true;
                        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                            double resistance = settings
                                                    .value(QZSettings::trainprogram_resistance_min,
                                                           QZSettings::default_trainprogram_resistance_min)
                                                    .toUInt();
                            if (settings
                                    .value(QZSettings::trainprogram_resistance_min,
                                           QZSettings::default_trainprogram_resistance_min)
                                    .toUInt() < settings
                                                    .value(QZSettings::trainprogram_resistance_max,
                                                           QZSettings::default_trainprogram_resistance_max)
                                                    .toUInt()) {
                                resistance =
                                    (double)r.bounded(settings
                                                          .value(QZSettings::trainprogram_resistance_min,
                                                                 QZSettings::default_trainprogram_resistance_min)
                                                          .toUInt(),
                                                      settings
                                                          .value(QZSettings::trainprogram_resistance_max,
                                                                 QZSettings::default_trainprogram_resistance_max)
                                                          .toUInt());
                            }
                            ((bike *)bluetoothManager->device())->changeResistance(resistance);

                            done = true;
                        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
                            double resistance = settings
                                                    .value(QZSettings::trainprogram_resistance_min,
                                                           QZSettings::default_trainprogram_resistance_min)
                                                    .toUInt();
                            if (settings
                                    .value(QZSettings::trainprogram_resistance_min,
                                           QZSettings::default_trainprogram_resistance_min)
                                    .toUInt() < settings
                                                    .value(QZSettings::trainprogram_resistance_max,
                                                           QZSettings::default_trainprogram_resistance_max)
                                                    .toUInt()) {
                                resistance =
                                    (double)r.bounded(settings
                                                          .value(QZSettings::trainprogram_resistance_min,
                                                                 QZSettings::default_trainprogram_resistance_min)
                                                          .toUInt(),
                                                      settings
                                                          .value(QZSettings::trainprogram_resistance_max,
                                                                 QZSettings::default_trainprogram_resistance_max)
                                                          .toUInt());
                            }
                            ((rower *)bluetoothManager->device())->changeResistance(resistance);

                            done = true;
                        }

                        if (done) {
                            if (last_seconds == 0) {

                                r.seed(QDateTime::currentDateTime().currentMSecsSinceEpoch());
                                last_seconds = 1; // in order to avoid to re-enter here again if the user doesn't ride
                            } else {

                                last_seconds = seconds;
                            }
                        }
                    }
                } else if (bluetoothManager->device()->currentSpeed().value() > 0) {
                    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

                        ((treadmill *)bluetoothManager->device())->changeSpeedAndInclination(0, 0);
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

                        ((bike *)bluetoothManager->device())->changeResistance(1);
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                        ((rower *)bluetoothManager->device())->changeResistance(1);
                    }
                }
            }
        } else if (!settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                        .toString()
                        .contains(QStringLiteral("Disabled")) ||
                   (trainProgram && trainProgram->currentRow().zoneHR >= 0)) {
            static uint32_t last_seconds_pid_heart_zone = 0;
            static uint32_t pid_heart_zone_small_inc_counter = 0;
            uint32_t seconds = bluetoothManager->device()->elapsedTime().second() +
                               (bluetoothManager->device()->elapsedTime().minute() * 60) +
                               (bluetoothManager->device()->elapsedTime().hour() * 3600);
            uint8_t delta = 10;
            bool trainprogram_pid_pushy = settings.value(QZSettings::trainprogram_pid_pushy, QZSettings::default_trainprogram_pid_pushy).toBool();
            bool fromTrainProgram = trainProgram && trainProgram->currentRow().zoneHR >= 0;
            double maxSpeed = 30;
            double minSpeed = 0;
            int8_t maxResistance = 100;
            static double lastInclination = 0;
            static double lastWattage = 0;

            if (fromTrainProgram) {
                delta = trainProgram->currentRow().loopTimeHR;
            }

            if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
                !settings.value(QZSettings::trainprogram_pid_ignore_inclination, QZSettings::default_trainprogram_pid_ignore_inclination).toBool() &&
                bluetoothManager->device()->currentInclination().value() != lastInclination && lastWattage != 0) {
                last_seconds_pid_heart_zone = seconds;

                double weightKg = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
                double newspeed = 0;
                double bestSpeed = 0.1;
                double bestDifference = fabs(((treadmill *)bluetoothManager->device())->wattsCalc(weightKg, bestSpeed, bluetoothManager->device()->currentInclination().value()) - lastWattage);
                for (int speed = 1; speed <= 300; speed++) {
                    double s = ((double)speed) / 10.0;
                    double thisDifference = fabs(((treadmill *)bluetoothManager->device())->wattsCalc(weightKg, s, bluetoothManager->device()->currentInclination().value()) - lastWattage);
                    if (thisDifference < bestDifference) {
                        bestDifference = thisDifference;
                        bestSpeed = s;
                    }
                }
                // Now bestSpeed is the speed closest to the desired wattage
                newspeed = bestSpeed;
                qDebug() << QStringLiteral("changing speed to") << newspeed << "due to inclination changed";
                ((treadmill *)bluetoothManager->device())->changeSpeedAndInclination(newspeed, ((treadmill *)bluetoothManager->device())->currentInclination().value());
            }

            lastInclination = bluetoothManager->device()->currentInclination().value();
            lastWattage = bluetoothManager->device()->wattsMetric().value();

            if (last_seconds_pid_heart_zone == 0 || ((seconds - last_seconds_pid_heart_zone) >= delta)) {

                last_seconds_pid_heart_zone = seconds;

                uint8_t zone =
                    settings.value(QZSettings::treadmill_pid_heart_zone, QZSettings::default_treadmill_pid_heart_zone)
                        .toString()
                        .toUInt();
                if (fromTrainProgram) {
                    zone = trainProgram->currentRow().zoneHR;
                    if (zone > 0) {
                        settings.setValue(QZSettings::treadmill_pid_heart_zone, QString::number(zone));                        
                    } else {
                        settings.setValue(QZSettings::treadmill_pid_heart_zone, QStringLiteral("Disabled"));
                    }
                    if (trainProgram->currentRow().maxSpeed > 0) {
                        maxSpeed = trainProgram->currentRow().maxSpeed;
                    }
                    if (trainProgram->currentRow().minSpeed > 0) {
                        minSpeed = trainProgram->currentRow().minSpeed;
                    }
                    if (trainProgram->currentRow().maxResistance > 0) {
                        maxResistance = trainProgram->currentRow().maxResistance;
                    }
                }

                if (!stopped && !paused && bluetoothManager->device()->currentHeart().value() && zone > 0 &&
                    bluetoothManager->device()->currentSpeed().value() > 0.0f) {
                    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

                        const double step = 0.2;
                        double currentSpeed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                        if (zone < ((uint8_t)currentHRZone) && minSpeed <= currentSpeed + step) {
                            ((treadmill *)bluetoothManager->device())
                                ->changeSpeedAndInclination(
                                    currentSpeed - step,
                                    ((treadmill *)bluetoothManager->device())->currentInclination().value());
                            pid_heart_zone_small_inc_counter = 0;
                        } else if (zone > ((uint8_t)currentHRZone) && maxSpeed >= currentSpeed + step) {
                            ((treadmill *)bluetoothManager->device())
                                ->changeSpeedAndInclination(

                                    currentSpeed + step,
                                    ((treadmill *)bluetoothManager->device())->currentInclination().value());
                            pid_heart_zone_small_inc_counter = 0;
                        } else if (maxSpeed >= currentSpeed + step && trainprogram_pid_pushy) {
                            pid_heart_zone_small_inc_counter++;
                            if (fabs(((float)zone) - currentHRZone) < 0.5 && pid_heart_zone_small_inc_counter > (10 * fabs(((float)zone) - currentHRZone))) {
                                ((treadmill *)bluetoothManager->device())
                                    ->changeSpeedAndInclination(
                                        currentSpeed + step,
                                        ((treadmill *)bluetoothManager->device())->currentInclination().value());
                                pid_heart_zone_small_inc_counter = 0;
                            }
                        }
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
                        double step = 1;
                        bool ergMode = ((bike*)bluetoothManager->device())->ergModeSupportedAvailableBySoftware();
                        if(ergMode) {
                            step = settings.value(QZSettings::pid_heart_zone_erg_mode_watt_step, QZSettings::default_pid_heart_zone_erg_mode_watt_step).toInt();
                        }
                        resistance_t currentResistance =
                            ((bike *)bluetoothManager->device())->currentResistance().value();
                        double current_target_watt = ((bike *)bluetoothManager->device())->lastRequestedPower().value();
                        if (zone < ((uint8_t)currentHRZone)) {
                            if(ergMode)
                                ((bike *)bluetoothManager->device())->changePower(current_target_watt - step);
                            else
                                ((bike *)bluetoothManager->device())->changeResistance(currentResistance - step);
                            pid_heart_zone_small_inc_counter = 0;
                        } else if (zone > ((uint8_t)currentHRZone) && ((maxResistance >= currentResistance + step && !ergMode) || ergMode)) {
                            if(ergMode)
                                ((bike *)bluetoothManager->device())->changePower(current_target_watt + step);
                            else
                                ((bike *)bluetoothManager->device())->changeResistance(currentResistance + step);
                            pid_heart_zone_small_inc_counter = 0;
                        } else if(trainprogram_pid_pushy) {
                            pid_heart_zone_small_inc_counter++;
                            if (pid_heart_zone_small_inc_counter > (5 * fabs(((float)zone) - currentHRZone))) {
                                if(ergMode)
                                    ((bike *)bluetoothManager->device())->changePower(current_target_watt + step);
                                else
                                    ((bike *)bluetoothManager->device())->changeResistance(currentResistance + step);
                                pid_heart_zone_small_inc_counter = 0;
                            }
                        }
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                        const int step = 1;
                        resistance_t currentResistance =
                            ((rower *)bluetoothManager->device())->currentResistance().value();
                        if (zone < ((uint8_t)currentHRZone)) {

                            ((rower *)bluetoothManager->device())->changeResistance(currentResistance - step);
                        } else if (zone > ((uint8_t)currentHRZone)) {

                            ((rower *)bluetoothManager->device())->changeResistance(currentResistance + step);
                        }
                    }
                }
            }
        } else if ((settings.value(QZSettings::treadmill_pid_heart_min, QZSettings::default_treadmill_pid_heart_min)
                            .toInt() > 0 &&
                    settings.value(QZSettings::treadmill_pid_heart_max, QZSettings::default_treadmill_pid_heart_max)
                            .toInt() > 0) ||
                   (trainProgram && trainProgram->currentRow().HRmin > 0 && trainProgram->currentRow().HRmax > 0)) {
            static uint32_t last_seconds_pid_heart_zone = 0;
            static uint32_t pid_heart_zone_small_inc_counter = 0;
            bool trainprogram_pid_pushy = settings.value(QZSettings::trainprogram_pid_pushy, QZSettings::default_trainprogram_pid_pushy).toBool();
            uint32_t seconds = bluetoothManager->device()->elapsedTime().second() +
                               (bluetoothManager->device()->elapsedTime().minute() * 60) +
                               (bluetoothManager->device()->elapsedTime().hour() * 3600);
            uint8_t delta = 10;
            bool fromTrainProgram =
                trainProgram && trainProgram->currentRow().HRmin > 0 && trainProgram->currentRow().HRmax > 0;
            double maxSpeed = 30;
            double minSpeed = 0;
            int8_t maxResistance = 100;

            if (fromTrainProgram) {
                delta = trainProgram->currentRow().loopTimeHR;
            }

            if (last_seconds_pid_heart_zone == 0 || ((seconds - last_seconds_pid_heart_zone) >= delta)) {

                last_seconds_pid_heart_zone = seconds;

                int16_t hrmin =
                    settings.value(QZSettings::treadmill_pid_heart_min, QZSettings::default_treadmill_pid_heart_min)
                        .toInt();
                int16_t hrmax =
                    settings.value(QZSettings::treadmill_pid_heart_max, QZSettings::default_treadmill_pid_heart_max)
                        .toInt();
                if (fromTrainProgram) {
                    hrmin = trainProgram->currentRow().HRmin;
                    hrmax = trainProgram->currentRow().HRmax;
                    if (trainProgram->currentRow().maxSpeed > 0) {
                        maxSpeed = trainProgram->currentRow().maxSpeed;
                    }
                    if (trainProgram->currentRow().minSpeed > 0) {
                        minSpeed = trainProgram->currentRow().minSpeed;
                    }
                    if (trainProgram->currentRow().maxResistance > 0) {
                        maxResistance = trainProgram->currentRow().maxResistance;
                    }
                }

                if (hrmax == 0 || hrmax == -1)
                    hrmax = 220;

                if (!stopped && !paused && bluetoothManager->device()->currentHeart().value() &&
                    bluetoothManager->device()->currentSpeed().value() > 0.0f) {
                    if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {

                        const double step = 0.2;
                        double currentSpeed = ((treadmill *)bluetoothManager->device())->currentSpeed().value();
                        if (hrmax < bluetoothManager->device()->currentHeart().average20s() &&
                            minSpeed <= currentSpeed - step) {
                            ((treadmill *)bluetoothManager->device())
                                ->changeSpeedAndInclination(
                                    currentSpeed - step,
                                    ((treadmill *)bluetoothManager->device())->currentInclination().value());
                            pid_heart_zone_small_inc_counter = 0;
                        } else if (hrmin > bluetoothManager->device()->currentHeart().average20s() &&
                                   maxSpeed >= currentSpeed + step) {
                            ((treadmill *)bluetoothManager->device())
                                ->changeSpeedAndInclination(

                                    currentSpeed + step,
                                    ((treadmill *)bluetoothManager->device())->currentInclination().value());
                            pid_heart_zone_small_inc_counter = 0;
                        } else if (maxSpeed >= currentSpeed + step &&
                                   hrmax < bluetoothManager->device()->currentHeart().average20s() && trainprogram_pid_pushy) {
                            pid_heart_zone_small_inc_counter++;
                            if (pid_heart_zone_small_inc_counter > (30 / abs(hrmax - bluetoothManager->device()->currentHeart().average20s()))) {
                                ((treadmill *)bluetoothManager->device())
                                    ->changeSpeedAndInclination(
                                        currentSpeed + step,
                                        ((treadmill *)bluetoothManager->device())->currentInclination().value());
                                pid_heart_zone_small_inc_counter = 0;
                            }
                        }
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {

                        const int step = 1;
                        resistance_t currentResistance =
                            ((bike *)bluetoothManager->device())->currentResistance().value();
                        if (hrmax < bluetoothManager->device()->currentHeart().average20s()) {

                            ((bike *)bluetoothManager->device())->changeResistance(currentResistance - step);
                        } else if (hrmin > bluetoothManager->device()->currentHeart().average20s() &&
                                   maxResistance >= currentResistance + step) {

                            ((bike *)bluetoothManager->device())->changeResistance(currentResistance + step);
                        }
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {

                        const int step = 1;
                        resistance_t currentResistance =
                            ((rower *)bluetoothManager->device())->currentResistance().value();
                        if (hrmax < bluetoothManager->device()->currentHeart().average20s()) {

                            ((rower *)bluetoothManager->device())->changeResistance(currentResistance - step);
                        } else if (hrmin > bluetoothManager->device()->currentHeart().average20s()) {

                            ((rower *)bluetoothManager->device())->changeResistance(currentResistance + step);
                        }
                    }
                }
            }
        }

        if (settings.value(QZSettings::fitmetria_fanfit_enable, QZSettings::default_fitmetria_fanfit_enable).toBool()) {
            if (!settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                     .toString()
                     .compare(QStringLiteral("Manual"))) {
                // do nothing here, the user change the fan value with the tile
            } else if (paused || stopped) {
                qDebug() << QStringLiteral("fitmetria_fanfit paused or stopped mode");
                bluetoothManager->device()->changeFanSpeed(0);
            }
            // Heart Mode
            else if (!settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                          .toString()
                          .compare(QStringLiteral("Heart"))) {
                qDebug() << QStringLiteral("fitmetria_fanfit heart mode")
                         << bluetoothManager->device()->currentHeart().value();
                const uint8_t min = 80;
                uint8_t v = 0;
                if (bluetoothManager->device()->currentHeart().value() > min && maxHeartRate > min)
                    v = ((bluetoothManager->device()->currentHeart().value() - min) * 100.0) /
                        (double)(maxHeartRate - min);
                bluetoothManager->device()->changeFanSpeed(v + fanOverride);
            }
            // Power Mode
            else if (!settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                          .toString()
                          .compare(QStringLiteral("Power"))) {
                qDebug() << QStringLiteral("fitmetria_fanfit power mode") << watts;
                const double percOverFtp = 1.20;
                const double min = 50;
                uint8_t v = 0;
                double a = (double)(((ftpSetting * percOverFtp) - min));
                if (watts >= min && a > 0)
                    v = ((watts - min) * 100.0) / a;
                bluetoothManager->device()->changeFanSpeed(v + fanOverride);
            }
            // Wind mode
            else if (!settings.value(QZSettings::fitmetria_fanfit_mode, QZSettings::default_fitmetria_fanfit_mode)
                          .toString()
                          .compare(QStringLiteral("Wind"))) {
                // Todo
                qDebug() << QStringLiteral("fitmetria_fanfit wind mode");
                // bluetoothManager->device()->changeFanSpeed((ftpZone - 1) * 1.5);
            }
        }

        if (!stopped && !paused) {
            if(settings.value(QZSettings::autolap_distance, QZSettings::default_autolap_distance).toDouble() != 0) {
                if (bluetoothManager->device()->currentDistance().lapValue() >=
                    settings.value(QZSettings::autolap_distance, QZSettings::default_autolap_distance).toDouble()) {
                        qDebug() << QStringLiteral("Autolap based on distance");
                        Lap();
                        setToastRequested("AutoLap " + QString::number(settings.value(QZSettings::autolap_distance, QZSettings::default_autolap_distance).toDouble(), 'f', 1));
                }
            }

            if (settings.value(QZSettings::tts_enabled, QZSettings::default_tts_enabled).toBool()) {
                static double tts_speed_played = 0;
                bool description =
                    settings.value(QZSettings::tts_description_enabled, QZSettings::default_tts_description_enabled)
                        .toBool();
                if (m_speech.state() == QTextToSpeech::Ready) {
                    if (++tts_summary_count >=
                        settings.value(QZSettings::tts_summary_sec, QZSettings::default_tts_summary_sec).toInt()) {
                        tts_summary_count = 0;

                        QString s;
                        if (settings.value(QZSettings::tts_act_speed, QZSettings::default_tts_act_speed).toBool())
                            s.append(
                                (description ? tr(", speed ") : ",") +
                                (!miles ? QString::number(bluetoothManager->device()->currentSpeed().value(), 'f', 1) +
                                              (description ? tr(" kilometers per hour") : "")
                                        : QString::number(bluetoothManager->device()->currentSpeed().value() *
                                                              unit_conversion,
                                                          'f', 1)) +
                                (description ? tr(" miles per hour") : ""));
                        if (settings.value(QZSettings::tts_avg_speed, QZSettings::default_tts_avg_speed).toBool())
                            s.append((description ? tr(", Average speed ") : ",") +
                                     (!miles ? QString::number(bluetoothManager->device()->currentSpeed().average(),
                                                               'f', 1) +
                                                   (description ? tr("kilometers per hour") : "")
                                             : QString::number(bluetoothManager->device()->currentSpeed().average() *
                                                                   unit_conversion,
                                                               'f', 1)) +
                                     (description ? tr(" miles per hour") : ""));
                        if (settings.value(QZSettings::tts_max_speed, QZSettings::default_tts_max_speed).toBool())
                            s.append((description ? tr(", Max speed ") : ",") +
                                     (!miles
                                          ? QString::number(bluetoothManager->device()->currentSpeed().max(), 'f', 1) +
                                                (description ? tr(" kilometers per hour") : "")
                                          : QString::number(bluetoothManager->device()->currentSpeed().max() *
                                                                unit_conversion,
                                                            'f', 1)) +
                                     (description ? tr(" miles per hour") : ""));
                        if (settings.value(QZSettings::tts_act_inclination, QZSettings::default_tts_act_inclination)
                                .toBool())
                            s.append((description ? tr(", inclination ") : ",") +
                                     QString::number(bluetoothManager->device()->currentInclination().value(), 'f', 1));
                        if (settings.value(QZSettings::tts_act_cadence, QZSettings::default_tts_act_cadence).toBool())
                            s.append((description ? tr(", cadence ") : ",") +
                                     QString::number(bluetoothManager->device()->currentCadence().value(), 'f', 0));
                        if (settings.value(QZSettings::tts_avg_cadence, QZSettings::default_tts_avg_cadence).toBool())
                            s.append((description ? tr(", Average cadence ") : ",") +
                                     QString::number(bluetoothManager->device()->currentCadence().average(), 'f', 0));
                        if (settings.value(QZSettings::tts_max_cadence, QZSettings::default_tts_max_cadence /* true */)
                                .toBool())
                            s.append((description ? tr(", Max cadence ") : ",") +
                                     QString::number(bluetoothManager->device()->currentCadence().max()));
                        if (settings.value(QZSettings::tts_act_elevation, QZSettings::default_tts_act_elevation)
                                .toBool())
                            s.append(
                                (description ? tr(", elevation ") : ",") +
                                (!miles ? QString::number(bluetoothManager->device()->elevationGain().value(), 'f', 1) +
                                              (description ? tr(" meters") : "")
                                        : QString::number(bluetoothManager->device()->elevationGain().value() *
                                                              meter_feet_conversion,
                                                          'f', 1)) +
                                (description ? tr(" feet") : ""));
                        if (settings.value(QZSettings::tts_act_calories, QZSettings::default_tts_act_calories).toBool())
                            s.append((description ? tr(", calories burned ") : ",") +
                                     QString::number(bluetoothManager->device()->calories().value(), 'f', 0));
                        if (settings.value(QZSettings::tts_act_odometer, QZSettings::default_tts_act_odometer).toBool())
                            s.append((description ? tr(", distance ") : ",") +
                                     (!miles ? QString::number(bluetoothManager->device()->odometer(), 'f', 1) +
                                                   (description ? tr("kilometers") : "")
                                             : QString::number(bluetoothManager->device()->odometer() * unit_conversion,
                                                               'f', 1)) +
                                     (description ? tr(" miles") : ""));
                        if (settings.value(QZSettings::tts_act_target_pace, QZSettings::default_tts_act_target_pace)
                                .toBool()) {
                            if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING)
                                s.append((description ? tr(", pace ") : ",") + ((rower *)bluetoothManager->device())
                                                                                   ->lastRequestedPace()
                                                                                   .toString(QStringLiteral("m:ss")));
                            else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
                                s.append((description ? tr(", pace ") : ",") + ((treadmill *)bluetoothManager->device())
                                                                                   ->lastRequestedPace()
                                                                                   .toString(QStringLiteral("m:ss")));
                        }
                        if (settings.value(QZSettings::tts_act_pace, QZSettings::default_tts_act_pace).toBool())
                            s.append((description ? tr(", pace ") : ",") +
                                     bluetoothManager->device()->currentPace().toString(QStringLiteral("m:ss")));
                        if (settings.value(QZSettings::tts_avg_pace, QZSettings::default_tts_avg_pace).toBool())
                            s.append((description ? tr(", pace ") : ",") +
                                     bluetoothManager->device()->averagePace().toString(QStringLiteral("m:ss")));
                        if (settings.value(QZSettings::tts_max_pace, QZSettings::default_tts_max_pace).toBool())
                            s.append((description ? tr(", pace ") : ",") +
                                     bluetoothManager->device()->maxPace().toString(QStringLiteral("m:ss")));
                        if (settings.value(QZSettings::tts_act_resistance, QZSettings::default_tts_act_resistance)
                                .toBool())
                            s.append((description ? tr(", resistance ") : ",") +
                                     QString::number(bluetoothManager->device()->currentResistance().value(), 'f', 0));
                        if (settings.value(QZSettings::tts_avg_resistance, QZSettings::default_tts_avg_resistance)
                                .toBool())
                            s.append(
                                (description ? tr(", average resistance ") : ",") +
                                QString::number(bluetoothManager->device()->currentResistance().average(), 'f', 0));
                        if (settings.value(QZSettings::tts_max_resistance, QZSettings::default_tts_max_resistance)
                                .toBool())
                            s.append((description ? tr(", max resistance ") : ",") +
                                     QString::number(bluetoothManager->device()->currentResistance().max(), 'f', 0));
                        if (settings.value(QZSettings::tts_act_watt, QZSettings::default_tts_act_watt).toBool())
                            s.append((description ? tr(", watt ") : ",") +
                                     QString::number(bluetoothManager->device()->wattsMetric().value(), 'f', 0));
                        if (settings.value(QZSettings::tts_avg_watt, QZSettings::default_tts_avg_watt).toBool())
                            s.append((description ? tr(", average watt ") : ",") +
                                     QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0));
                        if (settings.value(QZSettings::tts_max_watt, QZSettings::default_tts_max_watt).toBool())
                            s.append((description ? tr(", max watt ") : ",") +
                                     QString::number(bluetoothManager->device()->wattsMetric().max(), 'f', 0));
                        if (settings.value(QZSettings::tts_act_ftp, QZSettings::default_tts_act_ftp /* true */)
                                .toBool())
                            s.append((description ? tr(", ftp ") : ",") + QString::number(ftpZone, 'f', 1));
                        if (settings.value(QZSettings::tts_act_heart, QZSettings::default_tts_act_heart).toBool())
                            s.append((description ? tr(", heart rate ") : ",") +
                                     QString::number(bluetoothManager->device()->currentHeart().value(), 'f', 0));
                        if (settings.value(QZSettings::tts_avg_heart, QZSettings::default_tts_avg_heart).toBool())
                            s.append((description ? tr(", average heart rate ") : ",") +
                                     QString::number(bluetoothManager->device()->currentHeart().average(), 'f', 0));
                        if (settings.value(QZSettings::tts_max_heart, QZSettings::default_tts_max_heart).toBool())
                            s.append((description ? tr(", max heart rate ") : ",") +
                                     QString::number(bluetoothManager->device()->currentHeart().max(), 'f', 0));
                        if (settings.value(QZSettings::tts_act_jouls, QZSettings::default_tts_act_jouls).toBool())
                            s.append((description ? tr(", jouls ") : ",") +
                                     QString::number(bluetoothManager->device()->jouls().max(), 'f', 0));
                        if (settings.value(QZSettings::tts_act_elapsed, QZSettings::default_tts_act_elapsed).toBool())
                            s.append((description ? tr(", elapsed ") : ",") +
                                     QString::number(bluetoothManager->device()->elapsedTime().minute()) +
                                     (description ? tr(" minutes ") : "") +
                                     QString::number(bluetoothManager->device()->elapsedTime().second()) +
                                     (description ? tr(" seconds") : ""));
                        if (settings
                                .value(QZSettings::tts_act_peloton_resistance,
                                       QZSettings::default_tts_act_peloton_resistance)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", peloton resistance ") : ",") +
                                     QString::number(((bike *)bluetoothManager->device())->pelotonResistance().value(),
                                                     'f', 0));
                        if (settings
                                .value(QZSettings::tts_avg_peloton_resistance,
                                       QZSettings::default_tts_avg_peloton_resistance)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", average peloton resistance ") : ",") +
                                     QString::number(
                                         ((bike *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0));
                        if (settings
                                .value(QZSettings::tts_max_peloton_resistance,
                                       QZSettings::default_tts_max_peloton_resistance)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", max peloton resistance ") : ",") +
                                     QString::number(((bike *)bluetoothManager->device())->pelotonResistance().max(),
                                                     'f', 0));
                        if (settings
                                .value(QZSettings::tts_act_target_peloton_resistance,
                                       QZSettings::default_tts_act_target_peloton_resistance)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", target peloton resistance ") : ",") +
                                     QString::number(
                                         ((bike *)bluetoothManager->device())->lastRequestedPelotonResistance().value(),
                                         'f', 0));
                        if (settings
                                .value(QZSettings::tts_act_target_cadence, QZSettings::default_tts_act_target_cadence)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", target cadence ") : ",") +
                                     QString::number(
                                         ((bike *)bluetoothManager->device())->lastRequestedCadence().value(), 'f', 0));
                        if (settings.value(QZSettings::tts_act_target_power, QZSettings::default_tts_act_target_power)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", target power ") : ",") +
                                     QString::number(((bike *)bluetoothManager->device())->lastRequestedPower().value(),
                                                     'f', 0));
                        if (settings.value(QZSettings::tts_act_target_zone, QZSettings::default_tts_act_target_zone)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE)
                            s.append((description ? tr(", target zone ") : ",") +
                                     QString::number(requestedZone, 'f', 1));
                        if (settings.value(QZSettings::tts_act_target_speed, QZSettings::default_tts_act_target_speed)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
                            s.append(
                                (description ? tr(", target speed ") : ",") +
                                (!miles ? QString::number(
                                              ((treadmill *)bluetoothManager->device())->lastRequestedSpeed().value(),
                                              'f', 1) +
                                              (description ? tr(" kilometers per hour") : "")
                                        : QString::number(
                                              ((treadmill *)bluetoothManager->device())->lastRequestedSpeed().value() *
                                                  unit_conversion,
                                              'f', 1)) +
                                (description ? tr(" miles per hour") : ""));
                        if (settings
                                .value(QZSettings::tts_act_target_incline, QZSettings::default_tts_act_target_incline)
                                .toBool() &&
                            bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL)
                            s.append((description ? tr(", target incline ") : ",") +
                                     QString::number(
                                         ((treadmill *)bluetoothManager->device())->lastRequestedInclination().value(),
                                         'f', 1));
                        if (settings.value(QZSettings::tts_act_watt_kg, QZSettings::default_tts_act_watt_kg).toBool())
                            s.append((description ? tr(", watt for kilograms ") : ",") +
                                     QString::number(bluetoothManager->device()->wattKg().value(), 'f', 1));
                        if (settings.value(QZSettings::tts_avg_watt_kg, QZSettings::default_tts_avg_watt_kg).toBool())
                            s.append((description ? tr(", average watt for kilograms") : ",") +
                                     QString::number(bluetoothManager->device()->wattKg().average(), 'f', 1));
                        if (settings.value(QZSettings::tts_max_watt_kg, QZSettings::default_tts_max_watt_kg).toBool())
                            s.append((description ? tr(", max watt for kilograms") : ",") +
                                     QString::number(bluetoothManager->device()->wattKg().max(), 'f', 1));

                        qDebug() << "tts" << s;
                        m_speech.say(s);
                    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL &&
                               bluetoothManager->device()->currentSpeed().value() != tts_speed_played &&
                               settings.value(QZSettings::tts_act_speed, QZSettings::default_tts_act_speed).toBool()) {
                        tts_speed_played = bluetoothManager->device()->currentSpeed().value();
                        QString s;
                        s.append((description ? tr("speed changed to") : "") +
                                 (!miles ? QString::number(bluetoothManager->device()->currentSpeed().value(), 'f', 1) +
                                               (description ? tr(" kilometers per hour") : "")
                                         : QString::number(bluetoothManager->device()->currentSpeed().value() *
                                                               unit_conversion,
                                                           'f', 1)) +
                                 (description ? tr(" miles per hour") : ""));
                        qDebug() << "tts" << s;
                        m_speech.say(s);
                    }
                }
            }

            if(bluetoothManager->device()->currentSpeed().value() > 0 && !isinf(bluetoothManager->device()->currentSpeed().value()))
                bluetoothManager->device()->addCurrentDistance1s((bluetoothManager->device()->currentSpeed().value() / 3600.0));
            
            qDebug() << "Current Distance 1s:" << bluetoothManager->device()->currentDistance1s().value() << bluetoothManager->device()->currentSpeed().value() << watts;

            SessionLine s(
                bluetoothManager->device()->currentSpeed().value(), inclination, bluetoothManager->device()->currentDistance1s().value(),
                watts, resistance, peloton_resistance, (uint8_t)bluetoothManager->device()->currentHeart().value(),
                pace, cadence, bluetoothManager->device()->calories().value(),
                bluetoothManager->device()->elevationGain().value(),
                bluetoothManager->device()->elapsedTime().second() +
                    (bluetoothManager->device()->elapsedTime().minute() * 60) +
                    (bluetoothManager->device()->elapsedTime().hour() * 3600),

                lapTrigger, totalStrokes, avgStrokesRate, maxStrokesRate, avgStrokesLength,
                bluetoothManager->device()->currentCordinate(), strideLength, groundContact, verticalOscillation, stepCount,
                bluetoothManager->device()->CoreBodyTemperature.value(), bluetoothManager->device()->SkinTemperature.value(), bluetoothManager->device()->HeatStrainIndex.value());

            Session.append(s);

            if (lapTrigger) {
                lapTrigger = false;
            }

#ifndef Q_OS_IOS
            if (iphone_socket && iphone_socket->state() == QAbstractSocket::ConnectedState) {
                QString toSend =
                    "SENDER=PAD#HR=" + QString::number(bluetoothManager->device()->currentHeart().value()) +
                    "#KCAL=" + QString::number(bluetoothManager->device()->calories().value()) +
                    "#BCAD=" + QString::number(bluetoothManager->device()->currentCadence().value()) +
                    "#SPD=" + QString::number(bluetoothManager->device()->currentSpeed().value()) +
                    "#PWR=" + QString::number(bluetoothManager->device()->wattsMetric().value()) +
                    "#CAD=" + QString::number(bluetoothManager->device()->currentCadence().value()) +
                    "#ODO=" + QString::number(bluetoothManager->device()->odometer()) + "#";
                int write = iphone_socket->write(toSend.toLocal8Bit(), toSend.length());
                qDebug() << "iphone_socket send " << write << toSend;
            }
#endif
        }
        emit workoutStartDateChanged(workoutStartDate());
    }

    emit changeOfdevice();
    emit changeOflap();
}

bool homeform::getDevice() {

    static bool toggle = false;
    if (!this->bluetoothManager->device()) {

        // toggling the bluetooth icon
        toggle = !toggle;
        return toggle;
    }
    return this->bluetoothManager->device()->connected();
}

bool homeform::getLap() {
    if (!this->bluetoothManager->device()) {

        return false;
    }
    return true;
}

QString homeform::getFileNameFromContentUri(const QString &uriString) {
    qDebug() << "getFileNameFromContentUri" << uriString;
    if(!uriString.startsWith("content")) {
        return uriString;
    }
#ifdef Q_OS_ANDROID

    QJniObject jUriString = QJniObject::fromString(uriString);
    QJniObject jUri = QJniObject::callStaticObjectMethod("android/net/Uri", "parse", "(Ljava/lang/String;)Landroid/net/Uri;", jUriString.object<jstring>());
    QJniObject context5 = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "getContext", "()Landroid/content/Context;");
    QJniObject result = QJniObject::callStaticObjectMethod(
        "org/cagnulen/qdomyoszwift/ContentHelper",
        "getFileName",
        "(Landroid/content/Context;Landroid/net/Uri;)Ljava/lang/String;",
        context5.object(),
        jUri.object());
    return result.toString();
#else
    return uriString;
#endif
}

QString homeform::copyAndroidContentsURI(QUrl file, QString subfolder) {
#ifdef Q_OS_ANDROID        
    QString fileNameLocal = "";
    qDebug() << "Android Version:" << QOperatingSystemVersion::current();
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion(QOperatingSystemVersion::Android, 13))
        fileNameLocal = getFileNameFromContentUri(file.toString());
    if(fileNameLocal.contains(getWritableAppDir() + subfolder + "/")) {
        qDebug() << "no need to copy file, the file is already in QZ subfolder" << file << subfolder;
        return file.toString();
    }
    
    QString filename = "";
    QFile fileFile(QQmlFile::urlToLocalFileOrQrc(file));
    // android <14 fallback
    if(fileNameLocal.length() == 0) {
        qDebug() << "android <14 fallback" << fileNameLocal << filename << file.fileName();
        filename = file.fileName();
    } else {
        QFileInfo f(fileNameLocal);
        filename = f.fileName();        
    }
    QString dest = getWritableAppDir() + subfolder + "/" + filename;
    qDebug() << file.fileName() << fileNameLocal << filename;
    QFile::remove(dest);
    bool copy = fileFile.copy(dest);
    qDebug() << "copy" << dest << copy << fileFile.exists() << fileFile.isReadable();
    return dest;
#endif
    return file.toString();
}

void homeform::profile_open_clicked(const QUrl &fileName) {
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
#ifdef Q_OS_ANDROID
    copyAndroidContentsURI(fileName, "profiles");
#else
    QFileInfo fileInfo(file);
    bool r = file.copy(getWritableAppDir() + "profiles/" + fileInfo.fileName());
    qDebug() << "profile copy" << r << getWritableAppDir() + "profiles/" + fileInfo.fileName();
#endif
}

void homeform::trainprogram_open_other_folder(const QUrl &fileName) {
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    copyAndroidContentsURI(fileName, "training");
}

void homeform::gpx_open_other_folder(const QUrl &fileName) {
    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    copyAndroidContentsURI(fileName, "gpx");
}

void homeform::trainprogram_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("trainprogram_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));

    if (!file.fileName().isEmpty()) {
        {
            if (previewTrainProgram) {
                delete previewTrainProgram;
                previewTrainProgram = 0;
            }
            if (trainProgram) {
                delete trainProgram;
            }

            trainProgram = trainprogram::load(file.fileName(), bluetoothManager, file.fileName().right(3).toUpper());

            QString movieName = file.fileName().left(file.fileName().length() - 3) + "mp4";
            if (QFile::exists(movieName)) {
                qDebug() << movieName << QStringLiteral("exist!");
                movieFileName = QUrl::fromLocalFile(movieName);
                emit videoPathChanged(movieFileName);
                setVideoIconVisible(true);
                setVideoRate(1);
                trainingProgram()->setVideoAvailable(true);
            } else {
                qDebug() << movieName << QStringLiteral("doesn't exist!");
                movieFileName = "";
                setVideoIconVisible(false);
                trainingProgram()->setVideoAvailable(false);
            }

            stravaWorkoutName = QFileInfo(file.fileName()).baseName();
            stravaPelotonInstructorName = QStringLiteral("");
            emit workoutNameChanged(workoutName());
            emit instructorNameChanged(instructorName());

            QSettings settings;
            if (settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
                m_info = workoutName();
                emit infoChanged(m_info);
            }
        }

        trainProgramSignals();
    }
}

void homeform::trainprogram_preview(const QUrl &fileName) {
    qDebug() << QStringLiteral("trainprogram_preview") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    QString fileNameLocal = getFileNameFromContentUri(file.fileName());
    qDebug() << fileNameLocal;
    if (!fileNameLocal.isEmpty()) {
        {
            if (previewTrainProgram) {
                delete previewTrainProgram;
                previewTrainProgram = 0;
            }
            previewTrainProgram = trainprogram::load(file.fileName(), bluetoothManager, fileNameLocal.right(3).toUpper());
            emit previewWorkoutPointsChanged(preview_workout_points());
            emit previewWorkoutDescriptionChanged(previewWorkoutDescription());
            emit previewWorkoutTagsChanged(previewWorkoutTags());
        }
    }
}

void homeform::trainprogram_zwo_loaded(const QString &s) {
    qDebug() << QStringLiteral("trainprogram_zwo_loaded") << s;
    trainProgram = new trainprogram(zwiftworkout::loadJSON(s), bluetoothManager);
    if (trainProgram) {
        QJsonDocument doc = QJsonDocument::fromJson(s.toUtf8());
        if (doc.isObject()) {
            QJsonObject obj = doc.object();
            if (obj.contains(QStringLiteral("name"))) {
                stravaPelotonActivityName = obj[QStringLiteral("name")].toString();
                stravaPelotonInstructorName = QStringLiteral("");
                emit workoutNameChanged(workoutName());
                emit instructorNameChanged(instructorName());

                QSettings settings;
                if (!settings.value(QZSettings::top_bar_enabled, QZSettings::default_top_bar_enabled).toBool()) {
                    return;
                }
                m_info = workoutName();
                emit infoChanged(m_info);
            }
        }
    }
    trainProgramSignals();
}

void homeform::gpx_save_clicked() {

    QString path = getWritableAppDir();

    if (bluetoothManager->device()) {
        gpx::save(path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                      QStringLiteral(".gpx"),
                  Session, bluetoothManager->device()->deviceType());
    }
}

void homeform::fit_save_clicked() {

    QString path = getWritableAppDir();
    bluetoothdevice *dev = bluetoothManager->device();
    if (dev) {
        QString filename = path +
                           QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                           QStringLiteral(".fit");

        QString workoutName = "";
        if (!stravaPelotonActivityName.isEmpty() && !stravaPelotonInstructorName.isEmpty())
            workoutName = stravaPelotonActivityName + " - " + stravaPelotonInstructorName;

        qfit::save(filename, Session, dev->deviceType(),
                   qobject_cast<m3ibike *>(dev) ? QFIT_PROCESS_DISTANCENOISE : QFIT_PROCESS_NONE,
                   stravaPelotonWorkoutType, workoutName, dev->bluetoothDevice.name());
        lastFitFileSaved = filename;

        QSettings settings;
        if (!settings.value(QZSettings::strava_accesstoken, QZSettings::default_strava_accesstoken)
                 .toString()
                 .isEmpty()) {

            QString mode = settings.value(QZSettings::strava_upload_mode, QZSettings::default_strava_upload_mode).toString();
            if(mode.startsWith("Always")) { // always
                strava_upload_file_prepare();
            } else if(mode.startsWith("Request")) {
                setStravaUploadRequested(true);
                emit stravaUploadRequestedChanged(true);
            }
        }
    }
}

void homeform::strava_upload_file_prepare() {
    qDebug() << lastFitFileSaved;
    QFile f(lastFitFileSaved);
    f.open(QFile::OpenModeFlag::ReadOnly);
    QByteArray fitfile = f.readAll();
    strava_upload_file(fitfile, lastFitFileSaved);
    f.close();
}

void homeform::gpx_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("gpx_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));

    stravaWorkoutName = QFileInfo(file.fileName()).baseName();
    if (!file.fileName().isEmpty()) {
        {
            if (trainProgram) {

                delete trainProgram;
            }

            // KML to GPX https://www.gpsvisualizer.com/elevation
            gpx g;
            QList<trainrow> list;
            auto g_list = g.open(file.fileName(), bluetoothManager->device() ? bluetoothManager->device()->deviceType() : bluetoothdevice::BIKE);
            if (bluetoothManager->device())
                bluetoothManager->device()->setGPXFile(file.fileName());
            gpx_altitude_point_for_treadmill last;
            quint32 i = 0;
            list.reserve(g_list.size() + 1);
            for (const auto &p : g_list) {
                trainrow r;
                if (p.speed > 0 && i > 0) {
                    QGeoCoordinate p1(last.latitude, last.longitude);
                    QGeoCoordinate p2(p.latitude, p.longitude, p.elevation);
                    r.azimuth = p1.azimuthTo(p2);
                    r.speed = p.speed;
                    r.distance = p.distance;
                    r.duration = QTime(0, 0, 0, 0);
                    r.duration = r.duration.addSecs(p.seconds);
                    r.forcespeed = true;

                    r.altitude = last.elevation;
                    r.inclination = p.inclination;
                    r.latitude = last.latitude;
                    r.longitude = last.longitude;
                    r.gpxElapsed = QTime(0, 0, 0).addSecs(p.seconds);

                    list.append(r);

                } else {
                    if (i > 0) {
                        QGeoCoordinate p1(last.latitude, last.longitude);
                        QGeoCoordinate p2(p.latitude, p.longitude, p.elevation);
                        r.azimuth = p1.azimuthTo(p2);
                        r.distance = p.distance;
                        r.altitude = last.elevation;
                        r.inclination = p.inclination;
                        r.latitude = last.latitude;
                        r.longitude = last.longitude;
                        r.gpxElapsed = QTime(0, 0, 0).addSecs(p.seconds);

                        list.append(r);
                    }
                }

                last = p;
                i++;
            }
            setMapsVisible(true);
            if (g.getVideoURL().isEmpty() == false) {
                movieFileName = QUrl(g.getVideoURL());
                emit videoPathChanged(movieFileName);
                setVideoIconVisible(true);
            } else if (QFile::exists(file.fileName().replace(".gpx", ".mp4"))) {
                movieFileName = QUrl::fromLocalFile(file.fileName().replace(".gpx", ".mp4"));
                emit videoPathChanged(movieFileName);
                setVideoIconVisible(true);
            }
            trainProgram = new trainprogram(list, bluetoothManager, nullptr, nullptr, videoIconVisible());
        }

        trainProgramSignals();
    }
}

void homeform::gpxpreview_open_clicked(const QUrl &fileName) {
    qDebug() << QStringLiteral("gpxpreview_open_clicked") << fileName;

    QFile file(QQmlFile::urlToLocalFileOrQrc(fileName));
    qDebug() << file.fileName();

    if (!file.fileName().isEmpty()) {
        gpx g;
        auto g_list = g.open(file.fileName(), bluetoothManager->device() ? bluetoothManager->device()->deviceType() : bluetoothdevice::BIKE);
        gpx_preview.clearPath();
        for (const auto &p : g_list) {
            gpx_preview.addCoordinate(QGeoCoordinate(p.latitude, p.longitude, p.elevation));
        }
        pathController.setGeoPath(gpx_preview);
        pathController.setCenter(gpx_preview.center());
    }
}

QStringList homeform::bluetoothDevices() {

    QStringList r;
    r.append(QStringLiteral("Disabled"));
    r.append(QStringLiteral("Wifi"));
    for (const QBluetoothDeviceInfo &b : qAsConst(bluetoothManager->devices)) {
        if (!b.name().trimmed().isEmpty()) {

            r.append(b.name());
        }
    }
    return r;
}

QStringList homeform::metrics() { return bluetoothdevice::metrics(); }

QAbstractOAuth::ModifyParametersFunction
homeform::buildModifyParametersFunction(const QUrl &clientIdentifier, const QUrl &clientIdentifierSharedKey) {
    return [clientIdentifier, clientIdentifierSharedKey](QAbstractOAuth::Stage stage, QMultiMap<QString, QVariant> *parameters) {
        if (stage == QAbstractOAuth::Stage::RequestingAuthorization) {
            parameters->insert(QStringLiteral("responseType"), QStringLiteral("code"));
            parameters->insert(QStringLiteral("approval_prompt"), QStringLiteral("force"));
            QByteArray code = parameters->value(QStringLiteral("code")).toByteArray();
            parameters->remove(QStringLiteral("code"));
            parameters->insert(QStringLiteral("code"), QUrl::fromPercentEncoding(code));
        }
        if (stage == QAbstractOAuth::Stage::RefreshingAccessToken) {
            parameters->insert(QStringLiteral("client_id"), clientIdentifier);
            parameters->insert(QStringLiteral("client_secret"), clientIdentifierSharedKey);
        }
    };
}

void homeform::strava_refreshtoken() {

    QSettings settings;
    // QUrlQuery params; //NOTE: clazy-unuse-non-tirial-variable

    if (settings.value(QZSettings::strava_refreshtoken).toString().isEmpty()) {

        strava_connect();
        return;
    }

    QNetworkRequest request(QUrl(QStringLiteral("https://www.strava.com/oauth/token?")));
    request.setRawHeader("Content-Type", "application/x-www-form-urlencoded");

    // set params
    QString data;
    data += QStringLiteral("client_id=" STRAVA_CLIENT_ID_S);
#ifdef STRAVA_SECRET_KEY
    data += "&client_secret=";
    data += STRINGIFY(STRAVA_SECRET_KEY);
#endif
    data += QStringLiteral("&refresh_token=") + settings.value(QZSettings::strava_refreshtoken).toString();
    data += QStringLiteral("&grant_type=refresh_token");

    // make request
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    QNetworkReply *reply = manager->post(request, data.toLatin1());

    // blocking request
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();

    int statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    qDebug() << QStringLiteral("HTTP response code: ") << statusCode;

    // oops, no dice
    if (reply->error() != 0) {
        qDebug() << QStringLiteral("Got error") << reply->errorString().toStdString().c_str();
        setToastRequested("Strava Auth Failed!");
        return;
    }

    // lets extract the access token, and possibly a new refresh token
    QByteArray r = reply->readAll();
    qDebug() << QStringLiteral("Got response:") << r.data();

    QJsonParseError parseError;
    QJsonDocument document = QJsonDocument::fromJson(r, &parseError);

    // failed to parse result !?
    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << tr("JSON parser error") << parseError.errorString();
    }

    QString access_token = document[QStringLiteral("access_token")].toString();
    QString refresh_token = document[QStringLiteral("refresh_token")].toString();

    settings.setValue(QZSettings::strava_accesstoken, access_token);
    settings.setValue(QZSettings::strava_refreshtoken, refresh_token);
    settings.setValue(QZSettings::strava_lastrefresh, QDateTime::currentDateTime());

    setToastRequested("Strava Login OK!");
}

bool homeform::strava_upload_file(const QByteArray &data, const QString &remotename) {

    strava_refreshtoken();

    QSettings settings;
    QString token = settings.value(QZSettings::strava_accesstoken).toString();

    qDebug() << "File size to upload:" << data.size() << "bytes";
    qDebug() << "Remote filename:" << remotename;

    // The V3 API doc said "https://api.strava.com" but it is not working yet
    QUrl url = QUrl(QStringLiteral("https://www.strava.com/api/v3/uploads"));
    QNetworkRequest request = QNetworkRequest(url);

    // QString boundary = QString::number(qrand() * (90000000000) / (RAND_MAX + 1) + 10000000000, 16);
    QString boundary = QVariant(QRandomGenerator::global()->generate()).toString() +
                       QVariant(QRandomGenerator::global()->generate()).toString() +
                       QVariant(QRandomGenerator::global()->generate()).toString(); // NOTE: qrand is deprecated

    // MULTIPART *****************

    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);
    multiPart->setBoundary(boundary.toLatin1());

    QHttpPart accessTokenPart;
    accessTokenPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                              QVariant(QStringLiteral("form-data; name=\"access_token\"")));
    accessTokenPart.setBody(token.toLatin1());
    multiPart->append(accessTokenPart);

    QHttpPart activityNamePart;
    activityNamePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                               QVariant(QStringLiteral("form-data; name=\"name\"")));

    QString prefix = QStringLiteral("");
    if (settings.value(QZSettings::strava_date_prefix, QZSettings::default_strava_date_prefix).toBool())
        prefix = " " + QDate::currentDate().toString(Qt::TextDate);

    // use metadata config if the user selected it
    QString activityName =
        QStringLiteral(" ") + settings.value(QZSettings::strava_suffix, QZSettings::default_strava_suffix).toString();
    if (!stravaPelotonActivityName.isEmpty()) {
        activityName = stravaPelotonActivityName + QStringLiteral(" - ") + stravaPelotonInstructorName + activityName;
        if (pelotonHandler &&
            settings.value(QZSettings::peloton_description_link, QZSettings::default_peloton_description_link).toBool())
            activityDescription =
                QStringLiteral("https://members.onepeloton.com/classes/cycling?modal=classDetailsModal&classId=") +
                pelotonHandler->current_ride_id;
    } else {
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL) {
            activityName = prefix + QStringLiteral("Run") + activityName;
        } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
            activityName = prefix + QStringLiteral("Row") + activityName;
        } else {
            activityName = prefix + QStringLiteral("Ride") + activityName;
        }
    }
    activityNamePart.setHeader(QNetworkRequest::ContentTypeHeader,
                               QVariant(QStringLiteral("text/plain;charset=utf-8")));
    activityNamePart.setBody(activityName.toUtf8());
    if (activityName != QLatin1String("")) {
        multiPart->append(activityNamePart);
    }

    QHttpPart activityDescriptionPart;
    activityDescriptionPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                                      QVariant(QStringLiteral("form-data; name=\"description\"")));
    activityDescriptionPart.setHeader(QNetworkRequest::ContentTypeHeader,
                                      QVariant(QStringLiteral("text/plain;charset=utf-8")));
    activityDescriptionPart.setBody(activityDescription.toUtf8());
    if (activityDescription != QLatin1String("")) {
        multiPart->append(activityDescriptionPart);
    }

    // upload file data
    QString filename = QFileInfo(remotename).baseName();

    QHttpPart dataTypePart;
    dataTypePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                           QVariant(QStringLiteral("form-data; name=\"data_type\"")));
    dataTypePart.setBody("fit");
    multiPart->append(dataTypePart);

    QHttpPart externalIdPart;
    externalIdPart.setHeader(QNetworkRequest::ContentDispositionHeader,
                             QVariant(QStringLiteral("form-data; name=\"external_id\"")));
    externalIdPart.setBody(filename.toStdString().c_str());
    multiPart->append(externalIdPart);

    QHttpPart filePart;
    filePart.setHeader(QNetworkRequest::ContentTypeHeader, QVariant(QStringLiteral("application/octet-stream")));
    filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                       QVariant(QStringLiteral("form-data; name=\"file\"; filename=\"") + remotename +
                                QStringLiteral("\"; type=\"application/octet-stream\"")));
    filePart.setBody(data);
    multiPart->append(filePart);

    // this must be performed asynchronously and call made
    // to notifyWriteCompleted(QString remotename, QString message) when done
    if (manager) {

        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    replyStrava = manager->post(request, multiPart);

   connect(replyStrava, &QNetworkReply::uploadProgress,
            [](qint64 bytesSent, qint64 bytesTotal) {
                qDebug() << "Upload progress:" << bytesSent << "/" << bytesTotal;
            });    

    // catch finished signal
    connect(replyStrava, &QNetworkReply::finished, this, &homeform::writeFileCompleted);
#if (QT_VERSION >= QT_VERSION_CHECK(5, 13, 0))
    connect(replyStrava, &QNetworkReply::errorOccurred, this, &homeform::errorOccurredUploadStrava);
#endif
    return true;
}

void homeform::errorOccurredUploadStrava(QNetworkReply::NetworkError code) {
    qDebug() << "Strava upload error details:";
    qDebug() << "Error code:" << code;
    if(replyStrava) {
        qDebug() << "Error string:" << replyStrava->errorString();
        qDebug() << "HTTP status code:" << replyStrava->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();

        QByteArray errorData = replyStrava->readAll();
        qDebug() << "Error response body:" << QString(errorData);
        
        QJsonDocument jsonResponse = QJsonDocument::fromJson(errorData);
        if (!jsonResponse.isNull()) {
            qDebug() << "JSON error message:" << jsonResponse.toJson();
        }
        
        setToastRequested("Strava Upload Failed: " + replyStrava->errorString());        
    } else {
        setToastRequested("Strava Upload Failed");
    }
}

void homeform::writeFileCompleted() {
    qDebug() << QStringLiteral("strava upload completed!");

    QNetworkReply *reply = static_cast<QNetworkReply *>(QObject::sender());

    QString response = reply->readAll();
    // QString uploadError = QStringLiteral("invalid response or parser error");
    // NOTE: clazy-unused-non-trivial-variable

    qDebug() << "reply:" << response;

    setToastRequested("Strava Upload Completed!");
}

void homeform::onStravaGranted() {

    stravaAuthWebVisible = false;
    stravaWebVisibleChanged(stravaAuthWebVisible);
    QSettings settings;
    settings.setValue(QZSettings::strava_accesstoken, strava->token());
    settings.setValue(QZSettings::strava_refreshtoken, strava->refreshToken());
    settings.setValue(QZSettings::strava_lastrefresh, QDateTime::currentDateTime());
    qDebug() << QStringLiteral("strava authenticated successfully");
    strava_refreshtoken();
    setGeneralPopupVisible(true);
}

void homeform::onStravaAuthorizeWithBrowser(const QUrl &url) {

    // ui->textBrowser->append(tr("Open with browser:") + url.toString());
    QSettings settings;
    bool strava_auth_external_webbrowser =
        settings.value(QZSettings::strava_auth_external_webbrowser, QZSettings::default_strava_auth_external_webbrowser)
            .toBool();
#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS))
    strava_auth_external_webbrowser = true;
#endif
    stravaAuthUrl = url.toString();
    emit stravaAuthUrlChanged(stravaAuthUrl);

    if (strava_auth_external_webbrowser)
        QDesktopServices::openUrl(url);
    else {
        stravaAuthWebVisible = true;
        stravaWebVisibleChanged(stravaAuthWebVisible);
    }
}

void homeform::replyDataReceived(const QByteArray &v) {

    qDebug() << v;

    QByteArray data;
    QSettings settings;
    QString s(v);
    QJsonDocument jsonResponse = QJsonDocument::fromJson(s.toUtf8());
    settings.setValue(QZSettings::strava_accesstoken, jsonResponse[QStringLiteral("access_token")]);
    settings.setValue(QZSettings::strava_refreshtoken, jsonResponse[QStringLiteral("refresh_token")]);
    settings.setValue(QZSettings::strava_expires, jsonResponse[QStringLiteral("expires_at")]);

    qDebug() << "Strava tokens received successfully, expires at:" << jsonResponse[QStringLiteral("expires_at")];

    QString urlstr = QStringLiteral("https://www.strava.com/oauth/token?");
    QUrlQuery params;
    params.addQueryItem(QStringLiteral("client_id"), QStringLiteral(STRAVA_CLIENT_ID_S));
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    params.addQueryItem("client_secret", STRINGIFY(STRAVA_SECRET_KEY));
#endif

    params.addQueryItem(QStringLiteral("code"), strava_code);
    data.append(params.query(QUrl::FullyEncoded).toUtf8());

    // trade-in the temporary access code retrieved by the Call-Back URL for the finale token
    QUrl url = QUrl(urlstr);

    QNetworkRequest request = QNetworkRequest(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, QStringLiteral("application/x-www-form-urlencoded"));

    // now get the final token - but ignore errors
    if (manager) {

        delete manager;
        manager = 0;
    }
    manager = new QNetworkAccessManager(this);
    // connect(manager, SIGNAL(sslErrors(QNetworkReply*, const QList<QSslError> & )), this,
    // SLOT(onSslErrors(QNetworkReply*, const QList<QSslError> & ))); connect(manager,
    // SIGNAL(finished(QNetworkReply*)), this, SLOT(networkRequestFinished(QNetworkReply*)));
    manager->post(request, data);
}

void homeform::onSslErrors(QNetworkReply *reply, const QList<QSslError> &error) {

    reply->ignoreSslErrors();
    qDebug() << QStringLiteral("homeform::onSslErrors") << error;
}

void homeform::networkRequestFinished(QNetworkReply *reply) {

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
            refresh_token = document[QStringLiteral("refresh_token")].toString();
            access_token = document[QStringLiteral("access_token")].toString();
        }

        settings.setValue(QZSettings::strava_accesstoken, access_token);
        settings.setValue(QZSettings::strava_refreshtoken, refresh_token);
        settings.setValue(QZSettings::strava_lastrefresh, QDateTime::currentDateTime());

        qDebug() << "Strava tokens refreshed successfully";

    } else {

        // general error getting response
        QString error =
            QString(tr("Error retrieving access token, %1 (%2)")).arg(reply->errorString()).arg(reply->error());
        qDebug() << error << reply->url() << reply->readAll();
    }
}

void homeform::callbackReceived(const QVariantMap &values) {
    qDebug() << QStringLiteral("homeform::callbackReceived") << values;
    if (!values.value(QZSettings::code).toString().isEmpty()) {
        strava_code = values.value(QZSettings::code).toString();

        qDebug() << strava_code;
    }
}

QOAuth2AuthorizationCodeFlow *homeform::strava_connect() {
    if (manager) {

        delete manager;
        manager = nullptr;
    }
    if (strava) {

        delete strava;
        strava = nullptr;
    }
    if (stravaReplyHandler) {

        delete stravaReplyHandler;
        stravaReplyHandler = nullptr;
    }
    manager = new QNetworkAccessManager(this);
    OAuth2Parameter parameter;
    strava = new QOAuth2AuthorizationCodeFlow(manager, this);
    strava->setScope(QStringLiteral("activity:read_all,activity:write"));
    strava->setClientIdentifier(QStringLiteral(STRAVA_CLIENT_ID_S));
    strava->setAuthorizationUrl(QUrl(QStringLiteral("https://www.strava.com/oauth/authorize")));
    strava->setAccessTokenUrl(QUrl(QStringLiteral("https://www.strava.com/oauth/token")));
#ifdef STRAVA_SECRET_KEY
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    strava->setClientIdentifierSharedKey(STRINGIFY(STRAVA_SECRET_KEY));
#elif defined(WIN32)
#pragma message("DEFINE STRAVA_SECRET_KEY!!!")
#else
#pragma message "DEFINE STRAVA_SECRET_KEY!!!"
#endif
    strava->setModifyParametersFunction(
        buildModifyParametersFunction(QUrl(QLatin1String("")), QUrl(QLatin1String(""))));
    stravaReplyHandler = new QOAuthHttpServerReplyHandler(QHostAddress(QStringLiteral("127.0.0.1")), 8091, this);
    connect(stravaReplyHandler, &QOAuthHttpServerReplyHandler::replyDataReceived, this, &homeform::replyDataReceived);
    connect(stravaReplyHandler, &QOAuthHttpServerReplyHandler::callbackReceived, this, &homeform::callbackReceived);

    strava->setReplyHandler(stravaReplyHandler);

    return strava;
}

void homeform::strava_connect_clicked() {
    QLoggingCategory::setFilterRules(QStringLiteral("qt.networkauth.*=true"));

    strava_connect();
    connect(strava, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser, this, &homeform::onStravaAuthorizeWithBrowser);
    connect(strava, &QOAuth2AuthorizationCodeFlow::granted, this, &homeform::onStravaGranted);

    strava->grant();
    // qDebug() <<
    // QAbstractOAuth2::post("https://www.strava.com/oauth/authorize?client_id=7976&scope=activity:read_all,activity:write&redirect_uri=http://127.0.0.1&response_type=code&approval_prompt=force");
}

bool homeform::generalPopupVisible() { return m_generalPopupVisible; }

void homeform::setGeneralPopupVisible(bool value) {

    m_generalPopupVisible = value;
    emit generalPopupVisibleChanged(m_generalPopupVisible);
}

bool homeform::pelotonPopupVisible() { return m_pelotonPopupVisible; }

void homeform::setPelotonPopupVisible(bool value) {

    m_pelotonPopupVisible = value;
    emit pelotonPopupVisibleChanged(m_pelotonPopupVisible);
}

bool homeform::licensePopupVisible() { return m_LicensePopupVisible; }

void homeform::setLicensePopupVisible(bool value) {

    m_LicensePopupVisible = value;
    emit licensePopupVisibleChanged(m_LicensePopupVisible);
}

bool homeform::mapsVisible() { return m_MapsVisible; }

void homeform::setMapsVisible(bool value) {

    m_MapsVisible = value;
    emit mapsVisibleChanged(m_MapsVisible);
}

bool homeform::videoIconVisible() { return m_VideoIconVisible; }

void homeform::setVideoIconVisible(bool value) {

    m_VideoIconVisible = value;
    emit videoIconVisibleChanged(m_VideoIconVisible);
}

bool homeform::chartIconVisible() { return m_ChartIconVisible; }

void homeform::setChartIconVisible(bool value) {

    m_ChartIconVisible = value;
    emit chartIconVisibleChanged(m_ChartIconVisible);
}

int homeform::videoPosition() { return m_VideoPosition; }

void homeform::setVideoPosition(int value) {

    m_VideoPosition = value;
    emit videoPositionChanged(m_VideoPosition);
}

double homeform::videoRate() { return m_VideoRate; }

void homeform::setVideoRate(double value) {

    m_VideoRate = value;
    emit videoRateChanged(m_VideoRate);
}

void homeform::smtpError(SmtpClient::SmtpError e) { qDebug() << QStringLiteral("SMTP ERROR") << e; }

QByteArray homeform::currentPelotonImage() {
    if (pelotonHandler && pelotonHandler->current_image_downloaded &&
        !pelotonHandler->current_image_downloaded->downloadedData().isEmpty())
        return pelotonHandler->current_image_downloaded->downloadedData();
    return QByteArray();
}

void homeform::sendMail() {

    QSettings settings;

    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    double unit_conversion = 1.0;
    double meter_feet_conversion = 1.0;
    QString meter_feet_unit = QStringLiteral("meters");
    QString weightLossUnit = QStringLiteral("Kg");
    double WeightLoss = 0;

    // TODO: add a condition to avoid sending mail when the user look at the chart while is riding
    if (settings.value(QZSettings::user_email, QZSettings::default_user_email).toString().length() == 0 ||
        !bluetoothManager->device()) {
        return;
    }

    if (miles) {
        unit_conversion = 0.621371; // clang, don't touch it!
        weightLossUnit = QStringLiteral("Oz");
        meter_feet_conversion = 3.28084;
        meter_feet_unit = QStringLiteral("feet");
    }
    WeightLoss = (miles ? bluetoothManager->device()->weightLoss() * 35.274 : bluetoothManager->device()->weightLoss());

#ifdef SMTP_SERVER
#define _STR(x) #x
#define STRINGIFY(x) _STR(x)
    SmtpClient smtp(STRINGIFY(SMTP_SERVER), 587, SmtpClient::TlsConnection);
    connect(&smtp, SIGNAL(smtpError(SmtpClient::SmtpError)), this, SLOT(smtpError(SmtpClient::SmtpError)));
#else
#pragma message "stmp server is unset!"
    SmtpClient smtp(QLatin1String(""), 25, SmtpClient::TlsConnection);
    return;
#endif

    // Now we create a MimeMessage object. This will be the email.

    MimeMessage message;
    EmailAddress sender(QStringLiteral("no-reply@qzapp.it"), QStringLiteral("QZ"));
    message.setSender(sender);
    
    EmailAddress recipient(settings.value(QZSettings::user_email, QLatin1String("")).toString(),
                           settings.value(QZSettings::user_email, QLatin1String("")).toString());
    message.addRecipient(recipient);
  
    if (!Session.isEmpty()) {
        QString title = Session.constFirst().time.toString();
        if (!stravaPelotonActivityName.isEmpty()) {
            title +=
                QStringLiteral(" ") + stravaPelotonActivityName + QStringLiteral(" - ") + stravaPelotonInstructorName;
        }
        message.setSubject(title);
    } else {
        message.setSubject(QStringLiteral("Test"));
    }

    // Now add some text to the email.
    // First we create a MimeText object.

    MimeText text;

    QString textMessage = QStringLiteral("Great workout!\n\n");

    if (pelotonHandler) {
        if (!pelotonHandler->current_ride_id.isEmpty()) {
            textMessage +=
                stravaPelotonActivityName + QStringLiteral(" - ") + stravaPelotonInstructorName +
                QStringLiteral(" https://members.onepeloton.com/classes/cycling?modal=classDetailsModal&classId=") +
                pelotonHandler->current_ride_id;
        }
    }

    textMessage += '\n';
    textMessage += QStringLiteral("Average Speed: ") +
                   QString::number(bluetoothManager->device()->currentSpeed().average() * unit_conversion, 'f', 1) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Max Speed: ") +
                   QString::number(bluetoothManager->device()->currentSpeed().max() * unit_conversion, 'f', 1) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Calories burned: ") +
                   QString::number(bluetoothManager->device()->calories().value(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Distance: ") +
                   QString::number(bluetoothManager->device()->odometer() * unit_conversion, 'f', 1) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Elevation Gain (") + meter_feet_unit + "): " +
                   QString::number(bluetoothManager->device()->elevationGain().value() * meter_feet_conversion, 'f',
                                   (miles ? 0 : 1)) +
                   QStringLiteral("\n");
    textMessage += QStringLiteral("Average Watt: ") +
                   QString::number(bluetoothManager->device()->wattsMetric().average(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Max Watt: ") +
                   QString::number(bluetoothManager->device()->wattsMetric().max(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Average Watt/Kg: ") +
                   QString::number(bluetoothManager->device()->wattKg().average(), 'f', 1) + "\n";
    textMessage +=
        QStringLiteral("Max Watt/Kg: ") + QString::number(bluetoothManager->device()->wattKg().max(), 'f', 1) + "\n";
    textMessage += QStringLiteral("Average Heart Rate: ") +
                   QString::number(bluetoothManager->device()->currentHeart().average(), 'f', 0) + QStringLiteral("\n");
    textMessage += QStringLiteral("Max Heart Rate: ") +
                   QString::number(bluetoothManager->device()->currentHeart().max(), 'f', 0) + QStringLiteral("\n");

    for(int i=0; i<bluetoothManager->device()->maxHeartZone(); i++) {
        textMessage += QStringLiteral("Heart Rate Z") + QString::number(i + 1) + QStringLiteral(": ") +
                    QTime(0, 0, 0, 0).addSecs(bluetoothManager->device()->secondsForHeartZone(i)).toString("H:mm:ss") + QStringLiteral("\n");        
    }

    textMessage += QStringLiteral("Total Output: ") +
                   QString::number(bluetoothManager->device()->jouls().max() / 1000.0, 'f', 0) + QStringLiteral("\n");
    textMessage +=
        QStringLiteral("Elapsed Time: ") + bluetoothManager->device()->elapsedTime().toString() + QStringLiteral("\n");
    textMessage +=
        QStringLiteral("Moving Time: ") + bluetoothManager->device()->movingTime().toString() + QStringLiteral("\n");
    textMessage += QStringLiteral("Weight Loss (") + weightLossUnit + "): " + QString::number(WeightLoss, 'f', 2) +
                   QStringLiteral("\n");
    double vo2max = metric::calculateVO2Max(&Session);
    if(vo2max)
        textMessage += QStringLiteral("Estimated VO2Max: ") + QString::number(vo2max, 'f', 0) +
                   QStringLiteral("\n");
    if(bluetoothManager->device()->deviceType() == bluetoothdevice::BLUETOOTH_TYPE::TREADMILL) {
        textMessage += QStringLiteral("Running Stress Score: ") + QString::number(((treadmill*)bluetoothManager->device())->runningStressScore(), 'f', 0) +
                       QStringLiteral("\n");
    }
    double peak = metric::powerPeak(&Session, 5);
    double weightKg = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();
    textMessage += QStringLiteral("5 Seconds Power: ") + QString::number(peak, 'f', 0) +
                   QStringLiteral("W ") + QString::number(peak/weightKg, 'f', 1) + QStringLiteral("W/Kg\n");
    peak = metric::powerPeak(&Session, 60);
    textMessage += QStringLiteral("1 Minute Power: ") + QString::number(peak, 'f', 0) +
                   QStringLiteral("W ") + QString::number(peak/weightKg, 'f', 1) + QStringLiteral("W/Kg\n");
    peak = metric::powerPeak(&Session, 5 * 60);
    textMessage += QStringLiteral("5 Minutes Power: ") + QString::number(peak, 'f', 0) +
                   QStringLiteral("W ") + QString::number(peak/weightKg, 'f', 1) + QStringLiteral("W/Kg\n");    

    // FTP
    double ftpSetting = settings.value(QZSettings::ftp, QZSettings::default_ftp).toDouble();
    peak = (metric::powerPeak(&Session, 20 * 60) * 0.95) * 0.95;
    textMessage += QStringLiteral("Estimated FTP: ") + QString::number(peak, 'f', 0) +
                   QStringLiteral("W ");
    if(peak > ftpSetting) {
        textMessage += QStringLiteral(" FTP IMPROVED +") + QString::number(peak - ftpSetting, 'f', 0) +
                       QStringLiteral("W!");
    }
    textMessage += QStringLiteral("\n");

    if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
        textMessage += QStringLiteral("Average Cadence: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentCadence().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Cadence: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentCadence().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->currentResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Peloton Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Peloton Resistance: ") +
                       QString::number(((bike *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::ROWING) {
        textMessage += QStringLiteral("Average Cadence: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentCadence().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Cadence: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentCadence().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->currentResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Peloton Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->pelotonResistance().average(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Peloton Resistance: ") +
                       QString::number(((rower *)bluetoothManager->device())->pelotonResistance().max(), 'f', 0) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Average Pace: ") +
                       ((rower *)bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Pace: ") +
                       ((rower *)bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")) +
                       QStringLiteral("\n");
        textMessage +=
            QStringLiteral("Average Strokes Length: ") +
            QString::number(((rower *)bluetoothManager->device())->currentStrokesLength().average(), 'f', 1) + "\n";
    } else if (bluetoothManager->device()->deviceType() == bluetoothdevice::TREADMILL || bluetoothManager->device()->deviceType() == bluetoothdevice::ELLIPTICAL) {
        textMessage += QStringLiteral("Average Pace: ") +
                       (bluetoothManager->device())->averagePace().toString(QStringLiteral("m:ss")) +
                       QStringLiteral("\n");
        textMessage += QStringLiteral("Max Pace: ") +
                       (bluetoothManager->device())->maxPace().toString(QStringLiteral("m:ss")) +
                       QStringLiteral("\n");
        // for stryd and similars
        if (bluetoothManager->device()->currentCadence().average() > 0) {
            textMessage += QStringLiteral("Average Cadence: ") +
                           QString::number((bluetoothManager->device())->currentCadence().average(), 'f', 0) +
                           QStringLiteral("\n");
            textMessage += QStringLiteral("Max Cadence: ") +
                           QString::number((bluetoothManager->device())->currentCadence().max(), 'f', 0) +
                           QStringLiteral("\n");
        }
    }
    textMessage += QStringLiteral("\n\nQZ version: ") + QApplication::applicationVersion();
#ifdef Q_OS_ANDROID
    textMessage += " - Android";
#endif
#ifdef Q_OS_IOS
    textMessage += " - iOS";
#endif
    if (bluetoothManager) {
        textMessage += QStringLiteral("\nDevice: ") + bluetoothManager->device()->bluetoothDevice.name();

        if (bluetoothManager->heartRateDevice()) {
            textMessage +=
                QStringLiteral("\nHR Device: ") + bluetoothManager->heartRateDevice()->bluetoothDevice.name();
        }
    }

#ifdef SMTP_SERVER
    textMessage += QStringLiteral("\n\nSMTP server: ") + QString(STRINGIFY(SMTP_SERVER));
#endif

    text.setText(textMessage);
    message.addPart(&text);

    for (const QString &f : qAsConst(chartImagesFilenames)) {

        // Create a MimeInlineFile object for each image
        MimeInlineFile *image = new MimeInlineFile((new QFile(f)));

        // An unique content id must be setted
        image->setContentId(f);
        image->setContentType(QStringLiteral("image/jpg"));
        message.addPart(image);
    }

    if (!lastFitFileSaved.isEmpty()) {

        // Create a MimeInlineFile object for each image
        MimeInlineFile *fit = new MimeInlineFile((new QFile(lastFitFileSaved)));

        // An unique content id must be setted
        fit->setContentId(lastFitFileSaved);
        fit->setContentType(QStringLiteral("application/octet-stream"));
        message.addPart(fit);
    }

    if (!lastTrainProgramFileSaved.isEmpty()) {

        // Create a MimeInlineFile object for each image
        MimeInlineFile *xml = new MimeInlineFile((new QFile(lastTrainProgramFileSaved)));

        // An unique content id must be setted
        xml->setContentId(lastTrainProgramFileSaved);
        xml->setContentType(QStringLiteral("application/octet-stream"));
        message.addPart(xml);
        lastTrainProgramFileSaved = "";
    }

    QString filenameJPG = QStringLiteral("");
    if (pelotonHandler && pelotonHandler->current_image_downloaded &&
        !pelotonHandler->current_image_downloaded->downloadedData().isEmpty()) {

        QString path = getWritableAppDir();
        QString filename = path +
                           QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
                           QStringLiteral("_peloton_image.png");
        filenameJPG =
            path + QDateTime::currentDateTime().toString().replace(QStringLiteral(":"), QStringLiteral("_")) +
            QStringLiteral("_peloton_image.jpg");
        QFile file(filename);
        file.open(QIODevice::WriteOnly);
        file.write(pelotonHandler->current_image_downloaded->downloadedData());
        file.close();
        QImage image(filename);
        QImageWriter writer(filename, "png");
        writer.setFileName(filenameJPG);
        writer.setFormat("jpg");
        writer.setQuality(30);
        writer.write(image);
        QFile::remove(filename);        

        // Create a MimeInlineFile object for each image
        MimeInlineFile *pelotonImage = new MimeInlineFile((new QFile(filenameJPG)));

        // An unique content id must be setted
        pelotonImage->setContentId(filenameJPG);
        pelotonImage->setContentType(QStringLiteral("image/jpg"));
        message.addPart(pelotonImage);
    }

    /* THE SMTP SERVER DOESN'T LIKE THE ZIP FILE
    extern QString logfilename;
    if (settings.value(QZSettings::log_debug).toBool() && QFile::exists(getWritableAppDir() + logfilename)) {
        QString fileName = getWritableAppDir() + logfilename;
        QFile f(fileName);
        f.open(QIODevice::ReadOnly);
        QTextStream ts(&f);
        QByteArray b = f.readAll();
        f.close();
        QByteArray c = qCompress(b, 9);
        QFile fc(fileName.replace(".log", ".zip"));
        fc.open(QIODevice::WriteOnly);
        c.remove(0, 4);
        fc.write(c);
        fc.close();

        // Create a MimeInlineFile object for each image
        MimeInlineFile *log = new MimeInlineFile((new QFile(fileName)));

        // An unique content id must be setted
        log->setContentId(fileName);
        log->setContentType(QStringLiteral("application/octet-stream"));
        message.addPart(log);
    }*/

    bool r = false;
    uint8_t i = 0;
    
      qDebug() << "trying to send email #" << i;
      smtp.connectToHost();
      
      // Use the public login method with parameters instead of the protected one
      
         smtp.login(STRINGIFY(SMTP_USERNAME), STRINGIFY(SMTP_PASSWORD)); // Replace with actual credentials
      
      
     
          smtp.sendMail(message);
      
        
    smtp.quit();

    // delete image variable
    if(!filenameJPG.isEmpty())
        QFile::remove(filenameJPG);
}

#if defined(Q_OS_ANDROID)

QString homeform::getBluetoothName()
{
    QJniObject bluetoothAdapter = QJniObject::callStaticObjectMethod(
        "android/bluetooth/BluetoothAdapter",
        "getDefaultAdapter",
        "()Landroid/bluetooth/BluetoothAdapter;");
    
    if (bluetoothAdapter.isValid()) {
        QJniObject name = bluetoothAdapter.callObjectMethod(
            "getName",
            "()Ljava/lang/String;");
        
        if (name.isValid()) {
            return name.toString();
        }
    }
    
    return QString();
}

QString homeform::getAndroidDataAppDir() {
    static QString path = "";

    if (path.length()) {
        return path;
    }

    QJniObject activity = QJniObject::callStaticObjectMethod("org/qtproject/qt/android/QtNative", "activity", "()Landroid/app/Activity;");
    QJniObject filesArr = activity.callObjectMethod(
        "getExternalFilesDirs", "(Ljava/lang/String;)[Ljava/io/File;", nullptr);
    jobjectArray dataArray = filesArr.object<jobjectArray>();
    QString out;
    if (dataArray) {
        QJniEnvironment env;
        jsize dataSize = env->GetArrayLength(dataArray);
        if (dataSize) {
            QJniObject mediaPath;
            QJniObject file;
            for (int i = 0; i < dataSize; i++) {
                file = env->GetObjectArrayElement(dataArray, i);
                jboolean val = QJniObject::callStaticMethod<jboolean>(
                    "android/os/Environment", "isExternalStorageRemovable", "(Ljava/io/File;)Z", file.object());
                mediaPath = file.callObjectMethod("getAbsolutePath", "()Ljava/lang/String;");
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

quint64 homeform::cryptoKeySettingsProfiles() {
    QSettings settings;
    quint64 v = settings.value(QZSettings::cryptoKeySettingsProfiles, QZSettings::default_cryptoKeySettingsProfiles)
                    .toULongLong();
    if (!v) {
        QRandomGenerator r = QRandomGenerator();
        r.seed(QDateTime::currentMSecsSinceEpoch());
        v = r.generate64();
        settings.setValue(QZSettings::cryptoKeySettingsProfiles, v);
    }
    return v;
}

void homeform::saveSettings(const QUrl &filename) {
    Q_UNUSED(filename)
    QString path = getWritableAppDir();

    QDir().mkdir(path + QStringLiteral("settings/"));
    QSettings settings;
    QSettings settings2Save(path + QStringLiteral("settings/settings_") +
                                settings.value(QZSettings::profile_name).toString() + QStringLiteral("_") +
                                QDateTime::currentDateTime().toString("yyyyMMddhhmmss") + QStringLiteral(".qzs"),
                            QSettings::IniFormat);
    auto settigsAllKeys = settings.allKeys();
    for (const QString &s : qAsConst(settigsAllKeys)) {
        if (!s.contains(QZSettings::cryptoKeySettingsProfiles)) {
            if (!s.contains(QStringLiteral("password")) && !s.contains(QStringLiteral("token"))) {
                settings2Save.setValue(s, settings.value(s));
            } else {
                SimpleCrypt crypt;
                crypt.setKey(cryptoKeySettingsProfiles());
                settings2Save.setValue(s, crypt.encryptToString(settings.value(s).toString()));
            }
        }
    }
}

void homeform::loadSettings(const QUrl &filename) {

    QFile file(QQmlFile::urlToLocalFileOrQrc(filename));
    copyAndroidContentsURI(filename, "settings");

    qDebug() << "homeform::loadSettings" << file.fileName();

    QSettings settings;
    QSettings settings2Load(file.fileName(), QSettings::IniFormat);
    auto settings2LoadAllKeys = settings2Load.allKeys();
    for (const QString &s : qAsConst(settings2LoadAllKeys)) {
        if (!s.contains(QZSettings::cryptoKeySettingsProfiles)) {
            // peloton refresh token must not be changed because it has one refresh token for peloton user saved locally on the device
            if(!s.contains(QStringLiteral("peloton_refreshtoken"))) {
                if (!s.contains(QStringLiteral("password")) && !s.contains(QStringLiteral("token"))) {
                    settings.setValue(s, settings2Load.value(s));
                } else {
                    SimpleCrypt crypt;
                    crypt.setKey(cryptoKeySettingsProfiles());
                    settings.setValue(s, crypt.decryptToString(settings2Load.value(s).toString()));
                }
            }
        }
    }
    
    // Emit signal when settings are loaded as they might contain user profile changes
    if (homeform::singleton()) {
        emit homeform::singleton()->userProfileChanged();
    }
}

void homeform::deleteSettings(const QUrl &filename) { QFile(filename.toLocalFile()).remove(); }
void homeform::restoreSettings() { 
    QZSettings::restoreAll(); 
    // Emit signal when settings are restored as this might affect user profiles
    emit userProfileChanged();
}

QString homeform::getProfileDir() {
    QString path = getWritableAppDir() + "profiles";
    QDir().mkdir(path);
    return path;
}

void homeform::saveProfile(QString profilename) {
    qDebug() << "homeform::saveProfile";
    QString path = getProfileDir();

    QSettings settings;
    settings.setValue(QZSettings::profile_name, profilename);
    QSettings settings2Save(path + "/" + profilename + QStringLiteral(".qzs"), QSettings::IniFormat);
    auto settigsAllKeys = settings.allKeys();
    for (const QString &s : qAsConst(settigsAllKeys)) {
        if (!s.contains(QZSettings::cryptoKeySettingsProfiles)) {
            if (!s.contains(QStringLiteral("password")) && !s.contains(QStringLiteral("token"))) {
                settings2Save.setValue(s, settings.value(s));
            } else {
                SimpleCrypt crypt;
                crypt.setKey(cryptoKeySettingsProfiles());
                settings2Save.setValue(s, crypt.encryptToString(settings.value(s).toString()));
            }
        }
    }
}

void homeform::restart() {
    qApp->quit();
#if !defined(Q_OS_DARWIN) && !defined(Q_OS_IOS) && !defined(Q_OS_WINRT)
    QProcess::startDetached(qApp->arguments()[0], qApp->arguments());
#endif
}

double homeform::heartRateMax() {
    QSettings settings;
    double maxHeartRate = 220.0 - settings.value(QZSettings::age, QZSettings::default_age).toDouble();

    if (settings.value(QZSettings::heart_max_override_enable, QZSettings::default_heart_max_override_enable).toBool())
        maxHeartRate =
            settings.value(QZSettings::heart_max_override_value, QZSettings::default_heart_max_override_value)
                .toDouble();
    if (maxHeartRate == 0) {
        maxHeartRate = 190.0;
    }
    return maxHeartRate;
}

void homeform::clearFiles() {
    QString path = homeform::getWritableAppDir();
    QDir dir(path);
    QFileInfoList list = dir.entryInfoList(QDir::Files);
    foreach (QFileInfo f, list) {
        if (!f.suffix().toLower().compare("log") || !f.suffix().toLower().compare("jpg") ||
            !f.suffix().toLower().compare("fit") || !f.suffix().toLower().compare("png")) {
            QFile::remove(f.filePath());
        }
    }
}

int homeform::preview_workout_points() {
    if (previewTrainProgram) {
        QTime d = previewTrainProgram->duration();
        return (d.hour() * 3600) + (d.minute() * 60) + d.second();
    }
    return 0;
}

#if defined(Q_OS_WIN) || (defined(Q_OS_MAC) && !defined(Q_OS_IOS)) || (defined(Q_OS_ANDROID) && defined(LICENSE))
void homeform::licenseReply(QNetworkReply *reply) {
    QString r = reply->readAll();
    qDebug() << r;
    if (r.contains("OK")) {
        tLicense.stop();
    } else {
        licenseRequest();
    }
}

void homeform::licenseRequest() {
    QTimer::singleShot(30000, this, [this]() {
        QSettings settings;
        if (!mgr) {
            mgr = new QNetworkAccessManager(this);
            connect(mgr, &QNetworkAccessManager::finished, this, &homeform::licenseReply);
        }
        QUrl url(QStringLiteral("http://robertoviola.cloud:4010/?supporter=") +
                 settings.value(QZSettings::user_email, "").toString());
        QNetworkRequest request(url);
        mgr->get(request);
    });
}

void homeform::licenseTimeout() { setLicensePopupVisible(true); }
#endif

void homeform::changeTimestamp(QTime source, QTime actual) {
    QSettings settings;
    // only needed if a gpx is loaded and the video is visible, otherwise do nothing.
    if ((trainProgram) && (videoVisible() == true)) {
        QObject *rootObject = engine->rootObjects().constFirst();
        auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
        auto videoPlaybackHalfPlayer = qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
        double videoTimeStampSeconds = (double)videoPlaybackHalfPlayer->position() / 1000.0;
        // Check for time differences between Video and gpx Data
        if (videoTimeStampSeconds != 0.0) {
            double videoLengthSeconds = ((double)(videoPlaybackHalfPlayer->duration() / 1000.0));
            double trainProgramLengthSeconds = ((double)(trainProgram->TotalGPXSecs()));
            int recordingFactor = 1;

            // if Video is > 60 secs Shorter it will be a speed adjusted one
            if ((trainProgramLengthSeconds - videoLengthSeconds) >= 60.0) {
                double recfac = ((trainProgramLengthSeconds / videoLengthSeconds) + 0.5);
                recordingFactor = ((int)(recfac));
                qDebug() << "Video Recording Factor" << recordingFactor << trainProgramLengthSeconds
                         << videoLengthSeconds << (videoLengthSeconds * ((double)(recordingFactor)))
                         << videoTimeStampSeconds << (videoTimeStampSeconds * ((double)(recordingFactor)));
                videoLengthSeconds = (videoLengthSeconds * ((double)(recordingFactor)));
                videoTimeStampSeconds = (videoTimeStampSeconds * ((double)(recordingFactor)));
            }

            // check if there is a difference >= 1 second
            if ((fabs(videoLengthSeconds - trainProgramLengthSeconds)) >= 1.0) {
                // correct Video TimeStamp by difference
                videoTimeStampSeconds = (videoTimeStampSeconds - videoLengthSeconds + trainProgramLengthSeconds);
            }

            qDebug() << videoTimeStampSeconds;
            // Video was just displayed, set the start Position
            if (videoMustBeReset) {
                double videoStartPos =
                    ((double)(QTime(0, 0, 0).secsTo(source)) + videoLengthSeconds - trainProgramLengthSeconds);
                // if videoStartPos is negativ the Video is shorter then the GPX. Wait for the gpx to reach a point
                // where the Video can be played
                if (videoStartPos >= 0.0) {
                    videoTimeStampSeconds = (videoStartPos - videoLengthSeconds + trainProgramLengthSeconds);
                    videoStartPos = videoStartPos / ((double)(recordingFactor));
                    qDebug() << "SetVideoStartPosition" << (videoStartPos * 1000.0);
                    videoPlaybackHalfPlayer->setPosition(videoStartPos * 1000.0);
                    videoMustBeReset = false;
                }
            }
            // Video is started now, calculate and set the Rate
            if (!videoMustBeReset) {
                // calculate and set the new Video Rate
                double rate = trainProgram->TimeRateFromGPX(
                    ((double)QTime(0, 0, 0).msecsTo(source)) / 1000.0, videoTimeStampSeconds,
                    bluetoothManager->device()->currentSpeed().average5s(), recordingFactor);
                rate = rate / ((double)(recordingFactor));
                setVideoRate(rate);
            } else {
                qDebug() << "videoMustBeReset = True";
            }
        } else {
            qDebug() << "videoTimeStampSeconds = 0";
        }
    }

    if (!videoVisible()) {
        // set the maximum Speed that the player can reached based on the Video speed.
        // When Video is not displayed (or not displayed any longer) remove the Limit
        if (bluetoothManager->device()->deviceType() == bluetoothdevice::BIKE) {
            bike *dev = (bike *)bluetoothManager->device();
            dev->setSpeedLimit(0);
        }
        // Prepare for a possible Video play. Set the Start Position to 1 and a Rate so low that only a few frames
        // are played
        setVideoPosition(1);
        setVideoRate(0.01);
        videoMustBeReset = true;
    }
}

void homeform::videoSeekPosition(int ms) {
    QObject *rootObject = engine->rootObjects().constFirst();
    auto *videoPlaybackHalf = rootObject->findChild<QObject *>(QStringLiteral("videoplaybackhalf"));
    auto videoPlaybackHalfPlayer = qvariant_cast<QMediaPlayer *>(videoPlaybackHalf->property("mediaObject"));
    videoPlaybackHalfPlayer->setPosition(ms);
}

#ifdef Q_OS_ANDROID
extern "C" {
    JNIEXPORT void JNICALL
    Java_org_cagnulen_qdomyoszwift_ChannelService_nativeSetResistance(JNIEnv *env, jclass clazz, jint resistance) {
        qDebug() << "Native: ANT+ Setting resistance to:" << resistance;
        
        if (homeform::singleton()->bluetoothManager && homeform::singleton()->bluetoothManager->device()) {
            bluetoothdevice::BLUETOOTH_TYPE deviceType = homeform::singleton()->bluetoothManager->device()->deviceType();
            
            if (deviceType == bluetoothdevice::BIKE || 
                deviceType == bluetoothdevice::ROWING || 
                deviceType == bluetoothdevice::ELLIPTICAL) {
                
                homeform::singleton()->bluetoothManager->device()->changeResistance(resistance);
                qDebug() << "Applied ANT+ resistance change:" << resistance;
            } else {
                qDebug() << "Device type does not support resistance change";
            }
        } else {
            qDebug() << "No bluetooth device connected";
        }
    }

    JNIEXPORT void JNICALL
    Java_org_cagnulen_qdomyoszwift_ChannelService_nativeSetPower(JNIEnv *env, jclass clazz, jint power) {
        qDebug() << "Native: ANT+ Setting power to:" << power << "W";
        
        if (homeform::singleton()->bluetoothManager && homeform::singleton()->bluetoothManager->device()) {
            bluetoothdevice::BLUETOOTH_TYPE deviceType = homeform::singleton()->bluetoothManager->device()->deviceType();
            
            if (deviceType == bluetoothdevice::BIKE || 
                deviceType == bluetoothdevice::ROWING || 
                deviceType == bluetoothdevice::ELLIPTICAL ||
                deviceType == bluetoothdevice::TREADMILL) {
                
                homeform::singleton()->bluetoothManager->device()->changePower(power);
                qDebug() << "Applied ANT+ power change:" << power << "W";
            } else {
                qDebug() << "Device type does not support power change";
            }
        } else {
            qDebug() << "No bluetooth device connected";
        }
    }

    JNIEXPORT void JNICALL
    Java_org_cagnulen_qdomyoszwift_ChannelService_nativeSetInclination(JNIEnv *env, jclass clazz, jdouble inclination) {
        qDebug() << "Native: ANT+ Setting inclination to:" << inclination << "%";
        
        if (homeform::singleton()->bluetoothManager && homeform::singleton()->bluetoothManager->device()) {
            bluetoothdevice::BLUETOOTH_TYPE deviceType = homeform::singleton()->bluetoothManager->device()->deviceType();
            
            if (deviceType == bluetoothdevice::BIKE || 
                deviceType == bluetoothdevice::TREADMILL || 
                deviceType == bluetoothdevice::ELLIPTICAL) {
                
                homeform::singleton()->bluetoothManager->device()->changeInclination(inclination, inclination);
                qDebug() << "Applied ANT+ inclination change:" << inclination << "%";
            } else {
                qDebug() << "Device type does not support inclination change";
            }
        } else {
            qDebug() << "No bluetooth device connected";
        }
    }
}
#endif
