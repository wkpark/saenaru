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

#include <windows.h>
#include "immdev.h"
#include "saenaru.h"
#include "hangul.h"
#include "vksub.h"

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
  { ______, ______ }, /* backspace */
  { ______, ______ }, /* tab */
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
  { ______, ______ }, /* enter */
  { ______, ______ }, /* ??? */
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
  { ______, ______ }, /* L-Shift */
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

DWORD PASCAL checkHangulKey( hIMC, wParam, lParam, lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    WORD code = (WORD) HIWORD(wParam);
    WORD scan = (WORD) (HIWORD(lParam) & 0xFF);
    WORD ocode;

    DWORD fdwConversion;
    LPINPUTCONTEXT lpIMC;

    DWORD hkey;
    BOOL capital = lpbKeyState[VK_CAPITAL];

    MyDebugPrint((TEXT("check scan= 0x%x, code= 0x%x\r\n"), scan, code));

    if (!hIMC)
        return 0;

    lpIMC = ImmLockIMC(hIMC);
    if (!lpIMC)
        return 0;
    // Get ConvMode from IMC.
    fdwConversion = lpIMC->fdwConversion;

    ocode = code;

    if (dwScanCodeBased) {
	WORD mycode;
	if (fdwConversion & IME_CMODE_NATIVE &&
		scan >= 0x02 && scan <=0x35) {
	    // code conversion based on ScanCode
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
	    // use VK keycode.
	    hkey = code;
	}
	MyDebugPrint((TEXT("check scancode based = 0x%x, code= 0x%x\r\n"), scan, code));
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

    if (fdwConversion & IME_CMODE_NATIVE) {
	hkey = keyToHangulKey( code );

	if (dwScanCodeBased > 1 && hkey == code) {
	    // there are no keymap entry.
	    // use original VK codes.
	    hkey = code = (WORD) HIWORD(wParam);
	}
    }
    ImmUnlockIMC(hIMC);
    return hkey;
}

void PASCAL hangulKeyHandler( hIMC, wParam, lParam, lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
CONST LPBYTE lpbKeyState;
{
    WORD code = (WORD) HIWORD(wParam);
    WORD scan = (WORD) (HIWORD(lParam) & 0xFF);
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

    if (!hIMC)
        return;

    lpIMC = ImmLockIMC(hIMC);

    if (!lpIMC)
        return;

    // Get ConvMode from IMC.
    fdwConversion = lpIMC->fdwConversion;

    ocode = code;

    if (dwScanCodeBased) {
	WORD mycode;
	if (fdwConversion & IME_CMODE_NATIVE &&
		scan >= 0x02 && scan <=0x35) {
	    // code conversion based on ScanCode
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
	    // use VK keycode.
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

    if (fdwConversion & IME_CMODE_NATIVE) {
	hkey = keyToHangulKey( code );

	if (dwScanCodeBased > 1 && hkey == code) {
	    // there are no keymap entry.
	    // use original VK codes.
	    hkey = code = (WORD) HIWORD(wParam);
	}
    }
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
	LPCANDIDATEINFO lpCandInfo;
	LPCANDIDATELIST lpCandList;

	// 후보창을 보여주면서 그와 동시에 글자를 계속 조합할 수 있도록.
	lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
	if (lpCandInfo) {
	    // 후보창 정보를 가져온다. dwSelection != 0인 경우는 한글이 이미 한자로 변환된 경우
	    // 이 경우는 선택된 한자를 커밋하고 다시 한글을 입력받는 모드로
	    lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
	    if (lpCandList && lpCandList->dwSelection != 0) {
		// converted.
		MakeResultString(hIMC,FALSE);
		InitCompStr(lpCompStr,CLR_UNDET);
		dwGCR = GCS_RESULTALL;

		hangul_ic_init(&ic);
	    }
	    ImmUnlockIMCC(lpIMC->hCandInfo);
	}
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
		//if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS)) {
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

		//if (dwImeFlag & SAENARU_ONTHESPOT && !(dwOptionFlag & HANGUL_JAMOS))
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

    /*
    if (lpCompStr->dwCompReadStrLen > 0)
        lpCompStr->dwCompReadStrLen--;
    */

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
        GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
        //if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS))
        if (dwImeFlag & SAENARU_ONTHESPOT) {
	    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; //한글 IME 2002,2003
            GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
	}
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
	} else if ((lpCompStr->dwCompStrLen > 1 || ic.len > 1) && // 초성+중성 두글자 이상일 경우.
		IsCompStr(hIMC) &&
		(fdwConversion & IME_CMODE_HANJACONVERT) &&
                    (fdwConversion & IME_CMODE_NATIVE)) {
	    // 입력 즉시 후보 창 뜨게 하기.
            ConvHanja(hIMC,0,0);
	} else if (IsCandidate(lpIMC)) {
	    // 이미 한자창이 떠있는 경우에도
            ConvHanja(hIMC,0,0);
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

        //if (((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS)) && resultlen)
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
            GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
            //if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS))
            if (dwImeFlag & SAENARU_ONTHESPOT) {
		//GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
                GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
	    }
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

	    if (ic.len > 1 && // 초성+중성 두글자 이상일 경우.
		IsCompStr(hIMC) &&
		(fdwConversion & IME_CMODE_HANJACONVERT) &&
                    (fdwConversion & IME_CMODE_NATIVE)) {

		// 입력 즉시 후보 창 뜨게 하기.
                ConvHanja(hIMC,0,0);
	    }
	}

    }
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}
