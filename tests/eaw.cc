#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get East_Asian_Width values", "[eaw]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.east_asian_width(0x0d) == East_Asian_Width::N);
  REQUIRE(db.east_asian_width(' ') == East_Asian_Width::Na);
  REQUIRE(db.east_asian_width('A') == East_Asian_Width::Na);
  REQUIRE(db.east_asian_width(0xb4) == East_Asian_Width::A);
  REQUIRE(db.east_asian_width(0x3011) == East_Asian_Width::W);
}
