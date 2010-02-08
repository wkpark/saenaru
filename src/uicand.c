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
#include "immdev.h"
#include "saenaru.h"

int PASCAL GetCompFontHeight(LPUIEXTRA lpUIExtra);

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
#if 1
    HWND hSvrWnd;
    HIMC hIMC;
    LPINPUTCONTEXT lpIMC;
    static INT dy=0;
#endif
    DWORD dwPushedCand;
    POINT pt;
    RECT rc;
    INT select;
    INT offset;

    switch (message)
    {
        case WM_PAINT:
            MyDebugPrint((TEXT(" *** WM_PAINT\r\n")));
            PaintCandWindow(hWnd);
            break;
        case WM_CREATE:
            MyDebugPrint((TEXT(" * WM_CREATE Cand\r\n")));
            break;
        case WM_MOUSEACTIVATE:
            MyDebugPrint((TEXT(" * WM_MOUSEACTIVATE\r\n")));

            return MA_NOACTIVATE;
            //return MA_NOACTIVATEANDEAT;
            break;
        case WM_DESTROY:
            MyDebugPrint((TEXT(" * WM_DESTORY Cand\r\n")));
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
            while (HIWORD(lParam) == WM_LBUTTONDOWN) {
                HDC hDC;
                PAINTSTRUCT ps;
                GetCursorPos(&pt);
                ScreenToClient(hWnd,&pt);
                GetClientRect(hWnd,&rc);

                if (!PtInRect(&rc,pt)) {
                    // not in the candlist wind
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
                MyDebugPrint((TEXT("Cand offset %d\n"),offset));
                hSvrWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);
                if (select > 0 &&
                    (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC)) )
                {
                    ConvHanja(hIMC,offset,select);
                }
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
    RECT rc,rc2;
    LPMYSTR lpstr;
    int height;
    DWORD i;
    SIZE sz;
    HWND hSvrWnd;
    HBRUSH hbrHightLight = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
    HBRUSH hbrLGR = (HBRUSH)(COLOR_BTNFACE + 1);
    HFONT hOldFont = NULL;
    HBRUSH hBrush;

    LOGFONT lfFont;
    HFONT hNumFont;

    GetClientRect(hCandWnd,&rc);
    rc2.left=rc.left+25; // left margin: 25
    rc2.right=rc.right - 1;
    rc2.top=rc.top+1;
    rc2.bottom=rc.bottom- 8 * GetSystemMetrics(SM_CYEDGE); // bottom space: 8
    hDC = BeginPaint(hCandWnd,&ps);
    hBrush=CreateSolidBrush(GetBkColor(hDC));
    FillRect(hDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));
    FillRect(hDC,&rc2,hBrush); // fill text area
    SetBkMode(hDC,TRANSPARENT);

    // set number font
    hOldFont = GetCurrentObject(hDC, OBJ_FONT);
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
        hOldFont = CheckNativeCharset(hDC);
        if (lpCandInfo = (LPCANDIDATEINFO)ImmLockIMCC(lpIMC->hCandInfo))
        {
            height = GetSystemMetrics(SM_CYEDGE); 
            lpCandList = (LPCANDIDATELIST)((LPSTR)lpCandInfo  + lpCandInfo->dwOffset[0]);
            for (i = lpCandList->dwPageStart; 
                 i < (lpCandList->dwPageStart + lpCandList->dwPageSize); i++)
            {
                TCHAR num[4];
                INT wtype; // KSX 1002 set or not
                INT highlighted;
                wtype=0;
                highlighted=0;
                wsprintf(num, TEXT("%d "), i % lpCandList->dwPageSize + 1);

                if (i >= lpCandList->dwCount) 
                    lpstr=(LPMYSTR)(LPSTR)" ";
                else
                    lpstr=(LPMYSTR)((LPSTR)lpCandList +lpCandList->dwOffset[i]);

                MyGetTextExtentPoint(hDC,lpstr,Mylstrlen(lpstr),&sz);
                if (((LPMYCAND)lpCandInfo)->cl.dwSelection == (DWORD)i)
                {
                    hbr = SelectObject(hDC,hbrHightLight);
                    PatBlt(hDC,0,height,rc.right,sz.cy,PATCOPY);
                    SelectObject(hDC,hbr);
                    SetTextColor(hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
                    highlighted=1;
                }
                else
                {
                    hbr = SelectObject(hDC,hbrLGR);
                    //PatBlt(hDC,0,height,rc.right,sz.cy,PATCOPY);
                    SelectObject(hDC,hbr);
                    SetTextColor(hDC,RGB(0,0,0));
                }
                if (i < lpCandList->dwCount) {
                    MyTextOut(hDC,5 + GetSystemMetrics(SM_CXEDGE),height,num,Mylstrlen(num));
                    //MyTextOut(hDC,6 + GetSystemMetrics(SM_CXEDGE),height,num,Mylstrlen(num));
                }
                // 한 글자이면서 KSX1002 지원이 아니면 charset 체크
                if (wtype != -1 && dwOptionFlag & KSX1002_SUPPORT)
                {
                    WORD mb;
                    WideCharToMultiByte(949, WC_COMPOSITECHECK,
                        lpstr, 1, (char *)&mb, 2, NULL, NULL);

                    if(LOBYTE(mb) < 0xa1 || LOBYTE(mb) > 0xfe 
                        || HIBYTE(mb) < 0xa1 || HIBYTE(mb) > 0xfe)
                        wtype=1;
                    // XXX 한중일 통합을 동시에 지원할 경우
                    // KS X 1002를 처리하기 위한 특별 루틴 필요?
                }
                if (wtype) {
                    if (highlighted)
                        SetTextColor(hDC,RGB(0,0,0)); // shadow
                    else
                        SetTextColor(hDC,RGB(220,220,220));
                    MyTextOut(hDC,25 + 1 + GetSystemMetrics(SM_CXEDGE),
                            height + 1,lpstr, Mylstrlen(lpstr));
                }
                // 완성형 4888자가 아닌 경우 색을 청색조로.
                if (highlighted)
                    SetTextColor(hDC,GetSysColor(COLOR_HIGHLIGHTTEXT));
                else if (wtype==1)
                    SetTextColor(hDC,RGB(16,83,239));

                MyTextOut(hDC,25 + GetSystemMetrics(SM_CXEDGE),height,lpstr,
                    Mylstrlen(lpstr));
                // 25 is left margin of candwin
                height += sz.cy + 4;
                // 4 is linespace
            }
            ImmUnlockIMCC(lpIMC->hCandInfo);
        }

        if (hOldFont)
            DeleteObject(SelectObject(hDC, hOldFont));

        if (hNumFont)
            DeleteObject(SelectObject(hDC, hNumFont));

        ImmUnlockIMC(hIMC);
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

        SetTextColor(hDC,RGB(0,0,0));

        wsprintf(num, TEXT("%d/%d"), page, pages);

        SelectObject(hDC, hNumFont);
        MyTextOut(hDC,rc.left + 10 - Mylstrlen(num),rc.bottom - 15 ,num,Mylstrlen(num));

        //pos= GetScrollPos(hCandWnd,SB_VERT);
        SetScrollPos(hCandWnd,SB_VERT,pos,TRUE);
    }

    EndPaint(hCandWnd,&ps);

    DeleteDC(hDC);
    DeleteObject(hbrHightLight);
    DeleteObject(hBrush);

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
        HFONT hOldFont;

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
                lpstr = (LPMYSTR)((LPSTR)lpCandList + lpCandList->dwOffset[i]);
                MyGetTextExtentPoint(hDC,lpstr,Mylstrlen(lpstr),&sz);
                if (width < sz.cx)
                    width = sz.cx;
                height += sz.cy + 4; // plus linespace 4
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
                       width+ 25 + 4 + 4 * GetSystemMetrics(SM_CXEDGE) +
                       GetSystemMetrics(SM_CXVSCROLL),
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

    if (IsWindow(lpUIExtra->uiCand.hWnd))
    {
        GetWindowRect(lpUIExtra->uiCand.hWnd,(LPRECT)&rc);
        lpUIExtra->uiCand.pt.x = rc.left;
        lpUIExtra->uiCand.pt.y = rc.top;
        MoveWindow(lpUIExtra->uiCand.hWnd, -1 , -1 , 0 , 0, TRUE);
        ShowWindow(lpUIExtra->uiCand.hWnd, SW_HIDE);
        lpUIExtra->uiCand.bShow = FALSE;
    }
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
