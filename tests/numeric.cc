#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("the numeric type works", "[numeric]") {
  REQUIRE(NaN.isnan());
  REQUIRE(infinity.isinf());
  REQUIRE(int(numeric(1, 10, 0)) == 1);
  REQUIRE(int(numeric(-1, 10, 0)) == -1);
  REQUIRE(double(numeric(3, 16, -1)) == 3.0 / 16.0);
  REQUIRE(double(numeric(1, 10, 12)) == 1.0e12);
  REQUIRE(NaN.to_string() == "NaN");
  REQUIRE(infinity.to_string() == "infinity");
  REQUIRE((-infinity).to_string() == "-infinity");
  REQUIRE(numeric(3, 16, -1).to_string() == "3/16");
  REQUIRE(numeric(1, 10, 3).to_string() == "1000");
  REQUIRE(numeric(1, 10, 12).to_string() == "1000000000000");
  REQUIRE((-numeric(3, 16, -1)).to_string() == "-3/16");
  REQUIRE(-infinity != infinity);
  REQUIRE(-(-infinity) == infinity);
}

TEST_CASE("we can look up numeric types and values", "[numb]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.numeric_type('a') == Numeric_Type::None);
  REQUIRE(db.numeric_type('3') == Numeric_Type::Decimal);
  REQUIRE(db.numeric_type(0xb9) == Numeric_Type::Digit);
  REQUIRE(db.numeric_type(0xbc) == Numeric_Type::Numeric);
  
  REQUIRE(db.numeric_value('a') == NaN);
  REQUIRE(int(db.numeric_value('3')) == 3);
  REQUIRE(int(db.numeric_value(0xb9)) == 1);
  REQUIRE(double(db.numeric_value(0xbc)) == 0.25);
  REQUIRE(int(db.numeric_value(0x842c)) == 10000);
  REQUIRE(db.numeric_value(0x5146).to_string() == "1000000000000");
}
