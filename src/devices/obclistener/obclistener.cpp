#include "devices/obclistener/obclistener.h"

#include <QLowEnergyDescriptor>
#include <QSettings>

#include "homeform.h"

// OBC BLE UUIDs (controller side; the same constants live in our firmware's lib/proxy/Obc.h).
const QString obclistener::OBC_SERVICE_UUID = QStringLiteral("d273f680-d548-419d-b9d1-fa0472345229");
const QString obclistener::OBC_BUTTON_UUID = QStringLiteral("d273f681-d548-419d-b9d1-fa0472345229");

obclistener::obclistener(QObject *parent) : QObject(parent) {}

void obclistener::deviceDiscovered(const QBluetoothDeviceInfo &device) {
    emit debug(QStringLiteral("obclistener: connecting to OBC controller ") + device.name());
    m_control = QLowEnergyController::createCentral(device, this);
    connect(m_control, &QLowEnergyController::connected, this, &obclistener::controllerConnected);
    connect(m_control, &QLowEnergyController::disconnected, this, &obclistener::controllerDisconnected);
    connect(m_control, &QLowEnergyController::discoveryFinished, this, &obclistener::serviceScanDone);
    connect(m_control,
            static_cast<void (QLowEnergyController::*)(QLowEnergyController::Error)>(&QLowEnergyController::error),
            this, [this](QLowEnergyController::Error e) {
                emit debug(QStringLiteral("obclistener: controller error ") + QString::number((int)e));
            });
    m_control->connectToDevice();
}

void obclistener::controllerConnected() {
    m_connected = true;
    emit debug(QStringLiteral("obclistener: connected, discovering services"));
    m_control->discoverServices();
}

void obclistener::controllerDisconnected() {
    m_connected = false;
    m_services.clear();
    emit debug(QStringLiteral("obclistener: disconnected"));
}

void obclistener::serviceScanDone() {
    const QBluetoothUuid svc(OBC_SERVICE_UUID);
    for (const QBluetoothUuid &u : m_control->services()) {
        if (u == svc) {
            QLowEnergyService *s = m_control->createServiceObject(u, this);
            if (s) {
                connect(s, &QLowEnergyService::stateChanged, this, &obclistener::stateChanged);
                connect(s, &QLowEnergyService::characteristicChanged, this, &obclistener::characteristicChanged);
                m_services.append(s);
                s->discoverDetails();
            }
        }
    }
}

void obclistener::stateChanged(QLowEnergyService::ServiceState state) {
    if (state != QLowEnergyService::ServiceDiscovered)
        return;
    QLowEnergyService *s = qobject_cast<QLowEnergyService *>(sender());
    if (!s)
        return;
    const QBluetoothUuid btn(OBC_BUTTON_UUID);
    for (const QLowEnergyCharacteristic &c : s->characteristics()) {
        if (c.uuid() == btn && (c.properties() & QLowEnergyCharacteristic::Notify)) {
            QByteArray cccd;
            cccd.append((char)0x01);
            cccd.append((char)0x00);
            // Seed the edge detector from the characteristic's CURRENT value before subscribing.
            // Button-State is Read/Notify and retains the last state, so service discovery hands us
            // whatever was last pressed — possibly minutes ago. Without this we would see a 0->pressed
            // edge and fire that stale press as if it had just happened: connecting to a controller
            // whose last press was "ERG down" silently dropped the rider's target power. Observed on
            // hardware. Seeding makes the first REAL press the first edge we act on.
            const QByteArray retained = c.value();
            if (retained.size() >= 1 && (uint8_t)retained.at(0) == 0x01) {
                for (int i = 1; i + 1 < retained.size(); i += 2)
                    m_lastState[(uint8_t)retained.at(i)] = ((uint8_t)retained.at(i + 1) != 0x00) ? 1 : 0;
            }
            const QLowEnergyDescriptor d = c.descriptor(QBluetoothUuid::ClientCharacteristicConfiguration);
            if (d.isValid())
                s->writeDescriptor(d, cccd);
            emit debug(QStringLiteral("obclistener: subscribed to OBC Button-State"));
        }
    }
}

void obclistener::characteristicChanged(const QLowEnergyCharacteristic &c, const QByteArray &value) {
    Q_UNUSED(c);
    if (value.size() < 1 || (uint8_t)value.at(0) != 0x01)  // 0x01 = Button-State message type
        return;
    for (int i = 1; i + 1 < value.size(); i += 2)
        handleButton((uint8_t)value.at(i), (uint8_t)value.at(i + 1));
}

// Default action for an OBC standard button id (overridable per id via QSettings `obc_button_<id>`).
// The OBC ids are already semantic (Obc.h / the spec) so the listener works out of the box.
static QString defaultActionForObcId(uint8_t id) {
    switch (id) {
    case 0x01: return QStringLiteral("gear_up");    // OBC Shift Up
    case 0x02: return QStringLiteral("gear_down");  // OBC Shift Down
    case 0x30: return QStringLiteral("power_up");   // OBC ERG Up (increase difficulty)
    case 0x31: return QStringLiteral("power_down"); // OBC ERG Down
    case 0x35: return QStringLiteral("lap");        // OBC Lap
    default: return QStringLiteral("none");
    }
}

void obclistener::handleButton(uint8_t id, uint8_t state) {
    const bool pressed = (state != 0x00);
    if ((m_lastState[id] != 0) == pressed)  // no edge — a repeat of the same state
        return;
    m_lastState[id] = pressed ? 1 : 0;
    if (!pressed)  // act on press, not release
        return;
    QSettings settings;
    QString action = settings.value(QStringLiteral("obc_button_%1").arg(id), QString()).toString();
    if (action.isEmpty())
        action = defaultActionForObcId(id);
    emit debug(QStringLiteral("obclistener: button %1 -> %2").arg(id).arg(action));
    obcDoAction(action);
}

// Dispatch an action token to a qz action, all via homeform's keyboard action path (which owns the
// current device: gears go to the bike, target_power/peloton_offset/etc. to the workout/erg surfaces).
void obclistener::obcDoAction(const QString &action) {
    if (action.isEmpty() || action == QStringLiteral("none") || !homeform::singleton())
        return;
    homeform *h = homeform::singleton();
    if (action == QStringLiteral("gear_up"))
        h->keyboardPlus(QStringLiteral("gears"));
    else if (action == QStringLiteral("gear_down"))
        h->keyboardMinus(QStringLiteral("gears"));
    else if (action == QStringLiteral("power_up"))
        h->keyboardPlus(QStringLiteral("target_power"));
    else if (action == QStringLiteral("power_down"))
        h->keyboardMinus(QStringLiteral("target_power"));
    else if (action == QStringLiteral("offset_up"))
        h->keyboardPlus(QStringLiteral("peloton_offset"));
    else if (action == QStringLiteral("offset_down"))
        h->keyboardMinus(QStringLiteral("peloton_offset"));
    else if (action == QStringLiteral("resistance_up"))
        h->keyboardPlus(QStringLiteral("resistance"));
    else if (action == QStringLiteral("resistance_down"))
        h->keyboardMinus(QStringLiteral("resistance"));
    else if (action == QStringLiteral("zone_up"))
        h->keyboardPlus(QStringLiteral("target_zone"));
    else if (action == QStringLiteral("zone_down"))
        h->keyboardMinus(QStringLiteral("target_zone"));
    else if (action == QStringLiteral("lap"))
        h->keyboardLap();
    else if (action == QStringLiteral("start_stop"))
        h->keyboardStartStop();
}
