#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("maybe logic works", "[maybe]") {
  REQUIRE((maybe::no && maybe::no) == maybe::no);
  REQUIRE((maybe::no && maybe::maybe) == maybe::no);
  REQUIRE((maybe::no && maybe::yes) == maybe::no);
  REQUIRE((maybe::maybe && maybe::no) == maybe::no);
  REQUIRE((maybe::yes && maybe::no) == maybe::no);
  REQUIRE((maybe::maybe && maybe::maybe) == maybe::maybe);
  REQUIRE((maybe::maybe && maybe::yes) == maybe::maybe);
  REQUIRE((maybe::yes && maybe::maybe) == maybe::maybe);
  REQUIRE((maybe::yes && maybe::yes) == maybe::yes);

  REQUIRE((maybe::no || maybe::no) == maybe::no);
  REQUIRE((maybe::no || maybe::maybe) == maybe::maybe);
  REQUIRE((maybe::no || maybe::yes) == maybe::yes);
  REQUIRE((maybe::maybe || maybe::no) == maybe::maybe);
  REQUIRE((maybe::yes || maybe::no) == maybe::yes);
  REQUIRE((maybe::maybe || maybe::maybe) == maybe::maybe);
  REQUIRE((maybe::maybe || maybe::yes) == maybe::yes);
  REQUIRE((maybe::yes || maybe::maybe) == maybe::yes);
  REQUIRE((maybe::yes || maybe::yes) == maybe::yes);
  
  REQUIRE(!maybe::no == maybe::yes);
  REQUIRE(!maybe::yes == maybe::no);
  REQUIRE(!maybe::maybe == maybe::maybe);
}
