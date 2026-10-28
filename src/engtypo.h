/*
 * The hangul typed in the english mode and the reverse (engtypo.c).
 * The state of the word typed is kept without the TSF to be tested.
 */

#ifndef ENGTYPO_H
#define ENGTYPO_H

#ifdef __cplusplus
extern "C" {
#endif

#define ENGTYPO_MAXLEN  32

typedef struct _tagHangulTypo {
    WCHAR word[ENGTYPO_MAXLEN + 1]; // the letters of the word
    int   len;
    BOOL  off;      // the caret is moved in the word
    BOOL  hangul;   // the mode of the word
    WCHAR typo[ENGTYPO_MAXLEN + 1]; // the hangul of the word checked at <SPACE>
    BOOL  undo;     // just after the conversion
    BOOL  toEng;    // converted to the english
    WCHAR eng[ENGTYPO_MAXLEN + 1];  // the words of the conversion
    WCHAR han[ENGTYPO_MAXLEN + 1];
} HangulTypo;

// the actions of hangul_typo_key(). the TIP checks the text to eat the key.
#define HANGUL_TYPO_NONE    0
#define HANGUL_TYPO_CONVERT 1   // <SPACE> to convert the word
#define HANGUL_TYPO_UNDO    2   // <BACKSPACE> to restore the word

int hangul_eng_typo(LPCWSTR, LPWSTR, int);
BOOL hangul_load_eng_words(LPCTSTR);
void hangul_free_eng_words(void);
BOOL hangul_is_eng_word(const char *);
int hangul_han_typo(LPCWSTR, LPWSTR, int);

void hangul_typo_init(HangulTypo *);
int hangul_typo_key(HangulTypo *, UINT, WCHAR, BOOL, BOOL, BOOL, BOOL, BOOL);
void hangul_typo_end(HangulTypo *, BOOL);
BOOL hangul_typo_match(LPCWSTR, LPCWSTR, BOOL);

#ifdef __cplusplus
}
#endif

#endif // ENGTYPO_H
