#ifndef LOCALKEYPROVIDER_H
#define LOCALKEYPROVIDER_H

#include <openssl/evp.h>
#include <openssl/ec.h>
#include <QByteArray>
#include "encryptionUtils.h"

class LocalKeyProvider {
  public:
    LocalKeyProvider() {
        generateKeyPair();
    }

    ~LocalKeyProvider() {
        EVP_PKEY_free(pair);
    }

    QByteArray getPublicKeyBytes() {
        EC_KEY *eckey = EVP_PKEY_get1_EC_KEY(pair);
        QByteArray bytes = EncryptionUtils::publicKeyToByteArray(eckey);
        EC_KEY_free(eckey);
        return bytes;
    }

    EC_KEY* getPublicKey() {
        EC_KEY *eckey = EVP_PKEY_get1_EC_KEY(pair);
        return eckey; // Remember to free this EC_KEY when you're done with it
    }

    EVP_PKEY* getPrivateKey() {
        EVP_PKEY_up_ref(pair);
        return pair; // Remember to free this EVP_PKEY when you're done with it
    }

  private:
    EVP_PKEY *pair = nullptr;

    void generateKeyPair() {
        EVP_PKEY_CTX *pctx = EVP_PKEY_CTX_new_id(EVP_PKEY_EC, nullptr);
        EVP_PKEY_keygen_init(pctx);
        EVP_PKEY_CTX_set_ec_paramgen_curve_nid(pctx, NID_X9_62_prime256v1);
        EVP_PKEY_keygen(pctx, &pair);
        EVP_PKEY_CTX_free(pctx);
    }
};

#endif // LOCALKEYPROVIDER_H
