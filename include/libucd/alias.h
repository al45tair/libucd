/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_ALIAS_H_
#define LIBUCD_ALIAS_H_

#include <string>
#include "types.h"

namespace ucd {

  class alias {
  private:
    Alias_Type::Enum _type;
    std::string      _name;

  public:
    alias(Alias_Type::Enum type, const std::string &name)
      : _type(type), _name(name) {}
    
    Alias_Type::Enum type() const { return _type; }
    const std::string &name() const { return _name; }
  };

}

#endif /* LIBUCD_ALIAS_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
