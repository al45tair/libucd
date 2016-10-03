/*
 * ucd-format.h - Defines the binary format of the .ucd files used by this
 *                library.
 * libucd
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef UCD_FORMAT_H_
#define UCD_FORMAT_H_

#include <inttypes.h>

#pragma pack(push,1)

/* .. Base Types ............................................................ */

typedef uint32_t ucd_string_id_t;

/* .. Header ................................................................ */

enum {
  UCD_MAGIC = 'ucd2'
};

enum {
  UCD_MAX_TABLES = 510  // 510 * 8 + 12 = 4092
};

struct ucd_table {
  uint32_t table_id;
  uint32_t offset;
};

struct ucd_header {
  uint32_t        magic;
  uint32_t        unicode_version;     /* 00070102 = 7.1.2 */
  uint32_t        emoji_version;       /* 00030100 = 3.1 */
  uint32_t        num_tables;
  struct ucd_table tables[0];
};

/* .. Tables ................................................................ */

enum {
  UCD_blok = 'blok',    /* Blocks table                    */
  UCD_strn = 'strn',    /* String storage                  */
  UCD_name = 'name',    /* Character name table            */
  UCD_alis = 'alis',    /* Character name alias table      */
  UCD_u1nm = 'u1nm',    /* Unicode 1 name table            */
  UCD_isoc = 'isoc',    /* ISO Comment table               */
  UCD_jamo = 'jamo',    /* Hangul syllable type table      */
  UCD_jamn = 'jmo$',    /* Hangul syllable type name table */
  UCD_genc = 'genc',    /* General Category table          */
  UCD_gcn  = 'gc$ ',    /* General Category name table     */
  UCD_CASE = 'CASE',    /* Uppercase Mapping table         */
  UCD_case = 'case',    /* Lowercase Mapping table         */
  UCD_Case = 'Case',    /* Titlecase Mapping table         */
  UCD_csef = 'csef',    /* Case Folding table              */
  UCD_kccf = 'kccf',    /* NFKC Case Folding table         */
  UCD_nfkc = 'nfkc',    /* FC_NFKC_Closure table           */
  UCD_ccc  = 'ccc ',    /* Canonical Combining Class table */
  UCD_cccn = 'ccc$',    /* CCC name table                  */
  UCD_numb = 'numb',    /* Numeric Type/Value table        */
  UCD_numn = 'num$',    /* Numeric Type name table         */
  UCD_bidi = 'bidi',    /* Bidi table                      */
  UCD_bdin = 'bdi$',    /* Bidi name table                 */
  UCD_mirr = 'mirr',    /* Bidi mirroring table            */
  UCD_brak = 'brak',    /* Bidi bracket table              */
  UCD_deco = 'deco',    /* Decomposition table             */
  UCD_decn = 'dec$',    /* Decomposition name table        */
  UCD_age  = 'age ',    /* Age table                       */
  UCD_scpt = 'scpt',    /* Scripts table                   */
  UCD_scpn = 'scp$',    /* Script names table              */
  UCD_cqc  = 'cqc ',    /* NFC quick check table           */
  UCD_kcqc = 'kcqc',    /* NFKC quick check table          */
  UCD_dqc  = 'dqc ',    /* NFD quick check table           */
  UCD_kdqc = 'kdqc',    /* NFKD quick check table          */
  UCD_join = 'join',    /* Joining table                   */
  UCD_jonn = 'jon$',    /* Joining name table              */
  UCD_lbrk = 'lbrk',    /* Line breaking table             */
  UCD_lbkn = 'lbkn',    /* Line breaking name table        */
  UCD_gbrk = 'gbrk',    /* Grapheme Cluster Break table    */
  UCD_gbkn = 'gbk$',    /* GCB name table                  */
  UCD_sbrk = 'sbrk',    /* Sentence breaking table         */
  UCD_sbkn = 'sbk$',    /* Sentence break name table       */
  UCD_wbrk = 'wbrk',    /* Word breaking table             */
  UCD_wbkn = 'wbk$',    /* Word break name table           */
  UCD_eaw  = 'eaw ',    /* East Asian width table          */
  UCD_eawn = 'eaw$',    /* East Asian width name table     */
  UCD_rads = 'rads',    /* Unicode Radical Stroke table    */
  UCD_inmc = 'inmc',    /* Indic Matra Category table      */
  UCD_imcn = 'imc$',    /* Indic Matra Category name table */
  UCD_insc = 'insc',    /* Indic Syllabic Category table   */
  UCD_iscn = 'isc$',    /* Indic Syllabic Cat name table   */
  UCD_prmc = 'prmc',    /* Primary Composite table         */
};

/* There are a large number of tables ending with a '?' that are not defined
   here, but instead you will find a table of them in ucd-binprops.h */

/* Tables ending in '$' contain name data for the associated values */

/* .. ...$ .................................................................. */

/* In each case, there are num_fwd + num_rev entries; the first set is sorted by
   value, the second by (case-folded) name, so that we can map both ways.
   In general, num_fwd and num_rev can differ; specifically, there can be
   aliases, which means that there may be more reverse entries than forward
   entries. */
struct ucd_n32_entry {
  uint32_t        value;
  ucd_string_id_t name;
};

struct ucd_n32 {
  uint32_t             num_fwd;
  uint32_t             num_rev;
  struct ucd_n32_entry names[0];
};

struct ucd_n16_entry {
  uint16_t        value;
  ucd_string_id_t name;
};

struct ucd_n16 {
  uint32_t             num_fwd;
  uint32_t             num_rev;
  struct ucd_n16_entry names[0];
};

struct ucd_n8_entry {
  uint8_t         value;
  ucd_string_id_t name;
};

struct ucd_n8 {
  uint32_t             num_fwd;
  uint32_t             num_rev;
  struct ucd_n8_entry  names[0];
};

/* .. strn .................................................................. */

struct ucd_strings {
  uint32_t size;
  char     strings[0];
};

/* .. blok .................................................................. */

struct ucd_block {
  uint32_t        first_cp;
  uint32_t        last_cp;
  ucd_string_id_t name;
  ucd_string_id_t alias;
};

struct ucd_blok {
  uint32_t         num_blocks;
  struct ucd_block blocks[0];
};

/* .. name .................................................................. */

/* The name table contains two arrays of ucd_name_entry structures; the first
   is sorted by code point, the second by name (NOT by sid, but according to
   the actual strings). */

struct ucd_name_entry {
  uint32_t        code_point;
  ucd_string_id_t name;
};

enum {
  UCD_NAME_RANGE_CJK_UNIFIED_IDEOGRAPH = 1,
  UCD_NAME_RANGE_CJK_COMPATIBILITY_IDEOGRAPH = 2,
  UCD_NAME_RANGE_HANGUL_SYLLABLE = 3,
};

struct ucd_name_range {
  uint32_t first_cp;
  uint32_t last_cp;
  uint32_t kind;
};

struct ucd_name_ranges {
  uint32_t              num_ranges;
  struct ucd_name_range ranges[0];
};

struct ucd_names {
  uint32_t              num_names;
  struct ucd_name_entry names[0];
  // After 2 * num_names entries, there is a ucd_name_ranges struct
};

/* .. u1nm .................................................................. */

/* The Unicode 1 Name table *does not* contain the name-sorted table; it is
   only used for queries and not for codepoint lookups. */

struct ucd_u1nm {
  uint32_t              num_names;
  struct ucd_name_entry names[0];
};

/* .. isoc .................................................................. */

struct ucd_comment {
  uint32_t        code_point;
  ucd_string_id_t comment;
};

struct ucd_isoc {
  uint32_t              num_comments;
  struct ucd_comment    comments[0];
};

/* .. alis .................................................................. */

/* Similarly to the name table, there are two arrays of ucd_alias structures;
   the first is sorted by code point then by kind; the second is sorted by
   name (NOT by sid, but by the actual strings). */

struct ucd_alias {
  uint32_t        entry;
  ucd_string_id_t name;
};

typedef enum {
  UCD_ALIAS_KIND_CORRECTION   = 1,
  UCD_ALIAS_KIND_CONTROL      = 2,
  UCD_ALIAS_KIND_ALTERNATE    = 4,
  UCD_ALIAS_KIND_FIGMENT      = 8,
  UCD_ALIAS_KIND_ABBREVIATION = 16
} ucd_alias_kind_t;

#define UCD_ALIAS_CODE_POINT(entry) ((entry) & 0x00ffffff)
#define UCD_ALIAS_KIND(entry)       ((ucd_alias_kind_t)((entry) >> 24))

struct ucd_alis {
  uint32_t         num_aliases;
  struct ucd_alias aliases[0];
};

/* .. jamo .................................................................. */

enum {
  UCD_JAMO_NA            = 0x00,
  UCD_JAMO_L             = 0x01,
  UCD_JAMO_V             = 0x02,
  UCD_JAMO_T             = 0x04,
  UCD_JAMO_LV            = UCD_JAMO_L | UCD_JAMO_V,
  UCD_JAMO_LVT           = UCD_JAMO_LV | UCD_JAMO_T,
};

struct ucd_jamo_range {
  uint16_t first_cp;
  uint16_t last_cp;
  uint16_t kind;
};

struct ucd_jamo {
  int32_t               num_ranges;
  struct ucd_jamo_range ranges[0];
};

/* .. genc .................................................................. */

struct ucd_genc_range {
  uint32_t first_cp;
  uint16_t category;
};

struct ucd_genc {
  uint32_t              num_ranges;
  struct ucd_genc_range ranges[0];
};

/* .. CASE/case/Case/csef/nkfc .............................................. */

/* Note that the nkfc table (which contains the FC_NFKC_Closure property) is
   not, strictly speaking, a case mapping; codepoints that don't have values
   listed in the DerivedNormalizationProps.txt file default to FC_NFKC_Closure
   being empty, as opposed to the codepoint itself. */

/* These tables contain unconditional case mappings */
struct ucd_case_range {
  uint32_t entry;
  union {
    uint32_t mapped_cp;
    uint32_t last_cp;
    struct {
      uint32_t last_cp;
      int32_t  delta;
    } delta;
    struct {
      uint32_t last_cp;
      uint32_t offset;
    } table;
    struct {
      uint32_t simple_cp;
      uint16_t full_packed[2];
    } sf_packed;
    struct {
      uint32_t simple_cp;
      uint32_t full_offset;
    } sf_external;
    uint16_t packed[4];
    uint32_t ext_offset;
    uint16_t ucs2[4];
    uint32_t ucs4[2];
  };
};

typedef enum {
  UCD_CASE_RANGE_SINGLE      = 0,  // single cp to cp mapping
  UCD_CASE_RANGE_DELTA       = 1,  // integer offset for each cp in range
  UCD_CASE_RANGE_EVEN        = 2,  // clear bottom bit for each cp in range
  UCD_CASE_RANGE_ODD         = 3,  // set bottom bit for each cp in range
  UCD_CASE_RANGE_EVEN_UP     = 4,  // add one if odd for each cp in range
  UCD_CASE_RANGE_ODD_DOWN    = 5,  // sub one if even for each cp in range
  UCD_CASE_RANGE_PACKED      = 6,  // UTF-16 in packed
  UCD_CASE_RANGE_EXTERNAL    = 7,  // UTF-16 at ext_offset
  UCD_CASE_RANGE_TABLE       = 8,  // table at table.offset
  UCD_CASE_RANGE_UCS4TBL     = 9,  // inline 2-entry UCS-4 table
  UCD_CASE_RANGE_UCS2T3      = 10, // inline 3-entry UCS-2 table
  UCD_CASE_RANGE_UCS2T4      = 11, // inline 4-entry UCS-2 table
  UCD_CASE_RANGE_SF_PACKED   = 12, // simple/full combination, packed
  UCD_CASE_RANGE_SF_EXTERNAL = 13, // simple/full combination, external
  UCD_CASE_RANGE_EMPTY       = 14, // maps to nothing (i.e. remove codepoint)
} ucd_case_range_kind_t;

#define UCD_CASE_RANGE_CP(entry)        ((entry) & 0xffffff)
#define UCD_CASE_RANGE_KIND(entry)      ((ucd_case_range_kind_t)(entry >> 24))

struct ucd_case {
  uint32_t              num_ranges;
  struct ucd_case_range ranges[0];
};

/* .. ccc  .................................................................. */

struct ucd_ccc_range {
  uint32_t entry;
  union {
    struct {
      uint32_t last_cp;
      uint8_t  code;
    } run;
    struct {
      uint8_t count;
      uint8_t codes[4];
    } inline_tbl;
    struct {
      uint8_t count;
      uint32_t offset;
    } table;
  };
  uint8_t reserved;
};

typedef enum {
  UCD_CCC_RANGE_RUN    = 0,
  UCD_CCC_RANGE_INLINE = 1,
  UCD_CCC_RANGE_TABLE  = 2
} ucd_ccc_range_kind_t;

#define UCD_CCC_RANGE_CP(entry)        ((entry) & 0xffffff)
#define UCD_CCC_RANGE_KIND(entry)      ((ucd_ccc_range_kind_t)(entry >> 24))

struct ucd_ccc {
  uint32_t             num_ranges;
  struct ucd_ccc_range ranges[0];
};

/* .. numb .................................................................. */

typedef enum {
  UCD_NUMERIC_TYPE_DECIMAL = 1,
  UCD_NUMERIC_TYPE_DIGIT = 2,
  UCD_NUMERIC_TYPE_NUMERIC = 3
} ucd_numb_entry_type_t;

#define UCD_NUMB_ENTRY_CP(entry)        ((entry) & 0xffffff)
#define UCD_NUMB_ENTRY_TYPE(entry)      ((ucd_numb_entry_type_t)((entry) >> 24))

struct ucd_numb_entry {
  uint32_t entry;

  /* Value is given by multiplier x base**exponent

     We need to support plain digits (0..9), fractions (3/16, -1/2),
     and also large numbers (100000000000000). */
  int16_t multiplier;
  uint8_t base;
  int8_t  exponent;
};

struct ucd_numb {
  uint32_t              num_entries;
  struct ucd_numb_entry entries[0];
};

/* .. bidi .................................................................. */

typedef enum {
  UCD_BIDI_L = 1,
  UCD_BIDI_R = 2,
  UCD_BIDI_AL = 3,
  UCD_BIDI_EN = 4,
  UCD_BIDI_ES = 5,
  UCD_BIDI_ET = 6,
  UCD_BIDI_AN = 7,
  UCD_BIDI_CS = 8,
  UCD_BIDI_NSM = 9,
  UCD_BIDI_BN = 10,
  UCD_BIDI_B = 11,
  UCD_BIDI_S = 12,
  UCD_BIDI_WS = 13,
  UCD_BIDI_ON = 14,
  UCD_BIDI_LRE = 15,
  UCD_BIDI_LRO = 16,
  UCD_BIDI_RLE = 17,
  UCD_BIDI_RLO = 18,
  UCD_BIDI_PDF = 19,
  UCD_BIDI_LRI = 20,
  UCD_BIDI_RLI = 21,
  UCD_BIDI_FSI = 22,
  UCD_BIDI_PDI = 23
} ucd_bidi_class_t;

#define UCD_BIDI_ENTRY_CP(entry)        ((entry) & 0xffffff)
#define UCD_BIDI_ENTRY_CLASS(entry)     ((ucd_bidi_class_t)((entry) >> 24))

struct ucd_bidi {
  uint32_t num_entries;
  uint32_t entries[0];
};

/* .. mirr .................................................................. */

struct ucd_mirr_entry {
  uint32_t cp;
  uint32_t mcp;
};

struct ucd_mirr {
  uint32_t num_entries;
  struct ucd_mirr_entry entries[0];
};

/* .. brak .................................................................. */

struct ucd_brak_entry {
  uint32_t entry;
  uint32_t other_cp;
};

typedef enum {
  UCD_BRAK_ENTRY_NONE = 0,
  UCD_BRAK_ENTRY_OPEN = 1,
  UCD_BRAK_ENTRY_CLOSE = 2
} ucd_brak_entry_type_t;

#define UCD_BRAK_ENTRY_CP(entry)        ((entry) & 0x00ffffff)
#define UCD_BRAK_ENTRY_TYPE(entry)      ((ucd_brak_entry_type_t)((entry) >> 24))

struct ucd_brak {
  uint32_t num_entries;
  struct ucd_brak_entry entries[0];
};

/* .. deco .................................................................. */

typedef enum {
  UCD_DECO_CANONICAL_TAG = 0,
  UCD_DECO_FONT_TAG = 1,
  UCD_DECO_NOBREAK_TAG = 2,
  UCD_DECO_INITIAL_TAG = 3,
  UCD_DECO_MEDIAL_TAG = 4,
  UCD_DECO_FINAL_TAG = 5,
  UCD_DECO_ISOLATED_TAG = 6,
  UCD_DECO_CIRCLE_TAG = 7,
  UCD_DECO_SUPER_TAG = 8,
  UCD_DECO_SUB_TAG = 9,
  UCD_DECO_VERTICAL_TAG = 10,
  UCD_DECO_WIDE_TAG = 11,
  UCD_DECO_NARROW_TAG = 12,
  UCD_DECO_SMALL_TAG = 13,
  UCD_DECO_SQUARE_TAG = 14,
  UCD_DECO_FRACTION_TAG = 15,
  UCD_DECO_COMPAT_TAG = 16
} ucd_deco_tag_t;

typedef enum {
  UCD_DECO_ENTRY_SINGLE   = 0,  // single cp to cp mapping
  UCD_DECO_ENTRY_PACKED   = 1,  // UTF-16 in packed
  UCD_DECO_ENTRY_EXTERNAL = 2,  // UTF-16 at ext_offset
} ucd_deco_entry_kind_t;

struct ucd_deco_entry {
  uint32_t codepoint;
  uint8_t  kind;
  uint8_t  tag;
  union {
    uint32_t mapped_cp;
    uint16_t packed[2];
    uint32_t ext_offset;
  };
};

struct ucd_deco {
  uint32_t              num_entries;
  struct ucd_deco_entry entries[0];
};

/* .. Binary Property ....................................................... */

struct ucd_binprop_range {
  uint32_t first_cp;
  uint32_t last_cp;
};

struct ucd_binprop {
  uint32_t num_ranges;
  struct ucd_binprop_range ranges[0];
};

/* .. age  .................................................................. */

struct ucd_age_entries {
  uint32_t num_entries;
  uint32_t entries[0];
};

struct ucd_age {
  uint32_t num_versions;
  uint32_t versions[0];
  // struct ucd_age_entries entries;
};

#define UCD_AGE_CP(entry) ((entry) & 0x00ffffff)
#define UCD_AGE_VERSION_NDX(entry) ((entry) >> 24)
#define UCD_AGE_MAJOR(version) ((version) >> 16)
#define UCD_AGE_MINOR(version) ((version) & 0xffff)

/* .. scpt .................................................................. */

struct ucd_scpt_ext_entry {
  uint32_t entry;
  uint32_t offset;      // Offset to array of scripts relative to ucd_scpt
};

#define UCD_SCPT_EXT_CP(entry) ((entry) & 0x00ffffff)
#define UCD_SCPT_EXT_COUNT(entry) ((entry) >> 24)

struct ucd_scpt_extensions {
  uint32_t                  num_entries;
  struct ucd_scpt_ext_entry entries[0];
  // uint32_t               data[0];
};

/* Maps code points to the four-character codes from PropertyValueAliases */
struct ucd_scpt_entry {
  uint32_t code_point;
  uint32_t script;
};

struct ucd_scpt {
  uint32_t              num_entries;
  struct ucd_scpt_entry entries[0];
  // struct ucd_scpt_extensions extensions;
};

/* .. cqc /kcqc/dqc /kdqc ................................................... */

enum {
  UCD_QUICK_CHECK_NO    = 0,
  UCD_QUICK_CHECK_MAYBE = 1,
  UCD_QUICK_CHECK_YES   = 2,
};

#define UCD_QUICK_CHECK_CP(entry)       ((entry) & 0x00ffffff)
#define UCD_QUICK_CHECK_VALUE(entry)    ((entry) >> 24)

struct ucd_qc {
  uint32_t num_entries;
  uint32_t entries[0];
};

/* .. join .................................................................. */

typedef enum {
  UCD_JOIN_TYPE_NON_JOINING  = 0,
  UCD_JOIN_TYPE_TRANSPARENT  = 1,
  UCD_JOIN_TYPE_RIGHT        = 2,
  UCD_JOIN_TYPE_LEFT         = 3,
  UCD_JOIN_TYPE_DUAL         = 4,
  UCD_JOIN_TYPE_JOIN_CAUSING = 5,
} ucd_join_type_t;

struct ucd_join_entry {
  uint32_t codepoint;
  uint8_t  type;
  uint8_t  group;
};

struct ucd_join {
  uint32_t              num_entries;
  struct ucd_join_entry entries[0];
};

/* .. jon$ .................................................................. */

/* The jon$ table contains *two* name tables, the first for join type, and
   the second for joining group. */

/* .. lbrk/gbrk/sbrk/wbrk ................................................... */

struct ucd_brk {
  uint32_t num_entries;
  uint32_t entries[0];
};

#define UCD_BRK_CODEPOINT(entry)       ((entry) & 0x00ffffff)
#define UCD_BRK_BREAK(entry)           ((entry) >> 24)

/* .. eaw  .................................................................. */

struct ucd_eaw {
  uint32_t num_entries;
  uint32_t entries[0];
};

#define UCD_EAW_CODEPOINT(entry)        ((entry) & 0x00ffffff)
#define UCD_EAW_WIDTH(entry)            ((entry) >> 24)

/* .. rads .................................................................. */

struct ucd_rads_table_entry {
  uint8_t radical;
  int8_t stroke_info; // top bit means "simplified"
};

#define UCD_RADS_STROKE_COUNT(stroke_info) ((int8_t)(((stroke_info) & 0x7f)  \
                                                     | (((stroke_info) << 1) \
                                                        & 0x80)))
#define UCD_RADS_IS_SIMPLIFIED(stroke_info) ((stroke_info) & 0x80)

struct ucd_rads_range {
  uint32_t first_cp;
  uint32_t last_cp;
  uint32_t offset; // offset from ucd_rads to array of table entries
};

struct ucd_rads {
  uint32_t              num_ranges;
  struct ucd_rads_range ranges[0];
};

/* .. inmc/insc  ............................................................ */

struct ucd_inc {
  uint32_t num_entries;
  uint32_t entries[0];
};

#define UCD_INC_CODEPOINT(entry)        ((entry) & 0x00ffffff)
#define UCD_INC_CATEGORY(entry)         ((entry) >> 24)

/* .. prmc .................................................................. */

struct ucd_prmc_entry {
  uint32_t starter;
  uint32_t composing;
  uint32_t composite;
};

struct ucd_prmc {
  uint32_t              num_entries;
  struct ucd_prmc_entry entries[0];     // Stored in sorted order
};

#pragma pack(pop)

#endif /* UCD_FORMAT_H_ */
