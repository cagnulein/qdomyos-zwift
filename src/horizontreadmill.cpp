#include "horizontreadmill.h"

#include "ftmsbike.h"
#include "ios/lockscreen.h"
#include "virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>

#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include <QLowEnergyConnectionParameters>
#endif
#include "keepawakehelper.h"
#include <chrono>

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

horizontreadmill::horizontreadmill(bool noWriteResistance, bool noHeartService) {

    testProfileCRC();

#ifdef Q_OS_IOS
    QZ_EnableDiscoveryCharsAndDescripttors = true;
#endif

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

    if (wait_for_response) {
        connect(this, &horizontreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(6000, &loop, SLOT(quit())); // 6 seconds are important
    } else {
        connect(service, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop, SLOT(quit()));
        timeout.singleShot(3000, &loop, SLOT(quit()));
    }

    service->writeCharacteristic(characteristic, QByteArray((const char *)data, data_len));

    if (!disable_log)
        qDebug() << " >> " << QByteArray((const char *)data, data_len).toHex(' ') << " // " << info;

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

            if(!initPacketRecv) {
                qDebug() << "init 1 not received";
                waitForAPacket();
                goto init1;
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

            if(!initPacketRecv) {
                qDebug() << "init 2 not received";
                waitForAPacket();
                goto init2;
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

            if(!initPacketRecv) {
                qDebug() << "init 3 not received";
                waitForAPacket();
                goto init3;
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

            if(!initPacketRecv) {
                qDebug() << "init 4 not received";
                waitForAPacket();
                goto init4;
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

            if(!initPacketRecv) {
                qDebug() << "init 5 not received";
                waitForAPacket();
                goto init5;
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

            if(!initPacketRecv) {
                qDebug() << "init 6 not received";
                waitForAPacket();
                goto init6;
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

            if(!initPacketRecv) {
                qDebug() << "init 7 not received";
                waitForAPacket();
                goto init7;
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

            if(!initPacketRecv) {
                qDebug() << "init 8 not received";
                waitForAPacket();
                goto init8;
            }
        }
        messageID = 0x10;
    }

    initDone = true;
}

void horizontreadmill::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {

        emit disconnected();
        return;
    }

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
        update_metrics(true, watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestSpeed != -1) {
            qDebug() << "requestSpeed=" << requestSpeed;
            if (requestSpeed != currentSpeed().value() &&
                fabs(requestSpeed - currentSpeed().value()) > minStepSpeed() && requestSpeed >= 0 &&
                requestSpeed <= 22 && checkIfForceSpeedNeeding(requestSpeed)) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -100) {
            qDebug() << "requestInclination=" << requestInclination;
            if (requestInclination < 0)
                requestInclination = 0;
            else if(((int)requestInclination) != requestInclination) { // it has decimal
                // the treadmill accepts only .5 steps
                requestInclination = floor(requestInclination) + 0.5;
            }
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceIncline(requestInclination);
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
                            if(!settings.value(QZSettings::horizon_treadmill_disable_pause, QZSettings::default_horizon_treadmill_disable_pause).toBool()) {
                                messageID++;
                                uint8_t write1[] = {0x55, 0xaa, 0x13, 0x00, 0x01, 0x14, 0x00, 0x00, 0x00, 0x00};
                                write1[2] = messageID & 0xff;
                                write1[3] = messageID >> 8;

                                writeCharacteristic(gattCustomService, gattWriteCharCustomService, write1, sizeof(write1),
                                                    QStringLiteral("requestStop"), false, true);
                            }
                            // pause
                        } else {
                            requestPause = -1;
                            Speed = 0; // forcing the speed to be sure, maybe I could remove this
                            if(!settings.value(QZSettings::horizon_treadmill_disable_pause, QZSettings::default_horizon_treadmill_disable_pause).toBool()) {
                                messageID++;
                                uint8_t write1[] = {0x55, 0xaa, 0x12, 0x00, 0x03, 0x03, 0x01, 0x00, 0xf0, 0xe1, 0x00};
                                write1[2] = messageID & 0xff;
                                write1[3] = messageID >> 8;

                                writeCharacteristic(gattCustomService, gattWriteCharCustomService, write1, sizeof(write1),
                                                    QStringLiteral("requestPause"), false, false);
                                horizonPaused = true;
                            }
                        }
                    }
                } else {
                    uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x03, 0x00, 0x00, 0x00, 0x00, 0x0d, 0x0a};
                    writeCharacteristic(gattCustomService, gattWriteCharCustomService, write, sizeof(write),
                                        QStringLiteral("stopping"), false, true);
                }
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
    bool horizon_paragon_x =
        settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();

    if (gattCustomService) {
        if (!horizon_paragon_x) {
            messageID++;
            uint8_t datas[4];
            datas[0] = 0;
            datas[1] = (uint8_t)(requestSpeed * 0.621371 * 10) & 0xff;
            datas[2] = (uint16_t)(requestSpeed * 0.621371 * 10) >> 8;
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
            datas[0] = (uint8_t)(requestSpeed * miles_conversion * 10) & 0xff;
            datas[1] = (uint16_t)(requestSpeed * miles_conversion * 10) >> 8;
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
        uint8_t write[] = {FTMS_REQUEST_CONTROL};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl", false,
                            true);
        write[0] = {FTMS_START_RESUME};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "start simulation",
                            false, true);

        uint8_t writeS[] = {FTMS_SET_TARGET_SPEED, 0x00, 0x00};
        writeS[1] = ((uint16_t)requestSpeed * 100) & 0xFF;
        writeS[2] = ((uint16_t)requestSpeed * 100) >> 8;

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("forceSpeed"), false, true);
    }
}

// example frame: 55aa3800030603005d0b0a0000
void horizontreadmill::forceIncline(double requestIncline) {
    QSettings settings;
    bool horizon_paragon_x =
        settings.value(QZSettings::horizon_paragon_x, QZSettings::default_horizon_paragon_x).toBool();

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
        uint8_t write[] = {FTMS_REQUEST_CONTROL};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "requestControl", false,
                            true);
        write[0] = {FTMS_START_RESUME};
        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, write, sizeof(write), "start simulation",
                            false, true);

        uint8_t writeS[] = {FTMS_SET_TARGET_INCLINATION, 0x00, 0x00};
        writeS[1] = ((int16_t)requestIncline * 10) & 0xFF;
        writeS[2] = ((int16_t)requestIncline * 10) >> 8;

        writeCharacteristic(gattFTMSService, gattWriteCharControlPointId, writeS, sizeof(writeS),
                            QStringLiteral("forceIncline"), false, true);
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

    if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && lastPacketComplete.length() > 70 &&
        lastPacketComplete.at(0) == 0x55 && lastPacketComplete.at(5) == 0x17) {
        Speed = (((double)(((uint16_t)((uint8_t)lastPacketComplete.at(25)) << 8) |
                           (uint16_t)((uint8_t)lastPacketComplete.at(24)))) /
                 100.0) *
                1.60934; // miles/h
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        Inclination = (double)((uint8_t)lastPacketComplete.at(30)) / 10.0;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        if (firstDistanceCalculated && watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (firstDistanceCalculated)
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        distanceEval = true;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() > 70 &&
               newValue.at(0) == 0x55 && newValue.at(5) == 0x12) {
        Speed =
            (((double)(((uint16_t)((uint8_t)newValue.at(62)) << 8) | (uint16_t)((uint8_t)newValue.at(61)))) / 1000.0) *
            1.60934; // miles/h
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        Inclination = (double)((uint8_t)newValue.at(63)) / 10.0;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        if (firstDistanceCalculated && watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (firstDistanceCalculated)
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        distanceEval = true;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() == 29 &&
               newValue.at(0) == 0x55) {
        Speed = ((double)(((uint16_t)((uint8_t)newValue.at(15)) << 8) | (uint16_t)((uint8_t)newValue.at(14)))) / 10.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        // Inclination = (double)((uint8_t)newValue.at(3)) / 10.0;
        // emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
        if (firstDistanceCalculated && watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat()))
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QZSettings::weight, QZSettings::default_weight).toFloat())) +
                    1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
        // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        if (firstDistanceCalculated)
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
        distanceEval = true;
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && (uint8_t)newValue.at(0) == 0x55 &&
               (uint8_t)newValue.at(1) == 0xAA && (uint8_t)newValue.at(2) == 0x00 && (uint8_t)newValue.at(3) == 0x00 &&
               (uint8_t)newValue.at(4) == 0x03 && (uint8_t)newValue.at(5) == 0x03 && (uint8_t)newValue.at(6) == 0x01 &&
               (uint8_t)newValue.at(7) == 0x00 && (uint8_t)newValue.at(8) == 0xf0 && (uint8_t)newValue.at(9) == 0xe1 &&
               (uint8_t)newValue.at(10) == 0x00) {

        Speed = 0;
        horizonPaused = true;
        qDebug() << "stop from the treadmill";
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0x2ACD)) {
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

        if (Flags.totalDistance) {
            // ignoring the distance, because it's a total life odometer
            // Distance = ((double)((((uint32_t)((uint8_t)newValue.at(index + 2)) << 16) |
            // (uint32_t)((uint8_t)newValue.at(index + 1)) << 8) | (uint32_t)((uint8_t)newValue.at(index)))) / 1000.0;
            index += 3;
        }
        // else
        {
            if (firstDistanceCalculated)
                Distance += ((Speed.value() / 3600000.0) *
                             ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));

        if (Flags.inclination) {
            Inclination = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          10.0;
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
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
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
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate) {
                if (index < newValue.length()) {

                    heart = ((double)((newValue.at(index))));
                    emit debug(QStringLiteral("Current Heart: ") + QString::number(heart));
                } else {
                    emit debug(QStringLiteral("Error on parsing heart!"));
                }
                // index += 1; //NOTE: clang-analyzer-deadcode.DeadStores
            }
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
            Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                              (uint16_t)((uint8_t)newValue.at(index)))) /
                    100.0;
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
                             ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
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
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
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
            KCal = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                             (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;

            // energy per hour
            index += 2;

            // energy per minute
            index += 1;
        } else {
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
                          QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                            // kg * 3.5) / 200 ) / 60
            distanceEval = true;
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)((newValue.at(index))));
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
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 ||
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool()) {

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            lockscreen h;
            long appleWatchHeartRate = h.heartRate();
            h.setKcal(KCal.value());
            h.setDistance(Distance.value());
            Heart = appleWatchHeartRate;
            debug("Current Heart from Apple Watch: " + QString::number(appleWatchHeartRate));
#endif
#endif
        } else {

            Heart = heart;
        }
    }

    if (Speed.value() > 0)
        lastStart = 0;
    else
        lastStop = 0;

    if (distanceEval) {
        firstDistanceCalculated = true;
        lastRefreshCharacteristicChanged = QDateTime::currentDateTime();
    }

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void horizontreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    QBluetoothUuid _gattWriteCharCustomService((quint16)0xFFF3);
    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
    QBluetoothUuid _gattTreadmillDataId((quint16)0x2ACD);
    QBluetoothUuid _gattCrossTrainerDataId((quint16)0x2ACE);
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        qDebug() << QStringLiteral("stateChanged") << s->serviceUuid() << s->state();
        if (s->state() != QLowEnergyService::ServiceDiscovered && s->state() != QLowEnergyService::InvalidService) {
            qDebug() << QStringLiteral("not all services discovered");
            return;
        }
    }

    qDebug() << QStringLiteral("all services discovered!");

    notificationSubscribed = 0;

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            // establish hook into notifications
            connect(s, &QLowEnergyService::characteristicChanged, this, &horizontreadmill::characteristicChanged);
            connect(s, &QLowEnergyService::characteristicWritten, this, &horizontreadmill::characteristicWritten);
            connect(s, &QLowEnergyService::characteristicRead, this, &horizontreadmill::characteristicRead);
            connect(
                s, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &horizontreadmill::errorService);
            connect(s, &QLowEnergyService::descriptorWritten, this, &horizontreadmill::descriptorWritten);
            connect(s, &QLowEnergyService::descriptorRead, this, &horizontreadmill::descriptorRead);

            qDebug() << s->serviceUuid() << QStringLiteral("connected!");

            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                qDebug() << QStringLiteral("char uuid") << c.uuid() << QStringLiteral("handle") << c.handle();

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharControlPointId) {
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
                }

                if (c.properties() & QLowEnergyCharacteristic::Write && c.uuid() == _gattWriteCharCustomService) {
                    qDebug() << QStringLiteral("Custom service and Control Point found");
                    gattWriteCharCustomService = c;
                    gattCustomService = s;
                }
            }
        }
    }

    for (QLowEnergyService *s : qAsConst(gattCommunicationChannelService)) {
        if (s->state() == QLowEnergyService::ServiceDiscovered) {
            auto characteristics_list = s->characteristics();
            for (const QLowEnergyCharacteristic &c : qAsConst(characteristics_list)) {
                auto descriptors_list = c.descriptors();
                for (const QLowEnergyDescriptor &d : qAsConst(descriptors_list)) {
                    qDebug() << QStringLiteral("descriptor uuid") << d.uuid() << QStringLiteral("handle") << d.handle();
                }

                if ((c.properties() & QLowEnergyCharacteristic::Notify) == QLowEnergyCharacteristic::Notify &&
                    ((gattFTMSService && s->serviceUuid() == gattFTMSService->serviceUuid() && !gattCustomService) ||
                     (gattCustomService && s->serviceUuid() == gattCustomService->serviceUuid()))) {
                    QByteArray descriptor;
                    descriptor.append((char)0x01);
                    descriptor.append((char)0x00);
                    if (c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).isValid()) {
                        s->writeDescriptor(c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
                        notificationSubscribed++;
                    } else {
                        qDebug() << QStringLiteral("ClientCharacteristicConfiguration") << c.uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).uuid()
                                 << c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration).handle()
                                 << QStringLiteral(" is not valid");
                    }

                    qDebug() << s->serviceUuid() << c.uuid() << QStringLiteral("notification subscribed!");
                }
            }
        }
    }

    // ******************************************* virtual treadmill init *************************************
    if (!firstStateChanged && !virtualTreadmill && !virtualBike
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
                virtualTreadmill = new virtualtreadmill(this, noHeartService);
                connect(virtualTreadmill, &virtualtreadmill::debug, this, &horizontreadmill::debug);
                connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                        &horizontreadmill::changeInclinationRequested);
            } else {
                debug("creating virtual bike interface...");
                virtualBike = new virtualbike(this);
                connect(virtualBike, &virtualbike::changeInclination, this,
                        &horizontreadmill::changeInclinationRequested);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************
    }
}

void horizontreadmill::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
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
}

void horizontreadmill::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    initRequest = false;
    firstStateChanged = 0;
    auto services_list = m_control->services();
    QBluetoothUuid ftmsService((quint16)0x1826);
    for (const QBluetoothUuid &s : qAsConst(services_list)) {
        gattCommunicationChannelService.append(m_control->createServiceObject(s));
        connect(gattCommunicationChannelService.constLast(), &QLowEnergyService::stateChanged, this,
                &horizontreadmill::stateChanged);
        gattCommunicationChannelService.constLast()->discoverDetails();
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
        bluetoothDevice = device;

        if(device.name().toUpper().startsWith(QStringLiteral("MOBVOI TM"))) {
            mobvoi_treadmill = true;
            qDebug() << QStringLiteral("MOBVOI TM workaround ON!");
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &horizontreadmill::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &horizontreadmill::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &horizontreadmill::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &horizontreadmill::controllerStateChanged);

        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
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

void *horizontreadmill::VirtualTreadmill() { return virtualTreadmill; }

void *horizontreadmill::VirtualDevice() { return VirtualTreadmill(); }

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
    if (!horizon_paragon_x && !horizon_treadmill_7_8)
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

double horizontreadmill::minStepInclination() { return 0.5; }
double horizontreadmill::minStepSpeed() { return 0.1; }
