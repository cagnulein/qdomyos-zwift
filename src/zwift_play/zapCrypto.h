#ifndef ZAPCRYPTO_H
#define ZAPCRYPTO_H


#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/hmac.h>
#include <openssl/err.h>
#include <QByteArray>
#include <QtEndian>
#include <cassert>
#include "localKeyProvider.h"

class ZapCrypto {
public:
    ZapCrypto(LocalKeyProvider &localKeyProvider)
        : localKeyProvider(localKeyProvider), counter(0) {
        // Initialization code if needed
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

        if (counterArray.size() != static_cast<int>(sizeof(qint32)) || payload.size() <= EncryptionUtils::MAC_LENGTH) {
            return QByteArray();
        }

        QByteArray nonceBytes = ivBytes + counterArray;
        return encryptDecrypt(false, nonceBytes, payload);
    }

private:
    LocalKeyProvider &localKeyProvider;
    QByteArray encryptionKeyBytes;
    QByteArray ivBytes;
    int counter;

    QByteArray encryptDecrypt(bool encrypt, const QByteArray &nonceBytes, const QByteArray &data) {
        const int payloadLength = encrypt ? data.size() : data.size() - EncryptionUtils::MAC_LENGTH;
        if (payloadLength < 0) {
            return QByteArray();
        }

        QByteArray cipherText = encrypt ? data : data.left(payloadLength);
        QByteArray tagBytes = encrypt ? QByteArray() : data.mid(payloadLength, EncryptionUtils::MAC_LENGTH);
        QByteArray output(payloadLength, 0);
        EVP_CIPHER_CTX *ctx = EVP_CIPHER_CTX_new();
        if (!ctx) {
            return QByteArray();
        }

        EVP_CipherInit_ex(ctx, EVP_aes_256_ccm(), NULL, NULL, NULL, encrypt);
        EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_IVLEN, nonceBytes.size(), NULL);

        if (!encrypt) {
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_SET_TAG, EncryptionUtils::MAC_LENGTH,
                                const_cast<char *>(tagBytes.constData()));
        }

        EVP_CipherInit_ex(ctx, NULL, NULL,
                          reinterpret_cast<const unsigned char *>(encryptionKeyBytes.constData()),
                          reinterpret_cast<const unsigned char *>(nonceBytes.constData()), encrypt);

        int outlen = 0;
        if (EVP_CipherUpdate(ctx, NULL, &outlen, NULL, cipherText.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return QByteArray();
        }

        if (EVP_CipherUpdate(ctx, reinterpret_cast<unsigned char *>(output.data()), &outlen,
                             reinterpret_cast<const unsigned char *>(cipherText.constData()),
                             cipherText.size()) != 1) {
            EVP_CIPHER_CTX_free(ctx);
            return QByteArray();
        }

        if (EVP_CipherFinal_ex(ctx, reinterpret_cast<unsigned char *>(output.data()) + outlen, &outlen) <= 0) {
            EVP_CIPHER_CTX_free(ctx);
            return QByteArray();
        }

        if (encrypt) {
            unsigned char tag[EncryptionUtils::MAC_LENGTH];
            EVP_CIPHER_CTX_ctrl(ctx, EVP_CTRL_CCM_GET_TAG, EncryptionUtils::MAC_LENGTH, tag);
            output.append(reinterpret_cast<char *>(tag), EncryptionUtils::MAC_LENGTH);
        }

        EVP_CIPHER_CTX_free(ctx);
        return output;
    }


    QByteArray generateHmacKeyDerivationFunctionBytes(const QByteArray& devicePublicKeyBytes) {
        qDebug() << devicePublicKeyBytes.toHex(' ');
        EC_KEY* localPublicKey = localKeyProvider.getPublicKey();
        const EC_GROUP* group = EC_KEY_get0_group(localPublicKey);

        // Generating the server's public key as an EC_KEY*
        EC_KEY* serverPublicKeyEC = EncryptionUtils::generatePublicKey(devicePublicKeyBytes, group);

        // Converting EC_KEY* to EVP_PKEY* for server's public key
        EVP_PKEY* serverPublicKey = EVP_PKEY_new();
        if (!EVP_PKEY_assign_EC_KEY(serverPublicKey, serverPublicKeyEC)) {
            // Handle error: failed to assign EC_KEY to EVP_PKEY
            EVP_PKEY_free(serverPublicKey);
            EC_KEY_free(serverPublicKeyEC);
            return QByteArray();
        }

        // Now, use EVP_PKEY* for shared secret generation
        QByteArray sharedSecretBytes = EncryptionUtils::generateSharedSecretBytes(localKeyProvider.getPrivateKey(), serverPublicKey);
        qDebug() << "sharedSecretBytes" << sharedSecretBytes.toHex(' ');;
        QByteArray salt = EncryptionUtils::publicKeyToByteArray(serverPublicKeyEC) + localKeyProvider.getPublicKeyBytes();
        qDebug() << "salt" << salt.toHex(' ');;
        QByteArray hkdfOutput = hkdf(sharedSecretBytes, salt, QByteArray(), EncryptionUtils::HKDF_LENGTH);
        qDebug() << "hkdfOutput" << hkdfOutput.toHex(' ');

        EVP_PKEY_free(serverPublicKey); // This will also free serverPublicKeyEC
        return hkdfOutput;
    }




    QByteArray createNonce(const QByteArray &iv, int messageCounter) {
        return iv + createCounterBytes(messageCounter);
    }

    static QByteArray createCounterBytes(int messageCounter) {
        QByteArray counterBytes(sizeof(qint32), 0);
        qToBigEndian(static_cast<qint32>(messageCounter), reinterpret_cast<uchar *>(counterBytes.data()));
        return counterBytes;
    }

    QByteArray hkdf(const QByteArray& ikm, const QByteArray& salt, const QByteArray& info, int outputLength) {
        unsigned char prk[EVP_MAX_MD_SIZE];
        unsigned int prk_len;

        // HKDF-Extract
        HMAC(EVP_sha256(), salt.constData(), salt.length(),
             reinterpret_cast<const unsigned char*>(ikm.constData()), ikm.length(),
             prk, &prk_len);

        QByteArray result;
        QByteArray T;
        int n = (outputLength + EVP_MD_size(EVP_sha256()) - 1) / EVP_MD_size(EVP_sha256());
        QByteArray current;

        // HKDF-Expand
        for (int i = 1; i <= n; i++) {
            current = T + info + static_cast<char>(i);
            T.resize(EVP_MD_size(EVP_sha256()));

            HMAC(EVP_sha256(), prk, prk_len,
                 reinterpret_cast<const unsigned char*>(current.constData()), current.length(),
                 reinterpret_cast<unsigned char*>(T.data()), NULL);

            result.append(T);
        }

        return result.left(outputLength);
    }
};

#endif // ZAPCRYPTO_H
