#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can find Primary Composites from character pairs", "[prmc]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  codepoint cp;

  cp = db.primary_composite('e', 0x0301);
  REQUIRE(cp == 0xe9);

  cp = db.primary_composite('n', 0x0303);
  REQUIRE(cp == 0xf1);

  cp = db.primary_composite('!', 0x030a);
  REQUIRE(cp == 0);

  cp = db.primary_composite(0x1100, 0x1161);
  REQUIRE(cp == 0xac00);

  cp = db.primary_composite(0x1107, 0x116a);
  REQUIRE(cp == 0xbd10);

  cp = db.primary_composite(0xbd10, 0x11a9);
  REQUIRE(cp == 0xbd12);

  cp = db.primary_composite(0xbd10, 0x303);
  REQUIRE(cp == 0);

  cp = db.primary_composite('e', 'e');
  REQUIRE(cp == 0);
}
