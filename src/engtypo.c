/*
 * The hangul typed in the english mode, and the english typed in the hangul
 * mode on the 2-set keyboard.
 * "dkssud" -> "안녕", but not english words like as "the" -> "솓".
 * "ㅗ디ㅣㅐ" -> "hello" with the english words of engwords.dic.
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "hangul.h"
#include "engwords.h"

/*
 * Type the keys with the hangul automata. *pfCho and *pfJung are set if the
 * keys have consonants and vowels. Returns the length of pszHan.
 */
static int TypeHangul(LPCWSTR pszEng, BOOL fAnyCase, LPWSTR pszHan, int cch, BOOL *pfCho, BOOL *pfJung)
{
    HangulIC hic;
    TCHAR lcs[128];
    WCHAR c, hkey;
    int i, n = 0, ncs;

    *pfCho = *pfJung = FALSE;
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
        if (hangul_is_choseong(hkey))
            *pfCho = TRUE;
        else if (hangul_is_jungseong(hkey))
            *pfJung = TRUE;
        else
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
        if (*psz < 0xac00 || *psz > 0xd7a3)
            return FALSE;
    }
    return TRUE;
}

/*
 * Type the keys with the hangul automata and get the syllables.
 * Returns the length of pszHan, 0 and empty if they are not all syllables.
 */
int hangul_eng_to_syllables(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    BOOL fCho, fJung;
    int n = TypeHangul(pszEng, FALSE, pszHan, cch, &fCho, &fJung);

    if (n == 0 || !IsSyllables(pszHan))
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}

static int CompareWord(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

static void ToLowerWord(LPCWSTR pszEng, char *pszWord)
{
    int i;

    for (i = 0; pszEng[i] != L'\0'; i++)
        pszWord[i] = (char)(pszEng[i] | 0x20);
    pszWord[i] = '\0';
}

/*
 * The hangul syllables of the english typo. Returns the length of pszHan,
 * 0 if it is not a typo of the hangul or it is an english word.
 */
int hangul_eng_typo(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    char szWord[ENGTYPO_MAXLEN + 1];
    const char *pszWord = szWord;
    int n;

    if (!hangul_is_old2bul())
        return 0;
    if ((n = hangul_eng_to_syllables(pszEng, pszHan, cch)) == 0)
        return 0;

    ToLowerWord(pszEng, szWord);
    if (bsearch(&pszWord, eng_words, ARRAYSIZE(eng_words), sizeof(eng_words[0]), CompareWord) != NULL)
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}

/*
 * The english words for the english typed in the hangul mode.
 * engwords.dic has the sorted lower case words, one per line,
 * after the comment lines starting with '#'.
 */
static char *s_pWordFile;
static const char *s_pWords;
static const char *s_pWordsEnd;

BOOL hangul_load_eng_words(LPCTSTR pszFile)
{
    HANDLE hFile;
    DWORD cb, cbRead;
    char *p;

    if (s_pWords != NULL)
        return TRUE;

    hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
        return FALSE;

    cb = GetFileSize(hFile, NULL);
    p = (cb != INVALID_FILE_SIZE && cb > 0) ? (char *)HeapAlloc(GetProcessHeap(), 0, cb) : NULL;
    if (p == NULL || !ReadFile(hFile, p, cb, &cbRead, NULL) || cbRead != cb)
    {
        if (p != NULL)
            HeapFree(GetProcessHeap(), 0, p);
        CloseHandle(hFile);
        return FALSE;
    }
    CloseHandle(hFile);

    s_pWordFile = p;
    s_pWordsEnd = p + cb;
    // skip the comments
    while (p < s_pWordsEnd && *p == '#')
    {
        while (p < s_pWordsEnd && *p++ != '\n')
            ;
    }
    s_pWords = p;
    return TRUE;
}

void hangul_free_eng_words(void)
{
    if (s_pWordFile != NULL)
        HeapFree(GetProcessHeap(), 0, s_pWordFile);
    s_pWordFile = NULL;
    s_pWords = s_pWordsEnd = NULL;
}

static BOOL IsEngWord(const char *pszWord)
{
    const char *lo = s_pWords, *hi = s_pWordsEnd;
    const char *start, *end, *next;
    size_t len = strlen(pszWord), n;
    int cmp;

    if (lo == NULL)
        return FALSE;

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
 * The english word typed in the hangul mode. pszHan is the hangul typed by
 * the keys. Returns the length of pszHan, 0 if it is not an english word.
 * The hangul has a jamo not composed: "해" (go) is not an english typo.
 * Not the jamos for chatting: the same jamos like as "ㅠㅠ" (bb), "ㅇㅇ" (dd),
 * and two consonants like as "ㅁㅊ" (ac).
 */
int hangul_han_typo(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    char szWord[ENGTYPO_MAXLEN + 1];
    BOOL fCho, fJung;
    int i, n;

    pszHan[0] = L'\0';
    if (!hangul_is_old2bul())
        return 0;

    n = TypeHangul(pszEng, TRUE, pszHan, cch, &fCho, &fJung);
    if (n == 0 || IsSyllables(pszHan) || (!fJung && lstrlenW(pszEng) < 3))
    {
        pszHan[0] = L'\0';
        return 0;
    }

    ToLowerWord(pszEng, szWord);
    for (i = 1; szWord[i] == szWord[0]; i++)
        ;
    if (szWord[i] == '\0')
    {
        pszHan[0] = L'\0';
        return 0; // the same letters
    }
    if (!IsEngWord(szWord))
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}
