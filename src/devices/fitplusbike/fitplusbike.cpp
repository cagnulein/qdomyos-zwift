#include "fitplusbike.h"
#include "homeform.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <chrono>
#include <math.h>

#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#endif

using namespace std::chrono_literals;

#ifdef Q_OS_IOS
extern quint8 QZ_EnableDiscoveryCharsAndDescripttors;
#endif

fitplusbike::fitplusbike(bool noWriteResistance, bool noHeartService, int8_t bikeResistanceOffset,
                         double bikeResistanceGain) {
#ifdef Q_OS_IOS
    QSettings settings;
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();
    if (sportstech_sx600) {
        QZ_EnableDiscoveryCharsAndDescripttors = false;
    } else {
        QZ_EnableDiscoveryCharsAndDescripttors = true;
    }
#endif
    m_watt.setType(metric::METRIC_WATT, deviceType());
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->bikeResistanceGain = bikeResistanceGain;
    this->bikeResistanceOffset = bikeResistanceOffset;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &fitplusbike::update);
    refresh->start(200ms);
}

void fitplusbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;

    // if there are some crash here, maybe it's better to use 2 separate event for the characteristicChanged.
    // one for the resistance changed event (spontaneous), and one for the other ones.
    if (wait_for_response) {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (gattCommunicationChannelService->state() != QLowEnergyService::ServiceState::ServiceDiscovered ||
        m_control->state() == QLowEnergyController::UnconnectedState) {
        qDebug() << QStringLiteral("writeCharacteristic error because the connection is closed");
        return;
    }

    if (!gattWriteCharacteristic.isValid()) {
        qDebug() << QStringLiteral("gattWriteCharacteristic is invalid");
        return;
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    if (gattWriteCharacteristic.properties() & QLowEnergyCharacteristic::WriteNoResponse) {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer,
                                                             QLowEnergyService::WriteWithoutResponse);
    } else {
        gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, *writeBuffer);
    }

    if (!disable_log)
        qDebug() << QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info;

    loop.exec();
}

void fitplusbike::forceResistance(resistance_t requestResistance) {
    QSettings settings;
    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();
    requestResistanceCompleted = false;
    if (virtufit_etappe || merach_MRK || H9110_OSAKA) {
        if (requestResistance == 1) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x01, 0xf9, 0xb9, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 2) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x02, 0xf9, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 3) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x03, 0xfa, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 4) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x04, 0xfb, 0xbe, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 5) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x05, 0xfc, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 6) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x06, 0xfd, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 7) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x07, 0xfe, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 8) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x08, 0xff, 0xb6, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 9) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x09, 0x00, 0x48, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 10) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0a, 0x00, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 11) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0b, 0x01, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 12) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0c, 0x03, 0x4e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 13) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0d, 0x03, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 14) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0e, 0x05, 0x4a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 15) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0f, 0x05, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 16) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x10, 0x06, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 17) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x11, 0x07, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 18) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x12, 0x08, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 19) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x13, 0x09, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 20) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x14, 0x0a, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 21) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x15, 0x0b, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 22) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x16, 0x0c, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 23) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x17, 0x0d, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 24) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x18, 0x0e, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 25) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x19, 0x00, 0x58, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 26) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1a, 0x00, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 27) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1b, 0x00, 0x5a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 28) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1c, 0x00, 0x5d, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 29) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1d, 0x00, 0x5c, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 30) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1e, 0x00, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 31) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1f, 0x00, 0x5e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 32) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x20, 0x00, 0x61, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        }
    } else if (sportstech_sx600) {
        if (requestResistance == 1) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x00, 0x00, 0x41, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 2) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x02, 0x00, 0x43, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 3) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x03, 0xfa, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 4) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x04, 0xfb, 0xbe, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 5) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x05, 0x00, 0x44, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 6) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x06, 0xfd, 0xba, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 7) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x07, 0xfe, 0xb8, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 8) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x08, 0x00, 0x49, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 9) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x09, 0x00, 0x48, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 10) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0a, 0x00, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 11) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0b, 0x00, 0x4a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 12) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0c, 0x03, 0x4e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 13) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0d, 0x03, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 14) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0e, 0x00, 0x4f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 15) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x0f, 0x05, 0x4b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 16) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x10, 0x06, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 17) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x11, 0x00, 0x50, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 18) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x12, 0x08, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 19) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x13, 0x00, 0x52, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 20) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x14, 0x00, 0x55, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 21) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x15, 0x00, 0x54, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 22) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x16, 0x00, 0x57, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 23) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x17, 0x00, 0x56, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 24) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x18, 0x00, 0x59, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 25) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x19, 0x00, 0x58, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 26) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1a, 0x00, 0x59, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 27) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1b, 0x00, 0x5a, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 28) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1c, 0x00, 0x5b, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 29) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1d, 0x00, 0x5c, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 30) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1e, 0x00, 0x5f, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 31) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x1f, 0x00, 0x5e, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        } else if (requestResistance == 32) {
            uint8_t res[] = {0x02, 0x44, 0x05, 0x20, 0x00, 0x61, 0x03};
            writeCharacteristic(res, sizeof(res), "force resistance", false, true);
        }
    }
}

void fitplusbike::update() {

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
        update_metrics(false, watts());
        bool virtufit_etappe =
            settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
        bool sportstech_sx600 =
            settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

        if (virtufit_etappe || merach_MRK || sportstech_sx600 || H9110_OSAKA) {

        } else {
            m_watt = wattFromHR(false);
            qDebug() << QStringLiteral("Current Watt: ") + QString::number(m_watt.value());
        }

        // sending poll every 2 seconds
        if (sec1Update++ >= (2000 / refresh->interval())) {
            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (sportstech_sx600) {
            uint8_t noOpData[] = {0x02, 0x42, 0x42, 0x03};
            uint8_t noOpData1[] = {0x02, 0x02, 0x43, 0x01, 0x42, 0x03, 0x01, 0x03};
            uint8_t noOpData2[] = {0x02, 0x53, 0x9f, 0x02, 0x00, 0x00, 0xce, 0x03};
            uint8_t noOpData3[] = {0x02, 0x43, 0x01, 0x42, 0x03};
            uint8_t noOpData4[] = {0x02, 0x53, 0x9f, 0x02, 0x00, 0x00, 0xce, 0x03};
            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                break;
            case 1:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                break;
            case 2:
                writeCharacteristic(noOpData2, sizeof(noOpData2), QStringLiteral("noOp"), false, true);
                break;
            case 3:
                writeCharacteristic(noOpData3, sizeof(noOpData3), QStringLiteral("noOp"), false, true);
                break;
            default:
                writeCharacteristic(noOpData4, sizeof(noOpData4), QStringLiteral("noOp"), false, true);
                break;
            }

            counterPoll++;
            if (counterPoll > 4) {
                counterPoll = 0;
            }
        } else {
            uint8_t noOpData[] = {0x02, 0x42, 0x42, 0x03};
            uint8_t noOpData1[] = {0x02, 0x43, 0x01, 0x42, 0x03};
            switch (counterPoll) {
            case 0:
                writeCharacteristic(noOpData, sizeof(noOpData), QStringLiteral("noOp"), false, true);
                break;
            default:
                writeCharacteristic(noOpData1, sizeof(noOpData1), QStringLiteral("noOp"), false, true);
                break;
            }

            counterPoll++;
            if (counterPoll > 1) {
                counterPoll = 0;
            }
        }

        if (requestResistance != -1) {
            if (requestResistance > max_resistance) {
                requestResistance = max_resistance;
            } else if (requestResistance <= 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                qDebug() << QStringLiteral("writing resistance ") + QString::number(requestResistance);
                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        if (requestStart != -1) {
            qDebug() << QStringLiteral("starting...");

            // btinit();

            requestStart = -1;
            emit bikeStarted();
        }
        if (requestStop != -1) {
            qDebug() << QStringLiteral("stopping...");
            // writeCharacteristic(initDataF0C800B8, sizeof(initDataF0C800B8), "stop tape");
            requestStop = -1;
        }
    }
}

void fitplusbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    qDebug() << QStringLiteral("serviceDiscovered ") + gatt.toString();
}

void fitplusbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();
    // qDebug() << "characteristicChanged" << characteristic.uuid() << newValue << newValue.length();
    Q_UNUSED(characteristic);
    QSettings settings;
    QString heartRateBeltName =
        settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    qDebug() << QStringLiteral(" << ") + newValue.toHex(' ');

    lastPacket = newValue;

    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

    if (sportstech_sx600 && characteristic.uuid() == QBluetoothUuid((quint16)0x2AD2)) {
        bool disable_hr_frommachinery =
            settings.value(QZSettings::heart_ignore_builtin, QZSettings::default_heart_ignore_builtin).toBool();

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
            if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool()) {
                Speed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                  (uint16_t)((uint8_t)newValue.at(index)))) /
                        100.0;
            } else {
                Speed = metric::calculateSpeedFromPower(
                    watts(), Inclination.value(), Speed.value(),
                    fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
            }
            index += 2;
            qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
        }

        if (Flags.avgSpeed) {
            double avgSpeed;
            avgSpeed = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index)))) /
                       100.0;
            index += 2;
            qDebug() << QStringLiteral("Current Average Speed: ") + QString::number(avgSpeed);
        }

        if (Flags.instantCadence) {
            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled"))) {

                // this bike sent a cadence 1/10 of the real one
                Cadence = (((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                     (uint16_t)((uint8_t)newValue.at(index)))) /
                           2.0) *
                          settings
                              .value(QZSettings::horizon_gr7_cadence_multiplier,
                                     QZSettings::default_horizon_gr7_cadence_multiplier)
                              .toDouble();
            }
            index += 2;
            qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
        }

        if (Flags.avgCadence) {
            double avgCadence;
            avgCadence = (((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                    (uint16_t)((uint8_t)newValue.at(index)))) /
                          2.0) *
                         settings
                             .value(QZSettings::horizon_gr7_cadence_multiplier,
                                    QZSettings::default_horizon_gr7_cadence_multiplier)
                             .toDouble();
            index += 2;
            qDebug() << QStringLiteral("Current Average Cadence: ") + QString::number(avgCadence);
        }

        if (Flags.totDistance) {
            index += 3;
        } else {
        }

        qDebug() << QStringLiteral("Current Distance: ") + QString::number(Distance.value());

        if (Flags.resistanceLvl) {
            Resistance = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index)))) / 10.0;
            emit resistanceRead(Resistance.value());
            m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());
            index += 2;
            qDebug() << QStringLiteral("Current Resistance: ") + QString::number(Resistance.value());
        }

        if (Flags.instantPower) {
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                m_watt = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                   (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            qDebug() << (QStringLiteral("Current Watt: ") + QString::number(m_watt.value()));
        }

        if (Flags.avgPower) {
            double avgPower;
            avgPower = ((double)(((uint16_t)((uint8_t)newValue.at(index + 1)) << 8) |
                                 (uint16_t)((uint8_t)newValue.at(index))));
            index += 2;
            qDebug() << (QStringLiteral("Current Average Watt: ") + QString::number(avgPower));
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
            if (watts())
                KCal += ((((0.048 * ((double)watts()) + 1.19) *
                           settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
                          200.0) /
                         (60000.0 /
                          ((double)lastRefreshCharacteristicChanged.msecsTo(
                              now)))); //(( (0.048* Output in watts +1.19) * body weight in
                                                                // kg * 3.5) / 200 ) / 60
        }

        qDebug() << (QStringLiteral("Current KCal: ") + QString::number(KCal.value()));

#ifdef Q_OS_ANDROID
        if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
            Heart = (uint8_t)KeepAwakeHelper::heart();
        else
#endif
        {
            if (Flags.heartRate && !disable_hr_frommachinery && newValue.length() > index) {
                Heart = ((double)(((uint8_t)newValue.at(index))));
                // index += 1; // NOTE: clang-analyzer-deadcode.DeadStores
                qDebug() << (QStringLiteral("Current Heart: ") + QString::number(Heart.value()));
            } else {
                Flags.heartRate = false;
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
    } else if (virtufit_etappe || merach_MRK || H9110_OSAKA || (sportstech_sx600 && !gattCommunicationChannelServiceFTMS)) {
        if (newValue.length() != 15 && newValue.length() != 13)
            return;

        if (newValue.length() == 15) {
            resistance_t res = newValue.at(5);
            if (settings.value(QZSettings::gears_from_bike, QZSettings::default_gears_from_bike).toBool()) {
                qDebug() << QStringLiteral("gears_from_bike") << res << Resistance.value() << gears()
                         << lastRawRequestedResistanceValue << lastRequestedResistance().value() << requestResistance << requestResistanceCompleted;
                if (
                     // if the resistance is different from the previous one
                    res != qRound(Resistance.value()) &&
                    // and the last target resistance is different from the current one or there is no any pending last
                    // requested resistance
                    ((lastRequestedResistance().value() != res && lastRequestedResistance().value() != 0 && requestResistance == -1 && requestResistanceCompleted) ||
                     (lastRawRequestedResistanceValue == -1 && requestResistance == -1 && requestResistanceCompleted)) &&
                    // and the difference between the 2 resistances are less than 6
                    qRound(Resistance.value()) > 1 && qAbs(res - qRound(Resistance.value())) < 6) {

                    int8_t g = gears();
                    g += (res - qRound(Resistance.value()));
                    qDebug() << QStringLiteral("gears_from_bike APPLIED") << gears() << g;
                    lastRawRequestedResistanceValue = -1; // in order to avoid to change resistance with the setGears
                    setGears(g);
                }
            }
            requestResistanceCompleted = true;
            Resistance = res;
            emit resistanceRead(Resistance.value());
            if (merach_MRK || sportstech_sx600) {
                // if we change this, also change the wattsFromResistance function. We can create a standard function in
                // order to have all the costants in one place (I WANT MORE TIME!!!)
                double ac = 0.01243107769;
                double bc = 1.145964912;
                double cc = -23.50977444;

                double ar = 0.1469553975;
                double br = -5.841344538;
                double cr = 97.62165482;

                m_pelotonResistance =
                    (((sqrt(pow(br, 2.0) - 4.0 * ar *
                                               (cr - (m_watt.value() * 132.0 /
                                                      (ac * pow(Cadence.value(), 2.0) + bc * Cadence.value() + cc)))) -
                       br) /
                      (2.0 * ar)) *
                     settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
                    settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
            } else {
                m_pelotonResistance = bikeResistanceToPeloton(Resistance.value());
            }

            if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                    .toString()
                    .startsWith(QStringLiteral("Disabled")))
                Cadence = ((uint8_t)newValue.at(6));
            if (settings.value(QZSettings::power_sensor_name, QZSettings::default_power_sensor_name)
                    .toString()
                .startsWith(QStringLiteral("Disabled"))) {
                    m_watt = (double)((((uint8_t)newValue.at(10)) << 8) | ((uint8_t)newValue.at(9))) / 10.0;
            }

            /*if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool())
                Speed = (double)((((uint8_t)newValue.at(4)) << 10) | ((uint8_t)newValue.at(9))) / 100.0;
            else*/
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());

        } else if (newValue.length() == 13) {

            return;
        }

    } else {

        if (newValue.length() != 14) {
            return;
        }

        /*if ((uint8_t)(newValue.at(0)) != 0xf0 && (uint8_t)(newValue.at(1)) != 0xd1)
            return;*/

        Resistance = 1;
        m_pelotonResistance = 1;
        emit resistanceRead(Resistance.value());
        if (settings.value(QZSettings::cadence_sensor_name, QZSettings::default_cadence_sensor_name)
                .toString()
                .startsWith(QStringLiteral("Disabled")))
            Cadence = ((uint8_t)newValue.at(8));
        if (!settings.value(QZSettings::speed_power_based, QZSettings::default_speed_power_based).toBool())
            Speed = (double)((((uint8_t)newValue.at(7)) << 8) | ((uint8_t)newValue.at(6))) / 10.0;
        else
            Speed = metric::calculateSpeedFromPower(
                watts(), Inclination.value(), Speed.value(),
                fabs(now.msecsTo(Speed.lastChanged()) / 1000.0), this->speedLimit());
    }

    if (watts())
        KCal +=
            ((((0.048 * ((double)watts()) + 1.19) *
               settings.value(QZSettings::weight, QZSettings::default_weight).toFloat() * 3.5) /
              200.0) /
             (60000.0 / ((double)lastRefreshCharacteristicChanged.msecsTo(
                            now)))); //(( (0.048* Output in watts +1.19) * body weight in kg
                                                              //* 3.5) / 200 ) / 60
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

    qDebug() << QStringLiteral("Current Speed: ") + QString::number(Speed.value());
    qDebug() << QStringLiteral("Current Calculate Distance: ") + QString::number(Distance.value());
    qDebug() << QStringLiteral("Current Cadence: ") + QString::number(Cadence.value());
    qDebug() << QStringLiteral("Current CrankRevs: ") + QString::number(CrankRevs);
    qDebug() << QStringLiteral("Last CrankEventTime: ") + QString::number(LastCrankEventTime);
    qDebug() << QStringLiteral("Current Watt: ") + QString::number(watts());
    qDebug() << QStringLiteral("Current Resistance: ") + QString::number(Resistance.value());

    if (m_control->error() != QLowEnergyController::NoError) {
        qDebug() << QStringLiteral("QLowEnergyController ERROR!!") << m_control->errorString();
    }
}

resistance_t fitplusbike::pelotonToBikeResistance(int pelotonResistance) {
    QSettings settings;
    double adjustedPelotonResistance = (pelotonResistance - settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble()) /
                                       settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble();
    return (adjustedPelotonResistance * max_resistance) / 100;
}

double fitplusbike::bikeResistanceToPeloton(double resistance) {
    QSettings settings;
    return (((resistance * 100) / max_resistance) * settings.value(QZSettings::peloton_gain, QZSettings::default_peloton_gain).toDouble()) +
           settings.value(QZSettings::peloton_offset, QZSettings::default_peloton_offset).toDouble();
}

void fitplusbike::btinit() {

    QSettings settings;
    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

    if (merach_MRK) {
        uint8_t initData1[] = {0xaa, 0x01, 0x00, 0x01, 0x55};
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
    } else if (H9110_OSAKA) {
        uint8_t initData1[] = {0x02, 0xab, 0x3b, 0x30, 0x2a, 0x0d, 0x60, 0x01, 0x14, 0x11, 0xe3, 0x03};
        uint8_t initData2[] = {0x02, 0x43, 0x01, 0x42, 0x03};
        uint8_t initData3[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData5[] = {0x02, 0x44, 0x02, 0x46, 0x03};
        uint8_t initData6[] = {0x02, 0x41, 0x02, 0x43, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
    } else if (virtufit_etappe) {
        uint8_t initData1[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t initData2[] = {0x02, 0x41, 0x02, 0x43, 0x03};
        uint8_t initData3[] = {0x02, 0x41, 0x03, 0x42, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData5[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xaa, 0x18, 0x00, 0xc0, 0x03};
        uint8_t initData6[] = {0x02, 0x44, 0x0b, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x03};
        uint8_t initData7[] = {0x02, 0x44, 0x02, 0x46, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);

        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, false);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, false);
        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);

        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData7, sizeof(initData7), QStringLiteral("init"), false, true);
    } else if (sportstech_sx600) {
        uint8_t initData1[] = {0x02, 0x42, 0x42, 0x03};
        uint8_t initData2[] = {0x02, 0x53, 0x9f, 0x02, 0x00, 0x00, 0xce, 0x03};
        uint8_t initData3[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x3c, 0xaa, 0x18, 0x00, 0xc0, 0x03};
        uint8_t initData5[] = {0x02, 0x44, 0x0b, 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4e, 0x03};
        uint8_t initData6[] = {0x02, 0x44, 0x02, 0x46, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, false);

        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData2), QStringLiteral("init"), false, false);

        writeCharacteristic(initData6, sizeof(initData6), QStringLiteral("init"), false, true);
        writeCharacteristic(initData1, sizeof(initData2), QStringLiteral("init"), false, false);

        max_resistance = 32;

    } else {

        uint8_t initData1[] = {0x02, 0x44, 0x01, 0x45, 0x03};
        uint8_t initData2[] = {0x02, 0x44, 0x0b, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x4f, 0x03};
        uint8_t initData3[] = {0x02, 0x44, 0x0a, 0x00, 0x00, 0x00, 0x00, 0x46, 0xaa, 0x19, 0x00, 0xbb, 0x03};
        uint8_t initData4[] = {0x02, 0x44, 0x02, 0x46, 0x03};
        uint8_t initData5[] = {0x02, 0x41, 0x02, 0x43, 0x03};

        writeCharacteristic(initData1, sizeof(initData1), QStringLiteral("init"), false, true);
        writeCharacteristic(initData2, sizeof(initData2), QStringLiteral("init"), false, true);
        writeCharacteristic(initData3, sizeof(initData3), QStringLiteral("init"), false, true);
        writeCharacteristic(initData4, sizeof(initData4), QStringLiteral("init"), false, true);
        writeCharacteristic(initData5, sizeof(initData5), QStringLiteral("init"), false, true);
    }

    initDone = true;

    if (lastResistanceBeforeDisconnection != -1) {
        qDebug() << QStringLiteral("forcing resistance to ") + QString::number(lastResistanceBeforeDisconnection) +
                        QStringLiteral(". It was the last value before the disconnection.");
        forceResistance(lastResistanceBeforeDisconnection);
        lastResistanceBeforeDisconnection = -1;
    }
}

void fitplusbike::stateChanged(QLowEnergyService::ServiceState state) {
    QSettings settings;
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();

    QBluetoothUuid _gattWriteCharacteristicId((quint16)0xfff2);
    QBluetoothUuid _gattNotify1CharacteristicId((quint16)0xfff1);

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    qDebug() << QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state));

    if (sportstech_sx600 && gattCommunicationChannelServiceFTMS) {
        if (gattCommunicationChannelService->state() != QLowEnergyService::RemoteServiceDiscovered ||
            gattCommunicationChannelServiceFTMS->state() != QLowEnergyService::RemoteServiceDiscovered) {
            qDebug() << "sportstech_sx600 not all services discovered" << gattCommunicationChannelService->state()
                     << gattCommunicationChannelServiceFTMS->state();
            return;
        }
    }

    if (state == QLowEnergyService::RemoteServiceDiscovered) {
        // qDebug() << gattCommunicationChannelService->characteristics();

        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        Q_ASSERT(gattNotify1Characteristic.isValid());

        // establish hook into notifications
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicChanged, this,
                &fitplusbike::characteristicChanged);
        connect(gattCommunicationChannelService, &QLowEnergyService::characteristicWritten, this,
                &fitplusbike::characteristicWritten);
        connect(gattCommunicationChannelService,
                &QLowEnergyService::errorOccurred,
                this, &fitplusbike::errorService);
        connect(gattCommunicationChannelService, &QLowEnergyService::descriptorWritten, this,
                &fitplusbike::descriptorWritten);

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
                qDebug() << QStringLiteral("creating virtual bike interface...");
                auto virtualBike =
                    new virtualbike(this, noWriteResistance, noHeartService, bikeResistanceOffset, bikeResistanceGain);
                // connect(virtualBike,&virtualbike::debug ,this,&fitplusbike::debug);
                connect(virtualBike, &virtualbike::changeInclination, this, &fitplusbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        // ********************************************************************************************************

        QByteArray descriptor;
        descriptor.append((char)0x01);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(
            gattNotify1Characteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);

        if (sportstech_sx600 && gattCommunicationChannelServiceFTMS) {
            QBluetoothUuid _gattNotifyFTMSCharacteristicId((quint16)0x2AD2);
            gattNotifyFTMSCharacteristic =
                gattCommunicationChannelServiceFTMS->characteristic(_gattNotifyFTMSCharacteristicId);
            Q_ASSERT(gattNotifyFTMSCharacteristic.isValid());

            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::characteristicChanged, this,
                    &fitplusbike::characteristicChanged);
            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::characteristicWritten, this,
                    &fitplusbike::characteristicWritten);
            connect(
                gattCommunicationChannelServiceFTMS,
                &QLowEnergyService::errorOccurred,
                this, &fitplusbike::errorService);
            connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::descriptorWritten, this,
                    &fitplusbike::descriptorWritten);

            QByteArray descriptor;
            descriptor.append((char)0x01);
            descriptor.append((char)0x00);
            gattCommunicationChannelServiceFTMS->writeDescriptor(
                gattNotifyFTMSCharacteristic.descriptor(QBluetoothUuid::DescriptorType::ClientCharacteristicConfiguration), descriptor);
        }
    }
}

void fitplusbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorWritten ") + descriptor.name() + " " + newValue.toHex(' ');

    initRequest = true;
    emit connectedAndDiscovered();
}

void fitplusbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    Q_UNUSED(characteristic);
    qDebug() << QStringLiteral("characteristicWritten ") + newValue.toHex(' ');
}

void fitplusbike::serviceScanDone(void) {
    QSettings settings;
    bool sportstech_sx600 = settings.value(QZSettings::sportstech_sx600, QZSettings::default_sportstech_sx600).toBool();
    qDebug() << QStringLiteral("serviceScanDone");

    QBluetoothUuid _gattCommunicationChannelServiceId((quint16)0xfff0);

    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    if(gattCommunicationChannelService) {
        connect(gattCommunicationChannelService, &QLowEnergyService::stateChanged, this, &fitplusbike::stateChanged);
        gattCommunicationChannelService->discoverDetails();

        if (sportstech_sx600) {
            gattCommunicationChannelServiceFTMS = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
            if (gattCommunicationChannelServiceFTMS) {
                qDebug() << "FTMS found!";
                connect(gattCommunicationChannelServiceFTMS, &QLowEnergyService::stateChanged, this,
                        &fitplusbike::stateChanged);
                gattCommunicationChannelServiceFTMS->discoverDetails();
            }
        }
    } else {
        qDebug() << _gattCommunicationChannelServiceId << "not found!";
        gattCommunicationChannelServiceFTMS = m_control->createServiceObject(QBluetoothUuid((quint16)0x1826));
        if(gattCommunicationChannelServiceFTMS) {
            QSettings settings;
            settings.setValue(QZSettings::ftms_bike, bluetoothDevice.name());
            qDebug() << "forcing FTMS bike since it has FTMS";
            if(homeform::singleton())
                homeform::singleton()->setToastRequested("FTMS bike found, restart the app to apply the change!");
        }
    }
}

void fitplusbike::errorService(QLowEnergyService::ServiceError err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    qDebug() << QStringLiteral("fitplusbike::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusbike::error(QLowEnergyController::Error err) {
    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    qDebug() << QStringLiteral("fitplusbike::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
                    m_control->errorString();
}

void fitplusbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    qDebug() << QStringLiteral("Found new device: ") + device.name() + " (" + device.address().toString() + ')';
    {
        bluetoothDevice = device;

        if (device.name().startsWith(QStringLiteral("MRK-"))) {
            qDebug() << QStringLiteral("merach_MRK workaround enabled!");
            merach_MRK = true;
        } else if (device.name().toUpper().startsWith("H9110 OSAKA")) {
            qDebug() << QStringLiteral("H9110 OSAKA workaround enabled!");
            max_resistance = 32;
            H9110_OSAKA = true;
        }

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &fitplusbike::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &fitplusbike::serviceScanDone);
        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, &fitplusbike::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &fitplusbike::controllerStateChanged);

        connect(m_control,
                &QLowEnergyController::errorOccurred,
                this, [this](QLowEnergyController::Error error) {
                    Q_UNUSED(error);
                    Q_UNUSED(this);
                    qDebug() << QStringLiteral("Cannot connect to remote device.");
                    emit disconnected();
                });
        connect(m_control, &QLowEnergyController::connected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("Controller connected. Search services...");
            m_control->discoverServices();
        });
        connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
            Q_UNUSED(this);
            qDebug() << QStringLiteral("LowEnergy controller disconnected");
            emit disconnected();
        });

        // Connect
        m_control->connectToDevice();
        return;
    }
}

bool fitplusbike::connected() {
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

uint16_t fitplusbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void fitplusbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        lastResistanceBeforeDisconnection = Resistance.value();
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        m_control->connectToDevice();
    }
}

uint16_t fitplusbike::wattsFromResistance(double resistance) {
    QSettings settings;
    bool virtufit_etappe = settings.value(QZSettings::virtufit_etappe, QZSettings::default_virtufit_etappe).toBool();

    // https://github.com/cagnulein/qdomyos-zwift/issues/62#issuecomment-736913564
    /*if(currentCadence().value() < 90)
        return (uint16_t)((3.59 * exp(0.0217 * (double)(currentCadence().value()))) * exp(0.095 *
    (double)(currentResistance().value())) ); else return (uint16_t)((3.59 * exp(0.0217 *
    (double)(currentCadence().value()))) * exp(0.088 * (double)(currentResistance().value())) );*/    

    if(virtufit_etappe) {
        const double Epsilon = 4.94065645841247E-324;
        // VirtuFit Etappe 2.0i Spinbike ERG Table #1526
        const int wattTableFirstDimension = 25;
        const int wattTableSecondDimension = 11;
        double wattTable[wattTableFirstDimension][wattTableSecondDimension] = {
            {Epsilon, 15.0, 15.0, 15.0, 20.0, 30.0, 32.0, 38.0, 44.0, 56.0, 66.0},
            {Epsilon, 15.0, 15.0, 15.0, 20.0, 30.0, 32.0, 38.0, 44.0, 56.0, 66.0},
            {Epsilon, 16.0, 16.0, 16.0, 22.0, 30.0, 38.0, 45.0, 53.0, 67.0, 79.0},
            {Epsilon, 18.0, 18.0, 18.0, 26.0, 34.0, 43.0, 52.0, 62.0, 78.0, 92.0},
            {Epsilon, 20.0, 20.0, 20.0, 28.0, 38.0, 48.0, 59.0, 71.0, 89.0, 105.0},
            {Epsilon, 23.0, 23.0, 23.0, 32.0, 43.0, 54.0, 66.0, 80.0, 100.0, 118.0},
            {Epsilon, 24.0, 24.0, 24.0, 35.0, 46.0, 59.0, 73.0, 89.0, 110.0, 130.0},
            {Epsilon, 26.0, 26.0, 26.0, 37.0, 51.0, 65.0, 81.0, 98.0, 122.0, 143.0},
            {Epsilon, 28.0, 28.0, 28.0, 41.0, 56.0, 71.0, 88.0, 107.0, 133.0, 156.0},
            {Epsilon, 30.0, 30.0, 30.0, 44.0, 60.0, 77.0, 96.0, 116.0, 144.0, 169.0},
            {Epsilon, 33.0, 33.0, 33.0, 47.0, 65.0, 83.0, 103.0, 125.0, 155.0, 182.0},
            {Epsilon, 34.0, 34.0, 34.0, 50.0, 70.0, 89.0, 110.0, 134.0, 166.0, 195.0},
            {Epsilon, 37.0, 37.0, 37.0, 54.0, 74.0, 94.0, 117.0, 143.0, 177.0, 208.0},
            {Epsilon, 38.0, 38.0, 38.0, 56.0, 78.0, 100.0, 125.0, 152.0, 188.0, 220.0},
            {Epsilon, 41.0, 41.0, 41.0, 60.0, 82.0, 106.0, 132.0, 161.0, 199.0, 233.0},
            {Epsilon, 43.0, 43.0, 43.0, 62.0, 86.0, 111.0, 139.0, 170.0, 209.0, 245.0},
            {Epsilon, 45.0, 45.0, 45.0, 66.0, 91.0, 117.0, 147.0, 180.0, 220.0, 259.0},
            {Epsilon, 48.0, 48.0, 48.0, 70.0, 96.0, 124.0, 155.0, 190.0, 232.0, 273.0},
            {Epsilon, 50.0, 50.0, 50.0, 73.0, 101.0, 130.0, 163.0, 200.0, 244.0, 287.0},
            {Epsilon, 52.0, 52.0, 52.0, 76.0, 106.0, 136.0, 171.0, 210.0, 256.0, 300.0},
            {Epsilon, 54.0, 54.0, 54.0, 80.0, 111.0, 143.0, 179.0, 220.0, 268.0, 314.0},
            {Epsilon, 57.0, 57.0, 57.0, 84.0, 116.0, 149.0, 187.0, 230.0, 279.0, 327.0},
            {Epsilon, 59.0, 59.0, 59.0, 87.0, 121.0, 155.0, 195.0, 240.0, 290.0, 340.0},
            {Epsilon, 62.0, 62.0, 62.0, 91.0, 126.0, 162.0, 203.0, 250.0, 302.0, 353.0},
            {Epsilon, 64.0, 64.0, 64.0, 94.0, 130.0, 168.0, 211.0, 260.0, 313.0, 366.0}};

        int level = resistance;
        if (level < 0) {
            level = 0;
        }
        if (level >= wattTableFirstDimension) {
            level = wattTableFirstDimension - 1;
        }
        double *watts_of_level = wattTable[level];
        int watt_setp = (Cadence.value() / 10.0);
        if (watt_setp >= 10) {
            return (((double)Cadence.value()) / 100.0) * watts_of_level[wattTableSecondDimension - 1];
        }
        double watt_base = watts_of_level[watt_setp];
        return (((watts_of_level[watt_setp + 1] - watt_base) / 10.0) * ((double)(((int)(Cadence.value())) % 10))) +
               watt_base;
    } else {
        return _ergTable.estimateWattage(Cadence.value(), resistance);
    }
}

resistance_t fitplusbike::resistanceFromPowerRequest(uint16_t power) {
    return _ergTable.resistanceFromPowerRequest(power, Cadence.value(), max_resistance);
}
