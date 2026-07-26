#ifndef OBCLISTENER_H
#define OBCLISTENER_H

#include <QBluetoothDeviceInfo>
#include <QBluetoothUuid>
#include <QLowEnergyController>
#include <QLowEnergyService>
#include <QObject>
#include <QString>

// OpenBikeControl (OBC) LISTENER — qz as a BLE **central** that connects to an OBC controller (e.g. an
// ESP32 button box), subscribes to its Button-State notify characteristic, decodes the messages, and
// dispatches each button to a configurable qz action. This is the consumer side of the OBC work whose
// producer is #4504 (MyWhooshLink) — see qz issue #4791. Modeled on the zwiftclickremote BLE-central.
//
// Platform note: this uses QtBluetooth's central role, which works on **Android + desktop (BlueZ)**.
// It compiles on every platform, but on **iOS** qz drives BLE through a native Swift wrapper rather than
// QtBluetooth, so a native central bridge (like zwiftclickremote's `iOS_zwiftClickRemote`) is a follow-up
// — treat iOS as unsupported for now.
//
// Wire format (controller side, firmware/lib/proxy/Obc.h): Button-State = [0x01, id, state, id, state, …];
// state 0x00 released / 0x01 pressed / 0x02-0xFF analog. Per-button action is configurable via QSettings
// keys `obc_button_<id>` (tokens: gear_up/down, power_up/down, offset_up/down, resistance_up/down,
// zone_up/down, lap, start_stop, none), dispatched through homeform's keyboard action path.
class obclistener : public QObject {
    Q_OBJECT
  public:
    explicit obclistener(QObject *parent = nullptr);

    // Connect to a discovered OBC controller and start service discovery (called from bluetooth.cpp when
    // an advertiser exposes the OBC service UUID).
    void deviceDiscovered(const QBluetoothDeviceInfo &device);
    bool connected() const { return m_connected; }

    static const QString OBC_SERVICE_UUID;  // d273f680-d548-419d-b9d1-fa0472345229
    static const QString OBC_BUTTON_UUID;   // d273f681-… (Button-State, Read/Notify)

  signals:
    void debug(const QString &s);

  private slots:
    void serviceScanDone();
    void stateChanged(QLowEnergyService::ServiceState state);
    void characteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value);
    void controllerConnected();
    void controllerDisconnected();

  private:
    void handleButton(uint8_t id, uint8_t state);  // edge-detect (act on press) + config lookup
    void obcDoAction(const QString &action);        // dispatch an action token via homeform

    QLowEnergyController *m_control = nullptr;
    QList<QLowEnergyService *> m_services;
    bool m_connected = false;
    uint8_t m_lastState[256] = {0};  // per-button-id last pressed-state, for rising-edge detection
};

#endif  // OBCLISTENER_H
