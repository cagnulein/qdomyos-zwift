#include "proformbike.h"

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QThread>
#include <chrono>
#include <math.h>

using namespace std::chrono_literals;

proformbike::proformbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &proformbike::update);
    refresh->start(200ms);
}

void proformbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
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

resistance_t proformbike::resistanceFromPowerRequest(uint16_t power) {
    qDebug() << QStringLiteral("resistanceFromPowerRequest") << Cadence.value();

    QSettings settings;

    double watt_gain = settings.value(QZSettings::watt_gain, QZSettings::default_watt_gain).toDouble();
    double watt_offset = settings.value(QZSettings::watt_offset, QZSettings::default_watt_offset).toDouble();

    for (resistance_t i = 1; i < max_resistance; i++) {
        if (((wattsFromResistance(i) * watt_gain) + watt_offset) <= power &&
            ((wattsFromResistance(i + 1) * watt_gain) + watt_offset) >= power) {
            qDebug() << QStringLiteral("resistanceFromPowerRequest")
                     << ((wattsFromResistance(i) * watt_gain) + watt_offset)
                     << ((wattsFromResistance(i + 1) * watt_gain) + watt_offset) << power;
            return i;
        }
    }
    if (power < ((wattsFromResistance(1) * watt_gain) + watt_offset))
        return 1;
    else
        return max_resistance;
}

uint16_t proformbike::wattsFromResistance(resistance_t resistance) {

    if (currentCadence().value() == 0)
        return 0;

    switch (resistance) {
    case 0:
    case 1:
        // -13.5 + 0.999x + 0.00993x²
        return (-13.5 + (0.999 * currentCadence().value()) + (0.00993 * pow(currentCadence().value(), 2)));
    case 2:
        // -17.7 + 1.2x + 0.0116x²
        return (-17.7 + (1.2 * currentCadence().value()) + (0.0116 * pow(currentCadence().value(), 2)));

    case 3:
        // -17.5 + 1.24x + 0.014x²
        return (-17.5 + (1.24 * currentCadence().value()) + (0.014 * pow(currentCadence().value(), 2)));

    case 4:
        // -20.9 + 1.43x + 0.016x²
        return (-20.9 + (1.43 * currentCadence().value()) + (0.016 * pow(currentCadence().value(), 2)));

    case 5:
        // -27.9 + 1.75x+0.0172x²
        return (-27.9 + (1.75 * currentCadence().value()) + (0.0172 * pow(currentCadence().value(), 2)));

    case 6:
        // -26.7 + 1.9x + 0.0201x²
        return (-26.7 + (1.9 * currentCadence().value()) + (0.0201 * pow(currentCadence().value(), 2)));

    case 7:
        // -33.5 + 2.23x + 0.0225x²
        return (-33.5 + (2.23 * currentCadence().value()) + (0.0225 * pow(currentCadence().value(), 2)));

    case 8:
        // -36.5+2.5x+0.0262x²
        return (-36.5 + (2.5 * currentCadence().value()) + (0.0262 * pow(currentCadence().value(), 2)));

    case 9:
        // -38+2.62x+0.0305x²
        return (-38.0 + (2.62 * currentCadence().value()) + (0.0305 * pow(currentCadence().value(), 2)));

    case 10:
        // -41.2+2.85x+0.0327x²
        return (-41.2 + (2.85 * currentCadence().value()) + (0.0327 * pow(currentCadence().value(), 2)));

    case 11:
        // -43.4+3.01x+0.0359x²
        return (-43.4 + (3.01 * currentCadence().value()) + (0.0359 * pow(currentCadence().value(), 2)));

    case 12:
        // -46.8+3.23x+0.0364x²
        return (-46.8 + (3.23 * currentCadence().value()) + (0.0364 * pow(currentCadence().value(), 2)));

    case 13:
        // -49+3.39x+0.0371x²
        return (-49.0 + (3.39 * currentCadence().value()) + (0.0371 * pow(currentCadence().value(), 2)));

    case 14:
        // -53.4+3.55x+0.0383x²
        return (-53.4 + (3.55 * currentCadence().value()) + (0.0383 * pow(currentCadence().value(), 2)));

    case 15:
        // -49.9+3.37x+0.0429x²
        return (-49.9 + (3.37 * currentCadence().value()) + (0.0429 * pow(currentCadence().value(), 2)));

    case 16:
    default:
        // -47.1+3.25x+0.0464x²
        return (-47.1 + (3.25 * currentCadence().value()) + (0.0464 * pow(currentCadence().value(), 2)));
    }
}

void proformbike::forceResistance(resistance_t requestResistance) {
    QSettings settings;
    bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();
    bool proform_tdf_10 = settings.value(QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10).toBool();
    bool nordictrack_GX4_5_bike =
        settings.value(QZSettings::nordictrack_GX4_5_bike, QZSettings::default_nordictrack_GX4_5_bike).toBool();
    bool nordictrack_gx_2_7 =
        settings.value(QZSettings::nordictrack_gx_2_7, QZSettings::default_nordictrack_gx_2_7).toBool();
    bool proform_hybrid_trainer_PFEL03815 =
        settings
            .value(QZSettings::proform_hybrid_trainer_PFEL03815, QZSettings::default_proform_hybrid_trainer_PFEL03815)
            .toBool();
    bool proform_bike_sb = settings.value(QZSettings::proform_bike_sb, QZSettings::default_proform_bike_sb).toBool();
    bool proform_cycle_trainer_300_ci =
        settings.value(QZSettings::proform_cycle_trainer_300_ci, QZSettings::default_proform_cycle_trainer_300_ci)
            .toBool();    
    bool proform_bike_225_csx = settings.value(QZSettings::proform_bike_225_csx, QZSettings::default_proform_bike_225_csx).toBool();
    bool proform_bike_325_csx = settings.value(QZSettings::proform_bike_325_csx, QZSettings::default_proform_bike_325_csx).toBool();

    if (proform_studio || proform_tdf_10) {
        const uint8_t res1[] = {0xfe, 0x02, 0x16, 0x03};
        uint8_t res2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x12, 0x08, 0x12, 0x02, 0x04,
                          0x00, 0x00, 0x00, 0x04, 0x00, 0x00, 0x00, 0x00, 0x06, 0x06};

        res2[18] = requestResistance;

        uint8_t res3[] = {0xff, 0x04, 0x00, 0x1b, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00,
                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        res3[5] = 0x45 + requestResistance;

        writeCharacteristic((uint8_t *)res1, sizeof(res1), QStringLiteral("resistance1"), false, false);
        writeCharacteristic((uint8_t *)res2, sizeof(res2), QStringLiteral("resistance2"), false, false);
        writeCharacteristic((uint8_t *)res3, sizeof(res3), QStringLiteral("resistance3"), false, true);
    } else if (proform_hybrid_trainer_PFEL03815 || proform_bike_sb) {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x32, 0x02, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xa3, 0x04, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x14, 0x07, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x85, 0x09, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xf6, 0x0b, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x67, 0x0e, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xd8, 0x10, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x49, 0x13, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xba, 0x15, 0x00, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x2b, 0x18, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x9c, 0x1a, 0x00, 0xcd, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x0d, 0x1d, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x7e, 0x1f, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xef, 0x21, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x60, 0x24, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xd1, 0x26, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
        writeCharacteristic((uint8_t *)noOpData7, sizeof(noOpData7), QStringLiteral("resrequest"), false, false);

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
        }
    } else if (nordictrack_gx_2_7 || proform_bike_225_csx) {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xc2, 0x01, 0x00, 0xda, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xb6, 0x03, 0x00, 0xd0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xaa, 0x05, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x9e, 0x07, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x92, 0x09, 0x00, 0xb2, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x86, 0x0b, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x7a, 0x0d, 0x00, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x6e, 0x0f, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x62, 0x11, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x56, 0x13, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x4a, 0x15, 0x00, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x3e, 0x17, 0x00, 0x6c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x32, 0x19, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x26, 0x1b, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x1a, 0x1d, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x0e, 0x1f, 0x00, 0x44, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x02, 0x21, 0x00, 0x3a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xf6, 0x22, 0x00, 0x2f, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xea, 0x24, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xde, 0x26, 0x00, 0x1b, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
        writeCharacteristic((uint8_t *)noOpData7, sizeof(noOpData7), QStringLiteral("resrequest"), false, false);

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
        }
    } else if (proform_bike_325_csx) {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x98, 0x01, 0x00, 0xb0, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x5f, 0x03, 0x00, 0x79, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x25, 0x05, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xec, 0x06, 0x00, 0x09, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xb2, 0x08, 0x00, 0xd1, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x79, 0x0a, 0x00, 0x9a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x3f, 0x0c, 0x00, 0x62, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x06, 0x0e, 0x00, 0x2b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xcc, 0x0f, 0x00, 0xf2, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x93, 0x11, 0x00, 0xbb, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x5a, 0x13, 0x00, 0x84, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x20, 0x15, 0x00, 0x4c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xe7, 0x16, 0x00, 0x14, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xad, 0x18, 0x00, 0xdc, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x74, 0x1a, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x3a, 0x1c, 0x00, 0x6d, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x01, 0x1e, 0x00, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xc7, 0x1f, 0x00, 0xfd, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x8e, 0x21, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x54, 0x23, 0x00, 0x8e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res21[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x1b, 0x25, 0x00, 0x57, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xe2, 0x26, 0x00, 0x1f, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
        writeCharacteristic((uint8_t *)noOpData7, sizeof(noOpData7), QStringLiteral("resrequest"), false, false);

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
        }        
    } else if(nordictrack_GX4_5_bike) {
        const uint8_t res25[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xe8, 0x26, 0x00, 0x25, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res24[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x58, 0x25, 0x00, 0x94, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res23[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xc8, 0x23, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res22[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x38, 0x22, 0x00, 0x71, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res21[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xa8, 0x20, 0x00, 0xdf, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res20[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x18, 0x1f, 0x00, 0x4e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res19[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x88, 0x1d, 0x00, 0xbc, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res18[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xf8, 0x1b, 0x00, 0x2a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res17[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x68, 0x1a, 0x00, 0x99, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xd8, 0x18, 0x00, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x48, 0x17, 0x00, 0x76, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xb8, 0x15, 0x00, 0xe4, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x28, 0x14, 0x00, 0x53, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x98, 0x12, 0x00, 0xc1, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x08, 0x11, 0x00, 0x30, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x78, 0x0f, 0x00, 0x9e, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xe8, 0x0d, 0x00, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x58, 0x0c, 0x00, 0x7b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xc8, 0x0a, 0x00, 0xe9, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x38, 0x09, 0x00, 0x58, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xa8, 0x07, 0x00, 0xc6, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x18, 0x06, 0x00, 0x35, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x88, 0x04, 0x00, 0xa3, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0xf8, 0x02, 0x00, 0x11, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01, 0x04, 0x68, 0x01, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00};

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
        case 25:
            writeCharacteristic((uint8_t *)res25, sizeof(res25), QStringLiteral("resistance25"), false, true);
            break;            
        }
    } else {
        const uint8_t res1[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x32, 0x02, 0x00, 0x4b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res2[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xa3, 0x04, 0x00, 0xbe, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res3[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x14, 0x07, 0x00, 0x32, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res4[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x85, 0x09, 0x00, 0xa5, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res5[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xf6, 0x0b, 0x00, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res6[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x67, 0x0e, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xd8, 0x10, 0x00, 0xff, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res8[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0x49, 0x13, 0x00, 0x73, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res9[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                0x04, 0xba, 0x15, 0x00, 0xe6, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res10[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x2b, 0x18, 0x00, 0x5a, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res11[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x9c, 0x1a, 0x00, 0xcd, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res12[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x0d, 0x1d, 0x00, 0x41, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res13[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x7e, 0x1f, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res14[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xef, 0x21, 0x00, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res15[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0x60, 0x24, 0x00, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00};
        const uint8_t res16[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x01,
                                 0x04, 0xd1, 0x26, 0x00, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00};

        if(proform_cycle_trainer_300_ci) {
            uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};
            writeCharacteristic((uint8_t *)noOpData7, sizeof(noOpData7), QStringLiteral("resrequest"), false, false);            
        }

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
        }
    }
}

void proformbike::forceIncline(double incline) {
    uint8_t write[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x08, 0x09, 0x02, 0x01,
                       0x02, 0x38, 0xff, 0x00, 0x4d, 0x00, 0x00, 0x00, 0x00, 0x00};

    write[11] = ((uint16_t)(incline * 100)) & 0x00FF;
    write[12] = ((((uint16_t)(incline * 100)) & 0xFF00) >> 8) & 0x00FF;
    write[14] = write[6] + write[7] + write[8] + write[9] + write[10] + write[11] + write[12];

    writeCharacteristic((uint8_t *)write, sizeof(write), QStringLiteral("incline ") + QString::number(incline), false,
                        true);
}

void proformbike::innerWriteResistance() {
    if (requestResistance != -1) {
        if (requestResistance > max_resistance) {
            requestResistance = max_resistance;
        } else if (requestResistance == 0) {
            requestResistance = 1;
        }

        if (requestResistance != currentResistance().value()) {
            emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));
            forceResistance(requestResistance);
        }
        requestResistance = -1;
    }
}

void proformbike::update() {
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
        update_metrics(true, watts());

        QSettings settings;
        bool proform_tour_de_france_clc =
            settings.value(QZSettings::proform_tour_de_france_clc, QZSettings::default_proform_tour_de_france_clc)
                .toBool();
        bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();
        bool proform_tdf_10 = settings.value(QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10).toBool();
        bool nordictrack_gx_2_7 =
            settings.value(QZSettings::nordictrack_gx_2_7, QZSettings::default_nordictrack_gx_2_7).toBool();
        bool proform_cycle_trainer_300_ci =
            settings.value(QZSettings::proform_cycle_trainer_300_ci, QZSettings::default_proform_cycle_trainer_300_ci)
                .toBool();                   
        bool proform_cycle_trainer_400 =
            settings.value(QZSettings::proform_cycle_trainer_400, QZSettings::default_proform_cycle_trainer_400)
                .toBool();
        bool proform_hybrid_trainer_PFEL03815 = settings
                                                    .value(QZSettings::proform_hybrid_trainer_PFEL03815,
                                                           QZSettings::default_proform_hybrid_trainer_PFEL03815)
                                                    .toBool();
        bool proform_bike_sb =
            settings.value(QZSettings::proform_bike_sb, QZSettings::default_proform_bike_sb).toBool();
        bool proform_bike_PFEVEX71316_1 =
            settings.value(QZSettings::proform_bike_PFEVEX71316_1, QZSettings::default_proform_bike_PFEVEX71316_1)
                .toBool();
        bool proform_bike_225_csx = settings.value(QZSettings::proform_bike_225_csx, QZSettings::default_proform_bike_225_csx).toBool();
        bool proform_bike_325_csx = settings.value(QZSettings::proform_bike_325_csx, QZSettings::default_proform_bike_325_csx).toBool();

        uint8_t noOpData1[] = {0xfe, 0x02, 0x19, 0x03};
        uint8_t noOpData2[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00,
                               0x0f, 0xbc, 0x90, 0x70, 0x00, 0x00, 0x00, 0x40, 0x00, 0x00};
        uint8_t noOpData3[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x00, 0x08, 0x5d, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4[] = {0xfe, 0x02, 0x17, 0x03};
        uint8_t noOpData5[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00,
                               0x0d, 0x3c, 0x9c, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData6[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa9, 0x00, 0x00, 0x00,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData7[] = {0xfe, 0x02, 0x0d, 0x02};

        // proform_bike_225_csx
        uint8_t noOpData2_proform_bike_225_csx[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00, 0x0d, 0x3c, 0x96, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData3_proform_bike_225_csx[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData5_proform_bike_225_csx[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00, 0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6_proform_bike_225_csx[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x05, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


        // proform_bike_sb
        uint8_t noOpData2_proform_bike_sb[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00,
                                               0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData3_proform_bike_sb[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xb5, 0x00, 0x00, 0x00,
                                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData5_proform_bike_sb[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x00,
                                               0x03, 0x80, 0x00, 0x40, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00};

        // proform_tour_de_france_clc
        uint8_t noOpData2_proform_tour_de_france_clc[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00,
                                                          0x0f, 0x80, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3_proform_tour_de_france_clc[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xfd, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6_proform_tour_de_france_clc[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xb3, 0x00, 0x00, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        // proform_studio
        uint8_t noOpData2_proform_studio[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x08, 0x15, 0x02, 0x00,
                                              0x0f, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3_proform_studio[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0xbe, 0x00,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4_proform_studio[] = {0xfe, 0x02, 0x14, 0x03};
        uint8_t noOpData5_proform_studio[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x10, 0x08, 0x10, 0x02, 0x00,
                                              0x0a, 0x3e, 0x90, 0x30, 0x04, 0x00, 0x00, 0x50, 0x00, 0x00};
        uint8_t noOpData6_proform_studio[] = {0xff, 0x02, 0x08, 0x7e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                              0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        // nordictrack gx 2.7
        uint8_t noOpData2_nordictrack_gx_2_7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00,
                                                  0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData3_nordictrack_gx_2_7[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xb9, 0x00, 0x00, 0x00,
                                                  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData5_nordictrack_gx_2_7[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x00,
                                                  0x03, 0x80, 0x00, 0x40, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00};

        // proform_cycle_trainer_400
        uint8_t noOpData2_proform_cycle_trainer_400[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x00,
                                                         0x0f, 0x80, 0x08, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3_proform_cycle_trainer_400[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x05, 0x00,
                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData6_proform_cycle_trainer_400[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x81, 0xad, 0x00, 0x00, 0x00,
                                                         0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        // proform_hybrid_trainer_PFEL03815
        uint8_t noOpData2_proform_hybrid_trainer_PFEL03815[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07,
                                                                0x13, 0x02, 0x00, 0x0d, 0x3c, 0x9e, 0x31,
                                                                0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData3_proform_hybrid_trainer_PFEL03815[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xb9,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
                                                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData4_proform_hybrid_trainer_PFEL03815[] = {0xfe, 0x02, 0x11, 0x02};
        uint8_t noOpData5_proform_hybrid_trainer_PFEL03815[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x07,
                                                                0x0d, 0x02, 0x00, 0x07, 0xbc, 0x90, 0x70,
                                                                0x00, 0x00, 0x00, 0x40, 0x19, 0x00};

        // proform_bike_PFEVEX71316_1
        uint8_t noOpData2_proform_bike_PFEVEX71316_1[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x08, 0x15, 0x02, 0x00,
                                                          0x0f, 0xb6, 0x10, 0x30, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData3_proform_bike_PFEVEX71316_1[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x10, 0x38, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData5_proform_bike_PFEVEX71316_1[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x08, 0x13, 0x02, 0x00,
                                                          0x0d, 0x09, 0x8e, 0x41, 0x00, 0x00, 0x40, 0x50, 0x00, 0x80};
        uint8_t noOpData6_proform_bike_PFEVEX71316_1[] = {0xff, 0x05, 0x18, 0x00, 0x00, 0x81, 0xab, 0x00, 0x00, 0x00,
                                                          0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        uint8_t noOpData2_proform_bike_325_csx[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00, 0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
        uint8_t noOpData3_proform_bike_325_csx[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t noOpData5_proform_bike_325_csx[] = {0xff, 0x0d, 0x02, 0x04, 0x02, 0x09, 0x07, 0x09, 0x02, 0x00, 0x03, 0x80, 0x00, 0x40, 0xd5, 0x00, 0x00, 0x00, 0x00, 0x00};

        switch (counterPoll) {
        case 0:
            if (nordictrack_gx_2_7 || proform_cycle_trainer_300_ci || proform_hybrid_trainer_PFEL03815 || proform_bike_sb || proform_bike_225_csx || proform_bike_325_csx) {
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
            } else {
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
            }
            break;
        case 1:
            if (proform_studio || proform_tdf_10)
                writeCharacteristic(noOpData2_proform_studio, sizeof(noOpData2_proform_studio), QStringLiteral("noOp"));
            else if (proform_bike_325_csx) {
                writeCharacteristic(noOpData2_proform_bike_325_csx, sizeof(noOpData2_proform_bike_325_csx),
                                    QStringLiteral("noOp"));
            } else if (nordictrack_gx_2_7 || proform_cycle_trainer_300_ci) {
                writeCharacteristic(noOpData2_nordictrack_gx_2_7, sizeof(noOpData2_nordictrack_gx_2_7),
                                    QStringLiteral("noOp"));
            } else if (proform_hybrid_trainer_PFEL03815) {
                writeCharacteristic(noOpData2_proform_hybrid_trainer_PFEL03815,
                                    sizeof(noOpData2_proform_hybrid_trainer_PFEL03815), QStringLiteral("noOp"));
            } else if (proform_tour_de_france_clc) {
                writeCharacteristic(noOpData2_proform_tour_de_france_clc, sizeof(noOpData2_proform_tour_de_france_clc),
                                    QStringLiteral("noOp"));
            } else if (proform_bike_225_csx)
                writeCharacteristic(noOpData2_proform_bike_225_csx, sizeof(noOpData2_proform_bike_225_csx),
                                    QStringLiteral("noOp"));
            else if (proform_cycle_trainer_400)
                writeCharacteristic(noOpData2_proform_cycle_trainer_400, sizeof(noOpData2_proform_cycle_trainer_400),
                                    QStringLiteral("noOp"));
            else if (proform_bike_sb)
                writeCharacteristic(noOpData2_proform_bike_sb, sizeof(noOpData2_proform_bike_sb),
                                    QStringLiteral("noOp"));
            else if (proform_bike_PFEVEX71316_1)
                writeCharacteristic(noOpData2_proform_bike_PFEVEX71316_1, sizeof(noOpData2_proform_bike_PFEVEX71316_1),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"));
            break;
        case 2:
            if (proform_studio || proform_tdf_10)
                writeCharacteristic(noOpData3_proform_studio, sizeof(noOpData3_proform_studio), QStringLiteral("noOp"));
            else if (proform_bike_325_csx) {
                writeCharacteristic(noOpData3_proform_bike_325_csx, sizeof(noOpData3_proform_bike_325_csx),
                                    QStringLiteral("noOp"));
            } else if (proform_tour_de_france_clc) {
                writeCharacteristic(noOpData3_proform_tour_de_france_clc, sizeof(noOpData3_proform_tour_de_france_clc),
                                    QStringLiteral("noOp"));
            } else if (nordictrack_gx_2_7 || proform_cycle_trainer_300_ci) {
                writeCharacteristic(noOpData3_nordictrack_gx_2_7, sizeof(noOpData3_nordictrack_gx_2_7),
                                    QStringLiteral("noOp"));
            } else if (proform_hybrid_trainer_PFEL03815) {
                writeCharacteristic(noOpData3_proform_hybrid_trainer_PFEL03815,
                                    sizeof(noOpData3_proform_hybrid_trainer_PFEL03815), QStringLiteral("noOp"));
            } else if (proform_cycle_trainer_400) {
                writeCharacteristic(noOpData3_proform_cycle_trainer_400, sizeof(noOpData3_proform_cycle_trainer_400),
                                    QStringLiteral("noOp"));
            } else if (proform_bike_225_csx)
                writeCharacteristic(noOpData3_proform_bike_225_csx, sizeof(noOpData3_proform_bike_225_csx),
                                    QStringLiteral("noOp"));
            else if (proform_bike_sb)
                writeCharacteristic(noOpData3_proform_bike_sb, sizeof(noOpData3_proform_bike_sb),
                                    QStringLiteral("noOp"));
            else if (proform_bike_PFEVEX71316_1)
                writeCharacteristic(noOpData3_proform_bike_PFEVEX71316_1, sizeof(noOpData3_proform_bike_PFEVEX71316_1),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"));
            break;
        case 3:
            if (proform_studio || proform_tdf_10)
                writeCharacteristic(noOpData4_proform_studio, sizeof(noOpData4_proform_studio), QStringLiteral("noOp"));
            else if (nordictrack_gx_2_7 || proform_cycle_trainer_300_ci) {
                innerWriteResistance();
                writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("noOp"));
            } else if (proform_hybrid_trainer_PFEL03815) {
                innerWriteResistance();
                writeCharacteristic(noOpData4_proform_hybrid_trainer_PFEL03815,
                                    sizeof(noOpData4_proform_hybrid_trainer_PFEL03815), QStringLiteral("noOp"));
            } else if (proform_bike_sb || proform_bike_325_csx) {
                innerWriteResistance();
                writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("noOp"));
            } else if(proform_bike_225_csx) {
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"));
            } else
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"));
            break;
        case 4:
            if (proform_studio || proform_tdf_10)
                writeCharacteristic(noOpData5_proform_studio, sizeof(noOpData5_proform_studio), QStringLiteral("noOp"));
            else if (nordictrack_gx_2_7 || proform_cycle_trainer_300_ci) {
                writeCharacteristic(noOpData5_nordictrack_gx_2_7, sizeof(noOpData5_nordictrack_gx_2_7),
                                    QStringLiteral("noOp"));
            } else if (proform_hybrid_trainer_PFEL03815) {
                writeCharacteristic(noOpData5_proform_hybrid_trainer_PFEL03815,
                                    sizeof(noOpData5_proform_hybrid_trainer_PFEL03815), QStringLiteral("noOp"));
            } else if (proform_bike_325_csx) {
                writeCharacteristic(noOpData5_proform_bike_325_csx, sizeof(noOpData5_proform_bike_325_csx),
                                    QStringLiteral("noOp"));                                    
            } else if (proform_bike_225_csx) {
                writeCharacteristic(noOpData5_proform_bike_225_csx, sizeof(noOpData5_proform_bike_225_csx),
                                    QStringLiteral("noOp"));
            } else if (proform_bike_sb)
                writeCharacteristic(noOpData5_proform_bike_sb, sizeof(noOpData5_proform_bike_sb),
                                    QStringLiteral("noOp"));
            else if (proform_bike_PFEVEX71316_1)
                writeCharacteristic(noOpData5_proform_bike_PFEVEX71316_1, sizeof(noOpData5_proform_bike_PFEVEX71316_1),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData5, sizeof(noOpData5), QStringLiteral("noOp"));
            break;
        case 5:
            if (proform_studio || proform_tdf_10)
                writeCharacteristic(noOpData6_proform_studio, sizeof(noOpData6_proform_studio), QStringLiteral("noOp"));
            else if (proform_tour_de_france_clc) {
                writeCharacteristic(noOpData6_proform_tour_de_france_clc, sizeof(noOpData6_proform_tour_de_france_clc),
                                    QStringLiteral("noOp"));
            } else if (proform_bike_225_csx) {
                writeCharacteristic(noOpData6_proform_bike_225_csx, sizeof(noOpData6_proform_bike_225_csx),
                                    QStringLiteral("noOp"));
                innerWriteResistance();
            }
            else if (proform_cycle_trainer_400)
                writeCharacteristic(noOpData6_proform_cycle_trainer_400, sizeof(noOpData6_proform_cycle_trainer_400),
                                    QStringLiteral("noOp"));
            else if (proform_bike_PFEVEX71316_1)
                writeCharacteristic(noOpData6_proform_bike_PFEVEX71316_1, sizeof(noOpData6_proform_bike_PFEVEX71316_1),
                                    QStringLiteral("noOp"));
            else
                writeCharacteristic(noOpData6, sizeof(noOpData6), QStringLiteral("noOp"));
            break;
        case 6:
            if (proform_studio || proform_tdf_10) {
                innerWriteResistance();
            }
            writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("noOp"));
            if (!proform_studio && !proform_tdf_10) {
                innerWriteResistance();
            }
            if (requestInclination != -100 && (proform_studio || proform_tdf_10)) {
                // only 0.5 steps ara available
                double inc = qRound(requestInclination * 2.0) / 2.0;
                if (inc != currentInclination().value()) {
                    emit debug(QStringLiteral("writing inclination ") + QString::number(requestInclination) +
                               " rounded " + QString::number(inc));
                    forceIncline(inc);
                }
                requestInclination = -100;
            }
            break;
        }

        counterPoll++;
        if (counterPoll > 6) {
            counterPoll = 0;
        } else if(counterPoll == 6 && proform_bike_225_csx) {
            counterPoll = 0;
        } else if (counterPoll == 6 &&
                   (proform_tour_de_france_clc || proform_cycle_trainer_400 || proform_bike_PFEVEX71316_1) &&
                   requestResistance == -1) {
            // this bike sends the frame noOpData7 only when it needs to change the resistance
            counterPoll = 0;
        } else if (counterPoll == 5 && (nordictrack_gx_2_7 || proform_cycle_trainer_300_ci || proform_hybrid_trainer_PFEL03815 || proform_bike_sb || proform_bike_325_csx)) {
            counterPoll = 0;
        }

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestStart != -1) {
            emit debug(QStringLiteral("starting..."));

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            emit debug(QStringLiteral("stopping..."));
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

bool proformbike::inclinationAvailableByHardware() {
    QSettings settings;
    bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();
    bool proform_tdf_10 = settings.value(QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10).toBool();

    if (proform_studio || proform_tdf_10)
        return true;
    else
        return false;
}

resistance_t proformbike::pelotonToBikeResistance(int pelotonResistance) {
    if (pelotonResistance <= 10) {
        return 1;
    }
    if (pelotonResistance <= 20) {
        return 2;
    }
    if (pelotonResistance <= 25) {
        return 3;
    }
    if (pelotonResistance <= 30) {
        return 4;
    }
    if (pelotonResistance <= 35) {
        return 5;
    }
    if (pelotonResistance <= 40) {
        return 6;
    }
    if (pelotonResistance <= 45) {
        return 7;
    }
    if (pelotonResistance <= 50) {
        return 8;
    }
    if (pelotonResistance <= 55) {
        return 9;
    }
    if (pelotonResistance <= 60) {
        return 10;
    }
    if (pelotonResistance <= 65) {
        return 11;
    }
    if (pelotonResistance <= 70) {
        return 12;
    }
    if (pelotonResistance <= 75) {
        return 13;
    }
    if (pelotonResistance <= 80) {
        return 14;
    }
    if (pelotonResistance <= 85) {
        return 15;
    }
    if (pelotonResistance <= 100) {
        return 16;
    }
    return Resistance.value();
}

void proformbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void proformbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();
    bool proform_studio = settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool();
    bool proform_tdf_10 = settings.value(QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10).toBool();
    bool proform_tdf_jonseed_watt =
        settings.value(QZSettings::proform_tdf_jonseed_watt, QZSettings::default_proform_tdf_jonseed_watt).toBool();
    bool nordictrack_gx_2_7 =
        settings.value(QZSettings::nordictrack_gx_2_7, QZSettings::default_nordictrack_gx_2_7).toBool();
    bool proform_hybrid_trainer_PFEL03815 =
        settings
            .value(QZSettings::proform_hybrid_trainer_PFEL03815, QZSettings::default_proform_hybrid_trainer_PFEL03815)
            .toBool();
    bool proform_bike_sb = settings.value(QZSettings::proform_bike_sb, QZSettings::default_proform_bike_sb).toBool();
    bool proform_bike_PFEVEX71316_1 =
        settings.value(QZSettings::proform_bike_PFEVEX71316_1, QZSettings::default_proform_bike_PFEVEX71316_1).toBool();
    bool proform_bike_225_csx = settings.value(QZSettings::proform_bike_225_csx, QZSettings::default_proform_bike_225_csx).toBool();
    bool proform_bike_325_csx = settings.value(QZSettings::proform_bike_325_csx, QZSettings::default_proform_bike_325_csx).toBool();
    bool nordictrack_GX4_5_bike =
        settings.value(QZSettings::nordictrack_GX4_5_bike, QZSettings::default_nordictrack_GX4_5_bike).toBool();
    double cadence_gain = settings.value(QZSettings::cadence_gain, QZSettings::default_cadence_gain).toDouble();
    double cadence_offset = settings.value(QZSettings::cadence_offset, QZSettings::default_cadence_offset).toDouble();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));

    lastPacket = newValue;

    if (proform_studio || proform_tdf_10) {
        if (newValue.length() != 20 ||
            // frames with power
            (newValue.at(0) != 0x00 && newValue.at(0) != 0x01) || newValue.at(1) != 0x12 ||
            (newValue.at(0) == 0x00 &&
             (newValue.at(2) != 0x01 || newValue.at(3) != 0x04 || newValue.at(4) != 0x02 || newValue.at(5) != 0x2c))) {
            return;
        }

        if (newValue.at(0) == 0x00) {
            m_watts = ((uint16_t)(((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t)newValue.at(14)));
            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = ((double)((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12))) /
                         100.0);
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }

            double incline =
                ((double)((int16_t)(((int8_t)newValue.at(11)) << 8) + (int16_t)((uint8_t)newValue.at(10))) / 100.0);

            if ((uint16_t)(qAbs(incline) * 10) % 5 == 0) {
                Inclination = incline;
                emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
            } else {
                emit debug(QStringLiteral("Filtering bad inclination"));
            }
        } else {

            Resistance = newValue.at(19);
            m_pelotonResistance = (100 / 32) * Resistance.value();
            emit resistanceRead(Resistance.value());

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = (((uint8_t)newValue.at(2)) * cadence_gain) + cadence_offset;
            }
        }
    } else if (proform_bike_PFEVEX71316_1) {
        if (newValue.length() == 20 && newValue.at(0) == 0x00 && newValue.at(1) == 0x12 && newValue.at(2) == 0x01 &&
            newValue.at(3) == 0x04 && newValue.at(4) == 0x02 && newValue.at(5) == 0x2f && newValue.at(6) == 0x08 &&
            newValue.at(7) == 0x2f && newValue.at(8) == 0x02 && newValue.at(9) == 0x02 && newValue.at(10) == 0x00 &&
            newValue.at(11) == 0x00 && newValue.at(14) == 0x5a) {
            m_watt = ((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12)));
        } else if (newValue.length() == 20 && newValue.at(0) == 0x00 && newValue.at(1) == 0x12 &&
                   newValue.at(2) == 0x01 && newValue.at(3) == 0x04 && newValue.at(4) == 0x02 &&
                   newValue.at(5) == 0x2f && newValue.at(6) == 0x08 && newValue.at(7) == 0x2f &&
                   newValue.at(8) == 0x02 && newValue.at(9) == 0x02 && newValue.at(10) != 0x00 &&
                   newValue.at(11) != 0x00 && newValue.at(14) == 0x5a) {

            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = ((double)((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12))) /
                         100.0);
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }

            double incline =
                ((double)((int16_t)(((int8_t)newValue.at(11)) << 8) + (int16_t)((uint8_t)newValue.at(10))) / 100.0);

            if ((uint16_t)(qAbs(incline) * 10) % 5 == 0) {
                Inclination = incline;
                emit debug(QStringLiteral("Current Inclination: ") + QString::number(incline));
            } else {
                emit debug(QStringLiteral("Filtering bad inclination"));
            }

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = (((uint8_t)newValue.at(18)) * cadence_gain) + cadence_offset;
            }
        }
    } else {

        if (newValue.length() != 20 || newValue.at(0) != 0x00 || newValue.at(1) != 0x12 || newValue.at(2) != 0x01 ||
            newValue.at(3) != 0x04 ||
            (((uint8_t)newValue.at(12)) == 0xFF && ((uint8_t)newValue.at(13)) == 0xFF &&
             ((uint8_t)newValue.at(14)) == 0xFF && ((uint8_t)newValue.at(15)) == 0xFF &&
             ((uint8_t)newValue.at(16)) == 0xFF && ((uint8_t)newValue.at(17)) == 0xFF &&
             ((uint8_t)newValue.at(18)) == 0xFF && ((uint8_t)newValue.at(19)) == 0xFF)) {
            return;
        }

        // wattage needs to be calculate always as a local variable because it's used as a filter for strange values
        // from proform even if the proform_tdf_jonseed_watt is ON.
        m_watts = ((uint16_t)(((uint8_t)newValue.at(13)) << 8) + (uint16_t)((uint8_t)newValue.at(12)));

        // filter some strange values from proform
        if (m_watts > 3000) {
            m_watts = 0;
        } else {
            if(nordictrack_GX4_5_bike) {
                switch ((uint8_t)newValue.at(11)) {
                case 0:
                case 1:
                    Resistance = 1;
                    m_pelotonResistance = 1;
                    break;
                case 2:
                    Resistance = 2;
                    m_pelotonResistance = 4;
                    break;                 
                case 4:
                    Resistance = 3;
                    m_pelotonResistance = 9;
                    break;                 
                case 6:
                    Resistance = 4;
                    m_pelotonResistance = 13;
                    break;                  
                case 7:
                    Resistance = 5;
                    m_pelotonResistance = 18;
                    break;                  
                case 9:
                    Resistance = 6;
                    m_pelotonResistance = 23;
                    break;                   
                case 0x0a:
                    Resistance = 7;
                    m_pelotonResistance = 27;
                    break;                  
                case 0x0c:
                    Resistance = 8;
                    m_pelotonResistance = 32;
                    break;                  
                case 0x0d:
                    Resistance = 9;
                    m_pelotonResistance = 37;
                    break;                  
                case 0x0f:
                    Resistance = 10;
                    m_pelotonResistance = 42;
                    break;                  
                case 0x11:
                    Resistance = 11;
                    m_pelotonResistance = 46;
                    break;                  
                case 0x12:
                    Resistance = 12;
                    m_pelotonResistance = 50;
                    break;                  
                case 0x14:
                    Resistance = 13;
                    m_pelotonResistance = 55;
                    break;                  
                case 0x15:
                    Resistance = 14;
                    m_pelotonResistance = 59;
                    break;                  
                case 0x17:
                    Resistance = 15;
                    m_pelotonResistance = 63;
                    break;                  
                case 0x18:
                    Resistance = 16;
                    m_pelotonResistance = 68;
                    break;                  
                case 0x1a:
                    Resistance = 17;
                    m_pelotonResistance = 73;
                    break;                  
                case 0x1b:
                    Resistance = 18;
                    m_pelotonResistance = 77;
                    break;                  
                case 0x1d:
                    Resistance = 19;
                    m_pelotonResistance = 82;
                    break;                  
                case 0x1f:
                    Resistance = 20;
                    m_pelotonResistance = 86;
                    break;                  
                case 0x20:
                    Resistance = 21;
                    m_pelotonResistance = 90;
                    break;                  
                case 0x22:
                    Resistance = 22;
                    m_pelotonResistance = 93;
                    break;                 
                case 0x23:
                    Resistance = 23;
                    m_pelotonResistance = 95;
                    break;                  
                case 0x25:
                    Resistance = 24;
                    m_pelotonResistance = 98;
                    break;                  
                case 0x26:
                    Resistance = 25;
                    m_pelotonResistance = 100;
                    break;                   
                }
            } else if(proform_bike_325_csx) {
                switch ((uint8_t)newValue.at(11)) {
                case 0:
                case 1:
                    Resistance = 1;
                    m_pelotonResistance = 1;
                    break;
                case 3:
                    Resistance = 2;
                    m_pelotonResistance = 4;
                    break;                 
                case 5:
                    Resistance = 3;
                    m_pelotonResistance = 9;
                    break;                 
                case 6:
                    Resistance = 4;
                    m_pelotonResistance = 13;
                    break;                  
                case 7:
                case 8:
                    Resistance = 5;
                    m_pelotonResistance = 18;
                    break;                  
                case 0x0a:
                    Resistance = 6;
                    m_pelotonResistance = 23;
                    break;                   
                case 0x0c:
                    Resistance = 7;
                    m_pelotonResistance = 27;
                    break;                  
                case 0x0e:
                    Resistance = 8;
                    m_pelotonResistance = 32;
                    break;                  
                case 0x0f:
                    Resistance = 9;
                    m_pelotonResistance = 37;
                    break;                  
                case 0x11:
                    Resistance = 10;
                    m_pelotonResistance = 42;
                    break;                  
                case 0x13:
                    Resistance = 11;
                    m_pelotonResistance = 46;
                    break;                  
                case 0x15:
                    Resistance = 12;
                    m_pelotonResistance = 50;
                    break;                  
                case 0x16:
                    Resistance = 13;
                    m_pelotonResistance = 55;
                    break;                  
                case 0x18:
                    Resistance = 14;
                    m_pelotonResistance = 59;
                    break;                  
                case 0x1a:
                    Resistance = 15;
                    m_pelotonResistance = 63;
                    break;                  
                case 0x1c:
                    Resistance = 16;
                    m_pelotonResistance = 68;
                    break;                  
                case 0x1e:
                    Resistance = 17;
                    m_pelotonResistance = 73;
                    break;                  
                case 0x1f:
                    Resistance = 18;
                    m_pelotonResistance = 77;
                    break;                  
                case 0x21:
                    Resistance = 19;
                    m_pelotonResistance = 82;
                    break;                  
                case 0x23:
                    Resistance = 20;
                    m_pelotonResistance = 86;
                    break;                  
                case 0x25:
                    Resistance = 21;
                    m_pelotonResistance = 90;
                    break;                  
                case 0x26:
                    Resistance = 22;
                    m_pelotonResistance = 100;
                    break;                 
                }                                                                                                                                                                                                                                                                                                                                                                                                                                                                      
            } else if (proform_hybrid_trainer_PFEL03815 || proform_bike_sb) {
                switch ((uint8_t)newValue.at(11)) {
                case 0:
                    Resistance = 0;
                    m_pelotonResistance = 0;
                    break;
                case 2:
                    Resistance = 1;
                    m_pelotonResistance = 10;
                    break;
                case 4:
                    Resistance = 2;
                    m_pelotonResistance = 20;
                    break;
                case 7:
                case 8:
                    Resistance = 3;
                    m_pelotonResistance = 25;
                    break;
                case 9:
                    Resistance = 4;
                    m_pelotonResistance = 30;
                    break;
                case 0xb:
                case 0xc:
                    Resistance = 5;
                    m_pelotonResistance = 35;
                    break;
                case 0xe:
                    Resistance = 6;
                    m_pelotonResistance = 40;
                    break;
                case 0x10:
                case 0x11:
                    Resistance = 7;
                    m_pelotonResistance = 45;
                    break;
                case 0x13:
                    Resistance = 8;
                    m_pelotonResistance = 50;
                    break;
                case 0x15:
                    Resistance = 9;
                    m_pelotonResistance = 55;
                    break;
                case 0x18:
                    Resistance = 10;
                    m_pelotonResistance = 60;
                    break;
                case 0x1a:
                    Resistance = 11;
                    m_pelotonResistance = 65;
                    break;
                case 0x1d:
                    Resistance = 12;
                    m_pelotonResistance = 70;
                    break;
                case 0x1f:
                    Resistance = 13;
                    m_pelotonResistance = 80;
                    break;
                case 0x21:
                case 0x22:
                    Resistance = 14;
                    m_pelotonResistance = 90;
                    break;
                case 0x24:
                    Resistance = 15;
                    m_pelotonResistance = 95;
                    break;
                case 0x26:
                case 0x27:
                    Resistance = 16;
                    m_pelotonResistance = 100;
                    break;
                }
            } else if (!nordictrack_gx_2_7) {
                switch ((uint8_t)newValue.at(11)) {
                case 0x00:
                case 0x02:
                    Resistance = 1;
                    m_pelotonResistance = 10;
                    break;
                case 0x04:
                    Resistance = 2;
                    m_pelotonResistance = 20;
                    break;
                case 0x07:
                    Resistance = 3;
                    m_pelotonResistance = 25;
                    break;
                case 0x09:
                    Resistance = 4;
                    m_pelotonResistance = 30;
                    break;
                case 0x0b:
                case 0x0c:
                    Resistance = 5;
                    m_pelotonResistance = 35;
                    break;
                case 0x0e:
                    Resistance = 6;
                    m_pelotonResistance = 40;
                    break;
                case 0x10:
                case 0x11:
                    Resistance = 7;
                    m_pelotonResistance = 45;
                    break;
                case 0x13:
                    Resistance = 8;
                    m_pelotonResistance = 50;
                    break;
                case 0x15:
                    Resistance = 9;
                    m_pelotonResistance = 55;
                    break;
                case 0x18:
                    Resistance = 10;
                    m_pelotonResistance = 60;
                    break;
                case 0x1a:
                    Resistance = 11;
                    m_pelotonResistance = 65;
                    break;
                case 0x1d:
                    Resistance = 12;
                    m_pelotonResistance = 70;
                    break;
                case 0x1f:
                    Resistance = 13;
                    m_pelotonResistance = 75;
                    break;
                case 0x21:
                case 0x22:
                    Resistance = 14;
                    m_pelotonResistance = 80;
                    break;
                case 0x24:
                    Resistance = 15;
                    m_pelotonResistance = 85;
                    break;
                case 0x26:
                case 0x27:
                    Resistance = 16;
                    m_pelotonResistance = 100;
                    break;
                    /* when the proform bike is changing the resistance, it sends some strange values, so i'm keeping
                    the last good one default: Resistance = 0; m_pelotonResistance = 0; break;
                    */
                }
            } else {
                switch ((uint8_t)newValue.at(11)) {
                case 0x00:
                case 0x01:
                    Resistance = 1;
                    m_pelotonResistance = 10;
                    break;
                case 0x03:
                    Resistance = 2;
                    m_pelotonResistance = 20;
                    break;
                case 0x05:
                    Resistance = 3;
                    m_pelotonResistance = 25;
                    break;
                case 0x07:
                    Resistance = 4;
                    m_pelotonResistance = 30;
                    break;
                case 0x09:
                    Resistance = 5;
                    m_pelotonResistance = 33;
                    break;
                case 0x0b:
                    Resistance = 6;
                    m_pelotonResistance = 35;
                    break;
                case 0x0d:
                    Resistance = 7;
                    m_pelotonResistance = 38;
                    break;
                case 0x0f:
                    Resistance = 8;
                    m_pelotonResistance = 40;
                    break;
                case 0x11:
                    Resistance = 9;
                    m_pelotonResistance = 45;
                    break;
                case 0x13:
                    Resistance = 10;
                    m_pelotonResistance = 50;
                    break;
                case 0x15:
                    Resistance = 11;
                    m_pelotonResistance = 55;
                    break;
                case 0x17:
                    Resistance = 12;
                    m_pelotonResistance = 60;
                    break;
                case 0x19:
                    Resistance = 13;
                    m_pelotonResistance = 63;
                    break;
                case 0x1b:
                    Resistance = 14;
                    m_pelotonResistance = 65;
                    break;
                case 0x1d:
                    Resistance = 15;
                    m_pelotonResistance = 68;
                case 0x1f:
                    Resistance = 16;
                    m_pelotonResistance = 70;
                    break;
                case 0x21:
                    Resistance = 17;
                    m_pelotonResistance = 75;
                    break;
                case 0x22:
                case 0x23:
                    Resistance = 18;
                    m_pelotonResistance = 80;
                    break;
                case 0x24:
                    Resistance = 19;
                    m_pelotonResistance = 85;
                    break;
                case 0x26:
                case 0x27:
                    Resistance = 20;
                    m_pelotonResistance = 100;
                    break;
                }
            }
            emit resistanceRead(Resistance.value());

            if (proform_tdf_jonseed_watt) {
                m_watts = wattsFromResistance(Resistance.value());
                if (m_watts > 3000)
                    m_watts = 0;
            }

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {
                Cadence = (((uint8_t)newValue.at(18)) * cadence_gain) + cadence_offset;
            }

            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = (settings.value(QZSettings::proform_wheel_ratio, QZSettings::default_proform_wheel_ratio)
                             .toDouble()) *
                        ((double)Cadence.value());
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
        }
    }
    if (watts())
        KCal += ((((0.048 * ((double)watts()) + 1.19) *
                   settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                  200.0) /
                 (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                                now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                  // kg * 3.5) / 200 ) / 60
    // KCal = (((uint16_t)((uint8_t)newValue.at(15)) << 8) + (uint16_t)((uint8_t) newValue.at(14)));
    Distance += ((Speed.value() / 3600000.0) *
                 ((double)lastRefreshCharacteristicChanged.msecsTo(now)));

    if (Cadence.value() > 0) {
        CrankRevs++;
        LastCrankEventTime += (uint16_t)(1024.0 / (((double)(Cadence.value())) / 60.0));
    }

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

#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
    bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
    bool ios_peloton_workaround =
        settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
    if (ios_peloton_workaround && cadence && h && firstStateChanged) {
        h->virtualbike_setCadence(currentCrankRevolutions(), lastCrankEventTime());
        h->virtualbike_setHeartRate((uint8_t)metrics_override_heartrate());
    }
#endif
#endif

    emit debug(QStringLiteral("Current Resistance: ") + QString::number(Resistance.value()));
    emit debug(QStringLiteral("Current Speed: ") + QString::number(Speed.value()));
    emit debug(QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value()));
    emit debug(QStringLiteral("Current Cadence: ") + QString::number(Cadence.value()));
    // debug("Current Distance: " + QString::number(distance));
    emit debug(QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs));
    emit debug(QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime));
    emit debug(QStringLiteral("Current Watt: ") + QString::number(watts()));

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

void proformbike::btinit() {

    QSettings settings;
    bool nordictrack_gx_2_7 =
        settings.value(QZSettings::nordictrack_gx_2_7, QZSettings::default_nordictrack_gx_2_7).toBool();
    bool proform_cycle_trainer_400 =
        settings.value(QZSettings::proform_cycle_trainer_400, QZSettings::default_proform_cycle_trainer_400).toBool();
    bool proform_cycle_trainer_300_ci =
        settings.value(QZSettings::proform_cycle_trainer_300_ci, QZSettings::default_proform_cycle_trainer_300_ci).toBool();        
    bool proform_hybrid_trainer_PFEL03815 =
        settings
            .value(QZSettings::proform_hybrid_trainer_PFEL03815, QZSettings::default_proform_hybrid_trainer_PFEL03815)
            .toBool();
    bool proform_bike_sb = settings.value(QZSettings::proform_bike_sb, QZSettings::default_proform_bike_sb).toBool();
    bool proform_bike_225_csx = settings.value(QZSettings::proform_bike_225_csx, QZSettings::default_proform_bike_225_csx).toBool();
    bool proform_bike_325_csx = settings.value(QZSettings::proform_bike_325_csx, QZSettings::default_proform_bike_325_csx).toBool();
    bool nordictrack_GX4_5_bike =
        settings.value(QZSettings::nordictrack_GX4_5_bike, QZSettings::default_nordictrack_GX4_5_bike).toBool();

    if(nordictrack_GX4_5_bike)
        max_resistance = 25;

    if (settings.value(QZSettings::proform_studio, QZSettings::default_proform_studio).toBool()) {

        max_resistance = 32;

        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x08, 0x04, 0x80, 0x8c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x08, 0x04, 0x88, 0x94,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};

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

        uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x08, 0x28, 0x90, 0x04,
                                0x00, 0x68, 0xec, 0x62, 0xee, 0x6c, 0xf8, 0x7e, 0x0a, 0x80};
        uint8_t initData11[] = {0x01, 0x12, 0x14, 0xaa, 0x36, 0xc4, 0x70, 0xe6, 0x82, 0x38,
                                0xdc, 0x72, 0x1e, 0xbc, 0x48, 0xee, 0xba, 0x50, 0xe4, 0xba};
        uint8_t initData12[] = {0xff, 0x08, 0x46, 0x14, 0xe0, 0x88, 0x02, 0x00, 0x00, 0x66,
                                0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(400);
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(400);
    } else if (settings.value(QZSettings::proform_tdf_10, QZSettings::default_proform_tdf_10).toBool() ||
               settings.value(QZSettings::proform_bike_PFEVEX71316_1, QZSettings::default_proform_bike_PFEVEX71316_1)
                   .toBool()) {
        max_resistance = 26;

        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x08, 0x04, 0x80, 0x8c,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x08, 0x04, 0x88, 0x94,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};

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

        if (settings.value(QZSettings::proform_bike_PFEVEX71316_1, QZSettings::default_proform_bike_PFEVEX71316_1)
                .toBool()) {
            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x08, 0x28, 0x90, 0x04,
                                    0x00, 0xc1, 0x58, 0xfd, 0x90, 0x31, 0xd0, 0x75, 0x28, 0xc1};
            uint8_t initData11[] = {0x01, 0x12, 0x78, 0x2d, 0xc0, 0x71, 0x20, 0xf5, 0x88, 0x41,
                                    0x18, 0xdd, 0x90, 0x51, 0x10, 0xd5, 0x88, 0x41, 0x38, 0xed};
            uint8_t initData12[] = {0xff, 0x08, 0xa0, 0x91, 0x40, 0x88, 0x02, 0x00, 0x00, 0x21,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else {
            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x08, 0x28, 0x90, 0x04,
                                    0x00, 0x40, 0x0c, 0xca, 0x9e, 0x64, 0x38, 0xf6, 0xda, 0xa8};
            uint8_t initData11[] = {0x01, 0x12, 0x74, 0x42, 0x26, 0x0c, 0xf0, 0xae, 0xb2, 0x90,
                                    0x7c, 0x5a, 0x2e, 0x34, 0x08, 0xe6, 0xea, 0xf8, 0xc4, 0xd2};
            uint8_t initData12[] = {0xff, 0x08, 0xd6, 0xdc, 0xe0, 0x88, 0x02, 0x00, 0x00, 0x0e,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        }
    } else if (settings.value(QZSettings::proform_bike_325_csx, QZSettings::default_proform_bike_325_csx).toBool()) {
        max_resistance = 22;

        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData3[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x80, 0x8b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData5[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData6[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x88, 0x93, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData7[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData8[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00, 0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData8, sizeof(initData8), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData9[] = {0xfe, 0x02, 0x0a, 0x02};
        writeCharacteristic(initData9, sizeof(initData9), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData10[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00, 0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData11[] = {0xfe, 0x02, 0x08, 0x02};
        writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData12[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData13[] = {0xfe, 0x02, 0x2c, 0x04};
        writeCharacteristic(initData13, sizeof(initData13), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData14[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x04, 0x00, 0xd4, 0x7c, 0x1e, 0xc6, 0x60, 0x18, 0xb2, 0x62, 0x1c};
        writeCharacteristic(initData14, sizeof(initData14), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData15[] = {0x01, 0x12, 0xc4, 0x76, 0x3e, 0xe8, 0xb0, 0x4a, 0x0a, 0xc4, 0x8c, 0x4e, 0x36, 0xf0, 0xa8, 0x62, 0x52, 0x0c, 0xf4, 0xa6};
        writeCharacteristic(initData15, sizeof(initData15), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData16[] = {0xff, 0x08, 0x8e, 0x78, 0x60, 0x80, 0x02, 0x00, 0x00, 0xb1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData16, sizeof(initData16), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData17[] = {0xfe, 0x02, 0x19, 0x03};
        writeCharacteristic(initData17, sizeof(initData17), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData18[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData18, sizeof(initData18), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData19[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData19, sizeof(initData19), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData20[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData20, sizeof(initData20), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData21[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData21, sizeof(initData21), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData22[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData22, sizeof(initData22), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData23[] = {0xfe, 0x02, 0x17, 0x03};
        writeCharacteristic(initData23, sizeof(initData23), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData24[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
        writeCharacteristic(initData24, sizeof(initData24), QStringLiteral("init"), false, false);
        QThread::msleep(400);


        uint8_t initData25[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        writeCharacteristic(initData25, sizeof(initData25), QStringLiteral("init"), false, false);
        QThread::msleep(400);

    } else {

        uint8_t initData1[] = {0xfe, 0x02, 0x08, 0x02};
        uint8_t initData2[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x81, 0x87,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData3[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x80, 0x8b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData4[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x07, 0x04, 0x88, 0x93,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData5[] = {0xfe, 0x02, 0x0a, 0x02};
        uint8_t initData6[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x82, 0x00,
                               0x00, 0x8a, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData7[] = {0xff, 0x0a, 0x02, 0x04, 0x02, 0x06, 0x02, 0x06, 0x84, 0x00,
                               0x00, 0x8c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData8[] = {0xff, 0x08, 0x02, 0x04, 0x02, 0x04, 0x02, 0x04, 0x95, 0x9b,
                               0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
        uint8_t initData9[] = {0xfe, 0x02, 0x2c, 0x04};

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

        if (settings.value(QZSettings::proform_tour_de_france_clc, QZSettings::default_proform_tour_de_france_clc)
                .toBool()) {

            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                    0x01, 0xb9, 0xf8, 0x45, 0x80, 0xc9, 0x10, 0x6d, 0xb8, 0x09};
            uint8_t initData11[] = {0x01, 0x12, 0x58, 0xa5, 0xf0, 0x59, 0xa0, 0x1d, 0x78, 0xd9,
                                    0x38, 0x85, 0xe0, 0x49, 0xd0, 0x2d, 0xb8, 0x09, 0x98, 0xe5};
            uint8_t initData12[] = {0xff, 0x08, 0x70, 0xf9, 0x40, 0x98, 0x02, 0x00, 0x00, 0xfc,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (proform_bike_sb) {
            max_resistance = 16;

            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                    0x01, 0x86, 0x64, 0x38, 0x1a, 0xfa, 0xe8, 0xcc, 0xa6, 0x9e};
            uint8_t initData11[] = {0x01, 0x12, 0x8c, 0x60, 0x52, 0x42, 0x50, 0x24, 0x3e, 0x36,
                                    0x34, 0x08, 0x0a, 0x0a, 0x38, 0x3c, 0x36, 0x2e, 0x5c, 0x50};
            uint8_t initData12[] = {0xff, 0x08, 0x42, 0x72, 0xa0, 0x88, 0x02, 0x00, 0x00, 0x0f,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (nordictrack_gx_2_7) {
            max_resistance = 20;

            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x04,
                                    0x00, 0x9e, 0x84, 0x60, 0x4a, 0x32, 0x28, 0x04, 0xf6, 0xe6};
            uint8_t initData11[] = {0x01, 0x12, 0xec, 0xd8, 0xc2, 0xca, 0xd0, 0xac, 0xae, 0xae,
                                    0xd4, 0xd0, 0xda, 0xc2, 0xf8, 0xf4, 0xe6, 0x16, 0x3c, 0x28};
            uint8_t initData12[] = {0xff, 0x08, 0x52, 0x7a, 0xa0, 0x80, 0x02, 0x00, 0x00, 0x6b,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (proform_cycle_trainer_400) {
            max_resistance = 16;

            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                    0x01, 0x52, 0x74, 0x94, 0xb2, 0xde, 0x08, 0x20, 0x5e, 0x8a};
            uint8_t initData11[] = {0x01, 0x12, 0xbc, 0xec, 0x1a, 0x46, 0x90, 0xa8, 0xe6, 0x22,
                                    0x64, 0xa4, 0xe2, 0x2e, 0x98, 0xd0, 0x0e, 0x7a, 0xac, 0x1c};
            uint8_t initData12[] = {0xff, 0x08, 0x4a, 0xb6, 0x20, 0x98, 0x02, 0x00, 0x00, 0x93,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (proform_cycle_trainer_300_ci) {
            max_resistance = 16;

            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x04,
                                    0x00, 0x10, 0xcc, 0x7a, 0x3e, 0xf4, 0xb8, 0x66, 0x3a, 0xf8};
            uint8_t initData11[] = {0x01, 0x12, 0xb4, 0x72, 0x46, 0x1c, 0xf0, 0xbe, 0x92, 0x40,
                                    0x3c, 0xea, 0xce, 0xa4, 0x88, 0x76, 0x4a, 0x28, 0x04, 0xe2};
            uint8_t initData12[] = {0xff, 0x08, 0xf6, 0xcc, 0xe0, 0x80, 0x02, 0x00, 0x00, 0xb5, 
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);            

            uint8_t noOpData0[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData1[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData2[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData3[] = {0xfe, 0x02, 0x19, 0x03};
            uint8_t noOpData4[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x15, 0x07, 0x15, 0x02, 0x0e, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData5[] = {0xff, 0x07, 0x00, 0x00, 0x00, 0x10, 0x01, 0x00, 0x3d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData6[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData7[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00, 0x0d, 0x00, 0x10, 0x00, 0xc0, 0x1c, 0x4c, 0x00, 0x00, 0xe0};
            uint8_t noOpData8[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x10, 0x51, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData9[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData11[] = {0xff, 0x05, 0x00, 0x80, 0x01, 0x00, 0xa9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData12[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData13[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData14[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData15[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData16[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x0c, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData17[] = {0xff, 0x05, 0x00, 0x80, 0x00, 0x00, 0xa8, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData18[] = {0xfe, 0x02, 0x17, 0x03};
            uint8_t noOpData19[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x13, 0x07, 0x13, 0x02, 0x00, 0x0d, 0x3c, 0x9e, 0x31, 0x00, 0x00, 0x40, 0x40, 0x00, 0x80};
            uint8_t noOpData20[] = {0xff, 0x05, 0x00, 0x00, 0x00, 0x85, 0xb9, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
            uint8_t noOpData21[] = {0xfe, 0x02, 0x11, 0x02};
            uint8_t noOpData22[] = {0xff, 0x11, 0x02, 0x04, 0x02, 0x0d, 0x07, 0x0d, 0x02, 0x00, 0x07, 0xbc, 0x90, 0x70, 0x00, 0x00, 0x00, 0x40, 0x19, 0x00};

            writeCharacteristic(noOpData0, sizeof(noOpData0), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("init"), false, false);
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

            writeCharacteristic(noOpData7, sizeof(noOpData7), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData8, sizeof(noOpData8), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData9, sizeof(noOpData9), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData10, sizeof(noOpData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData11, sizeof(noOpData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData12, sizeof(noOpData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData13, sizeof(noOpData13), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData14, sizeof(noOpData14), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData15, sizeof(noOpData15), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData16, sizeof(noOpData16), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData17, sizeof(noOpData17), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData18, sizeof(noOpData18), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData19, sizeof(noOpData19), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData20, sizeof(noOpData20), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData21, sizeof(noOpData21), QStringLiteral("init"), false, false);
            QThread::msleep(400);

            writeCharacteristic(noOpData22, sizeof(noOpData22), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (proform_bike_225_csx) {
            max_resistance = 10;
            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07, 0x01, 0xd2, 0x74, 0x14, 0xb2, 0x5e, 0x08, 0xa0, 0x5e, 0x0a};
            uint8_t initData11[] = {0x01, 0x12, 0xbc, 0x6c, 0x1a, 0xc6, 0x90, 0x28, 0xe6, 0xa2, 0x64, 0x24, 0xe2, 0xae, 0x98, 0x50, 0x0e, 0xfa, 0xac, 0x9c};
            uint8_t initData12[] = {0xff, 0x08, 0x4a, 0x36, 0x20, 0x98, 0x02, 0x00, 0x00, 0x13, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else if (proform_hybrid_trainer_PFEL03815) {
            max_resistance = 16;
            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x04,
                                    0x00, 0xb8, 0xac, 0x92, 0x8e, 0x7c, 0x78, 0x6e, 0x6a, 0x50};
            uint8_t initData11[] = {0x01, 0x12, 0x54, 0x5a, 0x56, 0x54, 0x70, 0x76, 0x62, 0x68,
                                    0x9c, 0x82, 0xbe, 0xac, 0xc8, 0xfe, 0x1a, 0x00, 0x24, 0x4a};
            uint8_t initData12[] = {0xff, 0x08, 0x66, 0x84, 0xe0, 0x80, 0x02, 0x00, 0x00, 0x8d,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        } else {

            uint8_t initData10[] = {0x00, 0x12, 0x02, 0x04, 0x02, 0x28, 0x07, 0x28, 0x90, 0x07,
                                    0x01, 0x10, 0xcc, 0x7a, 0x3e, 0xf4, 0xb8, 0x66, 0x3a, 0xf8};
            uint8_t initData11[] = {0x01, 0x12, 0xb4, 0x72, 0x46, 0x1c, 0xf0, 0xbe, 0x92, 0x40,
                                    0x3c, 0xea, 0xce, 0xa4, 0x88, 0x76, 0x4a, 0x28, 0x04, 0xe2};
            uint8_t initData12[] = {0xff, 0x08, 0xf6, 0xcc, 0xe0, 0x98, 0x02, 0x00, 0x00, 0xd1,
                                    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

            writeCharacteristic(initData10, sizeof(initData10), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData11, sizeof(initData11), QStringLiteral("init"), false, false);
            QThread::msleep(400);
            writeCharacteristic(initData12, sizeof(initData12), QStringLiteral("init"), false, false);
            QThread::msleep(400);
        }
    }

    initDone = true;
}

void proformbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("00001534-1412-efde-1523-785feabcd123"));
    QBluetoothUuid _gattNotify1CharacteristicId(QStringLiteral("00001535-1412-efde-1523-785feabcd123"));

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if (state == QLowEnergyService::ServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &proformbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &proformbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &proformbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &proformbike::descriptorWritten);

        // ******************************************* virtual bike init *************************************
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
#ifdef Q_OS_IOS
#ifndef IO_UNDER_QT
            bool cadence =
                settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
            bool ios_peloton_workaround =
                settings.value(QZSettings::ios_peloton_workaround, QZSettings::default_ios_peloton_workaround).toBool();
            if (ios_peloton_workaround && cadence) {
                qDebug() << "ios_peloton_workaround activated!";
                h = new lockscreen();
                h->virtualbike_ios();
            } else
#endif
#endif
                if (virtual_device_enabled) {
                emit debug(QStringLiteral("creating virtual bike interface..."));
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&proformbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &proformbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);
    }
}

void proformbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' '));

    initRequest = true;
    emit connectedAndDiscovered();
}

void proformbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void proformbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    QBluetoothUuid _gattCommunicationChannelServiceId(QStringLiteral("00001533-1412-efde-1523-785feabcd123"));

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &proformbike::stateChanged);
    gattCommunicationChannelService->discoverDetails();
}

void proformbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("proformbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("proformbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void proformbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')');
    // if (device.name().startsWith(QStringLiteral("I_EB")))
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &proformbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &proformbike::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &proformbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &proformbike::controllerStateChanged);

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

bool proformbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t proformbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watts;
}

void proformbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}
