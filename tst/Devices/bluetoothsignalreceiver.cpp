#include "bluetoothsignalreceiver.h"


BluetoothSignalReceiver::BluetoothSignalReceiver(bluetooth &b, QObject *parent) : QObject(parent) {
    connect(&b, &bluetooth::bluetoothDeviceConnected, this, &BluetoothSignalReceiver::bluetoothDeviceConnected);
    connect(&b, &bluetooth::bluetoothDeviceDisconnected, this, &BluetoothSignalReceiver::bluetoothDeviceDisconnected);
}

BluetoothSignalReceiver::~BluetoothSignalReceiver() {}

bluetoothdevice *BluetoothSignalReceiver::get_device() const { return this->device; }

void BluetoothSignalReceiver::bluetoothDeviceConnected(bluetoothdevice *b) { this->device = b;}

void BluetoothSignalReceiver::bluetoothDeviceDisconnected() { this->device = nullptr; }

