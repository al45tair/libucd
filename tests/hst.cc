#include "catch.hpp"
#include <libucd/libucd.h>

TEST_CASE("we can get Hangul syllable types", "[hst]") {
  ucd::database db;
  using ucd::Hangul_Syllable_Type;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.hangul_syllable_type('A') == Hangul_Syllable_Type::NA);
  REQUIRE(db.hangul_syllable_type(0x1123) == Hangul_Syllable_Type::L);
  REQUIRE(db.hangul_syllable_type(0xa966) == Hangul_Syllable_Type::L);
  REQUIRE(db.hangul_syllable_type(0x1199) == Hangul_Syllable_Type::V);
  REQUIRE(db.hangul_syllable_type(0xd7c0) == Hangul_Syllable_Type::V);
  REQUIRE(db.hangul_syllable_type(0x11c0) == Hangul_Syllable_Type::T);
  REQUIRE(db.hangul_syllable_type(0xd7cc) == Hangul_Syllable_Type::T);
  REQUIRE(db.hangul_syllable_type(0xacfc) == Hangul_Syllable_Type::LV);
  REQUIRE(db.hangul_syllable_type(0xac02) == Hangul_Syllable_Type::LVT);
}
