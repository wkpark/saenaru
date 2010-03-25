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
 * $Saenaru: saenaru/src/uicomp.c,v 1.9 2006/11/24 11:44:04 wkpark Exp $
 */

/**********************************************************************/
#include "windows.h"
#include "immdev.h"
#include "saenaru.h"

/**********************************************************************/
/*                                                                    */
/* CompStrWndProc()                                                   */
/*                                                                    */
/* IME UI window procedure                                            */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK CompStrWndProc( hWnd, message, wParam, lParam )
HWND   hWnd;
UINT   message;
WPARAM wParam;
LPARAM lParam;
{
    HWND hUIWnd;

    switch (message)
    {
        case WM_PAINT:
            PaintCompWindow( hWnd);
            DrawCompCaret(hWnd);
            break;

        case WM_SETCURSOR:
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
#if 1
//        Comp창은 움직일 필요가 없는 창이다. 
            DragUI(hWnd,message,wParam,lParam);
            if ((message == WM_SETCURSOR) &&
                (HIWORD(lParam) != WM_LBUTTONDOWN) &&
                (HIWORD(lParam) != WM_RBUTTONDOWN)) 
                return DefWindowProc(hWnd,message,wParam,lParam);
#endif
#if 1
            if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP))
                SetWindowLong(hWnd,FIGWL_MOUSE,0L);
#endif
            break;

        case WM_MOVE:
            hUIWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);
            if (IsWindow(hUIWnd))
                SendMessage(hUIWnd,WM_UI_DEFCOMPMOVE,wParam,lParam);
            break;

        default:
            if (!MyIsIMEMessage(message))
                return DefWindowProc(hWnd,message,wParam,lParam);
            break;
    }
    return 0;
}

/**********************************************************************/
/*                                                                    */
/* CreateCompWindow()                                                 */
/*                                                                    */
/**********************************************************************/
void PASCAL CreateCompWindow( HWND hUIWnd, LPUIEXTRA lpUIExtra,LPINPUTCONTEXT lpIMC )
{
    int i;
    RECT rc;

    MyDebugPrint((TEXT("Create Comp Wind\n")));
    lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
    for (i=0; i < MAXCOMPWND ; i++)
    {
        if (!IsWindow(lpUIExtra->uiComp[i].hWnd))
        {
            lpUIExtra->uiComp[i].hWnd = 
                CreateWindowEx(0,
                             (LPTSTR)szCompStrClassName,NULL,
                             WS_COMPDEFAULT,
                             // WS_COMPNODEFAULT,
                             0,0,1,1,
                             hUIWnd,NULL,hInst,NULL);
        }
        lpUIExtra->uiComp[i].rc.left   = 0;
        lpUIExtra->uiComp[i].rc.top    = 0;
        lpUIExtra->uiComp[i].rc.right  = 1;
        lpUIExtra->uiComp[i].rc.bottom = 1;
        SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,FIGWL_FONT,(LONG_PTR)lpUIExtra->hFont);
        SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,FIGWL_SVRWND,(LONG_PTR)hUIWnd);
        ShowWindow(lpUIExtra->uiComp[i].hWnd,SW_HIDE);
        lpUIExtra->uiComp[i].bShow = FALSE;
    }

    // 일부 어플은 CompWindow의 글꼴과 크기 및 위치를 제대로 설정하지 못한다.
    // EditPlus가 대표적인 예.
    // 그런데 이 경우에 일본어 입력기에서는 모두 제대로 나온다;;;;
    // SPY++로 추적해봐도 별로 다른 것은 없는 듯.
    if (lpIMC && lpIMC->fdwInit & INIT_LOGFONT) {
        LOGFONT fon;
        UINT ret = 0;

        MyDebugPrint((TEXT(" **** COMP INIT_LOGFONT %s:%d\n"), lpIMC->lfFont.W.lfFaceName,
                                                    lpIMC->lfFont.W.lfHeight));

        MyDebugPrint((TEXT("IMR Font Test Create Comp Wind\n")));
      
        if (ret = (DWORD) MyImmRequestMessage(lpUIExtra->hIMC, IMR_COMPOSITIONFONT, (LPARAM)&fon)) {
            MyDebugPrint((TEXT(" ***** set FONT %s:%d\n"), fon.lfFaceName,
                                                fon.lfHeight));
        } else {
            MyDebugPrint((TEXT("******** FAIL to get IMR Font Test Create Comp Wind\n")));
        }
    }

    /*
     * EditPlus는 이상하게도 CFS_CANDIDATEPOS만 세팅이 되어있다.
     * 뭐가 잘못이 있는걸까?
     */
    if (lpIMC && lpIMC->fdwInit & INIT_COMPFORM) { // XXX
        POINT pt;
        pt.x = lpIMC->cfCompForm.ptCurrentPos.x;
        pt.y = lpIMC->cfCompForm.ptCurrentPos.y;

        MyDebugPrint((TEXT(" ***** C %dx%d\n"), pt.x, pt.y));
        MyDebugPrint((TEXT(" ***** Cand %dx%d INIT\n"), lpIMC->cfCandForm[0].ptCurrentPos.x, lpIMC->cfCandForm[0].ptCurrentPos.y));
        ClientToScreen(lpIMC->hWnd, &pt);
        MyDebugPrint((TEXT(" ***** S %dx%d\n"), pt.x, pt.y));
        MyDebugPrint((TEXT(" ***** INIT_COMPFORM CreateCompWnd\n")));
        MyDebugPrint((TEXT(" ***** dwStyle %d\n"), lpIMC->cfCompForm.dwStyle));
        MyDebugPrint((TEXT(" ***** CAND dwStyle %d\n"), lpIMC->cfCandForm[0].dwStyle));
        MyDebugPrint((TEXT(" ***** fdwInit %d\n"), lpIMC->fdwInit));

        lpUIExtra->uiDefComp.pt.x = pt.x;
        lpUIExtra->uiDefComp.pt.y = pt.y;
    }

    if (lpUIExtra->uiDefComp.pt.x == -1) {
        // not created yet ?
        LPIMECHARPOSITION lpCP;
        DWORD dwSize = sizeof(IMECHARPOSITION);
        UINT ret = 0;
        POINT pt;

        MyDebugPrint((TEXT(" ******* IMR Test Create Comp Wind\n")));
        
        lpCP = (LPIMECHARPOSITION)GlobalAlloc(GPTR, dwSize);
        lpCP->dwSize = dwSize;
 
        // 워드패드 같은 경우는 IMR_QUERYCHARPOSITION을 지원한다.
        if (ret = (DWORD) MyImmRequestMessage(lpUIExtra->hIMC, IMR_QUERYCHARPOSITION, (LPARAM)lpCP)) {
            rc.left = lpCP->pt.x;
            rc.bottom = lpCP->pt.y;
            MyDebugPrint((TEXT("IMR Comp pt.x :%d\n"),rc.left));

        } else if (GetCaretPos(&pt)) {
            // GetCaretPos()로 가져오기 시도.
            // EditPlus와 같은 경우.
            MyDebugPrint((TEXT(" ***** GetCaretPos INIT\n")));
            ClientToScreen(lpIMC->hWnd, &pt);
            MyDebugPrint((TEXT(" ***** S %dx%d INIT\n"), pt.x, pt.y));
            rc.left = pt.x;
            rc.bottom = pt.y;
        } else {
            // 없으면 Root window 스타일
            GetWindowRect(lpIMC->hWnd,&rc);
            MyDebugPrint((TEXT("*** Fail to IMR QUERYPOS Root Comp pt.x :%d\n"),rc.left));
        }
        GlobalFree(lpCP);

        lpUIExtra->uiDefComp.pt.x = rc.left;
        lpUIExtra->uiDefComp.pt.y = rc.bottom + 1;
    } else {
        MyDebugPrint((TEXT("Initial Comp pt.x :%d\n"),lpUIExtra->uiDefComp.pt.x));
    }

    // Root window style XXX
    // skkime에서 이것이 rootwindow style이라고 나와있다.
    if (!IsWindow(lpUIExtra->uiDefComp.hWnd))
    {
#if 0
        UINT myStyle=0;
        if (lpUIExtra->dwCompStyle == CFS_DEFAULT) {
            myStyle=WS_BORDER;
        }
#endif
        lpUIExtra->uiDefComp.hWnd = 
            CreateWindowEx( WS_EX_WINDOWEDGE,
                         (LPTSTR)szCompStrClassName,NULL,
                         WS_COMPDEFAULT,
                         //WS_COMPDEFAULT | myStyle,
                         lpUIExtra->uiDefComp.pt.x,
                         lpUIExtra->uiDefComp.pt.y,
                         1,1,
                         hUIWnd,NULL,hInst,NULL);
    }

    if (IsWindow(lpUIExtra->uiDefComp.hWnd))
    {
    SetWindowLongPtr(lpUIExtra->uiDefComp.hWnd,FIGWL_FONT,(LONG_PTR)lpUIExtra->hFont);
    SetWindowLongPtr(lpUIExtra->uiDefComp.hWnd,FIGWL_SVRWND,(LONG_PTR)hUIWnd);
    ShowWindow(lpUIExtra->uiDefComp.hWnd,SW_HIDE);
    lpUIExtra->uiDefComp.bShow = FALSE;
    }

    return;
}


/**********************************************************************/
/*                                                                    */
/* NumCharInDX()                                                      */
/*                                                                    */
/* Count how may the char can be arranged in DX.                      */
/*                                                                    */
/**********************************************************************/
int PASCAL NumCharInDX(HDC hDC,LPMYSTR lp,int dx)
{
    SIZE sz;
    int width = 0;
    int num   = 0;
    int numT  = 0;

    if (!*lp)
        return 0;

    while ((width < dx) && *(lp + numT))
    {
        num = numT;

        numT++;
        GetTextExtentPointW(hDC,lp,numT,&sz);
        width = sz.cx;
    }

    if (width < dx)
        num = numT;

    return num;
}
/**********************************************************************/
/*                                                                    */
/* NumCharInDY()                                                      */
/*                                                                    */
/* Count how may the char can be arranged in DY.                      */
/*                                                                    */
/**********************************************************************/
int PASCAL NumCharInDY(HDC hDC,LPMYSTR lp,int dy)
{
    SIZE sz;
    int width = 0;
    int num;
    int numT = 0;

    if (!*lp)
        return 0;

    while ((width < dy) && *(lp + numT))
    {
        num = numT;
        numT++;
        
        GetTextExtentPointW(hDC,lp,numT,&sz);
        width = sz.cy;
    }

    return num;
}
/**********************************************************************/
/*                                                                    */
/* MoveCompWindow()                                                   */
/*                                                                    */
/* Calc the position of composition windows and move them.            */
/*                                                                    */
/**********************************************************************/
void PASCAL MoveCompWindow( LPUIEXTRA lpUIExtra,LPINPUTCONTEXT lpIMC )
{
    HDC hDC;
    HFONT hFont  = (HFONT)NULL;
    HFONT hOldFont = (HFONT)NULL;
    LPCOMPOSITIONSTRING lpCompStr;
    LPMYSTR lpstr;
    RECT rc;
    RECT oldrc;
    SIZE sz;
    int width=0;
    int height=0;
    int i;

    //
    // Save the composition form style into lpUIExtra.
    //
    MyDebugPrint((TEXT("MoveCompWindow\r\n")));
    lpUIExtra->dwCompStyle = lpIMC->cfCompForm.dwStyle;
    MyDebugPrint((TEXT("MoveCompWindow dwStyle %x\r\n"), lpIMC->cfCompForm.dwStyle));

    if (lpUIExtra->uiDefComp.pt.x == -1) {
        // Not initialized CompWnd position.
        return;
    }

    if (lpIMC->cfCompForm.dwStyle)  // Style is not DEFAULT.
    {
        POINT ptSrc = lpIMC->cfCompForm.ptCurrentPos;
        RECT  rcSrc;
        LPMYSTR lpt;
        int   dx;
        int   dy;
        int   num;
        int   curx;
        int   cury;

        //
        // Lock the COMPOSITIONSTRING structure.
        //
        if (!(lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)))
            return;

        //
        // If there is no composition string, don't display anything.
        //
        if ((lpCompStr->dwSize <= sizeof(COMPOSITIONSTRING))
            || (lpCompStr->dwCompStrLen == 0))
        {
            ImmUnlockIMCC(lpIMC->hCompStr);
            return;
        }

        //
        // Set the rectangle for the composition string.
        //
        if (lpIMC->cfCompForm.dwStyle & CFS_RECT)
            rcSrc = lpIMC->cfCompForm.rcArea;
        else
            GetClientRect(lpIMC->hWnd,(LPRECT)&rcSrc);

        ClientToScreen(lpIMC->hWnd, &ptSrc);
        ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.left);
        ClientToScreen(lpIMC->hWnd, (LPPOINT)&rcSrc.right);
        //
        // Check the start position.
        //
        if (!PtInRect((LPRECT)&rcSrc,ptSrc))
        {
            ImmUnlockIMCC(lpIMC->hCompStr);
            return;
        }

        //
        // Hide the default composition window.
        //
        if (IsWindow(lpUIExtra->uiDefComp.hWnd))
        {
            ShowWindow(lpUIExtra->uiDefComp.hWnd,SW_HIDE);
            lpUIExtra->uiDefComp.bShow = FALSE;
        }

        lpt = lpstr = GETLPCOMPSTR(lpCompStr);
        num = 1;

        if (!lpUIExtra->bVertical)
        {
            dx = rcSrc.right - ptSrc.x;
            curx = ptSrc.x;
            cury = ptSrc.y;

            //
            // Set the composition string to each composition window.
            // The composition windows that are given the compostion string
            // will be moved and shown.
            //
            for (i = 0; i < MAXCOMPWND; i++)
            {
                if (IsWindow(lpUIExtra->uiComp[i].hWnd))
                {
                    hDC = GetDC(lpUIExtra->uiComp[i].hWnd);

                    if (hFont = (HFONT)GetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,FIGWL_FONT))
                        hOldFont = SelectObject(hDC,hFont);

                    sz.cy = 0;
                    oldrc = lpUIExtra->uiComp[i].rc;

                    if (num = NumCharInDX(hDC,lpt,dx))
                    {
                        GetTextExtentPoint(hDC,lpt,num,&sz);

                        lpUIExtra->uiComp[i].rc.left    = curx;
                        lpUIExtra->uiComp[i].rc.top     = cury;
                        lpUIExtra->uiComp[i].rc.right   = sz.cx;
                        lpUIExtra->uiComp[i].rc.bottom  = sz.cy;
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTSTR,
                                      (DWORD) (lpt - lpstr));
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTNUM,
                                      num);
                        MoveWindow(lpUIExtra->uiComp[i].hWnd, 
                                 curx,cury,sz.cx,sz.cy,TRUE);
                        ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_SHOWNOACTIVATE);
                        lpUIExtra->uiComp[i].bShow = TRUE;

                        lpt+=num;

                    }
                    else
                    {
                        lpUIExtra->uiComp[i].rc.left    = 0;
                        lpUIExtra->uiComp[i].rc.top     = 0;
                        lpUIExtra->uiComp[i].rc.right   = 0;
                        lpUIExtra->uiComp[i].rc.bottom  = 0;
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTSTR,
                                      0L);
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTNUM,
                                      0L);
                        ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
                        lpUIExtra->uiComp[i].bShow = FALSE;
                    }

                    InvalidateRect(lpUIExtra->uiComp[i].hWnd,NULL,FALSE);

                    dx = rcSrc.right - rcSrc.left;
                    curx = rcSrc.left;
                    cury += sz.cy;

                    if (hOldFont)
                        SelectObject(hDC,hOldFont);
                    ReleaseDC(lpUIExtra->uiComp[i].hWnd,hDC);
                }
            }
        }
        else  // when it is vertical fonts.
        {
            dy = rcSrc.bottom - ptSrc.y;
            curx = ptSrc.x;
            cury = ptSrc.y;

            for (i = 0; i < MAXCOMPWND; i++)
            {
                if (IsWindow(lpUIExtra->uiComp[i].hWnd))
                {
                    hDC = GetDC(lpUIExtra->uiComp[i].hWnd);

                    if (hFont = (HFONT)GetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,FIGWL_FONT))
                        hOldFont = SelectObject(hDC,hFont);

                    sz.cy = 0;

                    if (num = NumCharInDY(hDC,lpt,dy))
                    {
                        GetTextExtentPoint(hDC,lpt,num,&sz);

                        lpUIExtra->uiComp[i].rc.left    = curx - sz.cy;
                        lpUIExtra->uiComp[i].rc.top     = cury;
                        lpUIExtra->uiComp[i].rc.right   = sz.cy;
                        lpUIExtra->uiComp[i].rc.bottom  = sz.cx;
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTSTR,
                                      (DWORD) (lpt - lpstr));
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTNUM,
                                      num);
                        MoveWindow(lpUIExtra->uiComp[i].hWnd, 
                                 curx,cury,sz.cy,sz.cx,TRUE);
                        ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_SHOWNOACTIVATE);
                        lpUIExtra->uiComp[i].bShow = TRUE;

                        lpt+=num;
                    }
                    else
                    {
                        lpUIExtra->uiComp[i].rc.left    = 0;
                        lpUIExtra->uiComp[i].rc.top     = 0;
                        lpUIExtra->uiComp[i].rc.right   = 0;
                        lpUIExtra->uiComp[i].rc.bottom  = 0;
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTSTR,
                                      0L);
                        SetWindowLong(lpUIExtra->uiComp[i].hWnd,FIGWL_COMPSTARTNUM,
                                      0L);
                        ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
                        lpUIExtra->uiComp[i].bShow = FALSE;
                    }

                    InvalidateRect(lpUIExtra->uiComp[i].hWnd,NULL,FALSE);

                    dy = rcSrc.bottom - rcSrc.top;
                    cury = rcSrc.top;
                    curx -= sz.cy;

                    if (hOldFont)
                        SelectObject(hDC,hOldFont);
                    ReleaseDC(lpUIExtra->uiComp[i].hWnd,hDC);
                }
            }
        }


        ImmUnlockIMCC(lpIMC->hCompStr);
    }
    else
    {
        //
        // When the style is DEFAULT, show the default composition window.
        //
        // root window style XXX
        MyDebugPrint((TEXT("MoveCompWindow root window\r\n")));
        if (IsWindow(lpUIExtra->uiDefComp.hWnd))
        {
            for (i = 0; i < MAXCOMPWND; i++)
            {
                if (IsWindow(lpUIExtra->uiComp[i].hWnd))
                {
                    ShowWindow(lpUIExtra->uiComp[i].hWnd,SW_HIDE);
                    lpUIExtra->uiComp[i].bShow = FALSE;
                }
            }

            hDC = GetDC(lpUIExtra->uiDefComp.hWnd);

            if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
            {
                if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING))
                   && (lpCompStr->dwCompStrLen > 0))
                {
                    lpstr = GETLPCOMPSTR(lpCompStr);
                    GetTextExtentPoint(hDC,lpstr,lstrlen(lpstr),&sz);
                    width = sz.cx;
                    height = sz.cy;
                }
            }

            ReleaseDC(lpUIExtra->uiDefComp.hWnd,hDC);
            { 
                PIMECHARPOSITION lpCP;
                DWORD dwSize = sizeof(IMECHARPOSITION);
        
                lpCP = (PIMECHARPOSITION)GlobalAlloc(GPTR, dwSize);
                lpCP->dwSize = dwSize;
 
                // 워드패드 같은 경우는 IMR_QUERYCHARPOSITION을 지원한다.
                if (dwSize = (DWORD) MyImmRequestMessage(lpUIExtra->hIMC, IMR_QUERYCHARPOSITION, (LPARAM)lpCP)) {
                    rc.left = lpCP->pt.x;
                    rc.bottom = lpCP->pt.y;
                    MyDebugPrint((TEXT("Cand pt.x :%d\n"),lpCP->pt.x));
                } else {
                    UINT    ret = 0;
        
                    ret = (UINT) SendMessage( lpIMC->hWnd, WM_MSIME_QUERYPOSITION, 1, (LPARAM)lpCP );
                    if (ret) {
                        rc.left = lpCP->pt.x;
                        rc.bottom = lpCP->pt.y;
                        MyDebugPrint((TEXT("MSIME Cand pt.x :%d\n"),lpCP->pt.x));
                    } else {
                        POINT pt;
                        // XXX
                        if (GetCaretPos(&pt)) {
                            // GetCaretPos()가 작동되면,
                            MyDebugPrint((TEXT(" ***** GetCaretPos INIT\n")));
                            ClientToScreen(lpIMC->hWnd, &pt);
                            MyDebugPrint((TEXT(" ***** S %dx%d INIT\n"), pt.x, pt.y));
                            rc.left = pt.x;
                            rc.bottom = pt.y;
                        } else if (lpUIExtra->uiDefComp.pt.x != -1) {
                            // 값이 이미 세팅되어 있는 경우.
                            pt.x = lpIMC->cfCompForm.ptCurrentPos.x;
                            pt.y = lpIMC->cfCompForm.ptCurrentPos.y;
                            ClientToScreen(lpIMC->hWnd, &pt);
                            rc.left = pt.x;
                            rc.bottom = pt.y;

                            MyDebugPrint((TEXT(" ***** fdwInit %d\n"), lpIMC->fdwInit & INIT_COMPFORM));
                            MyDebugPrint((TEXT(" ***** dwStyle %d\n"), lpIMC->cfCompForm.dwStyle));

                            rc.left = lpUIExtra->uiDefComp.pt.x;
                            rc.bottom = lpUIExtra->uiDefComp.pt.y;
                        } else {
                            // 이도저도 안되면 root window 스타일.
                            MyDebugPrint((TEXT("*** Fail to get QUERYCHARPOS\r\n")));
                            GetWindowRect(lpIMC->hWnd,&rc);
                        }
                    }
                }
                GlobalFree(lpCP);
            }

            lpUIExtra->uiDefComp.pt.x = rc.left;
            lpUIExtra->uiDefComp.pt.y = rc.bottom;
            MoveWindow(lpUIExtra->uiDefComp.hWnd,
                       rc.left,
                       rc.bottom,
                       width+ 2 * GetSystemMetrics(SM_CXEDGE),
                       height+ 2 * GetSystemMetrics(SM_CYEDGE),
                       TRUE);
            if (lpCompStr->dwCompStrLen > 0) {
                ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_SHOWNOACTIVATE);
                lpUIExtra->uiDefComp.bShow = TRUE;
            } else {
                // comp str이 없을 경우는 감춘다.
                ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_HIDE);
                lpUIExtra->uiDefComp.bShow = FALSE;
            }
            InvalidateRect(lpUIExtra->uiDefComp.hWnd,NULL,FALSE);
            ImmUnlockIMCC(lpIMC->hCompStr);
        }
    }

}

/**********************************************************************/
/*                                                                    */
/* DrawTextOneLine(hDC, lpstr, lpattr, num)                           */
/*                                                                    */
/**********************************************************************/
void PASCAL DrawTextOneLine( HWND hCompWnd, HDC hDC, LPMYSTR lpstr, LPBYTE lpattr, int num, BOOL fVert)
{
    LPMYSTR lpStart = lpstr;
    LPMYSTR lpEnd = lpstr + num - 1;
    HPEN MyPen, hPen;
    int x,y;
    RECT rc;

    if (num == 0)
        return;

    GetClientRect (hCompWnd,&rc);

    MyPen = CreatePen(PS_DOT, 1, RGB(0, 0, 0));
    hPen = (HPEN)SelectObject(hDC, MyPen);

    if (!fVert)
    {
        x = 0;
        y = 0;
    }
    else
    {
        x = rc.right;
        y = 0;
    }

    while (lpstr <= lpEnd)
    {
        int cnt = 0;
        BYTE bAttr = *lpattr;
        SIZE sz;

        while ((bAttr == *lpattr) || (cnt <= num))
        {
            lpattr++;
            cnt++;
        }

        //TextOut(hDC,x,y,lpstr,cnt);
        switch (bAttr)
        {
            case ATTR_INPUT:
                // XXX SetTextColor(hDC,RGB(0,0,0));
                //SetBkMode(hDC,TRANSPARENT);
                MoveToEx(hDC, rc.left, rc.bottom - 1, NULL);
                LineTo  (hDC, rc.right, rc.bottom - 1);
                break;

            case ATTR_TARGET_CONVERTED:
                SetTextColor(hDC,RGB(200,200,200));
                if (!fVert)
                    TextOut(hDC,x + 1,y + 1,lpstr,cnt);
                else
                    TextOut(hDC,x - 1,y + 1,lpstr,cnt);
                SetTextColor(hDC,RGB(0,0,0));
                SetBkMode(hDC,TRANSPARENT);
                break;

            case ATTR_CONVERTED:
                SetTextColor(hDC,RGB(127,127,127));
                if (!fVert)
                    TextOut(hDC,x + 1,y + 1,lpstr,cnt);
                else
                    TextOut(hDC,x - 1,y + 1,lpstr,cnt);
                SetTextColor(hDC,RGB(0,0,255));
                SetBkMode(hDC,TRANSPARENT);
                break;

            case ATTR_TARGET_NOTCONVERTED:
            default:
                //SetTextColor(hDC,RGB(127,127,127));
                //SetBkMode(hDC,TRANSPARENT);
                //MoveToEx(hDC, rc.right, rc.top, NULL);
                //LineTo  (hDC, rc.right, rc.bottom);
                break;
        }

        TextOut(hDC,x,y,lpstr,cnt);
        GetTextExtentPoint(hDC,lpstr,cnt,&sz);
        lpstr += cnt;

        if (!fVert)
            x += sz.cx;
        else
            y += sz.cx;
    }
    SelectObject(hDC, hPen);
    DeleteObject(MyPen);
}

/**********************************************************************/
/*                                                                    */
/* DrawCompCaret(hCompWnd)                                            */
/*                                                                    */
/**********************************************************************/
int PASCAL DrawCompCaret( HWND hCompWnd) {
    RECT rc;
    HIMC hIMC;
    LPINPUTCONTEXT lpIMC;
    HWND hSvrWnd;
    LPCOMPOSITIONSTRING lpCompStr;
    LONG lstart, num;
    int ret = 0;

    GetClientRect(hCompWnd,&rc);

    if (rc.right == 0)
        return 0;

    hSvrWnd = (HWND)GetWindowLongPtr(hCompWnd,FIGWL_SVRWND);
    if (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC)) {
        lpIMC = ImmLockIMC(hIMC);
        if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr)) {
            if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING))
               && (lpCompStr->dwCompStrLen > 0)) {
                LPMYSTR lpstr;
                LPBYTE lpattr;
                LPMYSTR lpEnd;
                BYTE bAttr = ATTR_INPUT;

                lpstr = GETLPCOMPSTR(lpCompStr);
                lpattr = GETLPCOMPATTR(lpCompStr);

                lstart = GetWindowLong(hCompWnd,FIGWL_COMPSTARTSTR);
                num = GetWindowLong(hCompWnd,FIGWL_COMPSTARTNUM);

                lpstr+=lstart;
                lpattr+=lstart;

                lpEnd = lpstr + num - 1;

                // Pseudo caret. Invert last char
                if (bAttr == ATTR_INPUT && *(lpEnd + 1) == TEXT('\0') ) {
                    HDC hDC;
                    PAINTSTRUCT ps;
                    HBRUSH hMybr, hbr;
                    SIZE sz;
                    HFONT hFont = (HFONT)NULL;
                    HFONT hOldFont = (HFONT)NULL;

                    hDC = GetDC(hCompWnd);

                    if (hFont = (HFONT)GetWindowLongPtr(hCompWnd,FIGWL_FONT))
                        hOldFont = SelectObject(hDC,hFont);

                    hMybr = CreateSolidBrush(RGB(255, 255, 255));
                    hbr = SelectObject(hDC,hMybr);
                    if (ic.len > 0) { // 한글 조합중.
                        GetTextExtentPoint(hDC, lpEnd, 1,&sz);
                        rc.left = rc.right - sz.cx;
                    } else { // 조합중 아님.
                        rc.left = rc.right - 2;
                        //InvalidateRect(hCompWnd,NULL,FALSE); // FIXME
                    }
                    rc.right = rc.right;
                    // Invert
                    InvertRect(hDC,&rc);

                    SelectObject(hDC,hbr);
                    DeleteObject(hMybr);

                    if (hOldFont)
                        SelectObject(hDC,hOldFont);
                    ReleaseDC(hCompWnd, hDC);
                    DeleteDC(hDC);
                    ret = 1;
                }
                ImmUnlockIMCC(lpIMC->hCompStr);
            }
        }
        ImmUnlockIMC(hIMC);
    }
    return ret;
}

/**********************************************************************/
/*                                                                    */
/* PaintCompWindow(hCompWnd)                                          */
/*                                                                    */
/**********************************************************************/
void PASCAL PaintCompWindow( HWND hCompWnd)
{
    PAINTSTRUCT ps;
    HIMC hIMC;
    LPINPUTCONTEXT lpIMC;
    LPCOMPOSITIONSTRING lpCompStr;
    HDC hDC;
    RECT rc;
    HFONT hFont = (HFONT)NULL;
    HFONT hOldFont = (HFONT)NULL;
    HWND hSvrWnd;

    GetClientRect(hCompWnd,&rc);
    hDC = BeginPaint(hCompWnd,&ps);
    //FillRect(hDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));

    if (hFont = (HFONT)GetWindowLongPtr(hCompWnd,FIGWL_FONT))
        hOldFont = SelectObject(hDC,hFont);

    hSvrWnd = (HWND)GetWindowLongPtr(hCompWnd,FIGWL_SVRWND);

    if (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC))
    {
        lpIMC = ImmLockIMC(hIMC);
        if (lpCompStr = (LPCOMPOSITIONSTRING)ImmLockIMCC(lpIMC->hCompStr))
        {
            if ((lpCompStr->dwSize > sizeof(COMPOSITIONSTRING))
               && (lpCompStr->dwCompStrLen > 0))
            {
                LPMYSTR lpstr;
                LPBYTE lpattr;
                LONG lstart;
                LONG num;
                BOOL fVert = FALSE;

                if (hFont)
                    fVert = (lpIMC->lfFont.A.lfEscapement == 2700);

                lpstr = GETLPCOMPSTR(lpCompStr);
                lpattr = GETLPCOMPATTR(lpCompStr);
                SetBkMode(hDC,TRANSPARENT);
                if (lpIMC->cfCompForm.dwStyle)
                {
                    // 이곳의 색을 조절하면
                    // 한글을 제대로 지원하지 않는 어플에서
                    // 좀 더 깔끔한 한글 출력을 지원할 수 있을 것이다.
                    HDC hPDC;
#if 0
                    // normal white background color
                    HBRUSH hBrush;
                    hBrush=CreateSolidBrush(GetBkColor(hDC));
                    FillRect(hDC,&rc,hBrush);
#else
                    // BTNFACE background color like as IME 2002/2003.
                    FillRect(hDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));
#endif
                    hPDC = GetDC(GetParent(hCompWnd));

                    GetClientRect (hCompWnd,&rc);
                    SetBkColor(hDC,GetBkColor(hDC));
                    SetTextColor(hDC,GetTextColor(hDC));
                    //SetBkColor(hDC,GetBkColor(hPDC));
                    //SetTextColor(hDC,RGB(127,127,127));
                    //SetBkMode(hDC,OPAQUE);

                    lstart = GetWindowLong(hCompWnd,FIGWL_COMPSTARTSTR);
                    num = GetWindowLong(hCompWnd,FIGWL_COMPSTARTNUM);
                
                    if (!num || ((lstart + num) > Mylstrlen(lpstr)))
                        goto end_pcw;

                    lpstr+=lstart;
                    lpattr+=lstart;
                    DrawTextOneLine(hCompWnd, hDC, lpstr, lpattr, num, fVert);

                    ReleaseDC(GetParent(hCompWnd),hPDC);
                }
                else
                {
                    FillRect(hDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));
                    num = Mylstrlen(lpstr);
                    DrawTextOneLine(hCompWnd, hDC, lpstr, lpattr, num, fVert);
                }
            }
end_pcw:
            ImmUnlockIMCC(lpIMC->hCompStr);
        }
        ImmUnlockIMC(hIMC);
    }
    if (hOldFont) {
        SelectObject(hDC,hOldFont);
    }
    EndPaint(hCompWnd,&ps);

    DeleteDC(hDC);
}
/**********************************************************************/
/*                                                                    */
/* HideCompWindow(lpUIExtra)                                          */
/*                                                                    */
/**********************************************************************/
void PASCAL HideCompWindow(LPUIEXTRA lpUIExtra)
{
    int i;
    RECT rc;

    if (IsWindow(lpUIExtra->uiDefComp.hWnd))
    {
        if (!lpUIExtra->dwCompStyle)
            GetWindowRect(lpUIExtra->uiDefComp.hWnd,(LPRECT)&rc);

        ShowWindow(lpUIExtra->uiDefComp.hWnd, SW_HIDE);
        lpUIExtra->uiDefComp.bShow = FALSE;
    }

    for (i=0;i<MAXCOMPWND;i++)
    {
        if (IsWindow(lpUIExtra->uiComp[i].hWnd))
        {
            ShowWindow(lpUIExtra->uiComp[i].hWnd, SW_HIDE);
            lpUIExtra->uiComp[i].bShow = FALSE;
        }
    }
    // HACK HACK
    lpUIExtra->uiDefComp.pt.x = -1;
    lpUIExtra->uiDefComp.pt.y = -1;
}

/**********************************************************************/
/*                                                                    */
/* SetFontCompWindow(lpUIExtra)                                       */
/*                                                                    */
/**********************************************************************/
void PASCAL SetFontCompWindow(LPUIEXTRA lpUIExtra)
{
    int i;

    for (i=0;i<MAXCOMPWND;i++)
        if (IsWindow(lpUIExtra->uiComp[i].hWnd))
            SetWindowLongPtr(lpUIExtra->uiComp[i].hWnd,FIGWL_FONT,(LONG_PTR)lpUIExtra->hFont);

    MyDebugPrint((TEXT("SetFontCompWindow\r\n")));
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
