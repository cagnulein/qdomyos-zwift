// Host-side smoke test: compile with Qt Core and the rcc-generated source.
// No hardware or Bluetooth connection is needed.
#include <QCoreApplication>
#include <QCryptographicHash>
#include <QFile>
#include <QDebug>

int main(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    QFile table(QStringLiteral(":/kettlerc12/SecureData.txt"));
    if (!table.open(QIODevice::ReadOnly)) {
        qCritical() << "C12 Qt resource unavailable" << table.errorString();
        return 1;
    }
    const QByteArray expected =
        QByteArrayLiteral("70d3eeec6c510e132383fa69b2466e28dec0c89065ab0481a33d191e6281b3a3");
    if (QCryptographicHash::hash(table.readAll(), QCryptographicHash::Sha256).toHex() != expected) {
        qCritical() << "C12 Qt resource hash mismatch";
        return 1;
    }
    return 0;
}
