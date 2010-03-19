/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 1990-1998 Microsoft Corporation.
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
 * $Saenaru: saenaru/src/subs.c,v 1.4 2006/10/03 13:09:49 wkpark Exp $
 */

/**********************************************************************/

#include <windows.h>
#include "immdev.h"
#include "saenaru.h"

/**********************************************************************/
/*                                                                    */
/*      InitCompStr()                                                  */
/*                                                                    */
/**********************************************************************/
void PASCAL InitCompStr(LPCOMPOSITIONSTRING lpCompStr,DWORD dwClrFlag)
{
    lpCompStr->dwSize = sizeof(MYCOMPSTR);

    if (dwClrFlag & CLR_UNDET)
    {
        lpCompStr->dwCompReadAttrOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->bCompReadAttr - (LONG_PTR) lpCompStr);
        lpCompStr->dwCompReadClauseOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->dwCompReadClause - (LONG_PTR)lpCompStr);
        lpCompStr->dwCompReadStrOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->szCompReadStr - (LONG_PTR)lpCompStr);
        lpCompStr->dwCompAttrOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->bCompAttr - (LONG_PTR)lpCompStr);
        lpCompStr->dwCompClauseOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->dwCompClause - (LONG_PTR)lpCompStr);
        lpCompStr->dwCompStrOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->szCompStr - (LONG_PTR)lpCompStr);

        lpCompStr->dwCompStrLen = 0;
        lpCompStr->dwCompReadStrLen = 0;
        lpCompStr->dwCompAttrLen = 0;
        lpCompStr->dwCompReadAttrLen = 0;
        lpCompStr->dwCompClauseLen = 0;
        lpCompStr->dwCompReadClauseLen = 0;

        *GETLPCOMPSTR(lpCompStr) = MYTEXT('\0');
        *GETLPCOMPREADSTR(lpCompStr) = MYTEXT('\0');

        lpCompStr->dwCursorPos = 0;
    }


    if (dwClrFlag & CLR_RESULT)
    {
        lpCompStr->dwResultStrOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->szResultStr - (LONG_PTR)lpCompStr);
        lpCompStr->dwResultClauseOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->dwResultClause - (LONG_PTR)lpCompStr);
        lpCompStr->dwResultReadStrOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->szResultReadStr - (LONG_PTR)lpCompStr);
        lpCompStr->dwResultReadClauseOffset = 
            (DWORD) ((LONG_PTR)((LPMYCOMPSTR)lpCompStr)->dwResultReadClause - (LONG_PTR)lpCompStr);

        lpCompStr->dwResultStrLen = 0;
        lpCompStr->dwResultClauseLen = 0;
        lpCompStr->dwResultReadStrLen = 0;
        lpCompStr->dwResultReadClauseLen = 0;

        *GETLPRESULTSTR(lpCompStr) = MYTEXT('\0');
        *GETLPRESULTREADSTR(lpCompStr) = MYTEXT('\0');
    }

}

/**********************************************************************/
/*                                                                    */
/*      ClearCompStr()                                                */
/*                                                                    */
/**********************************************************************/
void PASCAL ClearCompStr(LPCOMPOSITIONSTRING lpCompStr,DWORD dwClrFlag)
{
    lpCompStr->dwSize = sizeof(MYCOMPSTR);

    if (dwClrFlag & CLR_UNDET)
    {
        lpCompStr->dwCompStrOffset = 0;
        lpCompStr->dwCompClauseOffset = 0;
        lpCompStr->dwCompAttrOffset = 0;
        lpCompStr->dwCompReadStrOffset = 0;
        lpCompStr->dwCompReadClauseOffset = 0;
        lpCompStr->dwCompReadAttrOffset = 0;
        lpCompStr->dwCompStrLen = 0;
        lpCompStr->dwCompClauseLen = 0;
        lpCompStr->dwCompAttrLen = 0;
        lpCompStr->dwCompReadStrLen = 0;
        lpCompStr->dwCompReadClauseLen = 0;
        lpCompStr->dwCompReadAttrLen = 0;
        ((LPMYCOMPSTR)lpCompStr)->szCompStr[0] = MYTEXT('\0');
        ((LPMYCOMPSTR)lpCompStr)->szCompReadStr[0] = MYTEXT('\0');
        lpCompStr->dwCursorPos = 0;
    }

    if (dwClrFlag & CLR_RESULT)
    {
        lpCompStr->dwResultStrOffset = 0;
        lpCompStr->dwResultClauseOffset = 0;
        lpCompStr->dwResultReadStrOffset = 0;
        lpCompStr->dwResultReadClauseOffset = 0;
        lpCompStr->dwResultStrLen = 0;
        lpCompStr->dwResultClauseLen = 0;
        lpCompStr->dwResultReadStrLen = 0;
        lpCompStr->dwResultReadClauseLen = 0;
        ((LPMYCOMPSTR)lpCompStr)->szResultStr[0] = MYTEXT('\0');
        ((LPMYCOMPSTR)lpCompStr)->szResultReadStr[0] = MYTEXT('\0');
    }

}

/**********************************************************************/
/*                                                                    */
/*      ClearCandidate()                                              */
/*                                                                    */
/**********************************************************************/
void PASCAL ClearCandidate(LPCANDIDATEINFO lpCandInfo)
{
    lpCandInfo->dwSize = 0L;
    lpCandInfo->dwCount = 0L;
    lpCandInfo->dwOffset[0] = 0L;
    
    ((LPMYCAND)lpCandInfo)->cl.dwSize =0L;
    ((LPMYCAND)lpCandInfo)->cl.dwStyle =0L;
    ((LPMYCAND)lpCandInfo)->cl.dwCount =0L;
    ((LPMYCAND)lpCandInfo)->cl.dwSelection =0L;
    ((LPMYCAND)lpCandInfo)->cl.dwPageStart =0L;
    ((LPMYCAND)lpCandInfo)->cl.dwPageSize =0L;
    ((LPMYCAND)lpCandInfo)->cl.dwOffset[0] =0L;

}
/**********************************************************************/
/*                                                                    */
/*      ChangeMode()                                                  */
/*                                                                    */
/*    return value: fdwConversion                                     */
/*                                                                    */
/**********************************************************************/
void PASCAL ChangeMode(HIMC hIMC, DWORD dwToMode)
{
    LPINPUTCONTEXT lpIMC;
    DWORD fdwConversion;
    TRANSMSG GnMsg;
    BOOL fOpen;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    fOpen = ImmGetOpenStatus(hIMC);

    if (!fOpen)
        ImmSetOpenStatus(hIMC, TRUE);

    fdwConversion = lpIMC->fdwConversion;

    switch (dwToMode)
    {
        case TO_CMODE_ALPHANUMERIC:
            fdwConversion = (fdwConversion & ~IME_CMODE_LANGUAGE);
            break;

        case TO_CMODE_HANGUL:
            // XXX turn off FULLSHAPE
            if (fdwConversion & IME_CMODE_FULLSHAPE)
                fdwConversion &= ~IME_CMODE_FULLSHAPE;

            // Toggle CHAR MODE to the HANGUL composition mode
            if (fdwConversion & IME_CMODE_HANGUL) {
                fOpen = FALSE;
                fdwConversion &= ~IME_CMODE_HANGUL;
                fdwConversion = (fdwConversion & ~IME_CMODE_LANGUAGE);
            } else {
                fOpen = TRUE;
                fdwConversion &= ~IME_CMODE_LANGUAGE;
                fdwConversion |= (IME_CMODE_NATIVE | IME_CMODE_HANGUL);
            }
            break;

/*
        case TO_CMODE_HIRAGANA:
            fdwConversion = 
                ((fdwConversion & ~IME_CMODE_LANGUAGE) | IME_CMODE_NATIVE);
            fdwConversion |= IME_CMODE_FULLSHAPE;
            break;
*/

        case TO_CMODE_FULLSHAPE:
            if (fdwConversion & IME_CMODE_FULLSHAPE) {
                // To SBCS mode.
                fdwConversion &= ~IME_CMODE_FULLSHAPE;
 
                // If hiragana mode, make it katakana mode.
                if ((fdwConversion & IME_CMODE_LANGUAGE) == IME_CMODE_NATIVE)
                    fdwConversion |= (IME_CMODE_NATIVE | IME_CMODE_HANGUL);
            } else {
                // To DBCS mode.
                fdwConversion |= IME_CMODE_FULLSHAPE;
            }
            break;

        case TO_CMODE_ROMAN:
            if (fdwConversion & IME_CMODE_ROMAN)
                fdwConversion &= ~IME_CMODE_ROMAN;
            else
                fdwConversion |= IME_CMODE_ROMAN;
            break;

        case TO_CMODE_CHARCODE:
            break;
        case TO_CMODE_TOOLBAR:
            break;
        default:
            break;
    }

    ImmSetConversionStatus (hIMC, fdwConversion, 0);

    if (!fOpen)
        ImmSetOpenStatus(hIMC, FALSE);

    ImmUnlockIMC(hIMC);
    return;    
}

/**********************************************************************/
/*                                                                    */
/*      ChangeCompStr()                                               */
/*                                                                    */
/**********************************************************************/
void PASCAL ChangeCompStr(HIMC hIMC, DWORD dwToMode)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD fdwConversion;
    TRANSMSG GnMsg;
    HANDLE hDst;
    LPMYSTR lpSrc;
    LPMYSTR lpDst;
    LPMYSTR lpSrc0;
    LPMYSTR lpDst0;
    WORD wCode;
    BOOL fChange = FALSE;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)))
        goto ccs_exit40;

    fdwConversion = lpIMC->fdwConversion;

    if (!(hDst = GlobalAlloc(GHND,(lpCompStr->dwCompStrLen+1)*sizeof(WCHAR))))
        goto ccs_exit30;

    if (!(lpDst = GlobalLock(hDst)))
        goto ccs_exit20;


    switch (dwToMode)
    {
        case TO_CMODE_ALPHANUMERIC:
            break;

        case TO_CMODE_HANGUL:
            lpSrc = ((LPMYCOMPSTR)lpCompStr)->szCompStr;
            lpSrc0 = lpSrc;
            lpDst0 = lpDst;
            while (*lpSrc)
            {
                *lpDst++ = *lpSrc; // XXX
                lpSrc++;
            }
            Mylstrcpy (lpSrc0,lpDst0);
            lpCompStr->dwCompStrLen = Mylstrlen(lpSrc0);
            fChange = TRUE;
            break;
        /*
            lpSrc = ((LPMYCOMPSTR)lpCompStr)->szCompStr;
            lpSrc0 = lpSrc;
            lpDst0 = lpDst;
            while (*lpSrc)
            {
                *lpDst++ = HiraToKata(*lpSrc);
                lpSrc++;
            }
            Mylstrcpy (lpSrc0,lpDst0);
            lpCompStr->dwCompStrLen = Mylstrlen(lpSrc0);
            fChange = TRUE;
            break;

        case TO_CMODE_HIRAGANA:
            lpSrc = ((LPMYCOMPSTR)lpCompStr)->szCompStr;
            lpSrc0 = lpSrc;
            lpDst0 = lpDst;
            while (*lpSrc)
            {
                *lpDst++ = KataToHira(*lpSrc);
                lpSrc++;
            }
            Mylstrcpy (lpSrc0,lpDst0);
            lpCompStr->dwCompStrLen = Mylstrlen(lpSrc0);
            fChange = TRUE;
            break;
        */

        case TO_CMODE_FULLSHAPE:
            break;

        case TO_CMODE_ROMAN:
            break;
    }

    if (fChange)
    {
        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = GCS_COMPSTR;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    GlobalUnlock(hDst);
ccs_exit20:
    GlobalFree(hDst);
ccs_exit30:
    ImmUnlockIMCC(lpIMC->hCompStr);
ccs_exit40:
    ImmUnlockIMC(hIMC);
    return;
}

/*****************************************************************************
*                                                                            *
* IsCompStr( hIMC )                                                          *
*                                                                            *
*****************************************************************************/
BOOL PASCAL IsCompStr(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    BOOL fRet = FALSE;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return FALSE;

    if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof (COMPOSITIONSTRING))
    {
        ImmUnlockIMC(hIMC);
        return FALSE;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    fRet = (lpCompStr->dwCompStrLen > 0);

    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);

    return fRet;
}
/*****************************************************************************
*                                                                            *
* IsConvertedCompStr( hIMC )                                                 *
*                                                                            *
*****************************************************************************/
BOOL PASCAL IsConvertedCompStr(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    BOOL fRet = FALSE;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return FALSE;

    if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof (MYCOMPSTR))
    {
        ImmUnlockIMC(hIMC);
        return FALSE;
    }

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    if (lpCompStr->dwCompStrLen > 0)
        fRet = (((LPMYCOMPSTR)lpCompStr)->bCompAttr[0] > ATTR_INPUT);

    MyDebugPrint((TEXT("CompAttr: %d\n"),((LPMYCOMPSTR)lpCompStr)->bCompAttr[0]));

    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);

    return fRet;
}
/*****************************************************************************
*                                                                            *
* IsCandidate( lpIMC )                                                       *
*                                                                            *
*****************************************************************************/
BOOL PASCAL IsCandidate(LPINPUTCONTEXT lpIMC)
{
    LPCANDIDATEINFO lpCandInfo;
    BOOL fRet = FALSE;

    if (ImmGetIMCCSize(lpIMC->hCandInfo) < sizeof (CANDIDATEINFO))
        return FALSE;

    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

    fRet = (lpCandInfo->dwCount > 0);

    ImmUnlockIMCC(lpIMC->hCandInfo);
    return fRet;
}

/**********************************************************************/
/*                                                                    */
/*      GetMyHKL()                                                    */
/*                                                                    */
/**********************************************************************/
HKL PASCAL GetMyHKL()
{
    DWORD dwSize;
    DWORD dwi;
    HKL hKL = 0;
    HKL *lphkl;

    dwSize = GetKeyboardLayoutList(0, NULL);

    lphkl = (HKL *)GlobalAlloc(GPTR, dwSize * sizeof(DWORD));

    if (!lphkl)
        return NULL;

    GetKeyboardLayoutList(dwSize, lphkl);


    for (dwi = 0; dwi < dwSize; dwi++)
    {
        TCHAR szFile[32];
        HKL hKLTemp = *(lphkl + dwi);
        ImmGetIMEFileName(hKLTemp, szFile, sizeof(szFile));

        if (!lstrcmpi(szFile, MyFileName))
        {
             hKL = hKLTemp;
             goto exit;
        }
    }
exit:

    GlobalFree((HANDLE)lphkl);
    return hKL;
}
/*****************************************************************************
*                                                                            *
* UpdateIndicIcon( hIMC )                                                    *
*                                                                            *
*****************************************************************************/
void PASCAL UpdateIndicIcon(HIMC hIMC)
{
    HWND hwndIndicate;
    BOOL fOpen = FALSE;
    LPINPUTCONTEXT      lpIMC;

    if (!hMyKL)
    {
       hMyKL = GetMyHKL();
       if (!hMyKL)
           return;
    }

#if !defined (NO_TSF)
    UpdateLanguageBar ();
#endif

    hwndIndicate = FindWindow(INDICATOR_CLASS, NULL);

    if (IsWindow(hwndIndicate))
    {
        ATOM atomTip;

        atomTip = GlobalAddAtom(TEXT("SaeNaru Open"));
        PostMessage(hwndIndicate, INDICM_SETIMEICON, 
                    fOpen ? 1 : (-1), (LPARAM)hMyKL);
        PostMessage(hwndIndicate, INDICM_SETIMETOOLTIPS, 
                    fOpen ? atomTip : (-1), (LPARAM)hMyKL);
        PostMessage(hwndIndicate, INDICM_REMOVEDEFAULTMENUITEMS, 
                    // fOpen ? (RDMI_LEFT | RDMI_RIGHT) : 0, (LPARAM)hMyKL);
                    fOpen ? (RDMI_LEFT) : 0, (LPARAM)hMyKL);
    }
}

/*****************************************************************************
*                                                                            *
* lememset( )                                                                *
*                                                                            *
*****************************************************************************/
void PASCAL lmemset(LPBYTE lp, BYTE b, UINT cnt)
{
    register UINT i;
    register BYTE bt = b;
    for (i=0;i<cnt;i++)
        *lp++ = bt;
}

/*****************************************************************************
*                                                                            *
* MylstrcmpW( )                                                              *
*                                                                            *
*****************************************************************************/
int PASCAL MylstrcmpW(LPWSTR lp0, LPWSTR lp1)
{
    while(*lp0 && *lp1 && (*lp0 == *lp1)) {
                lp0++;
                lp1++;
        }
        return (*lp0 - *lp1);
}
/*****************************************************************************
*                                                                            *
* MylstrcpyW( )                                                              *
*                                                                            *
*****************************************************************************/
int PASCAL MylstrcpyW(LPWSTR lp0, LPWSTR lp1)
{
    int n = 0;

    while(*lp1)
    {
        *lp0 = *lp1;
        lp0++;
        lp1++;
        n++;
    }
    *lp0 = *lp1;
    return n;
}
/*****************************************************************************
*                                                                            *
* MyCharPrevW( )                                                             *
*                                                                            *
*****************************************************************************/
LPWSTR PASCAL MyCharPrevW(LPWSTR lpStart, LPWSTR lpCur)
{
    LPWSTR lpRet = lpStart;
    if (lpCur > lpStart)
        lpRet = lpCur - 1;

    return lpRet;
}
/*****************************************************************************
*                                                                            *
* MyCharNextW( )                                                             *
*                                                                            *
*****************************************************************************/
LPWSTR PASCAL MyCharNextW(LPWSTR lp)
{
    return lp++;
}
/*****************************************************************************
*                                                                            *
* MylstrcpynW( )                                                             *
*                                                                            *
*****************************************************************************/
LPWSTR PASCAL MylstrcpynW(LPWSTR lp0, LPWSTR lp1, int nCount)
{
    int n;
    for (n = 0; *lp1 && n < nCount - 1; *lp0++ = *lp1++, n++)
               ;
    *lp0 = L'\0';
    return lp0;
}

HFONT CheckNativeCharset(HDC hDC) 
{
    BOOL bDiffCharSet = FALSE;
    LOGFONT lfFont;
    HFONT hOldFont, hTemp;
    static TCHAR font[256] = TEXT("");
    LPTSTR lpFont;
    long ret;

    lpFont = font;

    if (font[0] == TEXT('\0')) {
        ret = GetRegStringValue(NULL, TEXT("CandFont"), lpFont);
        if (ret == -1)
            /* 기본값을 새굴림으로 한다 */
            lpFont = TEXT("New Gulim");
    }

    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(LOGFONT), &lfFont);

    lfFont.lfWeight = FW_NORMAL;
    lfFont.lfCharSet = NATIVE_CHARSET;
    Mylstrcpy(lfFont.lfFaceName,lpFont);
    hTemp = CreateFontIndirect(&lfFont);
    hOldFont = SelectObject(hDC, hTemp);
    /* 아무 글꼴이 없으면 기본 값을 가져온다 */
    if (lfFont.lfCharSet != NATIVE_CHARSET) {
        DeleteObject(hTemp);

        bDiffCharSet = TRUE;
        lfFont.lfWeight = FW_NORMAL;
        lfFont.lfCharSet = NATIVE_CHARSET;
        lfFont.lfFaceName[0] = TEXT('\0');
        hOldFont = SelectObject(hDC, CreateFontIndirect(&lfFont));
    }
#ifdef DEBUG
    MyDebugPrint((TEXT("Cand FONT: %s\n"),lfFont.lfFaceName));
#endif
    return hOldFont;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
