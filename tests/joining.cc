#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get joining related values", "[joining]") {
  database db;

  db.open("ucd/packed/unicode-7.0.0.ucd");

  jg group;

  group = Joining_Group::Ain;
  REQUIRE(db.joining_type('A', group) == Joining_Type::Non_Joining);
  REQUIRE(group == Joining_Group::No_Joining_Group);

  group = Joining_Group::Ain;
  REQUIRE(db.joining_type(0x303, group) == Joining_Type::Transparent);
  REQUIRE(group == Joining_Group::No_Joining_Group);

  group = Joining_Group::Ain;
  REQUIRE(db.joining_type(0x600, group) == Joining_Type::Non_Joining);
  REQUIRE(group == Joining_Group::No_Joining_Group);

  REQUIRE(db.joining_type(0x620, group) == Joining_Type::Dual_Joining);
  REQUIRE(group == Joining_Group::Yeh);

  REQUIRE(db.joining_type(0x630, group) == Joining_Type::Right_Joining);
  REQUIRE(group == Joining_Group::Dal);

  REQUIRE(db.joining_type(0xa872, group) == Joining_Type::Left_Joining);
  REQUIRE(group == Joining_Group::No_Joining_Group);
}
