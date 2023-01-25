/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 2003 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2003-2023 Won-Kyu Park <wkpark@kldp.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#define hangul_is_choseong(ch)	(((ch) >= 0x1100 && (ch) <= 0x115f) || ((ch) >= 0xa960 && (ch) <= 0xa97c))
#define hangul_is_jungseong(ch)	(((ch) >= 0x1160 && (ch) <= 0x11a7) || ((ch) >= 0xd7b0 && (ch) <= 0xd7c6))
#define hangul_is_jongseong(ch)	(((ch) >= 0x11a8 && (ch) <= 0x11ff) || ((ch) >= 0xd7cb && (ch) <= 0xd7fb))
#define hangul_is_bangjum(ch)	((ch) == 0x302e || (ch) == 0x302f)
#define is_combining_mark(ch)	((ch) >= 0x0300 && (ch) <= 0x036f || \
	(ch) >= 0x1dc0 && (ch) <= 0x1de6 || (ch) >= 0x20d0 && (ch) <= 0x20f0)

#define LAYOUT_OLD2BUL      0x00000001
#define LAYOUT_3FIN         0x00000002
#define LAYOUT_390          0x00000003
#define LAYOUT_3SUN         0x00000004
#define LAYOUT_NEW2BUL      0x00000005
#define LAYOUT_NEW3BUL      0x00000006
#define LAYOUT_AHNMATAE     0x00000007
#define LAYOUT_YET2BUL      0x00000008
#define LAYOUT_YET3BUL      0x00000009
#define LAYOUT_NK_2BUL      0x0000000a
#define LAYOUT_USER         0x0000000b

#define COMPOSITE_TYPING       0x00000001
#define CONCURRENT_TYPING      0x00000002
#define PSEUDO_COMPOSITE       0x00000004
#define BACKSPACE_BY_JAMO      0x00000008
#define KSX1001_SUPPORT        0x00000010
#define KSX1002_SUPPORT        0x00000020
#define FULL_MULTIJOMO         0x00000040
#define USE_SHIFT_SPACE        0x00000080
#define DVORAK_SUPPORT         0x00000100
#define ESCENG_SUPPORT         0x00000200
#define QWERTY_HOTKEY_SUPPORT  0x00000400
#define HANGUL_JAMOS           0x00000800
#define HANJA_CAND_WITH_SPACE  0x00001000
#define SEARCH_SIMILAR_WORDS   0x00002000
#define USE_CTRL_SPACE         0x00004000
#define SCANCODE_BASED         0x00010000

#define SAENARU_ONTHESPOT      0x00020000
#define SAENARU_MODE_CANDIDATE 0x00040000
#define AUTOMATA_2SET          0x00100000
#define AUTOMATA_3SET          0x00200000
#define AUTOMATA_NULL          0x00400000

#ifndef LPMYSTR
typedef LPTSTR            LPMYSTR;
#endif

#ifndef MYTEXT
#define MYTEXT(x)         TEXT(x)
#endif

#define MAXICREADSIZE   10+2
#define MAXICCOMPSIZE   2
typedef struct _tagHangulIC {
    //WCHAR read[10]; // L{1,3}V{1,3}T{0,3}
    WCHAR read[MAXICREADSIZE];
    WCHAR comp[MAXICCOMPSIZE]; // NOT USED
    WCHAR cho;
    WCHAR jung;
    WCHAR jong;
    WCHAR last;
    WCHAR last0;
    UINT  len;
    UINT  lastvkey;
    int   laststate;
    BOOL  syllable;
} HangulIC;

struct _HangulCompose
{
  UINT key;
  WCHAR code;
};

typedef struct _HangulCompose	HangulCompose;

extern HangulIC ic;
extern WCHAR *keyboard_table;

extern int (*hangul_automata)(HangulIC*, WCHAR, LPMYSTR, int *);
int hangul_automata2( HangulIC *ic, WCHAR jamo, LPMYSTR lcs, int *ncs);
int hangul_automata3( HangulIC *ic, WCHAR jamo, LPMYSTR lcs, int *ncs);
void PASCAL hangul_ic_init( HangulIC *ic );

UINT PASCAL hangul_ic_get(HangulIC*, UINT, LPMYSTR);
WCHAR PASCAL hangul_ic_pop(HangulIC*);
WCHAR PASCAL hangul_ic_peek(HangulIC*);
void PASCAL  hangul_ic_init(HangulIC*);
void hangul_ic_push(HangulIC *, WCHAR);

int hangul_del_prev(LPWSTR);
WCHAR PASCAL keyToHangulKey(WCHAR);
WORD PASCAL checkDvorak(WORD);
int PASCAL set_keyboard(UINT);
int PASCAL set_compose(UINT);
int PASCAL set_automata(UINT);
WCHAR PASCAL hangul_compose(WCHAR first, WCHAR last);

BOOL hangul_is_syllable(WCHAR);
void PASCAL hangul_syllable_to_jamo(WCHAR, WCHAR*, WCHAR*, WCHAR*);
WCHAR PASCAL hangul_jamo_to_syllable(WCHAR, WCHAR, WCHAR);
WCHAR PASCAL hangul_jamo_to_cjamo_full(WCHAR key);
WCHAR PASCAL hangul_jamo_to_cjamo(WCHAR key);
BOOL hangul_is_cjamo(WCHAR c);
