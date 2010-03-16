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
 * $Saenaru: saenaru/src/toascii.c,v 1.2 2003/12/26 08:28:43 perky Exp $
 */

/**********************************************************************/
#include "windows.h"
#include "immdev.h"
#include "saenaru.h"

/**********************************************************************/
/*      ImeToAsciiEx                                                  */
/*                                                                    */
/*    HIBYTE of uVirtKey is char code now.                            */
/**********************************************************************/
UINT WINAPI ImeToAsciiEx (UINT uVKey,UINT uScanCode,CONST LPBYTE lpbKeyState,LPTRANSMSGLIST lpTransBuf,UINT fuState,HIMC hIMC)
{
    LPARAM lParam;
    LPINPUTCONTEXT lpIMC;
    BOOL fOpen;
    DWORD vkey = LOWORD(uVKey) & 0x00FF;

    ImeLog(LOGF_KEY | LOGF_API, TEXT("ImeToAsciiEx"));

    lpCurTransKey = lpTransBuf;
    lParam = ((DWORD)uScanCode << 16) + 1L;
    
    // Init uNumTransKey here.
    uNumTransKey = 0;

    // if hIMC is NULL, this means DISABLE IME.
    if (!hIMC)
        return 0;

    if (!(lpIMC = ImmLockIMC(hIMC)))
        return 0;

    fOpen = lpIMC->fOpen;

    ImmUnlockIMC(hIMC);

    // The current status of IME is "closed".
    if (!fOpen && vkey != VK_BACK)
        goto itae_exit;

    if (uScanCode & 0x8000)
        IMEKeyupHandler( hIMC, uVKey, lParam, lpbKeyState);
    else
        IMEKeydownHandler( hIMC, uVKey, lParam, lpbKeyState);

    // Clear static value, no more generated message!
    lpCurTransKey = NULL;

itae_exit:

    // If trans key buffer that is allocated by USER.EXE full up,
    // the return value is the negative number.
    if (fOverTransKey)
    {
#ifdef DEBUG
OutputDebugString((LPTSTR)TEXT("***************************************\r\n"));
OutputDebugString((LPTSTR)TEXT("*   TransKey OVER FLOW Messages!!!    *\r\n"));
OutputDebugString((LPTSTR)TEXT("*                by SAENARU.DLL       *\r\n"));
OutputDebugString((LPTSTR)TEXT("***************************************\r\n"));
#endif
        return (int)uNumTransKey;
    }

    return (int)uNumTransKey;
}


/**********************************************************************/
/*      GenerateMessageToTransKey()                                   */
/*                                                                    */
/*      Update the transrate key buffer.                              */
/**********************************************************************/
BOOL PASCAL GenerateMessageToTransKey(LPTRANSMSGLIST lpTransBuf,LPTRANSMSG lpGeneMsg)
{
    LPTRANSMSG lpgmT0;

    uNumTransKey++;
    if (uNumTransKey >= lpTransBuf->uMsgCount)
    {
        fOverTransKey = TRUE;
        return FALSE;
    }

    lpgmT0= lpTransBuf->TransMsg + (uNumTransKey - 1);
    *lpgmT0= *lpGeneMsg;

    return TRUE;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
