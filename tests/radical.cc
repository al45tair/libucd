#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Unicode_Radical_Stroke values", "[rads]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.unicode_radical_stroke('A') == stroke_count::none);
  REQUIRE(db.unicode_radical_stroke(0x3688) 
          == stroke_count(36, stroke_count::traditional, 2));
  REQUIRE(db.unicode_radical_stroke(0x4336)
          == stroke_count(120, stroke_count::simplified, 3));
  REQUIRE(db.unicode_radical_stroke(0x8c60)
          == stroke_count(152, stroke_count::traditional, 5));
  REQUIRE(db.unicode_radical_stroke(0x225a9)
          == stroke_count(61, stroke_count::traditional, -1));
  REQUIRE(db.unicode_radical_stroke(0x8002)
          == stroke_count(125, stroke_count::traditional, -2));
}
