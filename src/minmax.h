#ifndef MINMAX_H
#define MINMAX_H

template <typename T>
struct minmax {
  private:
    T _max, _min;
  public:
    const T max() { return _max; }
    const T min() { return _min; }

    explicit minmax(const T min, const T max) :  _max(max), _min(min) {}

    T clip(const T value) const {
        T result = value;
        if(value<_min) result=_min;
        else if(value>_max) result=_max;
        return result;
    }

    bool contains(const T value) const {
        return value>=_min && value<=_max;
    }
};

#endif
