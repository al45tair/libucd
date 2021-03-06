#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get Script values", "[sc]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.script('(') == Script::Common);
  REQUIRE(db.script('A') == Script::Latin);
  REQUIRE(db.script(0x1d66) == Script::Greek);
  REQUIRE(db.script(0x0a85) == Script::Gujarati);
  REQUIRE(db.script(0x118a0) == Script::Warang_Citi);
}

TEST_CASE("we can get Script names", "[sc-names]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  REQUIRE(db.name_from_script(Script::Latin) == "Latin");
  REQUIRE(db.name_from_script(Script::Hluw) == "Anatolian_Hieroglyphs");

  // FourCCs go straight through
  REQUIRE(db.name_from_script('Four') == "Four");
}

TEST_CASE("we can look up scripts by name", "[sc-lookup]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  sc script;

  REQUIRE(db.script_from_name("egyptian hieroglyphs", script));
  REQUIRE(script == Script::Egyp);
  REQUIRE(db.script_from_name("oldturkic", script));
  REQUIRE(script == Script::Orkh);
  REQUIRE(db.script_from_name("uGaRiTiC", script));
  REQUIRE(script == Script::Ugar);

  // FourCCs go straight through
  REQUIRE(db.script_from_name("four", script));
  REQUIRE(script == 'Four');

  // Except for Qaai and Qaac
  REQUIRE(db.script_from_name("qaai", script));
  REQUIRE(script == 'Zinh');
  REQUIRE(db.script_from_name("qAaC", script));
  REQUIRE(script == 'Copt');

  // Other unknown names return bad_script
  REQUIRE(!db.script_from_name("The Hitchhiker's Guide to the Galaxy", script));
}

TEST_CASE("we can get Script_Extensions information", "[sext]") {
  database db;

  db.open("ucd/packed/unicode-9.0.0.ucd");

  std::vector<sc> result;
  bool ok;

  result = db.script_extensions('A');
  REQUIRE(result.size() == 0);

  result = db.script_extensions(0x1cd4);
  ok = (result.size() == 1 && result[0] == Script::Deva);
  REQUIRE(ok);

  result = db.script_extensions(0x1dc0);
  ok = (result.size() == 1 && result[0] == Script::Grek);
  REQUIRE(ok);

  result = db.script_extensions(0x102e2);
  ok = (result.size() == 2 && result[0] == Script::Arab
        && result[1] == Script::Copt);
  REQUIRE(ok);

  result = db.script_extensions(0x303c);
  ok = (result.size() == 3 
        && result[0] == Script::Hani
        && result[1] == Script::Hira
        && result[2] == Script::Kana);
  REQUIRE(ok);

  result = db.script_extensions(0x964);
  ok = (result.size() == 16
        && result[0] == Script::Beng
        && result[1] == Script::Deva
        && result[2] == Script::Gran
        && result[3] == Script::Gujr
        && result[4] == Script::Guru
        && result[5] == Script::Knda
        && result[6] == Script::Mahj
        && result[7] == Script::Mlym
        && result[8] == Script::Orya
        && result[9] == Script::Sind
        && result[10] == Script::Sinh
        && result[11] == Script::Sylo
        && result[12] == Script::Takr
        && result[13] == Script::Taml
        && result[14] == Script::Telu
        && result[15] == Script::Tirh);
  REQUIRE(ok);
}
