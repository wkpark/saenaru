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
 * $Saenaru: saenaru/src/uicand.c,v 1.9 2006/11/20 08:53:02 wkpark Exp $
 */

/**********************************************************************/
#include "windows.h"
#include "commctrl.h"
#include "immdev.h"
#include "saenaru.h"

int PASCAL GetCompFontHeight(LPUIEXTRA lpUIExtra);

HWND g_hwndTip = NULL;
DWORD dwFixing = 0;

/**********************************************************************/
/*                                                                    */
/* CandWndProc()                                                      */
/* IME UI window procedure                                            */
/*                                                                    */
/**********************************************************************/
LRESULT CALLBACK CandWndProc( hWnd, message, wParam, lParam )
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    HWND hUIWnd;
    HBITMAP hbmpHanja, hbmpCheck;
#if 1
    HWND hSvrWnd;
    HIMC hIMC;
    LPINPUTCONTEXT lpIMC;
    static INT dx=0, dy=0;
#endif
    DWORD dwPushedCand;
    POINT pt;
    RECT rc;
    INT select, selpos;
    INT offset;
    static INT o_selpos = -1;
    DWORD  dwT;


    switch (message)
    {
        case WM_PAINT:
            MyDebugPrint((TEXT(" *** WM_PAINT\r\n")));
            PaintCandWindow(hWnd);
            break;
        case WM_CREATE:

            hbmpHanja = LoadImage(hInst,TEXT("HANJA_MODE_BMP"), IMAGE_BITMAP,
                    0,0,LR_LOADTRANSPARENT|LR_CREATEDIBSECTION);

            hbmpCheck = LoadImage(hInst,TEXT("HANJA_CHECK_BMP"), IMAGE_BITMAP,
                    0,0,LR_LOADTRANSPARENT|LR_CREATEDIBSECTION);

            if (hbmpHanja) {
                HDC hdc = CreateCompatibleDC(NULL);
                COLORREF cTransColor;
                DWORD dwColor= GetSysColor(COLOR_MENU);
                cTransColor= RGB(0,255,0);
                if (hdc) {
                    UINT iColor;
                    HBITMAP hbmPrev = SelectObject(hdc, hbmpHanja);
                    RGBQUAD rgbColors[256];
                    UINT cColors = GetDIBColorTable(hdc, 0, 256, rgbColors);
                    for (iColor = 0; iColor < cColors; iColor++) {
                        if (rgbColors[iColor].rgbRed == 0 &&
                            rgbColors[iColor].rgbGreen == 255 &&
                            rgbColors[iColor].rgbBlue == 0) {
                            rgbColors[iColor].rgbRed=GetRValue(dwColor);
                            rgbColors[iColor].rgbGreen=GetGValue(dwColor);
                            rgbColors[iColor].rgbBlue=GetBValue(dwColor);
                            break;
                        }
                    }
                    SetDIBColorTable(hdc, 0, cColors, rgbColors);
                    SelectObject(hdc, hbmPrev);
                    DeleteDC(hdc);
                }
                SetWindowLongPtr(hWnd,FIGWL_HANJA_MODEBMP,(LONG_PTR)hbmpHanja);
            }
            if (hbmpCheck) {
                HDC hdc = CreateCompatibleDC(NULL);
                COLORREF cTransColor;
                DWORD dwColor= GetSysColor(COLOR_BTNFACE + 1);
                cTransColor= RGB(0,255,0);
                if (hdc) {
                    UINT iColor;
                    HBITMAP hbmPrev = SelectObject(hdc, hbmpCheck);
                    RGBQUAD rgbColors[256];
                    UINT cColors = GetDIBColorTable(hdc, 0, 256, rgbColors);
                    for (iColor = 0; iColor < cColors; iColor++) {
                        if (rgbColors[iColor].rgbRed == 0 &&
                            rgbColors[iColor].rgbGreen == 255 &&
                            rgbColors[iColor].rgbBlue == 0) {
                            rgbColors[iColor].rgbRed=GetRValue(dwColor);
                            rgbColors[iColor].rgbGreen=GetGValue(dwColor);
                            rgbColors[iColor].rgbBlue=GetBValue(dwColor);
                            break;
                        }
                    }
                    SetDIBColorTable(hdc, 0, cColors, rgbColors);
                    SelectObject(hdc, hbmPrev);
                    DeleteDC(hdc);
                }
                SetWindowLongPtr(hWnd,FIGWL_HANJA_CHECKBMP,(LONG_PTR)hbmpCheck);
            }

            g_hwndTip = CreateWindowEx(WS_EX_TOPMOST, TOOLTIPS_CLASS, NULL,
                              0,
                              //WS_POPUP | TTS_NOFADE,
                              //WS_POPUP |TTS_ALWAYSTIP | TTS_BALLOON,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              CW_USEDEFAULT, CW_USEDEFAULT,
                              hWnd, NULL, 
                              hInst, NULL);

            if (g_hwndTip) {
                TOOLINFO tip = { 0 };

                tip.cbSize = sizeof(TOOLINFO);
                tip.hwnd = hWnd;
                tip.uFlags = TTF_TRACK;
                tip.lpszText = MYTEXT("");
                SetRect(&tip.rect,0,0,0,0);

                SendMessage(g_hwndTip, TTM_ADDTOOL, 0, (LPARAM) (LPTOOLINFO) &tip);
            }

            MyDebugPrint((TEXT(" * WM_CREATE Cand\r\n")));
            break;
        case WM_MOUSEACTIVATE:
            MyDebugPrint((TEXT(" * WM_MOUSEACTIVATE\r\n")));

            return MA_NOACTIVATE;
            //return MA_NOACTIVATEANDEAT;
            break;
        case WM_DESTROY:
            MyDebugPrint((TEXT(" * WM_DESTORY Cand\r\n")));
            hbmpHanja = (HBITMAP)GetWindowLongPtr(hWnd,FIGWL_HANJA_MODEBMP);
            hbmpCheck = (HBITMAP)GetWindowLongPtr(hWnd,FIGWL_HANJA_CHECKBMP);
            DeleteObject(hbmpHanja);
            DeleteObject(hbmpCheck);

            DestroyWindow(g_hwndTip);
            break;
#if 0
        case SBM_ENABLE_ARROWS:
            MyDebugPrint((TEXT(" * SBM_ENABLE_ARROWS\r\n")));
            break;

        case SBM_GETSCROLLBARINFO:
            MyDebugPrint((TEXT(" * SBM_GETSCROLLBARINFO\r\n")));
            break;
#endif
        case WM_SETCURSOR:
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:

            DragUI(hWnd,message,wParam,lParam);

            dwT = GetWindowLong(hWnd,FIGWL_MOUSE);

            if (dwT & FIM_MOVED)
                dwFixing = 1;

            if (dy == 0)
            {
                TEXTMETRIC tm;
                HDC hDC;

                hDC = CreateIC(TEXT("DISPLAY"),NULL,NULL,NULL);
                GetTextMetrics(hDC,&tm);
                dy = tm.tmHeight + tm.tmExternalLeading + 4;
                DeleteDC(hDC);
            }

            select = 0;
            offset = 0;
            while (HIWORD(lParam) == WM_LBUTTONDOWN || HIWORD(lParam) == WM_MOUSEMOVE) {
                LPCANDIDATEINFO lpCandInfo;
                LPCANDIDATELIST lpCandList;
                int sel;
                HDC hDC;
                PAINTSTRUCT ps;
                TOOLINFO tip = { 0 };
                GetCursorPos(&pt);
                ScreenToClient(hWnd,&pt);
                GetClientRect(hWnd,&rc);

                if (!PtInRect(&rc,pt)) {
                    // not in the candlist wind
                    break;
                }

                rc.bottom -= 15;
                if (!PtInRect(&rc,pt)) {
                    // on hanja-mode
                    //
                    if (HIWORD(lParam) == WM_LBUTTONDOWN) {
                        if (pt.x < 80) {
                            if (dwHanjaMode == 1)
                                dwHanjaMode = 0;
                            else
                                dwHanjaMode = 1;
                        } else if (pt.x < 144) {
                            if (dwHanjaMode == 2)
                                dwHanjaMode = 0;
                            else
                                dwHanjaMode = 2;
                        }
                        MyDebugPrint((TEXT(" * hanja_mode=%d\r\n"), dwHanjaMode));
                        PaintCandWindow(hWnd);
                        InvalidateRect(hWnd,NULL,FALSE);
                    }
                    break;
                }

                pt.y -= GetSystemMetrics(SM_CYEDGE);
                if (pt.y > 0)
                    select = pt.y / dy + ((pt.y % dy) ? 1:0);
                if (select == 0)
                    offset = -2;
#if 0
                else if (select > 9)
                {
                    if (pt.x < 14)
                        offset = -2;
                    else
                        offset = 2;
                    select = 0;
                }
#endif
                hSvrWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);

                if (!(hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC)) )
                    break;

                lpIMC = ImmLockIMC(hIMC);
                if (!lpIMC)
                    break;

                if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo)) {
                    int start;
                    UINT ii, jj;

                    lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);

                    if (select > (int) lpCandList->dwPageSize)
                        select = lpCandList->dwPageSize;

                    if (lpCandList->dwStyle == IME_CAND_CODE) {
                        dx = 50;
                        start = pt.x / dx + ((pt.x % dx) ? 1:0) - 1;
                        start *= lpCandList->dwPageSize;
                        ii = start + select - 1;
                        sel = (lpCandList->dwSelection + 1) / lpCandList->dwPageSize + 
                            ((lpCandList->dwSelection + 1) % lpCandList->dwPageSize ? 1:0) - 1;
                        sel *= lpCandList->dwPageSize;
                        sel = ii - sel + 1;
                    } else {
                        start = lpCandList->dwPageStart;
                        ii = start + select - 1;
                        sel = select;
                    }

                    selpos = ii;

                    if (selpos != o_selpos && ii <= lpCandList->dwCount) {
                        TCHAR buf[128];
                        LPMYSTR lpstr;

                        if (ii >= lpCandList->dwCount) {
                            buf[0] = TEXT('\0');
                        } else {
                            TCHAR mybuf[128];
                            LPMYSTR lpTemp, lpmystr;

                            lpstr=(LPMYSTR)((LPSTR)lpCandList +lpCandList->dwOffset[ii]);

                            lpmystr = (LPTSTR) mybuf;
                            Mylstrcpy(lpmystr, lpstr);
                            if (NULL != (lpTemp = Mystrchr(lpmystr, MYTEXT(':')))) {
                                *lpTemp = MYTEXT('\0');
                                wsprintf(buf, TEXT("%s"), (lpTemp + 1));
                            } else {
                                wsprintf(buf, TEXT("%s"), lpmystr);
                            }
                        }

                        /*
                        tip.cbSize = sizeof(tip);
                        tip.hwnd = hWnd;
                        tip.uFlags = TTF_SUBCLASS;
                        tip.lpszText = buf;
                        tip.rect.left = rc.left;
                        tip.rect.right = rc.right;
                        tip.rect.top = rc.right;
                        tip.rect.bottom = dy * select;
                        tip.rect.top = dy * (select - 1);

                        SendMessage(g_hwndTip, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &tip);
                        */
                        ImmUnlockIMCC(lpIMC->hCandInfo);

                        o_selpos = ii;

                        ConvHanja(hIMC,offset,sel);
                    }
                }
                ImmUnlockIMC(hIMC);
                break;
            }

            if ((message == WM_SETCURSOR) &&
                    (HIWORD(lParam) != WM_LBUTTONDOWN) &&
                    (HIWORD(lParam) != WM_RBUTTONDOWN)) {
                return DefWindowProc(hWnd,message,wParam,lParam);
            }

            // MyDebugPrint((TEXT(" $ CANDMOVE %dx%d\n"),LOWORD(lParam),HIWORD(lParam)));
            if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP))
                SetWindowLong(hWnd,FIGWL_MOUSE,0L);
            break;

        case WM_LBUTTONDBLCLK:

            hSvrWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);
            if (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC) )
                MakeResultString(hIMC,TRUE);
            break;

        case WM_MOVE:
            hUIWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);

            MyDebugPrint((TEXT(" x CANDMOVE %dx%d\n"),LOWORD(lParam),HIWORD(lParam)));
            if (IsWindow(hUIWnd))
                SendMessage(hUIWnd,WM_UI_CANDMOVE,wParam,lParam);
            break;

        case WM_VSCROLL:
        {
            LPCANDIDATEINFO lpCandInfo;
            LPCANDIDATELIST lpCandList;
            int select=0;
            int offset=0;
            int sel=0;
            int yPos=0;
            hSvrWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);

            if (!(hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC)) )
                // XXX not occur
                break;

            lpIMC = ImmLockIMC(hIMC);

            lpCandInfo= (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo);
            lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
            sel = lpCandList->dwSelection % lpCandList->dwPageSize;
            // get the current selected number
            // dwPageSize <= 9 (default)

            switch (LOWORD(wParam)) 
            { 
                case SB_PAGEUP:
                    offset=-2;
                    break;
                case SB_PAGEDOWN:
                    offset=2;
                    break;
                case SB_LINEUP:
                    offset=-1;
                    break;
                case SB_LINEDOWN:
                    offset=1;

                    break;
                case SB_THUMBPOSITION:
                    yPos = HIWORD(wParam);
                case SB_THUMBTRACK:
                    yPos = HIWORD(wParam);
                    MyDebugPrint((TEXT("THUMB Track %d\n"),yPos));
                    lpCandList->dwSelection=yPos;
                    SetScrollPos(hWnd,SB_VERT,yPos,TRUE);

                    break;
                default:
                    break;
            }

            MyDebugPrint((TEXT("THUMB %d\n"),yPos));

            ConvHanja(hIMC,offset,select);
            ImmUnlockIMCC(lpIMC->hCandInfo);
            ImmUnlockIMC(hIMC);
        }
            break;
        default:
            if (!MyIsIMEMessage(message))
                return DefWindowProc(hWnd,message,wParam,lParam);

            break;
    }
    return 0L;
}

/**********************************************************************/
/*                                                                    */
/* GetCandPosFromCompWnd()                                            */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL GetCandPosFromCompWnd(LPUIEXTRA lpUIExtra,LPPOINT lppt)
{
    RECT rc;

    if (lpUIExtra->dwCompStyle)
    {
        if (lpUIExtra->uiComp[0].bShow)
        {
            if (IsWindow(lpUIExtra->uiComp[0].hWnd)) {
                GetWindowRect(lpUIExtra->uiComp[0].hWnd,&rc);
                lppt->x = rc.left;
                lppt->y = rc.bottom+1;
                return TRUE;
            }
        }
    }
    else
    {
        if (lpUIExtra->uiDefComp.bShow)
        {
            if (IsWindow(lpUIExtra->uiDefComp.hWnd)) {
                GetWindowRect(lpUIExtra->uiDefComp.hWnd,&rc);
                lppt->x = rc.left;
                lppt->y = rc.bottom+1;
                return TRUE;
            }
        }
    }

    MyDebugPrint((TEXT(" *** GetCandPosFromCompWnd FALSE\n")));
    return FALSE;
}

/**********************************************************************/
/*                                                                    */
/* GetCandPosFromCompForm()                                           */
/*                                                                    */
/**********************************************************************/
BOOL PASCAL GetCandPosFromCompForm(LPINPUTCONTEXT lpIMC, LPUIEXTRA lpUIExtra,LPPOINT lppt)
{
    if (lpUIExtra->dwCompStyle)
    {
        if (lpIMC && lpIMC->fdwInit & INIT_COMPFORM)
        {
            if (!lpUIExtra->bVertical)
            {
                UINT dy = GetCompFontHeight(lpUIExtra);
                UINT hy = GetSystemMetrics(SM_CYFULLSCREEN);
                POINT pt;

                lppt->x = lpIMC->cfCompForm.ptCurrentPos.x;
                lppt->y = lpIMC->cfCompForm.ptCurrentPos.y + dy;
                                     //GetCompFontHeight(lpUIExtra);
                                     //
                pt.x = lppt->x;
                pt.y = lppt->y;
                
                ClientToScreen(lpIMC->hWnd,&pt);
                if ((pt.y + 9 * dy) > hy) {
                    pt.x += dy + 2;
                    pt.y = hy - 10 * dy;
                    ScreenToClient(lpIMC->hWnd,&pt);
                    lppt->x = pt.x;
                    lppt->y = pt.y;
                }
                MyDebugPrint((TEXT("FromCompForm #1\n")));
            }
            else
            {
                lppt->x = lpIMC->cfCompForm.ptCurrentPos.x -
                                     GetCompFontHeight(lpUIExtra);
                lppt->y = lpIMC->cfCompForm.ptCurrentPos.y;
                MyDebugPrint((TEXT("FromCompForm #2\n")));
            }
            return TRUE;
        }
    }
    else
    {
        if (!GetCandPosFromCompWnd(lpUIExtra,lppt))
        {
            if (lpIMC->cfCandForm[0].dwIndex != -1) {
                lppt->x = lpIMC->cfCandForm[0].ptCurrentPos.x;
                lppt->y = lpIMC->cfCandForm[0].ptCurrentPos.y;
                ClientToScreen(lpIMC->hWnd,lppt);

                MyDebugPrint((TEXT("FromCompForm : Already opened cand wnd #2\n")));
            } else {
                RECT rc;
                MyDebugPrint((TEXT("FromCompForm root style #2\n")));
                GetWindowRect(lpIMC->hWnd,&rc);
                lppt->x = rc.left;
                lppt->y = rc.bottom + 23; // 23 적당한 창 테두리 XXX
            }
        }
        ScreenToClient(lpIMC->hWnd,lppt);
        return TRUE;
    }
    return FALSE;
}

/**********************************************************************/
/*                                                                    */
/* CreateCandWindow()                                                 */
/*                                                                    */
/**********************************************************************/
void PASCAL CreateCandWindow( HWND hUIWnd,LPUIEXTRA lpUIExtra, LPINPUTCONTEXT lpIMC )
{
    POINT pt;

    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;

    MyDebugPrint((TEXT("CreateCandWindow\n")));
    // CandWindow는 CompWindow의 위치를 기반으로 생성된다.
    // 그러나, CompWindow가 없는 경우도 있다 (워드패드)
    if (!GetCandPosFromCompWnd(lpUIExtra,&pt))
    {
        RECT rc;

        PIMECHARPOSITION lpCP;
        DWORD dwSize = sizeof(IMECHARPOSITION);
        
        lpCP = (PIMECHARPOSITION)GlobalAlloc(GPTR, dwSize);
        lpCP->dwSize = dwSize;
        MyDebugPrint((TEXT("Cand dwSize :%d\n"),dwSize));
 
        // 워드패드 같은 경우는 IMR_QUERYCHARPOSITION을 지원한다.
        if (dwSize = (DWORD) MyImmRequestMessage(lpUIExtra->hIMC, IMR_QUERYCHARPOSITION, (LPARAM)lpCP)) {
            pt.x = lpCP->pt.x;
            pt.y = lpCP->pt.y + lpCP->cLineHeight + 1;
            MyDebugPrint((TEXT("Cand pt.x :%d\n"),pt.x));
        } else if (lpIMC->cfCandForm[0].dwIndex != -1) {
            // 이미 만들어 진 것이 있는 경우.
            pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
            pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
            ClientToScreen(lpIMC->hWnd,&pt);
            MyDebugPrint((TEXT("Already opened cand POS pt %dx%d\n"),pt.x,pt.y));
        } else {
            // 이 경우, 전체 윈도우 크기를 알아서 왼쪽 하단에 붙여준다.
            // 마치 리눅스의 입력상태 창같은 위치
            // skkime에서는 root 윈도라 부름.
            GetWindowRect(lpIMC->hWnd,&rc);
            pt.x = rc.left;
            pt.y = rc.bottom + 1;
        }
        GlobalFree(lpCP);
    }
    lpUIExtra->uiCand.pt.x = pt.x;
    lpUIExtra->uiCand.pt.y = pt.y;
    MyDebugPrint((TEXT("Cand Pos :%dx%d\n"),pt.x,pt.y));

    if (!IsWindow(lpUIExtra->uiCand.hWnd))
    {
        SCROLLINFO sScrollInfo;

        lpUIExtra->uiCand.hWnd = 
                CreateWindowEx(WS_EX_WINDOWEDGE|
#if 0
#if (WINVER >= 0x0500)
                             WS_EX_COMPOSITED|
#endif
#endif
                             WS_EX_RIGHTSCROLLBAR,
                             (LPTSTR)szCandClassName,NULL,
                             WS_COMPNODEFAULT|
                             WS_DISABLED|WS_VSCROLL|
                             SBS_VERT|SBS_RIGHTALIGN|
                             SBS_SIZEBOX|SBS_SIZEBOXBOTTOMRIGHTALIGN,
                             //WS_COMPDEFAULT | WS_DLGFRAME,
                             lpUIExtra->uiCand.pt.x,
                             lpUIExtra->uiCand.pt.y,
                             CW_USEDEFAULT,100,
                             hUIWnd,NULL,hInst,NULL);

        sScrollInfo.cbSize=sizeof(SCROLLINFO);
        sScrollInfo.fMask=SIF_POS|SIF_RANGE;
        sScrollInfo.nMin=0;
        sScrollInfo.nMax=30; // some arbitary value
        sScrollInfo.nPage=0;
        sScrollInfo.nPos=0;
        sScrollInfo.nTrackPos=0;

        SetScrollInfo((HWND)lpUIExtra->uiCand.hWnd,SB_VERT,
                (LPSCROLLINFO)&sScrollInfo,FALSE);
    }

    // get candidate info dwCount (is the number of pages
    if (lpCandInfo= (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo)) {
        int count = 0;
        lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
        count = lpCandList->dwCount - 1;

        MyDebugPrint((TEXT("CandList count = %d\r\n"),count));

        if (lpCandList->dwCount > 1) {
            // to make popup with scrollbar using SetCapture();
            EnableWindow( lpUIExtra->uiCand.hWnd, TRUE);
            // init scroll pos
            SetScrollRange( lpUIExtra->uiCand.hWnd, SB_VERT, 0, count, TRUE);
            SetScrollPos( lpUIExtra->uiCand.hWnd, SB_VERT, 0, TRUE);
            EnableScrollBar( lpUIExtra->uiCand.hWnd, SB_VERT, ESB_ENABLE_BOTH);
            ShowScrollBar( lpUIExtra->uiCand.hWnd, SB_VERT, TRUE);
        }
        ImmUnlockIMCC(lpIMC->hCandInfo);
    }

    SetWindowLongPtr(lpUIExtra->uiCand.hWnd,FIGWL_SVRWND,(LONG_PTR)hUIWnd);
    ShowWindow(lpUIExtra->uiCand.hWnd, SW_HIDE);
    lpUIExtra->uiCand.bShow = FALSE;

    return;
}

/**********************************************************************/
/*                                                                    */
/* PaintCandWindow()                                                  */
/*                                                                    */
/**********************************************************************/
void PASCAL PaintCandWindow( HWND hCandWnd)
{
    PAINTSTRUCT ps;
    HIMC hIMC;
    LPINPUTCONTEXT lpIMC;
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    HBRUSH hbr;
    HDC hDC;
    // double buffering
    HDC hMemDC, hTempDC, hTempDC2;
    HBITMAP hOldBit, hBit;
    RECT rc,rc2,rcn;
    LPMYSTR lpstr;
    int height;
    DWORD i;
    SIZE sz;
    HWND hSvrWnd;
    HBRUSH hbrHightLight;
    HBRUSH hbrLGR = (HBRUSH)(COLOR_BTNFACE + 1);
    HFONT hOldFont = NULL;
    HBRUSH hBrush;

    HBITMAP hbmpHanja, hbmpCheck, hbmpOld = NULL;

    LOGFONT lfFont;
    HFONT hNumFont;

    MyDebugPrint((TEXT("BEGIN GDI=%d\n"), GetGuiResources(GetCurrentProcess (),GR_GDIOBJECTS)));
    hDC = BeginPaint(hCandWnd,&ps);
    GetClientRect(hCandWnd,&rc);

    hbrHightLight = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));

    // Init Double Buffering
    hBit = CreateCompatibleBitmap(hDC, rc.right, rc.bottom);
    // save
    hMemDC = CreateCompatibleDC(hDC);
    hTempDC = CreateCompatibleDC(hDC);
    hTempDC2 = CreateCompatibleDC(hDC);
    // save OldBit
    hOldBit = (HBITMAP) SelectObject(hMemDC, hBit);

    rc2.left=rc.left; // left margin: 25
    rc2.right=rc.right;
    rc2.top=rc.top+1;
    rc2.bottom=rc.bottom- 8 * GetSystemMetrics(SM_CYEDGE); // bottom space: 8

    hBrush=CreateSolidBrush(GetBkColor(hMemDC));
    FillRect(hMemDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));
    FillRect(hMemDC,&rc2,hBrush); // fill text area

    rcn.left=rc.left; // left margin: 25
    rcn.right=rc.left + 25;
    rcn.top=rc.top;
    rcn.bottom=rc.bottom- 8 * GetSystemMetrics(SM_CYEDGE); // bottom space: 8

    SetBkMode(hMemDC,TRANSPARENT);

    // set number font
    hOldFont = GetCurrentObject(hMemDC, OBJ_FONT);
    GetObject(hOldFont, sizeof(LOGFONT), &lfFont);

    lfFont.lfWeight = FW_BOLD;
    lfFont.lfHeight = 12;
    lfFont.lfWidth = 0;
    lfFont.lfCharSet = DEFAULT_CHARSET;

    Mylstrcpy(lfFont.lfFaceName,TEXT("Verdana"));
    hNumFont = CreateFontIndirect(&lfFont);

#if 0
    MoveToEx(hDC, rc.left+1,rc.top+1,NULL);
    LineTo(hDC, rc.left+1,rc.bottom-1);
    LineTo(hDC, rc.right-1,rc.bottom-1);
    LineTo(hDC, rc.right-1,rc.top+1);
    LineTo(hDC, rc.left+1,rc.top+1);
#endif

    hSvrWnd = (HWND)GetWindowLongPtr(hCandWnd,FIGWL_SVRWND);

    if (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC))
    {
        lpIMC = ImmLockIMC(hIMC);       
        hOldFont = CheckNativeCharset(hMemDC);
        if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
        {
            UINT start, end;
            UINT left, width;
            UINT sel_page;
            UINT pos;

            height = GetSystemMetrics(SM_CYEDGE); 
            lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
            left = 0;

            // calc selected page.
            pos = ((LPMYCAND)lpCandInfo)->cl.dwSelection;
            sel_page = (pos + 1) / lpCandList->dwPageSize +
                (((pos + 1) % lpCandList->dwPageSize) ? 1:0);

            if (lpCandList->dwStyle == IME_CAND_READ) {
                start = lpCandList->dwPageStart;
                end = lpCandList->dwPageStart + lpCandList->dwPageSize;
                width = rc.right; // full width
            } else {
                start = 0;
                end = lpCandList->dwCount;
                width = 50;

                rcn.left += (sel_page - 1) * width;
                rcn.right += (sel_page - 1) * width;
            }
            // selection number area
            FillRect(hMemDC,&rcn,(HBRUSH) (COLOR_BTNFACE + 1));

            for (i = start; i < end; i++)
            {
                TCHAR num[4];
                INT wtype; // KSX 1002 set or not
                INT highlighted;
                TCHAR mybuf[128];
                TCHAR mycode[10];
                LPMYSTR lpTemp, lpmystr;
                UINT pg;
                BOOL mean = FALSE;

                pg = (i + 1) / lpCandList->dwPageSize +
                    (((i + 1) % lpCandList->dwPageSize) ? 1:0);

                if (i % lpCandList->dwPageSize == 0 && lpCandList->dwStyle == IME_CAND_CODE) {
                    left = (i / lpCandList->dwPageSize) * 50;
                    height = GetSystemMetrics(SM_CYEDGE); 
                }

                wtype=0;
                highlighted=0;
                wsprintf(num, TEXT("%d "), i % lpCandList->dwPageSize + 1);

                if (i >= lpCandList->dwCount) 
                    lpstr=(LPMYSTR)(LPSTR)" ";
                else {

                    lpstr=(LPMYSTR)((LPSTR)lpCandList +lpCandList->dwOffset[i]);

                    lpmystr = (LPTSTR) mybuf;
                    Mylstrcpy(lpmystr, lpstr);
                    lpmystr[MAXCANDSTRSIZE]=MYTEXT('\0'); // force terminate str
                    if (NULL != (lpTemp = Mystrchr(lpmystr, MYTEXT(':')))) {
                        mean = TRUE;
                        if (lpCandList->dwStyle == IME_CAND_READ) {
                            *lpTemp = MYTEXT(' '); // with meaning
                        } else {
                            *lpTemp = MYTEXT('\0'); // show only candidate
                        }
                        lpTemp ++;
                    } else {
                        DWORD code = (DWORD) *(lpstr);
                        if (i != 0 && lpCandList->dwStyle == IME_CAND_CODE && code >= 0x3400) {
                            if (code < 0xffff)
                                wsprintf(mycode, TEXT("U+%04X"), code);
                            else
                                wsprintf(mycode, TEXT("U+%05X"), code);
                            lpTemp = mycode;
                            mean = TRUE;
                        } else {
                            mean = FALSE;
                        }
                    }

                    lpstr = lpmystr;
                }

                MyGetTextExtentPoint(hMemDC,lpstr,Mylstrlen(lpstr),&sz);
                if (((LPMYCAND)lpCandInfo)->cl.dwSelection == (DWORD)i)
                {
                    TOOLINFO tip = { 0 };
                    tip.cbSize = sizeof(TOOLINFO);
                    tip.uFlags=0;
                    tip.hwnd=hCandWnd;
                    tip.uId=0;

                    // show tip
                    if (mean && lpCandList->dwStyle == IME_CAND_CODE) {
                        POINT pt;

                        tip.hwnd = hCandWnd;
                        //tip.uFlags = TTF_SUBCLASS;
                        tip.uFlags = TTF_TRACK;
                        tip.lpszText = lpTemp;
                        tip.rect.left = 5 + left;
                        tip.rect.right = tip.rect.left + width;
                        tip.rect.top = height;
                        tip.rect.bottom = height + sz.cy;

                        //SendMessage(g_hwndTip, TTM_SETTOOLINFO, 0, (LPARAM) (LPTOOLINFO) &tip);
                        SendMessage(g_hwndTip, TTM_UPDATETIPTEXT, 0, (LPARAM) (LPTOOLINFO) &tip);

                        pt.x = tip.rect.left + 20;
                        pt.y = tip.rect.top;
                        ClientToScreen(hCandWnd,&pt);
                        SendMessage(g_hwndTip, TTM_TRACKPOSITION, 0, (LPARAM) MAKELONG(pt.x, pt.y + 20));
                        SendMessage(g_hwndTip, TTM_TRACKACTIVATE, (WPARAM) TRUE, (LPARAM) &tip);
                        SendMessage(g_hwndTip, TTM_SETDELAYTIME, TTDT_AUTOMATIC, -1);
                    } else {
                        SendMessage(g_hwndTip, TTM_TRACKACTIVATE, (WPARAM) FALSE, (LPARAM) &tip);
                    }

                    hbr = SelectObject(hMemDC,hbrHightLight);
                    PatBlt(hMemDC,left,height,width,sz.cy,PATCOPY);
                    SelectObject(hMemDC,hbr);
                    SetTextColor(hMemDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
                    highlighted=1;
                }
                else
                {
                    hbr = SelectObject(hMemDC,hbrLGR);
                    //PatBlt(hDC,0,height,rc.right,sz.cy,PATCOPY);
                    SelectObject(hMemDC,hbr);
                    SetTextColor(hMemDC,RGB(0,0,0));
                }
                if (i < lpCandList->dwCount && sel_page == pg) {
                    MyTextOut(hMemDC,5 + GetSystemMetrics(SM_CXEDGE) + left,height,num,Mylstrlen(num));
                    //MyTextOut(hDC,6 + GetSystemMetrics(SM_CXEDGE),height,num,Mylstrlen(num));
                }
                // 한 글자이면서 KSX1002 지원이 아니면 charset 체크
                if (i < lpCandList->dwCount && wtype != -1 && dwOptionFlag & KSX1002_SUPPORT)
                {
                    WORD mb;
                    int ret;

                    ret = WideCharToMultiByte(949, WC_COMPOSITECHECK,
                        lpstr, 1, (char *)&mb, 2, NULL, NULL);

                    if (ret == 1) {
                        DWORD code = (DWORD) *(lpstr);
                        wtype = 1;

                        // KS X 1002에 포함 안되는 확장 한자의 경우
                        if (!is_ksx1002(code))
                            wtype = 2;
                    }
                }
                if (wtype) {
                    if (highlighted)
                        SetTextColor(hMemDC,RGB(0,0,0)); // shadow
                    else
                        SetTextColor(hMemDC,RGB(220,220,220));
                    MyTextOut(hMemDC,25 + 1 + GetSystemMetrics(SM_CXEDGE) + left,
                            height + 1,lpstr, Mylstrlen(lpstr));
                }
                // 완성형 4888자가 아닌 경우 색을 청색조로.
                if (highlighted) {
                    SetTextColor(hMemDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
                } else if (wtype) {
                    if (wtype == 1)
                        SetTextColor(hMemDC,RGB(16,83,239));
                    else
                        // KS X 1002에도 포함 안되는 경우는 녹색.
                        SetTextColor(hMemDC,RGB(30,150,30));
                }

                MyTextOut(hMemDC,25 + GetSystemMetrics(SM_CXEDGE) + left,height,lpstr,
                    Mylstrlen(lpstr));
                // 25 is left margin of candwin
                height += sz.cy + 4;
                // 4 is linespace
            }
            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        ImmUnlockIMC(hIMC);
    }

    if (hOldFont) {
        DeleteObject(SelectObject(hMemDC, hOldFont));
    }

    if (lpCandList->dwCount > 1) {
        int pages,page;
        int pos;
        TCHAR num[10];
        GetScrollPos(hCandWnd,SB_VERT);

        pos = lpCandList->dwSelection;
        pages = lpCandList->dwCount / lpCandList->dwPageSize +
            ((lpCandList->dwCount % lpCandList->dwPageSize) ? 1:0);

        page = (pos + 1) / lpCandList->dwPageSize +
            (((pos + 1) % lpCandList->dwPageSize) ? 1:0);

        SetTextColor(hMemDC,RGB(0,0,0));

        wsprintf(num, TEXT("%d/%d"), page, pages);

        SelectObject(hMemDC, hNumFont);
        MyTextOut(hMemDC,rc.left + 10 - Mylstrlen(num),rc.bottom - 15 ,num,Mylstrlen(num));

        //pos= GetScrollPos(hCandWnd,SB_VERT);
        ShowScrollBar(hCandWnd,SB_VERT, (pages == 1 || lpCandList->dwStyle == IME_CAND_CODE) ? FALSE : TRUE);
        SetScrollPos(hCandWnd,SB_VERT,pos,TRUE);
    }

    if (lpCandList->dwStyle == IME_CAND_READ) {
        // hanjamod icon text
        hbmpHanja = (HBITMAP)GetWindowLongPtr(hCandWnd,FIGWL_HANJA_MODEBMP);
        hbmpOld = SelectObject(hTempDC,hbmpHanja);

        BitBlt(hMemDC, rc.left + 32, rc.bottom - 15, rc.left + 32 + 114, rc.bottom - 1,
                hTempDC, 0, 0, SRCAND);

        if (dwHanjaMode) {
            hbmpCheck = (HBITMAP)GetWindowLongPtr(hCandWnd,FIGWL_HANJA_CHECKBMP);
            SelectObject(hTempDC2,hbmpCheck);
            if (dwHanjaMode == 1)
                BitBlt(hMemDC, rc.left + 32, rc.bottom - 15, rc.left + 32 + 8, rc.bottom - 1,
                        hTempDC2, 0, 0, SRCAND);
            else
                BitBlt(hMemDC, rc.left + 88, rc.bottom - 15, rc.left + 88 + 8, rc.bottom - 1,
                        hTempDC2, 0, 0, SRCAND);
        }
    }


    BitBlt(hDC, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);
    SelectObject(hMemDC, hOldBit);

    EndPaint(hCandWnd,&ps);

    DeleteObject(hNumFont);

    DeleteObject(hbrHightLight);
    DeleteObject(hBrush);
    DeleteDC(hDC);

    DeleteObject(hBit);
    DeleteDC(hMemDC);
    if (hbmpOld)
        SelectObject(hTempDC,hbmpOld);
    DeleteDC(hTempDC);
    DeleteDC(hTempDC2);
}

/**********************************************************************/
/*                                                                    */
/* ResizeCandWindow()                                                 */
/*                                                                    */
/**********************************************************************/
void PASCAL ResizeCandWindow( LPUIEXTRA lpUIExtra,LPINPUTCONTEXT lpIMC )
{
    LPCANDIDATEINFO lpCandInfo;
    LPCANDIDATELIST lpCandList;
    HDC hDC;
    LPMYSTR lpstr;
    int width;
    int height;
    DWORD i;
    RECT rc;
    SIZE sz;

    if (IsWindow(lpUIExtra->uiCand.hWnd))
    {
        HFONT hOldFont = NULL;

        hDC = GetDC(lpUIExtra->uiCand.hWnd);
        hOldFont = CheckNativeCharset(hDC);

        if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
        {
            width = 0; 
            height = 0; 
            lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
            for (i = lpCandList->dwPageStart; 
                 i < (lpCandList->dwPageStart + lpCandList->dwPageSize); i++)
            {
                LPTSTR lpTemp;
                TCHAR buf[128];

                lpstr = (LPMYSTR)((LPSTR)lpCandList + lpCandList->dwOffset[i]);
                Mylstrcpy(buf, lpstr);

                if (lpCandList->dwStyle == IME_CAND_CODE && (NULL != (lpTemp = Mystrchr(buf, MYTEXT(':'))))) {
                    *lpTemp = MYTEXT('\0');
                    MyGetTextExtentPoint(hDC,lpstr,Mylstrlen(buf),&sz); // show only candidates
                } else {
                    MyGetTextExtentPoint(hDC,lpstr,Mylstrlen(lpstr),&sz); // with meanings
                }
                if (width < sz.cx)
                    width = sz.cx;
                height += sz.cy + 4; // plus linespace 4
            }
            if (lpCandList->dwStyle == IME_CAND_CODE) {
                width = 50; // IME_CAND_CODE unit width.
                width *= (lpCandList->dwCount / lpCandList->dwPageSize) + (lpCandList->dwCount % lpCandList->dwPageSize ? 1 : 0);
            } else {
                if (width < 144) // Hanja mode bitmap width
                    width = 144;
                width += 25 + 4 + 4 * GetSystemMetrics(SM_CXEDGE);
                width+= GetSystemMetrics(SM_CXVSCROLL);
            }
            ImmUnlockIMCC(lpIMC->hCandInfo);
        }
        if (hOldFont) {
            DeleteObject(SelectObject(hDC, hOldFont));
        }
        ReleaseDC(lpUIExtra->uiCand.hWnd,hDC);
        
        GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
        MoveWindow(lpUIExtra->uiCand.hWnd,
                       rc.left,
                       rc.top,
                       width,
                       /* 25 is a left margin */
                       height+ 8 * GetSystemMetrics(SM_CYEDGE),
                       // bottom margin is 8 * SM_CYEDGE
                       TRUE);
    }
}

/**********************************************************************/
/*                                                                    */
/* HideCandWindow()                                                   */
/*                                                                    */
/**********************************************************************/
void PASCAL HideCandWindow( LPUIEXTRA lpUIExtra)
{
    RECT rc;
    TOOLINFO ti;

    if (IsWindow(lpUIExtra->uiCand.hWnd))
    {
        GetWindowRect(lpUIExtra->uiCand.hWnd,(LPRECT)&rc);
        lpUIExtra->uiCand.pt.x = rc.left;
        lpUIExtra->uiCand.pt.y = rc.top;
        MoveWindow(lpUIExtra->uiCand.hWnd, -1 , -1 , 0 , 0, TRUE);
        ShowWindow(lpUIExtra->uiCand.hWnd, SW_HIDE);
        lpUIExtra->uiCand.bShow = FALSE;
    }
    ti.cbSize=sizeof(TOOLINFO);
    ti.uFlags=0;
    ti.hwnd=lpUIExtra->uiCand.hWnd;
    ti.uId=0;
    dwFixing = 0;

    SendMessage(g_hwndTip, TTM_TRACKACTIVATE, (WPARAM) FALSE, (LPARAM) &ti);
}

/**********************************************************************/
/*                                                                    */
/* MoveCandWindow()                                                   */
/*                                                                    */
/**********************************************************************/
void PASCAL MoveCandWindow(HWND hUIWnd, LPINPUTCONTEXT lpIMC, LPUIEXTRA lpUIExtra, BOOL fForceComp)
{
    RECT rc;
    POINT pt;
    CANDIDATEFORM caf;

    MyDebugPrint((TEXT("MoveCandWindow\r\n")));
    if (fForceComp)
    {
        if (GetCandPosFromCompForm(lpIMC, lpUIExtra, &pt)) {
            caf.dwIndex        = 0;
            caf.dwStyle        = CFS_CANDIDATEPOS;
            caf.ptCurrentPos.x = pt.x;
            caf.ptCurrentPos.y = pt.y;

            MyDebugPrint((TEXT(" *** MoveCandWindow #1\r\n")));
            MyDebugPrint((TEXT(" *** Cand dwStyle: %x\r\n"), lpIMC->cfCandForm[0].dwStyle));
#if 0
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            if ((int)rc.left != -1 && (int)rc.left < 4096 ) {
                POINT spt;
                spt.x = (int)rc.left;
                spt.y = (int)rc.top;
                MyDebugPrint((TEXT("S X,Y:%d,%d\n"),spt.x,spt.y));
                ScreenToClient(lpIMC->hWnd, &spt);
                MyDebugPrint((TEXT("C X,Y:%d,%d\n"),spt.x,spt.y));

                caf.ptCurrentPos.x = spt.x;
                caf.ptCurrentPos.y = spt.y;
            }
#endif
            ImmSetCandidateWindow(lpUIExtra->hIMC,&caf);
        }
        return;
    }

    // Not initialized !!
    if (lpIMC->cfCandForm[0].dwIndex == -1)
    {
        MyDebugPrint((TEXT(" *** MoveCandWindow #2\r\n")));
        if (!GetCandPosFromCompWnd(lpUIExtra,&pt))
        {
            PIMECHARPOSITION lpCP;
            DWORD dwSize = sizeof(IMECHARPOSITION);
        
            lpCP = (PIMECHARPOSITION)GlobalAlloc(GPTR, dwSize);
            lpCP->dwSize = dwSize;
        
            // 어떤 어플리케이션은 CompWindow를 가지고 있지 않다.
            // 워드패드가 대표적인 케이스
            // 이 경우 IMR_QUERYCHARPOSITION을 사용한다.
            if (dwSize = (DWORD) MyImmRequestMessage(lpUIExtra->hIMC, IMR_QUERYCHARPOSITION, (LPARAM)lpCP)) {
                pt.x = lpCP->pt.x;
                pt.y = lpCP->pt.y + lpCP->cLineHeight + 1; // 글자 높이 + 1
                MyDebugPrint((TEXT("Cand pt.x :%d\n"),pt.x));
            } else {
                // IMR_QUERYCHARPOSITION도 지원하지 않으면 root window 스타일로.
                GetWindowRect(lpIMC->hWnd,&rc);
                lpUIExtra->uiCand.pt.x = rc.left;
                lpUIExtra->uiCand.pt.y = rc.bottom;

                pt.x = rc.left;
                pt.y = rc.bottom + 24; // 추정 높이 24 XXX
            }
            GlobalFree(lpCP);
        }
        lpUIExtra->uiCand.pt.x = pt.x;
        lpUIExtra->uiCand.pt.y = pt.y;

        {
            MyDebugPrint((TEXT(" *** pt %dx%d\r\n"),pt.x,pt.y));
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            MoveWindow(lpUIExtra->uiCand.hWnd,pt.x,pt.y, rc.right - rc.left ,rc.bottom - rc.top ,TRUE);
            ShowWindow(lpUIExtra->uiCand.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiCand.bShow = TRUE;
            InvalidateRect(lpUIExtra->uiCand.hWnd,NULL,FALSE);

            SendMessage(hUIWnd,WM_UI_CANDMOVE, 0,MAKELONG((WORD)pt.x,(WORD)pt.y-20));
        }
        return;
    }

    if (!IsCandidate(lpIMC))
        return;

    if (lpIMC->cfCandForm[0].dwStyle == CFS_EXCLUDE)
    {
        RECT rcWork;
        RECT rcAppWnd;

        MyDebugPrint((TEXT(" *** MoveCandWindow #3\r\n")));
        SystemParametersInfo(SPI_GETWORKAREA,0,&rcWork,FALSE);
        // patch from skkime
        if (IsWindow(lpUIExtra->uiCand.hWnd)) {
            GetClientRect(lpUIExtra->uiCand.hWnd,&rc);
        } else {
            rc.left = rc.right = rc.top = rc.bottom = 0;
        }
        GetWindowRect(lpIMC->hWnd,&rcAppWnd);

        if (!lpUIExtra->bVertical)
        {
            pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
            pt.y = lpIMC->cfCandForm[0].rcArea.bottom;
            ClientToScreen(lpIMC->hWnd,&pt);

            if (pt.y + rc.bottom > rcWork.bottom)
                pt.y = rcAppWnd.top + 
                       lpIMC->cfCandForm[0].rcArea.top - rc.bottom;
        }
        else
        {
            pt.x = lpIMC->cfCandForm[0].rcArea.left - rc.right;
            pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
            ClientToScreen(lpIMC->hWnd,&pt);

            if (pt.x < 0)
                pt.x = rcAppWnd.left + 
                       lpIMC->cfCandForm[0].rcArea.right;
        }

        
        if (IsWindow(lpUIExtra->uiCand.hWnd))
        {
            MyDebugPrint((TEXT(" *** MoveCandWindow #4\r\n")));
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            MoveWindow(lpUIExtra->uiCand.hWnd,pt.x,pt.y, rc.right - rc.left ,rc.bottom - rc.top ,TRUE);
            ShowWindow(lpUIExtra->uiCand.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiCand.bShow = TRUE;
            InvalidateRect(lpUIExtra->uiCand.hWnd,NULL,FALSE);

        }
        MyDebugPrint((TEXT(" *** MoveCandWindow #5\r\n")));
        SendMessage(hUIWnd,WM_UI_CANDMOVE, 0,MAKELONG((WORD)pt.x,(WORD)pt.y));
        return;
    } 
    else if (lpIMC->cfCandForm[0].dwStyle == CFS_CANDIDATEPOS)
    {
        MyDebugPrint((TEXT(" *** MoveCandWindow #6\r\n")));
        pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
        pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
        MyDebugPrint((TEXT("C X,Y:%d,%d\n"),pt.x,pt.y));
        ClientToScreen(lpIMC->hWnd,&pt);
        MyDebugPrint((TEXT("S X,Y:%d,%d\n"),pt.x,pt.y));
    } else {
        MyDebugPrint((TEXT(" *** MoveCandWindow #7\r\n")));
        MyDebugPrint((TEXT(" *** Cand dwStyle %x\r\n"), lpIMC->cfCandForm[0].dwStyle));
        if (!GetCandPosFromCompWnd(lpUIExtra,&pt))
        {
            PIMECHARPOSITION lpCP;
            DWORD dwSize = sizeof(IMECHARPOSITION);
        
            lpCP = (PIMECHARPOSITION)GlobalAlloc(GPTR, dwSize);
            lpCP->dwSize = dwSize;
            MyDebugPrint((TEXT("Cand dwSize :%d\n"),dwSize));
 
            // 워드패드 같은 경우는 IMR_QUERYCHARPOSITION을 지원한다.
            if (dwSize = (DWORD) MyImmRequestMessage(lpUIExtra->hIMC, IMR_QUERYCHARPOSITION, (LPARAM)lpCP)) {
                pt.x = lpCP->pt.x;
                pt.y = lpCP->pt.y + lpCP->cLineHeight + 1;
                MyDebugPrint((TEXT("Cand pt.x :%d\n"),pt.x));
            } else {
                // 어떤 어플리케이션은 CompWindow를 가지고 있지 않다.
                // 워드패드가 대표적인 케이스
                GetWindowRect(lpIMC->hWnd,&rc);
                lpUIExtra->uiCand.pt.x = rc.left;
                lpUIExtra->uiCand.pt.y = rc.bottom;

                pt.x = rc.left;
                pt.y = rc.bottom + 24; // 24는 추정 높이.
            }
            GlobalFree(lpCP);
        }
        //ScreenToClient(lpIMC->hWnd,&pt);
        //ClientToScreen(lpIMC->hWnd,&pt);
    }

    // moved and fixed ?
    if (dwFixing && IsWindow(lpUIExtra->uiCand.hWnd)) {
        GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
        pt.x = rc.left;
        pt.y = rc.top;
    }

    {
        if (IsWindow(lpUIExtra->uiCand.hWnd))
        {
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            MoveWindow(lpUIExtra->uiCand.hWnd,pt.x,pt.y, rc.right - rc.left ,rc.bottom - rc.top ,TRUE);
            ShowWindow(lpUIExtra->uiCand.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiCand.bShow = TRUE;
            InvalidateRect(lpUIExtra->uiCand.hWnd,NULL,FALSE);

        }
        SendMessage(hUIWnd,WM_UI_CANDMOVE, 0,MAKELONG((WORD)pt.x,(WORD)pt.y));
        MyDebugPrint((TEXT(" *** MoveCandWindow: dwStyle: %x\r\n"),
                lpIMC->cfCandForm[0].dwStyle));
    }
    MyDebugPrint((TEXT(" *** MoveCandWindow Fail\r\n")));
    MyDebugPrint((TEXT(" *** MoveCandWindow: dwStyle: %x\r\n"),
                lpIMC->cfCandForm[0].dwStyle));
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
