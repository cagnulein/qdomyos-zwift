#include <QByteArray>
#include <openssl/evp.h>
#include <openssl/ec.h>
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

    static EC_KEY* generatePublicKey(const QByteArray& publicKeyBytes, EC_GROUP* paramSpec) {
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
        }
        if (EVP_PKEY_derive_set_peer(ctx, serverPublicKey) <= 0) {
            // Handle error
        }
        size_t secret_len;
        if (EVP_PKEY_derive(ctx, nullptr, &secret_len) <= 0) {
            // Handle error
        }
        result.resize(secret_len);
        if (EVP_PKEY_derive(ctx, reinterpret_cast<unsigned char *>(result.data()), &secret_len) <= 0) {
            // Handle error
        }

        EVP_PKEY_CTX_free(ctx);
        return result;
    }

    static QByteArray generateHKDFBytes(const QByteArray& secretKey, const QByteArray& salt) {
        // HKDF implementation using OpenSSL
        QByteArray result(HKDF_LENGTH, Qt::Initialization::Uninitialized);
        // HKDF key derivation logic goes here.
        return result;
    }

           // Other helper functions as needed
};
