#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can examine NFC_Quick_Check et al", "[quick-check]") {
  database db;

  db.open("ucd/packed/unicode-7.0.0.ucd");

  REQUIRE(db.nfc_quick_check('A') == maybe::yes);
  REQUIRE(db.nfc_quick_check(0x0340) == maybe::no);
  REQUIRE(db.nfc_quick_check(0x0345) == maybe::maybe);

  REQUIRE(db.nfkc_quick_check('A') == maybe::yes);
  REQUIRE(db.nfkc_quick_check(0xaa) == maybe::no);
  REQUIRE(db.nfkc_quick_check(0x0f0c) == maybe::no);
  REQUIRE(db.nfkc_quick_check(0x1f240) == maybe::no);
  REQUIRE(db.nfkc_quick_check(0x0d57) == maybe::maybe);

  REQUIRE(db.nfd_quick_check(0xaa) == maybe::yes);
  REQUIRE(db.nfd_quick_check(0xc1) == maybe::no);
  REQUIRE(db.nfd_quick_check(0x304e) == maybe::no);
  
  REQUIRE(db.nfkd_quick_check('A') == maybe::yes);
  REQUIRE(db.nfkd_quick_check(0xaa) == maybe::no);
  REQUIRE(db.nfkd_quick_check(0x1f131) == maybe::no);
}
