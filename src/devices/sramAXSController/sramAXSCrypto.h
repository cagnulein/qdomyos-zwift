#ifndef SRAMAXSCRYPTO_H
#define SRAMAXSCRYPTO_H

#include <QByteArray>

namespace sramaxscrypto {

QByteArray eaxEncrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &message);
QByteArray eaxDecrypt(const QByteArray &key, const QByteArray &nonce, const QByteArray &ciphertextAndTag,
                     bool *authenticated = nullptr);

QByteArray computePublicKey(const QByteArray &privateKey);
QByteArray computeSharedSecret(const QByteArray &privateKey, const QByteArray &devicePublicKey);
QByteArray decryptTransportedKey(const QByteArray &sharedSecret, const QByteArray &transportBlob,
                                 bool *authenticated = nullptr);

} // namespace sramaxscrypto

#endif // SRAMAXSCRYPTO_H
