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
 * $Saenaru: saenaru/src/fdebug.c,v 1.4 2003/12/26 09:32:51 perky Exp $
 */

#include <windows.h>
#include "immdev.h"
#include "saenaru.h"

#ifdef DEBUG

#if   UNICODE
const LPTSTR g_szRegInfoPath = TEXT("software\\OpenHangulProject\\Saenaru");
#else
const LPTSTR g_szRegInfoPath = TEXT("software\\OpenHangulProject\\Saenaru");
#endif

int DebugPrint(LPCTSTR lpszFormat, ...)
{
    int nCount;
    TCHAR szMsg[1024];

    va_list marker;
    va_start(marker, lpszFormat);
    nCount = wvsprintf(szMsg, lpszFormat, marker);
    va_end(marker);
    OutputDebugString(szMsg);
    return nCount;
}

DWORD PASCAL GetDwordFromSetting(LPTSTR lpszFlag)
{
    HKEY hkey;
    DWORD dwRegType, dwData, dwDataSize, dwRet;

    dwData = 0;
    dwDataSize=sizeof(DWORD);
    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegInfoPath, 0, KEY_READ, &hkey)) {
        dwRet = RegQueryValueEx(hkey, lpszFlag, NULL, &dwRegType, (LPBYTE)&dwData, &dwDataSize);
        RegCloseKey(hkey);
    }
    MyDebugPrint((TEXT("Getting: %s=%#8.8x: dwRet=%#8.8x\n"), lpszFlag, dwData, dwRet));
    return dwData;
}

void SetDwordToSetting(LPCTSTR lpszFlag, DWORD dwFlag)
{
    HKEY hkey;
    DWORD dwDataSize, dwRet;

    if (ERROR_SUCCESS == RegOpenKeyEx(HKEY_CURRENT_USER, g_szRegInfoPath, 0, KEY_WRITE, &hkey)) {
        dwRet = RegSetValueEx(hkey, lpszFlag, 0, REG_DWORD, (CONST BYTE *) &dwFlag, sizeof(DWORD));
        RegCloseKey(hkey);
    }
    MyDebugPrint((TEXT("Setting: %s=%#8.8x: dwRet=%#8.8x\n"), lpszFlag, dwFlag, dwRet));
}

void PASCAL SetGlobalFlags()
{
    dwLogFlag = GetDwordFromSetting(TEXT("LogFlag"));
    dwDebugFlag = GetDwordFromSetting(TEXT("DebugFlag"));
}

void PASCAL ImeLog(DWORD dwFlag, LPTSTR lpStr)
{
    TCHAR szBuf[80];

    if (dwFlag & dwLogFlag)
    {
        if (dwDebugFlag & DEBF_THREADID)
        {
            DWORD dwThreadId = GetCurrentThreadId();
            wsprintf(szBuf, TEXT("ThreadID = %X "), dwThreadId);
            OutputDebugString(szBuf);
        }

        OutputDebugString(lpStr);
        OutputDebugString(TEXT("\r\n"));
    }
}


#endif //DEBUG

/*
 * ex: ts=8 sts=4 sw=4 et
 */
