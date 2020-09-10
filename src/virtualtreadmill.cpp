#include "virtualtreadmill.h"

volatile double currentSpeed = 0;

virtualtreadmill::virtualtreadmill()
{
    //! [Advertising Data]
    QLowEnergyAdvertisingData advertisingData;
    advertisingData.setDiscoverability(QLowEnergyAdvertisingData::DiscoverabilityGeneral);
    advertisingData.setIncludePowerLevel(true);
    advertisingData.setLocalName("DomyosBridge");
    advertisingData.setServices(QList<QBluetoothUuid>() << (QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    //! [Advertising Data]

    //! [Service Data]
    QLowEnergyCharacteristicData charData;
    charData.setUuid((QBluetoothUuid::CharacteristicType)0x2ACC); //FitnessMachineFeatureCharacteristicUuid
    QByteArray value;
    value.append(0x08);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    value.append((char)0x00);
    charData.setValue(value);
    charData.setProperties(QLowEnergyCharacteristic::Read);
/*    const QLowEnergyDescriptorData clientConfig(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                QByteArray(2, 0));
    charData.addDescriptor(clientConfig);*/

    QLowEnergyCharacteristicData charData2;
    charData2.setUuid((QBluetoothUuid::CharacteristicType)0x2ACD); //TreadmillDataCharacteristicUuid
    charData2.setProperties(QLowEnergyCharacteristic::Notify | QLowEnergyCharacteristic::Read);
    QByteArray descriptor;
    descriptor.append((char)0x01);
    descriptor.append((char)0x00);
    const QLowEnergyDescriptorData clientConfig2(QBluetoothUuid::ClientCharacteristicConfiguration,
                                                descriptor);
    charData2.addDescriptor(clientConfig2);

    QLowEnergyServiceData serviceData;
    serviceData.setType(QLowEnergyServiceData::ServiceTypePrimary);
    serviceData.setUuid((QBluetoothUuid::ServiceClassUuid)0x1826); //FitnessMachineServiceUuid
    serviceData.addCharacteristic(charData);
    serviceData.addCharacteristic(charData2);
    //! [Service Data]

    //! [Start Advertising]
    const QScopedPointer<QLowEnergyController> leController(QLowEnergyController::createPeripheral());
    QScopedPointer<QLowEnergyService> service(leController->addService(serviceData));
    leController->startAdvertising(QLowEnergyAdvertisingParameters(), advertisingData,
                                   advertisingData);
    //! [Start Advertising]

    //! [Provide Heartbeat]
    QTimer treadmillTimer;

    const auto treadmillProvider = [&service]() {
        QByteArray value;
        value.append(0x08); // Flags that specify the format of the value.
        value.append(char(0x00)); // Flags that specify the format of the value.

        uint32_t normalizeSpeed = (uint32_t)qRound(currentSpeed * 100);
        char a = (normalizeSpeed >> 24) & 0XFF;
        char b = (normalizeSpeed >> 16) & 0XFF;
        char c = (normalizeSpeed >> 8) & 0XFF;
        char d = normalizeSpeed & 0XFF;
        QByteArray speedBytes;
        speedBytes.append(d);
        speedBytes.append(c);
        speedBytes.append(b);
        speedBytes.append(a);

        value.append(speedBytes); // Actual value.

        value.append(char(0x00));  //incline (not handled)
        value.append(char(0x00));

        value.append(char(0xFF));  //ramp angle (auto calculated)
        value.append(char(0x7F));

        QLowEnergyCharacteristic characteristic
                = service->characteristic((QBluetoothUuid::CharacteristicType)0x2ACD); //TreadmillDataCharacteristicUuid
        Q_ASSERT(characteristic.isValid());
        service->writeCharacteristic(characteristic, value); // Potentially causes notification.
    };
    QObject::connect(&treadmillTimer, &QTimer::timeout, treadmillProvider);
    treadmillTimer.start(1000);
    //! [Provide Heartbeat]

    auto reconnect = [&leController, advertisingData, &service, serviceData]()
    {
        service.reset(leController->addService(serviceData));
        if (!service.isNull())
            leController->startAdvertising(QLowEnergyAdvertisingParameters(),
                                           advertisingData, advertisingData);
    };
    QObject::connect(leController.data(), &QLowEnergyController::disconnected, reconnect);
}
