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
 * $Saenaru: saenaru/src/dic.c,v 1.26 2006/11/24 11:44:04 wkpark Exp $
 */

#include <windows.h>
#include <immdev.h>
#include "saenaru.h"
#include "resource.h"
#include "vksub.h"
#include "immsec.h"

int GetCandidateStringsFromDictionary(LPWSTR lpString, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR szDicFileName);

BOOL GetAnsiPathName(LPCWSTR lpszUniPath,LPSTR lpszAnsiPath,UINT nMaxLen)
{
    if (WideCharToMultiByte(CP_ACP,
                            WC_COMPOSITECHECK,
                            lpszUniPath,
                            -1,
                            lpszAnsiPath,
                            nMaxLen,
                            NULL,
                            NULL) != 0) {
        return TRUE;
    }
    else {
        return FALSE;
    }

}


/**********************************************************************/
/*                                                                    */
/* FlushText()                                                        */
/*                                                                    */
/**********************************************************************/
void PASCAL FlushText(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    TRANSMSG GnMsg;

    if (!IsCompStr(hIMC))
        return;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (IsCandidate(lpIMC))
    {
        //
        // Flush candidate lists.
        //
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
    {
        //
        // Flush composition strings.
        //
        ClearCompStr(lpCompStr,CLR_RESULT_AND_UNDET);
        ImmUnlockIMCC(lpIMC->hCompStr);

        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

        GnMsg.message = WM_IME_ENDCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }
    ImmUnlockIMC(hIMC);
}

/**********************************************************************/
/*                                                                    */
/* RevertText()                                                       */
/*                                                                    */
/**********************************************************************/
void PASCAL RevertText(HIMC hIMC)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    TRANSMSG GnMsg;
    LPMYSTR lpread,lpstr;

    if (!IsCompStr(hIMC))
        return;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (IsCandidate(lpIMC))
    {
        //
        // Flush candidate lists.
        //
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
    {
        lpstr = GETLPCOMPSTR(lpCompStr);
        lpread = GETLPCOMPREADSTR(lpCompStr);
        /*
        lHanToZen(lpstr,lpread,lpIMC->fdwConversion);
        */

        //
        // make attribute
        //
        lpCompStr->dwCursorPos = Mylstrlen(lpstr);
        // DeltaStart is 0 at RevertText time.
        lpCompStr->dwDeltaStart = 0;

        lmemset(GETLPCOMPATTR(lpCompStr),ATTR_INPUT,Mylstrlen(lpstr));
        lmemset(GETLPCOMPREADATTR(lpCompStr),ATTR_INPUT,Mylstrlen(lpread));

        SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
        SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
        lpCompStr->dwCompClauseLen = 8;
        lpCompStr->dwCompReadClauseLen = 8;

        //
        // make length
        //
        lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
        lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);


        //
        // Generate messages.
        //
        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
        //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // �ѱ� IME 2002,2003
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

        ImmUnlockIMCC(lpIMC->hCompStr);
    }
    ImmUnlockIMC(hIMC);
}

/**********************************************************************/
/*                                                                    */
/* ConvHanja()                                                        */
/*                                                                    */
/* VK_HANJA Key handling function                                     */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL ConvHanja(HIMC hIMC, int offset, UINT select)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    MYCHAR szBuf[1024+2];
    int nBufLen;
    LPMYSTR lpstr;
    TRANSMSG GnMsg;
    LPBYTE lpb;
    OFSTRUCT ofs;
    LPMYSTR lpT, lpT2;
    int cnt;
    BOOL bRc = FALSE;
    WCHAR cs=0;
    LPMYSTR lpmystr, lpTemp;
    MYCHAR myBuf[128];
    BOOL isasc=FALSE;

    lpmystr = (LPMYSTR)myBuf;

    if ((GetFileAttributes(szDicFileName) == 0xFFFFFFFF) ||
        (GetFileAttributes(szDicFileName) == FILE_ATTRIBUTE_DIRECTORY)) {
        MakeGuideLine(hIMC,MYGL_NODICTIONARY);
    }

    if (!IsCompStr(hIMC))
        return FALSE;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return FALSE;

    if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)))
        goto cvk_exit10;

    if (!(lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo)))
        goto cvk_exit20;

    //
    // Since IME handles all string as Unicode, convert the CompReadStr
    // from Unicode into multibyte string. Also the dictionary holdsits data
    // as Hiragana, so map the string from Katakana to Hiragana.
    //
    lpT2 = GETLPCOMPREADSTR(lpCompStr);

    //
    // Get the candidate strings from dic file.
    //
    szBuf[1024] = 0;    // Double NULL-terminate
    szBuf[1025] = 0;    // Double NULL-terminate

    Mylstrcpy(szBuf,lpT2); // add Hangul
    szBuf[Mylstrlen(lpT2)] = 0;

    {
        LPTSTR  lpKey;
        TCHAR   szKey[128];
        LPTSTR  lpDic;
        TCHAR   szDic[256];
        LPTSTR  lpIdx;
        TCHAR   szIdx[256];
        INT sz;
        BOOL    fHanja=FALSE;

        if (0x7F > *lpT2) isasc=TRUE;
        MyDebugPrint((TEXT(" * Dic: %x\n"), *lpT2));

        lpKey = (LPTSTR)&szKey;
        lpDic = (LPTSTR)&szDic;
        if (Mylstrlen(lpT2) > 1) { // word dic.
            LoadString( hInst, IDS_WORD_KEY, lpKey, 128);
        } else if (isasc || (*lpT2 >= 0x3131 && 0x314e >= *lpT2) ) { // symbol
            LoadString( hInst, IDS_SYM_KEY, lpKey, 128);
        } else {
            LoadString( hInst, IDS_DIC_KEY, lpKey, 128);
        }

        lpDic += GetSaenaruDirectory(lpDic,256);
        if (*(lpDic-1) != TEXT('\\')) *lpDic++ = TEXT('\\');

        sz= GetRegStringValue(TEXT("\\Dictionary"),lpKey,NULL);
        if (sz <= 2) {
            lpDic = (LPTSTR)&szDic;
            lstrcpy (szDic, szDicFileName) ;
        } else
            GetRegStringValue(TEXT("\\Dictionary"),(LPTSTR)lpKey,lpDic);
        MyDebugPrint((TEXT("Saenaru: %s Dic %s:%d\n"), lpKey, szDic,sz));

        nBufLen = GetCandidateStringsFromDictionary(lpT2,
                szBuf+Mylstrlen(lpT2)+1, 1024, (LPTSTR)szDic);

        while (nBufLen < 1 && Mylstrlen(lpT2)==1) {
            MYCHAR han[2];
            han[0]=0;

            if (*lpT2 < 0x3400 || *lpT2 >=0xfaff) break;
            if (*lpT2 >= 0xa000 && *lpT2 <=0xf8ff) break;
            // check the hanja_to_hangul table

            lpIdx = (LPTSTR)&szIdx;
            LoadString( hInst, IDS_HIDX_KEY, lpKey, 128);
            lpIdx += GetSaenaruDirectory(lpIdx,256);
            if (*(lpIdx-1) != TEXT('\\')) *lpIdx++ = TEXT('\\');

            sz= GetRegStringValue(TEXT("\\Dictionary"),lpKey,NULL);
            if (sz > 2) {
                sz= GetRegStringValue(TEXT("\\Dictionary"),(LPTSTR)lpKey,lpIdx);
                nBufLen = GetHangulFromHanjaIndex(lpT2, han, 2, (LPTSTR)szIdx);
                MyDebugPrint((TEXT("Saenaru: %s Idx %s:%d\n"),lpKey, szIdx,sz));
            }
#if 0
            else 
                han[0] = (MYCHAR)hanja_search(*lpT2);
#endif
            if (han[0]) {
                han[1]= 0;
                szBuf[0]= (MYCHAR)han[0]; // set hangul
                szBuf[1]= 0;
                //szBuf[1]= MYTEXT(' ');
                //szBuf[3]= 0;
                //*lpT2=(MYCHAR)han;
                // get hanja list for this hangul
                nBufLen = GetCandidateStringsFromDictionary(han,
                        szBuf+Mylstrlen(lpT2)+1, 1024, (LPTSTR)szDic);
                if (nBufLen == 0) { // not found in the dic
                    nBufLen=1;
                    szBuf[2]= 0; // double terminate
                }
                fHanja=TRUE;
            }
            break;
        }
        
        if (nBufLen > 1 && !fHanja) {
            // always open a candidate window with hangul chars
            lmemset(GETLPCOMPATTR(lpCompStr),ATTR_TARGET_CONVERTED ,
                  Mylstrlen(GETLPCOMPSTR(lpCompStr)));
            lmemset(GETLPCOMPREADATTR(lpCompStr),ATTR_TARGET_CONVERTED,
                  Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));
        }
    }

    //
    // Check the result of dic. Because my candidate list has only MAXCANDSTRNUM
    // candidate strings.
    //
    lpT = &szBuf[0];
    cnt = 0;
    while(*lpT)
    {
        cnt++;
        lpT += (Mylstrlen(lpT) + 1);

        if (cnt > MAXCANDSTRNUM)
        {
            //
            // The dic is too big....
            //
            goto cvk_exit40;
        }
    }

    lpb = GETLPCOMPATTR(lpCompStr);

    if (nBufLen < 1)
    {
        if (!*lpb)
        {
            //goto cvk_exit40;
            //
            // make attribute
            //
            lmemset(GETLPCOMPATTR(lpCompStr),ATTR_TARGET_CONVERTED ,
                  Mylstrlen(GETLPCOMPSTR(lpCompStr)));
            lmemset(GETLPCOMPREADATTR(lpCompStr),ATTR_TARGET_CONVERTED,
                  Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));

            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // �ѱ� IME 2002,2003
            //GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
            if (lpCompStr->dwCompStrLen > 1 || isasc) {
                GnMsg.wParam = 0;
            } else {
                GnMsg.wParam = (WCHAR) *lpT2;
                if (!isasc && dwImeFlag & SAENARU_ONTHESPOT)
                    GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
            }
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }

        goto cvk_exit40;
    }


    lpstr = (LPMYSTR)szBuf;
    if (!*lpb)
    {
        //
        // String is not converted yet.
        //
        while (*lpstr)
        {
            if (0 != Mylstrcmp(lpstr,GETLPCOMPSTR(lpCompStr)))
            {
set_compstr:
                //
                // Set the composition string to the structure.
                //

                Mylstrcpy(lpmystr,lpstr);
                if (NULL != (lpTemp = Mystrchr(lpmystr, MYTEXT(' '))))
                    *lpTemp = MYTEXT('\0');

                Mylstrcpy(GETLPCOMPSTR(lpCompStr),lpmystr);
                if ((dwImeFlag & SAENARU_ONTHESPOT)&& Mylstrlen(lpmystr) == 1)
                   cs = *lpmystr;

                lpstr = GETLPCOMPSTR(lpCompStr);

                //
                // Set the length and cursor position to the structure.
                //
                lpCompStr->dwCompStrLen = Mylstrlen(lpmystr);
                lpCompStr->dwCursorPos = 0;
                // Because SAENARU does not support clause, DeltaStart is 0 anytime.
                lpCompStr->dwDeltaStart = 0;

                //
                // make attribute
                //
                lmemset((LPBYTE)GETLPCOMPATTR(lpCompStr),
                                ATTR_TARGET_CONVERTED, Mylstrlen(lpmystr));
                lmemset((LPBYTE)GETLPCOMPREADATTR(lpCompStr),
                                ATTR_TARGET_CONVERTED,
                                Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));

                //
                // make clause info
                //
                SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpmystr));
                SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));
                lpCompStr->dwCompClauseLen = 8;
                lpCompStr->dwCompReadClauseLen = 8;

                //
                // Generate messages.
                // XXX
                MyDebugPrint((TEXT(" *** ConvHanja: dwCompStrLen %d\r\n"), lpCompStr->dwCompStrLen));
#if 1
                if (lpCompStr->dwCompStrLen > 1 || isasc) {
                    GnMsg.message = WM_IME_COMPOSITION;
                    GnMsg.wParam = 0;
                    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
                    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // �ѱ� IME 2002,2003
                    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                    MyDebugPrint((TEXT(" *** ConvHanja: GnMsg > 1\r\n")));
                } else
#endif
                {
                    GnMsg.message = WM_IME_COMPOSITION;
                    GnMsg.wParam = cs;
                    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
                    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // �ѱ� IME 2002,2003
                    if (dwImeFlag & SAENARU_ONTHESPOT)
                        GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
                    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                    MyDebugPrint((TEXT(" *** ConvHanja: GnMsg == 1\r\n")));
                }
#if DEBUG
                MyOutputDebugString(lpmystr);
                MyDebugPrint((TEXT("\r\n *** ConvHanja: Pre candidate\r\n")));
#endif

                bRc = TRUE;
                goto cvk_exit40;
            }
            lpstr += (Mylstrlen(lpstr) + 1);

        }
    }
    else
    {
        //
        // String is converted, so that open candidate.
        //
        int i = 0;
        int pages = 0;
        LPDWORD lpdw;

        //
        // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
        //
        if (!IsCandidate(lpIMC))
        {
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_OPENCANDIDATE;
            GnMsg.lParam = 1L;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
            MyDebugPrint((TEXT("ConvHanja: OpenCandidate\r\n")));
        }

        //
        // Make candidate structures.
        //
        lpCandInfo->dwSize = sizeof(MYCAND);
        lpCandInfo->dwCount = 1;
        lpCandInfo->dwOffset[0] =
              (DWORD)((LPSTR)&((LPMYCAND)lpCandInfo)->cl - (LPSTR)lpCandInfo);
        lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
        lpdw = (LPDWORD)&(lpCandList->dwOffset);
        while (*lpstr)
        {
            lpCandList->dwOffset[i] =
                   (DWORD)((LPSTR)((LPMYCAND)lpCandInfo)->szCand[i] - (LPSTR)lpCandList);
            Mylstrcpy(lpmystr,lpstr);
            if (NULL != (lpTemp = Mystrchr(lpmystr, MYTEXT(','))))
              *lpTemp = MYTEXT('\0');
            Mylstrcpy((LPMYSTR)((LPSTR)lpCandList+lpCandList->dwOffset[i]),lpmystr);
            lpstr += (Mylstrlen(lpstr) + 1);
            i++;
        }

        lpCandList->dwSize = sizeof(CANDIDATELIST) +
                          (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
        lpCandList->dwStyle = IME_CAND_READ;
        lpCandList->dwCount = i;
        if (i < MAXCANDPAGESIZE)
            lpCandList->dwPageSize  = i;
        else
            lpCandList->dwPageSize  = MAXCANDPAGESIZE;
        // Total pages
        pages = i / lpCandList->dwPageSize + i % lpCandList->dwPageSize;

        if (select)
        {
            int sel = lpCandList->dwSelection / lpCandList->dwPageSize;
            sel *= lpCandList->dwPageSize;
            if (select != -1) {
                sel +=select - 1;
                if (sel >= i)
                    sel = i - 1;
            } else {
                sel = 0;
            }
            lpCandList->dwSelection = sel;
            offset = 0;
        }
        else if (offset > 2)
        {
            // last (END)
            lpCandList->dwSelection = i - 1;
            offset = 0; // reset
        } else if (offset < -2)
        {
            // first (HOME)
            lpCandList->dwSelection = 0;
            offset = 0; // reset
        }

        // PgUp/PgDn
        if (offset ==2) offset = lpCandList->dwPageSize;
        else if (offset ==-2) offset = -(int)lpCandList->dwPageSize;

        if (offset < 0 && (int)(lpCandList->dwSelection + offset) < 0)
        {
            int select = lpCandList->dwSelection + offset;
            //if (select < 0) select= pages * lpCandList->dwPageSize - offset;
            if (select < 0) select= 0;
            if (select >= i)
                select = i - 1;    
            lpCandList->dwSelection = (DWORD)select;
        } else {
            lpCandList->dwSelection += offset;
            if (offset > 1 && lpCandList->dwSelection >= (DWORD)i)
                lpCandList->dwSelection = i - 1;
        }

        // �� �� + 1�̸� �� ó������
        if (lpCandList->dwSelection >= (DWORD)i)
        {
            lpCandList->dwPageStart = 0;
            lpCandList->dwSelection = 0;
        } else {
            int spage = lpCandList->dwSelection / lpCandList->dwPageSize;
            lpCandList->dwPageStart= spage * lpCandList->dwPageSize;
        }

        //
        // Generate messages.
        //
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CHANGECANDIDATE;
        GnMsg.lParam = 1L;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        MyDebugPrint((TEXT("ConvHanja: ChangeCandidate\r\n")));

        //
        // If the selected candidate string is changed, the composition string
        // should be updated.
        //
        if (select != -1)
        lpstr = (LPMYSTR)((LPSTR)lpCandList +
                   lpCandList->dwOffset[lpCandList->dwSelection]);
        else
            lpstr = lpT2;
        goto set_compstr;

    }

cvk_exit40:
    ImmUnlockIMCC(lpIMC->hCandInfo);

cvk_exit20:
    ImmUnlockIMCC(lpIMC->hCompStr);

cvk_exit10:
    ImmUnlockIMC(hIMC);
    return bRc;
}

/**********************************************************************/
/*                                                                    */
/* IsEat( code )                                                      */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL IsEat( code )
register WORD code;
{
    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/* DeleteChar()                                                       */
/*                                                                    */
/**********************************************************************/
void PASCAL DeleteChar( HIMC hIMC ,UINT uVKey)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    LPMYSTR lpstr;
    LPMYSTR lpread;
    LPMYSTR lpptr;
    int nChar;
    BOOL fDone = FALSE;
    DWORD dwCurPos;
    TRANSMSG GnMsg;

    WCHAR cs = 0;

    if (!IsCompStr(hIMC))
        return;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    dwCurPos = lpCompStr->dwCursorPos;
    lpstr = GETLPCOMPSTR(lpCompStr);

    if( uVKey ==  VK_BACK )
    {
        if( dwCurPos == 0 )
            goto dc_exit;

        lpptr = MyCharPrev( lpstr, lpstr+dwCurPos );

        if ( ic.len && (dwOptionFlag & BACKSPACE_BY_JAMO)) {
            // Delete jamos
            if (--ic.len > 0) {
                ic.laststate--;    
                *lpptr = cs = hangul_ic_get(&ic,0);
            } else {
                hangul_ic_init(&ic);
                Mylstrcpy( lpptr, lpstr+dwCurPos );
                if (dwCurPos > 0)
                    cs = *(lpptr-1);
                dwCurPos--;
            }
        } else {
            nChar = 1;
            if( lpstr == lpptr && Mylstrlen(lpstr) == nChar )
            {
                dwCurPos = 0;
                *lpstr = MYTEXT('\0');
            }
            else
            {
                Mylstrcpy( lpptr, lpstr+dwCurPos );
                dwCurPos -= nChar;
                cs = *(lpptr-1);
            }
        }

        fDone = TRUE;
    }
    else if( uVKey == VK_DELETE )
    {
        if( dwCurPos == (DWORD)Mylstrlen(lpstr) )
            goto dc_exit;

        nChar = 1;
        Mylstrcpy( lpstr+dwCurPos, lpstr+dwCurPos+nChar );

        fDone = TRUE;

    }

    if (fDone)
    {
        lpstr = GETLPCOMPSTR(lpCompStr);
        lpread = GETLPCOMPREADSTR(lpCompStr);
        /*lZenToHan (lpread,lpstr);
         */
        Mylstrcpy(lpread,lpstr); // !!!

        lmemset(GETLPCOMPATTR(lpCompStr),ATTR_INPUT,Mylstrlen(lpstr));
        lmemset(GETLPCOMPREADATTR(lpCompStr),ATTR_INPUT,Mylstrlen(lpread));

        //
        // make length
        //
        lpCompStr->dwCompStrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadStrLen = Mylstrlen(lpread);
        lpCompStr->dwCompAttrLen = Mylstrlen(lpstr);
        lpCompStr->dwCompReadAttrLen = Mylstrlen(lpread);

        lpCompStr->dwCursorPos = dwCurPos;
        // DeltaStart is same of Cursor Pos at DeleteChar time.
        lpCompStr->dwDeltaStart = dwCurPos;

        //
        // make clause info
        //
        SetClause(GETLPCOMPCLAUSE(lpCompStr),Mylstrlen(lpstr));
        SetClause(GETLPCOMPREADCLAUSE(lpCompStr),Mylstrlen(lpread));
        lpCompStr->dwCompClauseLen = 8;
        lpCompStr->dwCompReadClauseLen = 8;

        if (lpCompStr->dwCompStrLen)
        {
            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.wParam = cs;
            //GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // �ѱ� IME 2002,2003
            if (dwImeFlag & SAENARU_ONTHESPOT)
                GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }
        else
        {
            if (IsCandidate(lpIMC))
            {
                lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                ClearCandidate(lpCandInfo);
                GnMsg.message = WM_IME_NOTIFY;
                GnMsg.wParam = IMN_CLOSECANDIDATE;
                GnMsg.lParam = 1;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                ImmUnlockIMCC(lpIMC->hCandInfo);
            }

            ClearCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.wParam = 0;
            //GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // �ѱ� IME 2002,2003
            if (dwImeFlag & SAENARU_ONTHESPOT)
                GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

            GnMsg.message = WM_IME_ENDCOMPOSITION;
            GnMsg.wParam = 0;
            GnMsg.lParam = 0;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
//            PostMessage(lpIMC->hWnd,WM_IME_ENDCOMPOSITION,0,0);
        }
    }

dc_exit:
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}

#if 0
/**********************************************************************/
/*                                                                    */
/* AddChar()                                                          */
/*                                                                    */
/* One character add function                                         */
/*                                                                    */
/**********************************************************************/
void PASCAL AddChar( hIMC, code )
HIMC hIMC;
WORD code;
{
    LPMYSTR lpchText;
    LPMYSTR lpread;
    LPMYSTR lpstr;
    LPMYSTR lpprev;
    WORD code2 = 0;
    WORD code3;
    DWORD fdwConversion;
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD dwStrLen;
    DWORD dwSize;
    TRANSMSG GnMsg;
    DWORD dwGCR = 0L;
    WCHAR Katakana, Sound;

    lpIMC = ImmLockIMC(hIMC);

    if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof (MYCOMPSTR))
    {
        // Init time.
        dwSize = sizeof(MYCOMPSTR);
        lpIMC->hCompStr = ImmReSizeIMCC(lpIMC->hCompStr,dwSize);
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
        lpCompStr->dwSize = dwSize;
    }
    else
    {
        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
    }

    dwStrLen = lpCompStr->dwCompStrLen;

    if (!dwStrLen)
    {
        // ������ �ʱ�ȭ
        InitCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

        GnMsg.message = WM_IME_STARTCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

    }
    else if (IsConvertedCompStr(hIMC))
    {
        MakeResultString(hIMC,FALSE);
        InitCompStr(lpCompStr,CLR_UNDET);
        dwGCR = GCS_RESULTALL;
    }

    if( IsEat( code ) )
    {
        // Get ConvMode from IMC.
        fdwConversion = lpIMC->fdwConversion;

        lpchText = GETLPCOMPSTR(lpCompStr);
        lpstr = lpchText;
        if( lpCompStr->dwCursorPos )
            lpstr += lpCompStr->dwCursorPos;
        lpstr = lpchText + Mylstrlen(lpchText);
        lpprev = MyCharPrev( lpchText, lpstr );

        if( fdwConversion & IME_CMODE_CHARCODE ) {
            code = (WORD)(LONG_PTR)AnsiUpper( (LPSTR)(LONG_PTR)code );
            if( !( (code >= MYTEXT('0') && code <= MYTEXT('9')) ||
                (code >= MYTEXT('A') && code <= MYTEXT('F')) ) || lpCompStr->dwCursorPos >= 4 ){
                MessageBeep( 0 );
                goto ac_exit;
            }
            *lpstr++ = (BYTE)code;
            lpCompStr->dwCursorPos++;
        }
        else if ( fdwConversion & IME_CMODE_FULLSHAPE )
        {
            if ( fdwConversion & IME_CMODE_ROMAN  &&
                 fdwConversion & IME_CMODE_NATIVE )
            {
                if (*lpprev) {
                     code2 = *lpprev;
                }
                else
                {
                    if( IsSecond( code ) )
                    {
                        code = ConvChar(hIMC, 0, code );
                        if (!(fdwConversion & IME_CMODE_HANGUL))
                        {
                            code = KataToHira(code);
                        }
                    }
                    goto DBCS_BETA;
                }

                if (!( code2 = ZenToHan( code2 ) ))
                {
                    if( IsSecond( code ) )
                    {
                        code = ConvChar(hIMC, 0, code );
                        if (!(fdwConversion & IME_CMODE_HANGUL))
                        {
                            code = KataToHira(code);
                        }
                    }
                    goto DBCS_BETA;
                }

                if ( IsSecond( code ) )
                {
                    if ( IsFirst( code2 ) &&
                       (code3 = ConvChar(hIMC, code2, code )))
                    {
                        if (fdwConversion & IME_CMODE_HANGUL) 
                        {
                            *lpprev = code3;
                        }
                        else
                        {
                           *lpprev = KataToHira(code3);
                        }
                    }
                    else
                    {
                        code = ConvChar(hIMC, 0, code );

                        if (!(fdwConversion & IME_CMODE_HANGUL))
                        {
                            code = KataToHira(code);
                        }
                        goto DBCS_BETA;
                    }
                }
                else if( (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)code ) == 'N'
                      && (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)code2 ) == 'N' )
                {
                    code3 = 0xFF9D;
                    code2 = HanToZen( code3, 0,fdwConversion);
                    *lpprev = code2;
                }
                else
                    goto DBCS_BETA;
            }
            else
            {
DBCS_BETA:
                if( code == MYTEXT('^') )
                {
                    code2 = *lpprev;
                    if( IsTenten( code2 ) == FALSE )
                        goto DBCS_BETA2;
                    code2 = ConvTenten( code2 );
                    *lpprev++ = code2;
                }
                else if( code == MYTEXT('_') )
                {
                    code2 = *lpprev;
                    if( IsMaru( code2 ) == FALSE )
                        goto DBCS_BETA2;
                    code2 = ConvMaru( code2 );
                    *lpprev = code2;
                }
                else
                {
                    code = HanToZen(code,0,fdwConversion);
DBCS_BETA2:
                    *lpstr++ = code;
                    lpCompStr->dwCursorPos += 1;
                }
            }
        }
        else
        {
            if (fdwConversion & IME_CMODE_ROMAN  &&
                 fdwConversion & IME_CMODE_NATIVE )
            {

                if (IsSecond( code ))
                {
                    if (IsFirst( *lpprev ) &&
                       (code2 = ConvChar(hIMC,*lpprev,code)))
                    {
                        if (OneCharZenToHan(code2,&Katakana, &Sound))
                        {
                            *lpprev = Katakana;
                            if (Sound) {
                                *lpstr++ = Sound;
                                lpCompStr->dwCursorPos++;
                            }
                        } 
                        else 
                        {
                            code = ConvChar(hIMC, 0, code );
                            goto SBCS_BETA;
                        }
                    }
                    else
                    {
                        code = ConvChar(hIMC, 0, code );
                        //MakeGuideLine(hIMC,MYGL_TYPINGERROR);
                        goto SBCS_BETA;
                    }
                }
                else
                {
                    if( (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)code ) == 'N'
                      && (WORD)(LONG_PTR)CharUpperW( (LPMYSTR)(LONG_PTR)(code2 = *lpprev ) ) == 'N' )
                    {
                        *lpprev = (MYCHAR) 0xFF9D;
                    }
                    else
                    {
                         //MakeGuideLine(hIMC,MYGL_TYPINGERROR);
                         goto SBCS_BETA;
                    }
                }
            }
            else
            {
SBCS_BETA:
                if (OneCharZenToHan(code,&Katakana,&Sound)) 
                {
                    *lpstr++ = Katakana;
                    if (Sound) 
                    {
                        *lpstr++ = Sound;
                        lpCompStr->dwCursorPos++;
                    }
                }
                else
                {
                    *lpstr++ = code;
                }
                lpCompStr->dwCursorPos++;
            }
        }
        *lpstr = MYTEXT('\0');
    }

    // make reading string.
    lpstr = GETLPCOMPSTR(lpCompStr);
    lpread = GETLPCOMPREADSTR(lpCompStr);
    if (fdwConversion & IME_CMODE_HANGUL) 
    {
            Mylstrcpy(lpread,lpstr);
/*        if (fdwConversion & IME_CMODE_FULLSHAPE)
        {
            Mylstrcpy(lpread,lpstr);
        }
        else 
        {
            lHanToZen(lpread,lpstr,fdwConversion);
        }
        */
    }
    else
    {
        LPMYSTR pSrc = lpstr;
        LPMYSTR pDst = lpread;

        for (; *pSrc;) {
            //*pDst++ = HiraToKata(*pSrc);
            *pDst++ = *pSrc;
            pSrc++;
        }
        *pDst = (MYCHAR) 0;
    }


    // make attribute
    lpCompStr->dwCursorPos = Mylstrlen(lpstr);
    lpCompStr->dwDeltaStart = (DWORD)(MyCharPrev(lpstr, lpstr+Mylstrlen(lpstr)) - lpstr);

    //MakeAttrClause(lpCompStr);
    lmemset((LPBYTE)GETLPCOMPATTR(lpCompStr),ATTR_INPUT, Mylstrlen(lpstr));
    lmemset((LPBYTE)GETLPCOMPREADATTR(lpCompStr),ATTR_INPUT, Mylstrlen(lpread));

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

    GnMsg.message = WM_IME_COMPOSITION;
    GnMsg.wParam = 0;
    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART | dwGCR;
    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR | dwGCR; // �ѱ� IME 2002,2003
    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

ac_exit:
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}

#endif


/**********************************************************************/
/*                                                                    */
/* DicKeydownHandler()                                                */
/*                                                                    */
/* WM_KEYDOWN handler for dictionary routine                          */
/*                                                                    */
/* wParam                                                             */
/* virtual key                                                        */
/*                                                                    */
/* lParam                                                             */
/* differ depending on wParam                                         */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL DicKeydownHandler( hIMC, wParam, lParam ,lpbKeyState)
HIMC hIMC;
UINT wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD fdwConversion;

    BOOL cf=FALSE;
    int next=0;
    UINT select=0;
    int candOk=FALSE;

    // Candidate���� ����
    if (IsConvertedCompStr(hIMC))
    {
        switch( wParam )
        {
            case VK_UP:
                next=-1;
                break;
            case VK_DOWN:
                next=1;
                break;
            case VK_NEXT:
                next=2;
                break;
            case VK_PRIOR:
                next=-2;
                break;
            case VK_HOME:
                next=-100;
                break;
            case VK_END:
                next=100;
                break;
            case VK_1:
            case VK_2:
            case VK_3:
            case VK_4:
            case VK_5:
            case VK_6:
            case VK_7:
            case VK_8:
            case VK_9:
                select= wParam - VK_1 + 1;
                break;
            case VK_ESCAPE:
                select= -1; // XXX
                break;
            default:
                break;
        }
        if (next || select)
        {
            ConvHanja(hIMC,next,select);
            if (next) return TRUE;
            wParam=VK_RETURN;
        }
    }

    switch( wParam )
    {
        case VK_DELETE:
        case VK_BACK:
            if (IsConvertedCompStr(hIMC))
            {
                FlushText(hIMC);
                return TRUE;
            } else if (IsCompStr(hIMC))
            {
                DeleteChar(hIMC,wParam);
                return TRUE;
            }
            break;

        case VK_HANJA:
        case VK_F9:
            hangul_ic_init(&ic);

            lpIMC = ImmLockIMC(hIMC);

#define USE_RECONVERSION
#ifdef USE_RECONVERSION
            // regard the VK_F9 key as the ReConversion key
            while (!IsCompStr(hIMC)) {
                TCHAR szFname[256];
                int ns=0;
                DWORD dwSize = (DWORD)MyImmRequestMessage(hIMC, IMR_RECONVERTSTRING, 0);

                ns=GetModuleFileName(NULL,(LPTSTR)szFname,sizeof(szFname));
                if (ns > 0) {
                    // check application names
                    int i;
                    LPTSTR lpFname;
#if DEBUG
                    OutputDebugString(szFname); // application path.
#endif

                    for (i=ns;i>0;i--) {
                        if (szFname[i] == TEXT('\\')) break;
                    }
                    if (i>0) {
                        lpFname=szFname + i + 1;
#if DEBUG
                        OutputDebugString(lpFname); // application real name
#endif
                    }
                }

                if (dwSize) {
                    LPRECONVERTSTRING lpRS;
        
                    lpRS = (LPRECONVERTSTRING)GlobalAlloc(GPTR, dwSize);
                    lpRS->dwSize = dwSize;
        
                    if (dwSize = (DWORD) MyImmRequestMessage(hIMC, IMR_RECONVERTSTRING, (LPARAM)lpRS)) {
                        BOOL convOk= FALSE;
                        TRANSMSG GnMsg;
                        MYCHAR myDump[2];
#ifdef DEBUG
                        TCHAR szDev[80];
#endif
                        //LPMYSTR lpDump= (LPMYSTR)(((LPSTR)lpRS) + lpRS->dwStrOffset);
                        //*(LPMYSTR)(lpDump + lpRS->dwStrLen) = MYTEXT('\0');
                        LPMYSTR lpDump= (LPMYSTR)(((LPSTR)lpRS) + lpRS->dwStrOffset + lpRS->dwCompStrOffset);
        
#ifdef DEBUG
                        OutputDebugString(TEXT("IMR_RECONVERTSTRING\r\n"));
                        wsprintf(szDev, TEXT("dwSize       %x\r\n"), lpRS->dwSize);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwVersion    %x\r\n"), lpRS->dwVersion);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwStrLen     %x\r\n"), lpRS->dwStrLen);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwStrOffset  %x\r\n"), lpRS->dwStrOffset);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwCompStrLen %x\r\n"), lpRS->dwCompStrLen);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwCompStrOffset %x\r\n"), lpRS->dwCompStrOffset);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwTargetStrLen %x\r\n"), lpRS->dwTargetStrLen);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwTargetStrOffset %x\r\n"), lpRS->dwTargetStrOffset);
                        OutputDebugString(szDev);
#endif
                        Mylstrcpyn(myDump,lpDump,1);
                        if ( (lpRS->dwStrOffset+lpRS->dwCompStrOffset) < lpRS->dwSize) {
                            if (lpRS->dwCompStrLen >1) {
                                // clause dictionary.
                                *(LPMYSTR)(lpDump + lpRS->dwCompStrLen) = MYTEXT('\0');
                            } else  {
                                const LPMYSTR szSep = MYTEXT("\r\n\t");
                                LPMYSTR lpToken;
                                // one char reconversion.
                                *(LPMYSTR)(lpDump + 1) = MYTEXT('\0');

                                lpToken = Mystrtok(lpDump, szSep);
                                if (lpToken == NULL) {
                                    GlobalFree((HANDLE)lpRS);
                                    break;
                                }
                        
                                lpRS->dwCompStrLen=1;
                                lpRS->dwTargetStrLen=1;
                                // XXX 1. manage not convertable ascii chars.
                            }
                        } else {
                            // XXX Mozilla bug.
                            OutputDebugString(TEXT(" Reconversion error\r\n"));
                            GlobalFree((HANDLE)lpRS);
                            //ImmUnlockIMC(hIMC);
                            break;
                        }
#ifdef DEBUG
                        MyOutputDebugString(lpDump);
                        OutputDebugString(TEXT("\r\n"));
#endif
        		{
        		    LPCOMPOSITIONSTRING	lpCompStr;
        
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
                                    // XXX
                                    InitCompStr(lpCompStr,CLR_RESULT_AND_UNDET);

#if 1
                                    // �����е�� M$ Explorer�� �ݵ�� WM_IME_STARTCOMPOSITION���� �����ؾ� �Ѵ�.
                                    // 2006/10/09
                                    GnMsg.message = WM_IME_STARTCOMPOSITION;
                                    GnMsg.wParam = 0;
                                    GnMsg.lParam = 0;
                                    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
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

#if 1
                                    Mylstrcpy(GETLPRESULTSTR(lpCompStr),GETLPCOMPSTR(lpCompStr));
                                    Mylstrcpy(GETLPRESULTREADSTR(lpCompStr),GETLPCOMPREADSTR(lpCompStr));

                                    lpCompStr->dwResultStrLen = lpCompStr->dwCompStrLen;
                                    lpCompStr->dwResultReadStrLen = lpCompStr->dwCompReadStrLen;
                                    //
                                    // make clause info
                                    //
                                    SetClause(GETLPRESULTCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTSTR(lpCompStr)));
                                    SetClause(GETLPRESULTREADCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTREADSTR(lpCompStr)));
                                    lpCompStr->dwResultClauseLen = 8;
#endif
                                    //
                                    //
                                    //if (lpCompStr->dwCompReadStrLen > 0)
                                    //    lpCompStr->dwCompReadStrLen--;

#if 0
                                    GnMsg.message = WM_IME_COMPOSITION;
                                    GnMsg.wParam = 0;
                                    //GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
                                    GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; //�ѱ� IME 2002,2003
                                    //if (dwImeFlag & SAENARU_ONTHESPOT)
                                    //    GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
                                    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
#endif
        		        } else {
                                    OutputDebugString(TEXT(" *** lpCompStr== NULL\r\n"));
                                }
                                convOk = (BOOL) MyImmRequestMessage(hIMC, IMR_CONFIRMRECONVERTSTRING, (LPARAM)lpRS);
#ifdef DEBUG
                                if (!convOk) {
                                    // MS IE�� ������� �� �κ��� �ڿ� �;� �Ѵ�. ��, CompStr�� ������ �Ŀ�
                                    // �����ؾ� �Ѵ� ?
                                    OutputDebugString(TEXT(" *** fail CONFIRM RECONVERT\r\n"));
                                } else {
                                    OutputDebugString(TEXT(" *** success CONFIRM RECONVERT\r\n"));
                                    wsprintf(szDev, TEXT(" *** result: dwCompStrLen %x\r\n"), lpRS->dwCompStrLen);
                                    OutputDebugString(szDev);
                                }
#endif
                            }
        	            ImmUnlockIMCC (lpIMC->hCompStr);
        		}
                    }
#ifdef DEBUG
                    else
                        OutputDebugString(TEXT("ImmRequestMessage returned 0\r\n"));
#endif
                    GlobalFree((HANDLE)lpRS);
                    //ImmUnlockIMC(hIMC);
                }
                break;
            }
#endif
            // Check Candidate ability of the IME application
            if (IsCompStr(hIMC)) {
                CANDIDATEFORM lc;

                // check the candidate list of the application supported
                // some applications like as M$ Word support its own candidate
                // suggest systems.
                candOk = MyImmRequestMessage(hIMC, IMR_CANDIDATEWINDOW, (LPARAM)&lc);
#ifdef DEBUG
                if (candOk) {
                    TCHAR szDev[80];
                    wsprintf(szDev, TEXT("IMR_CandidateWindow: success\r\n"));
                    OutputDebugString(szDev);
                } else {
                    TCHAR szDev[80];
                    wsprintf(szDev, TEXT("IMR_CandidateWindow: fail\r\n"));
                    OutputDebugString(szDev);
                }
#endif
            }

            // XXX hack hack !! IME 2003 TSF mode compatibility
            // M$ Word�� �����簡 ���� �ʾƼ� Spy++�� �޽��� �����غ�
            // ���, ����Ű�� �������� Comp��忡���� VK_PROCESSKEY��
            // ��������, Reconversion��忡���� VK_PROCESSKEY�� �Բ�
            // WM_IME_KEYDOWN�̺�Ʈ�� ����. �׷��� �� �̺�Ʈ�� ���������
            // M$ Word������ ����� �۵��Ͽ� ���� ������ ���.
            // * InternetExplorer������ Reconversion�� ��. IME 2003������ �ȵ�
            //

            if (!IsCompStr(hIMC)) {
                // for reconversion mode like as IME 2002
                TRANSMSG GnMsg;
                GnMsg.message = WM_IME_KEYDOWN;
                GnMsg.wParam = wParam;
                GnMsg.lParam = lParam;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                OutputDebugString(TEXT("DicKeydown: WM_IME_KEYDOWN\r\n"));
                MyDebugPrint((TEXT("DicKeydown: candOk %d\r\n"),candOk));
            } else if (candOk) {
                // � ������ ��ü ����� candidate����Ʈ�� ����, ����� �׷��� �ʴ�.
                // �׷��� �̷��� ������ �ϰ����� ��� � ������ Ư��ó�� �ؾ� �Ѵ�.
                // always generage WM_IME_KEYDOWN for VK_HANJA XXX
                TRANSMSG GnMsg;
                GnMsg.message = WM_IME_KEYDOWN;
                GnMsg.wParam = wParam;
                GnMsg.lParam = lParam;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                OutputDebugString(TEXT("DicKeydown: WM_IME_KEYDOWN #2\r\n"));
            }
            if (!candOk) {
                ConvHanja(hIMC,1,0);
            }
            ImmUnlockIMC(hIMC);
            return TRUE;
            break;

        case VK_SPACE:
            lpIMC = ImmLockIMC(hIMC);
            // Get ConvMode from IMC.
            fdwConversion = lpIMC->fdwConversion;
            ImmUnlockIMC(hIMC);

            if (IsCompStr(hIMC) &&
                    (fdwConversion & IME_CMODE_HANJACONVERT) &&
                    (fdwConversion & IME_CMODE_NATIVE)) {
                hangul_ic_init(&ic);
                ConvHanja(hIMC,1,0);
                return TRUE;
            }

            if (IsCompStr(hIMC))
            {
                MakeResultString(hIMC,TRUE);
#if 0
                lpIMC = ImmLockIMC(hIMC);
                lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
                InitCompStr(lpCompStr,CLR_UNDET);
                ImmUnlockIMC(hIMC);
#endif

                // Simulate a key press
                keybd_event( (BYTE)wParam, 0x0, 0, 0 );

                // Simulate a key release
                //keybd_event( (BYTE)wParam, 0x0, KEYEVENTF_KEYUP, 0);
                return TRUE;
            }
#if 0
            // �����̽��� ���� ������ �ѱ�� Ű�� ����Ѵ�.
            // �ɼ����� �Ѱ� ų �� �ְ�.
            if (IsConvertedCompStr(hIMC))
            {
                ConvHanja(hIMC,1);
                return TRUE;
            }
#endif
            break;

        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_HOME:
        case VK_END:
        case VK_NEXT:
        case VK_PRIOR:

        case VK_TAB:
        case VK_ESCAPE:
            if (IsCompStr(hIMC))
            {
                MakeResultString(hIMC,TRUE);
                // Simulate a key press
                keybd_event( (BYTE)wParam, 0x0, 0, 0 );

                // Simulate a key release
                //keybd_event( (BYTE)wParam, 0x0, KEYEVENTF_KEYUP, 0);
                return TRUE;
            }
            break;

        case VK_RETURN:
            lpIMC = ImmLockIMC(hIMC);
            // Get ConvMode from IMC.
            fdwConversion = lpIMC->fdwConversion;
            ImmUnlockIMC(hIMC);
            if (IsCompStr(hIMC) &&
                    (fdwConversion & IME_CMODE_FULLSHAPE) &&
                    (fdwConversion & IME_CMODE_NATIVE)) {
                MakeResultString(hIMC,TRUE);
                hangul_ic_init(&ic);
                return TRUE;
            }
            if (IsConvertedCompStr(hIMC))
                cf = TRUE;
            MakeResultString(hIMC,TRUE);

            lpIMC = ImmLockIMC(hIMC);

            if (!cf)
            {
#if 0
                if (IsWindow(lpIMC->hWnd))
                {
                   //PostMessage(lpIMC->hWnd,WM_IME_KEYDOWN,wParam,lParam);
                }
#endif
                TRANSMSG GnMsg;
                GnMsg.message = WM_IME_KEYDOWN;
                GnMsg.wParam = wParam;
                GnMsg.lParam = lParam;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                // Simulate a key press
                // keybd_event( VK_RETURN, 0x0d, 0, 0 );

                // Simulate a key release
                // keybd_event( VK_RETURN, 0x0d, KEYEVENTF_KEYUP, 0);
            }
            ImmUnlockIMC(hIMC);
            // spy++�� �ѱ� IME2002�� �ൿ�� ���캻 ���,
            // CompStr�� ���� ���� RETURN�� ġ�� ó�� �̺�Ʈ�� ������ �� �ѹ�
            // �׸���, CompStr�� commit�� �Ŀ� �ٽ� �ѹ� Ű�̺�Ʈ�� �����.
            // strings�� imekr61.ime�� ���캻 ��� keybd_event()�� ���� �־�����
            // saenaru�� �� ����� ������.
            //
            // ���Ÿ�ڸ� ���� �м��غ���, WM_IME_KEYDOWN�� ���� �����.
            // �׷��� ��ó�� GenerateMessage()�� �̿��غ��� keybd_event�� ����
            // �ʾƵ� �Ǿ���.
            //
            hangul_ic_init(&ic);
            return TRUE;
#if 0
            lpIMC = ImmLockIMC(hIMC);
            if (IsCandidate(lpIMC))
            {
                MakeResultString(hIMC,TRUE);
                cf = TRUE;
            }
            ImmUnlockIMC(hIMC);
            if (cf) return TRUE;
#endif
            break;

            /*
        case VK_HANGUL:
            // Toggle HANGUL
            ChangeMode(hIMC,TO_CMODE_HANGUL);
            return TRUE;
            break;
            */

#ifdef DEBUG
        case VK_G:
            if (dwDebugFlag & DEBF_GUIDELINE)
            {
                if (IsCTLPushed(lpbKeyState))
                {
                    MakeGuideLine(hIMC,MYGL_TESTGUIDELINE);
                    //MakeGuideLine(hIMC,MYGL_TYPINGERROR);
                    return( TRUE );
                }
            }
            break;
#endif
        case VK_LBUTTON:
        case VK_RBUTTON:
        case VK_MBUTTON:
            // for mozilla
            MyDebugPrint((TEXT(" *** VK_LBUTTON\r\n")));
            if (IsCompStr(hIMC)) {
                MakeResultString(hIMC,TRUE);
                hangul_ic_init(&ic);
                return TRUE;
            }
            break;
        default:
            break;
    }

    if (( VK_0 <= wParam && VK_9 >= wParam ) ||
        ( VK_A <= wParam && VK_Z >= wParam ) ||
        ( VK_RETURN == wParam || VK_SPACE == wParam ) ||
        //( VK_NUMPAD0 <= wParam && VK_NUMPAD9 >= wParam ) ||
        ( VK_OEM_1 <= wParam && VK_OEM_9 >= wParam ) ||
        ( VK_MULTIPLY <= wParam && VK_DIVIDE >= wParam ))
    {
        // RETURN�� ������� �ʰ�, �ٷ� ���κп��� ó���� ������.
        // ������ �� �� �������� �ʴ´ٸ� RETURNŰ�� ó���� �� �ִ�.
        return( FALSE );
    }
    else {
        MyDebugPrint((TEXT("code: %x\r\n"), wParam));
        return( TRUE );
    }
}

/**********************************************************************/
/*                                                                    */
/*  Entry    : MakeResultString( HIMC, BOOL)                          */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI MakeResultString( HIMC hIMC, BOOL fFlag)
{
    TRANSMSG GnMsg;
    LPCOMPOSITIONSTRING lpCompStr;
    LPCANDIDATEINFO lpCandInfo;
    LPINPUTCONTEXT lpIMC;

    if (!IsCompStr(hIMC))
        return FALSE;

    lpIMC = ImmLockIMC(hIMC);

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    if (IsCandidate(lpIMC))
    {
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1L;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    Mylstrcpy(GETLPRESULTSTR(lpCompStr),GETLPCOMPSTR(lpCompStr));
    Mylstrcpy(GETLPRESULTREADSTR(lpCompStr),GETLPCOMPREADSTR(lpCompStr));


    lpCompStr->dwResultStrLen = lpCompStr->dwCompStrLen;
    lpCompStr->dwResultReadStrLen = lpCompStr->dwCompReadStrLen;

    lpCompStr->dwCompStrLen = 0;
    lpCompStr->dwCompReadStrLen = 0;

    //
    // make clause info
    //
    SetClause(GETLPRESULTCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTSTR(lpCompStr)));
    SetClause(GETLPRESULTREADCLAUSE(lpCompStr),Mylstrlen(GETLPRESULTREADSTR(lpCompStr)));
    lpCompStr->dwResultClauseLen = 8;
    lpCompStr->dwResultReadClauseLen = 8;

    ImmUnlockIMCC(lpIMC->hCompStr);

    if (fFlag)
    {
#if 1
        // �ѱ� IME 2002�� �̻��ϰԵ�, ���� WM_IME_ENDCOMPOSITION�� ���
        // �� ������ WM_IME_COMPOSITION�� ��´�.
        // �̷��� �ؾ߸� ���Ÿ�ڿ��� ���� �۵��� �Ѵ�.
        GnMsg.message = WM_IME_ENDCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
#endif

        GnMsg.message = WM_IME_COMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = GCS_RESULTALL;
        if (dwImeFlag & SAENARU_ONTHESPOT)
        {
            LPMYSTR lpptr;
            LPMYSTR lpstr;
            DWORD dwCurPos;
        
            dwCurPos = lpCompStr->dwCursorPos;
            lpstr = GETLPCOMPSTR(lpCompStr);
            lpptr = MyCharPrev( lpstr, lpstr+dwCurPos );

            GnMsg.wParam = *lpptr;
        }
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
#if 0
        GnMsg.message = WM_IME_ENDCOMPOSITION;
        GnMsg.wParam = 0;
        GnMsg.lParam = 0;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
#endif
    }

    ImmUnlockIMC(hIMC);

    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/*      MakeGuideLine()                                               */
/*                                                                    */
/*      Update the transrate key buffer.                              */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL MakeGuideLine(HIMC hIMC, DWORD dwID)
{

    LPINPUTCONTEXT lpIMC;
    LPGUIDELINE    lpGuideLine;
    TRANSMSG GnMsg;
    DWORD dwSize = sizeof(GUIDELINE) + (MAXGLCHAR + sizeof(MYCHAR)) * 2 * sizeof(MYCHAR);
    LPMYSTR lpStr;

    lpIMC = ImmLockIMC(hIMC);
    lpIMC->hGuideLine = ImmReSizeIMCC(lpIMC->hGuideLine,dwSize);
    lpGuideLine = (LPGUIDELINE)ImmLockIMCC(lpIMC->hGuideLine);


    lpGuideLine->dwSize = dwSize;
    lpGuideLine->dwLevel = glTable[dwID].dwLevel;
    lpGuideLine->dwIndex = glTable[dwID].dwIndex;
    lpGuideLine->dwStrOffset = sizeof(GUIDELINE);
    lpStr = (LPMYSTR)(((LPSTR)lpGuideLine) + lpGuideLine->dwStrOffset);
    LoadString(hInst,glTable[dwID].dwStrID,lpStr, MAXGLCHAR);
    lpGuideLine->dwStrLen = Mylstrlen(lpStr);

    if (glTable[dwID].dwPrivateID)
    {
        lpGuideLine->dwPrivateOffset = sizeof(GUIDELINE) + (MAXGLCHAR + 1) * sizeof(MYCHAR);
        lpStr = (LPMYSTR)(((LPSTR)lpGuideLine) + lpGuideLine->dwPrivateOffset);
        LoadString(hInst,glTable[dwID].dwStrID,lpStr, MAXGLCHAR);
        lpGuideLine->dwPrivateSize = Mylstrlen(lpStr) * sizeof(MYCHAR);
    }
    else
    {
        lpGuideLine->dwPrivateOffset = 0L;
        lpGuideLine->dwPrivateSize = 0L;
    }

    GnMsg.message = WM_IME_NOTIFY;
    GnMsg.wParam = IMN_GUIDELINE;
    GnMsg.lParam = 0;
    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

    ImmUnlockIMCC(lpIMC->hGuideLine);
    ImmUnlockIMC(hIMC);

    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/*      GenerateMessage()                                             */
/*                                                                    */
/*      Update the transrate key buffer.                              */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL GenerateMessage(HIMC hIMC, LPINPUTCONTEXT lpIMC, LPTRANSMSGLIST lpTransBuf,LPTRANSMSG lpGeneMsg)
{
    MyDebugPrint((TEXT("GenerateMessage:TransKey %d\n"),lpGeneMsg->message));
    if (lpTransBuf)
        return GenerateMessageToTransKey(lpTransBuf,lpGeneMsg);
    MyDebugPrint((TEXT("GenerateMessage:ImmGenerateMessage %d\n"),lpGeneMsg->message));

    if (IsWindow(lpIMC->hWnd))
    {
        LPTRANSMSG lpTransMsg;
        if (!(lpIMC->hMsgBuf = ImmReSizeIMCC(lpIMC->hMsgBuf,
             sizeof(TRANSMSG) * (lpIMC->dwNumMsgBuf +1))))
            return FALSE;

        if (!(lpTransMsg = (LPTRANSMSG)ImmLockIMCC(lpIMC->hMsgBuf)))
            return FALSE;

        lpTransMsg[lpIMC->dwNumMsgBuf] = *lpGeneMsg;
        ImmUnlockIMCC(lpIMC->hMsgBuf);
        lpIMC->dwNumMsgBuf++;

        ImmGenerateMessage(hIMC);
    }
    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/*  Entry    : CheckAttr( LPCOMPOSITIONSTRING)                        */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL CheckAttr( LPCOMPOSITIONSTRING lpCompStr)
{
    int i,len;
    LPBYTE lpb = GETLPCOMPATTR(lpCompStr);

    len = lpCompStr->dwCompAttrLen;
    for (i = 0; i < len; i++)
        if (*lpb++ & 0x01)
            return TRUE;

    return FALSE;
}

/**********************************************************************/
/*                                                                    */
/*  Entry    : MakeAttrClause( LPCOMPOSITIONSTRING)                   */
/*                                                                    */
/**********************************************************************/
void PASCAL MakeAttrClause( LPCOMPOSITIONSTRING lpCompStr)
{
    int len = lpCompStr->dwCompAttrLen;
    int readlen = lpCompStr->dwCompReadAttrLen;
    LPDWORD lpdw;
    LPBYTE lpb;
    DWORD dwCursorPos = lpCompStr->dwCursorPos;
    int i;

    if (len != readlen)
        return;

    lpb = GETLPCOMPATTR(lpCompStr);
    for (i = 0;i < len; i++)
    {
        if ((DWORD)i < dwCursorPos)
            *lpb++ = 0x10;
        else
            *lpb++ = 0x00;
    }

    lpb = GETLPCOMPREADATTR(lpCompStr);
    for (i = 0;i < readlen; i++)
    {
        if ((DWORD)i < dwCursorPos)
            *lpb++ = 0x10;
        else
            *lpb++ = 0x00;
    }

    lpdw = GETLPCOMPCLAUSE(lpCompStr);
    *lpdw++ = 0;
    *lpdw++ = (BYTE)dwCursorPos;
    *lpdw++ = len;

    lpdw = GETLPCOMPREADCLAUSE(lpCompStr);
    *lpdw++ = 0;
    *lpdw++ = (BYTE)dwCursorPos;
    *lpdw++ = len;
}

/**********************************************************************/
/*                                                                    */
/*  Entry    : HandleShiftArrow( HIMC, fArrow)                        */
/*                                                                    */
/**********************************************************************/
void PASCAL HandleShiftArrow( HIMC hIMC, BOOL fArrow)
{
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    DWORD dwStartClause = 0;
    DWORD dwEndClause = 0;
    LPMYSTR lpstart,lpstr,lpend;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return;

    if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
    {

        // Temp! Error, if the string is already converted.
        if (CheckAttr(lpCompStr))
            goto hsa_exit;

        lpstart = GETLPCOMPSTR(lpCompStr);
        lpstr = lpstart + lpCompStr->dwCursorPos;
        lpend = lpstart + Mylstrlen(lpstart);

        if (fArrow == ARR_RIGHT)
        {
            if (lpstr < lpend)
                lpstr = MyCharNext(lpstr);
        }
        else
        {
            if (lpstr > lpstart)
                lpstr = MyCharPrev(lpstart,lpstr);
        }

        lpCompStr->dwCursorPos = (DWORD)(lpstr - lpstart);
        MakeAttrClause(lpCompStr);
    }

hsa_exit:
    ImmUnlockIMCC(lpIMC->hCompStr);
    ImmUnlockIMC(hIMC);
}


int CopyCandidateStringsFromDictionary(LPMYSTR lpDic, LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen)
{
    DWORD dwWritten = 0;
    LPMYSTR lpSection, lpTemp, lpTemp2;
    //const LPMYSTR szSep = MYTEXT(" \r\n\t");
    const LPMYSTR szSep = MYTEXT("\r\n\t");

    LPMYSTR lpToken = Mystrtok(lpDic, szSep);
    while (NULL != lpToken)
    {
        if (MYTEXT('[') == *lpToken)
        {
            lpSection = lpToken + 1;
            if (NULL != (lpTemp = Mystrchr(lpSection, MYTEXT(']'))))
                *lpTemp = MYTEXT('\0');
            if (0 == Mylstrcmp(lpSection, lpRead))
            {
                lpToken = Mystrtok(NULL, szSep);
                break; // found it.
            }
        }
        lpToken = Mystrtok(NULL, szSep);
    }
    if (NULL != lpToken)
    {
        LPMYSTR lpWrite = lpBuf;
        DWORD dwW;
        DWORD len;
        while ((NULL != lpToken) &&
               ((dwBufLen - dwWritten) > 1) &&
               (MYTEXT('[') != *lpToken))
        {
            INT isasc;
            isasc=FALSE;
            if (NULL != (lpTemp = Mystrchr(lpToken, MYTEXT('='))))
                *lpTemp = MYTEXT('\0');

            len = Mylstrlen(lpToken);
            if (*lpToken < 0x7f)
                isasc=TRUE;
            // �� �����̸鼭 KSX1002 ������ �ƴϸ� charset üũ
            if (!isasc && len == 1 && !(dwOptionFlag & KSX1002_SUPPORT))
            {
                WORD mb;
                WideCharToMultiByte(949, WC_COMPOSITECHECK,
                        lpToken, 1, (char *)&mb, 2, NULL, NULL);

                if(LOBYTE(mb) < 0xa1 || LOBYTE(mb) > 0xfe 
                    || HIBYTE(mb) < 0xa1 || HIBYTE(mb) > 0xfe)
                {
                    lpToken = Mystrtok(NULL, szSep);
                    continue;
                }
            }
            // ���� ������
            if ( Mylstrlen(lpTemp+1) > 1) {
                *lpTemp = MYTEXT(' ');
                len = Mylstrlen(lpToken);
            }
            // XXX ���� �� ��ü ũ�Ⱑ Ŀ���� �Է±Ⱑ �״´�
            if ((dwBufLen - dwWritten -1) < len) break;

            Mylstrcpyn(lpWrite, lpToken, dwBufLen - dwWritten - 1);
            dwW = len + 1;
            lpWrite += dwW;
            dwWritten += dwW;
            lpToken = Mystrtok(NULL, szSep);
        }
        *lpWrite = MYTEXT('\0');
        dwWritten++;
        return dwWritten;
    }
    return 0;
}

int GetCandidateStringsFromDictionary(LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename)
{
    HANDLE hTblFile;
    PSECURITY_ATTRIBUTES psa;
    int nSize = 0;
    DWORD dwFileSize, dwRead;
    LPMYSTR lpDic;

    psa = CreateSecurityAttributes();

    hTblFile = CreateFile(lpFilename, 
                          GENERIC_READ,
                          FILE_SHARE_READ,
                          NULL, 
                          OPEN_EXISTING,
                          FILE_ATTRIBUTE_NORMAL, 
                          (HANDLE)NULL);

    if (hTblFile == INVALID_HANDLE_VALUE) {
        goto Err0;
    }

    if (dwBufLen > 2) 
    {
        if ((dwFileSize = GetFileSize(hTblFile, (LPDWORD)NULL)) != 0xffffffff)
        {
            if ((lpDic = (LPMYSTR)GlobalAlloc(GPTR, dwFileSize + 2)))
            {
                if (ReadFile(hTblFile,    lpDic, dwFileSize, &dwRead, NULL))
                {
                    if (*lpDic == 0xfeff)
                    {
                        *(LPWSTR)(((LPBYTE)lpDic) + dwFileSize) = MYTEXT('\0');
                        nSize = CopyCandidateStringsFromDictionary(lpDic+1, lpRead, lpBuf, dwBufLen);
                    }
                }
                GlobalFree(lpDic);
            }
        }
    }

    CloseHandle(hTblFile);

Err0:
    FreeSecurityAttributes(psa);
    return nSize;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
