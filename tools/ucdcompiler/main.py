#
#  ucdc
#
#  Compiles UCD.zip/Unihan.zip into a binary .ucd file suitable for use
#  with libucd.
#
#  The format of the file is defined in src/ucd-format.h.
#
#

###TODO: This file is a big ad-hoc mess and could do with a good tidy

from __future__ import print_function
from __future__ import unicode_literals
import zipfile
import os.path
import struct
import re
import operator

from .rangeset import RangeSet
from .sparsearray import SparseArray

def fourcc(s):
    return struct.unpack(b'!I', s.encode('ascii'))[0]

def twocc(s):
    return struct.unpack(b'!H', s.encode('ascii'))[0]

UCD_MAGIC = fourcc('ucd1')
UCD_blok = fourcc('blok')
UCD_strn = fourcc('strn')
UCD_name = fourcc('name')
UCD_u1nm = fourcc('u1nm')
UCD_isoc = fourcc('isoc')
UCD_alis = fourcc('alis')
UCD_genc = fourcc('genc')
UCD_CASE = fourcc('CASE')
UCD_case = fourcc('case')
UCD_Case = fourcc('Case')
UCD_csef = fourcc('csef')
UCD_kccf = fourcc('kccf')
UCD_nfkc = fourcc('nfkc')
UCD_ccc = fourcc('ccc ')
UCD_jamo = fourcc('jamo')
UCD_numb = fourcc('numb')
UCD_bidi = fourcc('bidi')
UCD_mirr = fourcc('mirr')
UCD_brak = fourcc('brak')
UCD_deco = fourcc('deco')
UCD_age = fourcc('age ')
UCD_scpt = fourcc('scpt')
UCD_cqc = fourcc('cqc ')
UCD_kcqc = fourcc('kcqc')
UCD_dqc = fourcc('dqc ')
UCD_kdqc = fourcc('kdqc')
UCD_join = fourcc('join')
UCD_lbrk = fourcc('lbrk')
UCD_gbrk = fourcc('gbrk')
UCD_sbrk = fourcc('sbrk')
UCD_wbrk = fourcc('wbrk')
UCD_eaw = fourcc('eaw ')
UCD_rads = fourcc('rads')
UCD_inmc = fourcc('inmc')
UCD_insc = fourcc('insc')
UCD_prmc = fourcc('prmc')

binprop_tables = [
    # Proplist
    ('ASCII_Hex_Digit', 'AHD?'),
    ('Bidi_Control', 'BCt?'),
    ('Dash', 'Dsh?'),
    ('Deprecated', 'Dep?'),
    ('Diacritic', 'Dia?'),
    ('Extender', 'Ext?'),
    ('Hex_Digit', 'Hex?'),
    ('Hyphen', 'Hyp?'),
    ('Ideographic', 'Ido?'),
    ('IDS_Binary_Operator', 'IBO?'),
    ('IDS_Trinary_Operator', 'ITO?'),
    ('Join_Control', 'JCt?'),
    ('Logical_Order_Exception', 'LOE?'),
    ('Noncharacter_Code_Point', 'NCP?'),
    ('Other_Alphabetic', 'OAl?'),
    ('Other_Default_Ignorable_Code_Point', 'ODI?'),
    ('Other_Grapheme_Extend', 'OGE?'),
    ('Other_ID_Continue', 'OIC?'),
    ('Other_ID_Start', 'OIS?'),
    ('Other_Lowercase', 'OLc?'),
    ('Other_Math', 'OMa?'),
    ('Other_Uppercase', 'OUc?'),
    ('Pattern_Syntax', 'PSy?'),
    ('Pattern_White_Space', 'PWS?'),
    ('Quotation_Mark', 'QMa?'),
    ('Radical', 'Rad?'),
    ('Soft_Dotted', 'SDt?'),
    (('STerm', 'Sentence_Terminal'), 'Stm?'),
    ('Terminal_Punctuation', 'TPu?'),
    ('Unified_Ideograph', 'UId?'),
    ('Variation_Selector', 'VSl?'),
    ('White_Space', 'WSp?'),

    # DerivedCoreProperties
    ('Lowercase', 'Lca?'),
    ('Uppercase', 'Uca?'),
    ('Cased', 'Cse?'),
    ('Case_Ignorable', 'CIg?'),
    ('Changes_When_Lowercased', 'CWL?'),
    ('Changes_When_Uppercased', 'CWU?'),
    ('Changes_When_Titlecased', 'CWT?'),
    ('Changes_When_Casefolded', 'CWC?'),
    ('Changes_When_Casemapped', 'CWM?'),
    ('Alphabetic', 'Alp?'),
    ('Default_Ignorable_Code_Point', 'DIC?'),
    ('Grapheme_Base', 'GBa?'),
    ('Grapheme_Extend', 'GEx?'),
    ('Grapheme_Link', 'GLi?'),
    ('Math', 'Mth?'),
    ('ID_Start', 'IDS?'),
    ('ID_Continue', 'IDC?'),
    ('XID_Start', 'XID?'),
    ('XID_Continue', 'XIC?'),

    # CompositionExclusions
    ('Composition_Exclusion', 'CEx?'),
    
    # DerivedNormalizationProps
    ('Full_Composition_Exclusion', 'FCx?'),
    ('Expands_On_NFD', 'xND?'),
    ('Expands_On_NFC', 'xNC?'),
    ('Expands_On_NFKD', 'xNd?'),
    ('Expands_On_NFKC', 'xNc?'),
    ('Changes_When_NFKC_Casefolded', 'CWc?'),

    # Emoji Data
    ('Emoji', 'Emj?'),
    ('Emoji_Presentation', 'EmP?'),
    ('Emoji_Modifier', 'EmM?'),
    ('Emoji_Modifier_Base', 'EmB?'),
    
    ]

optional_binprops = set(['Emoji',
                         'Emoji_Presentation',
                         'Emoji_Modifier',
                         'Emoji_Modifier_Base'])

UCD_ALIS_KIND_CORRECTION   = 0x01
UCD_ALIS_KIND_CONTROL      = 0x02
UCD_ALIS_KIND_ALTERNATE    = 0x04
UCD_ALIS_KIND_FIGMENT      = 0x08
UCD_ALIS_KIND_ABBREVIATION = 0x10

# N.B. Some of these have different values here (we've folded in the <<24)
#      when compared to those shown in ucd-format.h.

UCD_CASE_RANGE_TYPEMASK    = 0xff000000
UCD_CASE_RANGE_SINGLE      = 0x00000000
UCD_CASE_RANGE_DELTA       = 0x01000000
UCD_CASE_RANGE_EVEN        = 0x02000000
UCD_CASE_RANGE_ODD         = 0x03000000
UCD_CASE_RANGE_EVEN_UP     = 0x04000000
UCD_CASE_RANGE_ODD_DOWN    = 0x05000000
UCD_CASE_RANGE_PACKED      = 0x06000000
UCD_CASE_RANGE_EXTERNAL    = 0x07000000
UCD_CASE_RANGE_TABLE       = 0x08000000
UCD_CASE_RANGE_UCS4TBL     = 0x09000000
UCD_CASE_RANGE_UCS2T3      = 0x0a000000
UCD_CASE_RANGE_UCS2T4      = 0x0b000000
UCD_CASE_RANGE_SF_PACKED   = 0x0c000000
UCD_CASE_RANGE_SF_EXTERNAL = 0x0d000000
UCD_CASE_RANGE_EMPTY       = 0x0e000000

UCD_CCC_RANGE_TYPEMASK = 0xff000000
UCD_CCC_RANGE_RUN      = 0x00000000
UCD_CCC_RANGE_INLINE   = 0x01000000
UCD_CCC_RANGE_TABLE    = 0x02000000

UCD_NAME_RANGE_CJK_UNIFIED_IDEOGRAPH = 1
UCD_NAME_RANGE_CJK_COMPATIBILITY_IDEOGRAPH = 2
UCD_NAME_RANGE_HANGUL_SYLLABLE = 3

UCD_NUMERIC_TYPE_DECIMAL = 0x01000000
UCD_NUMERIC_TYPE_DIGIT   = 0x02000000
UCD_NUMERIC_TYPE_NUMERIC = 0x03000000

UCD_QUICK_CHECK_NO    = 0x00000000
UCD_QUICK_CHECK_MAYBE = 0x01000000
UCD_QUICK_CHECK_YES   = 0x02000000

UCD_DECO_RANGE_SINGLE = 0
UCD_DECO_RANGE_PACKED = 1
UCD_DECO_RANGE_EXTERNAL = 2

UCD_BRAK_ENTRY_NONE  = 0x00000000
UCD_BRAK_ENTRY_OPEN  = 0x01000000
UCD_BRAK_ENTRY_CLOSE = 0x02000000

UCD_JOIN_TYPE_NON_JOINING  = 0
UCD_JOIN_TYPE_TRANSPARENT  = 1
UCD_JOIN_TYPE_RIGHT        = 2
UCD_JOIN_TYPE_LEFT         = 3
UCD_JOIN_TYPE_DUAL         = 4
UCD_JOIN_TYPE_JOIN_CAUSING = 5

joining_type_map = {
    'R': UCD_JOIN_TYPE_RIGHT,
    'L': UCD_JOIN_TYPE_LEFT,
    'D': UCD_JOIN_TYPE_DUAL,
    'C': UCD_JOIN_TYPE_JOIN_CAUSING,
    'U': UCD_JOIN_TYPE_NON_JOINING,
    'T': UCD_JOIN_TYPE_TRANSPARENT
}

joining_group_map = {
    "No_Joining_Group": 0,
    "Ain": 1,
    "Alaph": 2,
    "Alef": 3,
    "Beh": 4,
    "Beth": 5,
    "Burushaski_Yeh_Barree": 6,
    "Dal": 7,
    "Dalath_Rish": 8,
    "E": 9,
    "Farsi_Yeh": 10,
    "Fe": 11,
    "Feh": 12,
    "Final_Semkath": 13,
    "Gaf": 14,
    "Gamal": 15,
    "Hah": 16,
    "He": 17,
    "Heh": 18,
    "Heh_Goal": 19,
    "Heth": 20,
    "Kaf": 21,
    "Kaph": 22,
    "Khaph": 23,
    "Knotted_Heh": 24,
    "Lam": 25,
    "Lamadh": 26,
    "Manichaean_Aleph": 27,
    "Manichaean_Ayin": 28,
    "Manichaean_Beth": 29,
    "Manichaean_Daleth": 30,
    "Manichaean_Dhamedh": 31,
    "Manichaean_Five": 32,
    "Manichaean_Gimel": 33,
    "Manichaean_Heth": 34,
    "Manichaean_Hundred": 35,
    "Manichaean_Kaph": 36,
    "Manichaean_Lamedh": 37,
    "Manichaean_Mem": 38,
    "Manichaean_Nun": 39,
    "Manichaean_One": 40,
    "Manichaean_Pe": 41,
    "Manichaean_Qoph": 42,
    "Manichaean_Resh": 43,
    "Manichaean_Sadhe": 44,
    "Manichaean_Samekh": 45,
    "Manichaean_Taw": 46,
    "Manichaean_Ten": 47,
    "Manichaean_Teth": 48,
    "Manichaean_Thamedh": 49,
    "Manichaean_Twenty": 50,
    "Manichaean_Waw": 51,
    "Manichaean_Yodh": 52,
    "Manichaean_Zayin": 53,
    "Meem": 54,
    "Mim": 55,
    "Noon": 56,
    "Nun": 57,
    "Nya": 58,
    "Pe": 59,
    "Qaf": 60,
    "Qaph": 61,
    "Reh": 62,
    "Reversed_Pe": 63,
    "Rohingya_Yeh": 64,
    "Sad": 65,
    "Sadhe": 66,
    "Seen": 67,
    "Semkath": 68,
    "Shin": 69,
    "Straight_Waw": 70,
    "Swash_Kaf": 71,
    "Syriac_Waw": 72,
    "Tah": 73,
    "Taw": 74,
    "Teh_Marbuta": 75,
    "Teh_Marbuta_Goal": 76,
    "Teth": 77,
    "Waw": 78,
    "Yeh": 79,
    "Yeh_Barree": 80,
    "Yeh_With_Tail": 81,
    "Yudh": 82,
    "Yudh_He": 83,
    "Zain": 84,
    "Zhain": 85,

    # Unicode 9
    "African_Feh": 86,
    "African_Qaf": 87,
    "African_Noon": 88,
}

lbreak_map = {
    "XX": 0,

    "BK": 1,
    "CR": 2,
    "LF": 3,
    "CM": 4,
    "SG": 5,
    "GL": 6,
    "CB": 7,
    "SP": 8,
    "ZW": 9,
    "NL": 10,
    "WJ": 11,
    "JL": 12,
    "JV": 13,
    "JT": 14,
    "H2": 15,
    "H3": 16,

    "OP": 17,
    "CL": 18,
    "CP": 19,
    "QU": 20,
    "NS": 21,
    "EX": 22,
    "SY": 23,
    "IS": 24,
    "PR": 25,
    "PO": 26,
    "NU": 27,
    "AL": 28,
    "ID": 29,
    "IN": 30,
    "HY": 31,
    "BB": 32,
    "BA": 33,
    "SA": 34,
    "AI": 35,
    "B2": 36,
    "HL": 37,
    "CJ": 38,
    "RI": 39,

    # Unicode 9
    "ZWJ": 40,
    "EB": 41,
    "EM": 42,
}

# N.B. We DELIBERATELY map LV to LVT here, which dramatically reduces the size
# of the table in the file.  We can recover the LV values using some maths in
# the grapheme_cluster_break() method.
gbreak_map = {
    "XX": 0, "Other": 0,
    "CN": 1, "Control": 1,
    "CR": 2,
    "EX": 3, "Extend": 3,
    "L": 4,
    "LF": 5,
    "LV": 7,  # THIS IS NOT AN ERROR
    "LVT": 7,
    "PP": 8, "Prepend": 8,
    "RI": 9, "Regional_Indicator": 9,
    "SM": 10, "SpacingMark": 10,
    "T": 11,
    "V": 12,

    # Unicode 9
    "EB": 13, "E_Base": 13,
    "EM": 14, "E_Modifier": 14,
    "GAZ": 15, "Glue_After_Zwj": 15,
    "EBG": 16, "E_Base_GAZ": 16,
    "ZWJ": 17,
}

sbreak_map = {
    'XX': 0, 'Other': 0,
    'AT': 1, 'ATerm': 1,
    'CL': 2, 'Close': 2,
    'CR': 3,
    'EX': 4, 'Extend': 4,
    'FO': 5, 'Format': 5,
    'LE': 6, 'OLetter': 6,
    'LF': 7,
    'LO': 8, 'Lower': 8,
    'NU': 9, 'Numeric': 9,
    'SC': 10, 'SContinue': 10,
    'SE': 11, 'Sep': 11,
    'SP': 12, 'Sp': 12,
    'ST': 13, 'STerm': 13,
    'UP': 14, 'Upper': 14
}

wbreak_map = {
    'XX': 0,
    'CR': 1,
    'DQ': 2, 'Double_Quote': 2,
    'EX': 3, 'ExtendNumLet': 3,
    'Extend': 4,
    'FO': 5, 'Format': 5,
    'HL': 6, 'Hebrew_Letter': 6,
    'KA': 7, 'Katakana': 7,
    'LE': 8, 'ALetter': 8,
    'LF': 9,
    'MB': 10, 'MidNumLet': 10,
    'ML': 11, 'MidLetter': 11,
    'MN': 12, 'MidNum': 12,
    'NL': 13, 'Newline': 13,
    'NU': 14, 'Numeric': 14,
    'RI': 15, 'Regional_Indicator': 15,
    'SQ': 16, 'Single_Quote': 16,

    # Unicode 9
    "EB": 17, "E_Base": 17,
    "EM": 18, "E_Modifier": 18,
    "GAZ": 19, "Glue_After_Zwj": 19,
    "EBG": 20, "E_Base_GAZ": 20,
    "ZWJ": 21,
}

eaw_map = {
    'N': 0, 'Neutral': 0,
    'A': 1, 'Ambiguous': 1,
    'F': 2, 'Fullwidth': 2,
    'H': 3, 'Halfwidth': 3,
    'Na': 4, 'Narrow': 4,
    'W': 5, 'Wide': 5,
}

inmc_map = {
    'NA': 0,
    'Bottom': 1,
    'Bottom_And_Right': 2,
    'Left': 3,
    'Left_And_Right': 4,
    'Overstruck': 5,
    'Right': 6,
    'Top': 7,
    'Top_And_Bottom': 8,
    'Top_And_Bottom_And_Right': 9,
    'Top_And_Left': 10,
    'Top_And_Left_And_Right': 11,
    'Top_And_Right': 12,
    'Visual_Order_Left': 13    
}

insc_map = {
    'Other': 0,
    'Avagraha': 1,
    'Bindu': 2,
    'Brahmi_Joining_Number': 3,
    'Cantillation_Mark': 4,
    'Consonant': 5,
    'Consonant_Dead': 6,
    'Consonant_Final': 7,
    'Consonant_Head_Letter': 8,
    'Consonant_Medial': 9,
    'Consonant_Placeholder': 10,
    'Consonant_Preceding_Repha': 11,
    'Consonant_Subjoined': 12,
    'Consonant_Succeeding_Repha': 13,
    'Gemination_Mark': 14,
    'Invisible_Stacker': 15,
    'Joiner': 16,
    'Modifying_Letter': 17,
    'Non_Joiner': 18,
    'Nukta': 19,
    'Number': 20,
    'Number_Joiner': 21,
    'Pure_Killer': 22,
    'Register_Shifter': 23,
    'Tone_Letter': 24,
    'Tone_Mark': 25,
    'Virama': 26,
    'Visarga': 27,
    'Vowel': 28,
    'Vowel_Dependent': 29,
    'Vowel_Independent': 30,

    # Unicode 8
    'Consonant_Prefixed': 31,
    'Consonant_With_Stacker': 32,
    'Consonant_Killer': 33,
    'Syllable_Modifier': 34,
}
    
def gen_name_table(forward, reverse, special_ranges):
    """Generate the name table."""
    fwd_data = b''.join([struct.pack(b'=II', cp, sid) for cp, sid in forward])
    rev_data = b''.join([struct.pack(b'=II', cp, sid) for n, cp, sid in reverse])

    name_ranges = []
    for name, rng in special_ranges.items():
        if name.startswith('CJK Ideograph'):
            name_ranges.append((rng.first, rng.last,
                                UCD_NAME_RANGE_CJK_UNIFIED_IDEOGRAPH))
        elif name == 'Hangul Syllable':
            name_ranges.append((rng.first, rng.last,
                                UCD_NAME_RANGE_HANGUL_SYLLABLE))

    nrange_data = b''.join([struct.pack(b'=III', f, l, k)
                            for f, l, k in name_ranges])

    print('There are %s names' % len(forward))
    
    return b''.join([struct.pack(b'=I', len(forward)),
                     fwd_data,
                     rev_data,
                     struct.pack(b'=I', len(name_ranges)),
                     nrange_data])

def gen_alis_table(forward, reverse):
    """Generate the alias table."""
    fwd_data = b''.join([struct.pack(b'=II', (cp | (kind << 24)), sid)
                         for cp, kind, n, sid in forward])
    rev_data = b''.join([struct.pack(b'=II', (cp | (kind << 24)), sid)
                         for n, cp, kind, sid in reverse])

    return b''.join([struct.pack(b'=I', len(forward)),
                     fwd_data,
                     rev_data])

def utf16_encode(codepoints):
    utf16 = []
    for cp in codepoints:
        if cp < 0x10000:
            utf16.append(struct.pack(b'=H', cp))
        else:
            cp -= 0x10000
            hi = 0xd800 | (cp >> 10)
            lo = 0xdc00 | (cp & 0x3ff)
            utf16.append(struct.pack(b'=HH', hi, lo))
    return b''.join(utf16)

def gen_case_table(mapping, debug=False):
    """Generate a case table from a sparse array."""
    ranges = []
    data = []
    dofs = 0
    single_run = []
    for base,mapped in mapping.runs():
        cur_base = base
        cur_chunk = []
        empty_run = None

        def finish_empty_run():
            if empty_run is not None:
                if debug:
                    print('empty %08x..%08x' % empty_run)
                ranges.append(struct.pack(b'=III',
                                          empty_run[0] | UCD_CASE_RANGE_EMPTY,
                                          empty_run[1],
                                          0))
        
        def finish_single_run():
            if len(single_run) == 0:
                return
            
            if len(single_run) == 1:
                e = single_run[0]
                if debug:
                    print('single %08x -> %08x' % (e[0], e[1]))
                ranges.append(struct.pack(b'=III',
                                          e[0] | UCD_CASE_RANGE_SINGLE,
                                          e[1],
                                          0))
            else:
                s = single_run[0]
                e = single_run[-1]

                if s[1] & 1:
                    if s[0] > s[1]:
                        mode = UCD_CASE_RANGE_ODD_DOWN
                        if debug:
                            print('odd- %08x..%08x' % (s[0], e[0]))
                    else:
                        mode = UCD_CASE_RANGE_ODD
                        if debug:
                            print('odd+ %08x..%08x' % (s[0], e[0]))
                else:
                    if s[0] > s[1]:
                        mode = UCD_CASE_RANGE_EVEN
                        if debug:
                            print('even- %08x..%08x' % (s[0], e[0]))
                    else:
                        mode = UCD_CASE_RANGE_EVEN_UP
                        if debug:
                            print('even+ %08x..%08x' % (s[0], e[0]))
                        
                ranges.append(struct.pack(b'=III', s[0] | mode, e[0], 0))

            del single_run[:]
            
        def process_chunk(dofs, single_run):
            if len(cur_chunk) == 0:
                return (dofs, single_run)
            
            if len(cur_chunk) == 1:
                if len(single_run):
                    last_single_run = single_run[-1]
                    single_ofs = last_single_run[1] - last_single_run[0]
                    if cur_base == last_single_run[0] + 2 \
                      and cur_chunk[0] - cur_base == single_ofs:
                        single_run.append((cur_base, cur_chunk[0]))
                        return (dofs, single_run)
                    else:
                        finish_single_run()
                
                single_run = [(cur_base, cur_chunk[0])]

                if cur_chunk[0] ^ cur_base != 1:
                    finish_single_run()
                
                return (dofs, single_run)

            finish_single_run()
            
            is_delta = True
            delta = cur_chunk[0] - cur_base
            for ofs, mapped_cp in enumerate(cur_chunk):
                cp = cur_base + ofs
                if mapped_cp - cp != delta:
                    is_delta = False
                    break

            if is_delta:
                if debug:
                    print('delta %08x..%08x %d' % (cur_base,
                                                   cur_base + len(cur_chunk) - 1,
                                                   delta))
                ranges.append(struct.pack(b'=IIi',
                                          cur_base | UCD_CASE_RANGE_DELTA,
                                          cur_base + len(cur_chunk) - 1,
                                          delta))
                return (dofs, single_run)

            if len(cur_chunk) == 2:
                if debug:
                    print('ucs4tbl %08x: [%08x, %08x]' % (cur_base,
                                                          cur_chunk[0],
                                                          cur_chunk[1]))
                ranges.append(struct.pack(b'=III',
                                          cur_base | UCD_CASE_RANGE_UCS4TBL,
                                          cur_chunk[0],
                                          cur_chunk[1]))
                return (dofs, single_run)
            elif len(cur_chunk) <= 4:
                small_enough = True
                for cp in cur_chunk:
                    if cp > 0xffff:
                        small_enough = False
                        break
                if small_enough:
                    if len(cur_chunk) == 3:
                        mode = UCD_CASE_RANGE_UCS2T3
                        if debug:
                            print('ucs2t3 %08x: [%04x, %04x, %04x]' \
                                  % (cur_base, cur_chunk[0], cur_chunk[1],
                                     cur_chunk[2]))
                        ranges.append(struct.pack(b'=IHHHH',
                                                  cur_base \
                                                  | UCD_CASE_RANGE_UCS2T3,
                                                  cur_chunk[0],
                                                  cur_chunk[1],
                                                  cur_chunk[2],
                                                  0))
                    else:
                        mode = UCD_CASE_RANGE_UCS2T4
                        if debug:
                            print('ucs2t4 %08x: [%04x, %04x, %04x, %04x]' \
                                  % (cur_base, cur_chunk[0], cur_chunk[1],
                                     cur_chunk[2], cur_chunk[3]))
                        ranges.append(struct.pack(b'=IHHHH',
                                                  cur_base \
                                                  | UCD_CASE_RANGE_UCS2T3,
                                                  cur_chunk[0],
                                                  cur_chunk[1],
                                                  cur_chunk[2],
                                                  cur_chunk[3]))
                    return (dofs, single_run)

            if debug:
                print('table %08x..%08x' % (cur_base,
                                            cur_base + len(cur_chunk) - 1))
            
            ranges.append(struct.pack(b'=III',
                                      cur_base | UCD_CASE_RANGE_TABLE,
                                      cur_base + len(cur_chunk) - 1,
                                      dofs))
            table = b''.join([struct.pack(b'=I', cp) for cp in cur_chunk])
            data.append(table)
            dofs += len(table)

            return (dofs, single_run)
        
        for ofs,m in enumerate(mapped):
            cp = base + ofs

            # If this code point has a list, rather than just a code point,
            # we convert the list to UTF-16 and either pack it or put it in
            # as data later on.  Likewise if it's a tuple of a simple/full
            # mapping (only in the Case Folding table).
            if isinstance(m, tuple):
                finish_empty_run()
                empty_run = None

                dofs, single_run = process_chunk(dofs, single_run)

                finish_single_run()

                simple, full = m

                utf16 = utf16_encode(full)
                if len(utf16) < 4:
                    utf16 += b'\0' * (4 - len(utf16))
                if len(utf16) == 4:
                    if debug:
                        cus = struct.unpack(b'=HH', utf16)
                        print('sf_packed %08x: %08x, [%04x, %04x]'
                              % (cp, simple, cus[0], cus[1]))
                    ranges.append(struct.pack(b'=II',
                                              cp | UCD_CASE_RANGE_SF_PACKED,
                                              simple) + utf16)
                else:
                    if debug:
                        print('sf_ext %08x: %08x, ...' % (cp, simple))
                    ranges.append(struct.pack(b'=III',
                                              cp | UCD_CASE_RANGE_SF_EXTERNAL,
                                              simple,
                                              dofs))
                    data.append(utf16)
                    dofs += len(utf16)

                cur_base = cp + 1
                cur_chunk = []
            elif isinstance(m, list):
                finish_empty_run()
                empty_run = None

                dofs, single_run = process_chunk(dofs, single_run)

                finish_single_run()

                utf16 = utf16_encode(m)
                if len(utf16) < 8:
                    utf16 += b'\0' * (8 - len(utf16))
                if len(utf16) == 8:
                    if debug:
                        cus = struct.unpack(b'=HHHH', utf16)
                        print('packed %08x: [%04x, %04x, %04x, %04x]' \
                              % (cp, cus[0], cus[1], cus[2], cus[3]))
                    ranges.append(struct.pack(b'=I', cp | UCD_CASE_RANGE_PACKED)
                                  + utf16)
                else:
                    if debug:
                        print('utf16table %08x' % cp)
                    ranges.append(struct.pack(b'=III',
                                              cp | UCD_CASE_RANGE_EXTERNAL,
                                              dofs,
                                              0))
                    data.append(utf16)
                    dofs += len(utf16)
                    
                cur_base = cp + 1
                cur_chunk = []
            elif m is None:
                if empty_run is None:
                    dofs, single_run = process_chunk(dofs, single_run)

                    finish_single_run()

                    empty_run = (cp, cp)
                else:
                    empty_run = (empty_run[0], cp)
                    
                cur_base = cp + 1
                cur_chunk = []
            else:
                finish_empty_run()
                empty_run = None
                    
                cur_chunk.append(m)
                
        finish_empty_run()
        empty_run = None
        
        dofs, single_run = process_chunk(dofs, single_run)

    finish_single_run()

    # Now fix-up the data offsets
    fixup = 4 + len(ranges) * 12
    fixed_ranges = [struct.pack(b'=I', len(ranges))]
    for r in ranges:
        entry, a, b = struct.unpack(b'=III', r)
        kind = entry & UCD_CASE_RANGE_TYPEMASK
        if kind == UCD_CASE_RANGE_EXTERNAL:
            fixed_ranges.append(struct.pack(b'=III',
                                            entry,
                                            fixup + a,
                                            b))
        elif kind == UCD_CASE_RANGE_TABLE or kind == UCD_CASE_RANGE_SF_EXTERNAL:
            fixed_ranges.append(struct.pack(b'=III',
                                            entry,
                                            a,
                                            fixup + b))
        else:
            fixed_ranges.append(r)

    return b''.join(fixed_ranges + data)

def gen_ccc_table(mapping):
    """Generate the Canonical Combining Class table from a sparse array."""
    ranges = []
    data = []
    dofs = 0
    for base,mapped in mapping.runs():
        run_base = None
        run_class = None
        run_length = None
        tbl_base = None
        tbl = []

        def finish_tbl(dofs): 
            count = len(tbl)
            if not count:
                return dofs

            if count <= 4:
                while len(tbl) < 4:
                    tbl.append(0)
                ranges.append(struct.pack(b'=IBBBBBB',
                                          tbl_base | UCD_CCC_RANGE_INLINE,
                                          count,
                                          tbl[0], tbl[1], tbl[2], tbl[3], 0))
            else:
                ranges.append(struct.pack(b'=IBIB',
                                          tbl_base | UCD_CCC_RANGE_TABLE,
                                          count,
                                          dofs, 0))
                data.append(b''.join([struct.pack(b'B', c) for c in tbl]))
                dofs += count           

            del tbl[:]

            return dofs

        for ndx,c in enumerate(mapped):
            cp = base + ndx
            if c == run_class:
                dofs = finish_tbl(dofs)
                run_length += 1
                continue
            else:
                if run_length == 1:
                    if not tbl:
                        tbl_base = run_base
                    tbl.append(run_class)
                    run_class = c
                    run_base = cp
                else:
                    if run_length:
                        ranges.append(struct.pack(b'=IIBB',
                                                  run_base | UCD_CCC_RANGE_RUN,
                                                  run_base + run_length - 1,
                                                  run_class,
                                                  0))
                    run_class = c
                    run_length = 1
                    run_base = cp

        if run_length == 1:
            if not tbl:
                tbl_base = run_base
            tbl.append(run_class)
            dofs = finish_tbl(dofs)
        elif run_length:
            ranges.append(struct.pack(b'=IIBB',
                                      run_base | UCD_CCC_RANGE_RUN,
                                      run_base + run_length - 1,
                                      run_class,
                                      0))

    # Now fix-up the data offsets
    fixup = 4 + len(ranges) * 10 # (each entry is 10 bytes)
    fixed_ranges = [struct.pack(b'=I', len(ranges))]
    for r in ranges:
        entry, count, dofs, dummy = struct.unpack(b'=IBIB', r)
        kind = entry & UCD_CCC_RANGE_TYPEMASK
        if kind == UCD_CCC_RANGE_TABLE:
            fixed_ranges.append(struct.pack(b'=IBIB',
                                            entry,
                                            count,
                                            fixup + dofs,
                                            0))
        else:
            fixed_ranges.append(r)

    return b''.join(fixed_ranges + data)

_pack_re = re.compile(r'[^A-Z]')
def pack5(s):
    s += '@' * (3 - len(s))
    s = _pack_re.sub(s.upper(), '@')
    ndx = [ord(x) - ord('@') for x in s]
    return (ndx[0] << 10) | (ndx[1] << 5) | ndx[2]

def unpack5(n):
    chs = [chr(ord('@') + x) for x in ((n >> 10) & 0x1f,
                                       (n >> 5) & 0x1f,
                                       n & 0x1f)]
    return ''.join(chs).strip('@')

def gen_jamo_table(hst):
    "Generate the jamo table, which contains data about Hangul syllables."
    ranges = []
    for base,mapped in hst.runs():
        cur_base = base
        prev_start = None
        prev_m = None
        
        for ofs,m in enumerate(mapped):
            cp = base + ofs
            # We treat LV and LVT together; we can distinguish them because
            # of the mathematical relationship between Hangul syllable codepoints
            if m == 'LV':
                m = 'LVT'
            if m != prev_m:
                if prev_m is not None:
                    ranges.append((prev_start, cp - 1, prev_m))
                prev_start = cp
                prev_m = m
        ranges.append((prev_start, cp, prev_m))

    lvt_map = { 'L': 1,
                'V': 2,
                'T': 4,
                'LV': 6,
                'LVT': 7 }
        
    jrdata = b''.join([struct.pack(b'=HHH', f, l, lvt_map[k])
                       for f, l, k in ranges])

    return b''.join([struct.pack(b'=I', len(ranges)),
                     jrdata])

def gen_block_table(blocks):
    "Generate the block table, which contains the ranges and names for blocks."
    return struct.pack(b'=I', len(blocks)) \
        + b''.join([struct.pack(b'=III', f, l, n) for f, l, n in blocks])

_nvalue_re = re.compile(r'(?P<sign>[+-]?)(?P<numerator>[0-9]+)(?:/(?P<denominator>[0-9]+))?')

def gen_numb_table(numeric):
    "Generate the numb table, which contains numeric values."
    entries = []
    for cp,v in numeric.items():
        ntype, value = v

        m = _nvalue_re.match(value)

        sgn = m.group('sign')
        den = m.group('denominator')
        num = m.group('numerator')
        
        if den:
            base = int(den)
            exponent = -1
            if sgn == '-':
                multiplier = -int(num)
            else:
                multiplier = int(num)
        elif num == '0':
            multiplier = 0
            base = 10
            exponent = 0
        else:
            noz = num.rstrip('0')
            base = 10
            exponent = len(num) - len(noz)
            if sgn == '-':
                multiplier = -int(noz)
            else:
                multiplier = int(noz)

        entries.append(struct.pack(b'=IhBb',
                                   cp | ntype, multiplier, base, exponent))

    return b''.join([struct.pack(b'=I', len(entries))] + entries)

def gen_bidi_table(bidiclass):
    "Generate the bidi table, which holds information for the Bidi algorithm."
    entries = [(0, 'L')]
    curclass = 'L'
    last_cp = 0
    for cp,c in bidiclass.items():
        if cp != last_cp + 1 and curclass != 'L':
            entries.append((last_cp + 1, 'L'))
            curclass = 'L'
        if c != curclass:
            entries.append((cp, c))
            curclass = c
        last_cp = cp
    entries.append((0x110000, 'L'))
    
    classmap = {
        'L': 1,
        'R': 2,
        'AL': 3,
        'EN': 4,
        'ES': 5,
        'ET': 6,
        'AN': 7,
        'CS': 8,
        'NSM': 9,
        'BN': 10,
        'B': 11,
        'S': 12,
        'WS': 13,
        'ON': 14,
        'LRE': 15,
        'LRO': 16,
        'RLE': 17,
        'RLO': 18,
        'PDF': 19,
        'LRI': 20,
        'RLI': 21,
        'FSI': 22,
        'PDI': 23
        }

    classdata = b''.join([struct.pack(b'=I', cp | (classmap[c] << 24))
                          for cp, c in entries])

    return b''.join([struct.pack(b'=I', len(entries)),
                     classdata])

def to_utf16(items):
    result = []
    for item in items:
        if item < 0xffff:
            yield item
        else:
            ofs = item - 0x10000
            hi = 0xd800 | ((ofs >> 10) & 0x3ff)
            lo = 0xdc00 | (ofs & 0x3ff)
            yield hi
            yield lo

def gen_deco_table(deco):
    "Generate the deco table, which holds character decompositions."
    tagmap = {
        '<font>': 1,
        '<noBreak>': 2,
        '<initial>': 3,
        '<medial>': 4,
        '<final>': 5,
        '<isolated>': 6,
        '<circle>': 7,
        '<super>': 8,
        '<sub>': 9,
        '<vertical>': 10,
        '<wide>': 11,
        '<narrow>': 12,
        '<small>': 13,
        '<square>': 14,
        '<fraction>': 15,
        '<compat>': 16
        }

    deco_entries = []
    deco_data = []
    dofs = 0
    for cp,d in deco.items():
        tag,items = d
        if len(items) == 1:
            entry = struct.pack(b'=IBBI', cp,
                                UCD_DECO_RANGE_SINGLE, tagmap.get(tag, 0),
                                items[0])
        elif len(items) == 2 and items[0] <= 0xffff and items[1] <= 0xffff:
            entry = struct.pack(b'=IBBHH', cp,
                                UCD_DECO_RANGE_PACKED, tagmap.get(tag, 0),
                                items[0], items[1])
        else:
            entry = struct.pack(b'=IBBI', cp,
                                UCD_DECO_RANGE_EXTERNAL, tagmap.get(tag, 0),
                                dofs)
            chunk = b''.join([struct.pack(b'=H', cu) for cu in to_utf16(items)]
                             + [b'\0\0'])
            deco_data.append(chunk)
            dofs += len(chunk)
        deco_entries.append(entry)
        
    # Fix-up offsets
    deco_size = len(deco_entries) * 10 + 4
    fixed_entries = []
    for entry in deco_entries:
        cp, kind, tag, offset = struct.unpack(b'=IBBI', entry)
        if kind == UCD_DECO_RANGE_EXTERNAL:
            fixed_entries.append(struct.pack(b'=IBBI', cp, kind, tag,
                                             offset + deco_size))
        else:
            fixed_entries.append(entry)

    return b''.join([struct.pack(b'=I', len(deco_entries))] + fixed_entries
                    + deco_data)

def gen_prmc_table(primc):
    # Invert the primary composite table
    compose = []
    for cp,decomp in primc.items():
        first, second = decomp
        compose.append((first, second, cp))
    compose.sort()

    prmc_entries = []
    for fsc in compose:
        entry = struct.pack(b'=III', fsc[0], fsc[1], fsc[2])
        prmc_entries.append(entry)

    return b''.join([struct.pack(b'=I', len(prmc_entries))] + prmc_entries)

def gen_mirr_table(bidimirr, bidimglyph):
    entries = []
    for cp,f in bidimirr.items():
        g = bidimglyph[cp]
        if g is None:
            g = 0xffffffff
        entries.append(struct.pack(b'=II', cp, g))
    return b''.join([struct.pack(b'=I', len(entries))] + entries)

def gen_binprop_table(bpdata):
    ranges = []
    for base,mapped in bpdata.runs():
        ranges.append((base, base + len(mapped) - 1))
    return b''.join([struct.pack(b'=I', len(ranges))]
                    + [struct.pack(b'=II', f, l) for f, l in ranges])

def gen_string_table(sdata):
    return b''.join([struct.pack(b'=I', len(sdata))]
                    + [struct.pack(b'=II', cp, sid) for cp, sid in sdata])
        

def gen_age_table(versions, ages):
    versions = list(versions)
    versions.sort()
    vers_ndx = {}
    
    vers_entries = []
    for ndx, vers in enumerate(versions):
        major, minor = vers
        vers_ndx[vers] = ndx
        vers_entries.append(struct.pack(b'=I', (major << 16) | minor))

    entries = []
    prev_cp = -1
    prev_ndx = None
    for cp,vers in ages.items():
        ndx = vers_ndx[vers]
        if cp != prev_cp + 1:
            entries.append(struct.pack(b'=I', (prev_cp + 1) | 0xff000000))
            prev_ndx = None
        if prev_ndx != ndx:
            entries.append(struct.pack(b'=I', cp | (ndx << 24)))
            prev_ndx = ndx
        prev_cp = cp
    
    # Add the sentinel
    entries.append(struct.pack(b'=I', 0xff110000))

    return b''.join([struct.pack(b'=I', len(vers_entries))]
                    + vers_entries
                    + [struct.pack(b'=I', len(entries))]
                    + entries)

def gen_script_table(scripts, scriptexts):
    entries = []
    prev_cp = -1
    prev_script = None
    for cp, script in scripts.items():
        if cp != prev_cp + 1:
            entries.append(struct.pack(b'=II', (prev_cp + 1), fourcc('Zzzz')))
            prev_script = None
        if prev_script != script:
            entries.append(struct.pack(b'=II', cp, fourcc(script)))
            prev_script = script
        prev_cp = cp

    # Add the sentinel
    entries.append(struct.pack(b'=II', 0x00110000, fourcc('Zzzz')))

    extentries = []
    extdata = []
    dofs = 0
    prev_cp = -1
    prev_scripts = None
    for cp, scripts in scriptexts.items():
        if cp != prev_cp + 1:
            extentries.append(struct.pack(b'=II', (prev_cp + 1), 0))
            prev_scripts = None
        if prev_scripts != scripts:
            extentries.append(struct.pack(b'=II',
                                          cp | (len(scripts) << 24), dofs))
            chunk = b''.join([struct.pack(b'=I', fourcc(s)) for s in scripts])
            extdata.append(chunk)
            dofs += len(chunk)
            
            prev_scripts = scripts
        prev_cp = cp

    # Add the sentinel
    extentries.append(struct.pack(b'=II', 0x00110000, 0))

    # Fix-up the data offsets
    adjust = 4 + len(entries) * len(entries[0]) \
      + 4 + len(extentries) * len(extentries[0])
    fixed_entries = []
    for entry in extentries:
        cp, ofs = struct.unpack(b'=II', entry)
        fixed_entries.append(struct.pack(b'=II', cp, ofs + adjust))
    
    return b''.join([struct.pack(b'=I', len(entries))]
                    + entries
                    + [struct.pack(b'=I', len(extentries))]
                    + fixed_entries
                    + extdata)

def gen_qc_table(qcinfo):
    entries = []
    prev_cp = -1
    prev_value = None
    value_map = {
        'N': UCD_QUICK_CHECK_NO,
        'Y': UCD_QUICK_CHECK_YES,
        'M': UCD_QUICK_CHECK_MAYBE
        }
    for cp, value in qcinfo.items():
        if cp != prev_cp + 1:
            entries.append(struct.pack(b'=I',
                                       (prev_cp + 1) | UCD_QUICK_CHECK_YES))
            prev_value = None
        if prev_value != value:
            entries.append(struct.pack(b'=I',
                                       cp | value_map[value]))
            prev_value = value
        prev_cp = cp

    # And the sentinel
    entries.append(struct.pack(b'=I', 0x00110000))

    return b''.join([struct.pack(b'=I', len(entries))]
                    + entries)

def gen_brak_table(brakdata):
    entries = []
    kind_map = {
      'n': UCD_BRAK_ENTRY_NONE,
      'o': UCD_BRAK_ENTRY_OPEN,
      'c': UCD_BRAK_ENTRY_CLOSE
    }
    data = b''.join([struct.pack(b'=II', cp | kind_map[kind], ocp)
                     for cp, ocp, kind in brakdata])

    return b''.join([struct.pack(b'=I', len(brakdata)),
                     data])

def gen_join_table(joining):
    entries = []
    data = b''.join([struct.pack(b'=IBB', cp, jtype, jgroup)
                     for cp, jtype, jgroup in joining])
    return b''.join([struct.pack(b'=I', len(joining)),
                     data])

def gen_category_table(breaking):
    entries = []
    prev_brk = None
    prev_cp = -1
    for cp, brk in breaking.items():
        if cp != prev_cp + 1:
            entries.append(struct.pack(b'=I', (prev_cp + 1)))
            prev_brk = None
        if prev_brk != brk:
            entries.append(struct.pack(b'=I', cp | (brk << 24)))
            prev_brk = brk
        prev_cp = cp

    # And a sentinel
    entries.append(struct.pack(b'=I', 0x00110000))

    return b''.join([struct.pack(b'=I', len(entries))]
                    + entries)

def gen_eaw_table(eawidth):
    entries = []
    prev_eaw = None
    prev_cp = -1
    for cp, eaw in eawidth.items():
        if cp != prev_cp + 1:
            entries.append(struct.pack(b'=I', (prev_cp + 1)))
            prev_eaw = None
        if prev_eaw != eaw:
            entries.append(struct.pack(b'=I', cp | (eaw << 24)))
            prev_eaw = eaw
        prev_cp = cp

    # And a sentinel
    entries.append(struct.pack(b'=I', 0x00110000))

    return b''.join([struct.pack(b'=I', len(entries))]
                    + entries)

def gen_rs_table(radstroke):
    """Generate the Unicode Radical Stroke data."""
    def flag(s):
        if s:
            return 0x80
        return 0
    
    ranges = []
    entries = []
    num_ranges = radstroke.run_count()
    dofs = num_ranges * 12 + 4
    for base,mapped in radstroke.runs():
        ranges.append(struct.pack(b'=III', base, base + len(mapped) - 1,
                                  dofs))
        dofs += len(mapped) * 2
        entries.append(b''.join([struct.pack(b'=BB',
                                             r, (a & 0x7f) | flag(s))
                                             for r, s, a
                                             in mapped]))
    return b''.join([struct.pack(b'=I', num_ranges)]
                    + ranges
                    + entries)

class SimpleRange (object):
    def __init__(self, first=0, last=0):
        self.first = first
        self.last = last
    def __repr__(self):
        return 'SimpleRange(0x%04x, 0x%04x)' % (self.first, self.last)

class StringTableGenerator (object):
    def __init__(self):
        self.ndx = {}
        self.strings = []
        self.next_sid = 0

    def add(self, s):
        sid = self.ndx.get(s, None)
        if sid is None:
            self.strings.append(s)
            sid = self.next_sid
            self.ndx[s] = sid
            self.next_sid += len(s) + 1
        return sid

    def as_table(self):
        return struct.pack(b'=I', self.next_sid) + b'\0'.join(self.strings)
    
def build_data(version, ucd_path, emoji_version, emoji_path, output_path):
    if emoji_path:
        print('Building Unicode data for version %s with emoji version %s\n' % (
            '.'.join(str(v) for v in version),
            '.'.join(str(v) for v in emoji_version)))
    else:
        print('Building Unicode data for version %s with NO EMOJI\n' %
              '.'.join(str(v) for v in emoji_version))

    strings = StringTableGenerator()
    forward = []
    reverse = []
    alias_forward = []
    alias_reverse = []
    catranges = []
    catrange = None
    u1names = []
    isocomments = []
    ucase = SparseArray()
    lcase = SparseArray()
    tcase = SparseArray()
    foldcase = SparseArray()
    nfkc_fc = SparseArray()
    nfkc_closure = SparseArray()
    ccc = SparseArray()
    hst = SparseArray()
    numeric = SparseArray()
    bidiclass = SparseArray()
    bidimirr = SparseArray()
    bidimglyph = SparseArray()
    deco = SparseArray()
    ages = SparseArray()
    versions = set()
    scripts = SparseArray()
    scriptexts = SparseArray()
    scriptmap = {}
    binprops = {}
    special_ranges = {}
    blocks = []
    cqc = SparseArray()
    kcqc = SparseArray()
    dqc = SparseArray()
    kdqc = SparseArray()
    brakdata = []
    joining = []
    linebreak = SparseArray()
    gcbreak = SparseArray()
    sbreak = SparseArray()
    wbreak = SparseArray()
    eawidth = SparseArray()
    radstroke = SparseArray()
    inmcat = SparseArray()
    inscat = SparseArray()
    
    range_re = re.compile(r'([A-Fa-f0-9]{4,6})\.\.([A-Fa-f0-9]{4,6})')
    special_re = re.compile(r'<(.+), (First|Last)>')

    # First scan the data in the UCD.zip file
    with zipfile.ZipFile(os.path.join(ucd_path, 'UCD.zip'), 'r') as ucd_zip:
        with ucd_zip.open('Blocks.txt', 'r') as bdata:
            for line in bdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                rng, name_unicode = [f.strip() for f in line.split(';')]
                m = range_re.match(rng)
                first_cp = int(m.group(1), 16)
                last_cp = int(m.group(2), 16)
                name = name_unicode.encode('ascii')
                sid = strings.add(name)

                blocks.append((first_cp, last_cp, sid))

        prev_cp = None
        with ucd_zip.open('UnicodeData.txt', 'r') as udata:
            for line in udata:
                line = line.decode('utf-8')
                fields = [f.strip() for f in line.split(';')]
                cp = int(fields[0], 16)

                # Do the name table and the special ranges
                if not fields[1].startswith('<'):
                    name = fields[1].encode('ascii')
                    sid = strings.add(name)
                    forward.append((cp, sid))
                    reverse.append((name, cp, sid))
                else:
                    m = special_re.match(fields[1])
                    if m:
                        name = m.group(1)
                        r = special_ranges.setdefault(name, SimpleRange())
                        if m.group(2) == 'First':
                            r.first = cp
                        else:
                            r.last = cp
                            prev_cp = None

                # Do the Unicode 1 name table
                if fields[10]:
                    name = fields[10].encode('ascii')
                    sid = strings.add(name)
                    u1names.append((cp, sid))

                # And the ISO comment table
                if fields[11]:
                    comment = fields[11].encode('ascii')
                    sid = strings.add(comment)
                    isocomments.append((cp, sid))
                            
                # Build the general category table
                skipped = prev_cp is not None and cp != prev_cp + 1
                if not catrange:
                    catrange = (cp, fields[2])
                elif catrange[1] != fields[2] or skipped:
                    catranges.append(catrange)
                    if skipped:
                        catranges.append((prev_cp + 1, 'Cn'))
                    catrange = (cp, fields[2])
                
                prev_cp = cp
                
                # Extract the combining class values
                if fields[3]:
                    c = int(fields[3])
                    if c:
                        ccc[cp] = c
                    
                # Fill-in the case tables
                if fields[12]:
                    ucase[cp] = int(fields[12], 16)
                if fields[13]:
                    lcase[cp] = int(fields[13], 16)
                if fields[14]:
                    tcase[cp] = int(fields[14], 16)

                # Fill-in the numeric value/type
                if fields[6]:
                    numeric[cp] = (UCD_NUMERIC_TYPE_DECIMAL, fields[6])
                elif fields[7]:
                    numeric[cp] = (UCD_NUMERIC_TYPE_DIGIT, fields[7])
                elif fields[8]:
                    numeric[cp] = (UCD_NUMERIC_TYPE_NUMERIC, fields[8])

                if fields[9] == 'Y':
                    bidimirr[cp] = True
                
                # And the decomposition mapping
                if fields[5]:
                    items = fields[5].split(' ')
                    if items[0].startswith('<'):
                        tag = items[0]
                        items = items[1:]
                    else:
                        tag = None
                    items = [int(item, 16) for item in items]
                    deco[cp] = (tag, items)
        
        # Finish the case tables
        with ucd_zip.open('SpecialCasing.txt', 'r') as scdata:
            for line in scdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue
                
                fields = [f.strip() for f in line.split(';')][:-1]

                # Only worry about the unconditional entries
                if len(fields) == 4:
                    cp = int(fields[0], 16)
                    if fields[1] != fields[0]:
                        lcase[cp] = [int(c, 16) for c in fields[1].split(' ')]
                    if fields[2] != fields[0]:
                        tcase[cp] = [int(c, 16) for c in fields[2].split(' ')]
                    if fields[3] != fields[0]:
                        ucase[cp] = [int(c, 16) for c in fields[3].split(' ')]

        # Also do the Case Folding table
        with ucd_zip.open('CaseFolding.txt', 'r') as cfdata:
            for line in cfdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split(';')][:-1]

                # Ignore the Turkic I issue (can be handled elsewhere)
                if fields[1] == 'T':
                    continue
                
                cp = int(fields[0], 16)
                cps = [int(c, 16) for c in fields[2].split(' ')]
                
                c = foldcase[cp]
                if c is None:
                    if len(cps) == 1:
                        foldcase[cp] = cps[0]
                    else:
                        foldcase[cp] = cps
                elif isinstance(c, list):
                    foldcase[cp] = (cps[0], c)
                else:
                    foldcase[cp] = (c, cps)

        # Various normalization related tables
        with ucd_zip.open('DerivedNormalizationProps.txt', 'r') as dndata:
            for line in dndata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split(';')]

                m = range_re.match(fields[0])
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                else:
                    rstart = rend = int(fields[0], 16)

                if fields[1] == 'NFKC_CF':
                    # NFKC_CF can be empty(!)
                    if fields[2]:
                        cps = [int(c, 16) for c in fields[2].split(' ')]
                    else:
                        cps = None
                        
                    if cps is not None and len(cps) == 1:
                        for cp in range(rstart, rend + 1):
                            nfkc_fc[cp] = cps[0]
                    else:
                        for cp in range(rstart, rend + 1):
                            nfkc_fc[cp] = cps                                
                elif fields[1] == 'FC_NFKC':
                    cps = [int(c, 16) for c in fields[2].split(' ')]

                    if len(cps) == 0:
                        cps = None
                        
                    if cps is not None and len(cps) == 1:
                        for cp in range(rstart, rend + 1):
                            nfkc_closure[cp] = cps[0]
                    else:
                        for cp in range(rstart, rend + 1):
                            nfkc_closure[cp] = cps                                
                elif fields[1] == 'NFC_QC':
                    for cp in range(rstart, rend + 1):
                        cqc[cp] = fields[2]
                elif fields[1] == 'NFKC_QC':
                    for cp in range(rstart, rend + 1):
                        kcqc[cp] = fields[2]
                elif fields[1] == 'NFD_QC':
                    for cp in range(rstart, rend + 1):
                        dqc[cp] = fields[2]
                elif fields[1] == 'NFKD_QC':
                    for cp in range(rstart, rend + 1):
                        kdqc[cp] = fields[2]
                
        # Read the name aliases
        with ucd_zip.open('NameAliases.txt', 'r') as aliasdata:
            atype_map = {
                'correction': UCD_ALIS_KIND_CORRECTION,
                'control': UCD_ALIS_KIND_CONTROL,
                'alternate': UCD_ALIS_KIND_ALTERNATE,
                'figment': UCD_ALIS_KIND_FIGMENT,
                'abbreviation': UCD_ALIS_KIND_ABBREVIATION
                }
            for line in aliasdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cp, alias, atype = [f.strip() for f in line.split(';')]
                cp = int(cp, 16)
                alias = alias.encode('ascii')
                atype = atype_map[atype]
                sid = strings.add(alias)
                alias_forward.append((cp, atype, alias, sid))
                alias_reverse.append((alias, cp, atype, sid))
                        
        # Read the Hangul Syllable Type data
        with ucd_zip.open('HangulSyllableType.txt', 'r') as hstdata:
            for line in hstdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split(';')]
                m = range_re.match(fields[0])
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)

                    for cp in range(rstart, rend + 1):
                        hst[cp] = fields[1]
                else:
                    cp = int(fields[0], 16)
                    hst[cp] = fields[1]

        # Read the Bidi classes from the DerivedBidiClass.txt file
        with ucd_zip.open('extracted/DerivedBidiClass.txt', 'r') as bididata:
            for line in bididata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue
                
                fields = [f.strip() for f in line.split(';')]

                m = range_re.match(fields[0])
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        bidiclass[cp] = fields[1]
                else:
                    cp = int(fields[0], 16)
                    bidiclass[cp] = fields[1]

        # Read the Bidi_Mirroring_Glyph property
        with ucd_zip.open('BidiMirroring.txt', 'r') as bmdata:
            for line in bmdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cp, mcp = [f.strip() for f in line.split(';')]

                cp = int(cp, 16)
                mcp = int(mcp, 16)

                bidimglyph[cp] = mcp

        # Read the Bidi_Paired_Bracked property
        with ucd_zip.open('BidiBrackets.txt', 'r') as bkdata:
            for line in bkdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split(';')]

                cp = int(fields[0], 16)
                ocp = int(fields[1], 16)
                kind = fields[2]

                brakdata.append((cp, ocp, kind))

        brakdata.sort()
                
        # Read the binary properties from PropList, DerivedCoreProperties
        # and DerivedNormalizationProps.  Note that DerivedNormalizationProps
        # contains non-binary properties as well as binary ones.
        def do_binprops(thefile):
            for line in thefile:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split(';')]

                if len(fields) != 2:
                    continue

                cps, prop = fields
                
                parray = binprops.setdefault(prop, SparseArray())
                
                m = range_re.match(cps)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        parray[cp] = True
                else:
                    parray[int(cps, 16)] = True
            
        with ucd_zip.open('PropList.txt', 'r') as pldata:
            do_binprops(pldata)
            
        with ucd_zip.open('DerivedCoreProperties.txt', 'r') as dcdata:
            do_binprops(dcdata)

        with ucd_zip.open('DerivedNormalizationProps.txt', 'r') as dndata:
            do_binprops(dndata)
        
        # Read the Composition_Exclusion binary property
        parray = binprops.setdefault('Composition_Exclusion', SparseArray())
        with ucd_zip.open('CompositionExclusions.txt', 'r') as cxdata:
            for line in cxdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                m = range_re.match(line)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        parray[cp] = True
                else:
                    parray[int(line, 16)] = True

        # Read the Age property
        with ucd_zip.open('DerivedAge.txt', 'r') as adata:
            for line in adata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cps, vers = [f.strip() for f in line.split(';')]

                major, minor = [int(v) for v in vers.split('.')]

                versions.add((major, minor))
                
                m = range_re.match(cps)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        ages[cp] = (major, minor)
                else:
                    ages[int(cps, 16)] = (major, minor)

        # Read the script names from PropertyValueAliases
        with ucd_zip.open('PropertyValueAliases.txt', 'r') as pva:
            for line in pva:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split(';')]
                if fields[0] != 'sc':
                    continue

                for alias in fields[2:]:
                    scriptmap[alias] = fields[1]
                scriptmap[fields[1]] = fields[1]
                
        # Read the Script property
        with ucd_zip.open('Scripts.txt', 'r') as sdata:
            for line in sdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cps, script = [f.strip() for f in line.split(';')]
                script = scriptmap[script]

                m = range_re.match(cps)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        scripts[cp] = script
                else:
                    scripts[int(cps, 16)] = script

        # Read the ScriptExtensions property
        with ucd_zip.open('ScriptExtensions.txt', 'r') as sedata:
            for line in sedata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cps, exts = [f.strip() for f in line.split(';')]
                exts = exts.split(' ')
                exts.sort()
                
                m = range_re.match(cps)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        scriptexts[cp] = exts
                else:
                    scriptexts[int(cps, 16)] = exts

        # And the Joining properties
        with ucd_zip.open('ArabicShaping.txt', 'r') as asdata:
            for line in asdata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cp, name, jtype, jgroup = [f.strip() for f in line.split(';')]

                jgroup = jgroup.title().replace(' ', '_')
                jgroup = joining_group_map[jgroup]
                jtype = joining_type_map[jtype]
                cp = int(cp, 16)

                joining.append((cp, jtype, jgroup))

        # Line breaking, grapheme breaking, sentence breaking, word breaking
        def read_category_data(thefile, breaking, breakmap):
            for line in thefile:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cps, bm = [f.strip() for f in line.split(';')]

                bm = breakmap[bm]
                
                m = range_re.match(cps)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        breaking[cp] = bm
                else:
                    breaking[int(cps, 16)] = bm
            
        with ucd_zip.open('LineBreak.txt', 'r') as lbdata:
            read_category_data(lbdata, linebreak, lbreak_map)

        with ucd_zip.open('auxiliary/GraphemeBreakProperty.txt', 'r') as gbdata:
            read_category_data(gbdata, gcbreak, gbreak_map)

        with ucd_zip.open('auxiliary/SentenceBreakProperty.txt', 'r') as sbdata:
            read_category_data(sbdata, sbreak, sbreak_map)

        with ucd_zip.open('auxiliary/WordBreakProperty.txt', 'r') as wbdata:
            read_category_data(wbdata, wbreak, wbreak_map)

        with ucd_zip.open('EastAsianWidth.txt', 'r') as eadata:
            for line in eadata:
                line = line.decode('utf-8')
                line = line.split('#', 1)[0].strip()
                if not line:
                    continue

                cps, width = [f.strip() for f in line.split(';')]

                width = eaw_map[width]

                m = range_re.match(cps)
                if m:
                    rstart = int(m.group(1), 16)
                    rend = int(m.group(2), 16)
                    for cp in range(rstart, rend + 1):
                        eawidth[cp] = width
                else:
                    eawidth[int(cps, 16)] = width

        # Indic Matra/Syllabic categories
        try:
            imdata = ucd_zip.open('IndicMatraCategory.txt', 'r')
        except KeyError:
            imdata = ucd_zip.open('IndicPositionalCategory.txt', 'r')
        read_category_data(imdata, inmcat, inmc_map)

        with ucd_zip.open('IndicSyllabicCategory.txt', 'r') as isdata:
            read_category_data(isdata, inscat, insc_map)
        
    if catrange:
        catranges.append(catrange)
        catranges.append((prev_cp + 1, 'Cn'))

    forward.sort()
    reverse.sort()
    alias_forward.sort()
    alias_reverse.sort()

    # Now scan the Unihan database
    with zipfile.ZipFile(os.path.join(ucd_path, 'Unihan.zip'), 'r') as unihan_zip:
        with unihan_zip.open('Unihan_NumericValues.txt', 'r') as nvdata:
            for line in nvdata:
                line = line.decode('utf-8')
                if line.startswith('#'):
                    continue
                line = line.strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split('\t')]

                if not fields[0].startswith('U+'):
                    continue

                cp = int(fields[0][2:], 16)
                numeric[cp] = (UCD_NUMERIC_TYPE_NUMERIC, fields[2])

        with unihan_zip.open('Unihan_IRGSources.txt', 'r') as irgdata:
            for line in irgdata:
                line = line.decode('utf-8')
                if line.startswith('#'):
                    continue
                line = line.strip()
                if not line:
                    continue

                fields = [f.strip() for f in line.split('\t')]
                if not fields[0].startswith('U+'):
                    continue

                if fields[1] != 'kRSUnicode':
                    continue

                cp = int(fields[0][2:], 16)
                info = fields[2].split(' ')[0]  # There is a mistake at U+3687
                base,add = info.split('.')
                simp = base.endswith("'")
                if simp:
                    base = base[:-1]
                base = int(base)
                add = int(add)

                radstroke[cp] = (base, simp, add)

    # If we have Emoji data, parse that also
    if emoji_path:
        with open(os.path.join(emoji_path, 'emoji-data.txt'), 'r') as emdata:
            do_binprops(emdata)
    else:
        emoji_version=(0,0)
    
    # Generate primary composites
    primc = SparseArray()
    ## Canonical decompositions
    for cp,d in deco.items():
        tag,cps = d
        if tag is not None:
            continue
        primc[cp] = cps
    ## Hangul LV_Syllable and LVT_Syllable
    # SBase = 0xac00
    # LBase = 0x1100
    # VBase = 0x1161
    # TBase = 0x11a7
    # LCount = 19
    # VCount = 21
    # TCount = 28
    # NCount = VCount * TCount
    # SCount = LCount * NCount
    # for cp in range(SBase, SBase + SCount):
    #     SIndex = cp - SBase
    #     TIndex = SIndex % TCount

    #     if TIndex == 0:
    #         LIndex = SIndex / NCount
    #         VIndex = (SIndex % NCount) / TCount
    #         primc[cp] = (LBase + LIndex, VBase + VIndex)
    #     else:
    #         LVIndex = (SIndex / TCount) * TCount
    #         primc[cp] = (SBase + LVIndex, TBase + TIndex)
    ## Minus Full_Composition_Exclusion
    bp = binprops['Full_Composition_Exclusion']
    for cp,fce in bp.items():
        del primc[cp]

    name_tab = gen_name_table(forward, reverse, special_ranges)
    u1nm_tab = gen_string_table(u1names)
    isoc_tab = gen_string_table(isocomments)
    alis_tab = gen_alis_table(alias_forward, alias_reverse)
    jamo_tab = gen_jamo_table(hst)
    ucase_tab = gen_case_table(ucase)
    lcase_tab = gen_case_table(lcase)
    tcase_tab = gen_case_table(tcase)

    csef_tab = gen_case_table(foldcase)
    nfkc_cf_tab = gen_case_table(nfkc_fc)
    nfkc_clo_tab = gen_case_table(nfkc_closure)
    
    numb_tab = gen_numb_table(numeric)
    
    ccc_tab = gen_ccc_table(ccc)
    strings_tab = strings.as_table()
    blok_tab = gen_block_table(blocks)
    bidi_tab = gen_bidi_table(bidiclass)
    deco_tab = gen_deco_table(deco)
    mirr_tab = gen_mirr_table(bidimirr, bidimglyph)
    brak_tab = gen_brak_table(brakdata)
    age_tab = gen_age_table(versions, ages)
    scpt_tab = gen_script_table(scripts, scriptexts)

    cqc_tab = gen_qc_table(cqc)
    kcqc_tab = gen_qc_table(kcqc)
    dqc_tab = gen_qc_table(dqc)
    kdqc_tab = gen_qc_table(kdqc)

    join_tab = gen_join_table(joining)
    lbrk_tab = gen_category_table(linebreak)
    gbrk_tab = gen_category_table(gcbreak)
    sbrk_tab = gen_category_table(sbreak)
    wbrk_tab = gen_category_table(wbreak)

    eaw_tab = gen_eaw_table(eawidth)
    rads_tab = gen_rs_table(radstroke)

    inmc_tab = gen_category_table(inmcat)
    insc_tab = gen_category_table(inscat)

    prmc_tab = gen_prmc_table(primc)
    
    tables = [
        (UCD_blok, len(blok_tab)),
        (UCD_name, len(name_tab)),
        (UCD_u1nm, len(u1nm_tab)),
        (UCD_isoc, len(isoc_tab)),
        (UCD_alis, len(alis_tab)),
        (UCD_strn, len(strings_tab)),
        (UCD_genc, 4 + 6 * len(catranges) + 6),
        (UCD_CASE, len(ucase_tab)),
        (UCD_case, len(lcase_tab)),
        (UCD_Case, len(tcase_tab)),
        (UCD_csef, len(csef_tab)),
        (UCD_kccf, len(nfkc_cf_tab)),
        (UCD_nfkc, len(nfkc_clo_tab)),
        (UCD_ccc, len(ccc_tab)),
        (UCD_jamo, len(jamo_tab)),
        (UCD_numb, len(numb_tab)),
        (UCD_bidi, len(bidi_tab)),
        (UCD_deco, len(deco_tab)),
        (UCD_mirr, len(mirr_tab)),
        (UCD_brak, len(brak_tab)),
        (UCD_age, len(age_tab)),
        (UCD_scpt, len(scpt_tab)),
        (UCD_cqc, len(cqc_tab)),
        (UCD_kcqc, len(kcqc_tab)),
        (UCD_dqc, len(dqc_tab)),
        (UCD_kdqc, len(kdqc_tab)),
        (UCD_join, len(join_tab)),
        (UCD_lbrk, len(lbrk_tab)),
        (UCD_gbrk, len(gbrk_tab)),
        (UCD_sbrk, len(sbrk_tab)),
        (UCD_wbrk, len(wbrk_tab)),
        (UCD_eaw, len(eaw_tab)),
        (UCD_rads, len(rads_tab)),
        (UCD_inmc, len(inmc_tab)),
        (UCD_insc, len(insc_tab)),
        (UCD_prmc, len(prmc_tab)),
        ]

    extra_tables = []
    for prop, tsym in binprop_tables:
        if isinstance(prop, tuple):
            for n in prop:
                bp = binprops.get(n, None)
                if bp is not None:
                    break
        else:
            bp = binprops.get(prop, None)

        if bp is None:
            if prop not in optional_binprops:
                raise KeyError('Cannot find property %s' % '/'.join(prop))
            else:
                continue
                        
        tbl = gen_binprop_table(bp)
        tables.append((fourcc(tsym), len(tbl)))
        extra_tables.append(tbl)
        
    print('\nTable usage\n===========')

    total = 0
    for code, length in tables:
        print('%s  %s' % (struct.pack(b'!I', code), length))
        total += length

    print('-----------')
    print('total %s bytes' % total)
    print('===========\n')
    
    with open(output_path, 'wb') as out:
        # Write the header
        out.write(struct.pack(b'=IIII', UCD_MAGIC,
                              (version[0] << 16) | (version[1] << 8) | version[2],
                              (emoji_version[0] << 16) | (emoji_version[1] << 8),
                              len(tables)))

        hdr_len = 16 + 8 * len(tables)

        offset = hdr_len
        for tid, size in tables:
            out.write(struct.pack(b'=II', tid, offset))
            offset += size

        # Write the block table
        out.write(blok_tab)
        
        # Write the name table
        out.write(name_tab)

        # Write the Unicode 1 name table
        out.write(u1nm_tab)

        # Write the ISO Comment table
        out.write(isoc_tab)
        
        # Write the alias table
        out.write(alis_tab)
        
        # Write the strings table
        out.write(strings_tab)

        # Write the general category table
        out.write(struct.pack(b'=I', len(catranges) + 1))
        for first,category in catranges:
            out.write(struct.pack(b'=IH', first, twocc(category)))
        out.write(struct.pack(b'=IH', 0x110000, twocc('Cn'))) # Sentinel
        
        # Write the case tables
        out.write(ucase_tab)
        out.write(lcase_tab)
        out.write(tcase_tab)
        out.write(csef_tab)
        out.write(nfkc_cf_tab)
        out.write(nfkc_clo_tab)
        
        # Write the ccc table
        out.write(ccc_tab)
        
        # Write the jamo table
        out.write(jamo_tab)

        # Write the numb table
        out.write(numb_tab)
        
        # Write the bidi table
        out.write(bidi_tab)

        # Write the deco table
        out.write(deco_tab)

        # Write the mirr table
        out.write(mirr_tab)

        # Write the brak table
        out.write(brak_tab)
        
        # Write the age table
        out.write(age_tab)

        # Write the script table
        out.write(scpt_tab)

        # Write the quick check tables
        out.write(cqc_tab)
        out.write(kcqc_tab)
        out.write(dqc_tab)
        out.write(kdqc_tab)

        # And the joining table
        out.write(join_tab)

        # Breaking
        out.write(lbrk_tab)
        out.write(gbrk_tab)
        out.write(sbrk_tab)
        out.write(wbrk_tab)

        # East Asian Width
        out.write(eaw_tab)

        # Unicode_Radical_Stroke
        out.write(rads_tab)

        # Indic Matra/Syllabic Categories
        out.write(inmc_tab)
        out.write(insc_tab)

        # Primary Composition table
        out.write(prmc_tab)
        
        # Write the binary property tables
        for tbl in extra_tables:
            out.write(tbl)
