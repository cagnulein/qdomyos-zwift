#include "echelonunlock.h"

#include <QtGlobal>
#include <initializer_list>

/*
 * The challenge/response algorithm was discovered and documented by Christian Emerson
 * (@christianbemerson). Keep the attribution with the protocol codec as well as in the PR.
 */

namespace {

bool appendDecimalPrefix(const QByteArray &hexBytes, qint64 *value) {
    qint64 parsed = 0;
    bool hasDigit = false;

    for (const char byte : hexBytes) {
        if (byte < '0' || byte > '9') {
            break;
        }
        hasDigit = true;
        parsed = (parsed * 10) + (byte - '0');
    }

    if (!hasDigit) {
        return false;
    }

    *value = parsed;
    return true;
}

bool decimalPrefixFromBytes(std::initializer_list<uint8_t> bytes, qint64 *value) {
    QByteArray hex;
    for (const uint8_t byte : bytes) {
        hex.append(QByteArray::number(byte, 16).rightJustified(2, '0'));
    }
    return appendDecimalPrefix(hex, value);
}

} // namespace

namespace echelonunlock {

Result responseForChallenge(const QByteArray &challenge) {
    Result result;

    if (challenge.size() != 8) {
        result.error = Error::InvalidLength;
        return result;
    }

    if (static_cast<uint8_t>(challenge.at(0)) != 0xf0 || static_cast<uint8_t>(challenge.at(1)) != 0xe0) {
        result.error = Error::InvalidPrefix;
        return result;
    }

    const auto byteAt = [&challenge](int index) { return static_cast<uint8_t>(challenge.at(index)); };
    qint64 a = 0;
    qint64 b = 0;
    qint64 c = 0;
    if (!decimalPrefixFromBytes({byteAt(4), byteAt(2)}, &a) ||
        !decimalPrefixFromBytes({byteAt(3), byteAt(5)}, &b) ||
        !decimalPrefixFromBytes({byteAt(6)}, &c)) {
        result.error = Error::InvalidDecimalPrefix;
        return result;
    }

    if (c == 0) {
        result.error = Error::ZeroDivisor;
        return result;
    }

    const qint64 authValue = (a * b) % c;
    if (authValue < 0 || authValue > 99) {
        result.error = Error::ResultOutOfRange;
        return result;
    }

    const uint8_t packedBcd = static_cast<uint8_t>(((authValue / 10) << 4) | (authValue % 10));
    result.authValue = static_cast<int>(authValue);
    result.response = QByteArray::fromHex("f0e00000000000");
    result.response[4] = static_cast<char>(packedBcd);

    uint8_t checksum = 0;
    for (int i = 0; i < result.response.size(); ++i) {
        checksum = static_cast<uint8_t>(checksum + static_cast<uint8_t>(result.response.at(i)));
    }
    result.response.append(static_cast<char>(checksum));
    return result;
}

QByteArray activationCommand() {
    return QByteArray::fromHex("f0b00101a2");
}

} // namespace echelonunlock
