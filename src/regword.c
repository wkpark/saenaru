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
 * $Saenaru: saenaru/src/regword.c,v 1.2 2003/12/26 08:28:43 perky Exp $
 */

/**********************************************************************/
#include "windows.h"
#include "immdev.h"
#include "saenaru.h"


#define FAKEWORD_NOUN  IME_REGWORD_STYLE_USER_FIRST
#define FAKEWORD_VERB  (IME_REGWORD_STYLE_USER_FIRST+1)

BOOL    WINAPI ImeRegisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
    if ((dw == FAKEWORD_NOUN) || (dw== FAKEWORD_VERB))
        return WritePrivateProfileString(lpRead,lpStr,lpStr,szDicFileName);

    return FALSE;
}
BOOL    WINAPI ImeUnregisterWord(LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr)
{
    if ((dw == FAKEWORD_NOUN) || (dw== FAKEWORD_VERB))

        return WritePrivateProfileString(lpRead,lpStr,NULL,szDicFileName);

    return FALSE;
}
UINT    WINAPI ImeGetRegisterWordStyle(UINT u, LPSTYLEBUF lp)
{
    UINT uRet = 0;

    if (u > 0 && lp)
    {
        lp->dwStyle = FAKEWORD_NOUN; 
        lstrcpy(lp->szDescription,TEXT("NOUN"));
   
        if (u > 1)
        {
            lp++;
            lp->dwStyle = FAKEWORD_VERB; 
            lstrcpy(lp->szDescription,TEXT("VERB"));
        }
    }
    else
        uRet = 2;

    return uRet;
}
UINT    WINAPI ImeEnumRegisterWord(REGISTERWORDENUMPROC lpfn, LPCTSTR lpRead, DWORD dw, LPCTSTR lpStr, LPVOID lpData)
{
    UINT uRet = 0;
    char szBuf[256];
    int nBufLen;
    LPTSTR lpBuf;

    if (! lpfn)
        return 0;

    lpBuf = (LPTSTR)szBuf;

    if (!dw || (dw == FAKEWORD_NOUN))
    {
        if (lpRead)
        {
            nBufLen = GetPrivateProfileString(lpRead, NULL,(LPTSTR)"",
                            (LPTSTR)szBuf,sizeof(szBuf),(LPTSTR)szDicFileName );

            if (nBufLen)
            {
                while (*lpBuf)
                {
                    if (lpStr && lstrcmp(lpStr, lpBuf))
                        continue;

                    uRet = (*lpfn)(lpRead, dw, lpBuf, lpData);
                    lpBuf += (lstrlen(lpBuf) + 1);

                    if (!uRet)
                        break;
                }
            }
        }
        else
        {
        }
    }

    return uRet;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
