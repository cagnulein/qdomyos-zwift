#ifndef HOMEFORM_H
#define HOMEFORM_H

#include <QQuickItem>
#include <QQmlApplicationEngine>
#include "bluetooth.h"
#include "sessionline.h"

class DataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name)
    Q_PROPERTY(QString icon READ icon)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(bool writable READ writable)

public:
    DataObject(QString name, QString icon, QString value, bool writable);
    void setValue(QString value);
    QString name() {return m_name;}
    QString icon() {return m_icon;}
    QString value() {return m_value;}
    bool writable() {return m_writable;}

    QString m_name;
    QString m_icon;
    QString m_value;
    bool m_writable;

signals:
    void valueChanged(QString value);
};

class homeform: public QObject
{
    Q_OBJECT
    Q_PROPERTY( bool device READ getDevice NOTIFY changeOfdevice)
    Q_PROPERTY( bool zwift READ getZwift NOTIFY changeOfzwift)

public:
    homeform(QQmlApplicationEngine* engine, bluetooth* bl);

private:
    QList<QObject *> dataList;
    QList<SessionLine> Session;
    bluetooth* Bluetooth;

    DataObject* speed = new DataObject("Speed (km/h)", "icons/icons/speed.png", "0.0", true);
    DataObject* inclination = new DataObject("Inclination (%)", "icons/icons/inclination.png", "0.0", true);
    DataObject* cadence = new DataObject("Cadence (bpm)", "icons/icons/cadence.png", "0", false);
    DataObject* elevation = new DataObject("Elev. Gain (m)", "icons/icons/elevationgain.png", "0", false);
    DataObject* calories = new DataObject("Calories (KCal)", "icons/icons/kcal.png", "0", false);
    DataObject* odometer = new DataObject("Odometer (km)", "icons/icons/odometer.png", "0.0", false);
    DataObject* pace = new DataObject("Pace (m/km)", "icons/icons/pace.png", "0:00", false);
    DataObject* resistance = new DataObject("Resistance (%)", "icons/icons/resistance.png", "0", true);
    DataObject* watt = new DataObject("Watt", "icons/icons/watt.png", "0", false);
    DataObject* heart = new DataObject("Heart (bpm)", "icons/icons/heart_red.png", "0", false);
    DataObject* fan = new DataObject("Fan Speed", "icons/icons/fan.png", "0", true);

    QTimer* timer;

    void update();
    bool getDevice();
    bool getZwift();

signals:
 void changeOfdevice();
 void changeOfzwift();
};

#endif // HOMEFORM_H
