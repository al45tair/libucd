#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Indic category values", "[indic]") {
  database db;

  db.open("ucd/packed/unicode-7.0.0.ucd");

  SECTION("Indic_Matra_Category") {
    REQUIRE(db.indic_matra_category('A') == Indic_Matra_Category::NA);
    REQUIRE(db.indic_matra_category(0x093e) == Indic_Matra_Category::Right);
    REQUIRE(db.indic_matra_category(0x111b4) == Indic_Matra_Category::Left);
    REQUIRE(db.indic_matra_category(0xabe5) == Indic_Matra_Category::Top);
    REQUIRE(db.indic_matra_category(0x0941) == Indic_Matra_Category::Bottom);
    REQUIRE(db.indic_matra_category(0x10a01) == Indic_Matra_Category::Overstruck);
  }

  SECTION("Indic_Syllabic_Category") {
    REQUIRE(db.indic_syllabic_category('A') == Indic_Syllabic_Category::Other);
    REQUIRE(db.indic_syllabic_category(0x0901) == Indic_Syllabic_Category::Bindu);
    REQUIRE(db.indic_syllabic_category(0x0f7f)
            == Indic_Syllabic_Category::Visarga);
    REQUIRE(db.indic_syllabic_category(0x1c37) == Indic_Syllabic_Category::Nukta);
    REQUIRE(db.indic_syllabic_category(0x109c) 
            == Indic_Syllabic_Category::Vowel_Dependent);
    REQUIRE(db.indic_syllabic_category(0x17cc) 
            == Indic_Syllabic_Category::Consonant_Succeeding_Repha);
    REQUIRE(db.indic_syllabic_category(0x11055)
            == Indic_Syllabic_Category::Brahmi_Joining_Number);
  }
}
