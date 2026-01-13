#ifndef FLYWHEEL_H
#define FLYWHEEL_H

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

#include <QDateTime>
#include <QObject>
#include <QString>

#include "devices/bike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class flywheelbike : public bike {
    Q_OBJECT
  public:
    flywheelbike(bool noWriteResistance, bool noHeartService);
    bool connected() override;

  private:
    typedef enum DecoderRXState { WFSYNC_1 = 0, WFLENGTH, WFID, DATA, CHECKSUM, EOF_1 } DecoderRXState;

    typedef enum DecoderErrorState {
        MSG_NO_ERROR = 0,
        MSG_DATA_OK,
        MSG_COMPLETE,
        MSG_WFSIZE,
        MSG_BAD_CHECKSUM,
        MSG_BAD_EOF,
        UNKNOWN_STATE,
        MSG_UNKNOWN_ID
    } DecoderErrorState;

#pragma pack(push, 1)
    typedef struct BikeDataframe {
        uint8_t buffer[256];
        uint8_t crc;
        uint8_t len;
        uint8_t message_id;
    } BikeDataframe;

    typedef struct ICGLiveStreamData {
        uint16_t power;
        uint16_t ftp_percent;
        uint8_t training_zone;
        uint8_t heart_rate;
        uint8_t heart_rate_percent_of_max;
        uint8_t power_to_heart_rate_ratio;
        uint8_t power_to_weight_ratio;
        uint8_t cadence;
        uint16_t speed;
        uint8_t brake_level;
        uint8_t current_lap;
        uint32_t current_lap_time;
        uint16_t current_lap_distance;
        uint8_t total_laps;
        uint32_t workout_time;
        uint16_t distance;
        uint16_t calories;
    } ICGLiveStreamData;
#pragma pack(pop)

    typedef enum ICGMessageType {
        ID_NULL = 0,
        GET_ALL_USER_DATA,
        SET_ALL_USER_DATA,
        GENERAL_STREAM_DATA,
        GET_PHONE_NAME,
        SET_PHONE_NAME,
        GET_WLAN_SSID,
        SET_WLAN_SSID,
        GET_WLAN_PW,
        SET_WLAN_PW,
        GET_IP_ADDRESS,
        SET_IP_ADDRESS,
        SEND_ICG_LIVE_STREAM_DATA,
        SEND_ICG_AGGREGATED_STREAM_DATA,
        GET_FTP_TEST_RESULT,
        SET_FTP_TEST_RESULT,
        REQUEST_DISCONNECT,
        ENTER_PROGRAM_MODE,
        FLASH_PAGE_DATA,
        FLASH_PAGE_INVALID,
        FLASH_PAGE_REQ,
        FLASH_PAGE_RECV,
        CALL_APPLICATION,
        GET_PAGE_COUNT,
        SET_PAGE_COUNT,
        UPD_GET_CHECKSUM,
        UPD_SET_CHECKSUM,
        SEND_ICG_POWER_TEST_STREAM_DATA,
        REQUEST_WIFI_RESTART,
        BRAKE_CALIBRATION_RESET,
        BRAKE_CAL_MIN,
        BRAKE_CAL_MAX,
        BRAKE_CAL_DATA,
        FIRMWARE_VERSIONS,
        SET_BIKE_ID,
        GET_BIKE_ID,
        LAST_VALUE
    } ICGMessageType;

    int dataPacketLength;
    int dataPacketIndex;
    DecoderErrorState errorState = MSG_NO_ERROR;
    DecoderRXState rxState = WFSYNC_1;
    BikeDataframe bikeData;

    void flushDataframe(BikeDataframe *dataFrame);
    void decodeReceivedData(QByteArray buffer);

    double GetDistanceFromPacket(const QByteArray &packet);
    QTime GetElapsedFromPacket(QByteArray packet);
    void btinit();
    void writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log = false,
                             bool wait_for_response = false);
    void startDiscover();
    void sendPoll();
    uint16_t watts() override;
    void updateStats();

    QTimer *refresh;

    QLowEnergyService *gattCommunicationChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;
    QLowEnergyCharacteristic gattNotify1Characteristic;

    // uint8_t sec1Update = 0;
    QByteArray lastPacket;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;
    uint16_t m_watts = 0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;
    bool life_fitness_ic5 = false;

#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  Q_SIGNALS:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);

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

#endif // FLYWHEEL_H
