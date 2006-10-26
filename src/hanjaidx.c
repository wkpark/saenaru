/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 1990-1998 Microsoft Corporation.
 * Copyright (c) 2006 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2006 Won-Kyu Park <wkpark@kldp.org>.
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
 * $Saenaru: saenaru/src/hanjaidx.c,v 1.1 2006/10/24 11:32:49 wkpark Exp $
 */

#include <windows.h>
#include <immdev.h>
#include "saenaru.h"
#include "immsec.h"

#define IDX_COL	32
#define UNIHAN_UNI 20902
#define UNIHAN_EXTA 6582
#define UNIHAN_COMP 301
#define CRLF 4

UINT PASCAL
hanja_idx(UINT code)
{
    UINT offset=0;

    UINT numUni=UNIHAN_UNI;
    UINT numExta=UNIHAN_EXTA;
    int tmp;

    tmp = numUni % IDX_COL;
    numUni += tmp ? (IDX_COL - tmp):0;
    tmp = numExta % IDX_COL;
    numExta += tmp ? (IDX_COL - tmp):0;

    if (code < 0x3400 || code >=0xfaff) return -1;

    if (code >= 0xf900) {
    	offset= code - 0xf900;
	offset += numUni + numExta;
    } else if (code >= 0x4e00) {
    	offset= code - 0x4e00;
	offset += numExta;
    } else /* if (code >= 0x3400) */ {
    	offset= code - 0x3400;
    }

    tmp= offset / IDX_COL;
    offset*=2; /* 2-bytes */
    offset += tmp ? tmp*CRLF:0;
    return offset;
}

int GetHangulFromHanjaIndex(LPMYSTR lpRead, LPMYSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename)
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

    if (dwBufLen >=1) 
    {
        MYCHAR szBuf[4];
        int hsz=0;
        UINT offset;

        lpDic=szBuf;

        if (!ReadFile(hTblFile, lpDic, 2, &dwRead, NULL))
            goto Err0;
        if (*lpDic != 0xfeff)
            goto Err1;// bom ?

        if (ReadFile(hTblFile, lpDic, 2, &dwRead, NULL))
        {
            if (*lpDic == MYTEXT('#')) { // read some header
                hsz+=dwRead;
                while (ReadFile(hTblFile, lpDic, 2, &dwRead, NULL)) {
                    hsz+=dwRead;
                    if (dwRead == 2 && *lpDic == MYTEXT('\n'))
                        break;
                }
            }
        }
        if (!hsz)
            SetFilePointer(hTblFile, 2, NULL, FILE_BEGIN);

        offset=hanja_idx(*lpRead);

        SetFilePointer(hTblFile, offset, NULL, FILE_CURRENT);
        if (ReadFile(hTblFile, lpDic, 2, &dwRead, NULL)) {
            *(LPWSTR)(((LPBYTE)lpDic) + 2) = MYTEXT('\0');
            MyDebugPrint((TEXT(" *** Hangul= %x\n"),*lpDic));
            *lpBuf=*lpDic;
            nSize=2;
        }
    }

Err1:
    CloseHandle(hTblFile);

Err0:
    FreeSecurityAttributes(psa);
    return nSize;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
