#ifndef ELITESQUARECONTROLLER_H
#define ELITESQUARECONTROLLER_H

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

#include "devices/bluetoothdevice.h"
#include "zwift_play/abstractZapDevice.h"

// Button positions on Elite Square controller
enum EliteSquareButton {
    BUTTON_NONE = 0,
    // 1 and 2 are N/A
    BUTTON_UP = 3,
    BUTTON_LEFT = 4,
    BUTTON_DOWN = 5,
    BUTTON_RIGHT = 6,
    BUTTON_X = 7,
    BUTTON_SQUARE = 8,
    BUTTON_LEFT_CAMPAGNOLO = 9,
    BUTTON_LEFT_BRAKE = 10,
    BUTTON_LEFT_SHIFT_1 = 11,
    BUTTON_LEFT_SHIFT_2 = 12,
    BUTTON_Y = 13,
    BUTTON_A = 14,
    BUTTON_B = 15,
    BUTTON_Z = 16,
    BUTTON_CIRCLE = 17,
    BUTTON_TRIANGLE = 18,
    // 19 is N/A
    BUTTON_RIGHT_CAMPAGNOLO = 20,
    BUTTON_RIGHT_BRAKE = 21,
    BUTTON_RIGHT_SHIFT_1 = 22,
    BUTTON_RIGHT_SHIFT_2 = 23
};

// Button state
enum EliteSquareButtonState {
    RELEASED = 0,
    PRESSED = 1
};

class elitesquarecontroller : public bluetoothdevice {
    Q_OBJECT
  public:
    // Constants for Elite Square device
    static const QString DEVICE_NAME;
    static const QBluetoothUuid SERVICE_UUID;
    static const QBluetoothUuid CHARACTERISTIC_UUID;

    elitesquarecontroller(bluetoothdevice *parentDevice);
    bool connected() override;

    // Function to handle button states
    void processButtonEvent(int buttonId, EliteSquareButtonState state);

  private:
    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyCharacteristic gattNotifyCharacteristic;
    QLowEnergyService *gattService;

    bluetoothdevice *parentDevice = nullptr;

    bool connectionEstablished = false;
    QTimer *refresh;

    // Last known state of each button (0-23)
    QVector<int> buttonState;

    // Helper function to parse the button data
    void parseButtonData(const QString &data);

    // Function to handle button state changes
    void handleButtonStateChange(int buttonId, int newValue);

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();
    
    // Match AbstractZapDevice signals
    void plus();                    // Gear up/increase
    void minus();                   // Gear down/decrease
    void steeringLeft(bool active); // X button and Left Campagnolo
    void steeringRight(bool active); // Circle button and Right Campagnolo
    void buttonActivated(int buttonId, bool pressed); // General button signal

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
};

#endif // ELITESQUARECONTROLLER_H