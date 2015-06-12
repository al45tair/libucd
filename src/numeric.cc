#include <string>
#include <libucd/libucd.h>

using namespace ucd;

std::string
numeric::to_string() const
{
  if (isinf()) {
    if (_multiplier < 0)
      return "-infinity";
    else
      return "infinity";
  } else if (isnan()) {
    return "NaN";
  }

  char buffer[80];
  if (_exponent == -1) {
    sprintf(buffer, "%d/%d", _multiplier, _base);
    return buffer;
  } else if (_exponent) {
    if (_base == 10) {
      sprintf(buffer, "%d", _multiplier);

      std::string result(buffer);
      for (int n = 0; n < _exponent; ++n)
        result += '0';

      return result;
    }

    sprintf(buffer, "%dx%d^%d", _multiplier, _base, _exponent);
    return buffer;
  } else {
    sprintf(buffer, "%d", _multiplier);
    return buffer;
  }
}
