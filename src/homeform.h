#ifndef HOMEFORM_H
#define HOMEFORM_H

#include <QQuickItem>
#include <QQmlApplicationEngine>
#include "bluetooth.h"
#include "sessionline.h"
#include "trainprogram.h"

class DataObject : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString name READ name NOTIFY nameChanged)
    Q_PROPERTY(QString icon READ icon NOTIFY iconChanged)
    Q_PROPERTY(QString value READ value WRITE setValue NOTIFY valueChanged)
    Q_PROPERTY(int valueFontSize READ valueFontSize WRITE setValueFontSize NOTIFY valueFontSizeChanged)
    Q_PROPERTY(int labelFontSize READ labelFontSize WRITE setLabelFontSize NOTIFY labelFontSizeChanged)
    Q_PROPERTY(bool writable READ writable NOTIFY writableChanged)
    Q_PROPERTY(bool visibleItem READ visibleItem NOTIFY visibleChanged)
    Q_PROPERTY(QString plusName READ plusName NOTIFY plusNameChanged)
    Q_PROPERTY(QString minusName READ minusName NOTIFY minusNameChanged)

public:
    DataObject(QString name, QString icon, QString value, bool writable, QString id, int valueFontSize, int labelFontSize);
    void setValue(QString value);
    void setValueFontSize(int value);
    void setLabelFontSize(int value);
    void setVisible(bool visible);
    QString name() {return m_name;}
    QString icon() {return m_icon;}
    QString value() {return m_value;}
    int valueFontSize() {return m_valueFontSize;}
    int labelFontSize() {return m_labelFontSize;}
    bool writable() {return m_writable;}
    bool visibleItem() {return m_visible;}
    QString plusName() {return m_id + "_plus";}
    QString minusName() {return m_id + "_minus";}

    QString m_id;
    QString m_name;
    QString m_icon;
    QString m_value;
    int m_valueFontSize;
    int m_labelFontSize;
    bool m_writable;
    bool m_visible = true;

signals:
    void valueChanged(QString value);
    void valueFontSizeChanged(int value);
    void labelFontSizeChanged(int value);
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
    ~homeform();
    QString info() {return m_info;}
    QString signal();

private:
    QList<QObject *> dataList;
    QList<SessionLine> Session;
    bluetooth* bluetoothManager = 0;
    QQmlApplicationEngine* engine;
    trainprogram* trainProgram = 0;

    QString m_info = "Connecting...";

    DataObject* speed;
    DataObject* inclination;
    DataObject* cadence;
    DataObject* elevation;
    DataObject* calories;
    DataObject* odometer;
    DataObject* pace;
    DataObject* resistance;
    DataObject* watt;
    DataObject* heart;
    DataObject* fan;
    DataObject* jouls;
    DataObject* elapsed;
    DataObject* peloton_resistance;

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
    void trainprogram_open_clicked(QUrl fileName);
    void gpx_open_clicked(QUrl fileName);
    void gpx_save_clicked();
    void trainProgramSignals();

signals:
 void changeOfdevice();
 void changeOfzwift();
 void signalChanged(QString value);
 void infoChanged(QString value);
};

#endif // HOMEFORM_H
