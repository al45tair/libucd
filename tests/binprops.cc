#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("check that various binary properties work", "[binprops]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");
  
  /* This is far from an exhaustive test (there are *lots* of binary properties)
     but since they share the same implementation it's a reasonable assumption
     that if the ones here work then the others will too. */

  REQUIRE(db.ascii_hex_digit('q') == false);
  REQUIRE(db.ascii_hex_digit('A') == true);
  REQUIRE(db.ascii_hex_digit('3') == true);
  REQUIRE(db.ascii_hex_digit('e') == true);
  REQUIRE(db.bidi_control(0x202c) == true);
  REQUIRE(db.dash('-') == true);
  REQUIRE(db.deprecated(0x149) == true);
  REQUIRE(db.diacritic(0x2d2) == true);
  REQUIRE(db.extender(0xa9cf) == true);
  REQUIRE(db.hex_digit('B') == true);
  REQUIRE(db.hex_digit('9') == true);
  REQUIRE(db.hex_digit('c') == true);
  REQUIRE(db.hex_digit(0xff18) == true);
  REQUIRE(db.hex_digit(0xff22) == true);
  REQUIRE(db.hex_digit(0xff46) == true);
  REQUIRE(db.hyphen(0xad) == true);
  REQUIRE(db.ideographic(0x2b742) == true);
  REQUIRE(db.ids_binary_operator(0x2ff5) == true);
  REQUIRE(db.ids_binary_operator('!') == false);
  REQUIRE(db.ids_trinary_operator(0x2ff2) == true);
  REQUIRE(db.join_control(0x200c) == true);
  REQUIRE(db.logical_order_exception(0x0ec2) == true);
  REQUIRE(db.noncharacter_code_point(0x8fffe) == true);
  REQUIRE(db.lowercase('a') == true);
  REQUIRE(db.lowercase('A') == false);
  REQUIRE(db.uppercase('a') == false);
  REQUIRE(db.uppercase('A') == true);
  REQUIRE(db.cased('q') == true);
  REQUIRE(db.cased('$') == false);
  REQUIRE(db.case_ignorable(0x374) == true);
  REQUIRE(db.case_ignorable('A') == false);
  REQUIRE(db.alphabetic('a') == true);
  REQUIRE(db.alphabetic('*') == false);
  REQUIRE(db.grapheme_base(0x212a) == true);
}
