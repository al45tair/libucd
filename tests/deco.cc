#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Decomposition_Type and Decomposition_Mapping values",
          "[deco]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  SECTION("Decomposition_Type") {
    REQUIRE(db.decomposition_type('a') == Decomposition_Type::None);
    REQUIRE(db.decomposition_type(0xc9) == Decomposition_Type::Canonical);
    REQUIRE(db.decomposition_type(0x2102) == Decomposition_Type::Font);
    REQUIRE(db.decomposition_type(0x2007) == Decomposition_Type::Nobreak);
    REQUIRE(db.decomposition_type(0xfb52) == Decomposition_Type::Isolated);
    REQUIRE(db.decomposition_type(0xfb53) == Decomposition_Type::Final);
    REQUIRE(db.decomposition_type(0xfb54) == Decomposition_Type::Initial);
    REQUIRE(db.decomposition_type(0xfb55) == Decomposition_Type::Medial);
    REQUIRE(db.decomposition_type(0x2460) == Decomposition_Type::Circle);
    REQUIRE(db.decomposition_type(0xb2) == Decomposition_Type::Super);
    REQUIRE(db.decomposition_type(0x1d62) == Decomposition_Type::Sub);
    REQUIRE(db.decomposition_type(0x309f) == Decomposition_Type::Vertical);
    REQUIRE(db.decomposition_type(0x3000) == Decomposition_Type::Wide);
    REQUIRE(db.decomposition_type(0xff61) == Decomposition_Type::Narrow);
    REQUIRE(db.decomposition_type(0xfe61) == Decomposition_Type::Small);
    REQUIRE(db.decomposition_type(0x32cf) == Decomposition_Type::Square);
    REQUIRE(db.decomposition_type(0xbc) == Decomposition_Type::Fraction);
    REQUIRE(db.decomposition_type(0xa8) == Decomposition_Type::Compat);
    REQUIRE(db.decomposition_type(0xd4db) == Decomposition_Type::Canonical);
  }

  SECTION("Decomposition_Mapping") {
    codepoint out[4];
    size_t len;
    bool ok;

    len = db.decomposition_mapping('a', out, 4);
    ok = (len == 1 && out[0] == 'a');
    REQUIRE(ok);

    len = db.decomposition_mapping(0xc9, out, 4);
    ok = (len == 2 && out[0] == 0x45 && out[1] == 0x0301);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x2102, out, 4);
    ok = (len == 1 && out[0] == 0x43);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x2007, out, 4);
    ok = (len == 1 && out[0] == 0x20);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xfb52, out, 4);
    ok = (len == 1 && out[0] == 0x67b);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xfb53, out, 4);
    ok = (len == 1 && out[0] == 0x67b);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xfb54, out, 4);
    ok = (len == 1 && out[0] == 0x67b);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xfb55, out, 4);
    ok = (len == 1 && out[0] == 0x67b);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x2460, out, 4);
    ok = (len == 1 && out[0] == 0x31);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xb2, out, 4);
    ok = (len == 1 && out[0] == 0x32);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x1d62, out, 4);
    ok = (len == 1 && out[0] == 0x69);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x309f, out, 4);
    ok = (len == 2 && out[0] == 0x3088 && out[1] == 0x308a);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x3000, out, 4);
    ok = (len == 1 && out[0] == 0x20);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xff61, out, 4);
    ok = (len == 1 && out[0] == 0x3002);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xfe61, out, 4);
    ok = (len == 1 && out[0] == 0x2a);
    REQUIRE(ok);

    len = db.decomposition_mapping(0x32cf, out, 4);
    ok = (len == 3 && out[0] == 0x4c && out[1] == 0x54 && out[2] == 0x44);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xbc, out, 4);
    ok = (len == 3 && out[0] == 0x31 && out[1] == 0x2044 && out[2] == 0x34);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xa8, out, 4);
    ok = (len == 2 && out[0] == 0x20 && out[1] == 0x308);
    REQUIRE(ok);

    len = db.decomposition_mapping(0xd4db, out, 4);
    ok = (len == 2 && out[0] == 0xd4cc && out[1] == 0x11b6);
    REQUIRE(ok);
  }

  SECTION("std::vector API") {
    std::vector<codepoint> out;
    bool ok;

    out = db.decomposition_mapping(0xbc);
    ok = (out.size() == 3
          && out[0] == 0x31 && out[1] == 0x2044 && out[2] == 0x34);
    REQUIRE(ok);

    out = db.decomposition_mapping(0xd4db);
    ok = (out.size() == 2
          && out[0] == 0xd4cc && out[1] == 0x11b6);
    REQUIRE(ok);
  }
}
