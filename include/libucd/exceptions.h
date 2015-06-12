/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_EXCEPTIONS_H_
#define LIBUCD_EXCEPTIONS_H_

#include <stdexcept>

namespace ucd {
  class runtime_error : public std::runtime_error {
  public:
    runtime_error(const std::string &why) : std::runtime_error(why) {}
  };

  class bad_data_file : public runtime_error {
  public:
    bad_data_file(const std::string &why) : runtime_error(why) {}
  };

  class no_name_table : public runtime_error {
  public:
    no_name_table(const std::string &why) : runtime_error(why) {}
  };

  class no_alias_table : public runtime_error {
  public:
    no_alias_table(const std::string &why) : runtime_error(why) {}
  };
}

#endif /* LIBUCD_EXCEPTIONS_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
