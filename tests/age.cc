#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can inspect Age properties", "[age]") {
  database db;

  db.open("ucd/packed/unicode-7.0.0.ucd");

  REQUIRE(db.age('A') == version(1, 1));
  REQUIRE(db.age(0x0fb9) == version(2, 0));
  REQUIRE(db.age(0x1788) == version(3, 0));
  REQUIRE(db.age(0x03f5) == version(3, 1));
  REQUIRE(db.age(0x20e4) == version(3, 2));
  REQUIRE(db.age(0x0601) == version(4, 0));
  REQUIRE(db.age(0x10a05) == version(4, 1));
  REQUIRE(db.age(0x27c7) == version(5, 0));
  REQUIRE(db.age(0x1028) == version(5, 1));
  REQUIRE(db.age(0x10841) == version(5, 2));
  REQUIRE(db.age(0xa660) == version(6, 0));
  REQUIRE(db.age(0x1f634) == version(6, 1));
  REQUIRE(db.age(0x20ba) == version(6, 2));
  REQUIRE(db.age(0x061c) == version(6, 3));
  REQUIRE(db.age(0x0978) == version(7, 0));
  REQUIRE(db.age(0xefffd) == version::nil);
}
