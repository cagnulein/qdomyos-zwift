#ifndef CORESENSOR_H
#define CORESENSOR_H

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

#include "bluetoothdevice.h"

// UUID costanti per CORE Sensor secondo le specifiche
#define CORE_SERVICE_UUID "00002100-5B1E-4347-B07C-97B514DAE121"
#define CORE_TEMPERATURE_CHAR_UUID "00002101-5B1E-4347-B07C-97B514DAE121"
#define CORE_CONTROL_POINT_UUID "00002102-5B1E-4347-B07C-97B514DAE121"

// Flag bit definitions for Core Body Temperature characteristic
namespace CoreSensorFlags {
const uint8_t SKIN_TEMPERATURE_PRESENT = 0x01;
const uint8_t CORE_RESERVED_PRESENT = 0x02;
const uint8_t QUALITY_STATE_PRESENT = 0x04;
const uint8_t TEMPERATURE_UNIT_FAHRENHEIT = 0x08;
const uint8_t HEART_RATE_PRESENT = 0x10;
const uint8_t HEAT_STRAIN_INDEX_PRESENT = 0x20;
}

// Data quality values
namespace CoreSensorQuality {
const uint8_t INVALID = 0;
const uint8_t POOR = 1;
const uint8_t FAIR = 2;
const uint8_t GOOD = 3;
const uint8_t EXCELLENT = 4;
const uint8_t NOT_AVAILABLE = 7;
}

// Heart rate state values
namespace CoreSensorHRState {
const uint8_t NOT_SUPPORTED = 0;
const uint8_t SUPPORTED_NOT_RECEIVING = 1;
const uint8_t SUPPORTED_RECEIVING = 2;
const uint8_t NOT_AVAILABLE = 3;
}

// Control Point OpCodes
namespace CoreControlPoint {
// ANT+ Related OpCodes
const uint8_t CLEAR_ANT_PAIRED_HRM_LIST = 0x01;
const uint8_t ADD_ANT_HRM = 0x02;
const uint8_t REMOVE_ANT_HRM = 0x03;
const uint8_t GET_TOTAL_ANT_PAIRED_HRMS = 0x04;
const uint8_t GET_ANT_HRM_ID_AT_INDEX = 0x05;
const uint8_t SCAN_ANT_HRMS = 0x0A;
const uint8_t GET_TOTAL_SCANNED_ANT_HRMS = 0x0B;
const uint8_t GET_SCANNED_ANT_HRM_ID = 0x0C;

// BLE Related OpCodes
const uint8_t ADD_BLE_HRM = 0x06;
const uint8_t REMOVE_BLE_HRM = 0x07;
const uint8_t GET_TOTAL_BLE_PAIRED_HRMS = 0x08;
const uint8_t GET_BLE_HRM_NAME_STATE = 0x09;
const uint8_t SCAN_BLE_HRMS = 0x0D;
const uint8_t GET_TOTAL_SCANNED_BLE_HRMS = 0x0E;
const uint8_t GET_SCANNED_BLE_HRM_NAME = 0x0F;
const uint8_t GET_SCANNED_BLE_HRM_MAC = 0x10;
const uint8_t CLEAR_BLE_PAIRED_HRM_LIST = 0x11;
const uint8_t GET_BLE_HRM_MAC_STATE = 0x12;

// External heart rate input
const uint8_t EXTERNAL_HEART_RATE = 0x13;

// Response code (sent by sensor)
const uint8_t RESPONSE_CODE = 0x80;

// Scan parameters
const uint8_t START_SCAN = 0xFF;
const uint8_t START_PROXIMITY_PAIRING = 0xFE;

// Result codes
const uint8_t SUCCESS = 0x01;
const uint8_t OP_CODE_NOT_SUPPORTED = 0x02;
const uint8_t INVALID_PARAMETER = 0x03;
const uint8_t OPERATION_FAILED = 0x04;
}

// Struttura per la qualità e lo stato
struct QualityAndState {
    enum DataQuality {
        INVALID = 0,
        POOR = 1,
        FAIR = 2,
        GOOD = 3,
        EXCELLENT = 4,
        NOT_AVAILABLE = 7
    };

    enum HeartRateState {
        NOT_SUPPORTED = 0,
        SUPPORTED_NOT_RECEIVING = 1,
        SUPPORTED_RECEIVING = 2,
        NOT_AVAILABLE_STATE = 3
    };

    DataQuality quality;
    HeartRateState hrState;
};

class coresensor : public bluetoothdevice {
    Q_OBJECT

  public:
    coresensor();
    ~coresensor();
    bool connected() override;

    // Override from bluetoothdevice
    metric currentHeart() override;

  private:
    QLowEnergyService *m_coreService = nullptr;
    QLowEnergyCharacteristic m_coreTemperatureCharacteristic;
    QLowEnergyCharacteristic m_coreControlPointCharacteristic;

    // Other data from Core sensor
    uint8_t m_sensorHeartRate;     // Heart rate in BPM
    int m_dataQuality;             // Quality level (0=Invalid, 1=Poor, 2=Fair, 3=Good, 4=Excellent)
    int m_heartRateState;          // Heart rate state
    bool m_isCelsius;              // Temperature unit (true=°C, false=°F)

    // Control point state tracking
    bool m_operationInProgress = false;

  signals:
    void disconnected();
    void debug(QString string);
    void packetReceived();

    // Signals for property change notifications
    void coreBodyTemperatureChanged(double value);
    void skinTemperatureChanged(double value);
    void coreReservedChanged(double value);
    void heartRateChanged(double value);
    void heatStrainIndexChanged(double value);
    void dataQualityChanged(int value);
    void heartRateStateChanged(int value);
    void isCelsiusChanged(bool value);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void disconnectBluetooth();

    // Control point operations
    bool setExternalHeartRate(uint8_t bpm);
    bool disableExternalHeartRate();

    // Heart rate monitor operations - these depend on implementation
    bool scanForBLEHeartRateMonitors();
    bool scanForANTHeartRateMonitors();
    int getNumberOfPairedHeartRateMonitors();

  private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void serviceStateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);
    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void handleError(QLowEnergyController::Error err);
    void handleServiceError(QLowEnergyService::ServiceError err);

    // Handle control point responses
    void handleControlPointResponse(const QByteArray &value);
};

#endif // CORESENSOR_H
