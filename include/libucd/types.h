/*
 * libucd - Unicode database library
 *
 * Copyright (c) 2015 Alastair Houghton
 *
 */

#ifndef LIBUCD_TYPES_H_
#define LIBUCD_TYPES_H_

#include <cinttypes>
#include <vector>

namespace ucd {

  typedef uint32_t codepoint;
  typedef uint16_t gc;
  typedef uint8_t  ccc;

  typedef std::vector<codepoint> cpvector;

  // Bad code point value
  enum {
    bad_codepoint = 0xffffffff
  };

  /* maybe is a 3-value logic type

     Truth tables:

     AND   |   no  maybe  yes
    -------+-------------------
     no    |   no    no    no
     maybe |   no  maybe maybe
     yes   |   no  maybe  yes

     OR    |   no  maybe  yes
    -------+-------------------
     no    |   no  maybe  yes
     maybe | maybe maybe  yes
     yes   |  yes   yes   yes

     NOT   |   no  maybe  yes
    -------+------------------
           |  yes  maybe   no

  */
  enum class maybe {
    no = 0,
    maybe = 1,
    yes = 2
  };

  static inline maybe operator || (maybe a, maybe b) {
    return a > b ? a : b;
  }
  static inline maybe operator && (maybe a, maybe b) {
    return a < b ? a : b;
  }
  static inline maybe operator ! (maybe a) {
    return maybe(2 - int(a));
  }

  // Alias kinds
  namespace Alias_Type {
    typedef enum {
      none         = 0x00,

      correction   = 0x01,
      control      = 0x02,
      alternate    = 0x04,
      figment      = 0x08,
      abbreviation = 0x10,

      all          = 0x1f
    } Enum;
  }

  typedef Alias_Type::Enum at;

  // General_Category values
  namespace General_Category {
    enum {
      Group_Mask  = '\xff\0',

      Letter      = 'L\0',
      Mark        = 'M\0',
      Number      = 'N\0',
      Punctuation = 'P\0',
      Symbol      = 'S\0',
      Separator   = 'Z\0',
      Other       = 'C\0',

      L = Letter,
      M = Mark,
      N = Number,
      P = Punctuation,
      S = Symbol,
      Z = Separator,
      C = Other,

      Lu = 'Lu',        // Letter, uppercase
      Ll = 'Ll',        // Letter, lowercase
      Lt = 'Lt',        // Letter, titlecase
      Lm = 'Lm',        // Letter, modifier
      Lo = 'Lo',        // Letter, other

      Mn = 'Mn',        // Mark, nonspacing
      Mc = 'Mc',        // Mark, spacing combining
      Me = 'Me',        // Mark, enclosing

      Nd = 'Nd',        // Number, decimal digit
      Nl = 'Nl',        // Number, letter
      No = 'No',        // Number, other

      Pc = 'Pc',        // Punctuation, connector
      Pd = 'Pd',        // Punctuation, dash
      Ps = 'Ps',        // Punctuation, open
      Pe = 'Pe',        // Punctuation, close
      Pi = 'Pi',        // Punctuation, initial quote
      Pf = 'Pf',        // Punctuation, final quote
      Po = 'Po',        // Punctuation, other

      Sm = 'Sm',        // Symbol, math
      Sc = 'Sc',        // Symbol, currency
      Sk = 'Sk',        // Symbol, modifier
      So = 'So',        // Symbol, other

      Zs = 'Zs',        // Separator, space
      Zl = 'Zl',        // Separator, line
      Zp = 'Zp',        // Separator, paragraph

      Cc = 'Cc',        // Other, control
      Cf = 'Cf',        // Other, format
      Cs = 'Cs',        // Other, surrogate
      Co = 'Co',        // Other, private use
      Cn = 'Cn',        // Other, not assigned

      // Aliases
      Control = Cc,
      Format = Cf,
      Unassigned = Cn,
      Private_Use = Co,
      Surrogate = Cs,
      Lowercase_Letter = Ll,
      Modifier_Letter = Lm,
      Other_Letter = Lo,
      Titlecase_Letter = Lt,
      Uppercase_Letter = Lu,
      Spacing_Mark = Mc,
      Enclosing_Mark = Me,
      Nonspacing_Mark = Mn,
      Decimal_Number = Nd,
      Letter_Number = Nl,
      Other_Number = No,
      Connection_Punctuation = Pc,
      Dash_Punctuation = Pd,
      Close_Punctuation = Pe,
      Final_Punctuation = Pf,
      Initial_Punctuation = Pi,
      Other_Punctuation = Po,
      Open_Punctuation = Ps,
      Currency_Symbol = Sc,
      Modifier_Symbol = Sk,
      Math_Symbol = Sm,
      Other_Symbol = So,
      Line_Separator = Zl,
      Paragraph_Separator = Zp,
      Space_Separator = Zs
    };

    inline bool is_group(unsigned value) {
      return !(value & ~Group_Mask);
    }
  };

  // Canonical_Combining_Class values
  namespace Canonical_Combining_Class {
    enum {
      Not_Reordered = 0,
      Overlay = 1,
      Nukta = 7,
      Kana_Voicing = 8,
      Virama = 9,
      CCC10 = 10,
      CCC11 = 11,
      CCC12 = 12,
      CCC13 = 13,
      CCC14 = 14,
      CCC15 = 15,
      CCC16 = 16,
      CCC17 = 17,
      CCC18 = 18,
      CCC19 = 19,
      CCC20 = 20,
      CCC21 = 21,
      CCC22 = 22,
      CCC23 = 23,
      CCC24 = 24,
      CCC25 = 25,
      CCC26 = 26,
      CCC27 = 27,
      CCC28 = 28,
      CCC29 = 29,
      CCC30 = 30,
      CCC31 = 31,
      CCC32 = 32,
      CCC33 = 33,
      CCC34 = 34,
      CCC35 = 35,
      CCC36 = 36,
      CCC37 = 37,
      CCC38 = 38,
      CCC39 = 39,
      CCC40 = 40,
      CCC41 = 41,
      CCC42 = 42,
      CCC43 = 43,
      CCC44 = 44,
      CCC45 = 45,
      CCC46 = 46,
      CCC47 = 47,
      CCC48 = 48,
      CCC49 = 49,
      CCC50 = 50,
      CCC51 = 51,
      CCC52 = 52,
      CCC53 = 53,
      CCC54 = 54,
      CCC55 = 55,
      CCC56 = 56,
      CCC57 = 57,
      CCC58 = 58,
      CCC59 = 59,
      CCC60 = 60,
      CCC61 = 61,
      CCC62 = 62,
      CCC63 = 63,
      CCC64 = 64,
      CCC65 = 65,
      CCC66 = 66,
      CCC67 = 67,
      CCC68 = 68,
      CCC69 = 69,
      CCC70 = 70,
      CCC71 = 71,
      CCC72 = 72,
      CCC73 = 73,
      CCC74 = 74,
      CCC75 = 75,
      CCC76 = 76,
      CCC77 = 77,
      CCC78 = 78,
      CCC79 = 79,
      CCC80 = 80,
      CCC81 = 81,
      CCC82 = 82,
      CCC83 = 83,
      CCC84 = 84,
      CCC85 = 85,
      CCC86 = 86,
      CCC87 = 87,
      CCC88 = 88,
      CCC89 = 89,
      CCC90 = 90,
      CCC91 = 91,
      CCC92 = 92,
      CCC93 = 93,
      CCC94 = 94,
      CCC95 = 95,
      CCC96 = 96,
      CCC97 = 97,
      CCC98 = 98,
      CCC99 = 99,
      CCC100 = 100,
      CCC101 = 101,
      CCC102 = 102,
      CCC103 = 103,
      CCC104 = 104,
      CCC105 = 105,
      CCC106 = 106,
      CCC107 = 107,
      CCC108 = 108,
      CCC109 = 109,
      CCC110 = 110,
      CCC111 = 111,
      CCC112 = 112,
      CCC113 = 113,
      CCC114 = 114,
      CCC115 = 115,
      CCC116 = 116,
      CCC117 = 117,
      CCC118 = 118,
      CCC119 = 119,
      CCC120 = 120,
      CCC121 = 121,
      CCC122 = 122,
      CCC123 = 123,
      CCC124 = 124,
      CCC125 = 125,
      CCC126 = 126,
      CCC127 = 127,
      CCC128 = 128,
      CCC129 = 129,
      CCC130 = 130,
      CCC131 = 131,
      CCC132 = 132,
      CCC133 = 133,
      CCC134 = 134,
      CCC135 = 135,
      CCC136 = 136,
      CCC137 = 137,
      CCC138 = 138,
      CCC139 = 139,
      CCC140 = 140,
      CCC141 = 141,
      CCC142 = 142,
      CCC143 = 143,
      CCC144 = 144,
      CCC145 = 145,
      CCC146 = 146,
      CCC147 = 147,
      CCC148 = 148,
      CCC149 = 149,
      CCC150 = 150,
      CCC151 = 151,
      CCC152 = 152,
      CCC153 = 153,
      CCC154 = 154,
      CCC155 = 155,
      CCC156 = 156,
      CCC157 = 157,
      CCC158 = 158,
      CCC159 = 159,
      CCC160 = 160,
      CCC161 = 161,
      CCC162 = 162,
      CCC163 = 163,
      CCC164 = 164,
      CCC165 = 165,
      CCC166 = 166,
      CCC167 = 167,
      CCC168 = 168,
      CCC169 = 169,
      CCC170 = 170,
      CCC171 = 171,
      CCC172 = 172,
      CCC173 = 173,
      CCC174 = 174,
      CCC175 = 175,
      CCC176 = 176,
      CCC177 = 177,
      CCC178 = 178,
      CCC179 = 179,
      CCC180 = 180,
      CCC181 = 181,
      CCC182 = 182,
      CCC183 = 183,
      CCC184 = 184,
      CCC185 = 185,
      CCC186 = 186,
      CCC187 = 187,
      CCC188 = 188,
      CCC189 = 189,
      CCC190 = 190,
      CCC191 = 191,
      CCC192 = 192,
      CCC193 = 193,
      CCC194 = 194,
      CCC195 = 195,
      CCC196 = 196,
      CCC197 = 197,
      CCC198 = 198,
      CCC199 = 199,
      Attached_Below_Left = 200,
      Attached_Below = 202,
      Attached_Above = 214,
      Attached_Above_Right = 216,
      Below_Left = 218,
      Below = 220,
      Below_Right = 222,
      Left = 224,
      Right = 226,
      Above_Left = 228,
      Above = 230,
      Above_Right = 232,
      Double_Below = 233,
      Double_Above = 234,
      Iota_Subscript = 240,
    };
  };

  // Hangul_Syllable_Type values
  enum class Hangul_Syllable_Type {
    NA = 0,
    L = 1,
    V = 2,
    T = 4,
    LV = 3,
    LVT = 7,

    // Aliases
    Not_Applicable = NA,
    Leading_Jamo = L,
    Vowel_Jamo = V,
    Trailing_Jamo = T,
    LV_Syllable = LV,
    LVT_Syllable = LVT
  };

  // Numeric_Type values
  enum class Numeric_Type {
    None = 0,
    Decimal = 1,
    Digit = 2,
    Numeric = 3,
  };

  // Bidi_Class values
  enum class Bidi_Class {
    L = 1,
    R = 2,
    AL = 3,
    EN = 4,
    ES = 5,
    ET = 6,
    AN = 7,
    CS = 8,
    NSM = 9,
    BN = 10,
    B = 11,
    S = 12,
    WS = 13,
    ON = 14,
    LRE = 15,
    LRO = 16,
    RLE = 17,
    RLO = 18,
    PDF = 19,
    LRI = 20,
    RLI = 21,
    FSI = 22,
    PDI = 23,

    // Aliases
    Arabic_Letter = AL,
    Arabic_Number = AN,
    Paragraph_Separator = B,
    Boundary_Neutral = BN,
    Common_Separator = CS,
    European_Number = EN,
    European_Separator = ES,
    European_Terminator = ET,
    First_Strong_Isolate = FSI,
    Left_To_Right = L,
    Left_To_Right_Embedding = LRE,
    Left_To_Right_Isolate = LRI,
    Left_To_Right_Override = LRO,
    Nonspacing_Mark = NSM,
    Other_Neutral = ON,
    Pop_Directional_Format = PDF,
    Pop_Directional_Isolate = PDI,
    Right_To_Left = R,
    Right_To_Left_Embedding = RLE,
    Right_To_Left_Isolate = RLI,
    Right_To_Left_Override = RLO,
    Segment_Separator = S,
    White_Space = WS
  };

  // Decomposition_Type values
  enum class Decomposition_Type {
    None = -1,
    Can = 0,
    Font = 1,
    Nb = 2,
    Init = 3,
    Med = 4,
    Fin = 5,
    Iso = 6,
    Enc = 7,
    Sup = 8,
    Sub = 9,
    Vert = 10,
    Wide = 11,
    Nar = 12,
    Sml = 13,
    Sqr = 14,
    Fra = 15,
    Com = 16,

    // Aliases
    Canonical = Can,
    Compat = Com,
    Circle = Enc,
    Final = Fin,
    Fraction = Fra,
    Initial = Init,
    Isolated = Iso,
    Medial = Med,
    Narrow = Nar,
    Nobreak = Nb,
    Small = Sml,
    Square = Sqr,
    Super = Sup,
    Vertical = Vert,

    can = Can,
    com = Com,
    enc = Enc,
    fin = Fin,
    font = Font,
    fra = Fra,
    init = Init,
    iso = Iso,
    med = Med,
    nar = Nar,
    nb = Nb,
    none = None,
    sml = Sml,
    sqr = Sqr,
    sub = Sub,
    sup = Sup,
    vert = Vert,
    wide = Wide,
  };

  namespace Script {
    enum {
      Aghb = 'Aghb',
      Arab = 'Arab',
      Armi = 'Armi',
      Armn = 'Armn',
      Avst = 'Avst',
      Bali = 'Bali',
      Bamu = 'Bamu',
      Bass = 'Bass',
      Batk = 'Batk',
      Beng = 'Beng',
      Bopo = 'Bopo',
      Brah = 'Brah',
      Brai = 'Brai',
      Bugi = 'Bugi',
      Buhd = 'Buhd',
      Cakm = 'Cakm',
      Cans = 'Cans',
      Cari = 'Cari',
      Cham = 'Cham',
      Cher = 'Cher',
      Copt = 'Copt',
      Cprt = 'Cprt',
      Cyrl = 'Cyrl',
      Deva = 'Deva',
      Dsrt = 'Dsrt',
      Dupl = 'Dupl',
      Egyp = 'Egyp',
      Elba = 'Elba',
      Ethi = 'Ethi',
      Geor = 'Geor',
      Glag = 'Glag',
      Goth = 'Goth',
      Gran = 'Gran',
      Grek = 'Grek',
      Gujr = 'Gujr',
      Guru = 'Guru',
      Hang = 'Hang',
      Hani = 'Hani',
      Hano = 'Hano',
      Hebr = 'Hebr',
      Hira = 'Hira',
      Hmng = 'Hmng',
      Hrkt = 'Hrkt',
      Ital = 'Ital',
      Java = 'Java',
      Kali = 'Kali',
      Kana = 'Kana',
      Khar = 'Khar',
      Khmr = 'Khmr',
      Khoj = 'Khoj',
      Knda = 'Knda',
      Kthi = 'Kthi',
      Lana = 'Lana',
      Laoo = 'Laoo',
      Latn = 'Latn',
      Lepc = 'Lepc',
      Limb = 'Limb',
      Lina = 'Lina',
      Linb = 'Linb',
      Lisu = 'Lisu',
      Lyci = 'Lyci',
      Lydi = 'Lydi',
      Mahj = 'Mahj',
      Mand = 'Mand',
      Mani = 'Mani',
      Mend = 'Mend',
      Merc = 'Merc',
      Mero = 'Mero',
      Mlym = 'Mlym',
      Modi = 'Modi',
      Mong = 'Mong',
      Mroo = 'Mroo',
      Mtei = 'Mtei',
      Mymr = 'Mymr',
      Narb = 'Narb',
      Nbat = 'Nbat',
      Nkoo = 'Nkoo',
      Ogam = 'Ogam',
      Olck = 'Olck',
      Orkh = 'Orkh',
      Orya = 'Orya',
      Osma = 'Osma',
      Palm = 'Palm',
      Pauc = 'Pauc',
      Perm = 'Perm',
      Phag = 'Phag',
      Phli = 'Phli',
      Phlp = 'Phlp',
      Phnx = 'Phnx',
      Plrd = 'Plrd',
      Prti = 'Prti',
      Rjng = 'Rjng',
      Runr = 'Runr',
      Samr = 'Samr',
      Sarb = 'Sarb',
      Saur = 'Saur',
      Shaw = 'Shaw',
      Shrd = 'Shrd',
      Sidd = 'Sidd',
      Sind = 'Sind',
      Sinh = 'Sinh',
      Sora = 'Sora',
      Sund = 'Sund',
      Sylo = 'Sylo',
      Syrc = 'Syrc',
      Tagb = 'Tagb',
      Takr = 'Takr',
      Tale = 'Tale',
      Talu = 'Talu',
      Taml = 'Taml',
      Tavt = 'Tavt',
      Telu = 'Telu',
      Tfng = 'Tfng',
      Tglg = 'Tglg',
      Thaa = 'Thaa',
      Thai = 'Thai',
      Tibt = 'Tibt',
      Tirh = 'Tirh',
      Ugar = 'Ugar',
      Vaii = 'Vaii',
      Wara = 'Wara',
      Xpeo = 'Xpeo',
      Xsux = 'Xsux',
      Yiii = 'Yiii',
      Zinh = 'Zinh',
      Zyyy = 'Zyyy',
      Zzzz = 'Zzzz',

      // Aliases
      Caucasian_Albanian = Aghb,
      Arabic = Arab,
      Imperial_Aramaic = Armi,
      Armenian = Armn,
      Avestan = Avst,
      Balinese = Bali,
      Bamum = Bamu,
      Bassa_Vah = Bass,
      Batak = Batk,
      Bengali = Beng,
      Bopomofo = Bopo,
      Brahmi = Brah,
      Braille = Brai,
      Buginese = Bugi,
      Buhid = Buhd,
      Chakma = Cakm,
      Canadian_Aboriginal = Cans,
      Carian = Cari,
      Cherokee = Cher,
      Coptic = Copt,
      Cypriot = Cprt,
      Cyrillic = Cyrl,
      Devanagari = Deva,
      Deseret = Dsrt,
      Duployan = Dupl,
      Egyptian_Hieroglyphs = Egyp,
      Elbasan = Elba,
      Ethiopic = Ethi,
      Georgian = Geor,
      Glagolitic = Glag,
      Gothic = Goth,
      Grantha = Gran,
      Greek = Grek,
      Gujarati = Gujr,
      Gurmukhi = Guru,
      Hangul = Hang,
      Han = Hani,
      Hanunoo = Hano,
      Hebrew = Hebr,
      Hiragana = Hira,
      Pahawh_Hmong = Hmng,
      Katakana_Or_Hiragana = Hrkt,
      Old_Italic = Ital,
      Javanese = Java,
      Kayah_Li = Kali,
      Katakana = Kana,
      Kharoshthi = Khar,
      Khmer = Khmr,
      Khojki = Khoj,
      Kannada = Knda,
      Kaithi = Kthi,
      Tai_Tham = Lana,
      Lao = Laoo,
      Latin = Latn,
      Lepcha = Lepc,
      Limbu = Limb,
      Linear_A = Lina,
      Linear_B = Linb,
      Lycian = Lyci,
      Lydian = Lydi,
      Mahajani = Mahj,
      Mandaic = Mand,
      Manichaean = Mani,
      Mende_Kikakui = Mend,
      Meroitic_Cursive = Merc,
      Meroitic_Hieroglyphs = Mero,
      Malayalam = Mlym,
      Mongolian = Mong,
      Mro = Mroo,
      Meetei_Mayek = Mtei,
      Myanmar = Mymr,
      Old_North_Arabian = Narb,
      Nabataean = Nbat,
      Nko = Nkoo,
      Ogham = Ogam,
      Ol_Chiki = Olck,
      Old_Turkic = Orkh,
      Oriya = Orya,
      Osmanya = Osma,
      Palmyrene = Palm,
      Pau_Cin_Hau = Pauc,
      Old_Permic = Perm,
      Phags_Pa = Phag,
      Inscriptional_Pahlavi = Phli,
      Psalter_Pahlavi = Phlp,
      Phoenician = Phnx,
      Miao = Plrd,
      Inscriptional_Parthian = Prti,
      Rejang = Rjng,
      Runic = Runr,
      Samaritan = Samr,
      Old_South_Arabian = Sarb,
      Saurashtra = Saur,
      Shavian = Shaw,
      Sharada = Shrd,
      Siddham = Sidd,
      Khudawadi = Sind,
      Sinhala = Sinh,
      Sora_Sompeng = Sora,
      Sundanese = Sund,
      Syloti_Nagri = Sylo,
      Syriac = Syrc,
      Tagbanwa = Tagb,
      Takri = Takr,
      Tai_Le = Tale,
      New_Tai_Lue = Talu,
      Tamil = Taml,
      Tai_Viet = Tavt,
      Telugu = Telu,
      Tifinagh = Tfng,
      Tagalog = Tglg,
      Thaana = Thaa,
      Tibetan = Tibt,
      Tirhuta = Tirh,
      Ugaritic = Ugar,
      Vai = Vaii,
      Warang_Citi = Wara,
      Old_Persian = Xpeo,
      Cuneiform = Xsux,
      Yi = Yiii,
      Inherited = Zinh,
      Common = Zyyy,
      Unknown = Zzzz,

      Qaac = Copt,
      Qaai = Zinh
    };
  }

  enum class Bidi_Paired_Bracket_Type {
    None = 0,
    Open = 1,
    Close = 2,

    // Aliases
    n = None,
    o = Open,
    c = Close
  };

  enum class Joining_Type {
    Non_Joining = 0,
    Transparent = 1,
    Right_Joining = 2,
    Left_Joining = 3,
    Dual_Joining = 4,
    Join_Causing = 5,

    // Aliases
    U = Non_Joining,
    T = Transparent,
    R = Right_Joining,
    L = Left_Joining,
    D = Dual_Joining,
    C = Join_Causing
  };

  enum class Joining_Group {
    No_Joining_Group = 0,
    Ain = 1,
    Alaph = 2,
    Alef = 3,
    Beh = 4,
    Beth = 5,
    Burushaski_Yeh_Barree = 6,
    Dal = 7,
    Dalath_Rish = 8,
    E = 9,
    Farsi_Yeh = 10,
    Fe = 11,
    Feh = 12,
    Final_Semkath = 13,
    Gaf = 14,
    Gamal = 15,
    Hah = 16,
    He = 17,
    Heh = 18,
    Heh_Goal = 19,
    Heth = 20,
    Kaf = 21,
    Kaph = 22,
    Khaph = 23,
    Knotted_Heh = 24,
    Lam = 25,
    Lamadh = 26,
    Manichaean_Aleph = 27,
    Manichaean_Ayin = 28,
    Manichaean_Beth = 29,
    Manichaean_Daleth = 30,
    Manichaean_Dhamedh = 31,
    Manichaean_Five = 32,
    Manichaean_Gimel = 33,
    Manichaean_Heth = 34,
    Manichaean_Hundred = 35,
    Manichaean_Kaph = 36,
    Manichaean_Lamedh = 37,
    Manichaean_Mem = 38,
    Manichaean_Nun = 39,
    Manichaean_One = 40,
    Manichaean_Pe = 41,
    Manichaean_Qoph = 42,
    Manichaean_Resh = 43,
    Manichaean_Sadhe = 44,
    Manichaean_Samekh = 45,
    Manichaean_Taw = 46,
    Manichaean_Ten = 47,
    Manichaean_Teth = 48,
    Manichaean_Thamedh = 49,
    Manichaean_Twenty = 50,
    Manichaean_Waw = 51,
    Manichaean_Yodh = 52,
    Manichaean_Zayin = 53,
    Meem = 54,
    Mim = 55,
    Noon = 56,
    Nun = 57,
    Nya = 58,
    Pe = 59,
    Qaf = 60,
    Qaph = 61,
    Reh = 62,
    Reversed_Pe = 63,
    Rohingya_Yeh = 64,
    Sad = 65,
    Sadhe = 66,
    Seen = 67,
    Semkath = 68,
    Shin = 69,
    Straight_Waw = 70,
    Swash_Kaf = 71,
    Syriac_Waw = 72,
    Tah = 73,
    Taw = 74,
    Teh_Marbuta = 75,
    Teh_Marbuta_Goal = 76,
    Teth = 77,
    Waw = 78,
    Yeh = 79,
    Yeh_Barree = 80,
    Yeh_With_Tail = 81,
    Yudh = 82,
    Yudh_He = 83,
    Zain = 84,
    Zhain = 85,

    // Unicode 9
    African_Feh = 86,
    African_Qaf = 87,
    African_Noon = 88,

    // Aliases
    Hamza_On_Heh_Goal = Teh_Marbuta_Goal,
  };

  enum class Line_Break {
    XX = 0,

    // Normative
    BK = 1,
    CR = 2,
    LF = 3,
    CM = 4,
    SG = 5,
    GL = 6,
    CB = 7,
    SP = 8,
    ZW = 9,
    NL = 10,
    WJ = 11,
    JL = 12,
    JV = 13,
    JT = 14,
    H2 = 15,
    H3 = 16,

    // Informative
    OP = 17,
    CL = 18,
    CP = 19,
    QU = 20,
    NS = 21,
    EX = 22,
    SY = 23,
    IS = 24,
    PR = 25,
    PO = 26,
    NU = 27,
    AL = 28,
    ID = 29,
    IN = 30,
    HY = 31,
    BB = 32,
    BA = 33,
    SA = 34,
    AI = 35,
    B2 = 36,
    HL = 37,
    CJ = 38,
    RI = 39,

    // Unicode 9
    ZWJ = 40,
    EB = 41,
    EM = 42,

    // Aliases
    Ambiguous = AI,
    Alphabetic = AL,
    Break_Both = B2,
    Break_After = BA,
    Break_Before = BB,
    Mandatory_Break = BK,
    Contingent_Break = CB,
    Conditional_Japanese_Starter = CJ,
    Close_Punctuation = CL,
    Combining_Mark = CM,
    Close_Parenthesis = CP,
    Carriage_Return = CR,
    Exclamation = EX,
    Glue = GL,
    Hebrew_Letter = HL,
    Hyphen = HY,
    Ideographic = ID,
    Inseparable = IN,
    Inseperable = Inseparable,        // Spelling mistake intentional
    Infix_Numeric = IS,
    Line_Feed = LF,
    Next_Line = NL,
    Nonstarter = NS,
    Numeric = NU,
    Open_Punctuation = OP,
    Postfix_Numeric = PO,
    Prefix_Numeric = PR,
    Quotation = QU,
    Regional_Indicator = RI,
    Complex_Context = SA,
    Surrogate = SG,
    Space = SP,
    Break_Symbols = SY,
    Word_Joiner = WJ,
    Unknown = XX,
    ZWSpace = ZW,
    Zero_Width_Joiner = ZWJ,
  };

  enum class Grapheme_Cluster_Break {
    XX = 0,

    CN = 1,
    CR = 2,
    EX = 3,
    L = 4,
    LF = 5,
    LV = 6,
    LVT = 7,
    PP = 8,
    RI = 9,
    SM = 10,
    T = 11,
    V = 12,

    // Unicode 9
    EB = 13,
    EM = 14,
    GAZ = 15,
    EBG = 16,
    ZWJ = 17,

    // Aliases
    Control = CN,
    Extend = EX,
    Prepend = PP,
    Regional_Indicator = RI,
    SpacingMark = SM,
    Other = XX,
    E_Base = EB,
    E_Modifier = EM,
    Glue_After_Zwj = GAZ,
    E_Base_GAZ = EBG,
  };

  enum class Sentence_Break {
    XX = 0,

    AT = 1,
    CL = 2,
    CR = 3,
    EX = 4,
    FO = 5,
    LE = 6,
    LF = 7,
    LO = 8,
    NU = 9,
    SC = 10,
    SE = 11,
    SP = 12,
    ST = 13,
    UP = 14,

    // Aliases
    ATerm = AT,
    Close = CL,
    Extend = EX,
    Format = FO,
    OLetter = LE,
    Lower = LO,
    Numeric = NU,
    SContinue = SC,
    Sep = SE,
    Sp = SP,
    STerm = ST,
    Upper = UP,
    Other = XX,
  };

  enum class Word_Break {
    XX = 0,

    CR = 1,
    DQ = 2,
    EX = 3,
    Extend = 4,
    FO = 5,
    HL = 6,
    KA = 7,
    LE = 8,
    LF = 9,
    MB = 10,
    ML = 11,
    MN = 12,
    NL = 13,
    NU = 14,
    RI = 15,
    SQ = 16,

    // Unicode 9
    EB = 17,
    EM = 18,
    GAZ = 19,
    EBG = 20,
    ZWJ = 21,

    // Aliases
    Double_Quote = DQ,
    ExtendNumLet = EX,
    Format = FO,
    Hebrew_Letter = HL,
    Katakana = KA,
    ALetter = LE,
    MidNumLet = MB,
    MidLetter = ML,
    MidNum = MN,
    Newline = NL,
    Numeric = NU,
    Regional_Indicator = RI,
    Single_Quote = SQ,
    Other = XX,
    E_Base = EB,
    E_Modifier = EM,
    Glue_After_Zwj = GAZ,
    E_Base_GAZ = EBG,
  };

  enum class East_Asian_Width {
    N = 0,
    A = 1,
    F = 2,
    H = 3,
    Na = 4,
    W = 5,

    // Aliases
    Ambiguous = A,
    Fullwidth = F,
    Halfwidth = H,
    Neutral = N,
    Narrow = Na,
    Wide = W
  };

  enum class Indic_Positional_Category {
    NA = 0,
    Bottom = 1,
    Bottom_And_Right = 2,
    Left = 3,
    Left_And_Right = 4,
    Overstruck = 5,
    Right = 6,
    Top = 7,
    Top_And_Bottom = 8,
    Top_And_Bottom_And_Right = 9,
    Top_And_Left = 10,
    Top_And_Left_And_Right = 11,
    Top_And_Right = 12,
    Visual_Order_Left = 13
  };

  // Renamed in Unicode 8
  typedef Indic_Positional_Category Indic_Matra_Category;

  enum class Indic_Syllabic_Category {
    Other = 0,
    Avagraha = 1,
    Bindu = 2,
    Brahmi_Joining_Number = 3,
    Cantillation_Mark = 4,
    Consonant = 5,
    Consonant_Dead = 6,
    Consonant_Final = 7,
    Consonant_Head_Letter = 8,
    Consonant_Medial = 9,
    Consonant_Placeholder = 10,
    Consonant_Preceding_Repha = 11,
    Consonant_Subjoined = 12,
    Consonant_Succeeding_Repha = 13,
    Gemination_Mark = 14,
    Invisible_Stacker = 15,
    Joiner = 16,
    Modifying_Letter = 17,
    Non_Joiner = 18,
    Nukta = 19,
    Number = 20,
    Number_Joiner = 21,
    Pure_Killer = 22,
    Register_Shifter = 23,
    Tone_Letter = 24,
    Tone_Mark = 25,
    Virama = 26,
    Visarga = 27,
    Vowel = 28,
    Vowel_Dependent = 29,
    Vowel_Independent = 30,

    // Unicode 8
    Consonant_Prefixed = 31,
    Consonant_With_Stacker = 32,
    Consonant_Killer = 33,
    Syllable_Modifier = 34,
  };

  typedef Indic_Matra_Category InMC;
  typedef Indic_Positional_Category InPC;
  typedef Indic_Syllabic_Category InSC;
  typedef East_Asian_Width ea;
  typedef Word_Break WB;
  typedef Sentence_Break SB;
  typedef Grapheme_Cluster_Break GCB;
  typedef Line_Break lb;
  typedef Joining_Group jg;
  typedef Joining_Type jt;
  typedef Bidi_Paired_Bracket_Type bpt;
  typedef uint32_t sc;
  typedef Numeric_Type nt;
  typedef Hangul_Syllable_Type hst;
  typedef Decomposition_Type dt;
  typedef Bidi_Class bc;

  namespace GC = General_Category;
  namespace CCC = Canonical_Combining_Class;
}

#endif /* LIBUCD_TYPES_H_ */

/*
 * Local Variables:
 * mode: c++
 * End:
 *
 */
