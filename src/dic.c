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
 * $Id$
 */

#include <windows.h>
#include <immdev.h>
#include "saenaru.h"
#include "resource.h"
#include "vksub.h"
#include "immsec.h"

#define hangul_is_choseong(ch)	((ch) >= 0x1100 && (ch) <= 0x115e)
#define hangul_is_jungseong(ch)	((ch) >= 0x1161 && (ch) <= 0x11a7)
#define hangul_is_jongseong(ch)	((ch) >= 0x11a8 && (ch) <= 0x11ff)
#define hangul_is_bangjum(ch)	((ch) == 0x302e || (ch) == 0x302f)
#define is_combining_mark(ch)	((ch) >= 0x0300 && (ch) <= 0x036f || \
	(ch) >= 0x1dc0 && (ch) <= 0x1de6 || (ch) >= 0x20d0 && (ch) <= 0x20f0)

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

DWORD dwCandStyle = IME_CAND_READ;

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
        //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
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
BOOL PASCAL ConvHanja(HIMC hIMC, int offset, int select)
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
    MYCHAR myBuf[256];
    BOOL isasc=FALSE;
    UINT cand_mode = dwCandStyle;
    UINT word_mode = 0;
    int readcand = 0;
    BOOL fHanja=FALSE;

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

    if (IsCandidate(lpIMC)) {
        MYCHAR buf[128];
        LPMYSTR lpmy;

        // get already made cadidate info.
        lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);

        if (lpCandList->dwSelection == 0) {
            lpmy = (LPMYSTR)((LPSTR)lpCandList + lpCandList->dwOffset[0]);
            Mylstrcpy(buf, lpmy); // hangul word

            if (0 != Mylstrcmp(lpmy, lpT2)) {
                // need to update candidate info.
                readcand = 1;
            }
        }
    }

    // set candidate style
    if (Mylstrlen(lpT2) > 1) { // word dic.
        cand_mode = IME_CAND_READ; // force CAND_READ
    } else if (isasc || (*lpT2 >= 0x3131 && 0x314e >= *lpT2) ) { // symbol
        cand_mode = IME_CAND_CODE; // force CAND_CODE
    } else {
        word_mode = 1;
    }

    if (!IsCandidate(lpIMC) || readcand) {
        LPTSTR  lpKey;
        TCHAR   szKey[128];
        LPTSTR  lpDic;
        TCHAR   szDic[256];
        LPTSTR  lpIdx;
        TCHAR   szIdx[256];
        INT sz;

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

        //
        // 사전에서 발견되지 않음. 한자가 섞여있거나 한자단어이면 이것을 한글로 변환 시도.
        //
        while (nBufLen < 1 && Mylstrlen(lpT2) < 255) {
            MYCHAR han[256];
            LPMYSTR lpstr = lpT2, lpHan = han;
            han[0]=MYTEXT('\0');

            // check the hanja_to_hangul table
            lpIdx = (LPTSTR)&szIdx;
            LoadString( hInst, IDS_HIDX_KEY, lpKey, 128);
            lpIdx += GetSaenaruDirectory(lpIdx,256);
            if (*(lpIdx-1) != TEXT('\\')) *lpIdx++ = TEXT('\\');

            sz= GetRegStringValue(TEXT("\\Dictionary"),lpKey,NULL);
            if (sz > 2) {
                sz= GetRegStringValue(TEXT("\\Dictionary"),(LPTSTR)lpKey,lpIdx);
                MyDebugPrint((TEXT("Saenaru: %s Idx %s:%d\n"),lpKey, szIdx,sz));
            } else {
                break; // hanjaidx not found
            }

            while (*lpstr) {
                UINT n;
                if (*lpstr < 0x3400 || *lpstr >=0xfaff) {
                    *lpHan = *lpstr; // skip. just copy
                } else if (*lpstr >= 0xa000 && *lpstr <=0xf8ff) {
                    *lpHan = *lpstr; // skip. just copy
                } else {
                    // FIXME
                    nBufLen = GetHangulFromHanjaIndex(lpstr, lpHan, 2, (LPTSTR)szIdx); // search Hangul
                    if (nBufLen < 1)
                        break; // not found in the hanja idx
                }
                lpstr++;
                lpHan++;
            }

            if (nBufLen > 0) {
                *lpHan = MYTEXT('\0');
                Mylstrcpy(szBuf, han); // copy hangul to candidate list first.
                szBuf[Mylstrlen(han)] = 0;

                // get hanja list for this hangul
                nBufLen = GetCandidateStringsFromDictionary(han,
                        szBuf+Mylstrlen(han)+1, 1024, (LPTSTR)szDic);
                if (nBufLen == 0) { // not found in the dic
                    nBufLen=1;
                    szBuf[Mylstrlen(han) + 1]= 0; // double terminate
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

            lpCompStr->dwCompStrLen = Mylstrlen(GETLPCOMPSTR(lpCompStr));
        }
    } else {
        nBufLen = 1;
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
            MyDebugPrint((TEXT(">>> TOO Big Dic=%d\n"), cnt));
            goto cvk_exit40;
        }
    }

    lpb = GETLPCOMPATTR(lpCompStr);

    if (nBufLen < 1)
    {
        // 사전에 엔트리가 없다.
        if (IsCandidate(lpIMC))
        {
            // 이미 열린 후보창이 있으면 닫는다.
            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
            ClearCandidate(lpCandInfo);
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_CLOSECANDIDATE;
            GnMsg.lParam = 1;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
        }

        if (!*lpb)
        {
            //goto cvk_exit40;
            //
            // make attribute
            //
            lmemset(GETLPCOMPATTR(lpCompStr),ATTR_TARGET_NOTCONVERTED ,
                  Mylstrlen(GETLPCOMPSTR(lpCompStr)));
            lmemset(GETLPCOMPREADATTR(lpCompStr),ATTR_TARGET_NOTCONVERTED,
                  Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));

            GnMsg.message = WM_IME_COMPOSITION;
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
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
    if (!*lpb && 0)
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
                if (NULL != (lpTemp = Mystrchr(lpmystr, MYTEXT(':'))))
                    *lpTemp = MYTEXT('\0');

                Mylstrcpy(GETLPCOMPSTR(lpCompStr),lpmystr);
                if ((dwImeFlag & SAENARU_ONTHESPOT)&& Mylstrlen(lpmystr) == 1)
                   cs = *lpmystr;

                lpstr = GETLPCOMPSTR(lpCompStr);

                //
                // Set the length and cursor position to the structure.
                //
                lpCompStr->dwCompStrLen = Mylstrlen(lpmystr);
                //lpCompStr->dwCursorPos = 0; // XXX
                lpCompStr->dwCursorPos = Mylstrlen(lpmystr);
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
                    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
                    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                    MyDebugPrint((TEXT(" *** ConvHanja: GnMsg > 1\r\n")));
                } else
#endif
                {
                    GnMsg.message = WM_IME_COMPOSITION;
                    GnMsg.wParam = cs;
                    GnMsg.lParam = GCS_COMPALL | GCS_CURSORPOS | GCS_DELTASTART;
                    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
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

        if (!IsCandidate(lpIMC))
        {
            //
            // generate WM_IME_NOTFIY IMN_OPENCANDIDATE message.
            //
            GnMsg.message = WM_IME_NOTIFY;
            GnMsg.wParam = IMN_OPENCANDIDATE;
            GnMsg.lParam = 1L;
            GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
            MyDebugPrint((TEXT("ConvHanja: OpenCandidate\r\n")));
        }

        if (!IsCandidate(lpIMC) || readcand)
        {
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

                if (fHanja && *lpmystr == *lpT2) {
                    // 한자를 한글로 변환하는 경우.
                    // 한자 후보 리스트에 한자가 발견되면 이것을 가리키도록 한다.
                    select = i + 1;
                    fHanja = 0;
                }
                lpstr += (Mylstrlen(lpstr) + 1);
                i++;
            }
            MyDebugPrint((TEXT("ConvHanja: OpenCandidate - %d\r\n"), i));

            lpCandList->dwSize = sizeof(CANDIDATELIST) +
                (MAXCANDSTRNUM * (sizeof(DWORD) + MAXCANDSTRSIZE));
            lpCandList->dwCount = i;

            if (i < MAXCANDPAGESIZE)
                lpCandList->dwPageSize  = i;
            else
                lpCandList->dwPageSize  = MAXCANDPAGESIZE;
        }
        // set candidate style.
        lpCandList->dwStyle = cand_mode;

        // Total pages
        pages = lpCandList->dwCount / lpCandList->dwPageSize + lpCandList->dwCount % lpCandList->dwPageSize;

        if (select)
        {
            int sel = lpCandList->dwSelection / lpCandList->dwPageSize;
            sel *= lpCandList->dwPageSize;
            if (select != -999) {
                int count = lpCandList->dwCount;
                sel +=select - 1;
                if (sel >= count)
                    sel = count - 1;
            } else {
                sel = 0;
            }
            lpCandList->dwSelection = sel;
            offset = 0;
        }
        else if (offset > 2)
        {
            // last (END)
            lpCandList->dwSelection = lpCandList->dwCount - 1;
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
            int count = lpCandList->dwCount;
            //if (select < 0) select= pages * lpCandList->dwPageSize - offset;
            if (select < 0) select= 0;
            if (select >= count)
                select = count - 1;    
            lpCandList->dwSelection = (DWORD)select;
        } else {
            lpCandList->dwSelection += offset;
            if (offset > 1 && lpCandList->dwSelection >= (DWORD)lpCandList->dwCount)
                lpCandList->dwSelection = lpCandList->dwCount - 1;
        }

        // 맨 끝 + 1이면 맨 처음으로
        if (lpCandList->dwSelection >= (DWORD)lpCandList->dwCount)
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
        if (select != -999)
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

        MyDebugPrint((TEXT(">>>>>>>> lpptr: %s:%d\r\n"), lpptr, dwCurPos));
        if ( is_combining_mark(*lpptr)) {
            LPMYSTR lpprev;
            // combining mark가 발견되면 먼저 지운다.
            MyDebugPrint((TEXT(">> Delete Combining Mark: %s:%d\r\n"), lpptr, dwCurPos));
            lpprev = lpptr - 1;
            dwCurPos--;
            while(is_combining_mark(*lpprev)) {
                MyDebugPrint((TEXT(">> Delete Combining Mark: %s:%d\r\n"), lpprev, dwCurPos));
                lpprev--;
                dwCurPos--;
            }
            cs = *lpprev;
            *(lpprev+1) = MYTEXT('\0');
        } else if ( ic.len && (dwOptionFlag & BACKSPACE_BY_JAMO)) {
            // Delete jamos
            if (ic.len > 0) {
                // 조합중인 글자?
                WCHAR last;
                int ncs = 0;

                hangul_ic_pop(&ic); // delete
                last= hangul_ic_peek(&ic); // get last
                if (last) {
                    if (ic.laststate == 1) ic.cho=0;
                    else if (ic.laststate == 2) ic.jung=0;
                    else if (ic.laststate == 3) ic.jong=0;
                }

                if (ic.syllable) {
                    ncs = hangul_ic_get(&ic,0, lpptr);
                    if (ncs >= 1) {
                        cs = *(lpptr + ncs - 1);
                    } else {
                        // 조합중인 글자가 모두 지워짐.
                        hangul_ic_init(&ic);
                        if (dwCurPos > 0) {
                            cs = *(lpptr-1);

                            dwCurPos--;
                        }
                        Mylstrcpy( lpptr, lpstr+dwCurPos );
                        *(lpptr) = MYTEXT('\0');
                    }
                    MyDebugPrint((TEXT(">> Syllable Delete Han Char: len=%d, %s:%d\r\n"), ic.len, lpptr, dwCurPos));
                } else {
                    LPMYSTR lpprev;
                    // syllable이 아님.
                    // 옛한글.

                    MyDebugPrint((TEXT(">> Pre BACKSPACE Delete Han Char: %s:%d\r\n"), lpptr, dwCurPos));
                    lpprev = lpptr;

                    ncs = hangul_del_prev(lpprev);
                    dwCurPos-= ncs - 1;
                    MyDebugPrint((TEXT(">> Post BACKSPACE Delete Han Char: %s %d\r\n"), lpprev, ncs));

                    lpptr -= ncs - 1; // 이전 문자 위치로.
                    MyDebugPrint((TEXT(">>> Get Delete Han Char: %s:%d\r\n"), lpptr, ncs));

                    // 재조합된 문자열을 가져옴.
                    ncs = hangul_ic_get(&ic,0, lpptr);
                    if (ncs > 0) {
                        MyDebugPrint((TEXT(">>> Get result Han Char: %s : %d\r\n"), lpptr, ncs));
                        lpptr += ncs;
                        cs = *(lpptr - 1);
                        dwCurPos+= ncs - 1;
                    } else {
                        // 조합중인 글자가 모두 지워짐.
                        hangul_ic_init(&ic);
                        dwCurPos--;
                    }
                    MyDebugPrint((TEXT(">>> CompStr result: %s:%d\r\n"), lpstr, dwCurPos));
                    *lpptr = MYTEXT('\0');
                }
                // change last state
                if (last) {
                    if (hangul_is_choseong(last)) ic.laststate=1;
                    else if (hangul_is_jungseong(last)) ic.laststate=2;
                    else if (hangul_is_jongseong(last)) ic.laststate=3;

                    ic.last = last;
                }
            } else {
                // 조합중인 글자가 모두 지워짐.
                hangul_ic_init(&ic);
                if (dwCurPos > 0) {
                    cs = *(lpptr-1);
                }
                dwCurPos--;
                Mylstrcpy( lpptr, lpstr+dwCurPos );
                *lpptr = MYTEXT('\0');
                MyDebugPrint((TEXT(">>> Delete Single Char: %s:%d\r\n"), lpstr, dwCurPos));
            }
        } else {
            LPMYSTR lpprev;
            nChar = 0;
            /*
             * 첫가끝 조합 완료된 글자가 있는가?
             * 발견되면 첫가끝 조합 글자를 지워준다.
             * 이때, L*V*T+를 찾아서 모두 지운다?
             */
            // 지울 개수를 얻음.
            nChar = hangul_del_prev(lpptr);

            if (nChar == 0)
                nChar = 1;

            MyDebugPrint((TEXT("delete nChar: %x\r\n"), nChar));

            if( lpstr == lpptr && Mylstrlen(lpstr) == nChar )
            {
                dwCurPos = 0;
                *lpstr = MYTEXT('\0');
            }
            else
            {
                //Mylstrcpy( lpptr, lpstr+dwCurPos );
                dwCurPos -= nChar;
                *(lpptr-nChar+1) = MYTEXT('\0'); // 마지막 문자 지움.
                cs = *(lpptr-nChar);
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
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
            if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS))
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
            GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; // 한글 IME 2002,2003
            if ((dwImeFlag & SAENARU_ONTHESPOT) && !(dwOptionFlag & HANGUL_JAMOS))
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
        // 없으면 초기화
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
    //GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR | dwGCR; // 한글 IME 2002,2003
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
    UINT select=0, changed=0;
    LRESULT candOk=FALSE;

    // Candidate문자 선택
    if (IsConvertedCompStr(hIMC))
    {
        LPCANDIDATEINFO lpCandInfo;
        LPCANDIDATELIST lpCandList;
        DWORD cand_style = dwCandStyle;

        if ((wParam == VK_LEFT || wParam == VK_RIGHT) && (lpIMC = ImmLockIMC(hIMC)) ) {
            lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

            if (lpCandInfo) {
                lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);

                cand_style = lpCandList->dwStyle;
                ImmUnlockIMCC(lpIMC->hCandInfo);
            }
            ImmUnlockIMC(hIMC);
        }

        // XXX
        switch( wParam )
        {
            case VK_TAB:
                {
                    // toggle cand style
                    if (dwCandStyle == IME_CAND_READ)
                        dwCandStyle = IME_CAND_CODE;
                    else
                        dwCandStyle = IME_CAND_READ;
                    changed = 1;
                }
                break;
            case VK_LEFT:
                if (cand_style == IME_CAND_CODE) {
                    next=-2;
                }
                break;
            case VK_RIGHT:
                if (cand_style == IME_CAND_CODE) {
                    next=2;
                }
                break;
            
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
                select= -999; // XXX
                break;
            default:
                break;
        }
        if (next || select || changed)
        {
            ConvHanja(hIMC,next,select);
            if (next || changed) return TRUE;
            wParam=VK_RETURN;
        }
    }

    switch( wParam )
    {
        case VK_DELETE:
        case VK_BACK:
            if (IsConvertedCompStr(hIMC))
            {
                if (!(lpIMC = ImmLockIMC(hIMC)))
                    return FALSE;

                fdwConversion = lpIMC->fdwConversion;

                // 후보창이 열려있는 경우. 아래를 옵션으로 처리해야 한다. XXX
                if ((fdwConversion & IME_CMODE_HANJACONVERT) &&
                    (fdwConversion & IME_CMODE_NATIVE))
                {
                    TRANSMSG GnMsg;

                    if (ImmGetIMCCSize(lpIMC->hCompStr) < sizeof (MYCOMPSTR))
                    {
                        ImmUnlockIMC(hIMC);
                        return FALSE;
                    }

                    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

                    // 속성을 ATTR_TARGET_CONVERTED에서 ATTR_INPUT으로 원상복귀.
                    lmemset(GETLPCOMPATTR(lpCompStr),ATTR_INPUT,
                        Mylstrlen(GETLPCOMPSTR(lpCompStr)));
                    lmemset(GETLPCOMPREADATTR(lpCompStr),ATTR_INPUT,
                        Mylstrlen(GETLPCOMPREADSTR(lpCompStr)));

                    lpCompStr->dwCompAttrLen = 0;
                    lpCompStr->dwCompReadAttrLen = 0;
                    // 이상하게도 CursorPos가 0이 되어 있다. 이 값을 기본값으로 원상복귀.
                    lpCompStr->dwCursorPos = Mylstrlen(GETLPCOMPSTR(lpCompStr));

                    GnMsg.message = WM_IME_COMPOSITION;
                    GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR | GCS_CURSORPOS;
                    GnMsg.wParam = 0;
                    GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);

                    ImmUnlockIMCC(lpIMC->hCompStr);

                    DeleteChar(hIMC,wParam);

                    // 후보창이 열려있는데, 조합중인 한글이 초성만 있는 경우
                    if (ic.len == 1 && IsCandidate(lpIMC)) {
                        LPCANDIDATEINFO lpCandInfo;
                        // 열린 창을 닫아준다.
                        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                        ClearCandidate(lpCandInfo);
                        ImmUnlockIMCC(lpIMC->hCandInfo);
                        GnMsg.message = WM_IME_NOTIFY;
                        GnMsg.wParam = IMN_CLOSECANDIDATE;
                        GnMsg.lParam = 1L;
                        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                    } else {
                        ConvHanja(hIMC,0,0);
                    }
                } else {
                    FlushText(hIMC);
                }

                ImmUnlockIMC(hIMC);

                return TRUE;
            } else if (IsCompStr(hIMC))
            {
                if (IsSHFTPushed(lpbKeyState)) {
                    hangul_ic_init(&ic);
                    DeleteChar(hIMC,wParam);

                    return TRUE;
                }
                DeleteChar(hIMC,wParam);
                return TRUE;
            } else if (IsSHFTPushed(lpbKeyState)) {
                int ns=0;
                DWORD dwSize = (DWORD)MyImmRequestMessage(hIMC, IMR_DOCUMENTFEED, 0);

                MyDebugPrint((TEXT(">>>>>>>IMR_DOCUMENTFEED result %d\r\n"),dwSize));

                if (!(lpIMC = ImmLockIMC(hIMC)))
                     return FALSE;

                if (dwSize) {
                    LPRECONVERTSTRING lpRS;

                    lpRS = (LPRECONVERTSTRING)GlobalAlloc(GPTR, dwSize);
                    lpRS->dwSize = dwSize;

                    if (dwSize = (DWORD) MyImmRequestMessage(hIMC, IMR_DOCUMENTFEED, (LPARAM)lpRS)) {
                        BOOL convOk= FALSE;
                        TRANSMSG GnMsg;

#ifdef DEBUG
                        TCHAR szDev[80];
#endif
                        //LPMYSTR lpDump= (LPMYSTR)(((LPSTR)lpRS) + lpRS->dwStrOffset);
                        // *(LPMYSTR)(lpDump + lpRS->dwStrLen) = MYTEXT('\0');
                        //LPMYSTR lpDump= (LPMYSTR)(((LPSTR)lpRS) + lpRS->dwStrOffset);
                        LPMYSTR lpDump= (LPMYSTR)(((LPSTR)lpRS) + lpRS->dwStrOffset + lpRS->dwCompStrOffset);

                        MyDebugPrint((TEXT(">>>>>>>IMR_DOCUMENTFEED str %s\r\n"),lpDump));
#ifdef DEBUG
                        OutputDebugString(TEXT("IMR_DOCUMENTFEED\r\n"));
                        wsprintf(szDev, TEXT("dwSize       %d\r\n"), lpRS->dwSize);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwVersion    %d\r\n"), lpRS->dwVersion);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwStrLen     %d\r\n"), lpRS->dwStrLen);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwStrOffset  %d\r\n"), lpRS->dwStrOffset);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwCompStrLen %d\r\n"), lpRS->dwCompStrLen);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwCompStrOffset %d\r\n"), lpRS->dwCompStrOffset);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwTargetStrLen %d\r\n"), lpRS->dwTargetStrLen);
                        OutputDebugString(szDev);
                        wsprintf(szDev, TEXT("dwTargetStrOffset %d\r\n"), lpRS->dwTargetStrOffset);
                        OutputDebugString(szDev);
#endif

                        *(lpDump) = MYTEXT('\0');
                        if (lpRS->dwCompStrOffset > 1) {
                            lpDump--;
                            MyDebugPrint((TEXT(">>>>>> %s\r\n"), lpDump));
                            lpRS->dwCompStrLen = 1;
                            lpRS->dwTargetStrLen = 1;
                            lpRS->dwCompStrOffset -= sizeof(MYCHAR);

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
                                        // 워드패드와 M$ Explorer는 반드시 WM_IME_STARTCOMPOSITION으로 시작해야 한다.
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
                                        GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; //한글 IME 2002,2003
                                        //if (dwImeFlag & SAENARU_ONTHESPOT)
                                        //    GnMsg.lParam |= CS_INSERTCHAR | CS_NOMOVECARET;
                                        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
#endif
                                    } else {
                                        OutputDebugString(TEXT(" *** lpCompStr== NULL\r\n"));
                                    }

                                    convOk = (BOOL) MyImmRequestMessage(hIMC, IMR_CONFIRMRECONVERTSTRING, (LPARAM)lpRS);
                                    if (!convOk) {
                                        // MS IE와 모질라는 이 부분이 뒤에 와야 한다. 즉, CompStr으 세팅한 후에
                                        // 시작해야 한다 ?
                                        OutputDebugString(TEXT(" *** fail CONFIRM RECONVERT\r\n"));
                                    } else {
                                        OutputDebugString(TEXT(" *** success CONFIRM RECONVERT\r\n"));
#ifdef DEBUG
                                        wsprintf(szDev, TEXT(" *** result: dwCompStrLen %x\r\n"), lpRS->dwCompStrLen);
                                        OutputDebugString(szDev);
#endif

                                        if (hangul_is_syllable(*lpDump)) {
                                            WCHAR l, v, t;
                                            hangul_syllable_to_jamo(*lpDump, &l, &v, &t);
                                            if (l && v) {
                                                hangul_ic_init(&ic);
                                                ic.cho = l;
                                                ic.jung = v;
                                                ic.laststate = 2;
                                                hangul_ic_push(&ic, l);
                                                hangul_ic_push(&ic, v);
                                                if (t) {
                                                    hangul_ic_push(&ic, t);
                                                    ic.laststate = 3;
                                                }
                                            }
                                        }
                                        // 
                                        DeleteChar(hIMC,wParam);
                                    }
                                }
                                ImmUnlockIMCC (lpIMC->hCompStr);
                            }
                        }
                        GlobalFree((HANDLE)lpRS);

                    }
                }
                ImmUnlockIMC(hIMC);
                return TRUE;
            } else {
                // Simulate a key press
                keybd_event( (BYTE)wParam, 0x0, 0, 0 );
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
                                    // 워드패드와 M$ Explorer는 반드시 WM_IME_STARTCOMPOSITION으로 시작해야 한다.
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
                                    GnMsg.lParam = GCS_COMPSTR | GCS_COMPATTR; //한글 IME 2002,2003
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
                                    // MS IE와 모질라는 이 부분이 뒤에 와야 한다. 즉, CompStr으 세팅한 후에
                                    // 시작해야 한다 ?
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
            // M$ Word에 새나루가 붙지 않아서 Spy++로 메시지 추적해본
            // 결과, 한자키가 눌려지면 Comp모드에서는 VK_PROCESSKEY로
            // 들어오지만, Reconversion모드에서는 VK_PROCESSKEY와 함께
            // WM_IME_KEYDOWN이벤트도 생김. 그래서 이 이벤트를 집어넣으니
            // M$ Word에서도 제대로 작동하여 한자 사전이 뜬다.
            // * InternetExplorer에서도 Reconversion이 됨. IME 2003에서는 안됨
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
                // 어떤 어플은 자체 내장된 candidate리스트를 쓰고, 어떤것은 그렇지 않다.
                // 그런데 이러한 동작이 일관성이 없어서 어떤 어플은 특별처리 해야 한다.
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

            // 스페이스를 한자 페이지 넘기는 키로 사용한다.
            // 옵션으로 켜고 킬 수 있게.
            if (dwOptionFlag & HANJA_CAND_WITH_SPACE &&
                    IsCompStr(hIMC) && IsCandidate(lpIMC) &&
                    (fdwConversion & IME_CMODE_HANJACONVERT) &&
                    (fdwConversion & IME_CMODE_NATIVE)) {
                hangul_ic_init(&ic); // 이 경우는 한글을 초기화함.
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
            break;

        case VK_RIGHT:
            if (ic.len && IsCompStr(hIMC) && (!(dwImeFlag & SAENARU_ONTHESPOT) || dwOptionFlag & HANGUL_JAMOS))
            {
                hangul_ic_init(&ic);
                return TRUE;
            }
        case VK_LEFT:
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
                TRANSMSG GnMsg;

                MakeResultString(hIMC,TRUE);
                // Simulate a key press
                //keybd_event( (BYTE)wVKey, 0x0, 0, lParam ); // not work correctly for HOME,END etc.

                lpIMC = ImmLockIMC(hIMC);
                GnMsg.message = WM_IME_KEYDOWN;
                GnMsg.wParam = wParam;
                GnMsg.lParam = lParam;
                GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                ImmUnlockIMC(hIMC);

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
            // spy++로 한글 IME2002의 행동을 살펴본 결과,
            // CompStr이 있을 경우는 RETURN을 치면 처음 이벤트가 생겼을 때 한번
            // 그리고, CompStr을 commit한 후에 다시 한번 키이벤트가 생겼다.
            // strings로 imekr61.ime를 살펴본 결과 keybd_event()를 쓰고 있었으며
            // saenaru는 이 방식을 따랐다.
            //
            // 고고타자를 통해 분석해보니, WM_IME_KEYDOWN이 먼저 생겼다.
            // 그래서 위처럼 GenerateMessage()를 이용해보니 keybd_event를 쓰지
            // 않아도 되었다.
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
        // RETURN은 통과되지 않고, 바로 윗부분에서 처리가 끝난다.
        // 모질라만 잘 못 동작하지 않는다면 RETURN키도 처리할 수 있다.
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
    MYCHAR retbuf[128];
    retbuf[0]=MYTEXT('\0');

    if (!IsCompStr(hIMC))
        return FALSE;

    lpIMC = ImmLockIMC(hIMC);

    lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

    if (IsCandidate(lpIMC))
    {
        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);

        // check HanjaMode 
        if (dwHanjaMode) {
            LPCANDIDATELIST lpCandList;
            MYCHAR buf[128];
            MYCHAR buf2[128];
            LPMYSTR lpstr;

            lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);

            while (lpCandList->dwSelection != 0) {
                lpstr = (LPMYSTR)((LPSTR)lpCandList + lpCandList->dwOffset[0]);
                Mylstrcpy(buf, lpstr); // hangul word
                if (*lpstr >= 0x3131 && *lpstr <=0x314e)
                    break;

                Mylstrcpy(buf2, GETLPCOMPSTR(lpCompStr)); // hanja

                if (dwHanjaMode == 1) {
                    wsprintf(retbuf, TEXT("%s(%s)"), buf, buf2); // hangul(hanja)
                } else {
                    wsprintf(retbuf, TEXT("%s(%s)"), buf2, buf); // hanja(hangul)
                }
                MyDebugPrint(( TEXT("%s\n"), retbuf));
                break;
            }
        }

        ClearCandidate(lpCandInfo);
        ImmUnlockIMCC(lpIMC->hCandInfo);
        GnMsg.message = WM_IME_NOTIFY;
        GnMsg.wParam = IMN_CLOSECANDIDATE;
        GnMsg.lParam = 1L;
        GenerateMessage(hIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
    }

    if (retbuf[0] != MYTEXT('\0')) {
        // has hanja(hangul) or hangul(hanja) data ?
        Mylstrcpy(GETLPCOMPSTR(lpCompStr), retbuf);
        Mylstrcpy(GETLPCOMPREADSTR(lpCompStr), retbuf);

        lpCompStr->dwCompStrLen = Mylstrlen(retbuf);
        lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen;
    } else {
        lpCompStr->dwCompStrLen = Mylstrlen(GETLPCOMPSTR(lpCompStr));
        lpCompStr->dwCompReadStrLen = lpCompStr->dwCompStrLen;
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
        // 한글 IME 2002는 이상하게도, 먼저 WM_IME_ENDCOMPOSITION을 뱉고
        // 그 다음에 WM_IME_COMPOSITION을 뱉는다.
        // 이렇게 해야만 고고타자에서 정상 작동을 한다.
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


int CopyCandidateStringsFromDictionary(HANDLE hFile, LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen, int iFlag)
{
    DWORD dwWritten = 0;
    MYCHAR  myBuf[4098];
    LPMYSTR lpDic, lpSection, lpTemp, lpTemp2;
    const LPMYSTR szSep = MYTEXT("\n");
    LPMYSTR lpToken = NULL, lpNext = NULL;
    DWORD llen = 0, found = 0, dwRead;
    LPMYSTR lpWrite = lpBuf;

    myBuf[0] = MYTEXT('\0');
    lpDic = myBuf;

    while (ReadFile(hFile, lpDic + llen, 2048, &dwRead, NULL) && dwRead > 0)
    {
        *(LPMYSTR)(((LPBYTE)(lpDic + llen)) + dwRead) = MYTEXT('\0');

        lpToken = Mystrtok(lpDic, szSep);
        while (NULL != lpToken) {
            if (MYTEXT('[') == *lpToken)
            {
                lpSection = lpToken + 1;
                if (NULL != (lpTemp = Mystrchr(lpSection, MYTEXT(']'))))
                    *lpTemp = MYTEXT('\0');
                if (0 == Mylstrcmp(lpSection, lpRead))
                {
                    found = 1;
                    MyDebugPrint((TEXT(">>>>FOUND %s\n"), lpSection));
                    break; // found it.
                } else if ( *(lpSection) > *(lpRead) ) {
                    found = -1; // not found
                    break;
                }
            }
            lpToken = Mystrtok(NULL, szSep);
        }
        lpToken = Mystrtok(NULL, MYTEXT(""));
        llen = Mylstrlen(lpToken);
        if (llen > 0) {
            Mylstrcpy(lpDic, lpToken); // copy last
            llen = Mylstrlen(lpDic); // adjust XXX
            *(lpDic+llen) = MYTEXT('\0');
            MyDebugPrint((TEXT(">>>>lpDic Last:%d %s\n"), llen, lpDic));
        }
        if (found)
            break;
    }

    // not found
    if (found != 1)
        return 0;

    while (ReadFile(hFile, lpDic + llen, 2048, &dwRead, NULL)) {
        DWORD dwW;
        DWORD len;
        UINT force = 0;

        *(LPWSTR)(((LPBYTE)(lpDic + llen)) + dwRead) = MYTEXT('\0');
        lpNext = Mystrchr(lpDic, MYTEXT('\n'));
        if (lpNext != NULL) lpNext++; // point to next line

        //MyDebugPrint((TEXT(">>>>lpDic=%d, lpNext=%d\n"), lpDic, lpNext));

        lpToken = Mystrtok(lpDic, szSep);

        while ((NULL != lpToken) &&
                (lpWrite == NULL || (dwBufLen - dwWritten) > 1))
        {
            INT isasc, skip;
            LPMYSTR lpNext2;
            isasc=FALSE;
            skip=FALSE;

            if (*lpToken == MYTEXT('[')) {
                if (iFlag) {
                    // naver style search
                    lpSection = lpToken + 1;
                    if (NULL != (lpTemp = Mystrchr(lpSection, MYTEXT(']'))))
                        *lpTemp = MYTEXT('\0');
                    MyDebugPrint((TEXT(">>>> lpSection=%s\n"), lpSection));
                    if (Mylstrlen(lpSection) >= Mylstrlen(lpRead)) {
                        MYCHAR buf[128];
                        Mylstrcpyn(buf, lpSection, Mylstrlen(lpRead)+1);
                        if (0 == Mylstrcmp(buf, lpRead)) {
                            // ok. continue
                            MyDebugPrint((TEXT(">>>> lpSection=%s\n"), lpSection));
                            skip = TRUE;
                        } else {
                            force = TRUE;
                            break;
                        }
                    } else {
                        force = TRUE;
                        break;
                    }
                } else {
                    force = TRUE;
                    break;
                }
            }

            //MyDebugPrint((TEXT(">>>>WORD lpToken:%s\n"), lpToken));
            if (NULL != (lpTemp = Mystrchr(lpToken, MYTEXT('='))))
                *lpTemp = MYTEXT('\0');

            len = Mylstrlen(lpToken);
            if (*lpToken < 0x7f)
                isasc=TRUE;
            // 한 글자이면서 KSX1002 지원이 아니면 charset 체크
            if (!isasc && len == 1 && !(dwOptionFlag & KSX1002_SUPPORT))
            {
                WORD mb;
                WideCharToMultiByte(949, WC_COMPOSITECHECK,
                        lpToken, 1, (char *)&mb, 2, NULL, NULL);

                if(LOBYTE(mb) < 0xa1 || LOBYTE(mb) > 0xfe 
                        || HIBYTE(mb) < 0xa1 || HIBYTE(mb) > 0xfe)
                {
                    skip = TRUE;
                }
            }
            // 뜻이 있으면
            if ( !skip && lpTemp && Mylstrlen(lpTemp+1) > 1) {
                *lpTemp = MYTEXT(':');

                // 뜻이 길 경우는 잘라냄. FIXME
                if (NULL != (lpTemp = Mystrchr(lpToken, MYTEXT(','))))
                    *lpTemp = MYTEXT('\0');

                len = Mylstrlen(lpToken);
            }
            // XXX 뜻이 길어서 전체 크기가 커지면 입력기가 죽는다
            if ( !skip && lpWrite != NULL && (dwBufLen - dwWritten - 2) < len) {
                force = TRUE;
                break;
            }

            if (!skip) {
                dwW = len + 1;
                if (lpWrite != NULL) {
                    Mylstrcpyn(lpWrite, lpToken, dwW);
                    //Mylstrcpyn(lpWrite, lpToken, dwBufLen - dwWritten - 1);
                    lpWrite += dwW;
                }
                dwWritten += dwW;
            }

            if (lpNext == NULL) break; // empty line.
            lpNext2 = Mystrchr(lpNext, MYTEXT('\n'));
            if (lpNext2 == NULL) // have no CR
                break;
            else
                lpNext2++; // point to next line

            lpToken = Mystrtok(lpNext, szSep);
            lpNext = lpNext2;
        }

        if ((lpToken != NULL && (iFlag == 0 && MYTEXT('[') == *lpToken)) ||
                (lpWrite != NULL && (dwBufLen - dwWritten <= 1)) || force || !dwRead) {
            MyDebugPrint((TEXT(">>>>Wow lpToken:%c, force=%d, dwRead=%d, dwBuf - dwWrite=%d\n"), *lpToken, force, dwRead, dwBufLen - dwWritten));
            break;
        }

        if (lpToken != NULL) {
            lpToken = Mystrtok(NULL, MYTEXT(""));
            llen = Mylstrlen(lpToken);
            if (llen > 0) {
                Mylstrcpy(lpDic, lpToken); // copy last
                llen = Mylstrlen(lpDic); // adjust XXX
            }
        } else {
            llen = 0;
        }
    }
    if (lpWrite != NULL) {
        *lpWrite = MYTEXT('\0');
        dwWritten++;
    }
    return dwWritten;
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
            MYCHAR  buf[2];
            buf[0] = MYTEXT('\0');
            lpDic = (LPMYSTR)buf;
            if (ReadFile(hTblFile, lpDic, sizeof(MYCHAR), &dwRead, NULL)) // read BOM char.
            {
                if (*lpDic == 0xfeff) // is it BOM ?
                {
                    nSize = CopyCandidateStringsFromDictionary(hTblFile, lpRead, lpBuf, dwBufLen, dwOptionFlag & SEARCH_SIMILAR_WORDS ? 1:0);
                } // else ignore.
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
