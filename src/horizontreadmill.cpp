#include "horizontreadmill.h"

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

void horizontreadmill::writeCharacteristic(uint8_t *data, uint8_t data_len, QString info, bool disable_log,
                                           bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(this, &horizontreadmill::packetReceived, &loop, &QEventLoop::quit);
        timeout.singleShot(3000, &loop, SLOT(quit()));
    } else {
        connect(gattCustomService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic, QByteArray)), &loop,
                SLOT(quit()));
        timeout.singleShot(3000, &loop, SLOT(quit()));
    }

    gattCustomService->writeCharacteristic(gattWriteCharCustomService, QByteArray((const char *)data, data_len));

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
    uint8_t initData01[] = {0x55, 0xaa, 0x01, 0x00, 0x01, 0x02, 0x00, 0x00, 0x00, 0x00};

    uint8_t initData7[] = {0x55, 0xaa, 0x02, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xed, 0xc2,
                           0x00, 0x47, 0x75, 0x65, 0x73, 0x74, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData8[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                           0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x05, 0xc2, 0x07};
    uint8_t initData10[] = {0x01, 0x01, 0x00, 0xd3, 0x8a, 0x0c, 0x00, 0x01, 0x01, 0x02,
                            0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t initData11[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t initData12[] = {0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30,
                            0x30, 0x30, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData13[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30,
                            0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};
    uint8_t initData14[] = {0x30};

    uint8_t initData7_1[] = {0x55, 0xaa, 0x03, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xae, 0x2a,
                             0x01, 0x41, 0x69, 0x61, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_1[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x06, 0xc4, 0x07};
    uint8_t initData10_1[] = {0x09, 0x1c, 0x00, 0x9f, 0xef, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_2[] = {0x55, 0xaa, 0x04, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xae, 0x2a,
                             0x01, 0x41, 0x69, 0x61, 0x6e, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_2[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x06, 0xc4, 0x07};
    uint8_t initData10_2[] = {0x09, 0x1c, 0x00, 0x9f, 0xef, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_3[] = {0x55, 0xaa, 0x05, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xa9, 0xe7,
                             0x02, 0x4d, 0x65, 0x67, 0x68, 0x61, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_3[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa4, 0x06, 0xc5, 0x07};
    uint8_t initData10_3[] = {0x0b, 0x0f, 0x00, 0x4b, 0x40, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_4[] = {0x55, 0xaa, 0x06, 0x00, 0x01, 0x16, 0xdb, 0x02, 0xbc, 0x76,
                             0x03, 0x44, 0x61, 0x72, 0x70, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_4[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x08, 0x07, 0xca, 0x07};
    uint8_t initData10_4[] = {0x05, 0x1c, 0x00, 0x07, 0x25, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_5[] = {0x55, 0xaa, 0x07, 0x00, 0x01, 0x16, 0xdb, 0x02, 0x7d, 0xeb,
                             0x04, 0x41, 0x68, 0x6f, 0x6e, 0x61, 0x00, 0x00, 0x00, 0x00};
    uint8_t initData9_5[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xb0, 0x04, 0xcc, 0x07};
    uint8_t initData10_5[] = {0x01, 0x08, 0x00, 0xc2, 0x0f, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData7_6[] = {0x55, 0xaa, 0x08, 0x00, 0x01, 0x16, 0xdb, 0x02, 0x03, 0x0d,
                             0x05, 0x55, 0x73, 0x65, 0x72, 0x20, 0x35, 0x00, 0x00, 0x00};
    uint8_t initData9_6[] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xdc, 0x05, 0xc2, 0x07};
    uint8_t initData10_6[] = {0x01, 0x01, 0x00, 0x8e, 0x6a, 0x0c, 0x00, 0x01, 0x01, 0x02,
                              0x23, 0x00, 0x00, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30, 0x30};

    uint8_t initData02[] = {0x55, 0xaa, 0x09, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData03[] = {0x55, 0xaa, 0x0a, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData04[] = {0x55, 0xaa, 0x0b, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData05[] = {0x55, 0xaa, 0x0c, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};
    uint8_t initData06[] = {0x55, 0xaa, 0x0d, 0x00, 0x01, 0x00, 0x02, 0x00, 0xb7, 0xf1, 0x1a, 0x00};

    uint8_t initData2[] = {0x55, 0xaa, 0x0e, 0x00, 0x03, 0x14, 0x08, 0x00, 0x3f,
                           0x01, 0xe5, 0x07, 0x02, 0x08, 0x13, 0x12, 0x21, 0x00};
    uint8_t initData3[] = {0x55, 0xaa, 0x0f, 0x00, 0x03, 0x01, 0x01, 0x00, 0xd1, 0xf1, 0x01};
    uint8_t initData4[] = {0x55, 0xaa, 0x10, 0x00, 0x03, 0x10, 0x01, 0x00, 0xf0, 0xe1, 0x00};
    uint8_t initData5[] = {0x55, 0xaa, 0x11, 0x00, 0x03, 0x02, 0x11, 0x00, 0x84, 0xbe,
                           0x00, 0x00, 0x08, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x05};
    uint8_t initData6[] = {0x00, 0x00, 0x00, 0x01, 0x00, 0x00, 0x01};

    if (gattCustomService) {
        writeCharacteristic(initData01, sizeof(initData01), QStringLiteral("init"), false, true);
        waitForAPacket();

        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData7_1, sizeof(initData7_1), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9_1, sizeof(initData9_1), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10_1, sizeof(initData10_1), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData7_2, sizeof(initData7_2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9_2, sizeof(initData9_2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10_2, sizeof(initData10_2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData7_3, sizeof(initData7_3), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9_3, sizeof(initData9_3), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10_3, sizeof(initData10_3), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData7_4, sizeof(initData7_4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9_4, sizeof(initData9_4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10_4, sizeof(initData10_4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData7_5, sizeof(initData7_5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9_5, sizeof(initData9_5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10_5, sizeof(initData10_5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData7_6, sizeof(initData7_6), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData9_6, sizeof(initData9_6), QStringLiteral("init"), false, false);
        writeCharacteristic(initData10_6, sizeof(initData10_6), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, true);

        writeCharacteristic(initData02, sizeof(initData02), QStringLiteral("init"), false, true);
        writeCharacteristic(initData03, sizeof(initData03), QStringLiteral("init"), false, true);
        writeCharacteristic(initData04, sizeof(initData04), QStringLiteral("init"), false, true);
        writeCharacteristic(initData05, sizeof(initData05), QStringLiteral("init"), false, true);
        writeCharacteristic(initData06, sizeof(initData06), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);

        messageID = 0x11;
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
        update_metrics(true, watts(settings.value(QStringLiteral("weight"), 75.0).toFloat()));

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestSpeed != -1) {
            if (requestSpeed != currentSpeed().value() && requestSpeed >= 0 && requestSpeed <= 22) {
                emit debug(QStringLiteral("writing speed ") + QString::number(requestSpeed));
                forceSpeed(requestSpeed);
            }
            requestSpeed = -1;
        }
        if (requestInclination != -1) {
            if (requestInclination != currentInclination().value() && requestInclination >= 0 &&
                requestInclination <= 15) {
                emit debug(QStringLiteral("writing incline ") + QString::number(requestInclination));
                forceIncline(requestInclination);
            }
            requestInclination = -1;
        }
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));
            if (lastSpeed == 0.0) {

                lastSpeed = 0.5;
            }
            requestStart = -1;
            emit tapeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));

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

// example frame: 55aa320003050400532c00150000
void horizontreadmill::forceSpeed(double requestSpeed) {
    messageID++;
    uint8_t datas[3];
    datas[0] = (uint8_t)(requestSpeed * 0.621371 * 10) & 0xff;
    datas[1] = (uint16_t)(requestSpeed * 0.621371 * 10) >> 8;
    datas[2] = 0;
    int confirm = GenerateCRC_CCITT(datas, 3);
    uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x05, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    write[2] = messageID & 0xff;
    write[3] = messageID >> 8;
    write[9] = confirm & 0xff;
    write[10] = confirm >> 8;
    write[11] = datas[0];
    write[12] = datas[1];
    write[13] = datas[2];

    writeCharacteristic(write, sizeof(write), QStringLiteral("forceSpeed"), false, true);
}

// example frame: 55aa3800030603005d0b0a0000
void horizontreadmill::forceIncline(double requestIncline) {
    messageID++;
    uint8_t datas[2];
    datas[0] = (uint8_t)(requestIncline)&0xff;
    datas[1] = (uint16_t)(requestIncline) >> 8;
    int confirm = GenerateCRC_CCITT(datas, 2);
    uint8_t write[] = {0x55, 0xaa, 0x00, 0x00, 0x03, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    write[2] = messageID & 0xff;
    write[3] = messageID >> 8;
    write[9] = confirm & 0xff;
    write[10] = confirm >> 8;
    write[11] = datas[0];
    write[12] = datas[1];

    writeCharacteristic(write, sizeof(write), QStringLiteral("forceIncline"), false, true);
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
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QStringLiteral("heart_rate_belt_name"), QStringLiteral("Disabled")).toString();

    emit debug(QStringLiteral(" << ") + characteristic.uuid().toString() + " " + QString::number(newValue.length()) +
               " " + newValue.toHex(' '));

    if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4)) {
        if (newValue.at(0) == 0x55) {
            customRecv = (((uint16_t)((uint8_t)newValue.at(7)) << 8) | (uint16_t)((uint8_t)newValue.at(6))) + 10;
            qDebug() << "new custom packet received. Len expected: " << customRecv;
        }

        customRecv -= newValue.length();
        if (customRecv <= 0) {
            qDebug() << "full custom packet received";
            customRecv = 0;
            emit packetReceived();
        }
    }

    if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() > 70 && newValue.at(0) == 0x55 &&
        newValue.at(5) == 0x12) {
        Speed =
            (((double)(((uint16_t)((uint8_t)newValue.at(62)) << 8) | (uint16_t)((uint8_t)newValue.at(61)))) / 1000.0) *
            1.60934; // miles/h
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        Inclination = (double)((uint8_t)newValue.at(63)) / 10.0;
        emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        KCal += ((((0.048 * ((double)watts(settings.value(QStringLiteral("weight"), 75.0).toFloat())) + 1.19) *
                   settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
    } else if (characteristic.uuid() == QBluetoothUuid((quint16)0xFFF4) && newValue.length() == 29 &&
               newValue.at(0) == 0x55) {
        Speed = ((double)(((uint16_t)((uint8_t)newValue.at(15)) << 8) | (uint16_t)((uint8_t)newValue.at(14)))) / 10.0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));

        // Inclination = (double)((uint8_t)newValue.at(3)) / 10.0;
        // emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));

        KCal += ((((0.048 * ((double)watts(settings.value(QStringLiteral("weight"), 75.0).toFloat())) + 1.19) *
                   settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
        // kg * 3.5) / 200 ) / 60

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

        Distance += ((Speed.value() / 3600000.0) *
                     ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
        emit debug(QStringLiteral("Current Distance: ") + QString::number(Distance.value()));
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
            Distance += ((Speed.value() / 3600000.0) *
                         ((double)lastRefreshCharacteristicChanged.msecsTo(QDateTime::currentDateTime())));
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
            KCal +=
                ((((0.048 * ((double)watts(settings.value(QStringLiteral("weight"), 75.0).toFloat())) + 1.19) *
                   settings.value(QStringLiteral("weight"), 75.0).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                QDateTime::currentDateTime())))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
        }

        emit debug(QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value("ant_heart", false).toBool())
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
    }

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        if (heart == 0.0 || settings.value(QStringLiteral("heart_ignore_builtin"), false).toBool()) {

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

    lastRefreshCharacteristicChanged = QDateTime::currentDateTime();

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void horizontreadmill::stateChanged(QLowEnergyService::ServiceState state) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    QBluetoothUuid _gattWriteCharCustomService((quint16)0xFFF3);
    QBluetoothUuid _gattWriteCharControlPointId((quint16)0x2AD9);
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
                    ((s->serviceUuid() == gattFTMSService->serviceUuid() && !gattCustomService) ||
                     (s->serviceUuid() == gattCustomService->serviceUuid() && gattCustomService))) {
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
    if (!firstStateChanged && !virtualTreadmill
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
        && !h
#endif
#endif
    ) {

        QSettings settings;
        bool virtual_device_enabled = settings.value(QStringLiteral("virtual_device_enabled"), true).toBool();
        if (virtual_device_enabled) {
            emit debug(QStringLiteral("creating virtual treadmill interface..."));

            virtualTreadmill = new virtualtreadmill(this, noHeartService);
            connect(virtualTreadmill, &virtualtreadmill::debug, this, &horizontreadmill::debug);
        }
    }
    firstStateChanged = 1;
    // ********************************************************************************************************
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
    // horizon treadmill and F80 treadmill, so if we want to add inclination support we have to separate the 2 devices
    // ***************************************************************************************************************
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

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
       4129,
       8258,
       12387,
       16516,
       20645,
       24774,
       28903,
       33032,
       37161,
       41290,
       45419,
       49548,
       53677,
       57806,
       61935,
       4657,
       528,
       12915,
       8786,
       21173,
       17044,
       29431,
       25302,
       37689,
       33560,
       45947,
       41818,
       54205,
       50076,
       62463,
       58334,
       9314,
       13379,
       1056,
       5121,
       25830,
       29895,
       17572,
       21637,
       42346,
       46411,
       34088,
       38153,
       58862,
       62927,
       50604,
       54669,
       13907,
       9842,
       5649,
       1584,
       30423,
       26358,
       22165,
       18100,
       46939,
       42874,
       38681,
       34616,
       63455,
       59390,
       55197,
       51132,
       18628,
       22757,
       26758,
       30887,
       2112,
       6241,
       10242,
       14371,
       51660,
       55789,
       59790,
       63919,
       35144,
       39273,
       43274,
       47403,
       23285,
       19156,
       31415,
       27286,
       6769,
       2640,
       14899,
       10770,
       56317,
       52188,
       64447,
       60318,
       39801,
       35672,
       47931,
       43802,
       27814,
       31879,
       19684,
       23749,
       11298,
       15363,
       3168,
       7233,
       60846,
       64911,
       52716,
       56781,
       44330,
       48395,
       36200,
       40265,
       32407,
       28342,
       24277,
       20212,
       15891,
       11826,
       7761,
       3696,
       65439,
       61374,
       57309,
       53244,
       48923,
       44858,
       40793,
       36728,
       37256,
       33193,
       45514,
       41451,
       53516,
       49453,
       61774,
       57711,
       4224,
       161,
       12482,
       8419,
       20484,
       16421,
       28742,
       24679,
       33721,
       37784,
       41979,
       46042,
       49981,
       54044,
       58239,
       62302,
       689,
       4752,
       8947,
       13010,
       16949,
       21012,
       25207,
       29270,
       46570,
       42443,
       38312,
       34185,
       62830,
       58703,
       54572,
       50445,
       13538,
       9411,
       5280,
       1153,
       29798,
       25671,
       21540,
       17413,
       42971,
       47098,
       34713,
       38840,
       59231,
       63358,
       50973,
       55100,
       9939,
       14066,
       1681,
       5808,
       26199,
       30326,
       17941,
       22068,
       55628,
       51565,
       63758,
       59695,
       39368,
       35305,
       47498,
       43435,
       22596,
       18533,
       30726,
       26663,
       6336,
       2273,
       14466,
       10403,
       52093,
       56156,
       60223,
       64286,
       35833,
       39896,
       43963,
       48026,
       19061,
       23124,
       27191,
       31254,
       2801,
       6864,
       10931,
       14994,
       64814,
       60687,
       56684,
       52557,
       48554,
       44427,
       40424,
       36297,
       31782,
       27655,
       23652,
       19525,
       15522,
       11395,
       7392,
       3265,
       61215,
       65342,
       53085,
       57212,
       44955,
       49082,
       36825,
       40952,
       28183,
       32310,
       20053,
       24180,
       11923,
       16050,
       3793,
       7920,
   };

int horizontreadmill::GenerateCRC_CCITT(uint8_t* PUPtr8, int PU16_Count) {
    if (PU16_Count == 0) {
        return 0;
    }
    int crc = 65535;
    for (int i = 0; i < PU16_Count; i++) {
        crc = ((crc << 8) & 65280) ^ CRC_TABLE[((crc & 65280) >> 8) ^ ((PUPtr8[i] & 255) & 255)];
    }
    return crc;
}
