#ifndef VIRTUALROWERCADENCE_H
#define VIRTUALROWERCADENCE_H

namespace virtualrowercadence {

inline double transform(double cadence, double gain, double offset) {
    return cadence * gain + offset;
}

} // namespace virtualrowercadence

#endif // VIRTUALROWERCADENCE_H
