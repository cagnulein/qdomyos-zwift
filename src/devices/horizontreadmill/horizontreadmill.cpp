#include "horizontreadmill.h"
#include "homeform.h"
#include "devices/ftmsbike/ftmsbike.h"
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#endif
#include <chrono>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

horizontreadmill::horizontreadmill(bool noWriteResistance, bool noHeartService) {

    testProfileCRC();

    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &horizontreadmill::update);
    refresh->start(200ms);
}

void horizontreadmill::writeCharacteristic(QLowEnergyService *service, QLowEnergyCharacteristic characteristic,
                                           uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    if (!service) {
        qDebug() << "no gattCustomService available";
        return;
    }

    int timeA = 8000;
    int timeB = 3000;
    if (!gattCustomService) {
        timeA = timeB = 300; // for the normal treadmills, the timeout must be small, otherwise we are blocking the update timer on iOS
    }

    if (wait_for_response) {
        connect(this, &horizontreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(timeA, &loop, SLOT(quit())); // 6 seconds are important
    } else {
        connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(timeB, &loop, SLOT(quit()));
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (characteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        service->writeCharacteristic(characteristic, *writeBuffer, QLowEnergyService::WriteWithoutResponse);
    } else {
        service->writeCharacteristic(characteristic, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << " >> " << writeBuffer->toHex(' ') << " // " << info;

    loop.exec();
}

void horizontreadmill::waitForAPacket() {
    QEventLoop loop;
    QTimer timeout;
    connect(this, &horizontreadmill::packetReceived, &loop, &QEventLoop::quit);
    timeout.singleShot(3000, &loop, SLOT(quit()));
    loop.exec();
}

void horizontreadmill::btinit() {
    QSettings settings;
    QStringList horizon_treadmill_profile_users;
    horizon_treadmill_profile_users.append(
        settings.value(QZSettings::horizon_treadmill_profile_user1, QZSettings::default_horizon_treadmill_profile_user1)
            .toString());
    horizon_treadmill_profile_users.append(
        settings.value(QZSettings::horizon_treadmill_profile_user2, QZSettings::default_horizon_treadmill_profile_user2)
            .toString());
    horizon_treadmill_profile_users.append(
        settings.value(QZSettings::horizon_treadmill_profile_user3, QZSettings::default_horizon_treadmill_profile_user3)
            .toString());
    horizon_treadmill_profile_users.append(
        settings.value(QZSettings::horizon_treadmill_profile_user4, QZSettings::default_horizon_treadmill_profile_user4)
            .toString());
    horizon_treadmill_profile_users.append(
        settings.value(QZSettings::horizon_treadmill_profile_user5, QZSettings::default_horizon_treadmill_profile_user5)
            .toString());
    bool horizon_paragon_x =
        settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();

    uint8_t initData01_paragon[] = {0x55, 0xaa, 0x00, 0x00, 0x02, 0x20, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a};

    uint8_t initData02_paragon[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x02, 0x0e, 0x00, 0x42,
                                    0xef, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData03_paragon[] = {0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a};

    uint8_t initData01[] = {0x55, 0xaa, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00};

    uint8_t initData02[] = {0x55, 0xaa, 0x09, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData03[] = {0x55, 0xaa, 0x0a, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData04[] = {0x55, 0xaa, 0x0b, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData05[] = {0x55, 0xaa, 0x0c, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData06[] = {0x55, 0xaa, 0x0d, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};

    uint8_t initData2[] = {0x55, 0xaa, 0x0e, 0x00, 0x03, 0x14, 0x08, 0x00, 0x3f,
                           0x01, 0xe5, 0x07, 0x02, 0x08, 0x13, 0x12, 0x21, 0x00};
    uint8_t initData3[] = {0x55, 0xaa, 0x0f, 0x00, 0x03, 0x01, 0x01, 0x00, 0xd1, 0xf1, 0x01};
    uint8_t initData4[] = {0x55, 0xaa, 0x10, 0x00, 0x03, 0x10, 0x01, 0x00, 0xf0, 0xe1, 0x00};

    QByteArray username;
    for (int l = 0; l < horizon_treadmill_profile_users.length(); l++) {
        QString nickname = horizon_treadmill_profile_users.at(l);
        if (nickname.length() > 8)
            nickname = nickname.left(8);
        else if (nickname.length() == 0)
            nickname = "user";
        username = nickname.toLocal8Bit();
        for (int i = 0; i < 9; i++) {
            uint8_t Char;
            if (i < username.length())
                Char = username.at(i);
            if (nickname.length() <= i)
                Char = 0;
            switch (l) {
            case 0:
                initData7_2[11 + i] = Char;
                break;
            case 1:
                initData7_3[11 + i] = Char;
                break;
            case 2:
                initData7_4[11 + i] = Char;
                break;
            case 3:
                initData7_5[11 + i] = Char;
                break;
            default:
                initData7_6[11 + i] = Char;
                break;
            }
        }
    }

    updateProfileCRC();

    if (gattCustomService) {

        if (horizon_paragon_x) {
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData01_paragon,
                                sizeof(initData01_paragon), QStringLiteral("init"), false, true);

            waitForAPacket();
            waitForAPacket();
            waitForAPacket();
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData02_paragon,
                                sizeof(initData02_paragon), QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData03_paragon,
                                sizeof(initData03_paragon), QStringLiteral("init"), false, true);

        } else {
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData01, sizeof(initData01),
                                QStringLiteral("init"), false, true);
            waitForAPacket();

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...0%");
            }

        init1:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7, sizeof(initData7),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9, sizeof(initData9),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10, sizeof(initData10),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                if(gattFTMSService && homeform::singleton()) {
                    homeform::singleton()->setToastRequested("Enable the 'Force Using FTMS' setting under the Settings->Treadmill Options->Horizon Treadmill options and restart the app");
                }
                qDebug() << "init 1 not received";
                waitForAPacket();
                goto init1;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...15%");
            }        

        init2:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7_1, sizeof(initData7_1),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9_1, sizeof(initData9_1),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10_1, sizeof(initData10_1),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 2 not received";
                waitForAPacket();
                goto init2;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...25%");
            }            

        init3:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7_2, sizeof(initData7_2),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9_2, sizeof(initData9_2),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10_2, sizeof(initData10_2),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 3 not received";
                waitForAPacket();
                goto init3;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...35%");
            }

        init4:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7_3, sizeof(initData7_3),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9_3, sizeof(initData9_3),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10_3, sizeof(initData10_3),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 4 not received";
                waitForAPacket();
                goto init4;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...50%");
            }            

        init5:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7_4, sizeof(initData7_4),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9_4, sizeof(initData9_4),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10_4, sizeof(initData10_4),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 5 not received";
                waitForAPacket();
                goto init5;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...65%");
            }

        init6:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7_5, sizeof(initData7_5),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9_5, sizeof(initData9_5),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10_5, sizeof(initData10_5),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 6 not received";
                waitForAPacket();
                goto init6;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...80%");
            }            

        init7:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData7_6, sizeof(initData7_6),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData9_6, sizeof(initData9_6),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData10_6, sizeof(initData10_6),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData11, sizeof(initData11),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData12, sizeof(initData12),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData8, sizeof(initData8),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData13, sizeof(initData13),
                                QStringLiteral("init"), false, false);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData14, sizeof(initData14),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 7 not received";
                waitForAPacket();
                goto init7;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization in progress...90%");
            }

        init8:
            initPacketRecv = false;

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData02, sizeof(initData02),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData03, sizeof(initData03),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData04, sizeof(initData04),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData05, sizeof(initData05),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData06, sizeof(initData06),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData2, sizeof(initData2),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData3, sizeof(initData3),
                                QStringLiteral("init"), false, true);
            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData4, sizeof(initData4),
                                QStringLiteral("init"), false, true);

            if (!initPacketRecv) {
                qDebug() << "init 8 not received";
                waitForAPacket();
                goto init8;
            }

            if(homeform::singleton()) {
                homeform::singleton()->setToastRequested("Treadmill initialization completed!");
            }            
        }
        messageID = 0x10;
    }

    if(YPOO_MINI_PRO) {
        uint8_t write[] = {0x01, 0x00, 0x00, 0x03, 0x08, 0x00, 0x02, 0x09};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointIdYpooMiniPro, write, sizeof(write), "requestControl", false, false);
        QThread::msleep(500);
    }

    if(wellfit_treadmill || SW_TREADMILL || YPOO_MINI_PRO) {
        uint8_t write[] = {FTMS_REQUEST_CONTROL};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl", false,
                            false);
        QThread::msleep(500);
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl", false,
                            false);
    }

    initDone = true;
}

float horizontreadmill::float_one_point_round(float value) { return ((float)((int)(value * 10))) / 10; }

void horizontreadmill::update() {
    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

    qDebug() << initRequest << firstStateChanged << bluetoothDevice.isValid();

    if (initRequest && firstStateChanged) {
        btinit();
        initRequest = false;
    } else if (bluetoothDevice.isValid() //&&

               // m_control->state() == QLowEnergyController::DiscoveredState //&&
               // gattCommunicationChannelService &&
               // gattWriteCharacteristic.isValid() &&
               // gattNotify1Characteristic.isValid() &&
               /*initDone*/) {

        QSettings settings;
        bool horizon_treadmill_7_8 =
            settings.value(QZSettings::horizon_treadmill_7_8, QZSettings::default_horizon_treadmill_7_8).toBool();
        bool horizon_paragon_x =
            settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();
        update_metrics(!powerReceivedFromPowerSensor, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        if (firstDistanceCalculated) {
            QDateTime now = QDateTime::currentDateTime();
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
            1.19) *
            settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
            200.0) /
            (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                        now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60    
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

            lastRefreshCharacteristicChanged = now;
        }

        // updating the treadmill console every second
        if (sec1Update++ == (1000 / refresh->interval())) {

            sec1Update = 0;
            if(trx3500_treadmill) {
                uint8_t write[] = {FTMS_START_RESUME};
                writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write),
                                "start simulation", false, false);
            }
            // updateDisplay(elapsed);
        }

        // this treadmill can't go below 1
        if(mobvoi_tmp_treadmill && requestSpeed < 1)
            requestSpeed = -1;
        
        if (requestSpeed != -1) {
            bool minSpeed =
                fabs(requestSpeed - float_one_point_round(currentSpeed().value())) >= (minStepSpeed() - 0.09);
            bool forceSpeedNeed = checkIfForceSpeedNeeding(requestSpeed);
            qDebug() << "requestSpeed=" << requestSpeed << minSpeed << forceSpeedNeed
                     << float_one_point_round(currentSpeed().value());
            if (float_one_point_round(requestSpeed) != float_one_point_round(currentSpeed().value()) && minSpeed && requestSpeed >= 0 && requestSpeed <= 22 &&
                forceSpeedNeed) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(float_one_point_round(requestSpeed));
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            requestInclination = treadmillInclinationOverrideReverse(requestInclination);

            // this treadmill doesn't send the incline, so i'm forcing it manually
            if(schwinn_810_treadmill) {
                Inclination = requestInclination;
            }

            qDebug() << "requestInclination=" << requestInclination;
            if (requestInclination < minInclination)
                requestInclination = minInclination;
            else {
                // the treadmill accepts only .5 steps
                requestInclination = std::llround(requestInclination * 2) / 2.0;
                qDebug() << "requestInclination after rounding=" << requestInclination;
            }

            // this treadmill doesn't handle the 0.5%
            if(sole_f85_treadmill && requestInclination == 0.5)
                requestInclination = 1.0;

            if (requestInclination != currentInclination().value() && requestInclination >= minInclination &&
                requestInclination <= 15) {
               
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceIncline(requestInclination);

                // this treadmill doesn't send the incline, so i'm forcing it manually
                if(SW_TREADMILL) {
                    Inclination = requestInclination;
                }
            }
            requestInclination = -100;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
            if (gattCustomService) {
                if (!horizon_paragon_x) {
                    if (horizon_treadmill_7_8) {
                        messageID++;
                        // start
                        if (!horizonPaused) { // we can't use the paused, because it's updated while we're pressing the
                                              // start button
                            // 0x17 0x34 = 99 minutes (99 * 60 = 5940)
                            uint8_t write1[] = {0x55, 0xaa, 0x12, 0x00, 0x03, 0x02, 0x11, 0x00, 0x1a,
                                                0x17, 0x00, 0x00, 0x34, 0x17, 0x00, 0x00, 0x00, 0x00,
                                                0x00, 0x05, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01};
                            int confirm = GenerateCRC_CCITT(&write1[10], 17);
                            write1[2] = messageID & 0xff;
                            write1[3] = messageID >> 8;
                            write1[8] = confirm & 0xff;
                            write1[9] = confirm >> 8;

                            writeCharacteristic(gattCustomService, gattWriteCharCustomService, write1, 20,
                                                QStringLiteral("requestStart"), false, false);
                            writeCharacteristic(gattCustomService, gattWriteCharCustomService, &write1[20],
                                                sizeof(write1) - 20, QStringLiteral("requestStart"), false, true);
                        } else {
                            // resume
                            uint8_t write1[] = {0x55, 0xaa, 0x13, 0x00, 0x03, 0x03, 0x01, 0x00, 0xd1, 0xf1, 0x01};
                            write1[2] = messageID & 0xff;
                            write1[3] = messageID >> 8;

                            writeCharacteristic(gattCustomService, gattWriteCharCustomService, write1, sizeof(write1),
                                                QStringLiteral("requestResume"), false, false);
                        }
                    } else {
                        uint8_t initData5[] = {0x55, 0xaa, 0x11, 0x00, 0x03, 0x02, 0x11, 0x00, 0x84, 0xbe,
                                               0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
                        uint8_t initData6[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01};

                        writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData5, sizeof(initData5),
                                            QStringLiteral("init"), false, false);
                        writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData6, sizeof(initData6),
                                            QStringLiteral("init"), false, true);
                        messageID++;
                    }
                } else {
                    uint8_t initData02_paragon[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x02, 0x0e, 0x00, 0x38,
                                                    0xce, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

                    // TODO: calculate the checksum if you change the speed and incline

                    // uint8_t initData03_paragon[] = {0x01, 0x14 /*2 km/h * 10 */, 0x00, 0x00 /* incline *10 */, 0x00,
                    // 0x00, 0x0d, 0x0a};
                    uint8_t initData03_paragon[] = {0x01, 0x38, 0x00, 0x0f, 0x00, 0x00, 0x0d, 0x0a};

                    writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData02_paragon,
                                        sizeof(initData02_paragon), QStringLiteral("starting"), false, false);
                    writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData03_paragon,
                                        sizeof(initData03_paragon), QStringLiteral("starting"), false, true);
                }
            } else if (gattFTMSService) {
                uint8_t write[] = {FTMS_REQUEST_CONTROL};
                writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write),
                                    "requestControl", false, false);
                write[0] = {FTMS_START_RESUME};
                writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write),
                                    "start simulation", false, false);
            }
            horizonPaused = false;
            lastStart = QDateTime::currentMSecsSinceEpoch();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

            if (gattCustomService) {
                if (!horizon_paragon_x) {
                    /*if (horizon_treadmill_7_8)*/ {
                        // stop
                        if (requestPause == -1) {
                            Speed = 0; // forcing the speed to be sure, maybe I could remove this
                            if (!settings
                                     .value(QZSettings::horizon_treadmill_disable_pause,
                                            QZSettings::default_horizon_treadmill_disable_pause)
                                     .toBool()) {
                                messageID++;
                                uint8_t write1[] = {0x55, 0xaa, 0x13, 0x00, 0x01, 0x14, 0x00, 0x00, 0x00, 0x00};
                                write1[2] = messageID & 0xff;
                                write1[3] = messageID >> 8;

                                writeCharacteristic(gattCustomService, gattWriteCharCustomService, write1,
                                                    sizeof(write1), QStringLiteral("requestStop"), false, true);
                            }
                            // pause
                        } else {
                            requestPause = -1;
                            Speed = 0; // forcing the speed to be sure, maybe I could remove this
                            if (!settings
                                     .value(QZSettings::horizon_treadmill_disable_pause,
                                            QZSettings::default_horizon_treadmill_disable_pause)
                                     .toBool()) {
                                messageID++;
                                uint8_t write1[] = {0x55, 0xaa, 0x12, 0x00, 0x03, 0x03, 0x01, 0x00, 0xf0, 0xe1, 0x00};
                                write1[2] = messageID & 0xff;
                                write1[3] = messageID >> 8;

                                writeCharacteristic(gattCustomService, gattWriteCharCustomService, write1,
                                                    sizeof(write1), QStringLiteral("requestPause"), false, false);
                                horizonPaused = true;
                            }
                        }
                    }
                } else {
                    // horizon paragon x
                    Speed = 0;
                    if (requestPause == -1) {
                        uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x02, 0x14, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a};
                        writeCharacteristic(gattCustomService, gattWriteCharCustomService, write, sizeof(write),
                                            QStringLiteral("requestStop"), false, true);
                    } else {
                        requestPause = -1;
                        uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a};
                        writeCharacteristic(gattCustomService, gattWriteCharCustomService, write, sizeof(write),
                                            QStringLiteral("requestPause"), false, true);
                        horizonPaused = true;
                    }
                }
            } else if (gattFTMSService) {
                if (requestPause == -1) {
                    uint8_t writeS[] = {FTMS_STOP_PAUSE, 0x01};

                    writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                                        QStringLiteral("stop"), false, true);
                } else {
                    requestPause = -1;
                    Speed = 0; // forcing the speed to be sure, maybe I could remove this
                    uint8_t writeS[] = {FTMS_STOP_PAUSE, 0x02};

                    writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                                        QStringLiteral("stop"), false, true);
                }
            }

            if(SW_TREADMILL) {
                Inclination = 0;
            }
            
            lastStop = QDateTime::currentMSecsSinceEpoch();

            requestStop = -1;
        }
        if (requestIncreaseFan != -1) {
            emit debug(QStringLiteral("increasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed + 1);
            requestIncreaseFan = -1;
        } else if (requestDecreaseFan != -1) {
            emit debug(QStringLiteral("decreasing fan speed..."));

            // sendChangeFanSpeed(FanSpeed - 1);
            requestDecreaseFan = -1;
        }
    }
}

bool horizontreadmill::checkIfForceSpeedNeeding(double requestSpeed) {
    QSettings settings;
    bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
    const double miles_conversion = 0.621371;

    if (gattCustomService && miles) {
        requestSpeed *= miles_conversion;

        uint8_t uReqSpeed = (uint8_t)(requestSpeed * 10);
        uint8_t uCurSpeed = (uint8_t)(lastHorizonForceSpeed * 10);

        lastHorizonForceSpeed = requestSpeed;

        return uReqSpeed != uCurSpeed;
    }
    return true;
}

// example frame: 55aa320003050400532c00150000
void horizontreadmill::forceSpeed(double requestSpeed) {
    QSettings settings;
    const double miles_conversion = 0.621371;
    bool horizon_paragon_x =
        settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();

    if (gattCustomService) {
        if (!horizon_paragon_x) {
            messageID++;
            uint8_t datas[4];
            double s = qRound(requestSpeed * 0.621371 * 10);
            datas[0] = 0;
            datas[1] = (uint8_t)(s)&0xff;
            datas[2] = (uint16_t)(s) >> 8;
            datas[3] = 0;
            int confirm = GenerateCRC_CCITT(datas, 4);
            uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            write[2] = messageID & 0xff;
            write[3] = messageID >> 8;
            write[8] = confirm & 0xff;
            write[9] = confirm >> 8;
            write[10] = datas[0];
            write[11] = datas[1];
            write[12] = datas[2];
            write[13] = datas[3];

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, write, sizeof(write),
                                QStringLiteral("forceSpeed"), false, true);
        } else {
            uint8_t datas[3];
            bool miles = settings.value(QZSettings::miles_unit, QZSettings::default_miles_unit).toBool();
            double miles_conversion = 1.0;
            if (miles)
                miles_conversion = 0.621371;
            double s = qRound(requestSpeed * miles_conversion * 10);
            datas[0] = (uint8_t)(s)&0xff;
            datas[1] = (uint16_t)(s) >> 8;
            datas[2] = 0x01;
            uint8_t initData02_paragon[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x05, 0x03, 0x00,
                                            0x00, 0x00, 0x6f, 0x00, 0x01, 0x0d, 0x0a};

            int confirm = GenerateCRC_CCITT(datas, 3);

            initData02_paragon[8] = confirm & 0xff;
            initData02_paragon[9] = confirm >> 8;

            initData02_paragon[10] = datas[0];
            initData02_paragon[11] = datas[1];
            initData02_paragon[12] = datas[2];

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData02_paragon,
                                sizeof(initData02_paragon), QStringLiteral("forceSpeed"), false, false);
        }
    } else if (gattFTMSService) {
        // for the Tecnogym Myrun
        if(!anplus_treadmill && !trx3500_treadmill && !wellfit_treadmill && !mobvoi_tmp_treadmill && !SW_TREADMILL && !ICONCEPT_FTMS_treadmill && !YPOO_MINI_PRO && !T3G_PRO && !T3G_ELITE) {
            uint8_t write[] = {FTMS_REQUEST_CONTROL};
            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl", false,
                                false);
            write[0] = {FTMS_START_RESUME};
            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "start simulation",
                                false, false);
        }

        uint8_t writeS[] = {FTMS_SET_TARGET_SPEED, 0x00, 0x00};
        if(BOWFLEX_T9) {
            requestSpeed *= miles_conversion;   // this treadmill wants the speed in miles, at least seems so!!
        }
        uint16_t speed_int = round(requestSpeed * 100);
        writeS[1] = speed_int & 0xFF;
        writeS[2] = speed_int >> 8;

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("forceSpeed"), false, false);
    }
}

// example frame: 55aa3800030603005d0b0a0000
void horizontreadmill::forceIncline(double requestIncline) {
    QSettings settings;
    bool horizon_paragon_x =
        settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();

    if(tunturi_t60_treadmill)
        Inclination = treadmillInclinationOverride(requestIncline);

    if (gattCustomService) {
        if (!horizon_paragon_x) {
            messageID++;
            uint8_t datas[3];
            datas[0] = (uint8_t)(requestIncline * 10) & 0xff;
            datas[1] = (uint16_t)(requestIncline * 10) >> 8;
            datas[2] = 0;
            int confirm = GenerateCRC_CCITT(datas, 3);
            uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            write[2] = messageID & 0xff;
            write[3] = messageID >> 8;
            write[8] = confirm & 0xff;
            write[9] = confirm >> 8;
            write[10] = datas[0];
            write[11] = datas[1];
            write[12] = datas[2];

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, write, sizeof(write),
                                QStringLiteral("forceIncline"), false, true);
        } else {
            uint8_t initData02_paragon[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x06, 0x02,
                                            0x00, 0x39, 0xcc, 0x4b, 0x00, 0x0d, 0x0a};

            uint8_t datas[2];
            datas[0] = (uint8_t)(requestIncline * 10) & 0xff;
            datas[1] = 0;

            int confirm = GenerateCRC_CCITT(datas, 2);

            initData02_paragon[8] = confirm & 0xff;
            initData02_paragon[9] = confirm >> 8;

            initData02_paragon[10] = datas[0];

            writeCharacteristic(gattCustomService, gattWriteCharCustomService, initData02_paragon,
                                sizeof(initData02_paragon), QStringLiteral("forceIncline"), false, false);
        }
    } else if (gattFTMSService) {
        // for the Tecnogym Myrun
        if(!anplus_treadmill && !trx3500_treadmill && !mobvoi_tmp_treadmill && !SW_TREADMILL && !ICONCEPT_FTMS_treadmill && !YPOO_MINI_PRO && !T3G_PRO && !T3G_ELITE) {
            uint8_t write[] = {FTMS_REQUEST_CONTROL};
            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl", false,
                                false);
            write[0] = {FTMS_START_RESUME};
            writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "start simulation",
                                false, false);
        }

        uint8_t writeS[] = {FTMS_SET_TARGET_INCLINATION, 0x00, 0x00};
        if (kettler_treadmill) {
            int16_t r = ((int16_t)(requestIncline * 10.0));

            if (r < 0)
                r = 0;
            else if (r > 100) // max 10% inclination
                r = 100;

            // send:  1/0 a  14 1e 28 32 3c 46 50 5a 64 6e 78 82 8c 96
            // recv:  0   5   a 14 19 1e 28 2d 32 3c 41 46 50 55 5a 64
            // recv:  0   5  10 20 25 30 40 45 50 60 65 70 80 85 90 100

            /*
             * Kinomap | TM
                01.0% | 00.5%
                02.0% | 01.0%
                03.0% | 02.0%
                04.0% | 02.5%
                05.0% | 03.0%
                06.0% | 04.0%
                07.0% | 04.5%
                08.0% | 05.0%
                09.0% | 06.0%
                10.0% | 06.5%
                11.0% | 07.0%
                12.0% | 08.0%
                13.0% | 08.5%
                14.0% | 09.0%
                15.0% | 10.0%
               */

            QHash<uint8_t, uint8_t> conversion;
            QHash<uint8_t, uint8_t> conversion1;
            conversion[0] = 0;
            conversion1[0] = 0;
            conversion[5] = 0x05;
            conversion1[5] = 0;
            conversion[10] = 0x14;
            conversion1[10] = 0;
            conversion[15] = 0x15;
            conversion1[15] = 0;
            conversion[20] = 0x1e;
            conversion1[20] = 0;
            conversion[25] = 0x25;
            conversion1[25] = 0;
            conversion[30] = 0x32;
            conversion1[30] = 0;
            conversion[35] = 0x35;
            conversion1[35] = 0;
            conversion[40] = 0x3c;
            conversion1[40] = 0;
            conversion[45] = 0x45;
            conversion1[45] = 0;
            conversion[50] = 0x50;
            conversion1[50] = 0;
            conversion[55] = 0x55;
            conversion1[55] = 0;
            conversion[60] = 0x5a;
            conversion1[60] = 0;
            conversion[65] = 0x65;
            conversion1[65] = 0;
            conversion[70] = 0x6e;
            conversion1[70] = 0;
            conversion[75] = 0x75;
            conversion1[75] = 0;
            conversion[80] = 0x78;
            conversion1[80] = 0;
            conversion[85] = 0x85;
            conversion1[85] = 0;
            conversion[90] = 0x8c;
            conversion1[90] = 0;
            conversion[95] = 0x95;
            conversion1[95] = 0;
            conversion[100] = 0x96;
            conversion1[100] = 0x00;
            conversion[105] = 0x05;
            conversion1[105] = 0x01;
            conversion[110] = 0x10;
            conversion1[110] = 0x01;
            conversion[115] = 0x15;
            conversion1[115] = 0x01;
            conversion[120] = 0x20;
            conversion1[120] = 0x01;

            writeS[1] = conversion[r];
            writeS[2] = conversion1[r];
        } else if(ICONCEPT_FTMS_treadmill) {
            if(requestInclination > 0 && requestInclination < 1) {
                writeS[1] = 0x3C;
                writeS[2] = 0x00;
            } else if(requestInclination >= 1 && requestInclination < 2) {
                writeS[1] = 0x82;
                writeS[2] = 0x00;
            } else if(requestInclination >= 2 && requestInclination < 3) {
                writeS[1] = 0xC8;
                writeS[2] = 0x00;
            } else if(requestInclination >= 3 && requestInclination < 4) {
                writeS[1] = 0x04;
                writeS[2] = 0x01;
            } else if(requestInclination >= 4 && requestInclination < 5) {
                writeS[1] = 0x4A;
                writeS[2] = 0x01;
            } else if(requestInclination >= 5 && requestInclination < 6) {
                writeS[1] = 0x90;
                writeS[2] = 0x01;
            } else if(requestInclination >= 6 && requestInclination < 7) {
                writeS[1] = 0xCC;
                writeS[2] = 0x01;
            } else if(requestInclination >= 7 && requestInclination < 8) {
                writeS[1] = 0x12;
                writeS[2] = 0x02;
            } else if(requestInclination >= 8 && requestInclination < 9) {
                writeS[1] = 0x58;
                writeS[2] = 0x02;
            } else if(requestInclination >= 9 && requestInclination < 10) {
                writeS[1] = 0x94;
                writeS[2] = 0x02;
            } else if(requestInclination >= 10 && requestInclination < 11) {
                writeS[1] = 0xDA;
                writeS[2] = 0x02;
            } else if(requestInclination >= 11 && requestInclination < 12) {
                writeS[1] = 0x20;
                writeS[2] = 0x03;
            } else if(requestInclination >= 12 && requestInclination < 13) {
                writeS[1] = 0x5C;
                writeS[2] = 0x03;
            } else if(requestInclination >= 13 && requestInclination < 14) {
                writeS[1] = 0xA2;
                writeS[2] = 0x03;
            } else if(requestInclination >= 14 && requestInclination < 15) {
                writeS[1] = 0xE8;
                writeS[2] = 0x03;
            } else {
                writeS[1] = 0x00;
                writeS[2] = 0x00;
            }
        } else if(T3G_PRO || T3G_ELITE) {
            if(requestInclination > 0 && requestInclination < 1) {
                writeS[1] = 0x10;
                writeS[2] = 0x00;
            } else if(requestInclination >= 1 && requestInclination < 2) {
                writeS[1] = 0x1a;
                writeS[2] = 0x00;
            } else if(requestInclination >= 2 && requestInclination < 3) {
                writeS[1] = 0x24;
                writeS[2] = 0x00;
            } else if(requestInclination >= 3 && requestInclination < 4) {
                writeS[1] = 0x2e;
                writeS[2] = 0x00;
            } else if(requestInclination >= 4 && requestInclination < 5) {
                writeS[1] = 0x38;
                writeS[2] = 0x00;
            } else if(requestInclination >= 5 && requestInclination < 6) {
                writeS[1] = 0x42;
                writeS[2] = 0x00;
            } else if(requestInclination >= 6 && requestInclination < 7) {
                writeS[1] = 0x4c;
                writeS[2] = 0x00;
            } else if(requestInclination >= 7 && requestInclination < 8) {
                writeS[1] = 0x56;
                writeS[2] = 0x00;
            } else if(requestInclination >= 8 && requestInclination < 9) {
                writeS[1] = 0x60;
                writeS[2] = 0x00;
            } else if(requestInclination >= 9 && requestInclination < 10) {
                writeS[1] = 0x6A;
                writeS[2] = 0x00;
            } else if(requestInclination >= 10 && requestInclination < 11) {
                writeS[1] = 0x74;
                writeS[2] = 0x00;
            } else {
                writeS[1] = 0x00;
                writeS[2] = 0x00;
            }
        } else {
            if(HORIZON_78AT_treadmill)
                requestIncline = requestIncline / 2.0;
            writeS[1] = ((int16_t)(requestIncline * 10.0)) & 0xFF;
            writeS[2] = ((int16_t)(requestIncline * 10.0)) >> 8;
        }

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("forceIncline"), false, false);
    }
}

void horizontreadmill::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void horizontreadmill::characteristicChanged(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    double heart = 0; // NOTE : Should be initialized with a value to shut clang-analyzer's
                      // UndefinedBinaryOperatorResult
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    bool distanceEval = false;
    QSettings settings;
    // bool horizon_paragon_x = settings.value(QZSettings::horizon_paragon_x,
    // QZSettings::default_horizon_paragon_x).toBool();
    bool disable_hr_frommachinery =
        settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    QDateTime now = QDateTime::currentDateTime();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + " " + QString::number(newValue.length()) +
               " " + newValue.toHex(' '));

    if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4)) {
        if (newValue.at(0) == 0x55 && newValue.length() > 7) {
            lastPacketComplete.clear();
            customRecv = (((uint16_t)((uint8_t)newValue.at(7)) << 8) | (uint16_t)((uint8_t)newValue.at(6))) + 10;
            qDebug() << "new custom packet received. Len expected: " << customRecv;
        }

        lastPacketComplete.append(newValue);
        customRecv -= newValue.length();
        if (customRecv <= 0) {
            emit debug(QStringLiteral(" << FULL ") + " " + lastPacketComplete.toHex(' '));
            qDebug() << "full custom packet received";
            initPacketRecv = true;
            customRecv = 0;
            emit packetReceived();
        }
    }

    if (isPaused() && settings
                          .value(QZSettings::horizon_treadmill_suspend_stats_pause,
                                 QZSettings::default_horizon_treadmill_suspend_stats_pause)
                          .toBool()) {
        qDebug() << "treadmill paused so I'm ignoring the new metrics";
        return;
    }

    if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && lastPacketComplete.length() > 70 &&
        lastPacketComplete.at(0) == 0x55 && lastPacketComplete.at(5) == 0x17) {
        parseSpeed((((double)(((uint16_t)((uint8_t)lastPacketComplete.at(25)) << 8) |
                           (uint16_t)((uint8_t)lastPacketComplete.at(24)))) /
                 100.0) *
                1.60934); // miles/h
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        parseInclination(treadmillInclinationOverride((double)((uint8_t)lastPacketComplete.at(30)) / 10.0));
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        if (firstDistanceCalculated && watts(weight))
            KCal +=
                ((((0.048 * ((double)watts(weight)) +
                    1.19) *
                   weight * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (firstDistanceCalculated)
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        distanceEval = true;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() > 70 &&
               newValue.at(0) == 0x55 && newValue.at(5) == 0x12) {
        parseSpeed((((double)(((uint16_t)((uint8_t)newValue.at(62)) << 8) | (uint16_t)((uint8_t)newValue.at(61)))) / 1000.0) *
                       1.60934); // miles/h
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        parseInclination(treadmillInclinationOverride((double)((uint8_t)newValue.at(63)) / 10.0));
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        if (firstDistanceCalculated && watts(weight))
            KCal +=
                ((((0.048 * ((double)watts(weight)) +
                    1.19) *
                   weight * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (firstDistanceCalculated)
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        distanceEval = true;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() == 29 &&
               newValue.at(0) == 0x55) {
        parseSpeed(((double)(((uint16_t)((uint8_t)newValue.at(15)) << 8) | (uint16_t)((uint8_t)newValue.at(14)))) / 10.0);
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        // Inclination = (double)((uint8_t)newValue.at(3)) / 10.0;
        // emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        if (firstDistanceCalculated && watts(weight))
            KCal +=
                ((((0.048 * ((double)watts(weight)) +
                    1.19) *
                   weight * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
        // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (firstDistanceCalculated)
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        distanceEval = true;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() > 10 &&
               (uint8_t)newValue.at(0) == 0x55 && (uint8_t)newValue.at(1) == 0xAA && (uint8_t)newValue.at(2) == 0x00 &&
               (uint8_t)newValue.at(3) == 0x00 && (uint8_t)newValue.at(4) == 0x03 && (uint8_t)newValue.at(5) == 0x03 &&
               (uint8_t)newValue.at(6) == 0x01 && (uint8_t)newValue.at(7) == 0x00 && (uint8_t)newValue.at(8) == 0xf0 &&
               (uint8_t)newValue.at(9) == 0xe1 && (uint8_t)newValue.at(10) == 0x00) {

        Speed = 0;
        horizonPaused = true;
        qDebug() << "stop from the treadmill";
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2AD2)) {
        union flags {
            struct {
                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t instantCadence : 1;
                uint16_t avgCadence : 1;
                uint16_t totDistance : 1;
                uint16_t resistanceLvl : 1;
                uint16_t instantPower : 1;
                uint16_t avgPower : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                  (uint16_t)((uint8_t)newValue.at(index)))) /
                        100.0;
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.instantCadence) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          2.0;
            }
            index += 2;
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        }

        if (Flags.avgCadence) {
            double avgCadence;
            avgCadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index)))) /
                         2.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Cadence: ") + QString::number(avgCadence));
        }

        if (Flags.totDistance) {

            /*
             * the distance sent from the most trainers is a total distance, so it's useless for QZ
             *
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;*/
            index += 3;
        }

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.resistanceLvl) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        }

        if (Flags.instantPower) {
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                           .toString()
                           .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (Flags.avgPower && newValue.length() > index + 1) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            /*KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));*/
            index += 2;

                   // energy per hour
            index += 2;

                   // energy per minute
            index += 1;
        }

        if (watts(weight))
            KCal += ((((0.048 * ((double)watts(weight)) + 1.19) *
                       weight * 3.5) /
                      200.0) /
                     (60000.0 /
                      ((double)lastRefreshCharacteristicChanged.msecsTo(
                          now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                   // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
            heart = Flags.heartRate;
        }

        if (Flags.metabolic) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }

    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
        bool horizon_treadmill_7_0_at_24 = settings.value(QZSettings::horizon_treadmill_7_0_at_24, QZSettings::default_horizon_treadmill_7_0_at_24).toBool();
        lastPacket = newValue;

        // default flags for this treadmill is 84 04

        union flags {
            struct {

                uint16_t moreData : 1;
                uint16_t avgSpeed : 1;
                uint16_t totalDistance : 1;
                uint16_t inclination : 1;
                uint16_t elevation : 1;
                uint16_t instantPace : 1;
                uint16_t averagePace : 1;
                uint16_t expEnergy : 1;
                uint16_t heartRate : 1;
                uint16_t metabolic : 1;
                uint16_t elapsedTime : 1;
                uint16_t remainingTime : 1;
                uint16_t forceBelt : 1;
                uint16_t spare : 3;
            };

            uint16_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(1) << 8) | newValue.at(0);
        index += 2;

        if (!Flags.moreData) {
            double speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                     (uint16_t)((uint8_t)newValue.at(index)))) /
                           100.0;
            if(BOWFLEX_T9) {
                const double miles_conversion = 0.621371;
                // this treadmill sends the speed in miles!
                speed *= miles_conversion;
            }
            if(!mobvoi_tmp_treadmill || (mobvoi_tmp_treadmill && !horizonPaused))
                parseSpeed(speed);
            index += 2;
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totalDistance) {
            // ignoring the distance, because it's a total life odometer
            // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
            // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
            index += 3;
        }
        // else
        {
            if (firstDistanceCalculated && !isPaused())
                Distance += ((Speed.value() / 3600000.0) *
                             ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination) {
            if(!tunturi_t60_treadmill && !ICONCEPT_FTMS_treadmill)
                parseInclination(treadmillInclinationOverride((double)(
                                  (int16_t)(
                                      ((int16_t)(int8_t)newValue.at(index + 1) << 8) |
                                      (uint8_t)newValue.at(index)
                                      )
                                  ) /
                                                              10.0));
            else if(ICONCEPT_FTMS_treadmill) {
                uint8_t val1 = (uint8_t)newValue.at(index);
                uint8_t val2 = (uint8_t)newValue.at(index + 1);
                if(val1 == 0x3C && val2 == 0x00) {
                    Inclination = 1;
                } else if(val1 == 0x82 && val2 == 0x00) {
                    Inclination = 2;
                } else if(val1 == 0xC8 && val2 == 0x00) {
                    Inclination = 3;
                } else if(val1 == 0x04 && val2 == 0x01) {
                    Inclination = 4;
                } else if(val1 == 0x4A && val2 == 0x01) {
                    Inclination = 5;
                } else if(val1 == 0x90 && val2 == 0x01) {
                    Inclination = 6;
                } else if(val1 == 0xCC && val2 == 0x01) {
                    Inclination = 7;
                } else if(val1 == 0x12 && val2 == 0x02) {
                    Inclination = 8;
                } else if(val1 == 0x58 && val2 == 0x02) {
                    Inclination = 9;
                } else if(val1 == 0x94 && val2 == 0x02) {
                    Inclination = 10;
                } else if(val1 == 0xDA && val2 == 0x02) {
                    Inclination = 11;
                } else if(val1 == 0x20 && val2 == 0x03) {
                    Inclination = 12;
                } else if(val1 == 0x5C && val2 == 0x03) {
                    Inclination = 13;
                } else if(val1 == 0xA2 && val2 == 0x03) {
                    Inclination = 14;
                } else if(val1 == 0xE8 && val2 == 0x03) {
                    Inclination = 15;                    
                } else {
                    Inclination = 0;
                }
            }
            index += 4; // the ramo value is useless
            emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        }

        if (Flags.elevation) {
            index += 4; // TODO
        }

        if (Flags.instantPace) {
            index += 1; // TODO
        }

        if (Flags.averagePace) {
            index += 1; // TODO
        }

        if (Flags.expEnergy) {
            /*KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));*/
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        }
        if (firstDistanceCalculated &&
            watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 *
                        ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 /
                  ((double)lastRefreshCharacteristicChanged.msecsTo(
                      now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                        // kg * 3.5) / 200 ) / 60
        distanceEval = true;

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate) {
                if (index < newValue.length()) {

                    heart = ((double)(((uint8_t)newValue.at(index))));
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
                } else {
                    emit debug(QStringLiteral("Error on parsing heart!"));
                }
                index += 1;
            }
        }

        if (Flags.metabolic) {
            index += 1;
            // todo
        }

        if (Flags.elapsedTime) {
            if (index + 1 < newValue.length()) {
                static uint16_t old_local_elapsed = 0;
                static QDateTime last_local_elapsed_change = QDateTime::currentDateTime();
                uint16_t local_elapsed = ((((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
                qDebug() << QStringLiteral("Local Time Elapsed") << local_elapsed;
                // this treadmill sends here if the user is really running
                if(old_local_elapsed != local_elapsed) {
                    last_local_elapsed_change = QDateTime::currentDateTime();
                    horizonPaused = false;
                }
                QDateTime current_time = QDateTime::currentDateTime();
                 // Only if more than 2 seconds have passed since the last update
                 if(mobvoi_tmp_treadmill && last_local_elapsed_change.secsTo(current_time) >= 2) {
                    Speed = 0;
                    horizonPaused = true;
                    qDebug() << "Forcing Speed to 0 since the treadmill saws that the user is not really running";
                }
                old_local_elapsed = local_elapsed;
            }
            index += 2;
        }

        if (Flags.remainingTime) {
            // todo
        }

        if (Flags.forceBelt) {
            // todo
        }
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACE)) {
        union flags {
            struct {
                uint32_t moreData : 1;
                uint32_t avgSpeed : 1;
                uint32_t totDistance : 1;
                uint32_t stepCount : 1;
                uint32_t strideCount : 1;
                uint32_t elevationGain : 1;
                uint32_t rampAngle : 1;
                uint32_t resistanceLvl : 1;
                uint32_t instantPower : 1;
                uint32_t avgPower : 1;
                uint32_t expEnergy : 1;
                uint32_t heartRate : 1;
                uint32_t metabolicEq : 1;
                uint32_t elapsedTime : 1;
                uint32_t remainingTime : 1;
                uint32_t movementDirection : 1;
                uint32_t spare : 8;
            };

            uint32_t word_flags;
        };

        flags Flags;
        int index = 0;
        Flags.word_flags = (newValue.at(2) << 16) | (newValue.at(1) << 8) | newValue.at(0);
        index += 3;

        if (!Flags.moreData && newValue.length() > index + 1) {
            parseSpeed(((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0);
            emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
            index += 2;
        }

        if (Flags.avgSpeed && newValue.length() > index + 1) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            emit debug(QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed));
        }

        if (Flags.totDistance && newValue.length() > index + 2) {
            Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
                                  (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint32_t)((uint8_t)newValue.at(index)))) /
                       1000.0;
            index += 3;
        } else {
            if (firstDistanceCalculated)
                Distance += ((Speed.value() / 3600000.0) *
                             ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.stepCount && newValue.length() > index + 1) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index))));
            }
            emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));

            index += 2;
            index += 2;
        }

        if (Flags.strideCount) {
            index += 2;
        }

        if (Flags.elevationGain) {
            index += 2;
            index += 2;
        }

        if (Flags.rampAngle) {
            index += 2;
            index += 2;
        }

        if (Flags.resistanceLvl && newValue.length() > index + 1) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
        }

        if (Flags.instantPower && newValue.length() > index + 1) {
            if (!powerReceivedFromPowerSensor)
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            emit debug(QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
            index += 2;
        }

        if (Flags.avgPower && newValue.length() > index + 1) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            emit debug(QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
            index += 2;
        }

        if (Flags.expEnergy && newValue.length() > index + 1) {
            /*KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));*/
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        }

        if (firstDistanceCalculated &&
            watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 *
                        ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 /
                  ((double)lastRefreshCharacteristicChanged.msecsTo(
                      now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                        // kg * 3.5) / 200 ) / 60
        distanceEval = true;

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                emit debug(QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
            }
            heart = Flags.heartRate;
        }

        if (Flags.metabolicEq) {
            // todo
        }

        if (Flags.elapsedTime) {
            // todo
        }

        if (Flags.remainingTime) {
            // todo
        }
    } else if (characteristic.uuid() == QBluetoothUuid::CharacteristicType::RSCMeasurement) {
        uint8_t flags = (uint8_t)newValue.at(0);
        bool InstantaneousStrideLengthPresent = (flags & 0x01);
        bool TotalDistancePresent = (flags & 0x02) ? true : false;
        bool WalkingorRunningStatusbits = (flags & 0x04) ? true : false;
        bool double_cadence = settings
                                  .value(QZSettings::powr_sensor_running_cadence_double,
                                         QZSettings::default_powr_sensor_running_cadence_double)
                                  .toBool();
        double cadence_multiplier = 1.0;
        if (double_cadence)
            cadence_multiplier = 2.0;

        // Unit is in m/s with a resolution of 1/256
        uint16_t speedMs = (((uint16_t)((uint8_t)newValue.at(2)) << 8) | (uint16_t)((uint8_t)newValue.at(1)));
        double speed = (((double)speedMs) / 256.0) * 3.6; // km/h
        double cadence = (uint8_t)newValue.at(3) * cadence_multiplier;
        if (newValue.length() >= 6 && InstantaneousStrideLengthPresent) {
            instantaneousStrideLengthCMAvailableFromDevice = true;
            InstantaneousStrideLengthCM =
                (((uint16_t)((uint8_t)newValue.at(5)) << 8) | (uint16_t)((uint8_t)newValue.at(4))) / 2;
            emit instantaneousStrideLengthChanged(InstantaneousStrideLengthCM.value());
            qDebug() << QStringLiteral("Current InstantaneousStrideLengthCM:") << InstantaneousStrideLengthCM.value();
            if (InstantaneousStrideLengthCM.value() == 0) {
                GroundContactMS.setValue(0);
                VerticalOscillationMM.setValue(0);
                emit groundContactChanged(GroundContactMS.value());
                emit verticalOscillationChanged(VerticalOscillationMM.value());
                qDebug() << QStringLiteral("Current GroundContactMS:") << GroundContactMS.value();
                qDebug() << QStringLiteral("Current VerticalOscillationMM:") << VerticalOscillationMM.value();
            }
        }

        Cadence = cadence;
        emit cadenceChanged(cadence);
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(cadence));
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 ||
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {
            update_hr_from_external();
        } else {

            Heart = heart;
        }
    }

    cadenceFromAppleWatch();

    if (Speed.value() > 0)
        lastStart = 0;
    else
        lastStop = 0;

    if (distanceEval) {
        firstDistanceCalculated = true;
        lastRefreshCharacteristicChanged = now;
    }

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void horizontreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    QBluetoothUuid _gattWriteCharCustomService((quint16)0xFFF3);
    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
    QBluetoothUuid _gattTreadmillDataId((quint16)0x2ACD);
    QBluetoothUuid _gattCrossTrainerDataId((quint16)0x2ACE);
    QBluetoothUuid _gattInclinationSupported((quint16)0x2AD5);
    QBluetoothUuid _DomyosServiceId(QStringLiteral("49535343-fe7d-4ae5-8fa9-9fafd205e455"));
    QBluetoothUuid _YpooMiniProCharId(QStringLiteral("d18d2c10-c44c-11e8-a355-529269fb1459"));
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();

        if(s->serviceUuid() == _DomyosServiceId && DOMYOS) {
            settings.setValue(QZSettings::domyostreadmill_notfmts, true);
            settings.sync();
            if(homeform::singleton())
                homeform::singleton()->setToastRequested("Domyos Treadmill presents itself like a FTMS but it's not. Restart QZ to apply the fix, thanks.");
            return;
        }

        if (s->state() != QLowEnergyService::RemoteServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    notificationSubscribed = 0;

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &horizontreadmill::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &horizontreadmill::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &horizontreadmill::characteristicRead);
            connect(
                s, &QLowEnergyService::errorOccurred,
                this, &horizontreadmill::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &horizontreadmill::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &horizontreadmill::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << c.properties();

                if (c.uuid() == _gattWriteCharControlPointId) {
                    qDebug() << QStringLiteral("FTMS service and Control Point found");
                    gattWriteCharControlPointId = c;
                    gattFTMSService = s;
                } else if (c.uuid() == _gattTreadmillDataId && gattFTMSService == nullptr) {
                    // some treadmills doesn't have the control point so i need anyway to get the FTMS Service at least
                    gattFTMSService = s;
                } else if (c.uuid() == _gattCrossTrainerDataId && gattFTMSService == nullptr) {
                    // some treadmills doesn't have the control point and also are Cross Trainer devices so i need
                    // anyway to get the FTMS Service at least
                    gattFTMSService = s;
                } else if(c.uuid() == _YpooMiniProCharId && YPOO_MINI_PRO) {
                    qDebug() << QStringLiteral("YPOO MINI PRO Control Point found");
                    gattWriteCharControlPointIdYpooMiniPro = c;
                }
                /* else if (c.uuid() == _gattInclinationSupported) {
                    s->readCharacteristic(c);
                    qDebug() << s->serviceUuid() << c.uuid() << "reading!";
                }*/

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharCustomService && !BOWFLEX_T9 && !MX_TM &&
                    !settings
                         .value(QZSettings::horizon_treadmill_force_ftms,
                                QZSettings::default_horizon_treadmill_force_ftms)
                         .toBool()) {
                    qDebug() << QStringLiteral("Custom service and Control Point found");
                    gattWriteCharCustomService = c;
                    gattCustomService = s;
                }
            }
        }
    }

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::RemoteServiceDiscovered) {
            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify &&
                    // if it's a FTMS treadmill and has FTMS and/or RSC service too
                    ((((gattFTMSService && s->serviceUuid() == gattFTMSService->serviceUuid())
                       || (s->serviceUuid() == QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence))
                      && !gattCustomService) ||
                     (gattCustomService && s->serviceUuid() == gattCustomService->serviceUuid()))) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                        notificationSubscribed++;
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                } else if ((c.properties() & QLowEnergyCharacteristic::Indicate) == QLowEnergyCharacteristic::Indicate &&
                                                                                                                      // if it's a FTMS treadmill and has FTMS and/or RSC service too
                           ((((gattFTMSService && s->serviceUuid() == gattFTMSService->serviceUuid()))
                             && !gattCustomService))) {
                    QByteArray descriptor;
                    descriptor.append((char)0x02);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
                        notificationSubscribed++;
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration).uuid()
                                 
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("indication subscribed!");
                }
            }
        }
    }

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !this->hasVirtualDevice()
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {

        QSettings settings;
        bool virtual_device_enabled =
            settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
        bool virtual_device_force_bike =
            settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                .toBool();
        if (virtual_device_enabled) {
            if (!virtual_device_force_bike) {
                debug("creating virtual treadmill interface...");
                auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &horizontreadmill::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &horizontreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
            } else {
                debug("creating virtual bike interface...");
                auto virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &horizontreadmill::changeInclinationRequested);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************
    }
}

void horizontreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < minInclination)
        percentage = minInclination;
    changeInclination(grade, percentage);
}

void horizontreadmill::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    if (notificationSubscribed)
        notificationSubscribed--;

    if (!notificationSubscribed) {
        initRequest = true;
        emit connectedAndDiscovered();
    }
}

void horizontreadmill::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');
}

void horizontreadmill::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void horizontreadmill::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');
    QBluetoothUuid _gattInclinationSupported((quint16)0x2AD5);
    if(characteristic.uuid() == _gattInclinationSupported && newValue.length() > 2) {
        minInclination = ((double)(
             (int16_t)(
                 ((int16_t)(int8_t)newValue.at(1) << 8) |
                 (uint8_t)newValue.at(0)
                 )
             ) /
         10.0);
        qDebug() << "new minInclination is " << minInclination;
    }
}

void horizontreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    firstStateChanged = 0;
    auto services_list = m_control->services();

    for (const QBluetoothUuid &s : qAsConst(services_list)) {
       if (s == QBluetoothUuid(static_cast<quint16>(0x1826)) || 
           s == QBluetoothUuid(static_cast<quint16>(0xFFF0)) || 
            s == QBluetoothUuid(QBluetoothUuid::ServiceClassUuid::RunningSpeedAndCadence)) {
            qDebug() << s << "discovering...";
            gattCommunicationChannelService.append(m_control->createServiceObject(s));
            connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                    &horizontreadmill::stateChanged);
            gattCommunicationChannelService.constLast()->discoverDetails(QLowEnergyService::SkipValueDiscovery);
        } else {
           qDebug() << "skipping service " << s;
       }
    }    
}

void horizontreadmill::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("horizontreadmill::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void horizontreadmill::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("horizontreadmill::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void horizontreadmill::deviceDiscovered(const QBluetoothDeviceInfo &device) {

    // ***************************************************************************************************************
    // horizon treadmill and F80 treadmill, so if we want to add inclination support we have to separate the 2
    // devices
    // ***************************************************************************************************************

    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        QSettings settings;
        bluetoothDevice = device;

        if (device.name().toUpper().startsWith(QStringLiteral("MOBVOI TMP"))) {
            mobvoi_tmp_treadmill = true;
            qDebug() << QStringLiteral("MOBVOI TMP workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("MOBVOI TM"))) {
            mobvoi_treadmill = true;
            qDebug() << QStringLiteral("MOBVOI TM workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("KETTLER TREADMILL"))) {
            kettler_treadmill = true;
            qDebug() << QStringLiteral("KETTLER TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("WELLFIT TM"))) {
            wellfit_treadmill = true;
            qDebug() << QStringLiteral("WELLFIT TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("ANPLUS-"))) {
            anplus_treadmill = true;
            qDebug() << QStringLiteral("ANPLUS TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("TUNTURI T60-")) ||
                   device.name().toUpper().startsWith(QStringLiteral("TUNTURI T90-"))) {
            tunturi_t60_treadmill = true;
            qDebug() << QStringLiteral("TUNTURI T60 TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("F85"))) {
            sole_f85_treadmill = true;
            settings.setValue(QZSettings::treadmill_step_incline, 1.0); // this treadmill doesn't handle 0.5 inclination
            minInclination = -5.0;
            qDebug() << QStringLiteral("SOLE F85 TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("F89"))) {
            sole_f89_treadmill = true;
            minInclination = -5.0;
            qDebug() << QStringLiteral("SOLE F89 TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("TT8"))) {
            sole_tt8_treadmill = true;
            minInclination = -6.0;
            qDebug() << QStringLiteral("SOLE TT8 TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("S77"))) {
            sole_s77_treadmill = true;
            qDebug() << QStringLiteral("SOLE S77 TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("SCHWINN 810"))) {
            schwinn_810_treadmill = true;
            qDebug() << QStringLiteral("Schwinn 810 TREADMILL workaround ON!");
        } else if (device.name().toUpper().startsWith(QStringLiteral("TREADMILL"))) { // Technogym Run
            technogymrun = true;
            qDebug() << QStringLiteral("Technogym Run TREADMILL workaround ON!");
        } else if(device.name().toUpper().startsWith(QStringLiteral("SW"))) {
            qDebug() << QStringLiteral("SW TREADMILL workaround ON!");
            SW_TREADMILL = true;
            disableAutoPause = true;
        } else if(device.name().toUpper().startsWith("HORIZON_7.8AT")) {
            HORIZON_78AT_treadmill = true;
            qDebug() << QStringLiteral("HORIZON_7.8AT workaround ON!");
        } else if(device.name().toUpper().startsWith("T01_")) {
            ICONCEPT_FTMS_treadmill = true;
            qDebug() << QStringLiteral("ICONCEPT_FTMS_treadmill workaround ON!");
        } else if ((device.name().toUpper().startsWith("DOMYOS"))) {
            qDebug() << QStringLiteral("DOMYOS found");
            DOMYOS = true;
        } else if ((device.name().toUpper().startsWith(QStringLiteral("BFX_T9_")))) {
            qDebug() << QStringLiteral("BOWFLEX T9 found");
            BOWFLEX_T9 = true;
        } else if ((device.name().toUpper().startsWith(QStringLiteral("YPOO-MINI PRO-")))) {
            qDebug() << QStringLiteral("YPOO-MINI PRO found");
            YPOO_MINI_PRO = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("MX-TM "))) {
            qDebug() << QStringLiteral("MX-TM found");
            MX_TM = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("FIT-"))) {
            qDebug() << QStringLiteral("FIT- found");
            FIT = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("3G PRO "))) {
            qDebug() << QStringLiteral("3G PRO");
            T3G_PRO = true;
        } else if (device.name().toUpper().startsWith(QStringLiteral("3G ELITE "))) {
            qDebug() << QStringLiteral("3G ELITE");
            T3G_ELITE = true;
        }

        if (device.name().toUpper().startsWith(QStringLiteral("TRX3500"))) {
            trx3500_treadmill = true;
            qDebug() << QStringLiteral("TRX3500 TREADMILL workaround ON!");            
#ifdef Q_OS_IOS            
            QZ_EnableDiscoveryCharsAndDescripttors = false;
#endif            
        } else if(device.name().toUpper().startsWith(QStringLiteral("TRX4500"))) {
            qDebug() << QStringLiteral("TRX4500 TREADMILL workaround ON!");
#ifdef Q_OS_IOS
            QZ_EnableDiscoveryCharsAndDescripttors = false;
#endif
        } else {
#ifdef Q_OS_IOS            
            QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif            
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &horizontreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &horizontreadmill::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &horizontreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &horizontreadmill::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    emit debug(QStringLiteral("Cannot connect to remote device."));
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("Controller connected. Search services..."));
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            emit debug(QStringLiteral("LowEnergy controller disconnected"));
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool horizontreadmill::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void horizontreadmill::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");

        initDone = false;
        m_control->connectToDevice();
    }
}

const int CRC_TABLE[256] = {
    0,     4129,  8258,  12387, 16516, 20645, 24774, 28903, 33032, 37161, 41290, 45419, 49548, 53677, 57806, 61935,
    4657,  528,   12915, 8786,  21173, 17044, 29431, 25302, 37689, 33560, 45947, 41818, 54205, 50076, 62463, 58334,
    9314,  13379, 1056,  5121,  25830, 29895, 17572, 21637, 42346, 46411, 34088, 38153, 58862, 62927, 50604, 54669,
    13907, 9842,  5649,  1584,  30423, 26358, 22165, 18100, 46939, 42874, 38681, 34616, 63455, 59390, 55197, 51132,
    18628, 22757, 26758, 30887, 2112,  6241,  10242, 14371, 51660, 55789, 59790, 63919, 35144, 39273, 43274, 47403,
    23285, 19156, 31415, 27286, 6769,  2640,  14899, 10770, 56317, 52188, 64447, 60318, 39801, 35672, 47931, 43802,
    27814, 31879, 19684, 23749, 11298, 15363, 3168,  7233,  60846, 64911, 52716, 56781, 44330, 48395, 36200, 40265,
    32407, 28342, 24277, 20212, 15891, 11826, 7761,  3696,  65439, 61374, 57309, 53244, 48923, 44858, 40793, 36728,
    37256, 33193, 45514, 41451, 53516, 49453, 61774, 57711, 4224,  161,   12482, 8419,  20484, 16421, 28742, 24679,
    33721, 37784, 41979, 46042, 49981, 54044, 58239, 62302, 689,   4752,  8947,  13010, 16949, 21012, 25207, 29270,
    46570, 42443, 38312, 34185, 62830, 58703, 54572, 50445, 13538, 9411,  5280,  1153,  29798, 25671, 21540, 17413,
    42971, 47098, 34713, 38840, 59231, 63358, 50973, 55100, 9939,  14066, 1681,  5808,  26199, 30326, 17941, 22068,
    55628, 51565, 63758, 59695, 39368, 35305, 47498, 43435, 22596, 18533, 30726, 26663, 6336,  2273,  14466, 10403,
    52093, 56156, 60223, 64286, 35833, 39896, 43963, 48026, 19061, 23124, 27191, 31254, 2801,  6864,  10931, 14994,
    64814, 60687, 56684, 52557, 48554, 44427, 40424, 36297, 31782, 27655, 23652, 19525, 15522, 11395, 7392,  3265,
    61215, 65342, 53085, 57212, 44955, 49082, 36825, 40952, 28183, 32310, 20053, 24180, 11923, 16050, 3793,  7920,
};

// https://crccalc.com/
int horizontreadmill::GenerateCRC_CCITT(uint8_t *PUPtr8, int PU16_Count, int crcStart) {
    if (PU16_Count == 0) {
        return 0;
    }
    int crc = crcStart;
    for (int i = 0; i < PU16_Count; i++) {
        int c = CRC_TABLE[((crc & 65280) >> 8) ^ ((PUPtr8[i] & 255) & 255)];
        crc = ((crc << 8) & 65280) ^ c;
    }
    return crc;
}

bool horizontreadmill::autoPauseWhenSpeedIsZero() {
    if(disableAutoPause == true)
        return false;
    if (lastStart == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStart + 10000))
        return true;
    else
        return false;
}

bool horizontreadmill::autoStartWhenSpeedIsGreaterThenZero() {
    QSettings settings;
    bool horizon_treadmill_7_8 =
        settings.value(QZSettings::horizon_treadmill_7_8, QZSettings::default_horizon_treadmill_7_8).toBool();
    bool horizon_paragon_x =
        settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();

    // the horizon starts with a strange speed, since that i can auto start (maybe the best way to solve this
    // is to understand why it's starting with this strange speed)
    if (!horizon_paragon_x && !horizon_treadmill_7_8 && !mobvoi_tmp_treadmill)
        return false;

    if ((lastStop == 0 || QDateTime::currentMSecsSinceEpoch() > (lastStop + 25000)) && requestStop == -1)
        return true;
    else
        return false;
}

void horizontreadmill::updateProfileCRC() {
    int confirm = GenerateCRC_CCITT(&initData7[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9, sizeof(initData9), confirm);
    confirm = GenerateCRC_CCITT(initData10, sizeof(initData10), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7[8] = (confirm & 0xff);
    initData7[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_1[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_1, sizeof(initData9_1), confirm);
    confirm = GenerateCRC_CCITT(initData10_1, sizeof(initData10_1), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_1[8] = (confirm & 0xff);
    initData7_1[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_2[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_2, sizeof(initData9_2), confirm);
    confirm = GenerateCRC_CCITT(initData10_2, sizeof(initData10_2), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_2[8] = (confirm & 0xff);
    initData7_2[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_3[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_3, sizeof(initData9_3), confirm);
    confirm = GenerateCRC_CCITT(initData10_3, sizeof(initData10_3), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_3[8] = (confirm & 0xff);
    initData7_3[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_4[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_4, sizeof(initData9_4), confirm);
    confirm = GenerateCRC_CCITT(initData10_4, sizeof(initData10_4), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_4[8] = (confirm & 0xff);
    initData7_4[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_5[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_5, sizeof(initData9_5), confirm);
    confirm = GenerateCRC_CCITT(initData10_5, sizeof(initData10_5), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_5[8] = (confirm & 0xff);
    initData7_5[9] = (confirm >> 8);

    confirm = GenerateCRC_CCITT(&initData7_6[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_6, sizeof(initData9_6), confirm);
    confirm = GenerateCRC_CCITT(initData10_6, sizeof(initData10_6), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    initData7_6[8] = (confirm & 0xff);
    initData7_6[9] = (confirm >> 8);
}

void horizontreadmill::testProfileCRC() {
    int confirm = GenerateCRC_CCITT(&initData7[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9, sizeof(initData9), confirm);
    confirm = GenerateCRC_CCITT(initData10, sizeof(initData10), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7[8] == (confirm & 0xff));
    assert(initData7[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_1[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_1, sizeof(initData9_1), confirm);
    confirm = GenerateCRC_CCITT(initData10_1, sizeof(initData10_1), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_1[8] == (confirm & 0xff));
    assert(initData7_1[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_2[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_2, sizeof(initData9_2), confirm);
    confirm = GenerateCRC_CCITT(initData10_2, sizeof(initData10_2), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_2[8] == (confirm & 0xff));
    assert(initData7_2[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_3[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_3, sizeof(initData9_3), confirm);
    confirm = GenerateCRC_CCITT(initData10_3, sizeof(initData10_3), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_3[8] == (confirm & 0xff));
    assert(initData7_3[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_4[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_4, sizeof(initData9_4), confirm);
    confirm = GenerateCRC_CCITT(initData10_4, sizeof(initData10_4), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_4[8] == (confirm & 0xff));
    assert(initData7_4[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_5[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_5, sizeof(initData9_5), confirm);
    confirm = GenerateCRC_CCITT(initData10_5, sizeof(initData10_5), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_5[8] == (confirm & 0xff));
    assert(initData7_5[9] == (confirm >> 8));

    confirm = GenerateCRC_CCITT(&initData7_6[10], 10);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData9_6, sizeof(initData9_6), confirm);
    confirm = GenerateCRC_CCITT(initData10_6, sizeof(initData10_6), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData11, sizeof(initData11), confirm);
    confirm = GenerateCRC_CCITT(initData12, sizeof(initData12), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData8, sizeof(initData8), confirm);
    confirm = GenerateCRC_CCITT(initData13, sizeof(initData13), confirm);
    confirm = GenerateCRC_CCITT(initData14, sizeof(initData14), confirm);

    assert(initData7_6[8] == (confirm & 0xff));
    assert(initData7_6[9] == (confirm >> 8));
}

double horizontreadmill::minStepInclination() {
    QSettings settings;
    bool toorx_ftms_treadmill = settings.value(QZSettings::toorx_ftms_treadmill, QZSettings::default_toorx_ftms_treadmill).toBool();
    if (kettler_treadmill || trx3500_treadmill || toorx_ftms_treadmill || sole_tt8_treadmill || ICONCEPT_FTMS_treadmill || SW_TREADMILL || sole_s77_treadmill || FIT || T3G_PRO || T3G_ELITE)
        return 1.0;
    else
        return 0.5;
}

double horizontreadmill::minStepSpeed() { return 0.1; }
