/*
 * The hangul typed in the english mode on the 2-set keyboard, and the
 * english typed in the hangul mode on any keyboard.
 * "dkssud" -> "안녕", but not english words like as "the" -> "솓".
 * "ㅗ듸ㅐ" (2-set), "녀ㅉㅊ" (3-set) -> "hello".
 * Both check the keys typed with engwords.dic.
 */

#include <windows.h>
#include <string.h>
#include "hangul.h"

/*
 * Type the keys with the hangul automata of the keyboard. Returns the length
 * of pszHan, 0 if a key is not a jamo like as the symbols of the 3-set.
 */
static int TypeHangul(LPCWSTR pszEng, BOOL fAnyCase, LPWSTR pszHan, int cch)
{
    HangulIC hic;
    TCHAR lcs[128];
    WCHAR c, hkey;
    int i, n = 0, ncs;

    if (hangul_automata == NULL || lstrlenW(pszEng) > ENGTYPO_MAXLEN)
        return 0;

    hangul_ic_init(&hic);
    for (i = 0; (c = pszEng[i]) != L'\0'; i++)
    {
        if (c >= L'A' && c <= L'Z')
        {
            // the shift is only for the double consonants, ㅒ and ㅖ
            if (!fAnyCase && wcschr(L"QWERTOP", c) == NULL)
                return 0;
        }
        else if (c < L'a' || c > L'z')
        {
            return 0;
        }

        hkey = keyToHangulKey(c);
        if (!hangul_is_choseong(hkey) && !hangul_is_jungseong(hkey) && !hangul_is_jongseong(hkey))
            return 0;

        ncs = 0;
        hangul_automata(&hic, hkey, lcs, &ncs);
        if (ncs)
        {
            if (n >= cch - 1)
                return 0;
            pszHan[n++] = lcs[ncs - 1]; // committed syllable
        }
    }

    ncs = hangul_ic_get(&hic, 1, lcs);
    if (ncs > 0)
    {
        if (n >= cch - 1)
            return 0;
        pszHan[n++] = lcs[ncs - 1];
    }
    pszHan[n] = L'\0';
    return n;
}

static BOOL IsSyllables(LPCWSTR psz)
{
    for (; *psz != L'\0'; psz++)
    {
        if (!hangul_is_syllable(*psz))
            return FALSE;
    }
    return TRUE;
}

/*
 * Type the keys with the hangul automata and get the syllables.
 * Returns the length of pszHan, 0 and empty if they are not all syllables.
 */
static int hangul_eng_to_syllables(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    int n = TypeHangul(pszEng, FALSE, pszHan, cch);

    if (n == 0 || !IsSyllables(pszHan))
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}

static void ToLowerWord(LPCWSTR pszEng, char *pszWord)
{
    int i;

    for (i = 0; pszEng[i] != L'\0'; i++)
        pszWord[i] = (char)(pszEng[i] | 0x20);
    pszWord[i] = '\0';
}

/*
 * The english words of engwords.dic, the sorted lower case words, one per
 * line, after the comment lines starting with '#'. The file is mapped, so
 * the pages are shared by the processes and only the pages searched are read.
 */
static const char *s_pWordFile;
static const char *s_pWords;
static const char *s_pWordsEnd;

/*
 * The hangul syllables of the english typo. Returns the length of pszHan,
 * 0 if it is not a typo of the hangul, it is an english word, or
 * engwords.dic is not loaded.
 * Two letters like as "gh" (호) are often abbreviations, so 3 letters at least.
 */
int hangul_eng_typo(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    char szWord[ENGTYPO_MAXLEN + 1];
    int n;

    pszHan[0] = L'\0';
    if (s_pWords == NULL || !hangul_is_old2bul() || lstrlenW(pszEng) < 3)
        return 0;
    if ((n = hangul_eng_to_syllables(pszEng, pszHan, cch)) == 0)
        return 0;

    ToLowerWord(pszEng, szWord);
    if (hangul_is_eng_word(szWord))
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}

BOOL hangul_load_eng_words(LPCTSTR pszFile)
{
    HANDLE hFile, hMap;
    DWORD cb;
    const char *p;

    if (s_pWords != NULL)
        return TRUE;

    hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_EXISTING,
                       FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    cb = GetFileSize(hFile, NULL);
    hMap = (cb != INVALID_FILE_SIZE && cb > 0) ? CreateFileMapping(hFile, NULL, PAGE_READONLY, 0, 0, NULL) : NULL;
    CloseHandle(hFile);
    if (hMap == NULL)
        return FALSE;
    // the view keeps the mapping
    p = (const char *)MapViewOfFile(hMap, FILE_MAP_READ, 0, 0, 0);
    CloseHandle(hMap);
    if (p == NULL)
        return FALSE;

    s_pWordFile = p;
    s_pWordsEnd = p + cb;
    // skip the comments
    while (p < s_pWordsEnd && *p == '#')
    {
        while (p < s_pWordsEnd && *p++ != '\n')
            ;
    }
    // the other threads read s_pWords without the lock
    MemoryBarrier();
    s_pWords = p;
    return TRUE;
}

void hangul_free_eng_words(void)
{
    if (s_pWordFile != NULL)
        UnmapViewOfFile(s_pWordFile);
    s_pWordFile = NULL;
    s_pWords = s_pWordsEnd = NULL;
}

BOOL hangul_is_eng_word(const char *pszWord)
{
    const char *lo = s_pWords, *hi;
    const char *start, *end, *next;
    size_t len = strlen(pszWord), n;
    int cmp;

    if (lo == NULL)
        return FALSE;
    MemoryBarrier(); // s_pWordsEnd is set before s_pWords
    hi = s_pWordsEnd;

    // binary search of the lines in [lo, hi)
    while (lo < hi)
    {
        start = lo + (hi - lo) / 2;
        while (start > lo && start[-1] != '\n')
            start--;
        end = (const char *)memchr(start, '\n', s_pWordsEnd - start);
        next = (end != NULL) ? end + 1 : s_pWordsEnd;
        if (end == NULL)
            end = s_pWordsEnd;
        if (end > start && end[-1] == '\r')
            end--;

        n = (size_t)(end - start);
        cmp = memcmp(pszWord, start, (len < n) ? len : n);
        if (cmp == 0)
            cmp = (len < n) ? -1 : (len > n);
        if (cmp == 0)
            return TRUE;
        if (cmp < 0)
            hi = start;
        else
            lo = next;
    }
    return FALSE;
}

/*
 * The english word typed in the hangul mode of any keyboard. pszHan is the
 * hangul typed by the keys. Returns the length of pszHan, 0 if it is not an
 * english word.
 * The hangul has a jamo not composed: "해" (go) is not an english typo.
 * 3 letters at least like as the english mode, not the jamos for chatting
 * like as "ㅜㅠ" (nb), "ㅁㅊ" (ac), nor the same jamos like as "ㅋㅋㅋ" (zzz),
 * nor the faces like as "ㅇㅁㅇ" (dad), "ㅎㅁㅎ" (gag).
 */
int hangul_han_typo(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    char szWord[ENGTYPO_MAXLEN + 1];
    int i, n;

    pszHan[0] = L'\0';
    if (lstrlenW(pszEng) < 3)
        return 0;

    n = TypeHangul(pszEng, TRUE, pszHan, cch);
    if (n == 0 || IsSyllables(pszHan))
    {
        pszHan[0] = L'\0';
        return 0;
    }
    if (n == 3 && pszHan[0] == pszHan[2] && !hangul_is_syllable(pszHan[0]) && !hangul_is_syllable(pszHan[1]))
    {
        pszHan[0] = L'\0';
        return 0; // a face of the jamos
    }

    ToLowerWord(pszEng, szWord);
    for (i = 1; szWord[i] == szWord[0]; i++)
        ;
    if (szWord[i] == '\0')
    {
        pszHan[0] = L'\0';
        return 0; // the same letters
    }
    if (!hangul_is_eng_word(szWord))
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}

/*
 * The state of the word typed for the TIP. The TIP calls hangul_typo_key()
 * in OnTestKeyDown() (fTest) and OnKeyDown(). OnKeyDown() is called only
 * for the keys eaten by OnTestKeyDown(), and the letters are eaten in the
 * hangul mode, so they are kept in OnKeyDown() in the hangul mode.
 */
void hangul_typo_init(HangulTypo *t)
{
    ZeroMemory(t, sizeof(*t));
}

static void hangul_typo_reset_word(HangulTypo *t, BOOL fOff)
{
    t->len = 0;
    t->word[0] = L'\0';
    t->off = fOff;
    t->typo[0] = L'\0';
}

/*
 * The conversion of the word, kept in typo with the word for the TIP. typo
 * is the hangul typed in the english mode, or the hangul typed by the keys
 * in the hangul mode. engwords.dic should be loaded for both of the modes.
 */
static BOOL hangul_typo_check(HangulTypo *t, BOOL fHangul)
{
    if (t->off || t->len == 0)
        return FALSE;
    return (fHangul ? hangul_han_typo(t->word, t->typo, ARRAYSIZE(t->typo)) :
                      hangul_eng_typo(t->word, t->typo, ARRAYSIZE(t->typo))) > 0;
}

static BOOL IsAsciiLetter(WCHAR c)
{
    return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z');
}

static BOOL IsWordChar(WCHAR c)
{
    return IsAsciiLetter(c) || (c >= L'0' && c <= L'9') || c == L'_';
}

static BOOL IsHangul(WCHAR c)
{
    return hangul_is_syllable(c) || hangul_is_cjamo(c) ||
           hangul_is_choseong(c) || hangul_is_jungseong(c) || hangul_is_jongseong(c);
}

/*
 * A key of the TIP. Returns HANGUL_TYPO_CONVERT for <SPACE> after a typo,
 * HANGUL_TYPO_UNDO for <BACKSPACE> just after the conversion. The TIP eats
 * the key only for the text before the caret, and calls hangul_typo_end()
 * after HANGUL_TYPO_CONVERT, converted or not. The word of <SPACE> eaten
 * without OnKeyDown() goes on.
 */
int hangul_typo_key(HangulTypo *t, UINT vk, WCHAR ch, BOOL fHangul, BOOL fComposing,
                    BOOL fCtrlAlt, BOOL fShift, BOOL fTest)
{
    // <BACKSPACE> just after the conversion restores the word and the mode
    if (vk == VK_BACK && t->undo && fHangul != t->toEng && !fComposing && !fCtrlAlt && !fShift)
    {
        if (!fTest)
            t->undo = FALSE;
        return HANGUL_TYPO_UNDO;
    }
    // only just after the conversion. the other <BACKSPACE> is of the word.
    t->undo = FALSE;

    // a new word in the other mode. the mode is changed in many ways
    // like as the language bar, <ESC> and the hangul key.
    if (fHangul != t->hangul)
    {
        t->hangul = fHangul;
        hangul_typo_reset_word(t, FALSE);
    }

    if (fCtrlAlt)
    {
        // shortcuts like as <CTRL>-<V>
        hangul_typo_reset_word(t, TRUE);
        return HANGUL_TYPO_NONE;
    }

    switch (vk)
    {
        case VK_SPACE:
            // checked again in OnKeyDown(). it may be called without OnTestKeyDown().
            if (!hangul_typo_check(t, fHangul))
            {
                hangul_typo_reset_word(t, FALSE); // a new word
                return HANGUL_TYPO_NONE;
            }
            return HANGUL_TYPO_CONVERT;

        case VK_RETURN:
        case VK_TAB:
            hangul_typo_reset_word(t, FALSE); // a new word
            break;

        case VK_BACK:
            if (fHangul)
            {
                hangul_typo_reset_word(t, TRUE); // a jamo is deleted, not a key
            }
            else
            {
                // a typo fixed in the word
                if (t->len > 0)
                    t->word[--t->len] = L'\0';
                else
                    t->off = TRUE; // back in the previous word
            }
            break;

        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_DELETE:
        case VK_INSERT:
        case VK_ESCAPE:
            // the user moved the caret on purpose. not this word.
            hangul_typo_reset_word(t, TRUE);
            break;

        default:
            if (IsAsciiLetter(ch))
            {
                // OnKeyDown() for the letters eaten in the hangul mode.
                // some applications call OnKeyDown() without OnTestKeyDown().
                if (fHangul && fTest)
                    break;
                if (t->len < ENGTYPO_MAXLEN)
                {
                    t->word[t->len++] = ch;
                    t->word[t->len] = L'\0';
                }
                else
                {
                    t->off = TRUE; // too long
                }
            }
            else if (ch != 0 && t->len > 0)
            {
                t->off = TRUE; // digits, punctuations in the word
            }
            break;
    }
    return HANGUL_TYPO_NONE;
}

/*
 * The end of the word after HANGUL_TYPO_CONVERT. The conversion is kept in
 * eng and han for the undo only if converted.
 */
void hangul_typo_end(HangulTypo *t, BOOL fConverted)
{
    if (fConverted)
    {
        lstrcpynW(t->eng, t->word, ARRAYSIZE(t->eng));
        lstrcpynW(t->han, t->typo, ARRAYSIZE(t->han));
        t->toEng = t->hangul;
        t->undo = TRUE;
    }
    hangul_typo_reset_word(t, FALSE);
}

/*
 * Is the text before the caret the same as the word typed?
 * For the conversion, pszBefore has one more char for the start of the
 * word, and the word is not a part of an identifier like as "abc_dkssud".
 * The hangul of the english word is not a part of a hangul word like as
 * "ㅗ녀ㅈㅍ" typed by "/help" on the 3-set, whose '/' is ㅗ and not kept as
 * a letter. The english after the hangul like as "녕gktpdy" is converted.
 * For the undo, pszBefore is the word of the conversion and the space.
 */
BOOL hangul_typo_match(LPCWSTR pszBefore, LPCWSTR pszTyped, BOOL fUndo)
{
    int n = lstrlenW(pszBefore), len = lstrlenW(pszTyped);
    WCHAR c;

    if (len == 0)
        return FALSE;
    if (fUndo)
        return n == len + 1 && pszBefore[len] == L' ' && wcsncmp(pszBefore, pszTyped, len) == 0;
    if (n < len || wcscmp(pszBefore + n - len, pszTyped) != 0)
        return FALSE;
    if (n == len)
        return TRUE;
    c = pszBefore[n - len - 1];
    return !IsWordChar(c) && !(IsHangul(c) && IsHangul(pszTyped[0]));
}
