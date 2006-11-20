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
 * $Saenaru: saenaru/src/uistate.c,v 1.4 2006/11/17 08:55:28 wkpark Exp $
 */

/**********************************************************************/
#include "windows.h"
#include "immdev.h"
#include "saenaru.h"
#include "resource.h"

/**********************************************************************/
/*                                                                    */
/* StatusWndProc()                                                    */
/* IME UI window procedure                                            */
/**********************************************************************/
// 상태창 : 별도의 상태창이 만들어지므로 이 기능을 꺼둔 상태 wkpark
LRESULT CALLBACK StatusWndProc( hWnd, message, wParam, lParam )
HWND hWnd;
UINT message;
WPARAM wParam;
LPARAM lParam;
{
    PAINTSTRUCT ps;
    HWND hUIWnd;
    HDC hDC;
    HBITMAP hbmpStatus;

    switch (message)
    {
        case WM_UI_UPDATE:
            InvalidateRect(hWnd,NULL,FALSE);
            break;

        case WM_PAINT:
            hDC = BeginPaint(hWnd,&ps);
            PaintStatus(hWnd,hDC,NULL,0);
            EndPaint(hWnd,&ps);
            break;

        case WM_MOUSEMOVE:
        case WM_SETCURSOR:
        case WM_LBUTTONUP:
        case WM_RBUTTONUP:
            ButtonStatus(hWnd,message,wParam,lParam);
            if ((message == WM_SETCURSOR) &&
                (HIWORD(lParam) != WM_LBUTTONDOWN) &&
                (HIWORD(lParam) != WM_RBUTTONDOWN)) 
                return DefWindowProc(hWnd,message,wParam,lParam);
            if ((message == WM_LBUTTONUP) || (message == WM_RBUTTONUP))
            {
                SetWindowLong(hWnd,FIGWL_MOUSE,0L);
                SetWindowLong(hWnd,FIGWL_PUSHSTATUS,0L);
            }
            break;

        case WM_MOVE:
            hUIWnd = (HWND)GetWindowLongPtr(hWnd,FIGWL_SVRWND);
            if (IsWindow(hUIWnd))
                SendMessage(hUIWnd,WM_UI_STATEMOVE,wParam,lParam);
            break;

        case WM_CREATE:
            hbmpStatus = LoadBitmap(hInst,TEXT("STATUSBMP"));
/*
            hbmpStatus =(HBITMAP)LoadImage(hInst,TEXT("STATUSBMP"),IMAGE_BITMAP,
                    0,0,LR_LOADTRANSPARENT);
                    */

            SetWindowLongPtr(hWnd,FIGWL_STATUSBMP,(LONG_PTR)hbmpStatus);
            hbmpStatus = LoadBitmap(hInst,TEXT("CLOSEBMP"));
            SetWindowLongPtr(hWnd,FIGWL_CLOSEBMP,(LONG_PTR)hbmpStatus);
            break;

        case WM_DESTROY:
            hbmpStatus = (HBITMAP)GetWindowLongPtr(hWnd,FIGWL_STATUSBMP);
            DeleteObject(hbmpStatus);
            hbmpStatus = (HBITMAP)GetWindowLongPtr(hWnd,FIGWL_CLOSEBMP);
            DeleteObject(hbmpStatus);
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
/* CheckPushedStatus()                                                   */
/*                                                                    */
/**********************************************************************/
DWORD PASCAL CheckPushedStatus( HWND hStatusWnd, LPPOINT lppt)
{
    POINT pt;
    RECT rc;

    if (lppt)
    {
        pt = *lppt;
        ScreenToClient(hStatusWnd,&pt);
        GetClientRect(hStatusWnd,&rc);
        if (!PtInRect(&rc,pt))
            return 0;

        if (pt.y > 0)
        {
            if (pt.x < BTX)
                return PUSHED_STATUS_HDR;
            else if (pt.x < (BTX * 2))
                return PUSHED_STATUS_MODE;
#if 0
            else if (pt.x < (BTX * 3))
                return PUSHED_STATUS_ROMAN;
#endif
        }
    }
    return 0;
}

/**********************************************************************/
/*                                                                    */
/* BTXFromCmode()                                                     */
/*                                                                    */
/**********************************************************************/
int PASCAL BTXFromCmode( DWORD dwCmode)
{
    if (dwCmode & IME_CMODE_FULLSHAPE)
    {
        return 80;
    }
    else
    {
        return 60;
    }

}

void DrawTransBtn(HDC hdc, HDC hdcTemp, int xStart, int yStart,
        COLORREF cTransparentColor)
{
   BITMAP     bm;
   COLORREF   cColor;
   HBITMAP    bmAndBack, bmAndObject, bmAndMem, bmSave;
   HBITMAP    bmBackOld, bmObjectOld, bmMemOld, bmSaveOld;
   HDC        hdcMem, hdcBack, hdcObject, hdcSave;
   POINT      ptSize;
/*
   hdcTemp = CreateCompatibleDC(hdc);
   SelectObject(hdcTemp, hBitmap);   // Select the bitmap

   GetObject(hBitmap, sizeof(BITMAP), (LPSTR)&bm);
   ptSize.x = bm.bmWidth;            // Get width of bitmap
   ptSize.y = bm.bmHeight;           // Get height of bitmap
   */
   ptSize.x = BTX;            // Get width of bitmap
   ptSize.y = BTY;            // Get height of bitmap
   DPtoLP(hdcTemp, &ptSize, 1);      // Convert from device
                                     // to logical points

   // Create some DCs to hold temporary data.
   hdcBack   = CreateCompatibleDC(hdc);
   hdcObject = CreateCompatibleDC(hdc);
   hdcMem    = CreateCompatibleDC(hdc);
   hdcSave   = CreateCompatibleDC(hdc);

   // Create a bitmap for each DC. DCs are required for a number of
   // GDI functions.

   // Monochrome DC
   bmAndBack   = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

   // Monochrome DC
   bmAndObject = CreateBitmap(ptSize.x, ptSize.y, 1, 1, NULL);

   bmAndMem    = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);
   bmSave      = CreateCompatibleBitmap(hdc, ptSize.x, ptSize.y);

   // Each DC must select a bitmap object to store pixel data.
   bmBackOld   = SelectObject(hdcBack, bmAndBack);
   bmObjectOld = SelectObject(hdcObject, bmAndObject);
   bmMemOld    = SelectObject(hdcMem, bmAndMem);
   bmSaveOld   = SelectObject(hdcSave, bmSave);

   // Set proper mapping mode.
   SetMapMode(hdcTemp, GetMapMode(hdc));

   // Save the bitmap sent here, because it will be overwritten.
   BitBlt(hdcSave, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCCOPY);

   // Set the background color of the source DC to the color.
   // contained in the parts of the bitmap that should be transparent
   cColor = SetBkColor(hdcTemp, cTransparentColor);

   // Create the object mask for the bitmap by performing a BitBlt
   // from the source bitmap to a monochrome bitmap.
   BitBlt(hdcObject, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0,
          SRCCOPY);

   // Set the background color of the source DC back to the original
   // color.
   SetBkColor(hdcTemp, cColor);

   // Create the inverse of the object mask.
   BitBlt(hdcBack, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0,
          NOTSRCCOPY);

   // Copy the background of the main DC to the destination.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdc, xStart, yStart,
          SRCCOPY);

   // Mask out the places where the bitmap will be placed.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcObject, 0, 0, SRCAND);

   // Mask out the transparent colored pixels on the bitmap.
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcBack, 0, 0, SRCAND);

   // XOR the bitmap with the background on the destination DC.
   BitBlt(hdcMem, 0, 0, ptSize.x, ptSize.y, hdcTemp, 0, 0, SRCPAINT);

   // Copy the destination to the screen.
   BitBlt(hdc, xStart, yStart, ptSize.x, ptSize.y, hdcMem, 0, 0,
          SRCCOPY);

   // Place the original bitmap back into the bitmap sent here.
   BitBlt(hdcTemp, 0, 0, ptSize.x, ptSize.y, hdcSave, 0, 0, SRCCOPY);

   // Delete the memory bitmaps.
   DeleteObject(SelectObject(hdcBack, bmBackOld));
   DeleteObject(SelectObject(hdcObject, bmObjectOld));
   DeleteObject(SelectObject(hdcMem, bmMemOld));
   DeleteObject(SelectObject(hdcSave, bmSaveOld));

   // Delete the memory DCs.
   DeleteDC(hdcMem);
   DeleteDC(hdcBack);
   DeleteDC(hdcObject);
   DeleteDC(hdcSave);
   //DeleteDC(hdcTemp);
}


/**********************************************************************/
/*                                                                    */
/* PaintStatus()                                                      */
/*                                                                    */
/**********************************************************************/
void PASCAL PaintStatus( HWND hStatusWnd , HDC hDC, LPPOINT lppt, DWORD dwPushedStatus)
{
    HIMC hIMC;
    LPINPUTCONTEXT lpIMC;
    HDC hMemDC,hTempDC,hNewDC;
    HBITMAP hbmpOld;
    int x;
    HWND hSvrWnd;
    HBITMAP bmTemp,bmTempOld;

    hSvrWnd = (HWND)GetWindowLongPtr(hStatusWnd,FIGWL_SVRWND);

    if (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC))
    {
        HBITMAP hbmpStatus;
        HBRUSH hOldBrush,hBrush;
        COLORREF cColor,cTransColor;
//      int nCyCap = GetSystemMetrics(SM_CYSMCAPTION);
        int nCyCap = 0;
        RECT rc;

        lpIMC = ImmLockIMC(hIMC);
        hMemDC = CreateCompatibleDC(hDC);

        hTempDC = CreateCompatibleDC(hDC);
        hNewDC = CreateCompatibleDC(hDC);

#if 0
        // Paint Caption.
        hBrush = CreateSolidBrush(GetSysColor(COLOR_ACTIVECAPTION));
        hOldBrush = SelectObject(hDC,hBrush);
        rc.top = rc.left = 0;
        rc.right = BTX*3;
        rc.bottom = nCyCap;
        FillRect(hDC,&rc,hBrush);
        SelectObject(hDC,hOldBrush);
        DeleteObject(hBrush);

        // Paint CloseButton.
        hbmpStatus = (HBITMAP)GetWindowLongPtr(hStatusWnd,FIGWL_CLOSEBMP);
        hbmpOld = SelectObject(hMemDC,hbmpStatus);

        if (!(dwPushedStatus & PUSHED_STATUS_CLOSE))
            BitBlt(hDC,STCLBT_X,STCLBT_Y,STCLBT_DX,STCLBT_DY,
                   hMemDC,0,0,SRCCOPY);
        else
            BitBlt(hDC,STCLBT_X,STCLBT_Y,STCLBT_DX,STCLBT_DY,
                   hMemDC,STCLBT_DX,0,SRCCOPY);
#endif

        hbmpStatus = (HBITMAP)GetWindowLongPtr(hStatusWnd,FIGWL_STATUSBMP);
        hbmpOld = SelectObject(hMemDC,hbmpStatus);
        cTransColor= RGB(192,192,192); // XXX

        cColor= SetBkColor(hTempDC,cTransColor);
        SetBkMode(hTempDC,TRANSPARENT);

        // Paint HDR.
        x = BTEMPT;
        if (lpIMC->fOpen) {
            if (lpIMC->fdwConversion & IME_CMODE_HANJACONVERT)
                x = 40;
            else if (lpIMC->fdwConversion & IME_CMODE_NATIVE)
                x = 0;
        }

        bmTemp = CreateCompatibleBitmap(hDC, BTX, BTY);
        bmTempOld = SelectObject(hTempDC, bmTemp);

        // clear
        rc.top = rc.left = 0;
        rc.right = BTX*2;
        rc.bottom = BTY;
        FillRect(hDC,&rc,(HBRUSH) (COLOR_BTNFACE + 1));

        if (!(dwPushedStatus & PUSHED_STATUS_HDR)) {
            BitBlt(hTempDC,0,0,BTX,BTY,hMemDC,x,0,SRCCOPY);
            DrawTransBtn(hDC,hTempDC,0,0,cTransColor);
        }
        else {
            BitBlt(hTempDC,0,0,BTX,BTY,hMemDC,x,BTY,SRCCOPY);
            DrawTransBtn(hDC,hTempDC,0,0,cTransColor);
        }

        // Paint MODE.
        x = BTXFromCmode(lpIMC->fdwConversion);

        if (!(dwPushedStatus & PUSHED_STATUS_MODE)) {
            BitBlt(hTempDC,0,0,BTX,BTY,hMemDC,x,0,SRCCOPY);
            DrawTransBtn(hDC,hTempDC,BTX,nCyCap,cTransColor);
        } else {
            BitBlt(hTempDC,0,0,BTX,BTY,hMemDC,x,BTY,SRCCOPY);
            DrawTransBtn(hDC,hTempDC,BTX,nCyCap,cTransColor);
        }

        DeleteObject(SelectObject(hTempDC, bmTempOld));

        SetBkColor(hDC,cColor);
#if 0
        // Paint Roman MODE.
        x = BTEMPT;
        if (lpIMC->fdwConversion & IME_CMODE_ROMAN)
            x = BTROMA;

        if (!(dwPushedStatus & PUSHED_STATUS_ROMAN))
            BitBlt(hDC,BTX*2,nCyCap,BTX,BTY,hMemDC,x,0,SRCCOPY);
        else
            BitBlt(hDC,BTX*2,nCyCap,BTX,BTY,hMemDC,x,BTY,SRCCOPY);
#endif
        SelectObject(hMemDC,hbmpOld);
        DeleteDC(hMemDC);
        ImmUnlockIMC(hIMC);
    }


}

/**********************************************************************/
/*                                                                    */
/* GetUINextMode(hWnd,message,wParam,lParam)                          */
/*                                                                    */
/**********************************************************************/
DWORD PASCAL GetUINextMode( DWORD fdwConversion, DWORD dwPushed)
{
    BOOL fTemp;
    BOOL fFullShape = ((fdwConversion & IME_CMODE_FULLSHAPE) != 0);

    //
    // When the mode button is pushed,
    // the convmode will be switched as following
    //
    //     FULLSHAPE -> HALFSHAPE
    //     NATIVE -> ROMAN
    //
    if (dwPushed == PUSHED_STATUS_HDR)
    {
        fTemp = fdwConversion & IME_CMODE_NATIVE;
        fdwConversion &= ~IME_CMODE_FULLSHAPE;
        if (fTemp) {
            fdwConversion &= ~IME_CMODE_NATIVE;
        } else {
            fdwConversion |= IME_CMODE_NATIVE;
        }
    } else if (dwPushed == PUSHED_STATUS_MODE)
    {
        if (fFullShape)
        {
            fdwConversion &= ~IME_CMODE_FULLSHAPE;
        }
        else
        {
            fdwConversion |= IME_CMODE_FULLSHAPE;
        }
    }
    return fdwConversion;

}
/**********************************************************************/
/*                                                                    */
/* ButtonStatus(hStatusWnd,message,wParam,lParam)                     */
/*                                                                    */
/**********************************************************************/
void PASCAL ButtonStatus( HWND hStatusWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    POINT     pt;
    HDC hDC;
    DWORD dwMouse;
    DWORD dwPushedStatus;
    DWORD dwTemp=0;
    DWORD fdwConversion;
    HIMC hIMC;
    HWND hSvrWnd;
    BOOL fOpen;
    HMENU hMenu;
    static    POINT ptdif;
    static    RECT drc;
    static    RECT rc;
    static    DWORD dwCurrentPushedStatus;

    HKEY hKey;
    INT i;
    INT idLayout;

    hDC = GetDC(hStatusWnd);
    switch (message)
    {
        case WM_SETCURSOR:
            if ( HIWORD(lParam) == WM_LBUTTONDOWN
                || HIWORD(lParam) == WM_RBUTTONDOWN ) 
            {
                GetCursorPos( &pt );
                SetCapture(hStatusWnd);
                GetWindowRect(hStatusWnd,&drc);
                ptdif.x = pt.x - drc.left;
                ptdif.y = pt.y - drc.top;
                rc = drc;
                rc.right -= rc.left;
                rc.bottom -= rc.top;
                SetWindowLong(hStatusWnd,FIGWL_MOUSE,FIM_CAPUTURED);
                    SetWindowLong(hStatusWnd, FIGWL_PUSHSTATUS, dwPushedStatus = CheckPushedStatus(hStatusWnd,&pt));
                if (HIWORD(lParam) == WM_LBUTTONDOWN) {
                    PaintStatus(hStatusWnd,hDC,&pt, dwPushedStatus);
                }
                dwCurrentPushedStatus = dwPushedStatus;
            }
            break;

        case WM_MOUSEMOVE:
            dwMouse = GetWindowLong(hStatusWnd,FIGWL_MOUSE);
            if (!(dwPushedStatus = GetWindowLong(hStatusWnd, FIGWL_PUSHSTATUS)))
            {
                if (dwMouse & FIM_MOVED)
                {
                    DrawUIBorder(&drc);
                    GetCursorPos( &pt );
                    drc.left   = pt.x - ptdif.x;
                    drc.top    = pt.y - ptdif.y;
                    drc.right  = drc.left + rc.right;
                    drc.bottom = drc.top + rc.bottom;
                    DrawUIBorder(&drc);
                }
                else if (dwMouse & FIM_CAPUTURED)
                {
                    DrawUIBorder(&drc);
                    SetWindowLong(hStatusWnd,FIGWL_MOUSE,dwMouse | FIM_MOVED);
                }
            }
            else
            {
                GetCursorPos(&pt);
                dwTemp = CheckPushedStatus(hStatusWnd,&pt);
                if ((dwTemp ^ dwCurrentPushedStatus) & dwPushedStatus)
                    PaintStatus(hStatusWnd,hDC,&pt, dwPushedStatus & dwTemp);
                dwCurrentPushedStatus = dwTemp;
            }
            break;

        case WM_RBUTTONUP:
            dwMouse = GetWindowLong(hStatusWnd,FIGWL_MOUSE);
            if (dwMouse & FIM_CAPUTURED)
            {
                ReleaseCapture();
                if (dwMouse & FIM_MOVED)
                {
                    DrawUIBorder(&drc);
                    GetCursorPos( &pt );
                    MoveWindow(hStatusWnd,pt.x - ptdif.x,
                                    pt.y - ptdif.y,
                                    rc.right,
                                    rc.bottom,TRUE);
                }
            }
            PaintStatus(hStatusWnd,hDC,NULL,0);

            hSvrWnd = (HWND)GetWindowLongPtr(hStatusWnd,FIGWL_SVRWND);

            hMenu = LoadMenu(hInst, TEXT("RIGHTCLKMENU"));
            idLayout = dwLayoutFlag - LAYOUT_OLD2BUL;
            idLayout += IDM_OLD2BUL;

            if (hMenu && (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC)))
            {
                int cmd;
                POINT pt;
                LPINPUTCONTEXT lpIMC;
                BOOL fOpen;
                INT idMode,idFlag;
                HMENU hSubMenu = GetSubMenu(hMenu, 0);
                MENUITEMINFO mk;

                if (gfSaenaruSecure) {
                    EnableMenuItem(hSubMenu,IDM_CONFIG,MF_GRAYED);
                    EnableMenuItem(hSubMenu,IDM_RECONVERT,MF_GRAYED);
                }

                fOpen = ImmGetOpenStatus(hIMC);
                ImmGetConversionStatus(hIMC,&fdwConversion,&dwTemp);

                idMode=IDM_ENG;
                if (fOpen) {
                    if (fdwConversion & IME_CMODE_HANJACONVERT)
                        idMode=IDM_HANJA;
                    else if (fdwConversion & IME_CMODE_NATIVE)
                        idMode=IDM_HANGUL;
                }

                mk.cbSize = sizeof(MENUITEMINFO);
                // check CMODE
                GetMenuItemInfo(hSubMenu,idMode,FALSE,&mk);
                mk.fMask=MIIM_STATE | MIIM_FTYPE;
                mk.fState=MFS_CHECKED;
                mk.fType=MFT_RADIOCHECK;
                SetMenuItemInfo(hSubMenu,idMode,FALSE,&mk);


                if (fdwConversion & IME_CMODE_SOFTKBD) {
                    idFlag= IDM_SHOW_KEYBOARD;
                    GetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                    mk.fMask=MIIM_STATE;
                    mk.fState=MFS_CHECKED;
                    SetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                }

                // check Saenaru option flags
                if (dwOptionFlag & ESCENG_SUPPORT) {
                    idFlag= IDM_ESC_ASCII;
                    GetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                    mk.fMask=MIIM_STATE;
                    mk.fState=MFS_CHECKED;
                    SetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                }
                if (dwOptionFlag & DVORAK_SUPPORT) {
                    idFlag= IDM_DVORAK;
                    GetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                    mk.fMask=MIIM_STATE;
                    mk.fState=MFS_CHECKED;
                    SetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                }
                if (!(dwImeFlag & SAENARU_ONTHESPOT)) {
                    idFlag= IDM_WORD_UNIT;
                    GetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                    mk.fMask=MIIM_STATE;
                    mk.fState=MFS_CHECKED;
                    SetMenuItemInfo(hSubMenu,idFlag,FALSE,&mk);
                }

                // Layout
                GetMenuItemInfo(hSubMenu,idLayout,FALSE,&mk);
                mk.fMask=MIIM_STATE;
                mk.fState=MFS_CHECKED;
                SetMenuItemInfo(hSubMenu,idLayout,FALSE,&mk);

                if (idLayout > IDM_3SOON) {
                    idLayout = dwLayoutFlag - LAYOUT_USER;
                    idLayout += IDM_USER + 1;
                }

                if (GetRegKeyHandle(TEXT("\\Keyboard"), &hKey)) {
                    MENUITEMINFO mif;
                    HMENU hKbdMenu;
                    INT i;
                    INT id=IDM_USER;
                    hKbdMenu=CreateMenu();

                    GetMenuItemInfo(hSubMenu,IDM_USER,FALSE,&mif);
                    mif.fMask =MIIM_SUBMENU;
                    mif.hSubMenu =hKbdMenu;

                    mif.cbSize = sizeof(MENUITEMINFO);
                    for (i=0;i<10;i++) {
                        WCHAR achValue[256]; 
                        DWORD cchValue = 256;
                        DWORD retCode;
                        LPCWSTR wstrDesc;
                        ULONG nstrDesc;
                        ULONG ucheck;
    
                        achValue[0] = '\0';
                        retCode = RegEnumValue(hKey, i, 
                            achValue, 
                            &cchValue, 
                            NULL, 
                            NULL,
                            NULL,
                            NULL);
     
                        if (retCode != ERROR_SUCCESS ) 
                        { 
                            MyDebugPrint((TEXT("(%d) %s\n"), i+1, achValue));
                            break;
                        }
                        wstrDesc = achValue;
                        nstrDesc = wcslen (wstrDesc);
                        achValue[nstrDesc]='\0';
                        
                        ucheck = MF_STRING|MF_ENABLED;
                        if (idLayout == ++id) ucheck |= MF_CHECKED;
    
                        retCode= AppendMenu(hKbdMenu,ucheck,
                                id,
                                wstrDesc);
                        MyDebugPrint((TEXT("\t*** Append MENU ret (%d)\n"), retCode));
                    }
                    SetMenuItemInfo(hSubMenu,IDM_USER,FALSE,&mif);
                    DestroyMenu(hKbdMenu);
                }

                if (hKey)
                    RegCloseKey (hKey);

                hKey = NULL;
                if (GetRegKeyHandle(TEXT("\\Compose"), &hKey)) {
                    MENUITEMINFO mif;
                    HMENU hCompMenu;
                    INT i;
                    INT id=IDM_USER_COMP;

                    INT idCompose;

                    idCompose=dwComposeFlag - 3;
                    if (idCompose > 0)
                        idCompose += IDM_USER_COMP;

                    hCompMenu=CreateMenu();

                    GetMenuItemInfo(hSubMenu,IDM_USER_COMP,FALSE,&mif);
                    mif.fMask =MIIM_SUBMENU;
                    mif.hSubMenu =hCompMenu;

                    mif.cbSize = sizeof(MENUITEMINFO);
                    for (i=0;i<10;i++) {
                        WCHAR achValue[256]; 
                        DWORD cchValue = 256;
                        DWORD retCode;
                        LPCWSTR wstrDesc;
                        ULONG nstrDesc;
                        ULONG ucheck;
    
                        achValue[0] = '\0';
                        retCode = RegEnumValue(hKey, i, 
                            achValue, 
                            &cchValue, 
                            NULL, 
                            NULL,
                            NULL,
                            NULL);
     
                        if (retCode != ERROR_SUCCESS ) 
                        { 
                            MyDebugPrint((TEXT("(%d) %s\n"), i+1, achValue));
                            break;
                        }
                        wstrDesc = achValue;
                        nstrDesc = wcslen (wstrDesc);
                        achValue[nstrDesc]='\0';
    
                        ucheck = MF_STRING|MF_ENABLED;
                        if (idCompose == ++id) ucheck |= MF_CHECKED;

                        AppendMenu(hCompMenu,ucheck,
                                id,
                                wstrDesc);
                    }
                    SetMenuItemInfo(hSubMenu,IDM_USER_COMP,FALSE,&mif);

                    DestroyMenu(hCompMenu);
                }

                if (hKey)
                    RegCloseKey (hKey);

                pt.x = (int)LOWORD(lParam), 
                pt.y = (int)HIWORD(lParam), 

                ClientToScreen(hStatusWnd, &pt);

                cmd = TrackPopupMenu(hSubMenu, 
                                     TPM_RETURNCMD, 
                                     pt.x,
                                     pt.y,
                                     0, 
                                     hStatusWnd, 
                                     NULL);
                switch (cmd)
                {
                    case IDM_RECONVERT:
                    {
                        DWORD dwSize = (DWORD) MyImmRequestMessage(hIMC, IMR_RECONVERTSTRING, 0);
                        if (dwSize)
                        {
                            LPRECONVERTSTRING lpRS;

                            lpRS = (LPRECONVERTSTRING)GlobalAlloc(GPTR, dwSize);
                            lpRS->dwSize = dwSize;

                            if (dwSize = (DWORD) MyImmRequestMessage(hIMC, IMR_RECONVERTSTRING, (LPARAM)lpRS)) {

#ifdef DEBUG
{
    TCHAR szDev[80];
    LPMYSTR lpDump= (LPMYSTR)(((LPSTR)lpRS) + lpRS->dwStrOffset);
    *(LPMYSTR)(lpDump + lpRS->dwStrLen) = MYTEXT('\0');

    OutputDebugString(TEXT("IMR_RECONVERTSTRING\r\n"));
    wsprintf(szDev, TEXT("dwSize            %x\r\n"), lpRS->dwSize);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwVersion         %x\r\n"), lpRS->dwVersion);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwStrLen          %x\r\n"), lpRS->dwStrLen);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwStrOffset       %x\r\n"), lpRS->dwStrOffset);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwCompStrLen      %x\r\n"), lpRS->dwCompStrLen);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwCompStrOffset   %x\r\n"), lpRS->dwCompStrOffset);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwTargetStrLen    %x\r\n"), lpRS->dwTargetStrLen);
    OutputDebugString(szDev);
    wsprintf(szDev, TEXT("dwTargetStrOffset %x\r\n"), lpRS->dwTargetStrOffset);
    OutputDebugString(szDev);
    MyOutputDebugString(lpDump);
    OutputDebugString(TEXT("\r\n"));
}
#endif
                                MyImmRequestMessage(hIMC, IMR_CONFIRMRECONVERTSTRING, (LPARAM)lpRS);
                            }
#ifdef DEBUG
                            else
                                OutputDebugString(TEXT("ImmRequestMessage returned 0\r\n"));
#endif
                            GlobalFree((HANDLE)lpRS);
                        }
                        break;
                    }

                    case IDM_ABOUT:
                        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                        //(void) ImeConfigure (GetKeyboardLayout(0), lpIMC->hWnd, IME_CONFIG_GENERAL, NULL);
                        ImmConfigureIME(GetKeyboardLayout(0), lpIMC->hWnd, IME_CONFIG_GENERAL, 0);
                        //ImmConfigureIME(GetKeyboardLayout(0), NULL, IME_CONFIG_GENERAL, 0);
                        ImmUnlockIMC(hIMC);
                        break;

                    case IDM_OLD2BUL:
                    case IDM_3BUL:
                    case IDM_390:
                    case IDM_NEW2BUL:
                    case IDM_NEW3BUL:
                    case IDM_AHNMATAE:
                    case IDM_3SOON:
                        dwLayoutFlag = cmd - IDM_OLD2BUL + LAYOUT_OLD2BUL;
                        /*
                            MENUITEMINFO mk;

                            GetMenuItemInfo(hSubMenu,cmd,FALSE,&mk);
                            mk.fMask=MIIM_STATE;
                            mk.fState=MFS_CHECKED;
                            mk.cbSize = sizeof(MENUITEMINFO);
                            SetMenuItemInfo(hSubMenu,cmd,FALSE,&mk);
                        */
                        set_keyboard(dwLayoutFlag);
                        break;
                    case IDM_HANGUL:
                        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                        fdwConversion = lpIMC->fdwConversion;
                        fdwConversion &= ~IME_CMODE_HANJACONVERT;
                        fdwConversion |= IME_CMODE_NATIVE;
                        ImmSetConversionStatus(hIMC,fdwConversion,dwTemp);
                        ImmUnlockIMC(hIMC);
                        break;
                    case IDM_HANJA:
                        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                        fdwConversion = lpIMC->fdwConversion;
                        if (!(fdwConversion & IME_CMODE_HANJACONVERT)) {
                            fdwConversion |= IME_CMODE_HANJACONVERT;
                            fdwConversion |= IME_CMODE_NATIVE;
                            ImmSetConversionStatus(hIMC,fdwConversion,dwTemp);
                        }
                        ImmUnlockIMC(hIMC);
                        break;
                    case IDM_ENG:
                        ChangeMode(hIMC,TO_CMODE_ALPHANUMERIC);
                        break;

                    case IDM_ESC_ASCII:
                        if (dwOptionFlag & ESCENG_SUPPORT)
                            dwOptionFlag &= ~ESCENG_SUPPORT;
                        else
                            dwOptionFlag |= ESCENG_SUPPORT;
                        break;
                    case IDM_DVORAK:
                        if (dwOptionFlag & DVORAK_SUPPORT)
                            dwOptionFlag &= ~DVORAK_SUPPORT;
                        else
                            dwOptionFlag |= DVORAK_SUPPORT;
                        break;
                    case IDM_WORD_UNIT:
                        if (dwImeFlag & SAENARU_ONTHESPOT)
                            dwImeFlag &= ~SAENARU_ONTHESPOT;
                        else
                            dwImeFlag |= SAENARU_ONTHESPOT;
                        break;
                    case IDM_SHOW_KEYBOARD:
                        lpIMC = (LPINPUTCONTEXT)ImmLockIMC(hIMC);
                        fdwConversion = lpIMC->fdwConversion;
                        if (fdwConversion & IME_CMODE_SOFTKBD)
                            fdwConversion &= ~IME_CMODE_SOFTKBD;
                        else
                            fdwConversion |= IME_CMODE_SOFTKBD;

                        ImmSetConversionStatus(hIMC,fdwConversion,dwTemp);
                        ImmUnlockIMC(hIMC);

                        break;
                    default:
                        // check some dynamic menu items
                        if (cmd > IDM_USER && cmd < IDM_USER_COMP) {
                            MENUITEMINFO mk;
                            int myLayout;

                            myLayout = cmd - IDM_USER;
                            myLayout += LAYOUT_USER - 1; // adjust

                            dwLayoutFlag = myLayout;
                            set_keyboard(dwLayoutFlag);
                        } else if (cmd > IDM_USER_COMP && cmd < IDM_DVORAK ) {
                            MENUITEMINFO mk;
                            int myCompose;

                            myCompose = cmd - IDM_USER_COMP + 3; // adjust
                            dwComposeFlag = myCompose;
                            set_compose(dwComposeFlag);
                            MyDebugPrint((TEXT("\t*** Compose (%d)\n"), myCompose));
                        }
                        MyDebugPrint((TEXT("\t*** RIGHTMENU (%d)\n"), cmd));
                        break;
                }

            }
            if (hMenu)
                DestroyMenu(hMenu);

            break;

        case WM_LBUTTONUP:
            dwMouse = GetWindowLong(hStatusWnd,FIGWL_MOUSE);
            if (dwMouse & FIM_CAPUTURED)
            {
                ReleaseCapture();
                if (dwMouse & FIM_MOVED)
                {
                    DrawUIBorder(&drc);
                    GetCursorPos( &pt );
                    MoveWindow(hStatusWnd,pt.x - ptdif.x,
                                    pt.y - ptdif.y,
                                    rc.right,
                                    rc.bottom,TRUE);
                }
            }
            hSvrWnd = (HWND)GetWindowLongPtr(hStatusWnd,FIGWL_SVRWND);

            if (hIMC = (HIMC)GetWindowLongPtr(hSvrWnd,IMMGWLP_IMC))
            {
                GetCursorPos(&pt);
                dwPushedStatus = GetWindowLong(hStatusWnd, FIGWL_PUSHSTATUS);
                dwPushedStatus &= CheckPushedStatus(hStatusWnd,&pt);
                if (!dwPushedStatus) {
                } else if (dwPushedStatus == PUSHED_STATUS_CLOSE) {
                } else if (dwPushedStatus == PUSHED_STATUS_HDR)
                {
                    fOpen = ImmGetOpenStatus(hIMC);
                    ImmGetConversionStatus(hIMC,&fdwConversion,&dwTemp);
                    if (!fOpen)
                        ImmSetOpenStatus(hIMC,TRUE);
                    else if (fdwConversion & IME_CMODE_NATIVE)
                        ImmSetOpenStatus(hIMC,FALSE);

                    fdwConversion = GetUINextMode(fdwConversion,dwPushedStatus);

                    ImmSetConversionStatus(hIMC,fdwConversion,dwTemp);
                }
                else
                {
                    fOpen = ImmGetOpenStatus(hIMC);
                    if (fOpen) {
                        ImmGetConversionStatus(hIMC,&fdwConversion,&dwTemp);
                        fdwConversion = GetUINextMode(fdwConversion,dwPushedStatus);
                        ImmSetConversionStatus(hIMC,fdwConversion,dwTemp);
                    }
                }
            }
            PaintStatus(hStatusWnd,hDC,NULL,0);
            break;
    }
    ReleaseDC(hStatusWnd,hDC);
}
/**********************************************************************/
/*                                                                    */
/* UpdateStatusWindow(lpUIExtra)                                      */
/*                                                                    */
/**********************************************************************/
void PASCAL UpdateStatusWindow(LPUIEXTRA lpUIExtra)
{
    if (IsWindow(lpUIExtra->uiStatus.hWnd))
        SendMessage(lpUIExtra->uiStatus.hWnd,WM_UI_UPDATE,0,0L);
}

/*
 * ex: ts=8 sts=4 sw=4 et
 */
