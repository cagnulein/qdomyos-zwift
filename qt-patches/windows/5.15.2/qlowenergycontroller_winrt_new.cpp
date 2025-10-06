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

#include "qlowenergycontroller_winrt_new_p.h"
#include "qlowenergycontroller_winrt_p.h"
#include "qbluetoothutils_winrt_p.h"

#include <QtBluetooth/qbluetoothlocaldevice.h>
#include <QtBluetooth/QLowEnergyCharacteristicData>
#include <QtBluetooth/QLowEnergyDescriptorData>
#include <QtBluetooth/private/qbluetoothutils_winrt_p.h>

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
#include <windows.devices.bluetooth.genericattributeprofile.h>
#include <windows.foundation.collections.h>
#include <windows.foundation.metadata.h>
#include <windows.storage.streams.h>

using namespace Microsoft::WRL;
using namespace Microsoft::WRL::Wrappers;
using namespace ABI::Windows::Foundation;
using namespace ABI::Windows::Foundation::Collections;
using namespace ABI::Windows::Foundation::Metadata;
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

#define EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr, ret) \
    if (FAILED(hr)) { \
        emitErrorAndQuitThread(hr); \
        ret; \
    }

#define WARN_AND_CONTINUE_IF_FAILED(hr, msg) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINRT) << msg; \
        continue; \
    }

#define CHECK_FOR_DEVICE_CONNECTION_ERROR_IMPL(this, hr, msg, ret) \
    if (FAILED(hr)) { \
        qCWarning(QT_BT_WINRT) << msg; \
        this->unregisterFromStatusChanges(); \
        this->setError(QLowEnergyController::ConnectionError); \
        this->setState(QLowEnergyController::UnconnectedState); \
        ret; \
    }

#define CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, msg, ret) \
    CHECK_FOR_DEVICE_CONNECTION_ERROR_IMPL(this, hr, msg, ret)

#define CHECK_HR_AND_SET_SERVICE_ERROR(hr, msg, service, error, ret) \
    if (FAILED(hr)) { \
        qCDebug(QT_BT_WINRT) << msg; \
        service->setError(error); \
        ret; \
    }

Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINRT)
Q_DECLARE_LOGGING_CATEGORY(QT_BT_WINRT_SERVICE_THREAD)

QLowEnergyControllerPrivate *createWinRTLowEnergyController()
{
    if (supportsNewLEApi()) {
        qCDebug(QT_BT_WINRT) << "Using new low energy controller";
        return new QLowEnergyControllerPrivateWinRTNew();
    }

    qCDebug(QT_BT_WINRT) << "Using pre 15063 low energy controller";
    return new QLowEnergyControllerPrivateWinRT();
}

static QByteArray byteArrayFromGattResult(const ComPtr<IGattReadResult> &gattResult,
                                          bool isWCharString = false)
{
    ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
    HRESULT hr;
    hr = gattResult->get_Value(&buffer);
    if (FAILED(hr) || !buffer) {
        qCWarning(QT_BT_WINRT) << "Could not obtain buffer from GattReadResult";
        return QByteArray();
    }
    return byteArrayFromBuffer(buffer, isWCharString);
}

class QWinRTLowEnergyServiceHandlerNew : public QObject
{
    Q_OBJECT
public:
    QWinRTLowEnergyServiceHandlerNew(const QBluetoothUuid &service,
                                     const ComPtr<IGattDeviceService3> &deviceService)
        : mService(service)
        , mDeviceService(deviceService)
    {
        qCDebug(QT_BT_WINRT) << __FUNCTION__;
    }

    ~QWinRTLowEnergyServiceHandlerNew()
    {
    }

public slots:
    void obtainCharList()
    {
        mIndicateChars.clear();
        qCDebug(QT_BT_WINRT) << __FUNCTION__;
        ComPtr<IAsyncOperation<GattCharacteristicsResult *>> characteristicsOp;
        ComPtr<IGattCharacteristicsResult> characteristicsResult;
        HRESULT hr = mDeviceService->GetCharacteristicsAsync(&characteristicsOp);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr, return);
        hr = QWinRTFunctions::await(characteristicsOp, characteristicsResult.GetAddressOf(),
                                    QWinRTFunctions::ProcessMainThreadEvents, 5000);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr, return);
        GattCommunicationStatus status;
        hr = characteristicsResult->get_Status(&status);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr, return);
        if (status != GattCommunicationStatus_Success) {
            emitErrorAndQuitThread(QLatin1String("Could not obtain char list"));
            return;
        }
        ComPtr<IVectorView<GattCharacteristic *>> characteristics;
        hr = characteristicsResult->get_Characteristics(&characteristics);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr, return);

        uint characteristicsCount;
        hr = characteristics->get_Size(&characteristicsCount);
        EMIT_WORKER_ERROR_AND_QUIT_IF_FAILED(hr, return);

        mCharacteristicsCountToBeDiscovered = characteristicsCount;
        for (uint i = 0; i < characteristicsCount; ++i) {
            ComPtr<IGattCharacteristic> characteristic;
            hr = characteristics->GetAt(i, &characteristic);
            if (FAILED(hr)) {
                qCWarning(QT_BT_WINRT) << "Could not obtain characteristic at" << i;
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }

            ComPtr<IGattCharacteristic3> characteristic3;
            hr = characteristic.As(&characteristic3);
            if (FAILED(hr)) {
                qCWarning(QT_BT_WINRT) << "Could not cast characteristic";
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }

            // For some strange reason, Windows doesn't discover descriptors of characteristics (if not paired).
            // Qt API assumes that all characteristics and their descriptors are discovered in one go.
            // So we start 'GetDescriptorsAsync' for each discovered characteristic and finish only
            // when GetDescriptorsAsync for all characteristics return.
            ComPtr<IAsyncOperation<GattDescriptorsResult*>> descAsyncResult;
            hr = characteristic3->GetDescriptorsAsync(&descAsyncResult);
            if (FAILED(hr)) {
                qCWarning(QT_BT_WINRT) << "Could not obtain list of descriptors";
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }
            hr = descAsyncResult->put_Completed(
                        Callback<IAsyncOperationCompletedHandler<GattDescriptorsResult*>>(
                            [this, characteristic]
                            (IAsyncOperation<GattDescriptorsResult *> *op,
                            AsyncStatus status) {
                if (status != AsyncStatus::Completed) {
                    qCWarning(QT_BT_WINRT) << "Descriptor operation unsuccessful";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }
                quint16 handle;

                HRESULT hr = characteristic->get_AttributeHandle(&handle);
                if (FAILED(hr)) {
                    qCWarning(QT_BT_WINRT) << "Could not obtain characteristic's attribute handle";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }
                QLowEnergyServicePrivate::CharData charData;
                charData.valueHandle = handle + 1;
                if (mStartHandle == 0 || mStartHandle > handle)
                    mStartHandle = handle;
                if (mEndHandle == 0 || mEndHandle < handle)
                    mEndHandle = handle;
                GUID guuid;
                hr = characteristic->get_Uuid(&guuid);
                if (FAILED(hr)) {
                    qCWarning(QT_BT_WINRT) << "Could not obtain characteristic's Uuid";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }
                charData.uuid = QBluetoothUuid(guuid);
                GattCharacteristicProperties properties;
                hr = characteristic->get_CharacteristicProperties(&properties);
                if (FAILED(hr)) {
                    qCWarning(QT_BT_WINRT) << "Could not obtain characteristic's properties";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }
                charData.properties = QLowEnergyCharacteristic::PropertyTypes(properties & 0xff);
                if (charData.properties & QLowEnergyCharacteristic::Read) {
                    ComPtr<IAsyncOperation<GattReadResult *>> readOp;
                    hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached,
                                                                     &readOp);
                    if (FAILED(hr)) {
                        qCWarning(QT_BT_WINRT) << "Could not read characteristic";
                        --mCharacteristicsCountToBeDiscovered;
                        checkAllCharacteristicsDiscovered();
                        return S_OK;
                    }
                    ComPtr<IGattReadResult> readResult;
                    hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf());
                    if (FAILED(hr)) {
                        qCWarning(QT_BT_WINRT) << "Could not obtain characteristic read result";
                        --mCharacteristicsCountToBeDiscovered;
                        checkAllCharacteristicsDiscovered();
                        return S_OK;
                    }
                    if (!readResult)
                        qCWarning(QT_BT_WINRT) << "Characteristic read result is null";
                    else
                        charData.value = byteArrayFromGattResult(readResult);
                }
                mCharacteristicList.insert(handle, charData);

                ComPtr<IVectorView<GattDescriptor *>> descriptors;

                ComPtr<IGattDescriptorsResult> result;
                hr = op->GetResults(&result);
                if (FAILED(hr)) {
                    qCWarning(QT_BT_WINRT) << "Could not obtain descriptor read result";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }
                GattCommunicationStatus commStatus;
                hr = result->get_Status(&commStatus);
                if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
                    qCWarning(QT_BT_WINRT) << "Descriptor operation failed";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }

                hr = result->get_Descriptors(&descriptors);
                if (FAILED(hr)) {
                    qCWarning(QT_BT_WINRT) << "Could not obtain list of descriptors";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }

                uint descriptorCount;
                hr = descriptors->get_Size(&descriptorCount);
                if (FAILED(hr)) {
                    qCWarning(QT_BT_WINRT) << "Could not obtain list of descriptors' size";
                    --mCharacteristicsCountToBeDiscovered;
                    checkAllCharacteristicsDiscovered();
                    return S_OK;
                }
                for (uint j = 0; j < descriptorCount; ++j) {
                    QLowEnergyServicePrivate::DescData descData;
                    ComPtr<IGattDescriptor> descriptor;
                    hr = descriptors->GetAt(j, &descriptor);
                    WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor")
                    quint16 descHandle;
                    hr = descriptor->get_AttributeHandle(&descHandle);
                    WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor's attribute handle")
                    GUID descriptorUuid;
                    hr = descriptor->get_Uuid(&descriptorUuid);
                    WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain descriptor's Uuid")
                    descData.uuid = QBluetoothUuid(descriptorUuid);
                    charData.descriptorList.insert(descHandle, descData);
                    if (descData.uuid == QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration)) {
			mIndicateChars << charData.uuid;
                        ComPtr<IAsyncOperation<ClientCharConfigDescriptorResult *>> readOp;
                        hr = characteristic->ReadClientCharacteristicConfigurationDescriptorAsync(&readOp);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not read descriptor value")
                        ComPtr<IClientCharConfigDescriptorResult> readResult;
                        hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf());
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not await descriptor read result")
                        GattClientCharacteristicConfigurationDescriptorValue value;
                        hr = readResult->get_ClientCharacteristicConfigurationDescriptor(&value);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not get descriptor value from result")
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
                        
                    } else {
                        ComPtr<IAsyncOperation<GattReadResult *>> readOp;
                        hr = descriptor->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached,
                                                                     &readOp);
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not read descriptor value")
                        ComPtr<IGattReadResult> readResult;
                        hr = QWinRTFunctions::await(readOp, readResult.GetAddressOf());
                        WARN_AND_CONTINUE_IF_FAILED(hr, "Could await descriptor read result")
                        if (descData.uuid == QBluetoothUuid::CharacteristicUserDescription)
                            descData.value = byteArrayFromGattResult(readResult, true);
                        else
                            descData.value = byteArrayFromGattResult(readResult);
                    }
                    charData.descriptorList.insert(descHandle, descData);
                }

                mCharacteristicList.insert(handle, charData);
                --mCharacteristicsCountToBeDiscovered;
                checkAllCharacteristicsDiscovered();
                return S_OK;
            }).Get());
            if (FAILED(hr)) {
                qCWarning(QT_BT_WINRT) << "Could not register descriptor callback";
                --mCharacteristicsCountToBeDiscovered;
                continue;
            }
        }
        checkAllCharacteristicsDiscovered();
    }

private:
    bool checkAllCharacteristicsDiscovered();
    void emitErrorAndQuitThread(HRESULT hr);
    void emitErrorAndQuitThread(const QString &error);

public:
    QBluetoothUuid mService;
    ComPtr<IGattDeviceService3> mDeviceService;
    QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> mCharacteristicList;
    uint mCharacteristicsCountToBeDiscovered;
    quint16 mStartHandle = 0;
    quint16 mEndHandle = 0;
    QVector<QBluetoothUuid> mIndicateChars;

signals:
    void charListObtained(const QBluetoothUuid &service, QHash<QLowEnergyHandle,
                          QLowEnergyServicePrivate::CharData> charList,
                          QVector<QBluetoothUuid> indicateChars,
                          QLowEnergyHandle startHandle, QLowEnergyHandle endHandle);
    void errorOccured(const QString &error);
};

bool QWinRTLowEnergyServiceHandlerNew::checkAllCharacteristicsDiscovered()
{
    if (mCharacteristicsCountToBeDiscovered == 0) {
        emit charListObtained(mService, mCharacteristicList, mIndicateChars,
                              mStartHandle, mEndHandle);
        QThread::currentThread()->quit();
        return true;
    }

    return false;
}

void QWinRTLowEnergyServiceHandlerNew::emitErrorAndQuitThread(HRESULT hr)
{
    emitErrorAndQuitThread(qt_error_string(hr));
}

void QWinRTLowEnergyServiceHandlerNew::emitErrorAndQuitThread(const QString &error)
{
    emit errorOccured(error);
    QThread::currentThread()->quit();
}

QLowEnergyControllerPrivateWinRTNew::QLowEnergyControllerPrivateWinRTNew()
    : QLowEnergyControllerPrivate()
{
    registerQLowEnergyControllerMetaType();
    connect(this, &QLowEnergyControllerPrivateWinRTNew::characteristicChanged,
            this, &QLowEnergyControllerPrivateWinRTNew::handleCharacteristicChanged,
            Qt::QueuedConnection);
}

QLowEnergyControllerPrivateWinRTNew::~QLowEnergyControllerPrivateWinRTNew()
{
    unregisterFromStatusChanges();
    unregisterFromValueChanges();
    mAbortPending = true;
}

void QLowEnergyControllerPrivateWinRTNew::init()
{
}

void QLowEnergyControllerPrivateWinRTNew::connectToDevice()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    mAbortPending = false;
    Q_Q(QLowEnergyController);
    if (remoteDevice.isNull()) {
        qWarning() << "Invalid/null remote device address";
        setError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    setState(QLowEnergyController::ConnectingState);

    ComPtr<IBluetoothLEDeviceStatics> deviceStatics;
    HRESULT hr = GetActivationFactory(
                HString::MakeReference(RuntimeClass_Windows_Devices_Bluetooth_BluetoothLEDevice).Get(),
                &deviceStatics);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain device factory", return)
    ComPtr<IAsyncOperation<BluetoothLEDevice *>> deviceFromIdOperation;
    hr = deviceStatics->FromBluetoothAddressAsync(remoteDevice.toUInt64(), &deviceFromIdOperation);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not find LE device from address", return)
    hr = QWinRTFunctions::await(deviceFromIdOperation, mDevice.GetAddressOf(),
                                QWinRTFunctions::ProcessMainThreadEvents, 5000);
    if (FAILED(hr) || !mDevice) {
        qCWarning(QT_BT_WINRT) << "Could not find LE device";
        setError(QLowEnergyController::InvalidBluetoothAdapterError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }
    BluetoothConnectionStatus status;
    hr = mDevice->get_ConnectionStatus(&status);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain device's connection status", return)
    if (status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
        return;
    }

    QBluetoothLocalDevice localDevice;
    QBluetoothLocalDevice::Pairing pairing = localDevice.pairingStatus(remoteDevice);
    if (pairing == QBluetoothLocalDevice::Unpaired)
        connectToUnpairedDevice();
    else
        connectToPairedDevice();
}

void QLowEnergyControllerPrivateWinRTNew::disconnectFromDevice()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    Q_Q(QLowEnergyController);
    setState(QLowEnergyController::ClosingState);
    unregisterFromValueChanges();
    unregisterFromStatusChanges();
    mAbortPending = true;
    mDevice = nullptr;
    setState(QLowEnergyController::UnconnectedState);
    emit q->disconnected();
}

ComPtr<IGattDeviceService> QLowEnergyControllerPrivateWinRTNew::getNativeService(
        const QBluetoothUuid &serviceUuid)
{
    ComPtr<IGattDeviceService> deviceService;
    HRESULT hr;
    hr = mDevice->GetGattService(serviceUuid, &deviceService);
    if (FAILED(hr))
        qCDebug(QT_BT_WINRT) << "Could not obtain native service for Uuid" << serviceUuid;
    return deviceService;
}

ComPtr<IGattCharacteristic> QLowEnergyControllerPrivateWinRTNew::getNativeCharacteristic(
        const QBluetoothUuid &serviceUuid, const QBluetoothUuid &charUuid)
{
    ComPtr<IGattDeviceService> service = getNativeService(serviceUuid);
    if (!service)
        return nullptr;

    ComPtr<IGattDeviceService3> service3;
    HRESULT hr = service.As(&service3);
    RETURN_IF_FAILED("Could not cast service", return nullptr);

    ComPtr<IAsyncOperation<GattCharacteristicsResult *>> op;
    ComPtr<IGattCharacteristicsResult> result;
    hr = service3->GetCharacteristicsForUuidAsync(charUuid, &op);
    RETURN_IF_FAILED("Could not obtain native characteristics for service", return nullptr);
    hr = QWinRTFunctions::await(op, result.GetAddressOf(), QWinRTFunctions::ProcessMainThreadEvents, 5000);
    RETURN_IF_FAILED("Could not await completion of characteristic operation", return nullptr);
    GattCommunicationStatus status;
    hr = result->get_Status(&status);
    if (FAILED(hr) || status != GattCommunicationStatus_Success) {
        qErrnoWarning(hr, "Native characteristic operation failed.");
        return nullptr;
    }
    ComPtr<IVectorView<GattCharacteristic *>> characteristics;
    hr = result->get_Characteristics(&characteristics);
    RETURN_IF_FAILED("Could not obtain characteristic list.", return nullptr);
    uint size;
    hr = characteristics->get_Size(&size);
    RETURN_IF_FAILED("Could not obtain characteristic list's size.", return nullptr);
    if (size != 1)
        qErrnoWarning("More than 1 characteristic found.");
    ComPtr<IGattCharacteristic> characteristic;
    hr = characteristics->GetAt(0, &characteristic);
    RETURN_IF_FAILED("Could not obtain first characteristic for service", return nullptr);
    return characteristic;
}

void QLowEnergyControllerPrivateWinRTNew::registerForValueChanges(const QBluetoothUuid &serviceUuid,
                                                                  const QBluetoothUuid &charUuid)
{
    qCDebug(QT_BT_WINRT) << "Registering characteristic" << charUuid << "in service"
                         << serviceUuid << "for value changes";
    for (const ValueChangedEntry &entry : qAsConst(mValueChangedTokens)) {
        GUID guuid;
        HRESULT hr;
        hr = entry.characteristic->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain characteristic's Uuid")
        if (QBluetoothUuid(guuid) == charUuid)
            return;
    }
    ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(serviceUuid, charUuid);
    if (!characteristic) {
        qCDebug(QT_BT_WINRT).nospace() << "Could not obtain native characteristic " << charUuid
                             << " from service " << serviceUuid << ". Qt will not be able to signal"
                             << " changes for this characteristic.";
        return;
    }

    EventRegistrationToken token;
    HRESULT hr;
    hr = characteristic->add_ValueChanged(
                Callback<ValueChangedHandler>(this, &QLowEnergyControllerPrivateWinRTNew::onValueChange).Get(),
                &token);
    RETURN_IF_FAILED("Could not register characteristic for value changes", return)
    mValueChangedTokens.append(ValueChangedEntry(characteristic, token));
    qCDebug(QT_BT_WINRT) << "Characteristic" << charUuid << "in service"
        << serviceUuid << "registered for value changes";
}

void QLowEnergyControllerPrivateWinRTNew::unregisterFromValueChanges()
{
    qCDebug(QT_BT_WINRT) << "Unregistering " << mValueChangedTokens.count() << " value change tokens";
    HRESULT hr;
    for (const ValueChangedEntry &entry : qAsConst(mValueChangedTokens)) {
        if (!entry.characteristic) {
            qCWarning(QT_BT_WINRT) << "Unregistering from value changes for characteristic failed."
                                   << "Characteristic has been deleted";
            continue;
        }
        hr = entry.characteristic->remove_ValueChanged(entry.token);
        if (FAILED(hr))
            qCWarning(QT_BT_WINRT) << "Unregistering from value changes for characteristic failed.";
    }
    mValueChangedTokens.clear();
}

HRESULT QLowEnergyControllerPrivateWinRTNew::onValueChange(IGattCharacteristic *characteristic, IGattValueChangedEventArgs *args)
{
    HRESULT hr;
    quint16 handle;
    qCDebug(QT_BT_WINRT) << __FUNCTION__ << characteristic;
    hr = characteristic->get_AttributeHandle(&handle);
    RETURN_IF_FAILED("Could not obtain characteristic's handle", return S_OK)
    ComPtr<IBuffer> buffer;
    hr = args->get_CharacteristicValue(&buffer);
    RETURN_IF_FAILED("Could not obtain characteristic's value", return S_OK)
    emit characteristicChanged(handle, byteArrayFromBuffer(buffer));
    return S_OK;
}

bool QLowEnergyControllerPrivateWinRTNew::registerForStatusChanges()
{
    if (!mDevice)
        return false;

    qCDebug(QT_BT_WINRT) << __FUNCTION__;

    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([this]() {
        HRESULT hr;
        hr = mDevice->add_ConnectionStatusChanged(
            Callback<StatusHandler>(this, &QLowEnergyControllerPrivateWinRTNew::onStatusChange).Get(),
                                    &mStatusChangedToken);
        RETURN_IF_FAILED("Could not register connection status callback", return hr)
        return S_OK;
    });
    RETURN_FALSE_IF_FAILED("Could not add status callback on Xaml thread")
    return true;
}

void QLowEnergyControllerPrivateWinRTNew::unregisterFromStatusChanges()
{
    qCDebug(QT_BT_WINRT) << __FUNCTION__;
    if (mDevice && mStatusChangedToken.value) {
        mDevice->remove_ConnectionStatusChanged(mStatusChangedToken);
        mStatusChangedToken.value = 0;
    }
}

HRESULT QLowEnergyControllerPrivateWinRTNew::onStatusChange(IBluetoothLEDevice *dev, IInspectable *)
{
    Q_Q(QLowEnergyController);
    BluetoothConnectionStatus status;
    HRESULT hr;
    hr = dev->get_ConnectionStatus(&status);
    RETURN_IF_FAILED("Could not obtain connection status", return S_OK)
    if (state == QLowEnergyController::ConnectingState
        && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Connected) {
        setState(QLowEnergyController::ConnectedState);
        emit q->connected();
    } else if (state != QLowEnergyController::UnconnectedState
        && status == BluetoothConnectionStatus::BluetoothConnectionStatus_Disconnected) {
        invalidateServices();
        unregisterFromValueChanges();
        unregisterFromStatusChanges();
        mDevice = nullptr;
        setError(QLowEnergyController::RemoteHostClosedError);
        setState(QLowEnergyController::UnconnectedState);
        emit q->disconnected();
    }
    return S_OK;
}

void QLowEnergyControllerPrivateWinRTNew::obtainIncludedServices(
        QSharedPointer<QLowEnergyServicePrivate> servicePointer,
        ComPtr<IGattDeviceService> service)
{
    Q_Q(QLowEnergyController);
    ComPtr<IGattDeviceService3> service3;
    HRESULT hr = service.As(&service3);
    RETURN_IF_FAILED("Could not cast service", return);
    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> op;
    hr = service3->GetIncludedServicesAsync(&op);
    // Some devices return ERROR_ACCESS_DISABLED_BY_POLICY
    RETURN_IF_FAILED("Could not obtain included services", return);
    ComPtr<IGattDeviceServicesResult> result;
    hr = QWinRTFunctions::await(op, result.GetAddressOf(), QWinRTFunctions::ProcessMainThreadEvents, 5000);
    RETURN_IF_FAILED("Could not await service operation", return);
    GattCommunicationStatus status;
    hr = result->get_Status(&status);
    if (FAILED(hr) || status != GattCommunicationStatus_Success) {
        qErrnoWarning("Could not obtain list of included services");
        return;
    }
    ComPtr<IVectorView<GattDeviceService *>> includedServices;
    hr = result->get_Services(&includedServices);
    RETURN_IF_FAILED("Could not obtain service list", return);

    uint count;
    hr = includedServices->get_Size(&count);
    RETURN_IF_FAILED("Could not obtain service list's size", return);
    for (uint i = 0; i < count; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = includedServices->GetAt(i, &includedService);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service from list");
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain included service's Uuid");
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
        includedPointer->type |= QLowEnergyService::IncludedService;
        servicePointer->includedServices.append(includedUuid);

        obtainIncludedServices(includedPointer, includedService);

        emit q->serviceDiscovered(includedUuid);
    }
}

HRESULT QLowEnergyControllerPrivateWinRTNew::onServiceDiscoveryFinished(ABI::Windows::Foundation::IAsyncOperation<GattDeviceServicesResult *> *op, AsyncStatus status)
{
    Q_Q(QLowEnergyController);
    if (status != AsyncStatus::Completed) {
        qCDebug(QT_BT_WINRT) << "Could not obtain services";
        return S_OK;
    }
    ComPtr<IGattDeviceServicesResult> result;
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    HRESULT hr = op->GetResults(&result);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service discovery result",
                                      return S_OK);
    GattCommunicationStatus commStatus;
    hr = result->get_Status(&commStatus);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service discovery status",
                                      return S_OK);
    if (commStatus != GattCommunicationStatus_Success)
        return S_OK;

    hr = result->get_Services(&deviceServices);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service list",
                                      return S_OK);

    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service list size",
                                      return S_OK);
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> deviceService;
        hr = deviceServices->GetAt(i, &deviceService);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service");
        GUID guuid;
        hr = deviceService->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service's Uuid");
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

        // If QLowEnergyService::discoverDetails is called from this callback,
        // deviceService3->GetIncludedServicesAsync can fail with AccessDenied
        // error. To work around this, we defer emitting the signal.
        QTimer::singleShot(0, [q, service] () {
            emit q->serviceDiscovered(service);
        });
    }

    setState(QLowEnergyController::DiscoveredState);
     // If QLowEnergyService::discoverDetails is called from this callback,
    // deviceService3->GetIncludedServicesAsync can fail with AccessDenied
    // error. To work around this, we defer emitting the signal.
    QTimer::singleShot(0, [q] () {
        emit q->discoveryFinished();
    });

    return S_OK;
}

void QLowEnergyControllerPrivateWinRTNew::discoverServices()
{
    qCDebug(QT_BT_WINRT) << "Service discovery initiated";

    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not cast device", return);
    ComPtr<IAsyncOperation<GenericAttributeProfile::GattDeviceServicesResult *>> asyncResult;
    hr = device3->GetGattServicesAsync(&asyncResult);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain services", return);
    hr = QEventDispatcherWinRT::runOnXamlThread( [asyncResult, this] () {
        HRESULT hr = asyncResult->put_Completed(
            Callback<IAsyncOperationCompletedHandler<GenericAttributeProfile::GattDeviceServicesResult *>>(
                    this, &QLowEnergyControllerPrivateWinRTNew::onServiceDiscoveryFinished).Get());
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not register service discovery callback",
                                          return S_OK)
        return hr;
    });
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not run registration in Xaml thread",
                                      return)
}

void QLowEnergyControllerPrivateWinRTNew::discoverServiceDetails(const QBluetoothUuid &service)
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

    auto reactOnDiscoveryError = [](QSharedPointer<QLowEnergyServicePrivate> service,
            const QString &msg)
    {
        qCDebug(QT_BT_WINRT) << msg;
        service->setError(QLowEnergyService::UnknownError);
        service->setState(QLowEnergyService::DiscoveryRequired);
    };
    //update service data
    QSharedPointer<QLowEnergyServicePrivate> pointer = serviceList.value(service);
    qCDebug(QT_BT_WINRT_SERVICE_THREAD) << __FUNCTION__ << "Changing service pointer from thread"
                                        << QThread::currentThread();
    pointer->setState(QLowEnergyService::DiscoveringServices);
    ComPtr<IGattDeviceService3> deviceService3;
    HRESULT hr = deviceService.As(&deviceService3);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer, QStringLiteral("Could not cast service: %1").arg(hr));
        return;
    }
    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> op;
    hr = deviceService3->GetIncludedServicesAsync(&op);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer, QStringLiteral("Could not obtain included service list: %1").arg(hr));
        return;
    }
    ComPtr<IGattDeviceServicesResult> result;
    hr = QWinRTFunctions::await(op, result.GetAddressOf());
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer, QStringLiteral("Could not await service operation: %1").arg(hr));
        return;
    }
    GattCommunicationStatus status;
    hr = result->get_Status(&status);
    if (FAILED(hr) || status != GattCommunicationStatus_Success) {
        reactOnDiscoveryError(pointer,
                         QStringLiteral("Obtaining list of included services failed: %1").arg(hr));
        return;
    }
    ComPtr<IVectorView<GattDeviceService *>> deviceServices;
    hr = result->get_Services(&deviceServices);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer,
                         QStringLiteral("Could not obtain service list from result: %1").arg(hr));
        return;
    }
    uint serviceCount;
    hr = deviceServices->get_Size(&serviceCount);
    if (FAILED(hr)) {
        reactOnDiscoveryError(pointer,
                         QStringLiteral("Could not obtain included service list's size: %1").arg(hr));
        return;
    }
    for (uint i = 0; i < serviceCount; ++i) {
        ComPtr<IGattDeviceService> includedService;
        hr = deviceServices->GetAt(i, &includedService);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service from list")
        GUID guuid;
        hr = includedService->get_Uuid(&guuid);
        WARN_AND_CONTINUE_IF_FAILED(hr, "Could not obtain service Uuid")

        const QBluetoothUuid service(guuid);
        if (service.isNull()) {
            qCDebug(QT_BT_WINRT) << "Could not find service";
            continue;
        }

        pointer->includedServices.append(service);

        // update the type of the included service
        QSharedPointer<QLowEnergyServicePrivate> otherService = serviceList.value(service);
        if (!otherService.isNull())
            otherService->type |= QLowEnergyService::IncludedService;
    }

    QWinRTLowEnergyServiceHandlerNew *worker
            = new QWinRTLowEnergyServiceHandlerNew(service, deviceService3);
    QThread *thread = new QThread;
    worker->moveToThread(thread);
    connect(thread, &QThread::started, worker, &QWinRTLowEnergyServiceHandlerNew::obtainCharList);
    connect(thread, &QThread::finished, thread, &QObject::deleteLater);
    connect(thread, &QThread::finished, worker, &QObject::deleteLater);
    connect(worker, &QWinRTLowEnergyServiceHandlerNew::errorOccured,
            this, &QLowEnergyControllerPrivateWinRTNew::handleServiceHandlerError);
    connect(worker, &QWinRTLowEnergyServiceHandlerNew::charListObtained,
            [this, reactOnDiscoveryError, thread](const QBluetoothUuid &service, QHash<QLowEnergyHandle,
            QLowEnergyServicePrivate::CharData> charList, QVector<QBluetoothUuid> indicateChars,
            QLowEnergyHandle startHandle, QLowEnergyHandle endHandle) {
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
        if (FAILED(hr)) {
            reactOnDiscoveryError(pointer,
                             QStringLiteral("Could not register for value changes in Xaml thread: %1").arg(hr));
            thread->exit(0);
            return;
        }

        pointer->setState(QLowEnergyService::ServiceDiscovered);
        thread->exit(0);
    });
    thread->start();
}

void QLowEnergyControllerPrivateWinRTNew::startAdvertising(
        const QLowEnergyAdvertisingParameters &,
        const QLowEnergyAdvertisingData &,
        const QLowEnergyAdvertisingData &)
{
    setError(QLowEnergyController::AdvertisingError);
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRTNew::stopAdvertising()
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRTNew::requestConnectionUpdate(const QLowEnergyConnectionParameters &)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRTNew::readCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
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
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not read characteristic",
                                       service, QLowEnergyService::CharacteristicReadError, return S_OK)
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
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for characteristic",
                                           service, QLowEnergyService::CharacteristicReadError, return S_OK)

            const QByteArray value = byteArrayFromGattResult(characteristicValue);
            QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
            charData.value = value;
            service->characteristicList.insert(charHandle, charData);
            emit service->characteristicRead(QLowEnergyCharacteristic(service, charHandle), value);
            return S_OK;
        };
        hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattReadResult *>>(
                                       readCompletedLambda).Get());
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register characteristic read callback",
                                       service, QLowEnergyService::CharacteristicReadError, return S_OK)
        return S_OK;
    });
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not run registration on Xaml thread",
                                   service, QLowEnergyService::CharacteristicReadError, return)
}

void QLowEnergyControllerPrivateWinRTNew::readDescriptor(
        const QSharedPointer<QLowEnergyServicePrivate> service,
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
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not read client characteristic configuration",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
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
                CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for descriptor",
                                               service, QLowEnergyService::DescriptorReadError, return S_OK)
                GattClientCharacteristicConfigurationDescriptorValue value;
                hr = iValue->get_ClientCharacteristicConfigurationDescriptor(&value);
                CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain value for descriptor",
                                               service, QLowEnergyService::DescriptorReadError, return S_OK)
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
            hr = readOp->put_Completed(
                        Callback<IAsyncOperationCompletedHandler<ClientCharConfigDescriptorResult *>>(
                            readCompletedLambda).Get());
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor read callback",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            return S_OK;
        } else {
            ComPtr<IGattCharacteristic3> characteristic3;
            HRESULT hr = characteristic.As(&characteristic3);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast characteristic",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            ComPtr<IAsyncOperation<GattDescriptorsResult *>> op;
            hr = characteristic3->GetDescriptorsForUuidAsync(descData.uuid, &op);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor for uuid",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            ComPtr<IGattDescriptorsResult> result;
            hr = QWinRTFunctions::await(op, result.GetAddressOf(), QWinRTFunctions::ProcessMainThreadEvents, 5000);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not await descritpor read result",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)

            GattCommunicationStatus commStatus;
            hr = result->get_Status(&commStatus);
            if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
                qErrnoWarning("Could not obtain list of descriptors");
                service->setError(QLowEnergyService::DescriptorReadError);
                return S_OK;
            }

            ComPtr<IVectorView<GattDescriptor *>> descriptors;
            hr = result->get_Descriptors(&descriptors);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor list",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            uint size;
            hr = descriptors->get_Size(&size);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not await descritpor list's size",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            if (size == 0) {
                qCWarning(QT_BT_WINRT) << "No descriptor with uuid" << descData.uuid << "was found.";
                service->setError(QLowEnergyService::DescriptorReadError);
                return S_OK;
            } else if (size > 1) {
                qCWarning(QT_BT_WINRT) << "There is more than 1 descriptor with uuid" << descData.uuid;
            }

            ComPtr<IGattDescriptor> descriptor;
            hr = descriptors->GetAt(0, &descriptor);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descritpor from list",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            ComPtr<IAsyncOperation<GattReadResult*>> readOp;
            hr = descriptor->ReadValueWithCacheModeAsync(BluetoothCacheMode_Uncached, &readOp);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not read descriptor value",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
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
                descData.uuid = descUuid;
                if (descData.uuid == QBluetoothUuid::CharacteristicUserDescription)
                    descData.value = byteArrayFromGattResult(descriptorValue, true);
                else
                    descData.value = byteArrayFromGattResult(descriptorValue);
                service->characteristicList[charHandle].descriptorList[descHandle] = descData;
                emit service->descriptorRead(QLowEnergyDescriptor(service, charHandle, descHandle),
                    descData.value);
                return S_OK;
            };
            hr = readOp->put_Completed(Callback<IAsyncOperationCompletedHandler<GattReadResult *>>(
                                           readCompletedLambda).Get());
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor read callback",
                                           service, QLowEnergyService::DescriptorReadError, return S_OK)
            return S_OK;
        }
    });
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not run registration on Xaml thread",
                                   service, QLowEnergyService::DescriptorReadError, return)
}

void QLowEnergyControllerPrivateWinRTNew::writeCharacteristic(
        const QSharedPointer<QLowEnergyServicePrivate> service,
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
        qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "cannot be found in service"
                             << service->uuid;
        service->setError(QLowEnergyService::CharacteristicWriteError);
        return;
    }

    QLowEnergyServicePrivate::CharData charData = service->characteristicList.value(charHandle);
    const bool writeWithResponse = mode == QLowEnergyService::WriteWithResponse;
    if (!(charData.properties & (writeWithResponse ? QLowEnergyCharacteristic::Write
             : QLowEnergyCharacteristic::WriteNoResponse)))
        qCDebug(QT_BT_WINRT) << "Write flag is not set for characteristic" << charHandle;

    HRESULT hr;
    hr = QEventDispatcherWinRT::runOnXamlThread([charData, charHandle, this, service, newValue,
                                                writeWithResponse]() {
        ComPtr<IGattCharacteristic> characteristic = getNativeCharacteristic(service->uuid,
                                                                             charData.uuid);
        if (!characteristic) {
            qCDebug(QT_BT_WINRT) << "Could not obtain native characteristic" << charData.uuid
                                 << "from service" << service->uuid;
            service->setError(QLowEnergyService::CharacteristicWriteError);
            return S_OK;
        }
        ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
        HRESULT hr = GetActivationFactory(
                    HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
                    &bufferFactory);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain buffer factory",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
        const quint32 length = quint32(newValue.length());
        hr = bufferFactory->Create(length, &buffer);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not create buffer",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        hr = buffer->put_Length(length);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer length",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
        hr = buffer.As(&byteAccess);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast buffer",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        byte *bytes;
        hr = byteAccess->Buffer(&bytes);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        memcpy(bytes, newValue, length);
        ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
        GattWriteOption option = writeWithResponse ? GattWriteOption_WriteWithResponse
                                                   : GattWriteOption_WriteWithoutResponse;
        hr = characteristic->WriteValueWithOptionAsync(buffer.Get(), option, &writeOp);
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could write characteristic",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        QPointer<QLowEnergyControllerPrivateWinRTNew> thisPtr(this);
        auto writeCompletedLambda = [charData, charHandle, newValue, service, writeWithResponse, thisPtr]
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
                qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle
                                     << "write operation was tried with invalid value length";
                service->setError(QLowEnergyService::CharacteristicWriteError);
                return S_OK;
            }
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain characteristic write result",
                                           service, QLowEnergyService::CharacteristicWriteError, return S_OK)
            if (result != GattCommunicationStatus_Success) {
                qCDebug(QT_BT_WINRT) << "Characteristic" << charHandle << "write operation failed";
                service->setError(QLowEnergyService::CharacteristicWriteError);
                return S_OK;
            }
            // only update cache when property is readable. Otherwise it remains
            // empty.
            if (charData.properties & QLowEnergyCharacteristic::Read)
                thisPtr->updateValueOfCharacteristic(charHandle, newValue, false);
            if (writeWithResponse)
                emit service->characteristicWritten(QLowEnergyCharacteristic(service, charHandle),
                                                    newValue);
            return S_OK;
        };
        hr = writeOp->put_Completed(
                    Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>(
                        writeCompletedLambda).Get());
        CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register characteristic write callback",
                                       service, QLowEnergyService::CharacteristicWriteError, return S_OK)
        return S_OK;
    });
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not run registration on Xaml thread",
                                   service, QLowEnergyService::CharacteristicWriteError, return)
}

void QLowEnergyControllerPrivateWinRTNew::writeDescriptor(
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
            qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "could not be found in Characteristic"
                                 << charHandle;

        QLowEnergyServicePrivate::DescData descData = charData.descriptorList.value(descHandle);
        if (descData.uuid == QBluetoothUuid(QBluetoothUuid::ClientCharacteristicConfiguration)) {
            GattClientCharacteristicConfigurationDescriptorValue value;
            quint16 intValue = qFromLittleEndian<quint16>(newValue);
            if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Indicate
                    && intValue & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                qCWarning(QT_BT_WINRT) << "Setting both Indicate and Notify is not supported on WinRT";
                value = GattClientCharacteristicConfigurationDescriptorValue(
                        (GattClientCharacteristicConfigurationDescriptorValue_Indicate
                         | GattClientCharacteristicConfigurationDescriptorValue_Notify));
            } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Indicate) {
                value = GattClientCharacteristicConfigurationDescriptorValue_Indicate;
            } else if (intValue & GattClientCharacteristicConfigurationDescriptorValue_Notify) {
                value = GattClientCharacteristicConfigurationDescriptorValue_Notify;
            } else if (intValue == 0) {
                value = GattClientCharacteristicConfigurationDescriptorValue_None;
            } else {
                qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle
                                     << "write operation failed: Invalid value";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return S_OK;
            }
            ComPtr<IAsyncOperation<enum GattCommunicationStatus>> writeOp;
            HRESULT hr = characteristic->WriteClientCharacteristicConfigurationDescriptorAsync(value, &writeOp);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not write client characteristic configuration",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            QPointer<QLowEnergyControllerPrivateWinRTNew> thisPtr(this);
            auto writeCompletedLambda = [charHandle, descHandle, newValue, service, thisPtr]
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
                CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for descriptor",
                                               service, QLowEnergyService::DescriptorWriteError, return S_OK)
                if (result != GattCommunicationStatus_Success) {
                    qCWarning(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed";
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                thisPtr->updateValueOfDescriptor(charHandle, descHandle, newValue, false);
                emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle),
                                                newValue);
                return S_OK;
            };
            hr = writeOp->put_Completed(
                        Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus >>(
                            writeCompletedLambda).Get());
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor write callback",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
        } else {
            ComPtr<IGattCharacteristic3> characteristic3;
            HRESULT hr = characteristic.As(&characteristic3);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast characteristic",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            ComPtr<IAsyncOperation<GattDescriptorsResult *>> op;
            hr = characteristic3->GetDescriptorsForUuidAsync(descData.uuid, &op);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor from Uuid",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            ComPtr<IGattDescriptorsResult> result;
            hr = QWinRTFunctions::await(op, result.GetAddressOf(), QWinRTFunctions::ProcessMainThreadEvents, 5000);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not await descriptor operation",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            GattCommunicationStatus commStatus;
            hr = result->get_Status(&commStatus);
            if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
                qCWarning(QT_BT_WINRT) << "Descriptor operation failed";
                service->setError(QLowEnergyService::DescriptorWriteError);
                return S_OK;
            }
            ComPtr<IVectorView<GattDescriptor *>> descriptors;
            hr = result->get_Descriptors(&descriptors);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain list of descriptors",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            uint size;
            hr = descriptors->get_Size(&size);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain list of descriptors' size",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            if (size == 0) {
                qCWarning(QT_BT_WINRT) << "No descriptor with uuid" << descData.uuid << "was found.";
                return S_OK;
            } else if (size > 1) {
                qCWarning(QT_BT_WINRT) << "There is more than 1 descriptor with uuid" << descData.uuid;
            }
            ComPtr<IGattDescriptor> descriptor;
            hr = descriptors->GetAt(0, &descriptor);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain descriptor",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            ComPtr<ABI::Windows::Storage::Streams::IBufferFactory> bufferFactory;
            hr = GetActivationFactory(
                        HStringReference(RuntimeClass_Windows_Storage_Streams_Buffer).Get(),
                        &bufferFactory);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain buffer factory",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
            const quint32 length = quint32(newValue.length());
            hr = bufferFactory->Create(length, &buffer);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not create buffer",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            hr = buffer->put_Length(length);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer length",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            ComPtr<Windows::Storage::Streams::IBufferByteAccess> byteAccess;
            hr = buffer.As(&byteAccess);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not cast buffer",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            byte *bytes;
            hr = byteAccess->Buffer(&bytes);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not set buffer",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            memcpy(bytes, newValue, length);
            ComPtr<IAsyncOperation<GattCommunicationStatus>> writeOp;
            hr = descriptor->WriteValueAsync(buffer.Get(), &writeOp);
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not write descriptor value",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            QPointer<QLowEnergyControllerPrivateWinRTNew> thisPtr(this);
            auto writeCompletedLambda = [charHandle, descHandle, newValue, service, thisPtr]
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
                CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not obtain result for descriptor",
                                               service, QLowEnergyService::DescriptorWriteError, return S_OK)
                if (result != GattCommunicationStatus_Success) {
                    qCDebug(QT_BT_WINRT) << "Descriptor" << descHandle << "write operation failed";
                    service->setError(QLowEnergyService::DescriptorWriteError);
                    return S_OK;
                }
                thisPtr->updateValueOfDescriptor(charHandle, descHandle, newValue, false);
                emit service->descriptorWritten(QLowEnergyDescriptor(service, charHandle, descHandle),
                                                newValue);
                return S_OK;
            };
            hr = writeOp->put_Completed(
                        Callback<IAsyncOperationCompletedHandler<GattCommunicationStatus>>(
                            writeCompletedLambda).Get());
            CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not register descriptor write callback",
                                           service, QLowEnergyService::DescriptorWriteError, return S_OK)
            return S_OK;
        }
        return S_OK;
    });
    CHECK_HR_AND_SET_SERVICE_ERROR(hr, "Could not run registration on Xaml thread",
                                   service, QLowEnergyService::DescriptorWriteError, return)
}


void QLowEnergyControllerPrivateWinRTNew::addToGenericAttributeList(const QLowEnergyServiceData &,
                                                                    QLowEnergyHandle)
{
    Q_UNIMPLEMENTED();
}

void QLowEnergyControllerPrivateWinRTNew::handleCharacteristicChanged(
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

void QLowEnergyControllerPrivateWinRTNew::handleServiceHandlerError(const QString &error)
{
    if (state != QLowEnergyController::DiscoveringState)
        return;

    qCWarning(QT_BT_WINRT) << "Error while discovering services:" << error;
    setState(QLowEnergyController::UnconnectedState);
    setError(QLowEnergyController::ConnectionError);
}

void QLowEnergyControllerPrivateWinRTNew::connectToPairedDevice()
{
    Q_Q(QLowEnergyController);
    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not cast device", return)
    ComPtr<IAsyncOperation<GattDeviceServicesResult *>> deviceServicesOp;
    while (!mAbortPending) {
        hr = device3->GetGattServicesAsync(&deviceServicesOp);
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain services", return)
        ComPtr<IGattDeviceServicesResult> deviceServicesResult;
        hr = QWinRTFunctions::await(deviceServicesOp, deviceServicesResult.GetAddressOf(),
                                    QWinRTFunctions::ProcessThreadEvents, 5000);
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not await services operation", return)

        GattCommunicationStatus commStatus;
        hr = deviceServicesResult->get_Status(&commStatus);
        if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
            qCWarning(QT_BT_WINRT()) << "Service operation failed";
            setError(QLowEnergyController::ConnectionError);
            setState(QLowEnergyController::UnconnectedState);
            unregisterFromStatusChanges();
            return;
        }

        ComPtr<IVectorView <GattDeviceService *>> deviceServices;
        hr = deviceServicesResult->get_Services(&deviceServices);
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain list of services", return)
            uint serviceCount;
        hr = deviceServices->get_Size(&serviceCount);
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service count", return)

        if (serviceCount == 0) {
            qCWarning(QT_BT_WINRT()) << "Found devices without services";
            setError(QLowEnergyController::ConnectionError);
            setState(QLowEnergyController::UnconnectedState);
            unregisterFromStatusChanges();
            return;
        }

        // Windows automatically connects to the device as soon as a service value is read/written.
        // Thus we read one value in order to establish the connection.
        for (uint i = 0; i < serviceCount; ++i) {
            ComPtr<IGattDeviceService> service;
            hr = deviceServices->GetAt(i, &service);
            CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain service", return);
            ComPtr<IGattDeviceService3> service3;
            hr = service.As(&service3);
            CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not cast service", return);
            ComPtr<IAsyncOperation<GattCharacteristicsResult *>> characteristicsOp;
            hr = service3->GetCharacteristicsAsync(&characteristicsOp);
            CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain characteristic", return);
            ComPtr<IGattCharacteristicsResult> characteristicsResult;
            hr = QWinRTFunctions::await(characteristicsOp, characteristicsResult.GetAddressOf(),
                QWinRTFunctions::ProcessThreadEvents, 5000);
            CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not await characteristic operation", return);
            GattCommunicationStatus commStatus;
            hr = characteristicsResult->get_Status(&commStatus);
            if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
                qCWarning(QT_BT_WINRT) << "Characteristic operation failed";
                break;
            }
            ComPtr<IVectorView<GattCharacteristic *>> characteristics;
            hr = characteristicsResult->get_Characteristics(&characteristics);
            if (hr == E_ACCESSDENIED) {
                // Everything will work as expected up until this point if the manifest capabilties
                // for bluetooth LE are not set.
                qCWarning(QT_BT_WINRT) << "Could not obtain characteristic list. Please check your "
                    "manifest capabilities";
                setState(QLowEnergyController::UnconnectedState);
                setError(QLowEnergyController::ConnectionError);
                unregisterFromStatusChanges();
                return;
            }
            CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain characteristic list", return);
            uint characteristicsCount;
            hr = characteristics->get_Size(&characteristicsCount);
            CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain characteristic list's size", return);
            for (uint j = 0; j < characteristicsCount; ++j) {
                ComPtr<IGattCharacteristic> characteristic;
                hr = characteristics->GetAt(j, &characteristic);
                CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain characteristic", return);
                ComPtr<IAsyncOperation<GattReadResult *>> op;
                GattCharacteristicProperties props;
                hr = characteristic->get_CharacteristicProperties(&props);
                CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain characteristic's properties", return);
                if (!(props & GattCharacteristicProperties_Read))
                    continue;
		/* QZ rviola
                hr = characteristic->ReadValueWithCacheModeAsync(BluetoothCacheMode::BluetoothCacheMode_Uncached, &op);
                CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not read characteristic value", return);
                ComPtr<IGattReadResult> result;
                hr = QWinRTFunctions::await(op, result.GetAddressOf(), QWinRTFunctions::ProcessThreadEvents, 500);
                // E_ILLEGAL_METHOD_CALL will be the result for a device, that is not reachable at
                // the moment. In this case we should jump back into the outer loop and keep trying.
                if (hr == E_ILLEGAL_METHOD_CALL)
                    break;
                CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not await characteristic read", return);
                ComPtr<ABI::Windows::Storage::Streams::IBuffer> buffer;
                hr = result->get_Value(&buffer);
                CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain characteristic value", return);
                if (!buffer) {
                    qCDebug(QT_BT_WINRT) << "Problem reading value";
                    break;
                }
		*/

                setState(QLowEnergyController::ConnectedState);
                emit q->connected();
                if (!registerForStatusChanges()) {
                    setError(QLowEnergyController::ConnectionError);
                    setState(QLowEnergyController::UnconnectedState);
                    return;
                }
                return;
            }
        }
    }
}

void QLowEnergyControllerPrivateWinRTNew::connectToUnpairedDevice()
{
    if (!registerForStatusChanges()) {
        setError(QLowEnergyController::ConnectionError);
        setState(QLowEnergyController::UnconnectedState);
        return;
    }
    ComPtr<IBluetoothLEDevice3> device3;
    HRESULT hr = mDevice.As(&device3);
    CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not cast device", return)
    ComPtr<IGattDeviceServicesResult> deviceServicesResult;
    while (!mAbortPending) {
        ComPtr<IAsyncOperation<GattDeviceServicesResult *>> deviceServicesOp;
        hr = device3->GetGattServicesAsync(&deviceServicesOp);
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not obtain services", return)
        hr = QWinRTFunctions::await(deviceServicesOp, deviceServicesResult.GetAddressOf(),
                                    QWinRTFunctions::ProcessMainThreadEvents);
        CHECK_FOR_DEVICE_CONNECTION_ERROR(hr, "Could not await services operation", return)

        GattCommunicationStatus commStatus;
        hr = deviceServicesResult->get_Status(&commStatus);
        if (commStatus == GattCommunicationStatus_Unreachable)
            continue;

        if (FAILED(hr) || commStatus != GattCommunicationStatus_Success) {
            qCWarning(QT_BT_WINRT()) << "Service operation failed";
            setError(QLowEnergyController::ConnectionError);
            setState(QLowEnergyController::UnconnectedState);
            unregisterFromStatusChanges();
            return;
        }

        break;
    }
}

QT_END_NAMESPACE

#include "qlowenergycontroller_winrt_new.moc"
