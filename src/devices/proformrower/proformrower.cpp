#include "proformrower.h"
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include "virtualdevices/virtualrower.h"
#include "virtualdevices/virtualtreadmill.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

proformrower::proformrower(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformrower::update);
    refresh->start(200ms);
}

void proformrower::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                       bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void proformrower::forceResistance(resistance_t requestResistance) {
    QSettings settings;
    bool proform_rower_750r =
        settings.value(QZSettings::proform_rower_750r, QZSettings::default_proform_rower_750r).toBool();
    bool proform_rower_sport_rl =
        settings.value(QZSettings::proform_rower_sport_rl, QZSettings::default_proform_rower_sport_rl).toBool();

    if (proform_rower_750r) {
        const uint8_t unlock_res[] = {0xfe, 0x02, 0x0d, 0x02};
        static const uint8_t res_table[][20] = {
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x9f, 0x01, 0x00, 0xc4, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x3f, 0x03, 0x00, 0x66, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xdf, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x7f, 0x06, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x1f, 0x08, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xbf, 0x09, 0x00, 0xec, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x5f, 0x0b, 0x00, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xff, 0x0c, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x9f, 0x0e, 0x00, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x3f, 0x10, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xdf, 0x11, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x7f, 0x13, 0x00, 0xb6, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x1f, 0x15, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xbf, 0x16, 0x00, 0xf9, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x5f, 0x18, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xff, 0x19, 0x00, 0x3c, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x9f, 0x1b, 0x00, 0xde, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x3f, 0x1d, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xdf, 0x1e, 0x00, 0x21, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x7f, 0x20, 0x00, 0xc3, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x1f, 0x22, 0x00, 0x65, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xbf, 0x23, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0x5f, 0x25, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00},
            {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01, 0x04, 0xff, 0x26, 0x00, 0x49, 0x00, 0x00, 0x00, 0x00, 0x00}
        };
        constexpr size_t table_len = sizeof(res_table) / sizeof(res_table[0]);
        if (requestResistance >= 1 && static_cast<size_t>(requestResistance) <= table_len) {
            writeCharacteristic(const_cast<uint8_t *>(unlock_res), sizeof(unlock_res),
                                QStringLiteral("unlock_resistance"), false, false);
            writeCharacteristic(const_cast<uint8_t *>(res_table[requestResistance - 1]), sizeof(res_table[0]),
                                QStringLiteral("resistance750r"), false, true);
        }
        return;
    }

    if (proform_rower_sport_rl) {
        const uint8_t unlock_res[] = {0xfe, 0x02, 0x0d, 0x02};

        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x76, 0x01, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x17, 0x03, 0x00, 0x3e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0xb8, 0x04, 0x00, 0xe0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x58, 0x06, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0xf9, 0x07, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x9a, 0x09, 0x00, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x3a, 0x0b, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0xdb, 0x0c, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x7c, 0x0e, 0x00, 0xae, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x1c, 0x10, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xbd, 0x11, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x5e, 0x13, 0x00, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xfe, 0x14, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x9f, 0x16, 0x00, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x40, 0x18, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xe0, 0x19, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x81, 0x1b, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x22, 0x1d, 0x00, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xc2, 0x1e, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x63, 0x20, 0x00, 0xa7, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res21[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x04, 0x22, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xa4, 0x23, 0x00, 0xeb, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res23[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x45, 0x25, 0x00, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res24[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xe6, 0x26, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic((uint8_t*)unlock_res, sizeof(unlock_res), QStringLiteral("unlock_resistance"), false, false);

        switch (requestResistance) {
        case 1:
            writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, true);
            break;
        case 2:
            writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, true);
            break;
        case 3:
            writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
            break;
        case 4:
            writeCharacteristic((uint8_t *)res4, sizeof(res4), QStringLiteral("resistance4"), false, true);
            break;
        case 5:
            writeCharacteristic((uint8_t *)res5, sizeof(res5), QStringLiteral("resistance5"), false, true);
            break;
        case 6:
            writeCharacteristic((uint8_t *)res6, sizeof(res6), QStringLiteral("resistance6"), false, true);
            break;
        case 7:
            writeCharacteristic((uint8_t *)res7, sizeof(res7), QStringLiteral("resistance7"), false, true);
            break;
        case 8:
            writeCharacteristic((uint8_t *)res8, sizeof(res8), QStringLiteral("resistance8"), false, true);
            break;
        case 9:
            writeCharacteristic((uint8_t *)res9, sizeof(res9), QStringLiteral("resistance9"), false, true);
            break;
        case 10:
            writeCharacteristic((uint8_t *)res10, sizeof(res10), QStringLiteral("resistance10"), false, true);
            break;
        case 11:
            writeCharacteristic((uint8_t *)res11, sizeof(res11), QStringLiteral("resistance11"), false, true);
            break;
        case 12:
            writeCharacteristic((uint8_t *)res12, sizeof(res12), QStringLiteral("resistance12"), false, true);
            break;
        case 13:
            writeCharacteristic((uint8_t *)res13, sizeof(res13), QStringLiteral("resistance13"), false, true);
            break;
        case 14:
            writeCharacteristic((uint8_t *)res14, sizeof(res14), QStringLiteral("resistance14"), false, true);
            break;
        case 15:
            writeCharacteristic((uint8_t *)res15, sizeof(res15), QStringLiteral("resistance15"), false, true);
            break;
        case 16:
            writeCharacteristic((uint8_t *)res16, sizeof(res16), QStringLiteral("resistance16"), false, true);
            break;
        case 17:
            writeCharacteristic((uint8_t *)res17, sizeof(res17), QStringLiteral("resistance17"), false, true);
            break;
        case 18:
            writeCharacteristic((uint8_t *)res18, sizeof(res18), QStringLiteral("resistance18"), false, true);
            break;
        case 19:
            writeCharacteristic((uint8_t *)res19, sizeof(res19), QStringLiteral("resistance19"), false, true);
            break;
        case 20:
            writeCharacteristic((uint8_t *)res20, sizeof(res20), QStringLiteral("resistance20"), false, true);
            break;
        case 21:
            writeCharacteristic((uint8_t *)res21, sizeof(res21), QStringLiteral("resistance21"), false, true);
            break;
        case 22:
            writeCharacteristic((uint8_t *)res22, sizeof(res22), QStringLiteral("resistance22"), false, true);
            break;
        case 23:
            writeCharacteristic((uint8_t *)res23, sizeof(res23), QStringLiteral("resistance23"), false, true);
            break;
        case 24:
            writeCharacteristic((uint8_t *)res24, sizeof(res24), QStringLiteral("resistance24"), false, true);
            break;
        }
    } else {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x02,
                                0x00, 0x10, 0x01, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x02,
                                0x00, 0x10, 0x03, 0x00, 0x34, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x02,
                                0x00, 0x10, 0x04, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x58, 0x06, 0x00, 0x82, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0xf9, 0x07, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x9a, 0x09, 0x00, 0xc7, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x3a, 0x0b, 0x00, 0x69, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0xdb, 0x0c, 0x00, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                0x04, 0x7c, 0x0e, 0x00, 0xae, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x1c, 0x10, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xbd, 0x11, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x5e, 0x13, 0x00, 0x95, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xfe, 0x14, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x9f, 0x16, 0x00, 0xd9, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x40, 0x18, 0x00, 0x7c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xe0, 0x19, 0x00, 0x1d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x81, 0x1b, 0x00, 0xc0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x22, 0x1d, 0x00, 0x63, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xc2, 0x1e, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x63, 0x20, 0x00, 0xa7, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res21[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x04, 0x22, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xa4, 0x23, 0x00, 0xeb, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res23[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0x45, 0x25, 0x00, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res24[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x14, 0x09, 0x02, 0x01,
                                 0x04, 0xe6, 0x26, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};

        switch (requestResistance) {
        case 1:
            writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, true);
            break;
        case 2:
            writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, true);
            break;
        case 3:
            writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
            break;
        case 4:
            writeCharacteristic((uint8_t *)res4, sizeof(res4), QStringLiteral("resistance4"), false, true);
            break;
        case 5:
            writeCharacteristic((uint8_t *)res5, sizeof(res5), QStringLiteral("resistance5"), false, true);
            break;
        case 6:
            writeCharacteristic((uint8_t *)res6, sizeof(res6), QStringLiteral("resistance6"), false, true);
            break;
        case 7:
            writeCharacteristic((uint8_t *)res7, sizeof(res7), QStringLiteral("resistance7"), false, true);
            break;
        case 8:
            writeCharacteristic((uint8_t *)res8, sizeof(res8), QStringLiteral("resistance8"), false, true);
            break;
        case 9:
            writeCharacteristic((uint8_t *)res9, sizeof(res9), QStringLiteral("resistance9"), false, true);
            break;
        case 10:
            writeCharacteristic((uint8_t *)res10, sizeof(res10), QStringLiteral("resistance10"), false, true);
            break;
        case 11:
            writeCharacteristic((uint8_t *)res11, sizeof(res11), QStringLiteral("resistance11"), false, true);
            break;
        case 12:
            writeCharacteristic((uint8_t *)res12, sizeof(res12), QStringLiteral("resistance12"), false, true);
            break;
        case 13:
            writeCharacteristic((uint8_t *)res13, sizeof(res13), QStringLiteral("resistance13"), false, true);
            break;
        case 14:
            writeCharacteristic((uint8_t *)res14, sizeof(res14), QStringLiteral("resistance14"), false, true);
            break;
        case 15:
            writeCharacteristic((uint8_t *)res15, sizeof(res15), QStringLiteral("resistance15"), false, true);
            break;
        case 16:
            writeCharacteristic((uint8_t *)res16, sizeof(res16), QStringLiteral("resistance16"), false, true);
            break;
        case 17:
            writeCharacteristic((uint8_t *)res17, sizeof(res17), QStringLiteral("resistance17"), false, true);
            break;
        case 18:
            writeCharacteristic((uint8_t *)res18, sizeof(res18), QStringLiteral("resistance18"), false, true);
            break;
        case 19:
            writeCharacteristic((uint8_t *)res19, sizeof(res19), QStringLiteral("resistance19"), false, true);
            break;
        case 20:
            writeCharacteristic((uint8_t *)res20, sizeof(res20), QStringLiteral("resistance20"), false, true);
            break;
        case 21:
            writeCharacteristic((uint8_t *)res21, sizeof(res21), QStringLiteral("resistance21"), false, true);
            break;
        case 22:
            writeCharacteristic((uint8_t *)res22, sizeof(res22), QStringLiteral("resistance22"), false, true);
            break;
        case 23:
            writeCharacteristic((uint8_t *)res23, sizeof(res23), QStringLiteral("resistance23"), false, true);
            break;
        case 24:
            writeCharacteristic((uint8_t *)res24, sizeof(res24), QStringLiteral("resistance24"), false, true);
            break;
        }
    }
}

void proformrower::update() {

    if (!m_control)
        return;

    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    if (initRequest) {
        initRequest = false;
        btinit();
    } else if (bluetoothDevice.isValid() && m_control->state() == QLowEnergyController::DiscoveredState &&
               gattCommunicationChannelService && gattWriteCharacteristic.isValid() &&
               gattNotify1Characteristic.isValid() && initDone) {
        QSettings settings;
        update_metrics(true, watts());

        {
            bool proform_rower_750r =
                settings.value(QZSettings::proform_rower_750r, QZSettings::default_proform_rower_750r).toBool();
            bool proform_rower_sport_rl =
                settings.value(QZSettings::proform_rower_sport_rl, QZSettings::default_proform_rower_sport_rl).toBool();

            if (proform_rower_750r) {
                const uint8_t poll1[] = {0xfe, 0x02, 0x18, 0x03};
                const uint8_t poll2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x14, 0x14, 0x14, 0x02, 0x00,
                                         0x0e, 0x1c, 0x96, 0x31, 0x00, 0x10, 0x40, 0x40, 0x00, 0x80};
                const uint8_t poll3[] = {0xff, 0x06, 0x00, 0x00, 0x00, 0x81, 0x60, 0x0c, 0x00, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                const uint8_t poll4[] = {0xfe, 0x02, 0x19, 0x03};
                const uint8_t poll5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x00,
                                         0x0f, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                const uint8_t poll6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x80, 0x11, 0x9b, 0x00,
                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

                switch (counterPoll) {
                case 0:
                    writeCharacteristic(const_cast<uint8_t *>(poll1), sizeof(poll1), QStringLiteral("poll750r"));
                    break;
                case 1:
                    writeCharacteristic(const_cast<uint8_t *>(poll2), sizeof(poll2), QStringLiteral("poll750r"));
                    break;
                case 2:
                    writeCharacteristic(const_cast<uint8_t *>(poll3), sizeof(poll3), QStringLiteral("poll750r"));
                    break;
                case 3:
                    writeCharacteristic(const_cast<uint8_t *>(poll4), sizeof(poll4), QStringLiteral("poll750r"));
                    break;
                case 4:
                    writeCharacteristic(const_cast<uint8_t *>(poll5), sizeof(poll5), QStringLiteral("poll750r"));
                    break;
                case 5:
                    writeCharacteristic(const_cast<uint8_t *>(poll6), sizeof(poll6), QStringLiteral("poll750r"), false,
                                        true);
                    if (requestResistance != -1) {
                        if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                            requestResistance <= max_resistance) {
                            emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                            forceResistance(requestResistance);
                        }
                        requestResistance = -1;
                    }
                    break;
                }
                counterPoll++;
                if (counterPoll > 5) {
                    counterPoll = 0;
                }
            } else {
                uint8_t noOpData1[] = {0xfe, 0x02, 0x17, 0x03};
                uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x14, 0x13, 0x02, 0x00,
                                       0x0d, 0x1c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
                uint8_t noOpData3[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xa2, 0x00, 0x00, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                uint8_t noOpData4[] = {0xfe, 0x02, 0x19, 0x03};
                uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x00,
                                       0x0f, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
                uint8_t noOpData6[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0xe0, 0x11, 0xeb, 0x00,
                                       0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

                switch (counterPoll) {
                case 0:
                    writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
                    break;
                case 1:
                    writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
                    break;
                case 2:
                    writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
                    break;
                case 3:
                    if (requestResistance != -1 && proform_rower_sport_rl) {
                        if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                            requestResistance <= max_resistance) {
                            emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                            forceResistance(requestResistance);
                        }
                        requestResistance = -1;
                    }
                    writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), true);
                    if (requestResistance != -1 && !proform_rower_sport_rl) {
                        if (requestResistance != currentResistance().value() && requestResistance >= 0 &&
                            requestResistance <= max_resistance) {
                            emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
                            forceResistance(requestResistance);
                        }
                        requestResistance = -1;
                    }
                    break;
                case 4:
                    writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
                    break;
                case 5:
                    writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
                    break;
                }
                counterPoll++;
                if (counterPoll > 5) {
                    counterPoll = 0;
                }
            }
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }
        /*
        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

        // btinit();

        requestStart = -1;
        emit tapeStarted();
    }*/
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void proformrower::changeInclinationRequested(double grade, double percentage) {
    if (percentage < 0)
        percentage = 0;
    changeInclination(grade, percentage);
}

void proformrower::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

double proformrower::GetResistanceFromPacket(QByteArray packet) {
    uint8_t r = (uint8_t)(packet.at(11));
    switch (r) {
    case 1:
        return 1;
    case 3:
        return 2;
    case 4:
        return 3;
    case 6:
        return 4;
    case 7:
    case 8:
        return 5;
    case 9:
        return 6;
    case 0x0b:
        return 7;
    case 0x0c:
    case 0x0d:
        return 8;
    case 0x0e:
        return 9;
    case 0x10:
        return 10;
    case 0x11:
        return 11;
    case 0x13:
        return 12;
    case 0x14:
    case 0x15:
        return 13;
    case 0x16:
        return 14;
    case 0x18:
        return 15;
    case 0x19:
    case 0x1a:
        return 16;
    case 0x1b:
        return 17;
    case 0x1d:
        return 18;
    case 0x1e:
        return 19;
    case 0x20:
        return 20;
    case 0x22:
        return 21;
    case 0x23:
        return 22;
    case 0x25:
        return 23;
    case 0x26:
    case 0x27:
        return 24;
    }
    return 1;
}

void proformrower::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    double weight = settings.value(QZSettings::weight, QZSettings::default_weight).toFloat();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (newValue.length() == 20 && (uint8_t)newValue.at(0) == 0xff && newValue.at(1) == 0x11) {
        Cadence = (uint8_t)(newValue.at(12));
        StrokesCount += (Cadence.value()) *
                        ((double)lastRefreshCharacteristicChanged.msecsTo(now)) / 60000;
        emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
        emit debug(QStringLiteral("Strokes Count: ") + QString::number(StrokesCount.value()));
        uint16_t s = (((uint16_t)((uint8_t)newValue.at(14)) << 8) + (uint16_t)((uint8_t)newValue.at(13)));
        if (s > 0)
            Speed = (60.0 / (double)(s)) * 30.0;
        else
            Speed = 0;
        emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
        return;
    }

    if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
        newValue.at(3) != 0x04 || newValue.at(4) != 0x02 || (newValue.at(5) != 0x2e && newValue.at(5) != 0x30) ||
        (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
         ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
         ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
         ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
        return;
    }

    m_watts = ((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12)));
    Resistance = GetResistanceFromPacket(newValue);
    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) * weight * 3.5) / 200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
    // Distance += ((Speed.value() / 3600000.0) *
    // ((double)lastRefreshCharacteristicChanged.msecsTo(now)));
    Distance = (((uint16_t)(((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14)))) / 1000.0;

    lastRefreshCharacteristicChanged = now;

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
            update_hr_from_external();
        }
    }

    emit debug(QStringLiteral("Current Inclination: ") + QString::number(Inclination.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void proformrower::btinit() {
    QSettings settings;
    bool proform_rower_750r =
        settings.value(QZSettings::proform_rower_750r, QZSettings::default_proform_rower_750r).toBool();
    bool proform_rower_sport_rl =
        settings.value(QZSettings::proform_rower_sport_rl, QZSettings::default_proform_rower_sport_rl).toBool();

    if (proform_rower_750r) {
        auto sendInit = [&](const uint8_t *data, size_t len, bool wait = false) {
            writeCharacteristic(const_cast<uint8_t *>(data), len, QStringLiteral("init"), false, wait);
            QThread::msleep(400);
        };

        const uint8_t init1[] = {0xfe, 0x02, 0x08, 0x02};
        const uint8_t init2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init3[] = {0xfe, 0x02, 0x08, 0x02};
        const uint8_t init4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x14, 0x04, 0x80, 0x98,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init5[] = {0xfe, 0x02, 0x08, 0x02};
        const uint8_t init6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x14, 0x04, 0x88, 0xa0,
                                 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init7[] = {0xfe, 0x02, 0x0b, 0x02};
        const uint8_t init8[] = {0xff, 0x0b, 0x02, 0x04, 0x02, 0x07, 0x02, 0x07, 0x82, 0x00,
                                 0x00, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init9[] = {0xfe, 0x02, 0x0a, 0x02};
        const uint8_t init10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                                  0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init11[] = {0xfe, 0x02, 0x08, 0x02};
        const uint8_t init12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init13[] = {0xfe, 0x02, 0x2c, 0x04};
        const uint8_t init14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x14, 0x28, 0x90, 0x07,
                                  0x01, 0x1b, 0x20, 0x37, 0x44, 0x53, 0x60, 0x77, 0x9c, 0xbb};
        const uint8_t init15[] = {0x01, 0x12, 0xc0, 0xe7, 0x04, 0x23, 0x40, 0x67, 0x8c, 0xdb,
                                  0xe0, 0x37, 0x44, 0x93, 0xa0, 0xf7, 0x3c, 0x7b, 0x80, 0xc7};
        const uint8_t init16[] = {0xff, 0x08, 0x04, 0x43, 0x80, 0x98, 0x02, 0x00, 0x00, 0xcb,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init17[] = {0xfe, 0x02, 0x19, 0x03};
        const uint8_t init18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x0e,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x4a, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init20[] = {0xfe, 0x02, 0x17, 0x03};
        const uint8_t init21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x14, 0x13, 0x02, 0x0c,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init22[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xb6, 0x00, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init23[] = {0xfe, 0x02, 0x19, 0x03};
        const uint8_t init24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x00,
                                  0x0f, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        const uint8_t init25[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6a, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init26[] = {0xfe, 0x02, 0x18, 0x03};
        const uint8_t init27[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x14, 0x14, 0x14, 0x02, 0x00,
                                  0x0e, 0x1c, 0x96, 0x31, 0x00, 0x10, 0x40, 0x40, 0x00, 0x80};
        const uint8_t init28[] = {0xff, 0x06, 0x00, 0x00, 0x00, 0x81, 0x60, 0x0c, 0x00, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init29[] = {0xfe, 0x02, 0x11, 0x02};
        const uint8_t init30[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x14, 0x0d, 0x02, 0x05,
                                  0x00, 0x00, 0x00, 0x00, 0x08, 0x58, 0x02, 0x00, 0x8a, 0x00};
        const uint8_t init31[] = {0xfe, 0x02, 0x19, 0x03};
        const uint8_t init32[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x0e,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init33[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x4a, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init34[] = {0xfe, 0x02, 0x10, 0x02};
        const uint8_t init35[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x14, 0x0c, 0x02, 0x04,
                                  0x00, 0x00, 0x00, 0x02, 0xe4, 0x1f, 0x00, 0x2b, 0x00, 0x00};
        const uint8_t init36[] = {0xfe, 0x02, 0x19, 0x03};
        const uint8_t init37[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x00,
                                  0x0f, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init38[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x80, 0x11, 0x9b, 0x00,
                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t init39[] = {0xfe, 0x02, 0x10, 0x02};
        const uint8_t init40[] = {0xff, 0x10, 0x02, 0x04, 0x02, 0x0c, 0x14, 0x0c, 0x02, 0x05,
                                  0x00, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x37, 0x00, 0x00};

        sendInit(init1, sizeof(init1));
        sendInit(init2, sizeof(init2));
        sendInit(init3, sizeof(init3));
        sendInit(init4, sizeof(init4));
        sendInit(init5, sizeof(init5));
        sendInit(init6, sizeof(init6));
        sendInit(init7, sizeof(init7));
        sendInit(init8, sizeof(init8));
        sendInit(init9, sizeof(init9));
        sendInit(init10, sizeof(init10));
        sendInit(init11, sizeof(init11));
        sendInit(init12, sizeof(init12));
        sendInit(init13, sizeof(init13));
        sendInit(init14, sizeof(init14));
        sendInit(init15, sizeof(init15));
        sendInit(init16, sizeof(init16));
        sendInit(init17, sizeof(init17));
        sendInit(init18, sizeof(init18));
        sendInit(init19, sizeof(init19));
        sendInit(init20, sizeof(init20));
        sendInit(init21, sizeof(init21));
        sendInit(init22, sizeof(init22));
        sendInit(init23, sizeof(init23));
        sendInit(init24, sizeof(init24));
        sendInit(init25, sizeof(init25));
        sendInit(init26, sizeof(init26));
        sendInit(init27, sizeof(init27));
        sendInit(init28, sizeof(init28));
        sendInit(init29, sizeof(init29));
        sendInit(init30, sizeof(init30));
        sendInit(init31, sizeof(init31));
        sendInit(init32, sizeof(init32));
        sendInit(init33, sizeof(init33));
        sendInit(init34, sizeof(init34));
        sendInit(init35, sizeof(init35));
        sendInit(init36, sizeof(init36));
        sendInit(init37, sizeof(init37));
        sendInit(init38, sizeof(init38));
        sendInit(init39, sizeof(init39));
        sendInit(init40, sizeof(init40));
        initDone = true;
        return;
    }

    {
        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x14, 0x04, 0x80, 0x98,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x14, 0x04, 0x88, 0xa0,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};

        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x14, 0x28, 0x90, 0x07,
                                0x01, 0xed, 0xe8, 0xe9, 0xe8, 0xf5, 0xf0, 0x09, 0x00, 0x1d};
        uint8_t initData11[] = {0x01, 0x12, 0x28, 0x39, 0x48, 0x55, 0x60, 0x99, 0xb0, 0xad,
                                0xc8, 0xe9, 0x08, 0x55, 0x70, 0x89, 0xa0, 0xfd, 0x08, 0x59};
        uint8_t initData12[] = {0xff, 0x08, 0x68, 0xb5, 0xc0, 0x98, 0x02, 0x00, 0x00, 0x55,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x00,
                               0x0f, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x6a, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x14, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xb6, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x00,
                               0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x84, 0xc4, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData9[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x13, 0x13, 0x02, 0x0c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t initData10_proform_rower_sport_rl[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x14, 0x28, 0x90, 0x07, 0x01, 0xf2, 0xf4, 0xf4, 0xf2, 0xfe, 0x08, 0x00, 0x1e, 0x2a};
        uint8_t initData11_proform_rower_sport_rl[] = {0x01, 0x12, 0x3c, 0x4c, 0x5a, 0x66, 0x90, 0x88, 0xa6, 0xc2, 0xe4, 0x04, 0x22, 0x4e, 0x98, 0xb0, 0xce, 0x1a, 0x2c, 0x7c};
        uint8_t initData12_proform_rower_sport_rl[] = {0xff, 0x08, 0x8a, 0xd6, 0x20, 0x98, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData7_proform_rower_sport_rl[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x14, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData8_proform_rower_sport_rl[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x4a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        if (proform_rower_sport_rl) {
            writeCharacteristic(initData10_proform_rower_sport_rl, sizeof(initData10_proform_rower_sport_rl), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11_proform_rower_sport_rl, sizeof(initData11_proform_rower_sport_rl), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12_proform_rower_sport_rl, sizeof(initData12_proform_rower_sport_rl), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData7_proform_rower_sport_rl, sizeof(noOpData7_proform_rower_sport_rl), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData8_proform_rower_sport_rl, sizeof(noOpData8_proform_rower_sport_rl), QStringLiteral("init"), false, true);
            QThread::msleep(400);
            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else {
            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        }
        /*writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("init"), false, false);
            QThread::msleep(400);*/
    }

    initDone = true;
}

void proformrower::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("00001534-1412-efde-1523-785feabcd123"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("00001535-1412-efde-1523-785feabcd123"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &proformrower::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &proformrower::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &proformrower::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &proformrower::descriptorWritten);

        // ******************************************* virtual treadmill init *************************************
        QSettings settings;
        bool virtual_device_rower =
            settings.value(QZSettings::virtual_device_rower, QZSettings::default_virtual_device_rower).toBool();
        if (!firstStateChanged && !this->hasVirtualDevice()) {
            bool virtual_device_enabled =
                settings.value(QZSettings::virtual_device_enabled, QZSettings::default_virtual_device_enabled).toBool();
            bool virtual_device_force_bike =
                settings.value(QZSettings::virtual_device_force_bike, QZSettings::default_virtual_device_force_bike)
                    .toBool();
            if (virtual_device_enabled) {
                if (virtual_device_rower) {
                    qDebug() << QStringLiteral("creating virtual rower interface...");
                    auto virtualRower = new virtualrower(this, noWriteResistance, noHeartService);
                    // connect(virtualRower,&virtualrower::debug ,this,&echelonrower::debug);
                    this->setVirtualDevice(virtualRower, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                } else if (!virtual_device_force_bike) {
                    debug("creating virtual treadmill interface...");
                    auto virtualTreadmill = new virtualtreadmill(this, noHeartService);
                    connect(virtualTreadmill, &virtualtreadmill::debug, this, &proformrower::debug);
                    connect(virtualTreadmill, &virtualtreadmill::changeInclination, this,
                            &proformrower::changeInclinationRequested);
                    this->setVirtualDevice(virtualTreadmill, VIRTUAL_DEVICE_MODE::PRIMARY);
                } else {
                    debug("creating virtual bike interface...");
                    auto virtualBike = new virtualbike(this);
                    connect(virtualBike, &virtualbike::changeInclination, this,
                            &proformrower::changeInclinationRequested);
                    this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::ALTERNATIVE);
                }
                firstStateChanged = 1;
            }
        }
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
    }
}

void proformrower::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformrower::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void proformrower::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &proformrower::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void proformrower::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("proformrower::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformrower::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("proformrower::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformrower::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_TL")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &proformrower::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &proformrower::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &proformrower::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &proformrower::controllerStateChanged);

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

bool proformrower::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void proformrower::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

uint16_t proformrower::watts() {
    /*if (currentCadence().value() == 0) {
        return 0;
    }*/

    return m_watts;
}
