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

/**********************************************************************/
#include <windows.h>
#include <immdev.h>
#include "saenaru.h"
#include "hangul.h"
#include "vksub.h"

void PASCAL ShowUIWindows(HWND hWnd, BOOL fFlag);

void PASCAL UpdateSoftKeyboard(LPUIEXTRA, int);
#ifdef DEBUG
void PASCAL DumpUIExtra(LPUIEXTRA lpUIExtra);
#endif

LRESULT CALLBACK SAENARUKbdProc(int, WPARAM, LPARAM);
LRESULT CALLBACK SAENARUConKbdProc(int, WPARAM, LPARAM);
BOOL WINAPI SetHookFunc(void);
BOOL WINAPI UnsetHookFunc(void);
BOOL WINAPI SetConsoleHookFunc(void);
BOOL WINAPI UnsetConsoleHookFunc(void);
void PASCAL DvorakKey(BOOL,WPARAM, LPARAM);

static HHOOK hHookWnd = 0;
static HHOOK hConsoleHookID = 0;

#if 0
void PASCAL HideGuideLine (LPUIEXTRA lpUIExtra);
void PASCAL CreateGuideLine (HWND, LPUIEXTRA, LPINPUTCONTEXT);
#endif

#define CS_SAENARU (CS_VREDRAW | CS_HREDRAW | CS_DBLCLKS | CS_IME)
/**********************************************************************/
/*                                                                    */
/* IMERegisterClass()                                                 */
/*                                                                    */
/* This function is called by IMMInquire.                             */
/*    Register the classes for the child windows.                     */
/*    Create global GDI objects.                                      */
/*                                                                    */
/**********************************************************************/
BOOL IMERegisterClass( hInstance )
HANDLE hInstance;
{
    WNDCLASSEX wc;

    //
    // register class of UI window.
    //
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_SAENARU;
    wc.lpfnWndProc    = SAENARUWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = sizeof (LONG_PTR) * 2;
    //wc.cbWndExtra     = 8;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wc.hIcon          = NULL;
    wc.lpszMenuName   = (LPTSTR)NULL;
    wc.lpszClassName  = (LPTSTR)szUIClassName;
    wc.hbrBackground  = NULL;
    wc.hIconSm        = NULL;

    if( !RegisterClassEx( (LPWNDCLASSEX)&wc ) )
        return FALSE;

    //
    // register class of composition window.
    //
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_SAENARU;
    wc.lpfnWndProc    = CompStrWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = UIEXTRASIZE;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor( NULL, IDC_IBEAM );
    wc.hIcon          = NULL;
    wc.lpszMenuName   = (LPTSTR)NULL;
    wc.lpszClassName  = (LPTSTR)szCompStrClassName;
    //wc.hbrBackground  = NULL;
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.hIconSm        = NULL;

    if( !RegisterClassEx( (LPWNDCLASSEX)&wc ) )
        return FALSE;

    //
    // register class of candidate window.
    //
    //
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_SAENARU|CS_DROPSHADOW;
    wc.lpfnWndProc    = CandWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = UIEXTRASIZE;
    wc.hInstance      = hInstance;
#if (WINVER >= 0x0410)
    wc.hCursor        = LoadCursor( NULL, IDC_HAND );
#else
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
#endif
    wc.hIcon          = NULL;
    wc.lpszMenuName   = (LPTSTR)NULL;
    wc.lpszClassName  = (LPTSTR)szCandClassName;
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    //wc.hbrBackground  = GetStockObject(LTGRAY_BRUSH);
    wc.hIconSm        = NULL;

    if( !RegisterClassEx( (LPWNDCLASSEX)&wc ) )
        return FALSE;

    //
    // register class of status window.
    //
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_SAENARU;
    wc.lpfnWndProc    = StatusWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = UIEXTRASIZE;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wc.hIcon          = NULL;
    wc.lpszMenuName   = (LPTSTR)NULL;
    wc.lpszClassName  = (LPTSTR)szStatusClassName;
    wc.hbrBackground  = NULL;
    wc.hbrBackground  = (HBRUSH)(COLOR_BTNFACE + 1);
    //wc.hbrBackground  = GetStockObject(LTGRAY_BRUSH);
    wc.hIconSm        = NULL;

    if( !RegisterClassEx( (LPWNDCLASSEX)&wc ) )
        return FALSE;
#if 1
    //
    // register class of guideline window.
    //
    wc.cbSize         = sizeof(WNDCLASSEX);
    wc.style          = CS_SAENARU;
    wc.lpfnWndProc    = GuideWndProc;
    wc.cbClsExtra     = 0;
    wc.cbWndExtra     = UIEXTRASIZE;
    wc.hInstance      = hInstance;
    wc.hCursor        = LoadCursor( NULL, IDC_ARROW );
    wc.hIcon          = NULL;
    wc.lpszMenuName   = (LPTSTR)NULL;
    wc.lpszClassName  = (LPTSTR)szGuideClassName;
    wc.hbrBackground  = NULL;
    //wc.hbrBackground  = GetStockObject(LTGRAY_BRUSH);
    wc.hIconSm        = NULL;

    if( !RegisterClassEx( (LPWNDCLASSEX)&wc ) )
        return FALSE;
#endif

    hangul_ic_init(&ic); // init hangul

    return TRUE;
}

void caretTimerProc( hWnd, uMsg, idEvent, dwTime)
HWND hWnd;
UINT uMsg;
UINT_PTR idEvent;
DWORD dwTime;
{
    PostMessage(hWnd, WM_IME_NOTIFY, IMN_PRIVATE, WM_UI_CARET);
}

/**********************************************************************/
/*                                                                    */
/* SAENARUWndProc()                                                   */
/*                                                                    */
/* IME UI window procedure                                            */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK SAENARUWndProc( hWnd, message, wParam, lParam )
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    HIMC           hUICurIMC;
    LPINPUTCONTEXT lpIMC;
    LPUIEXTRA      lpUIExtra;
    HGLOBAL        hUIExtra;
    LONG           lRet = 0L;
    int            i;

    hUICurIMC = (HIMC)GetWindowLongPtr(hWnd,IMMGWLP_IMC);

    //
    // Even if there is no current UI. these messages should not be pass to 
    // DefWindowProc().
    //
    if (!hUICurIMC)
    {
        switch (message)
        {
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
            case WM_IME_COMPOSITION:
            case WM_IME_NOTIFY:
            case WM_IME_CONTROL:
            case WM_IME_COMPOSITIONFULL:
            case WM_IME_SELECT:
            case WM_IME_CHAR:
#ifdef DEBUG
                {
                TCHAR szDev[80];
                OutputDebugString((LPTSTR)TEXT("Why hUICurIMC is NULL????\r\n"));
                wsprintf((LPTSTR)szDev,TEXT("\thWnd is %x\r\n"),hWnd);
                OutputDebugString((LPTSTR)szDev);
                wsprintf((LPTSTR)szDev,TEXT("\tmessage is %x\r\n"),message);
                OutputDebugString((LPTSTR)szDev);
                wsprintf((LPTSTR)szDev,TEXT("\twParam is hi=0x%x,lo=0x%x\r\n"),HIWORD(wParam), LOWORD(wParam));
                OutputDebugString((LPTSTR)szDev);
                wsprintf((LPTSTR)szDev,TEXT("\tlParam is %lx\r\n"),lParam);
                OutputDebugString((LPTSTR)szDev);
                }
#endif
                return 0L;
            default:
                MyDebugPrint((TEXT("hUICurIMC == NULL\r\n")));
                break;
        }
    }

    MyDebugPrint((TEXT("* WM_IME_XXX %x : %d\r\n"), message, message));
    switch (message)
    {
        case WM_CREATE:
            //
            // Allocate UI's extra memory block.
            //
            hUIExtra = (HGLOBAL)GlobalAlloc(GHND,sizeof(UIEXTRA));

            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

            //
            // Initialize the extra memory block.
            //
            lpUIExtra->uiStatus.pt.x = -1;
            lpUIExtra->uiStatus.pt.y = -1;
            lpUIExtra->uiDefComp.pt.x = -1;
            lpUIExtra->uiDefComp.pt.y = -1;
            lpUIExtra->uiCand.pt.x = -1;
            lpUIExtra->uiCand.pt.y = -1;
#if 1
            lpUIExtra->uiGuide.pt.x = -1;
            lpUIExtra->uiGuide.pt.y = -1;
#endif
#if 1
            lpUIExtra->uiSoftKbd.pt.x = -1;
            lpUIExtra->uiSoftKbd.pt.y = -1;
#endif
            lpUIExtra->hFont = (HFONT)NULL;

            GlobalUnlock(hUIExtra);

            SetWindowLongPtr(hWnd,IMMGWLP_PRIVATE,(LONG_PTR)hUIExtra);

            MyDebugPrint((TEXT("WM_CREATE\n")));
            SetHookFunc();
            //SetConsoleHookFunc();
#if !defined(NO_TSF)
            ActivateLanguageBar(TRUE); // ActivateLanguageBar() again for some cases
#endif

            break;

        case WM_IME_SETCONTEXT:
            MyDebugPrint((TEXT("* WM_IME_SETCONTEXT\r\n")));
            if (wParam)
            {
                hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
                lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
                lpUIExtra->hIMC = hUICurIMC;

                if (hUICurIMC)
                {
                    LPINPUTCONTEXT lpIMCT = NULL;
                    //
                    // input context was chenged.
                    // if there are the child windows, the diplay have to be
                    // updated.
                    //
                    lpIMC = ImmLockIMC(hUICurIMC);
                    if (lpIMC)
                    {
                        LPCOMPOSITIONSTRING lpCompStr;
                        LPCANDIDATEINFO lpCandInfo;
                        lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);
                        lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                        if (IsWindow(lpUIExtra->uiCand.hWnd))
                            HideCandWindow(lpUIExtra);

                        // http://msdn.microsoft.com/en-us/library/dd374142(VS.85).aspx
                        // Is it needed for Korean IME ?
                        if (lParam & ISC_SHOWUICANDIDATEWINDOW)
                        {
                            // Saenaru always commit compStr when it lost its focus.
                            // and then the following if statement always false.
                            if (lpCandInfo->dwCount && IsConvertedCompStr(hUICurIMC))
                            {
                                // not reach :)
                                CreateCandWindow(hWnd,lpUIExtra,lpIMC );
                                ResizeCandWindow(lpUIExtra,lpIMC);
                                MoveCandWindow(hWnd,lpIMC,lpUIExtra,FALSE);
                            }
                        }

                        if (IsWindow(lpUIExtra->uiDefComp.hWnd))
                            HideCompWindow(lpUIExtra);

#if 0
                        if (lParam & ISC_SHOWUICANDIDATEWINDOW)
                        {
                            if (lpCompStr->dwCompStrLen)
                            {
                                CreateCompWindow(hWnd,lpUIExtra,lpIMC );
                                MoveCompWindow(lpUIExtra,lpIMC);
                            }
                        }
                        if (lParam & ISC_SHOWUIGUIDELINE) {
                            DWORD dwLevel, dwSize = 0;
                            if (ImmGetGuideLine (hUICurIMC, GGL_LEVEL, NULL, 0))
                            {
                                dwLevel = ImmGetGuideLine (hUICurIMC, GGL_LEVEL, NULL, 0);
                                if (dwLevel) 
                                    dwSize = ImmGetGuideLine (hUICurIMC, GGL_STRING, NULL, 0);
                            }
                            if (dwSize > 0) {
                                CreateGuideLine (hWnd, lpUIExtra, lpIMC);
                                ShowWindow (lpUIExtra->uiGuide.hWnd, SW_SHOWNOACTIVATE);
                                lpUIExtra->uiGuide.bShow = TRUE;
                            } else {
                                HideGuideLine (lpUIExtra);
                            }
                        } else {
                            HideGuideLine (lpUIExtra);
                        }
#endif

                        ImmUnlockIMCC(lpIMC->hCompStr);
                        ImmUnlockIMCC(lpIMC->hCandInfo);
                    }
                    else
                    {
                        HideCandWindow(lpUIExtra);
                        HideCompWindow(lpUIExtra);
                    }

                    if (lpUIExtra->uiStatus.bShow == FALSE) {
                        ShowWindow(lpUIExtra->uiStatus.hWnd, SW_SHOWNOACTIVATE);
                        lpUIExtra->uiStatus.bShow = TRUE;
                    }
                    UpdateStatusWindow(lpUIExtra);
                    ImmUnlockIMC(hUICurIMC);
                }
                else   // it is NULL input context.
                {
                    HideCandWindow(lpUIExtra);
                    HideCompWindow(lpUIExtra);
                }
                GlobalUnlock(hUIExtra);
            }
            else 
               ShowUIWindows(hWnd, FALSE);
            break;


        case WM_IME_STARTCOMPOSITION:
            //
            // Start composition! Ready to display the composition string.
            //
            lpIMC = ImmLockIMC(hUICurIMC);
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            //if (lpIMC && !(dwImeFlag & SAENARU_ONTHESPOT))
            CreateCompWindow(hWnd,lpUIExtra,lpIMC );

            MyDebugPrint((TEXT("WM_IME_STARTCOMPOSITION\r\n")));

            GlobalUnlock(hUIExtra);
            ImmUnlockIMC(hUICurIMC);
            break;
            // Windows XP notepad bug ??
            // 한글 IME일 경우 STARTCOMPOSITION 메시지가 제대로 전달되지
            // 않는다.

        case WM_IME_COMPOSITION:
            //
            // Update to display the composition string.
            //
            lpIMC = ImmLockIMC(hUICurIMC);
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            // WM_IME_STARTCOMPOSITION 메시지가 전달되지 않는 경우
            if (lpIMC && !(dwImeFlag & SAENARU_ONTHESPOT))
            {
                LPCOMPOSITIONSTRING lpCompStr;
                lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr);

                if (lpCompStr->dwCompStrLen &&
                    (!IsWindow(lpUIExtra->uiDefComp.hWnd) || !lpUIExtra->uiDefComp.bShow) )
                {
                    // 아래의 내용은 IME_STARTCOMPOSITION에 들어가야 한다.
                    // 그러나 몇몇 어플은 이상하게도 IME_STARTCOMPOSITION으로 조합을 시작하지 않는다. (노트패드 등등)
                    CreateCompWindow(hWnd,lpUIExtra,lpIMC );
                }

                ImmUnlockIMCC(lpIMC->hCompStr); // XXX
            }

            // Set Fake caret.
            if (lpUIExtra->uiDefComp.pt.x != -1) {
                UINT mt;

                mt = GetCaretBlinkTime();
                if (mt) {
                    // Set Fake caret.
                    SetTimer(hWnd, 1, mt, (TIMERPROC) caretTimerProc);
                }
            }
            MoveCompWindow(lpUIExtra,lpIMC);
            //
            MoveCandWindow(hWnd,lpIMC,lpUIExtra, TRUE);
            //MoveCandWindow(hWnd,lpIMC,lpUIExtra, FALSE); // NateOn flicking
            //MoveCandWindow(hWnd,lpIMC,lpUIExtra, TRUE); // EditPlus problem
            GlobalUnlock(hUIExtra);
            ImmUnlockIMC(hUICurIMC);

            break;

        case WM_IME_ENDCOMPOSITION:
            //
            // Finish to display the composition string.
            //
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

            lpIMC = ImmLockIMC(hUICurIMC);
            //hangul_ic_init(&ic);
            if (lpIMC && !(dwImeFlag & SAENARU_ONTHESPOT)) {
                KillTimer(hWnd, 1);
            }

            HideCompWindow(lpUIExtra);
            GlobalUnlock(hUIExtra);
            ImmUnlockIMC(hUICurIMC);

            break;

        case WM_IME_COMPOSITIONFULL:
            //XXX
            break;

        case WM_IME_SELECT:
            MyDebugPrint((TEXT("* WM_IME_SELECT\r\n")));
            if (wParam)
            {
                MyDebugPrint((TEXT("***** TRUE\r\n")));
                hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
                lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
                lpUIExtra->hIMC = hUICurIMC;

                //
                // Close and clear CandInfo!
                // Some IME like as the MS IME 2002 do not close the CandInfo properly.
                // Saenaru check prev CandInfo and force to close it.
                lpIMC = ImmLockIMC(hUICurIMC);
                if (lpIMC) {
                    LPCANDIDATEINFO lpCandInfo;
                    lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
                    if (lpCandInfo) {
                        if (IsCandidate(lpIMC)) {
                            TRANSMSG GnMsg;

                            ClearCandidate(lpCandInfo);
                            GnMsg.message = WM_IME_NOTIFY;
                            GnMsg.wParam = IMN_CLOSECANDIDATE;
                            GnMsg.lParam = 1;
                            GenerateMessage(hUICurIMC, lpIMC, lpCurTransKey,(LPTRANSMSG)&GnMsg);
                        }
                        ImmUnlockIMCC(lpIMC->hCandInfo);
                    }
                }

                GlobalUnlock(hUIExtra);
            }
            if (lParam)
                MyDebugPrint((TEXT("* hKL=%x\r\n"),(HANDLE)lParam));

            if (!dwScanCodeBased &&
                    ((HANDLE)lParam != LongToHandle(0xE0120412) || (HANDLE)lParam != LongToHandle(0xE0130412)))
                dwScanCodeBased = 1;
            //
            break;

        case WM_IME_CONTROL:
            lRet = ControlCommand(hUICurIMC, hWnd,message,wParam,lParam);
            break;


        case WM_IME_NOTIFY:
            lRet = NotifyCommand(hUICurIMC, hWnd,message,wParam,lParam);
            break;

        case WM_DESTROY:
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

            if (IsWindow(lpUIExtra->uiStatus.hWnd))
                DestroyWindow(lpUIExtra->uiStatus.hWnd);

            if (IsWindow(lpUIExtra->uiCand.hWnd))
                DestroyWindow(lpUIExtra->uiCand.hWnd);

            if (IsWindow(lpUIExtra->uiDefComp.hWnd))
                DestroyWindow(lpUIExtra->uiDefComp.hWnd);
#if 1
            if (IsWindow(lpUIExtra->uiSoftKbd.hWnd))
                ImmDestroySoftKeyboard(lpUIExtra->uiSoftKbd.hWnd);
#endif
            for (i = 0; i < MAXCOMPWND; i++)
            {
                if (IsWindow(lpUIExtra->uiComp[i].hWnd))
                    DestroyWindow(lpUIExtra->uiComp[i].hWnd);
            }

#if 1
            if (IsWindow(lpUIExtra->uiGuide.hWnd))
                DestroyWindow(lpUIExtra->uiGuide.hWnd);
#endif

            if (lpUIExtra->hFont)
                DeleteObject(lpUIExtra->hFont);

            GlobalUnlock(hUIExtra);
            GlobalFree(hUIExtra);

            MyDebugPrint((TEXT("WM_DELETE\n")));
            UnsetHookFunc();
#if !defined(NO_TSF)
            //ActivateLanguageBar(FALSE);
            //UninitLanguageBar();
#endif
            //UnsetConsoleHookFunc();

            break;

        case WM_UI_STATEHIDE:
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            if (IsWindow (lpUIExtra->uiStatus.hWnd))
                ShowWindow(lpUIExtra->uiStatus.hWnd, SW_HIDE);
            lpUIExtra->uiStatus.bShow= FALSE ;
            GlobalUnlock(hUIExtra);
            break;

        case WM_UI_STATEMOVE:
            //
            // Set the position of the status window to UIExtra.
            // This message is sent by the status window.
            //
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            lpUIExtra->uiStatus.pt.x = (long)LOWORD(lParam);
            lpUIExtra->uiStatus.pt.y = (long)HIWORD(lParam);

            SetDwordToSetting(TEXT("StatusPos"),(UINT)lParam);

            GlobalUnlock(hUIExtra);
            break;

        case WM_UI_DEFCOMPMOVE:
            //
            // Set the position of the composition window to UIExtra.
            // This message is sent by the composition window.
            //
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            if (!lpUIExtra->dwCompStyle)
            {
                lpUIExtra->uiDefComp.pt.x = (long)LOWORD(lParam);
                lpUIExtra->uiDefComp.pt.y = (long)HIWORD(lParam);
            }
            GlobalUnlock(hUIExtra);
            break;

        case WM_UI_CANDMOVE:
            //
            // Set the position of the candidate window to UIExtra.
            // This message is sent by the candidate window.
            //
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            lpUIExtra->uiCand.pt.x = (long)LOWORD(lParam);
            lpUIExtra->uiCand.pt.y = (long)HIWORD(lParam);

            MyDebugPrint((TEXT(" * CANDMOVE %dx%d\n"),LOWORD(lParam),HIWORD(lParam)));
            GlobalUnlock(hUIExtra);
            break;
#if 1
        case WM_UI_GUIDEMOVE:
            //
            // Set the position of the status window to UIExtra.
            // This message is sent by the status window.
            //
            hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
            lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);
            lpUIExtra->uiGuide.pt.x = (long)LOWORD(lParam);
            lpUIExtra->uiGuide.pt.y = (long)HIWORD(lParam);
            GlobalUnlock(hUIExtra);
            break;
#endif

        default:
            if (HIWORD(lParam) == WM_LBUTTONDOWN)
            {
                 if (hUICurIMC && IsCompStr(hUICurIMC))
                 MakeResultString(hUICurIMC,TRUE);
                 MyDebugPrint((TEXT("WM_LBUTTONDOWN\r\n")));
            }

            return DefWindowProc(hWnd,message,wParam,lParam);
    }

    return lRet;
}


int PASCAL GetCompFontHeight(LPUIEXTRA lpUIExtra)
{
    HDC hIC;
    HFONT hOldFont = 0;
    SIZE sz;

    hIC = CreateIC(TEXT("DISPLAY"),NULL,NULL,NULL);

    if (lpUIExtra->hFont)
        hOldFont = SelectObject(hIC,lpUIExtra->hFont);
    GetTextExtentPoint(hIC,TEXT("A"),1,&sz);

    if (hOldFont)
        SelectObject(hIC,hOldFont);

    DeleteDC(hIC);

    return sz.cy;
}

/**********************************************************************/
/*                                                                    */
/* NotifyCommand()                                                    */
/*                                                                    */
/* Handle WM_IME_NOTIFY messages.                                     */
/*                                                                    */
/**********************************************************************/
LONG PASCAL NotifyCommand(HIMC hUICurIMC, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG lRet = 0L;
    LPINPUTCONTEXT lpIMC;
    HGLOBAL hUIExtra;
    LPUIEXTRA lpUIExtra;
    RECT rc;
    LOGFONT lf;
    HWND hwndIndicate;

    if (!(lpIMC = ImmLockIMC(hUICurIMC)))
        return 0L;

    hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
    lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

    switch (wParam)
    {
        case IMN_CLOSESTATUSWINDOW:
#ifndef NO_USE_SOFTKBD
            if (IsWindow(lpUIExtra->uiSoftKbd.hWnd))
            {
                GetWindowRect(lpUIExtra->uiSoftKbd.hWnd,(LPRECT)&rc);
                lpUIExtra->uiSoftKbd.pt.x = rc.left;
                lpUIExtra->uiSoftKbd.pt.y = rc.top;
                //ShowWindow(lpUIExtra->uiSoftKbd.hWnd,SW_HIDE);
                ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_HIDE);
                lpUIExtra->uiSoftKbd.bShow = FALSE;
            }
#endif
            if (IsWindow(lpUIExtra->uiStatus.hWnd))
            {
                GetWindowRect(lpUIExtra->uiStatus.hWnd,(LPRECT)&rc);
                lpUIExtra->uiStatus.pt.x = rc.left;
                lpUIExtra->uiStatus.pt.y = rc.top;
                ShowWindow(lpUIExtra->uiStatus.hWnd,SW_HIDE);
                lpUIExtra->uiStatus.bShow = FALSE;
            }
        break;
        case IMN_OPENSTATUSWINDOW:
#ifndef NO_USE_SOFTKBD
            if (lpUIExtra->uiSoftKbd.pt.x == -1)
            {
                GetWindowRect(lpIMC->hWnd,&rc);
                lpUIExtra->uiSoftKbd.pt.x = rc.right + 1;
                lpUIExtra->uiSoftKbd.pt.y = rc.top;
            }
            if (!IsWindow(lpUIExtra->uiSoftKbd.hWnd))
            {
                lpUIExtra->uiSoftKbd.hWnd = 
                ImmCreateSoftKeyboard(SOFTKEYBOARD_TYPE_C1, hWnd,
                0, 0);
            }

            if (lpIMC->fdwConversion & IME_CMODE_SOFTKBD ) {
                UpdateSoftKeyboard(lpUIExtra, lpIMC->fdwConversion & IME_CMODE_SOFTKBD);
                ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_SHOWNOACTIVATE);
            } else
                ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_HIDE);

            /*
            ShowWindow(lpUIExtra->uiSoftKbd.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiSoftKbd.bShow = TRUE;
            SetWindowLongPtr(lpUIExtra->uiSoftKbd.hWnd,FIGWL_SVRWND,(LONG_PTR)hWnd);
            */
#endif
            if (lpUIExtra->uiStatus.pt.x == -1)
            {
                //GetWindowRect(lpIMC->hWnd,&rc);
                DWORD pos;
                pos=GetDwordFromSetting(TEXT("StatusPos"));
                if (pos) {
                    lpUIExtra->uiStatus.pt.x = pos & 0xffff;
                    lpUIExtra->uiStatus.pt.y = pos >> 16;
                } else {
                    GetWindowRect(GetDesktopWindow(),&rc);
                    lpUIExtra->uiStatus.pt.x = rc.right - 100; // XXX +1 => -100
                    lpUIExtra->uiStatus.pt.y = rc.bottom - 100;
                    pos=(lpUIExtra->uiStatus.pt.y << 16)
                        & lpUIExtra->uiStatus.pt.x;
                    SetDwordToSetting(TEXT("StatusPos"),pos);
                }
            }
#ifndef USE_STATUS_WIN98_XXX
            if (!GetDwordFromSetting(TEXT("HideStatus"))
#if !defined(NO_TSF)
                    && !IsLangBarEnabled()
#endif
                    ) {
                MyDebugPrint((TEXT("***** LangBar NOT Enabled ******\r\n")));
            if (!IsWindow(lpUIExtra->uiStatus.hWnd))
            {
                lpUIExtra->uiStatus.hWnd = 
                      CreateWindowEx(WS_EX_WINDOWEDGE,
                            (LPTSTR)szStatusClassName,NULL,
                            WS_COMPDEFAULT | WS_DLGFRAME,
                            lpUIExtra->uiStatus.pt.x,
                            lpUIExtra->uiStatus.pt.y,
                            BTX * 2 + 2 * GetSystemMetrics(SM_CXBORDER)
                                    + 2 * GetSystemMetrics(SM_CXEDGE),
                            BTX // + GetSystemMetrics(SM_CYSMCAPTION)
                                + 2 * GetSystemMetrics(SM_CYBORDER)
                                + 2 * GetSystemMetrics(SM_CYEDGE),
                            hWnd,NULL,hInst,NULL);
            }

            ShowWindow(lpUIExtra->uiStatus.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiStatus.bShow = TRUE;
            SetWindowLongPtr(lpUIExtra->uiStatus.hWnd,FIGWL_SVRWND,(LONG_PTR)hWnd);
            }
#endif
            MyDebugPrint((TEXT("IMN_OPENSTATUSWINDOW\r\n")));
            break;

        case IMN_SETCONVERSIONMODE:
            UpdateStatusWindow(lpUIExtra);
            UpdateIndicIcon(hUICurIMC);

            UpdateSoftKeyboard(lpUIExtra,
            lpIMC->fdwConversion & IME_CMODE_SOFTKBD);
#if 0
        if (!lpIMC->fdwConversion & IME_CMODE_SOFTKBD )
                ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_HIDE);
        else 
                ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_SHOWNOACTIVATE);
#endif
            break;

        case IMN_SETSENTENCEMODE:
            break;

        case IMN_SETCOMPOSITIONFONT:
            MyDebugPrint((TEXT("IMN_SETCOMPOSITIONFONT\r\n")));
            lf = lpIMC->lfFont.W;
            if (lpUIExtra->hFont)
                DeleteObject(lpUIExtra->hFont);

            if (lf.lfEscapement == 2700)
                lpUIExtra->bVertical = TRUE;
            else
            {
                lf.lfEscapement = 0;
                lpUIExtra->bVertical = FALSE;
            }

            //
            // if current font can't display Korean characters, 
            // try to find Korean font
            //
            if (lf.lfCharSet != NATIVE_CHARSET) {
                lf.lfCharSet = NATIVE_CHARSET;
                lf.lfFaceName[0] = TEXT('\0');
            }

            lpUIExtra->hFont = CreateFontIndirect((LPLOGFONT)&lf);
            SetFontCompWindow(lpUIExtra);
            MoveCompWindow(lpUIExtra,lpIMC);

            // lRet=1L; for fail

            break;

        case IMN_SETOPENSTATUS:
            MyDebugPrint((TEXT("IMN_SETOPENSTATUS\r\n")));
            UpdateStatusWindow(lpUIExtra);
            UpdateSoftKeyboard(lpUIExtra,
            lpIMC->fdwConversion & IME_CMODE_SOFTKBD);
            break;

        case IMN_OPENCANDIDATE:
            CreateCandWindow(hWnd,lpUIExtra,lpIMC );
            break;

        case IMN_CHANGECANDIDATE:
            ResizeCandWindow(lpUIExtra,lpIMC);
            MoveCandWindow(hWnd,lpIMC,lpUIExtra, FALSE);
            break;

        case IMN_CLOSECANDIDATE:
            HideCandWindow(lpUIExtra);
            break;

#if 1
        case IMN_GUIDELINE:
            if (ImmGetGuideLine(hUICurIMC,GGL_LEVEL,NULL,0))
            {
                if (!IsWindow(lpUIExtra->uiGuide.hWnd))
                {
                    HDC hdcIC;
                    TEXTMETRIC tm;
                    int dx,dy;
                    
                    if (lpUIExtra->uiGuide.pt.x == -1)
                    {
                        GetWindowRect(lpIMC->hWnd,&rc);
                        lpUIExtra->uiGuide.pt.x = rc.left;
                        lpUIExtra->uiGuide.pt.y = rc.bottom;
                    }

                    hdcIC = CreateIC(TEXT("DISPLAY"),NULL,NULL,NULL);
                    GetTextMetrics(hdcIC,&tm);
                    dx = tm.tmAveCharWidth * MAXGLCHAR;
                    dy = tm.tmHeight + tm.tmExternalLeading;
                    DeleteDC(hdcIC);
                
                    lpUIExtra->uiGuide.hWnd = 
                         CreateWindowEx( WS_EX_WINDOWEDGE | WS_EX_DLGMODALFRAME,
                                         (LPTSTR)szGuideClassName,NULL,
                                         WS_DISABLED | WS_POPUP | WS_BORDER,
                                         lpUIExtra->uiGuide.pt.x,
                                         lpUIExtra->uiGuide.pt.y,
                                         dx + 2 * GetSystemMetrics(SM_CXBORDER)
                                            + 2 * GetSystemMetrics(SM_CXEDGE),
                                         dy + GetSystemMetrics(SM_CYSMCAPTION)
                                            + 2 * GetSystemMetrics(SM_CYBORDER)
                                            + 2 * GetSystemMetrics(SM_CYEDGE),
                                         hWnd,NULL,hInst,NULL);
                }
                ShowWindow(lpUIExtra->uiGuide.hWnd,SW_SHOWNOACTIVATE);
                lpUIExtra->uiGuide.bShow = TRUE;
                SetWindowLongPtr(lpUIExtra->uiGuide.hWnd,FIGWL_SVRWND,(LONG_PTR)hWnd);
                UpdateGuideWindow(lpUIExtra);
            }
            break;
#endif

        case IMN_SETCANDIDATEPOS:
            // SAENARU supports only one candidate list.
            if (lParam != 0x01)
                break;

            MoveCandWindow(hWnd,lpIMC,lpUIExtra, FALSE);
            break;

        case IMN_SETCOMPOSITIONWINDOW:
            MoveCompWindow(lpUIExtra,lpIMC);
            MoveCandWindow(hWnd,lpIMC,lpUIExtra, TRUE);
            MyDebugPrint((TEXT("IMN_SETCOMPOSITIONWINDOW\n")));

            break;

        case IMN_SETSTATUSWINDOWPOS:
            break;

        case IMN_PRIVATE:
            if (lParam == WM_UI_CARET) {
                // Create Fake Caret
                if (lpUIExtra) {
                    int i;
                    for (i = 0; i < MAXCOMPWND; i++) {
                        if (IsWindow(lpUIExtra->uiComp[i].hWnd))
                            DrawCompCaret(lpUIExtra->uiComp[i].hWnd);
                    }
                }
            }
            break;

        default:
            break;
    }

    GlobalUnlock(hUIExtra);
    ImmUnlockIMC(hUICurIMC);

    return lRet;
}

/**********************************************************************/
/*                                                                    */
/* ControlCommand()                                                   */
/*                                                                    */
/* Handle WM_IME_CONTROL messages.                                    */
/*                                                                    */
/**********************************************************************/
#define lpcfCandForm ((LPCANDIDATEFORM)lParam)
LONG PASCAL ControlCommand(HIMC hUICurIMC, HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    LONG lRet = 1L;
    LPINPUTCONTEXT lpIMC;
    HGLOBAL hUIExtra;
    LPUIEXTRA lpUIExtra;

    if (!(lpIMC = ImmLockIMC(hUICurIMC)))
        return 1L;

    hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE);
    lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra);

    switch (wParam)
    {
        TCHAR szDev[80];
        case IMC_GETCANDIDATEPOS:
            if (IsWindow(lpUIExtra->uiCand.hWnd))
            {
                // SAENARU has only one candidate list.
                *(LPCANDIDATEFORM)lParam  = lpIMC->cfCandForm[0]; 
                lRet = 0;
            }
            break;
        case IMC_SETCANDIDATEPOS:
            MyDebugPrint((TEXT("IMC_SETCANDIDATEPOS\n")));
            break;

        case IMC_SETCOMPOSITIONFONT:
            MyDebugPrint((TEXT("SETCOMPOSITIONFONT\r\n")));

            break;

        case IMC_GETCOMPOSITIONWINDOW:
            MyDebugPrint((TEXT("GETCOMPOSITIONWINDOW\r\n")));

            *(LPCOMPOSITIONFORM)lParam  = lpIMC->cfCompForm; 
            lRet = 0;
            break;

        case IMC_GETSTATUSWINDOWPOS:
            lRet = (lpUIExtra->uiStatus.pt.y  << 16) & lpUIExtra->uiStatus.pt.x;
            break;

        case IMC_CLOSESTATUSWINDOW:
            MyDebugPrint((TEXT("IMC_CLOSESTATUSWINDOW\r\n")));
            if (IsWindow (lpUIExtra->uiStatus.hWnd))
                ShowWindow(lpUIExtra->uiStatus.hWnd, SW_HIDE);
            lpUIExtra->uiStatus.bShow= FALSE ;

            lRet = 0;
            break;
        case IMC_OPENSTATUSWINDOW:
            MyDebugPrint((TEXT("IMC_OPENSTATUSWINDOW\r\n")));
            if (IsWindow (lpUIExtra->uiStatus.hWnd))
                ShowWindow(lpUIExtra->uiStatus.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiStatus.bShow = TRUE;
            lRet = 0;
        default:
            break;
    }

    GlobalUnlock(hUIExtra);
    ImmUnlockIMC(hUICurIMC);

    return lRet;
}
/**********************************************************************/
/*                                                                    */
/* DrawUIBorder()                                                     */
/*                                                                    */
/* When draging the child window, this function draws the border.     */
/*                                                                    */
/**********************************************************************/
void PASCAL DrawUIBorder( LPRECT lprc )
{
    HDC hDC;
    int sbx, sby;

    hDC = CreateDC( TEXT("DISPLAY"), NULL, NULL, NULL );
    SelectObject( hDC, GetStockObject( GRAY_BRUSH ) );
    sbx = GetSystemMetrics( SM_CXBORDER );
    sby = GetSystemMetrics( SM_CYBORDER );
    PatBlt( hDC, lprc->left, 
                 lprc->top, 
                 lprc->right - lprc->left-sbx, 
                 sby, PATINVERT );
    PatBlt( hDC, lprc->right - sbx, 
                 lprc->top, 
                 sbx, 
                 lprc->bottom - lprc->top-sby, PATINVERT );
    PatBlt( hDC, lprc->right, 
                 lprc->bottom-sby, 
                 -(lprc->right - lprc->left-sbx), 
                 sby, PATINVERT );
    PatBlt( hDC, lprc->left, 
                 lprc->bottom, 
                 sbx, 
                 -(lprc->bottom - lprc->top-sby), PATINVERT );
    DeleteDC( hDC );
}

/**********************************************************************/
/*                                                                    */
/* DragUI(hWnd,message,wParam,lParam)                                 */
/*                                                                    */
/* Handling mouse messages for the child windows.                     */
/*                                                                    */
/**********************************************************************/
void PASCAL DragUI( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT     pt;
    static    POINT ptdif;
    static    RECT drc;
    static    RECT rc;
    DWORD  dwT;

    switch (message)
    {
        case WM_SETCURSOR:
            if ( HIWORD(lParam) == WM_RBUTTONDOWN ) 
            {
                SetCursor(LoadCursor(NULL,IDC_SIZEALL));

                GetCursorPos( &pt );
                SetCapture(hWnd);

                GetWindowRect(hWnd,&drc);
                ptdif.x = pt.x - drc.left;
                ptdif.y = pt.y - drc.top;
                rc = drc;
                rc.right -= rc.left;
                rc.bottom -= rc.top;
                SetWindowLong(hWnd,FIGWL_MOUSE,FIM_CAPUTURED);
            }
#if (WINVER >= 0x0500)
              else
                SetCursor(LoadCursor(NULL,IDC_HAND));
#endif
            break;

        case WM_MOUSEMOVE:
            dwT = GetWindowLong(hWnd,FIGWL_MOUSE);
            if (dwT & FIM_MOVED)
            {
                DrawUIBorder(&drc);
                GetCursorPos( &pt );
                drc.left   = pt.x - ptdif.x;
                drc.top    = pt.y - ptdif.y;
                drc.right  = drc.left + rc.right;
                drc.bottom = drc.top + rc.bottom;
                DrawUIBorder(&drc);
            }
            else if (dwT & FIM_CAPUTURED)
            {
                DrawUIBorder(&drc);
                SetWindowLong(hWnd,FIGWL_MOUSE,dwT | FIM_MOVED);
            }
            break;

        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            dwT = GetWindowLong(hWnd,FIGWL_MOUSE);

            if (dwT & FIM_CAPUTURED)
            {
                ReleaseCapture();
                if (dwT & FIM_MOVED)
                {
                    DrawUIBorder(&drc);
                    GetCursorPos( &pt );
                    MoveWindow(hWnd,pt.x - ptdif.x,
                                    pt.y - ptdif.y,
                                    rc.right,
                                    rc.bottom,TRUE);
                }
            }
            break;
    }
}

/**********************************************************************/
/*                                                                    */
/* MyIsIMEMessage(message)                                            */
/*                                                                    */
/* Any UI window should not pass the IME messages to DefWindowProc.   */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL MyIsIMEMessage(UINT message)
{
    switch(message)
    {
            case WM_IME_STARTCOMPOSITION:
            case WM_IME_ENDCOMPOSITION:
            case WM_IME_COMPOSITION:
            case WM_IME_NOTIFY:
            case WM_IME_SETCONTEXT:
            case WM_IME_CONTROL:
            case WM_IME_COMPOSITIONFULL:
            case WM_IME_SELECT:
            case WM_IME_CHAR:
                return TRUE;
    }

    return FALSE;
}

/**********************************************************************/
/*                                                                    */
/* ShowUIWindows(hWnd,fFlag)                                          */
/*                                                                    */
/**********************************************************************/
void PASCAL ShowUIWindows(HWND hWnd, BOOL fFlag)
{
    HGLOBAL hUIExtra;
    LPUIEXTRA lpUIExtra;
    int nsw = fFlag ? SW_SHOWNOACTIVATE : SW_HIDE;

    if (!(hUIExtra = (HGLOBAL)GetWindowLongPtr(hWnd,IMMGWLP_PRIVATE)))
        return;

    if (!(lpUIExtra = (LPUIEXTRA)GlobalLock(hUIExtra)))
        return;

    if (IsWindow(lpUIExtra->uiStatus.hWnd))
    {
        ShowWindow(lpUIExtra->uiStatus.hWnd,nsw);
        lpUIExtra->uiStatus.bShow = fFlag;
    }

    if (IsWindow(lpUIExtra->uiCand.hWnd))
    {
        ShowWindow(lpUIExtra->uiCand.hWnd,nsw);
        lpUIExtra->uiCand.bShow = fFlag;
    }

    if (IsWindow(lpUIExtra->uiDefComp.hWnd))
    {
        ShowWindow(lpUIExtra->uiDefComp.hWnd,nsw);
        lpUIExtra->uiDefComp.bShow = fFlag;
    }
#if 1
    if (IsWindow(lpUIExtra->uiGuide.hWnd))
    {
        ShowWindow(lpUIExtra->uiGuide.hWnd,nsw);
        lpUIExtra->uiGuide.bShow = fFlag;
    }
#endif 

    GlobalUnlock(hUIExtra);

}

/**********************************************************************/
/* UpdateSoftKbd()                                                    */
/**********************************************************************/
void PASCAL UpdateSoftKeyboard(
    LPUIEXTRA   lpUIExtra, int mode)
{
    LPINPUTCONTEXT lpIMC;

    if (!mode)
    {
        ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_HIDE);
    }
    else
    {
        DWORD dwbg;
        HBRUSH hbrbg;
        SetSoftKbdData(lpUIExtra->uiSoftKbd.hWnd);
        dwbg=(DWORD) GetClassLongPtr(lpUIExtra->uiSoftKbd.hWnd,GCLP_HBRBACKGROUND);
        MyDebugPrint((TEXT("\tBackground color: 0x%x\r\n"),dwbg));

        SetClassLongPtr(lpUIExtra->uiSoftKbd.hWnd,GCLP_HBRBACKGROUND,(COLOR_BTNFACE + 1));
        //hbrbg = CreateSolidBrush(GetSysColor(COLOR_BTNFACE));
        //SetClassLongPtr(lpUIExtra->uiSoftKbd.hWnd,GCL_HBRBACKGROUND,(LONG)hbrbg);
        // XXX

        //DeleteObject(hbrbg);

        ImmShowSoftKeyboard(lpUIExtra->uiSoftKbd.hWnd,SW_SHOWNOACTIVATE);
    }
    return;
}

BOOL WINAPI SetHookFunc(void)
{
    if((hHookWnd=SetWindowsHookEx(WH_GETMESSAGE, SAENARUKbdProc, (HINSTANCE) NULL, GetCurrentThreadId())) == NULL)
        return FALSE;
    return TRUE;
}

BOOL WINAPI SetConsoleHookFunc(void)
{
    if((hConsoleHookID=SetWindowsHookEx(WH_KEYBOARD_LL, SAENARUConKbdProc, (HINSTANCE) hInst, 0 )) == NULL) // all
    //if((hConsoleHookID=SetWindowsHookEx(WH_KEYBOARD_LL, SAENARUConKbdProc, (HINSTANCE) NULL,  GetCurrentThreadId() )) == NULL)
        return FALSE;
    return TRUE;
}

BOOL WINAPI UnsetHookFunc(void)
{
    if(UnhookWindowsHookEx(hHookWnd) == 0)
        return FALSE;
    return TRUE;
}

BOOL WINAPI UnsetConsoleHookFunc(void)
{
    if(UnhookWindowsHookEx(hConsoleHookID) == 0)
        return FALSE;
    return TRUE;
}

static const WCHAR qwerty2dvorak_table[] = {
    0x021,    /* GDK_exclam */
    0x05f,    /* GDK_underscore */
    0x023,    /* GDK_numbersign */
    0x024,    /* GDK_dollar */
    0x025,    /* GDK_percent */
    0x026,    /* GDK_ampersand */
    0x02d,    /* GDK_minus */
    0x028,    /* GDK_parenleft */
    0x029,    /* GDK_parenright */
    0x02a,    /* GDK_asterisk */
    0x07d,    /* GDK_braceright */
    0x077,    /* GDK_w */
    0x05b,    /* GDK_bracketleft */
    0x076,    /* GDK_v */
    0x07a,    /* GDK_z */
    0x030,    /* GDK_0 */
    0x031,    /* GDK_1 */
    0x032,    /* GDK_2 */
    0x033,    /* GDK_3 */
    0x034,    /* GDK_4 */
    0x035,    /* GDK_5 */
    0x036,    /* GDK_6 */
    0x037,    /* GDK_7 */
    0x038,    /* GDK_8 */
    0x039,    /* GDK_9 */
    0x053,    /* GDK_S */
    0x073,    /* GDK_s */
    0x057,    /* GDK_W */
    0x05d,    /* GDK_bracketright */
    0x056,    /* GDK_V */
    0x05a,    /* GDK_Z */
    0x040,    /* GDK_at */
    0x041,    /* GDK_A */
    0x058,    /* GDK_X */
    0x04a,    /* GDK_J */
    0x045,    /* GDK_E */
    0x03e,    /* GDK_greater */
    0x055,    /* GDK_U */
    0x049,    /* GDK_I */
    0x044,    /* GDK_D */
    0x043,    /* GDK_C */
    0x048,    /* GDK_H */
    0x054,    /* GDK_T */
    0x04e,    /* GDK_N */
    0x04d,    /* GDK_M */
    0x042,    /* GDK_B */
    0x052,    /* GDK_R */
    0x04c,    /* GDK_L */
    0x022,    /* GDK_quotedbl */
    0x050,    /* GDK_P */
    0x04f,    /* GDK_O */
    0x059,    /* GDK_Y */
    0x047,    /* GDK_G */
    0x04b,    /* GDK_K */
    0x03c,    /* GDK_less */
    0x051,    /* GDK_Q */
    0x046,    /* GDK_F */
    0x03a,    /* GDK_colon */
    0x02f,    /* GDK_slash */
    0x05c,    /* GDK_backslash */
    0x03d,    /* GDK_qual */
    0x05e,    /* GDK_asciicircum */
    0x07b,    /* GDK_braceleft */
    0x060,    /* GDK_grave */
    0x061,    /* GDK_a */
    0x078,    /* GDK_x */
    0x06a,    /* GDK_j */
    0x065,    /* GDK_e */
    0x02e,    /* GDK_period */
    0x075,    /* GDK_u */
    0x069,    /* GDK_i */
    0x064,    /* GDK_d */
    0x063,    /* GDK_c */
    0x068,    /* GDK_h */
    0x074,    /* GDK_t */
    0x06e,    /* GDK_n */
    0x06d,    /* GDK_m */
    0x062,    /* GDK_b */
    0x072,    /* GDK_r */
    0x06c,    /* GDK_l */
    0x027,    /* GDK_apostrophe */
    0x070,    /* GDK_p */
    0x06f,    /* GDK_o */
    0x079,    /* GDK_y */
    0x067,    /* GDK_g */
    0x06b,    /* GDK_k */
    0x02c,    /* GDK_comma */
    0x071,    /* GDK_q */
    0x066,    /* GDK_f */
    0x03b,    /* GDK_semicolon */
    0x03f,    /* GDK_question */
    0x07c,    /* GDK_bar */
    0x02b,    /* GDK_plus */
    0x07e,    /* GDK_asciitilde */
};

LRESULT CALLBACK SAENARUKbdProc(int code, WPARAM wParam, LPARAM lParam)
{
    BYTE pbKeyState [256];
    LPMSG lpmsg;
    UINT vKey;
    SHORT ALT;
    HKL hcur;
    BOOL nocheck = FALSE;
    BOOL dvorak = FALSE;
    BOOL fDvorak = dwOptionFlag & DVORAK_SUPPORT;

    if (code < 0)
        return CallNextHookEx(hHookWnd, code, wParam, lParam);

    /* check dvorak layout */
    /* XXX */
    hcur= GetKeyboardLayout(0);
    if (hcur != LongToHandle(0xE0120412))
        fDvorak = FALSE; // do not convert. It use it's own kbd driver

    GetKeyboardState((LPBYTE)&pbKeyState);
    nocheck = pbKeyState[VK_CONTROL] & 0x80 || pbKeyState[VK_MENU] & 0x80;

    if (fDvorak) {
        HWND hwnd = GetFocus ();
        
        dvorak = TRUE;

        if (hwnd != NULL) {
            HIMC hIMC = NULL;
            LPINPUTCONTEXT lpIMC;

            hIMC = ImmGetContext (hwnd);
            lpIMC = ImmLockIMC(hIMC);
            if (lpIMC)
                dvorak = !(lpIMC->fdwConversion & IME_CMODE_NATIVE);
            ImmUnlockIMC(hIMC);
        }
        dvorak = nocheck || dvorak;
    }

    lpmsg = (LPMSG)lParam;
    vKey = (UINT) lpmsg->wParam;

#if DEBUG
    if (vKey == VK_PROCESSKEY) {
        MyDebugPrint((TEXT("\tMainProc: VK_PROCESSKEY and 0x%lx\r\n"),lpmsg->lParam));
    } else if (vKey == VK_MENU) {
        MyDebugPrint((TEXT("\tMainProc: VK_MENU and 0x%lx\r\n"),lpmsg->lParam));
    }
#endif

    // XXX
    // check SendMessage(hWnd, WM_TIMER, WM_IME_NOTIFY, WM_UI_CARET);
    //
    while (!(dwImeFlag & SAENARU_ONTHESPOT) &&
            lpmsg->wParam == WM_IME_NOTIFY &&
            lpmsg->message == WM_TIMER) {
        HIMC hIMC;
        LPINPUTCONTEXT lpIMC;
        HWND hwnd = GetFocus();
        MyDebugPrint((TEXT("\tWM_TIMER: wParam=%d, lParam=%ld\r\n"),
                    lpmsg->wParam, lpmsg->lParam));

        hIMC = ImmGetContext(hwnd);
        if (!hIMC) break;

        lpIMC = ImmLockIMC(hIMC);
        if (lpIMC)
            PostMessage(lpIMC->hWnd, WM_IME_NOTIFY, IMN_PRIVATE, WM_UI_CARET);

        break;
    }

    // check soft Han/Eng toggle key.
    if ( dwOptionFlag & USE_SHIFT_SPACE && (dwToggleKey & 0xFF) == vKey ) {
        switch (lpmsg->message) {
        case WM_SYSKEYDOWN:
        case WM_KEYDOWN:
        //case WM_SYSKEYUP:
        //case WM_KEYUP:
            while (1) {
                UINT ok;

                if (dwToggleKey & MASK_SHIFT) {
                    ok = 0;
                    if ((dwToggleKey & MASK_LSHIFT) && (pbKeyState[VK_LSHIFT] & 0x80)) ok = 1;
                    if ((dwToggleKey & MASK_RSHIFT) && (pbKeyState[VK_RSHIFT] & 0x80)) ok = 1;
                    if (!ok) break;
                }
                if (dwToggleKey & MASK_CTRL) {
                    ok = 0;
                    if ((dwToggleKey & MASK_LCTRL) && (pbKeyState[VK_LCONTROL] & 0x80)) ok = 1;
                    if ((dwToggleKey & MASK_RCTRL) && (pbKeyState[VK_RCONTROL] & 0x80)) ok = 1;
                    if (!ok) break;
                }
                if (dwToggleKey & MASK_ALT) {
                    ok = 0;
                    if ((dwToggleKey & MASK_LALT) && (pbKeyState[VK_LMENU] & 0x80)) ok = 1;
                    if ((dwToggleKey & MASK_RALT) && (pbKeyState[VK_RMENU] & 0x80)) ok = 1;
                    if (!ok) break;
                }

                // WM_SYSKEYXXX events can't posted to ImeProcesskey()
                // so ChangeMode() in it.
                if (lpmsg->message == WM_SYSKEYDOWN) {
                    HWND hwnd = GetFocus();
                    if (hwnd != NULL) {
                        HIMC hIMC = NULL;
                        hIMC = ImmGetContext(hwnd);
                        if (hIMC)
                            ChangeMode(hIMC, TO_CMODE_HANGUL);
                    }
                }

                // make this events as WM_KEYXXX...
                // but still ImeProcesskey() doesn't accept it.
                if (lpmsg->message > WM_KEYUP)
                    lpmsg->message-=4; // WM_SYSKEYUP - WM_KEYUP = 4. See WINUSER.H

                lpmsg->wParam=VK_HANGUL; // XXX
                //lpmsg->lParam=(0xF2<<16) | 0x41000001;
                MyDebugPrint((TEXT(">>> Fake Hangul Toggle + %lx\n"), vKey));
                break;
            }
            break;
        }
    }

    switch (lpmsg->message)
    {
        case WM_KEYUP:
            break;
        case WM_KEYDOWN:
            if ( dvorak && pbKeyState[VK_CONTROL] & 0x80 &&
                    !(dwOptionFlag & QWERTY_HOTKEY_SUPPORT) ) {
                if (vKey >= VK_OEM_1 && vKey <= VK_OEM_102) {
                    WORD ch;
                    UINT sc;
                    HKL hcur;
                    hcur= GetKeyboardLayout(0);
                    ch = (WORD)MapVirtualKeyEx(vKey,2,hcur);

                    //ToAsciiEx(vKey,sc,pbKeyState,&ch,0,hcur);
                    ch = 0xff & ch;
                MyDebugPrint((TEXT("\t** Try to translate 0x%lx 0x%x\r\n"),vKey,ch));
                    if (ch < '!' || ch > '~')
                        break;
                    lpmsg->wParam=ch;
                } else if (vKey < VK_1 || vKey > VK_Z)
                    break;
                DvorakKey(TRUE,wParam,lParam);
            }
            break;
        case WM_SYSCHAR:
            if  (dwOptionFlag & QWERTY_HOTKEY_SUPPORT)
                break;
        case WM_CHAR:
#ifndef NO_DVORAK
            if (dvorak &&
                (vKey >= '!' && vKey <= '~'))
            {
                WORD dv;
                SHORT sc;
                UINT caps = 0;

                //GetKeyboardState((LPBYTE)&pbKeyState);

                caps = pbKeyState[VK_CAPITAL];
                if (caps) {
                    if (vKey >= 'A' && vKey <= 'Z')
                        vKey += 'a' - 'A';
                    else if (vKey >= 'a' && vKey <= 'z')
                        vKey -= 'a' - 'A';
                    dv = qwerty2dvorak_table[vKey - '!'];
                    if (dv >= 'A' && dv <= 'Z')
                        dv += 'a' - 'A';
                    else if (dv >= 'a' && dv <= 'z')
                        dv -= 'a' - 'A';
                } else
                    dv = qwerty2dvorak_table[vKey - '!'];
                 
                lpmsg->wParam = dv;
                sc = VkKeyScan(dv);
                lpmsg->lParam &= ~0x00ff0000;
                lpmsg->lParam |= (sc <<16);
                pbKeyState[dv]=1;
                pbKeyState[vKey]=0;
                SetKeyboardState((LPBYTE)&pbKeyState);
            }
#endif
            break;
        case WM_SYSKEYDOWN:
        case WM_SYSKEYUP:
            //GetKeyboardState((LPBYTE)&pbKeyState);
            // commit all CompStr like as IME 2002/2003
            if ( pbKeyState[VK_MENU] & 0x80)
            {
                HWND hwnd = GetFocus ();
                if (hwnd != NULL) {
                    HIMC hIMC = NULL;
                    hIMC = ImmGetContext (hwnd);
                    if (hIMC && IsCompStr(hIMC))
                        MakeResultString(hIMC,TRUE);
                }
            }

            // hack to use RALT(VK_RMENU) as a Mode_Switch
            if ( pbKeyState[VK_RMENU] & 0x80 && !(lpmsg->lParam & 0x01000000))
            {
                WORD ch;

                ToAscii(vKey,((UINT)lpmsg->lParam | 0xff0000)>>16,pbKeyState,&ch,0);
                ch = 0xff & ch;
                if (ch < '!' || ch > '~')
                     break;
                MyDebugPrint((TEXT("RALT + %x\n"), ch));
#if 1
                lpmsg->message-=4; // WM_SYSKEYUP - WM_KEYUP = 4 See WINUSER.H
                MyDebugPrint((TEXT("RALT %lx\n"), lpmsg->lParam));

                if (FALSE && dvorak)
                 {
                    WORD dv;
                    SHORT sc;

                    dv = qwerty2dvorak_table[ch - '!'];
                    sc = VkKeyScan(dv);
                    lpmsg->wParam = sc;
                }
#endif
            }
            break;
        default:
            break;
    }

    return CallNextHookEx(hHookWnd, code, wParam, lParam);
}

LRESULT CALLBACK SAENARUConKbdProc(int nCode,WPARAM wParam, LPARAM lParam)
{
    if (nCode >= 0 && wParam == WM_KEYDOWN)
    {
#if 0
        int vkCode = Marshal.ReadInt32(lParam);
        Console.WriteLine((Keys)vkCode);
#endif
        MyDebugPrint((TEXT("Console hook\r\n")));
    }
    return CallNextHookEx(hConsoleHookID, nCode, wParam, lParam);
}

#if 1
void PASCAL DvorakKey(BOOL mode,WPARAM wParam, LPARAM lParam)
{
    BYTE pbKeyState [256];
    LPMSG lpmsg;
    WORD dv,vk;
    SHORT sc;
    UINT caps = 0;
    UINT vKey;

    lpmsg = (LPMSG)lParam;
    vKey = (UINT)lpmsg->wParam;

    GetKeyboardState((LPBYTE)&pbKeyState);

    caps = pbKeyState[VK_CAPITAL];
    if (caps) {
        if (vKey >= 'A' && vKey <= 'Z')
            vKey += 'a' - 'A';
        else if (vKey >= 'a' && vKey <= 'z')
            vKey -= 'a' - 'A';
        dv = qwerty2dvorak_table[vKey - '!'];
        if (dv >= 'A' && dv <= 'Z')
            dv += 'a' - 'A';
        else if (dv >= 'a' && dv <= 'z')
            dv -= 'a' - 'A';
    } else
        dv = qwerty2dvorak_table[vKey - '!'];

    vk = dv;
    if (mode) { // for Ctrl-X mode
        if (vk >='a' && vk <= 'z')
            vk -= 'a' - 'A';
    }
     
    lpmsg->wParam = vk;
    sc = VkKeyScan(vk);
    lpmsg->lParam &= ~0x00ff0000;
    lpmsg->lParam |= (sc <<16);
    pbKeyState[dv]=1;
    pbKeyState[vKey]=0;
    SetKeyboardState((LPBYTE)&pbKeyState);
}

#endif

#ifdef DEBUG
void PASCAL DumpUIExtra(LPUIEXTRA lpUIExtra)
{
    TCHAR szDev[80];
    int i;

    wsprintf((LPTSTR)szDev,TEXT("Status hWnd %lX  [%d,%d]\r\n"),
                                               lpUIExtra->uiStatus.hWnd,
                                               lpUIExtra->uiStatus.pt.x,
                                               lpUIExtra->uiStatus.pt.y);
    OutputDebugString((LPTSTR)szDev);

    wsprintf((LPTSTR)szDev,TEXT("Cand hWnd %lX  [%d,%d]\r\n"),
                                               lpUIExtra->uiCand.hWnd,
                                               lpUIExtra->uiCand.pt.x,
                                               lpUIExtra->uiCand.pt.y);
    OutputDebugString((LPTSTR)szDev);

    wsprintf((LPTSTR)szDev,TEXT("CompStyle hWnd %lX]\r\n"), lpUIExtra->dwCompStyle);
    OutputDebugString((LPTSTR)szDev);

    wsprintf((LPTSTR)szDev,TEXT("DefComp hWnd %lX  [%d,%d]\r\n"),
                                               lpUIExtra->uiDefComp.hWnd,
                                               lpUIExtra->uiDefComp.pt.x,
                                               lpUIExtra->uiDefComp.pt.y);
    OutputDebugString((LPTSTR)szDev);

    for (i=0;i<5;i++)
    {
        wsprintf((LPTSTR)szDev,TEXT("Comp hWnd %lX  [%d,%d]-[%d,%d]\r\n"),
                                               lpUIExtra->uiComp[i].hWnd,
                                               lpUIExtra->uiComp[i].rc.left,
                                               lpUIExtra->uiComp[i].rc.top,
                                               lpUIExtra->uiComp[i].rc.right,
                                               lpUIExtra->uiComp[i].rc.bottom);
        OutputDebugString((LPTSTR)szDev);
    }
}
#endif

/*
 * ex: ts=8 sts=4 sw=4 et
 */
