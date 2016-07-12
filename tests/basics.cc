#include "catch.hpp"
#include <libucd/libucd.h>

TEST_CASE("we can open a UCD file", "[opening]") {
  ucd::database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.unicode_version() == ucd::version(9,0,0));
}

TEST_CASE("we can close a UCD file and open it again", "[closing]") {
  ucd::database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  db.close();

  db.open("ucd/packed/unicode-9.0.0.ucd");
}
