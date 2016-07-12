#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Canonical_Combining_Class values", "[ccc]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.canonical_combining_class('A') == Canonical_Combining_Class::Not_Reordered);
  REQUIRE(db.canonical_combining_class(0x30a1) == Canonical_Combining_Class::Not_Reordered);
  REQUIRE(db.canonical_combining_class(0x20d2) == Canonical_Combining_Class::Overlay);
  REQUIRE(db.canonical_combining_class(0x1d168) == Canonical_Combining_Class::Overlay);
  REQUIRE(db.canonical_combining_class(0x93c) == Canonical_Combining_Class::Nukta);
  REQUIRE(db.canonical_combining_class(0x11173) == Canonical_Combining_Class::Nukta);
  REQUIRE(db.canonical_combining_class(0x3099) == Canonical_Combining_Class::Kana_Voicing);
  REQUIRE(db.canonical_combining_class(0xf84) == Canonical_Combining_Class::Virama);
  REQUIRE(db.canonical_combining_class(0x1714) == Canonical_Combining_Class::Virama);
  REQUIRE(db.canonical_combining_class(0x1bf2) == Canonical_Combining_Class::Virama);
  REQUIRE(db.canonical_combining_class(0x5b3) == Canonical_Combining_Class::CCC13);
  REQUIRE(db.canonical_combining_class(0xe38) == Canonical_Combining_Class::CCC103);
  REQUIRE(db.canonical_combining_class(0x1d165) == Canonical_Combining_Class::Attached_Above_Right);
  REQUIRE(db.canonical_combining_class(0xfc6) == Canonical_Combining_Class::Below);
}
