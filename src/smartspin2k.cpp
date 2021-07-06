#include "smartspin2k.h"
#include "ios/lockscreen.h"
#include "virtualbike.h"
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

smartspin2k::smartspin2k(bool noWriteResistance, bool noHeartService) {
    m_watt.setType(metric::METRIC_WATT);
    Speed.setType(metric::METRIC_SPEED);
    refresh = new QTimer(this);
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    initDone = false;
    connect(refresh, &QTimer::timeout, this, &smartspin2k::update);
    refresh->start(200ms);
}

void smartspin2k::resistanceReadFromTheBike(int8_t resistance) {

    static bool first = true;
    qDebug() << "resistanceReadFromTheBike startupResistance:" << startupResistance << "initRequest:" << initRequest;
    if(startupResistance == -1) {
        startupResistance = resistance;
    }
    if(initRequest && first) {

        first = false;
        uint8_t enable_syncmode[] = { 0x02, 0x1B, 0x01 };
        uint8_t disable_syncmode[] = { 0x02, 0x1B, 0x00 };
        writeCharacteristic(enable_syncmode, sizeof(enable_syncmode), "BLE_syncMode enabling", false, true);
        forceResistance(startupResistance);
        QThread::sleep(2);
        writeCharacteristic(disable_syncmode, sizeof(disable_syncmode), "BLE_syncMode disabling", false, true);
        initRequest = false;
    }
    Resistance = resistance;
}

void smartspin2k::writeCharacteristic(uint8_t *data, uint8_t data_len, const QString &info, bool disable_log,
                                      bool wait_for_response) {
    QEventLoop loop;
    QTimer timeout;
    if(wait_for_response)
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }
    else
    {
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(QLowEnergyCharacteristic,QByteArray)),
                &loop, SLOT(quit()));
        timeout.singleShot(300, &loop, SLOT(quit()));
    }

    gattCommunicationChannelService->writeCharacteristic(gattWriteCharacteristic, QByteArray((const char*)data, data_len));

    if (!disable_log) {
        emit debug(QStringLiteral(" >> ") + QByteArray((const char *)data, data_len).toHex(' ') +
                   QStringLiteral(" // ") + info);
    }

    loop.exec();
}

void smartspin2k::forceResistance(int8_t requestResistance) {

    uint8_t write[] = { 0x02, 0x17, 0x00, 0x00 };
    write[2] = (uint8_t)(requestResistance & 0xFF);
    write[3] = (uint8_t)(requestResistance >> 8);




    writeCharacteristic(write, sizeof(write), QStringLiteral("forceResistance ") + QString::number(requestResistance));
}

void smartspin2k::update() {
    if (m_control->state() == QLowEnergyController::UnconnectedState) {


        emit disconnected();
        return;
    }

    if (initRequest) {

        // if the first event from the bike occurs before connecting with SS2k, we have to force it
        if(startupResistance != -1)
            resistanceReadFromTheBike(startupResistance);
    }
    else if(bluetoothDevice.isValid() &&
       m_control->state() == QLowEnergyController::DiscoveredState //&&
       //gattCommunicationChannelService &&
       //gattWriteCharacteristic.isValid() &&
       //gattNotify1Characteristic.isValid() &&
       /*initDone*/)
    {
        update_metrics(true, watts());

        // updating the treadmill console every second
        if (sec1Update++ == (500 / refresh->interval())) {

            sec1Update = 0;
            // updateDisplay(elapsed);
        }

        if (requestResistance != -1) {
            if (requestResistance > 100) {
                requestResistance = 100;
            } // TODO, use the bluetooth value
            else if (requestResistance == 0) {
                requestResistance = 1;
            }

            if (requestResistance != currentResistance().value()) {
                emit debug(QStringLiteral("writing resistance ") + QString::number(requestResistance));


                forceResistance(requestResistance);
            }
            requestResistance = -1;
        }
        else {
            uint8_t read[] = { 0x01, 0x17 };
            writeCharacteristic(read, sizeof(read), QStringLiteral("polling"));









        }
    }
}

void smartspin2k::serviceDiscovered(const QBluetoothUuid &gatt) {
    emit debug(QStringLiteral("serviceDiscovered ") + gatt.toString());

}

void smartspin2k::characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {


    Q_UNUSED(characteristic);




    emit debug(QStringLiteral(" << ") + newValue.toHex(' '));





    lastPacket = newValue;

    if(newValue.length() >= 4 ) {
        Resistance = (int16_t)(((uint16_t)newValue.at(2)) + (((uint16_t)newValue.at(3)) << 8));
        qDebug() << "Resistance received from SS2k:" << Resistance.value();


















































































    }





}

void smartspin2k::stateChanged(QLowEnergyService::ServiceState state) {

    QBluetoothUuid _gattWriteCharacteristicId((QString)"77776277-7877-7774-4466-896665500001");
    //QBluetoothUuid _gattNotify1CharacteristicId((QString)"77776277-7877-7774-4466-896665500001");

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceState>();
    emit debug(QStringLiteral("BTLE stateChanged ") + QString::fromLocal8Bit(metaEnum.valueToKey(state)));

    if(state == QLowEnergyService::ServiceDiscovered)
    {
        //qDebug() << gattCommunicationChannelService->characteristics();


        gattWriteCharacteristic = gattCommunicationChannelService->characteristic(_gattWriteCharacteristicId);
        //gattNotify1Characteristic = gattCommunicationChannelService->characteristic(_gattNotify1CharacteristicId);
        Q_ASSERT(gattWriteCharacteristic.isValid());
        //Q_ASSERT(gattNotify1Characteristic.isValid());



        // establish hook into notifications
        connect(gattCommunicationChannelService, SIGNAL(characteristicChanged(QLowEnergyCharacteristic,QByteArray)),
                this, SLOT(characteristicChanged(QLowEnergyCharacteristic,QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)),
                this, SLOT(characteristicWritten(const QLowEnergyCharacteristic, const QByteArray)));
        connect(gattCommunicationChannelService, SIGNAL(error(QLowEnergyService::ServiceError)),
                this, SLOT(errorService(QLowEnergyService::ServiceError)));
        connect(gattCommunicationChannelService, SIGNAL(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)), this,
                SLOT(descriptorWritten(const QLowEnergyDescriptor, const QByteArray)));



        firstStateChanged = 1;












        QByteArray descriptor;
        descriptor.append((char)0x02);
        descriptor.append((char)0x00);
        gattCommunicationChannelService->writeDescriptor(gattWriteCharacteristic.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration), descriptor);














    }

}

void smartspin2k::descriptorWritten(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    emit debug(QStringLiteral("descriptorWritten ") + descriptor.name() + QStringLiteral(" ") + newValue.toHex(' '));


    initRequest = true;
    emit connectedAndDiscovered();
}

void smartspin2k::descriptorRead(const QLowEnergyDescriptor &descriptor, const QByteArray &newValue) {
    qDebug() << QStringLiteral("descriptorRead ") << descriptor.name() << descriptor.uuid() << newValue.toHex(' ');

}

void smartspin2k::characteristicWritten(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {

    Q_UNUSED(characteristic);
    emit debug(QStringLiteral("characteristicWritten ") + newValue.toHex(' '));
}

void smartspin2k::characteristicRead(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue) {
    qDebug() << QStringLiteral("characteristicRead ") << characteristic.uuid() << newValue.toHex(' ');

}

void smartspin2k::serviceScanDone(void) {
    emit debug(QStringLiteral("serviceScanDone"));


    QBluetoothUuid _gattCommunicationChannelServiceId((QString)"77776277-7877-7774-4466-896665500000");




    gattCommunicationChannelService = m_control->createServiceObject(_gattCommunicationChannelServiceId);
    connect(gattCommunicationChannelService, SIGNAL(stateChanged(QLowEnergyService::ServiceState)), this, SLOT(stateChanged(QLowEnergyService::ServiceState)));
    gattCommunicationChannelService->discoverDetails();



}

void smartspin2k::errorService(QLowEnergyService::ServiceError err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyService::ServiceError>();
    emit debug(QStringLiteral("smartspin2k::errorService") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void smartspin2k::error(QLowEnergyController::Error err) {

    QMetaEnum metaEnum = QMetaEnum::fromType<QLowEnergyController::Error>();
    emit debug(QStringLiteral("smartspin2k::error") + QString::fromLocal8Bit(metaEnum.valueToKey(err)) +
               m_control->errorString());
}

void smartspin2k::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("Found new device: ") + device.name() + QStringLiteral(" (") +
               device.address().toString() + ')');
    {
        bluetoothDevice = device;

        m_control = QLowEnergyController::createCentral(bluetoothDevice, this);
        connect(m_control, &QLowEnergyController::serviceDiscovered, this, &smartspin2k::serviceDiscovered);
        connect(m_control, &QLowEnergyController::discoveryFinished, this, &smartspin2k::serviceScanDone);
        connect(m_control,
                static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
                this, &smartspin2k::error);
        connect(m_control, &QLowEnergyController::stateChanged, this, &smartspin2k::controllerStateChanged);



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

bool smartspin2k::connected() {
    if (!m_control) {

        return false;
    }
    return m_control->state() == QLowEnergyController::DiscoveredState;
}

void *smartspin2k::VirtualBike() { return virtualBike; }




void *smartspin2k::VirtualDevice() { return VirtualBike(); }


uint16_t smartspin2k::watts() {
    if (currentCadence().value() == 0) {
        return 0;
    }

    return m_watt.value();
}

void smartspin2k::controllerStateChanged(QLowEnergyController::ControllerState state) {
    qDebug() << QStringLiteral("controllerStateChanged") << state;
    if (state == QLowEnergyController::UnconnectedState && m_control) {
        qDebug() << QStringLiteral("trying to connect back again...");


        initDone = false;
        m_control->connectToDevice();
    }
}