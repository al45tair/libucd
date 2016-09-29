/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_DATABASE_H_
#define LIBUCD_DATABASE_H_

#include "types.h"
#include "version.h"
#include "block.h"
#include "numeric.h"
#include "alias.h"
#include "stroke_count.h"

#include <vector>
#include <string>

namespace ucd {

  class database {
  private:
    struct impl;
    std::unique_ptr<impl> _pimpl;

  public:
    database();
    database(const char *filename);
    database(const void *base, size_t length);
    ~database();

    void open(const char *filename);
    void close();

    version unicode_version() const;
    version emoji_version() const;

    /* This method does lookups based on Name and Name_Alias; it does not
       and will not use Unicode_1_Name. */
    codepoint codepoint_from_name(const std::string &name,
                                  unsigned allowed_types = Alias_Type::all) const;

    std::string name(codepoint cp) const;

    /* N.B. There can be more than one OF THE SAME TYPE (e.g. U+0089), as
            well as multiple aliases of different types. */
    std::vector<alias> name_alias(codepoint cp,
                                  unsigned allowed_types = Alias_Type::all) const;

    // These two are obsolete, but someone might still want them
    std::string unicode_1_name(codepoint cp) const;
    std::string iso_comment(codepoint cp) const;

    std::string jamo_short_name(codepoint cp) const;
    hst hangul_syllable_type(codepoint cp) const;
    gc general_category(codepoint cp) const;
    ccc canonical_combining_class(codepoint cp) const;

    nt numeric_type(codepoint cp) const;
    numeric numeric_value(codepoint cp) const;

    const class block *block(codepoint cp) const;
    const class block *block_from_name(const std::string &name) const;
    const std::vector<class block> &blocks() const;

    /* These unify Simple_Uppercase_Mapping with Uppercase_Mapping, etc.
       If you need Simple_Uppercase_Mapping specifically, just check if
       there is only a single codepoint of output.  Similarly if you just
       need Uppercase_Mapping on its own. */
    size_t uppercase_mapping(codepoint from_cp,
                             codepoint *out, size_t out_len) const;
    size_t lowercase_mapping(codepoint from_cp,
                             codepoint *out, size_t out_len) const;
    size_t titlecase_mapping(codepoint from_cp,
                             codepoint *out, size_t out_len) const;

    cpvector uppercase_mapping(codepoint cp) const;
    cpvector lowercase_mapping(codepoint cp) const;
    cpvector titlecase_mapping(codepoint cp) const;

    codepoint simple_case_folding(codepoint cp) const;
    size_t case_folding(codepoint from_cp,
                        codepoint *out, size_t out_len) const;
    cpvector case_folding(codepoint cp) const;

    bc bidi_class(codepoint cp) const;
    bool bidi_mirrored(codepoint cp) const;
    codepoint bidi_mirroring_glyph(codepoint cp) const;

    codepoint bidi_paired_bracket(codepoint cp) const;
    bpt bidi_paired_bracket_type(codepoint cp) const;
    codepoint bidi_paired_bracket(codepoint cp, bpt &type) const;

    dt decomposition_type(codepoint cp) const;
    size_t decomposition_mapping(codepoint cp,
                                 codepoint *out, size_t out_len) const;
    size_t decomposition_mapping(codepoint cp,
                                 codepoint *out, size_t out_len,
                                 dt &dtype) const;
    cpvector decomposition_mapping(codepoint cp) const;
    cpvector decomposition_mapping(codepoint cp, dt &dtype) const;

    codepoint primary_composite(codepoint starter, codepoint composing) const;

    version age(codepoint cp) const;
    sc script(codepoint cp) const;
    std::vector<sc> script_extensions(codepoint cp) const;

    sc script_from_name(const std::string &name) const;
    std::string name_from_script(sc script) const;

    ea east_asian_width(codepoint cp) const;
    stroke_count unicode_radical_stroke(codepoint cp) const;

    InPC indic_positional_category(codepoint cp) const;
    InSC indic_syllabic_category(codepoint cp) const;

    // Backwards compatibility (renamed in Unicode 8)
    InMC indic_matra_category(codepoint cp) {
      return indic_positional_category(cp);
    }

    // Normalisation properties
    bool composition_exclusion(codepoint cp) const;
    bool full_composition_exclusion(codepoint cp) const;
    bool expands_on_nfd(codepoint cp) const;
    bool expands_on_nfc(codepoint cp) const;
    bool expands_on_nfkd(codepoint cp) const;
    bool expands_on_nfkc(codepoint cp) const;
    bool changes_when_nfkc_casefolded(codepoint cp) const;

    size_t nfkc_casefold(codepoint from_cp,
                         codepoint *out, size_t out_len) const;
    cpvector nfkc_casefold(codepoint cp) const;

    size_t fc_nfkc_closure(codepoint cp,
                           codepoint *out, size_t out_len) const;
    cpvector fc_nfkc_closure(codepoint cp) const;

    maybe nfc_quick_check(codepoint cp) const;
    maybe nfkc_quick_check(codepoint cp) const;
    maybe nfd_quick_check(codepoint cp) const;
    maybe nfkd_quick_check(codepoint cp) const;

    // Shaping
    jg joining_group(codepoint cp) const;
    jt joining_type(codepoint cp) const;
    jt joining_type(codepoint cp, jg &jgroup) const;

    // Breaking
    lb line_break(codepoint cp) const;
    GCB grapheme_cluster_break(codepoint cp) const;
    SB sentence_break(codepoint cp) const;
    WB word_break(codepoint cp) const;

    // Binary properties
    bool ascii_hex_digit(codepoint cp) const;
    bool bidi_control(codepoint cp) const;
    bool dash(codepoint cp) const;
    bool deprecated(codepoint cp) const;
    bool diacritic(codepoint cp) const;
    bool extender(codepoint cp) const;
    bool hex_digit(codepoint cp) const;
    bool hyphen(codepoint cp) const;
    bool ideographic(codepoint cp) const;
    bool ids_binary_operator(codepoint cp) const;
    bool ids_trinary_operator(codepoint cp) const;
    bool join_control(codepoint cp) const;
    bool logical_order_exception(codepoint cp) const;
    bool noncharacter_code_point(codepoint cp) const;
    bool other_alphabetic(codepoint cp) const;
    bool other_default_ignorable_code_point(codepoint cp) const;
    bool other_grapheme_extend(codepoint cp) const;
    bool other_id_continue(codepoint cp) const;
    bool other_id_start(codepoint cp) const;
    bool other_lowercase(codepoint cp) const;
    bool other_math(codepoint cp) const;
    bool other_uppercase(codepoint cp) const;
    bool pattern_syntax(codepoint cp) const;
    bool pattern_white_space(codepoint cp) const;
    bool prepended_concatenation_mark(codepoint cp) const;
    bool quotation_mark(codepoint cp) const;
    bool radical(codepoint cp) const;
    bool soft_dotted(codepoint cp) const;
    bool sterm(codepoint cp) const;
    bool sentence_terminal(codepoint cp) const { return sterm(cp); }
    bool terminal_punctuation(codepoint cp) const;
    bool unified_ideograph(codepoint cp) const;
    bool variation_selector(codepoint cp) const;
    bool white_space(codepoint cp) const;

    bool lowercase(codepoint cp) const;
    bool uppercase(codepoint cp) const;
    bool cased(codepoint cp) const;
    bool case_ignorable(codepoint cp) const;
    bool changes_when_lowercased(codepoint cp) const;
    bool changes_when_uppercased(codepoint cp) const;
    bool changes_when_titlecased(codepoint cp) const;
    bool changes_when_casefolded(codepoint cp) const;
    bool changes_when_casemapped(codepoint cp) const;
    bool alphabetic(codepoint cp) const;
    bool default_ignorable_code_point(codepoint cp) const;
    bool grapheme_base(codepoint cp) const;
    bool grapheme_extend(codepoint cp) const;
    bool grapheme_link(codepoint cp) const;
    bool math(codepoint cp) const;
    bool id_start(codepoint cp) const;
    bool id_continue(codepoint cp) const;
    bool xid_start(codepoint cp) const;
    bool xid_continue(codepoint cp) const;

    // Emoji properties
    bool emoji(codepoint cp) const;
    bool emoji_presentation(codepoint cp) const;
    bool emoji_modifier(codepoint cp) const;
    bool emoji_modifier_base(codepoint cp) const;
  };

}

#endif /* LIBUCD_DATABASE_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
