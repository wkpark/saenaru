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
 * $Saenaru: saenaru/src/hangul.c,v 1.30 2006/12/16 09:30:24 wkpark Exp $
 */

#include <windows.h>
#include <immdev.h>
#include "saenaru.h"
#include "vksub.h"

// Hangul Input context
HangulIC ic;

int hangul_automata2( HangulIC *ic, WCHAR jamo, LPMYSTR lcs, int *ncs);
int hangul_automata3( HangulIC *ic, WCHAR jamo, LPMYSTR lcs, int *ncs);
void PASCAL hangul_ic_init( HangulIC *ic );

/**
 * convert jamo to compatible jamo.
 * binary search version of the libhangul's func.
 */

WCHAR PASCAL
hangul_jamo_to_cjamo_full(WCHAR key)
{
    static const HangulCompose lookup[] = {
        { 0x1100, 0x3131 },
        { 0x1101, 0x3132 },
        { 0x1102, 0x3134 },
        { 0x1103, 0x3137 },
        { 0x1104, 0x3138 },
        { 0x1105, 0x3139 },
        { 0x1106, 0x3141 },
        { 0x1107, 0x3142 },
        { 0x1108, 0x3143 },
        { 0x1109, 0x3145 },
        { 0x110a, 0x3146 },
        { 0x110b, 0x3147 },
        { 0x110c, 0x3148 },
        { 0x110d, 0x3149 },
        { 0x110e, 0x314a },
        { 0x110f, 0x314b },
        { 0x1110, 0x314c },
        { 0x1111, 0x314d },
        { 0x1112, 0x314e },
        { 0x1114, 0x3165 },
        { 0x1115, 0x3166 },
        { 0x111a, 0x3140 },
        { 0x111c, 0x316e },
        { 0x111d, 0x3171 },
        { 0x111e, 0x3172 },
        { 0x1120, 0x3173 },
        { 0x1121, 0x3144 },
        { 0x1122, 0x3174 },
        { 0x1123, 0x3175 },
        { 0x1127, 0x3176 },
        { 0x1129, 0x3177 },
        { 0x112b, 0x3178 },
        { 0x112c, 0x3179 },
        { 0x112d, 0x317a },
        { 0x112e, 0x317b },
        { 0x112f, 0x317c },
        { 0x1132, 0x317d },
        { 0x1136, 0x317e },
        { 0x1140, 0x317f },
        { 0x1146, 0x3183 },
        { 0x1147, 0x3180 },
        { 0x114c, 0x3181 },
        { 0x1157, 0x3184 },
        { 0x1158, 0x3185 },
        { 0x1159, 0x3186 },
        { 0x115B, 0x3167 },
        { 0x115C, 0x3135 },
        { 0x115D, 0x3136 },
        { 0x115f, 0x3164 }, /* L Fill */
        { 0x1160, 0x3164 }, /* V Fill */
        { 0x1161, 0x314f },
        { 0x1162, 0x3150 },
        { 0x1163, 0x3151 },
        { 0x1164, 0x3152 },
        { 0x1165, 0x3153 },
        { 0x1166, 0x3154 },
        { 0x1167, 0x3155 },
        { 0x1168, 0x3156 },
        { 0x1169, 0x3157 },
        { 0x116a, 0x3158 },
        { 0x116b, 0x3159 },
        { 0x116c, 0x315a },
        { 0x116d, 0x315b },
        { 0x116e, 0x315c },
        { 0x116f, 0x315d },
        { 0x1170, 0x315e },
        { 0x1171, 0x315f },
        { 0x1172, 0x3160 },
        { 0x1173, 0x3161 },
        { 0x1174, 0x3162 },
        { 0x1175, 0x3163 },
        { 0x1184, 0x3187 },
        { 0x1185, 0x3188 },
        { 0x1188, 0x3189 },
        { 0x1191, 0x318a },
        { 0x1192, 0x318b },
        { 0x1194, 0x318c },
        { 0x119e, 0x318d },
        { 0x11a1, 0x318e },
        { 0x11aa, 0x3133 },
        { 0x11ac, 0x3135 },
        { 0x11ad, 0x3136 },
        { 0x11b0, 0x313a },
        { 0x11b1, 0x313b },
        { 0x11b2, 0x313c },
        { 0x11b3, 0x313d },
        { 0x11b4, 0x313e },
        { 0x11b5, 0x313f },
        { 0x11c7, 0x3167 },
        { 0x11c8, 0x3168 },
        { 0x11cc, 0x3169 },
        { 0x11ce, 0x316a },
        { 0x11d3, 0x316b },
        { 0x11d7, 0x316c },
        { 0x11d9, 0x316d },
        { 0x11dd, 0x316f },
        { 0x11df, 0x3170 },
        { 0x11f1, 0x3182 },
        { 0x11f2, 0x3183 },
        { 0xa964, 0x313a },
        { 0xa966, 0x316a },
        { 0xa968, 0x313b },
        { 0xa969, 0x313c },
        { 0xa96c, 0x313d },
        { 0xa971, 0x316f },
        { 0xd7cd, 0x3138 },
        { 0xd7e3, 0x3173 },
        { 0xd7e6, 0x3143 },
        { 0xd7e7, 0x3175 },
        { 0xd7e8, 0x3176 },
        { 0xd7ef, 0x317e },
        { 0xd7f9, 0x3149 },
    };

  int min, max, mid;
  int size = (sizeof (lookup) / sizeof ((lookup)[0]));

  /* binary search in table */
  min = 0;
  max = size - 1;

  while (max >= min) {
    mid = (min + max) / 2;
    if (lookup[mid].key < key)
      min = mid + 1;
    else if (lookup[mid].key > key)
      max = mid - 1;
    else {
      return lookup[mid].code;
    }
  }
  return key;
}


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
	return ch;
    }
    return table[ch - 0x1100];
}

#define hangul_is_choseong(ch)	(((ch) >= 0x1100 && (ch) <= 0x115f) || ((ch) >= 0xa960 && (ch) <= 0xa97c))
#define hangul_is_jungseong(ch)	(((ch) >= 0x1160 && (ch) <= 0x11a7) || ((ch) >= 0xd7b0 && (ch) <= 0xd7c6))
#define hangul_is_jongseong(ch)	(((ch) >= 0x11a8 && (ch) <= 0x11ff) || ((ch) >= 0xd7cb && (ch) <= 0xd7fb))
#define hangul_is_bangjum(ch)	((ch) == 0x302e || (ch) == 0x302f)
#define is_combining_mark(ch)	((ch) >= 0x0300 && (ch) <= 0x036f || \
	(ch) >= 0x1dc0 && (ch) <= 0x1de6 || (ch) >= 0x20d0 && (ch) <= 0x20f0)

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
	return ch;
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
	return ch;
    }
    return table[ch - 0x11a8];
}

/**
 * full featured binary search version.
 * support Unicode 5.2
 *
 * all empty entries are not removed yet. FIXME
 */

WCHAR PASCAL
hangul_choseong_to_jongseong_full(WCHAR ch)
{
static const HangulCompose lookup[] = {
    { 0x1100, 0x11a8 }, /* choseong kiyeok      */
    { 0x1101, 0x11a9 }, /* choseong ssangkiyeok */
    { 0x1102, 0x11ab }, /* choseong nieun       */
    { 0x1103, 0x11ae }, /* choseong tikeut      */
    { 0x1104, 0x0000 }, /* choseong ssangtikeut */
    { 0x1105, 0x11af }, /* choseong rieul       */
    { 0x1106, 0x11b7 }, /* choseong mieum       */
    { 0x1107, 0x11b8 }, /* choseong pieup       */
    { 0x1108, 0x0000 }, /* choseong ssangpieup  */
    { 0x1109, 0x11ba }, /* choseong sios        */
    { 0x110a, 0x11bb }, /* choseong ssangsios   */
    { 0x110b, 0x11bc }, /* choseong ieung       */
    { 0x110c, 0x11bd }, /* choseong cieuc       */
    { 0x110d, 0x0000 }, /* choseong ssangcieuc  */
    { 0x110e, 0x11be }, /* choseong chieuch     */
    { 0x110f, 0x11bf }, /* choseong khieukh     */
    { 0x1110, 0x11c0 }, /* choseong thieuth     */
    { 0x1111, 0x11c1 }, /* choseong phieuph     */
    { 0x1112, 0x11c2 }, /* choseong hieuh       */
    { 0x1113, 0x11c5 },	/* choseong nieun-kiyeok          */
    { 0x1114, 0x11ff },	/* choseong ssangnieun            */
    { 0x1115, 0x11c6 },	/* choseong nieun-tikeut          */
    { 0x1116, 0x0000 },	/* choseong nieun-pieup           */
    { 0x1117, 0x11ca },	/* choseong tikeut-kiyeok         */
    { 0x1118, 0x11cd },	/* choseong rieul-nieun           */
    { 0x1119, 0x11d0 },	/* choseong ssangrieul            */
    { 0x111a, 0x0000 },	/* choseong rieul-hieuh           */
    { 0x111b, 0xd7dd },	/* choseong kapyeounrieul         */
    { 0x111c, 0x11dc },	/* choseong mieum-pieup           */
    { 0x111d, 0x11e2 },	/* choseong kapyeounmieum         */
    { 0x111e, 0x0000 },	/* choseong pieup-kiyeok          */
    { 0x111f, 0x0000 },	/* choseong pieup-nieun           */
    { 0x1120, 0xd7e3 },	/* choseong pieup-tikeut          */
    { 0x1121, 0x0000 },	/* choseong pieup-sios            */
    { 0x1122, 0x0000 },	/* choseong pieup-sios-kiyeok     */
    { 0x1123, 0x0000 },	/* choseong pieup-sios-tikeut     */
    { 0x1124, 0x0000 },	/* choseong pieup-sios-pieup      */
    { 0x1125, 0x0000 },	/* choseong pieup-ssangsios       */
    { 0x1126, 0x0000 },	/* choseong pieup-sios-cieuc      */
    { 0x1127, 0xd7e8 },	/* choseong pieup-cieuc           */
    { 0x1128, 0xd7e9 },	/* choseong pieup-chieuch         */
    { 0x1129, 0x0000 },	/* choseong pieup-thieuth         */
    { 0x112a, 0x11ea },	/* choseong pieup-phieuph         */
    { 0x112b, 0x0000 },	/* choseong kapyeounpieup         */
    { 0x112c, 0x0000 },	/* choseong kapyeounssangpieup    */
    { 0x112d, 0x11e7 },	/* choseong sios-kiyeok           */
    { 0x112e, 0x0000 },	/* choseong sios-nieun            */
    { 0x112f, 0x11e8 },	/* choseong sios-tikeut           */
    { 0x1130, 0x11e9 },	/* choseong sios-rieul            */
    { 0x1131, 0xd7ea },	/* choseong sios-mieum            */
    { 0x1132, 0x11ea },	/* choseong sios-pieup            */
    { 0x1133, 0x0000 },	/* choseong sios-pieup-kiyeok     */
    { 0x1134, 0x0000 },	/* choseong sios-ssangsios        */
    { 0x1135, 0x0000 },	/* choseong sios-ieung            */
    { 0x1136, 0xd7ef },	/* choseong sios-cieuc            */
    { 0x1137, 0xd7f0 },	/* choseong sios-chieuch          */
    { 0x1138, 0x0000 },	/* choseong sios-khieukh          */
    { 0x1139, 0xd7f1 },	/* choseong sios-thieuth          */
    { 0x113a, 0x0000 },	/* choseong sios-phieuph          */
    { 0x113b, 0xd7f2 },	/* choseong sios-hieuh		  */
    { 0x113c, 0x0000 },	/* choseong chitueumsios          */
    { 0x113d, 0x0000 },	/* choseong chitueumssangsios     */
    { 0x113e, 0x0000 },	/* choseong ceongchieumsios       */
    { 0x113f, 0x0000 },	/* choseong ceongchieumssangsios  */
    { 0x1140, 0x11eb },	/* choseong pansios		  */
    { 0x1141, 0x11ec },	/* choseong ieung-kiyeok          */
    { 0x1142, 0x0000 },	/* choseong ieung-tikeut          */
    { 0x1143, 0xd7f5 },	/* choseong ieung-mieum           */
    { 0x1144, 0x0000 },	/* choseong ieung-pieup           */
    { 0x1145, 0x11f1 },	/* choseong ieung-sios            */
    { 0x1146, 0x11f2 },	/* choseong ieung-pansios         */
    { 0x1147, 0x11ee },	/* choseong ssangieung            */
    { 0x1148, 0x0000 },	/* choseong ieung-cieuc           */
    { 0x1149, 0x0000 },	/* choseong ieung-chieuch         */
    { 0x114a, 0x0000 },	/* choseong ieung-thieuth         */
    { 0x114b, 0x0000 },	/* choseong ieung-phieuph         */
    { 0x114c, 0x11f0 },	/* choseong yesieung              */
    { 0x114d, 0x0000 },	/* choseong cieuc-ieung           */
    { 0x114e, 0x0000 },	/* choseong chitueumcieuc         */
    { 0x114f, 0x0000 },	/* choseong chitueumssangcieuc    */
    { 0x1150, 0x0000 },	/* choseong ceongchieumcieuc      */
    { 0x1151, 0x0000 },	/* choseong ceongchieumssangcieuc */
    { 0x1152, 0x0000 },	/* choseong chieuch-khieukh       */
    { 0x1153, 0x0000 },	/* choseong chieuch-hieuh         */
    { 0x1154, 0x0000 },	/* choseong chitueumchieuch       */
    { 0x1155, 0x0000 },	/* choseong ceongchieumchieuch    */
    { 0x1156, 0x11f3 },	/* choseong phieuph-pieup         */
    { 0x1157, 0x11f4 },	/* choseong kapyeounphieuph       */
    { 0x1158, 0x0000 },	/* choseong ssanghieuh            */
    { 0x1159, 0x11f9 },	/* choseong yeorinhieuh           */
    { 0x115a, 0x0000 },	/* choseong kiyeok-tikeut         */
    { 0x115b, 0x0000 },	/* choseong nieun-sios            */
    { 0x115c, 0x11ac },	/* choseong nieun-cieuc           */
    { 0x115d, 0x11ad },	/* choseong nieun-hieuh           */
    { 0x115e, 0x11cb },	/* choseong tikeut-rieul          */
    { 0x115f, 0x0000 },	/* choseong filler                */

    { 0xa960, 0x0000 },	/* choseong tikeut-mieum          */
    { 0xa961, 0xd7cf },	/* choseong tikeut-pieup          */
    { 0xa962, 0xd7d0 },	/* choseong tikeut-sios           */
    { 0xa963, 0xd7d2 },	/* choseong tikeut-cieuc          */
    { 0xa964, 0x11b0 },	/* choseong rieul-kiyeok          */
    { 0xa965, 0xd7d5 },	/* choseong rieul-ssangkiyeok     */
    { 0xa966, 0x11ce },	/* choseong rieul-tikeut          */
    { 0xa967, 0x0000 },	/* choseong rieul-ssangtikeut     */
    { 0xa968, 0x0000 },	/* choseong rieul-mieum           */
    { 0xa969, 0x11b1 },	/* choseong rieul-pieup           */
    { 0xa96a, 0x11b2 },	/* choseong rieul-ssangpieup      */
    { 0xa96b, 0x11d5 },	/* choseong rieul-kapyeounpieup   */
    { 0xa96c, 0x0000 },	/* choseong rieul-sios            */
    { 0xa96d, 0x0000 },	/* choseong rieul-cieuc           */
    { 0xa96e, 0x11d8 },	/* choseong rieul-khieukh         */
    { 0xa96f, 0x11da },	/* choseong mieum-kiyeok          */
    { 0xa970, 0x0000 },	/* choseong mieum-tikeut          */
    { 0xa971, 0x11dd },	/* choseong mieum-sios            */
    { 0xa972, 0x0000 },	/* choseong pieup-sios-thieuth    */
    { 0xa973, 0x0000 },	/* choseong pieup-khieukh         */
    { 0xa974, 0x11e5 },	/* choseong pieup-hieuh           */
    { 0xa975, 0x0000 },	/* choseong ssangsios-pieup       */
    { 0xa976, 0x0000 },	/* choseong ieung-rieul           */
    { 0xa977, 0x0000 },	/* choseong ieung-hieuh           */
    { 0xa978, 0x0000 },	/* choseong ssangcieuc-hieuh      */
    { 0xa979, 0x0000 },	/* choseong ssangthieuth          */
    { 0xa97a, 0x0000 },	/* choseong phieuph-hieuh         */
    { 0xa97b, 0x0000 },	/* choseong hieuh-sios            */
    { 0xa97c, 0x0000 }	/* choseong ssangyeorinhieuh      */
};
    int min, max, mid;
    int size = (sizeof (lookup) / sizeof ((lookup)[0]));

    /* binary search in table */
    min = 0;
    max = size - 1;

    while (max >= min) {
	mid = (min + max) / 2;
	if (lookup[mid].key < ch)
	    min = mid + 1;
	else if (lookup[mid].key > ch)
	    max = mid - 1;
	else {
	    return lookup[mid].code;
	}
    }
    return ch;
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

/**
 * support Unicode 5.2
 */

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
    { 0,      0x1112 }, /* jong hieuh         = cho  hieuh                */
    { 0x11a8, 0x1105 }, /* jong ㄱㄹ          = jong kiyeok + cho ㄹ  */
    { 0x11aa, 0x1100 }, /* jong ㄱㅅㄱ	      = jong kiyeok-sios + cho kiyeok  */
    { 0x11ab, 0x1100 }, /* jong ㄴㄱ          = jong ㄴ + cho ㄱ */
    { 0x11ab, 0x1103 }, /* jong ㄴㄷ          = jong ㄴ + cho ㄷ */
    { 0x11ab, 0x1109 }, /* jong ㄴㅅ          = jong ㄴ + cho ㅅ */
    { 0x11ab, 0x1140 }, /* jong ㄴㅿ          = jong ㄴ + cho ㅿ */
    { 0x11ab, 0x1110 }, /* jong ㄴㅌ          = jong ㄴ + cho ㅌ */
    { 0x11ae, 0x1100 }, /* jong ㄷㄱ          = jong ㄷ + cho ㄱ */
    { 0x11ae, 0x1105 }, /* jong ㄷㄹ          = jong ㄷ + cho ㄹ */
    { 0x11b0, 0x1109 }, /* jong ㄹㄱㅅ        = jong ㄹㄱ + cho ㅅ */
    { 0x11af, 0x1102 }, /* jong ㄹㄴ          = jong ㄹ + cho ㄴ */
    { 0x11af, 0x1103 }, /* jong ㄹㄷ          = jong ㄹ + cho ㄷ */
    { 0x11ce, 0x1112 }, /* jong ㄹㄷㅎ        = jong ㄹㄷ + cho ㅎ */
    { 0x11af, 0x1105 }, /* jong ㄹㄹ          = jong ㄹ + cho ㄹ */
    { 0x11b1, 0x1100 }, /* jong ㄹㅁㄱ        = jong ㄻ + cho ㄱ */
    { 0x11b1, 0x1109 }, /* jong ㄹㅁㅅ        = jong ㄻ + cho ㅅ */
    { 0x11b2, 0x1109 }, /* jong ㄹㅂㅅ        = jong ㄼ + cho ㅅ */
    { 0x11b2, 0x1112 }, /* jong ㄹㅂㅎ        = jong ㄼ + cho ㅎ */
    { 0x11b2, 0x110b }, /* jong ㄹㅸ          = jong ㄼ + cho ㅇ */
    { 0x11b3, 0x1109 }, /* jong ㄹㅅㅅ        = jong ㄽ + cho ㅅ */
    { 0x11af, 0x1140 }, /* jong ㄹㅿ          = jong ㄹ + cho ㅿ */
    { 0x11af, 0x110f }, /* jong ㄹㅋ          = jong ㄹ + cho ㅋ */
    { 0x11af, 0x1159 }, /* jong ㄹㆆ          = jong ㄹ + cho ㆆ */
    { 0x11b7, 0x1100 }, /* jong ㅁㄱ          = jong ㅁ + cho ㄱ */
    { 0x11b7, 0x1105 }, /* jong ㅁㄹ          = jong ㅁ + cho ㄹ */
    { 0x11b7, 0x1107 }, /* jong ㅁㅂ          = jong ㅁ + cho ㅂ */
    { 0x11b7, 0x1109 }, /* jong ㅁㅅ          = jong ㅁ + cho ㅅ */
    { 0x11dd, 0x1109 }, /* jong ㅁㅅㅅ        = jong ㅁㅅ + cho ㅅ */
    { 0x11b7, 0x1140 }, /* jong ㅁㅿ          = jong ㅁ + cho ㅿ */
    { 0x11b7, 0x110e }, /* jong ㅁㅊ          = jong ㅁ + cho ㅊ */
    { 0x11b7, 0x1112 }, /* jong ㅁㅎ          = jong ㅁ + cho ㅎ */
    { 0x11b7, 0x110b }, /* jong ㅱ            = jong ㅁ + cho ㅇ */
    { 0x11b8, 0x1105 }, /* jong ㅂㄹ          = jong ㅂ + cho ㄹ */
    { 0x11b8, 0x1111 }, /* jong ㅂㅍ          = jong ㅂ + cho ㅍ */
    { 0x11b8, 0x1112 }, /* jong ㅂㅎ          = jong ㅂ + cho ㅎ */
    { 0x11b8, 0x110b }, /* jong ㅸ            = jong ㅂ + cho ㅇ */
    { 0x11ba, 0x1100 }, /* jong ㅅㄱ          = jong ㅅ + cho ㄱ */
    { 0x11ba, 0x1103 }, /* jong ㅅㄷ          = jong ㅅ + cho ㄷ */
    { 0x11ba, 0x1105 }, /* jong ㅅㄹ          = jong ㅅ + cho ㄹ */
    { 0x11ba, 0x1107 }, /* jong ㅅㅂ          = jong ㅅ + cho ㅂ */
    { 0,      0x1140 }, /* jong ㅿ            = cho ㅿ */
    { 0x11bc, 0x1100 }, /* jong ㅇㄱ          = jong ㅇ + cho ㄱ */
    { 0x11ec, 0x1100 }, /* jong ㅇㄲ          = jong ㅇㄱ + cho ㄱ */
    { 0x11bc, 0x110b }, /* jong ㅇㅇ          = jong ㅇ + cho ㅇ */
    { 0x11bc, 0x110f }, /* jong ㅇㅋ          = jong ㅇ + cho ㅋ */
    { 0,      0x110b }, /* jong ㆁ            = cho ㅇ */
    { 0x11bc, 0x1109 }, /* jong ㅇㅅ          = jong ㅇ + cho ㅅ */
    { 0x11bc, 0x1140 }, /* jong ㅇㅿ          = jong ㅇ + cho ㅿ */
    { 0x11c1, 0x1107 }, /* jong ㅍㅂ          = jong ㅍ + cho ㅂ */
    { 0x11c1, 0x110b }, /* jong ㆄ            = jong ㅍ + cho ㅇ */
    { 0x11c2, 0x1102 }, /* jong ㅎㄴ          = jong ㅎ + cho ㄴ */
    { 0x11c2, 0x1105 }, /* jong ㅎㄹ          = jong ㅎ + cho ㄹ */
    { 0x11c2, 0x1106 }, /* jong ㅎㅁ          = jong ㅎ + cho ㅁ */
    { 0x11c2, 0x1107 }, /* jong ㅎㅂ          = jong ㅎ + cho ㅂ */
    { 0,      0x1159 }, /* jong ㆆ            = cho ㆆ */
    { 0x11a8, 0x1102 }, /* jong ㄱㄴ          = jong ㄱ + cho ㄴ */
    { 0x11a8, 0x1107 }, /* jong ㄱㅂ          = jong ㄱ + cho ㅂ */
    { 0x11a8, 0x110e }, /* jong ㄱㅊ          = jong ㄱ + cho ㅊ */
    { 0x11a8, 0x110f }, /* jong ㄱㅋ          = jong ㄱ + cho ㅋ */
    { 0x11a8, 0x1112 }, /* jong ㄱㅎ          = jong ㄱ + cho ㅎ */
    { 0x11ab, 0x1102 }  /* jong ㄴㄴ          = jong ㄴ + cho ㄴ */
};

static const WCHAR table_ext_B[][2] = {
    { 0x11ab, 0x1105 }, /* jong ㄴㄹ          = jong ㄴ + cho ㄹ */
    { 0x11ab, 0x110e }, /* jong ㄴㅊ          = jong ㄴ + cho ㅊ */
    { 0x11ae, 0x1103 }, /* jong ㄸ            = jong ㄷ + cho ㄷ */
    { 0xd7cd, 0x1107 }, /* jong ㄸㅂ          = jong ㄸ + cho ㅂ */
    { 0x11ae, 0x11b8 }, /* jong ㄷㅂ          = jong ㄷ + cho ㅂ */
    { 0x11ae, 0x11ba }, /* jong ㄷㅅ          = jong ㄷ + cho ㅅ */
    { 0xd7d0, 0x1100 }, /* jong ㄷㅅㄱ        = jong ㄷㅅ + cho ㄱ */
    { 0x11ae, 0x110c }, /* jong ㄷㅈ          = jong ㄷ + cho ㅈ */
    { 0x11ae, 0x110e }, /* jong ㄷㅊ          = jong ㄷ + cho ㅊ */
    { 0x11ae, 0x11c0 }, /* jong ㄷㅌ          = jong ㄷ + cho ㅌ */

    { 0x11ae, 0x1100 }, /* jong ㄺㄱ          = jong ㄺ + cho ㄱ */
    { 0x11ae, 0x1112 }, /* jong ㄺㅎ          = jong ㄺ + cho ㅎ */
    { 0x11d0, 0x110f }, /* jong ㄹㄹㅋ        = jong ㄹㄹ + cho ㅋ */
    { 0x11b1, 0x1112 }, /* jong ㄹㅁㅎ        = jong ㄻ + cho ㅎ */
    { 0x11b2, 0x1103 }, /* jong ㄹㅂㄷ        = jong ㄼ + cho ㄷ */
    { 0x11b2, 0x1111 }, /* jong ㄹㅂㅍ        = jong ㄼ + cho ㅍ */
    { 0x11af, 0x114c }, /* jong ㄹㆁ          = jong ㄹ + cho ㆁ */
    { 0x11d9, 0x1112 }, /* jong ㅭㅎ          = jong ㅭ + cho ㅎ */
    { 0x11af, 0x110b }, /* jong ㄹㅇ          = jong ㄹ + cho ㅇ */
    { 0x11b7, 0x1102 }, /* jong ㅁㄴ          = jong ㅁ + cho ㄴ */
    { 0xd7de, 0x1102 }, /* jong ㅁㄴㄴ        = jong ㅁㄴ + cho ㄴ */
    { 0x11b7, 0x1106 }, /* jong ㅁㅁ          = jong ㅁ + cho ㅁ */
    { 0x11b7, 0x1109 }, /* jong ㅁㅄ          = jong ㅁㅂ + cho ㅅ */
    { 0x11b7, 0x110c }, /* jong ㅁㅈ          = jong ㅁ + cho ㅈ */
    { 0x11b8, 0x1103 }, /* jong ㅂㄷ          = jong ㅂ + cho ㄷ */
    { 0x11b8, 0x1111 }, /* jong ㅂㄿ          = jong ㅂㄹ + cho ㅍ */
    { 0x11b8, 0x1106 }, /* jong ㅂㅁ          = jong ㅂ + cho ㅁ */
    { 0x11b8, 0x1107 }, /* jong ㅃ            = jong ㅂ + cho ㅂ */
    { 0x11b9, 0x1103 }, /* jong ㅄㄷ          = jong ㅄ + cho ㄷ */
    { 0x11b8, 0x110c }, /* jong ㅂㅈ          = jong ㅂ + cho ㅈ */
    { 0x11b8, 0x110e }, /* jong ㅂㅊ          = jong ㅂ + cho ㅊ */
    { 0x11ba, 0x1106 }, /* jong ㅅㅁ          = jong ㅅ + cho ㅁ */
    { 0x11ba, 0x110b }, /* jong ㅅㅸ          = jong ㅅㅂ + cho ㅇ */
    { 0x11bb, 0x1100 }, /* jong ㅆㄱ          = jong ㅆ + cho ㄱ */
    { 0x11bb, 0x1103 }, /* jong ㅆㄷ          = jong ㅆ + cho ㄷ */
    { 0x11ba, 0x1140 }, /* jong ㅅㅿ          = jong ㅅ + cho ㅿ */
    { 0x11ba, 0x110c }, /* jong ㅅㅈ          = jong ㅅ + cho ㅈ */
    { 0x11ba, 0x110e }, /* jong ㅅㅊ          = jong ㅅ + cho ㅊ */
    { 0x11ba, 0x1110 }, /* jong ㅅㅌ          = jong ㅅ + cho ㅌ */
    { 0x11ba, 0x1112 }, /* jong ㅅㅎ          = jong ㅅ + cho ㅎ */
    { 0x11eb, 0x1107 }, /* jong ㅿㅂ          = jong ㅿ + cho ㅂ */
    { 0x11eb, 0x110b }, /* jong ㅿㅸ          = jong ㅿㅂ + cho ㅇ */
    { 0x11f0, 0x1106 }, /* jong ㆁㅁ          = jong ㆁ + cho ㅁ */
    { 0x11f0, 0x1112 }, /* jong ㆁㅎ          = jong ㆁ + cho ㅎ */
    { 0x11bd, 0x1107 }, /* jong ㅈㅂ          = jong ㅈ + cho ㅂ */
    { 0xd7f7, 0x1107 }, /* jong ㅈㅃ          = jong ㅈㅂ + cho ㅂ */
    { 0x11bd, 0x110c }, /* jong ㅈㅈ          = jong ㅈ + cho ㅈ */
    { 0x11c1, 0x1109 }, /* jong ㅍㅅ          = jong ㅍ + cho ㅅ */
    { 0x11c1, 0x1110 }, /* jong ㅍㅌ          = jong ㅍ + cho ㅌ */
};

    *jong  = 0;
    *cho = 0;

    if ( ch >= 0x11a8 && ch <= 0x11ff ) {
	*jong = table[ch - 0x11a8][0];
	*cho  = table[ch - 0x11a8][1];
	return;
    }

    if ( ch >= 0xd7cb && ch <= 0xd7fb ) {
	*jong = table_ext_B[ch - 0xd7cb][0];
	*cho  = table_ext_B[ch - 0xd7cb][1];
	return;
    }

    return;
}

/**
 * support Unicode 5.2
 */

void PASCAL
hangul_jungseong_dicompose(WCHAR ch, WCHAR* jung, WCHAR* jung2)
{
static const WCHAR table[][2] = {
    { 0x1163, 0x1175 }, /* ya     + i	= yae		*/
    { 0x1165, 0x0000 }, /* */
    { 0x1166, 0x0000 }, /* */
    { 0x1167, 0x0000 }, /* */
    { 0x1167, 0x1175 }, /* yeo    + i	= ye		*/
    { 0x1169, 0x0000 }, /* */
    { 0x1169, 0x1161 }, /* o      + a	= wa		*/
    { 0x1169, 0x1162 }, /* o      + ae	= wae		*/
    { 0x1169, 0x1175 }, /* o      + i	= oe		*/
    { 0x116d, 0x0000 }, /* */
    { 0x116e, 0x0000 }, /* */
    { 0x116e, 0x1165 }, /* u      + eo	= weo		*/

    { 0x116e, 0x1166 }, /* u      + e	= we		*/
    { 0x116e, 0x1175 }, /* u      + i	= wi		*/
    { 0x1172, 0x0000 }, /* */
    { 0x1173, 0x0000 }, /* */
    { 0x1173, 0x1175 }, /* eu     + i	= yi		*/
    { 0x1175, 0x0000 }, /* i */
    { 0x1161, 0x1169 }, /* a      + o	= a-o		*/
    { 0x1161, 0x116e }, /* a      + u	= a-u		*/
    { 0x1163, 0x1169 }, /* ya     + o	= ya-o		*/
    { 0x1163, 0x116e }, /* ya     + yo	= ya-yo		*/
    { 0x1165, 0x1169 }, /* eo     + o	= eo-o		*/
    { 0x1165, 0x116e }, /* eo     + u	= eo-u		*/
    { 0x1165, 0x1173 }, /* eo     + eu	= eo-eu		*/
    { 0x1167, 0x1169 }, /* yeo    + o	= yeo-o		*/
    { 0x1167, 0x116e }, /* yeo    + u	= yeo-u		*/
    { 0x1169, 0x1165 }, /* o      + eo	= o-eo		*/
  
    { 0x1169, 0x1166 }, /* o      + e	= o-e		*/
    { 0x1169, 0x1168 }, /* o      + ye	= o-ye		*/
    { 0x1169, 0x1169 }, /* o      + o	= o-o		*/
    { 0x1169, 0x116e }, /* o      + u	= o-u		*/
    { 0x116d, 0x1163 }, /* yo     + ya	= yo-ya		*/
    { 0x116d, 0x1164 }, /* yo     + yae	= yo-yae	*/
    { 0x116d, 0x1167 }, /* yo     + yo	= yo-yeo	*/
    { 0x116d, 0x1169 }, /* yo     + o	= yo-o		*/
    { 0x116d, 0x1175 }, /* yo	+ i	= yo-i		*/
    { 0x116e, 0x1161 }, /* u	+ a	= u-a		*/
    { 0x116e, 0x1162 }, /* u	+ ae	= u-ae		*/
    { 0x116f, 0x1173 }, /* weo	+ eu	= weo-eu	*/
    { 0x116e, 0x1168 }, /* u	+ ye	= u-ye		*/
    { 0x116e, 0x116e }, /* u	+ u	= u-u		*/
    { 0x1172, 0x1161 }, /* yu	+ a	= yu-a		*/
    { 0x1172, 0x1165 }, /* yu	+ eo	= yu-eo		*/
  
    { 0x1172, 0x1166 }, /* yu	+ e	= yu-e		*/
    { 0x1172, 0x1167 }, /* yu	+ yeo	= yu-yeo	*/
    { 0x1172, 0x1168 }, /* yu	+ ye	= yu-ye		*/
    { 0x1172, 0x116e }, /* yu	+ u	= yu-u		*/
    { 0x1172, 0x1175 }, /* yu	+ i	= yu-i		*/
    { 0x1173, 0x116e }, /* eu	+ u	= eu-u		*/
    { 0x1173, 0x1173 }, /* eu	+ eu	= eu-eu		*/
    { 0x1174, 0x116e }, /* yi	+ u	= yi-u		*/
    { 0x1175, 0x1161 }, /* i	+ a	= i-a		*/
    { 0x1175, 0x1163 }, /* i	+ ya	= i-ya		*/
    { 0x1175, 0x1169 }, /* i	+ o	= i-o		*/
    { 0x1175, 0x116e }, /* i	+ u	= i-u		*/
    { 0x1175, 0x1173 }, /* i	+ eu	= i-eu		*/
    { 0x1175, 0x119e }, /* i	+ araea	= i-araea	*/
    { 0x119e, 0x0000 }, /* araea */
    { 0x119e, 0x1165 }, /* araea	+ eo	= araea-eo	*/
  
    { 0x119e, 0x116e }, /* araea	+ u	= araea-u	*/
    { 0x119e, 0x1175 }, /* araea	+ i	= araea-i	*/
    { 0x119e, 0x119e }, /* araea	+ araea	= araea-araea	*/
    { 0x1161, 0x1173 }, /* a	+ eu	= a-eu		*/
    { 0x1163, 0x116e }, /* ya	+ u	= ya-u		*/
    { 0x1167, 0x1163 }, /* yeo	+ ya	= yeo-ya	*/
    { 0x1169, 0x1163 }, /* o	+ ya	= o-ya		*/
    { 0x1169, 0x1164 }, /* o	+ yae	= o-yae		*/
};

static const WCHAR table_ext_B[][2] = {
    { 0x1169, 0x1167 }, /* o	+ yeo	= o-yeo		*/
    { 0x1182, 0x1175 }, /* o-o	+ i	= o-o-i		*/
    { 0x116d, 0x1161 }, /* yo	+ a	= yo-a		*/
    { 0x116d, 0x1166 }, /* yo	+ ae	= yo-ae		*/
    { 0x116d, 0x1165 }, /* yo	+ eo	= yo-eo		*/
    { 0x116e, 0x1167 }, /* u	+ yeo	= u-yeo		*/
    { 0x1171, 0x1175 }, /* wi	+ i	= wi-i		*/
    { 0x1194, 0x1162 }, /* u	+ ae	= u-ae		*/
    { 0x1172, 0x1169 }, /* u	+ o	= u-o		*/
    { 0x1173, 0x1161 }, /* eu	+ a	= eu-a		*/
    { 0x1173, 0x1165 }, /* eu	+ eo	= eu-eo		*/
    { 0x1173, 0x1166 }, /* eu	+ ae	= eu-ae		*/
    { 0x1173, 0x1169 }, /* eu	+ o	= eu-o		*/
    { 0x1199, 0x1169 }, /* i-ya	+ o	= i-ya-o	*/
    { 0x1175, 0x1164 }, /* i	+ yae	= i-yae		*/
    { 0x1175, 0x1167 }, /* i	+ yeo	= i-yeo		*/
    { 0x1175, 0x1168 }, /* i	+ ye	= i-ye		*/
    { 0x119a, 0x1173 }, /* i-o	+ eu	= i-eu		*/
    { 0x1175, 0x116d }, /* i	+ yo	= i-yo		*/
    { 0x1175, 0x116e }, /* i	+ u	= i-u		*/
    { 0x1175, 0x1175 }, /* i	+ i	= i-i		*/
    { 0x119e, 0x1161 }, /* araea	+ a	= araea-a	*/
    { 0x119e, 0x1166 }, /* araea	+ ae	= araea-ae	*/
};

    *jung  = 0;
    *jung2 = 0;

    if ( ch >= 0x1164 && ch <= 0x11a7 ) {
	*jung  = table[ch - 0x1164][0];
	*jung2 = table[ch - 0x1164][1];
    } else if ( ch >= 0xd7b0 && ch <= 0xd7c6 ) {
	*jung  = table_ext_B[ch - 0xd7b0][0];
	*jung2 = table_ext_B[ch - 0xd7b0][1];
    }
    return;
}

static const WCHAR hangul_base    = 0xac00;
static const WCHAR choseong_base  = 0x1100;
static const WCHAR jungseong_base = 0x1161;
static const WCHAR jongseong_base = 0x11a7;
static const int njungseong = 21;
static const int njongseong = 28;

WCHAR PASCAL
hangul_jamo_to_syllable(WCHAR choseong, WCHAR jungseong, WCHAR jongseong)
{
    WCHAR ch;

    /* we use 0x11a7 like as a Jongseong filler internally */
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

/**
 * @ingroup hangulctype
 * @brief 한글 음절 인지 확인
 * @param c UCS4 코드 값
 * @return @a c가 한글 음절 코드이면 true, 그 외에는 false
 *
 * 이 함수는 @a c로 주어진 UCS4 코드가 현대 한글 음절에 해당하는지
 * 확인한다.
 */
BOOL
hangul_is_syllable(WCHAR c)
{
    return c >= 0xac00 && c <= 0xd7a3;
}

BOOL
hangul_is_cjamo(WCHAR c)
{
    return c >= 0x3131 && c <= 0x318e;
}

/**
 * @ingroup hangulctype
 * @brief 음절을 자모로 분해
 * @param syllable 분해할 음절
 * @retval choseong 음절에서 초성 부분의 코드
 * @retval jungseong 음절에서 중성 부분의 코드
 * @retval jongseong 음절에서 종성 부분의 코드, 종성이 없으면 0을 반환한다
 * @return 없음
 *
 * 이 함수는 @a syllable 로 주어진 음절 코드를 분해하여 자모 코드를 반환한다.
 * 반환하는 값은 @a choseong, @a jungseong, @a jongseong 의 포인터에 대입하여
 * 리턴한다. 종성이 없는 음절인 경우에는 @a jongseong 에 0을 반환한다.
 */
void PASCAL
hangul_syllable_to_jamo(WCHAR syllable,
			WCHAR* choseong,
			WCHAR* jungseong,
			WCHAR* jongseong)
{
    UINT syl;

    if (jongseong != NULL)
	*jongseong = 0;
    if (jungseong != NULL)
	*jungseong = 0;
    if (choseong != NULL)
	*choseong = 0;

    if (!hangul_is_syllable(syllable))
	return;

    syl = (UINT) syllable;

    syl -= hangul_base;
    if (jongseong != NULL) {
	if (syl % njongseong != 0)
	    *jongseong = jongseong_base + syl % njongseong;
    }
    syl /= njongseong;

    if (jungseong != NULL) {
	*jungseong = jungseong_base + syl % njungseong;
    }
    syl /= njungseong;

    if (choseong != NULL) {
	*choseong = choseong_base + syl;
    }
}


WCHAR PASCAL
hangul_jamo_to_cjamo(WCHAR jamo)
{
    if (jamo >= 0x1100 && jamo <= 0x1112)
        return hangul_choseong_to_cjamo(jamo);
    if (jamo >= 0x1161 && jamo <= 0x1175)
        return hangul_jungseong_to_cjamo(jamo);
    if (jamo >= 0x11a8 && jamo <= 0x11c2)
        return hangul_jongseong_to_cjamo(jamo);

    return hangul_jamo_to_cjamo_full(jamo);

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

#define ______ 0x0000
typedef struct _SCANCODE_MAP {
  unsigned short code[2];
} SCANCODE_MAP;

const SCANCODE_MAP scan2qwerty[] = {
  { 0x0031, /* GDK_1 */ 0x0021 /* GDK_exclam */ },
  { 0x0032, /* GDK_2 */ 0x0040 /* GDK_at */ },
  { 0x0033, /* GDK_3 */ 0x0023 /* GDK_numbersign */ },
  { 0x0034, /* GDK_4 */ 0x0024 /* GDK_dollar */ },
  { 0x0035, /* GDK_5 */ 0x0025 /* GDK_percent */ },
  { 0x0036, /* GDK_6 */ 0x005e /* GDK_asciicircum */ },
  { 0x0037, /* GDK_7 */ 0x0026 /* GDK_ampersand */ },
  { 0x0038, /* GDK_8 */ 0x002a /* GDK_asterisk */ },
  { 0x0039, /* GDK_9 */ 0x0028 /* GDK_parenleft */ },
  { 0x0030, /* GDK_0 */ 0x0029 /* GDK_parenright */ },
  { 0x002d, /* GDK_minus */ 0x005f /* GDK_underscore */ },
  { 0x003d, /* GDK_equal */ 0x002b /* GDK_plus */ },
  { ______, ______ },
  { ______, ______ },
  { 0x0071, /* GDK_q */ 0x0051 /* GDK_Q */ },
  { 0x0077, /* GDK_w */ 0x0057 /* GDK_W */ },
  { 0x0065, /* GDK_e */ 0x0045 /* GDK_E */ },
  { 0x0072, /* GDK_r */ 0x0052 /* GDK_R */ },
  { 0x0074, /* GDK_t */ 0x0054 /* GDK_T */ },
  { 0x0079, /* GDK_y */ 0x0059 /* GDK_Y */ },
  { 0x0075, /* GDK_u */ 0x0055 /* GDK_U */ },
  { 0x0069, /* GDK_i */ 0x0049 /* GDK_I */ },
  { 0x006f, /* GDK_o */ 0x004f /* GDK_O */ },
  { 0x0070, /* GDK_p */ 0x0050 /* GDK_P */ },
  { 0x005b, /* GDK_bracketleft */ 0x007b /* GDK_braceleft */ },
  { 0x005d, /* GDK_bracketright */ 0x007d /* GDK_braceright */ },
  { ______, ______ },
  { ______, ______ },
  { 0x0061, /* GDK_a */ 0x0041 /* GDK_A */ },
  { 0x0073, /* GDK_s */ 0x0053 /* GDK_S */ },
  { 0x0064, /* GDK_d */ 0x0044 /* GDK_D */ },
  { 0x0066, /* GDK_f */ 0x0046 /* GDK_F */ },
  { 0x0067, /* GDK_g */ 0x0047 /* GDK_G */ },
  { 0x0068, /* GDK_h */ 0x0048 /* GDK_H */ },
  { 0x006a, /* GDK_j */ 0x004a /* GDK_J */ },
  { 0x006b, /* GDK_k */ 0x004b /* GDK_K */ },
  { 0x006c, /* GDK_l */ 0x004c /* GDK_L */ },
  { 0x003b, /* GDK_semicolon */ 0x003a /* GDK_colon */ },
  { 0x0027, /* GDK_apostrophe */ 0x0022 /* GDK_quotedbl */ },
  { 0x0060, /* GDK_quoteleft */ 0x007e /* GDK_asciitilde */ },
  { ______, ______ },
  { 0x005c, /* GDK_backslash */ 0x007c /* GDK_bar */ },
  { 0x007a, /* GDK_z */ 0x005a /* GDK_Z */ },
  { 0x0078, /* GDK_x */ 0x0058 /* GDK_X */ },
  { 0x0063, /* GDK_c */ 0x0043 /* GDK_C */ },
  { 0x0076, /* GDK_v */ 0x0056 /* GDK_V */ },
  { 0x0062, /* GDK_b */ 0x0042 /* GDK_B */ },
  { 0x006e, /* GDK_n */ 0x004e /* GDK_N */ },
  { 0x006d, /* GDK_m */ 0x004d /* GDK_M */ },
  { 0x002c, /* GDK_comma */ 0x003c /* GDK_less */ },
  { 0x002e, /* GDK_period */ 0x003e /* GDK_greater */ },
  { 0x002f, /* GDK_slash */ 0x003f /* GDK_question */ }
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
  { 0x11031103, 0x1104 }, /* choseong  tikeut + tikeut	= ssangtikeut	*/
  { 0x11071107, 0x1108 }, /* choseong  pieup  + pieup	= ssangpieup	*/
  { 0x11091109, 0x110a }, /* choseong  sios   + sios	= ssangsios	*/
  { 0x110c110c, 0x110d }, /* choseong  cieuc  + cieuc	= ssangcieuc	*/

  { 0x11611161, 0x1163 }, /* jungseong a      + a	= ya		*/
  { 0x11621162, 0x1164 }, /* jungseong ae     + ae	= yae		*/
  { 0x11631175, 0x1164 }, /* jungseong ya     + i	= yae		*/
  { 0x11651165, 0x1167 }, /* jungseong eo     + eo	= yeo		*/
  { 0x11661166, 0x1168 }, /* jungseong e      + e	= ye		*/
  { 0x11671175, 0x1168 }, /* jungseong yeo    + i	= ye		*/
  { 0x11691161, 0x116a }, /* jungseong o      + a	= wa		*/
  { 0x11691162, 0x116b }, /* jungseong o      + ae	= wae		*/
  { 0x11691169, 0x116d }, /* jungseong o      + o	= yo		*/
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
  { 0x11001109, 0x11aa }, /* jongseong kiyeok + sios	= kiyeok-sios	*/
  { 0x1102110c, 0x11ac }, /* jongseong nieun  + cieuc	= nieun-cieuc	*/
  { 0x11021112, 0x11ad }, /* jongseong nieun  + hieuh	= nieun-hieuh	*/
  { 0x11031103, 0x1104 }, /* choseong  tikeut + tikeut	= ssangtikeut	*/
  { 0x11051100, 0x11b0 }, /* jongseong rieul  + kiyeok	= rieul-kiyeok	*/
  { 0x11051106, 0x11b1 }, /* jongseong rieul  + mieum	= rieul-mieum	*/
  { 0x11051107, 0x11b2 }, /* jongseong rieul  + pieup	= rieul-pieup	*/
  { 0x11051109, 0x11b3 }, /* jongseong rieul  + sios	= rieul-sios	*/
  { 0x11051110, 0x11b4 }, /* jongseong rieul  + thieuth = rieul-thieuth	*/
  { 0x11051111, 0x11b5 }, /* jongseong rieul  + phieuph = rieul-phieuph	*/
  { 0x11051112, 0x11b6 }, /* jongseong rieul  + hieuh	= rieul-hieuh	*/
  { 0x11071107, 0x1108 }, /* choseong  pieup  + pieup	= ssangpieup	*/
  { 0x11071109, 0x11b9 }, /* jongseong pieup  + sios	= pieup-sios	*/
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
  { 0x11001102, 0x11fa }, /* JONGSEONG KIYEOK-NIEUN         = KIYEOK + NIEUN */
  { 0x11001103, 0x115a }, /* CHOSEONG KIYEOK-TIKEUT         = KIYEOK + TIKEUT */
  { 0x11001105, 0x11c3 }, /* choseong kiyeok-rieul	    = kiyeok + rieul */
  { 0x11001107, 0x11fb }, /* JONGSEONG KIYEOK-PIEUP         = KIYEOK + PIEUP */
  { 0x11001109, 0x11aa }, /* choseong kiyeok-sios	    = kiyeok + sios */
  { 0x1100110e, 0x11fc }, /* JONGSEONG KIYEOK-CHIEUCH       = KIYEOK + CHIEUCH */
  { 0x1100110f, 0x11fd }, /* JONGSEONG KIYEOK-KHIEUKH       = KIYEOK + KHIEUKH */
  { 0x11001112, 0x11fe }, /* JONGSEONG KIYEOK-HIEUH         = KIYEOK + CHIEUCH */
  { 0x11021100, 0x1113 }, /* CHOSEONG NIEUN-KIYEOK          = NIEUN + KIYEOK */
  { 0x11021102, 0x1114 }, /* CHOSEONG SSANGNIEUN            = NIEUN + NIEUN */
  { 0x11021103, 0x1115 }, /* CHOSEONG NIEUN-TIKEUT          = NIEUN + TIKEUT */
  { 0x11021107, 0x1116 }, /* CHOSEONG NIEUN-PIEUP           = NIEUN + PIEUP */
  { 0x11021109, 0x115b }, /* CHOSEONG NIEUN-SIOS            = NIEUN + SIOS */
  { 0x1102110c, 0x115c }, /* CHOSEONG NIEUN-CIEUC           = NIEUN + CIEUC */
  { 0x11021110, 0x11c9 }, /* CHOSEONG NIEUN-THIEUTH         = NIEUN + THIEUTH */
  { 0x11021112, 0x115d }, /* CHOSEONG NIEUN-HIEUH           = NIEUN + HIEUH */
  { 0x11021140, 0x11c8 }, /* CHOSEONG NIEUN-PANSIOS         = NIEUN + PANSIOS */
  { 0x11031100, 0x1117 }, /* CHOSEONG TIKEUT-KIYEOK         = TIKEUT + KIYEOK */
  { 0x11031103, 0x1104 }, /* CHOSEONG SSANGTIKEUT           = TIKEUT + TIKEUT */
  { 0x11031105, 0x115e }, /* CHOSEONG TIKEUT-RIEUL          = TIKEUT + RIEUL */
  { 0x11031106, 0xa960 }, /* CHOSEONG TIKEUT-MIEUM          = TIKEUT + MIEUM */
  { 0x11031107, 0xa961 }, /* CHOSEONG TIKEUT-PIEUP          = TIKEUT + PIEUP */
  { 0x11031109, 0xa962 }, /* CHOSEONG TIKEUT-SIOS           = TIKEUT + SIOS */
  { 0x1103110c, 0xa963 }, /* CHOSEONG TIKEUT-SIOS           = TIKEUT + SIOS */
  { 0x11051100, 0xa964 }, /* CHOSEONG RIEUL-KIYEOK          = RIEUL + KIYEOK */
  { 0x11051101, 0xa965 }, /* CHOSEONG RIEUL-KIYEOK-KIYEOK   = RIEUL + SSANGKIYEOK */
  { 0x11051102, 0x1118 }, /* CHOSEONG RIEUL-NIEUN           = RIEUL + NIEUN */
  { 0x11051103, 0xa966 }, /* CHOSEONG RIEUL-TIKEUT          = RIEUL + TIKEUT */
  { 0x11051104, 0xa967 }, /* CHOSEONG RIEUL-TIKEUT-TIKEUT   = RIEUL + SSANGTIKEUT */
  { 0x11051105, 0x1119 }, /* CHOSEONG SSANGRIEUL            = RIEUL + RIEUL */
  { 0x11051106, 0xa968 }, /* CHOSEONG RIEUL-MIEUM           = RIEUL + MIEUM */
  { 0x11051107, 0xa969 }, /* CHOSEONG RIEUL-PIEUP           = RIEUL + PIEUP */
  { 0x11051109, 0xa96c }, /* CHOSEONG RIEUL-SIOS            = RIEUL + SIOS */
  { 0x1105110b, 0x111b }, /* CHOSEONG KAPYEOUNRIEUL         = RIEUL + IEUNG */
  { 0x1105110c, 0xa96d }, /* CHOSEONG RIEUL-CIEUC           = RIEUL + CIEUC */
  { 0x1105110f, 0xa96e }, /* CHOSEONG RIEUL-KHIEUKH         = RIEUL + KHIEUKH */
  { 0x11051110, 0x11b4 }, /* CHOSEONG RIEUL-THIEUTH         = RIEUL + THIEUTH */
  { 0x11051111, 0x11b5 }, /* CHOSEONG RIEUL-PHIEUPH         = RIEUL + PHIEUPH */
  { 0x11051112, 0x111a }, /* CHOSEONG RIEUL-HIEUH           = RIEUL + HIEUH */
  { 0x1105112b, 0xa96b }, /* CHOSEONG RIEUL-KAPYEOUNPIEUP   = RIEUL + KAPYEOUNPIEUP */
  { 0x11051159, 0x11d9 }, /* JONGSEONG RIEUL-YEORINHIEUH    = RIEUL + YEORINHIEUH */

  { 0x11061100, 0xa96f }, /* CHOSEONG MIEUM-KIYEOK          = MIEUM + KIYEOK */
  { 0x11061102, 0xd7de }, /* JONGSEONG MIEUM-NIEUN          = MIEUM + NIEUN */
  { 0x11061103, 0xa970 }, /* CHOSEONG MIEUM-NIEUN           = MIEUM + NIEUN */
  { 0x11061105, 0x11db }, /* CHOSEONG MIEUM-KIYEOK          = MIEUM + KIYEOK */
  { 0x11061106, 0xd7e0 }, /* JONGSEONG MIEUM-MIEUM          = MIEUM + MIEUM */
  { 0x11061107, 0x111c }, /* CHOSEONG MIEUM-PIEUP           = MIEUM + PIEUP */
  { 0x11061109, 0xa971 }, /* CHOSEONG MIEUM-SIOS            = MIEUM + SIOS */
  { 0x1106110b, 0x111d }, /* CHOSEONG KAPYEOUNMIEUM         = MIEUM + IEUNG */
  { 0x1106110c, 0xd7e2 }, /* JONGSEONG MIEUM-CIEUC          = MIEUM + CIEUC */
  { 0x1106110e, 0x11e0 }, /* JONGSEONG MIEUM-CHIEUCH        = MIEUM + CHIEUCH */
  { 0x11061112, 0x11e1 }, /* JONGSEONG MIEUM-HIEUH          = MIEUM + HIEUH */
  { 0x11061140, 0x11df }, /* JONGSEONG MIEUM-PANSIOS        = MIEUM + PANSIOS */

  { 0x11071100, 0x111e }, /* CHOSEONG PIEUP-KIYEOK          = PIEUP + KIYEOK */
  { 0x11071102, 0x111f }, /* CHOSEONG PIEUP-NIEUN           = PIEUP + NIEUN */
  { 0x11071103, 0x1120 }, /* CHOSEONG PIEUP-TIKEUT          = PIEUP + TIKEUT */
  { 0x11071106, 0xd7e5 }, /* CHOSEONG PIEUP-MIEUM           = PIEUP + MIEUM */
  { 0x11071107, 0x1108 }, /* CHOSEONG SSANGPIEUP            = PIEUP + PIEUP */
  { 0x11071109, 0x1121 }, /* CHOSEONG PIEUP-SIOS            = PIEUP + SIOS */
  { 0x1107110b, 0x112b }, /* CHOSEONG KAPYEOUNPIEUP         = PIEUP + IEUNG */
  { 0x1107110c, 0x1127 }, /* CHOSEONG PIEUP-CIEUC           = PIEUP + CIEUC */
  { 0x1107110e, 0x1128 }, /* CHOSEONG PIEUP-CHIEUCH         = PIEUP + CHIEUCH */
  { 0x1107110f, 0xa973 }, /* CHOSEONG PIEUP-KHIEUKH         = PIEUP + KHIEUKH */
  { 0x11071110, 0x1129 }, /* CHOSEONG PIEUP-THIEUTH         = PIEUP + THIEUTH */
  { 0x11071111, 0x112a }, /* CHOSEONG PIEUP-PHIEUPH         = PIEUP + PHIEUPH */
  { 0x11071112, 0xa974 }, /* CHOSEONG PIEUP-HIEUH           = PIEUP + HIEUH */
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

  { 0x1109112b, 0xd7eb }, /* CHOSEONG SIOS-KAPYEOUNPIEUP    = SIOS + KAPYEOUNPIEUP */
  { 0x1109112d, 0xd7ec }, /* CHOSEONG SIOS-SIOS-KIYOEK      = SIOS + SIOS-KIYOEK */
  { 0x1109112f, 0xd7ed }, /* CHOSEONG SIOS-SIOS-TIKEUT      = SIOS + SIOS-TIKEUT */
  { 0x11091140, 0xd7ee }, /* CHOSEONG SIOS-PANSIOS          = SIOS + SIOS-PANSIOS */

  { 0x110a1107, 0xa975 }, /* CHOSEONG SSANGSIOS-PIEUP       = SSANGSIOS + PIEUP */
  { 0x110a1109, 0x1134 }, /* CHOSEONG SSANGSIOS-SIOS        = SSANGSIOS + SIOS */

  { 0x110b1100, 0x1141 }, /* CHOSEONG IEUNG-KIYEOK          = IEUNG + KIYEOK */
  { 0x110b1101, 0x11ed }, /* JONGSEONG IEUNG-SSANGKIYOEK    = IEUNG + SSANGKIYOEK */
  { 0x110b1103, 0x1142 }, /* CHOSEONG IEUNG-TIKEUT          = IEUNG + TIKEUT */
  { 0x110b1105, 0xa976 }, /* CHOSEONG IEUNG-RIEUL           = IEUNG + RIEUL */
  { 0x110b1106, 0x1143 }, /* CHOSEONG IEUNG-MIEUM           = IEUNG + MIEUM */
  { 0x110b1107, 0x1144 }, /* CHOSEONG IEUNG-PIEUP           = IEUNG + PIEUP */
  { 0x110b1109, 0x1145 }, /* CHOSEONG IEUNG-SIOS            = IEUNG + SIOS */
  { 0x110b110b, 0x1147 }, /* CHOSEONG SSANGIEUNG            = IEUNG + IEUNG */
  { 0x110b110c, 0x1148 }, /* CHOSEONG IEUNG-CIEUC           = IEUNG + CIEUC */
  { 0x110b110e, 0x1149 }, /* CHOSEONG IEUNG-CHIEUCH         = IEUNG + CHIEUCH */
  { 0x110b110f, 0x11ef }, /* JONGSEONG IEUNG-KHIEUKH        = IEUNG + KHIEUKH */
  { 0x110b1110, 0x114a }, /* CHOSEONG IEUNG-THIEUTH         = IEUNG + THIEUTH */
  { 0x110b1111, 0x114b }, /* CHOSEONG IEUNG-PHIEUPH         = IEUNG + PHIEUPH */
  { 0x110b1112, 0xa977 }, /* CHOSEONG IEUNG-HIEUH           = IEUNG + HIEUH */
  { 0x110b1140, 0x1146 }, /* CHOSEONG IEUNG-PANSIOS         = IEUNG + PANSIOS */



  { 0x110c1107, 0xd7f7 }, /* JONGSEONG CIEUC-PIEUP          = CIEUC + PIEUP */
  { 0x110c110b, 0x114d }, /* CHOSEONG CIEUC-IEUNG           = CIEUC + IEUNG */
  { 0x110c110c, 0x110d }, /* CHOSEONG SSANGCIEUC            = CIEUC + CIEUC */
  { 0x110d1112, 0xa978 }, /* CHOSEONG SSANGCIEUC-HIEUH      = SSANGCIEUC + HIEUH */
  { 0x110e110f, 0x1152 }, /* CHOSEONG CHIEUCH-KHIEUKH       = CHIEUCH + KHIEUKH */
  { 0x110e1112, 0x1153 }, /* CHOSEONG CHIEUCH-HIEUH         = CHIEUCH + HIEUH */
  { 0x11101110, 0xa979 }, /* CHOSEONG SSANGTHIEUTH          = THIEUTH + THIEUTH */
  { 0x11111107, 0x1156 }, /* CHOSEONG PHIEUPH-PIEUP         = PHIEUPH + PIEUP */
  { 0x11111109, 0xd7fa }, /* JONGSEONG PHIEUPH-SIOS         = PHIEUPH + SIOS */
  { 0x1111110b, 0x1157 }, /* CHOSEONG KAPYEOUNPHIEUPH       = PHIEUPH + IEUNG */
  { 0x11111110, 0xd7f8 }, /* JONGSEONG PHIEUPH-THIEUTH      = PHIEUPH + THIEUTH */
  { 0x11111112, 0xa97a }, /* CHOSEONG PHIEUPH-HIEUH         = PHIEUPH + HIEUH */
  { 0x11121109, 0xa97b }, /* CHOSEONG HIEUH-SIOS            = HIEUH + SIOS */
  { 0x11121112, 0x1158 }, /* CHOSEONG SSANGHIEUH            = HIEUH + HIEUH */
  { 0x11211100, 0x1122 }, /* CHOSEONG PIEUP-SIOS-KIYEOK     = PIEUP-SIOS + KIYEOK */
  { 0x11211103, 0x1123 }, /* CHOSEONG PIEUP-SIOS-TIKEUT     = PIEUP-SIOS + TIKEUT */
  { 0x11211107, 0x1124 }, /* CHOSEONG PIEUP-SIOS-PIEUP      = PIEUP-SIOS + PIEUP */
  { 0x11211109, 0x1125 }, /* CHOSEONG PIEUP-SSANGSIOS       = PIEUP-SIOS + SIOS */
  { 0x1121110c, 0x1126 }, /* CHOSEONG PIEUP-SIOS-CIEUC      = PIEUP-SIOS + CIEUC */
  { 0x11211110, 0xa972 }, /* CHOSEONG PIEUP-SIOS-THIKEUTH   = PIEUP-SIOS + THIKEUTH */
  { 0x11321100, 0x1133 }, /* CHOSEONG SIOS-PIEUP-KIYEOK     = SIOS-PIEUP + KIYEOK */
  //{ 0x11351109, 0x1140 }, /* CHOSEONG PANSIOS               = SIOS-IEUNG + SIOS => PANSIOS (cyclic input) */
  //{ 0x1135110b, 0x1140 }, /* CHOSEONG PANSIOS               = SIOS-IEUNG + IEUNG => PANSIOS (cyclic input) */
  { 0x113c113c, 0x113d }, /* CHOSEONG CHITUEUMSSANGSIOS     = CHITUEUMSIOS + CHITUEUMSIOS */
  { 0x113e113e, 0x113f }, /* CHOSEONG CEONGCHIEUMSSANGSIOS  = CEONGCHIEUMSIOS + CEONGCHIEUMSIOS */
  //{ 0x11401109, 0x1109 }, /* CHOSEONG SIOS                  = PANSIOS + SIOS => SIOS (cyclic input) */
  //{ 0x1140110b, 0x110b }, /* CHOSEONG IEUNG                 = PANSIOS + IEUNG => IEUNG (cyclic input) */
  //{ 0x11451109, 0x1140 }, /* CHOSEONG PANSIOS               = IEUNG-SIOS + SIOS => PANSIOS (cyclic input) */
  //{ 0x1145110b, 0x1140 }, /* CHOSEONG PANSIOS               = IEUNG-SIOS + IEUNG => PANSIOS (cyclic input) */

  { 0x114c1100, 0x11ec }, /* JONGSEONG IEUNG-KIYOEK         = YESIEUNG + KIYOEK */
  { 0x114c1101, 0x11ed }, /* JONGSEONG IEUNG-SSANGKIYOEK    = YESIEUNG + SSANGKIYOEK */
  { 0x114c110f, 0x11ef }, /* JONGSEONG IEUNG-KHIEUKH        = YESIEUNG + KHIEUKH */
  { 0x114c114c, 0x1147 }, /* CHOSEONG SSANGIEUNG            = YESIEUNG + YESIEUNG XXX */

  { 0x114e114e, 0x114f }, /* CHOSEONG CHITUEUMSSANGCIEUC    = CHITUEUMCIEUC + CHITUEUMCIEUC */
  { 0x11501150, 0x1151 }, /* CHOSEONG CEONGCHIEUMSSANGCIEUC = CEONGCHIEUMCIEUC + CEONGCHIEUMCIEUC */
  { 0x11591159, 0xa97c }, /* JONGSEONG SSANGYEORINHIEUH     = YEORINHIEUH + YEORINHIEUH */

    /* (cyclic change for simple 2-set automata like as a cellphone pad) */
  { 0x11611161, 0x119e }, /* JUNGSEONG A-A -> ARAEA         = A + A => ARAEA */

  { 0x11611169, 0x1176 }, /* JUNGSEONG A-O                  = A + O */
  { 0x1161116e, 0x1177 }, /* JUNGSEONG A-U                  = A + U */
  { 0x11611173, 0x11a3 }, /* JUNGSEONG A-EU                 = A + EU */
  { 0x11611175, 0x1162 }, /* JUNGSEONG AE                   = A */
  { 0x11631169, 0x1178 }, /* JUNGSEONG YA-O                 = YA + O */
  { 0x1163116d, 0x1179 }, /* JUNGSEONG YA-YO                = YA + YO */
  { 0x1163116e, 0x11a4 }, /* JUNGSEONG YA-U                 = YA + U */
  { 0x11631175, 0x1164 }, /* JUNGSEONG YAE                  = YA */
  { 0x11651169, 0x117a }, /* JUNGSEONG EO-O                 = EO + O */
  { 0x1165116e, 0x117b }, /* JUNGSEONG EO-U                 = EO + U */
  { 0x11651173, 0x117c }, /* JUNGSEONG EO-EU                = EO + EU */
  { 0x11651175, 0x1166 }, /* JUNGSEONG E                    = EO */
  { 0x11671163, 0x11a5 }, /* JUNGSEONG YEO-YA               = YEO + YA */
  { 0x11671169, 0x117d }, /* JUNGSEONG YEO-O                = YEO + O */
  { 0x1167116e, 0x117e }, /* JUNGSEONG YEO-U                = YEO + U */
  { 0x11671175, 0x1168 }, /* JUNGSEONG YE                   = YEO */
  { 0x11691161, 0x116a }, /* JUNGSEONG WA                   = O + A */
  { 0x11691162, 0x116b }, /* JUNGSEONG WAE		    = O + AE */
  { 0x11691163, 0x11a6 }, /* JUNGSEONG O-YA                 = O + YA */
  { 0x11691164, 0x11a7 }, /* JUNGSEONG O-YAE                = O + YAE */
  { 0x11691165, 0x117f }, /* JUNGSEONG O-EO                 = O + EO */
  { 0x11691166, 0x1180 }, /* JUNGSEONG O-E                  = O + E */
  { 0x11691167, 0xd7b0 }, /* JUNGSEONG O-YOE                = O + YOE */
  { 0x11691168, 0x1181 }, /* JUNGSEONG O-YE                 = O + YE */
  { 0x11691169, 0x1182 }, /* JUNGSEONG O-O                  = O + O */
  { 0x1169116e, 0x1183 }, /* JUNGSEONG O-U                  = O + U */
  { 0x11691175, 0x116c }, /* JUNGSEONG OE                   = O */
  { 0x116a1175, 0x116b }, /* JUNGSEONG WAE                  = WA */
  { 0x116d1161, 0xd7b2 }, /* JUNGSEONG YO-A                 = YO + A */
  { 0x116d1162, 0xd7b3 }, /* JUNGSEONG YO-AE                = YO + AE*/
  { 0x116d1163, 0x1184 }, /* JUNGSEONG YO-YA                = YO + YA */
  { 0x116d1164, 0x1185 }, /* JUNGSEONG YO-YAE               = YO + YAE */
  { 0x116d1165, 0xd7b4 }, /* JUNGSEONG YO-EO                = YO + EO */
  { 0x116d1167, 0x1186 }, /* JUNGSEONG YO-YEO               = YO + YEO */
  { 0x116d1169, 0x1187 }, /* JUNGSEONG YO-O                 = YO + O */
  { 0x116d1175, 0x1188 }, /* JUNGSEONG YO-I                 = YO + I */
  { 0x116e1161, 0x1189 }, /* JUNGSEONG U-A                  = U + A */
  { 0x116e1162, 0x118a }, /* JUNGSEONG U-AE                 = U + AE */
  { 0x116e1165, 0x116f }, /* JUNGSEONG WEO                  = U + EO */
  { 0x116e1166, 0x1170 }, /* JUNGSEONG WE		    = U + E */
  { 0x116e1167, 0xd7b5 }, /* JUNGSEONG U-YEO                = U + YEO */
  { 0x116e1168, 0x118c }, /* JUNGSEONG U-YE                 = U + YE */
  { 0x116e116e, 0x118d }, /* JUNGSEONG U-U                  = U + U */
  { 0x116e1175, 0x1171 }, /* JUNGSEONG WI                   = U */
  { 0x116f1173, 0x118b }, /* JUNGSEONG U-EO-EU              = WEO + EU */
  { 0x116f1175, 0x1170 }, /* JUNGSEONG WE                   = WEO */
  { 0x11711175, 0xd7b6 }, /* JUNGSEONG WI-I                 = WI + I */
  { 0x11721161, 0x118e }, /* JUNGSEONG YU-A                 = YU + A */
  { 0x11721162, 0xd7b7 }, /* JUNGSEONG YU-AE                = YU + AE */
  { 0x11721165, 0x118f }, /* JUNGSEONG YU-EO                = YU + EO */
  { 0x11721166, 0x1190 }, /* JUNGSEONG YU-E                 = YU + E */
  { 0x11721167, 0x1191 }, /* JUNGSEONG YU-YEO               = YU + YEO */
  { 0x11721168, 0x1192 }, /* JUNGSEONG YU-YE                = YU + YE */
  { 0x11721169, 0xd7b8 }, /* JUNGSEONG YU-O                 = YU + O */
  { 0x1172116e, 0x1193 }, /* JUNGSEONG YU-U                 = YU + U */
  { 0x11721175, 0x1194 }, /* JUNGSEONG YU-I                 = YU + I */
  { 0x11731161, 0xd7b9 }, /* JUNGSEONG EU-A                 = EU + A */
  { 0x11731165, 0xd7ba }, /* JUNGSEONG EU-EO                = EU + EO */
  { 0x11731166, 0xd7bb }, /* JUNGSEONG EU-E                 = EU + E */
  { 0x11731169, 0xd7bc }, /* JUNGSEONG EU-O                 = EU + O */
  { 0x1173116e, 0x1195 }, /* JUNGSEONG EU-U                 = EU + U */
  { 0x11731173, 0x1196 }, /* JUNGSEONG EU-EU                = EU + EU */
  { 0x11731175, 0x1174 }, /* JUNGSEONG YI                   = EU */
  { 0x1174116e, 0x1197 }, /* JUNGSEONG YI-U                 = YI + U */
  { 0x11751161, 0x1198 }, /* JUNGSEONG I-A                  = I + A */
  { 0x11751163, 0x1199 }, /* JUNGSEONG I-YA                 = I + YA */
  { 0x11751164, 0xd7be }, /* JUNGSEONG I-YA-I               = I + YAE */
  { 0x11751167, 0xd7bf }, /* JUNGSEONG I-YEO                = I + YEO */
  { 0x11751168, 0xd7c0 }, /* JUNGSEONG I-YEO-I              = I + YE */
  { 0x11751169, 0x119a }, /* JUNGSEONG I-O                  = I + O */
  { 0x1175116d, 0xd7c2 }, /* JUNGSEONG I-YO                 = I + YO */
  { 0x1175116e, 0x119b }, /* JUNGSEONG I-U                  = I + U */
  { 0x11751172, 0xd7c3 }, /* JUNGSEONG I-YU                 = I + YU */
  { 0x11751173, 0x119c }, /* JUNGSEONG I-EU                 = I + EU */
  { 0x11751175, 0xd7c4 }, /* JUNGSEONG I-I                  = I + I */
  { 0x1175119e, 0x119d }, /* JUNGSEONG I-ARAEA              = I + ARAEA */
  { 0x11821175, 0xd7b1 }, /* JUNGSEONG O-O-I                = O-O + I */
  { 0x11841175, 0x1184 }, /* JUNGSEONG YO-YAE               = YO-YA + I */
  { 0x118e1175, 0xd7b7 }, /* JUNGSEONG YU-AE                = YU-A + I */
  { 0x11981161, 0x119d }, /* JUNGSEONG I-ARAEA              = I-A + A => A (cyclic change like as cellphone pad) */
  { 0x11991169, 0xd7bd }, /* JUNGSEONG I-YA-O               = I-YA + O */
  { 0x11991175, 0xd7be }, /* JUNGSEONG I-YA-I               = I-YA + I */
  { 0x119a1175, 0xd7c1 }, /* JUNGSEONG I-O-I                = I-O + I */
  { 0x119d1161, 0x1198 }, /* JUNGSEONG I-A                  = I-ARAEA+ A => I-A (cyclic change like as cellphone pad) */
  //{ 0x119e1161, 0x1161 }, /* JUNGSEONG ARAEA-A              = ARAEA + A => A (cyclic change like as cellphone pad) */
  //{ 0x119e1161, 0x11a2 }, /* JUNGSEONG ARAEA-ARAEA          = ARAEA + A => ARAEA-ARAEA (cyclic change like as cellphone pad) */
  { 0x119e1161, 0xd7c5 }, /* JUNGSEONG ARAEA-A              = ARAEA + A => ARAEA-A (cyclic change like as cellphone pad) */
  { 0x119e1165, 0x119f }, /* JUNGSEONG ARAEA-EO             = ARAEA + EO */
  { 0x119e1166, 0xd7c6 }, /* JUNGSEONG ARAEA-E              = ARAEA + E */
  { 0x119e116e, 0x11a0 }, /* JUNGSEONG ARAEA-U              = ARAEA + U */
  { 0x119e1175, 0x11a1 }, /* JUNGSEONG ARAEA-I              = ARAEA + I */
  { 0x119e119e, 0x11a2 }, /* JUNGSEONG SSANGARAEA           = ARAEA + ARAEA */
  { 0x119f1175, 0xd7c6 }, /* JUNGSEONG ARAEA-E              = ARAEA-EO + I */
  { 0x11a21161, 0x1161 }, /* JUNGSEONG A                    = ARAEA-ARAEA + A => A (cyclic change like as cellphone pad) */
  { 0x11a811a8, 0x11a9 }, /* JONGSEONG SSANGKIYEOK          = KIYEOK + KIYEOK */
  { 0x11a811ab, 0x11fa }, /* JONGSEONG KIYEOK-NIEUN         = KIYEOK + NIEUN */
  { 0x11a811af, 0x11c3 }, /* JONGSEONG KIYEOK-RIEUL         = KIYEOK + RIEUL */
  { 0x11a811b8, 0x11fb }, /* JONGSEONG KIYEOK-PIEUP         = KIYEOK + PIEUP */
  { 0x11a811ba, 0x11aa }, /* JONGSEONG KIYEOK-SIOS          = KIYEOK */
  { 0x11a811be, 0x11fc }, /* JONGSEONG KIYEOK-CHIEUCH       = KIYEOK + CHIEUCH */
  { 0x11a811bf, 0x11fd }, /* JONGSEONG KIYEOK-KHIEUKH       = KIYEOK + KHIEUKH */
  { 0x11a811c2, 0x11fe }, /* JONGSEONG KIYEOK-HIEUH         = KIYEOK + CHIEUCH */

  { 0x11aa11a8, 0x11c4 }, /* JONGSEONG KIYEOK-SIOS-KIYEOK   = KIYEOK-SIOS + KIYEOK */
  { 0x11ab11a8, 0x11c5 }, /* JONGSEONG NIEUN-KIYEOK         = NIEUN + KIYEOK */
  { 0x11ab11ab, 0x11ff }, /* JONGSEONG SSANGNIEUN           = NIEUN + NIEUN */
  { 0x11ab11ae, 0x11c6 }, /* JONGSEONG NIEUN-TIKEUT         = NIEUN + TIKEUT */
  { 0x11ab11af, 0xd7cb }, /* JONGSEONG NIEUN-RIEUL          = NIEUN + RIEUL */
  { 0x11ab11ba, 0x11c7 }, /* JONGSEONG NIEUN-SIOS           = NIEUN + SIOS */
  { 0x11ab11bd, 0x11ac }, /* JONGSEONG NIEUN-CIEUC          = NIEUN + CIEUC */
  { 0x11ab11be, 0xd7cc }, /* JONGSEONG NIEUN-CHIEUCH        = NIEUN + CHIEUCH */
  { 0x11ab11c0, 0x11c9 }, /* JONGSEONG NIEUN-THIEUTH        = NIEUN + THIEUTH */
  { 0x11ab11c2, 0x11ad }, /* JONGSEONG NIEUN-HIEUH          = NIEUN + HIEUH */
  { 0x11ab11eb, 0x11c8 }, /* JONGSEONG NIEUN-PANSIOS        = NIEUN + PANSIOS */
  { 0x11ae11a8, 0x11ca }, /* JONGSEONG TIKEUT-KIYEOK        = TIKEUT + KIYEOK */
  { 0x11ae11ae, 0xd7cd }, /* JONGSEONG SSANGTIKEUT          = TIKEUT + TIKEUT */
  { 0x11ae11af, 0x11cb }, /* JONGSEONG TIKEUT-RIEUL         = TIKEUT + RIEUL */
  { 0x11ae11b8, 0xd7cf }, /* JONGSEONG TIKEUT-PIEUP         = TIKEUT + PIEUP */
  { 0x11ae11ba, 0xd7d0 }, /* JONGSEONG TIKEUT-SIOS          = TIKEUT + SIOS */

  { 0x11ae11bd, 0xd7d2 }, /* JONGSEONG TIKEUT-CIEUC         = TIKEUT + CIEUC */
  { 0x11ae11be, 0xd7d3 }, /* JONGSEONG TIKEUT-CHIEUCH       = TIKEUT + CHIEUCH */
  { 0x11ae11c0, 0xd7d4 }, /* JONGSEONG TIKEUT-TIKEUT        = TIKEUT + TIKEUT */

  { 0x11af11a8, 0x11b0 }, /* JONGSEONG RIEUL-KIYEOK         = RIEUL + KIYEOK */
  { 0x11af11ab, 0x11cd }, /* JONGSEONG RIEUL-NIEUN          = RIEUL + NIEUN */
  { 0x11af11ae, 0x11ce }, /* JONGSEONG RIEUL-TIKEUT         = RIEUL + TIKEUT */
  { 0x11af11af, 0x11d0 }, /* JONGSEONG SSANGRIEUL           = RIEUL + RIEUL */
  { 0x11af11b7, 0x11b1 }, /* JONGSEONG RIEUL-MIEUM          = RIEUL + MIEUM */
  { 0x11af11b8, 0x11b2 }, /* JONGSEONG RIEUL-PIEUP          = RIEUL + PIEUP */
  { 0x11af11ba, 0x11b3 }, /* JONGSEONG RIEUL-SIOS           = RIEUL + SIOS */
  { 0x11af11bc, 0xd7dd }, /* JONGSEONG KAPYEOUNRIEUL        = RIEUL + IEUNG */
  { 0x11af11bf, 0x11d8 }, /* JONGSEONG RIEUL-KHIEUKH        = RIEUL + KHIEUKH */
  { 0x11af11c0, 0x11b4 }, /* JONGSEONG RIEUL-THIEUTH        = RIEUL + THIEUTH */
  { 0x11af11c1, 0x11b5 }, /* JONGSEONG RIEUL-PHIEUPH        = RIEUL + PHIEUPH */
  { 0x11af11c2, 0x11b6 }, /* JONGSEONG RIEUL-HIEUH          = RIEUL + HIEUH */
  { 0x11af11eb, 0x11d7 }, /* JONGSEONG RIEUL-PANSIOS        = RIEUL + PANSIOS */
  { 0x11af11f0, 0xd7db }, /* JONGSEONG RIEUL-YESIEUNG       = RIEUL + YESIEUNG */
  { 0x11af11f9, 0x11d9 }, /* JONGSEONG RIEUL-YEORINHIEUH    = RIEUL + YEORINHIEUH */
  { 0x11b011a8, 0xd7d5 }, /* JONGSEONG RIEUL-KIYEOK-KIYEOK  = RIEUL-KIYEOK + KIYEOK */
  { 0x11b011ba, 0x11cc }, /* JONGSEONG RIEUL-KIYEOK-SIOS    = RIEUL-KIYEOK + SIOS */
  { 0x11b011c2, 0xd7d6 }, /* JONGSEONG RIEUL-KIYEOK-HIEUH   = RIEUL-KIYEOK + HIEUH */
  { 0x11b111a8, 0x11d1 }, /* JONGSEONG RIEUL-MIEUM-KIYEOK   = RIEUL-MIEUM + KIYEOK */
  { 0x11b111ba, 0x11d2 }, /* JONGSEONG RIEUL-MIEUM-SIOS     = RIEUL-MIEUM + SIOS */
  { 0x11b111c2, 0xd7d8 }, /* JONGSEONG RIEUL-MIEUM-HIEUH    = RIEUL-MIEUM + HIEUH */
  { 0x11b211ae, 0xd7d9 }, /* JONGSEONG RIEUL-PIEUP-TIKEUH   = RIEUL-PIEUP + TIKEUH */
  { 0x11b211ba, 0x11d3 }, /* JONGSEONG RIEUL-PIEUP-SIOS     = RIEUL-PIEUP + SIOS */
  { 0x11b211bc, 0x11d5 }, /* JONGSEONG RIEUL-KAPYEOUNPIEUP  = RIEUL-PIEUP + IEUNG */
  { 0x11b211c1, 0xd7da }, /* JONGSEONG RIEUL-PIEUP-PHIEUPH  = RIEUL-PIEUP + PHIEUPH */
  { 0x11b211c2, 0x11d4 }, /* JONGSEONG RIEUL-PIEUP-HIEUH    = RIEUL-PIEUP + HIEUH */
  { 0x11b311ba, 0x11d6 }, /* JONGSEONG RIEUL-SSANGSIOS      = RIEUL-SIOS + SIOS */
  { 0x11b711a8, 0x11da }, /* JONGSEONG MIEUM-KIYEOK         = MIEUM + KIYEOK */
  { 0x11b711ab, 0xd7de }, /* JONGSEONG MIEUM-NIEUN          = MIEUM + NIEUN */
  { 0x11b711af, 0x11db }, /* JONGSEONG MIEUM-RIEUL          = MIEUM + RIEUL */
  { 0x11b711b7, 0xd7e0 }, /* JONGSEONG SSANGMIEUM           = MIEUM + MIEUM */
  { 0x11b711b8, 0x11dc }, /* JONGSEONG MIEUM-PIEUP          = MIEUM + PIEUP */
  { 0x11b711ba, 0x11dd }, /* JONGSEONG MIEUM-SIOS           = MIEUM + SIOS */
  { 0x11b711bc, 0x11e2 }, /* JONGSEONG KAPYEOUNMIEUM        = MIEUM + IEUNG */
  { 0x11b711bd, 0xd7e2 }, /* JONGSEONG MIEUM-CIEUC          = MIEUM + CIEUC */
  { 0x11b711be, 0x11e0 }, /* JONGSEONG MIEUM-CHIEUCH        = MIEUM + CHIEUCH */
  { 0x11b711c2, 0x11e1 }, /* JONGSEONG MIEUM-HIEUH          = MIEUM + HIEUH */
  { 0x11b711eb, 0x11df }, /* JONGSEONG MIEUM-PANSIOS        = MIEUM + PANSIOS */
  { 0x11b811ae, 0xd7e3 }, /* JONGSEONG PIEUP-TIKEUT         = PIEUP + TIKEUT */
  { 0x11b811af, 0x11e3 }, /* JONGSEONG PIEUP-RIEUL          = PIEUP + RIEUL */
  { 0x11b811b7, 0xd7e5 }, /* JONGSEONG PIEUP-MIEUM          = PIEUP + MIEUM */
  { 0x11b811b8, 0xd7e6 }, /* JONGSEONG SSANGPIEUP           = PIEUP + PIEUP */
  { 0x11b811ba, 0x11b9 }, /* JONGSEONG PIEUP-SIOS           = PIEUP + SIOS */
  { 0x11b811bc, 0x11e6 }, /* JONGSEONG KAPYEOUNPIEUP        = PIEUP + IEUNG */
  { 0x11b811bd, 0xd7e8 }, /* JONGSEONG PIEUP-CIEUC          = PIEUP + CIEUC */
  { 0x11b811be, 0xd7e9 }, /* JONGSEONG PIEUP-CHIEUCH        = PIEUP + CHIEUCH */
  { 0x11b811c1, 0x11e4 }, /* JONGSEONG PIEUP-PHIEUPH        = PIEUP + PHIEUPH */
  { 0x11b811c2, 0x11e5 }, /* JONGSEONG PIEUP-HIEUH          = PIEUP + HIEUH */
  { 0x11b911ae, 0xd7e7 }, /* JONGSEONG PIEUP-SIOS-TIKEUT    = PIEUP-SIOS + TIKEUT */
  { 0x11ba11a8, 0x11e7 }, /* JONGSEONG SIOS-KIYEOK          = SIOS + KIYEOK */
  { 0x11ba11ae, 0x11e8 }, /* JONGSEONG SIOS-TIKEUT          = SIOS + TIKEUT */
  { 0x11ba11af, 0x11e9 }, /* JONGSEONG SIOS-RIEUL           = SIOS + RIEUL */
  { 0x11ba11b7, 0xd7ea }, /* JONGSEONG SIOS-MIEUM           = SIOS + MIEUM */
  { 0x11ba11b8, 0x11ea }, /* JONGSEONG SIOS-PIEUP           = SIOS + PIEUP */
  { 0x11ba11ba, 0x11bb }, /* JONGSEONG SSANGSIOS            = SIOS + SIOS */
  { 0x11ba11bd, 0xd7ef }, /* JONGSEONG SIOS-CIEUC           = SIOS + CIEUC */
  { 0x11ba11be, 0xd7f0 }, /* JONGSEONG SIOS-CHIEUCH         = SIOS + CHIEUCH */
  { 0x11ba11c0, 0xd7f1 }, /* JONGSEONG SIOS-THIKEUTH        = SIOS + THIKEUTH */
  { 0x11ba11c2, 0xd7f2 }, /* JONGSEONG SIOS-HIEUH           = SIOS + HIEUH */
  { 0x11ba11eb, 0xd7ee }, /* JONGSEONG SIOS-PANSIOS         = SIOS + PANSIOS */
  { 0x11bb11a8, 0xd7ec }, /* JONGSEONG SSANGSIOS-KIYEOK     = SSANGSIOS + KIYEOK */
  { 0x11bb11ae, 0xd7ed }, /* JONGSEONG SSANGSIOS-TIKEUT     = SSANGSIOS + TIKEUT */
  { 0x11bc11a8, 0x11ec }, /* JONGSEONG IEUNG-KIYEOK         = IEUNG + KIYEOK */
  { 0x11bc11a9, 0x11ed }, /* JONGSEONG IEUNG-SSANGKIYOEK    = IEUNG + SSANGKIYOEK */
  //{ 0x11bc11ba, 0x11f1 }, /* JONGSEONG YESIEUNG-SIOS        = IEUNG + SIOS */
  { 0x11bc11bc, 0x11ee }, /* JONGSEONG SSANGIEUNG           = IEUNG + IEUNG */
  { 0x11bc11bf, 0x11ef }, /* JONGSEONG IEUNG-KHIEUKH        = IEUNG + KHIEUKH */
  //{ 0x11bc11eb, 0x11f2 }, /* JONGSEONG YESIEUNG-PANSIOS     = IEUNG + PANSIOS */

  { 0x11bd11b8, 0xd7f7 }, /* JONGSEONG CIEUC-PIEUP          = CIEUC + PIEUP */
  { 0x11bd11bd, 0xd7f9 }, /* JONGSEONG SSANGCIEUC           = CIEUC + CIEUC */
  { 0x11c011c0, 0xa979 }, /* JONGSEONG SSANGTHIEUTH         = THIEUTH + THIEUTH */
  { 0x11c111b8, 0x11f3 }, /* JONGSEONG PHIEUPH-PIEUP        = PHIEUPH + PIEUP */
  { 0x11c111ba, 0xd7fa }, /* JONGSEONG PHIEUPH-SIOS         = PHIEUPH + SIOS */
  { 0x11c111bc, 0x11f4 }, /* JONGSEONG KAPYEOUNPHIEUPH      = PHIEUPH + IEUNG */
  { 0x11c111c0, 0xd7fb }, /* JONGSEONG PHIEUPH-THIEUTH      = PHIEUPH + THIEUTH */
  { 0x11c111c2, 0xd77a }, /* JONGSEONG PHIEUPH-HIEUH        = PHIEUPH + HIEUH */
  { 0x11c211ab, 0x11f5 }, /* JONGSEONG HIEUH-NIEUN          = HIEUH + NIEUN */
  { 0x11c211af, 0x11f6 }, /* JONGSEONG HIEUH-RIEUL          = HIEUH + RIEUL */
  { 0x11c211b7, 0x11f7 }, /* JONGSEONG HIEUH-MIEUM          = HIEUH + MIEUM */
  { 0x11c211b8, 0x11f8 }, /* JONGSEONG HIEUH-PIEUP          = HIEUH + PIEUP */
  { 0x11c211ba, 0xa97b }, /* JONGSEONG HIEUH-SIOS           = HIEUH + SIOS */
  { 0x11ce11c2, 0x11cf }, /* JONGSEONG RIEUL-TIKEUT-HIEUH   = RIEUL-TIKEUT + HIEUH */
  { 0x11d011bf, 0xd7d7 }, /* JONGSEONG SSANGRIEUL-KHIEUKH   = SSANGRIEUL + KHIEUKH */
  { 0x11d911c2, 0xd7dc }, /* JONGSEONG RIEUL-YEORINHIEUH-HIEUH  = RIEUL-YEORINHIEUH + HIEUH */
  { 0x11dc11ba, 0xd7e1 }, /* JONGSEONG MIEUM-PIEUP-SIOS     = MIEUM-PIEUP + SIOS */
  { 0x11dd11ba, 0x11de }, /* JONGSEONG MIEUM-SSANGSIOS      = MIEUM-SIOS + SIOS */

  { 0x11e311c1, 0xd7e4 }, /* JONGSEONG PIEUP-RIEUL-PHIEUPH  = PIEUP-RIEUL + PHIEUPH */
  { 0x11ea11bc, 0xd7eb }, /* JONGSEONG SIOS-KAPYEOUNPIEUP   = SIOS-PIEUP + IEUNG */
  { 0x11eb11b8, 0xd7f3 }, /* JONGSEONG PANSIOS-PIEUP        = PANSIOS + PIEUP */
  { 0x11eb11e6, 0xd7f4 }, /* JONGSEONG PANSIOS-KAPYEOUNPIEUP= PANSIOS + KAPYEOUNPIEUP */

  { 0x11ec11a8, 0x11ed }, /* JONGSEONG IEUNG-SSANGKIYEOK    = IEUNG-KIYEOK + KIYEOK */

  { 0x11f011a8, 0x11ec }, /* JONGSEONG YESIEUNG-KIYOEK      = YESIEUNG + KIYOEK */
  { 0x11f011a9, 0x11ed }, /* JONGSEONG IEUNG-SSANGKIYOEK    = YESIEUNG + SSANGKIYOEK */
  { 0x11f011b7, 0xd7f5 }, /* JONGSEONG IEUNG-MIEUM          = YESIEUNG + MIEUM */
  { 0x11f011ba, 0x11f1 }, /* JONGSEONG YESIEUNG-SIOS        = YESIEUNG + SIOS */
  { 0x11f011bf, 0x11ef }, /* JONGSEONG IEUNG-KHIEUKH        = YESIEUNG + KHIEUKH */
  { 0x11f011c2, 0xd7f6 }, /* JONGSEONG YESIEUNG-HIEUH       = YESIEUNG + HIEUH */
  { 0x11f011eb, 0x11f2 }, /* JONGSEONG YESIEUNG-PANSIOS     = YESIEUNG + PANSIOS */
  { 0x11f011f0, 0x11ee }, /* JONGSEONG SSANGIEUNG           = YESIEUNG + YESIEUNG */
  { 0x11f911f9, 0xa97c }, /* JONGSEONG SSANGYEORINHIEUH     = YEORINHIEUH + YEORINHIEUH */

  { 0xa9641100, 0xa965 }, /* CHOSEONG RIEUL-KIYEOK-KIYEOK   = RIEUL-KIYEOK + KIYEOK */
  { 0xa9661103, 0xa967 }, /* CHOSEONG RIEUL-TIKEUT-TIKEUT   = RIEUL-TIKEUT + TIKEUT */
  { 0xa9691107, 0xa96a }, /* CHOSEONG RIEUL-PIEUP-PIEUP     = RIEUL-PIEUP + PIEUP */
  { 0xa969110b, 0xa96b }, /* CHOSEONG RIEUL-KAPYEOUNPIEUP   = RIEUL-PIEUP + IEUNG */

  { 0xd7ba1175, 0xd7bb }, /* JUNGSEONG EU-E                 = EU-EO + I */
  { 0xd7bf1175, 0xd7c0 }, /* JUNGSEONG I-YEO-I              = I-YEO + I */

  { 0xd7c51161, 0x11a2 }, /* JUNGSEONG ARAEA-ARAEA          = ARAEA-A + A => ARAEA-ARAEA (cyclic change like as cellphone pad) */

  { 0xd7cd11b8, 0xd7ce }, /* JONGSEONG SSANGTIKEUT-PIEUP    = SSANGTIKEUT + PIEUP */
  { 0xd7d011a8, 0xd7d1 }, /* JONGSEONG TIKEUT-SIOS-KIYEOK   = TIKEUT-SIOS + KIYEOK */
  { 0xd7de11ab, 0xd7df }, /* JONGSEONG MIEUM-NIEUN-NIEUN    = MIEUM-NIEUN + NIEUN */
  { 0xd7f311bc, 0xd7f4 }, /* JONGSEONG PANSIOS-KAPYEOUNPIEUP= PANSIOS-PIEUP + IEUNG */
  { 0xd7f711b8, 0xd7f8 }, /* JONGSEONG CIEUC-PIEUP-PIEUP    = CIEUC-PIEUP + PIEUP */
};

//#define compose_table compose_table_default
static HangulCompose compose_table_user[256];
static HangulCompose *compose_table = (HangulCompose *) compose_table_2set;
static int compose_table_size = sizeof(compose_table_2set) / sizeof(HangulCompose);

int PASCAL set_compose(UINT type)
{
    INT atype,i;
    MyDebugPrint((TEXT("compose map type: %d\r\n"), type));
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
        case 4:
           compose_table = (HangulCompose *) compose_table_yet;
           compose_table_size =
		   sizeof(compose_table_yet) / sizeof(HangulCompose);
	   dwComposeFlag=4;
	   break;
	default:
	    // User defined compose map
#define USER_DEFINED_COMPOSE_OFFSET	5
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
static int (*hangul_automata)(HangulIC*, WCHAR, LPMYSTR, int *)= hangul_automata2;

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
	case LAYOUT_389:
            atype=
             load_keyboard_map_from_reg(TEXT("세벌식 389"),0,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    } else {
		ctype=1;
	    }
	    break;
	case LAYOUT_YET2BUL:
            atype=
	     load_keyboard_map_from_reg(TEXT("옛한글 두벌"),0,keyboard_table_user);
	    if (atype)
	        keyboard_table=keyboard_table_user;
	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    } else {
		ctype=2;
	    }
	    break;
	case LAYOUT_YET3BUL:
            atype=
             load_keyboard_map_from_reg(TEXT("옛한글 세벌"),0,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    } else {
		ctype=1;
	    }
	    break;
	case LAYOUT_NEW2BUL:
            atype=
	     load_keyboard_map_from_reg(TEXT("새두벌"),0,keyboard_table_user);
	    if (atype)
	        keyboard_table=keyboard_table_user;
	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    } else {
		ctype=2;
	    }
	    break;
	case LAYOUT_NEW3BUL:
            atype=
             load_keyboard_map_from_reg(TEXT("새세벌"),0,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    } else {
	        ctype=1;
	    }
	    break;
	case LAYOUT_AHNMATAE:
            atype=
             load_keyboard_map_from_reg(TEXT("안마태"),0,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    } else {
		ctype=3;
	    }
	    break;
	default:
	    // User defined keyboard
#define USER_DEFINED_KEYBOARD_OFFSET	11
	    type-=USER_DEFINED_KEYBOARD_OFFSET;
            atype=
             load_keyboard_map_from_reg(NULL,type,keyboard_table_user);
	    if (atype)
                keyboard_table=keyboard_table_user;
	    else
                keyboard_table=(WCHAR *)keyboard_table_2;

	    if (atype & 0xff00) {
		ctype = (atype & 0xff00) >> 8;
		atype = atype & 0x00ff;
	    MyDebugPrint((TEXT("user defined keyboard ctype: %x\r\n"), ctype));

	    } else {
		ctype=0;
	    MyDebugPrint((TEXT("user defined keyboard type: %x\r\n"), atype));

	    }
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
    if (hcur == LongToHandle(0xE0130412) && code >= TEXT('!') && code <= TEXT('~'))
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

/*****************************************************************************
*                                                                            *
* MyHanCharPrevW( )                                                          *
*                                                                            *
* XXX L*V*T*
*****************************************************************************/
LPWSTR PASCAL MyHanCharPrevW(LPWSTR lpStart, LPWSTR lpCur)
{
    LPWSTR lpRet = lpStart;
    if (lpCur > lpStart) {
        while (hangul_is_jongseong(*lpRet)) {
            lpRet--;
        }

        while (hangul_is_jungseong(*lpRet) || *lpRet == 0x1160) {
            lpRet--;
        }

        while (hangul_is_choseong(*lpRet) || *lpRet == 0x115f) {
            lpRet--;
        }
    }
    if (lpRet == lpStart)
	lpRet--;

    return lpRet;
}

int hangul_del_prev(LPMYSTR lptr)
{
    int flag = 0;
    LPMYSTR lpSave = lptr;

    MyDebugPrint((TEXT("-- Input for del_prev: %s %x\r\n"), lptr, *lptr));
    if (hangul_is_cjamo(*lptr) || hangul_is_syllable(*lptr)) {
        MyDebugPrint((TEXT("--- CJamo or syllable: %s %x\r\n"), lptr, *lptr));
	lptr--;
	return 1;
    }
    if (hangul_is_jongseong(*lptr)) {
        MyDebugPrint((TEXT("--- Jong del_prev: %s %x\r\n"), lptr, *lptr));
        lptr--;
        if (hangul_is_jungseong(*lptr) || *lptr == 0x1160) {
	    lptr--;
	    if (hangul_is_choseong(*lptr) || *lptr == 0x115f) {
		lptr--;
	    }
	}
    } else if (hangul_is_jungseong(*lptr) || *lptr == 0x1160) {
	MyDebugPrint((TEXT("--- Jung del_prev: %s %d\r\n"), lptr));
        lptr--;
	if (hangul_is_choseong(*lptr) || *lptr == 0x115f) {
	    MyDebugPrint((TEXT("---- Cho Han Char: %s %d\r\n"), lptr));
	    lptr--;
	}
    } else if (hangul_is_choseong(*lptr) || *lptr == 0x115f) {
	MyDebugPrint((TEXT("--- Cho del_prev: %s %x\r\n"), lptr, *lptr));
        lptr--;
    }

    return (int) (lpSave - lptr);
}

void PASCAL hangulKeyHandler( hIMC, wParam, lParam, lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    WORD code = (WORD) HIWORD(wParam);
    WORD scan = (WORD) HIWORD(lParam);
    WORD ocode;

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
    MYCHAR lcs[MAXCOMPSIZE];
    register BOOL fModeInit = FALSE;
    BOOL capital = lpbKeyState[VK_CAPITAL];
    lcs[0] = 0;

    MyDebugPrint((TEXT("scan= 0x%x, code= 0x%x\r\n"), scan, code));

    lpIMC = ImmLockIMC(hIMC);
    // Get ConvMode from IMC.
    fdwConversion = lpIMC->fdwConversion;

    ocode = code;

    if (dwScanCodeBased) {
	WORD mycode;
	if (fdwConversion & IME_CMODE_NATIVE &&
		scan >= 0x02 && scan <=0x35) {
	    if (IsSHFTPushed(lpbKeyState)) {
		mycode = scan2qwerty[scan - 0x02].code[1];
	    } else {
		mycode = scan2qwerty[scan - 0x02].code[0];
	    }
	    if (mycode)
		hkey = code = mycode; // scan based conversion
	    else
		hkey = code;
	} else {
	    hkey = code;
	}
	MyDebugPrint((TEXT("scancode based = 0x%x, code= 0x%x\r\n"), scan, code));
    } else {
	if (fdwConversion & IME_CMODE_NATIVE) {
	    if (capital) {
		WORD ch = code;
		if (ch >= 'a' && ch <= 'z')
		    code -= 'a' - 'A';
		if (ch >= 'A' && ch <= 'Z')
		    code += 'a' - 'A';
	    }
	    /* check dvorak layout */
	    hkey= code = checkDvorak(code);
	} else {
	    hkey= code;
	}
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

    if (0 && IsConvertedCompStr(hIMC)) // 후보창을 보여주면서 그와 동시에 글자를 계속 조합할 수 있도록. XXX
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
		if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS)) {
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
	} else if ( (hkey >= 0x1100 && hkey <= 0x11ff) &&
                   !(dwOptionFlag & FULL_MULTIJOMO) &&
                    (fdwConversion & IME_CMODE_NATIVE) )
        {
	    int ret, ncs;
	    MYCHAR	marks[128];
	    UINT    nmark = 0;
	    marks[0] = MYTEXT('\0');

            MyDebugPrint((TEXT("input jamo: %x\r\n"), hkey));

	    // Combining Mark가 있는경우 - combining mark를 저장해둔다.
	    if (IsCompStr(hIMC) && is_combining_mark(*(lpstr - 1))) {
		UINT i = 0;
		MYCHAR	m;
		while (is_combining_mark(*(lpstr - 1 - i))) {
		    m = *(lpstr - 1 - i);
		    if (is_combining_mark(m))
			marks[i] = m;
		    else
			break;
		    i++;
		}
		lpstr -= i;
		lpCompStr->dwCursorPos -= i;
		// mark counter.
		nmark = i;
	    }

            ret = hangul_automata(&ic, (WCHAR) hkey, lcs, &ncs);
	    if (ncs)
		cs = lcs[ncs - 1]; /* XXX */

            MyDebugPrint((TEXT("automata result : %d, %s\r\n"), ncs, lcs));

            ic.lastvkey = (UINT)VkKeyScan(ocode);
            MyDebugPrint((TEXT(" * last vkey: 0x%x\r\n"), ic.lastvkey));
            MyDebugPrint((TEXT(" * last scancode: 0x%x\r\n"), lParam >> 16));


	    if (ncs)
	    {
		// not composable. emit hangul syllable
		if (ncs == 1 && ic.syllable) { // syllable
		    MYCHAR c = *(lpstr - 1);
		    if (hangul_is_syllable(c) || hangul_is_cjamo(c)) {
			// syllable이거나 compatible jamo이면
			*(lpstr - 1) = cs;
		    } else if (hangul_is_choseong(c) || hangul_is_jungseong(c) || hangul_is_jongseong(c)) {
			    // 그렇지 않고 첫가끝 자모인 경우.
			    int n = 0;
			    LPWSTR lpstrprev = lpstr - 1;
			    n = hangul_del_prev(lpstrprev);

			    MyDebugPrint((TEXT(">> Delete Han Char: %d\r\n"), n));

			    lpCompStr->dwCursorPos -= n;
			    lpstr -= n;

			    *lpstr++ = cs;
			    lpCompStr->dwCursorPos++;
			    *lpstr = MYTEXT('\0');
		    } else {
			// 그밖에
			*(lpstr - 1) = cs;
		    }
		} else if (ncs > 0) { // 옛한글 XXX
		    /*
                    lpprev = MyHanCharPrev( lpchText, lpstr );
		    MyDebugPrint((TEXT(" * 한글 앞글자: %s - %d\r\n"), lpprev, (lpstr - lpprev)));
		    */

		    // for automata2 XXX
		    if (hangul_is_jongseong(*(lpstr - 1)) && !hangul_is_jongseong(lcs[ncs - 1])) {
			*(lpstr - 1) = MYTEXT('\0');
			lpCompStr->dwCursorPos--;
			lpstr--;
		    } else {
			// lpstr (compStr)을 lcs로 모두 대체해야 한다.
			// 다음과 같이 편법적으로 할 수도 있다.
			*(lpstr - 1) = lcs[ncs - 1];
		    }

		    MyDebugPrint((TEXT(" * 옛한글: %s\r\n"), lcs));
		}

		// combining mark를 되돌린다.
		if (nmark > 0) {
		    UINT i = 0;
		    for (i = 0; i < nmark; i++) {
			*(lpstr + i) = marks[i];
		    }
		    lpstr += nmark;
		    lpCompStr->dwCursorPos += nmark;
		    nmark = 0;
		}
		*lpstr = MYTEXT('\0');

		if (dwImeFlag & SAENARU_ONTHESPOT && !(dwOptionFlag & HANGUL_JAMOS))
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
		// 조합중
		int ncs;
		ncs = hangul_ic_get(&ic, 0, lcs);
		if (ncs > 0)
		    cs = lcs[ncs - 1]; // XXX 마지막 입력 문자로
		MyDebugPrint((TEXT(" * 조합중: %d, %s, syl=%d\r\n"), ncs, lcs,ic.syllable));

		if (ncs) {
		    if (ncs == 1 && ic.syllable) {
			// composable with compatability jamo
			MYCHAR c = *(lpstr - 1);
			// syllable이거나 compatible jamo이면
			if (hangul_is_syllable(c) || hangul_is_cjamo(c)) {
			    *(lpstr - 1) = cs;
			} else if (hangul_is_choseong(c) ||
				hangul_is_jungseong(c) ||
				hangul_is_jongseong(c)) {
			    // 그렇지 않고 첫가끝 자모인 경우.
			    int n = 0;
			    LPWSTR lpstrprev = lpstr - 1;
			    n = hangul_del_prev(lpstrprev);

			    MyDebugPrint((TEXT("** Delete Han Char: %d\r\n"), n));
			    lpCompStr->dwCursorPos -= n;
			    lpstr -= n;

			    *lpstr++ = cs;
			    lpCompStr->dwCursorPos++;

			} else {
			    // 그밖에.
			    *(lpstr - 1) = cs;
			}
		    } else {
			int n = 0;
			LPWSTR lpstrprev = lpstr - 1;

			// delete prev compstr
			n = hangul_del_prev(lpstrprev);
			lpstr -= n;
			lpCompStr->dwCursorPos -= n;

			for (n = 0; n < ncs; n++) {
			    *lpstr++ = lcs[n];
			    lpCompStr->dwCursorPos++;
			}
		    }
		}
		// combining mark를 되돌린다.
		if (nmark > 0) {
		    UINT i = 0;
		    for (i = 0; i < nmark; i++) {
			*(lpstr + i) = marks[i];
		    }
		    lpstr += nmark;
		    lpCompStr->dwCursorPos += nmark;
		    nmark = 0;
		}
	    } else {
		// combining mark를 되돌린다.
		if (nmark > 0) {
		    UINT i = 0;
		    for (i = 0; i < nmark; i++) {
			*(lpstr + i) = marks[i];
		    }
		    lpstr += i;
		    lpCompStr->dwCursorPos += i;
		    nmark = 0;
		}

		// 새조합
		// new hangul composition
		ncs = hangul_ic_get(&ic,0,lcs);
		cs = lcs[0];

		*lpstr++ = cs;
               	lpCompStr->dwCursorPos++;
	    }
	    *lpstr = MYTEXT('\0');
        }
	else
	{
	    if (!hkey) hkey = code;
	    code = (WORD) hkey;
            MyDebugPrint((TEXT("ascii code: %x\r\n"), (WORD)hkey));
            if ((fdwConversion & IME_CMODE_NATIVE) && IsCompStr(hIMC) && is_combining_mark(hkey)) {
		cs = *(lpstr-1);
		MyDebugPrint((TEXT("CompStr code: %x\r\n"), (WORD)cs));
	    } else {
		/* for Hanme Typing tutor */
		MakeResultString(hIMC,FALSE);
		InitCompStr(lpCompStr,CLR_UNDET);

		dwGCR = GCS_RESULTALL;

		hangul_ic_init(&ic);
		lpchText = GETLPCOMPSTR(lpCompStr);
		lpstr = lpchText;
	    }

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
        if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS))
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
	if (!cs) {
            MakeResultString(hIMC,TRUE);
	} else if (ic.len > 1 && // 초성+중성 두글자 이상일 경우.
		IsCompStr(hIMC) &&
		(fdwConversion & IME_CMODE_HANJACONVERT) &&
                    (fdwConversion & IME_CMODE_NATIVE)) {
	    // 입력 즉시 후보 창 뜨게 하기.
	    BOOL candOk = FALSE;
	    CANDIDATEFORM lc;
	    /*
	    candOk = MyImmRequestMessage(hIMC, IMR_CANDIDATEWINDOW, (LPARAM)&lc);   
	    if (candOk) {
                // 어떤 어플은 자체 내장된 candidate리스트를 쓰고, 어떤것은 그렇지 않다.
                // 그런데 이러한 동작이 일관성이 없어서 어떤 어플은 특별처리 해야 한다.
                // always generage WM_IME_KEYDOWN for VK_HANJA XXX
                TRANSMSG GnMsg;
                GnMsg.message = WM_IME_KEYDOWN;
                GnMsg.wParam = wParam;
                GnMsg.lParam = lParam;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                OutputDebugString(TEXT("Auto DicKeydown: WM_IME_KEYDOWN\r\n"));
            }
	    */
            if (!candOk) {
                ConvHanja(hIMC,0,0);
            }
	}
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

        if (((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS)) && resultlen)
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
            if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS))
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
    ic->last0=0;
    ic->len=0;
    ic->laststate=0;
    ic->lastvkey=0;
    ic->syllable=TRUE;
}

UINT PASCAL hangul_ic_get( HangulIC *ic, UINT mode, LPMYSTR lcs)
{
    WCHAR code=0;
    UINT i = 0;

    if (!ic->len) {
	if (lcs)
	    lcs[0] = MYTEXT('\0');
	return 0;
    }

    if (ic->len == 1)
    {
	if (ic->cho)
            code = hangul_jamo_to_cjamo(ic->cho);
	else if (ic->jung)
            code = hangul_jamo_to_cjamo(ic->jung);
	else if (ic->jong)
            code = hangul_jamo_to_cjamo(ic->jong);

	if (!hangul_is_cjamo(code)) {
	    ic->syllable = FALSE;
	}
    }
    else if (ic->len == 3)
    {
        code = hangul_jamo_to_syllable(ic->cho,ic->jung,ic->jong);
	if (!ic->syllable && hangul_is_syllable(code))
	    ic->syllable = TRUE;
    }
    else if (ic->len == 2)
    {
#define HANGUL_PSEUDO_FILL_CHO	0x110b
#define HANGUL_PSEUDO_FILL_JUNG	0x1161
	if (ic->cho && ic->jung) {
            code = hangul_jamo_to_syllable(ic->cho,ic->jung,0);
	    if (!ic->syllable && hangul_is_syllable(code))
		ic->syllable = TRUE;
	}
	else
	{
            if (dwOptionFlag & HANGUL_JAMOS) {
		ic->syllable = FALSE;
	    } else {
		if (ic->cho && ic->jong)
		    code = hangul_jamo_to_syllable(ic->cho,0x1161,ic->jong);
		else if (ic->jung && ic->jong)
		    code = hangul_jamo_to_syllable(0x110b,ic->jung,ic->jong);

		if (!ic->syllable && hangul_is_syllable(code))
		    ic->syllable = TRUE;
	    }
	}
    }

    if (ic->len > 3 || !ic->syllable) {
	int state = 0;
	UINT j = 0;

	if (lcs == NULL)
	    return 0;

	if (ic->len == 1 && hangul_is_cjamo(code)) {
	    if (mode == 1)
		hangul_ic_init(ic);

	    lcs[0] = code;
	    lcs[1] = MYTEXT('\0');
	    return 1;
	}

	// the hangul ic buffer does not have fill code info.
	while (i < ic->len && hangul_is_choseong(ic->read[i])) {
	    lcs[j++] = ic->read[i++];
	    state = 1;
	}
	if (state != 1) {
	    lcs[j++] = 0x115f;
	    state = 1;
	}

	while (i < ic->len && hangul_is_jungseong(ic->read[i])) {
	    lcs[j++] = ic->read[i++];
	    state = 2;
	}
	if (state != 2) {
	    lcs[j++] = 0x1160;
	    state = 2;
	}

	while (i < ic->len) {
	    lcs[j++] = ic->read[i++];
	}

	lcs[j] = MYTEXT('\0');

	if (mode == 1)
	    hangul_ic_init(ic);

	// 개수 리턴
	return j;
    }

    if (mode == 1)
        hangul_ic_init(ic);

    lcs[0] = code;
    lcs[1] = MYTEXT('\0');
    return 1; // syllable일 경우.
}

void hangul_ic_push( HangulIC *ic, WCHAR ch )
{
    ic->read[ic->len++] = ch;
    if (ic->last)
	ic->last0 = ic->last;
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

#define hangul_ic_commit(ic,lcs) hangul_ic_get(ic, 1, lcs)

int hangul_automata2( HangulIC *ic, WCHAR jamo, LPMYSTR lcs, int *ncs )
{
    int kind;
    WCHAR comb=0, cho=0, jong=0, last=0, tmp=0,tmp2=0;
    BOOL ctyping=0;
    *ncs = 0;

    if (jamo >= 0x1100 && jamo <= 0x115e) kind=1;
    else if (jamo >= 0x1161 && jamo <= 0x11a7) kind=2;
    else if (jamo >= 0x11a8 && jamo <= 0x11ff) kind=3;
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
                // 초성+초성은 두벌식의 윈도우 기본 오토마타에서 원래 지원하지 않는다.
                // 그러나, 나비가 지원하므로 지원한다 :)
                comb = hangul_compose(ic->cho, jamo);
		// 동시치기일 경우는 순서를 바꿔서 초+초 조합
		if ( ctyping && !comb)
		    comb = hangul_compose(jamo,ic->cho);

                if ( comb ) {
		    if ( hangul_is_choseong(comb) ) {
		        ic->cho=comb;
                        hangul_ic_pop(ic);
                        hangul_ic_push(ic,comb);
                        ic->last=jamo;

		        //if (!hangul_jamo_to_syllable(ic->cho, 0x1161,0)) {
		        if (hangul_jamo_to_cjamo(ic->cho) == ic->cho) {
		 	    // 옛한글이면 완성형 음절로 만들지 못함.
			    ic->syllable=FALSE;
		        }

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
                        return 0;
		    }
                }
                // 초성을 compose할 수 없다.
                *ncs = hangul_ic_commit(ic, lcs);

                ic->cho=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=1;
                return -1;
                break;
            case 2: // 초성+중성
                if (hangul_jamo_to_syllable(ic->cho,jamo,0)) {
		    ic->jung=jamo;
		    hangul_ic_push(ic,jamo);
		    ic->laststate=2;
		    return 0;
                } else if (dwOptionFlag & HANGUL_JAMOS) {
		    // 옛한글 조합.
		    ic->jung=jamo;
		    hangul_ic_push(ic,jamo);
		    ic->laststate=2;
                    ic->syllable=FALSE;
		    return -2;
		} else {
                    // 허락되지 않는 조합.
                    *ncs = hangul_ic_commit(ic, lcs);

                    ic->jung=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=2;
                    return -1;
                }
                break;
            default: // 초성+종성
                // 두벌식 오토마타는 종성처리가 필요 없다.
                // 그러나, 세벌식에서 두벌식 오토마타를 쓰기
                // 위해서 종성 처리를 넣는다.
                *ncs = hangul_ic_commit(ic, lcs);

                ic->jong=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=3;
		//ic->syllable=FALSE; // XXX
                return -1;
                break;
            }
	} else if (ic->laststate == 2) {
            switch(kind) {
            case 1: // TODO 중성 + 초성
                // 초성이 없을 경우 : 나비에서는 중성+초성을 교정한다.
                if (!ic->cho &&
                   ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
		    int commit = FALSE;
			// 입력 순서 교정.
			// 따라서, ㅏ,ㄹ 를 차례로 입력하면 "라"가 된다.
			// 여기서 backspace를 누르면 "ㅏ"가 지워지게 된다.
			// FIXME "ㅏ"가 지워지는 것이 옳은가? "ㄹ"이 지워지는 게 맞나?
		    if (!hangul_jamo_to_syllable(jamo,ic->jung,0)) {
			if (dwOptionFlag & HANGUL_JAMOS) {
			    ic->syllable = FALSE;
			} else {
			    commit = TRUE;
			}
		    }

		    if (commit) {
			*ncs = hangul_ic_commit(ic, lcs);

			ic->cho=jamo;
			ic->laststate=1;
			hangul_ic_push(ic,jamo);
			return -1;
		    }
		    ic->cho=jamo;
		    hangul_ic_pop(ic);
		    hangul_ic_push(ic,jamo);
		    hangul_ic_push(ic,ic->jung);
                    ic->laststate=2;
		    return 0;	
                }
                // 초성을 종성으로 바꾸고,
                jong = hangul_choseong_to_jongseong_full(jamo);
                if (jong && hangul_jamo_to_syllable(ic->cho,ic->jung,jong)) {
                    ic->jong=jong;
                    hangul_ic_push(ic,jong);
                    ic->laststate=3;
                    return 0;
                } else if (jong && dwOptionFlag & HANGUL_JAMOS) {
		    // 옛한글
                    ic->jong=jong;
                    hangul_ic_push(ic,jong);
                    ic->laststate=3;
		    ic->syllable=FALSE;
                    return 0;
                } else {
		    // 종성 아님
                    *ncs = hangul_ic_commit(ic, lcs);

                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 중성 + 중성
                comb = hangul_compose(ic->jung, jamo);
		// 동시치기일 경우는 순서를 바꿔서 중+중 조합
		if ( !comb && ctyping && !hangul_is_jungseong(comb))
		    comb = hangul_compose(jamo,ic->jung);
                if ( comb && hangul_is_jungseong(comb) ) {
		    ic->jung=comb;
		    hangul_ic_pop(ic);
		    hangul_ic_push(ic,comb);
		    ic->last=jamo;

		    if (ic->len == 1) {
		        if (hangul_jamo_to_cjamo(ic->jung) == ic->jung) {
			    ic->syllable=FALSE;
			}
			return 0;
		    } else if (ic->cho && hangul_jamo_to_syllable(ic->cho,comb,0)) {
			return 0;
		    } else {
			// 옛한글이면 완성형 음절 아님.
			// XXX
			ic->syllable=FALSE;
			return -1;
		    }
                } else {
                    // XXX 자음을 연달아 누르는 경우는 많으나,모음을 연달아
                    // 누르는 경우는 드물다.
		    // ㅠㅠ 등등.
                    // 1. 원래 있던 종성을 대치하게 하는가 ?
                    *ncs = hangul_ic_commit(ic, lcs);

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
                } else if (dwOptionFlag & HANGUL_JAMOS) {
		    // 옛한글 조합.
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    ic->syllable=FALSE;
                    return 0;
                } else {
                    *ncs = hangul_ic_commit(ic, lcs);

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
                jong = hangul_choseong_to_jongseong_full(jamo);
                comb = hangul_compose(ic->jong, jong);
		// 동시치기일 경우는 순서를 바꿔서 종+종 조합
		if ( ctyping && !hangul_is_jongseong(comb))
		    comb = hangul_compose(jong,ic->jong);
                if ( comb && hangul_is_jongseong(comb) )
                {
		    if (!hangul_jamo_to_syllable(ic->cho,ic->jung,comb)) {
			ic->syllable = FALSE;
		    }

                    if (ic->syllable || dwOptionFlag & HANGUL_JAMOS)
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
			// 첫가끝 입력의 경우 두벌식으로는 처리하기 곤란하다.
			// 1) 종성 조합도 쉽게 가능하게 하되, 화살표 ->키를 조합을 끝내는 키로 생각하게끔
			// 2) 종성을 누를때 반드시 shift를 쓰게끔.
			// 3) shift를 누르고 자음을 치면 초성으로 인식하게끔.
		    }
		}
                {
                    // 초성을 compose할 수 없다.
                    *ncs = hangul_ic_commit(ic, lcs);

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
			ic->jong = hangul_choseong_to_jongseong_full(tmp);
		    } else {
			ic->jong = jong;
		    }
		    hangul_ic_pop(ic);
		    hangul_ic_push(ic,ic->jong);
		} else {
		    // 분해되지 않으면 단종성의 경우
		    // 종성 + 중성: 마지막 종성을 초성으로 바꾸고 잠시 저장
		    cho = hangul_jongseong_to_choseong(ic->jong);
		    if (cho)
			ic->jong = 0; //종성을 지운다.
		    hangul_ic_pop(ic);
                }

		*ncs = hangul_ic_commit(ic, lcs);

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
                    *ncs = hangul_ic_commit(ic, lcs);

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

int hangul_automata3( HangulIC *ic, WCHAR jamo, LPMYSTR lcs, int *ncs )
{
    int kind;
    WCHAR comb=0, cho=0, jung=0, jong=0;
    BOOL ctyping;

    *ncs = 0;

    if (jamo >= 0x1100 && jamo <= 0x115e) kind=1;
    else if (jamo >= 0x1161 && jamo <= 0x11a7) kind=2;
    else if (jamo >= 0x11a8 && jamo <= 0x11ff) kind=3;
    else return -1;

    // 동시치기이고, 연타가 아니고, key_down 이벤트인 경우
    ctyping = (dwOptionFlag & CONCURRENT_TYPING) && (ic->last != jamo) &&
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

		// comb가 안되고 동시치기인 경우는 순서를 바꿔서 조합
		if ( !comb && ctyping )
		    comb = hangul_compose(jamo, ic->cho);

                if ( hangul_is_choseong(comb) )
                {
                    ic->cho=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=1;

 		    if (!hangul_jamo_to_syllable(ic->cho,0x1161,0)) {
			// 옛한글이면 완성형 음절로 만들지 못함.
			ic->syllable=FALSE;
		    }

                    return 0;
                }
                // 초성을 compose할 수 없다.
                *ncs = hangul_ic_commit(ic, lcs);

                ic->cho=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=1;
                return -1;
                break;
            case 2: // 초성+중성
                if (hangul_jamo_to_syllable(ic->cho,jamo,0)) {
		    ic->jung=jamo;
		    hangul_ic_push(ic,jamo);
		    ic->laststate=2;
		    return 0;
		} else if (dwOptionFlag & HANGUL_JAMOS) {
		    // 옛한글 조합.
		    ic->jung=jamo;
		    hangul_ic_push(ic,jamo);
		    ic->laststate=2;
                    ic->syllable=FALSE;
		    return -2;
		} else {
                    // 허락되지 않는 조합.
                    *ncs = hangul_ic_commit(ic, lcs);

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
                *ncs = hangul_ic_commit(ic, lcs);

                ic->jong=jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate=3;
		//ic->syllable=FALSE; // XXX 초성+[ ]+종성의 경우. syllable이 가능한지 판별해야 함. FIXME
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
			// 입력순서 교정.
			// ㅏ + ㄹ 입력 => 라.
			// 여기서 backspace를 누르면 ㄹ이 지워져야 하는가? ㅏ가 지워져야 하는가?
			// FIXME
                    ic->cho=jamo;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,jamo);
                    hangul_ic_push(ic,ic->jung);
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

			if (j1 && j2 && ic->last0 == j1 && ic->last == j2) {
			    // 중성이 분해되고 last0 == j1, last == j2가 맞는 경우.
			    hangul_ic_pop(ic);
			    hangul_ic_push(ic,j1);
			    ic->jung = j1;
			    jung = j2;
			} else if (hangul_is_jungseong(ic->last0)) {
			    // 중성을 분해할 수 없는 경우: ㅜ+ㅜ=ㅠ
			    // last0, last 버퍼를 근거로 되돌림.
			    j1 = ic->last0;
			    j2 = ic->last;
			    hangul_ic_pop(ic);
			    hangul_ic_push(ic,j1);
			    ic->jung = j1;
			    jung = j2;
			}
                    } else
                        jung = 0;
                    *ncs = hangul_ic_commit(ic, lcs);

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
                    *ncs = hangul_ic_commit(ic, lcs);

                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 중성 + 중성
                comb = hangul_compose(ic->jung, jamo);
		// 동시치기일 경우 순서를 바꿔서 조합
		if (ctyping && !comb )
		    comb = hangul_compose(jamo,ic->jung);
                if ( hangul_is_jungseong(comb) ) {
                    ic->jung=comb;
                    hangul_ic_pop(ic);
                    hangul_ic_push(ic,comb);
                    ic->last=jamo;
                    ic->laststate=2;

		    if (hangul_jamo_to_syllable(ic->cho,comb,0)) {
			return 0;
		    } else {
			// 옛한글이면 완성형 음절 아님.
			// XXX
			ic->syllable=FALSE;
			return -1;
		    }
                    return 0;
                } else {
                    // XXX 자음을 연달아 누르는 경우는 많으나,모음을 연달아
                    // 누르는 경우는 드물다.
                    // 1. 원래 있던 종성을 대치하게 하는가 ?
                    *ncs = hangul_ic_commit(ic, lcs);

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
			    *ncs = hangul_ic_commit(ic, lcs);

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
                } else if (dwOptionFlag & HANGUL_JAMOS) {
		    // 옛한글 조합.
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    ic->syllable=FALSE;
                    return 0;
                }
#if 1
		// 초성과 종성이 없는 상태에서,
		// 중성 + 종성인 경우.
                else if (!ic->jong && !ic->cho &&
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    ic->jong=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=3;
                    return 0;
                }
#endif
                {
                    *ncs = hangul_ic_commit(ic, lcs);

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
                    if (!hangul_jamo_to_syllable(jamo,ic->jung ? ic->jung:0x1161,ic->jong)) {
			ic->syllable = FALSE;
		    } else {
			ic->syllable = TRUE;
		    }
		    if (ic->syllable || dwOptionFlag & HANGUL_JAMOS)
		    {
			// XXX
			ic->cho=jamo;
			hangul_ic_pop(ic);
			hangul_ic_push(ic,jamo);
			hangul_ic_push(ic,ic->jong); // fix order
			ic->laststate=3;
			return 0;
		    }
                    *ncs = hangul_ic_commit(ic, lcs);

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
			    jong = hangul_choseong_to_jongseong_full(j2); // XXX
			    hangul_ic_pop(ic);
			    hangul_ic_push(ic,j1);
			    ic->jong = j1;
			    *ncs = hangul_ic_commit(ic, lcs);

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
                    *ncs = hangul_ic_commit(ic, lcs);

                    ic->cho = jamo;
                    ic->jong = jong;
                    hangul_ic_push(ic,jamo);
                    hangul_ic_push(ic,jong);
                    ic->laststate = 3;
                    return -1;
                }
                {
                    // compose할 수 없다.
                    *ncs = hangul_ic_commit(ic, lcs);

                    ic->cho=jamo;
                    hangul_ic_push(ic,jamo);
                    ic->laststate=1;
                    return -1;
                }
                break;
            case 2: // 종성 + 중성
                if (!ic->jung &&
                    ( (dwOptionFlag & COMPOSITE_TYPING) || ctyping ) ) {
                    if (!hangul_jamo_to_syllable(ic->cho ? ic->cho:0x110b,jamo,ic->jong)) {
			ic->syllable = FALSE;
		    } else {
			ic->syllable = TRUE;
		    }

		    if (ic->syllable || dwOptionFlag & HANGUL_JAMOS)
		    {
			ic->jung=jamo;
			hangul_ic_pop(ic);
			hangul_ic_push(ic,jamo);
			hangul_ic_push(ic,ic->jong); // fix order
			ic->laststate=3;
			return 0;
		    }
                    *ncs = hangul_ic_commit(ic, lcs);

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
			    jong = hangul_choseong_to_jongseong_full(j2);
			    hangul_ic_pop(ic);
			    hangul_ic_push(ic,j1);
			    ic->jong = j1;
			    *ncs = hangul_ic_commit(ic, lcs);

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
                    *ncs = hangul_ic_commit(ic, lcs);

                    ic->jung = jamo;
                    ic->jong = jong;
                    hangul_ic_push(ic,jamo);
                    hangul_ic_push(ic,jong);
                    ic->laststate = 3;
                    return -1;
                }
                *ncs = hangul_ic_commit(ic, lcs);

                ic->jung = jamo;
                hangul_ic_push(ic,jamo);
                ic->laststate = 2;
                return -1; // 앞의 초+중 두개를 commit
                break;
            default: // 종성 + 종성
		comb = 0;
		if (hangul_is_jongseong(ic->last))
		    comb = hangul_compose(ic->jong, jamo);
                if ( comb && hangul_is_jongseong(comb) )
		{
		    if (ic->len == 1) { // has only ic->jong
			if (hangul_jamo_to_cjamo(comb) == comb)
			    ic->syllable = FALSE;
		    } else if (!hangul_jamo_to_syllable(ic->cho, ic->jung, comb)) {
			ic->syllable = FALSE;
		    }
		    if (ic->syllable || dwOptionFlag & HANGUL_JAMOS) {
			ic->jong=comb;
			hangul_ic_pop(ic);
			hangul_ic_push(ic,comb);
			ic->last=jamo;
			ic->laststate=3;
			return 0;
		    }
		}
		{
                    *ncs = hangul_ic_commit(ic, lcs);

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
