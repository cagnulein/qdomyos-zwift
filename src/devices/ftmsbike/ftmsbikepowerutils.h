#ifndef FTMSBIKEPOWERUTILS_H
#define FTMSBIKEPOWERUTILS_H

namespace ftmsbikepowerutils {

inline bool shouldForceZeroAveragePower(bool mrkS26c, bool instantCadencePresent, double cadence) {
    return mrkS26c && instantCadencePresent && cadence == 0.0;
}

} // namespace ftmsbikepowerutils

#endif // FTMSBIKEPOWERUTILS_H
