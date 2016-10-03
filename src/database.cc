#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <stdexcept>
#include <string>
#include <system_error>

#include <libucd/libucd.h>
#include "ucd-format.h"

using namespace ucd;

// These constants are used for Hangul decomposition
static const codepoint SBase = 0xAC00;
static const codepoint LBase = 0x1100;
static const codepoint VBase = 0x1161;
static const codepoint TBase = 0x11A7;
static const unsigned LCount = 19;
static const unsigned VCount = 21;
static const unsigned TCount = 28;
static const unsigned NCount = VCount * TCount;
static const unsigned SCount = LCount * NCount;

static bool
is_decomposable_hangul(codepoint cp) {
  return cp >= SBase && cp < SBase + SCount;
}

static const char *choseong[LCount] = {
  "G", "GG", "N", "D", "DD", "R", "M", "B", "BB", "S", "SS", "", "J", "JJ", "C",
  "K", "T", "P", "H"
};
static const char *jungseong[VCount] = {
  "A", "AE", "YA", "YAE", "EO", "E", "YEO", "YE", "O", "WA", "WAE", "OE", "YO",
  "U", "WEO", "WE", "WI", "YU", "EU", "YI", "I"
};
static const char *jongseong[TCount] = {
  "", "G", "GG", "GS", "N", "NJ", "NH", "D", "L", "LG", "LM", "LB", "LS", "LT",
  "LP", "LH", "M", "B", "BS", "S", "SS", "NG", "J", "C", "K", "T", "P", "H"
};

enum {
  LOOSE_IGNORE_MEDIAL = 1,
  LOOSE_IGNORE_DASHES = 2
};

static bool
char_is_space(char ch)
{
  return ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n';
}

static int
loose_match(const char *a, const char *aend, const char *b,
            unsigned options=LOOSE_IGNORE_DASHES)
{
  bool ignore_dashes = options & LOOSE_IGNORE_DASHES;
  bool ignore_medial = options & LOOSE_IGNORE_MEDIAL;
  bool last_was_letter = false;

  while (a != aend && *a && *b) {
    if (ignore_medial && last_was_letter && a != aend && *a && *a == '-'
        && aend - a > 1
        && !char_is_space(a[1]) && a[1] != '_')
      ++a;

    while (a != aend && *a && (char_is_space(*a) || *a == '_'
                               || (ignore_dashes && *a == '-')))
      ++a;
    if (a == aend || !*a)
      break;

    while (*b && (char_is_space(*b) || *b == '_'))
      ++b;
    if (!*b)
      break;

    char cha = *a++;
    char chb = *b++;

    if (cha >= 'A' && cha <= 'Z')
      cha = cha - 'A' + 'a';
    if (chb >= 'A' && chb <= 'Z')
      chb = chb - 'A' + 'a';

    if (cha < chb)
      return -1;
    if (cha > chb)
      return +1;

    if (ignore_medial)
      last_was_letter = true;
  }

  while (a != aend && *a && (char_is_space(*a) || *a == '_'))
    ++a;
  while (*b && (char_is_space(*b) || *b == '_'))
    ++b;

  if (a != aend && *a)
    return +1;
  else if (*b)
    return -1;
  else
    return 0;
}

struct database::impl {
  int                       fd;
  off_t                     len;
  bool                      mapped;

  const struct ucd_header  *pheader;
  const struct ucd_strings *pstrings;
  const struct ucd_names   *pnames;
  const struct ucd_u1nm    *pu1nm;
  const struct ucd_isoc    *pisoc;
  const struct ucd_alis    *palis;
  const struct ucd_jamo    *pjamo;
  const struct ucd_genc    *pgenc;
  const struct ucd_numb    *pnumb;
  const struct ucd_ccc     *pccc;
  const struct ucd_case    *pCASE, *pcase, *pCase, *pcsef, *pkccf, *pnfkc;
  const struct ucd_bidi    *pbidi;
  const struct ucd_deco    *pdeco;
  const struct ucd_mirr    *pmirr;
  const struct ucd_brak    *pbrak;
  const struct ucd_age     *page;
  const struct ucd_scpt    *pscpt;
  const struct ucd_qc      *pnfcqc, *pnfkcqc, *pnfdqc, *pnfkdqc;
  const struct ucd_join    *pjoin;
  const struct ucd_brk     *plbrk, *pgbrk, *psbrk, *pwbrk;
  const struct ucd_eaw     *peaw;
  const struct ucd_rads    *prads;
  const struct ucd_inc     *pinmc, *pinsc;
  const struct ucd_prmc    *pprmc;

  const struct ucd_n32     *pscpn;
  const struct ucd_n16     *pjamn;
  const struct ucd_n16     *pgcn;
  const struct ucd_n8      *pcccn;
  const struct ucd_n8      *pnumn;
  const struct ucd_n8      *pbdin;
  const struct ucd_n8      *pdecn;
  const struct ucd_n8      *pjtn, *pjgn;
  const struct ucd_n8      *plbkn, *pgbkn, *psbkn, *pwbkn;
  const struct ucd_n8      *peawn;
  const struct ucd_n8      *pimcn, *piscn;

#define BINPROP(n,m,t) const struct ucd_binprop *pbinprop_ ## m;
#include "ucd-binprops.h"

  std::vector<class block> blocks;

  ~impl();

  const void *get_table(uint32_t table_id) const;
  const char *get_strptr_unsafe(ucd_string_id_t sid, size_t &max_len);
  const char *get_strptr(ucd_string_id_t sid, size_t &len);
  std::string get_string(ucd_string_id_t sid);
  const struct ucd_names *get_names();
  const struct ucd_u1nm *get_u1nm();
  const struct ucd_isoc *get_isoc();
  const struct ucd_alis *get_alis();
  const struct ucd_jamo *get_jamo();
  const struct ucd_genc *get_genc();
  const struct ucd_numb *get_numb();
  const struct ucd_ccc  *get_ccc();
  const struct ucd_case *get_CASE();
  const struct ucd_case  *get_case();
  const struct ucd_case  *get_Case();
  const struct ucd_case *get_csef();
  const struct ucd_case *get_kccf();
  const struct ucd_case *get_nfkc();
  const struct ucd_bidi *get_bidi();
  const struct ucd_deco *get_deco();
  const struct ucd_mirr *get_mirr();
  const struct ucd_brak *get_brak();
  const struct ucd_age *get_age();
  const struct ucd_scpt *get_scpt();
  const struct ucd_qc *get_nfcqc();
  const struct ucd_qc *get_nfkcqc();
  const struct ucd_qc *get_nfdqc();
  const struct ucd_qc *get_nfkdqc();
  const struct ucd_join *get_join();
  const struct ucd_brk *get_lbrk();
  const struct ucd_brk *get_gbrk();
  const struct ucd_brk *get_sbrk();
  const struct ucd_brk *get_wbrk();
  const struct ucd_eaw *get_eaw();
  const struct ucd_rads *get_rads();
  const struct ucd_inc *get_inmc();
  const struct ucd_inc *get_insc();
  const struct ucd_prmc *get_prmc();

  const struct ucd_n16 *get_jamn();
  const struct ucd_n16 *get_gcn();
  const struct ucd_n8  *get_cccn();
  const struct ucd_n8  *get_numn();
  const struct ucd_n8  *get_bdin();
  const struct ucd_n8  *get_decn();
  const struct ucd_n8  *get_jtn();
  const struct ucd_n8  *get_jgn();
  const struct ucd_n8  *get_lbkn();
  const struct ucd_n8  *get_gbkn();
  const struct ucd_n8  *get_sbkn();
  const struct ucd_n8  *get_wbkn();
  const struct ucd_n8  *get_eawn();
  const struct ucd_n8  *get_imcn();
  const struct ucd_n8  *get_iscn();
  const struct ucd_n32 *get_scpn();

#undef BINPROP
#define BINPROP(n,m,t)                                           \
  const struct ucd_binprop *get_binprop_##m() {                  \
    if (!pbinprop_ ## m)                                         \
      pbinprop_ ## m = (const struct ucd_binprop *)get_table(t); \
    return pbinprop_ ## m;                                       \
  }
#include "ucd-binprops.h"

  void init_blocks();

  std::string strip(const std::string &s);
  template <class table, class valtype>
  bool search(const table *ptbl, valtype value, std::string &str)
  {
    uint32_t min = 0, max = ptbl->num_fwd, mid;

    while (min < max) {
      mid = (min + max) / 2;

      if (value < ptbl->names[mid].value)
        max = mid;
      else if (value > ptbl->names[mid].value)
        min = mid + 1;
      else {
        str = get_string(ptbl->names[mid].name);
        return true;
      }
    }

    return false;
  }

  template <class table, class valtype>
  bool search(const table *ptbl, const std::string &str, valtype &result)
  {
    std::string stripped = strip(str);
    const char *nstr = stripped.c_str();
    uint32_t min = 0, max = ptbl->num_rev, mid;
    auto *entries = ptbl->names + ptbl->num_fwd;

    while (min < max) {
      mid = (min + max) / 2;

      uint32_t sid = entries[mid].name;
      size_t max_len;
      const char *nameptr = get_strptr_unsafe(sid, max_len);
      const char *nameend = nameptr + max_len;

      int ret = loose_match(nameptr, nameend, nstr, LOOSE_IGNORE_DASHES);

      if (ret > 0)
        max = mid;
      else if (ret < 0)
        min = mid + 1;
      else {
        result = entries[mid].value;
        return true;
      }
    }

    return false;
  }
};

database::impl::~impl()
{
  if (fd >= 0)
    ::close(fd);
  if (pheader && mapped)
    ::munmap((void *)pheader, len);
}

const void *
database::impl::get_table(uint32_t table_id) const
{
  for (unsigned n = 0; n < pheader->num_tables; ++n) {
    if (pheader->tables[n].table_id == table_id) {
      const uint8_t *base = (const uint8_t *)pheader;
      return base + pheader->tables[n].offset;
    }
  }

  return nullptr;
}

const char *
database::impl::get_strptr_unsafe(ucd_string_id_t sid, size_t &max_len)
{
  if (!pstrings) {
    pstrings = (struct ucd_strings *)get_table(UCD_strn);
    if (!pstrings)
      throw bad_data_file("missing string table");
  }

  if (sid >= pstrings->size)
    throw std::out_of_range("string ID out of range");

  max_len = pstrings->size - sid;

  return pstrings->strings + sid;
}

const char *
database::impl::get_strptr(ucd_string_id_t sid, size_t &len)
{
  size_t max_len;
  const char *str = get_strptr_unsafe(sid, max_len);

  len = ::strnlen(str, max_len);

  return str;
}

std::string
database::impl::get_string(ucd_string_id_t sid)
{
  size_t len;
  const char *ptr = get_strptr(sid, len);

  return std::string(ptr, len);
}

#define GETTER(name,type,ident)                 \
const struct type *                             \
database::impl::get_##name() {                  \
  if (!p##name)                                 \
    p##name = (struct type *)get_table(ident);  \
  return p##name;                               \
}

GETTER(names, ucd_names,UCD_name)
GETTER(u1nm, ucd_u1nm, UCD_u1nm)
GETTER(isoc, ucd_isoc, UCD_isoc)
GETTER(alis, ucd_alis, UCD_alis)
GETTER(jamo, ucd_jamo, UCD_jamo)
GETTER(genc, ucd_genc, UCD_genc)
GETTER(numb, ucd_numb, UCD_numb)
GETTER(ccc, ucd_ccc, UCD_ccc)
GETTER(CASE, ucd_case, UCD_CASE)
GETTER(case, ucd_case, UCD_case)
GETTER(Case, ucd_case, UCD_Case)
GETTER(csef, ucd_case, UCD_csef)
GETTER(kccf, ucd_case, UCD_kccf)
GETTER(nfkc, ucd_case, UCD_nfkc)
GETTER(bidi, ucd_bidi, UCD_bidi)
GETTER(deco, ucd_deco, UCD_deco)
GETTER(mirr, ucd_mirr, UCD_mirr)
GETTER(brak, ucd_brak, UCD_brak)
GETTER(age, ucd_age, UCD_age)
GETTER(scpt, ucd_scpt, UCD_scpt)
GETTER(nfcqc, ucd_qc, UCD_cqc)
GETTER(nfkcqc, ucd_qc, UCD_kcqc)
GETTER(nfdqc, ucd_qc, UCD_dqc)
GETTER(nfkdqc, ucd_qc, UCD_kdqc)
GETTER(join, ucd_join, UCD_join)
GETTER(lbrk, ucd_brk, UCD_lbrk)
GETTER(gbrk, ucd_brk, UCD_gbrk)
GETTER(sbrk, ucd_brk, UCD_sbrk)
GETTER(wbrk, ucd_brk, UCD_wbrk)
GETTER(eaw, ucd_eaw, UCD_eaw)
GETTER(rads, ucd_rads, UCD_rads)
GETTER(inmc, ucd_inc, UCD_inmc)
GETTER(insc, ucd_inc, UCD_insc)
GETTER(prmc, ucd_prmc, UCD_prmc)

GETTER(jamn, ucd_n16, UCD_jamn)
GETTER(gcn, ucd_n16, UCD_gcn)
GETTER(cccn, ucd_n8, UCD_cccn)
GETTER(numn, ucd_n8, UCD_numn)
GETTER(bdin, ucd_n8, UCD_bdin)
GETTER(decn, ucd_n8, UCD_decn)
GETTER(lbkn, ucd_n8, UCD_lbkn)
GETTER(gbkn, ucd_n8, UCD_gbkn)
GETTER(sbkn, ucd_n8, UCD_sbkn)
GETTER(wbkn, ucd_n8, UCD_wbkn)
GETTER(eawn, ucd_n8, UCD_eawn)
GETTER(imcn, ucd_n8, UCD_imcn)
GETTER(iscn, ucd_n8, UCD_iscn)
GETTER(scpn, ucd_n32, UCD_scpn)

const struct ucd_n8 *
database::impl::get_jtn() {
  if (!pjtn)
    pjtn = (struct ucd_n8 *)get_table(UCD_jonn);
  return pjtn;
}

const struct ucd_n8 *
database::impl::get_jgn() {
  if (!pjgn) {
    const uint8_t *ptr = (const uint8_t *)get_jtn();
    ptr += (sizeof(struct ucd_n8)
            + (pjtn->num_fwd + pjtn->num_rev) * sizeof(struct ucd_n8_entry));
    pjgn = (const struct ucd_n8 *)ptr;
  }
  return pjgn;
}

void
database::impl::init_blocks()
{
  const struct ucd_blok *pblok = (const struct ucd_blok *)get_table(UCD_blok);

  for (unsigned n = 0; n < pblok->num_blocks; ++n) {
    blocks.push_back(ucd::block(pblok->blocks[n].first_cp,
                                pblok->blocks[n].last_cp,
                                get_string(pblok->blocks[n].name),
                                get_string(pblok->blocks[n].alias)));
  }
}

database::database()
{
}

database::database(const char *filename)
{
  open(filename);
}

void
database::open(const char *filename)
{
  if (_pimpl)
    throw std::runtime_error("database already open");

  _pimpl = std::unique_ptr<impl>(new database::impl());

  _pimpl->fd = ::open(filename, O_RDONLY);

  if (_pimpl->fd < 0)
    throw std::system_error(errno, std::system_category());

  _pimpl->len = ::lseek(_pimpl->fd, 0, SEEK_END);

  if (_pimpl->len < 0)
    throw std::system_error(errno, std::system_category());

  _pimpl->pheader = (struct ucd_header *)::mmap(nullptr,
                                                _pimpl->len,
                                                PROT_READ,
                                                MAP_FILE|MAP_SHARED,
                                                _pimpl->fd,
                                                0);
  _pimpl->mapped = true;

  const struct ucd_header *phead = _pimpl->pheader;

  if (!phead)
    throw std::system_error(errno, std::system_category());

  if (phead->magic != UCD_MAGIC)
    throw bad_data_file("not a UCD database");

  if (phead->num_tables > UCD_MAX_TABLES)
    throw bad_data_file("too many tables in UCD database");
}

database::database(const void *base, size_t length)
{
  _pimpl = std::unique_ptr<impl>(new database::impl());
  _pimpl->fd = -1;
  _pimpl->len = length;
  _pimpl->pheader = (struct ucd_header *)base;
  _pimpl->mapped = false;

  const struct ucd_header *phead = _pimpl->pheader;

  if (phead->magic != UCD_MAGIC)
    throw bad_data_file("not a UCD database");

  if (phead->num_tables > UCD_MAX_TABLES)
    throw bad_data_file("too many tables in UCD database");
}

void
database::close()
{
  _pimpl = nullptr;
}

database::~database()
{
}

version
database::unicode_version() const
{
  uint32_t uver = _pimpl->pheader->unicode_version;

  return version(uver >> 16, (uver >> 8) & 0xff, uver & 0xff);
}

version
database::emoji_version() const
{
  uint32_t uver = _pimpl->pheader->emoji_version;

  return version(uver >> 16, (uver >> 8) & 0xff, uver & 0xff);
}

std::string
database::impl::strip(const std::string &s)
{
  std::string stripped;

  for (auto it = s.begin(); it != s.end(); ++it) {
    while (char_is_space(*it)
           || *it == '_'
           || *it == '-')
      ++it;
    if (it == s.end())
      break;
    char ch = *it;
    if (ch >= 'A' && ch <= 'Z')
      ch = ch - 'A' + 'a';
    stripped += ch;
  }

  return stripped;
}

codepoint
database::codepoint_from_name(const std::string &name,
                              unsigned allowed_types) const
{
  const struct ucd_names *pnames = _pimpl->get_names();
  const struct ucd_alis *palis = _pimpl->get_alis();
  const struct ucd_name_entry *entries;
  const char *cname = name.c_str();

  // Handle U+xxxx syntax
  unsigned hexoffset = 0;

  if (::strncasecmp(cname, "U+", 2) == 0)
    hexoffset = 2;
  else if (::strncasecmp(cname, "CJK UNIFIED IDEOGRAPH-", 22) == 0)
    hexoffset = 22;
  else if (::strncasecmp(cname, "CJK COMPATIBILITY IDEOGRAPH-", 28) == 0)
    hexoffset = 28;

  if (hexoffset) {
    char *endptr;
    unsigned long cp = std::strtoul(cname + hexoffset, &endptr, 16);
    size_t count = endptr - cname  - hexoffset;
    if (*endptr == '\0' && count >= 4 && count <= 6) {
      if (cp > 0x10ffff || (cp >= 0xd800 && cp <= 0xdfff))
        return bad_codepoint;
      return (codepoint)cp;
    }
  } else if (::strncasecmp(cname, "HANGUL SYLLABLE ", 16) == 0) {
    const char *ptr = cname + 16;
    unsigned LIndex = LCount, VIndex = VCount, TIndex = 0;
    size_t match_len;

    match_len = 0;
    for (unsigned n = 0; n < LCount; ++n) {
      size_t len = std::strlen(choseong[n]);
      if (len >= match_len && ::strncasecmp(ptr, choseong[n], len) == 0) {
        LIndex = n;
        match_len = len;
      }
    }

    ptr += match_len;

    match_len = 0;
    for (unsigned n = 0; n < VCount; ++n) {
      size_t len = std::strlen(jungseong[n]);
      if (len >= match_len && ::strncasecmp(ptr, jungseong[n], len) == 0) {
        VIndex = n;
        match_len = len;
      }
    }

    ptr += match_len;

    match_len = 0;
    for (unsigned n = 1; n < TCount; ++n) {
      size_t len = std::strlen(jongseong[n]);
      if (len >= match_len && ::strncasecmp(ptr, jongseong[n], len) == 0) {
        TIndex = n;
        match_len = len;
      }
    }

    if (LIndex < LCount && VIndex < VCount) {
      codepoint cp = SBase + LIndex * NCount + VIndex * TCount + TIndex;
      return cp;
    }
  }

  if (!pnames)
    throw no_name_table("data file doesn't contain name table");

  // Handle U+1180 as a special case
  static const char *jungseong = "hangul jungseong o-e";

  if (loose_match(jungseong, jungseong + 20, cname, 0) == 0)
    return 0x1180;

  // Strip dashes, underscores and whitespace
  std::string stripped;
  const char *ptr = cname;
  bool last_was_letter = false;

  while (*ptr) {
    if (last_was_letter && *ptr == '-' && !char_is_space(ptr[1]) && ptr[1] != '_')
      ++ptr;
    while (char_is_space(*ptr) || *ptr == '_')
      ++ptr;

    stripped += *ptr++;
    last_was_letter = true;
  }

  cname = stripped.c_str();

  entries = pnames->names + pnames->num_names;

  uint32_t min = 0, max = pnames->num_names, mid;

  while (min < max) {
    mid = (min + max) / 2;

    uint32_t sid = entries[mid].name;
    size_t max_len;
    const char *nameptr = _pimpl->get_strptr_unsafe(sid, max_len);
    const char *nameend = nameptr + max_len;

    /* U+1180 is a special case because U+116C and U+1180 only differ in
       the medial hyphen; when comparing with U+1180, we *do not* ignore
       medial hyphens */
    int ret = loose_match(nameptr, nameend, cname,
                          entries[mid].code_point!=0x1180
                          ? LOOSE_IGNORE_MEDIAL : 0);

    if (ret > 0)
      max = mid;
    else if (ret < 0)
      min = mid + 1;
    else
      return entries[mid].code_point;
  }

  // Support aliases
  if (allowed_types && palis) {
    const struct ucd_alias *aliases = palis->aliases + palis->num_aliases;
    min = 0;
    max = palis->num_aliases;

    while (min < max) {
      mid = (min + max) / 2;

      uint32_t sid = aliases[mid].name;
      size_t max_len;
      const char *nameptr = _pimpl->get_strptr_unsafe(sid, max_len);
      const char *nameend = nameptr + max_len;
      int ret = loose_match(nameptr, nameend, cname, LOOSE_IGNORE_MEDIAL);

      if (ret > 0)
        max = mid;
      else if (ret < 0)
        min = mid + 1;
      else {
        codepoint cp = UCD_ALIAS_CODE_POINT(aliases[mid].entry);
        ucd_alias_kind_t kind = UCD_ALIAS_KIND(aliases[mid].entry);

        if (kind & allowed_types)
          return cp;
        return bad_codepoint;
      }
    }
  }

  return bad_codepoint;
}

std::string
database::jamo_short_name(codepoint cp) const
{
  if (cp >= LBase && cp < LBase + LCount)
    return choseong[cp - LBase];
  if (cp >= VBase && cp < VBase + VCount)
    return jungseong[cp - VBase];
  if (cp > TBase && cp < TBase + TCount)
    return jongseong[cp - TBase];

  return "";
}

std::string
database::name(codepoint cp) const
{
  const struct ucd_names *pnames = _pimpl->get_names();
  const struct ucd_name_entry *entries = pnames->names;

  if (pnames) {
    const struct ucd_name_ranges *pranges
      = (const struct ucd_name_ranges *)&pnames->names[2 * pnames->num_names];

    // First check the special ranges
    for (unsigned n = 0; n < pranges->num_ranges; ++n) {
      if (cp >= pranges->ranges[n].first_cp
          && cp <= pranges->ranges[n].last_cp) {
        switch (pranges->ranges[n].kind) {
        case UCD_NAME_RANGE_CJK_UNIFIED_IDEOGRAPH:
          {
            char buffer[80];
            std::sprintf(buffer, "CJK UNIFIED IDEOGRAPH-%04X", cp);
            return std::string(buffer);
          }
        case UCD_NAME_RANGE_CJK_COMPATIBILITY_IDEOGRAPH:
          {
            char buffer[80];
            std::sprintf(buffer, "CJK COMPATIBILITY IDEOGRAPH-%04X", cp);
            return std::string(buffer);
          }
        case UCD_NAME_RANGE_HANGUL_SYLLABLE:
          {
            unsigned SIndex = cp - SBase;
            unsigned LIndex = SIndex / NCount;
            unsigned VIndex = (SIndex % NCount) / TCount;
            unsigned TIndex = SIndex % TCount;

            char buffer[80];
            std::sprintf(buffer, "HANGUL SYLLABLE %s%s%s",
                         choseong[LIndex],
                         jungseong[VIndex],
                         jongseong[TIndex]);
            return std::string(buffer);
          }
        default:
          break;
        }
      }
    }

    uint32_t min = 0, max = pnames->num_names, mid;

    while (min < max) {
      mid = (min + max) / 2;

      codepoint ecp = entries[mid].code_point;

      if (cp < ecp)
        max = mid;
      else if (cp > ecp)
        min = mid + 1;
      else
        return _pimpl->get_string(entries[mid].name);
    }
  }

  if (cp > 0x10ffff)
    return "<bad codepoint>";

  char buffer[80];
  std::sprintf(buffer, "U+%04X", cp);

  return std::string(buffer);
}

std::string
database::unicode_1_name(codepoint cp) const
{
  const struct ucd_u1nm *pu1nm = _pimpl->get_u1nm();

  unsigned min = 0, max = pu1nm->num_names, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pu1nm->names[mid].code_point;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else
      return _pimpl->get_string(pu1nm->names[mid].name);
  }

  return "";
}

std::string
database::iso_comment(codepoint cp) const
{
  const struct ucd_isoc *pisoc = _pimpl->get_isoc();

  unsigned min = 0, max = pisoc->num_comments, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pisoc->comments[mid].code_point;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else
      return _pimpl->get_string(pisoc->comments[mid].comment);
  }

  return "";
}

std::vector<alias>
database::name_alias(codepoint cp,
                     unsigned allowed_types) const
{
  const struct ucd_alis *palis = _pimpl->get_alis();
  std::vector<alias> result;

  if (!palis)
    throw no_alias_table("data file doesn't contain alias table");

  unsigned min = 0, max = palis->num_aliases, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_ALIAS_CODE_POINT(palis->aliases[mid].entry);

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else {
      // Find the *first* alias for this code point
      while (mid >= min
             && UCD_ALIAS_CODE_POINT(palis->aliases[mid - 1].entry) == cp)
        --mid;

      while (mid < max && UCD_ALIAS_CODE_POINT(palis->aliases[mid].entry) == cp) {
        ucd_alias_kind_t kind = UCD_ALIAS_KIND(palis->aliases[mid].entry);

        if (kind & allowed_types) {
          std::string name = _pimpl->get_string(palis->aliases[mid].name);
          result.push_back(alias(Alias_Type::Enum(kind), name));
        }

        ++mid;
      }

      return result;
    }
  }

  return result;
}

hst
database::hangul_syllable_type(codepoint cp) const
{
  const struct ucd_jamo *pjamo = _pimpl->get_jamo();

  uint32_t min = 0, max = pjamo->num_ranges, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint fcp = pjamo->ranges[mid].first_cp;
    codepoint lcp = pjamo->ranges[mid].last_cp;

    if (cp < fcp)
      max = mid;
    else if (cp > lcp)
      min = mid + 1;
    else {
      hst result = (hst)pjamo->ranges[mid].kind;

      if (result == Hangul_Syllable_Type::LVT) {
        unsigned SIndex = cp - SBase;
        unsigned TIndex = SIndex % TCount;

        if (!TIndex)
          result = Hangul_Syllable_Type::LV;
      }

      return result;
    }
  }

  return Hangul_Syllable_Type::NA;
}

gc
database::general_category(codepoint cp) const
{
  const struct ucd_genc *pgenc = _pimpl->get_genc();

  // There is a sentinel on the general category table
  unsigned min = 0, max = pgenc->num_ranges - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pgenc->ranges[mid].first_cp;
    codepoint ncp = pgenc->ranges[mid + 1].first_cp;

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return pgenc->ranges[mid].category;
  }

  return General_Category::Cn;
}

ccc
database::canonical_combining_class(codepoint cp) const
{
  const struct ucd_ccc *pccc = _pimpl->get_ccc();

  unsigned min = 0, max = pccc->num_ranges, mid;

  while (min < max) {
    mid = (min + max) / 2;

    const struct ucd_ccc_range &range = pccc->ranges[mid];
    codepoint ecp = UCD_CCC_RANGE_CP(range.entry);
    ucd_ccc_range_kind_t kind = UCD_CCC_RANGE_KIND(range.entry);
    codepoint lcp;

    switch (kind) {
    case UCD_CCC_RANGE_RUN:
      lcp = range.run.last_cp;
      if (cp >= ecp && cp <= lcp)
        return ccc(range.run.code);
      break;
    case UCD_CCC_RANGE_INLINE:
      lcp = ecp + range.inline_tbl.count - 1;
      if (cp >= ecp && cp <= lcp)
        return ccc(range.inline_tbl.codes[cp - ecp]);
      break;
    case UCD_CCC_RANGE_TABLE:
      lcp = ecp + range.table.count - 1;
      if (cp >= ecp && cp <= lcp) {
        const uint8_t *table = (const uint8_t *)pccc + range.table.offset;
        return ccc(table[cp - ecp]);
      }
      break;
    }

    if (cp < ecp)
      max = mid;
    else
      min = mid + 1;
  }

  return Canonical_Combining_Class::Not_Reordered;
}

stroke_count
database::unicode_radical_stroke(codepoint cp) const
{
  const struct ucd_rads *prads = _pimpl->get_rads();

  unsigned min = 0, max = prads->num_ranges, mid = 0;

  while (min < max) {
    mid = (min + max) / 2;

    if (cp < prads->ranges[mid].first_cp)
      max = mid;
    else if (cp > prads->ranges[mid].last_cp)
      min = mid + 1;
    else {
      const struct ucd_rads_table_entry *entries
        = (const struct ucd_rads_table_entry *)((uint8_t *)prads
                                                + prads->ranges[mid].offset);
      unsigned ndx = cp - prads->ranges[mid].first_cp;

      return stroke_count(entries[ndx].radical,
                          UCD_RADS_IS_SIMPLIFIED(entries[ndx].stroke_info)
                          ? stroke_count::simplified
                          : stroke_count::traditional,
                          UCD_RADS_STROKE_COUNT(entries[ndx].stroke_info));
    }
  }

  return stroke_count::none;
}

static void
get_case_range_bounds(const struct ucd_case_range &range,
                      codepoint &ecp, codepoint &lcp)
{
  ecp = UCD_CASE_RANGE_CP(range.entry);
  ucd_case_range_kind_t kind = UCD_CASE_RANGE_KIND(range.entry);

  switch (kind) {
  case UCD_CASE_RANGE_SINGLE:
  case UCD_CASE_RANGE_PACKED:
  case UCD_CASE_RANGE_EXTERNAL:
  case UCD_CASE_RANGE_SF_PACKED:
  case UCD_CASE_RANGE_SF_EXTERNAL:
    lcp = ecp;
    break;
  case UCD_CASE_RANGE_DELTA:
  case UCD_CASE_RANGE_EMPTY:
    lcp = range.delta.last_cp;
    break;
  case UCD_CASE_RANGE_EVEN:
  case UCD_CASE_RANGE_ODD:
  case UCD_CASE_RANGE_EVEN_UP:
  case UCD_CASE_RANGE_ODD_DOWN:
    lcp = range.last_cp;
    break;
  case UCD_CASE_RANGE_TABLE:
    lcp = range.table.last_cp;
    break;
  case UCD_CASE_RANGE_UCS4TBL:
    lcp = ecp + 1;
    break;
  case UCD_CASE_RANGE_UCS2T3:
    lcp = ecp + 2;
    break;
  case UCD_CASE_RANGE_UCS2T4:
    lcp = ecp + 3;
    break;
  }
}

static const struct ucd_case_range *
find_case_range(const struct ucd_case *pcase, codepoint cp)
{
  unsigned min = 0, max = pcase->num_ranges, mid;

  while (min < max) {
    mid = (min + max) / 2;

    const struct ucd_case_range &range = pcase->ranges[mid];
    codepoint ecp, lcp;

    get_case_range_bounds(range, ecp, lcp);

    if (cp < ecp)
      max = mid;
    else if (cp > lcp)
      min = mid + 1;
    else
      return &range;
  }

  return nullptr;
}

static size_t
utf16_expand(const uint16_t *utf16,
             const uint16_t *utf16end,
             codepoint *out,
             size_t out_len)
{
  codepoint *end = out + out_len;
  size_t count = 0;

  while (!utf16end || utf16 < utf16end) {
    uint16_t cu0 = *utf16++;

    if (!cu0)
      break;

    if (cu0 >= 0xd800 && cu0 <= 0xdbff) {
      if (utf16end && utf16 >= utf16end)
        break;

      uint16_t cu1 = *utf16++;

      if (out && out < end)
        *out++ = 0x10000 + (((cu0 & 0x3ff) << 10) | (cu1 & 0x3ff));
      ++count;
    } else {
      if (out && out < end)
        *out++ = cu0;
      ++count;
    }
  }

  return count;
}

enum class MappingType {
  Full = 0,
  Simple = 1,
};

enum class DefaultMapping {
  None = 0,
  Codepoint = 1
};

static size_t
case_mapping(const struct ucd_case *pcase,
             codepoint from_cp,
             codepoint *out, size_t out_len,
             MappingType fos=MappingType::Full,
             DefaultMapping dm=DefaultMapping::Codepoint)
{
  const struct ucd_case_range *range = find_case_range(pcase, from_cp);

  if (!range) {
    switch (dm) {
    case DefaultMapping::Codepoint:
      if (out && out_len >= 1)
        *out = from_cp;
      return 1;
    case DefaultMapping::None:
      return 0;
    }
  }

  codepoint base = UCD_CASE_RANGE_CP(range->entry);
  switch (UCD_CASE_RANGE_KIND(range->entry)) {
  case UCD_CASE_RANGE_SINGLE:
    if (out && out_len >= 1)
      *out = range->mapped_cp;
    return 1;
  case UCD_CASE_RANGE_DELTA:
    if (out && out_len >= 1)
      *out = from_cp + range->delta.delta;
    return 1;
  case UCD_CASE_RANGE_EVEN:
    if (out && out_len >= 1)
      *out = from_cp & ~1;
    return 1;
  case UCD_CASE_RANGE_ODD:
    if (out && out_len >= 1)
      *out = from_cp | 1;
    return 1;
  case UCD_CASE_RANGE_EVEN_UP:
    if (out && out_len >= 1)
      *out = from_cp + (from_cp & 1);
    return 1;
  case UCD_CASE_RANGE_ODD_DOWN:
    if (out && out_len >= 1)
      *out = from_cp - ((~from_cp) & 1);
    return 1;
  case UCD_CASE_RANGE_PACKED:
    return utf16_expand(range->packed, &range->packed[4], out, out_len);
  case UCD_CASE_RANGE_EXTERNAL:
    {
      uint16_t *ptr = (uint16_t *)((uint8_t *)pcase + range->ext_offset);
      return utf16_expand(ptr, nullptr, out, out_len);
    }
  case UCD_CASE_RANGE_TABLE:
    {
      uint32_t *table = (uint32_t *)((uint8_t *)pcase + range->table.offset);
      if (out && out_len >= 1)
        *out = table[from_cp - base];
      return 1;
    }
  case UCD_CASE_RANGE_UCS4TBL:
    if (out && out_len >= 1)
      *out = range->ucs4[from_cp - base];
    return 1;
  case UCD_CASE_RANGE_UCS2T3:
  case UCD_CASE_RANGE_UCS2T4:
    if (out && out_len >= 1)
      *out = range->ucs2[from_cp - base];
    return 1;
  case UCD_CASE_RANGE_SF_PACKED:
    switch (fos) {
    case MappingType::Full:
      return utf16_expand(range->sf_packed.full_packed,
                          &range->sf_packed.full_packed[2],
                          out, out_len);
    case MappingType::Simple:
      if (out && out_len >= 1)
        *out = range->sf_packed.simple_cp;
      return 1;
    }
  case UCD_CASE_RANGE_SF_EXTERNAL:
    switch (fos) {
    case MappingType::Full:
      {
        uint16_t *ptr = (uint16_t *)((uint8_t *)pcase 
                                     + range->sf_external.full_offset);
        return utf16_expand(ptr, nullptr, out, out_len);
      }
    case MappingType::Simple:
      if (out && out_len >= 1)
        *out = range->sf_external.simple_cp;
      return 1;
    }
  case UCD_CASE_RANGE_EMPTY:
    return 0;
  }
}

size_t
database::uppercase_mapping(codepoint from_cp,
                            codepoint *out, size_t out_len) const
{
  return case_mapping(_pimpl->get_CASE(), from_cp, out, out_len);
}

size_t
database::lowercase_mapping(codepoint from_cp,
                            codepoint *out, size_t out_len) const
{
  return case_mapping(_pimpl->get_case(), from_cp, out, out_len);
}

size_t
database::titlecase_mapping(codepoint from_cp,
                            codepoint *out, size_t out_len) const
{
  return case_mapping(_pimpl->get_Case(), from_cp, out, out_len);
}

codepoint
database::simple_case_folding(codepoint cp) const
{
  codepoint out;
  case_mapping(_pimpl->get_csef(), cp, &out, 1, MappingType::Simple);
  return out;
}

size_t
database::case_folding(codepoint from_cp,
                       codepoint *out, size_t out_len) const
{
  return case_mapping(_pimpl->get_csef(), from_cp, out, out_len,
                      MappingType::Full);
}

size_t
database::nfkc_casefold(codepoint from_cp,
                        codepoint *out, size_t out_len) const
{
  return case_mapping(_pimpl->get_kccf(), from_cp, out, out_len);
}

size_t
database::fc_nfkc_closure(codepoint cp,
                          codepoint *out, size_t out_len) const
{
  return case_mapping(_pimpl->get_nfkc(), cp, out, out_len,
                      MappingType::Full, DefaultMapping::None);
}

static void
utf16_expand(std::vector<codepoint> &result,
             const uint16_t *utf16,
             const uint16_t *utf16end)
{
  while (!utf16end || utf16 < utf16end) {
    uint16_t cu0 = *utf16++;

    if (!cu0)
      break;

    if (cu0 >= 0xd800 && cu0 <= 0xdbff) {
      if (utf16end && utf16 >= utf16end)
        break;

      uint16_t cu1 = *utf16++;

      result.push_back(0x10000 + (((cu0 & 0x3ff) << 10) | (cu1 & 0x3ff)));
    } else {
      result.push_back(cu0);
    }
  }
}

static std::vector<codepoint>
case_mapping(const struct ucd_case *pcase,
             codepoint from_cp,
             MappingType fos=MappingType::Full,
             DefaultMapping dm=DefaultMapping::Codepoint)
{
  std::vector<codepoint> result;
  const struct ucd_case_range *range = find_case_range(pcase, from_cp);

  if (!range) {
    switch (dm) {
    case DefaultMapping::Codepoint:
      result.push_back(from_cp);
      break;
    case DefaultMapping::None:
      break;
    }
    return result;
  }

  codepoint base = UCD_CASE_RANGE_CP(range->entry);
  switch (UCD_CASE_RANGE_KIND(range->entry)) {
  case UCD_CASE_RANGE_SINGLE:
    result.push_back(range->mapped_cp);
    break;
  case UCD_CASE_RANGE_DELTA:
    result.push_back(from_cp + range->delta.delta);
    break;
  case UCD_CASE_RANGE_EVEN:
    result.push_back(from_cp & ~1);
    break;
  case UCD_CASE_RANGE_ODD:
    result.push_back(from_cp | 1);
    break;
  case UCD_CASE_RANGE_EVEN_UP:
    result.push_back(from_cp + (from_cp & 1));
    break;
  case UCD_CASE_RANGE_ODD_DOWN:
    result.push_back(from_cp - ((~from_cp) & 1));
    break;
  case UCD_CASE_RANGE_PACKED:
    utf16_expand(result, range->packed, &range->packed[4]);
    break;
  case UCD_CASE_RANGE_EXTERNAL:
    {
      uint16_t *ptr = (uint16_t *)((uint8_t *)pcase + range->ext_offset);
      utf16_expand(result, ptr, nullptr);
    }
    break;
  case UCD_CASE_RANGE_TABLE:
    {
      uint32_t *table = (uint32_t *)((uint8_t *)pcase + range->table.offset);
      result.push_back(table[from_cp - base]);
    }
    break;
  case UCD_CASE_RANGE_UCS4TBL:
    result.push_back(range->ucs4[from_cp - base]);
    break;
  case UCD_CASE_RANGE_UCS2T3:
  case UCD_CASE_RANGE_UCS2T4:
    result.push_back(range->ucs2[from_cp - base]);
    break;
  case UCD_CASE_RANGE_SF_PACKED:
    switch (fos) {
    case MappingType::Full:
      utf16_expand(result,
                   range->sf_packed.full_packed,
                   &range->sf_packed.full_packed[2]);
      break;
    case MappingType::Simple:
      result.push_back(range->sf_packed.simple_cp);
      break;
    }
    break;
  case UCD_CASE_RANGE_SF_EXTERNAL:
    switch (fos) {
    case MappingType::Full:
      {
        uint16_t *ptr = (uint16_t *)((uint8_t *)pcase 
                                     + range->sf_external.full_offset);
        utf16_expand(result, ptr, nullptr);
      }
      break;
    case MappingType::Simple:
      result.push_back(range->sf_external.simple_cp);
      break;
    }
    break;
  case UCD_CASE_RANGE_EMPTY:
    break;
  }

  return result;
}

std::vector<codepoint>
database::uppercase_mapping(codepoint cp) const
{
  return case_mapping(_pimpl->get_CASE(), cp);
}

std::vector<codepoint>
database::lowercase_mapping(codepoint cp) const
{
  return case_mapping(_pimpl->get_case(), cp);
}

std::vector<codepoint>
database::titlecase_mapping(codepoint cp) const
{
  return case_mapping(_pimpl->get_Case(), cp);
}

std::vector<codepoint>
database::case_folding(codepoint cp) const
{
  return case_mapping(_pimpl->get_csef(), cp, MappingType::Full);
}

std::vector<codepoint>
database::nfkc_casefold(codepoint cp) const
{
  return case_mapping(_pimpl->get_kccf(), cp);
}

std::vector<codepoint>
database::fc_nfkc_closure(codepoint cp) const
{
  return case_mapping(_pimpl->get_nfkc(), cp, MappingType::Full,
                      DefaultMapping::None);
}

const class block *
database::block(codepoint cp) const
{
  if (!_pimpl->blocks.size())
    _pimpl->init_blocks();

  unsigned min = 0, max = (unsigned)_pimpl->blocks.size(), mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint first = _pimpl->blocks[mid].first();
    codepoint last = _pimpl->blocks[mid].last();

    if (cp < first)
      max = mid;
    else if (cp > last)
      min = mid + 1;
    else
      return &_pimpl->blocks[mid];
  }

  return nullptr;
}

const std::vector<block> &
database::blocks() const
{
  if (!_pimpl->blocks.size())
    _pimpl->init_blocks();

  return _pimpl->blocks;
}

const class block *
database::block_from_name(const std::string &name) const
{
  std::string stripped;

  for (auto it = name.begin(); it != name.end(); ++it) {
    while (char_is_space(*it)
           || *it == '_'
           || *it == '-')
      ++it;
    if (it == name.end())
      break;
    char ch = *it;
    if (ch >= 'A' && ch <= 'Z')
      ch = ch - 'A' + 'a';
    stripped += ch;
  }

  const char *nstr = stripped.c_str();

  if (!_pimpl->blocks.size())
    _pimpl->init_blocks();

  for (auto i = _pimpl->blocks.begin(); i != _pimpl->blocks.end(); ++i) {
    if (loose_match(i->name().c_str(), NULL, nstr, LOOSE_IGNORE_DASHES) == 0
        || loose_match(i->alias().c_str(), NULL, nstr, LOOSE_IGNORE_DASHES) == 0)
      return &*i;
  }

  return nullptr;
}

nt
database::numeric_type(codepoint cp) const
{
  const struct ucd_numb *pnumb = _pimpl->get_numb();

  unsigned min = 0, max = pnumb->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_NUMB_ENTRY_CP(pnumb->entries[mid].entry);

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else
      return nt(UCD_NUMB_ENTRY_TYPE(pnumb->entries[mid].entry));
  }

  return Numeric_Type::None;
}

numeric
database::numeric_value(codepoint cp) const
{
  const struct ucd_numb *pnumb = _pimpl->get_numb();

  unsigned min = 0, max = pnumb->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_NUMB_ENTRY_CP(pnumb->entries[mid].entry);

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else {
      return numeric(pnumb->entries[mid].multiplier,
                     pnumb->entries[mid].base,
                     pnumb->entries[mid].exponent);
    }
  }

  return NaN;
}

bc
database::bidi_class(codepoint cp) const
{
  const struct ucd_bidi *pbidi = _pimpl->get_bidi();

  unsigned min = 0, max = pbidi->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_BIDI_ENTRY_CP(pbidi->entries[mid]);
    codepoint lcp = UCD_BIDI_ENTRY_CP(pbidi->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= lcp)
      min = mid + 1;
    else
      return bc(UCD_BIDI_ENTRY_CLASS(pbidi->entries[mid]));
  }

  return Bidi_Class::L;
}

dt
database::decomposition_type(codepoint cp) const
{
  const struct ucd_deco *pdeco = _pimpl->get_deco();

  if (is_decomposable_hangul(cp))
    return Decomposition_Type::Canonical;

  unsigned min = 0, max = pdeco->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    const struct ucd_deco_entry &entry = pdeco->entries[mid];
    codepoint ecp = entry.codepoint;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else
      return dt(entry.tag);
  }

  return Decomposition_Type::None;
}

size_t
database::decomposition_mapping(codepoint cp,
                                codepoint *out,
                                size_t out_len) const
{
  dt dummy;
  return decomposition_mapping(cp, out, out_len, dummy);
}

size_t
database::decomposition_mapping(codepoint cp,
                                codepoint *out,
                                size_t out_len,
                                dt &dtype) const
{
  const struct ucd_deco *pdeco = _pimpl->get_deco();

  if (is_decomposable_hangul(cp)) {
    unsigned SIndex = cp - SBase;
    unsigned TIndex = SIndex % TCount;

    dtype = Decomposition_Type::Canonical;

    if (!TIndex) {
      unsigned LIndex = SIndex / NCount;
      unsigned VIndex = (SIndex % NCount) / TCount;

      if (out_len >= 1)
        out[0] = LBase + LIndex;
      if (out_len >= 2)
        out[1] = VBase + VIndex;

      return 2;
    } else {
      unsigned LVIndex = (SIndex / TCount) * TCount;

      if (out_len >= 1)
        out[0] = SBase + LVIndex;
      if (out_len >= 2)
        out[1] = TBase + TIndex;

      return 2;
    }
  }

  unsigned min = 0, max = pdeco->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    const struct ucd_deco_entry &entry = pdeco->entries[mid];
    codepoint ecp = entry.codepoint;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else {
      dtype = dt(entry.tag);

      switch (entry.kind) {
      case UCD_DECO_ENTRY_SINGLE:
        if (out_len >= 1)
          *out = entry.mapped_cp;
        return 1;
      case UCD_DECO_ENTRY_PACKED:
        return utf16_expand(entry.packed, entry.packed + 2, out, out_len);
      case UCD_DECO_ENTRY_EXTERNAL:
        {
          uint16_t *putf16 = (uint16_t *)((uint8_t *)pdeco + entry.ext_offset);
          return utf16_expand(putf16, nullptr, out, out_len);
        }
      default:
        if (out_len >= 1)
          *out = cp;
        return 1;
      }
    }
  }

  if (out_len >= 1) {
    dtype = Decomposition_Type::None;
    *out = cp;
  }
  return 1;
}

std::vector<codepoint>
database::decomposition_mapping(codepoint cp) const
{
  dt dummy;
  return decomposition_mapping(cp, dummy);
}

std::vector<codepoint>
database::decomposition_mapping(codepoint cp, dt &dtype) const
{
  const struct ucd_deco *pdeco = _pimpl->get_deco();
  std::vector<codepoint> result;
  unsigned min = 0, max = pdeco->num_entries, mid;

  if (is_decomposable_hangul(cp)) {
    unsigned SIndex = cp - SBase;
    unsigned TIndex = SIndex % TCount;

    dtype = Decomposition_Type::Canonical;

    if (!TIndex) {
      unsigned LIndex = SIndex / NCount;
      unsigned VIndex = (SIndex % NCount) / TCount;

      result.push_back(LBase + LIndex);
      result.push_back(VBase + VIndex);
    } else {
      unsigned LVIndex = (SIndex / TCount) * TCount;

      result.push_back(SBase + LVIndex);
      result.push_back(TBase + TIndex);
    }

    return result;
  }

  while (min < max) {
    mid = (min + max) / 2;

    const struct ucd_deco_entry &entry = pdeco->entries[mid];
    codepoint ecp = entry.codepoint;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else {
      dtype = dt(entry.tag);

      switch (entry.kind) {
      case UCD_DECO_ENTRY_SINGLE:
        result.push_back(entry.mapped_cp);
        break;
      case UCD_DECO_ENTRY_PACKED:
        utf16_expand(result, entry.packed, entry.packed + 2);
        break;
      case UCD_DECO_ENTRY_EXTERNAL:
        {
          uint16_t *putf16 = (uint16_t *)((uint8_t *)pdeco + entry.ext_offset);
          utf16_expand(result, putf16, nullptr);
        }
        break;
      default:
        result.push_back(cp);
        break;
      }
      return result;
    }
  }

  dtype = Decomposition_Type::None;
  result.push_back(cp);
  return result;
}

codepoint
database::primary_composite(codepoint starter, codepoint composing) const
{
  const struct ucd_prmc *pprmc = _pimpl->get_prmc();

  // Deal with Hangul
  if (starter >= LBase && starter < LBase + LCount
      && composing >= VBase && composing < VBase + VCount) {
    unsigned LIndex = starter - LBase;
    unsigned VIndex = composing - VBase;
    unsigned LVIndex = LIndex * NCount + VIndex * TCount;

    return SBase + LVIndex;
  } else if (starter >= SBase && starter < SBase + SCount
             && composing >= TBase && composing < TBase + TCount) {
    unsigned SIndex = starter - SBase;

    if ((SIndex % TCount) == 0) {
      unsigned TIndex = composing - TBase;

      return starter + TIndex;
    }
  }

  // Otherwise, look up in the prmc table
  unsigned min = 0, max = pprmc->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    const struct ucd_prmc_entry &entry = pprmc->entries[mid];

    if (starter < entry.starter
        || (starter == entry.starter && composing < entry.composing))
      max = mid;
    else if (starter > entry.starter
             || (starter == entry.starter && composing > entry.composing))
      min = mid + 1;
    else
      return entry.composite;
  }

  return 0;
}

bool
database::bidi_mirrored(codepoint cp) const
{
  const struct ucd_mirr *pmirr = _pimpl->get_mirr();

  unsigned min = 0, max = pmirr->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pmirr->entries[mid].cp;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else
      return true;
  }

  return false;
}

codepoint
database::bidi_mirroring_glyph(codepoint cp) const
{
  const struct ucd_mirr *pmirr = _pimpl->get_mirr();

  unsigned min = 0, max = pmirr->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pmirr->entries[mid].cp;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else
      return pmirr->entries[mid].mcp;
  }

  return bad_codepoint;
}

codepoint
database::bidi_paired_bracket(codepoint cp, bpt &type) const
{
  const struct ucd_brak *pbrak = _pimpl->get_brak();

  unsigned min = 0, max = pbrak->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_BRAK_ENTRY_CP(pbrak->entries[mid].entry);

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else {
      type = bpt(UCD_BRAK_ENTRY_TYPE(pbrak->entries[mid].entry));
      return pbrak->entries[mid].other_cp;
    }
  }

  type = Bidi_Paired_Bracket_Type::None;
  return bad_codepoint;
}

codepoint
database::bidi_paired_bracket(codepoint cp) const
{
  bpt type;
  return bidi_paired_bracket(cp, type);
}

bpt
database::bidi_paired_bracket_type(codepoint cp) const
{
  bpt type;
  bidi_paired_bracket(cp, type);
  return type;
}

version
database::age(codepoint cp) const
{
  const struct ucd_age *page = _pimpl->get_age();
  const struct ucd_age_entries *pentries \
    = (const struct ucd_age_entries *)(page->versions + page->num_versions);

  // There is a sentinel on the age table
  unsigned min = 0, max = pentries->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_AGE_CP(pentries->entries[mid]);
    codepoint ncp = UCD_AGE_CP(pentries->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else {
      unsigned vndx = UCD_AGE_VERSION_NDX(pentries->entries[mid]);
      if (vndx >= page->num_versions)
        return version::nil;

      return version(UCD_AGE_MAJOR(page->versions[vndx]),
                     UCD_AGE_MINOR(page->versions[vndx]),
                     0);
    }
  }

  return version::nil;
}

sc
database::script(codepoint cp) const
{
  const struct ucd_scpt *pscpt = _pimpl->get_scpt();

  // There is a sentinel on the script table
  unsigned min = 0, max = pscpt->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pscpt->entries[mid].code_point;
    codepoint ncp = pscpt->entries[mid + 1].code_point;

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return sc(pscpt->entries[mid].script);
  }

  return Script::Unknown;
}

std::vector<sc>
database::script_extensions(codepoint cp) const
{
  std::vector<sc> result;
  const struct ucd_scpt *pscpt = _pimpl->get_scpt();
  const struct ucd_scpt_extensions *psext
    = (const struct ucd_scpt_extensions *)(pscpt->entries + pscpt->num_entries);

  // There is a sentinel on the script extensions table
  unsigned min = 0, max = psext->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_SCPT_EXT_CP(psext->entries[mid].entry);
    codepoint ncp = UCD_SCPT_EXT_CP(psext->entries[mid + 1].entry);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else {
      unsigned count = UCD_SCPT_EXT_COUNT(psext->entries[mid].entry);
      const uint32_t *scripts = (const uint32_t *)((uint8_t *)pscpt
                                                   + psext->entries[mid].offset);

      result.assign(scripts, scripts + count);

      return result;
    }
  }

  return result;
}

static maybe
get_qc(const struct ucd_qc *pqc, codepoint cp) {
  unsigned min = 0, max = pqc->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_QUICK_CHECK_CP(pqc->entries[mid]);
    codepoint ncp = UCD_QUICK_CHECK_CP(pqc->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return maybe(UCD_QUICK_CHECK_VALUE(pqc->entries[mid]));
  }

  return maybe::yes;
}

maybe
database::nfc_quick_check(codepoint cp) const
{
  return get_qc(_pimpl->get_nfcqc(), cp);
}

maybe
database::nfkc_quick_check(codepoint cp) const
{
  return get_qc(_pimpl->get_nfkcqc(), cp);
}

maybe
database::nfd_quick_check(codepoint cp) const
{
  return get_qc(_pimpl->get_nfdqc(), cp);
}

maybe
database::nfkd_quick_check(codepoint cp) const
{
  return get_qc(_pimpl->get_nfkdqc(), cp);
}

jt
database::joining_type(codepoint cp, jg &jgroup) const
{
  const struct ucd_join *pjoin = _pimpl->get_join();

  unsigned min = 0, max = pjoin->num_entries, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = pjoin->entries[mid].codepoint;

    if (cp < ecp)
      max = mid;
    else if (cp > ecp)
      min = mid + 1;
    else {
      jgroup = Joining_Group(pjoin->entries[mid].group);
      return Joining_Type(pjoin->entries[mid].type);
    }
  }

  jgroup = Joining_Group::No_Joining_Group;

  gc gcat = general_category(cp);

  if (gcat == General_Category::Mn
      || gcat == General_Category::Me
      || gcat == General_Category::Cf)
    return Joining_Type::Transparent;

  return Joining_Type::Non_Joining;
}

jg
database::joining_group(codepoint cp) const
{
  jg jgroup;
  joining_type(cp, jgroup);
  return jgroup;
}

jt
database::joining_type(codepoint cp) const
{
  jg dummy;
  return joining_type(cp, dummy);
}

lb
database::line_break(codepoint cp) const
{
  const struct ucd_brk *pbrk = _pimpl->get_lbrk();

  // There is a sentinel on the lbrk table
  unsigned min = 0, max = pbrk->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_BRK_CODEPOINT(pbrk->entries[mid]);
    codepoint ncp = UCD_BRK_CODEPOINT(pbrk->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return Line_Break(UCD_BRK_BREAK(pbrk->entries[mid]));
  }

  return Line_Break::Unknown;
}

GCB
database::grapheme_cluster_break(codepoint cp) const
{
  const struct ucd_brk *pbrk = _pimpl->get_gbrk();

  // There is a sentinel on the gbrk table
  unsigned min = 0, max = pbrk->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_BRK_CODEPOINT(pbrk->entries[mid]);
    codepoint ncp = UCD_BRK_CODEPOINT(pbrk->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else {
      GCB gcb = Grapheme_Cluster_Break(UCD_BRK_BREAK(pbrk->entries[mid]));

      if (gcb == Grapheme_Cluster_Break::LVT) {
        unsigned SIndex = cp - SBase;
        unsigned TIndex = SIndex % TCount;

        if (!TIndex)
          gcb = Grapheme_Cluster_Break::LV;
      }

      return gcb;
    }
  }

  return Grapheme_Cluster_Break::Other;
}

SB
database::sentence_break(codepoint cp) const
{
  const struct ucd_brk *pbrk = _pimpl->get_sbrk();

  // There is a sentinel on the lbrk table
  unsigned min = 0, max = pbrk->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_BRK_CODEPOINT(pbrk->entries[mid]);
    codepoint ncp = UCD_BRK_CODEPOINT(pbrk->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return Sentence_Break(UCD_BRK_BREAK(pbrk->entries[mid]));
  }

  return Sentence_Break::Other;
}

WB
database::word_break(codepoint cp) const
{
  const struct ucd_brk *pbrk = _pimpl->get_wbrk();

  // There is a sentinel on the lbrk table
  unsigned min = 0, max = pbrk->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_BRK_CODEPOINT(pbrk->entries[mid]);
    codepoint ncp = UCD_BRK_CODEPOINT(pbrk->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return Word_Break(UCD_BRK_BREAK(pbrk->entries[mid]));
  }

  return Word_Break::Other;
}

ea
database::east_asian_width(codepoint cp) const
{
  const struct ucd_eaw *peaw = _pimpl->get_eaw();

  // There is a sentinel on the eaw table
  unsigned min = 0, max = peaw->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_EAW_CODEPOINT(peaw->entries[mid]);
    codepoint ncp = UCD_EAW_CODEPOINT(peaw->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return East_Asian_Width(UCD_EAW_WIDTH(peaw->entries[mid]));
  }

  return East_Asian_Width::Neutral;
}

InPC
database::indic_positional_category(codepoint cp) const
{
  const struct ucd_inc *pinc = _pimpl->get_inmc();

  // There is a sentinel on the inmc table
  unsigned min = 0, max = pinc->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_INC_CODEPOINT(pinc->entries[mid]);
    codepoint ncp = UCD_INC_CODEPOINT(pinc->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return Indic_Positional_Category(UCD_INC_CATEGORY(pinc->entries[mid]));
  }

  return Indic_Positional_Category::NA;
}

InSC
database::indic_syllabic_category(codepoint cp) const
{
  const struct ucd_inc *pinc = _pimpl->get_insc();

  // There is a sentinel on the insc table
  unsigned min = 0, max = pinc->num_entries - 1, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint ecp = UCD_INC_CODEPOINT(pinc->entries[mid]);
    codepoint ncp = UCD_INC_CODEPOINT(pinc->entries[mid + 1]);

    if (cp < ecp)
      max = mid;
    else if (cp >= ncp)
      min = mid + 1;
    else
      return Indic_Syllabic_Category(UCD_INC_CATEGORY(pinc->entries[mid]));
  }

  return Indic_Syllabic_Category::Other;
}

static bool
get_binprop(const struct ucd_binprop *pbp, codepoint cp) {
  unsigned min = 0, max = pbp->num_ranges, mid;

  while (min < max) {
    mid = (min + max) / 2;

    codepoint fcp = pbp->ranges[mid].first_cp;
    codepoint lcp = pbp->ranges[mid].last_cp;

    if (cp < fcp)
      max = mid;
    else if (cp > lcp)
      min = mid + 1;
    else
      return true;
  }

  return false;
}

#undef BINPROP
#define BINPROP(n,m,t)                                          \
bool                                                            \
database::m(codepoint cp) const                                 \
{                                                               \
  const struct ucd_binprop *pbp = _pimpl->get_binprop_ ## m (); \
  return get_binprop(pbp, cp);                                  \
}
#include "ucd-binprops.h"

#define NAME_FNS(prop,table,size,type)                                  \
bool                                                                    \
database::prop##_from_name(const std::string &name, type &v) const      \
{                                                                       \
  auto *ptbl = _pimpl->get_##table();                                   \
  uint##size##_t result;                                                \
                                                                        \
  if (_pimpl->search(ptbl, name, result)) {                             \
    v = type(result);                                                   \
    return true;                                                        \
  }                                                                     \
                                                                        \
  return false;                                                         \
}                                                                       \
                                                                        \
std::string                                                             \
database::name_from_##prop(type v) const                                \
{                                                                       \
  const struct ucd_n##size *ptbl = _pimpl->get_##table();               \
  std::string result;                                                   \
                                                                        \
  if (!_pimpl->search(ptbl,                                             \
                      uint##size##_t(v),                                \
                      result))                                          \
    return "<unknown>";                                                 \
                                                                        \
  return result;                                                        \
}

bool
database::general_category_from_name(const std::string &name, gc &result) const
{
  // Cope with arbitrary twocc names
  if (name.size() == 2) {
    gc cat = 0;
    unsigned n;

    for (n = 0; n < 2; ++n) {
      char ch = name[n];

      if ((ch < 'a' || ch > 'z')
          && (ch < 'A' || ch > 'Z'))
        break;

      if (n == 0)
        ch &= ~0x20;
      else
        ch |= 0x20;

      cat = (cat << 8) | ch;
    }

    if (n == 2) {
      result = cat;
      return true;
    }
  }

  const struct ucd_n16 *pgcn = _pimpl->get_gcn();
  uint16_t ures;

  if (_pimpl->search(pgcn, name, ures)) {
    result = gc(ures);
    return true;
  }

  return false;
}

std::string
database::name_from_general_category(gc c) const
{
  const struct ucd_n16 *pgcn = _pimpl->get_gcn();
  std::string result;

  if (!_pimpl->search(pgcn, uint16_t(c), result)) {
    char ch[3];
    ch[0] = (c >> 8) & 0xff;
    ch[1] = c & 0xff;
    ch[2] = 0;
    return ch;
  }

  return result;
}

bool
database::canonical_combining_class_from_name(const std::string &name,
                                              ccc &result) const
{
  const char *pname = name.c_str();

  if ((pname[0] == 'C' || pname[0] == 'c')
      && (pname[1] == 'C' || pname[1] == 'c')
      && (pname[2] == 'C' || pname[2] == 'c')) {
    unsigned n;
    if (std::sscanf(pname + 3, "%u", &n) == 1 && n < 256) {
      result = ccc(n);
      return true;
    }
  }

  const struct ucd_n8 *pcccn = _pimpl->get_cccn();
  uint8_t ures;

  if (_pimpl->search(pcccn, name, ures)) {
    result = ccc(ures);
    return true;
  }

  return false;
}

std::string
database::name_from_canonical_combining_class(ccc c) const
{
  const struct ucd_n8 *pcccn = _pimpl->get_cccn();
  std::string result;

  if (!_pimpl->search(pcccn, uint8_t(c), result)) {
    char buf[8];
    std::sprintf(buf, "CCC%u", c);
    return buf;
  }

  return result;
}

static struct {
  const char *name;
  bpt         bpt;
} bpt_names[] = { { "n", bpt::n },
                  { "o", bpt::o },
                  { "c", bpt::c },
                  { "None", bpt::None },
                  { "Open", bpt::Open },
                  { "Close", bpt::Close } };

bool
database::bidi_paired_bracket_type_from_name(const std::string &name,
                                             bpt &t) const
{
  std::string name_st = _pimpl->strip(name);
  const char *nptr = name_st.c_str();

  for (unsigned n = 0; n < sizeof(bpt_names) / sizeof(bpt_names[0]); ++n) {
    if (loose_match(bpt_names[n].name, NULL, nptr) == 0) {
      t = bpt_names[n].bpt;
      return true;
    }
  }

  return false;
}

std::string
database::name_from_bidi_paired_bracket_type(bpt t) const
{
  if (t > bpt::c)
    return "<unknown>";
  return bpt_names[unsigned(t)].name;
}

bool
database::decomposition_type_from_name(const std::string &name,
                                       dt &t) const
{
  auto *ptbl = _pimpl->get_decn();
  uint8_t result;

  if (_pimpl->search(ptbl, name, result)) {
    if (result == 0xff)
      t = dt::None;
    else
      t = dt(result);
    return true;
  }

  return false;
}

std::string
database::name_from_decomposition_type(dt t) const
{
  auto *ptbl = _pimpl->get_decn();
  std::string result;

  if (!_pimpl->search(ptbl, uint8_t(t), result))
    return "<unknown>";

  return result;
}

NAME_FNS(hangul_syllable_type, jamn, 16, hst)
NAME_FNS(numeric_type, numn, 8, nt)
NAME_FNS(bidi_class, bdin, 8, bc)
NAME_FNS(east_asian_width, eawn, 8, ea)
NAME_FNS(indic_positional_category, imcn, 8, InPC)
NAME_FNS(indic_syllabic_category, iscn, 8, InSC)
NAME_FNS(joining_type, jtn, 8, jt)
NAME_FNS(joining_group, jgn, 8, jg)
NAME_FNS(line_break, lbkn, 8, lb)
NAME_FNS(grapheme_cluster_break, gbkn, 8, GCB)
NAME_FNS(sentence_break, sbkn, 8, SB)
NAME_FNS(word_break, wbkn, 8, WB)

bool
database::script_from_name(const std::string &name, sc &result) const
{
  // Cope with arbitrary fourcc names
  if (name.size() == 4) {
    sc script = 0;
    unsigned n;

    for (n = 0; n < 4; ++n) {
      char ch = name[n];

      if ((ch < 'a' || ch > 'z')
          && (ch < 'A' || ch > 'Z'))
        break;

      if (n == 0)
        ch &= ~0x20;
      else
        ch |= 0x20;

      script = (script << 8) | ch;
    }

    if (n == 4) {
      if (script == 'Qaac')
        script = ucd::SC::Copt;
      else if (script == 'Qaai')
        script = ucd::SC::Zinh;

      result = script;
      return true;
    }
  }

  // Otherwise, search the table
  const struct ucd_n32 *pscpn = _pimpl->get_scpn();
  uint32_t ures;

  if (_pimpl->search(pscpn, name, ures)) {
    result = sc(ures);
    return true;
  }

  return false;
}

std::string
database::name_from_script(sc script) const
{
  const struct ucd_n32 *pscpn = _pimpl->get_scpn();
  std::string result;

  if (!_pimpl->search(pscpn, script, result)) {
    char ch[5];
    ch[0] = script >> 24;
    ch[1] = script >> 16;
    ch[2] = script >> 8;
    ch[3] = script;
    ch[4] = 0;
    return ch;
  }

  return result;
}

