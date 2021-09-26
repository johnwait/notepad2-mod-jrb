/**
 * @file  OnigmoRegExEngine.cxx
 * @brief integrate Onigmo regex engine for Scintilla library
 *        (Scintilla Lib is copyright 1998-2017 by Neil Hodgson <neilh@scintilla.org>)
 *
 *        uses Onigmo - Regular Expression Engine (v6.1.3) (onigmo.h) - https://github.com/k-takata/Onigmo
 *
 *   Onigmo is a regular expressions library forked from Oniguruma (https://github.com/kkos/oniguruma). 
 *   It focuses to support new expressions like \K, \R, (?(cond)yes|no) and etc. which are supported in Perl 5.10+.
 *   Since Onigmo is used as the default regexp library of Ruby 2.0 or later, many patches are backported from Ruby 2.x.
 *
 *   See also the Wiki page: https://github.com/k-takata/Onigmo/wiki
 *
 *
 * @autor Rainer Kottenhoff (RaiKoHoff)
 *
 * TODO: add interface to onig_scan() API (mark occ, hyperlink)
 */

#ifdef SCI_OWNREGEX

 // Added on 2019-08-08
#ifdef _DEBUG
#  define ONIG_DEBUG_PARSE_TREE 1
#  define ONIG_DEBUG_COMPILE    1
#  define ONIG_DEBUG_SEARCH     1
#  define ONIG_DEBUG_MATCH      1
#endif // _DEBUG

// clang-format off
#include <cstdlib>
#include <string>
#include <vector>
#include <sstream>
// 2020-01-30: Below: added for std::transform() in OnigmoRegExEngine::SubstituteByPosition()
#include <algorithm>
#include <locale>

// 2021-07-10: Feature flag for backref transforms until implementation if complete
//             NOTE: Does not control inclusion (oe exclusion) of their documentation
//                   within the regex syntax (RTF) file
#define X_ONIG_BACKREF_TRANSFORMS

// 2020-01-30: Implemented as a helper for backref transforms
std::string toTitle( std::string s, const std::locale& loc = std::locale() ) {
    bool last = true;
    for (char& ch : s) {
        ch = last ? std::toupper( ch, loc ) : std::tolower( ch, loc );
        last = std::isspace( ch, loc );
    }
    return s;
}

// 2021-07-09: Implemented as (yet) another helper for backref transforms
std::string toProper( std::string s, const std::locale& loc = std::locale() ) {
    bool last = true;
    for (char& ch : s) {
        ch = last ? std::toupper( ch, loc ) : std::tolower( ch, loc );
        last = std::isspace( ch, loc ) || std::ispunct( ch, loc );
    }
    return s;
}

#define VC_EXTRALEAN 1
#include <windows.h>

#pragma warning( push )
#pragma warning( disable : 4996 )   // Scintilla's "unsafe" use of std::copy() (SplitVector.h)
//                                  // or use -D_SCL_SECURE_NO_WARNINGS preprocessor define

#ifndef USE_NAMED_GROUP
#define USE_NAMED_GROUP
#endif
#ifndef USE_BACKREF_WITH_LEVEL
#define USE_BACKREF_WITH_LEVEL
#endif
#ifndef USE_SUBEXP_CALL
#define USE_SUBEXP_CALL
#endif
#ifndef USE_WORD_BEGIN_END
#define USE_WORD_BEGIN_END
#endif

#include "Platform.h"
#include "Scintilla.h"
#include "ILoader.h"
#include "ILexer.h"
#include "SplitVector.h"
#include "Partitioning.h"
#include "CellBuffer.h"
#include "CaseFolder.h"
#include "RunStyles.h"
#include "Decoration.h"
#include "CharClassify.h"
#include "Document.h"

// ---------------------------------------------------------------
#ifdef ONIG_ESCAPE_UCHAR_COLLISION
#undef ONIG_ESCAPE_UCHAR_COLLISION
#endif
#include "onigmo.h"   // Onigmo - Regular Expression Engine (v6.2.0)
// ---------------------------------------------------------------
// clang-format on


#define UCharPtr(pchar)     reinterpret_cast<OnigUChar*>(pchar)
#define UCharCPtr(pchar)    reinterpret_cast<const OnigUChar*>(pchar)

using namespace Scintilla;

#define SciPos(pos)     static_cast<Sci::Position>(pos)
#define SciLn(line)     static_cast<Sci::Line>(line)
#define SciPosExt(pos)  static_cast<Sci_Position>(pos)

// ============================================================================
// ***   Oningmo configuration   ***
// ============================================================================

#define ONIG_OPTION_ONOFF(opt, boolswitch, flags) (boolswitch ? ONIG_OPTION_ON(opt, flags) : ONIG_OPTION_OFF(opt, flags))

constexpr OnigEncoding g_pOnigEncodingType = ONIG_ENCODING_UTF8; // ONIG_ENCODING_ASCII;
static OnigEncoding g_UsedEncodingsTypes[] = { g_pOnigEncodingType };

// ============================================================================
// ============================================================================

class OnigmoRegExEngine : public RegexSearchBase {

public:
    explicit OnigmoRegExEngine(CharClassify* charClassTable)
        // Base ourselves on Perl v5.10's syntax (+/- a few features)
        : m_OnigSyntax(*ONIG_SYNTAX_PERL)
        // Default options (i.e. none)
        , m_CmplOptions(ONIG_OPTION_DEFAULT)
        , m_RegExpr(nullptr)
        , m_Region({ 0, 0, nullptr, nullptr, nullptr })
        , m_ErrorInfo()
        , m_MatchPos(ONIG_MISMATCH)
        , m_MatchLen(0)
    {
        OutputDebugString(L"Test from OutputDebugString()\n");
        fwprintf(stderr, L"Test from %s\n", L"stderr");

        // 2020-01-30: Explicitely set default std::locale
        std::locale::global(std::locale(""));

        // Add to base syntax support for GNU's \< and \> word boundaries
        m_OnigSyntax.op |= ONIG_SYN_OP_ESC_LTGT_WORD_BEGIN_END;
        // Other changes to syntax:
        // + Subexpression calls of the sort: \g<name>, \g<#>, \g'name', \g'#'
        // + The escaped vertical tab: \v
        // + Un-braced hexa Unicode code points: \uHHHH
        m_OnigSyntax.op2 |= ONIG_SYN_OP2_ESC_G_SUBEXP_CALL |
                            ONIG_SYN_OP2_ESC_V_VTAB |
                            ONIG_SYN_OP2_ESC_U_HEX4;
        // - Named backreference \g{name}
        m_OnigSyntax.op2 &= ~ONIG_SYN_OP2_ESC_G_BRACE_BACKREF;

        onig_initialize(g_UsedEncodingsTypes, _ARRAYSIZE(g_UsedEncodingsTypes));
        onig_region_init(&m_Region);
    }

    ~OnigmoRegExEngine() override
    {
        onig_region_free(&m_Region, 0);
        onig_free(m_RegExpr);
        onig_end();
    }

    Sci::Position FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char* pattern,
        bool caseSensitive, bool word, bool wordStart, int searchFlags, Sci::Position* length) override;

    const char* SubstituteByPosition(Document* doc, const char* text, Sci::Position* length) override;

    const OnigRegion& GetRegion() const { return m_Region; };

    std::string GetErrorInfo() override;

private:
    std::string& translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart, int eolMode, OnigOptionType& rxOptions);

    std::string& convertReplExpr(std::string& replStr);

    ///void regexFindAndReplace(std::string& inputStr_inout, const std::string& patternStr, const std::string& replStr);

private:

    std::string     m_RegExprStrg;

    OnigSyntaxType  m_OnigSyntax;
    OnigOptionType  m_CmplOptions;
    OnigRegex       m_RegExpr;
    OnigRegion      m_Region;

    char            m_ErrorInfo[ONIG_MAX_ERROR_MESSAGE_LEN];

    Sci::Position   m_MatchPos;
    Sci::Position   m_MatchLen;

public:

    std::string     m_SubstBuffer;

};
// ============================================================================


RegexSearchBase *Scintilla::CreateRegexSearch(CharClassify* charClassTable)
{
    return new OnigmoRegExEngine(charClassTable);
}

// ============================================================================



// ============================================================================
//   Some Helpers
// ============================================================================


/******************************************************************************
 *
 *  UnSlash functions
 *  Mostly taken from SciTE, (c) Neil Hodgson, http://www.scintilla.org
 *
 * Is the character an octal digit?
 */
constexpr bool IsOctalDigit(char ch)
{
    return ((ch >= '0') && (ch <= '7'));
}
// ----------------------------------------------------------------------------

/**
 * If the character is an hex digit, get its value.
 */
constexpr int GetHexDigit(char ch)
{
    if ((ch >= '0') && (ch <= '9')) {
        return (ch - '0');
    }
    if ((ch >= 'A') && (ch <= 'F')) {
        return (ch - 'A' + 10);
    }
    if ((ch >= 'a') && (ch <= 'f')) {
        return (ch - 'a' + 10);
    }
    return -1;
}
// ----------------------------------------------------------------------------

static void replaceAll(std::string& source, const std::string& from, const std::string& to)
{
    std::string newString;
    newString.reserve(source.length() * 2); // avoids a few memory allocations

    std::string::size_type lastPos = 0;
    std::string::size_type findPos;

    while (std::string::npos != (findPos = source.find(from, lastPos))) {
        newString.append(source, lastPos, findPos - lastPos);
        newString += to;
        lastPos = findPos + from.length();
    }
    // Care for the rest after last occurrence
    newString += source.substr(lastPos);

    source.swap(newString);
}
// ----------------------------------------------------------------------------


/**
 * Find text in document, supporting both forward and backward
 * searches (just pass minPos > maxPos to do a backward search)
 * Has not been tested with backwards DBCS searches yet.
 */
Sci::Position OnigmoRegExEngine::FindText(Document* doc, Sci::Position minPos, Sci::Position maxPos, const char* pattern,
                                          bool caseSensitive, bool word, bool wordStart, int searchFlags, Sci::Position* length)
/*
 * Error return values:
 *  -1 => Not found
 *  -2 => Invalid regex
 *  -3 => ONIGMO-specific error
 *  -4 => (Caught) Exception
 *
 * INVALID_REGEX, REGEX_ERROR & REGEX_EXCEPTION are defined in Scintilla.h
 */
{
    if (!(pattern && (strlen(pattern) > 0))) {
        *length = 0;
        return SciPos(INVALID_POSITION);
    }

    auto const docLen = SciPos(doc->Length());

    bool const findForward = (minPos <= maxPos);
    int const increment = findForward ? 1 : -1;

    // Range endpoints should not be inside DBCS characters, but just in case, move them.
    minPos = doc->MovePositionOutsideChar(minPos, increment, false);
    maxPos = doc->MovePositionOutsideChar(maxPos, increment, false);

    Sci::Position const rangeBeg = (findForward) ? minPos : maxPos;
    Sci::Position const rangeEnd = (findForward) ? maxPos : minPos;
    Sci::Position const rangeLen = (rangeEnd - rangeBeg);

    // -----------------------------
    // --- Onigmo Engine Options ---
    // -----------------------------

    // fixed options
    OnigOptionType onigmoOptions = ONIG_OPTION_DEFAULT;

    // Default options before flags are processed
    ONIG_OPTION_OFF(onigmoOptions, ONIG_OPTION_EXTEND);
    ONIG_OPTION_OFF(onigmoOptions, ONIG_OPTION_SINGLELINE);
    ONIG_OPTION_ON(onigmoOptions, ONIG_OPTION_NEGATE_SINGLELINE);

    ONIG_OPTION_ONOFF(onigmoOptions, searchFlags & SCFIND_DOT_MATCH_ALL, ONIG_OPTION_MULTILINE);
    ONIG_OPTION_ONOFF(onigmoOptions, searchFlags & SCFIND_DOT_MATCH_ALL, ONIG_SYN_OP_DOT_ANYCHAR);
    ONIG_OPTION_ONOFF(onigmoOptions, !caseSensitive, ONIG_OPTION_IGNORECASE);
    ONIG_OPTION_ON(onigmoOptions, (rangeBeg != 0) ? ONIG_OPTION_NOTBOL : ONIG_OPTION_NONE);
    ONIG_OPTION_ON(onigmoOptions, (rangeEnd != docLen) ? ONIG_OPTION_NOTEOL : ONIG_OPTION_NONE);

    std::string sPattern(pattern);
    std::string const& sRegExprStrg = translateRegExpr(sPattern, word, wordStart, doc->eolMode, onigmoOptions);

    // 2019-08-26: Now only considering compile-time options when checking whether their change
    //             requires the recompilation of an otherwise unchanged regex pattern
    // @github-fixes #5
    short compileTimeOptionsMask = ONIG_OPTION_NOTBOL - 1; // to exclude search-time options (and higher), ignored dusing RE compilation
    bool const bReCompile = (m_RegExpr == nullptr) ||
                            ((m_CmplOptions & compileTimeOptionsMask) != (onigmoOptions & compileTimeOptionsMask)) ||
                            (m_RegExprStrg.compare(sRegExprStrg) != 0);

    if (bReCompile) {
        m_RegExprStrg.clear();
        m_RegExprStrg = sRegExprStrg;
        m_CmplOptions = onigmoOptions;
        m_ErrorInfo[0] = '\0';
        try {
            OnigErrorInfo einfo;
            onig_free(m_RegExpr);
            int res = onig_new(&m_RegExpr, UCharCPtr(m_RegExprStrg.c_str()), UCharCPtr(m_RegExprStrg.c_str() + m_RegExprStrg.length()),
                               m_CmplOptions, g_pOnigEncodingType, &m_OnigSyntax, &einfo);
            if (res != ONIG_NORMAL) {
                onig_error_code_to_str(UCharPtr(m_ErrorInfo), res, &einfo);
                return SciPos(INVALID_REGEX); // -1 is normally used for not found, -2 is used here for invalid regex
            }
        } catch (...) {
            return SciPos(INVALID_REGEX);
        }
    }

    m_MatchPos = SciPos(ONIG_MISMATCH); // not found
    m_MatchLen = SciPos(0);

    // ---  search document range for pattern match   ---
    // !!! Performance issue: Scintilla: moving Gap needs memcopy - high costs for find/replace in large document
    // 2019-08-12: Modified Scintilla's code to use something other than
    //             RangePointer(), which is optimized for insertion of text
    //             and thus has a habit of giving out pointers passed
    //             deleted text.
    //
    // TODO: Report bug at least to the Notepad3 project
    // Steps to reproduce bug:
    //  1) Paste some predefined text in a Scntilla's edit window.
    //  2) Devise a regular expression that matches the whole text, using
    //     at least the $ anchor and making sure the regex doesn't
    //     end with any of the quantifiers (?, *, + or {m,n}) but instead
    //     a fixed length section, e.g. [..](?:-(?:20|19)[0-9][0-9]))
    //  3) Match the regex pattern 
    auto const docBegPtr = UCharCPtr(doc->RangePointer(0, docLen));
    auto const docEndPtr = UCharCPtr(doc->RangePointer(docLen, 0));
    auto const rangeBegPtr = UCharCPtr(doc->RangePointer(rangeBeg, rangeLen));
    auto const rangeEndPtr = UCharCPtr(doc->RangePointer(rangeEnd, 0));

    OnigPosition result = ONIG_MISMATCH;
    try {
        onig_region_free(&m_Region, 0);
        onig_region_init(&m_Region);

        if (findForward)
            result = onig_search(m_RegExpr, docBegPtr, docEndPtr, rangeBegPtr, rangeEndPtr, &m_Region, onigmoOptions);
        else
            result = onig_search(m_RegExpr, docBegPtr, docEndPtr, rangeEndPtr, rangeBegPtr, &m_Region, onigmoOptions);
    } catch (...) {
        return SciPos(REGEX_EXCEPTION); // -4 here is used here for an uncaught exception
    }

    if (result < ONIG_MISMATCH) {
        onig_error_code_to_str(UCharPtr(m_ErrorInfo), result);
        return SciPos(REGEX_ERROR); // // -3 is used ONIGMO-specific errors
    }

    if ((result >= 0) && (rangeBegPtr <= rangeEndPtr)) {
        ///~m_MatchPos = SciPos(result);
        m_MatchPos = SciPos(m_Region.beg[0]);
        ///~m_MatchLen = SciPos(m_Region.end[0] - result);
        m_MatchLen = SciPos(m_Region.end[0] - m_Region.beg[0]);
    }
    // NOTE: potential 64-bit-size issue at interface here:
    *length = m_MatchLen;
    return SciPos(m_MatchPos);
}
// ============================================================================

std::string OnigmoRegExEngine::GetErrorInfo() {
	std::string sText(m_ErrorInfo, strlen(m_ErrorInfo));
    return sText;
}
/*
static int GrpNameCallback(const UChar* name, const UChar* name_end,
                           int ngroup_num, int* group_nums, regex_t* reg, void* arg)
{
    OnigmoRegExEngine* pRegExInstance = dynamic_cast<OnigmoRegExEngine*>(arg);

    const OnigRegion& region = pRegExInstance->GetRegion();

    for (int i = 0; i < ngroup_num; i++) {
        int grpNum = group_nums[i];

        int ref = onig_name_to_backref_number(reg, name, name_end, &region);

        if (ref == grpNum) {

            Sci::Position rBeg = SciPos(region.beg[grpNum]);
            Sci::Position len = SciPos(region.end[grpNum] - rBeg);

            (pRegExInstance->m_SubstBuffer).append(doc->RangePointer(rBeg, len), (size_t)len);

        }
    }
    return 0; // 0: continue
}
//   called by:  int r = onig_foreach_name(m_RegExpr, GrpNameCallback, (void*)this);
*/

#ifdef X_ONIG_BACKREF_TRANSFORMS
std::string applyTransform( std::string s, const char chTransform ) {
    switch(chTransform) {
        /*
         * Upper-case letter metas for which a feature/behaviour already exists:
         *   \A..\Z     => as buffer anchors
         *   \B         => non-word boundary
         *   \C         => C-bar control
         *   \D \S \W   => as negated versions of their respective char classes
         *   \E         => unescaped sequence end
         *   \G         => begin anchor
         *   \H         => hex sequence (reserved)
         *   \H         => horizontal whitespace (not implemented)
         *   \K         => "keep"
         *   \M         => M-bar meta
         *   \P
         *   \Q         => unescaped sequence start
         *   \R         => line-break sequence i.e. (?>\x0D\x0A|[\x0A-\x0D\x{85}\x{2028}\x{2029}])
         *   \V         => vertical whitespace (not implemented)
         *   \X         => extended grapheme cluster
         *
         * Upper-case letter metas otherwise available:
         *  \F \I \J \L \N \O \T \U \Y
         *
         * [Personally, I would keep the \Y as a multiplexer/branching meta,
         *  since we'll run out of letters quick if we intend to implement
         *  other features]
         */
        case 'L': // lower-case:  llll lll-ll llllll
        case 'F': // Phrase-case: Ulll lll-ll llllll
            std::transform(s.begin(), s.end(), s.begin(), tolower);
            if (chTransform == 'F') s[0] = toupper(s[0]);
            break;
        case 'I': // Title-case:  Ulll Ull-ll Ulllll
            s = toTitle(s);
            break;
        case 'N': // Name-Case:  Ulll Ull-Ul Ulllll
            s = toProper(s);
            break;
        case 'U': // UPPER-CASE:  UUUU UUU-UU UUUUUU
            std::transform(s.begin(), s.end(), s.begin(), toupper);
            break;
        case 'l': // lower-case, first letter only:  lxxx xxx-xx xxxxxx
            s[0] = tolower(s[0]);
            break;
        case 'u': // UPPER-CASE, first letter only:  Uxxx xxx-xx xxxxxx
            s[0] = toupper(s[0]);
            break;
    }
    return s;
}
#endif // X_ONIG_BACKREF_TRANSFORMS

// ============================================================================

// 2020-01-30: Added preliminary support for transform sequences in replacement strings
const char* OnigmoRegExEngine::SubstituteByPosition(Document* doc, const char* text, Sci::Position* length)
{
    if (m_MatchPos < 0) {
        *length = SciPos(-1);
        return nullptr;
    }
    std::string sText(text, *length);
    std::string const& rawReplStrg = convertReplExpr(sText);
#ifdef X_ONIG_BACKREF_TRANSFORMS
    char chCaseXform = '\0';
#endif // X_ONIG_BACKREF_TRANSFORMS

    m_SubstBuffer.clear();

    //TODO: allow for arbitrary number of groups/regions

    for (size_t j = 0; j < rawReplStrg.length(); j++) {
        bool bReplaced = false;
#ifdef X_ONIG_BACKREF_TRANSFORMS
        int grpNum = -1;
#endif // X_ONIG_BACKREF_TRANSFORMS
        if ((rawReplStrg[j] == '\\') || (rawReplStrg[j] == '$')) {
            // here, allowing either forms: \0 .. \9 or $0 .. $9
            if ((rawReplStrg[j + 1] >= '0') && (rawReplStrg[j + 1] <= '9')) {
#ifdef X_ONIG_BACKREF_TRANSFORMS
                // only evaluate the backreference index for now (we'll process it later on)
                grpNum = rawReplStrg[j + 1] - '0';
#else // !X_ONIG_BACKREF_TRANSFORMS
                // evaluate capturing group number and perform the substitution
                int const grpNum = rawReplStrg[j + 1] - '0';
                if (grpNum < m_Region.num_regs) {
                    auto const rBeg = SciPos(m_Region.beg[grpNum]);
                    auto const len = SciPos(m_Region.end[grpNum] - rBeg);

                    m_SubstBuffer.append(doc->RangePointer(rBeg, len), static_cast<size_t>(len));
                }
                // mark as replaced
                bReplaced = true;
#endif // X_ONIG_BACKREF_TRANSFORMS
                ++j;
#ifdef X_ONIG_BACKREF_TRANSFORMS
            // anything else that starts with a "\" that we might support
            } else if (rawReplStrg[j] == '\\') {
                // backreferences like \{#}
                if ((rawReplStrg[j + 1] == '{') &&
                    (rawReplStrg[j + 2] >= '0') && (rawReplStrg[j + 2] <= '9') &&
                    (rawReplStrg[j + 3] == '}')) {
                    grpNum = rawReplStrg[j + 2] - '0';
                    bReplaced = true;
                    j += 3;
                // backreferences like \{##}
                } else if ((rawReplStrg[j + 1] == '{') &&
                    (rawReplStrg[j + 2] >= '0') && (rawReplStrg[j + 2] <= '9') &&
                           (rawReplStrg[j + 3] >= '0') && (rawReplStrg[j + 3] <= '9') &&
                           (rawReplStrg[j + 4] == '}')) {
                    grpNum = (rawReplStrg[j + 2] - '0') * 10 + (rawReplStrg[j + 3] - '0');
                    bReplaced = true;
                    j += 4;
                // literals "\"
                } else if (rawReplStrg[j + 1] == '\\') {
                    m_SubstBuffer.push_back('\\');
                    bReplaced = true;
                    ++j;
                } else {
                    switch (rawReplStrg[j + 1]) {
                        /*
                         * Support for backref transforms
                         *
                         * What do that do?
                         *
                         *   Let's say you have in you replacement string (for a regex
                         *   search-and-replace operaton) some backreferences (captured
                         *   or named groups you want to insert back); well, if you
                         *   want to alter the text of the backreference you're putting
                         *   back, you now have, with "backref transforms", a new
                         *   albeit limited set of options.
                         *
                         * An example?
                         *
                         *   Normally, the best you can do is move text around without
                         *   altering it; consider the following:
                         *
                         *     Text being searched:   JONATHAN|RICHARD-BROCHU
                         *     Search pattern:        ^([^\|]+)\|([^\|]+)$
                         *     Replace with:          \2, \1
                         *     Result:                RICHARD-BROCHU, JONATHAN
                         *
                         *   Now, if you wanted to alter the casing of the text in the
                         *   same operation as the search-and-replace one but could not,
                         *   now you can:
                         *
                         *     Text being searched:   JONATHAN|RICHARD-BROCHU
                         *     Search pattern:        ^([^\|]+)\|([^\|]+)$
                         *     Replace with:          \N\2, \1\E
                         *     Result:                Richard-Brochu, Jonathan
                         *
                         */
                        // TODO: Since we're modifying the Onigmo regex engine,
                        //       me might as well define new option / behaviour
                        //       flags for those new features
                        case 'L': // all-lowercase transform
                        case 'U': // all-uppercase transform
                        case 'l': // only-first-letter-lowercase transform
                        case 'u': // only-first-letter-uppercase transform
                        case 'F': // phrase-case transform
                        case 'I': // title-case transform
                        case 'N': // name-case transform
                            chCaseXform = rawReplStrg[j + 1];
                            bReplaced = true;
                            ++j;
                            break;
                        case 'E': // end of transform sequence
                            chCaseXform = '\0';
                            bReplaced = true;
                            ++j;
                            break;
                        default:
                            // add as literal?
                            // 2021-07-09: Fixed the fact that a literal backslash was being
                            //             pushed instead of the char itself (copy-paste mistake)
                            m_SubstBuffer.push_back(rawReplStrg[j + 1]);
                            // let the if (!bReplaced) {} block append rawReplStrg[j+1]
                            ++j;
                            // TODO: Confirm code above represents the desired behaviour;
                            //       i.e. should everything that gets through here be added
                            //            back as a literal, or (re)processed elsewhere?
                    } // switch()
                } // if ( == "\{#}" || == "\{##}")
#endif // X_ONIG_BACKREF_TRANSFORMS
            } else if (rawReplStrg[j] == '$') {
                size_t k = ((rawReplStrg[j + 1] == '+') && (rawReplStrg[j + 2] == '{')) ? (j + 3) : ((rawReplStrg[j + 1] == '{') ? (j + 2) : 0);
                if (k > 0) {
                    // named group replacemment
                    auto const name_beg = UCharCPtr(&(rawReplStrg[k]));
                    while (rawReplStrg[k] && IsCharAlphaNumericA(rawReplStrg[k])) {
                        ++k;
                    }
                    if (rawReplStrg[k] == '}') {
#ifdef X_ONIG_BACKREF_TRANSFORMS
                        // only evaluate the backreference index for now (we'll process it later on)
                        grpNum = onig_name_to_backref_number(m_RegExpr, name_beg, UCharCPtr(&(rawReplStrg[k])), &m_Region);
#else // !X_ONIG_BACKREF_TRANSFORMS
                        // evaluate capturing group number and perform the substitution
                        int const grpNum = onig_name_to_backref_number(m_RegExpr, name_beg, UCharCPtr(&(rawReplStrg[k])), &m_Region);
                        if ((grpNum >= 0) && (grpNum < m_Region.num_regs)) {
                            auto const rBeg = SciPos(m_Region.beg[grpNum]);
                            auto const len = SciPos(m_Region.end[grpNum] - rBeg);

                            m_SubstBuffer.append(doc->RangePointer(rBeg, len), static_cast<size_t>(len));
                        }
                        // mark as replaced
                        bReplaced = true;
#endif // X_ONIG_BACKREF_TRANSFORMS
                        j = k;
                    }
                }
#ifdef X_ONIG_BACKREF_TRANSFORMS
            }
            // check if we got a valid backreference
            if ((grpNum >= 0) && (grpNum < m_Region.num_regs)) {
                auto const rBeg = SciPos(m_Region.beg[grpNum]);
                auto const len = SciPos(m_Region.end[grpNum] - rBeg);
                // check for case transforms
                if (chCaseXform) {
                    // copy backref, apply transform and append
                    std::string sTmp(doc->RangePointer(rBeg, len), static_cast<size_t>(len));
                    m_SubstBuffer.append(applyTransform(sTmp, chCaseXform));
                } else { // no transform
                    m_SubstBuffer.append(doc->RangePointer(rBeg, len), static_cast<size_t>(len));
                }
                bReplaced = true;
            }
#else // !X_ONIG_BACKREF_TRANSFORMS
            } else if ((rawReplStrg[j] == '\\') && (rawReplStrg[j + 1] == '\\')) {
                m_SubstBuffer.push_back('\\');
                bReplaced = true;
                ++j;
            }
#endif // X_ONIG_BACKREF_TRANSFORMS
        } // if ((rawReplStrg[j] == '\\') || (rawReplStrg[j] == '$'))
        // append anything that wasn't escaped
        if (!bReplaced) {
#ifdef X_ONIG_BACKREF_TRANSFORMS
            // check for case transforms
            if (!chCaseXform) // no transform
                m_SubstBuffer.push_back(rawReplStrg[j]);
            else {
                // TODO: Consider if transforms should also be applied here
                //       (theoretically here they're literals, and transforms are
                //       meant to allow conversion of text which isn't already
                //       provided literally).
                m_SubstBuffer.push_back(rawReplStrg[j]);
            }
#else // !X_ONIG_BACKREF_TRANSFORMS
            m_SubstBuffer.push_back(rawReplStrg[j]);
#endif // X_ONIG_BACKREF_TRANSFORMS
        }
    }

    // NOTE: potential 64-bit-size issue at interface here:
    *length = SciPos(m_SubstBuffer.length());
    return m_SubstBuffer.c_str();
}
// ============================================================================

// ============================================================================
//
// private methods
//
// ============================================================================

/*
void OnigmoRegExEngine::regexFindAndReplace(std::string& inputStr_inout, const std::string& patternStr, const std::string& replStr)
{
    OnigRegex       oRegExpr;
    OnigRegion      oRegion;

    const UChar* pattern = (UChar*)patternStr.c_str();

    OnigErrorInfo einfo;
    int res = onig_new(&oRegExpr, pattern, pattern + strlen((char*)pattern),
                       ONIG_OPTION_DEFAULT, ONIG_ENCODING_ASCII, ONIG_SYNTAX_DEFAULT, &einfo);

    if (res != ONIG_NORMAL) { return; }
  
    const UChar* strg = (UChar*)inputStr_inout.c_str();
    const UChar* start = strg;
    const UChar* end = (start + patternStr.length());
    const UChar* range = end;

    onig_region_init(&oRegion);

    OnigPosition pos = onig_search(oRegExpr, strg, end, start, range, &oRegion, ONIG_OPTION_DEFAULT);

    if (pos >= 0) 
    {
        std::string replace = replStr; // copy
        for (int i = 1; i < oRegion.num_regs; i++) {
            std::ostringstream nr;
            nr << R"(\)" << i;
            std::string regio((char*)(strg + oRegion.beg[i]), (oRegion.end[i] - oRegion.beg[i]));
            replaceAll(replace, nr.str(), regio);
        }
        inputStr_inout.replace(oRegion.beg[0], (oRegion.end[0] - oRegion.beg[0]), replace);
    }

    onig_region_free(&oRegion, 0);
    onig_free(oRegExpr);
}
// ----------------------------------------------------------------------------
*/

std::string& OnigmoRegExEngine::translateRegExpr(std::string& regExprStr, bool wholeWord, bool wordStart, int eolMode, OnigOptionType& rxOptions)
{
    std::string tmpStr;
    bool bUseTmpStrg = false;

    if (wholeWord || wordStart) { // push '\b' at the begin of regexpr
        tmpStr.push_back('\\');
        tmpStr.push_back('b');
        tmpStr.append(regExprStr);
        if (wholeWord) { // push '\b' at the end of regexpr
            tmpStr.push_back('\\');
            tmpStr.push_back('b');
        }
        replaceAll(tmpStr, ".", R"(\w)");
        bUseTmpStrg = true;
    } else {
        tmpStr.append(regExprStr);
    }

    // Onigmo supports LTGT word boundary by: ONIG_SYN_OP_ESC_LTGT_WORD_BEGIN_END
    //
    //~replaceAll(tmpStr, R"(\<)", R"((?<!\w)(?=\w))");  // word begin
    //~replaceAll(tmpStr, R"(\(?<!\w)(?=\w))", R"(\\<)"); // esc'd
    //~replaceAll(tmpStr, R"(\>)", R"((?<=\w)(?!\w))"); // word end
    //~replaceAll(tmpStr, R"(\(?<=\w)(?!\w))", R"(\\>)"); // esc'd
    //~bUseTmpStrg = true;

    // EOL modes
    switch (eolMode) {
    case SC_EOL_LF:
    case SC_EOL_CR:
        ONIG_OPTION_OFF(rxOptions, ONIG_OPTION_NEWLINE_CRLF);
        break;

    case SC_EOL_CRLF:
        ONIG_OPTION_ON(rxOptions, ONIG_OPTION_NEWLINE_CRLF);
        break;
    }

    if (bUseTmpStrg) {
        std::swap(regExprStr, tmpStr);
    }

    return regExprStr;
}
// ----------------------------------------------------------------------------

std::string& OnigmoRegExEngine::convertReplExpr(std::string& replStr)
{
    std::string tmpStr;
    for (size_t i = 0; i < replStr.length(); ++i) {
        char ch = replStr[i];
        if (ch == '\\') {
            ch = replStr[++i]; // next char
            if (ch >= '1' && ch <= '9') {
                // former behavior convenience:
                // change "\\<n>" to deelx's group reference ($<n>)
                tmpStr.push_back('$');
                // the backref index will be appended when reaching the
                // default: clause of the next switch() block
            }
            switch (ch) {
#ifdef X_ONIG_BACKREF_TRANSFORMS
                // 2020-01-30: Added preliminary support for backreferences with
                //             transforms
                case 'L': // all-lowercase transform
                case 'U': // all-uppercase transform
                case 'l': // only-first-ltr-lowercase transform
                // NOTE: We abstain from processing \u[..] here (i.e. first) as
                //       we still want to support \uFFFF escape sequences;
                //       only if hex-char checks fail then we'll consider \u
                //       as introducing an only-first-ltr-uppercase transform
                ///case 'u': // only-first-ltr-uppercase transform
                case 'F': // phrase-case transform (e.g. Ulll lll-ll)
                case 'I': // title-case transform (e.g. Ulll Ull-ll)
                case 'N': // namee-case transform (e.g. Ulll Ull-Ul)
                {
                    // push modifier (we needed to make sure we had digits first)
                    tmpStr.push_back('\\'); tmpStr.push_back(ch);
                    // Check if next char if a digit, for catching sequences of the sort:
                    // - \U1 .. \U99  =>  \U\1\E .. \U\{99}\E
                    // - \L1 .. \L99  =>  \L\1\E .. \L\{99}\E
                    // - \F1 .. \F99  =>  \F\1\E .. \F\{99}\E
                    // - \I1 .. \I99  =>  \I\1\E .. \I\{99}\E
                    // - \N1 .. \N99  =>  \N\1\E .. \N\{99}\E
                    char ch2 = replStr[i + 1];
                    if (ch2 >= '1' && ch2 <= '9') {
                        i++;
                        // start pushing back backref index
                        tmpStr.push_back('\\'); tmpStr.push_back('{'); tmpStr.push_back(ch2);
                        // check if we have a second digit
                        ch2 = replStr[i + 1];
                        if (ch2 >= '0' && ch2 <= '9') {
                            i++;
                            // push second digit of backref index
                            tmpStr.push_back(ch2);
                        }
                        // finalize backref
                        tmpStr.push_back('}');
                        // close transform sequence
                        tmpStr.push_back('\\'); tmpStr.push_back('E');
                    } else {
                        // sequences \L, \U, \l, \F, \I & \N kept as-is
                    }
                    break;
                } // case 'L'/'U'/'l'/'F'/'I'/'N'

                case 'E': // end of transform sequence
                    tmpStr.push_back('\\'); tmpStr.push_back('E');
                    break;
#endif // X_ONIG_BACKREF_TRANSFORMS
                    // check for escape seq:
                case 'a':
                    tmpStr.push_back('\a');
                    break;
                case 'b':
                    tmpStr.push_back('\b');
                    break;
                case 'f':
                    tmpStr.push_back('\f');
                    break;
                case 'n':
                    tmpStr.push_back('\n');
                    break;
                case 'r':
                    tmpStr.push_back('\r');
                    break;
                case 't':
                    tmpStr.push_back('\t');
                    break;
                case 'v':
                    tmpStr.push_back('\v');
                    break;
                case '\\':
                    tmpStr.push_back('\\'); // preserve escd "\"
                    tmpStr.push_back('\\');
                    break;
                case 'x':
                case 'u': {
                    bool bShort = (ch == 'x');
                    char buf[8] = { '\0' };
                    char* pch = buf;
                    WCHAR val[2] = L"";
                    int hex;
                    val[0] = 0;
                    ++i;
                    hex = GetHexDigit(replStr[i]);
                    if (hex >= 0) {
                        ++i;
                        val[0] = static_cast<WCHAR>(hex);
                        hex = GetHexDigit(replStr[i]);
                        if (hex >= 0) {
                            ++i;
                            val[0] *= 16;
                            val[0] += static_cast<WCHAR>(hex);
                            if (!bShort) {
                                hex = GetHexDigit(replStr[i]);
                                if (hex >= 0) {
                                    ++i;
                                    val[0] *= 16;
                                    val[0] += static_cast<WCHAR>(hex);
                                    hex = GetHexDigit(replStr[i]);
                                    if (hex >= 0) {
                                        ++i;
                                        val[0] *= 16;
                                        val[0] += static_cast<WCHAR>(hex);
                                    }
                                }
                            }
                        }
                        if (val[0]) {
                            val[1] = 0;
                            WideCharToMultiByte(CP_UTF8, 0, val, -1, buf, ARRAYSIZE(val), nullptr, nullptr);
                            tmpStr.push_back(*pch++);
                            while (*pch)
                                tmpStr.push_back(*pch++);
                        } else
                            tmpStr.push_back(ch); // unknown ctrl seq
                    // not hex; could still be the \u one-letter uppercase transform
                    } else if (ch == 'u') {
                        // preserve as literal
                        tmpStr.push_back('\\'); tmpStr.push_back(ch);
                    } else
                        tmpStr.push_back(ch); // unknown ctrl seq
                } break;

            default:
                tmpStr.push_back(ch); // unknown ctrl seq *or* backref index (following the '$')
                break;
            }
        } else {
            tmpStr.push_back(ch);
        }
    } //for

    std::swap(replStr, tmpStr);
    return replStr;
}
// ============================================================================

#pragma warning(pop)

#endif //SCI_OWNREGEX
