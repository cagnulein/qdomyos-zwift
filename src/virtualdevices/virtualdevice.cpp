#include "virtualdevice.h"
#include <QBluetoothLocalDevice>
#include <QDebug>

virtualdevice::virtualdevice(QObject *parent)
    : QObject{parent}
{

}

virtualdevice::~virtualdevice() {
    qDebug() << "Deleting virtual device";
}

// Without an adapter (Android emulators, Waydroid) the GATT server cannot open and Android aborts QZ,
// so a virtual device then runs as with virtual_device_bluetooth off. Same check as in bluetooth.cpp
bool virtualdevice::noBluetoothAdapter() {
#if !defined(WIN32) && !defined(Q_OS_IOS)
    return QBluetoothLocalDevice::allDevices().isEmpty();
#else
    return false;
#endif
}
