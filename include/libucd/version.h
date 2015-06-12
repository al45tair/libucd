/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_VERSION_H_
#define LIBUCD_VERSION_H_

namespace ucd {

  class version {
  public:
    unsigned major, minor, bugfix;

    constexpr version(unsigned maj=0, unsigned min=0, unsigned bug=0)
      : major(maj), minor(min), bugfix(bug) {}
    constexpr version(const version &other)
      : major(other.major), minor(other.minor), bugfix(other.bugfix) {}

    bool operator<(const version &other) const {
      return (major < other.major
              || (major == other.major
                  && (minor < other.minor
                      || (minor == other.minor
                          && bugfix < other.bugfix))));
    }
    bool operator<=(const version &other) const {
      return (major < other.major
              || (major == other.major
                  && (minor < other.minor
                      || (minor == other.minor
                          && bugfix <= other.bugfix))));
    }
    bool operator==(const version &other) const {
      return (major == other.major 
              && minor == other.minor 
              && bugfix == other.bugfix);
    }
    bool operator!=(const version &other) const {
      return (major != other.major 
              || minor != other.minor 
              || bugfix != other.bugfix);
    }
    bool operator>(const version &other) const {
      return (major > other.major
              || (major == other.major
                  && (minor > other.minor
                      || (minor == other.minor
                          && bugfix > other.bugfix))));
    }
    bool operator>=(const version &other) const {
      return (major > other.major
              || (major == other.major
                  && (minor > other.minor
                      || (minor == other.minor
                          && bugfix >= other.bugfix))));
    }

    version &operator=(const version &other) {
      major = other.major;
      minor = other.minor;
      bugfix = other.bugfix;
      return *this;
    }

    static const version nil;
  };

  extern const ucd::version libucd_version;
}

#endif /* LIBUCD_EXCEPTIONS_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
