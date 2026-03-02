#include <QByteArray>
#include <openssl/evp.h>
#include <openssl/ec.h>
#include <openssl/hmac.h>
#include <openssl/sha.h>
#include <iostream>

class EncryptionUtils {
  public:
    static const int KEY_LENGTH = 32;
    static const int HKDF_LENGTH = 36;
    static const int MAC_LENGTH = 4;

    static QByteArray publicKeyToByteArray(EC_KEY* eCPublicKey) {
        const EC_POINT *point = EC_KEY_get0_public_key(eCPublicKey);
        const EC_GROUP *group = EC_KEY_get0_group(eCPublicKey);
        BIGNUM *x = BN_new();
        BIGNUM *y = BN_new();
        QByteArray result;

        if (EC_POINT_get_affine_coordinates_GFp(group, point, x, y, nullptr)) {
            int sizeX = BN_num_bytes(x);
            int sizeY = BN_num_bytes(y);
            result.resize(sizeX + sizeY);
            BN_bn2bin(x, reinterpret_cast<unsigned char *>(result.data()));
            BN_bn2bin(y, reinterpret_cast<unsigned char *>(result.data()) + sizeX);
        }

        BN_free(x);
        BN_free(y);
        return result;
    }

    static EC_KEY* generatePublicKey(const QByteArray& publicKeyBytes, const EC_GROUP* paramSpec) {
        EC_KEY *key = EC_KEY_new();
        EC_KEY_set_group(key, paramSpec);
        EC_POINT *point = EC_POINT_new(paramSpec);

        BIGNUM *x = BN_bin2bn(reinterpret_cast<const unsigned char *>(publicKeyBytes.data()), publicKeyBytes.size() / 2, nullptr);
        BIGNUM *y = BN_bin2bn(reinterpret_cast<const unsigned char *>(publicKeyBytes.data() + publicKeyBytes.size() / 2), publicKeyBytes.size() / 2, nullptr);

        EC_POINT_set_affine_coordinates_GFp(paramSpec, point, x, y, nullptr);
        EC_KEY_set_public_key(key, point);

        EC_POINT_free(point);
        BN_free(x);
        BN_free(y);
        return key;
    }

    static QByteArray generateSharedSecretBytes(EVP_PKEY* privateKey, EVP_PKEY* serverPublicKey) {
        QByteArray result;
        EVP_PKEY_CTX *ctx = EVP_PKEY_CTX_new(privateKey, nullptr);

        if (EVP_PKEY_derive_init(ctx) <= 0) {
            // Handle error
            qDebug() << "error";
            return "";
        }
        if (EVP_PKEY_derive_set_peer(ctx, serverPublicKey) <= 0) {
            // Handle error
            qDebug() << "error";
            return "";
        }
        size_t secret_len;
        if (EVP_PKEY_derive(ctx, nullptr, &secret_len) <= 0) {
            // Handle error
            qDebug() << "error";
            return "";
        }
        result.resize(secret_len);
        if (EVP_PKEY_derive(ctx, reinterpret_cast<unsigned char *>(result.data()), &secret_len) <= 0) {
            // Handle error
            qDebug() << "error";
            return "";
        }

        EVP_PKEY_CTX_free(ctx);
        return result;
    }


    QByteArray generateHKDFBytes(const QByteArray& secretKey, const QByteArray& salt) {
        unsigned char prk[EVP_MAX_MD_SIZE];
        unsigned int prk_len;

        // HKDF-Extract
        HMAC(EVP_sha256(), reinterpret_cast<const unsigned char*>(salt.constData()), salt.length(),
             reinterpret_cast<const unsigned char*>(secretKey.constData()), secretKey.length(),
             prk, &prk_len);

        QByteArray result;
        QByteArray T;
        int n = (EncryptionUtils::HKDF_LENGTH + EVP_MD_size(EVP_sha256()) - 1) / EVP_MD_size(EVP_sha256());
        QByteArray current;

        // HKDF-Expand
        for (int i = 1; i <= n; i++) {
            current = T + QByteArray(1, static_cast<char>(i));
            T.resize(EVP_MD_size(EVP_sha256()));

            HMAC(EVP_sha256(), prk, prk_len,
                 reinterpret_cast<const unsigned char*>(current.constData()), current.length(),
                 reinterpret_cast<unsigned char*>(T.data()), nullptr);

            result.append(T);
        }

        return result.left(EncryptionUtils::HKDF_LENGTH);
    }
           // Other helper functions as needed
};
