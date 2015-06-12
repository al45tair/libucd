#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can look up blocks", "[blocks]") {
  database db;

  db.open("ucd/packed/unicode-7.0.0.ucd");

  SECTION("enumerating blocks") {
    for (auto i = db.blocks().begin(); i != db.blocks().end(); ++i) {
      (void)*i;
    }
  }

  SECTION("getting a block from a code point") {
    REQUIRE(db.block('A')->name() == "Basic Latin");
    REQUIRE(db.block(0x1f600)->name() == "Emoticons");
    REQUIRE(db.block(0x20a3)->name() == "Currency Symbols");
    REQUIRE(db.block(0xe0082) == NULL);
  }

  SECTION("getting a block from its name") {
    REQUIRE(db.block_from_name("Basic Latin")->name() == "Basic Latin");
    REQUIRE(db.block_from_name("basic latin")->name() == "Basic Latin");
    REQUIRE(db.block_from_name("ARABIC PRESENTATION FORMS-B")->name()
            == "Arabic Presentation Forms-B");
    REQUIRE(db.block_from_name("<nonexistent block>") == NULL);
  }
}
