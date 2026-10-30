/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 1990-1998 Microsoft Corporation.
 * Copyright (c) 2003 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2003-2026 Won-Kyu Park <wkpark@kldp.org>.
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


/* Dictionary file lookup shared by the IMM IME and the TSF TIP. */

#include <windows.h>
#include "unicode.h"
#include "hangul.h"
#include "debug.h"
#include "immsec.h"

extern DWORD dwOptionFlag;

int
PASCAL
GetSaenaruDirectory(LPTSTR lpDest, int max)
{
    static const TCHAR szIMEDir[] = TEXT("\\IME\\Saenaru");
    LPTSTR lpDestOrig = lpDest;

    lpDest += GetWindowsDirectory(lpDest, 256);
    memcpy(lpDest, szIMEDir, sizeof(szIMEDir) - sizeof(TCHAR));
    lpDest += sizeof(szIMEDir) / sizeof(TCHAR) - 1;

    return (int) (lpDest - lpDestOrig);
}

WCHAR PASCAL
hangul_cho_to_jamo(WCHAR key)
{
#define ______ 0x0
    static const WCHAR table[] = {
        0x1100,
        0x1101,
        ______,
        0x1102,
        ______,
        ______,
        0x1103,
        0x1104,
        0x1105,
        ______,
        ______,
        ______,
        ______,
        ______,
        ______,
        ______,
        0x1106,
        0x1107,
        0x1108,
        ______,
        0x1109,
        0x110a,
        0x110b,
        0x110c,
        0x110d,
        0x110e,
        0x110f,
        0x1110,
        0x1111,
        0x1112
    };
    if (0x3131 <= key && key <= 0x314e) {
        return table[ key - 0x3131];
    }
    return 0;
}

// terminate the text of a read of 2048 bytes at lpText, without a byte of
// half a char; returns its end if its last line is cut by the read, NULL
// if the read ended a line or the file
static LPMYSTR EndOfRead(LPMYSTR lpText, DWORD dwRead)
{
    LPMYSTR lpEnd = lpText + dwRead / sizeof(MYCHAR);

    *lpEnd = MYTEXT('\0');
    if (dwRead < 2048 || *(lpEnd - 1) == MYTEXT('\n'))
        return NULL;
    return lpEnd;
}

int CopyCandidateStringsFromDictionary(HANDLE hFile, LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen, int iFlag)
{
    DWORD dwWritten = 0;
    MYCHAR  myBuf[4098];
    LPMYSTR lpDic, lpSection, lpTemp;
    WCHAR *save;
    MYCHAR  myBase[128];
    LPMYSTR lpReadBase;
    // CR too: no empty line of CR LF, no CR at the end of a line
    const LPMYSTR szSep = MYTEXT("\r\n");
    LPMYSTR lpToken = NULL, lpCut;
    DWORD llen = 0, dwRead;
    int found = 0;
    LPMYSTR lpWrite = lpBuf;
    int olen = Mylstrlen(lpRead);
    MYCHAR r_from = 0, r_to = 0;

    lpReadBase = myBase;
    //
    // make a code range to match hangul words
    // check 자유(ㄷ) or 자유(도)
    //
    while (iFlag && olen > 1 && olen < 128) {
        MYCHAR last = *(lpRead + olen - 1);

        if (0x3131 <= last && last <= 0x314e) {
            // check choseong
            last = hangul_cho_to_jamo(last);
            if (!last)
                break;
        } else if (hangul_is_syllable(last)) {
            // check syllable without jongseong
            WCHAR cho, jung, jong;
            hangul_syllable_to_jamo(last, &cho, &jung, &jong);
            if (!jong) {
                // make range from cho-jung to cho-jung-HIEUH
                r_from = last;
                r_to = hangul_jamo_to_syllable(cho, jung, 0x11c2);
                Mylstrcpyn(lpReadBase, lpRead, olen);
                *(lpReadBase + olen - 1) = MYTEXT('\0');
                break;
            }
        }

        if (0x1100 <= last && last <= 0x1112) {
            // make range from cho-A to cho-I-HIEUH
            r_from = hangul_jamo_to_syllable(last, 0x1161,0);
            r_to = hangul_jamo_to_syllable(last, 0x1175, 0x11c2);
            MyDebugPrint((TEXT(">>>> Range from=%c, to=%c\n"), r_from, r_to));

            Mylstrcpyn(lpReadBase, lpRead, olen);
            *(lpReadBase + olen - 1) = MYTEXT('\0');
        }
        break;
    }

    myBuf[0] = MYTEXT('\0');
    lpDic = myBuf;

    // a line carried over at the end of the file is read once more
    while (ReadFile(hFile, lpDic + llen, 2048, &dwRead, NULL) && (dwRead > 0 || llen > 0))
    {
        lpCut = EndOfRead(lpDic + llen, dwRead);

        lpToken = Mystrtok(lpDic, szSep, &save);
        while (NULL != lpToken) {
            // read it again with the next part, a header may be cut
            if (lpToken + Mylstrlen(lpToken) == lpCut)
                break;
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
                    // the sections are sorted by their first char
                    found = -1; // not found
                    break;
                } else if ( *(lpSection) == *(lpRead) && iFlag) {
                    int nlen = Mylstrlen(lpSection);
                    if (nlen > olen && olen > 1 && olen < 128) {
                        MYCHAR tmp[128];
                        Mylstrcpyn(tmp, lpSection, olen + 1);
                        tmp[olen] = MYTEXT('\0');
                        if (0 == Mylstrcmp(lpRead, tmp)) {
                            found = 1;
                            break;
                        } else if (r_from) {
                            MYCHAR next = lpSection[olen - 1];
                            tmp[olen - 1] = MYTEXT('\0');
                            if (0 == Mylstrcmp(lpReadBase, tmp) && next >= r_from && next <= r_to) {
                                found = 1;
                                break;
                            }
                        }
                    }
                }
            }
            lpToken = Mystrtok(NULL, szSep, &save);
        }
        if (found < 0)
            break;
        // the cut line, or the text after the section found
        if (found)
            lpToken = Mystrtok(NULL, MYTEXT(""), &save);
        llen = Mylstrlen(lpToken);
        // not a header that long, and room for the next read
        if (!found && llen >= 2048 / sizeof(MYCHAR))
            llen = 0;
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

    // the carried-over text has to leave room for the next read and its
    // terminator, a line longer than the reads makes it grow
    while (llen + 2048 / sizeof(MYCHAR) < ARRAYSIZE(myBuf) &&
            ReadFile(hFile, lpDic + llen, 2048, &dwRead, NULL)) {
        DWORD dwW;
        DWORD len;
        UINT force = 0;

        lpCut = EndOfRead(lpDic + llen, dwRead);

        lpToken = Mystrtok(lpDic, szSep, &save);

        while ((NULL != lpToken) &&
                (lpWrite == NULL || (dwBufLen - dwWritten) > 1))
        {
            INT isasc, skip;
            isasc=FALSE;
            skip=FALSE;

            // read it again with the next part
            if (lpToken + Mylstrlen(lpToken) == lpCut)
                break;

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
                        } else if (r_from) {
                            MYCHAR next = lpSection[olen - 1];
                            buf[olen - 1] = MYTEXT('\0');
                            if (0 == Mylstrcmp(lpReadBase, buf) && next >= r_from && next <= r_to) {
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
            // 뜻이 있으면 (the CR at the end of a line is a separator now)
            if ( !skip && lpTemp && Mylstrlen(lpTemp+1) > 0) {
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

            lpToken = Mystrtok(NULL, szSep, &save);
        }

        if ((lpToken != NULL && (iFlag == 0 && MYTEXT('[') == *lpToken)) ||
                (lpWrite != NULL && (dwBufLen - dwWritten <= 1)) || force || !dwRead) {
            MyDebugPrint((TEXT(">>>>Wow lpToken:%c, force=%d, dwRead=%d, dwBuf - dwWrite=%d\n"), *lpToken, force, dwRead, dwBufLen - dwWritten));
            break;
        }

        // the cut line
        llen = Mylstrlen(lpToken);
        if (llen > 0)
            Mylstrcpy(lpDic, lpToken); // copy last
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
