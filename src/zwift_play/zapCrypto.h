#ifndef ZAPCRYPTO_H
#define ZAPCRYPTO_H

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <QByteArray>
#include <vector>
#include <cassert>
#include "localKeyProvider.h"

class ZapCrypto {
  public:
    ZapCrypto(LocalKeyProvider &localKeyProvider)
        : localKeyProvider(localKeyProvider), counter(0) {
        // Initialize any necessary components
    }

    ~ZapCrypto() {
        // Clean up if necessary
    }

    void initialise(const QByteArray &devicePublicKeyBytes) {
        QByteArray hkdfBytes = generateHmacKeyDerivationFunctionBytes(devicePublicKeyBytes);
        encryptionKeyBytes = hkdfBytes.mid(0, EncryptionUtils::KEY_LENGTH);
        ivBytes = hkdfBytes.mid(32, EncryptionUtils::HKDF_LENGTH);
    }

    QByteArray encrypt(const QByteArray &data) {
        assert(!encryptionKeyBytes.isEmpty() && !ivBytes.isEmpty());

        QByteArray nonceBytes = createNonce(ivBytes, counter);
        QByteArray encrypted = encryptDecrypt(true, nonceBytes, data);

        counter++;
        return createCounterBytes(counter - 1) + encrypted;
    }

    QByteArray decrypt(const QByteArray &counterArray, const QByteArray &payload) {
        assert(!encryptionKeyBytes.isEmpty() && !ivBytes.isEmpty());

        QByteArray nonceBytes = ivBytes + counterArray;
        return encryptDecrypt(false, nonceBytes, payload);
    }

  private:
    LocalKeyProvider &localKeyProvider;
    QByteArray encryptionKeyBytes;
    QByteArray ivBytes;
    int counter;

    QByteArray encryptDecrypt(bool encrypt, const QByteArray &nonceBytes, const QByteArray &data) {
        // Implement AES encryption/decryption using OpenSSL here.
        // This is a non-trivial task and requires a good understanding of OpenSSL's EVP API.
        // You will need to setup the EVP_CIPHER_CTX, handle initialization, update, and finalization.
        // OpenSSL's AES CCM mode support will be used here.

               // Placeholder for the encrypted/decrypted data
        return QByteArray();
    }

    QByteArray generateHmacKeyDerivationFunctionBytes(const QByteArray &devicePublicKeyBytes) {
        // Generate HKDF bytes using OpenSSL.
        // Placeholder for HKDF bytes.
        return QByteArray();
    }

    QByteArray createNonce(const QByteArray &iv, int messageCounter) {
        return iv + createCounterBytes(messageCounter);
    }

    static QByteArray createCounterBytes(int messageCounter) {
        QByteArray counterBytes;
        counterBytes.setNum(messageCounter, 16); // Convert int to hexadecimal string.
        return counterBytes;
    }
};

#endif // ZAPCRYPTO_H
