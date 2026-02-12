#ifndef BLUETOOTHSIGNALRECEIVER_H
#define BLUETOOTHSIGNALRECEIVER_H

#include <QObject>
#include "devices/bluetoothdevice.h"
#include "bluetooth.h"

/**
 * @brief Catches bluetoothdevice connection signals from a bluetooth object.
 */
class BluetoothSignalReceiver : public QObject
{
    Q_OBJECT

 private:
    bluetoothdevice* device = nullptr;


public:
    explicit BluetoothSignalReceiver(bluetooth& b, QObject *parent = nullptr);
    ~BluetoothSignalReceiver();

    bluetoothdevice * get_device() const;

public Q_SLOTS:
    void bluetoothDeviceConnected(bluetoothdevice *b);
    void bluetoothDeviceDisconnected();
};

#endif // BLUETOOTHSIGNALRECEIVER_H
