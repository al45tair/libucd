/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_BLOCK_H_
#define LIBUCD_BLOCK_H_

#include <string>

namespace ucd {

  class block {
  private:
    codepoint   _first, _last;
    std::string _name;

  public:
    block() {}
  block(codepoint first, codepoint last, const std::string &name)
    : _first(first), _last(last), _name(name) {}
  block(const block &o)
    : _first(o._first), _last(o._last), _name(o._name) {}
  block(block &&o)
    : _first(o._first), _last(o._last), _name(std::move(o._name)) {}

    bool operator==(const block &other) const {
      return (_first == other._first
              && _last == other._last);
    }
    bool operator!=(const block &other) const {
      return (_first != other._first
              || _last != other._last);
    }

  public:
    codepoint first() const { return _first; }
    codepoint last() const { return _last; }
    bool contains(codepoint cp) const { return cp >= _first && cp <= _last; }
    std::string name() const { return _name; }
  };

}

#endif /* LIBUCD_BLOCK_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
