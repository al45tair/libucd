#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("check that emoji properties work", "[emoji]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");
  
  REQUIRE(db.emoji_version() == ucd::version(3,0,0));

  REQUIRE(db.emoji('A') == false);
  REQUIRE(db.emoji('#') == true);
  REQUIRE(db.emoji(0x1f57a) == true);
  REQUIRE(db.emoji_presentation(0x26bd) == true);
  REQUIRE(db.emoji_modifier(0x1f3fc) == true);
  REQUIRE(db.emoji_modifier_base(0x261d) == true);
}
