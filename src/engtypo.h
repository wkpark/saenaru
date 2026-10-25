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

// the english typed in the hangul mode is converted too
#define ENGTYPO_HAN_TO_ENG  1

typedef struct _tagHangulTypo {
    WCHAR word[ENGTYPO_MAXLEN + 1]; // the letters of the word
    int   len;
    BOOL  off;      // the caret is moved in the word
    BOOL  hangul;   // the mode of the word
    BOOL  undo;     // just after the conversion
    BOOL  toEng;    // converted to the english
    WCHAR eng[ENGTYPO_MAXLEN + 1];  // the words of the conversion
    WCHAR han[ENGTYPO_MAXLEN + 1];
} HangulTypo;

// the actions of hangul_typo_key()
#define HANGUL_TYPO_NONE    0
#define HANGUL_TYPO_CONVERT 1   // eat <SPACE> to convert the word
#define HANGUL_TYPO_UNDO    2   // eat <BACKSPACE> to restore the word

BOOL hangul_is_old2bul(void);
int hangul_eng_to_syllables(LPCWSTR, LPWSTR, int);
int hangul_eng_typo(LPCWSTR, LPWSTR, int);
BOOL hangul_load_eng_words(LPCTSTR);
void hangul_free_eng_words(void);
BOOL hangul_is_eng_word(const char *);
int hangul_han_typo(LPCWSTR, LPWSTR, int);

void hangul_typo_init(HangulTypo *);
void hangul_typo_reset_word(HangulTypo *, BOOL);
int hangul_typo_check(HangulTypo *, BOOL, LPWSTR, int);
int hangul_typo_key(HangulTypo *, UINT, WCHAR, BOOL, BOOL, BOOL, BOOL, BOOL);
void hangul_typo_end(HangulTypo *, BOOL, LPCWSTR);
BOOL hangul_typo_match(LPCWSTR, LPCWSTR, BOOL);

#ifdef __cplusplus
}
#endif

#endif // ENGTYPO_H
