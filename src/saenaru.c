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
 * $Saenaru: saenaru/src/saenaru.c,v 1.2 2003/12/26 08:28:43 perky Exp $
 */

#include <windows.h>
#include "immdev.h"
#include "saenaru.h"
#include "resource.h"
#include "immsec.h"

extern HANDLE hMutex;
/**********************************************************************/
/*    DLLEntry()                                                      */
/**********************************************************************/
BOOL WINAPI DLLEntry (
    HINSTANCE    hInstDLL,
    DWORD        dwFunction,
    LPVOID       lpNot)
{
    LPTSTR lpDicFileName;
#ifdef DEBUG
    TCHAR szDev[80];
#endif
    MyDebugPrint((TEXT("DLLEntry:dwFunc=%d\n"),dwFunction));

    switch(dwFunction)
    {
        PSECURITY_ATTRIBUTES psa;

        case DLL_PROCESS_ATTACH:
            //
            // Create/open a system global named mutex.
            // The initial ownership is not needed.
            // CreateSecurityAttributes() will create
            // the proper security attribute for IME.
            //
#if 1
            psa = CreateSecurityAttributes();
            if ( psa != NULL ) {
                 hMutex = CreateMutex( psa, FALSE, TEXT("SaeNaru_Mutex"));
                 FreeSecurityAttributes( psa );
                 if ( hMutex == NULL ) {
                 // Failed
                 }
            }
            else {
                  // Failed, not NT system
            }
#endif

            hInst= hInstDLL;
            IMERegisterClass( hInst );

            // Initialize for SAENARU.
            lpDicFileName = (LPTSTR)&szDicFileName;
            lpDicFileName += GetWindowsDirectory(lpDicFileName,256);
            if (*(lpDicFileName-1) != TEXT('\\'))
                *lpDicFileName++ = TEXT('\\');
            LoadString( hInst, IDS_DICFILENAME, lpDicFileName, 128);

            // read registry
            SetGlobalFlags();
            // set keyboard layout
            set_keyboard(dwLayoutFlag);

#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Process Attach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_PROCESS_DETACH:
            UnregisterClass(szUIClassName,hInst);
            UnregisterClass(szCompStrClassName,hInst);
            UnregisterClass(szCandClassName,hInst);
            UnregisterClass(szStatusClassName,hInst);
            if (hMutex)
                CloseHandle( hMutex );
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Process Detach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_THREAD_ATTACH:
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Thread Attach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;

        case DLL_THREAD_DETACH:
#ifdef DEBUG
            wsprintf(szDev,TEXT("DLLEntry Thread Detach hInst is %lx"),hInst);
            ImeLog(LOGF_ENTRY, szDev);
#endif
            break;
    }
    return TRUE;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */