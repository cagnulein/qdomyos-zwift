#ifndef ECHELONUNLOCK_H
#define ECHELONUNLOCK_H

#include <QByteArray>

namespace echelonunlock {

enum class Error {
    None,
    InvalidLength,
    InvalidPrefix,
    InvalidDecimalPrefix,
    ZeroDivisor,
    ResultOutOfRange,
};

struct Result {
    QByteArray response;
    int authValue = -1;
    Error error = Error::None;

    bool valid() const { return error == Error::None; }
};

Result responseForChallenge(const QByteArray &challenge);
QByteArray activationCommand();

} // namespace echelonunlock

#endif // ECHELONUNLOCK_H
