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
 * $Saenaru: saenaru/src/input.c,v 1.2 2003/12/26 08:28:43 perky Exp $
 */

/**********************************************************************/
#include "windows.h"
#include "immdev.h"
#include "saenaru.h"

/**********************************************************************/
/*                                                                    */
/* IMEKeydownHandler()                                                */
/*                                                                    */
/* A function which handles WM_IMEKEYDOWN                             */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL IMEKeydownHandler( hIMC, wParam, lParam,lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    WORD wVKey;


    switch( wVKey = ( LOWORD(wParam) & 0x00FF ) ){
        case VK_SHIFT:
        case VK_CONTROL:
            //goto not_proccessed;
            break;
            /*
        case VK_ESCAPE:
        case VK_LEFT:
        case VK_RIGHT:
            break;
            */

        default:
            if( !DicKeydownHandler( hIMC, wVKey, lParam, lpbKeyState ) ) {
                TCHAR szDev[80];

                wsprintf(szDev, TEXT("Vkey: %x\r\n"), wVKey);
                OutputDebugString(szDev);
                // This WM_IMEKEYDOWN has actual character code in itself.
                hangulKeyHandler( hIMC, wParam, lParam, lpbKeyState );
                //AddChar( hIMC,  HIWORD(wParam));
                //CharHandler( hIMC,  (WORD)((BYTE)HIBYTE(wParam)), lParam );
            }
            break;
    }
    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/* IMEKeyupHandler()                                                  */
/*                                                                    */
/* A function which handles WM_IMEKEYUP                               */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL IMEKeyupHandler( hIMC, wParam, lParam ,lpbKeyState)
HIMC hIMC;
WPARAM wParam;
LPARAM lParam;
LPBYTE lpbKeyState;
{
    return FALSE;
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
