/*
 * Unit tests for the hangul automata and the dictionary lookup.
 * Run in the resource directory to read the dictionaries.
 */

#include <windows.h>
#include <stdio.h>
#include "hangul.h"

int GetCandidateStringsFromDictionary(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
int GetHangulFromHanjaIndex(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);

static int nPassed, nFailed;

// code points, the console may not show hangul
static void PrintCodes(LPCWSTR psz)
{
    for (; *psz; psz++)
        printf(" U+%04X", (unsigned)*psz);
}

static void CheckStr(LPCSTR name, LPCWSTR expected, LPCWSTR actual)
{
    if (lstrcmpW(expected, actual) == 0)
    {
        nPassed++;
        return;
    }
    nFailed++;
    printf("FAIL %s\n  expected:", name);
    PrintCodes(expected);
    printf("\n  actual:  ");
    PrintCodes(actual);
    printf("\n");
}

static void CheckInt(LPCSTR name, int expected, int actual)
{
    if (expected == actual)
    {
        nPassed++;
        return;
    }
    nFailed++;
    printf("FAIL %s: expected %d, actual %d\n", name, expected, actual);
}

//
// Type ASCII keys like as _HandleKeyDown() of the TIP and return the text.
//
static void TypeKeys(LPCSTR keys, LPWSTR out, int cch)
{
    TCHAR lcs[128];
    WCHAR hkey;
    int ncs, n = 0;

    hangul_ic_init(&ic);
    for (; *keys; keys++)
    {
        hkey = keyToHangulKey((WCHAR)*keys);
        if (hkey >= 0x1100 && hkey <= 0x11ff)
        {
            ncs = 0;
            hangul_automata(&ic, hkey, lcs, &ncs);
            if (ncs && n < cch - 1)
                out[n++] = lcs[ncs - 1]; // committed syllable
            continue;
        }

        // not a hangul key. commit the preedit and the key itself.
        ncs = hangul_ic_get(&ic, 1, lcs);
        if (ncs > 0 && n < cch - 1)
            out[n++] = lcs[ncs - 1];
        hangul_ic_init(&ic);
        if (n < cch - 1)
            out[n++] = (WCHAR)*keys;
    }

    ncs = hangul_ic_get(&ic, 1, lcs);
    if (ncs > 0 && n < cch - 1)
        out[n++] = lcs[ncs - 1];
    hangul_ic_init(&ic);
    out[n] = L'\0';
}

static void TestHangul2(void)
{
    WCHAR out[64];

    TypeKeys("rkqt", out, ARRAYSIZE(out));
    CheckStr("2set: rkqt", L"값", out);

    TypeKeys("dkssudgktpdy", out, ARRAYSIZE(out));
    CheckStr("2set: dkssudgktpdy", L"안녕하세요", out);

    TypeKeys("gksrmf", out, ARRAYSIZE(out));
    CheckStr("2set: gksrmf", L"한글", out);

    TypeKeys("rk rk", out, ARRAYSIZE(out));
    CheckStr("2set: rk rk", L"가 가", out);
}

static void TestDictionary(void)
{
    WCHAR buf[1024 + 2];
    LPCWSTR psz;
    int n;

    n = GetCandidateStringsFromDictionary(L"가", buf, 1024, L"nabi.dic");
    CheckInt("nabi.dic: found", 1, n > 1);
    psz = buf;
    CheckStr("nabi.dic: 1st", L"可:옳을 가", psz);
    psz += lstrlenW(psz) + 1;
    CheckStr("nabi.dic: 2nd", L"家:집 가", psz);

    n = GetCandidateStringsFromDictionary(L"대한민국", buf, 1024, L"word.dic");
    CheckInt("word.dic: found", 1, n > 1);
    CheckStr("word.dic: 1st", L"大韓民國", buf);

    n = GetCandidateStringsFromDictionary(L"없는말없는말", buf, 1024, L"word.dic");
    CheckInt("word.dic: not found", 0, n);
}

static void TestHanjaIndex(void)
{
    WCHAR han[2];

    han[0] = han[1] = L'\0';
    CheckInt("jinsuk.dic: 韓", 2, GetHangulFromHanjaIndex(L"韓", han, 2, L"jinsuk.dic"));
    CheckStr("jinsuk.dic: 韓 -> 한", L"한", han);

    han[0] = han[1] = L'\0';
    GetHangulFromHanjaIndex(L"大", han, 2, L"jinsuk.dic");
    CheckStr("jinsuk.dic: 大 -> 대", L"대", han);
}

int main(void)
{
    TestHangul2();
    TestDictionary();
    TestHanjaIndex();

    printf("%d passed, %d failed\n", nPassed, nFailed);
    return nFailed ? 1 : 0;
}
