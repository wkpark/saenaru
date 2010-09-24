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
 * $Saenaru: saenaru/src/imm.c,v 1.27 2007/12/29 16:50:28 wkpark Exp $
 */

#include "windows.h"
#include "immdev.h"
#include "saenaru.h"

int GetCandidateStringsFromDictionary(LPWSTR lpString, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR szDicFileName);

/**********************************************************************/
/*      ImeInquire()                                                  */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeInquire(LPIMEINFO lpIMEInfo,LPTSTR lpszClassName,DWORD dwSystemInfoFlags)
{
    ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeInquire"));

    // Init IMEINFO Structure.
    lpIMEInfo->dwPrivateDataSize = sizeof(UIEXTRA);
    lpIMEInfo->fdwProperty = IME_PROP_KBD_CHAR_FIRST |
                             IME_PROP_UNICODE |
                             IME_PROP_COMPLETE_ON_UNSELECT |
                             IME_PROP_AT_CARET;
    lpIMEInfo->fdwConversionCaps = IME_CMODE_LANGUAGE |
                                IME_CMODE_FULLSHAPE |
//                                IME_CMODE_NOCONVERSION |
                                IME_CMODE_ROMAN |
                                IME_CMODE_SOFTKBD |
                                IME_CMODE_CHARCODE;
    lpIMEInfo->fdwSentenceCaps = 0L;
    lpIMEInfo->fdwUICaps = UI_CAP_2700
                           | UI_CAP_SOFTKBD
                           ;

    lpIMEInfo->fdwSCSCaps = SCS_CAP_COMPSTR |
                            SCS_CAP_MAKEREAD |
                            SCS_CAP_SETRECONVERTSTRING;

    lpIMEInfo->fdwSelectCaps = SELECT_CAP_CONVERSION;

    lstrcpy(lpszClassName,(LPTSTR)szUIClassName);


    /* WIN Logon */
    gfSaenaruSecure = ((dwSystemInfoFlags & IME_SYSINFO_WINLOGON) != 0) ;
    return TRUE;
}

/**********************************************************************/
/*      ImeConversionList()                                           */
/*                                                                    */
/**********************************************************************/
DWORD WINAPI ImeConversionList(HIMC hIMC,LPCTSTR lpSource,LPCANDIDATELIST lpCandList,DWORD dwBufLen,UINT uFlags)
{
    ImeLog(LOGF_API, TEXT("ImeConversionList"));

    return 0;
}

/**********************************************************************/
/*      ImeDestroy()                                                  */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeDestroy(UINT uForce)
{
    ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeDestroy"));

    return TRUE;
}

/**********************************************************************/
/*      ImeEscape()                                                   */
/*                                                                    */
/**********************************************************************/
LRESULT WINAPI ImeEscape(HIMC hIMC,UINT uSubFunc,LPVOID lpData)
{
    LRESULT lRet = FALSE;
    LPINPUTCONTEXT lpIMC;
    LPTRANSMSG lpTransMsg;
    BOOL fOpen;

    ImeLog(LOGF_API, TEXT("ImeEscape"));

    switch (uSubFunc)
    {
        case IME_ESC_QUERY_SUPPORT:
            switch (*(LPUINT)lpData)
            {
                case IME_ESC_QUERY_SUPPORT:
                case IME_ESC_PRI_GETDWORDTEST:
                case IME_ESC_GETHELPFILENAME:
                case IME_ESC_HANJA_MODE:
                    lRet = TRUE;
                    break;

                default:
                    lRet = FALSE;
                    break;
            }
            break;

        case IME_ESC_HANJA_MODE:
            // EditPlus,AcroEdit only use it. XXX
            // EditPlus can not receive IMR_* message
            MyDebugPrint((TEXT("\tIME_ESC_HANJA_MODE:\r\n")));
            if (lpData == NULL) return FALSE;

            hangul_ic_init(&ic);

            lpIMC = ImmLockIMC(hIMC);
            if (!lpIMC) return FALSE;

            fOpen = lpIMC->fOpen;
            if (!fOpen) {
                MyDebugPrint((TEXT("\t === XXX IME NOT OPEN:\r\n")));
                //ImmUnlockIMC(hIMC);
                //return FALSE;
                ImmSetOpenStatus(hIMC,TRUE);
            }

#ifndef USE_NO_IME_ESCAPE
            while (!IsCompStr(hIMC)) {
        		    LPCOMPOSITIONSTRING	lpCompStr;
                            LPMYSTR lpDump, lpToken;
                            MYCHAR szBuf[256];

                            lpDump=szBuf;

                            // 노트패드에서는 이상하게 lpData가 제대로 세팅이 안된다.
                            if (Mylstrlen(lpData) < 255) {
                                const LPMYSTR szSep = MYTEXT(" \r\n\t");
                                MyDebugPrint((TEXT("ImeEscape '%s':%d\n"),lpData, Mylstrlen(lpData)));

                                lpToken = Mystrtok(lpData, szSep);
                                if (lpToken == NULL) {
                                    TRANSMSG GnMsg;
                                    lRet = FALSE;

        		            lpCompStr =
                                        (LPCOMPOSITIONSTRING)ImmLockIMCC (lpIMC->hCompStr);
        		            if (lpCompStr != NULL) {
                                        InitCompStr(lpCompStr,CLR_RESULT_AND_UNDET);
                                        ImmUnlockIMCC (lpIMC->hCompStr);
                                    }
#if 1
                                    // 노트패드에서 이상하게 쓰레기값이 붙는다.
                                    // 그래서 WM_IME_ENDCOMPOSITION 메시지를 날리니
                                    // 정상적으로 작동
                                    GnMsg.message = WM_IME_ENDCOMPOSITION;
                                    GnMsg.wParam = 0;
                                    GnMsg.lParam = 0;
                                    GenerateMessage(hIMC, lpIMC, NULL,(LPTRANSMSG)&GnMsg);
#endif

                                    break;
                                }
                                Mylstrcpy(lpDump,lpToken);
                                MyDebugPrint((TEXT("ImeEscape '%s'\n"),lpToken));
                            } else {
                                Mylstrcpyn(lpDump,lpData,2); // XXX
                                szBuf[1]=MYTEXT('\0');
                                MyDebugPrint((TEXT("ImeEscape #2 '%s'\n"),lpDump));
                            }
 
        		    if (ImmGetIMCCSize (lpIMC->hCompStr) < sizeof (MYCOMPSTR))
                            {
                                DWORD dwSize = sizeof(MYCOMPSTR);
                                lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr,dwSize);
                                lpCompStr =
                                    (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
                                lpCompStr->dwSize = dwSize;
                            } else
        		        lpCompStr =
                                    (LPCOMPOSITIONSTRING)ImmLockIMCC (lpIMC->hCompStr) ;
                            {
                                LPMYSTR lpstr,lpread;
        
        		        if (lpCompStr != NULL) {
                                    TRANSMSG GnMsg;
                                    // XXX
                                    InitCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

#if 1
                                    // ImeEscape()를 쓸 때
                                    // M$ Explorer는 반드시 WM_IME_STARTCOMPOSITION으로 시작해야 한다.
                                    // 2006/10/18
                                    GnMsg.message = WM_IME_STARTCOMPOSITION;
                                    GnMsg.wParam = 0;
                                    GnMsg.lParam = 0;
                                    GenerateMessage(hIMC, lpIMC, NULL,(LPTRANSMSG)&GnMsg);
#endif

                                    lpstr = GETLPCOMPSTR(lpCompStr);
                                    lpread = GETLPCOMPREADSTR(lpCompStr);
                                    Mylstrcpy(lpread,lpDump);
                                    Mylstrcpy(lpstr,lpDump);
        
                                    // delta start
                                    lpCompStr->dwDeltaStart =
        	                        (DWORD)(MyCharPrev(lpstr, lpstr+Mylstrlen(lpstr)) - lpstr);
                                    // cursor pos
                                    //lpCompStr->dwCursorPos-=lpRS->dwStrLen;
                                    //if (lpCompStr->dwCursorPos <= 0) lpCompStr->dwCursorPos=0;
                                    //lpCompStr->dwCursorPos = Mylstrlen(lpstr)-1; // Err
                                    //lpCompStr->dwCursorPos = 0;
                                    lpCompStr->dwCursorPos = Mylstrlen(lpstr);
        
                                    //MakeAttrClause(lpCompStr);
                                    lmemset((LPBYTE)GETLPCOMPATTR(lpCompStr),ATTR_INPUT,
                                            Mylstrlen(lpstr));
                                    lmemset((LPBYTE)GETLPCOMPREADATTR(lpCompStr),ATTR_INPUT,
                                            Mylstrlen(lpread));
        
                                    // make length
                                    lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
                                    lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
                                    lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
                                    lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);
        
                                    //
                                    // make clause info
                                    //
                                    SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
                                    SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
                                    lpCompStr->dwCompClauseLen = 8;
                                    lpCompStr->dwCompReadClauseLen = 8;
                                    //
                                    MyDebugPrint((TEXT(" *** dwCompStrLen= %d\r\n"),lpCompStr->dwCompStrLen));

#if 0
                                    GnMsg.message = WM_IME_COMPOSITION;
                                    GnMsg.wParam = 0;
                                    //GnMsg.wParam = cs;
                                    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
                                    GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; //한글 IME 2002,2003
                                    //if (dwImeFlag & SAENARU_ONTHESPOT)
                                    //    GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
                                    GenerateMessage(hIMC, lpIMC, NULL,(LPTRANSMSG)&GnMsg);
#endif
                                    ImmUnlockIMCC (lpIMC->hCompStr);
                                    lRet=TRUE;
        		        } else {
                                    OutputDebugString(TEXT(" *** lpCompStr== NULL\r\n"));
                                }
                            }
                            break;
            }

#endif
            if (!fOpen)
                ImmSetOpenStatus(hIMC,FALSE);

            ImmUnlockIMC (hIMC);

            break;

        case IME_ESC_PRI_GETDWORDTEST:
            lRet = 0x12345678;
            break;

        case IME_ESC_GETHELPFILENAME:
            Mylstrcpy((LPMYSTR)lpData, MYTEXT("saenaru.hlp"));
            MyDebugPrint((TEXT("\tIME_ESC_GETHELPFILENAME\r\n")));
            lRet = TRUE;
            break;

        default:
            lRet = FALSE;
            break;
    }

    return lRet;
}

/**********************************************************************/
/*      ImeSetActiveContext()                                         */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeSetActiveContext(HIMC hIMC,BOOL fFlag)
{
    ImeLog(LOGF_API, TEXT("ImeSetActiveContext"));

    UpdateIndicIcon(hIMC);

    return TRUE;
}

/**********************************************************************/
/*      ImeProcessKey()                                               */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeProcessKey(HIMC hIMC,UINT vKey,LPARAM lKeyData,CONST LPBYTE lpbKeyState)
{
    BOOL fRet = FALSE;
    BOOL fOpen;
    BOOL fCompStr = FALSE;
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;

    DWORD dwConversion, dwSentense ;
    WORD ch;
    UINT vkey = LOWORD(vKey) & 0x00FF;
    UINT vkToggle = (dwToggleKey & 0x00FF) ? dwToggleKey & 0x00FF : VK_SPACE;

    ImeLog(LOGF_KEY | LOGF_API, TEXT("ImeProcessKey"));

    {
        MyDebugPrint((TEXT("\tImeProcessKey: vKey is 0x%x\r\n"),vKey));
    }

#ifdef NO_MOZILLA_HACK
    if (lKeyData & 0x80000000)
    {
        return FALSE;
    }
#endif
    MyDebugPrint((TEXT("\t** lKeyData is 0x%x\r\n"),lKeyData));

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return FALSE;


    if (IsSHFTPushed(lpbKeyState) && vkey == VK_BACK && !IsCompStr(hIMC)) {
        return TRUE;
        // Make it option or default ? FIXME see also process.c
    }
    if (vkey == 0xFF) {
        // unknown VK Key.
        // builtin support HANGUL, HANJA
        UINT vk = (UINT) (lKeyData & 0x00ff0000) >> 16;
        if (vk == 0xf2) { // VK_HANGUL
            vKey = vkey = VK_HANGUL;
        } if (vk == 0xf1) { // VK_HANJA
            vKey = vkey = VK_HANJA;
        }
    }

    if ( !(lKeyData & 0x80000000) && 
            (LOWORD(vKey) & 0x00FF) == VK_HANGUL) {
        OSVERSIONINFO os;
        os.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
        //
        // IME 2002/2003 compatible VK_HANGUL event
        // 
        if (IsCompStr(hIMC))
            MakeResultString(hIMC,TRUE);

        SendMessage(lpIMC->hWnd, WM_IME_KEYDOWN, VK_PROCESSKEY, lKeyData);
        MyDebugPrint((TEXT("Generate VK_PROCESSKEY instead of VK_HANGUL\r\n")));

        // Windows7 amd64 workaround!! :(
        if (GetVersionEx(&os)) {
            MyDebugPrint((TEXT("WinVer %x\n"),
            os.dwMajorVersion));

            if (os.dwMajorVersion == 6) {
                ImmUnlockIMC(hIMC);
                return FALSE;
            }
        }
    }

    // use SHIFT-SPACE or User defined toggle key.
    // See ui.c how to hook the shift-space event.
    if ( !(lKeyData & 0x80000000) &&
            dwOptionFlag & USE_SHIFT_SPACE)
    {
        MyDebugPrint((TEXT(">>>>>>>> Check Hangul Toggle key=%x, vkey=%x\n"), vkToggle, LOWORD(vKey) & 0xFF));
        while ((LOWORD(vKey) & 0x00FF) == vkToggle) {
            UINT ModState = dwToggleKey & 0xffff0000;
            UINT ok;
            MyDebugPrint((TEXT(">>>>>>>> Hangul Toggle key\n")));

            if (ModState & MASK_SHIFT) {
                ok = 0;
                if ((ModState & MASK_LSHIFT) && lpbKeyState[VK_LSHIFT] & 0x80) ok = 1;
                else if ((ModState & MASK_RSHIFT) && lpbKeyState[VK_RSHIFT] & 0x80) ok = 1;
                if (!ok) break;
            }
            if (ModState & MASK_CTRL) {
                ok = 0;
                if ((ModState & MASK_LCTRL) && lpbKeyState[VK_LCONTROL] & 0x80) ok = 1;
                else if ((ModState & MASK_RCTRL) && lpbKeyState[VK_RCONTROL] & 0x80) ok = 1;
                if (!ok) break;
            }
            if (ModState & MASK_ALT) {
                ok = 0;
                if ((ModState & MASK_LALT) && lpbKeyState[VK_LMENU] & 0x80) ok = 1;
                else if ((ModState & MASK_RALT) && lpbKeyState[VK_RMENU] & 0x80) ok = 1;
                if (!ok) break;
            }

            vkey = VK_HANGUL;
            break;
        }

        // FIXME
        //if (vkey == VK_SPACE && vkToggle == VK_SPACE && dwImeFlag & USE_CTRL_SPACE && IsCTLPushed(lpbKeyState))
        if (IsCompStr(hIMC) && vkey == VK_SPACE && vkToggle == VK_SPACE && IsCTLPushed(lpbKeyState))
            vkey = VK_HANJA;
    }

    fOpen = lpIMC->fOpen;

    // support auto hangul/english toggle with M$ Word
#if 0
    if (!fOpen && ImmGetConversionStatus (hIMC, &dwConversion, &dwSentense) && dwConversion & IME_CMODE_NATIVE) {
        ImmSetOpenStatus(hIMC,TRUE);
        fOpen = TRUE;
    }
#endif

    switch ( vkey ) {
        case VK_HANJA:
            if ( lKeyData & 0x80000000 ) break;

            if (!IsCompStr(hIMC) && IsSHFTPushed(lpbKeyState)) {
                fOpen = ImmGetOpenStatus(hIMC);
                if (!fOpen)
                    ImmSetOpenStatus(hIMC,TRUE);

                if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentense))
                {
                    if (fOpen && dwConversion & IME_CMODE_HANJACONVERT)
                    {
                        dwConversion &= ~IME_CMODE_HANJACONVERT;
                        dwConversion &= ~IME_CMODE_FULLSHAPE;
                        MyDebugPrint((TEXT("x Hanja key\n")));
                    } else {
                        dwConversion |= IME_CMODE_HANJACONVERT;
                        dwConversion |= IME_CMODE_NATIVE;
                        MyDebugPrint((TEXT("o Hanja key\n")));
                        fOpen = TRUE;
                    }
                    ImmSetConversionStatus (hIMC, dwConversion, 0) ;
                }
                if (!fOpen)
                    ImmSetOpenStatus(hIMC,FALSE);

            }

            if ((LOWORD(vKey) & 0x00FF) == VK_SPACE) {
                // Ctrl-SPACE
                ImmUnlockIMC(hIMC);
                return TRUE;
            }
            break;
#if 0
        case VK_HANJA:
            if (IsCompStr(hIMC)) {
                ImmUnlockIMC(hIMC);
                return TRUE;
                break;
            }
            if (fOpen) break;

            ImmSetOpenStatus(hIMC,TRUE);
            ChangeMode(hIMC,TO_CMODE_ALPHANUMERIC);
            fOpen = TRUE;
            break;
#endif
        case VK_HANGUL:
            // for 101 Keyboard type 3 (shift-space enabled kbd)
            if ( lKeyData & 0x80000000 ) break;
            // Toggle Hangul composition state
            //
            if (IsCompStr(hIMC))
                 MakeResultString(hIMC,TRUE);

            fOpen = ImmGetOpenStatus(hIMC);
            if (!fOpen)
                ImmSetOpenStatus(hIMC,TRUE);

            if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentense))
            {
                if (fOpen && dwConversion & IME_CMODE_NATIVE)
                {
                    dwConversion &= ~IME_CMODE_NATIVE;
                    dwConversion &= ~IME_CMODE_FULLSHAPE;
                    MyDebugPrint((TEXT("x Hangul key\n")));
                    fOpen=FALSE;
                }
                else
                {
                    dwConversion &= ~IME_CMODE_FULLSHAPE;
                    dwConversion |= IME_CMODE_NATIVE;
                    MyDebugPrint((TEXT("O Hangul key\n")));
                    fOpen=TRUE;
                }
                ImmSetConversionStatus (hIMC, dwConversion, dwSentense) ;
            }
            if (!fOpen)
                ImmSetOpenStatus(hIMC,FALSE);

            ImmUnlockIMC(hIMC);

            return FALSE;
            break;
        case VK_ESCAPE:
            if (dwOptionFlag & ESCENG_SUPPORT)
                ChangeMode(hIMC,TO_CMODE_ALPHANUMERIC);
            //ChangeMode(hIMC,TO_CMODE_ROMAN);
            break;
        case VK_SHIFT:
        case VK_CONTROL:
            ImmUnlockIMC(hIMC);
            return FALSE;
            break;
        default:
            break;
    }

    fOpen = lpIMC->fOpen;

    if (fOpen)
    {
        if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
        {
            if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING)) &&
                (lpCompStr->dwCompStrLen))
                fCompStr = TRUE;
        }

        if (lpbKeyState[VK_MENU] & 0x80)
        {
            //MakeResultString(hIMC,TRUE);
            fRet = FALSE;
        }
        else if (lpbKeyState[VK_CONTROL] & 0x80)
        {
            if (fCompStr)
                fRet = (BOOL)bCompCtl[vKey];
            else
                fRet = (BOOL)bNoCompCtl[vKey];
        }
        else if (lpbKeyState[VK_SHIFT] & 0x80)
        {
            if (fCompStr)
                fRet = (BOOL)bCompSht[vKey];
            else
                fRet = (BOOL)bNoCompSht[vKey];
        }
        else
        {
            if (fCompStr)
                fRet = (BOOL)bComp[vKey];
            else
                fRet = (BOOL)bNoComp[vKey];
        }

        if (!fRet && IsCompStr(hIMC)) {
            UINT wParam;
            // Simulate KEYDOWN event.
            if ( lKeyData & 0x80000000 ) {
                if ((LOWORD(vKey) & 0x00FF) == VK_PROCESSKEY)
                    MakeResultString(hIMC,TRUE);

                fRet=TRUE;
            } else {
                wParam=LOWORD(vKey) & 0x00FF;
                MakeResultString(hIMC,TRUE);
                hangul_ic_init(&ic);
                if (lpbKeyState[VK_CONTROL] & 0x80) {
                    // simulate key for CTRL-X
                    keybd_event( (BYTE)wParam, 0x0, 0, 0 );
                    fRet=TRUE;
                }
            }
        }

        if (lpCompStr)
            ImmUnlockIMCC(lpIMC->hCompStr);

    }
    // Some application do not accept WM_CHAR events with VK_PROCESSKEY.
    // For this appls we need a following hack:
    if (fRet && !IsCompStr(hIMC)) {
        ToAscii(vKey,((UINT)lKeyData | 0xff0000)>>16,lpbKeyState,&ch,0);
        if (ch) {
            WORD nch;
            nch=checkDvorak(ch);
            nch=keyToHangulKey((WORD)nch);
            if ( ch==nch && (ch >= TEXT('!') && ch <= TEXT('~')) )
                fRet=FALSE;
        }
    }
    ImmUnlockIMC(hIMC);
    MyDebugPrint((TEXT("END of ImeProcessKey:%x, Ret=%x\n"), vKey, fRet));
    return fRet;
}

/**********************************************************************/
/*      NotifyIME()                                                   */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI NotifyIME(HIMC hIMC,DWORD dwAction,DWORD dwIndex,DWORD dwValue)
{
    LPINPUTCONTEXT      lpIMC;
    BOOL                bRet = FALSE;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO     lpCandInfo;
    LPCANDIDATELIST     lpCandList;
    MYCHAR              szBuf[256];
    int                 nBufLen;
    LPMYSTR             lpstr;
    TRANSMSG             GnMsg;
    int                 i = 0;
    LPDWORD             lpdw;

    ImeLog(LOGF_API, TEXT("NotifyIME"));

    switch(dwAction)
    {

        case NI_CONTEXTUPDATED:
             switch (dwValue)
             {
                 case IMC_SETOPENSTATUS:
                     lpIMC = ImmLockIMC(hIMC);
                     if (lpIMC)
                     {
                         if (!lpIMC->fOpen && IsCompStr(hIMC))
                             FlushText(hIMC);
                         ImmUnlockIMC(hIMC);
                     }
                     UpdateIndicIcon(hIMC);
                     bRet = TRUE;
                     break;

                 case IMC_SETCONVERSIONMODE:
                     break;

                 case IMC_SETCOMPOSITIONWINDOW:
                     break;

                 default:
                     break;
             }
             break;

        case NI_COMPOSITIONSTR:
             switch (dwIndex)
             {
                 case CPS_COMPLETE:
                     MakeResultString(hIMC,TRUE);
                     bRet = TRUE;
                     break;

                 case CPS_CONVERT:
                     ConvHanja(hIMC,1, 0);
                     bRet = TRUE;
                     break;

                 case CPS_REVERT:
                     RevertText(hIMC);
                     bRet = TRUE;
                     break;

                 case CPS_CANCEL:
                     FlushText(hIMC);
                     bRet = TRUE;
                     break;

                 default:
                     break;
             }
             break;

        case  NI_OPENCANDIDATE:
             if (IsConvertedCompStr(hIMC))
             {

                 if (!(lpIMC = ImmLockIMC(hIMC)))
                     return FALSE;

                 if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)))
                 {
                     ImmUnlockIMC(hIMC);
                     return FALSE;
                 }

                 if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
                 {

                     //
                     // Get the candidate strings from dic file.
                     //
                     nBufLen = GetCandidateStringsFromDictionary(GETLPCOMPREADSTR(lpCompStr),
                                                                 (LPMYSTR)szBuf,256,
                                                                 (LPTSTR)szDicFileName );

                     //
                     // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
                     //
                     GnMsg.message = WM_IME_NOTIFY;
                     GnMsg.wParam = IMN_OPENCANDIDATE;
                     GnMsg.lParam = 1L;
                     GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

                     //
                     // Make candidate structures.
                     //
                     lpCandInfo->dwSize = sizeof(MYCAND);
                     lpCandInfo->dwCount = 1;
                     lpCandInfo->dwOffset[0] =
                           (DWORD)((LPSTR)&((LPMYCAND)lpCandInfo)->cl - (LPSTR)lpCandInfo);
                     lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
                     lpdw = (LPDWORD)&(lpCandList->dwOffset);

                     lpstr = &szBuf[0];
                     while (*lpstr && (i < MAXCANDSTRNUM))
                     {
                         lpCandList->dwOffset[i] =
                                (DWORD)((LPSTR)((LPMYCAND)lpCandInfo)->szCand[i] - (LPSTR)lpCandList);
                         Mylstrcpy((LPMYSTR)((LPMYSTR)lpCandList+lpCandList->dwOffset[i]),lpstr);
                         lpstr += (Mylstrlen(lpstr) + 1);
                         i++;
                     }

                     lpCandList->dwSize = sizeof(CANDIDATELIST) +
                          (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
                     lpCandList->dwStyle = IME_CAND_READ;
                     lpCandList->dwCount = i;
                     lpCandList->dwPageStart = 0;
                     if (i < MAXCANDPAGESIZE)
                         lpCandList->dwPageSize  = i;
                     else
                         lpCandList->dwPageSize  = MAXCANDPAGESIZE;

                     lpCandList->dwSelection++;
                     if (lpCandList->dwSelection == (DWORD)i)
                         lpCandList->dwSelection = 0;

                     //
                     // Generate messages.
                     //
                     GnMsg.message = WM_IME_NOTIFY;
                     GnMsg.wParam = IMN_CHANGECANDIDATE;
                     GnMsg.lParam = 1L;
                     GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

                     ImmUnlockIMCC(lpIMC->hCandInfo);
                     ImmUnlockIMC(hIMC);

                     bRet = TRUE;
                 }
             }
             break;

        case  NI_CLOSECANDIDATE:
             if (!(lpIMC = ImmLockIMC(hIMC)))
                 return FALSE;
             if (IsCandidate(lpIMC))
             {
                 GnMsg.message = WM_IME_NOTIFY;
                 GnMsg.wParam = IMN_CLOSECANDIDATE;
                 GnMsg.lParam = 1L;
                 GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                 bRet = TRUE;
             }
             ImmUnlockIMC(hIMC);
             break;

        case  NI_SELECTCANDIDATESTR:
             if (!(lpIMC = ImmLockIMC(hIMC)))
                 return FALSE;

             if (dwIndex == 1 && IsCandidate(lpIMC))
             {

                 if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
                 {

                     lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
                     if (lpCandList->dwCount > dwValue)
                     {
                         lpCandList->dwSelection = dwValue;
                         bRet = TRUE;

                         //
                         // Generate messages.
                         //
                         GnMsg.message = WM_IME_NOTIFY;
                         GnMsg.wParam = IMN_CHANGECANDIDATE;
                         GnMsg.lParam = 1L;
                         GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                     }
                     ImmUnlockIMCC(lpIMC->hCandInfo);

                 }
             }
             ImmUnlockIMC(hIMC);
             break;

        case  NI_CHANGECANDIDATELIST:
             if (!(lpIMC = ImmLockIMC(hIMC)))
                return FALSE;

             if (dwIndex == 1 && IsCandidate(lpIMC))
                 bRet = TRUE;

             ImmUnlockIMC(hIMC);
             break;

        case NI_SETCANDIDATE_PAGESIZE:
             if (!(lpIMC = ImmLockIMC(hIMC)))
                return FALSE;

             if (dwIndex == 1 && IsCandidate(lpIMC))
             {
                 if (dwValue > MAXCANDPAGESIZE)
                     return FALSE;


                 if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
                 {
                     lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
                     if (lpCandList->dwCount > dwValue)
                     {
                         lpCandList->dwPageSize = dwValue;
                         bRet = TRUE;

                         //
                         // Generate messages.
                         //
                         GnMsg.message = WM_IME_NOTIFY;
                         GnMsg.wParam = IMN_CHANGECANDIDATE;
                         GnMsg.lParam = 1L;
                         GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                     }
                     ImmUnlockIMCC(lpIMC->hCandInfo);
                 }
             }
             ImmUnlockIMC(hIMC);
             break;

        case NI_SETCANDIDATE_PAGESTART:
             if (!(lpIMC = ImmLockIMC(hIMC)))
                 return FALSE;

             if (dwIndex == 1 && IsCandidate(lpIMC))
             {
                 if (dwValue > MAXCANDPAGESIZE)
                     return FALSE;


                 if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
                 {
                     lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
                     if (lpCandList->dwCount > dwValue)
                     {
                         lpCandList->dwPageStart = dwValue;
                         bRet = TRUE;

                         //
                         // Generate messages.
                         //
                         GnMsg.message = WM_IME_NOTIFY;
                         GnMsg.wParam = IMN_CHANGECANDIDATE;
                         GnMsg.lParam = 1L;
                         GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                     }
                     ImmUnlockIMCC(lpIMC->hCandInfo);

                 }
             }
             ImmUnlockIMC(hIMC);
             break;

        case NI_IMEMENUSELECTED:
#ifdef DEBUG
             {
             TCHAR szDev[80];
             OutputDebugString((LPTSTR)TEXT("NotifyIME IMEMENUSELECTED\r\n"));
             wsprintf((LPTSTR)szDev,TEXT("\thIMC is 0x%x\r\n"),hIMC);
             OutputDebugString((LPTSTR)szDev);
             wsprintf((LPTSTR)szDev,TEXT("\tdwIndex is 0x%x\r\n"),dwIndex);
             OutputDebugString((LPTSTR)szDev);
             wsprintf((LPTSTR)szDev,TEXT("\tdwValue is 0x%x\r\n"),dwValue);
             OutputDebugString((LPTSTR)szDev);
             }
#endif
             break;

        default:
             break;
    }
    return bRet;
}

/**********************************************************************/
/*      ImeSelect()                                                   */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeSelect(HIMC hIMC, BOOL fSelect)
{
    LPINPUTCONTEXT lpIMC;

    ImeLog(LOGF_ENTRY | LOGF_API, TEXT("ImeSelect"));

    if (fSelect)
        UpdateIndicIcon(hIMC);

    // it's NULL context.
    if (!hIMC)
        return TRUE;

    if (lpIMC = ImmLockIMC(hIMC))
    {
        if (fSelect)
        {
            LPCOMPOSITIONSTRING lpCompStr;
            LPCANDIDATEINFO lpCandInfo;

            // Init the general member of IMC.
            if (!(lpIMC->fdwInit & INIT_LOGFONT))
            {
                lpIMC->lfFont.A.lfCharSet = NATIVE_CHARSET;
                lpIMC->fdwInit |= INIT_LOGFONT;
            }


            if (!(lpIMC->fdwInit & INIT_CONVERSION))
            {
                // 한자 전용
                //lpIMC->fdwConversion = IME_CMODE_ROMAN | IME_CMODE_FULLSHAPE | IME_CMODE_NATIVE;
                // 시작부터 한글 ??
                //lpIMC->fdwConversion = IME_CMODE_ROMAN | IME_CMODE_NATIVE;
                lpIMC->fdwConversion = IME_CMODE_ROMAN;
                lpIMC->fdwConversion &= ~IME_CMODE_NATIVE;
                lpIMC->fdwInit |= INIT_CONVERSION;
            }

            lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr,sizeof(MYCOMPSTR));
            if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
            {
                lpCompStr->dwSize = sizeof(MYCOMPSTR);
                ImmUnlockIMCC(lpIMC->hCompStr);
            }
            lpIMC->hCandInfo = ImmReSizeIMCC(lpIMC->hCandInfo,sizeof(MYCAND));
            if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
            {
                lpCandInfo->dwSize = sizeof(MYCAND);
                ImmUnlockIMCC(lpIMC->hCandInfo);
            }
        }
#if !defined (NO_TSF)
        /*
        if (fSelect) {
            if (InitLanguageBar ())
                ActivateLanguageBar (TRUE) ;
        } else {
            ActivateLanguageBar (FALSE) ;
        }
        */
#endif
        ImmUnlockIMC(hIMC);
    }

    return TRUE;
}

#ifdef DEBUG
void DumpRS(LPRECONVERTSTRING lpRS)
{
    TCHAR szDev[80];
    LPMYSTR lpDump= ((LPMYSTR)lpRS) + lpRS->dwStrOffset;
    *(LPMYSTR)(lpDump + lpRS->dwStrLen) = MYTEXT('\0');

    OutputDebugString(TEXT("DumpRS\r\n"));
    wsprintf(szDev, TEXT("dwSize            %x\r\n"), lpRS->dwSize);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwStrLen          %x\r\n"), lpRS->dwStrLen);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwStrOffset       %x\r\n"), lpRS->dwStrOffset);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwCompStrLen      %x\r\n"), lpRS->dwCompStrLen);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwCompStrOffset   %x\r\n"), lpRS->dwCompStrOffset);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwTargetStrLen    %x\r\n"), lpRS->dwTargetStrLen);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwTargetStrOffset %x\r\n"), lpRS->dwTargetStrOffset);
    OutputDebugString(szDev);
    MyOutputDebugString(lpDump);
    OutputDebugString(TEXT("\r\n"));
}
#endif

/**********************************************************************/
/*      ImeSetCompositionString()                                     */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeSetCompositionString(HIMC hIMC, DWORD dwIndex, LPVOID lpComp, DWORD dwComp, LPVOID lpRead, DWORD dwRead)
{
    LPCOMPOSITIONSTRING lpCompStr;
    LPINPUTCONTEXT lpIMC;
    
    ImeLog(LOGF_API, TEXT("ImeSetCompositionString"));

    MyDebugPrint((TEXT("ImeSetCompoitionString\n")));

    switch (dwIndex)
    {
        case SCS_QUERYRECONVERTSTRING:
#ifdef DEBUG
            OutputDebugString(TEXT("SCS_QUERYRECONVERTSTRING\r\n"));
            if (lpComp)
                DumpRS((LPRECONVERTSTRING)lpComp);
            if (lpRead)
                DumpRS((LPRECONVERTSTRING)lpRead);
#endif
            return TRUE;
            break;

        case SCS_SETRECONVERTSTRING:
#ifdef DEBUG
            OutputDebugString(TEXT("SCS_SETRECONVERTSTRING\r\n"));
            if (lpComp)
                DumpRS((LPRECONVERTSTRING)lpComp);
            if (lpRead)
                DumpRS((LPRECONVERTSTRING)lpRead);
#endif
            {
		LPRECONVERTSTRING	pRStr	= (LPRECONVERTSTRING)lpComp ;

		if (!pRStr)
		    break ;

		lpIMC	= ImmLockIMC (hIMC) ;
		if (!lpIMC) {
		    break ;
		}
		if (ImmGetIMCCSize (lpIMC->hCompStr) < sizeof (MYCOMPSTR)){
		    ImmUnlockIMC (hIMC) ;
		    break ;
		}
		//SKKChangeConversionMode (hIMC, IDM_CMODE_TO_ROMANHIRA) ;
		lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC (lpIMC->hCompStr) ;
		if (lpCompStr != NULL) {
		    //SKKSetReconvertStr (hIMC, lpIMC, lpCompStr, pRStr, TRUE) ;
		    ImmUnlockIMCC (lpIMC->hCompStr) ;
		}
		ImmUnlockIMC (hIMC) ;
		return	TRUE ;
            }
            break;

            return TRUE;
            break;
        case SCS_CHANGEATTR:
            MyDebugPrint((TEXT("SCS_CHANGEATTR\n")));
            break;
        case SCS_SETSTR:
            MyDebugPrint((TEXT("*** SCS_SETSTR start\n")));
            lpIMC = ImmLockIMC (hIMC) ;
            if (!lpIMC)
                break ;

            lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC (lpIMC->hCompStr) ;
            if (lpCompStr != NULL) {
                if (lpComp != NULL && dwComp > 0) {
                    //InitCompStr (lpCompStr, CLR_RESULT_AND_UNDET);
                    MyDebugPrint((TEXT(" * LPCOMP %s:%d\n"),lpComp,dwComp / sizeof(TCHAR)));
                    Mylstrcpyn(GETLPCOMPSTR(lpCompStr),lpComp,dwComp);
                } else if (lpRead != NULL && dwRead >0) {
                    MyDebugPrint((TEXT(" * LPREAD %s:%d\n"),lpRead,dwRead));
                    Mylstrcpyn(GETLPCOMPREADSTR(lpCompStr),lpRead,dwRead / sizeof(TCHAR));
                    if (!Mylstrlen(lpRead)) {
                        // 고고타자의 행동을 분석해서 얻은 결론:
                        // lpRead가 지워지면 다시 조합에 들어감.
                        // ImeSetCompostionString()을 쓰는 어플이 많지 않아서
                        // 이 행동이 올바른 행동인지 알 수가 없다 ;p
                        hangul_ic_init(&ic);
                        InitCompStr (lpCompStr, CLR_RESULT_AND_UNDET);
                    }
                }
                ImmUnlockIMCC (lpIMC->hCompStr);
            }
            ImmUnlockIMC (hIMC) ;
            MyDebugPrint((TEXT("*** SCS_SETSTR end\n")));
            return TRUE ;
            break;
        case SCS_CHANGECLAUSE:
            MyDebugPrint((TEXT("SCS_CHANGECLAUSE\n")));
            break;
    }

    return FALSE;
}

/**********************************************************************/
/*      ImeGetImeMenuItemInfo()                                       */
/*                                                                    */
/**********************************************************************/
DWORD WINAPI ImeGetImeMenuItems(HIMC hIMC, DWORD dwFlags, DWORD dwType, LPMYIMEMENUITEMINFO lpImeParentMenu, LPMYIMEMENUITEMINFO lpImeMenu, DWORD dwSize)
{
    ImeLog(LOGF_API, TEXT("ImeGetImeMenuItems"));


    if (!lpImeMenu)
    {
        if (!lpImeParentMenu)
        {
            if (dwFlags & IGIMIF_RIGHTMENU)
                return NUM_ROOT_MENU_R;
            else
                return NUM_ROOT_MENU_L;
        }
        else
        {
            if (dwFlags & IGIMIF_RIGHTMENU)
                return NUM_SUB_MENU_R;
            else
                return NUM_SUB_MENU_L;
        }

        return 0;
    }

    if (!lpImeParentMenu)
    {
        if (dwFlags & IGIMIF_RIGHTMENU)
        {
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_MR_1;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("RootRightMenu1"));
            lpImeMenu->hbmpItem = 0;

            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = IMFT_SUBMENU;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_MR_2;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("RootRightMenu2"));
            lpImeMenu->hbmpItem = 0;

            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_MR_3;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("RootRightMenu3"));
            lpImeMenu->hbmpItem = 0;

            return NUM_ROOT_MENU_R;
        }
        else
        {
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_ML_1;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("RootLeftMenu1"));
            lpImeMenu->hbmpItem = 0;

            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = IMFT_SUBMENU;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_ML_2;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("RootLeftMenu2"));
            lpImeMenu->hbmpItem = 0;

            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_ROOT_ML_3;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("RootLeftMenu3"));
            lpImeMenu->hbmpItem = LoadBitmap(hInst,TEXT("FACEBMP"));

            return NUM_ROOT_MENU_L;
        }
    }
    else
    {
        if (dwFlags & IGIMIF_RIGHTMENU)
        {
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_SUB_MR_1;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("SubRightMenu1"));
            lpImeMenu->hbmpItem = 0;

            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = 0;
            lpImeMenu->wID = IDIM_SUB_MR_2;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("SubRightMenu2"));
            lpImeMenu->hbmpItem = 0;

            return NUM_SUB_MENU_R;
        }
        else
        {
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = IMFS_CHECKED;
            lpImeMenu->wID = IDIM_SUB_ML_1;
            lpImeMenu->hbmpChecked = 0;
            lpImeMenu->hbmpUnchecked = 0;
            Mylstrcpy(lpImeMenu->szString, MYTEXT("SubLeftMenu1"));
            lpImeMenu->hbmpItem = 0;

            lpImeMenu++;
            lpImeMenu->cbSize = sizeof(IMEMENUITEMINFO);
            lpImeMenu->fType = 0;
            lpImeMenu->fState = IMFS_CHECKED;
            lpImeMenu->wID = IDIM_SUB_ML_2;
            lpImeMenu->hbmpChecked = LoadBitmap(hInst,TEXT("CHECKBMP"));
            lpImeMenu->hbmpUnchecked = LoadBitmap(hInst,TEXT("UNCHECKBMP"));
            Mylstrcpy(lpImeMenu->szString, MYTEXT("SubLeftMenu2"));
            lpImeMenu->hbmpItem = 0;

            return NUM_SUB_MENU_L;
        }
    }

    return 0;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
