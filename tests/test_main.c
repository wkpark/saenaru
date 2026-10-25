/*
 * Unit tests for the hangul automata and the dictionary lookup.
 * Run in the resource directory to read the dictionaries.
 */

#include <windows.h>
#include <stdio.h>
#include "hangul.h"

int GetCandidateStringsFromDictionary(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
int GetHangulFromHanjaIndex(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
extern DWORD dwOptionFlag;
extern DWORD dwImeFlag;

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

// compare the NUL separated candidate list
static void CheckList(LPCSTR name, LPCWSTR expected, LPCWSTR actual)
{
    while (*expected && lstrcmpW(expected, actual) == 0)
    {
        expected += lstrlenW(expected) + 1;
        actual += lstrlenW(actual) + 1;
    }
    if (!*expected && !*actual)
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

static BOOL HasCandidate(LPCWSTR list, LPCWSTR cand)
{
    for (; *list; list += lstrlenW(list) + 1)
        if (lstrcmpW(list, cand) == 0)
            return TRUE;
    return FALSE;
}

//
// Type ASCII keys like as _HandleKeyDown() of the TIP and return the text.
// '\b' deletes the last jamo like as _HandleBackspace(), and a single jamo
// goes to the application like as OnTestKeyDown().
//
static void TypeKeys(LPCSTR keys, LPWSTR out, int cch)
{
    TCHAR lcs[128];
    WCHAR hkey;
    int ncs, n = 0;

    hangul_ic_init(&ic);
    for (; *keys; keys++)
    {
        if (*keys == '\b')
        {
            if (!ic.len)
            {
                // not composing. the application deletes the previous char.
                if (n > 0)
                    n--;
                continue;
            }
            if (ic.len > 1 || hangul_ic_restorable(&ic))
                hangul_ic_backspace(&ic);
            else
                hangul_ic_init(&ic);
            continue;
        }

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

// compose the keys without commit
static void ComposeKeys(LPCSTR keys)
{
    TCHAR lcs[128];
    int ncs;

    hangul_ic_init(&ic);
    for (; *keys; keys++)
        hangul_automata(&ic, keyToHangulKey((WCHAR)*keys), lcs, &ncs);
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

    TypeKeys("Rk", out, ARRAYSIZE(out));
    CheckStr("2set: Rk", L"까", out);

    TypeKeys("rhk", out, ARRAYSIZE(out));
    CheckStr("2set: rhk", L"과", out);

    TypeKeys("ekfr", out, ARRAYSIZE(out));
    CheckStr("2set: ekfr", L"닭", out);

    // the jongseong moves to the next syllable
    TypeKeys("rkrk", out, ARRAYSIZE(out));
    CheckStr("2set: rkrk", L"가가", out);

    TypeKeys("ekfrk", out, ARRAYSIZE(out));
    CheckStr("2set: ekfrk", L"달가", out);
}

static void TestBackspace(void)
{
    WCHAR out[64];

    TypeKeys("rkq\b", out, ARRAYSIZE(out));
    CheckStr("backspace: rkq\\b", L"가", out);

    TypeKeys("rkq\b\b", out, ARRAYSIZE(out));
    CheckStr("backspace: rkq\\b\\b", L"ㄱ", out);

    TypeKeys("rkq\b\b\b", out, ARRAYSIZE(out));
    CheckStr("backspace: rkq\\b\\b\\b", L"", out);

    // BACKSPACE_BY_JAMO deletes a compound jamo at once
    TypeKeys("rkqt\b", out, ARRAYSIZE(out));
    CheckStr("backspace: rkqt\\b", L"가", out);

    TypeKeys("rhk\b", out, ARRAYSIZE(out));
    CheckStr("backspace: rhk\\b", L"ㄱ", out);

    // type again after the backspace
    TypeKeys("rkq\bs", out, ARRAYSIZE(out));
    CheckStr("backspace: rkq\\bs", L"간", out);

    // not composing
    TypeKeys("rk \b\b", out, ARRAYSIZE(out));
    CheckStr("backspace: rk \\b\\b", L"", out);
}

// without BACKSPACE_BY_JAMO, restore the jamo before composed
static void TestBackspaceCompat(void)
{
    WCHAR out[64];

    dwOptionFlag &= ~BACKSPACE_BY_JAMO;

    TypeKeys("rkqt\b", out, ARRAYSIZE(out));
    CheckStr("compat: rkqt\\b", L"갑", out);

    TypeKeys("rkqt\b\b", out, ARRAYSIZE(out));
    CheckStr("compat: rkqt\\b\\b", L"가", out);

    TypeKeys("rhk\b", out, ARRAYSIZE(out));
    CheckStr("compat: rhk\\b", L"고", out);

    TypeKeys("rhkq\b\b", out, ARRAYSIZE(out));
    CheckStr("compat: rhkq\\b\\b", L"고", out);

    TypeKeys("rhk\bl", out, ARRAYSIZE(out));
    CheckStr("compat: rhk\\bl", L"괴", out);

    TypeKeys("ekfr\bk", out, ARRAYSIZE(out));
    CheckStr("compat: ekfr\\bk", L"다라", out);

    // 2-set choseong + choseong = jongseong
    TypeKeys("rt\bk", out, ARRAYSIZE(out));
    CheckStr("compat: rt\\bk", L"가", out);

    // one key jamo is deleted at once
    TypeKeys("Rk\b\b", out, ARRAYSIZE(out));
    CheckStr("compat: Rk\\b\\b", L"", out);

    TypeKeys("goT\b", out, ARRAYSIZE(out));
    CheckStr("compat: goT\\b", L"해", out);

    // the order fixed by COMPOSITE_TYPING keeps the compound jamo
    dwOptionFlag |= COMPOSITE_TYPING;
    TypeKeys("hkr\b", out, ARRAYSIZE(out));
    CheckStr("compat composite: hkr\\b", L"고", out);
    dwOptionFlag &= ~COMPOSITE_TYPING;

    // the restored modern jamo makes a syllable again
    set_compose(4);
    ComposeKeys("rhj");
    CheckInt("compat old hangul: rhj", 0, ic.syllable);
    hangul_ic_backspace(&ic);
    CheckInt("compat old hangul: rhj\\b", 1, ic.syllable);
    set_keyboard(LAYOUT_OLD2BUL);
    dwImeFlag |= SAENARU_ONTHESPOT;

    set_keyboard(LAYOUT_3FIN);
    TypeKeys("kkf\b\b", out, ARRAYSIZE(out));
    CheckStr("compat 3set: kkf\\b\\b", L"ㄱ", out);

    TypeKeys("kfwx\b", out, ARRAYSIZE(out));
    CheckStr("compat 3set: kfwx\\b", L"갈", out);

    TypeKeys("kf@\b", out, ARRAYSIZE(out));
    CheckStr("compat 3set: kf@\\b", L"가", out);
    set_keyboard(LAYOUT_OLD2BUL);

    dwOptionFlag |= BACKSPACE_BY_JAMO;
}

// shift + backspace on the committed syllable like as DicKeydownHandler()
static void BackspaceSyllable(WCHAR syllable, LPWSTR out)
{
    TCHAR lcs[128];

    hangul_ic_set_syllable(&ic, syllable);
    hangul_ic_backspace(&ic);
    if (hangul_ic_get(&ic, 1, lcs) > 0)
        lstrcpyW(out, lcs);
    else
        out[0] = L'\0';
}

static void TestBackspaceSyllable(void)
{
    WCHAR out[64];

    BackspaceSyllable(L'값', out);
    CheckStr("syllable: 값", L"가", out);

    BackspaceSyllable(L'과', out);
    CheckStr("syllable: 과", L"ㄱ", out);

    dwOptionFlag &= ~BACKSPACE_BY_JAMO;

    BackspaceSyllable(L'값', out);
    CheckStr("syllable compat: 값", L"갑", out);

    BackspaceSyllable(L'과', out);
    CheckStr("syllable compat: 과", L"고", out);

    BackspaceSyllable(L'닭', out);
    CheckStr("syllable compat: 닭", L"달", out);

    // one key jamo of 2-set
    BackspaceSyllable(L'했', out);
    CheckStr("syllable compat: 했", L"해", out);

    BackspaceSyllable(L'밖', out);
    CheckStr("syllable compat: 밖", L"바", out);

    BackspaceSyllable(L'걔', out);
    CheckStr("syllable compat: 걔", L"ㄱ", out);

    // one key jamo of 3-set
    set_keyboard(LAYOUT_3FIN);
    BackspaceSyllable(L'값', out);
    CheckStr("syllable compat 3set: 값", L"가", out);

    BackspaceSyllable(L'과', out);
    CheckStr("syllable compat 3set: 과", L"고", out);
    set_keyboard(LAYOUT_OLD2BUL);

    dwOptionFlag |= BACKSPACE_BY_JAMO;
}

static void TestHangul3(void)
{
    static const UINT layouts[] = { LAYOUT_3FIN, LAYOUT_390 };
    WCHAR out[64];
    UINT i;

    for (i = 0; i < ARRAYSIZE(layouts); i++)
    {
        set_keyboard(layouts[i]);

        TypeKeys("mfskgw", out, ARRAYSIZE(out));
        CheckStr("3set: mfskgw", L"한글", out);

        TypeKeys("jfshea", out, ARRAYSIZE(out));
        CheckStr("3set: jfshea", L"안녕", out);

        // a choseong key always starts a new syllable
        TypeKeys("kfkf", out, ARRAYSIZE(out));
        CheckStr("3set: kfkf", L"가가", out);
    }
    set_keyboard(LAYOUT_OLD2BUL);
}

static void TestJamo(void)
{
    WCHAR cho, jung, jong;

    CheckInt("jamo to syllable", L'한', hangul_jamo_to_syllable(0x1112, 0x1161, 0x11ab));

    hangul_syllable_to_jamo(L'닭', &cho, &jung, &jong);
    CheckInt("syllable to cho", 0x1103, cho);
    CheckInt("syllable to jung", 0x1161, jung);
    CheckInt("syllable to jong", 0x11b0, jong);

    CheckInt("choseong to cjamo", L'ㄱ', hangul_jamo_to_cjamo(0x1100));
    CheckInt("jongseong to cjamo", L'ㄺ', hangul_jamo_to_cjamo(0x11b0));
}

static void CheckTypo(LPCWSTR eng, LPCWSTR expected)
{
    WCHAR han[ENGTYPO_MAXLEN + 1];
    char name[64];

    hangul_eng_typo(eng, han, ARRAYSIZE(han));
    wsprintfA(name, "eng typo: %ls", eng);
    CheckStr(name, expected, han);
}

static void TestEngTypo(void)
{
    CheckTypo(L"dkssudgktpdy", L"안녕하세요");
    CheckTypo(L"gksrmf", L"한글");
    CheckTypo(L"sp", L"네");
    CheckTypo(L"Rkt", L"깟"); // shift for the double consonant

    // not all syllables
    CheckTypo(L"hello", L"");
    CheckTypo(L"json", L"");
    // english words
    CheckTypo(L"the", L"");
    CheckTypo(L"with", L"");
    CheckTypo(L"The", L"");
    // other upper case letters, digits and too long words
    CheckTypo(L"Dkssud", L"");
    CheckTypo(L"dkssud1", L"");
    CheckTypo(L"dkssudgktpdydkssudgktpdydkssudgkt", L"");

    // only the standard 2-set keyboard
    set_keyboard(LAYOUT_3FIN);
    CheckTypo(L"gksrmf", L"");
    set_keyboard(LAYOUT_OLD2BUL);
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

    // the meaning is cut at the comma
    GetCandidateStringsFromDictionary(L"한", buf, 1024, L"nabi.dic");
    CheckStr("nabi.dic: comma", L"韓:나라 이름 한", buf);

    // hanja out of KS X 1001 only with KSX1002_SUPPORT
    CheckInt("nabi.dic: KS X 1001", 0, HasCandidate(buf, L"㸁"));
    dwOptionFlag |= KSX1002_SUPPORT;
    GetCandidateStringsFromDictionary(L"한", buf, 1024, L"nabi.dic");
    CheckInt("nabi.dic: KS X 1002", 1, HasCandidate(buf, L"㸁"));
    dwOptionFlag &= ~KSX1002_SUPPORT;

    n = GetCandidateStringsFromDictionary(L"ㄴ", buf, 1024, L"symwin.dic");
    CheckInt("symwin.dic: found", 1, n > 1);
    psz = buf;
    CheckStr("symwin.dic: 1st", L"＂", psz);
    psz += lstrlenW(psz) + 1;
    CheckStr("symwin.dic: 2nd", L"（", psz);

    GetCandidateStringsFromDictionary(L"대한", buf, 1024, L"word.dic");
    CheckList("word.dic: 대한", L"大寒\0大旱\0", buf);

    // the words starting with the reading
    dwOptionFlag |= SEARCH_SIMILAR_WORDS;
    GetCandidateStringsFromDictionary(L"대한", buf, 1024, L"word.dic");
    CheckList("word.dic: similar 대한", L"大寒\0大旱\0大韓民國\0", buf);

    // the last choseong matches any syllable starting with it
    GetCandidateStringsFromDictionary(L"대한ㅁ", buf, 1024, L"word.dic");
    CheckList("word.dic: similar 대한ㅁ", L"大韓民國\0", buf);
    dwOptionFlag &= ~SEARCH_SIMILAR_WORDS;
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
    TestBackspace();
    TestBackspaceCompat();
    TestBackspaceSyllable();
    TestHangul3();
    TestJamo();
    TestEngTypo();
    TestDictionary();
    TestHanjaIndex();

    printf("%d passed, %d failed\n", nPassed, nFailed);
    return nFailed ? 1 : 0;
}
