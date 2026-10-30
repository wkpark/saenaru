/*
 * Unit tests for the hangul automata and the dictionary lookup.
 * Run in the resource directory to read the dictionaries.
 */

#include <windows.h>
#include <stdio.h>
#include "hangul.h"

int GetCandidateStringsFromDictionary(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
int GetHangulFromHanjaIndex(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
UINT load_compose_map_from_buffer(LPWSTR kbuf, HangulCompose *compose_map);
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
    // the english words are not known without engwords.dic
    CheckTypo(L"dkssud", L"");
    CheckInt("engwords.dic: load for the english mode", TRUE, hangul_load_eng_words(TEXT("engwords.dic")));

    CheckTypo(L"dkssudgktpdy", L"안녕하세요");
    CheckTypo(L"gksrmf", L"한글");
    CheckTypo(L"gksrmfdl", L"한글이");
    CheckTypo(L"spt", L"넷");
    CheckTypo(L"sp", L"");  // 3 letters at least
    CheckTypo(L"gh", L"");
    CheckTypo(L"Rkt", L"깟"); // shift for the double consonant

    // not all syllables
    CheckTypo(L"hello", L"");
    CheckTypo(L"json", L"");
    // english words
    CheckTypo(L"the", L"");
    CheckTypo(L"with", L"");
    CheckTypo(L"The", L"");
    // the commands and terms of engwords-extra.txt
    CheckTypo(L"git", L"");     // 햣
    CheckTypo(L"sudo", L"");    // 녀애
    CheckTypo(L"env", L"");     // 둪
    CheckTypo(L"dir", L"");     // 약
    CheckTypo(L"div", L"");     // 얖
    CheckTypo(L"goto", L"");    // 해새
    CheckTypo(L"gpt", L"");     // 헷
    CheckTypo(L"tty", L"");     // 쑈
    // the words typed into the korean words are converted, except the commands
    CheckTypo(L"chr", L"촉");
    CheckTypo(L"dns", L"운");
    // other upper case letters, digits and too long words
    CheckTypo(L"Dkssud", L"");
    CheckTypo(L"dkssud1", L"");
    CheckTypo(L"gksrmfgksrmfgksrmfgksrmfgksrmfrk", L"한글한글한글한글한글가"); // 32 letters
    CheckTypo(L"dkssudgktpdydkssudgktpdydkssudgkt", L"");  // 33 letters

    // COMPOSITE_TYPING types more english words into syllables
    dwOptionFlag |= COMPOSITE_TYPING;
    CheckTypo(L"dkssud", L"안녕");
    CheckTypo(L"able", L"");
    CheckTypo(L"name", L"");    // 무드
    CheckTypo(L"window", L"");  // 쟈우재
    dwOptionFlag &= ~COMPOSITE_TYPING;

    // and the compose maps of the language bar
    set_compose(3);
    CheckTypo(L"chord", L"");   // 쵂
    set_keyboard(LAYOUT_OLD2BUL);

    // only the standard 2-set keyboard
    set_keyboard(LAYOUT_3FIN);
    CheckTypo(L"gksrmf", L"");
    set_keyboard(LAYOUT_OLD2BUL);

    hangul_free_eng_words();
}

static void CheckHanTypo(LPCWSTR eng, LPCWSTR expected)
{
    WCHAR han[ENGTYPO_MAXLEN + 1];
    char name[64];

    hangul_han_typo(eng, han, ARRAYSIZE(han));
    wsprintfA(name, "han typo: %ls", eng);
    CheckStr(name, expected, han);
}

static void TestHanTypo(void)
{
    CheckHanTypo(L"hello", L"");    // no dictionary
    CheckInt("engwords.dic: load", TRUE, hangul_load_eng_words(TEXT("engwords.dic")));

    CheckHanTypo(L"hello", L"ㅗ듸ㅐ"); // ㅣ+ㅣ is composed
    CheckHanTypo(L"Hello", L"ㅗ듸ㅐ"); // any upper case letters
    CheckHanTypo(L"you", L"ㅛㅐㅕ");    // vowels only
    CheckHanTypo(L"npm", L"ㅞㅡ");      // engwords-extra.txt
    CheckHanTypo(L"git", L"");          // 햣
    CheckHanTypo(L"water", L"ㅈㅁㅅㄷㄱ"); // consonants only

    // syllables, not english words, the jamos for chatting
    CheckHanTypo(L"go", L"");     // 해
    CheckHanTypo(L"dkssud", L""); // 안녕
    CheckHanTypo(L"zzz", L"");    // ㅋㅋㅋ
    CheckHanTypo(L"bb", L"");     // ㅠㅠ
    CheckHanTypo(L"dd", L"");     // ㅇㅇ
    CheckHanTypo(L"ac", L"");     // ㅁㅊ
    // 3 letters at least like as the english mode
    CheckHanTypo(L"ok", L"");     // ㅐㅏ
    CheckHanTypo(L"nb", L"");     // ㅜㅠ
    CheckHanTypo(L"mn", L"");     // ㅡㅜ
    CheckHanTypo(L"ml", L"");     // ㅢ
    // the faces of the jamos
    CheckHanTypo(L"dad", L"");    // ㅇㅁㅇ
    CheckHanTypo(L"gag", L"");    // ㅎㅁㅎ
    CheckHanTypo(L"dvd", L"");    // ㅇㅍㅇ
    CheckHanTypo(L"add", L"ㅁㅇㅇ");

    // 3 letters of the jamos for chatting converted now. not english words would be better.
    CheckHanTypo(L"gee", L"ㅎㄸ"); // ㄷ+ㄷ is composed

    // the same keys are composed by the options
    dwOptionFlag |= COMPOSITE_TYPING;
    CheckHanTypo(L"hello", L"되ㅒ");
    CheckHanTypo(L"korea", L"ㅏ갣ㅁ");
    CheckHanTypo(L"name", L"");   // 무드, syllables
    dwOptionFlag &= ~COMPOSITE_TYPING;

    // the 3-set keyboards: the jongseong keys, not the symbols of the shift
    set_keyboard(LAYOUT_3FIN);
    CheckHanTypo(L"hello", L"녀ㅉㅊ");
    CheckHanTypo(L"window", L"ㄹㅁ시ㅊㄹ"); // w is ㄹ of jongseong
    CheckHanTypo(L"Hello", L"");             // H is 0
    set_keyboard(LAYOUT_390);
    CheckHanTypo(L"hello", L"녀ㅉㅊ");
    set_keyboard(LAYOUT_3SUN);
    CheckHanTypo(L"window", L"ㄹㅁ시ㅊㄹ");
    dwOptionFlag |= COMPOSITE_TYPING;
    CheckHanTypo(L"window", L"");            // 말시찰, syllables
    dwOptionFlag &= ~COMPOSITE_TYPING;
    set_keyboard(LAYOUT_OLD2BUL);

    // the binary search of the lines
    CheckInt("eng word: a (first)", TRUE, hangul_is_eng_word("a"));
    CheckInt("eng word: zyuganov (last)", TRUE, hangul_is_eng_word("zyuganov"));
    CheckInt("eng word: hell", TRUE, hangul_is_eng_word("hell"));
    CheckInt("eng word: hellos", TRUE, hangul_is_eng_word("hellos"));
    CheckInt("eng word: hel (prefix)", FALSE, hangul_is_eng_word("hel"));
    CheckInt("eng word: hellox", FALSE, hangul_is_eng_word("hellox"));
    CheckInt("eng word: (before the first)", FALSE, hangul_is_eng_word("0"));
    CheckInt("eng word: zzzzzz (after the last)", FALSE, hangul_is_eng_word("zzzzzz"));
    CheckInt("eng word: empty", FALSE, hangul_is_eng_word(""));
    CheckInt("engwords.dic: loaded once", TRUE, hangul_load_eng_words(TEXT("nofile.dic")));

    hangul_free_eng_words();
    CheckHanTypo(L"hello", L"");
    CheckInt("engwords.dic: no file", FALSE, hangul_load_eng_words(TEXT("nofile.dic")));
}

// a word list with CRLF and comments
static void TestEngWordsFile(void)
{
    static const char c_szFile[] = "# comment\r\n#\r\napple\r\nhello\r\nzoo\r\n";
    TCHAR szPath[MAX_PATH];
    HANDLE hFile;
    DWORD cb;

    GetTempPath(MAX_PATH, szPath);
    lstrcat(szPath, TEXT("saenaru_engwords.dic"));
    hFile = CreateFile(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, c_szFile, sizeof(c_szFile) - 1, &cb, NULL);
    CloseHandle(hFile);

    CheckInt("CRLF words: load", TRUE, hangul_load_eng_words(szPath));
    CheckInt("CRLF words: apple", TRUE, hangul_is_eng_word("apple"));
    CheckInt("CRLF words: hello", TRUE, hangul_is_eng_word("hello"));
    CheckInt("CRLF words: zoo", TRUE, hangul_is_eng_word("zoo"));
    CheckInt("CRLF words: comment", FALSE, hangul_is_eng_word("# comment"));
    CheckInt("CRLF words: hell", FALSE, hangul_is_eng_word("hell"));
    hangul_free_eng_words();
    DeleteFile(szPath);
}

// 2: the text before the caret
static void TestTypoMatch(void)
{
    CheckInt("match: dkssud", TRUE, hangul_typo_match(L" dkssud", L"dkssud", FALSE));
    CheckInt("match: the start of the text", TRUE, hangul_typo_match(L"dkssud", L"dkssud", FALSE));
    CheckInt("match: after hangul", TRUE, hangul_typo_match(L"녕gktpdy", L"gktpdy", FALSE));
    CheckInt("match: after (", TRUE, hangul_typo_match(L"(dkssud", L"dkssud", FALSE));
    CheckInt("match: ㅗ듸ㅐ", TRUE, hangul_typo_match(L" ㅗ듸ㅐ", L"ㅗ듸ㅐ", FALSE));
    CheckInt("match: a part of a word", FALSE, hangul_typo_match(L"xdkssud", L"dkssud", FALSE));
    CheckInt("match: identifier _", FALSE, hangul_typo_match(L"_dkssud", L"dkssud", FALSE));
    CheckInt("match: identifier 1", FALSE, hangul_typo_match(L"1dkssud", L"dkssud", FALSE));
    CheckInt("match: not the same", FALSE, hangul_typo_match(L" dkssux", L"dkssud", FALSE));
    // not a part of a hangul word like as "/help" (ㅗ녀ㅈㅍ) on the 3-set
    CheckInt("match: jamo before", FALSE, hangul_typo_match(L"ㅗ녀ㅈㅍ", L"녀ㅈㅍ", FALSE));
    CheckInt("match: symbol before", TRUE, hangul_typo_match(L"(ㅗ듸ㅐ", L"ㅗ듸ㅐ", FALSE));
    CheckInt("match: too short", FALSE, hangul_typo_match(L"ssud", L"dkssud", FALSE));
    CheckInt("match: empty", FALSE, hangul_typo_match(L" ", L"", FALSE));

    CheckInt("undo: 안녕 ", TRUE, hangul_typo_match(L"안녕 ", L"안녕", TRUE));
    CheckInt("undo: no space", FALSE, hangul_typo_match(L"x안녕", L"안녕", TRUE));
    CheckInt("undo: two spaces", FALSE, hangul_typo_match(L"녕  ", L"안녕", TRUE));
    CheckInt("undo: other text", FALSE, hangul_typo_match(L"한글 ", L"안녕", TRUE));
}

/*
 * 1: type the keys like as the TIP. OnTestKeyDown() is called for all the
 * keys, and OnKeyDown() for the keys eaten by it and the letters in the
 * hangul mode. '<' is <LEFT>, '\b' <BACKSPACE>, '\n' <ENTER>, '\x1b' <ESC>,
 * '^' <CTRL> with the next key, and '|' changes the mode not by the hangul
 * key like as the language bar. Returns the actions for the keys:
 * '.' none, 'C' converted, 'U' restored.
 */
static void TypoKeys(HangulTypo *t, BOOL *pfHangul, LPCSTR keys, LPSTR actions)
{
    BOOL fCtrl = FALSE, fLetter;
    UINT vk;
    WCHAR ch;
    int action;

    for (; *keys; keys++)
    {
        switch (*keys)
        {
            case '|': *pfHangul = !*pfHangul; continue;
            case '^': fCtrl = TRUE; continue;
            case '<': vk = VK_LEFT; ch = 0; break;
            case '\b': vk = VK_BACK; ch = L'\b'; break;
            case '\n': vk = VK_RETURN; ch = L'\r'; break;
            case '\x1b': vk = VK_ESCAPE; ch = 0x1b; break;
            case ' ': vk = VK_SPACE; ch = L' '; break;
            default:
                vk = (*keys >= 'a' && *keys <= 'z') ? (UINT)(*keys - 'a' + 'A') : (UINT)*keys;
                ch = (WCHAR)*keys;
                break;
        }
        fLetter = (ch >= L'a' && ch <= L'z') || (ch >= L'A' && ch <= L'Z');

        action = hangul_typo_key(t, vk, ch, *pfHangul, FALSE, fCtrl, FALSE, TRUE);
        if (action != HANGUL_TYPO_NONE || (*pfHangul && fLetter && !fCtrl))
            action = hangul_typo_key(t, vk, ch, *pfHangul, FALSE, fCtrl, FALSE, FALSE);
        fCtrl = FALSE;

        if (action == HANGUL_TYPO_CONVERT)
        {
            // the text is the same
            hangul_typo_end(t, TRUE);
            *pfHangul = !*pfHangul;
            *actions++ = 'C';
        }
        else if (action == HANGUL_TYPO_UNDO)
        {
            *pfHangul = t->toEng;
            *actions++ = 'U';
        }
        else
        {
            *actions++ = '.';
        }
    }
    *actions = '\0';
}

static void CheckTypoKeys(LPCSTR name, BOOL fHangul, LPCSTR keys, LPCSTR expected, BOOL fHangulExpected)
{
    HangulTypo t;
    char actions[128];

    hangul_typo_init(&t);
    TypoKeys(&t, &fHangul, keys, actions);
    if (lstrcmpA(actions, expected) == 0 && fHangul == fHangulExpected)
    {
        nPassed++;
        return;
    }
    nFailed++;
    printf("FAIL %s: expected %s (%s), actual %s (%s)\n", name, expected,
           fHangulExpected ? "hangul" : "english", actions, fHangul ? "hangul" : "english");
}

static void TestTypoKeys(void)
{
    HangulTypo t;
    BOOL fHangul;
    char actions[64];
    LPCSTR keys;

    hangul_load_eng_words(TEXT("engwords.dic"));

    // english mode
    CheckTypoKeys("eng: dkssud", FALSE, "dkssud ", "......C", TRUE);
    CheckTypoKeys("eng: hello", FALSE, "hello ", "......", FALSE);
    CheckTypoKeys("eng: two words", FALSE, "hi dkssud ", ".........C", TRUE);
    CheckTypoKeys("eng: backspace in the word", FALSE, "dkssux\bd ", "........C", TRUE);
    CheckTypoKeys("eng: left in the word", FALSE, "dks<sud ", "........", FALSE);
    CheckTypoKeys("eng: escape", FALSE, "dks\x1bsud ", "........", FALSE);
    CheckTypoKeys("eng: ctrl in the word", FALSE, "dk^vssud ", "........", FALSE);
    CheckTypoKeys("eng: digit in the word", FALSE, "dks1sud ", "........", FALSE);
    CheckTypoKeys("eng: a new word after the space", FALSE, "dks<sud dkssud ", "..............C", TRUE);
    CheckTypoKeys("eng: a new word after enter", FALSE, "dk<\ndkssud ", "..........C", TRUE);
    CheckTypoKeys("eng: backspace from the previous word", FALSE, "ab \bdkssud ", "...........", FALSE);

    // hangul mode
    CheckTypoKeys("han: hello", TRUE, "hello ", ".....C", FALSE);
    CheckTypoKeys("han: backspace in the word", TRUE, "helo\blo ", "........", TRUE);
    CheckTypoKeys("han: gksrmf", TRUE, "gksrmf ", ".......", TRUE);
    set_keyboard(LAYOUT_3FIN);
    CheckTypoKeys("han 3-set: hello", TRUE, "hello ", ".....C", FALSE);
    CheckTypoKeys("han 3-set: undo", TRUE, "window \b", "......CU", TRUE);
    CheckTypoKeys("han 3-set: jfshea", TRUE, "jfshea ", ".......", TRUE); // 안녕
    set_keyboard(LAYOUT_OLD2BUL);
    // <BACKSPACE> deletes a jamo, not a key. the next word after <ENTER>.
    CheckTypoKeys("han: backspace, then hello", TRUE, "rk\bhello ", ".........", TRUE);
    CheckTypoKeys("han: backspace, enter, then hello", TRUE, "rk\b\nhello ", ".........C", FALSE);

    // the mode changed not by the hangul key: a new word
    CheckTypoKeys("mode: hangul backspace, then english", TRUE, "rk\b|dkssud ", ".........C", TRUE);
    CheckTypoKeys("mode: hangul keys, then english", TRUE, "rk|dkssud ", "........C", TRUE);
    CheckTypoKeys("mode: english left, then hangul", FALSE, "dk<|hello ", "........C", FALSE);

    // undo just after the conversion
    CheckTypoKeys("undo: dkssud", FALSE, "dkssud \b", "......CU", FALSE);
    CheckTypoKeys("undo: hello", TRUE, "hello \b", ".....CU", TRUE);
    CheckTypoKeys("undo: gksrmfdl", FALSE, "gksrmfdl \b", "........CU", FALSE);
    CheckTypoKeys("undo: not after a key", FALSE, "dkssud r\b", "......C..", TRUE);
    CheckTypoKeys("undo: not after a space", FALSE, "dkssud  \b", "......C..", TRUE);
    CheckTypoKeys("undo: once", FALSE, "dkssud \b\b", "......CU.", FALSE);
    // not the undo, the <BACKSPACE> back in the previous word
    CheckTypoKeys("undo: mode changed back", FALSE, "dkssud |\brkd ", "......C.....", FALSE);
    CheckTypoKeys("undo: hangul mode changed back", TRUE, "hello |\bhello ", ".....C.......", TRUE);
    CheckTypoKeys("undo: ctrl", TRUE, "hello ^\bdkssud ", ".....C........", FALSE);

    // OnTestKeyDown() twice gives the same answer
    hangul_typo_init(&t);
    fHangul = FALSE;
    TypoKeys(&t, &fHangul, "dkssud", actions);
    CheckInt("twice: space 1", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', FALSE, FALSE, FALSE, FALSE, TRUE));
    CheckInt("twice: space 2", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', FALSE, FALSE, FALSE, FALSE, TRUE));
    CheckStr("twice: the word", L"dkssud", t.word);
    hangul_typo_init(&t);
    TypoKeys(&t, &fHangul, "hi", actions);
    CheckInt("twice: not a typo 1", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_SPACE, L' ', FALSE, FALSE, FALSE, FALSE, TRUE));
    CheckInt("twice: not a typo 2", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_SPACE, L' ', FALSE, FALSE, FALSE, FALSE, TRUE));

    // the word checked at <SPACE> is kept in typo for the TIP, not for the undo
    hangul_typo_init(&t);
    fHangul = FALSE;
    TypoKeys(&t, &fHangul, "dkssud", actions);
    CheckInt("check: test", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', fHangul, FALSE, FALSE, FALSE, TRUE));
    CheckStr("check: the hangul", L"안녕", t.typo);
    CheckStr("check: no undo", L"", t.han);

    // not the same text on the screen. the application gets the <SPACE>, and
    // the undo of the conversion before is kept.
    hangul_typo_init(&t);
    fHangul = FALSE;
    TypoKeys(&t, &fHangul, "dkssud |rkd", actions);
    CheckInt("not the same: test", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', fHangul, FALSE, FALSE, FALSE, TRUE));
    hangul_typo_end(&t, FALSE);
    CheckStr("not the same: the word", L"", t.typo);
    CheckStr("not the same: the hangul of the undo", L"안녕", t.han);
    CheckStr("not the same: the english of the undo", L"dkssud", t.eng);
    CheckInt("not the same: keydown", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_SPACE, L' ', fHangul, FALSE, FALSE, FALSE, FALSE));

    // OnKeyDown() checks the word again, like as after the layout changed
    // for the <SPACE> eaten without OnKeyDown()
    hangul_typo_init(&t);
    fHangul = FALSE;
    TypoKeys(&t, &fHangul, "dkssud", actions);
    CheckInt("keydown: test", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', fHangul, FALSE, FALSE, FALSE, TRUE));
    set_keyboard(LAYOUT_3FIN);
    CheckInt("keydown: the layout changed", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_SPACE, L' ', fHangul, FALSE, FALSE, FALSE, FALSE));
    set_keyboard(LAYOUT_OLD2BUL);

    // OnKeyDown() without OnTestKeyDown()
    hangul_typo_init(&t);
    CheckInt("keydown: space", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_SPACE, L' ', FALSE, FALSE, FALSE, FALSE, FALSE));
    hangul_typo_key(&t, 'D', L'd', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'K', L'k', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'S', L's', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'S', L's', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'X', L'x', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, VK_BACK, L'\b', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'U', L'u', FALSE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'D', L'd', FALSE, FALSE, FALSE, FALSE, FALSE);
    CheckInt("keydown: dkssud", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', FALSE, FALSE, FALSE, FALSE, FALSE));
    CheckStr("keydown: the word", L"dkssud", t.word);
    hangul_typo_init(&t);
    hangul_typo_key(&t, 'Y', L'y', TRUE, FALSE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'O', L'o', TRUE, TRUE, FALSE, FALSE, FALSE);
    hangul_typo_key(&t, 'U', L'u', TRUE, TRUE, FALSE, FALSE, FALSE);
    CheckInt("keydown: han you", HANGUL_TYPO_CONVERT, hangul_typo_key(&t, VK_SPACE, L' ', TRUE, TRUE, FALSE, FALSE, FALSE));

    // OnTestKeyDown() twice for a letter eaten in the hangul mode
    hangul_typo_init(&t);
    for (keys = "hello"; *keys; keys++)
    {
        hangul_typo_key(&t, (UINT)(*keys - 'a' + 'A'), (WCHAR)*keys, TRUE, TRUE, FALSE, FALSE, TRUE);
        hangul_typo_key(&t, (UINT)(*keys - 'a' + 'A'), (WCHAR)*keys, TRUE, TRUE, FALSE, FALSE, TRUE);
        hangul_typo_key(&t, (UINT)(*keys - 'a' + 'A'), (WCHAR)*keys, TRUE, TRUE, FALSE, FALSE, FALSE);
    }
    CheckStr("test twice: han word", L"hello", t.word);

    // the word of the undo is the hangul typed with the options
    dwOptionFlag |= COMPOSITE_TYPING;
    hangul_typo_init(&t);
    fHangul = TRUE;
    TypoKeys(&t, &fHangul, "hello ", actions);
    CheckInt("moachigi: hello", 0, lstrcmpA(actions, ".....C"));
    CheckInt("moachigi: the english mode", FALSE, fHangul);
    CheckStr("moachigi: the hangul", L"되ㅒ", t.han);
    CheckStr("moachigi: the english", L"hello", t.eng);
    dwOptionFlag &= ~COMPOSITE_TYPING;

    // the shift, the composition and the ctrl stop the undo
    hangul_typo_init(&t);
    fHangul = FALSE;
    TypoKeys(&t, &fHangul, "dkssud ", actions);
    CheckInt("undo: shift", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_BACK, L'\b', fHangul, FALSE, FALSE, TRUE, TRUE));
    hangul_typo_init(&t);
    fHangul = FALSE;
    TypoKeys(&t, &fHangul, "dkssud ", actions);
    CheckInt("undo: composing", HANGUL_TYPO_NONE, hangul_typo_key(&t, VK_BACK, L'\b', fHangul, TRUE, FALSE, FALSE, TRUE));
    CheckStr("undo: the hangul", L"안녕", t.han);
    CheckStr("undo: the english", L"dkssud", t.eng);

    hangul_free_eng_words();
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

    // the double consonants were after ㅎ, past the end of the search
    for (psz = L"ㄲㄸㅃㅆ"; *psz; psz++) {
        WCHAR read[2];
        read[0] = *psz;
        read[1] = L'\0';
        n = GetCandidateStringsFromDictionary(read, buf, 1024, L"symwin.dic");
        CheckInt("symwin.dic: a double consonant", 1, n > 1);
    }

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

// a blank line before each word made the carried-over text of the parser
// grow on every read, past its stack buffer
static void TestDictionaryCarry(void)
{
    WCHAR szDir[MAX_PATH], szPath[MAX_PATH];
    WCHAR buf[1024 + 2];
    LPCWSTR psz;
    HANDLE hFile;
    DWORD dw;
    int i, n;

    GetTempPathW(MAX_PATH, szDir);
    GetTempFileNameW(szDir, L"sdc", 0, szPath);
    hFile = CreateFileW(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, L"\xfeff[가]\n", 5 * sizeof(WCHAR), &dw, NULL);
    for (i = 0; i < 2000; i++)
        WriteFile(hFile, L"\n가\n", 3 * sizeof(WCHAR), &dw, NULL);
    CloseHandle(hFile);

    n = GetCandidateStringsFromDictionary(L"가", buf, 1024, szPath);
    CheckInt("crafted dic: found", 1, n > 1);
    CheckStr("crafted dic: 1st", L"가", buf);
    // the words up to the buffer, not a few of them
    for (i = 0, psz = buf; n > 0 && *psz; psz += lstrlenW(psz) + 1)
        i++;
    CheckInt("crafted dic: the buffer filled", 1, i >= 500);
    DeleteFileW(szPath);
}

// a header cut by a read of 1024 chars was not found
static void TestDictionaryCutHeader(void)
{
    WCHAR szDir[MAX_PATH], szPath[MAX_PATH];
    WCHAR buf[1024 + 2];
    HANDLE hFile;
    DWORD dw;
    int i, n;

    GetTempPathW(MAX_PATH, szDir);
    GetTempFileNameW(szDir, L"sdc", 0, szPath);
    hFile = CreateFileW(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    // 5 + 509 * 2 chars: [나라] starts at 1023, the first read ends at 1024
    WriteFile(hFile, L"\xfeff[가]\n", 5 * sizeof(WCHAR), &dw, NULL);
    for (i = 0; i < 509; i++)
        WriteFile(hFile, L"可\n", 2 * sizeof(WCHAR), &dw, NULL);
    WriteFile(hFile, L"[나라]\n那=어찌 나\n", 12 * sizeof(WCHAR), &dw, NULL);
    CloseHandle(hFile);

    n = GetCandidateStringsFromDictionary(L"나라", buf, 1024, szPath);
    CheckInt("cut header: found", 1, n > 1);
    CheckStr("cut header: 1st", L"那:어찌 나", buf);
    DeleteFileW(szPath);

    // the last line of a file without its line end
    hFile = CreateFileW(szPath, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    WriteFile(hFile, L"\xfeff[가]\n可=옳을 가\n家=집 가", 17 * sizeof(WCHAR), &dw, NULL);
    CloseHandle(hFile);
    buf[0] = buf[1] = L'\0';
    GetCandidateStringsFromDictionary(L"가", buf, 1024, szPath);
    CheckStr("last line: 2nd", L"家:집 가", buf + lstrlenW(buf) + 1);
    DeleteFileW(szPath);
}

// the terminator of a compose map of 256 entries stays in the map
static void TestComposeMapSize(void)
{
    static WCHAR kbuf[256 * 16 + 2];
    HangulCompose map[256 + 1];
    LPWSTR p = kbuf;
    int i;

    for (i = 0; i < 256; i++)
        p += wsprintfW(p, L"1100 %X AC00", 0x1000 + i) + 1;
    *p = L'\0';
    map[256].key = 0x5a5a5a5a;
    load_compose_map_from_buffer(kbuf, map);
    CheckInt("compose map: 256 entries", 0x5a5a5a5a, (int)map[256].key);
    CheckInt("compose map: terminator", -1, (int)map[255].key);
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
    TestHanTypo();
    TestEngWordsFile();
    TestTypoMatch();
    TestTypoKeys();
    TestDictionary();
    TestHanjaIndex();
    TestDictionaryCarry();
    TestDictionaryCutHeader();
    TestComposeMapSize();

    printf("%d passed, %d failed\n", nPassed, nFailed);
    return nFailed ? 1 : 0;
}
