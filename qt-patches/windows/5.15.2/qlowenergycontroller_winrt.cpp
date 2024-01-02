/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the QtBluetooth module of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 3 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL3 included in the
** packaging of this file. Please review the following information to
** ensure the GNU Lesser General Public License version 3 requirements
** will be met: https://www.gnu.org/licenses/lgpl-3.0.html.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 2.0 or (at your option) the GNU General
** Public license version 3 or any later version approved by the KDE Free
** Qt Foundation. The licenses are as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL2 and LICENSE.GPL3
** included in the packaging of this file. Please review the following
** information to ensure the GNU General Public License requirements will
** be met: https://www.gnu.org/licenses/gpl-2.0.html and
** https://www.gnu.org/licenses/gpl-3.0.html.
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "qlowenergycontroller_winrt_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>

#ifdef CLASSIC_APP_BUILD
#define Q_OS_WINRT
#endif
#include <QtCore/qfunctions_winrt.h>
#include <QtCore/QtEndian>
#include <QtCore/QLoggingCategory>
#include <private/qeventdispatcher_winrt_p.h>

#include <functional>
#include <robuffer.h>
#include <windows.devices.enumeration.h>
#include <windows.devices.bluetooth.h>
#include <windows.foundation.collections.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Devices;
using namespace ABI::Windows::Devices::Bluetooth;
using namespace ABI::Windows::Devices::Bluetooth::GenericAttributeProfile;
using namespace ABI::Windows::Devices::Enumeration;
using namespace ABI::Windows::Storage::Streams;

QT_BEGIN_NAMESPACE

typedef ITypedEventHandler<BluetoothLEDevice *, IInspectable *> StatusHandler;
typedef ITypedEventHandler<GattCharacteristic *, GattValueChangedEventArgs *> ValueChangedHandler;
typedef GattReadClientCharacteristicConfigurationDescriptorResult ClientCharConfigDescriptorResult;
typedef IGattReadClientCharacteristicConfigurationDescriptorResult IClientCharConfigDescriptorResult;

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINRT)
Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINRT_SERVICE_THREAD)

static QByteArray byteArrayFromGattResult(const ComPtr<IGattReadResult> &gattResult, bool isWCharString = false)
{
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    HRESULT hr;
    hr = gattResult->get_Value(&buffer);
    Q_ASSERT_SUCCEEDED(hr);
    return byteArrayFromBuffer(buffer, isWCharString);
}

class QWinRTLowEnergyServiceHandler : public QObject
{
    Q_OBJECT
public:
    QWinRTLowEnergyServiceHandler(const QBluetoothUuid &service, const ComPtr<IGattDeviceService2> &deviceService)
        : mService(service)
        , mDeviceService(deviceService)
    {
        qCDebug(QT_BT_WINRT) << __FUNCTION__;
    }

    ~QWinRTLowEnergyServiceHandler()
    {
    }

public slots:
    void obtainCharList()
    {
        QVector<QBluetoothUuid> indicateChars;
        quint16 startHandle = 0;
        quint16 endHandle = 0;
        qCDebug(QT_BT_WINRT) << __FUNCTION__;
        ComPtr<IVectorView<GattCharacteristic *>> characteristics;
        HRESULT hr = mDeviceService->GetAllCharacteristics(&characteristics);
        Q_ASSERT_SUCCEEDED(hr);
        if (!characteristics) {
            emit charListObtained(mService, mCharacteristicList, indicateChars, startHandle, endHandle);
            QThread::currentThread()->quit();
            return;
        }

        uint characteristicsCount;
        hr = characteristics->get_Size(&characteristicsCount);
        Q_ASSERT_SUCCEEDED(hr);
        for (uint i = 0; i < characteristicsCount; ++i) {
            ComPtr<IGattCharacteristic> characteristic;
            hr = characteristics->GetAt(i, &characteristic);
            Q_ASSERT_SUCCEEDED(hr);
            quint16 handle;
            hr = characteristic->get_AttributeHandle(&handle);
            Q_ASSERT_SUCCEEDED(hr);
            QLowEnergyServicePrivate::CharData charData;
            charData.valueHandle = handle + 1;
            if (startHandle == 0 || startHandle > handle)
                startHandle = handle;
            if (endHandle == 0 || endHandle < handle)
                endHandle = handle;
            GUID guuid;
            hr = characteristic->get_Uuid(&guuid);
            Q_ASSERT_SUCCEEDED(hr);
            charData.uuid = QBluetoothUuid(guuid);
            GattCharacteristicProperties properties;
            hr = characteristic->get_CharacteristicProperties(&properties);
            Q_ASSERT_SUCCEEDED(hr);
            charData.properties = QLowEnergyCharacteristic::PropertyTypes(properties & 0xff);
            if (charData.properties & QLowEnergyCharacteristic::Read) {
                ComPtr<IAsyncOperation<GattReadResult *>> readOp;
                hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
                Q_ASSERT_SUCCEEDED(hr);
                ComPtr<IGattReadResult> readResult;
                hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf());
                Q_ASSERT_SUCCEEDED(hr);
                if (readResult)
                    charData.value = byteArrayFromGattResult(readResult);
            }
            ComPtr<IGattCharacteristic2> characteristic2;
            hr = characteristic.As(&characteristic2);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IVectorView<GattDescriptor *>> descriptors;
            hr = characteristic2->GetAllDescriptors(&descriptors);
            Q_ASSERT_SUCCEEDED(hr);
            uint descriptorCount;
            hr = descriptors->get_Size(&descriptorCount);
            Q_ASSERT_SUCCEEDED(hr);
            for (uint j = 0; j < descriptorCount; ++j) {
                QLowEnergyServicePrivate::DescData descData;
                ComPtr<IGattDescriptor> descriptor;
                hr = descriptors->GetAt(j, &descriptor);
                Q_ASSERT_SUCCEEDED(hr);
                quint16 descHandle;
                hr = descriptor->get_AttributeHandle(&descHandle);
                Q_ASSERT_SUCCEEDED(hr);
                GUID descriptorUuid;
                hr = descriptor->get_Uuid(&descriptorUuid);
                Q_ASSERT_SUCCEEDED(hr);
                descData.uuid = QBluetoothUuid(descriptorUuid);
                if (descData.uuid == QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration)) {
                    ComPtr<IAsyncOperation<ClientCharConfigDescriptorResult *>> readOp;
                    hr = characteristic->ReadClientCharacteristicConfigurationDescriptorAsync(&readOp);
                    Q_ASSERT_SUCCEEDED(hr);
                    ComPtr<IClientCharConfigDescriptorResult> readResult;
                    hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf());
                    Q_ASSERT_SUCCEEDED(hr);
                    GattClientCharacteristicConfigurationDescriptorValue value;
                    hr = readResult->get_ClientCharacteristicConfigurationDescriptor(&value);
                    Q_ASSERT_SUCCEEDED(hr);
                    quint16 result = 0;
                    bool correct = false;
                    if (value & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
                        result |= GattClientCharacteristicConfigurationDescriptorValue_Indicate;
                        correct = true;
                    }
                    if (value & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                        result |= GattClientCharacteristicConfigurationDescriptorValue_Notify;
                        correct = true;
                    }
                    if (value == GattClientCharacteristicConfigurationDescriptorValue_None) {
                        correct = true;
                    }
                    if (!correct)
                        continue;

                    descData.value = QByteArray(2, Qt::Uninitialized);
                    qToLittleEndian(result, descData.value.data());
                    indicateChars << charData.uuid;
                } else {
                    ComPtr<IAsyncOperation<GattReadResult *>> readOp;
                    hr = descriptor->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
                    Q_ASSERT_SUCCEEDED(hr);
                    ComPtr<IGattReadResult> readResult;
                    hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf());
                    Q_ASSERT_SUCCEEDED(hr);
                    if (descData.uuid == QBluetoothUuid::CharacteristicUserDescription)
                        descData.value = byteArrayFromGattResult(readResult, true);
                    else
                        descData.value = byteArrayFromGattResult(readResult);
                }
                charData.descriptorList.insert(descHandle, descData);
            }
            mCharacteristicList.insert(handle, charData);
        }
        emit charListObtained(mService, mCharacteristicList, indicateChars, startHandle, endHandle);
        QThread::currentThread()->quit();
    }

public:
    QBluetoothUuid mService;
    ComPtr<IGattDeviceService2> mDeviceService;
    QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> mCharacteristicList;

signals:
    void charListObtained(const QBluetoothUuid &service, QHash<QLowEnergyHandle,
                          QLowEnergyServicePrivate::CharData> charList,
                          QVector<QBluetoothUuid> indicateChars,
                          QLowEnergyHandle startHandle, QLowEnergyHandle endHandle);
};

QLowEnergyControllerPrivateWinRT::QLowEnergyControllerPrivateWinRT()
    : QLowEnergyControllerPrivate()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;

    registerQLowEnergyControllerMetaType();
    connect(this, &QLowEnergyControllerPrivateWinRT::characteristicChanged,
            this, &QLowEnergyControllerPrivateWinRT::handleCharacteristicChanged,
            Qt::QueuedConnection);
}

QLowEnergyControllerPrivateWinRT::~QLowEnergyControllerPrivateWinRT()
{
    if (mDevice && mStatusChangedToken.value)
        mDevice->remove_ConnectionStatusChanged(mStatusChangedToken);

    unregisterFromValueChanges();
}

void QLowEnergyControllerPrivateWinRT::init()
{
}

void QLowEnergyControllerPrivateWinRT::connectToDevice()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    ComPtr<IBluetoothLEDeviceStatics> deviceStatics;
    HRESULT hr = GetActivationFactory(HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(), &deviceStatics);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromBluetoothAddressAsync(remoteDevice.toUInt64(), &deviceFromIdOperation);
    Q_ASSERT_SUCCEEDED(hr);
    hr = QWinRTFunctions::await(deviceFromIdOperation, mDevice.GetAddressOf());
    Q_ASSERT_SUCCEEDED(hr);

    if (!mDevice) {
        qCDebug(QT_BT_WINRT) << "Could not find LE device";
        setError(QLowEnergyController::InvalidBluetoothAdapterError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }
    BluetoothConnectionStatus status;
    hr = mDevice->get_ConnectionStatus(&status);
    Q_ASSERT_SUCCEEDED(hr);
    hr = QEventDispatcherWinRT::runOnXamlThread([this, q]() {
        HRESULT hr;
        hr = mDevice->add_ConnectionStatusChanged(Callback<StatusHandler>([this, q](IBluetoothLEDevice *dev, IInspectable *) {
            BluetoothConnectionStatus status;
            HRESULT hr;
            hr = dev->get_ConnectionStatus(&status);
            Q_ASSERT_SUCCEEDED(hr);
            if (state == QLowEnergyController::ConnectingState
                    && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
                setState(QLowEnergyController::ConnectedState);
                emit q->connected();
            } else if (state != QLowEnergyController::UnconnectedState
                       && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Disconnected) {
                invalidateServices();
                unregisterFromValueChanges();
                setError(QLowEnergyController::RemoteHostClosedError);
                setState(QLowEnergyController::UnconnectedState);
                emit q->disconnected();
            }
            return S_OK;
        }).Get(), &mStatusChangedToken);
        Q_ASSERT_SUCCEEDED(hr);
        return S_OK;
    });
    Q_ASSERT_SUCCEEDED(hr);

    if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
        return;
    }

    ComPtr<IVectorView <GattDeviceService *>> deviceServices;
    hr = mDevice->get_GattServices(&deviceServices);
    Q_ASSERT_SUCCEEDED(hr);
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    // Windows Phone automatically connects to the device as soon as a service value is read/written.
    // Thus we read one value in order to establish the connection.
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> service;
        hr = deviceServices->GetAt(i, &service);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IGattDeviceService2> service2;
        hr = service.As(&service2);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IVectorView<GattCharacteristic *>> characteristics;
        hr = service2->GetAllCharacteristics(&characteristics);
        if (hr == E_ACCESSDENIED) {
            // Everything will work as expected up until this point if the manifest capabilties
            // for bluetooth LE are not set.
            qCWarning(QT_BT_WINRT) << "Could not obtain characteristic list. Please check your "
                                      "manifest capabilities";
            setState(QLowEnergyController::UnconnectedState);
            setError(QLowEnergyController::ConnectionError);
            return;
        } else {
            Q_ASSERT_SUCCEEDED(hr);
        }
        uint characteristicsCount;
        hr = characteristics->get_Size(&characteristicsCount);
        Q_ASSERT_SUCCEEDED(hr);
        for (uint j = 0; j < characteristicsCount; ++j) {
            ComPtr<IGattCharacteristic> characteristic;
            hr = characteristics->GetAt(j, &characteristic);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IAsyncOperation<GattReadResult *>> op;
            GattCharacteristicProperties props;
            hr = characteristic->get_CharacteristicProperties(&props);
            Q_ASSERT_SUCCEEDED(hr);
            if (!(props & GattCharacteristicProperties_Read))
                continue;
            /* QZ rviola
            hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode::BluetoothCacheMode_Uncached, &op);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IGattReadResult> result;
            hr = QWinRTFunctions::await(op, result.GetAddressOf());
            if (hr == E_INVALIDARG) {
                // E_INVALIDARG happens when user tries to connect to a device that was paired
                // before but is not available.
                qCDebug(QT_BT_WINRT) << "Could not obtain characteristic read result that triggers"
                                        "device connection. Is the device reachable?";
                setError(QLowEnergyController::ConnectionError);
                setState(QLowEnergyController::UnconnectedState);
                return;
            } else if (hr != S_OK) {
                qCWarning(QT_BT_WINRT) << "Connecting to device failed: "
                                       << qt_error_string(hr);
                setError(QLowEnergyController::ConnectionError);
                setState(QLowEnergyController::UnconnectedState);
                return;
            }
            ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
            hr = result->get_Value(&buffer);
            Q_ASSERT_SUCCEEDED(hr);
            if (!buffer) {
                qCDebug(QT_BT_WINRT) << "Problem reading value";
                setError(QLowEnergyController::ConnectionError);
                setState(QLowEnergyController::UnconnectedState);
            }
            */
            return;
        }
    }
}

void QLowEnergyControllerPrivateWinRT::disconnectFromDevice()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    setState(QLowEnergyController::ClosingState);
    unregisterFromValueChanges();
    if (mDevice) {
        if (mStatusChangedToken.value) {
            mDevice->remove_ConnectionStatusChanged(mStatusChangedToken);
            mStatusChangedToken.value = 0;
        }
        mDevice = nullptr;
    }
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

ComPtr<IGattDeviceService> QLowEnergyControllerPrivateWinRT::getNativeService(const QBluetoothUuid &serviceUuid)
{
    ComPtr<IGattDeviceService> deviceService;
    HRESULT hr;
    hr = mDevice->GetGattService(serviceUuid, &deviceService);
    if (FAILED(hr))
        qCDebug(QT_BT_WINRT) << "Could not obtain native service for Uuid" << serviceUuid;
    return deviceService;
}

ComPtr<IGattCharacteristic> QLowEnergyControllerPrivateWinRT::getNativeCharacteristic(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid)
{
    ComPtr<IGattDeviceService> service = getNativeService(serviceUuid);
    if (!service)
        return nullptr;

    ComPtr<IVectorView<GattCharacteristic *>> characteristics;
    HRESULT hr = service->GetCharacteristics(charUuid, &characteristics);
    RETURN_IF_FAILED("Could not obtain native characteristics for service", return nullptr);
    ComPtr<IGattCharacteristic> characteristic;
    hr = characteristics->GetAt(0, &characteristic);
    RETURN_IF_FAILED("Could not obtain first characteristic for service", return nullptr);
    return characteristic;
}

void QLowEnergyControllerPrivateWinRT::registerForValueChanges(const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid)
{
    qCDebug(QT_BT_WINRT) << "Registering characteristic" << charUuid << "in service"
                         << serviceUuid << "for value changes";
    for (const ValueChangedEntry &entry : qAsConst(mValueChangedTokens)) {
        GUID guuid;
        HRESULT hr;
        hr = entry.characteristic->get_Uuid(&guuid);
        Q_ASSERT_SUCCEEDED(hr);
        if (QBluetoothUuid(guuid) == charUuid)
            return;
    }
    ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(serviceUuid, charUuid);

    EventRegistrationToken token;
    HRESULT hr;
    hr = characteristic->add_ValueChanged(Callback<ValueChangedHandler>([this](IGattCharacteristic *characteristic, IGattValueChangedEventArgs *args) {
        HRESULT hr;
        quint16 handle;
        hr = characteristic->get_AttributeHandle(&handle);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<IBuffer> buffer;
        hr = args->get_CharacteristicValue(&buffer);
        Q_ASSERT_SUCCEEDED(hr);
        emit characteristicChanged(handle, byteArrayFromBuffer(buffer));
        return S_OK;
    }).Get(), &token);
    Q_ASSERT_SUCCEEDED(hr);
    mValueChangedTokens.append(ValueChangedEntry(characteristic, token));
    qCDebug(QT_BT_WINRT) << "Characteristic" << charUuid << "in service"
        << serviceUuid << "registered for value changes";
}

void QLowEnergyControllerPrivateWinRT::unregisterFromValueChanges()
{
    qCDebug(QT_BT_WINRT) << "Unregistering " << mValueChangedTokens.count() << " value change tokens";
    HRESULT hr;
    for (const ValueChangedEntry &entry : qAsConst(mValueChangedTokens)) {
        hr = entry.characteristic->remove_ValueChanged(entry.token);
        Q_ASSERT_SUCCEEDED(hr);
    }
    mValueChangedTokens.clear();
}

void QLowEnergyControllerPrivateWinRT::obtainIncludedServices(QSharedPointer<QLowEnergyServicePrivate> servicePointer,
    ComPtr<IGattDeviceService> service)
{
    Q_Q(QLowEnergyController);
    ComPtr<IGattDeviceService2> service2;
    HRESULT hr = service.As(&service2);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IVectorView<GattDeviceService *>> includedServices;
    hr = service2->GetAllIncludedServices(&includedServices);
    // Some devices return ERROR_ACCESS_DISABLED_BY_POLICY
    if (FAILED(hr))
        return;

    uint count;
    hr = includedServices->get_Size(&count);
    Q_ASSERT_SUCCEEDED(hr);
    for (uint i = 0; i < count; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = includedServices->GetAt(i, &includedService);
        Q_ASSERT_SUCCEEDED(hr);
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        Q_ASSERT_SUCCEEDED(hr);
        const QBluetoothUuid includedUuid(guuid);
        QSharedPointer<QLowEnergyServicePrivate> includedPointer;
        qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
        if (serviceList.contains(includedUuid)) {
            includedPointer = serviceList.value(includedUuid);
        } else {
            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = includedUuid;
            priv->setController(this);

            includedPointer = QSharedPointer<QLowEnergyServicePrivate>(priv);
            serviceList.insert(includedUuid, includedPointer);
        }
        qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
        includedPointer->type |= QLowEnergyService::IncludedService;
        servicePointer->includedServices.append(includedUuid);

        obtainIncludedServices(includedPointer, includedService);

        emit q->serviceDiscovered(includedUuid);
    }
}

void QLowEnergyControllerPrivateWinRT::discoverServices()
{
    Q_Q(QLowEnergyController);

    qCDebug(QT_BT_WINRT) << "Service discovery initiated";
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    HRESULT hr = mDevice->get_GattServices(&deviceServices);
    Q_ASSERT_SUCCEEDED(hr);
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> deviceService;
        hr = deviceServices->GetAt(i, &deviceService);
        Q_ASSERT_SUCCEEDED(hr);
        GUID guuid;
        hr = deviceService->get_Uuid(&guuid);
        Q_ASSERT_SUCCEEDED(hr);
        const QBluetoothUuid service(guuid);

        qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__
                                            << "Changing service pointer from thread"
                                            << QThread::currentThread();
        QSharedPointer<QLowEnergyServicePrivate> pointer;
        if (serviceList.contains(service)) {
            pointer = serviceList.value(service);
        } else {
            QLowEnergyServicePrivate *priv = new QLowEnergyServicePrivate();
            priv->uuid = service;
            priv->setController(this);

            pointer = QSharedPointer<QLowEnergyServicePrivate>(priv);
            serviceList.insert(service, pointer);
        }
        pointer->type |= QLowEnergyService::PrimaryService;

        obtainIncludedServices(pointer, deviceService);

        emit q->serviceDiscovered(service);
    }

    setState(QLowEnergyController::DiscoveredState);
    emit q->discoveryFinished();
}

void QLowEnergyControllerPrivateWinRT::discoverServiceDetails(const QBluetoothUuid &service)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << service;
    if (!serviceList.contains(service)) {
        qCWarning(QT_BT_WINRT) << "Discovery done of unknown service:"
            << service.toString();
        return;
    }

    ComPtr<IGattDeviceService> deviceService = getNativeService(service);
    if (!deviceService) {
        qCDebug(QT_BT_WINRT) << "Could not obtain native service for uuid " << service;
        return;
    }

    //update service data
    QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();

    pointer->setState(QLowEnergyService::DiscoveringServices);
    ComPtr<IGattDeviceService2> deviceService2;
    HRESULT hr = deviceService.As(&deviceService2);
    Q_ASSERT_SUCCEEDED(hr);
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    hr = deviceService2->GetAllIncludedServices(&deviceServices);
    if (FAILED(hr)) { // ERROR_ACCESS_DISABLED_BY_POLICY
        qCDebug(QT_BT_WINRT) << "Could not obtain included services list for" << service;
        pointer->setError(QLowEnergyService::UnknownError);
        pointer->setState(QLowEnergyService::InvalidService);
        return;
    }
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    Q_ASSERT_SUCCEEDED(hr);
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = deviceServices->GetAt(i, &includedService);
        Q_ASSERT_SUCCEEDED(hr);
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        Q_ASSERT_SUCCEEDED(hr);

        const QBluetoothUuid service(guuid);
        if (service.isNull()) {
            qCDebug(QT_BT_WINRT) << "Could not find service";
            return;
        }

        pointer->includedServices.append(service);

        // update the type of the included service
        QSharedPointer<QLowEnergyServicePrivate> otherService = serviceList.value(service);
        if (!otherService.isNull())
            otherService->type |= QLowEnergyService::IncludedService;
    }

    QWinRTLowEnergyServiceHandler *worker = new QWinRTLowEnergyServiceHandler(service, deviceService2);
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &QWinRTLowEnergyServiceHandler::obtainCharList);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QWinRTLowEnergyServiceHandler::charListObtained,
            [this, thread](const QBluetoothUuid &service, QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> charList
            , QVector<QBluetoothUuid> indicateChars
            , QLowEnergyHandle startHandle, QLowEnergyHandle endHandle) {
        if (!serviceList.contains(service)) {
            qCWarning(QT_BT_WINRT) << "Discovery done of unknown service:"
                                   << service.toString();
            return;
        }

        QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
        pointer->startHandle = startHandle;
        pointer->endHandle = endHandle;
        pointer->characteristicList = charList;

        HRESULT hr;
        hr = QEventDispatcherWinRT::runOnXamlThread([indicateChars, service, this]() {
            for (const QBluetoothUuid &indicateChar : qAsConst(indicateChars))
                registerForValueChanges(service, indicateChar);
            return S_OK;
        });
        Q_ASSERT_SUCCEEDED(hr);

        pointer->setState(QLowEnergyService::ServiceDiscovered);
        thread->exit(0);
    });
    thread->start();
}

void QLowEnergyControllerPrivateWinRT::startAdvertising(const QLowEnergyAdvertisingParameters &, const QLowEnergyAdvertisingData &, const QLowEnergyAdvertisingData &)
{
    setError(QLowEnergyController::AdvertisingError);
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::stopAdvertising()
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::requestConnectionUpdate(const QLowEnergyConnectionParameters &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::readCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
                        const QLowEnergyHandle charHandle)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << service << charHandle;
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::CharacteristicReadError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINRT) << charHandle << "could not be found in service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicReadError);
        return;
    }

    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([charHandle, service, this]() {
        const QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
        if (!(charData.properties & QLowEnergyCharacteristic::Read))
            qCDebug(QT_BT_WINRT) << "Read flag is not set for characteristic" << charData.uuid;

        ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(service->uuid, charData.uuid);
        if (!characteristic) {
            qCDebug(QT_BT_WINRT) << "Could not obtain native characteristic" << charData.uuid
                                 << "from service" << service->uuid;
            service->setError(QLowEnergyService::CharacteristicReadError);
            return S_OK;
        }
        ComPtr<IAsyncOperation<GattReadResult*>> readOp;
        HRESULT hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
        Q_ASSERT_SUCCEEDED(hr);
        auto readCompletedLambda = [charData, charHandle, service]
                (IAsyncOperation<GattReadResult*> *op, AsyncStatus status)
        {
            if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "read operation failed.";
                service->setError(QLowEnergyService::CharacteristicReadError);
                return S_OK;
            }
            ComPtr<IGattReadResult> characteristicValue;
            HRESULT hr;
            hr = op->GetResults(&characteristicValue);
            if (FAILED(hr)) {
                qCDebug(QT_BT_WINRT) << "Could not obtain result for characteristic" << charHandle;
                service->setError(QLowEnergyService::CharacteristicReadError);
                return S_OK;
            }

            const QByteArray value = byteArrayFromGattResult(characteristicValue);
            QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
            charData.value = value;
            service->characteristicList.insert(charHandle, charData);
            emit service->characteristicRead(QLowEnergyCharacteristic(service, charHandle), value);
            return S_OK;
        };
        hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattReadResult *>>(readCompletedLambda).Get());
        Q_ASSERT_SUCCEEDED(hr);
        return S_OK;
    });
    Q_ASSERT_SUCCEEDED(hr);
}

void QLowEnergyControllerPrivateWinRT::readDescriptor(const QSharedPointer<QLowEnergyServicePrivate> service,
                    const QLowEnergyHandle charHandle,
                    const QLowEnergyHandle descHandle)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << service << charHandle << descHandle;
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::DescriptorReadError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "in characteristic" << charHandle
                             << "cannot be found in service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorReadError);
        return;
    }

    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([charHandle, descHandle, service, this]() {
        const QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
        ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(service->uuid, charData.uuid);
        if (!characteristic) {
            qCDebug(QT_BT_WINRT) << "Could not obtain native characteristic" << charData.uuid
                                 << "from service" << service->uuid;
            service->setError(QLowEnergyService::DescriptorReadError);
            return S_OK;
        }

        // Get native descriptor
        if (!charData.descriptorList.contains(descHandle))
            qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "cannot be found in characteristic" << charHandle;
        const QLowEnergyServicePrivate::DescData descData = charData.descriptorList.value(descHandle);
        const QBluetoothUuid descUuid = descData.uuid;
        if (descUuid == QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration)) {
            ComPtr<IAsyncOperation<ClientCharConfigDescriptorResult *>> readOp;
            HRESULT hr = characteristic->ReadClientCharacteristicConfigurationDescriptorAsync(&readOp);
            Q_ASSERT_SUCCEEDED(hr);
            auto readCompletedLambda = [charHandle, descHandle, service]
                    (IAsyncOperation<ClientCharConfigDescriptorResult *> *op, AsyncStatus status)
            {
                if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "read operation failed";
                    service->setError(QLowEnergyService::DescriptorReadError);
                    return S_OK;
                }
                ComPtr<IClientCharConfigDescriptorResult> iValue;
                HRESULT hr;
                hr = op->GetResults(&iValue);
                if (FAILED(hr)) {
                    qCDebug(QT_BT_WINRT) << "Could not obtain result for descriptor" << descHandle;
                    service->setError(QLowEnergyService::DescriptorReadError);
                    return S_OK;
                }
                GattClientCharacteristicConfigurationDescriptorValue value;
                hr = iValue->get_ClientCharacteristicConfigurationDescriptor(&value);
                if (FAILED(hr)) {
                    qCDebug(QT_BT_WINRT) << "Could not obtain value for descriptor" << descHandle;
                    service->setError(QLowEnergyService::DescriptorReadError);
                    return S_OK;
                }
                quint16 result = 0;
                bool correct = false;
                if (value & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
                    result |= QLowEnergyCharacteristic::Indicate;
                    correct = true;
                }
                if (value & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                    result |= QLowEnergyCharacteristic::Notify;
                    correct = true;
                }
                if (value == GattClientCharacteristicConfigurationDescriptorValue_None)
                    correct = true;
                if (!correct) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle
                                         << "read operation failed. Obtained unexpected value.";
                    service->setError(QLowEnergyService::DescriptorReadError);
                    return S_OK;
                }
                QLowEnergyServicePrivate::DescData descData;
                descData.uuid = QBluetoothUuid::ClientCharacteristicConfiguration;
                descData.value = QByteArray(2, Qt::Uninitialized);
                qToLittleEndian(result, descData.value.data());
                service->characteristicList[charHandle].descriptorList[descHandle] = descData;
                emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                    descData.value);
                return S_OK;
            };
            hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<ClientCharConfigDescriptorResult *>>(readCompletedLambda).Get());
            Q_ASSERT_SUCCEEDED(hr);
            return S_OK;
        } else {
            ComPtr<IVectorView<GattDescriptor *>> descriptors;
            HRESULT hr = characteristic->GetDescriptors(descData.uuid, &descriptors);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IGattDescriptor> descriptor;
            hr = descriptors->GetAt(0, &descriptor);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IAsyncOperation<GattReadResult*>> readOp;
            hr = descriptor->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
            Q_ASSERT_SUCCEEDED(hr);
            auto readCompletedLambda = [charHandle, descHandle, descUuid, service]
                    (IAsyncOperation<GattReadResult*> *op, AsyncStatus status)
            {
                if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "read operation failed";
                    service->setError(QLowEnergyService::DescriptorReadError);
                    return S_OK;
                }
                ComPtr<IGattReadResult> descriptorValue;
                HRESULT hr;
                hr = op->GetResults(&descriptorValue);
                if (FAILED(hr)) {
                    qCDebug(QT_BT_WINRT) << "Could not obtain result for descriptor" << descHandle;
                    service->setError(QLowEnergyService::DescriptorReadError);
                    return S_OK;
                }
                QLowEnergyServicePrivate::DescData descData;
                if (descUuid == QBluetoothUuid::CharacteristicUserDescription)
                    descData.value = byteArrayFromGattResult(descriptorValue, true);
                else
                    descData.value = byteArrayFromGattResult(descriptorValue);
                service->characteristicList[charHandle].descriptorList[descHandle] = descData;
                emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                    descData.value);
                return S_OK;
            };
            hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattReadResult *>>(readCompletedLambda).Get());
            return S_OK;
        }
    });
    Q_ASSERT_SUCCEEDED(hr);
}

void QLowEnergyControllerPrivateWinRT::writeCharacteristic(const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QByteArray &newValue,
        QLowEnergyService::WriteMode mode)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << service << charHandle << newValue << mode;
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::CharacteristicWriteError);
        Q_UNIMPLEMENTED();
        return;
    }
    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "cannot be found in service" << service->uuid;
        service->setError(QLowEnergyService::CharacteristicWriteError);
        return;
    }

    QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
    const bool writeWithResponse = mode == QLowEnergyService::WriteWithResponse;
    if (!(charData.properties & (writeWithResponse ? QLowEnergyCharacteristic::Write : QLowEnergyCharacteristic::WriteNoResponse)))
        qCDebug(QT_BT_WINRT) << "Write flag is not set for characteristic" << charHandle;

    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([charData, charHandle, this, service, newValue, writeWithResponse]() {
        ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(service->uuid, charData.uuid);
        if (!characteristic) {
            qCDebug(QT_BT_WINRT) << "Could not obtain native characteristic" << charData.uuid
                                 << "from service" << service->uuid;
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return S_OK;
        }
        ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
        HRESULT hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(), &bufferFactory);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
        const int length = newValue.length();
        hr = bufferFactory->Create(length, &buffer);
        Q_ASSERT_SUCCEEDED(hr);
        hr = buffer->put_Length(length);
        Q_ASSERT_SUCCEEDED(hr);
        ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
        hr = buffer.As(&byteAccess);
        Q_ASSERT_SUCCEEDED(hr);
        byte *bytes;
        hr = byteAccess->Buffer(&bytes);
        Q_ASSERT_SUCCEEDED(hr);
        memcpy(bytes, newValue, length);
        ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
        GattWriteOption option = writeWithResponse ? GattWriteOption_WriteWithResponse : GattWriteOption_WriteWithoutResponse;
        hr = characteristic->WriteValueWithOptionAsync(buffer.Get(), option, &writeOp);
        Q_ASSERT_SUCCEEDED(hr);
        auto writeCompletedLambda =[charData, charHandle, newValue, service, writeWithResponse, this]
                (IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
        {
            if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "write operation failed";
                service->setError(QLowEnergyService::CharacteristicWriteError);
                return S_OK;
            }
            GattCommunicationStatus result;
            HRESULT hr;
            hr = op->GetResults(&result);
            if (hr == E_BLUETOOTH_ATT_INVALID_ATTRIBUTE_VALUE_LENGTH) {
                qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "write operation was tried with invalid value length";
                service->setError(QLowEnergyService::CharacteristicWriteError);
                return S_OK;
            }
            Q_ASSERT_SUCCEEDED(hr);
            if (result != GattCommunicationStatus_Success) {
                qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "write operation failed";
                service->setError(QLowEnergyService::CharacteristicWriteError);
                return S_OK;
            }
            // only update cache when property is readable. Otherwise it remains
            // empty.
            if (charData.properties & QLowEnergyCharacteristic::Read)
                updateValueOfCharacteristic(charHandle, newValue, false);
            if (writeWithResponse)
                emit service->characteristicWritten(QLowEnergyCharacteristic(service, charHandle), newValue);
            return S_OK;
        };
        hr = writeOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>(writeCompletedLambda).Get());
        Q_ASSERT_SUCCEEDED(hr);
        return S_OK;
    });
    Q_ASSERT_SUCCEEDED(hr);
}

void QLowEnergyControllerPrivateWinRT::writeDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
        const QLowEnergyHandle charHandle,
        const QLowEnergyHandle descHandle,
        const QByteArray &newValue)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << service << charHandle << descHandle << newValue;
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    Q_ASSERT(!service.isNull());
    if (role == QLowEnergyController::PeripheralRole) {
        service->setError(QLowEnergyService::DescriptorWriteError);
        Q_UNIMPLEMENTED();
        return;
    }

    if (!service->characteristicList.contains(charHandle)) {
        qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "in characteristic" << charHandle
                             << "could not be found in service" << service->uuid;
        service->setError(QLowEnergyService::DescriptorWriteError);
        return;
    }

    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([charHandle, descHandle, this, service, newValue]() {
        const QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
        ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(service->uuid, charData.uuid);
        if (!characteristic) {
            qCDebug(QT_BT_WINRT) << "Could not obtain native characteristic" << charData.uuid
                                 << "from service" << service->uuid;
            service->setError(QLowEnergyService::DescriptorWriteError);
            return S_OK;
        }

        // Get native descriptor
        if (!charData.descriptorList.contains(descHandle))
            qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "could not be found in Characteristic" << charHandle;

        QLowEnergyServicePrivate::DescData descData = charData.descriptorList.value(descHandle);
        if (descData.uuid == QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration)) {
            GattClientCharacteristicConfigurationDescriptorValue value;
            quint16 intValue = qFromLittleEndian<quint16>(newValue);
            if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Indicate && intValue & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                qCWarning(QT_BT_WINRT) << "Setting both Indicate and Notify is not supported on WinRT";
                value = (GattClientCharacteristicConfigurationDescriptorValue)(GattClientCharacteristicConfigurationDescriptorValue_Indicate | GattClientCharacteristicConfigurationDescriptorValue_Notify);
            } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
                value = GattClientCharacteristicConfigurationDescriptorValue_Indicate;
            } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                value = GattClientCharacteristicConfigurationDescriptorValue_Notify;
            } else if (intValue == 0) {
                value = GattClientCharacteristicConfigurationDescriptorValue_None;
            } else {
                qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed: Invalid value";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return S_OK;
            }
            ComPtr<IAsyncOperation<enum GattCommunicationStatus>> writeOp;
            HRESULT hr = characteristic->WriteClientCharacteristicConfigurationDescriptorAsync(value, &writeOp);
            Q_ASSERT_SUCCEEDED(hr);
            auto writeCompletedLambda = [charHandle, descHandle, newValue, service, this]
                    (IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
            {
                if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed";
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                GattCommunicationStatus result;
                HRESULT hr;
                hr = op->GetResults(&result);
                if (FAILED(hr)) {
                    qCDebug(QT_BT_WINRT) << "Could not obtain result for descriptor" << descHandle;
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                if (result != GattCommunicationStatus_Success) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed";
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                updateValueOfDescriptor(charHandle, descHandle, newValue, false);
                emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle), newValue);
                return S_OK;
            };
            hr = writeOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus >>(writeCompletedLambda).Get());
            Q_ASSERT_SUCCEEDED(hr);
        } else {
            ComPtr<IVectorView<GattDescriptor *>> descriptors;
            HRESULT hr = characteristic->GetDescriptors(descData.uuid, &descriptors);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<IGattDescriptor> descriptor;
            hr = descriptors->GetAt(0, &descriptor);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
            hr = GetActivationFactory(HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(), &bufferFactory);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
            const int length = newValue.length();
            hr = bufferFactory->Create(length, &buffer);
            Q_ASSERT_SUCCEEDED(hr);
            hr = buffer->put_Length(length);
            Q_ASSERT_SUCCEEDED(hr);
            ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
            hr = buffer.As(&byteAccess);
            Q_ASSERT_SUCCEEDED(hr);
            byte *bytes;
            hr = byteAccess->Buffer(&bytes);
            Q_ASSERT_SUCCEEDED(hr);
            memcpy(bytes, newValue, length);
            ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
            hr = descriptor->WriteValueAsync(buffer.Get(), &writeOp);
            Q_ASSERT_SUCCEEDED(hr);
            auto writeCompletedLambda = [charHandle, descHandle, newValue, service, this]
                    (IAsyncOperation<GattCommunicationStatus> *op, AsyncStatus status)
            {
                if (status == AsyncStatus::Canceled || status == AsyncStatus::Error) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed";
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                GattCommunicationStatus result;
                HRESULT hr;
                hr = op->GetResults(&result);
                if (FAILED(hr)) {
                    qCDebug(QT_BT_WINRT) << "Could not obtain result for descriptor" << descHandle;
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                if (result != GattCommunicationStatus_Success) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed";
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                updateValueOfDescriptor(charHandle, descHandle, newValue, false);
                emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle), newValue);
                return S_OK;
            };
            hr = writeOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>(writeCompletedLambda).Get());
            Q_ASSERT_SUCCEEDED(hr);
            return S_OK;
        }
        return S_OK;
    });
    Q_ASSERT_SUCCEEDED(hr);
}


void QLowEnergyControllerPrivateWinRT::addToGenericAttributeList(const QLowEnergyServiceData &, QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRT::handleCharacteristicChanged(
        quint16 charHandle, const QByteArray &data)
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << charHandle << data;
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    QSharedPointer<QLowEnergyServicePrivate> service =
            serviceForHandle(charHandle);
    if (service.isNull())
        return;

    qCDebug(QT_BT_WINRT) << "Characteristic change notification" << service->uuid
                           << charHandle << data.toHex();

    QLowEnergyCharacteristic characteristic = characteristicForHandle(charHandle);
    if (!characteristic.isValid()) {
        qCWarning(QT_BT_WINRT) << "characteristicChanged: Cannot find characteristic";
        return;
    }

    // only update cache when property is readable. Otherwise it remains
    // empty.
    if (characteristic.properties() & QLowEnergyCharacteristic::Read)
        updateValueOfCharacteristic(characteristic.attributeHandle(),
                                data, false);
    emit service->characteristicChanged(characteristic, data);
}

QT_END_NAMESPACE

#include "qlowenergycontroller_winrt.moc"
