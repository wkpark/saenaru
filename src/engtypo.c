/*
 * The hangul typed in the english mode on the 2-set keyboard.
 * "dkssud" -> "안녕", but not english words like as "the" -> "솓".
 */

#include <windows.h>
#include <stdlib.h>
#include <string.h>
#include "hangul.h"
#include "engwords.h"

static int TypeSyllables(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    HangulIC hic;
    TCHAR lcs[128];
    WCHAR c, hkey;
    int i, n = 0, ncs;

    pszHan[0] = L'\0';
    if (hangul_automata == NULL || lstrlenW(pszEng) > ENGTYPO_MAXLEN)
        return 0;

    hangul_ic_init(&hic);
    for (i = 0; (c = pszEng[i]) != L'\0'; i++)
    {
        if (c >= L'A' && c <= L'Z')
        {
            // the shift is only for the double consonants, ㅒ and ㅖ
            if (wcschr(L"QWERTOP", c) == NULL)
                return 0;
        }
        else if (c < L'a' || c > L'z')
        {
            return 0;
        }

        hkey = keyToHangulKey(c);
        if (hkey < 0x1100 || hkey > 0x11ff)
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

    for (i = 0; i < n; i++)
    {
        if (pszHan[i] < 0xac00 || pszHan[i] > 0xd7a3)
            return 0;
    }
    return n;
}

/*
 * Type the keys with the hangul automata and get the syllables.
 * Returns the length of pszHan, 0 and empty if they are not all syllables.
 */
int hangul_eng_to_syllables(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    int n = TypeSyllables(pszEng, pszHan, cch);

    if (n == 0)
        pszHan[0] = L'\0';
    return n;
}

static int CompareWord(const void *a, const void *b)
{
    return strcmp(*(const char **)a, *(const char **)b);
}

/*
 * The hangul syllables of the english typo. Returns the length of pszHan,
 * 0 if it is not a typo of the hangul or it is an english word.
 */
int hangul_eng_typo(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    char szWord[ENGTYPO_MAXLEN + 1];
    const char *pszWord = szWord;
    int i, n;

    if (!hangul_is_old2bul())
        return 0;
    if ((n = hangul_eng_to_syllables(pszEng, pszHan, cch)) == 0)
        return 0;

    // the lower case letters
    for (i = 0; pszEng[i] != L'\0'; i++)
        szWord[i] = (char)(pszEng[i] | 0x20);
    szWord[i] = '\0';

    if (bsearch(&pszWord, eng_words, ARRAYSIZE(eng_words), sizeof(eng_words[0]), CompareWord) != NULL)
    {
        pszHan[0] = L'\0';
        return 0;
    }
    return n;
}
