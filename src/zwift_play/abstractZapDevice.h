#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include "localKeyProvider.h"
#include "zapCrypto.h"
#include "zapConstants.h"

class AbstractZapDevice: public QObject {
    Q_OBJECT
public:
    QByteArray RIDE_ON;
    QByteArray REQUEST_START;
    QByteArray RESPONSE_START;

    ZapCrypto zapEncryption;
    AbstractZapDevice() : localKeyProvider(), zapEncryption(localKeyProvider) {
        RIDE_ON = QByteArray::fromRawData("\x52\x69\x64\x65\x4F\x6E", 6);  // "RideOn"
        REQUEST_START = QByteArray::fromRawData("\x00\x09", 2);  // {0, 9}
        RESPONSE_START = QByteArray::fromRawData("\x01\x03", 2);  // {1, 3}
    }

    void processCharacteristic(const QString& characteristicName, const QByteArray& bytes) {
        if (bytes.isEmpty()) return;

        qDebug() << characteristicName << bytes.toHex();

        if (bytes.startsWith(RIDE_ON + RESPONSE_START)) {
            processDevicePublicKeyResponse(bytes);
        } else if (bytes.size() > static_cast<int>(sizeof(int)) + EncryptionUtils::MAC_LENGTH) {
            processEncryptedData(bytes);
        } else {
            qDebug() << "Unprocessed - Data Type:" << bytes.toHex();
        }
    }

    QByteArray buildHandshakeStart() {
        return RIDE_ON + REQUEST_START + localKeyProvider.getPublicKeyBytes();
    }

protected:
    virtual void processEncryptedData(const QByteArray& bytes) = 0;

private:
    QByteArray devicePublicKeyBytes;
    LocalKeyProvider localKeyProvider;    

    void processDevicePublicKeyResponse(const QByteArray& bytes) {
        devicePublicKeyBytes = bytes.mid(RIDE_ON.size() + RESPONSE_START.size());
        zapEncryption.initialise(devicePublicKeyBytes);
        qDebug() << "Device Public Key -" << devicePublicKeyBytes.toHex();
    }
};

#endif // ABSTRACTZAPDEVICE_H
