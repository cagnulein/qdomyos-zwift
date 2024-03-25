#ifndef MINMAX_H
#define MINMAX_H

template <typename T>
struct minmax {
  private:
    T _max, _min;
  public:
    const T max();
    const T min();

    explicit minmax(const T min, const T max);

    T clip(const T value) const;

    bool contains(const T value) const;
};

#endif
