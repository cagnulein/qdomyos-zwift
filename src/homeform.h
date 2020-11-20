#ifndef HOMEFORM_H
#define HOMEFORM_H

#include <QQuickItem>
#include <QQmlApplicationEngine>
#include "bluetooth.h"

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
    Q_PROPERTY( QString speed READ getSpeed NOTIFY changeOfspeed)
    Q_PROPERTY( bool device READ getDevice NOTIFY changeOfdevice)
    Q_PROPERTY( bool zwift READ getZwift NOTIFY changeOfzwift)

public:
    homeform(QQmlApplicationEngine* engine, bluetooth* bl);

private:
    QList<QObject *> dataList;
    bluetooth* Bluetooth;
    QString getSpeed();
    bool getDevice();
    bool getZwift();

signals:
 void changeOfspeed(QString i);
 void changeOfdevice(bool i);
 void changeOfzwift(bool i);
};

#endif // HOMEFORM_H
