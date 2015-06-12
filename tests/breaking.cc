#include "catch.hpp"
#include <libucd/libucd.h>

using namespace ucd;

TEST_CASE("we can get breaking related values", "[breaking]") {
  database db;

  db.open("ucd/packed/unicode-7.0.0.ucd");

  SECTION("Line_Break") {
    REQUIRE(db.line_break(0x08) == Line_Break::CM);
    REQUIRE(db.line_break(0x09) == Line_Break::BA);
    REQUIRE(db.line_break(0x0a) == Line_Break::LF);
    REQUIRE(db.line_break(0x0b) == Line_Break::BK);
    REQUIRE(db.line_break(0x0d) == Line_Break::CR);
    REQUIRE(db.line_break(0x20) == Line_Break::SP);
    REQUIRE(db.line_break(0x21) == Line_Break::EX);
    REQUIRE(db.line_break(0x22) == Line_Break::QU);
    REQUIRE(db.line_break(0x23) == Line_Break::AL);
    REQUIRE(db.line_break('3') == Line_Break::NU);
    REQUIRE(db.line_break('C') == Line_Break::AL);
    REQUIRE(db.line_break(0x0589) == Line_Break::IS);
    REQUIRE(db.line_break(0x05ea) == Line_Break::HL);
    REQUIRE(db.line_break(0x106e) == Line_Break::SA);
    REQUIRE(db.line_break(0x1f39e) == Line_Break::ID);
    REQUIRE(db.line_break(0xe0020) == Line_Break::CM);
  }

  SECTION("Grapheme_Cluster_Break") {
    REQUIRE(db.grapheme_cluster_break(0x08) == Grapheme_Cluster_Break::Control);
    REQUIRE(db.grapheme_cluster_break(0x0d) == Grapheme_Cluster_Break::CR);
    REQUIRE(db.grapheme_cluster_break(0x0a) == Grapheme_Cluster_Break::LF);
    REQUIRE(db.grapheme_cluster_break(0x0483) == Grapheme_Cluster_Break::Extend);
    REQUIRE(db.grapheme_cluster_break(0x0b48) == Grapheme_Cluster_Break::SpacingMark);
    REQUIRE(db.grapheme_cluster_break(0xc8c4) == Grapheme_Cluster_Break::LV);
    REQUIRE(db.grapheme_cluster_break(0xaed9) == Grapheme_Cluster_Break::LVT);
  }

  SECTION("Sentence_Break") {
    REQUIRE(db.sentence_break(0x00) == Sentence_Break::Other);
    REQUIRE(db.sentence_break(0x0d) == Sentence_Break::CR);
    REQUIRE(db.sentence_break(0x0a) == Sentence_Break::LF);
    REQUIRE(db.sentence_break(0x0483) == Sentence_Break::Extend);
    REQUIRE(db.sentence_break(0x0188) == Sentence_Break::Lower);
    REQUIRE(db.sentence_break(0x1d63c) == Sentence_Break::Upper);
    REQUIRE(db.sentence_break(0x0b0f) == Sentence_Break::OLetter);
    REQUIRE(db.sentence_break(0x3016) == Sentence_Break::Close);
    REQUIRE(db.sentence_break(0xfe55) == Sentence_Break::SContinue);
  }

  SECTION("Word_Break") {
    REQUIRE(db.word_break(0x00) == Word_Break::Other);
    REQUIRE(db.word_break(0x22) == Word_Break::Double_Quote);
    REQUIRE(db.word_break(0x27) == Word_Break::Single_Quote);
    REQUIRE(db.word_break(0xfb38) == Word_Break::Hebrew_Letter);
    REQUIRE(db.word_break(0x0d) == Word_Break::CR);
    REQUIRE(db.word_break(0x0a) == Word_Break::LF);
    REQUIRE(db.word_break(0x0085) == Word_Break::Newline);
    REQUIRE(db.word_break(0x07eb) == Word_Break::Extend);
    REQUIRE(db.word_break(0x1f21) == Word_Break::ALetter);
    REQUIRE(db.word_break(0x33) == Word_Break::Numeric);
    REQUIRE(db.word_break(0xfe34) == Word_Break::ExtendNumLet);
  }
}
