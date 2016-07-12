#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can obtain case mapping data", "[case]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  codepoint out[4];
  size_t len;
  bool ok;

  // A <-> a
  len = db.uppercase_mapping('a', out, 4);
  ok = (len == 1 && out[0] == 'A');
  REQUIRE(ok);

  len = db.uppercase_mapping('A', out, 4);
  ok = (len == 1 && out[0] == 'A');
  REQUIRE(ok);

  len = db.lowercase_mapping('A', out, 4);
  ok = (len == 1 && out[0] == 'a');
  REQUIRE(ok);

  len = db.titlecase_mapping('a', out, 4);
  ok = (len == 1 && out[0] == 'A');
  REQUIRE(ok);

  // Æ <-> æ
  len = db.uppercase_mapping(0xe6, out, 4);
  ok = (len == 1 && out[0] == 0xc6);
  REQUIRE(ok);

  len = db.titlecase_mapping(0xe6, out, 4);
  ok = (len == 1 && out[0] == 0xc6);
  REQUIRE(ok);

  len = db.lowercase_mapping(0xe6, out, 4);
  ok = (len == 1 && out[0] == 0xe6);
  REQUIRE(ok);

  // Ϊ <-> ϊ
  len = db.uppercase_mapping(0x3ca, out, 4);
  ok = (len == 1 && out[0] == 0x3aa);
  REQUIRE(ok);
  
  len = db.titlecase_mapping(0x3ca, out, 4);
  ok = (len == 1 && out[0] == 0x3aa);
  REQUIRE(ok);

  len = db.lowercase_mapping(0x3aa, out, 4);
  ok = (len == 1 && out[0] == 0x3ca);
  REQUIRE(ok);

  // ffi ligature -> FFI
  len = db.uppercase_mapping(0xfb03, out, 4);
  ok = (len == 3 && out[0] == 'F' && out[1] == 'F' && out[2] == 'I');
  REQUIRE(ok);
  
  len = db.titlecase_mapping(0xfb03, out, 4);
  ok = (len == 3 && out[0] == 'F' && out[1] == 'f' && out[2] == 'i');
  REQUIRE(ok);

  len = db.lowercase_mapping(0xfb03, out, 4);
  ok = (len == 1 && out[0] == 0xfb03);
  REQUIRE(ok);

  // U+0390 -> U+0399 U+0308 U+0301
  len = db.uppercase_mapping(0x390, out, 4);
  ok = (len == 3 && out[0] == 0x399 && out[1] == 0x308 && out[2] == 0x301);
  REQUIRE(ok);

  len = db.titlecase_mapping(0x390, out, 4);
  ok = (len == 3 && out[0] == 0x399 && out[1] == 0x308 && out[2] == 0x301);
  REQUIRE(ok);

  len = db.lowercase_mapping(0x390, out, 4);
  ok = (len == 1 && out[0] == 0x390);
  REQUIRE(ok);

  // Check the std::vector version
  cpvector out2;

  out2 = db.uppercase_mapping(0x390);
  ok = (out2.size() == 3
          && out2[0] == 0x399 && out2[1] == 0x308 && out2[2] == 0x0301);
  REQUIRE(ok);
}

TEST_CASE("we can obtain case folding data", "[casefold]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  SECTION("simple case folding") {
    REQUIRE(db.simple_case_folding('A') == codepoint('a'));
    REQUIRE(db.simple_case_folding(0xc1) == 0xe1);
    REQUIRE(db.simple_case_folding(0x1e9e) == 0xdf);
    REQUIRE(db.simple_case_folding(0x1f9f) == 0x1f97);
  }

  SECTION("full case folding") {
    cpvector out;
    bool ok;

    out = db.case_folding('A');
    ok = out.size() == 1 && out[0] == 'a';
    REQUIRE(ok);

    out = db.case_folding(0xc1);
    ok = out.size() == 1 && out[0] == 0xe1;
    REQUIRE(ok);

    out = db.case_folding(0x1e9e);
    ok = out.size() == 2 && out[0] == 's' && out[1] == 's';
    REQUIRE(ok);

    out = db.case_folding(0x1f9f);
    ok = out.size() == 2 && out[0] == 0x1f27 && out[1] == 0x03b9;
    REQUIRE(ok);
  }
}

TEST_CASE("normalisation case folding works", "[nfkc-casefold]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  SECTION("NFKC case folding") {
    REQUIRE(db.nfkc_casefold('A') == cpvector({'a'}));
    REQUIRE(db.nfkc_casefold(0xad) == cpvector({}));
    REQUIRE(db.nfkc_casefold(0xc1) == cpvector({ 0xe1 }));
    REQUIRE(db.nfkc_casefold(0x1e9e) == cpvector({ 's', 's' }));
    REQUIRE(db.nfkc_casefold(0x1f9f) == cpvector({ 0x1f27, 0x03b9 }));
  }

  SECTION("NFKC closure") {
    REQUIRE(db.fc_nfkc_closure('A') == cpvector({}));
    REQUIRE(db.fc_nfkc_closure(0x37a) == cpvector({ 0x0020, 0x03b9 }));
    REQUIRE(db.fc_nfkc_closure(0x3250) == cpvector({ 'p', 't', 'e' }));
    REQUIRE(db.fc_nfkc_closure(0x1f146) == cpvector({ 'w' }));
  }
}
