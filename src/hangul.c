/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 2002, 2003 Choe Hwan-Jin <krisna@kldp.org>
 * Copyright (c) 2003 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2003 Won-Kyu Park <wkpark@kldp.org>.
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
 * $Saenaru: saenaru/src/hangul.c,v 1.29 2006/11/24 23:00:37 wkpark Exp $
 */

#include <windows.h>
#include <immdev.h>
#include "saenaru.h"
#include "vksub.h"

// Hangul Input context
HangulIC ic;

int hangul_automata2( HangulIC *ic, WCHAR jamo, WCHAR *cs);
int hangul_automata3( HangulIC *ic, WCHAR jamo, WCHAR *cs);
void PASCAL hangul_ic_init( HangulIC *ic );

WCHAR PASCAL
hangul_choseong_to_cjamo(WCHAR ch)
{
    static const WCHAR table[] = {
	0x3131,	    /* 0x1100 */
	0x3132,	    /* 0x1101 */
	0x3134,	    /* 0x1102 */
	0x3137,	    /* 0x1103 */
	0x3138,	    /* 0x1104 */
	0x3139,	    /* 0x1105 */
	0x3141,	    /* 0x1106 */
	0x3142,	    /* 0x1107 */
	0x3143,	    /* 0x1108 */
	0x3145,	    /* 0x1109 */
	0x3146,	    /* 0x110a */
	0x3147,	    /* 0x110b */
	0x3148,	    /* 0x110c */
	0x3149,	    /* 0x110d */
	0x314a,	    /* 0x110e */
	0x314b,	    /* 0x110f */
	0x314c,	    /* 0x1110 */
	0x314d,	    /* 0x1111 */
	0x314e	    /* 0x1112 */
    };
    if (ch < 0x1100 || ch > 0x1112)
    {
        if (ch == 0x1140)
            return 0x317f;
        else if (ch == 0x114C)
            return 0x3181;
        else if (ch == 0x1159)
            return 0x3186;
	return 0;
    }
    return table[ch - 0x1100];
}

#define hangul_is_choseong(ch)	((ch) >= 0x1100 && (ch) <= 0x1159)
#define hangul_is_jungseong(ch)	((ch) >= 0x1161 && (ch) <= 0x11a2)
#define hangul_is_jongseong(ch)	((ch) >= 0x11a7 && (ch) <= 0x11f9)

WCHAR PASCAL
hangul_jungseong_to_cjamo(WCHAR ch)
{
    static const WCHAR table[] = {
	0x314f,	    /* 0x1161 */
	0x3150,	    /* 0x1162 */
	0x3151,	    /* 0x1163 */
	0x3152,	    /* 0x1164 */
	0x3153,	    /* 0x1165 */
	0x3154,	    /* 0x1166 */
	0x3155,	    /* 0x1167 */
	0x3156,	    /* 0x1168 */
	0x3157,	    /* 0x1169 */
	0x3158,	    /* 0x116a */
	0x3159,	    /* 0x116b */
	0x315a,	    /* 0x116c */
	0x315b,	    /* 0x116d */
	0x315c,	    /* 0x116e */
	0x315d,	    /* 0x116f */
	0x315e,	    /* 0x1170 */
	0x315f,	    /* 0x1171 */
	0x3160,	    /* 0x1172 */
	0x3161,	    /* 0x1173 */
	0x3162,	    /* 0x1174 */
	0x3163	    /* 0x1175 */
    };
    if (ch < 0x1161 || ch > 0x1175)
    {
        if (ch == 0x119E)
            return 0x318D;
	return 0;
    }
    return table[ch - 0x1161];
}

WCHAR PASCAL
hangul_jongseong_to_cjamo(WCHAR ch)
{
    static const WCHAR table[] = {
	0x3131,	    /* 0x11a8 */
	0x3132,	    /* 0x11a9 */
	0x3133,	    /* 0x11aa */
	0x3134,	    /* 0x11ab */
	0x3135,	    /* 0x11ac */
	0x3136,	    /* 0x11ad */
	0x3137,	    /* 0x11ae */
	0x3139,	    /* 0x11af */
	0x313a,	    /* 0x11b0 */
	0x313b,	    /* 0x11b1 */
	0x313c,	    /* 0x11b2 */
	0x313d,	    /* 0x11b3 */
	0x313e,	    /* 0x11b4 */
	0x313f,	    /* 0x11b5 */
	0x3140,	    /* 0x11b6 */
	0x3141,	    /* 0x11b7 */
	0x3142,	    /* 0x11b8 */
	0x3144,	    /* 0x11b9 */
	0x3145,	    /* 0x11ba */
	0x3146,	    /* 0x11bb */
	0x3147,	    /* 0x11bc */
	0x3148,	    /* 0x11bd */
	0x314a,	    /* 0x11be */
	0x314b,	    /* 0x11bf */
	0x314c,	    /* 0x11c0 */
	0x314d,	    /* 0x11c1 */
	0x314e	    /* 0x11c2 */
    };
    if (ch < 0x11a8 || ch > 0x11c2)
    {
        if (ch == 0x11EB)
            return 0x317f;
        else if (ch == 0x11F0)
            return 0x3181;
        else if (ch == 0x11F9)
            return 0x3186;
	return 0;
    }
    return table[ch - 0x11a8];
}

WCHAR PASCAL
hangul_choseong_to_jongseong(WCHAR ch)
{
    static const WCHAR table[] = {
      0x11a8,  /* choseong kiyeok      -> jongseong kiyeok      */
      0x11a9,  /* choseong ssangkiyeok -> jongseong ssangkiyeok */
      0x11ab,  /* choseong nieun       -> jongseong nieun       */
      0x11ae,  /* choseong tikeut      -> jongseong tikeut      */
      0x0,     /* choseong ssangtikeut -> jongseong tikeut      */
      0x11af,  /* choseong rieul       -> jongseong rieul       */
      0x11b7,  /* choseong mieum       -> jongseong mieum       */
      0x11b8,  /* choseong pieup       -> jongseong pieup       */
      0x0,     /* choseong ssangpieup  -> jongseong pieup       */
      0x11ba,  /* choseong sios        -> jongseong sios        */
      0x11bb,  /* choseong ssangsios   -> jongseong ssangsios   */
      0x11bc,  /* choseong ieung       -> jongseong ieung       */
      0x11bd,  /* choseong cieuc       -> jongseong cieuc       */
      0x0,     /* choseong ssangcieuc  -> jongseong cieuc       */
      0x11be,  /* choseong chieuch     -> jongseong chieuch     */
      0x11bf,  /* choseong khieukh     -> jongseong khieukh     */
      0x11c0,  /* choseong thieuth     -> jongseong thieuth     */
      0x11c1,  /* choseong phieuph     -> jongseong phieuph     */
      0x11c2   /* choseong hieuh       -> jongseong hieuh       */
    };
    if (ch < 0x1100 || ch > 0x1112)
	return 0;
    return table[ch - 0x1100];
}

WCHAR PASCAL
hangul_jongseong_to_choseong(WCHAR ch)
{
    static const WCHAR table[] = {
      0x1100,  /* jongseong kiyeok        -> choseong kiyeok       */
      0x1101,  /* jongseong ssangkiyeok   -> choseong ssangkiyeok  */
      0x1109,  /* jongseong kiyeok-sios   -> choseong sios         */
      0x1102,  /* jongseong nieun         -> choseong nieun        */
      0x110c,  /* jongseong nieun-cieuc   -> choseong cieuc        */
      0x1112,  /* jongseong nieun-hieuh   -> choseong hieuh        */
      0x1103,  /* jongseong tikeut        -> choseong tikeut       */
      0x1105,  /* jongseong rieul         -> choseong rieul        */
      0x1100,  /* jongseong rieul-kiyeok  -> choseong kiyeok       */
      0x1106,  /* jongseong rieul-mieum   -> choseong mieum        */
      0x1107,  /* jongseong rieul-pieup   -> choseong pieup        */
      0x1109,  /* jongseong rieul-sios    -> choseong sios         */
      0x1110,  /* jongseong rieul-thieuth -> choseong thieuth      */
      0x1111,  /* jongseong rieul-phieuph -> choseong phieuph      */
      0x1112,  /* jongseong rieul-hieuh   -> choseong hieuh        */
      0x1106,  /* jongseong mieum         -> choseong mieum        */
      0x1107,  /* jongseong pieup         -> choseong pieup        */
      0x1109,  /* jongseong pieup-sios    -> choseong sios         */
      0x1109,  /* jongseong sios          -> choseong sios         */
      0x110a,  /* jongseong ssangsios     -> choseong ssangsios    */
      0x110b,  /* jongseong ieung         -> choseong ieung        */
      0x110c,  /* jongseong cieuc         -> choseong cieuc        */
      0x110e,  /* jongseong chieuch       -> choseong chieuch      */
      0x110f,  /* jongseong khieukh       -> choseong khieukh      */
      0x1110,  /* jongseong thieuth       -> choseong thieuth      */
      0x1111,  /* jongseong phieuph       -> choseong phieuph      */
      0x1112   /* jongseong hieuh         -> choseong hieuh        */
    };
    if (ch < 0x11a8 || ch > 0x11c2)
	return 0;
    return table[ch - 0x11a8];
}

void PASCAL
hangul_jongseong_dicompose(WCHAR ch, WCHAR* jong, WCHAR* cho)
{
static const WCHAR table[][2] = {
    { 0,      0x1100 }, /* jong kiyeok	      = cho  kiyeok               */
    { 0x11a8, 0x1100 }, /* jong ssangkiyeok   = jong kiyeok + cho kiyeok  */
    { 0x11a8, 0x1109 }, /* jong kiyeok-sios   = jong kiyeok + cho sios    */
    { 0,      0x1102 }, /* jong nieun	      = cho  nieun                */
    { 0x11ab, 0x110c }, /* jong nieun-cieuc   = jong nieun  + cho cieuc   */
    { 0x11ab, 0x1112 }, /* jong nieun-hieuh   = jong nieun  + cho hieuh   */
    { 0,      0x1103 }, /* jong tikeut	      = cho  tikeut               */
    { 0,      0x1105 }, /* jong rieul         = cho  rieul                */
    { 0x11af, 0x1100 }, /* jong rieul-kiyeok  = jong rieul  + cho kiyeok  */
    { 0x11af, 0x1106 }, /* jong rieul-mieum   = jong rieul  + cho mieum   */
    { 0x11af, 0x1107 }, /* jong rieul-pieup   = jong rieul  + cho pieup   */
    { 0x11af, 0x1109 }, /* jong rieul-sios    = jong rieul  + cho sios    */
    { 0x11af, 0x1110 }, /* jong rieul-thieuth = jong rieul  + cho thieuth */
    { 0x11af, 0x1111 }, /* jong rieul-phieuph = jong rieul  + cho phieuph */
    { 0x11af, 0x1112 }, /* jong rieul-hieuh   = jong rieul  + cho hieuh   */
    { 0,      0x1106 }, /* jong mieum         = cho  mieum                */
    { 0,      0x1107 }, /* jong pieup         = cho  pieup                */
    { 0x11b8, 0x1109 }, /* jong pieup-sios    = jong pieup  + cho sios    */
    { 0,      0x1109 }, /* jong sios          = cho  sios                 */
    { 0x11ba, 0x1109 }, /* jong ssangsios     = jong sios   + cho sios    */
    { 0,      0x110b }, /* jong ieung         = cho  ieung                */
    { 0,      0x110c }, /* jong cieuc         = cho  cieuc                */
    { 0,      0x110e }, /* jong chieuch       = cho  chieuch              */
    { 0,      0x110f }, /* jong khieukh       = cho  khieukh              */
    { 0,      0x1110 }, /* jong thieuth       = cho  thieuth              */
    { 0,      0x1111 }, /* jong phieuph       = cho  phieuph              */
    { 0,      0x1112 }  /* jong hieuh         = cho  hieuh                */
};
    *jong = table[ch - 0x11a8][0];
    *cho  = table[ch - 0x11a8][1];
}

void PASCAL
hangul_jungseong_dicompose(WCHAR ch, WCHAR* jung, WCHAR* jung2)
{
static const WCHAR table[][2] = {
  { 0x1163, 0x1175 }, /* jungseong ya     + i	= yae		*/
  { 0x1165, 0x0000 }, /* */
  { 0x1166, 0x0000 }, /* */
  { 0x1167, 0x0000 }, /* */
  { 0x1167, 0x1175 }, /* jungseong yeo    + i	= ye		*/
  { 0x1169, 0x0000 }, /* */
  { 0x1169, 0x1161 }, /* jungseong o      + a	= wa		*/
  { 0x1169, 0x1162 }, /* jungseong o      + ae	= wae		*/
  { 0x1169, 0x1175 }, /* jungseong o      + i	= oe		*/
  { 0x116d, 0x0000 }, /* */
  { 0x116e, 0x0000 }, /* */
  { 0x116e, 0x1165 }, /* jungseong u      + eo	= weo		*/
  { 0x116e, 0x1166 }, /* jungseong u      + e	= we		*/
  { 0x116e, 0x1175 }, /* jungseong u      + i	= wi		*/
  { 0x1172, 0x0000 }, /* */
  { 0x1173, 0x0000 }, /* */
  { 0x1173, 0x1175 }, /* jungseong eu     + i	= yi		*/
};
  *jung  = 0;
  *jung2 = 0;

  if ( ch < 0x1164 || ch > 0x1174 )
     return;
  *jung  = table[ch - 0x1164][0];
  *jung2 = table[ch - 0x1164][1];
}

WCHAR PASCAL
hangul_jamo_to_syllable(WCHAR choseong, WCHAR jungseong, WCHAR jongseong)
{
    static const WCHAR hangul_base    = 0xac00;
    static const WCHAR choseong_base  = 0x1100;
    static const WCHAR jungseong_base = 0x1161;
    static const WCHAR jongseong_base = 0x11a7;
    static const int njungseong = 21;
    static const int njongseong = 28;
    WCHAR ch;

    /* we use 0x11a7 like a Jongseong filler */
    if (jongseong == 0)
	jongseong = 0x11a7;         /* Jongseong filler */

    if (!(choseong  >= 0x1100 && choseong  <= 0x1112))
	return 0;
    if (!(jungseong >= 0x1161 && jungseong <= 0x1175))
	return 0;
    if (!(jongseong >= 0x11a7 && jongseong <= 0x11c2))
	return 0;

    choseong  -= choseong_base;
    jungseong -= jungseong_base;
    jongseong -= jongseong_base;

    ch = ((choseong * njungseong) + jungseong) * njongseong + jongseong
	+ hangul_base;

    if (dwOptionFlag & KSX1001_SUPPORT)
    {
        WORD mb;
        WideCharToMultiByte(949, WC_COMPOSITECHECK,
                        &ch, 1, (char *)&mb, 2, NULL, NULL);

        if(LOBYTE(mb) < 0xa1 || LOBYTE(mb) > 0xfe 
        || HIBYTE(mb) < 0xa1 || HIBYTE(mb) > 0xfe)
            return 0;
    }
    return ch;
}

WCHAR PASCAL
hangul_jamo_to_cjamo(WCHAR jamo)
{
    if (jamo >= 0x1100 && jamo <= 0x1112)
        return hangul_choseong_to_cjamo(jamo);
    if (jamo >= 0x1161 && jamo <= 0x1175)
        return hangul_jungseong_to_cjamo(jamo);
    if (jamo >= 0x11a7 && jamo <= 0x11c2)
        return hangul_jongseong_to_cjamo(jamo);
    // else
    return jamo;
}

// dvorak 2 qwerty mapping for the kbddv.dll driver.
static const WCHAR dvorak2qwerty_table[] = {
  0x0021,    /* GDK_exclam */
  0x0051,    /* GDK_Q */
  0x0023,    /* GDK_numbersign */
  0x0024,    /* GDK_dollar */
  0x0025,    /* GDK_percent */
  0x0026,    /* GDK_ampersand */
  0x0071,    /* GDK_q */
  0x0028,    /* GDK_parenleft */
  0x0029,    /* GDK_parenright */
  0x002a,    /* GDK_KP_Multiply */
  0x007d,    /* GDK_braceright */
  0x0077,    /* GDK_w */
  0x0027,    /* GDK_quoteright */
  0x0065,    /* GDK_e */
  0x005b,    /* GDK_bracketleft */
  0x0030,    /* GDK_KP_0 */
  0x0031,    /* GDK_KP_1 */
  0x0032,    /* GDK_KP_2 */
  0x0033,    /* GDK_KP_3 */
  0x0034,    /* GDK_KP_4 */
  0x0035,    /* GDK_KP_5 */
  0x0036,    /* GDK_KP_6 */
  0x0037,    /* GDK_KP_7 */
  0x0038,    /* GDK_KP_8 */
  0x0039,    /* GDK_KP_9 */
  0x005a,    /* GDK_Z */
  0x007a,    /* GDK_z */
  0x0057,    /* GDK_W */
  0x005d,    /* GDK_bracketright */
  0x0045,    /* GDK_E */
  0x007b,    /* GDK_braceleft */
  0x0040,    /* GDK_at */
  0x0041,    /* GDK_A */
  0x004e,    /* GDK_N */
  0x0049,    /* GDK_I */
  0x0048,    /* GDK_H */
  0x0044,    /* GDK_D */
  0x0059,    /* GDK_Y */
  0x0055,    /* GDK_U */
  0x004a,    /* GDK_J */
  0x0047,    /* GDK_G */
  0x0043,    /* GDK_C */
  0x0056,    /* GDK_V */
  0x0050,    /* GDK_P */
  0x004d,    /* GDK_M */
  0x004c,    /* GDK_L */
  0x0053,    /* GDK_S */
  0x0052,    /* GDK_R */
  0x0058,    /* GDK_X */
  0x004f,    /* GDK_O */
  0x003a,    /* GDK_colon */
  0x004b,    /* GDK_K */
  0x0046,    /* GDK_F */
  0x003e,    /* GDK_rightcaret */
  0x003c,    /* GDK_leftcaret */
  0x0042,    /* GDK_B */
  0x0054,    /* GDK_T */
  0x003f,    /* GDK_question */
  0x002d,    /* GDK_KP_Subtract */
  0x005c,    /* GDK_backslash */
  0x003d,    /* GDK_KP_Equal */
  0x005e,    /* GDK_asciicircum */
  0x0022,    /* GDK_quotedbl */
  0x0060,    /* GDK_quoteleft */
  0x0061,    /* GDK_a */
  0x006e,    /* GDK_n */
  0x0069,    /* GDK_i */
  0x0068,    /* GDK_h */
  0x0064,    /* GDK_d */
  0x0079,    /* GDK_y */
  0x0075,    /* GDK_u */
  0x006a,    /* GDK_j */
  0x0067,    /* GDK_g */
  0x0063,    /* GDK_c */
  0x0076,    /* GDK_v */
  0x0070,    /* GDK_p */
  0x006d,    /* GDK_m */
  0x006c,    /* GDK_l */
  0x0073,    /* GDK_s */
  0x0072,    /* GDK_r */
  0x0078,    /* GDK_x */
  0x006f,    /* GDK_o */
  0x003b,    /* GDK_semicolon */
  0x006b,    /* GDK_k */
  0x0066,    /* GDK_f */
  0x002e,    /* GDK_decimalpoint */
  0x002c,    /* GDK_KP_Separator */
  0x0062,    /* GDK_b */
  0x0074,    /* GDK_t */
  0x002f,    /* GDK_KP_Divide */
  0x005f,    /* GDK_underbar */
  0x007c,    /* GDK_bar */
  0x002b,    /* GDK_KP_Add */
  0x007e,    /* GDK_asciitilde */
};

// imported from the imhangul 0.9.7
// s/gunichar/WCHAR/g

/* Hangul keyboard map table:
 * 2set keyboard (dubulsik) */
static const WCHAR keyboard_table_2[] = {
  0x0021,	/* GDK_exclam:		exclamation mark	*/
  0x0022,	/* GDK_quotedbl:	quotation mark 		*/
  0x0023,	/* GDK_numbersign:	number sign	 	*/
  0x0024,	/* GDK_dollar:		dollar sign		*/
  0x0025,	/* GDK_percent:		percent sign		*/
  0x0026,	/* GDK_ampersand:	ampersand		*/
  0x0027,	/* GDK_apostrophe:	apostrophe		*/
  0x0028,	/* GDK_parenleft:	left parenthesis	*/
  0x0029,	/* GDK_parenright:	right parenthesis	*/
  0x002a,	/* GDK_asterisk:	asterisk		*/
  0x002b,	/* GDK_plus:		plus sign		*/
  0x002c,	/* GDK_comma:		comma			*/
  0x002d,	/* GDK_minus:		minus sign		*/
  0x002e,	/* GDK_period:		period			*/
  0x002f,	/* GDK_slash:		slash			*/
  0x0030,	/* GDK_0:		0			*/
  0x0031,	/* GDK_1:		1			*/
  0x0032,	/* GDK_2:		2			*/
  0x0033,	/* GDK_3:		3			*/
  0x0034,	/* GDK_4:		4			*/
  0x0035,	/* GDK_5:		5			*/
  0x0036,	/* GDK_6:		6			*/
  0x0037,	/* GDK_7:		7			*/
  0x0038,	/* GDK_8:		8			*/
  0x0039,	/* GDK_9:		9			*/
  0x003a,	/* GDK_colon:		colon			*/
  0x003b,	/* GDK_semicolon:	semicolon		*/
  0x003c,	/* GDK_less:		less-than sign		*/
  0x003d,	/* GDK_equal:		equals sign		*/
  0x003e,	/* GDK_greater:		greater-than sign	*/
  0x003f,	/* GDK_question:	question mark	 	*/
  0x0040,	/* GDK_at:		commercial at		*/
  0x1106,	/* GDK_A:		choseong mieum		*/
  0x1172,	/* GDK_B:		jungseong yu		*/
  0x110e,	/* GDK_C:		choseong chieuch	*/
  0x110b,	/* GDK_D:		choseong ieung		*/
  0x1104,	/* GDK_E:		choseong ssangtikeut	*/
  0x1105,	/* GDK_F:		choseong rieul		*/
  0x1112,	/* GDK_G:		choseong hieuh		*/
  0x1169,	/* GDK_H:		jungseong o		*/
  0x1163,	/* GDK_I:		jungseong ya		*/
  0x1165,	/* GDK_J:		jungseong eo		*/
  0x1161,	/* GDK_K:		jungseong a		*/
  0x1175,	/* GDK_L:		jungseong i		*/
  0x1173,	/* GDK_M:		jungseong eu		*/
  0x116e,	/* GDK_N:		jungseong u		*/
  0x1164,	/* GDK_O:		jungseong yae		*/
  0x1168,	/* GDK_P:		jungseong ye		*/
  0x1108,	/* GDK_Q:		choseong ssangpieup	*/
  0x1101,	/* GDK_R:		choseong ssangkiyeok	*/
  0x1102,	/* GDK_S:		choseong nieun		*/
  0x110a,	/* GDK_T:		choseong ssangsios	*/
  0x1167,	/* GDK_U:		jungseong yeo		*/
  0x1111,	/* GDK_V:		choseong phieuph	*/
  0x110d,	/* GDK_W:		choseong ssangcieuc	*/
  0x1110,	/* GDK_X:		choseong thieuth	*/
  0x116d,	/* GDK_Y:		jungseong yo		*/
  0x110f,	/* GDK_Z:		choseong khieukh	*/
  0x005b,	/* GDK_bracketleft:	left bracket		*/
  0x005c,	/* GDK_backslash:	backslash		*/
  0x005d,	/* GDK_bracketright:	right bracket		*/
  0x005e,	/* GDK_asciicircum:	circumflex accent	*/
  0x005f,	/* GDK_underscore:	underscore		*/
  0x0060,	/* GDK_quoteleft:	grave accent		*/
  0x1106,	/* GDK_a:		choseong  mieum		*/
  0x1172,	/* GDK_b:		jungseong yu		*/
  0x110e,	/* GDK_c:		choseong  chieuch	*/
  0x110b,	/* GDK_d:		choseong  ieung		*/
  0x1103,	/* GDK_e:		choseong  tikeut	*/
  0x1105,	/* GDK_f:		choseong  rieul		*/
  0x1112,	/* GDK_g:		choseong  hieuh		*/
  0x1169,	/* GDK_h:		jungseong o		*/
  0x1163,	/* GDK_i:		jungseong ya		*/
  0x1165,	/* GDK_j:		jungseong eo		*/
  0x1161,	/* GDK_k:		jungseong a		*/
  0x1175,	/* GDK_l:		jungseong i		*/
  0x1173,	/* GDK_m:		jungseong eu		*/
  0x116e,	/* GDK_n:		jungseong u		*/
  0x1162,	/* GDK_o:		jungseong ae		*/
  0x1166,	/* GDK_p:		jungseong e		*/
  0x1107,	/* GDK_q:		choseong  pieup		*/
  0x1100,	/* GDK_r:		choseong  kiyeok	*/
  0x1102,	/* GDK_s:		choseong  nieun		*/
  0x1109, 	/* GDK_t:		choseong  sios		*/
  0x1167,	/* GDK_u:		jungseong yeo		*/
  0x1111,	/* GDK_v:		choseong  phieuph	*/
  0x110c,	/* GDK_w:		choseong  cieuc		*/
  0x1110,	/* GDK_x:		choseong  thieuth	*/
  0x116d,	/* GDK_y:		jungseong yo		*/
  0x110f,	/* GDK_z:		choseong  khieukh	*/
  0x007b,	/* GDK_braceleft:	left brace		*/
  0x007c,	/* GDK_bar:		vertical bar		*/
  0x007d,	/* GDK_braceright:	right brace		*/
  0x007e,	/* GDK_asciitilde:	tilde			*/
};

/* Hangul keyboard map table:
 * 3set keyboard 2bul shifted (sebulsik dubulsik badchim) */
static const WCHAR keyboard_table_32[] = {
  0x0021,	/* GDK_exclam:		exclamation mark		*/
  0x0022,	/* GDK_quotedbl:	quotation mark 			*/
  0x0023,	/* GDK_numbersign:	number sign	 		*/
  0x0024,	/* GDK_dollar:		dollar sign			*/
  0x0025,	/* GDK_percent:		percent sign			*/
  0x0026,	/* GDK_ampersand:	ampersand		 	*/
  0x0027,	/* GDK_apostrophe:	apostrophe			*/
  0x0028,	/* GDK_parenleft:	left parenthesis		*/
  0x0029,	/* GDK_parenright:	right parenthesis		*/
  0x002a,	/* GDK_asterisk:	asterisk			*/
  0x002b,	/* GDK_plus:		plus sign			*/
  0x002c,	/* GDK_comma:		comma				*/
  0x002d,	/* GDK_minus:		minus sign			*/
  0x002e,	/* GDK_period:		period				*/
  0x002f,	/* GDK_slash:		slash				*/
  0x0030,	/* GDK_0:		0				*/
  0x0031,	/* GDK_1:		1				*/
  0x0032,	/* GDK_2:		2				*/
  0x0033,	/* GDK_3:		3				*/
  0x0034,	/* GDK_4:		4				*/
  0x0035,	/* GDK_5:		5				*/
  0x0036,	/* GDK_6:		6				*/
  0x0037,	/* GDK_7:		7				*/
  0x0038,	/* GDK_8:		8				*/
  0x0039,	/* GDK_9:		9				*/
  0x003a,	/* GDK_colon:		colon				*/
  0x003b,	/* GDK_semicolon:	semicolon			*/
  0x003c,	/* GDK_less:		less-than sign			*/
  0x003d,	/* GDK_equal:		equals sign			*/
  0x003e,	/* GDK_greater:		greater-than sign		*/
  0x003f,	/* GDK_question:	question mark	 		*/
  0x0040,	/* GDK_at:		commercial at			*/
  0x11b7,	/* GDK_A:		jongseong mieum			*/
  0x1172,	/* GDK_B:		jungseong yu			*/
  0x11be,	/* GDK_C:		jongseong chieuch		*/
  0x11bc,	/* GDK_D:		jongseong ieung			*/
  0x11ae,	/* GDK_E:		jongseong tikeut		*/
  0x11af,	/* GDK_F:		jongseong rieul			*/
  0x11c2,	/* GDK_G:		jongseong hieuh			*/
  0x1169,	/* GDK_H:		jungseong o			*/
  0x1163,	/* GDK_I:		jungseong ya			*/
  0x1165,	/* GDK_J:		jungseong eo			*/
  0x1161,	/* GDK_K:		jungseong a			*/
  0x1175,	/* GDK_L:		jungseong i			*/
  0x1173,	/* GDK_M:		jungseong eu			*/
  0x116e,	/* GDK_N:		jungseong u			*/
  0x1164,	/* GDK_O:		jungseong yae			*/
  0x1168,	/* GDK_P:		jungseong ye			*/
  0x11b8,	/* GDK_Q:		jongseong pieup			*/
  0x11a8,	/* GDK_R:		jongseong kiyeok		*/
  0x11ab,	/* GDK_S:		jongseong nieun			*/
  0x11ba,	/* GDK_T:		jongseong sios			*/
  0x1167,	/* GDK_U:		jungseong yeo			*/
  0x11c1,	/* GDK_V:		jongseong phieuph		*/
  0x11bd,	/* GDK_W:		jongseong cieuc			*/
  0x11c0,	/* GDK_X:		jongseong thieuth		*/
  0x116d,	/* GDK_Y:		jungseong yo			*/
  0x11bf,	/* GDK_Z:		jongseong khieukh		*/
  0x005b,	/* GDK_bracketleft:	left bracket			*/
  0x005c,	/* GDK_backslash:	backslash			*/
  0x005d,	/* GDK_bracketright:	right bracket			*/
  0x005e,	/* GDK_asciicircum:	circumflex accent		*/
  0x005f,	/* GDK_underscore:	underscore			*/
  0x0060,	/* GDK_quoteleft:	grave accent			*/
  0x1106,	/* GDK_a:		choseong  mieum			*/
  0x1172,	/* GDK_b:		jungseong yu			*/
  0x110e,	/* GDK_c:		choseong  chieuch		*/
  0x110b,	/* GDK_d:		choseong  ieung			*/
  0x1103,	/* GDK_e:		choseong  tikeut		*/
  0x1105,	/* GDK_f:		choseong  rieul			*/
  0x1112,	/* GDK_g:		choseong  hieuh			*/
  0x1169,	/* GDK_h:		jungseong o			*/
  0x1163,	/* GDK_i:		jungseong ya			*/
  0x1165,	/* GDK_j:		jungseong eo			*/
  0x1161,	/* GDK_k:		jungseong a			*/
  0x1175,	/* GDK_l:		jungseong i			*/
  0x1173,	/* GDK_m:		jungseong eu			*/
  0x116e,	/* GDK_n:		jungseong u			*/
  0x1162,	/* GDK_o:		jungseong ae			*/
  0x1166,	/* GDK_p:		jungseong e			*/
  0x1107,	/* GDK_q:		choseong  pieup			*/
  0x1100,	/* GDK_r:		choseong  kiyeok		*/
  0x1102,	/* GDK_s:		choseong  nieun			*/
  0x1109, 	/* GDK_t:		choseong  sios			*/
  0x1167,	/* GDK_u:		jungseong yeo			*/
  0x1111,	/* GDK_v:		choseong  phieuph		*/
  0x110c,	/* GDK_w:		choseong  cieuc			*/
  0x1110,	/* GDK_x:		choseong  thieuth		*/
  0x116d,	/* GDK_y:		jungseong yo			*/
  0x110f,	/* GDK_z:		choseong  khieukh		*/
  0x007b,	/* GDK_braceleft:	left brace			*/
  0x007c,	/* GDK_bar:		vertical bar			*/
  0x007d,	/* GDK_braceright:	right brace			*/
  0x007e,	/* GDK_asciitilde:	tilde				*/
};

/* Hangul keyboard map table:
 * 3set keyboard 390 (sebulsik 390) */
static const WCHAR keyboard_table_390[] = {
  0x11bd,	/* GDK_exclam:		jongseong cieuc			*/
  0x0022,	/* GDK_quotedbl:	quotatioin mark			*/
  0x0023,	/* GDK_numbersign:	number sign	 		*/
  0x0024,	/* GDK_dollar:		dollar sign			*/
  0x0025,	/* GDK_percent:		percent sign			*/
  0x0026,	/* GDK_ampersand:	ampersand		 	*/
  0x1110,	/* GDK_apostrophe:	choseong thieuth		*/
  0x0028,	/* GDK_parenleft:	left parenthesis		*/
  0x0029,	/* GDK_parenright:	right parenthesis		*/
  0x002a,	/* GDK_asterisk:	asterisk			*/
  0x002b,	/* GDK_plus:		plus sign			*/
  0x002c,	/* GDK_comma:		comma				*/
  0x002d,	/* GDK_minus:		minus sign			*/
  0x002e,	/* GDK_period:		period				*/
  0x1169,	/* GDK_slash:		jungseong o			*/
  0x110f,	/* GDK_0:		choseong  khieukh		*/
  0x11c2,	/* GDK_1:		jongseong hieuh			*/
  0x11bb,	/* GDK_2:		jongseong ssangsios		*/
  0x11b8,	/* GDK_3:		jongseong pieup			*/
  0x116d,	/* GDK_4:		jungseong yo			*/
  0x1172,	/* GDK_5:		jungseong yu			*/
  0x1163,	/* GDK_6:		jungseong ya			*/
  0x1168,	/* GDK_7:		jungseong ye			*/
  0x1174,	/* GDK_8:		jungseong yi			*/
  0x116e,	/* GDK_9:		jungseong u			*/
  0x003a,	/* GDK_colon:		colon				*/
  0x1107,	/* GDK_semicolon:	choseong  pieup			*/
  0x0032,	/* GDK_less:		2				*/
  0x003d,	/* GDK_equal:		euals sign			*/
  0x0033,	/* GDK_greater:		3				*/
  0x003f,	/* GDK_question:	question mark	 		*/
  0x0040,	/* GDK_at:		commertial at			*/
  0x11ae,	/* GDK_A:		jongseong tikeut		*/
  0x0021,	/* GDK_B:		exclamation mark		*/
  0x11b1,	/* GDK_C:		jongseong rieul-mieum		*/
  0x11b0,	/* GDK_D:		jongseong rieul-kiyeok		*/
  0x11bf,	/* GDK_E:		jongseong khieukh		*/
  0x11a9,	/* GDK_F:		jongseong ssangkiyeok		*/
  0x002f,	/* GDK_G:		slash				*/
  0x0027,	/* GDK_H:		apostrophe			*/
  0x0038,	/* GDK_I:		8				*/
  0x0034,	/* GDK_J:		4				*/
  0x0035,	/* GDK_K:		5				*/
  0x0036,	/* GDK_L:		6				*/
  0x0031,	/* GDK_M:		1				*/
  0x0030,	/* GDK_N:		0				*/
  0x0039,	/* GDK_O:		9				*/
  0x003e,	/* GDK_P:		greater-than sign		*/
  0x11c1,	/* GDK_Q:		jongseong phieuph		*/
  0x1164,	/* GDK_R:		jungseong yae			*/
  0x11ad,	/* GDK_S:		jongseong nieun-hieuh		*/
  0x003b,	/* GDK_T:		semicolon			*/
  0x0037,	/* GDK_U:		7				*/
  0x11b6,	/* GDK_V:		jongseong rieul-hieuh		*/
  0x11c0,	/* GDK_W:		jongseong thikeuth		*/
  0x11b9,	/* GDK_X:		jongseong pieup-sios		*/
  0x003c,	/* GDK_Y:		less-than sign			*/
  0x11be,	/* GDK_Z:		jongseong chieuch		*/
  0x005b,	/* GDK_bracketleft:	left bracket			*/
  0x005c,	/* GDK_backslash:	backslash			*/
  0x005d,	/* GDK_bracketright:	right bracket			*/
  0x005e,	/* GDK_asciicircum:	circumflex accent		*/
  0x005f,	/* GDK_underscore:	underscore			*/
  0x0060,	/* GDK_quoteleft:	grave accent			*/
  0x11bc,	/* GDK_a:		jongseong ieung			*/
  0x116e,	/* GDK_b:		jungseong u			*/
  0x1166,	/* GDK_c:		jungseong e			*/
  0x1175,	/* GDK_d:		jungseong i			*/
  0x1167,	/* GDK_e:		jungseong yeo			*/
  0x1161,	/* GDK_f:		jungseong a			*/
  0x1173,	/* GDK_g:		jungseong eu			*/
  0x1102,	/* GDK_h:		choseong  nieun			*/
  0x1106,	/* GDK_i:		choseong  mieum			*/
  0x110b,	/* GDK_j:		choseong  ieung			*/
  0x1100,	/* GDK_k:		choseong  kiyeok		*/
  0x110c,	/* GDK_l:		choseong  cieuc			*/
  0x1112,	/* GDK_m:		choseong  hieuh			*/
  0x1109,	/* GDK_n:		choseong  sios			*/
  0x110e,	/* GDK_o:		choseong  chieuch		*/
  0x1111,	/* GDK_p:		choseong  phieuph		*/
  0x11ba,	/* GDK_q:		jongseong sios			*/
  0x1162,	/* GDK_r:		jungseong ae			*/
  0x11ab,	/* GDK_s:		jongseong nieun			*/
  0x1165, 	/* GDK_t:		jungseong eo			*/
  0x1103,	/* GDK_u:		choseong  tikeut		*/
  0x1169,	/* GDK_v:		jungseong o			*/
  0x11af,	/* GDK_w:		jongseong rieul			*/
  0x11a8,	/* GDK_x:		jongseong kiyeok		*/
  0x1105,	/* GDK_y:		choseong  rieul			*/
  0x11b7,	/* GDK_z:		jongseong mieum			*/
  0x007b,	/* GDK_braceleft:	left brace			*/
  0x007c,	/* GDK_bar:		vertical line(bar)		*/
  0x007d,	/* GDK_braceright:	right brace			*/
  0x007e,	/* GDK_asciitilde:	tilde				*/
};

/* Hangul keyboard map table:
 * 3set keyboard final (sebulsik choejong) */
static const WCHAR keyboard_table_3final[] = {
  0x11a9,	/* GDK_exclam:		jongseong ssangkiyeok		*/
  0x00b7,	/* GDK_quotedbl:	middle dot 			*/
  0x11bd,	/* GDK_numbersign:	jognseong cieuc 		*/
  0x11b5,	/* GDK_dollar:		jongseong rieul-phieuph		*/
  0x11b4,	/* GDK_percent:		jongseong rieul-thieuth		*/
  0x201c,	/* GDK_ampersand:	left double quotation mark 	*/
  0x1110,	/* GDK_apostrophe:	choseong  thieuth		*/
  0x0027,	/* GDK_parenleft:	apostrophe			*/
  0x007e,	/* GDK_parenright:	Tilde				*/
  0x201d,	/* GDK_asterisk:	right double quotation mark	*/
  0x002b,	/* GDK_plus:		plus sign			*/
  0x002c,	/* GDK_comma:		comma				*/
  0x0029,	/* GDK_minus:		right parenthesis		*/
  0x002e,	/* GDK_period:		period				*/
  0x1169,	/* GDK_slash:		jungseong o			*/
  0x110f,	/* GDK_0:		choseong  khieukh		*/
  0x11c2,	/* GDK_1:		jongseong hieuh			*/
  0x11bb,	/* GDK_2:		jongseong ssangsios		*/
  0x11b8,	/* GDK_3:		jongseong pieup			*/
  0x116d,	/* GDK_4:		jungseong yo			*/
  0x1172,	/* GDK_5:		jungseong yu			*/
  0x1163,	/* GDK_6:		jungseong ya			*/
  0x1168,	/* GDK_7:		jungseong ye			*/
  0x1174,	/* GDK_8:		jungseong yi			*/
  0x116e,	/* GDK_9:		jungseong u			*/
  0x0034,	/* GDK_colon:		4				*/
  0x1107,	/* GDK_semicolon:	choseong  pieup			*/
  0x002c,	/* GDK_less:		comma				*/
  0x003e,	/* GDK_equal:		greater-than sign		*/
  0x002e,	/* GDK_greater:		period				*/
  0x0021,	/* GDK_question:	exclamation mark 		*/
  0x11b0,	/* GDK_at:		jongseong rieul-kiyeok		*/
  0x11ae,	/* GDK_A:		jongseong tikeut		*/
  0x003f,	/* GDK_B:		question mark			*/
  0x11bf,	/* GDK_C:		jongseong khieukh		*/
  0x11b2,	/* GDK_D:		jongseong rieul-pieup		*/
  0x11ac,	/* GDK_E:		jongseong nieun-cieuc		*/
  0x11b1,	/* GDK_F:		jongseong rieul-mieum		*/
  0x1164,	/* GDK_G:		jungseong yae			*/
  0x0030,	/* GDK_H:		0				*/
  0x0037,	/* GDK_I:		7				*/
  0x0031,	/* GDK_J:		1				*/
  0x0032,	/* GDK_K:		2				*/
  0x0033,	/* GDK_L:		3				*/
  0x0022,	/* GDK_M:		double quotation mark		*/
  0x002d,	/* GDK_N:		minus sign			*/
  0x0038,	/* GDK_O:		8				*/
  0x0039,	/* GDK_P:		9				*/
  0x11c1,	/* GDK_Q:		jongseong phieuph		*/
  0x11b6,	/* GDK_R:		jongseong rieul-hieuh		*/
  0x11ad,	/* GDK_S:		jongseong nieun-hieuh		*/
  0x11b3,	/* GDK_T:		jongseong rieul-sios		*/
  0x0036,	/* GDK_U:		6				*/
  0x11aa,	/* GDK_V:		jongseong kiyeok-sios		*/
  0x11c0,	/* GDK_W:		jongseong thikeuth		*/
  0x11b9,	/* GDK_X:		jongseong pieup-sios		*/
  0x0035,	/* GDK_Y:		5				*/
  0x11be,	/* GDK_Z:		jongseong chieuch		*/
  0x0028,	/* GDK_bracketleft:	left parenthesis		*/
  0x003a,	/* GDK_backslash:	colon				*/
  0x003c,	/* GDK_bracketright:	less-than sign			*/
  0x003d,	/* GDK_asciicircum:	equals sign			*/
  0x003b,	/* GDK_underscore:	semicolon			*/
  0x002a,	/* GDK_quoteleft:	asterisk			*/
  0x11bc,	/* GDK_a:		jongseong ieung			*/
  0x116e,	/* GDK_b:		jungseong u			*/
  0x1166,	/* GDK_c:		jungseong e			*/
  0x1175,	/* GDK_d:		jungseong i			*/
  0x1167,	/* GDK_e:		jungseong yeo			*/
  0x1161,	/* GDK_f:		jungseong a			*/
  0x1173,	/* GDK_g:		jungseong eu			*/
  0x1102,	/* GDK_h:		choseong  nieun			*/
  0x1106,	/* GDK_i:		choseong  mieum			*/
  0x110b,	/* GDK_j:		choseong  ieung			*/
  0x1100,	/* GDK_k:		choseong  kiyeok		*/
  0x110c,	/* GDK_l:		choseong  cieuc			*/
  0x1112,	/* GDK_m:		choseong  hieuh			*/
  0x1109,	/* GDK_n:		choseong  sios			*/
  0x110e,	/* GDK_o:		choseong  chieuch		*/
  0x1111,	/* GDK_p:		choseong  phieuph		*/
  0x11ba,	/* GDK_q:		jongseong sios			*/
  0x1162,	/* GDK_r:		jungseong ae			*/
  0x11ab,	/* GDK_s:		jongseong nieun			*/
  0x1165, 	/* GDK_t:		jungseong eo			*/
  0x1103,	/* GDK_u:		choseong  tikeut		*/
  0x1169,	/* GDK_v:		jungseong o			*/
  0x11af,	/* GDK_w:		jongseong rieul			*/
  0x11a8,	/* GDK_x:		jongseong kiyeok		*/
  0x1105,	/* GDK_y:		choseong  rieul			*/
  0x11b7,	/* GDK_z:		jongseong mieum			*/
  0x0025,	/* GDK_braceleft:	percent sign			*/
  0x005c,	/* GDK_bar:		backslash			*/
  0x002f,	/* GDK_braceright:	slash				*/
  0x203b,	/* GDK_asciitilde:	reference mark			*/
};

/* Hangul keyboard map table:
 * 3set keyboard no-shift (sebulsik sunarae) */
static const WCHAR keyboard_table_3sun[] = {
  0x0021,	/* GDK_exclam:		exclamation mark		*/
  0x0022,	/* GDK_quotedbl:	quotatioin mark			*/
  0x0023,	/* GDK_numbersign:	number sign	 		*/
  0x0024,	/* GDK_dollar:		dollar sign			*/
  0x0025,	/* GDK_percent:		percent sign			*/
  0x0026,	/* GDK_ampersand:	ampersand		 	*/
  0x1110,	/* GDK_apostrophe:	choseong thieuth		*/
  0x0028,	/* GDK_parenleft:	left parenthesis		*/
  0x0029,	/* GDK_parenright:	right parenthesis		*/
  0x002a,	/* GDK_asterisk:	asterisk			*/
  0x002b,	/* GDK_plus:		plus sign			*/
  0x002c,	/* GDK_comma:		comma				*/
  0x11bd,	/* GDK_minus:		jongseong cieuc			*/
  0x002e,	/* GDK_period:		period				*/
  0x11ae,	/* GDK_slash:		jongseong tikeut		*/
  0x1164,	/* GDK_0:		choseong  yae			*/
  0x11c2,	/* GDK_1:		jongseong hieuh			*/
  0x11bb,	/* GDK_2:		jongseong ssangsios		*/
  0x11b8,	/* GDK_3:		jongseong pieup			*/
  0x116d,	/* GDK_4:		jungseong yo			*/
  0x1172,	/* GDK_5:		jungseong yu			*/
  0x1163,	/* GDK_6:		jungseong ya			*/
  0x1168,	/* GDK_7:		jungseong ye			*/
  0x1174,	/* GDK_8:		jungseong yi			*/
  0x110f,	/* GDK_9:		choseong khieukh		*/
  0x003a,	/* GDK_colon:		colon				*/
  0x1107,	/* GDK_semicolon:	choseong  pieup			*/
  0x0032,	/* GDK_less:		2				*/
  0x11be,	/* GDK_equal:		jongseong chieuch		*/
  0x0033,	/* GDK_greater:		3				*/
  0x003f,	/* GDK_question:	question mark	 		*/
  0x0040,	/* GDK_at:		commertial at			*/
  0x11bc,	/* GDK_A:		jongseong ieung			*/
  0x0021,	/* GDK_B:		exclamation mark		*/
  0x005c,	/* GDK_C:		backslash			*/
  0x005d,	/* GDK_D:		right bracket			*/
  0x1167,	/* GDK_E:		jungseong yeo			*/
  0x1161,	/* GDK_F:		jungseong a			*/
  0x002f,	/* GDK_G:		slash				*/
  0x0027,	/* GDK_H:		apostrophe			*/
  0x0038,	/* GDK_I:		8				*/
  0x0034,	/* GDK_J:		4				*/
  0x0035,	/* GDK_K:		5				*/
  0x0036,	/* GDK_L:		6				*/
  0x0031,	/* GDK_M:		1				*/
  0x0030,	/* GDK_N:		0				*/
  0x0039,	/* GDK_O:		9				*/
  0x003e,	/* GDK_P:		greater-than sign		*/
  0x11ba,	/* GDK_Q:		jongseong sios			*/
  0x1162,	/* GDK_R:		jungseong ae			*/
  0x005b,	/* GDK_S:		left bracket			*/
  0x003b,	/* GDK_T:		semicolon			*/
  0x0037,	/* GDK_U:		7				*/
  0x1169,	/* GDK_V:		jungseong o			*/
  0x11af,	/* GDK_W:		jongseong rieul			*/
  0x003d,	/* GDK_X:		equals sign			*/
  0x003c,	/* GDK_Y:		less-than sign			*/
  0x002d,	/* GDK_Z:		minus sign			*/
  0x11c0,	/* GDK_bracketleft:	jongseong thieuth		*/
  0x11bf,	/* GDK_backslash:	jongseong khieukh		*/
  0x11c1,	/* GDK_bracketright:	jongseong phieuph		*/
  0x005e,	/* GDK_asciicircum:	circumflex accent		*/
  0x005f,	/* GDK_underscore:	underscore			*/
  0x0060,	/* GDK_quoteleft:	grave accent			*/
  0x11bc,	/* GDK_a:		jongseong ieung			*/
  0x116e,	/* GDK_b:		jungseong u			*/
  0x1166,	/* GDK_c:		jungseong e			*/
  0x1175,	/* GDK_d:		jungseong i			*/
  0x1167,	/* GDK_e:		jungseong yeo			*/
  0x1161,	/* GDK_f:		jungseong a			*/
  0x1173,	/* GDK_g:		jungseong eu			*/
  0x1102,	/* GDK_h:		choseong  nieun			*/
  0x1106,	/* GDK_i:		choseong  mieum			*/
  0x110b,	/* GDK_j:		choseong  ieung			*/
  0x1100,	/* GDK_k:		choseong  kiyeok		*/
  0x110c,	/* GDK_l:		choseong  cieuc			*/
  0x1112,	/* GDK_m:		choseong  hieuh			*/
  0x1109,	/* GDK_n:		choseong  sios			*/
  0x110e,	/* GDK_o:		choseong  chieuch		*/
  0x1111,	/* GDK_p:		choseong  phieuph		*/
  0x11ba,	/* GDK_q:		jongseong sios			*/
  0x1162,	/* GDK_r:		jungseong ae			*/
  0x11ab,	/* GDK_s:		jongseong nieun			*/
  0x1165, 	/* GDK_t:		jungseong eo			*/
  0x1103,	/* GDK_u:		choseong  tikeut		*/
  0x1169,	/* GDK_v:		jungseong o			*/
  0x11af,	/* GDK_w:		jongseong rieul			*/
  0x11a8,	/* GDK_x:		jongseong kiyeok		*/
  0x1105,	/* GDK_y:		choseong  rieul			*/
  0x11b7,	/* GDK_z:		jongseong mieum			*/
  0x007b,	/* GDK_braceleft:	left brace			*/
  0x007c,	/* GDK_bar:		vertical line(bar)		*/
  0x007d,	/* GDK_braceright:	right brace			*/
  0x007e,	/* GDK_asciitilde:	tilde				*/
};

/* Hangul keyboard map table:
 * 3set keyboard yetguel (sebulsik yetguel)
 * see http://asadal.cs.pusan.ac.kr/hangeul/kbd/oldkbd.jpg */
static const WCHAR keyboard_table_3yet[] = {
  0x11bd,	/* GDK_exclam:		jongseong cieuc			*/
  0x0022,	/* GDK_quotedbl:	quotatioin mark			*/
  0x0023,	/* GDK_numbersign:	number sign	 		*/
  0x0024,	/* GDK_dollar:		dollar sign			*/
  0x0025,	/* GDK_percent:		percent sign			*/
  0x0026,	/* GDK_ampersand:	ampersand		 	*/
  0x1110,	/* GDK_apostrophe:	choseong thieuth		*/
  0x0028,	/* GDK_parenleft:	left parenthesis		*/
  0x0029,	/* GDK_parenright:	right parenthesis		*/
  0x002a,	/* GDK_asterisk:	asterisk			*/
  0x002b,	/* GDK_plus:		plus sign			*/
  0x002c,	/* GDK_comma:		comma				*/
  0x002d,	/* GDK_minus:		minus sign			*/
  0x002e,	/* GDK_period:		period				*/
  0x1169,	/* GDK_slash:		jungseong o			*/
  0x110f,	/* GDK_0:		choseong  khieukh		*/
  0x11c2,	/* GDK_1:		jongseong hieuh			*/
  0x11bb,	/* GDK_2:		jongseong ssangsios		*/
  0x11b8,	/* GDK_3:		jongseong pieup			*/
  0x116d,	/* GDK_4:		jungseong yo			*/
  0x1172,	/* GDK_5:		jungseong yu			*/
  0x1163,	/* GDK_6:		jungseong ya			*/
  0x1168,	/* GDK_7:		jungseong ye			*/
  0x1174,	/* GDK_8:		jungseong yi			*/
  0x116e,	/* GDK_9:		jungseong u			*/
  0x003a,	/* GDK_colon:		colon				*/
  0x1107,	/* GDK_semicolon:	choseong  pieup			*/
  0x113c,	/* GDK_less:		choseong chitueumsios		*/
  0x003d,	/* GDK_equal:		euals sign			*/
  0x113e,	/* GDK_greater:		choseong ceongchieumsios	*/
  0x003f,	/* GDK_question:	question mark	 		*/
  0x11eb,	/* GDK_at:		jongseong pansios		*/
  0x11ae,	/* GDK_A:		jongseong tikeut		*/
  0x116e,	/* GDK_B:		jungseong u			*/
  0x11b1,	/* GDK_C:		jongseong rieul-mieum		*/
  0x11b0,	/* GDK_D:		jongseong rieul-kiyeok		*/
  0x11bf,	/* GDK_E:		jongseong khieukh		*/
  0x11a9,	/* GDK_F:		jongseong ssangkiyeok		*/
  0x119e,	/* GDK_G:		jungseong araea			*/
  0x1102,	/* GDK_H:		choseong nieun			*/
  0x1154,	/* GDK_I:		choseong chitueumchieuch	*/
  0x114c,	/* GDK_J:		choseong yesieung		*/
  0x114e,	/* GDK_K:		choseong chitueumcieuc		*/
  0x1150,	/* GDK_L:		choseong ceongchieumcieuc	*/
  0x1159,	/* GDK_M:		choseong yeorinhieuh		*/
  0x1140,	/* GDK_N:		choseong pansios		*/
  0x1155,	/* GDK_O:		choseong ceongchieumchieuch	*/
  0x1111,	/* GDK_P:		choseong phieuph		*/
  0x11c1,	/* GDK_Q:		jongseong phieuph		*/
  0x1164,	/* GDK_R:		jungseong yae			*/
  0x11ad,	/* GDK_S:		jongseong nieun-hieuh		*/
  0x1165,	/* GDK_T:		jungseong eo			*/
  0x302e,	/* GDK_U:		single dot tone mark		*/
  0x11b6,	/* GDK_V:		jongseong rieul-hieuh		*/
  0x11c0,	/* GDK_W:		jongseong thikeuth		*/
  0x11b9,	/* GDK_X:		jongseong pieup-sios		*/
  0x302f,	/* GDK_Y:		double dot tone mark		*/
  0x11be,	/* GDK_Z:		jongseong chieuch		*/
  0x005b,	/* GDK_bracketleft:	left bracket			*/
  0x005c,	/* GDK_backslash:	backslash			*/
  0x005d,	/* GDK_bracketright:	right bracket			*/
  0x005e,	/* GDK_asciicircum:	circumflex accent		*/
  0x005f,	/* GDK_underscore:	underscore			*/
  0x11f9,	/* GDK_quoteleft:	jongseong yeorinhieuh		*/
  0x11bc,	/* GDK_a:		jongseong ieung			*/
  0x116e,	/* GDK_b:		jungseong u			*/
  0x1166,	/* GDK_c:		jungseong e			*/
  0x1175,	/* GDK_d:		jungseong i			*/
  0x1167,	/* GDK_e:		jungseong yeo			*/
  0x1161,	/* GDK_f:		jungseong a			*/
  0x1173,	/* GDK_g:		jungseong eu			*/
  0x1102,	/* GDK_h:		choseong  nieun			*/
  0x1106,	/* GDK_i:		choseong  mieum			*/
  0x110b,	/* GDK_j:		choseong  ieung			*/
  0x1100,	/* GDK_k:		choseong  kiyeok		*/
  0x110c,	/* GDK_l:		choseong  cieuc			*/
  0x1112,	/* GDK_m:		choseong  hieuh			*/
  0x1109,	/* GDK_n:		choseong  sios			*/
  0x110e,	/* GDK_o:		choseong  chieuch		*/
  0x1111,	/* GDK_p:		choseong  phieuph		*/
  0x11ba,	/* GDK_q:		jongseong sios			*/
  0x1162,	/* GDK_r:		jungseong ae			*/
  0x11ab,	/* GDK_s:		jongseong nieun			*/
  0x1165, 	/* GDK_t:		jungseong eo			*/
  0x1103,	/* GDK_u:		choseong  tikeut		*/
  0x1169,	/* GDK_v:		jungseong o			*/
  0x11af,	/* GDK_w:		jongseong rieul			*/
  0x11a8,	/* GDK_x:		jongseong kiyeok		*/
  0x1105,	/* GDK_y:		choseong  rieul			*/
  0x11b7,	/* GDK_z:		jongseong mieum			*/
  0x007b,	/* GDK_braceleft:	left brace			*/
  0x007c,	/* GDK_bar:		vertical line(bar)		*/
  0x007d,	/* GDK_braceright:	right brace			*/
  0x11f0,	/* GDK_asciitilde:	jongseong yesieung		*/
};

/*
struct _HangulCompose 
{
  UINT key;
  WCHAR code;
};

typedef struct _HangulCompose	HangulCompose;
*/

static const HangulCompose compose_table_default[] = {
  { 0x11001100, 0x1101 }, /* choseong  kiyeok + kiyeok	= ssangkiyeok	*/
  { 0x11001109, 0x11a9 }, /* choseong  kiyeok + sios	= kiyeok-sios	*/
  { 0x11031103, 0x1104 }, /* choseong  tikeut + tikeut	= ssangtikeut	*/
  { 0x11071107, 0x1108 }, /* choseong  pieup  + pieup	= ssangpieup	*/
  { 0x11091109, 0x110a }, /* choseong  sios   + sios	= ssangsios	*/
  { 0x110c110c, 0x110d }, /* choseong  cieuc  + cieuc	= ssangcieuc	*/
  { 0x11621162, 0x1164 }, /* jungseong ae      + ae	= yae		*/
  { 0x11631175, 0x1164 }, /* jungseong ya     + i	= yae		*/
  { 0x11661166, 0x1168 }, /* jungseong e      + e	= ye		*/
  { 0x11671175, 0x1168 }, /* jungseong yeo    + i	= ye		*/
  { 0x11691161, 0x116a }, /* jungseong o      + a	= wa		*/
  { 0x11691162, 0x116b }, /* jungseong o      + ae	= wae		*/
  { 0x11691175, 0x116c }, /* jungseong o      + i	= oe		*/
  { 0x116e1165, 0x116f }, /* jungseong u      + eo	= weo		*/
  { 0x116e1166, 0x1170 }, /* jungseong u      + e	= we		*/
  { 0x116e116e, 0x1172 }, /* jungseong u      + u	= yu		*/
  { 0x116e1175, 0x1171 }, /* jungseong u      + i	= wi		*/
  { 0x11731173, 0x1174 }, /* jungseong eu     + eu	= yi		*/
  { 0x11731175, 0x1174 }, /* jungseong eu     + i	= yi		*/
  { 0x11751162, 0x1164 }, /* jungseong i      + ae	= yae		*/
  { 0x11751166, 0x1168 }, /* jungseong i      + e	= ye		*/
  { 0x11751175, 0x1174 }, /* jungseong i      + i	= yi		*/

  { 0x11a811a8, 0x11a9 }, /* jongseong kiyeok + kiyeok	= ssangekiyeok	*/
  { 0x11a811ba, 0x11aa }, /* jongseong kiyeok + sios	= kiyeok-sois	*/
  { 0x11ab11bd, 0x11ac }, /* jongseong nieun  + cieuc	= nieun-cieuc	*/
  { 0x11ab11c2, 0x11ad }, /* jongseong nieun  + hieuh	= nieun-hieuh	*/
  { 0x11af11a8, 0x11b0 }, /* jongseong rieul  + kiyeok	= rieul-kiyeok	*/
  { 0x11af11b7, 0x11b1 }, /* jongseong rieul  + mieum	= rieul-mieum	*/
  { 0x11af11b8, 0x11b2 }, /* jongseong rieul  + pieup	= rieul-pieup	*/
  { 0x11af11ba, 0x11b3 }, /* jongseong rieul  + sios	= rieul-sios	*/
  { 0x11af11c0, 0x11b4 }, /* jongseong rieul  + thieuth = rieul-thieuth	*/
  { 0x11af11c1, 0x11b5 }, /* jongseong rieul  + phieuph = rieul-phieuph	*/
  { 0x11af11c2, 0x11b6 }, /* jongseong rieul  + hieuh	= rieul-hieuh	*/
  { 0x11b811ba, 0x11b9 }, /* jongseong pieup  + sios	= pieup-sios	*/
  { 0x11ba11ba, 0x11bb }, /* jongseong sios   + sios	= ssangsios	*/
};

static const HangulCompose compose_table_2set[] = {
  { 0x11001100, 0x1101 }, /* choseong  kiyeok + kiyeok	= ssangkiyeok	*/
  { 0x11001109, 0x11aa }, /* choseong  kiyeok + sios	= kiyeok-sios	*/
  { 0x1102110c, 0x11ac }, /* choseong  nieun  + cieuc	= nieun-cieuc	*/
  { 0x11021112, 0x11ad }, /* choseong  nieun  + hieuh	= nieun-hieuh	*/
  { 0x11031103, 0x1104 }, /* choseong  tikeut + tikeut	= ssangtikeut	*/
  { 0x11051100, 0x11b0 }, /* choseong  rieul  + kiyeok	= rieul-kiyeok	*/
  { 0x11051106, 0x11b1 }, /* choseong  rieul  + mieum	= rieul-mieum	*/
  { 0x11051107, 0x11b2 }, /* choseong  rieul  + pieup	= rieul-pieup	*/
  { 0x11051109, 0x11b3 }, /* choseong  rieul  + sios	= rieul-sios	*/
  { 0x11051110, 0x11b4 }, /* choseong  rieul  + thieuth = rieul-thieuth	*/
  { 0x11051111, 0x11b5 }, /* choseong  rieul  + phieuph = rieul-phieuph	*/
  { 0x11051112, 0x11b6 }, /* choseong  rieul  + hieuh	= rieul-hieuh	*/
  { 0x11071107, 0x1108 }, /* choseong  pieup  + pieup	= ssangpieup	*/
  { 0x11071109, 0x11b9 }, /* choseong  pieup  + sios	= pieup-sios	*/
  { 0x11091109, 0x110a }, /* choseong  sios   + sios	= ssangsios	*/
  { 0x110c110c, 0x110d }, /* choseong  cieuc  + cieuc	= ssangcieuc	*/
  { 0x11621162, 0x1164 }, /* jungseong ae      + ae	= yae		*/
  { 0x11631175, 0x1164 }, /* jungseong ya     + i	= yae		*/
  { 0x11661166, 0x1168 }, /* jungseong e      + e	= ye		*/
  { 0x11671175, 0x1168 }, /* jungseong yeo    + i	= ye		*/
  { 0x11691161, 0x116a }, /* jungseong o      + a	= wa		*/
  { 0x11691162, 0x116b }, /* jungseong o      + ae	= wae		*/
  { 0x11691175, 0x116c }, /* jungseong o      + i	= oe		*/
  { 0x116e1165, 0x116f }, /* jungseong u      + eo	= weo		*/
  { 0x116e1166, 0x1170 }, /* jungseong u      + e	= we		*/
  { 0x116e116e, 0x1172 }, /* jungseong u      + u	= yu		*/
  { 0x116e1175, 0x1171 }, /* jungseong u      + i	= wi		*/
  { 0x11731173, 0x1174 }, /* jungseong eu     + eu	= yi		*/
  { 0x11731175, 0x1174 }, /* jungseong eu     + i	= yi		*/
  { 0x11751162, 0x1164 }, /* jungseong i      + ae	= yae		*/
  { 0x11751166, 0x1168 }, /* jungseong i      + e	= ye		*/
  { 0x11751175, 0x1174 }, /* jungseong i      + i	= yi		*/

  { 0x11a811a8, 0x11a9 }, /* jongseong kiyeok + kiyeok	= ssangekiyeok	*/
  { 0x11a811ba, 0x11aa }, /* jongseong kiyeok + sios	= kiyeok-sois	*/
  { 0x11ab11bd, 0x11ac }, /* jongseong nieun  + cieuc	= nieun-cieuc	*/
  { 0x11ab11c2, 0x11ad }, /* jongseong nieun  + hieuh	= nieun-hieuh	*/
  { 0x11af11a8, 0x11b0 }, /* jongseong rieul  + kiyeok	= rieul-kiyeok	*/
  { 0x11af11b7, 0x11b1 }, /* jongseong rieul  + mieum	= rieul-mieum	*/
  { 0x11af11b8, 0x11b2 }, /* jongseong rieul  + pieup	= rieul-pieup	*/
  { 0x11af11ba, 0x11b3 }, /* jongseong rieul  + sios	= rieul-sios	*/
  { 0x11af11c0, 0x11b4 }, /* jongseong rieul  + thieuth = rieul-thieuth	*/
  { 0x11af11c1, 0x11b5 }, /* jongseong rieul  + phieuph = rieul-phieuph	*/
  { 0x11af11c2, 0x11b6 }, /* jongseong rieul  + hieuh	= rieul-hieuh	*/
  { 0x11b811ba, 0x11b9 }, /* jongseong pieup  + sios	= pieup-sios	*/
  { 0x11ba11ba, 0x11bb }, /* jongseong sios   + sios	= ssangsios	*/
};

static const HangulCompose compose_table_ahnmatae[] = {
  { 0x11001100, 0x1101 }, /* choseong  kiyeok + kiyeok	= ssangkiyeok	*/
  { 0x1100110b, 0x1101 }, /* choseong  kiyeok + ieung 	= ssangkiyeok	*/
  { 0x11001112, 0x110f }, /* choseong  kiyeok +       	= 	*/

  { 0x11031100, 0x1104 }, /* choseong  tikeut + kiyeok	= ssangtikeut	*/
  { 0x11031103, 0x1104 }, /* choseong  tikeut + tikeut	= ssangtikeut	*/
  { 0x11031112, 0x1110 }, /* choseong  tikeut + kiyeok	= 	*/

  { 0x11071107, 0x1108 }, /* choseong  pieup  + pieup	= ssangpieup	*/
  { 0x1107110c, 0x1108 }, /* choseong  pieup  + xxx  	= ssangpieup	*/
  { 0x11071112, 0x1111 }, /* choseong  pieup  + pieup	= 	*/

  { 0x11091102, 0x110a }, /* choseong  sios   + nieun	= ssangsios	*/
  { 0x11091109, 0x110a }, /* choseong  sios   + sios	= ssangsios	*/
  { 0x110c1103, 0x110d }, /* choseong  cieuc  + xxx  	= ssangcieuc	*/
  { 0x110c110c, 0x110d }, /* choseong  cieuc  + cieuc	= ssangcieuc	*/
  { 0x110c1112, 0x110e }, /* choseong  cieuc  + xxx  	= ssangcieuc	*/

  { 0x11611175, 0x1162 }, /* jungseong a      + i	= ae		*/
  { 0x11631175, 0x1164 }, /* jungseong ya     + i	= yae		*/
  { 0x11651175, 0x1166 }, /* jungseong eo     + i	= e		*/
  { 0x11671175, 0x1168 }, /* jungseong yeo    + i	= ye		*/

  { 0x11691161, 0x116a }, /* jungseong o      + a	= wa		*/
  { 0x11691162, 0x116b }, /* jungseong o      + ae	= wae		*/
  { 0x11691175, 0x116c }, /* jungseong o      + i	= oe		*/

  { 0x116e1165, 0x116f }, /* jungseong u      + eo	= weo		*/
  { 0x116e1166, 0x1170 }, /* jungseong u      + e	= we		*/

  { 0x116e1175, 0x1171 }, /* jungseong u      + i	= wi		*/
  { 0x11731175, 0x1174 }, /* jungseong eu     + i	= yi		*/
  { 0x11a811a8, 0x11a9 }, /* jongseong kiyeok + kiyeok	= ssangekiyeok	*/
  { 0x11a811ba, 0x11aa }, /* jongseong kiyeok + sios	= kiyeok-sois	*/
  { 0x11a811bc, 0x11a9 }, /* jongseong kiyeok + ieung	= ssangekiyeok	*/

  { 0x11ab11bd, 0x11ac }, /* jongseong nieun  + cieuc	= nieun-cieuc	*/
  { 0x11ab11c2, 0x11ad }, /* jongseong nieun  + hieuh	= nieun-hieuh	*/
  { 0x11af11a8, 0x11b0 }, /* jongseong rieul  + kiyeok	= rieul-kiyeok	*/
  { 0x11af11b7, 0x11b1 }, /* jongseong rieul  + mieum	= rieul-mieum	*/
  { 0x11af11b8, 0x11b2 }, /* jongseong rieul  + pieup	= rieul-pieup	*/
  { 0x11af11ba, 0x11b3 }, /* jongseong rieul  + sios	= rieul-sios	*/
  { 0x11af11c0, 0x11b4 }, /* jongseong rieul  + thieuth = rieul-thieuth	*/
  { 0x11af11c1, 0x11b5 }, /* jongseong rieul  + phieuph = rieul-phieuph	*/
  { 0x11af11c2, 0x11b6 }, /* jongseong rieul  + hieuh	= rieul-hieuh	*/
  { 0x11b811ba, 0x11b9 }, /* jongseong pieup  + sios	= pieup-sios	*/
  { 0x11ba11ba, 0x11bb }, /* jongseong sios   + sios	= ssangsios	*/
  { 0x11ba11bd, 0x11bb }, /* jongseong sios   + nieun	= ssangsios	*/
};

static const HangulCompose compose_table_yet[] = {
  { 0x11001100, 0x1101 }, /* CHOSEONG SSANGKIYEOK           = KIYEOK + KIYEOK */
  { 0x11021100, 0x1113 }, /* CHOSEONG NIEUN-KIYEOK          = NIEUN + KIYEOK */
  { 0x11021102, 0x1114 }, /* CHOSEONG SSANGNIEUN            = NIEUN + NIEUN */
  { 0x11021103, 0x1115 }, /* CHOSEONG NIEUN-TIKEUT          = NIEUN + TIKEUT */
  { 0x11021107, 0x1116 }, /* CHOSEONG NIEUN-PIEUP           = NIEUN + PIEUP */
  { 0x11031100, 0x1117 }, /* CHOSEONG TIKEUT-KIYEOK         = TIKEUT + KIYEOK */
  { 0x11031103, 0x1104 }, /* CHOSEONG SSANGTIKEUT           = TIKEUT + TIKEUT */
  { 0x11051102, 0x1118 }, /* CHOSEONG RIEUL-NIEUN           = RIEUL + NIEUN */
  { 0x11051105, 0x1119 }, /* CHOSEONG SSANGRIEUL            = RIEUL + RIEUL */
  { 0x1105110b, 0x111b }, /* CHOSEONG KAPYEOUNRIEUL         = RIEUL + IEUNG */
  { 0x11051112, 0x111a }, /* CHOSEONG RIEUL-HIEUH           = RIEUL + HIEUH */
  { 0x11061107, 0x111c }, /* CHOSEONG MIEUM-PIEUP           = MIEUM + PIEUP */
  { 0x1106110b, 0x111d }, /* CHOSEONG KAPYEOUNMIEUM         = MIEUM + IEUNG */
  { 0x11071100, 0x111e }, /* CHOSEONG PIEUP-KIYEOK          = PIEUP + KIYEOK */
  { 0x11071102, 0x111f }, /* CHOSEONG PIEUP-NIEUN           = PIEUP + NIEUN */
  { 0x11071103, 0x1120 }, /* CHOSEONG PIEUP-TIKEUT          = PIEUP + TIKEUT */
  { 0x11071107, 0x1108 }, /* CHOSEONG SSANGPIEUP            = PIEUP + PIEUP */
  { 0x11071109, 0x1121 }, /* CHOSEONG PIEUP-SIOS            = PIEUP + SIOS */
  { 0x1107110b, 0x112b }, /* CHOSEONG KAPYEOUNPIEUP         = PIEUP + IEUNG */
  { 0x1107110c, 0x1127 }, /* CHOSEONG PIEUP-CIEUC           = PIEUP + CIEUC */
  { 0x1107110e, 0x1128 }, /* CHOSEONG PIEUP-CHIEUCH         = PIEUP + CHIEUCH */
  { 0x11071110, 0x1129 }, /* CHOSEONG PIEUP-THIEUTH         = PIEUP + THIEUTH */
  { 0x11071111, 0x112a }, /* CHOSEONG PIEUP-PHIEUPH         = PIEUP + PHIEUPH */
  { 0x1108110b, 0x112c }, /* CHOSEONG KAPYEOUNSSANGPIEUP    = SSANGPIEUP + IEUNG */
  { 0x11091100, 0x112d }, /* CHOSEONG SIOS-KIYEOK           = SIOS + KIYEOK */
  { 0x11091102, 0x112e }, /* CHOSEONG SIOS-NIEUN            = SIOS + NIEUN */
  { 0x11091103, 0x112f }, /* CHOSEONG SIOS-TIKEUT           = SIOS + TIKEUT */
  { 0x11091105, 0x1130 }, /* CHOSEONG SIOS-RIEUL            = SIOS + RIEUL */
  { 0x11091106, 0x1131 }, /* CHOSEONG SIOS-MIEUM            = SIOS + MIEUM */
  { 0x11091107, 0x1132 }, /* CHOSEONG SIOS-PIEUP            = SIOS + PIEUP */
  { 0x11091109, 0x110a }, /* CHOSEONG SSANGSIOS             = SIOS + SIOS */
  { 0x1109110b, 0x1135 }, /* CHOSEONG SIOS-IEUNG            = SIOS + IEUNG */
  { 0x1109110c, 0x1136 }, /* CHOSEONG SIOS-CIEUC            = SIOS + CIEUC */
  { 0x1109110e, 0x1137 }, /* CHOSEONG SIOS-CHIEUCH          = SIOS + CHIEUCH */
  { 0x1109110f, 0x1138 }, /* CHOSEONG SIOS-KHIEUKH          = SIOS + KHIEUKH */
  { 0x11091110, 0x1139 }, /* CHOSEONG SIOS-THIEUTH          = SIOS + THIEUTH */
  { 0x11091111, 0x113a }, /* CHOSEONG SIOS-PHIEUPH          = SIOS + PHIEUPH */
  { 0x11091112, 0x113b }, /* CHOSEONG SIOS-HIEUH            = SIOS + HIEUH */
  { 0x110a1109, 0x1134 }, /* CHOSEONG SIOS-SSANGSIOS        = SSANGSIOS + SIOS */
  { 0x110b1100, 0x1141 }, /* CHOSEONG IEUNG-KIYEOK          = IEUNG + KIYEOK */
  { 0x110b1103, 0x1142 }, /* CHOSEONG IEUNG-TIKEUT          = IEUNG + TIKEUT */
  { 0x110b1106, 0x1143 }, /* CHOSEONG IEUNG-MIEUM           = IEUNG + MIEUM */
  { 0x110b1107, 0x1144 }, /* CHOSEONG IEUNG-PIEUP           = IEUNG + PIEUP */
  { 0x110b1109, 0x1145 }, /* CHOSEONG IEUNG-SIOS            = IEUNG + SIOS */
  { 0x110b110b, 0x1147 }, /* CHOSEONG SSANGIEUNG            = IEUNG + IEUNG */
  { 0x110b110c, 0x1148 }, /* CHOSEONG IEUNG-CIEUC           = IEUNG + CIEUC */
  { 0x110b110e, 0x1149 }, /* CHOSEONG IEUNG-CHIEUCH         = IEUNG + CHIEUCH */
  { 0x110b1110, 0x114a }, /* CHOSEONG IEUNG-THIEUTH         = IEUNG + THIEUTH */
  { 0x110b1111, 0x114b }, /* CHOSEONG IEUNG-PHIEUPH         = IEUNG + PHIEUPH */
  { 0x110b1140, 0x1146 }, /* CHOSEONG IEUNG-PANSIOS         = IEUNG + PANSIOS */
  { 0x110c110b, 0x114d }, /* CHOSEONG CIEUC-IEUNG           = CIEUC + IEUNG */
  { 0x110c110c, 0x110d }, /* CHOSEONG SSANGCIEUC            = CIEUC + CIEUC */
  { 0x110e110f, 0x1152 }, /* CHOSEONG CHIEUCH-KHIEUKH       = CHIEUCH + KHIEUKH */
  { 0x110e1112, 0x1153 }, /* CHOSEONG CHIEUCH-HIEUH         = CHIEUCH + HIEUH */
  { 0x11111107, 0x1156 }, /* CHOSEONG PHIEUPH-PIEUP         = PHIEUPH + PIEUP */
  { 0x1111110b, 0x1157 }, /* CHOSEONG KAPYEOUNPHIEUPH       = PHIEUPH + IEUNG */
  { 0x11121112, 0x1158 }, /* CHOSEONG SSANGHIEUH            = HIEUH + HIEUH */
  { 0x11211100, 0x1122 }, /* CHOSEONG PIEUP-SIOS-KIYEOK     = PIEUP-SIOS + KIYEOK */
  { 0x11211103, 0x1123 }, /* CHOSEONG PIEUP-SIOS-TIKEUT     = PIEUP-SIOS + TIKEUT */
  { 0x11211107, 0x1124 }, /* CHOSEONG PIEUP-SIOS-PIEUP      = PIEUP-SIOS + PIEUP */
  { 0x11211109, 0x1125 }, /* CHOSEONG PIEUP-SSANGSIOS       = PIEUP-SIOS + SIOS */
  { 0x1121110c, 0x1126 }, /* CHOSEONG PIEUP-SIOS-CIEUC      = PIEUP-SIOS + CIEUC */
  { 0x11321100, 0x1133 }, /* CHOSEONG SIOS-PIEUP-KIYEOK     = SIOS-PIEUP + KIYEOK */
  { 0x113c113c, 0x113d }, /* CHOSEONG CHITUEUMSSANGSIOS     = CHITUEUMSIOS + CHITUEUMSIOS */
  { 0x113e113e, 0x113f }, /* CHOSEONG CEONGCHIEUMSSANGSIOS  = CEONGCHIEUMSIOS + CEONGCHIEUMSIOS */
  { 0x114e114e, 0x114f }, /* CHOSEONG CHITUEUMSSANGCIEUC    = CHITUEUMCIEUC + CHITUEUMCIEUC */
  { 0x11501150, 0x1151 }, /* CHOSEONG CEONGCHIEUMSSANGCIEUC = CEONGCHIEUMCIEUC + CEONGCHIEUMCIEUC */
  { 0x11611169, 0x1176 }, /* JUNGSEONG A-O                  = A + O */
  { 0x1161116e, 0x1177 }, /* JUNGSEONG A-U                  = A + U */
  { 0x11611175, 0x1162 }, /* JUNGSEONG AE                   = A */
  { 0x11631169, 0x1178 }, /* JUNGSEONG YA-O                 = YA + O */
  { 0x1163116d, 0x1179 }, /* JUNGSEONG YA-YO                = YA + YO */
  { 0x11631175, 0x1164 }, /* JUNGSEONG YAE                  = YA */
  { 0x11651169, 0x117a }, /* JUNGSEONG EO-O                 = EO + O */
  { 0x1165116e, 0x117b }, /* JUNGSEONG EO-U                 = EO + U */
  { 0x11651173, 0x117c }, /* JUNGSEONG EO-EU                = EO + EU */
  { 0x11651175, 0x1166 }, /* JUNGSEONG E                    = EO */
  { 0x11671169, 0x117d }, /* JUNGSEONG YEO-O                = YEO + O */
  { 0x1167116e, 0x117e }, /* JUNGSEONG YEO-U                = YEO + U */
  { 0x11671175, 0x1168 }, /* JUNGSEONG YE                   = YEO */
  { 0x11691161, 0x116a }, /* JUNGSEONG WA                   = O + A */
  { 0x11691165, 0x117f }, /* JUNGSEONG O-EO                 = O + EO */
  { 0x11691166, 0x1180 }, /* JUNGSEONG O-E                  = O + E */
  { 0x11691168, 0x1181 }, /* JUNGSEONG O-YE                 = O + YE */
  { 0x11691169, 0x1182 }, /* JUNGSEONG O-O                  = O + O */
  { 0x1169116e, 0x1183 }, /* JUNGSEONG O-U                  = O + U */
  { 0x11691175, 0x116c }, /* JUNGSEONG OE                   = O */
  { 0x116a1175, 0x116b }, /* JUNGSEONG WAE                  = WA */
  { 0x116d1163, 0x1184 }, /* JUNGSEONG YO-YA                = YO + YA */
  { 0x116d1164, 0x1185 }, /* JUNGSEONG YO-YAE               = YO + YAE */
  { 0x116d1167, 0x1186 }, /* JUNGSEONG YO-YEO               = YO + YEO */
  { 0x116d1169, 0x1187 }, /* JUNGSEONG YO-O                 = YO + O */
  { 0x116d1175, 0x1188 }, /* JUNGSEONG YO-I                 = YO + I */
  { 0x116e1161, 0x1189 }, /* JUNGSEONG U-A                  = U + A */
  { 0x116e1162, 0x118a }, /* JUNGSEONG U-AE                 = U + AE */
  { 0x116e1165, 0x116f }, /* JUNGSEONG WEO                  = U + EO */
  { 0x116e1168, 0x118c }, /* JUNGSEONG U-YE                 = U + YE */
  { 0x116e116e, 0x118d }, /* JUNGSEONG U-U                  = U + U */
  { 0x116e1175, 0x1171 }, /* JUNGSEONG WI                   = U */
  { 0x116f1173, 0x118b }, /* JUNGSEONG U-EO-EU              = WEO + EU */
  { 0x116f1175, 0x1170 }, /* JUNGSEONG WE                   = WEO */
  { 0x11721161, 0x118e }, /* JUNGSEONG YU-A                 = YU + A */
  { 0x11721165, 0x118f }, /* JUNGSEONG YU-EO                = YU + EO */
  { 0x11721166, 0x1190 }, /* JUNGSEONG YU-E                 = YU + E */
  { 0x11721167, 0x1191 }, /* JUNGSEONG YU-YEO               = YU + YEO */
  { 0x11721168, 0x1192 }, /* JUNGSEONG YU-YE                = YU + YE */
  { 0x1172116e, 0x1193 }, /* JUNGSEONG YU-U                 = YU + U */
  { 0x11721175, 0x1194 }, /* JUNGSEONG YU-I                 = YU + I */
  { 0x1173116e, 0x1195 }, /* JUNGSEONG EU-U                 = EU + U */
  { 0x11731173, 0x1196 }, /* JUNGSEONG EU-EU                = EU + EU */
  { 0x11731175, 0x1174 }, /* JUNGSEONG YI                   = EU */
  { 0x1174116e, 0x1197 }, /* JUNGSEONG YI-U                 = YI + U */
  { 0x11751161, 0x1198 }, /* JUNGSEONG I-A                  = I + A */
  { 0x11751163, 0x1199 }, /* JUNGSEONG I-YA                 = I + YA */
  { 0x11751169, 0x119a }, /* JUNGSEONG I-O                  = I + O */
  { 0x1175116e, 0x119b }, /* JUNGSEONG I-U                  = I + U */
  { 0x11751173, 0x119c }, /* JUNGSEONG I-EU                 = I + EU */
  { 0x1175119e, 0x119d }, /* JUNGSEONG I-ARAEA              = I */
  { 0x119e1165, 0x119f }, /* JUNGSEONG ARAEA-EO             = ARAEA + EO */
  { 0x119e116e, 0x11a0 }, /* JUNGSEONG ARAEA-U              = ARAEA + U */
  { 0x119e1175, 0x11a1 }, /* JUNGSEONG ARAEA-I              = ARAEA + I */
  { 0x119e119e, 0x11a2 }, /* JUNGSEONG SSANGARAEA           = ARAEA + ARAEA */
  { 0x11a811a8, 0x11a9 }, /* JONGSEONG SSANGKIYEOK          = KIYEOK + KIYEOK */
  { 0x11a811af, 0x11c3 }, /* JONGSEONG KIYEOK-RIEUL         = KIYEOK + RIEUL */
  { 0x11a811ba, 0x11aa }, /* JONGSEONG KIYEOK-SIOS          = KIYEOK */
  { 0x11aa11a8, 0x11c4 }, /* JONGSEONG KIYEOK-SIOS-KIYEOK   = KIYEOK-SIOS + KIYEOK */
  { 0x11ab11a8, 0x11c5 }, /* JONGSEONG NIEUN-KIYEOK         = NIEUN + KIYEOK */
  { 0x11ab11ae, 0x11c6 }, /* JONGSEONG NIEUN-TIKEUT         = NIEUN + TIKEUT */
  { 0x11ab11ba, 0x11c7 }, /* JONGSEONG NIEUN-SIOS           = NIEUN + SIOS */
  { 0x11ab11bd, 0x11ac }, /* JONGSEONG NIEUN-CIEUC          = NIEUN */
  { 0x11ab11c0, 0x11c9 }, /* JONGSEONG NIEUN-THIEUTH        = NIEUN + THIEUTH */
  { 0x11ab11c2, 0x11ad }, /* JONGSEONG NIEUN-HIEUH          = NIEUN */
  { 0x11ab11eb, 0x11c8 }, /* JONGSEONG NIEUN-PANSIOS        = NIEUN */
  { 0x11ae11a8, 0x11ca }, /* JONGSEONG TIKEUT-KIYEOK        = TIKEUT + KIYEOK */
  { 0x11ae11af, 0x11cb }, /* JONGSEONG TIKEUT-RIEUL         = TIKEUT + RIEUL */
  { 0x11af11a8, 0x11b0 }, /* JONGSEONG RIEUL-KIYEOK         = RIEUL + KIYEOK */
  { 0x11af11ab, 0x11cd }, /* JONGSEONG RIEUL-NIEUN          = RIEUL + NIEUN */
  { 0x11af11ae, 0x11ce }, /* JONGSEONG RIEUL-TIKEUT         = RIEUL + TIKEUT */
  { 0x11af11af, 0x11d0 }, /* JONGSEONG SSANGRIEUL           = RIEUL + RIEUL */
  { 0x11af11b7, 0x11b1 }, /* JONGSEONG RIEUL-MIEUM          = RIEUL */
  { 0x11af11b8, 0x11b2 }, /* JONGSEONG RIEUL-PIEUP          = RIEUL */
  { 0x11af11ba, 0x11b3 }, /* JONGSEONG RIEUL-SIOS           = RIEUL */
  { 0x11af11bf, 0x11d8 }, /* JONGSEONG RIEUL-KHIEUKH        = RIEUL + KHIEUKH */
  { 0x11af11c0, 0x11b4 }, /* JONGSEONG RIEUL-THIEUTH        = RIEUL */
  { 0x11af11c1, 0x11b5 }, /* JONGSEONG RIEUL-PHIEUPH        = RIEUL */
  { 0x11af11c2, 0x11b6 }, /* JONGSEONG RIEUL-HIEUH          = RIEUL */
  { 0x11af11eb, 0x11d7 }, /* JONGSEONG RIEUL-PANSIOS        = RIEUL */
  { 0x11af11f9, 0x11d9 }, /* JONGSEONG RIEUL-YEORINHIEUH    = RIEUL */
  { 0x11b011ba, 0x11cc }, /* JONGSEONG RIEUL-KIYEOK-SIOS    = RIEUL-KIYEOK + SIOS */
  { 0x11b111a8, 0x11d1 }, /* JONGSEONG RIEUL-MIEUM-KIYEOK   = RIEUL-MIEUM + KIYEOK */
  { 0x11b111ba, 0x11d2 }, /* JONGSEONG RIEUL-MIEUM-SIOS     = RIEUL-MIEUM + SIOS */
  { 0x11b211ba, 0x11d3 }, /* JONGSEONG RIEUL-PIEUP-SIOS     = RIEUL-PIEUP + SIOS */
  { 0x11b211bc, 0x11d5 }, /* JONGSEONG RIEUL-KAPYEOUNPIEUP  = RIEUL-PIEUP + IEUNG */
  { 0x11b211c2, 0x11d4 }, /* JONGSEONG RIEUL-PIEUP-HIEUH    = RIEUL-PIEUP + HIEUH */
  { 0x11b311ba, 0x11d6 }, /* JONGSEONG RIEUL-SSANGSIOS      = RIEUL-SIOS + SIOS */
  { 0x11b711a8, 0x11da }, /* JONGSEONG MIEUM-KIYEOK         = MIEUM + KIYEOK */
  { 0x11b711af, 0x11db }, /* JONGSEONG MIEUM-RIEUL          = MIEUM + RIEUL */
  { 0x11b711b8, 0x11dc }, /* JONGSEONG MIEUM-PIEUP          = MIEUM + PIEUP */
  { 0x11b711ba, 0x11dd }, /* JONGSEONG MIEUM-SIOS           = MIEUM + SIOS */
  { 0x11b711bc, 0x11e2 }, /* JONGSEONG KAPYEOUNMIEUM        = MIEUM + IEUNG */
  { 0x11b711be, 0x11e0 }, /* JONGSEONG MIEUM-CHIEUCH        = MIEUM + CHIEUCH */
  { 0x11b711c2, 0x11e1 }, /* JONGSEONG MIEUM-HIEUH          = MIEUM + HIEUH */
  { 0x11b711eb, 0x11df }, /* JONGSEONG MIEUM-PANSIOS        = MIEUM */
  { 0x11b811af, 0x11e3 }, /* JONGSEONG PIEUP-RIEUL          = PIEUP + RIEUL */
  { 0x11b811ba, 0x11b9 }, /* JONGSEONG PIEUP-SIOS           = PIEUP */
  { 0x11b811bc, 0x11e6 }, /* JONGSEONG KAPYEOUNPIEUP        = PIEUP + IEUNG */
  { 0x11b811c1, 0x11e4 }, /* JONGSEONG PIEUP-PHIEUPH        = PIEUP + PHIEUPH */
  { 0x11b811c2, 0x11e5 }, /* JONGSEONG PIEUP-HIEUH          = PIEUP + HIEUH */
  { 0x11ba11a8, 0x11e7 }, /* JONGSEONG SIOS-KIYEOK          = SIOS + KIYEOK */
  { 0x11ba11ae, 0x11e8 }, /* JONGSEONG SIOS-TIKEUT          = SIOS + TIKEUT */
  { 0x11ba11af, 0x11e9 }, /* JONGSEONG SIOS-RIEUL           = SIOS + RIEUL */
  { 0x11ba11b8, 0x11ea }, /* JONGSEONG SIOS-PIEUP           = SIOS + PIEUP */
  { 0x11ba11ba, 0x11bb }, /* JONGSEONG SSANGSIOS            = SIOS + SIOS */
  { 0x11bc11a8, 0x11ec }, /* JONGSEONG IEUNG-KIYEOK         = IEUNG + KIYEOK */
  { 0x11bc11bc, 0x11ee }, /* JONGSEONG SSANGIEUNG           = IEUNG + IEUNG */
  { 0x11bc11bf, 0x11ef }, /* JONGSEONG IEUNG-KHIEUKH        = IEUNG + KHIEUKH */
  { 0x11c111b8, 0x11f3 }, /* JONGSEONG PHIEUPH-PIEUP        = PHIEUPH + PIEUP */
  { 0x11c111bc, 0x11f4 }, /* JONGSEONG KAPYEOUNPHIEUPH      = PHIEUPH + IEUNG */
  { 0x11c211ab, 0x11f5 }, /* JONGSEONG HIEUH-NIEUN          = HIEUH + NIEUN */
  { 0x11c211af, 0x11f6 }, /* JONGSEONG HIEUH-RIEUL          = HIEUH + RIEUL */
  { 0x11c211b7, 0x11f7 }, /* JONGSEONG HIEUH-MIEUM          = HIEUH + MIEUM */
  { 0x11c211b8, 0x11f8 }, /* JONGSEONG HIEUH-PIEUP          = HIEUH + PIEUP */
  { 0x11ce11c2, 0x11cf }, /* JONGSEONG RIEUL-TIKEUT-HIEUH   = RIEUL-TIKEUT + HIEUH */
  { 0x11dd11ba, 0x11de }, /* JONGSEONG MIEUM-SSANGSIOS      = MIEUM-SIOS + SIOS */
  { 0x11ec11a8, 0x11ed }, /* JONGSEONG IEUNG-SSANGKIYEOK    = IEUNG-KIYEOK + KIYEOK */
  { 0x11f011ba, 0x11f1 }, /* JONGSEONG YESIEUNG-SIOS        = YESIEUNG + SIOS */
  { 0x11f011eb, 0x11f2 }, /* JONGSEONG YESIEUNG-PANSIOS     = YESIEUNG + PANSIOS */
};

//#define compose_table compose_table_default
static HangulCompose compose_table_user[256];
static HangulCompose *compose_table = (HangulCompose *) compose_table_2set;
static int compose_table_size = sizeof(compose_table_2set) / sizeof(HangulCompose);

int PASCAL set_compose(UINT type)
{
    INT atype,i;
    switch (type)
    {
	case 1:
           compose_table = (HangulCompose *) compose_table_default;
           compose_table_size =
		   sizeof(compose_table_default) / sizeof(HangulCompose);
	   dwComposeFlag=1;
           break;
	case 2:
           compose_table = (HangulCompose *) compose_table_2set;
           compose_table_size =
		   sizeof(compose_table_2set) / sizeof(HangulCompose);
	   dwComposeFlag=2;
           break;
        case 3:
           compose_table = (HangulCompose *) compose_table_ahnmatae;
           compose_table_size =
		   sizeof(compose_table_ahnmatae) / sizeof(HangulCompose);
	   dwComposeFlag=3;
	   break;
	default:
	    // User defined compose map
#define USER_DEFINED_COMPOSE_OFFSET	4
	    type-=USER_DEFINED_COMPOSE_OFFSET;
            atype=
             load_compose_map_from_reg(NULL,type,compose_table_user);
	    if (atype) {
                compose_table=(HangulCompose *) compose_table_user;
	        for (i=0;i<256;i++) {
		    if (compose_table_user[i].key == (UINT)-1) break;
		}
		MyDebugPrint((TEXT("compose map size: %d\r\n"), i));
		compose_table_size = i;
		set_automata(atype);
	    } else {
                compose_table=(HangulCompose *) compose_table_default;
                compose_table_size =
		   sizeof(compose_table_default) / sizeof(HangulCompose);
                break;
	    }

           break;
    }
    return 0;
}

WCHAR PASCAL
hangul_compose(WCHAR first, WCHAR last)
{
    int min, max, mid;
    UINT key;

    /* make key */
    key = first << 16 | last;

    /* binary search in table */
    min = 0;
    max = compose_table_size;
    while (max >= min) {
        mid = (min + max) / 2;
        if (compose_table[mid].key < key)
            min = mid + 1;
        else if (compose_table[mid].key > key)
            max = mid - 1;
        else
            return compose_table[mid].code;
    }
    return 0;
}

static WCHAR *keyboard_table= (WCHAR *)keyboard_table_2;
static WCHAR keyboard_table_user[94];
static int (*hangul_automata)(HangulIC*, WCHAR, WCHAR*)= hangul_automata2;

int PASCAL set_keyboard(UINT type)
{
    INT atype;
    INT ctype;
    switch (type)
    {
        case LAYOUT_OLD2BUL:
            keyboard_table=(WCHAR *)keyboard_table_2;
	    atype=2;
	    ctype=2;
	    break;
	case LAYOUT_3FIN:
            keyboard_table=(WCHAR *)keyboard_table_3final;
	    atype=3;
	    ctype=1;
	    break;
	case LAYOUT_390:
            keyboard_table=(WCHAR *)keyboard_table_390;
	    atype=3;
	    ctype=1;
	    break;
	case LAYOUT_3SUN:
            keyboard_table=(WCHAR *)keyboard_table_3sun;
	    atype=3;
	    ctype=1;
	    break;
	case LAYOUT_NEW2BUL:
            atype=
	     load_keyboard_map_from_reg(TEXT("새두벌"),0,keyboard_table_user);
	    if (atype)
	        keyboard_table=keyboard_table_user;
	    ctype=2;
	    break;
	case LAYOUT_NEW3BUL:
            atype=
             load_keyboard_map_from_reg(TEXT("새세벌"),0,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    ctype=1;
	    break;
	case LAYOUT_AHNMATAE:
            atype=
             load_keyboard_map_from_reg(TEXT("안마태"),0,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    ctype=3;
	    break;
#if 0
	case LAYOUT_3YET:
            keyboard_table=(WCHAR *)keyboard_table_3yet;
	    atype=3;
	    ctype=0;
	    break;
#endif
	default:
	    // User defined keyboard
#define USER_DEFINED_KEYBOARD_OFFSET	8
	    type-=USER_DEFINED_KEYBOARD_OFFSET;
            atype=
             load_keyboard_map_from_reg(NULL,type,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    else
                keyboard_table=(WCHAR *)keyboard_table_2;
	    ctype=0;
	    break;
    }
    set_automata(atype);
    set_compose(ctype);
    return 0;
}

int PASCAL set_automata(UINT type)
{
    if (type == 0 || type == 1)
        return 0;

    dwImeFlag &= ~AUTOMATA_3SET;
    dwImeFlag &= ~AUTOMATA_2SET;
    dwImeFlag|= (type==3) ? AUTOMATA_3SET:AUTOMATA_2SET;
    if (type == 3)
        hangul_automata=hangul_automata3;
    else if (type == 2)
        hangul_automata=hangul_automata2;
    else { // raw automata
        hangul_automata=NULL;
	dwImeFlag &= ~AUTOMATA_3SET;
	dwImeFlag &= ~AUTOMATA_2SET;
    }
    return 0;
}

WORD PASCAL checkDvorak(code)
WORD code;
{
    HKL hcur;

    /* check dvorak layout */
    hcur= GetKeyboardLayout(0);
    if ((DWORD)hcur == 0xE0130412 && code >= TEXT('!') && code <= TEXT('~'))
	code = dvorak2qwerty_table[code - '!'];
    return code;
}

WCHAR PASCAL keyToHangulKey(code)
WORD code;
{
    WORD key;
    if (code >= MYTEXT('!') && code <= MYTEXT('~'))
    {
        return keyboard_table[code - MYTEXT('!')];
    }
    return 0;
}

void PASCAL hangulKeyHandler( hIMC, wParam, lParam, lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    WORD code = (WORD) HIWORD(wParam);

    LPMYSTR lpchText;
    LPMYSTR lpstr, lpread;
    LPMYSTR lpprev;

    DWORD fdwConversion;
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD dwStrLen;
    DWORD dwSize;
    TRANSMSG GnMsg;
    DWORD dwGCR = 0L;

    DWORD hkey;
    WCHAR cs = 0;
    register BOOL fModeInit = FALSE;
    BOOL capital = lpbKeyState[VK_CAPITAL];

    lpIMC = ImmLockIMC(hIMC);
    // Get ConvMode from IMC.
    fdwConversion = lpIMC->fdwConversion;

    /* check dvorak layout */
    if (fdwConversion & IME_CMODE_NATIVE)
	hkey= code = checkDvorak(code);
    else
	hkey= code;

    if (fdwConversion & IME_CMODE_NATIVE && capital) {
	WORD ch = code;
	if (ch >= 'a' && ch <= 'z')
	    code -= 'a' - 'A';
	if (ch >= 'A' && ch <= 'Z')
	    code += 'a' - 'A';
    }

    if (fdwConversion & IME_CMODE_NATIVE)
	hkey = keyToHangulKey( code );
#ifndef USE_NO_WM_CHAR
    if ( (!hkey || (hkey >= TEXT('!') && hkey <= TEXT('~')) )
	    && !IsCompStr(hIMC)) {
	// 이 경우 마지막 입력받은 ascii문자를 그대로 내뱉는다.
	PostMessage(lpIMC->hWnd,WM_CHAR,hkey,lParam);

        ImmUnlockIMC(hIMC);
	return;
    }
#endif

    if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof (MYCOMPSTR))
    {
        // Init time.
        dwSize = sizeof(MYCOMPSTR);
        lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr,dwSize);
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        lpCompStr->dwSize = dwSize;
	fModeInit=TRUE;

        hangul_ic_init(&ic);
    }
    else
    {
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    }

    dwStrLen = lpCompStr->dwCompStrLen;

    if (!dwStrLen)
    {
        // 초기화
        InitCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

        GnMsg.message = WM_IME_STARTCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

        hangul_ic_init(&ic);
    }

    if (IsConvertedCompStr(hIMC))
    {
        MakeResultString(hIMC,FALSE);
        InitCompStr(lpCompStr,CLR_UNDET);
        dwGCR = GCS_RESULTALL;

        hangul_ic_init(&ic);
    }

    // Init lpstr
    lpchText = GETLPCOMPSTR(lpCompStr);
    lpstr = lpchText;
    if( lpCompStr->dwCursorPos )
        lpstr += lpCompStr->dwCursorPos;
    lpstr = lpchText + Mylstrlen(lpchText);
    lpprev = MyCharPrev( lpchText, lpstr );

    if ( hkey )
    {
        MyDebugPrint((TEXT("hkey: 0x%x -> 0x%x\r\n"), code, hkey));

        // MODE_CHARCODE
	if( fdwConversion & IME_CMODE_CHARCODE )
	{
	    MyDebugPrint((TEXT("char code: %x\r\n"), hkey));

            code = (WORD)(LONG_PTR)AnsiUpper( (LPSTR)(LONG_PTR)code );
            if( !( (code >= MYTEXT('0') && code <= MYTEXT('9')) ||
                (code >= MYTEXT('A') && code <= MYTEXT('F')) ) || lpCompStr->dwCursorPos >= 4 ){
                MessageBeep( 0 );
                goto ac_exit;
            }

            *lpstr++ = (BYTE)code;
            lpCompStr->dwCursorPos++;
	} else if ( hangul_automata == NULL && fdwConversion & IME_CMODE_NATIVE) {
	    // RAW automata: composemap only
	    WCHAR comb=0;
	    if (lpCompStr->dwCursorPos > 0) {
                comb = hangul_compose(*(lpstr-1), (WCHAR) hkey);
		if (comb) { // composable
		    // replace
		    cs = *(lpstr-1) = comb;
		}
	    }
	    if (!comb) {
		if (dwImeFlag & SAENARU_ONTHESPOT) {
		    if (lpCompStr->dwCursorPos > 0)
			// check jamos and convert it to compatible jamos
			*lpprev = hangul_jamo_to_cjamo((WCHAR) *lpprev);
		    dwGCR=GCS_RESULTALL;

		    MakeResultString(hIMC,FALSE);
		    InitCompStr(lpCompStr,CLR_UNDET);

		    lpchText = GETLPCOMPSTR(lpCompStr);
		    lpstr = lpchText;
		    if( lpCompStr->dwCursorPos )
			lpstr += lpCompStr->dwCursorPos;
		    lpstr = lpchText + Mylstrlen(lpchText);
		    lpprev = MyCharPrev( lpchText, lpstr );
		}
		*lpstr = (WCHAR) hkey;
	        cs = hangul_jamo_to_cjamo((WCHAR) hkey);
		lpstr++;
		lpCompStr->dwCursorPos++;
		// emit
	    }
	} else if ( (hkey >= 0x1100 && hkey <= 0x11c2) &&
                   !(dwOptionFlag & FULL_MULTIJOMO) &&
                    (fdwConversion & IME_CMODE_NATIVE) )
        {
	    int ret;

            MyDebugPrint((TEXT("input jamo: %x\r\n"), hkey));

            ret = hangul_automata(&ic, (WCHAR) hkey, &cs);
            ic.lastvkey = (UINT)VkKeyScan(code);
            MyDebugPrint((TEXT(" * last vkey: 0x%x\r\n"), ic.lastvkey));
            MyDebugPrint((TEXT(" * last scancode: 0x%x\r\n"), lParam >> 16));

	    if (cs)
	    {
		// not composable. emit hangul syllable
		*(lpstr - 1) = cs;
		if (dwImeFlag & SAENARU_ONTHESPOT)
		{
                    dwGCR=GCS_RESULTALL;

                    MakeResultString(hIMC,FALSE);
                    InitCompStr(lpCompStr,CLR_UNDET);

                    lpchText = GETLPCOMPSTR(lpCompStr);
                    lpstr = lpchText;
                    if( lpCompStr->dwCursorPos )
                        lpstr += lpCompStr->dwCursorPos;
                    lpstr = lpchText + Mylstrlen(lpchText);
                    lpprev = MyCharPrev( lpchText, lpstr );
		}
                lpstr++;
               	lpCompStr->dwCursorPos++;
	    }

	    if (ret <= 0) {
		cs = hangul_ic_get(&ic,0);
		*(lpstr - 1) = cs;
	    } else {
		// new hangul composition
		cs = hangul_ic_get(&ic,0);
		*lpstr++ = cs;
               	lpCompStr->dwCursorPos++;
	    }
        }
	else
	{
	    if (!hkey) hkey = code;
	    code = (WORD) hkey;
            MyDebugPrint((TEXT("ascii code: %x\r\n"), (WORD)hkey));
	    /* for Hanme Typing tutor */
            MakeResultString(hIMC,FALSE);
            InitCompStr(lpCompStr,CLR_UNDET);

	    dwGCR = GCS_RESULTALL;

	    hangul_ic_init(&ic);

            lpchText = GETLPCOMPSTR(lpCompStr);
            lpstr = lpchText;
	    /* */
	    /* XXX how to fix EditPlus problem ? */
            *lpstr++ = (WORD)hkey;
            lpCompStr->dwCursorPos++;
	}
        *lpstr = MYTEXT('\0');
    }
    else // Non transtated key as ', . ;' etc.
    {
        MyDebugPrint((TEXT("RAW code: %x\r\n"), code));
#if 1
        hangul_ic_init(&ic);

	// if (code == 0xd) code = '\n';
	// 이 방식으로 리턴키를 쓰면 모질라, vim 등등에서 모두 되었으나,
	// 모질라에서만 제대로 되지 않는 문제로 인하여 keybd_event()를 썼다.
        *lpstr++ = (BYTE)code;
        lpCompStr->dwCursorPos++;
#endif
        *lpstr = MYTEXT('\0');
    }

    // make reading string.
    lpstr = GETLPCOMPSTR(lpCompStr);
    lpread = GETLPCOMPREADSTR(lpCompStr);

    MyDebugPrint((TEXT("CompReadStr %s\n"),lpread));

    Mylstrcpy(lpread,lpstr);

    // make attribute
    // lpCompStr->dwCursorPos = Mylstrlen(lpstr);
    lpCompStr->dwDeltaStart =
	    (DWORD)(MyCharPrev(lpstr, lpstr+Mylstrlen(lpstr)) - lpstr);

    //MakeAttrClause(lpCompStr);
    lmemset((LPBYTE)GETLPCOMPATTR(lpCompStr),ATTR_INPUT, Mylstrlen(lpstr));
    lmemset((LPBYTE)GETLPCOMPREADATTR(lpCompStr),ATTR_INPUT, Mylstrlen(lpread));

    // make length
    lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
    lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
    lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
    lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);

    if (lpCompStr->dwCompReadStrLen > 0)
        lpCompStr->dwCompReadStrLen--;

    /* ReadStr은 무엇이고, CompStr은 무엇인가 ?
     * ReadStr은 버퍼이고, CompStr은 그것을 해석한 결과를 저장하는 ?
     */

    //
    // make clause info
    //
    SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
    SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
    lpCompStr->dwCompClauseLen = 8;
    lpCompStr->dwCompReadClauseLen = 8;

    /*
http://msdn.microsoft.com/library/default.asp?url=/library/en-us/intl/ime_88q6.asp
     wParam은 DBCS값 혹은 unicode값이다.
     */

    if (dwGCR != GCS_RESULTALL && lpCompStr->dwCompStrLen)
    {
        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = cs;
        //GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
        GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; //한글 IME 2002,2003
        if (dwImeFlag & SAENARU_ONTHESPOT)
            GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }
    /*
     * 한글 IME 2002는 CS_INSERTCHAR | CS_NOMOVECARET 를 쓴다.
     */
    /*
     * GenerateMessage()를 할 때, COMPSTR을 한 후에 RESULTSTR을 해야 한다.
     * 즉, 순서를 지켜야 한다.
     * 워드패드는 GCS_RESULTSTR | GCS_COMPSTR은 되지만, 순서를 뒤바꾸면
     * 안된다. 그런데 이 경우는 모질라가 안된다. -_-;;
     * 
     * 순서를 지키지 않아도 되는 어플은 "노트패드"정도 뿐 ?
     * 
     * 이 순서를 지키지 않으면, COMPSTR이 되고 RESULTSTR으로 덮어써져서
     * 원래 보여야 할 문자가 보이지 않는 결과가 초래한다.
     *
     */

ac_exit:
    //
    // RESULTSTR이 있고, CompStr에 남은 문자가 한글 이외의 문자이면
    // CompStr을 commit, RESULTSTR을 commit.
    // 
    // RESULTSTR이 없고, CompStr이 한글 이외의 문자이면 전체를 commit
    //
    if ( dwGCR != GCS_RESULTALL ) {
	if (!cs)
            MakeResultString(hIMC,TRUE);
    } else {
        WCHAR result = 0;
	UINT resultlen = 0;
        lpstr = GETLPRESULTSTR(lpCompStr);

        resultlen = Mylstrlen(lpstr);
        lpstr += resultlen;

        lpread = GETLPRESULTREADSTR(lpCompStr);
        lpread += Mylstrlen(lpread);

	if (!cs) // emit all chars
	{
	    hangul_ic_init(&ic);
	    *lpstr++ = code;
            *lpread++ = code;
	    *lpstr = '\0';
	    *lpread = '\0';
            lpCompStr->dwResultStrLen++;
            lpCompStr->dwResultReadStrLen++;

            SetClause(GETLPRESULTCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTSTR(lpCompStr)));
            SetClause(GETLPRESULTREADCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTREADSTR(lpCompStr)));

            ClearCompStr(lpCompStr,CLR_UNDET);
	}

        if ((dwImeFlag & SAENARU_ONTHESPOT) && resultlen)
	    result = *(lpstr - 1);

        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = result;
        GnMsg.lParam = GCS_RESULTALL;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

	if (!lpCompStr->dwCompStrLen)
	{
            // 고고타자, 워드패드에서 제대로 동작하도록 수정
            GnMsg.message = WM_IME_ENDCOMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = 0;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

	    // 한글로 이루어진 CompStr이 없다는 것은, 마지막
	    // 입력된 문자가 한글 자소가 아닌 경우
	    // 이 경우 마지막 입력받은 ascii문자를 그대로 내뱉는다.
	    //PostMessage(lpIMC->hWnd,WM_CHAR,code,lParam);
	} else {
#if 0
            // 고고타자, 워드패드에서 제대로 동작하도록 수정
	    // 이 부분을 *비활성* 시켜야 제대로 작동한다.
	    //
	    // EditPlus 2.21 (330), WordPad
	    //
            GnMsg.message = WM_IME_STARTCOMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = 0;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
#endif
            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.wParam = cs;
            //GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
            if (dwImeFlag & SAENARU_ONTHESPOT)
                GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
	}
    }
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}

void PASCAL hangul_ic_init( HangulIC *ic )
{
    ic->cho=0;
    ic->jung=0;
    ic->jong=0;
    ic->last=0;
    ic->len=0;
    ic->laststate=0;
    ic->lastvkey=0;
    ic->syllable=TRUE;
}

WCHAR PASCAL hangul_ic_get( HangulIC *ic, UINT mode)
{
    WCHAR code=0;

    if (ic->len > 3 || !ic->syllable) return 0;

    if (ic->len == 1)
    {
	if (ic->cho)
            code = hangul_choseong_to_cjamo(ic->cho);
	else if (ic->jung)
            code = hangul_jungseong_to_cjamo(ic->jung);
	else if (ic->jong)
            code = hangul_jongseong_to_cjamo(ic->jong);
    }
    else if (ic->len == 3)
    {
        code = hangul_jamo_to_syllable(ic->cho,ic->jung,ic->jong);
    }
    else if (ic->len == 2)
    {
#define HANGUL_PSEUDO_FILL_CHO	0x110b
#define HANGUL_PSEUDO_FILL_JUNG	0x1161
	if (ic->cho && ic->jung)
            code = hangul_jamo_to_syllable(ic->cho,ic->jung,0);
	else
	{
#if 0
            if (1)
                code = 0x3000;
#endif
	    if (ic->cho && ic->jong)
                code = hangul_jamo_to_syllable(ic->cho,0x1161,ic->jong);
	    else if (ic->jung && ic->jong)
                code = hangul_jamo_to_syllable(0x110b,ic->jung,ic->jong);
	}
    }
    if (mode == 1)
        hangul_ic_init(ic);
    return code;
}

void hangul_ic_push( HangulIC *ic, WCHAR ch )
{
    ic->read[ic->len++] = ch;
    ic->last = ch;
}

WCHAR hangul_ic_pop( HangulIC *ic )
{
    if (ic->len <= 0)
	return 0;
    ic->last = ic->read[--ic->len];
    return ic->read[ic->len];
}

WCHAR hangul_ic_peek( HangulIC *ic )
{
    if (ic->len <= 0)
	return 0;

    return ic->read[ic->len - 1];
}

BOOL hangul_ic_insert( HangulIC *ic, WCHAR ch, UINT idx)
{
    UINT i;

    if (idx > ic->len)
        return FALSE;

    for (i=ic->len ; i > idx ; i--)
        ic->read[i] = ic->read[i-1];
    ic->read[idx] = ch;
    ic->len++;
    return TRUE;
}

#define hangul_ic_commit(ic) hangul_ic_get(ic, 1)

int hangul_automata2( HangulIC *ic, WCHAR jamo, WCHAR *cs )
{
    int kind;
    WCHAR comb=0, cho=0, jong=0, last=0, tmp=0,tmp2=0;
    BOOL ctyping=0;

    *cs = 0;

    if (jamo >= 0x1100 && jamo <= 0x1159) kind=1;
    else if (jamo >= 0x1161 && jamo <= 0x11a2) kind=2;
    else if (jamo >= 0x11a8 && jamo <= 0x11f9) kind=3;
    else return -1;

    ctyping = (dwOptionFlag & CONCURRENT_TYPING) && (ic->last != jamo) &&
              (GetAsyncKeyState(ic->lastvkey) & 0x80000000);

    if (ic->laststate)
    {
        if (ic->laststate == 1)
	{
            MyDebugPrint((TEXT("cho code: %x\r\n"), jamo));
            switch(kind) {
            case 1:
                // 초성+초성은 두벌식의 오토마타에서 원래 지원하지 않는다.
                // 그러나, 나비가 지원하므로 지원한다 :)
                comb = hangul_compose(ic->cho, jamo);
		// 동시치기일 경우는 순서를 바꿔서 초+초 조합
		if ( ctyping && !comb)
		    comb = hangul_compose(jamo,ic->cho);

                if ( hangul_is_choseong(comb) ) {
                    ic->cho=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    return 0;
		} else if ( hangul_is_jongseong(comb) ) {
		    // 초+초 -> 종성이 되는 경우
		    // IME 2002에서 지원하므로
                    ic->jong=comb;
                    ic->cho=0;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=3;
                    return -1;
                }
                // 초성을 compose할 수 없다.
                *cs = hangul_ic_commit(ic);
                ic->cho=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=1;
                return -1;
                break;
            case 2: // 초성+중성
                ic->jung=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=2;
                return 0;
                break;
            default: // 초성+종성
                // 두벌식 오토마타는 종성처리가 필요 없다.
                // 그러나, 세벌식에서 두벌식 오토마타를 쓰기
                // 위해서 종성 처리를 넣는다.
                *cs = hangul_ic_commit(ic);
                ic->jong=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=3;
                return -1;
                break;
            }
	} else if (ic->laststate == 2) {
            switch(kind) {
            case 1: // TODO 중성 + 초성
                // 초성이 없을 경우 : 나비에서는 중성+초성을 교정한다.
                if (!ic->cho &&
                   ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=2;
		    return 0;	
                }
                // 초성을 종성으로 바꾸고,
                jong = hangul_choseong_to_jongseong(jamo);
                if (jong && hangul_jamo_to_syllable(ic->cho,ic->jung,jong)) {
                    ic->jong=jong;
                    hangul_ic_push(ic,jong);
                    ic->laststate=3;
                    return 0;
                } else {
                    *cs = hangul_ic_commit(ic);
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 중성 + 중성
                comb = hangul_compose(ic->jung, jamo);
		// 동시치기일 경우는 순서를 바꿔서 중+중 조합
		if ( ctyping && !hangul_is_jungseong(comb))
		    comb = hangul_compose(jamo,ic->jung);
                if ( hangul_is_jungseong(comb) ) {
                    ic->jung=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    return 0;
                } else {
                    // XXX 자음을 연달아 누르는 경우는 많으나,모음을 연달아
                    // 누르는 경우는 드물다.
                    // 1. 원래 있던 종성을 대치하게 하는가 ?
                    *cs = hangul_ic_commit(ic);
                    ic->jung=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=2;
                    return -1;
#if 0			
                    // 2. 그냥 연달아 누르게 하는가 ?
                    ic->last=0;
                    return -1;
#endif
                }
                break;
            default: // 중성 + 종성
                // 두벌식 오토마타는 종성처리가 필요 없다.
                // 그러나, 세벌식에서 두벌식 오토마타를 쓰기
                // 위해서 종성 처리를 넣는다.
                if (hangul_jamo_to_syllable(ic->cho,ic->jung,jamo)) {
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return 0;
                } else {
                    *cs = hangul_ic_commit(ic);
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return -1;
                }
                break;
            }
	} else if (ic->laststate == 3) {
            // 종성
            switch(kind) {
            case 1: // 초성을 종성으로 바꿔서
		cho = jamo;
                jong = hangul_choseong_to_jongseong(jamo);
                comb = hangul_compose(ic->jong, jong);
		// 동시치기일 경우는 순서를 바꿔서 종+종 조합
		if ( ctyping && !hangul_is_jongseong(comb))
		    comb = hangul_compose(jong,ic->jong);
                if ( hangul_is_jongseong(comb) )
                {
                    if (hangul_jamo_to_syllable(ic->cho,ic->jung,comb))
                    {
                        ic->jong=comb;
                        hangul_ic_pop(ic);
                        hangul_ic_push(ic,comb);
                        ic->last=jong;
                        ic->laststate=3;
                        return 0;
                    }
                }
                hangul_jongseong_dicompose(ic->jong, &tmp, &tmp2);
		if (tmp && tmp2) {
		    jong= tmp;
		    // 쌍초성에 대한 특별처리
		    last=ic->last;
		    if ( hangul_is_jongseong(ic->last) )
			last = hangul_jongseong_to_choseong(ic->last);
		    // ㅆ ㅉ은 쓸 수 없다. ㅆ은 종성으로 많이 쓰이고
		    // ㅉ은 앉지 ... 등등 때문에 제외
		    if (jong != 0x11af && last == jamo &&
			(jamo == 0x1100 || jamo == 0x1103 ||
			 jamo == 0x1107 )) // ㄲ ㄸ ㅃ
		    {
			ic->jong=jong;
			jamo= hangul_compose(last,last); // make ssang cho
			cho= jamo;
		    }
		} else if (ctyping && ic->last != jong) { // for ahnmatae
		    last = hangul_jongseong_to_choseong(ic->jong);
		    comb = hangul_compose(last,jamo);
		    MyDebugPrint((TEXT("ssang cho ?: %x\r\n"), comb));
		    if (comb &&
			    comb == 0x1101 || comb == 0x1104 ||
			    comb == 0x1108 || comb == 0x110a || comb == 0x110d) // ㄲ ㄸ ㅃ ㅆ ㅉ
		    {
			ic->jong = 0; //마지막 종성을 지운다.
			cho= comb; // 쌍초성으로 대치
		    }
		}
                {
                    // 초성을 compose할 수 없다.
                    *cs = hangul_ic_commit(ic);
                    ic->cho=cho;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 종성 + 중성
		if (!ic->jung && ctyping) {
		    // XXX
		    jong = ic->last;
		    ic->jung = jamo;
		    hangul_ic_pop(ic);
		    hangul_ic_push(ic,jong);
		    hangul_ic_push(ic,jamo);
		    ic->laststate = 2;
                    return 0;
		}
                if (ic->jong != ic->last) {
                    hangul_jongseong_dicompose(ic->jong, &jong, &cho);
		}
		last=ic->last;
		if ( hangul_is_jongseong(ic->last) )
		    last = hangul_jongseong_to_choseong(ic->last);

                if (jong && cho) {
                    if (last != cho) {
			WCHAR tmp;
			tmp = cho;
			//cho = hangul_jongseong_to_choseong(jong);
			cho = last;
			ic->jong = hangul_choseong_to_jongseong(tmp);
		    } else {
			ic->jong = jong;
		    }
		} else {
		    // 분해되지 않으면 단종성의 경우
		    // 종성 + 중성: 마지막 종성을 초성으로 바꾸고 잠시 저장
		    cho = hangul_jongseong_to_choseong(ic->jong);
		    if (cho)
			ic->jong = 0; //종성을 지운다.
		    hangul_ic_pop(ic);
                }

		*cs = hangul_ic_commit(ic);
		// XXX
		ic->cho = cho;
		ic->jung = jamo;
		hangul_ic_push(ic,cho);
		hangul_ic_push(ic,jamo);
		ic->laststate = 2;
		return -1; // 앞의 초+중 두개를 commit
                break;
            default: // 종성 + 종성
                // 두벌식 오토마타는 종성처리가 필요 없다.
                // 그러나, 세벌식에서 두벌식 오토마타를 쓰기
                // 위해서 종성 처리를 넣는다.
                comb = hangul_compose(ic->jong, jamo);
		// 동시치기일 경우는 순서를 바꿔서 종+종 조합
		if ( ctyping && !comb)
		    comb = hangul_compose(jamo,ic->jong);
                if ( comb )
                {
                    if (hangul_jamo_to_syllable(ic->cho,ic->jung,comb))
                    {
                        ic->jong=comb;
                        hangul_ic_pop(ic);
                        hangul_ic_push(ic,comb);
                        ic->last=jamo;
                        ic->laststate=3;
                        return 0;
                    }
		    // 종+종
                    ic->jong=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=3;
                    return 0;
                }
		{
                    *cs = hangul_ic_commit(ic);
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return -1;
                }
                break;
            }
	}
    } else { // ic가 초기화되어 있다.
        switch (kind) {
        case 1:
            MyDebugPrint((TEXT("single cho automata: %x\r\n"), jamo));

            ic->cho=jamo;
            hangul_ic_push(ic,jamo);
            ic->laststate=1;
            return 1;
	    break;
        case 2:
            MyDebugPrint((TEXT("single jung automata: %x\r\n"), jamo));

            ic->jung=jamo;
            hangul_ic_push(ic,jamo);
            ic->laststate=2;
            return 1;
	    break;
        default:
            MyDebugPrint((TEXT("single jong automata: %x\r\n"), jamo));

            ic->jong=jamo;
            hangul_ic_push(ic,jamo);
            ic->laststate=3;
            return 1;
            break;
	}
    }
    return 0;
}

int hangul_automata3( HangulIC *ic, WCHAR jamo, WCHAR *cs )
{
    int kind;
    WCHAR comb=0, cho=0, jung=0, jong=0;
    BOOL ctyping;

    *cs = 0;

    if (jamo >= 0x1100 && jamo <= 0x1159) kind=1;
    else if (jamo >= 0x1161 && jamo <= 0x11a2) kind=2;
    else if (jamo >= 0x11a8 && jamo <= 0x11f9) kind=3;
    else return -1;

    ctyping = (dwOptionFlag & CONCURRENT_TYPING) &&
              (GetKeyState(ic->lastvkey) & 0x80000000);

    if (ic->laststate)
    {
        if (ic->laststate == 1)
	{
            MyDebugPrint((TEXT("cho code: %x\r\n"), jamo));
            switch(kind) {
            case 1:
                // 초성+초성
                comb = hangul_compose(ic->cho, jamo);
                if ( hangul_is_choseong(comb) )
                {
                    ic->cho=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=1;
                    return 0;
                } else {
                    *cs = hangul_ic_commit(ic);
                    // 초성을 compose할 수 없다.
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 초성+중성
                if (hangul_jamo_to_syllable(ic->cho,jamo,0)) {
		    ic->jung=jamo;
		    hangul_ic_push(ic,jamo);
		    ic->laststate=2;
		    return 0;
                } else {
                    // 허락되지 않는 조합.
                    *cs = hangul_ic_commit(ic);
                    ic->jung=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=2;
                    return -1;
                }

                break;
            default: // 초성+종성
                if ( !ic->jong && 
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    ic->jong=jamo;
                    ic->laststate=3;
                    hangul_ic_push(ic,jamo);
                    return 0;
                }
                *cs = hangul_ic_commit(ic);
                ic->jong=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=3;
                return -1;
                break;
            }
	}
	else if (ic->laststate == 2)
	{
            switch(kind) {
            case 1: // 중성 + 초성
                // 초성이 없을 경우 : 나비에서는 중성+초성을 교정한다.
                if (!ic->cho &&
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
		    ic->last=ic->jung;
		    ic->laststate=2;
                    return 0;
                }
//#ifdef USE_CONCURRENT_TYPING
                // 중성이 계속 눌려진 채로 있고
                // 초성이 입력된 것이라면 
                else if (ic->cho && ctyping) {
                    MyDebugPrint((TEXT("CONJ cho !!: %x\r\n"), jamo));
                    if (ic->jung != ic->last) {
                        WCHAR j1,j2;
                        comb = ic->jung;
                        hangul_jungseong_dicompose(comb, &j1, &j2);
                        if (j1 && j2) {
                            hangul_ic_pop(ic);
                            hangul_ic_push(ic,j1);
                            ic->jung = j1;
                            jung = j2;
                        }
                    } else
                        jung = 0;
                    *cs = hangul_ic_commit(ic);
                    ic->cho = jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate = 1;
                    if (jung) {
                        ic->jung = jung;
                        hangul_ic_push(ic,jung);
                        ic->laststate = 2;
                    }
                    return -1;
                }
//#endif
                {
                    *cs = hangul_ic_commit(ic);
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 중성 + 중성
                comb = hangul_compose(ic->jung, jamo);
		if (ctyping && !comb )
		    comb = hangul_compose(jamo,ic->jung);
                if ( comb ) {
                    ic->jung=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=2;
                    return 0;
                } else {
                    // XXX 자음을 연달아 누르는 경우는 많으나,모음을 연달아
                    // 누르는 경우는 드물다.
                    // 1. 원래 있던 종성을 대치하게 하는가 ?
                    *cs = hangul_ic_commit(ic);
                    ic->cho=0;
                    ic->jung=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=2;
                    return -1;
#if 0			
                    // 2. 그냥 연달아 누르게 하는가 ?
                    ic->last=0;
                    return -1;
#endif
                }
                break;
            default: // 중성 + 종성
#if 0
                if (ic->cho && !ic->jong && ctyping) {
		    // 중성이 계속 눌려진 채로 있고
		    // 종성이 입력된 것이며, 중성이 나누어질 수 있는 경우
                    if (ic->jung != ic->last) {
                        WCHAR j1,j2;
                        comb = ic->jung;
                        hangul_jungseong_dicompose(comb, &j1, &j2);
                        if (j1 && j2) {
                            hangul_ic_pop(ic);
                            hangul_ic_push(ic,j1);
                            ic->jung = j1;
			    *cs = hangul_ic_commit(ic);

			    ic->jung = j2;
			    ic->jong = jamo;
			    hangul_ic_push(ic,j2);
			    hangul_ic_push(ic,jamo);
			    ic->laststate = 3;
			    return -1;
                        }
                    }
		}
#endif

                if (hangul_jamo_to_syllable(ic->cho,ic->jung,jamo)) {
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return 0;
                }
#if 1
		// XXX
                else if (!ic->jong && !ic->cho &&
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return 0;
                }
#endif
                {
                    *cs = hangul_ic_commit(ic);
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return -1;
                }
                break;
            }
	}
	else if (ic->laststate == 3)
	{
            // 종성
            switch(kind) {
            case 1: // 종성 + 초성
                if (!ic->cho &&
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    if (hangul_jamo_to_syllable(jamo,ic->jung ? ic->jung:0x1161,ic->jong)){
			// XXX
			ic->cho=jamo;
			hangul_ic_pop(ic);
			hangul_ic_push(ic,jamo);
			hangul_ic_push(ic,ic->jong); // fix order
			ic->laststate=3;
			return 0;
		    }
                    *cs = hangul_ic_commit(ic);
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
		    ic->laststate=1;
		    return -1;
                }
                // 종성이 계속 눌려진 채로 있고
                // 초성이 입력된 것이라면 
                else if (!ic->cho && hangul_is_jongseong(ic->last) && ctyping) {
		    if (ic->jong != ic->last) {
			WCHAR j1,j2;
			MyDebugPrint((TEXT("CON cho !!: %x\r\n"), jamo));
			// 마지막 눌려졌던 종성을 분해하여, 한개는 앞에
			// 나머지 한개는 뒤의 음절에 붙인다.
			hangul_jongseong_dicompose(ic->jong, &j1, &j2);

			if (j1 && j2) {
			    jong = hangul_choseong_to_jongseong(j2); // XXX
			    hangul_ic_pop(ic);
			    hangul_ic_push(ic,j1);
			    ic->jong = j1;
			    *cs = hangul_ic_commit(ic);
			    ic->cho = jamo;
			    ic->jong = jong;
			    hangul_ic_push(ic,jamo);
			    hangul_ic_push(ic,jong);
			    ic->laststate = 3;
			    return -1;
			}
		    }

                    jong = ic->jong;
                    hangul_ic_pop(ic);
                    ic->jong = 0;
                    *cs = hangul_ic_commit(ic);
                    ic->cho = jamo;
                    ic->jong = jong;
                    hangul_ic_push(ic,jamo);
                    hangul_ic_push(ic,jong);
                    ic->laststate = 3;
                    return -1;
                }
                {
                    // compose할 수 없다.
                    *cs = hangul_ic_commit(ic);
                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 종성 + 중성
                if (!ic->jung &&
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    if (hangul_jamo_to_syllable(ic->cho ? ic->cho:0x110b,jamo,ic->jong)) {
			ic->jung=jamo;
			hangul_ic_pop(ic);
			hangul_ic_push(ic,jamo);
			hangul_ic_push(ic,ic->jong); // fix order
			ic->laststate=3;
			return 0;
		    }
                    *cs = hangul_ic_commit(ic);
                    ic->jung=jamo;
                    hangul_ic_push(ic,jamo);
		    ic->laststate=2;
		    return -1;
                }
                // 종성이 계속 눌려진 채로 있고
                // 중성이 입력된 것이라면 
                else if (ic->jung && hangul_is_jongseong(ic->last) && ctyping) {
                    MyDebugPrint((TEXT("CON jung !!: %x\r\n"), jamo));
                    jong = ic->jong;

		    if (ic->jong != ic->last) {
                        WCHAR j1,j2;
			// 마지막 눌려졌던 종성을 분해하여, 한개는 앞에
			// 나머지 한개는 뒤의 음절에 붙인다.
			hangul_jongseong_dicompose(ic->jong, &j1, &j2);
			// 겹종성을 종성+종성으로 분해하는 루틴 필요 ?
			// hangul_jongseong_discompose2()?

			if (j1 && j2) {
			    jong = hangul_choseong_to_jongseong(j2);
			    hangul_ic_pop(ic);
			    hangul_ic_push(ic,j1);
			    ic->jong = j1;
			    *cs = hangul_ic_commit(ic);
			    ic->jung = jamo;
			    ic->jong = jong;
			    hangul_ic_push(ic,jamo);
			    hangul_ic_push(ic,jong);
			    ic->laststate = 3;
			    return -1;
			}
		    }

		    // 마지막 눌렸던 종성을 앞 음절에 붙이지 않고
		    // 뒷 음절에 붙인다.
                    hangul_ic_pop(ic);
                    ic->jong = 0;
                    *cs = hangul_ic_commit(ic);
                    ic->jung = jamo;
                    ic->jong = jong;
                    hangul_ic_push(ic,jamo);
                    hangul_ic_push(ic,jong);
                    ic->laststate = 3;
                    return -1;
                }
                *cs = hangul_ic_commit(ic);

                ic->jung = jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate = 2;
                return -1; // 앞의 초+중 두개를 commit
                break;
            default: // 종성 + 종성
		comb = 0;
		if (hangul_is_jongseong(ic->last))
		    comb = hangul_compose(ic->jong, jamo);
                if ( comb )
                {
                    // CP949인가 ?
                    // TODO KS X 1001영역만 입력되는 옵션도 넣음
                    if (ic->cho && ic->jung &&
                        hangul_jamo_to_syllable(ic->cho,ic->jung,comb))
                    {
                        ic->jong=comb;
                        hangul_ic_pop(ic);
                        hangul_ic_push(ic,comb);
                        ic->last=jamo;
                        ic->laststate=3;
                        return 0;
                    }
		    // XXX 종+종
                    ic->jong=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=3;
                    return 0;
		}
		{
                    *cs = hangul_ic_commit(ic);
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return -1;
		}
	        break;
            }
	}
    } else { // ic가 초기화되어 있다.
        switch (kind) {
        case 1:
            MyDebugPrint((TEXT("single cho automata: %x\r\n"), jamo));

            ic->cho=jamo;
            hangul_ic_push(ic,jamo);
            ic->laststate=1;
            return 1;
            break;
        case 2:
            MyDebugPrint((TEXT("single jung automata: %x\r\n"), jamo));

            ic->jung=jamo;
            hangul_ic_push(ic,jamo);
            ic->laststate=2;
            return 1;
            break;
        default:
            MyDebugPrint((TEXT("single jong automata: %x\r\n"), jamo));
            ic->jong=jamo;
            hangul_ic_push(ic,jamo);
            ic->laststate=3;
            return 1;
            break;
	}
    }
    return 0;
}

void PASCAL SetSoftKbdData(HWND hSoftKbdWnd)
{
    int key;
    LPSOFTKBDDATA lpSoftKbdData;
    HGLOBAL hsSoftKbdData;
#define SH  0x1000
static const WORD _VKeyTable[] = {
    VK_1 /* exclam */ |SH, VK_OEM_7 /* quotedbl */ |SH,
    VK_3 /* numbersign */ |SH, VK_4 /* dollar */ |SH,
    VK_5 /* percent */ |SH, VK_7 /* ampersand */ |SH,
    VK_OEM_7 /* apostrophe */, VK_9 /* parenleft */ |SH,
    VK_0 /* parenright */|SH, VK_8 /* asterisk */|SH,
    VK_OEM_PLUS|SH, VK_OEM_COMMA, VK_OEM_MINUS, VK_OEM_PERIOD,
    VK_OEM_2 /* slash */,
    VK_0, VK_1, VK_2, VK_3, VK_4, VK_5, VK_6, VK_7, VK_8, VK_9,
    VK_OEM_1 |SH /* colon */, VK_OEM_1 /* semicolon */, VK_OEM_PERIOD |SH,
    VK_OEM_PLUS /* equal */, VK_OEM_PERIOD /* greater */ |SH,
    VK_OEM_2 /* question */ |SH, VK_2 /* at */ |SH,
    VK_A |SH, VK_B |SH, VK_C |SH, VK_D |SH, VK_E |SH, VK_F |SH, VK_G |SH,
    VK_H |SH, VK_I |SH, VK_J |SH, VK_K |SH, VK_L |SH, VK_M |SH, VK_N |SH,
    VK_O |SH, VK_P |SH, VK_Q |SH, VK_R |SH, VK_S |SH, VK_T |SH, VK_U |SH,
    VK_V |SH, VK_W |SH, VK_X |SH, VK_Y |SH, VK_Z |SH,
    VK_OEM_4 /* bracketleft */, VK_OEM_5 /* backslash */,
    VK_OEM_6 /* bracketright */, VK_6 /* asciicircum */ | SH,
    VK_OEM_MINUS /* underscore */ | SH, VK_OEM_3 /* quoteleft */,
    VK_A, VK_B, VK_C, VK_D, VK_E, VK_F, VK_G, VK_H, VK_I, VK_J, VK_K, VK_L,
    VK_M, VK_N, VK_O, VK_P, VK_Q, VK_R, VK_S, VK_T, VK_U, VK_V, VK_W, VK_X,
    VK_Y, VK_Z,
    VK_OEM_4 /* braceleft  */ |SH, VK_OEM_5 /* bar */ |SH,
    VK_OEM_6 /* braceright */ |SH, VK_OEM_3 /* asciitilde */ |SH
};
    
    hsSoftKbdData = GlobalAlloc(GHND, sizeof(SOFTKBDDATA) * 2);
    if (!hsSoftKbdData) {
        //ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }

    lpSoftKbdData = (LPSOFTKBDDATA)GlobalLock(hsSoftKbdData);
    if (!lpSoftKbdData) {
        // can not draw soft keyboard window
        //ImmUnlockIMCC(lpIMC->hPrivate);
        return;
    }

    lpSoftKbdData->uCount = 2;

    for (key = 0; key < 94; key++) {
        WCHAR val;
        WORD bVKey;
	int shifted;
#if 0
        BYTE bVirtKey;
        bVirtKey = VirtKey48Map[i];

        if (!bVirtKey) {
            continue;
        }
#endif
	val = keyboard_table[key];
	bVKey = _VKeyTable[key];
	if (val >= 0x1100 && val <= 0x11c2)
        {
            // XXX
        }
	shifted = (bVKey & 0xff00) ? 1:0;
        lpSoftKbdData->wCode[shifted][bVKey & 0x00ff] = val;
    }

    SendMessage(hSoftKbdWnd, WM_IME_CONTROL, IMC_SETSOFTKBDDATA,
        (LPARAM)lpSoftKbdData);

    GlobalUnlock(hsSoftKbdData);

    // free storage for UI settings
    GlobalFree(hsSoftKbdData);
}

/*
 * ex: ts=8 sts=4 sw=4 noet
 */
