#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Hangul syllable types", "[hst]") {
  database db;

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

TEST_CASE("we can get Hangul syllable type names", "[hst-names]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.name_from_hangul_syllable_type(hst::L) == "Leading_Jamo");
  REQUIRE(db.name_from_hangul_syllable_type(hst::LVT) == "LVT_Syllable");
}

TEST_CASE("we can get Hangul syllable types from names", "[hst-from-names]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  hst v;

  REQUIRE(db.hangul_syllable_type_from_name("lvt", v));
  REQUIRE(v == hst::LVT);

  REQUIRE(db.hangul_syllable_type_from_name("leading jamo", v));
  REQUIRE(v == hst::L);
}
