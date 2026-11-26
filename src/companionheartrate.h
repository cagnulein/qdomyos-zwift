#pragma once

#include <QString>
#include <QStringList>

// A deliberately selected BLE heart-rate belt owns the HR metric. The
// network companion may belong to another athlete on the same network;
// do not let it overwrite the belt even while the belt is connecting.
template <typename Receiver>
void dispatchCompanionHeartRate(const QString &payload, const QString &beltName, Receiver receive) {
    if (!beltName.isEmpty() && !beltName.startsWith(QStringLiteral("Disabled")))
        return;

    // Preserve the existing companion parsing and delivery when no belt is selected.
    const QStringList fields = payload.split(QStringLiteral("#"));
    for (const QString &field : fields) {
        if (field.contains(QStringLiteral("HR"))) {
            const QStringList values = field.split(QStringLiteral("="));
            if (values.length() > 1)
                receive(values[1].toDouble());
        }
    }
}
