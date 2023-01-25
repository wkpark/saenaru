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
 */
#include <windows.h>

#ifdef DEBUG
extern DWORD dwLogFlag;
extern DWORD dwDebugFlag;
#ifndef DEBF_THREADID
#define DEBF_THREADID       0x00000001
#endif
#endif

extern DWORD dwOptionFlag;
extern DWORD dwLayoutFlag;
extern DWORD dwScanCodeBased;
extern DWORD dwToggleKey;
extern DWORD dwImeFlag;

DWORD PASCAL GetDwordFromSetting(LPTSTR lpszKey);

void PASCAL SetGlobalFlags()
{
    DWORD tmp;
#ifdef DEBUG
    dwLogFlag = GetDwordFromSetting(TEXT("LogFlag"));
    dwDebugFlag = GetDwordFromSetting(TEXT("DebugFlag"));
#endif

    dwOptionFlag = GetDwordFromSetting(TEXT("OptionFlag"));
    dwLayoutFlag = GetDwordFromSetting(TEXT("LayoutFlag"));

    // get ScanCode based setting.
    dwScanCodeBased = GetDwordFromSetting(TEXT("ScanCodeBased"));

    // HangulToggle Key
    dwToggleKey = GetDwordFromSetting(TEXT("HangulToggle"));

    tmp = GetDwordFromSetting(TEXT("ImeFlag"));
    if (tmp) {
        dwImeFlag = tmp;
    }
}

#ifdef DEBUG
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
