/*
 * This file is part of Saenaru.
 *
 * Copyright (C) 2003 Hye-Shik Chang. All rights reserved.
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
 * $Perky$
 */
/*++

Copyright (c) 1990-1998 Microsoft Corporation, All Rights Reserved

Module Name:

    UICAND.C
    
++*/

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
            PaintCandWindow(hWnd);
            break;

        case WM_SETCURSOR:
#if (WINVER >= 0x0500)
            SetCursor(LoadCursor(NULL,IDC_HAND));
#endif
            if (dy == 0)
            {
                TEXTMETRIC tm;
                HDC hDC;

                hDC = CreateIC(TEXT("DISPLAY"),NULL,NULL,NULL);
                GetTextMetrics(hDC,&tm);
                dy = tm.tmHeight + tm.tmExternalLeading;
                DeleteDC(hDC);
            }

            select = 0;
            offset = 0;
            {
                GetCursorPos(&pt);
                ScreenToClient(hWnd,&pt);
                GetClientRect(hWnd,&rc);
                if (!PtInRect(&rc,pt))
                    return 0;

                pt.y -= GetSystemMetrics(SM_CYEDGE);
                if (pt.y > 0)
                    select = pt.y / dy + ((pt.y % dy) ? 1:0);
                if (select == 0)
                    offset = -2;
            }
            if ((HIWORD(lParam) != WM_LBUTTONDOWN) &&
                (HIWORD(lParam) != WM_RBUTTONDOWN)) 
                 return DefWindowProc(hWnd,message,wParam,lParam);
            if ((HIWORD(lParam) == WM_LBUTTONDOWN))
            {
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
                    lpIMC = ImmLockIMC(hIMC);
                    ConvHanja(hIMC,offset,select);
	            ImmUnlockIMC(hIMC);
                    break;
                }
            }
        case WM_MOUSEMOVE:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            DragUI(hWnd,message,wParam,lParam);
            if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP)) {
	        MyDebugPrint((TEXT(" $ CANDMOVE %dx%d\n"),LOWORD(lParam),HIWORD(lParam)));
                SetWindowLong(hWnd,FIGWL_MOUSE,0L);
	    }
            break;

        case WM_MOVE:
            hUIWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);

	    MyDebugPrint((TEXT(" x CANDMOVE %dx%d\n"),LOWORD(lParam),HIWORD(lParam)));
            if (IsWindow(hUIWnd))
                SendMessage(hUIWnd,WM_UI_CANDMOVE,wParam,lParam);
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
            GetWindowRect(lpUIExtra->uiComp[0].hWnd,&rc);
            lppt->x = rc.left;
            lppt->y = rc.bottom+1;
            return TRUE;
        }
    }
    else
    {
        if (lpUIExtra->uiDefComp.bShow)
        {
            GetWindowRect(lpUIExtra->uiDefComp.hWnd,&rc);
            lppt->x = rc.left;
            lppt->y = rc.bottom+1;
            return TRUE;
        }
    }
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
		MyDebugPrint((TEXT("XXX #1\n")));
            }
            else
            {
                lppt->x = lpIMC->cfCompForm.ptCurrentPos.x -
                                     GetCompFontHeight(lpUIExtra);
                lppt->y = lpIMC->cfCompForm.ptCurrentPos.y;
            }
            return TRUE;
        }
    }
    else
    {
        if (GetCandPosFromCompWnd(lpUIExtra,lppt))
        {
            ScreenToClient(lpIMC->hWnd,lppt);
            return TRUE;
        }
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

    if (GetCandPosFromCompWnd(lpUIExtra,&pt))
    {
        lpUIExtra->uiCand.pt.x = pt.x;
        lpUIExtra->uiCand.pt.y = pt.y;
    }

    if (!IsWindow(lpUIExtra->uiCand.hWnd))
    {
        lpUIExtra->uiCand.hWnd = 
                CreateWindowEx(WS_EX_WINDOWEDGE,
                             (LPTSTR)szCandClassName,NULL,
                             WS_COMPNODEFAULT,
                             //WS_COMPDEFAULT | WS_DLGFRAME,
                             lpUIExtra->uiCand.pt.x,
                             lpUIExtra->uiCand.pt.y,
                             1,1,
                             hUIWnd,NULL,hInst,NULL);
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
    RECT rc;
    LPMYSTR lpstr;
    int height;
    DWORD i;
    SIZE sz;
    HWND hSvrWnd;
    HBRUSH hbrHightLight = CreateSolidBrush(GetSysColor(COLOR_HIGHLIGHT));
#if 0
    HBRUSH hbrLGR = GetStockObject(LTGRAY_BRUSH);
#else
    HBRUSH hbrLGR = (HBRUSH)(COLOR_BTNFACE + 1);
#endif
    HFONT hOldFont = NULL;

    GetClientRect(hCandWnd,&rc);
    hDC = BeginPaint(hCandWnd,&ps);
    FillRect(hDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));
    SetBkMode(hDC,TRANSPARENT);

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
		TCHAR num[3];
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
                }
                else
                {
                    hbr = SelectObject(hDC,hbrLGR);
                    //PatBlt(hDC,0,height,rc.right,sz.cy,PATCOPY);
                    SelectObject(hDC,hbr);
                    SetTextColor(hDC,RGB(0,0,0));
                }
		if (i < lpCandList->dwCount)
                    MyTextOut(hDC,GetSystemMetrics(SM_CXEDGE),height,num,Mylstrlen(num));
                MyTextOut(hDC,10 + GetSystemMetrics(SM_CXEDGE),height,lpstr,Mylstrlen(lpstr));
                height += sz.cy;
            }
            ImmUnlockIMCC(lpIMC->hCandInfo);
        }
        if (hOldFont) {
            DeleteObject(SelectObject(hDC, hOldFont));
        }
        ImmUnlockIMC(hIMC);
    }
    EndPaint(hCandWnd,&ps);

    DeleteObject(hbrHightLight);
}

/**********************************************************************/
/*                                                                    */
/* ResizeCandWindow()                                                   */
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
                height += sz.cy;
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
                       width+ 10 + 4 * GetSystemMetrics(SM_CXEDGE),
		       /* 10 is a left margin */
                       height+ 4 * GetSystemMetrics(SM_CYEDGE),
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

    if (fForceComp)
    {
        if (GetCandPosFromCompForm(lpIMC, lpUIExtra, &pt))
        {
            caf.dwIndex        = 0;
            caf.dwStyle        = CFS_CANDIDATEPOS;
            caf.ptCurrentPos.x = pt.x;
            caf.ptCurrentPos.y = pt.y;
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
        if (GetCandPosFromCompWnd(lpUIExtra,&pt))
        {
            lpUIExtra->uiCand.pt.x = pt.x;
            lpUIExtra->uiCand.pt.y = pt.y;
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            MoveWindow(lpUIExtra->uiCand.hWnd,pt.x,pt.y, rc.right - rc.left ,rc.bottom - rc.top ,TRUE);
            ShowWindow(lpUIExtra->uiCand.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiCand.bShow = TRUE;
            InvalidateRect(lpUIExtra->uiCand.hWnd,NULL,FALSE);

            SendMessage(hUIWnd,WM_UI_CANDMOVE, 0,MAKELONG((WORD)pt.x,(WORD)pt.y));
        }
        return;
    }

    if (!IsCandidate(lpIMC))
        return;

    if (lpIMC->cfCandForm[0].dwStyle == CFS_EXCLUDE)
    {
        RECT rcWork;
        RECT rcAppWnd;

        SystemParametersInfo(SPI_GETWORKAREA,0,&rcWork,FALSE);
        GetClientRect(lpUIExtra->uiCand.hWnd,&rc);
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
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            MoveWindow(lpUIExtra->uiCand.hWnd,pt.x,pt.y, rc.right - rc.left ,rc.bottom - rc.top ,TRUE);
            ShowWindow(lpUIExtra->uiCand.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiCand.bShow = TRUE;
            InvalidateRect(lpUIExtra->uiCand.hWnd,NULL,FALSE);

        }
        SendMessage(hUIWnd,WM_UI_CANDMOVE, 0,MAKELONG((WORD)pt.x,(WORD)pt.y));
    } 
    else if (lpIMC->cfCandForm[0].dwStyle == CFS_CANDIDATEPOS)
    {
        pt.x = lpIMC->cfCandForm[0].ptCurrentPos.x;
        pt.y = lpIMC->cfCandForm[0].ptCurrentPos.y;
        ClientToScreen(lpIMC->hWnd,&pt);
        
        if (IsWindow(lpUIExtra->uiCand.hWnd))
        {
            GetWindowRect(lpUIExtra->uiCand.hWnd,&rc);
            MoveWindow(lpUIExtra->uiCand.hWnd,pt.x,pt.y, rc.right - rc.left ,rc.bottom - rc.top ,TRUE);
            ShowWindow(lpUIExtra->uiCand.hWnd,SW_SHOWNOACTIVATE);
            lpUIExtra->uiCand.bShow = TRUE;
            InvalidateRect(lpUIExtra->uiCand.hWnd,NULL,FALSE);

        }
        SendMessage(hUIWnd,WM_UI_CANDMOVE, 0,MAKELONG((WORD)pt.x,(WORD)pt.y));
    }
}

