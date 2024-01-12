#ifndef ABSTRACTZAPDEVICE_H
#define ABSTRACTZAPDEVICE_H

#include <QByteArray>
#include <QString>
#include <QDebug>
#include "localKeyProvider.h"
#include "zapCrypto.h"
#include "zapConstants.h"

class AbstractZapDevice {
public:
    ZapCrypto zapEncryption;
    AbstractZapDevice() : localKeyProvider(), zapEncryption(localKeyProvider) {}

    void processCharacteristic(const QString& characteristicName, const QByteArray& bytes) {
        if (bytes.isEmpty()) return;

        qDebug() << characteristicName << bytes.toHex();

        if (bytes.startsWith(ZapConstants::RIDE_ON + ZapConstants::RESPONSE_START)) {
            processDevicePublicKeyResponse(bytes);
        } else if (bytes.size() > static_cast<int>(sizeof(int)) + EncryptionUtils::MAC_LENGTH) {
            processEncryptedData(bytes);
        } else {
            qDebug() << "Unprocessed - Data Type:" << bytes.toHex();
        }
    }

    QByteArray buildHandshakeStart() {
        return ZapConstants::RIDE_ON + ZapConstants::REQUEST_START + localKeyProvider.getPublicKeyBytes();
    }

protected:
    virtual void processEncryptedData(const QByteArray& bytes) = 0;

private:
    QByteArray devicePublicKeyBytes;
    LocalKeyProvider localKeyProvider;    

    void processDevicePublicKeyResponse(const QByteArray& bytes) {
        devicePublicKeyBytes = bytes.mid(ZapConstants::RIDE_ON.size() + ZapConstants::RESPONSE_START.size());
        zapEncryption.initialise(devicePublicKeyBytes);
        qDebug() << "Device Public Key -" << devicePublicKeyBytes.toHex();
    }
};

#endif // ABSTRACTZAPDEVICE_H
