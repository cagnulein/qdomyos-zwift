#ifndef WAHOOKICKRSNAPBIKE_H
#define WAHOOKICKRSNAPBIKE_H

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
#include "virtualdevices/virtualbike.h"

#ifdef Q_OS_IOS
#include "ios/lockscreen.h"
#endif

class wahookickrsnapbike : public bike {
    Q_OBJECT
  public:
    wahookickrsnapbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                       double bikeResistanceGain);
    resistance_t pelotonToBikeResistance(int pelotonResistance) override;
    bool connected() override;
    resistance_t maxResistance() override { return 100; }
    bool inclinationAvailableByHardware() override;
    double maxGears() override;
    double minGears() override;

    enum OperationCode : uint8_t {
        _unlock = 32,
        _setResistanceMode = 64,
        _setStandardMode = 65,
        _setErgMode = 66,
        _setSimMode = 67,
        _setSimCRR = 68,
        _setSimWindResistance = 69,
        _setSimGrade = 70,
        _setSimWindSpeed = 71,
        _setWheelCircumference = 72,
    };

  private:    
    QByteArray unlockCommand();
    QByteArray setResistanceMode(double resistance);
    QByteArray setStandardMode(uint8_t level);
    QByteArray setErgMode(uint16_t watts);
    QByteArray setSimMode(double weight, double rollingResistanceCoefficient, double windResistanceCoefficient);
    QByteArray setSimCRR(double rollingResistanceCoefficient);
    QByteArray setSimWindResistance(double windResistanceCoefficient);
    QByteArray setSimGrade(double grade);
    QByteArray setSimWindSpeed(double metersPerSecond);
    QByteArray setWheelCircumference(double millimeters);

    bool writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log = false,
                             bool wait_for_response = false);
    uint16_t wattsFromResistance(double resistance);
    metric ResistanceFromFTMSAccessory;
    void startDiscover();
    uint16_t watts() override;

    QTimer *refresh;
    virtualbike *virtualBike = nullptr;

    QList<QLowEnergyService *> gattCommunicationChannelService;
    QLowEnergyService *gattPowerChannelService = nullptr;
    QLowEnergyCharacteristic gattWriteCharacteristic;

    uint8_t sec1Update = 0;
    QByteArray lastPacket;
    double lastGearValue = -1;
    double lastGrade = 0;
    QDateTime lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    QDateTime lastGoodCadence = QDateTime::currentDateTime();
    uint8_t firstStateChanged = 0;

    int8_t bikeResistanceOffset = 4;
    double bikeResistanceGain = 1.0;

    bool initDone = false;
    bool initRequest = false;

    bool noWriteResistance = false;
    bool noHeartService = false;

    uint16_t oldLastCrankEventTime = 0;
    uint16_t oldCrankRevs = 0;

    bool WAHOO_KICKR = false;
    bool KICKR_BIKE = false;
    
    bool lastCommandErgMode = false;

    volatile int notificationSubscribed = 0;

    resistance_t lastForcedResistance = -1;    

    double gearsToWheelDiameter(double gear);

    class GearTable {
      public:

        int maxGears = 12;

        struct GearInfo {
            int gear;
            int crankset;
            int rearCog;
        };

        void loadGearSettings() {
            QSettings settings;

            QString gearConfig = settings.value("gear_configuration").toString();
            if (gearConfig.isEmpty()) {

                gearConfig = "1|38|44|true\n2|38|38|true\n3|38|32|true\n4|38|28|true\n"
                             "5|38|24|true\n6|38|21|true\n7|38|19|true\n8|38|17|true\n"
                             "9|38|15|true\n10|38|13|true\n11|38|11|true\n12|38|10|true";
            }

            gears.clear();
            maxGears = 0;

                   // Parsa la configurazione
            QStringList rows = gearConfig.split('\n');
            for (const QString& row : rows) {
                QStringList parts = row.split('|');
                if (parts.size() >= 4 && (parts[3] == "true")) {
                    GearInfo config;
                    config.gear = parts[0].toInt();
                    config.crankset = parts[1].toInt();
                    config.rearCog = parts[2].toInt();

                    gears.push_back(config);
                    maxGears = qMax(maxGears, config.gear);
                }
            }
        }

        void addGear(int gear, int crankset, int rearCog) {
            gears.push_back({gear, crankset, rearCog});
        }

        void removeGear(int gear) {
            gears.erase(std::remove_if(gears.begin(), gears.end(),
                                       [gear](const GearInfo& info) { return info.gear == gear; }),
                        gears.end());
        }

        void printTable() const {
            qDebug() << "| Gear | Crankset | Rear Cog |\n";
            qDebug() << "|------|----------|----------|\n";
            for (const auto& gear : gears) {
                qDebug()  << "| " << gear.gear << " | " << gear.crankset
                          << " | " << gear.rearCog << " |\n";
            }
        }

        GearInfo getGear(int gearNumber) const {
            auto it = std::find_if(gears.begin(), gears.end(),
                                   [gearNumber](const GearInfo& info) { return info.gear == gearNumber; });

            if (it != gears.end()) {
                return *it;
            }
            return GearInfo();
        }

        GearTable() {
            loadGearSettings();
        }

      private:
        std::vector<GearInfo> gears;
    };    


#ifdef Q_OS_IOS
    lockscreen *h = 0;
#endif

  signals:
    void disconnected();
    void debug(QString string);

  public slots:
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    void resistanceFromFTMSAccessory(resistance_t res) override;

  private slots:

    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue);
    void stateChanged(QLowEnergyService::ServiceState state);
    void controllerStateChanged(QLowEnergyController::ControllerState state);

    void serviceDiscovered(const QBluetoothUuid &gatt);
    void serviceScanDone(void);
    void update();
    void error(QLowEnergyController::Error err);
    void errorService(QLowEnergyService::ServiceError);

    void inclinationChanged(double grade, double percentage);
};
#endif // WAHOOKICKRSNAPBIKE_H
