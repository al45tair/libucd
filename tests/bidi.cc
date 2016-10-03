#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Bidi_Class values", "[bc]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.bidi_class('\t') == Bidi_Class::S);
  REQUIRE(db.bidi_class('!') == Bidi_Class::ON);
  REQUIRE(db.bidi_class('+') == Bidi_Class::ES);
  REQUIRE(db.bidi_class(' ') == Bidi_Class::WS);
  REQUIRE(db.bidi_class('3') == Bidi_Class::EN);
  REQUIRE(db.bidi_class('a') == Bidi_Class::L);
  REQUIRE(db.bidi_class(0xa3) == Bidi_Class::ET);
  REQUIRE(db.bidi_class(0x05d1) == Bidi_Class::R);
  REQUIRE(db.bidi_class(0x0603) == Bidi_Class::AN);
  REQUIRE(db.bidi_class(0x0618) == Bidi_Class::NSM);
  REQUIRE(db.bidi_class(0x061f) == Bidi_Class::AL);
  REQUIRE(db.bidi_class(0x200b) == Bidi_Class::BN);
  REQUIRE(db.bidi_class(0x2029) == Bidi_Class::B);
  REQUIRE(db.bidi_class(0x202a) == Bidi_Class::LRE);
  REQUIRE(db.bidi_class(0x202b) == Bidi_Class::RLE);
  REQUIRE(db.bidi_class(0x202c) == Bidi_Class::PDF);
  REQUIRE(db.bidi_class(0x202d) == Bidi_Class::LRO);
  REQUIRE(db.bidi_class(0x202e) == Bidi_Class::RLO);
  REQUIRE(db.bidi_class(0x202f) == Bidi_Class::CS);
  REQUIRE(db.bidi_class(0x2066) == Bidi_Class::LRI);
  REQUIRE(db.bidi_class(0x2067) == Bidi_Class::RLI);
  REQUIRE(db.bidi_class(0x2068) == Bidi_Class::FSI);
  REQUIRE(db.bidi_class(0x2069) == Bidi_Class::PDI);
}

TEST_CASE("we can check the Bidi_Mirrored property", "[bidi-mirrored]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.bidi_mirrored('(') == true);
  REQUIRE(db.bidi_mirrored('A') == false);
  REQUIRE(db.bidi_mirrored(0x220b) == true);
  REQUIRE(db.bidi_mirrored(0x1d789) == true);
}

TEST_CASE("we can obtain the Bidi_Mirroring_Glyph property", "[bidi-mglyph]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.bidi_mirroring_glyph('(') == codepoint(')'));
  REQUIRE(db.bidi_mirroring_glyph(')') == codepoint('('));
  REQUIRE(db.bidi_mirroring_glyph('A') == bad_codepoint);
  REQUIRE(db.bidi_mirroring_glyph(0x220b) == char32_t(0x2208));
  REQUIRE(db.bidi_mirroring_glyph(0x2208) == char32_t(0x220b));
  REQUIRE(db.bidi_mirroring_glyph(0x1d789) == bad_codepoint);
}

TEST_CASE("we can read the Bidi_Paired_Bracket property", "[bidi-brak]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  bpt type;

  REQUIRE(db.bidi_paired_bracket('(', type) == codepoint(')'));
  REQUIRE(type == Bidi_Paired_Bracket_Type::Open);

  REQUIRE(db.bidi_paired_bracket(')', type) == codepoint('('));
  REQUIRE(type == Bidi_Paired_Bracket_Type::Close);

  REQUIRE(db.bidi_paired_bracket('A', type) == bad_codepoint);
  REQUIRE(type == Bidi_Paired_Bracket_Type::None);

  REQUIRE(db.bidi_paired_bracket(0x27ee, type) == char32_t(0x27ef));
  REQUIRE(type == Bidi_Paired_Bracket_Type::Open);

  REQUIRE(db.bidi_paired_bracket(0x27ef, type) == char32_t(0x27ee));
  REQUIRE(type == Bidi_Paired_Bracket_Type::Close);
}
