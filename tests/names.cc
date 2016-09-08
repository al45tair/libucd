#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can map names to code points", "[names]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  SECTION("mapping names to code points") {
    REQUIRE(db.codepoint_from_name("<<<not a real name>>>") == bad_codepoint);
    // U+XXXX, U+XXXXX syntax
    REQUIRE(db.codepoint_from_name("U+0001f600") == bad_codepoint);
    REQUIRE(db.codepoint_from_name("U+1f600") == 0x1f600);
    REQUIRE(db.codepoint_from_name("U+20A3") == 0x20a3);
    REQUIRE(db.codepoint_from_name("U+41") == bad_codepoint);
    REQUIRE(db.codepoint_from_name("U+0041") == 0x0041);

    // CJK UNIFIED IDEOGRAPH+ syntax
    REQUIRE(db.codepoint_from_name("CJK UNIFIED IDEOGRAPH-3433") == 0x3433);
    REQUIRE(db.codepoint_from_name("CJK UNIFIED IDEOGRAPH-4EE8") == 0x4ee8);

    // CJK COMPATIBILITY IDEOGRAPH+ syntax
    REQUIRE(db.codepoint_from_name("CJK COMPATIBILITY IDEOGRAPH-F947") == 0xf947);
    REQUIRE(db.codepoint_from_name("CJK COMPATIBILITY IDEOGRAPH-F91D") == 0xf91D);

    // HANGUL SYLLABLE syntax
    REQUIRE(db.codepoint_from_name("HANGUL SYLLABLE GANG") == 0xac15);
    REQUIRE(db.codepoint_from_name("HANGUL SYLLABLE GYEON") == 0xacac);
    REQUIRE(db.codepoint_from_name("HANGUL SYLLABLE BBWAE") == 0xbf78);

    // Proper names
    REQUIRE(db.codepoint_from_name("GRINNING FACE") == 0x1f600);
    REQUIRE(db.codepoint_from_name("FRENCH FRANC SIGN") == 0x20a3);
    REQUIRE(db.codepoint_from_name("LATIN CAPITAL LETTER A") == 0x0041);

    // Should be case insensitive
    REQUIRE(db.codepoint_from_name("grinning FACE") == 0x1f600);
    REQUIRE(db.codepoint_from_name("FRENCH franc SIGN") == 0x20a3);
    REQUIRE(db.codepoint_from_name("LATIN CAPITAL letter A") == 0x0041);

    // Should ignore whitespace
    REQUIRE(db.codepoint_from_name("zerowidthspace") == 0x200b);
    REQUIRE(db.codepoint_from_name("RIGHTWARDSarrowWITHCornerDownwards") == 0x21b4);

    // Should ignore medial hyphens, except for U+1180 because of U+116c
    REQUIRE(db.codepoint_from_name("Linear B Ideogram B107M hegoat") == 0x10089);
    REQUIRE(db.codepoint_from_name("hangul jungseong oe") == 0x116c);
    REQUIRE(db.codepoint_from_name("hangul jungseong o-e") == 0x1180);
    REQUIRE(db.codepoint_from_name("TIBETAN MARK TSA -PHRU") == 0x0f39);
    REQUIRE(db.codepoint_from_name("alchemical symbol for borax 3") == 0x1f744);
  }

  SECTION("mapping code points to names") {
    // bad_codepoint
    REQUIRE(db.name(bad_codepoint) == "<bad codepoint>");

    // Codepoints without names
    REQUIRE(db.name(0x10ffff) == "U+10FFFF");

    // CJK UNIFIED IDEOGRAPH+ syntax
    REQUIRE(db.name(0x3433) == "CJK UNIFIED IDEOGRAPH-3433");
    REQUIRE(db.name(0x4ee8) == "CJK UNIFIED IDEOGRAPH-4EE8");

    // CJK COMPATIBILITY IDEOGRAPH+ syntax
    REQUIRE(db.name(0xf947) == "CJK COMPATIBILITY IDEOGRAPH-F947");
    REQUIRE(db.name(0xf91d) == "CJK COMPATIBILITY IDEOGRAPH-F91D");

    // HANGUL SYLLABLE syntax
    REQUIRE(db.name(0xac15) == "HANGUL SYLLABLE GANG");
    REQUIRE(db.name(0xacac) == "HANGUL SYLLABLE GYEON");
    REQUIRE(db.name(0xbf78) == "HANGUL SYLLABLE BBWAE");

    // Codepoints with names
    REQUIRE(db.name(0x1f600) == "GRINNING FACE");
    REQUIRE(db.name(0x20a3) == "FRENCH FRANC SIGN");
    REQUIRE(db.name(0x0041) == "LATIN CAPITAL LETTER A");
  }
}

TEST_CASE("aliases work", "[alias]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  SECTION("mapping names to code points") {
    REQUIRE(db.codepoint_from_name("NUL") == 0x0000);
    REQUIRE(db.codepoint_from_name("LATIN CAPITAL LETTER GHA") == 0x01a2);
    REQUIRE(db.codepoint_from_name("WEIERSTRASS ELLIPTIC FUNCTION") == 0x2118);

    REQUIRE(db.codepoint_from_name("STX", Alias_Type::abbreviation) == 0x02);
    REQUIRE(db.codepoint_from_name("STX", Alias_Type::none) == bad_codepoint);
    REQUIRE(db.codepoint_from_name("START OF TEXT",
                                   Alias_Type::abbreviation
                                   | Alias_Type::control
                                   | Alias_Type::alternate) == 0x02);
    REQUIRE(db.codepoint_from_name("STX", Alias_Type::correction)
            == bad_codepoint);
  }

  SECTION("retrieving aliases") {
    std::vector<alias> aliases;

    aliases = db.name_alias(0x89);
    REQUIRE(aliases.size() == 3);
    REQUIRE(aliases[0].type() == Alias_Type::control);
    REQUIRE(aliases[0].name() == "CHARACTER TABULATION WITH JUSTIFICATION");
    REQUIRE(aliases[1].type() == Alias_Type::control);
    REQUIRE(aliases[1].name() == "HORIZONTAL TABULATION WITH JUSTIFICATION");
    REQUIRE(aliases[2].type() == Alias_Type::abbreviation);
    REQUIRE(aliases[2].name() == "HTJ");

    aliases = db.name_alias(0x89, Alias_Type::abbreviation);
    REQUIRE(aliases.size() == 1);
    REQUIRE(aliases[0].type() == Alias_Type::abbreviation);
    REQUIRE(aliases[0].name() == "HTJ");

    aliases = db.name_alias(0x2118, Alias_Type::correction);
    REQUIRE(aliases.size() == 1);
    REQUIRE(aliases[0].type() == Alias_Type::correction);
    REQUIRE(aliases[0].name() == "WEIERSTRASS ELLIPTIC FUNCTION");

    aliases = db.name_alias('A', Alias_Type::correction);
    REQUIRE(aliases.size() == 0);
  }
}

TEST_CASE("can retrieve Unicode 1 names", "[u1names]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.unicode_1_name(0x0027) == "APOSTROPHE-QUOTE");
  REQUIRE(db.unicode_1_name(0x0041) == "");
  REQUIRE(db.unicode_1_name(0x00a0) == "NON-BREAKING SPACE");
}

TEST_CASE("can retrieve ISO comments", "[iso-comments]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  // There are no ISO comments in Unicode 7
  REQUIRE(db.iso_comment('A') == "");
  REQUIRE(db.iso_comment('x') == "");
}
