#include "kettlerracersbike.h"
#include "kettlerhandshake.h"
#include "virtualdevices/virtualbike.h"
#include <QBluetoothLocalDevice>
#include <QDateTime>
#include <QFile>
#include <QMetaEnum>
#include <QSettings>
#include <QElapsedTimer>
#include <QThread>
#include <math.h>
#ifdef Q_OS_ANDROID
#include "keepawakehelper.h"
#include <QLowEnergyConnectionParameters>
#include <QAndroidJniObject>
#include <QAndroidJniEnvironment>
#endif
#include "homeform.h"

#include <chrono>

using namespace std::chrono_literals;

kettlerracersbike::kettlerracersbike(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    handshakeRequested = false;
    handshakeDone = false;
    notificationsSubscribed = false;
    kettlerServiceReady = false;
    connect(refresh, &QTimer::timeout, this, &kettlerracersbike::update);
    refresh->start(200ms);
}

kettlerracersbike::~kettlerracersbike() {
    qDebug() << QStringLiteral("Kettler bike destructor called");
}

void kettlerracersbike::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                           bool wait_for_response) {

    if(!gattKettlerService) {
        qDebug() << "gattKettlerService is null!";
        return;
    }

    QEventLoop loop;
    QTimer timeout;
    if (wait_for_response) {
        connect(gattKettlerService, &QLowEnergyService::characteristicChanged, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    } else {
        connect(gattKettlerService, &QLowEnergyService::characteristicWritten, &loop, &QEventLoop::quit);
        timeout.singleShot(300ms, &loop, &QEventLoop::quit);
    }

    if (writeBuffer) {
        delete writeBuffer;
    }
    writeBuffer = new QByteArray((const char *)data, data_len);

    gattKettlerService->writeCharacteristic(gattWriteCharKettlerId, *writeBuffer);

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + writeBuffer->toHex(' ') + QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void kettlerracersbike::changePower(int32_t power) {
    RequestedPower = power;

    if (power < 0)
        power = 0;

    // Use Qt Bluetooth for all platforms
    uint8_t powerData[2];
    powerData[0] = (uint8_t)(power & 0xFF);
    powerData[1] = (uint8_t)((power >> 8) & 0xFF);
    writeCharacteristic(powerData, sizeof(powerData), QStringLiteral("changePower ") + QString::number(power) + "W", false, false);
}

void kettlerracersbike::forceInclination(double inclination) {
    // Store inclination for SIM mode
    Inclination = inclination;

    // For grade mode, we need to send the grade value
    // Based on test logs, grade values are sent to the same characteristic as power
    // TODO: Analyze test logs to determine exact grade format
    // For now, using simple format similar to power
    int16_t gradeValue = (int16_t)(inclination * 100); // Convert percentage to integer format

    uint8_t gradeData[2];
    gradeData[0] = (uint8_t)(gradeValue & 0xFF);
    gradeData[1] = (uint8_t)((gradeValue >> 8) & 0xFF);

    writeCharacteristic(gradeData, sizeof(gradeData), QStringLiteral("forceInclination ") + QString::number(inclination) + "%", false, false);
}

uint16_t kettlerracersbike::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }
    return m_watt.value();
}

double kettlerracersbike::bikeResistanceToPeloton(double resistance) {
    // Simple linear mapping for now
    return resistance;
}

resistance_t kettlerracersbike::pelotonToBikeResistance(int pelotonResistance) {
    // Simple linear mapping for now
    return pelotonResistance;
}

bool kettlerracersbike::connected() {
    // Use Qt Bluetooth state for all platforms
    if (!m_control) {
        return false;
    }
    return m_control->state() == QLowEnergyController::ConnectedState;
}

void kettlerracersbike::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");
        initDone = false;
        handshakeRequested = false;
        handshakeDone = false;
        notificationsSubscribed = false;
        kettlerServiceReady = false;
        connectedAndDiscoveredEmitted = false;
        m_control->connectToDevice();
    }
}

void kettlerracersbike::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');

    bluetoothDevice = device;

    // Use Qt Bluetooth for all platforms
    m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
    connect(m_control, &QLowEnergyController::serviceDiscovered, this, &kettlerracersbike::serviceDiscovered);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &kettlerracersbike::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, &kettlerracersbike::error);
    connect(m_control, &QLowEnergyController::stateChanged, this, &kettlerracersbike::controllerStateChanged);

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
        emit debug(QStringLiteral("Controller connected. Waiting 1s before discovering services..."));
        // IMPORTANT: 1000ms delay before discovering services (matching Java implementation timing)
        QTimer::singleShot(1000ms, this, [this]() {
            if (m_control && m_control->state() == QLowEnergyController::ConnectedState) {
                emit debug(QStringLiteral("Starting service discovery..."));
                m_control->discoverServices();
            }
        });
    });
    connect(m_control, &QLowEnergyController::disconnected, this, [this]() {
        Q_UNUSED(this);
        emit debug(QStringLiteral("LowEnergy controller disconnected"));
        emit disconnected();
    });

    // Connect
    m_control->connectToDevice();
}

void kettlerracersbike::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());
}

void kettlerracersbike::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));

    // Kettler custom service: 638af000-7bde-3e25-ffc5-9de9b2a0197a
    QBluetoothUuid kettlerServiceUuid(QStringLiteral("638af000-7bde-3e25-ffc5-9de9b2a0197a"));
    gattKettlerService = m_control->createServiceObject(kettlerServiceUuid);

    if (gattKettlerService == nullptr) {
        emit debug(QStringLiteral("invalid service") + kettlerServiceUuid.toString());
        return;
    }

    connect(gattKettlerService, &QLowEnergyService::stateChanged, this, &kettlerracersbike::stateChanged);

    // Discover Kettler service details first
    emit debug(QStringLiteral("Discovering Kettler service details..."));
    gattKettlerService->discoverDetails();

    // CSC service will be discovered AFTER Kettler service is fully discovered
    // to avoid concurrent GATT operations (see stateChanged handler)
    QBluetoothUuid cscServiceUuid(QStringLiteral("00001816-0000-1000-8000-00805f9b34fb"));
    gattCSCService = m_control->createServiceObject(cscServiceUuid);

    if (gattCSCService != nullptr) {
        connect(gattCSCService, &QLowEnergyService::stateChanged, this, &kettlerracersbike::stateChanged);
        // Don't call discoverDetails() here - will be called after Kettler service is ready
    }

    // Prepare Cycling Power service (0x1818) if present; discover after handshake
    QBluetoothUuid powerServiceUuid(QStringLiteral("00001818-0000-1000-8000-00805f9b34fb"));
    gattPowerService = m_control->createServiceObject(powerServiceUuid);
    if (gattPowerService != nullptr) {
        connect(gattPowerService, &QLowEnergyService::stateChanged, this, &kettlerracersbike::stateChanged);
    }
}

void kettlerracersbike::error(QLowEnergyController::Error err) {
    qDebug() << QStringLiteral("controller ERROR ") << err;
}

void kettlerracersbike::errorService(QLowEnergyService::ServiceError err) {
    qDebug() << QStringLiteral("service ERROR ") << err;
    qDebug() << QStringLiteral("Kettler service error: ") << err;

    if (err == QLowEnergyService::CharacteristicReadError) {
        qDebug() << QStringLiteral("Kettler :: CharacteristicReadError - resetting handshakeRequested");
        handshakeRequested = false;
    }
}

void kettlerracersbike::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));

    // Note: connectedAndDiscovered and prime notifications are now called after handshake completion
}

void kettlerracersbike::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << newValue.toHex(' ');
}

void kettlerracersbike::characteristicWritten(const QLowEnergyCharacteristic &characteristic,
                                             const QByteArray &newValue) {
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));

    // Check if this is the handshake write confirmation
    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("638a1105-7bde-3e25-ffc5-9de9b2a0197a"))) {
        emit debug(QStringLiteral("Kettler :: Handshake write confirmed"));
        handshakeDone = true;

        // After handshake is complete, discover additional services first,
        // then enable notifications to avoid CCC writes before discovery completes.
        if (gattCSCService && gattCSCService->state() != QLowEnergyService::ServiceDiscovered) {
            emit debug(QStringLiteral("Discovering CSC service details after handshake..."));
            gattCSCService->discoverDetails();
        }
        if (gattPowerService && gattPowerService->state() != QLowEnergyService::ServiceDiscovered) {
            emit debug(QStringLiteral("Discovering Cycling Power (0x1818) service details after handshake..."));
            gattPowerService->discoverDetails();
        }

        // Create virtual bike interface after handshake
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
            bool cadence = settings.value(QZSettings::bike_cadence_sensor, QZSettings::default_bike_cadence_sensor).toBool();
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
                auto virtualBike = new virtualbike(this, noWriteResistance, noHeartService, 4, 1);
                connect(virtualBike, &virtualbike::changeInclination, this, &kettlerracersbike::changeInclination);
                this->setVirtualDevice(virtualBike, VIRTUAL_DEVICE_MODE::PRIMARY);
            }
        }
        firstStateChanged = 1;
        initDone = true;

        // Defer connectedAndDiscovered until after CCC subscriptions
        // If CSC service is not present, subscribe now and emit immediately
        if (!gattCSCService) {
            subscribeKettlerNotifications();
            if (!primedNotifyStart && gattWriteCharKettlerId.isValid()) {
                uint8_t zero[2] = {0x00, 0x00};
                writeCharacteristic(zero, sizeof(zero), QStringLiteral("prime notifications"), false, false);
                primedNotifyStart = true;
            }
            if (!connectedAndDiscoveredEmitted) {
                initRequest = true;
                emit connectedAndDiscovered();
                connectedAndDiscoveredEmitted = true;
            }
        }
    }
}

void kettlerracersbike::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid().toString() << newValue.toHex(' ');

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("638a1104-7bde-3e25-ffc5-9de9b2a0197a"))) {
        sendHandshake(newValue);
    }
}

void kettlerracersbike::sendHandshake(const QByteArray &seed) {
    if (handshakeDone) {
        emit debug(QStringLiteral("Kettler :: handshake already completed, ignoring additional seed."));
        return;
    }

    const QByteArray handshakeRaw = kettler::buildHandshakeSeed(seed);
    if (handshakeRaw.size() != 6) {
        emit debug(QStringLiteral("Kettler :: handshake seed not available (requires 2 bytes from device)."));
        return;
    }

    emit debug(QStringLiteral("Kettler :: handshake data raw: ") + handshakeRaw.toHex(' '));

    const QByteArray payload = kettler::computeHandshake(handshakeRaw);
    if (payload.size() != 16) {
        emit debug(QStringLiteral("Kettler :: handshake computation failed."));
        handshakeRequested = false;
        return;
    }

    emit debug(QStringLiteral("Kettler :: handshake data encrypted: ") + payload.toHex(' '));

    if (!gattKettlerService || !gattKeyWriteCharKettlerId.isValid()) {
        emit debug(QStringLiteral("Kettler :: handshake write characteristic invalid."));
        handshakeRequested = false;
        return;
    }

    // Use Qt Bluetooth for all platforms
    gattKettlerService->writeCharacteristic(gattKeyWriteCharKettlerId, payload);
    handshakeRequested = false;
    // handshakeDone will be set in characteristicWritten after write confirmation
    // subscribeKettlerNotifications will be called there as well
}

void kettlerracersbike::requestHandshakeSeed()
{
    if (handshakeRequested || handshakeDone) {
        qDebug() << QStringLiteral("Kettler :: requestHandshakeSeed early return - handshakeRequested:") << handshakeRequested <<
                   QStringLiteral(" handshakeDone:") << handshakeDone;
        return;
    }

    handshakeRequested = true;
    notificationsSubscribed = false;

    // Use Qt Bluetooth for all platforms
    if (!gattKettlerService) {
        qDebug() << QStringLiteral("Kettler :: gattKettlerService is null");
        handshakeRequested = false;
        return;
    }

    if (!gattKeyReadCharKettlerId.isValid()) {
        qDebug() << QStringLiteral("Kettler :: handshake read characteristic invalid.");
        handshakeRequested = false;
        return;
    }

    if (!(gattKeyReadCharKettlerId.properties() & QLowEnergyCharacteristic::Read)) {
        qDebug() << QStringLiteral("Kettler :: handshake read characteristic does not support Read property");
        handshakeRequested = false;
        return;
    }

    qDebug() << QStringLiteral("reading Kettler handshake seed - characteristic properties:") << gattKeyReadCharKettlerId.properties();

#ifdef Q_OS_ANDROID
    // On Android, use direct Java call to bypass Qt's buggy executeReadJob
    QString serviceUuid = QStringLiteral("638af000-7bde-3e25-ffc5-9de9b2a0197a");
    QString charUuid = QStringLiteral("638a1104-7bde-3e25-ffc5-9de9b2a0197a"); // Handshake seed READ characteristic

    qDebug() << QStringLiteral("Kettler :: Calling readCharacteristicDirectlyStatic with service:") << serviceUuid << " char:" << charUuid;

    QAndroidJniEnvironment env;
    bool result = QAndroidJniObject::callStaticMethod<jboolean>(
        "org/qtproject/qt5/android/bluetooth/QtBluetoothLE",
        "readCharacteristicDirectlyStatic",
        "(Ljava/lang/String;Ljava/lang/String;)Z",
        QAndroidJniObject::fromString(serviceUuid).object<jstring>(),
        QAndroidJniObject::fromString(charUuid).object<jstring>()
    );

    if (env->ExceptionCheck()) {
        qDebug() << QStringLiteral("Kettler :: JNI Exception occurred!");
        env->ExceptionDescribe();
        env->ExceptionClear();
        result = false;
    }

    qDebug() << QStringLiteral("Kettler :: readCharacteristicDirectlyStatic result:") << result;
#else
    gattKettlerService->readCharacteristic(gattKeyReadCharKettlerId);
    qDebug() << QStringLiteral("Kettler :: readCharacteristic call successful, waiting for response...");
#endif
}

void kettlerracersbike::subscribeKettlerNotifications()
{
    if (notificationsSubscribed || !gattKettlerService) {
        return;
    }

    QBluetoothUuid rpmUuid(QStringLiteral("638a1002-7bde-3e25-ffc5-9de9b2a0197a"));
    QBluetoothUuid powerUuid(QStringLiteral("638a100e-7bde-3e25-ffc5-9de9b2a0197a"));
    QBluetoothUuid char1Uuid(QStringLiteral("638a100c-7bde-3e25-ffc5-9de9b2a0197a"));
    QBluetoothUuid char2Uuid(QStringLiteral("638a1010-7bde-3e25-ffc5-9de9b2a0197a"));

    auto subscribeDescriptor = [this](const QLowEnergyCharacteristic &characteristic, const QString &label) {
        if (!characteristic.isValid()) {
            return;
        }
        auto descriptor = characteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
        if (descriptor.isValid()) {
            gattKettlerService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
            emit debug(label + QStringLiteral(" notification subscribed"));
        }
    };

    subscribeDescriptor(gattKettlerService->characteristic(rpmUuid), QStringLiteral("Kettler RPM"));
    subscribeDescriptor(gattKettlerService->characteristic(powerUuid), QStringLiteral("Kettler power"));
    subscribeDescriptor(gattKettlerService->characteristic(char1Uuid), QStringLiteral("Kettler char1"));
    subscribeDescriptor(gattKettlerService->characteristic(char2Uuid), QStringLiteral("Kettler char2"));

    // Also subscribe to CSC service if available
    if (gattCSCService) {
        QBluetoothUuid cscMeasurementUuid(QStringLiteral("00002a5b-0000-1000-8000-00805f9b34fb"));
        auto cscChar = gattCSCService->characteristic(cscMeasurementUuid);
        if (cscChar.isValid()) {
            auto descriptor = cscChar.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (descriptor.isValid()) {
                gattCSCService->writeDescriptor(descriptor, QByteArray::fromHex("0100"));
                emit debug(QStringLiteral("CSC notification subscribed"));
            }
        }
    }

    notificationsSubscribed = true;
}

void kettlerracersbike::stateChanged(QLowEnergyService::ServiceState state) {
    QBluetoothUuid _gattWriteCharacteristicId(QStringLiteral("638a100e-7bde-3e25-ffc5-9de9b2a0197a")); // Power control
    QBluetoothUuid _gattKeyReadCharacteristicId(QStringLiteral("638a1104-7bde-3e25-ffc5-9de9b2a0197a"));  // Handshake seed
    QBluetoothUuid _gattKeyWriteCharacteristicId(QStringLiteral("638a1105-7bde-3e25-ffc5-9de9b2a0197a")); // Handshake key
    QBluetoothUuid _gattNotifyCharacteristicCSCId(QStringLiteral("00002a5b-0000-1000-8000-00805f9b34fb"));

    qDebug() << QStringLiteral("BTLE stateChanged ") << state;

    QLowEnergyService *service = qobject_cast<QLowEnergyService *>(sender());

    if (service == gattKettlerService && state == QLowEnergyService::ServiceDiscovered) {
        handshakeRequested = false;
        handshakeDone = false;
        notificationsSubscribed = false;
        kettlerServiceReady = true;

        emit debug(QStringLiteral("Kettler service connected"));

        connect(gattKettlerService, &QLowEnergyService::characteristicWritten, this,
                &kettlerracersbike::characteristicWritten);
        connect(gattKettlerService, &QLowEnergyService::characteristicRead, this,
                &kettlerracersbike::characteristicRead);
        connect(gattKettlerService, &QLowEnergyService::descriptorWritten, this,
                &kettlerracersbike::descriptorWritten);
        connect(gattKettlerService, &QLowEnergyService::descriptorRead, this,
                &kettlerracersbike::descriptorRead);
        connect(gattKettlerService, &QLowEnergyService::characteristicChanged, this,
                &kettlerracersbike::characteristicChanged);
        connect(gattKettlerService, static_cast<void (QLowEnergyService::*)(QLowEnergyService::ServiceError)>(&QLowEnergyService::error),
                this, &kettlerracersbike::errorService);

        gattWriteCharKettlerId = gattKettlerService->characteristic(_gattWriteCharacteristicId);
        if (!gattWriteCharKettlerId.isValid()) {
            emit debug(QStringLiteral("gattWriteCharKettlerId invalid"));
        }

        gattKeyReadCharKettlerId = gattKettlerService->characteristic(_gattKeyReadCharacteristicId);
        if (!gattKeyReadCharKettlerId.isValid()) {
            qDebug() << QStringLiteral("gattKeyReadCharKettlerId invalid");
        } else {
            qDebug() << QStringLiteral("gattKeyReadCharKettlerId valid, properties:") << gattKeyReadCharKettlerId.properties();
        }

        gattKeyWriteCharKettlerId = gattKettlerService->characteristic(_gattKeyWriteCharacteristicId);
        if (!gattKeyWriteCharKettlerId.isValid()) {
            qDebug() << QStringLiteral("gattKeyWriteCharKettlerId invalid");
        } else {
            qDebug() << QStringLiteral("gattKeyWriteCharKettlerId valid, properties:") << gattKeyWriteCharKettlerId.properties();
        }

        // Request handshake seed FIRST before any other operations
        // CSC service discovery and virtual bike creation will happen AFTER handshake
        QTimer::singleShot(500, this, [this]() {
            if (!handshakeRequested && !handshakeDone) {
                requestHandshakeSeed();
            }
        });
    }

    if (service == gattCSCService && state == QLowEnergyService::ServiceDiscovered) {
        emit debug(QStringLiteral("CSC service connected"));

        connect(gattCSCService, &QLowEnergyService::characteristicWritten, this,
                &kettlerracersbike::characteristicWritten);
        connect(gattCSCService, &QLowEnergyService::characteristicRead, this,
                &kettlerracersbike::characteristicRead);
        connect(gattCSCService, &QLowEnergyService::descriptorWritten, this,
                &kettlerracersbike::descriptorWritten);
        connect(gattCSCService, &QLowEnergyService::descriptorRead, this,
                &kettlerracersbike::descriptorRead);
        connect(gattCSCService, &QLowEnergyService::characteristicChanged, this,
                &kettlerracersbike::characteristicChanged);

        // Now that CSC is discovered, if handshake is done, enable notifications and prime
        if (handshakeDone && !notificationsSubscribed) {
            subscribeKettlerNotifications();
            if (!primedNotifyStart && gattWriteCharKettlerId.isValid()) {
                uint8_t zero[2] = {0x00, 0x00};
                writeCharacteristic(zero, sizeof(zero), QStringLiteral("prime notifications"), false, false);
                primedNotifyStart = true;
            }
            // Now signal ready to the rest of the stack/UI
            if (!connectedAndDiscoveredEmitted) {
                initRequest = true;
                emit connectedAndDiscovered();
                connectedAndDiscoveredEmitted = true;
            }
        }
    }

    if (service == gattPowerService && state == QLowEnergyService::ServiceDiscovered) {
        emit debug(QStringLiteral("Cycling Power (0x1818) service connected"));
        // Nothing to subscribe for Kettler path here
    }

    // Virtual bike creation and CSC discovery moved to characteristicWritten after handshake completion
}

void kettlerracersbike::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    QDateTime now = QDateTime::currentDateTime();

    emit debug(QStringLiteral(" << ") + newValue.toHex(' ') + QStringLiteral(" // ") + characteristic.uuid().toString());

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("638a1104-7bde-3e25-ffc5-9de9b2a0197a"))) {
        sendHandshake(newValue);
        return;
    }

    if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("00002a5b-0000-1000-8000-00805f9b34fb"))) {
        // CSC measurement characteristic
        cscPacketReceived(newValue);
    } else if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("638a1002-7bde-3e25-ffc5-9de9b2a0197a"))) {
        // Kettler RPM characteristic
        kettlerPacketReceived(newValue);
    } else if (characteristic.uuid() == QBluetoothUuid(QStringLiteral("638a100e-7bde-3e25-ffc5-9de9b2a0197a")) ||
               characteristic.uuid() == QBluetoothUuid(QBluetoothUuid::CyclingPowerMeasurement)) {
        powerPacketReceived(newValue);
    }

    QSettings settings;
    QString heartRateBeltName = settings.value(QZSettings::heart_rate_belt_name, QZSettings::default_heart_rate_belt_name).toString();

    if (heartRateBeltName.startsWith(QStringLiteral("Disabled"))) {
        update_hr_from_external();
    }

#ifdef Q_OS_ANDROID
    if (settings.value(QZSettings::ant_heart, QZSettings::default_ant_heart).toBool())
        Heart = (uint8_t)KeepAwakeHelper::heart();
    else
#endif
    {
        if (heartRateBeltName.startsWith(QStringLiteral("Disabled")) &&
            heartRateBeltName != QStringLiteral("Disabled")) {
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

    lastRefreshCharacteristicChangedKettler = now;
}

void kettlerracersbike::cscPacketReceived(const QByteArray &packet) {
    // Standard CSC packet parsing
    if (packet.length() < 11)
        return;

    uint8_t flags = packet.at(0);

    if (flags & 0x01) { // Wheel revolution data present
        uint32_t wheelRevolutions = (packet.at(4) << 24) | (packet.at(3) << 16) | (packet.at(2) << 8) | packet.at(1);
        uint16_t wheelEventTime = (packet.at(6) << 8) | packet.at(5);

        // Calculate speed from wheel data if available
        static uint32_t lastWheelRevolutions = 0;
        static uint16_t lastWheelEventTime = 0;

        if (lastWheelRevolutions > 0) {
            uint32_t wheelRevDelta = wheelRevolutions - lastWheelRevolutions;
            uint16_t wheelTimeDelta = wheelEventTime - lastWheelEventTime;

            if (wheelTimeDelta > 0) {
                // Speed calculation (assuming wheel circumference of 2.1m)
                double speed = (wheelRevDelta * 2.1 * 1024.0) / (wheelTimeDelta * 3.6);
                Speed = speed;
            }
        }

        lastWheelRevolutions = wheelRevolutions;
        lastWheelEventTime = wheelEventTime;
    }

    if (flags & 0x02) { // Crank revolution data present
        uint16_t crankRevolutions = (packet.at(8) << 8) | packet.at(7);
        uint16_t crankEventTime = (packet.at(10) << 8) | packet.at(9);

        // Calculate cadence from CSC data
        if (oldCrankRevs > 0) {
            uint16_t crankRevsDelta = crankRevolutions - oldCrankRevs;
            uint16_t crankTimeDelta = crankEventTime - oldLastCrankEventTime;

            if (crankTimeDelta > 0 && crankRevsDelta > 0) {
                // Cadence calculation: (revolutions * 1024 * 60) / (time_delta)
                // 1024 is the time resolution, 60 converts to RPM
                double cadence = (double(crankRevsDelta) * 1024.0 * 60.0) / double(crankTimeDelta);
                if (cadence > 0 && cadence < 255) {
                    Cadence = cadence;
                    lastGoodCadence = QDateTime::currentDateTime();
                }
            }
        }

        oldCrankRevs = crankRevolutions;
        oldLastCrankEventTime = crankEventTime;
        CrankRevsRead = crankRevolutions;
    }
}

void kettlerracersbike::kettlerPacketReceived(const QByteArray &packet)
{
    Q_UNUSED(packet);
}

void kettlerracersbike::powerPacketReceived(const QByteArray &packet) {
    if (packet.size() < 2) {
        return;
    }

    int wattsValue = 0;

    if (packet.size() == 2) {
        wattsValue = static_cast<int>(static_cast<quint8>(packet.at(0)) |
                                      (static_cast<quint8>(packet.at(1)) << 8));
    } else if (packet.size() >= 4) {
        qint16 instantPower = static_cast<qint16>(static_cast<quint8>(packet.at(2)) |
                                                  (static_cast<quint8>(packet.at(3)) << 8));
        wattsValue = qMax(0, static_cast<int>(instantPower));
    } else {
        return;
    }

    wattsValue = qBound(0, wattsValue, 2000);
    powerSensor(static_cast<uint16_t>(wattsValue));
    lastRefreshCharacteristicChangedPower = QDateTime::currentDateTime();
}

void kettlerracersbike::update() {
    if (!m_control) {
        emit disconnected();
        return;
    }

    // Use Qt Bluetooth state checking for all platforms
    if (m_control->state() == QLowEnergyController::UnconnectedState) {
        emit disconnected();
        return;
    }

    static QElapsedTimer handshakeTimer;
    if (!handshakeDone && kettlerServiceReady && gattKettlerService && gattKettlerService->state() == QLowEnergyService::ServiceDiscovered) {
        const qint64 intervalMs = 1000;

        if (!handshakeRequested) {
            handshakeTimer.restart();
            requestHandshakeSeed();
        } else if (!handshakeTimer.isValid() || handshakeTimer.hasExpired(intervalMs)) {
            emit debug(QStringLiteral("retrying Kettler handshake seed read"));
            handshakeTimer.restart();
            handshakeRequested = false;
            requestHandshakeSeed();
        }
    }

    if (!handshakeDone) {
        return;
    }

    if (initRequest) {
        initRequest = false;
    } else if (bluetoothDevice.isValid() && m_control &&
               m_control->state() == QLowEnergyController::DiscoveredState && gattKettlerService &&
               gattKettlerService->state() == QLowEnergyService::ServiceDiscovered &&
               gattWriteCharKettlerId.isValid() && initDone) {
        update_metrics(true, watts());

        // Check if we need to send power or grade commands
        if (requestPower != -1) {
            changePower(requestPower);
            requestPower = -1;
        }
        if (requestInclination != -100) {
            forceInclination(requestInclination);
            requestInclination = -100;
        }
    }
}

void kettlerracersbike::startDiscover() {
    // Called by bluetooth class
    qDebug() << QStringLiteral("kettlerracersbike::startDiscover");
}
