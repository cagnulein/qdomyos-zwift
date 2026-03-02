#ifndef MOXY5SENSOR_H
#define MOXY5SENSOR_H

#include <QBluetoothDeviceDiscoveryAgent>
#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qbytearray.h>

#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qmutex.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include <QObject>
#include <QTime>
#include <QDateTime>

#include "treadmill.h"

// UUID costanti per Moxy 5
#define MOXY5_SERVICE_UUID "6404D801-4CB9-11E8-B566-0800200C9A66"
#define MOXY5_CHARACTERISTIC_UUID "6404D804-4CB9-11E8-B566-0800200C9A66"

class moxy5sensor : public treadmill {
    Q_OBJECT
    
    // Properties exposed for data access
    Q_PROPERTY(double currentSaturatedHemoglobin READ getCurrentSaturatedHemoglobin NOTIFY currentSaturatedHemoglobinChanged)
    Q_PROPERTY(double previousSaturatedHemoglobin READ getPreviousSaturatedHemoglobin NOTIFY previousSaturatedHemoglobinChanged)
    Q_PROPERTY(double totalHemoglobinConcentration READ getTotalHemoglobinConcentration NOTIFY totalHemoglobinConcentrationChanged)
    
  public:
    moxy5sensor();
    ~moxy5sensor();
    bool connected() override;
    
    // Getters for properties
    double getCurrentSaturatedHemoglobin() const { return m_currentSaturatedHemoglobin; }
    double getPreviousSaturatedHemoglobin() const { return m_previousSaturatedHemoglobin; }
    double getTotalHemoglobinConcentration() const { return m_totalHemoglobinConcentration; }

  private:
    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattNotifyCharacteristic;
    
    // Variables for Moxy sensor data
    double m_currentSaturatedHemoglobin;
    double m_previousSaturatedHemoglobin;
    double m_totalHemoglobinConcentration;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    void heartRate(uint8_t heart) override; // Kept for compatibility with treadmill
    
    // Signals for property change notifications
    void currentSaturatedHemoglobinChanged(double value);
    void previousSaturatedHemoglobinChanged(double value);
    void totalHemoglobinConcentrationChanged(double value);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
};

#endif // MOXY5SENSOR_H
