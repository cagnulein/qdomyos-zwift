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

#include "qlowenergyserviceprivate_p.h"
#include "qlowenergycharacteristic.h"
#include "qlowenergycontroller.h"
#include "osxbtcentralmanager_p.h"
#include "osxbtnotifier_p.h"

#include <QtCore/qloggingcategory.h>
#include <QtCore/qdebug.h>

#include <algorithm>
#include <vector>
#include <limits>

Q_DECLARE_METATYPE(QLowEnergyHandle)

QT_BEGIN_NAMESPACE

namespace OSXBluetooth {

NSUInteger qt_countGATTEntries(CBService *service)
{
    // Identify, how many characteristics/descriptors we have on a given service,
    // +1 for the service itself.
    // No checks if NSUInteger is big enough :)
    // Let's assume such number of entries is not possible :)

    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const cs = service.characteristics;
    if (!cs || !cs.count)
        return 1;

    NSUInteger n = 1 + cs.count;
    for (CBCharacteristic *c in cs) {
        NSArray *const ds = c.descriptors;
        if (ds)
            n += ds.count;
    }

    return n;
}

ObjCStrongReference<NSError> qt_timeoutNSError(OperationTimeout type)
{
    // For now we do not provide details, since nobody is using this NSError
    // after all (except callbacks checking if an operation was successful
    // or not).
    Q_ASSERT(type != OperationTimeout::none);
    Q_UNUSED(type)
    NSError *nsError = [[NSError alloc] initWithDomain:CBErrorDomain
                                        code:CBErrorOperationCancelled
                                        userInfo:nil];
    return ObjCStrongReference<NSError>(nsError, false /*do not retain, done already*/);
}

auto qt_find_watchdog(const std::vector<GCDTimer> &watchdogs, id object, OperationTimeout type)
{
    return std::find_if(watchdogs.begin(), watchdogs.end(), [object, type](const GCDTimer &other){
                        return [other objectUnderWatch] == object && [other timeoutType] == type;});
}

} // namespace OSXBluetooth

QT_END_NAMESPACE

QT_USE_NAMESPACE

@interface QT_MANGLE_NAMESPACE(OSXBTCentralManager) (PrivateAPI)

- (void)watchAfter:(id)object timeout:(OSXBluetooth::OperationTimeout)type;
- (bool)objectIsUnderWatch:(id)object operation:(OSXBluetooth::OperationTimeout)type;
- (void)stopWatchingAfter:(id)object operation:(OSXBluetooth::OperationTimeout)type;
- (void)stopWatchers;
- (void)retrievePeripheralAndConnect;
- (void)connectToPeripheral;
- (void)discoverIncludedServices;
- (void)readCharacteristics:(CBService *)service;
- (void)serviceDetailsDiscoveryFinished:(CBService *)service;
- (void)performNextRequest;
- (void)performNextReadRequest;
- (void)performNextWriteRequest;

// Aux. functions.
- (CBService *)serviceForUUID:(const QBluetoothUuid &)qtUuid;
- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)from;
- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)from
                      withProperties:(CBCharacteristicProperties)properties;
- (CBDescriptor *)nextDescriptorForCharacteristic:(CBCharacteristic *)characteristic
                  startingFrom:(CBDescriptor *)descriptor;
- (CBDescriptor *)descriptor:(const QBluetoothUuid &)dUuid
                  forCharacteristic:(CBCharacteristic *)ch;
- (bool)cacheWriteValue:(const QByteArray &)value for:(NSObject *)obj;
- (void)handleReadWriteError:(NSError *)error;
- (void)reset;

@end

@implementation QT_MANGLE_NAMESPACE(OSXBTCentralManager)
{
@private
    CBCentralManager *manager;
    OSXBluetooth::CentralManagerState managerState;
    bool disconnectPending;

    QBluetoothUuid deviceUuid;

    OSXBluetooth::LECBManagerNotifier *notifier;

    // Quite a verbose service discovery machinery
    // (a "graph traversal").
    OSXBluetooth::ObjCStrongReference<NSMutableArray> servicesToVisit;
    // The service we're discovering now (included services discovery):
    NSUInteger currentService;
    // Included services, we'll iterate through at the end of 'servicesToVisit':
    OSXBluetooth::ObjCStrongReference<NSMutableArray> servicesToVisitNext;
    // We'd like to avoid loops in a services' topology:
    OSXBluetooth::ObjCStrongReference<NSMutableSet> visitedServices;

    QList<QBluetoothUuid> servicesToDiscoverDetails;

    OSXBluetooth::ServiceHash serviceMap;
    OSXBluetooth::CharHash charMap;
    OSXBluetooth::DescHash descMap;

    QLowEnergyHandle lastValidHandle;

    bool requestPending;
    OSXBluetooth::RequestQueue requests;
    QLowEnergyHandle currentReadHandle;

    OSXBluetooth::ValueHash valuesToWrite;

    qint64 timeoutMS;
    std::vector<OSXBluetooth::GCDTimer> timeoutWatchdogs;

    CBPeripheral *peripheral;
}

- (id)initWith:(OSXBluetooth::LECBManagerNotifier *)aNotifier
{
    using namespace OSXBluetooth;

    if (self = [super init]) {
        manager = nil;
        managerState = CentralManagerIdle;
        disconnectPending = false;
        peripheral = nil;
        notifier = aNotifier;
        currentService = 0;
        lastValidHandle = 0;
        requestPending = false;
        currentReadHandle = 0;

        if (Q_UNLIKELY(!qEnvironmentVariableIsEmpty("BLUETOOTH_GATT_TIMEOUT"))) {
            bool ok = false;
            const int value = qEnvironmentVariableIntValue("BLUETOOTH_GATT_TIMEOUT", &ok);
            if (ok && value >= 0)
                timeoutMS = value;
        }

        if (!timeoutMS)
            timeoutMS = 20000;
    }

    return self;
}

- (void)dealloc
{
    // In the past I had a 'transient delegate': I've seen some crashes
    // while deleting a manager _before_ its state updated.
    // Strangely enough, I can not reproduce this anymore, so this
    // part is simplified now. To be investigated though.

    visitedServices.reset(nil);
    servicesToVisit.reset(nil);
    servicesToVisitNext.reset(nil);

    [manager setDelegate:nil];
    [manager release];

    [peripheral setDelegate:nil];
    [peripheral release];

    if (notifier)
        notifier->deleteLater();

    [self stopWatchers];
    [super dealloc];
}

- (CBPeripheral *)peripheral
{
    return peripheral;
}

- (void)watchAfter:(id)object timeout:(OSXBluetooth::OperationTimeout)type
{
    using namespace OSXBluetooth;

    GCDTimer newWatcher([[GCDTimerObjC alloc] initWithDelegate:self], false /*do not retain*/);
    [newWatcher watchAfter:object withTimeoutType:type];
    timeoutWatchdogs.push_back(newWatcher);
    [newWatcher startWithTimeout:timeoutMS step:200];
}

- (bool)objectIsUnderWatch:(id)object operation:(OSXBluetooth::OperationTimeout)type
{
    return OSXBluetooth::qt_find_watchdog(timeoutWatchdogs, object, type) != timeoutWatchdogs.end();
}

- (void)stopWatchingAfter:(id)object operation:(OSXBluetooth::OperationTimeout)type
{
    auto pos = OSXBluetooth::qt_find_watchdog(timeoutWatchdogs, object, type);
    if (pos != timeoutWatchdogs.end()) {
        [*pos cancelTimer];
        timeoutWatchdogs.erase(pos);
    }
}

- (void)stopWatchers
{
    for (auto &watchdog : timeoutWatchdogs)
        [watchdog cancelTimer];
    timeoutWatchdogs.clear();
}

- (void)timeout:(id)sender
{
    Q_UNUSED(sender)

    using namespace OSXBluetooth;

    GCDTimerObjC *watcher = static_cast<GCDTimerObjC *>(sender);
    id cbObject = [watcher objectUnderWatch];
    const OperationTimeout type = [watcher timeoutType];

    Q_ASSERT([self objectIsUnderWatch:cbObject operation:type]);

    NSLog(@"Timeout caused by: %@", cbObject);

    // Note that after this switch the 'watcher' is released (we don't
    // own it anymore), though GCD is probably still holding a reference.
    const ObjCStrongReference<NSError> nsError(qt_timeoutNSError(type));
    switch (type) {
    case OperationTimeout::serviceDiscovery:
        qCWarning(QT_BT_OSX, "Timeout in services discovery");
        [self peripheral:peripheral didDiscoverServices:nsError];
        break;
    case OperationTimeout::includedServicesDiscovery:
        qCWarning(QT_BT_OSX, "Timeout in included services discovery");
        [self peripheral:peripheral didDiscoverIncludedServicesForService:cbObject error:nsError];
        break;
    case OperationTimeout::characteristicsDiscovery:
        qCWarning(QT_BT_OSX, "Timeout in characteristics discovery");
        [self peripheral:peripheral didDiscoverCharacteristicsForService:cbObject error:nsError];
        break;
    case OperationTimeout::characteristicRead:
        qCWarning(QT_BT_OSX, "Timeout while reading a characteristic");
        [self peripheral:peripheral didUpdateValueForCharacteristic:cbObject error:nsError];
        break;
    case OperationTimeout::descriptorsDiscovery:
        qCWarning(QT_BT_OSX, "Timeout in descriptors discovery");
        [self peripheral:peripheral didDiscoverDescriptorsForCharacteristic:cbObject error:nsError];
        break;
    case OperationTimeout::descriptorRead:
        qCWarning(QT_BT_OSX, "Timeout while reading a descriptor");
        [self peripheral:peripheral didUpdateValueForDescriptor:cbObject error:nsError];
        break;
    case OperationTimeout::characteristicWrite:
        qCWarning(QT_BT_OSX, "Timeout while writing a characteristic with response");
        [self peripheral:peripheral didWriteValueForCharacteristic:cbObject error:nsError];
    default:;
    }
}

- (void)connectToDevice:(const QBluetoothUuid &)aDeviceUuid
{
    disconnectPending = false; // Cancel the previous disconnect if any.
    deviceUuid = aDeviceUuid;

    if (!manager) {
        // The first time we try to connect, no manager created yet,
        // no status update received.
        if (const dispatch_queue_t leQueue = OSXBluetooth::qt_LE_queue()) {
            managerState = OSXBluetooth::CentralManagerUpdating;
            manager = [[CBCentralManager alloc] initWithDelegate:self queue:leQueue];
        }

        if (!manager) {
            managerState = OSXBluetooth::CentralManagerIdle;
            qCWarning(QT_BT_OSX) << "failed to allocate a central manager";
            if (notifier)
                emit notifier->CBManagerError(QLowEnergyController::ConnectionError);
        }
    } else if (managerState != OSXBluetooth::CentralManagerUpdating) {
        [self retrievePeripheralAndConnect];
    }
}

- (void)retrievePeripheralAndConnect
{
    Q_ASSERT_X(manager, Q_FUNC_INFO, "invalid central manager (nil)");
    Q_ASSERT_X(managerState == OSXBluetooth::CentralManagerIdle,
               Q_FUNC_INFO, "invalid state");

    if ([self isConnected]) {
        qCDebug(QT_BT_OSX) << "already connected";
        if (notifier)
            emit notifier->connected();
        return;
    } else if (peripheral) {
        // Was retrieved already, but not connected
        // or disconnected.
        [self connectToPeripheral];
        return;
    }

    using namespace OSXBluetooth;

    // Retrieve a peripheral first ...
    ObjCScopedPointer<NSMutableArray> uuids([[NSMutableArray alloc] init]);
    if (!uuids) {
        qCWarning(QT_BT_OSX) << "failed to allocate identifiers";
        if (notifier)
            emit notifier->CBManagerError(QLowEnergyController::ConnectionError);
        return;
    }


    const quint128 qtUuidData(deviceUuid.toUInt128());
    uuid_t uuidData = {};
    std::copy(qtUuidData.data, qtUuidData.data + 16, uuidData);
    const ObjCScopedPointer<NSUUID> nsUuid([[NSUUID alloc] initWithUUIDBytes:uuidData]);
    if (!nsUuid) {
        qCWarning(QT_BT_OSX) << "failed to allocate NSUUID identifier";
        if (notifier)
            emit notifier->CBManagerError(QLowEnergyController::ConnectionError);
        return;
    }

    [uuids addObject:nsUuid];
    // With the latest CoreBluetooth, we can synchronously retrive peripherals:
    QT_BT_MAC_AUTORELEASEPOOL;
    NSArray *const peripherals = [manager retrievePeripheralsWithIdentifiers:uuids];
    if (!peripherals || peripherals.count != 1) {
        qCWarning(QT_BT_OSX) << "failed to retrive a peripheral";
        if (notifier)
            emit notifier->CBManagerError(QLowEnergyController::UnknownRemoteDeviceError);
        return;
    }

    peripheral = [static_cast<CBPeripheral *>([peripherals objectAtIndex:0]) retain];
    [self connectToPeripheral];
}

- (void)connectToPeripheral
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(manager, Q_FUNC_INFO, "invalid central manager (nil)");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");
    Q_ASSERT_X(managerState == CentralManagerIdle, Q_FUNC_INFO, "invalid state");

    // The state is still the same - connecting.
    if ([self isConnected]) {
        qCDebug(QT_BT_OSX) << "already connected";
        if (notifier)
            emit notifier->connected();
    } else {
        qCDebug(QT_BT_OSX) << "trying to connect";
        managerState = CentralManagerConnecting;
        [manager connectPeripheral:peripheral options:nil];
    }
}

- (bool)isConnected
{
    if (!peripheral)
        return false;

    return peripheral.state == CBPeripheralStateConnected;
}

- (void)disconnectFromDevice
{
    [self reset];

    if (managerState == OSXBluetooth::CentralManagerUpdating) {
        disconnectPending = true; // this is for 'didUpdate' method.
        if (notifier) {
            // We were waiting for the first update
            // with 'PoweredOn' status, when suddenly got disconnected called.
            // Since we have not attempted to connect yet, emit now.
            // Note: we do not change the state, since we still maybe interested
            // in the status update before the next connect attempt.
            emit notifier->disconnected();
        }
    } else {
        disconnectPending = false;
        if ([self isConnected])
            managerState = OSXBluetooth::CentralManagerDisconnecting;
        else
            managerState = OSXBluetooth::CentralManagerIdle;

        // We have to call -cancelPeripheralConnection: even
        // if not connected (to cancel a pending connect attempt).
        // Unfortunately, didDisconnect callback is not always called
        // (despite of Apple's docs saying it _must_ be).
        if (peripheral)
            [manager cancelPeripheralConnection:peripheral];
    }
}

- (void)discoverServices
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");
    Q_ASSERT_X(managerState == CentralManagerIdle, Q_FUNC_INFO, "invalid state");

    // From Apple's docs:
    //
    //"If the servicesUUIDs parameter is nil, all the available
    //services of the peripheral are returned; setting the
    //parameter to nil is considerably slower and is not recommended."
    //
    // ... but we'd like to have them all:
    [peripheral setDelegate:self];
    managerState = CentralManagerDiscovering;
    [self watchAfter:peripheral timeout:OperationTimeout::serviceDiscovery];
    [peripheral discoverServices:nil];
}

- (void)discoverIncludedServices
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(managerState == CentralManagerIdle, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(manager, Q_FUNC_INFO, "invalid manager (nil)");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const services = peripheral.services;
    if (!services || !services.count) {
        // A peripheral without any services at all.
        if (notifier)
            emit notifier->serviceDiscoveryFinished();
    } else {
        // 'reset' also calls retain on a parameter.
        servicesToVisitNext.reset(nil);
        servicesToVisit.reset([NSMutableArray arrayWithArray:services]);
        currentService = 0;
        visitedServices.reset([NSMutableSet setWithCapacity:peripheral.services.count]);

        CBService *const s = [services objectAtIndex:currentService];
        [visitedServices addObject:s];
        managerState = CentralManagerDiscovering;
        [self watchAfter:s timeout:OperationTimeout::includedServicesDiscovery];
        [peripheral discoverIncludedServices:nil forService:s];
    }
}

- (void)discoverServiceDetails:(const QBluetoothUuid &)serviceUuid
{
    // This function does not change 'managerState', since it
    // can be called concurrently (not waiting for the previous
    // discovery to finish).

    using namespace OSXBluetooth;

    Q_ASSERT_X(managerState != CentralManagerUpdating, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(!serviceUuid.isNull(), Q_FUNC_INFO, "invalid service UUID");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    if (servicesToDiscoverDetails.contains(serviceUuid)) {
        qCWarning(QT_BT_OSX) << "already discovering for"
                             << serviceUuid;
        return;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    if (CBService *const service = [self serviceForUUID:serviceUuid]) {
        servicesToDiscoverDetails.append(serviceUuid);
        [self watchAfter:service timeout:OperationTimeout::characteristicsDiscovery];
        [peripheral discoverCharacteristics:nil forService:service];
        return;
    }

    qCWarning(QT_BT_OSX) << "unknown service uuid"
                         << serviceUuid;

    if (notifier)
        emit notifier->CBManagerError(serviceUuid, QLowEnergyService::UnknownError);
}

- (void)readCharacteristics:(CBService *)service
{
    // This method does not change 'managerState', we can
    // have several 'detail discoveries' active.
    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT_X(managerState != CentralManagerUpdating, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(manager, Q_FUNC_INFO, "invalid manager (nil)");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    if (!service.characteristics || !service.characteristics.count)
        return [self serviceDetailsDiscoveryFinished:service];

    NSArray *const cs = service.characteristics;
    for (CBCharacteristic *c in cs) {
        if (c.properties & CBCharacteristicPropertyRead) {
            [self watchAfter:c timeout:OperationTimeout::characteristicRead];
            return [peripheral readValueForCharacteristic:c];
        }
    }

    // No readable properties? Discover descriptors then:
    [self discoverDescriptors:service];
}

- (void)discoverDescriptors:(CBService *)service
{
    // This method does not change 'managerState', we can have
    // several discoveries active.
    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT_X(managerState != CentralManagerUpdating,
               Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(manager, Q_FUNC_INFO, "invalid manager (nil)");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    if (!service.characteristics || !service.characteristics.count) {
        [self serviceDetailsDiscoveryFinished:service];
    } else {
        // Start from 0 and continue in the callback.
        CBCharacteristic *ch = [service.characteristics objectAtIndex:0];
        [self watchAfter:ch timeout:OperationTimeout::descriptorsDiscovery];
        [peripheral discoverDescriptorsForCharacteristic:ch];
    }
}

- (void)readDescriptors:(CBService *)service
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");
    Q_ASSERT_X(managerState != CentralManagerUpdating, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const cs = service.characteristics;
    // We can never be here if we have no characteristics.
    Q_ASSERT_X(cs && cs.count, Q_FUNC_INFO, "invalid service");
    for (CBCharacteristic *c in cs) {
        if (c.descriptors && c.descriptors.count) {
            CBDescriptor *desc = [c.descriptors objectAtIndex:0];
            [self watchAfter:desc timeout:OperationTimeout::descriptorRead];
            return [peripheral readValueForDescriptor:desc];
        }
    }

    // No descriptors to read, done.
    [self serviceDetailsDiscoveryFinished:service];
}

- (void)serviceDetailsDiscoveryFinished:(CBService *)service
{
    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    const QBluetoothUuid serviceUuid(qt_uuid(service.UUID));
    servicesToDiscoverDetails.removeAll(serviceUuid);

    const NSUInteger nHandles = qt_countGATTEntries(service);
    Q_ASSERT_X(nHandles, Q_FUNC_INFO, "unexpected number of GATT entires");

    const QLowEnergyHandle maxHandle = std::numeric_limits<QLowEnergyHandle>::max();
    if (nHandles >= maxHandle || lastValidHandle > maxHandle - nHandles) {
        // Well, that's unlikely :) But we must be sure.
        qCWarning(QT_BT_OSX) << "can not allocate more handles";
        if (notifier)
            notifier->CBManagerError(serviceUuid, QLowEnergyService::OperationError);
        return;
    }

    // A temporary service object to pass the details.
    // Set only uuid, characteristics and descriptors (and probably values),
    // nothing else is needed.
    QSharedPointer<QLowEnergyServicePrivate> qtService(new QLowEnergyServicePrivate);
    qtService->uuid = serviceUuid;
    // We 'register' handles/'CBentities' even if qlowenergycontroller (delegate)
    // later fails to do this with some error. Otherwise, if we try to implement
    // rollback/transaction logic interface is getting too ugly/complicated.
    ++lastValidHandle;
    serviceMap[lastValidHandle] = service;
    qtService->startHandle = lastValidHandle;

    NSArray *const cs = service.characteristics;
    // Now map chars/descriptors and handles.
    if (cs && cs.count) {
        QHash<QLowEnergyHandle, QLowEnergyServicePrivate::CharData> charList;

        for (CBCharacteristic *c in cs) {
            ++lastValidHandle;
            // Register this characteristic:
            charMap[lastValidHandle] = c;
            // Create a Qt's internal characteristic:
            QLowEnergyServicePrivate::CharData newChar = {};
            newChar.uuid = qt_uuid(c.UUID);
            const int cbProps = c.properties & 0xff;
            newChar.properties = static_cast<QLowEnergyCharacteristic::PropertyTypes>(cbProps);
            newChar.value = qt_bytearray(c.value);
            newChar.valueHandle = lastValidHandle;

            NSArray *const ds = c.descriptors;
            if (ds && ds.count) {
                QHash<QLowEnergyHandle, QLowEnergyServicePrivate::DescData> descList;
                for (CBDescriptor *d in ds) {
                    // Register this descriptor:
                    ++lastValidHandle;
                    descMap[lastValidHandle] = d;
                    // Create a Qt's internal descriptor:
                    QLowEnergyServicePrivate::DescData newDesc = {};
                    newDesc.uuid = qt_uuid(d.UUID);
                    newDesc.value = qt_bytearray(static_cast<NSObject *>(d.value));
                    descList[lastValidHandle] = newDesc;
                    // Check, if it's client characteristic configuration descriptor:
                    if (newDesc.uuid == QBluetoothUuid::ClientCharacteristicConfiguration) {
                        if (newDesc.value.size() && (newDesc.value[0] & 3))
                            [peripheral setNotifyValue:YES forCharacteristic:c];
                    }
                }

                newChar.descriptorList = descList;
            }

            charList[newChar.valueHandle] = newChar;
        }

        qtService->characteristicList = charList;
    }

    qtService->endHandle = lastValidHandle;

    if (notifier)
        emit notifier->serviceDetailsDiscoveryFinished(qtService);
}

- (void)performNextRequest
{
    using namespace OSXBluetooth;

    if (requestPending || !requests.size())
        return;

    switch (requests.head().type) {
    case LERequest::CharRead:
    case LERequest::DescRead:
        return [self performNextReadRequest];
    case LERequest::CharWrite:
    case LERequest::DescWrite:
    case LERequest::ClientConfiguration:
        return [self performNextWriteRequest];
    default:
        // Should never happen.
        Q_ASSERT(0);
    }
}

- (void)performNextReadRequest
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");
    Q_ASSERT_X(!requestPending, Q_FUNC_INFO, "processing another request");
    Q_ASSERT_X(requests.size(), Q_FUNC_INFO, "no requests to handle");
    Q_ASSERT_X(requests.head().type == LERequest::CharRead
               || requests.head().type == LERequest::DescRead,
               Q_FUNC_INFO, "not a read request");

    const LERequest request(requests.dequeue());
    if (request.type == LERequest::CharRead) {
        if (!charMap.contains(request.handle)) {
            qCWarning(QT_BT_OSX) << "characteristic with handle"
                                 << request.handle << "not found";
            return [self performNextRequest];
        }

        requestPending = true;
        currentReadHandle = request.handle;
        // Timeouts: for now, we do not alert timeoutWatchdog - never had such
        // bug reports and after all a read timeout can be handled externally.
        [peripheral readValueForCharacteristic:charMap[request.handle]];
    } else {
        if (!descMap.contains(request.handle)) {
            qCWarning(QT_BT_OSX) << "descriptor with handle"
                                 << request.handle << "not found";
            return [self performNextRequest];
        }

        requestPending = true;
        currentReadHandle = request.handle;
        // Timeouts: see the comment above (CharRead).
        [peripheral readValueForDescriptor:descMap[request.handle]];
    }
}

- (void)performNextWriteRequest
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");
    Q_ASSERT_X(!requestPending, Q_FUNC_INFO, "processing another request");
    Q_ASSERT_X(requests.size(), Q_FUNC_INFO, "no requests to handle");
    Q_ASSERT_X(requests.head().type == LERequest::CharWrite
               || requests.head().type == LERequest::DescWrite
               || requests.head().type == LERequest::ClientConfiguration,
               Q_FUNC_INFO, "not a write request");

    const LERequest request(requests.dequeue());

    if (request.type == LERequest::DescWrite) {
        if (!descMap.contains(request.handle)) {
            qCWarning(QT_BT_OSX) << "handle:" << request.handle
                                 << "not found";
            return [self performNextRequest];
        }

        CBDescriptor *const descriptor = descMap[request.handle];
        ObjCStrongReference<NSData> data(data_from_bytearray(request.value));
        if (!data) {
            // Even if qtData.size() == 0, we still need NSData object.
            qCWarning(QT_BT_OSX) << "failed to allocate an NSData object";
            return [self performNextRequest];
        }

        if (![self cacheWriteValue:request.value for:descriptor])
            return [self performNextRequest];

        requestPending = true;
        return [peripheral writeValue:data.data() forDescriptor:descriptor];
    } else {
        if (!charMap.contains(request.handle)) {
            qCWarning(QT_BT_OSX) << "characteristic with handle:"
                                 << request.handle << "not found";
            return [self performNextRequest];
        }

        CBCharacteristic *const characteristic = charMap[request.handle];

        if (request.type == LERequest::ClientConfiguration) {
            const QBluetoothUuid qtUuid(QBluetoothUuid::ClientCharacteristicConfiguration);
            CBDescriptor *const descriptor = [self descriptor:qtUuid forCharacteristic:characteristic];
            Q_ASSERT_X(descriptor, Q_FUNC_INFO, "no client characteristic "
                       "configuration descriptor found");

            if (![self cacheWriteValue:request.value for:descriptor])
                return [self performNextRequest];

            bool enable = false;
            if (request.value.size())
                enable = request.value[0] & 3;

            requestPending = true;
            [peripheral setNotifyValue:enable forCharacteristic:characteristic];
        } else {
            ObjCStrongReference<NSData> data(data_from_bytearray(request.value));
            if (!data) {
                // Even if qtData.size() == 0, we still need NSData object.
                qCWarning(QT_BT_OSX) << "failed to allocate NSData object";
                return [self performNextRequest];
            }

            // TODO: check what happens if I'm using NSData with length 0.
            if (request.withResponse) {
                if (![self cacheWriteValue:request.value for:characteristic])
                    return [self performNextRequest];

                requestPending = true;
                [self watchAfter:characteristic timeout:OperationTimeout::characteristicWrite];
                [peripheral writeValue:data.data() forCharacteristic:characteristic
                            type:CBCharacteristicWriteWithResponse];
            } else {
                [peripheral writeValue:data.data() forCharacteristic:characteristic
                            type:CBCharacteristicWriteWithoutResponse];
                [self performNextRequest];
            }
        }
    }
}

- (void)setNotifyValue:(const QByteArray &)value
        forCharacteristic:(QLowEnergyHandle)charHandle
        onService:(const QBluetoothUuid &)serviceUuid
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle (0)");

    if (!charMap.contains(charHandle)) {
        qCWarning(QT_BT_OSX) << "unknown characteristic handle"
                             << charHandle;
        if (notifier) {
            emit notifier->CBManagerError(serviceUuid,
                           QLowEnergyService::DescriptorWriteError);
        }
        return;
    }

    // At the moment we call setNotifyValue _only_ from 'writeDescriptor';
    // from Qt's API POV it's a descriptor write operation and we must report
    // it back, so check _now_ that we really have this descriptor.
    const QBluetoothUuid qtUuid(QBluetoothUuid::ClientCharacteristicConfiguration);
    if (![self descriptor:qtUuid forCharacteristic:charMap[charHandle]]) {
        qCWarning(QT_BT_OSX) << "no client characteristic configuration found";
        if (notifier) {
            emit notifier->CBManagerError(serviceUuid,
                           QLowEnergyService::DescriptorWriteError);
        }
        return;
    }

    LERequest request;
    request.type = LERequest::ClientConfiguration;
    request.handle = charHandle;
    request.value = value;

    requests.enqueue(request);
    [self performNextRequest];
}

- (void)readCharacteristic:(QLowEnergyHandle)charHandle
        onService:(const QBluetoothUuid &)serviceUuid
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle (0)");

    QT_BT_MAC_AUTORELEASEPOOL;

    if (!charMap.contains(charHandle)) {
        qCWarning(QT_BT_OSX) << "characteristic:" << charHandle << "not found";
        if (notifier) {
            emit notifier->CBManagerError(serviceUuid,
                           QLowEnergyService::CharacteristicReadError);

        }
        return;
    }

    LERequest request;
    request.type = LERequest::CharRead;
    request.handle = charHandle;

    requests.enqueue(request);
    [self performNextRequest];
}

- (void)write:(const QByteArray &)value
        charHandle:(QLowEnergyHandle)charHandle
        onService:(const QBluetoothUuid &)serviceUuid
        withResponse:(bool)withResponse
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(charHandle, Q_FUNC_INFO, "invalid characteristic handle (0)");

    QT_BT_MAC_AUTORELEASEPOOL;

    if (!charMap.contains(charHandle)) {
        qCWarning(QT_BT_OSX) << "characteristic:" << charHandle << "not found";
        if (notifier) {
            emit notifier->CBManagerError(serviceUuid,
                           QLowEnergyService::CharacteristicWriteError);
        }
        return;
    }

    LERequest request;
    request.type = LERequest::CharWrite;
    request.withResponse = withResponse;
    request.handle = charHandle;
    request.value = value;

    requests.enqueue(request);
    [self performNextRequest];
}

- (void)readDescriptor:(QLowEnergyHandle)descHandle
        onService:(const QBluetoothUuid &)serviceUuid
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(descHandle, Q_FUNC_INFO, "invalid descriptor handle (0)");

    if (!descMap.contains(descHandle)) {
        qCWarning(QT_BT_OSX) << "handle:" << descHandle << "not found";
        if (notifier) {
            emit notifier->CBManagerError(serviceUuid,
                           QLowEnergyService::DescriptorReadError);
        }
        return;
    }

    LERequest request;
    request.type = LERequest::DescRead;
    request.handle = descHandle;

    requests.enqueue(request);
    [self performNextRequest];
}

- (void)write:(const QByteArray &)value
        descHandle:(QLowEnergyHandle)descHandle
        onService:(const QBluetoothUuid &)serviceUuid
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(descHandle, Q_FUNC_INFO, "invalid descriptor handle (0)");

    if (!descMap.contains(descHandle)) {
        qCWarning(QT_BT_OSX) << "handle:" << descHandle << "not found";
        if (notifier) {
            emit notifier->CBManagerError(serviceUuid,
                           QLowEnergyService::DescriptorWriteError);
        }
        return;
    }

    LERequest request;
    request.type = LERequest::DescWrite;
    request.handle = descHandle;
    request.value = value;

    requests.enqueue(request);
    [self performNextRequest];
}

// Aux. methods:

- (CBService *)serviceForUUID:(const QBluetoothUuid &)qtUuid
{
    using namespace OSXBluetooth;

    Q_ASSERT_X(!qtUuid.isNull(), Q_FUNC_INFO, "invalid uuid");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    ObjCStrongReference<NSMutableArray> toVisit([NSMutableArray arrayWithArray:peripheral.services], true);
    ObjCStrongReference<NSMutableArray> toVisitNext([[NSMutableArray alloc] init], false);
    ObjCStrongReference<NSMutableSet> visitedNodes([[NSMutableSet alloc] init], false);

    while (true) {
        for (NSUInteger i = 0, e = [toVisit count]; i < e; ++i) {
            CBService *const s = [toVisit objectAtIndex:i];
            if (equal_uuids(s.UUID, qtUuid))
                return s;
            if (![visitedNodes containsObject:s] && s.includedServices && s.includedServices.count) {
                [visitedNodes addObject:s];
                [toVisitNext addObjectsFromArray:s.includedServices];
            }
        }

        if (![toVisitNext count])
            return nil;

        toVisit.resetWithoutRetain(toVisitNext.take());
        toVisitNext.resetWithoutRetain([[NSMutableArray alloc] init]);
    }

    return nil;
}

- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)characteristic
{
    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");
    Q_ASSERT_X(characteristic, Q_FUNC_INFO, "invalid characteristic (nil)");
    Q_ASSERT_X(service.characteristics, Q_FUNC_INFO, "invalid service");
    Q_ASSERT_X(service.characteristics.count, Q_FUNC_INFO, "invalid service");

    QT_BT_MAC_AUTORELEASEPOOL;

    // TODO: test that we NEVER have the same characteristic twice in array!
    // At the moment I just protect against this by iterating in a reverse
    // order (at least avoiding a potential inifite loop with '-indexOfObject:').
    NSArray *const cs = service.characteristics;
    if (cs.count == 1)
        return nil;

    for (NSUInteger index = cs.count - 1; index != 0; --index) {
        if ([cs objectAtIndex:index] == characteristic) {
            if (index + 1 == cs.count)
                return nil;
            else
                return [cs objectAtIndex:index + 1];
        }
    }

    Q_ASSERT_X([cs objectAtIndex:0] == characteristic, Q_FUNC_INFO,
               "characteristic was not found in service.characteristics");

    return [cs objectAtIndex:1];
}

- (CBCharacteristic *)nextCharacteristicForService:(CBService*)service
                      startingFrom:(CBCharacteristic *)characteristic
                      properties:(CBCharacteristicProperties)properties
{
    Q_ASSERT_X(service, Q_FUNC_INFO, "invalid service (nil)");
    Q_ASSERT_X(characteristic, Q_FUNC_INFO, "invalid characteristic (nil)");
    Q_ASSERT_X(service.characteristics, Q_FUNC_INFO, "invalid service");
    Q_ASSERT_X(service.characteristics.count, Q_FUNC_INFO, "invalid service");

    QT_BT_MAC_AUTORELEASEPOOL;

    // TODO: test that we NEVER have the same characteristic twice in array!
    // At the moment I just protect against this by iterating in a reverse
    // order (at least avoiding a potential inifite loop with '-indexOfObject:').
    NSArray *const cs = service.characteristics;
    if (cs.count == 1)
        return nil;

    NSUInteger index = cs.count - 1;
    for (; index != 0; --index) {
        if ([cs objectAtIndex:index] == characteristic) {
            if (index + 1 == cs.count) {
                return nil;
            } else {
                index += 1;
                break;
            }
        }
    }

    if (!index) {
        Q_ASSERT_X([cs objectAtIndex:0] == characteristic, Q_FUNC_INFO,
                   "characteristic not found in service.characteristics");
        index = 1;
    }

    for (const NSUInteger e = cs.count; index < e; ++index) {
        CBCharacteristic *const c = [cs objectAtIndex:index];
        if (c.properties & properties)
            return c;
    }

    return nil;
}

- (CBDescriptor *)nextDescriptorForCharacteristic:(CBCharacteristic *)characteristic
                  startingFrom:(CBDescriptor *)descriptor
{
    Q_ASSERT_X(characteristic, Q_FUNC_INFO, "invalid characteristic (nil)");
    Q_ASSERT_X(descriptor, Q_FUNC_INFO, "invalid descriptor (nil)");
    Q_ASSERT_X(characteristic.descriptors, Q_FUNC_INFO, "invalid characteristic");
    Q_ASSERT_X(characteristic.descriptors.count, Q_FUNC_INFO, "invalid characteristic");

    QT_BT_MAC_AUTORELEASEPOOL;

    NSArray *const ds = characteristic.descriptors;
    if (ds.count == 1)
        return nil;

    for (NSUInteger index = ds.count - 1; index != 0; --index) {
        if ([ds objectAtIndex:index] == descriptor) {
            if (index + 1 == ds.count)
                return nil;
            else
                return [ds objectAtIndex:index + 1];
        }
    }

    Q_ASSERT_X([ds objectAtIndex:0] == descriptor, Q_FUNC_INFO,
               "descriptor was not found in characteristic.descriptors");

    return [ds objectAtIndex:1];
}

- (CBDescriptor *)descriptor:(const QBluetoothUuid &)qtUuid
                  forCharacteristic:(CBCharacteristic *)ch
{
    if (qtUuid.isNull() || !ch)
        return nil;

    QT_BT_MAC_AUTORELEASEPOOL;

    CBDescriptor *descriptor = nil;
    NSArray *const ds = ch.descriptors;
    if (ds && ds.count) {
        for (CBDescriptor *d in ds) {
            if (OSXBluetooth::equal_uuids(d.UUID, qtUuid)) {
                descriptor = d;
                break;
            }
        }
    }

    return descriptor;
}

- (bool)cacheWriteValue:(const QByteArray &)value for:(NSObject *)obj
{
    Q_ASSERT_X(obj, Q_FUNC_INFO, "invalid object (nil)");

    if ([obj isKindOfClass:[CBCharacteristic class]]) {
        CBCharacteristic *const ch = static_cast<CBCharacteristic *>(obj);
        if (!charMap.key(ch)) {
            qCWarning(QT_BT_OSX) << "unexpected characteristic, no handle found";
            return false;
        }
    } else if ([obj isKindOfClass:[CBDescriptor class]]) {
        CBDescriptor *const d = static_cast<CBDescriptor *>(obj);
        if (!descMap.key(d)) {
            qCWarning(QT_BT_OSX) << "unexpected descriptor, no handle found";
            return false;
        }
    } else {
        qCWarning(QT_BT_OSX) << "invalid object, characteristic "
                                "or descriptor required";
        return false;
    }

    if (valuesToWrite.contains(obj)) {
        // It can be a result of some previous errors - for example,
        // we never got a callback from a previous write.
        qCWarning(QT_BT_OSX) << "already has a cached value for this "
                                "object, the value will be replaced";
    }

    valuesToWrite[obj] = value;
    return true;
}

- (void)reset
{
    requestPending = false;
    valuesToWrite.clear();
    requests.clear();
    servicesToDiscoverDetails.clear();
    lastValidHandle = 0;
    serviceMap.clear();
    charMap.clear();
    descMap.clear();
    currentReadHandle = 0;
    [self stopWatchers];
    // TODO: also serviceToVisit/VisitNext and visitedServices ?
}

- (void)handleReadWriteError:(NSError *)error
{
    Q_ASSERT(notifier);

    switch (error.code) {
    case 0x05: // GATT_INSUFFICIENT_AUTHORIZATION
    case 0x0F: // GATT_INSUFFICIENT_ENCRYPTION
        emit notifier->CBManagerError(QLowEnergyController::AuthorizationError);
        [self detach];
        break;
    default:
        break;
    }
}

// CBCentralManagerDelegate (the real one).

- (void)centralManagerDidUpdateState:(CBCentralManager *)central
{
    using namespace OSXBluetooth;

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunguarded-availability-new"

    const auto state = central.state;
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    if (state == CBManagerStateUnknown
        || state == CBManagerStateResetting) {
#else
    if (state == CBCentralManagerStateUnknown
        || state == CBCentralManagerStateResetting) {
#endif
        // We still have to wait, docs say:
        // "The current state of the central manager is unknown;
        // an update is imminent." or
        // "The connection with the system service was momentarily
        // lost; an update is imminent."
        return;
    }

    // Let's check some states we do not like first:
#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    if (state == CBManagerStateUnsupported || state == CBManagerStateUnauthorized) {
#else
    if (state == CBCentralManagerStateUnsupported || state == CBCentralManagerStateUnauthorized) {
#endif
        if (managerState == CentralManagerUpdating) {
            // We tried to connect just to realize, LE is not supported. Report this.
            managerState = CentralManagerIdle;
            if (notifier)
                emit notifier->LEnotSupported();
        } else {
            // TODO: if we are here, LE _was_ supported and we first managed to update
            // and reset managerState from CentralManagerUpdating.
            managerState = CentralManagerIdle;
            if (notifier)
                emit notifier->CBManagerError(QLowEnergyController::InvalidBluetoothAdapterError);
        }
        [self stopWatchers];
        return;
    }

#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    if (state == CBManagerStatePoweredOff) {
#else
    if (state == CBCentralManagerStatePoweredOff) {
#endif

        if (managerState == CentralManagerUpdating) {
            managerState = CentralManagerIdle;
            // I've seen this instead of Unsupported on OS X.
            if (notifier)
                emit notifier->LEnotSupported();
        } else {
            managerState = CentralManagerIdle;
            // TODO: we need a better error +
            // what will happen if later the state changes to PoweredOn???
            if (notifier)
                emit notifier->CBManagerError(QLowEnergyController::InvalidBluetoothAdapterError);
        }
        [self stopWatchers];
        return;
    }

#if QT_IOS_PLATFORM_SDK_EQUAL_OR_ABOVE(__IPHONE_10_0) || QT_OSX_PLATFORM_SDK_EQUAL_OR_ABOVE(__MAC_10_13)
    if (state == CBManagerStatePoweredOn) {
#else
    if (state == CBCentralManagerStatePoweredOn) {
#endif
        if (managerState == CentralManagerUpdating && !disconnectPending) {
            managerState = CentralManagerIdle;
            [self retrievePeripheralAndConnect];
        }
    } else {
        // We actually handled all known states, but .. Core Bluetooth can change?
        Q_ASSERT_X(0, Q_FUNC_INFO, "invalid central's state");
    }

#pragma clang diagnostic pop
}

- (void)centralManager:(CBCentralManager *)central didConnectPeripheral:(CBPeripheral *)aPeripheral
{
    Q_UNUSED(central)
    Q_UNUSED(aPeripheral)

    if (managerState != OSXBluetooth::CentralManagerConnecting) {
        // We called cancel but before disconnected, managed to connect?
        return;
    }

    managerState = OSXBluetooth::CentralManagerIdle;
    if (notifier)
        emit notifier->connected();
}

- (void)centralManager:(CBCentralManager *)central didFailToConnectPeripheral:(CBPeripheral *)aPeripheral
        error:(NSError *)error
{
    Q_UNUSED(central)
    Q_UNUSED(aPeripheral)
    Q_UNUSED(error)

    if (managerState != OSXBluetooth::CentralManagerConnecting) {
        // Canceled already.
        return;
    }

    managerState = OSXBluetooth::CentralManagerIdle;
    // TODO: better error mapping is required.
    if (notifier)
        notifier->CBManagerError(QLowEnergyController::UnknownRemoteDeviceError);
}

- (void)centralManager:(CBCentralManager *)central didDisconnectPeripheral:(CBPeripheral *)aPeripheral
        error:(NSError *)error
{
    Q_UNUSED(central)
    Q_UNUSED(aPeripheral)

    // Clear internal caches/data.
    [self reset];

    if (error && managerState == OSXBluetooth::CentralManagerDisconnecting) {
        managerState = OSXBluetooth::CentralManagerIdle;
        qCWarning(QT_BT_OSX) << "failed to disconnect";
        if (notifier)
            emit notifier->CBManagerError(QLowEnergyController::UnknownRemoteDeviceError);
    } else {
        managerState = OSXBluetooth::CentralManagerIdle;
        if (notifier)
            emit notifier->disconnected();
    }
}

// CBPeripheralDelegate.

- (void)peripheral:(CBPeripheral *)aPeripheral didDiscoverServices:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    if (managerState != OSXBluetooth::CentralManagerDiscovering) {
        // Canceled by -disconnectFromDevice, or as a result of a timeout.
        return;
    }

    using namespace OSXBluetooth;
    if (![self objectIsUnderWatch:aPeripheral operation:OperationTimeout::serviceDiscovery]) // Timed out already
        return;

    QT_BT_MAC_AUTORELEASEPOOL;

    [self stopWatchingAfter:aPeripheral operation:OperationTimeout::serviceDiscovery];

    managerState = OSXBluetooth::CentralManagerIdle;

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);
        // TODO: better error mapping required.
        if (notifier)
            emit notifier->CBManagerError(QLowEnergyController::UnknownError);
    }

    [self discoverIncludedServices];
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didModifyServices:(NSArray<CBService *> *)invalidatedServices
{
    Q_UNUSED(aPeripheral)
    Q_UNUSED(invalidatedServices)

    qCWarning(QT_BT_OSX) << "The peripheral has modified its services.";
    // "This method is invoked whenever one or more services of a peripheral have changed.
    // A peripheral’s services have changed if:
    // * A service is removed from the peripheral’s database
    // * A new service is added to the peripheral’s database
    // * A service that was previously removed from the peripheral’s
    //   database is readded to the database at a different location"

    // In case new services were added - we have to discover them.
    // In case some were removed - we can end up with dangling pointers
    // (see our 'watchdogs', for example). To handle the situation
    // we stop all current operations here, report to QLowEnergyController
    // so that it can trigger re-discovery.
    [self reset];
    managerState = OSXBluetooth::CentralManagerIdle;
    if (notifier)
        emit notifier->servicesWereModified();
}

- (void)peripheral:(CBPeripheral *)aPeripheral didDiscoverIncludedServicesForService:(CBService *)service
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    using namespace OSXBluetooth;

    if (managerState != CentralManagerDiscovering) {
        // Canceled by disconnectFromDevice or -peripheralDidDisconnect...
        return;
    }

    if (![self objectIsUnderWatch:service operation:OperationTimeout::includedServicesDiscovery])
        return;

    QT_BT_MAC_AUTORELEASEPOOL;

    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    [self stopWatchingAfter:service operation:OperationTimeout::includedServicesDiscovery];
    managerState = CentralManagerIdle;

    if (error) {
        NSLog(@"%s: finished with error %@ for service %@",
              Q_FUNC_INFO, error, service.UUID);
    } else if (service.includedServices && service.includedServices.count) {
        // Now we have even more services to do included services discovery ...
        if (!servicesToVisitNext)
            servicesToVisitNext.reset([NSMutableArray arrayWithArray:service.includedServices]);
        else
            [servicesToVisitNext addObjectsFromArray:service.includedServices];
    }

    // Do we have something else to discover on this 'level'?
    ++currentService;

    for (const NSUInteger e = [servicesToVisit count]; currentService < e; ++currentService) {
        CBService *const s = [servicesToVisit objectAtIndex:currentService];
        if (![visitedServices containsObject:s]) {
            // Continue with discovery ...
            [visitedServices addObject:s];
            managerState = CentralManagerDiscovering;
            [self watchAfter:s timeout:OperationTimeout::includedServicesDiscovery];
            return [peripheral discoverIncludedServices:nil forService:s];
        }
    }

    // No services to visit more on this 'level'.

    if (servicesToVisitNext && [servicesToVisitNext count]) {
        servicesToVisit.resetWithoutRetain(servicesToVisitNext.take());

        currentService = 0;
        for (const NSUInteger e = [servicesToVisit count]; currentService < e; ++currentService) {
            CBService *const s = [servicesToVisit objectAtIndex:currentService];
            if (![visitedServices containsObject:s]) {
                [visitedServices addObject:s];
                managerState = CentralManagerDiscovering;
                [self watchAfter:s timeout:OperationTimeout::includedServicesDiscovery];
                return [peripheral discoverIncludedServices:nil forService:s];
            }
        }
    }

    // Finally, if we're here, the service discovery is done!

    // Release all these things now, no need to prolong their lifetime.
    visitedServices.reset(nil);
    servicesToVisit.reset(nil);
    servicesToVisitNext.reset(nil);

    if (notifier)
        emit notifier->serviceDiscoveryFinished();
}

- (void)peripheral:(CBPeripheral *)aPeripheral didDiscoverCharacteristicsForService:(CBService *)service
        error:(NSError *)error
{
    // This method does not change 'managerState', we can have several
    // discoveries active.
    Q_UNUSED(aPeripheral)

    if (!notifier) {
        // Detached.
        return;
    }

    using namespace OSXBluetooth;

    if (![self objectIsUnderWatch:service operation:OperationTimeout::characteristicsDiscovery])
        return;

    QT_BT_MAC_AUTORELEASEPOOL;

    [self stopWatchingAfter:service operation:OperationTimeout::characteristicsDiscovery];

    Q_ASSERT_X(managerState != CentralManagerUpdating, Q_FUNC_INFO, "invalid state");

    if (error) {
        NSLog(@"%s failed with error: %@", Q_FUNC_INFO, error);
        // We did not discover any characteristics and can not discover descriptors,
        // inform our delegate (it will set a service state also).
        emit notifier->CBManagerError(qt_uuid(service.UUID), QLowEnergyController::UnknownError);
    }

    [self readCharacteristics:service];
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didUpdateValueForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    if (!notifier) // Detached.
        return;

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    const bool readMatch = [self objectIsUnderWatch:characteristic operation:OperationTimeout::characteristicRead];
    if (readMatch)
        [self stopWatchingAfter:characteristic operation:OperationTimeout::characteristicRead];

    Q_ASSERT_X(managerState != CentralManagerUpdating, Q_FUNC_INFO, "invalid state");
    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");


    // First, let's check if we're discovering a service details now.
    CBService *const service = characteristic.service;
    const QBluetoothUuid qtUuid(qt_uuid(service.UUID));
    const bool isDetailsDiscovery = servicesToDiscoverDetails.contains(qtUuid);
    const QLowEnergyHandle chHandle = charMap.key(characteristic);

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);
        if (!isDetailsDiscovery) {
            if (chHandle && chHandle == currentReadHandle) {
                currentReadHandle = 0;
                requestPending = false;
                emit notifier->CBManagerError(qtUuid, QLowEnergyService::CharacteristicReadError);
                [self handleReadWriteError:error];
                [self performNextRequest];
            }
            return;
        }
    }

    if (isDetailsDiscovery) {
        if (readMatch) {
            // Test if we have any other characteristic to read yet.
            CBCharacteristic *const next = [self nextCharacteristicForService:characteristic.service
                                                 startingFrom:characteristic properties:CBCharacteristicPropertyRead];
            if (next) {
                [self watchAfter:next timeout:OperationTimeout::characteristicRead];
                [peripheral readValueForCharacteristic:next];
            } else {
                [self discoverDescriptors:characteristic.service];
            }
        }
    } else {
        // This is (probably) the result of update notification.
        // It's very possible we can have an invalid handle here (0) -
        // if something esle is wrong (we subscribed for a notification),
        // disconnected (but other application is connected) and still receiveing
        // updated values ...
        // TODO: this must be properly tested.
        if (!chHandle) {
            qCCritical(QT_BT_OSX) << "unexpected update notification, "
                                     "no characteristic handle found";
            return;
        }

        if (currentReadHandle == chHandle) {
            // Even if it was not a reply to our read request (no way to test it)
            // report it.
            requestPending = false;
            currentReadHandle = 0;
            //
            emit notifier->characteristicRead(chHandle, qt_bytearray(characteristic.value));
            [self performNextRequest];
        } else {
            emit notifier->characteristicUpdated(chHandle, qt_bytearray(characteristic.value));
        }
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didDiscoverDescriptorsForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    // This method does not change 'managerState', we can
    // have several discoveries active at the same time.
    Q_UNUSED(aPeripheral)

    if (!notifier) {
        // Detached, no need to continue ...
        return;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    using namespace OSXBluetooth;

    if (![self objectIsUnderWatch:characteristic operation:OperationTimeout::descriptorsDiscovery])
        return;

    [self stopWatchingAfter:characteristic operation:OperationTimeout::descriptorsDiscovery];

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);
        // We can continue though ...
    }

    // Do we have more characteristics on this service to discover descriptors?
    CBCharacteristic *const next = [self nextCharacteristicForService:characteristic.service
                                         startingFrom:characteristic];
    if (next) {
        [self watchAfter:next timeout:OperationTimeout::descriptorsDiscovery];
        [peripheral discoverDescriptorsForCharacteristic:next];
    } else {
        [self readDescriptors:characteristic.service];
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didUpdateValueForDescriptor:(CBDescriptor *)descriptor
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    Q_ASSERT_X(peripheral, Q_FUNC_INFO, "invalid peripheral (nil)");

    if (!notifier) {
        // Detached ...
        return;
    }

    QT_BT_MAC_AUTORELEASEPOOL;

    using namespace OSXBluetooth;

    if (![self objectIsUnderWatch:descriptor operation:OperationTimeout::descriptorRead])
        return;

    [self stopWatchingAfter:descriptor operation:OperationTimeout::descriptorRead];

    CBService *const service = descriptor.characteristic.service;
    const QBluetoothUuid qtUuid(qt_uuid(service.UUID));
    const bool isDetailsDiscovery = servicesToDiscoverDetails.contains(qtUuid);
    const QLowEnergyHandle dHandle = descMap.key(descriptor);

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);

        if (!isDetailsDiscovery) {
            if (dHandle && dHandle == currentReadHandle) {
                currentReadHandle = 0;
                requestPending = false;
                emit notifier->CBManagerError(qtUuid, QLowEnergyService::DescriptorReadError);
                [self handleReadWriteError:error];
                [self performNextRequest];
            }
            return;
        }
    }

    if (isDetailsDiscovery) {
        // Test if we have any other characteristic to read yet.
        CBDescriptor *const next = [self nextDescriptorForCharacteristic:descriptor.characteristic
                                         startingFrom:descriptor];
        if (next) {
            [self watchAfter:next timeout:OperationTimeout::descriptorRead];
            [peripheral readValueForDescriptor:next];
        } else {
            // We either have to read a value for a next descriptor
            // on a given characteristic, or continue with the
            // next characteristic in a given service (if any).
            CBCharacteristic *const ch = descriptor.characteristic;
            CBCharacteristic *nextCh = [self nextCharacteristicForService:ch.service
                                             startingFrom:ch];
            while (nextCh) {
                if (nextCh.descriptors && nextCh.descriptors.count) {
                    CBDescriptor *desc = [nextCh.descriptors objectAtIndex:0];
                    [self watchAfter:desc timeout:OperationTimeout::descriptorRead];
                    return [peripheral readValueForDescriptor:desc];
                }

                nextCh = [self nextCharacteristicForService:ch.service
                               startingFrom:nextCh];
            }

            [self serviceDetailsDiscoveryFinished:service];
        }
    } else {
        if (!dHandle) {
            qCCritical(QT_BT_OSX) << "unexpected value update notification, "
                                     "no descriptor handle found";
            return;
        }

        if (dHandle == currentReadHandle) {
            currentReadHandle = 0;
            requestPending = false;
            emit notifier->descriptorRead(dHandle, qt_bytearray(static_cast<NSObject *>(descriptor.value)));
            [self performNextRequest];
        }
    }
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didWriteValueForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)
    Q_UNUSED(characteristic)

    if (!notifier) {
        // Detached.
        return;
    }

    // From docs:
    //
    // "This method is invoked only when your app calls the writeValue:forCharacteristic:type:
    //  method with the CBCharacteristicWriteWithResponse constant specified as the write type.
    //  If successful, the error parameter is nil. If unsuccessful,
    //  the error parameter returns the cause of the failure."

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    if (![self objectIsUnderWatch:characteristic operation:OperationTimeout::characteristicWrite])
        return;

    [self stopWatchingAfter:characteristic operation:OperationTimeout::characteristicWrite];
    requestPending = false;

    // Error or not, but the cached value has to be deleted ...
    const QByteArray valueToReport(valuesToWrite.value(characteristic, QByteArray()));
    if (!valuesToWrite.remove(characteristic)) {
        qCWarning(QT_BT_OSX) << "no updated value found "
                                "for characteristic";
    }

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);
        emit notifier->CBManagerError(qt_uuid(characteristic.service.UUID),
                                             QLowEnergyService::CharacteristicWriteError);
        [self handleReadWriteError:error];
    } else {
        const QLowEnergyHandle cHandle = charMap.key(characteristic);
        emit notifier->characteristicWritten(cHandle, valueToReport);
    }

    [self performNextRequest];
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didWriteValueForDescriptor:(CBDescriptor *)descriptor
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    if (!notifier) {
        // Detached already.
        return;
    }

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    requestPending = false;

    // Error or not, a value (if any) must be removed.
    const QByteArray valueToReport(valuesToWrite.value(descriptor, QByteArray()));
    if (!valuesToWrite.remove(descriptor))
        qCWarning(QT_BT_OSX) << "no updated value found";

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);
        emit notifier->CBManagerError(qt_uuid(descriptor.characteristic.service.UUID),
                       QLowEnergyService::DescriptorWriteError);
        [self handleReadWriteError:error];
    } else {
        const QLowEnergyHandle dHandle = descMap.key(descriptor);
        Q_ASSERT_X(dHandle, Q_FUNC_INFO, "descriptor not found in the descriptors map");
        emit notifier->descriptorWritten(dHandle, valueToReport);
    }

    [self performNextRequest];
}

- (void)peripheral:(CBPeripheral *)aPeripheral
        didUpdateNotificationStateForCharacteristic:(CBCharacteristic *)characteristic
        error:(NSError *)error
{
    Q_UNUSED(aPeripheral)

    if (!notifier)
        return;

    using namespace OSXBluetooth;

    QT_BT_MAC_AUTORELEASEPOOL;

    requestPending = false;

    const QBluetoothUuid qtUuid(QBluetoothUuid::ClientCharacteristicConfiguration);
    CBDescriptor *const descriptor = [self descriptor:qtUuid forCharacteristic:characteristic];
    const QByteArray valueToReport(valuesToWrite.value(descriptor, QByteArray()));
    const int nRemoved = valuesToWrite.remove(descriptor);

    if (error) {
        NSLog(@"%s failed with error %@", Q_FUNC_INFO, error);
        // In Qt's API it's a descriptor write actually.
        emit notifier->CBManagerError(qt_uuid(characteristic.service.UUID),
                                             QLowEnergyService::DescriptorWriteError);
    } else if (nRemoved) {
        const QLowEnergyHandle dHandle = descMap.key(descriptor);
        emit notifier->descriptorWritten(dHandle, valueToReport);
    }

    [self performNextRequest];
}

- (void)detach
{
    if (notifier) {
        notifier->disconnect();
        notifier->deleteLater();
        notifier = nullptr;
    }

    [self stopWatchers];
    [self disconnectFromDevice];
}

@end
