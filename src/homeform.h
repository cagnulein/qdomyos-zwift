#ifndef HOMEFORM_H
#define HOMEFORM_H

#include <QQuickItem>
#include <QQmlApplicationEngine>
#include "bluetooth.h"
#include "sessionline.h"

class DataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool writable READ writable NOTIFY writableChanged)
    Q_PROPERTY(bool visibleItem READ visibleItem NOTIFY visibleChanged)
    Q_PROPERTY(QString plusName READ plusName NOTIFY plusNameChanged)
    Q_PROPERTY(QString minusName READ minusName NOTIFY minusNameChanged)

public:
    DataObject(QString name, QString icon, QString value, bool writable, QString id);
    void setValue(QString value);
    void setVisible(bool visible);
    QString name() {return m_name;}
    QString icon() {return m_icon;}
    QString value() {return m_value;}
    bool writable() {return m_writable;}
    bool visibleItem() {return m_visible;}
    QString plusName() {return m_id + "_plus";}
    QString minusName() {return m_id + "_minus";}

    QString m_id;
    QString m_name;
    QString m_icon;
    QString m_value;
    bool m_writable;
    bool m_visible = true;

signals:
    void valueChanged(QString value);
    void nameChanged(QString value);
    void iconChanged(QString value);
    void writableChanged(bool value);
    void visibleChanged(bool value);
    void plusNameChanged(QString value);
    void minusNameChanged(QString value);
};

class homeform: public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool device READ getDevice NOTIFY changeOfdevice)
    Q_PROPERTY( bool zwift READ getZwift NOTIFY changeOfzwift)
    Q_PROPERTY(QString info READ info NOTIFY infoChanged)
    Q_PROPERTY(QString signal READ signal NOTIFY signalChanged)

public:
    homeform(QQmlApplicationEngine* engine, bluetooth* bl);
    QString info() {return m_info;}
    QString signal();

private:
    QList<QObject *> dataList;
    QList<SessionLine> Session;
    bluetooth* bluetoothManager;
    QQmlApplicationEngine* engine;

    QString m_info = "Connecting...";

    DataObject* speed = new DataObject("Speed (km/h)", "icons/icons/speed.png", "0.0", true, "speed");
    DataObject* inclination = new DataObject("Inclination (%)", "icons/icons/inclination.png", "0.0", true, "inclination");
    DataObject* cadence = new DataObject("Cadence (bpm)", "icons/icons/cadence.png", "0", false, "cadence");
    DataObject* elevation = new DataObject("Elev. Gain (m)", "icons/icons/elevationgain.png", "0", false, "elevation");
    DataObject* calories = new DataObject("Calories (KCal)", "icons/icons/kcal.png", "0", false, "calories");
    DataObject* odometer = new DataObject("Odometer (km)", "icons/icons/odometer.png", "0.0", false, "odometer");
    DataObject* pace = new DataObject("Pace (m/km)", "icons/icons/pace.png", "0:00", false, "pace");
    DataObject* resistance = new DataObject("Resistance (%)", "icons/icons/resistance.png", "0", true, "resistance");
    DataObject* watt = new DataObject("Watt", "icons/icons/watt.png", "0", false, "watt");
    DataObject* heart = new DataObject("Heart (bpm)", "icons/icons/heart_red.png", "0", false, "heart");
    DataObject* fan = new DataObject("Fan Speed", "icons/icons/fan.png", "0", true, "fan");

    QTimer* timer;

    void update();
    bool getDevice();
    bool getZwift();

private slots:
    void Start();
    void Stop();
    void Minus(QString);
    void Plus(QString);
    void deviceFound(QString name);
    void deviceConnected();

signals:
 void changeOfdevice();
 void changeOfzwift();
 void signalChanged(QString value);
 void infoChanged(QString value);
};

#endif // HOMEFORM_H
