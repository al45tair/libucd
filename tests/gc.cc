#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get General_Category values", "[gc]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.general_category('X') == General_Category::Lu);
  REQUIRE(db.general_category('q') == General_Category::Ll);
  REQUIRE(db.general_category(0x1c5) == General_Category::Lt);
  REQUIRE(db.general_category(0x2bc) == General_Category::Lm);
  REQUIRE(db.general_category(0xaa) == General_Category::Lo);

  REQUIRE(db.general_category(0x308) == General_Category::Mn);
  REQUIRE(db.general_category(0x94a) == General_Category::Mc);
  REQUIRE(db.general_category(0x488) == General_Category::Me);

  REQUIRE(db.general_category('5') == General_Category::Nd);
  REQUIRE(db.general_category(0x16ef) == General_Category::Nl);
  REQUIRE(db.general_category(0xb2) == General_Category::No);

  REQUIRE(db.general_category(0x203f) == General_Category::Pc);
  REQUIRE(db.general_category('-') == General_Category::Pd);
  REQUIRE(db.general_category('(') == General_Category::Ps);
  REQUIRE(db.general_category(')') == General_Category::Pe);
  REQUIRE(db.general_category(0xab) == General_Category::Pi);
  REQUIRE(db.general_category(0xbb) == General_Category::Pf);
  REQUIRE(db.general_category('!') == General_Category::Po);

  REQUIRE(db.general_category('+') == General_Category::Sm);
  REQUIRE(db.general_category('$') == General_Category::Sc);
  REQUIRE(db.general_category('`') == General_Category::Sk);
  REQUIRE(db.general_category(0xa9) == General_Category::So);

  REQUIRE(db.general_category(' ') == General_Category::Zs);
  REQUIRE(db.general_category(0x2028) == General_Category::Zl);
  REQUIRE(db.general_category(0x2029) == General_Category::Zp);

  REQUIRE(db.general_category(0x08) == General_Category::Cc);
  REQUIRE(db.general_category(0x200c) == General_Category::Cf);
  REQUIRE(db.general_category(0xd808) == General_Category::Cs);
  REQUIRE(db.general_category(0xe105) == General_Category::Co);
  REQUIRE(db.general_category(0xeffff) == General_Category::Cn);
}
