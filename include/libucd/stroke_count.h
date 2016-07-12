/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_STROKE_COUNT_H_
#define LIBUCD_STROKE_COUNT_H_

#include <string>
#include "types.h"

namespace ucd {

  class stroke_count {
  private:
    unsigned _radical;
    bool     _simplified;
    int      _additional_strokes;

  public:
    typedef enum {
      traditional = 0,
      simplified = 1
    } simplified_t;

    stroke_count(unsigned r, simplified_t s, int a)
      : _radical(r), _simplified(s == simplified), _additional_strokes(a) {}

    unsigned radical() const { return _radical; }
    bool is_simplified() const { return _simplified; }
    int additional_strokes() const { return _additional_strokes; }

    bool operator==(const stroke_count &other) const {
      return (_radical == other._radical 
              && _simplified == other._simplified
              && _additional_strokes == other._additional_strokes);
    }
    bool operator!=(const stroke_count &other) const {
      return (_radical != other._radical 
              || _simplified != other._simplified
              || _additional_strokes != other._additional_strokes);
    }

    static const stroke_count none;
  };
}

#endif /* LIBUCD_STROKE_COUNT_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
