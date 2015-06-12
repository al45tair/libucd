/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_NUMERIC_H_
#define LIBUCD_NUMERIC_H_

#include <string>
#include <iosfwd>
#include <cmath>

namespace ucd {

  inline int ipow(int b, int e) {
      int r = 1;
      while (e) {
        if (e & 1)
          r *= b;
        e >>= 1;
        b *= b;
      }
      return r;
    }

  inline long long llpow(int b, int e) {
      long long r = 1;
      while (e) {
        if (e & 1)
          r *= b;
        e >>= 1;
        b *= b;
      }
      return r;
    }

  class numeric {
  private:
    int _multiplier;
    int _base;
    int _exponent;

  public:
    numeric() {}
    constexpr numeric(int n) : _multiplier(n), _base(10), _exponent(0) {}
    constexpr numeric(int n, int b, int e)
      : _multiplier(n), _base(b), _exponent(e) {}
    constexpr numeric(const numeric &o) 
      : _multiplier(o._multiplier), 
      _base(o._base),
      _exponent(o._exponent)
    {};

    int multiplier() const { return _multiplier; }
    int base() const { return _base; }
    int exponent() const { return _exponent; }

    bool isnan() const { return _multiplier == 0 && _base == 0 && _exponent < 0; }
    bool isinf() const { return _multiplier && _base == 0 && _exponent < 0; }

    /* N.B. These DO NOT test numerical equality. */
    bool operator==(const numeric &other) {
      return (_multiplier == other._multiplier
              && _base == other._base
              && _exponent == other._exponent);
    }

    bool operator!=(const numeric &other) {
      return (_multiplier != other._multiplier
              || _base != other._base
              || _exponent != other._exponent);
    }

    explicit operator double() const {
      if (!_exponent)
        return _multiplier;
      return _multiplier * std::pow(_base, _exponent);
    }

    explicit operator int() const {
      if (!_exponent)
        return _multiplier;
      if (_exponent < 0)
        return _multiplier / ipow(_base, -_exponent);
      return _multiplier * ipow(_base, _exponent);
    }

    explicit operator long long() const {
      if (!_exponent)
        return _multiplier;
      if (_exponent < 0)
        return _multiplier / llpow(_base, -_exponent);
      return _multiplier * llpow(_base, _exponent);
    }

    std::string to_string() const;

    // This exists so you can write -infinity
    numeric operator-() const {
      return numeric(-_multiplier, _base, _exponent);
    }
  };

  inline std::ostream &operator<<(std::ostream &os, const numeric &n) {
    return os << n.to_string();
  }

  const numeric NaN = numeric(0, 0, -1);
  const numeric infinity = numeric(1, 0, -1);

}

#endif /* LIBUCD_BLOCK_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
