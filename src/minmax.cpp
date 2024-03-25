#include "minmax.h"

template<typename T>
const T minmax<T>::max() { return this->_max; }

template<typename T>
const T minmax<T>::min() { return this->_min; }

template<typename T>
minmax<T>::minmax(const T min, const T max) :  _max(max), _min(min) {}

template<typename T>
T minmax<T>::clip(const T value) const {
    T result = value;
    if(value<this->_min) result=_min;
    else if(value>this->_max) result=_max;
    return result;
}

template<typename T>
bool minmax<T>::contains(const T value) const {
    return value>=this->_min && value<=this->_max;
}
