#pragma once

#include <QByteArray>

namespace kettler {

// Builds the raw 6-byte payload to be encrypted using the two-byte session seed.
QByteArray buildHandshakeSeed(const QByteArray &sessionSeed);

// Encrypts the raw handshake payload using the static bike key.
QByteArray computeHandshake(const QByteArray &handshakeRaw);

}
