#include "homeform.h"
#include <QQmlContext>

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
        new DataObject("Speed (km/h)", "icons/icons/speed.png", "0.0", true),
        new DataObject("Inclination (%)", "icons/icons/inclination.png", "0.0", true),
        new DataObject("Cadence (bpm)", "icons/icons/cadence.png", "0", false),
        new DataObject("Elev. Gain (m)", "icons/icons/elevationgain.png", "0", false),
        new DataObject("Calories (KCal)", "icons/icons/kcal.png", "0", false),
        new DataObject("Odometer (km)", "icons/icons/odometer.png", "0.0", false),
        new DataObject("Pace (m/km)", "icons/icons/pace.png", "0:00", false),
        new DataObject("Resistance (%)", "icons/icons/resistance.png", "0", true),
        new DataObject("Watt", "icons/icons/watt.png", "0", false),
        new DataObject("Heart (bpm)", "icons/icons/heart_red.png", "0", false),
        new DataObject("Fan Speed", "icons/icons/fan.png", "0", true),
    };

    engine->rootContext()->setContextProperty("appModel", QVariant::fromValue(dataList));

    ((DataObject*)dataList.at(0))->setValue("10");
}

QString homeform::getSpeed()
{
    return "10.0";
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
