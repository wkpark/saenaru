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

#include "windows.h"
#include "immdev.h"
#include "saenaru.h"
#include "prsht.h"
#include "resource.h"
#define MEMALLOC(x)      LocalAlloc(LMEM_FIXED, x)
#define MEMFREE(x)       LocalFree(x)

#define MAX_PAGES 5



/**********************************************************************/
/*                                                                    */
/*      AddPage()                                                     */
/*                                                                    */
/**********************************************************************/
void PASCAL AddPage(LPPROPSHEETHEADER ppsh, UINT id, DLGPROC pfn)
{
    if (ppsh->nPages < MAX_PAGES) {
        PROPSHEETPAGE psp;

        psp.dwSize = sizeof(psp);
        psp.dwFlags = PSP_DEFAULT;
        psp.hInstance = hInst;
        psp.pszTemplate = MAKEINTRESOURCE(id);
        psp.pfnDlgProc = pfn;
        psp.lParam = 0;

        ppsh->phpage[ppsh->nPages] = CreatePropertySheetPage(&psp);
        if (ppsh->phpage[ppsh->nPages])
             ppsh->nPages++;
    }
}  

/**********************************************************************/
/*                                                                    */
/*      ImeConfigure()                                                */
/*                                                                    */
/**********************************************************************/
BOOL WINAPI ImeConfigure(HKL hKL,HWND hWnd, DWORD dwMode, LPVOID lpData)
{
    HPROPSHEETPAGE rPages[MAX_PAGES];
    PROPSHEETHEADER psh;

    if (gfSaenaruSecure) return FALSE;

    ImeLog(LOGF_API, TEXT("ImeConfigure"));

    psh.dwSize = sizeof(psh);
    psh.dwFlags = PSH_PROPTITLE | PSH_NOAPPLYNOW;
    psh.hwndParent = hWnd;
    psh.hInstance = hInst;
    psh.pszCaption = MAKEINTRESOURCE(IDS_CONFIGNAME);
    psh.nPages = 0;
    psh.nStartPage = 0;
    psh.phpage = rPages;

    
    switch (dwMode)
    {
        case IME_CONFIG_GENERAL:
            AddPage(&psh, DLG_GENERAL, GeneralDlgProc);
            //AddPage(&psh, DLG_REGISTERWORD, RegWordDlgProc);
            //AddPage(&psh, DLG_SELECTDICTIONARY, SelectDictionaryDlgProc);
            AddPage(&psh, DLG_ABOUT, AboutDlgProc);
#ifdef DEBUG
            AddPage(&psh, DLG_DEBUG, DebugOptionDlgProc);
#endif
            PropertySheet(&psh);
            break;

        case IME_CONFIG_REGISTERWORD:
            AddPage(&psh, DLG_REGISTERWORD, RegWordDlgProc);
            AddPage(&psh, DLG_ABOUT, AboutDlgProc);
            PropertySheet(&psh);
            break;

        case IME_CONFIG_SELECTDICTIONARY:
            AddPage(&psh, DLG_SELECTDICTIONARY, SelectDictionaryDlgProc);
            AddPage(&psh, DLG_ABOUT, AboutDlgProc);
            PropertySheet(&psh);
            break;

        default:
            break;
    }

    return TRUE;
}

/**********************************************************************/
/*                                                                    */
/*      RegWordConfigure()                                            */
/*                                                                    */
/**********************************************************************/
INT_PTR CALLBACK RegWordDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    UINT nItem;
    UINT i;
    LRESULT dwIndex;
    TCHAR szRead[128];
    TCHAR szString[128];
    TCHAR szBuf[128];
    LRESULT dwStyle;

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    break;
        
                case PSN_KILLACTIVE:
                    break;
        
                case PSN_APPLY:

                    if (!GetDlgItemText(hDlg, ID_WR_READING, szRead, sizeof(szRead)/sizeof(szRead[0])))
                    {
                        LoadString(hInst,IDS_NOREADING,szBuf,sizeof(szBuf)/sizeof(szBuf[0]));
                        MessageBox(hDlg, szBuf, NULL, MB_OK);
                        return FALSE;
                    }


                    if (!GetDlgItemText(hDlg, ID_WR_STRING, szString, sizeof(szString)/sizeof(szString[0])))
                    {
                        LoadString(hInst,IDS_NOSTRING,szBuf,sizeof(szBuf)/sizeof(szBuf[0]));
                        MessageBox(hDlg, szBuf, NULL, MB_OK);
                        return FALSE;
                    }

                    dwIndex = SendDlgItemMessage(hDlg, ID_WR_STYLE,CB_GETCURSEL,0,0);
                    dwStyle = SendDlgItemMessage(hDlg, ID_WR_STYLE,CB_GETITEMDATA,dwIndex,0);

                    if (!ImeRegisterWord(szRead, (DWORD) dwStyle, szString))
                    {
                        LoadString(hInst,IDS_REGWORDRET,szBuf,sizeof(szBuf)/sizeof(szBuf[0]));
                        MessageBox(hDlg, szBuf, NULL, MB_OK);
                    }
                    break;

                case PSN_RESET:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
            lpPropSheet = (LPPROPSHEETPAGE)lParam;

            if (nItem = ImeGetRegisterWordStyle(0, NULL))
            {
                LPSTYLEBUF lpStyleBuf = (LPSTYLEBUF)GlobalAlloc(GPTR,nItem * sizeof(STYLEBUF));

                if (!lpStyleBuf)
                {
                    LoadString(hInst,IDS_NOMEMORY,szBuf,sizeof(szBuf)/sizeof(szBuf[0]));
                    MessageBox(hDlg, szBuf, NULL, MB_OK);
                    return TRUE;
                }

                ImeGetRegisterWordStyle(nItem,lpStyleBuf);

                for (i = 0; i < nItem; i++)
                {
                    dwIndex = SendDlgItemMessage(hDlg,ID_WR_STYLE,CB_ADDSTRING,0,(LPARAM)lpStyleBuf->szDescription);
                    SendDlgItemMessage(hDlg,ID_WR_STYLE,CB_SETITEMDATA,dwIndex,lpStyleBuf->dwStyle);
                    lpStyleBuf++;
                }

                GlobalFree((HANDLE)lpStyleBuf);
            }
            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            break;

        default:
            return FALSE;

    }
    return TRUE;
} 


/**********************************************************************/
/*                                                                    */
/*      SelectDictionaryConfigure()                                   */
/*                                                                    */
/**********************************************************************/
INT_PTR CALLBACK SelectDictionaryDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    break;
        
                case PSN_KILLACTIVE:
                    break;
        
                case PSN_APPLY:
                    break;

                case PSN_RESET:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
            lpPropSheet = (LPPROPSHEETPAGE)lParam;
            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            break;

        default:
            return FALSE;

    }
    return TRUE;
} 



/**********************************************************************/
/*                                                                    */
/*      AboutConfigure()                                            */
/*                                                                    */
/**********************************************************************/
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    break;
        
                case PSN_KILLACTIVE:
                    break;
        
                case PSN_APPLY:
                    break;

                case PSN_RESET:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) lParam);
            lpPropSheet = (LPPROPSHEETPAGE)lParam;
            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            EndDialog(hDlg, wParam); 
            break;

        default:
            return FALSE;

    }
    return TRUE;
} 

LPTSTR getToggleKeyName(LPTSTR lpName, UINT len)
{
    UINT flag = dwToggleKey & 0xffff0000;
    UINT vk = dwToggleKey & 0x00ff;
    UINT sc;
    TCHAR szVk[128];
    TCHAR szMod[128], szTmp[128];
    LPTSTR lpSz;

    szMod[0] = TEXT('\0');

    if (flag & MASK_SHIFT) {
        if ((flag & MASK_SHIFT) == MASK_SHIFT) {
            wsprintf(szMod, TEXT("%s-"), TEXT("Shift"));
        } else if (flag & MASK_LSHIFT) {
            wsprintf(szMod, TEXT("%s-"), TEXT("LShift"));
        } else {
            wsprintf(szMod, TEXT("%s-"), TEXT("RShift"));
        }
    }

    if (flag & MASK_CTRL) {
        if ((flag & MASK_CTRL) == MASK_CTRL) {
            lstrcat (szMod, TEXT("Ctrl-"));
        } else if (flag & MASK_LCTRL) {
            lstrcat (szMod, TEXT("LCtrl-"));
        } else {
            lstrcat (szMod, TEXT("RCtrl-"));
        }
    }

    if (flag & MASK_ALT) {
        if ((flag & MASK_ALT) == MASK_ALT) {
            lstrcat (szMod, TEXT("Alt-"));
        } else if (flag & MASK_LALT) {
            lstrcat (szMod, TEXT("LAlt-"));
        } else {
            lstrcat (szMod, TEXT("RAlt-"));
        }
    }

    sc = MapVirtualKey(vk, 0);
    //sc = MapVirtualKey(vk, MAPVK_VK_TO_VSC);
    GetKeyNameText(sc << 16, szVk, 128);
    wsprintf(szTmp, TEXT("%s<%s>"), szMod, szVk);

    Mylstrcpyn(lpName, szTmp, len);
    return lpName;
}

/**********************************************************************/
/*                                                                    */
/*      SelectToggleDlgProc()                                         */
/*                                                                    */
/**********************************************************************/
INT_PTR CALLBACK SelectToggleDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));
    DWORD   dwTemp = 0;
    HWND  hwndRadio;
    TCHAR  szName[128];

    switch(message)
    {
        case WM_KEYDOWN:
            MyDebugPrint((TEXT(" >>>>>>> WM_KEYDOWN : vk=%d\n"), wParam));
            break;

        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    break;
        
                case PSN_KILLACTIVE:
                    break;

                case PSN_APPLY:
                    dwTemp = 0;
                    if (dwToggleKey == 0)
                        dwToggleKey = MASK_LSHIFT | VK_SPACE;

                    if (IsDlgButtonChecked(hDlg, IDC_LSHIFT))
                        dwTemp |= MASK_LSHIFT;
                    if (IsDlgButtonChecked(hDlg, IDC_RSHIFT))
                        dwTemp |= MASK_RSHIFT;

                    if (IsDlgButtonChecked(hDlg, IDC_LCTRL))
                        dwTemp |= MASK_LCTRL;
                    if (IsDlgButtonChecked(hDlg, IDC_RCTRL))
                        dwTemp |= MASK_RCTRL;

                    if (IsDlgButtonChecked(hDlg, IDC_LALT))
                        dwTemp |= MASK_LALT;
                    if (IsDlgButtonChecked(hDlg, IDC_RALT))
                        dwTemp |= MASK_RALT;

                    // Set the default modifier *SHIFT* for empty Modifier.
                    if (!dwTemp)
                        dwTemp = MASK_LSHIFT;

                    // Set VK_XXX value
                    if ((hwndRadio = GetDlgItem (hDlg, IDC_TOGGLE))) {
                        UINT ret;
                        UINT vk;
                        ret = (UINT)SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                        if (ret == CB_ERR || ret <= 0 || ret == 29 || ret == 42 || ret >= 54) {
                            // Set the default VK key *SPACE* for invalid VK keys.
                            vk = VK_SPACE;
                        } else {
                            vk = MapVirtualKey(ret, 1);
                            //vk = MapVirtualKey(i, MAPVK_VSC_TO_VK);
                        }
                        MyDebugPrint((TEXT(" >>>>>>> Key Apply dialog : vk=%d, sc=%d\n"), vk, ret));
                        dwTemp |= vk;
                    }

                    dwToggleKey = dwTemp;
                    getToggleKeyName(szName, 128);
                    MyDebugPrint((TEXT(" >>>>>>> Toggle Key Name ='%s'\n"), szName));

                    break;

                case PSN_RESET:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
            lpPropSheet = (LPPROPSHEETPAGE)lParam;

            if (dwToggleKey == 0)
                dwToggleKey = MASK_LSHIFT | VK_SPACE;

            if ((hwndRadio = GetDlgItem (hDlg, IDC_TOGGLE))) {
                MYCHAR  szTmp[80];
                int     i, id;
                int     ret;
                DWORD   dwFlag;
                LPCWSTR wstrDesc;
                ULONG   nstrDesc;
                UINT vk, sc;

                SendMessage(hwndRadio, CB_ADDSTRING, 0, (LPARAM)TEXT("Space"));
                for (i = 1; i < 54; i++) {
                    LPTSTR lpVk;
                    TCHAR szVk[128];

                    vk = MapVirtualKey(i, 1);
                    //vk = MapVirtualKey(i, MAPVK_VSC_TO_VK);
                    GetKeyNameText(i << 16, szVk, 128);
                    // wsprintf(szTmp, TEXT("%c"), i);
                    SendMessage(hwndRadio, CB_ADDSTRING, 0, (LPARAM)szVk);
                }

                sc = MapVirtualKey(dwToggleKey & 0x00FF, 0);
                //sc = MapVirtualKey(i, MAPVK_VK_TO_VSC);
                //ret = SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                if (sc == 61 || sc <= 0 || sc == 29 || sc== 42 || sc >= 54) // Space
                    SendMessage(hwndRadio, CB_SETCURSEL, 0, 0);
                else
                    SendMessage(hwndRadio, CB_SETCURSEL, sc, 0);

                MyDebugPrint((TEXT(" >>>>>>> Key Scan dialog : %d\n"), sc));
            }

            CheckDlgButton(hDlg, IDC_LSHIFT, 
                                (dwToggleKey & MASK_LSHIFT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_RSHIFT, 
                                (dwToggleKey & MASK_RSHIFT) ? 1 : 0);

            CheckDlgButton(hDlg, IDC_LCTRL, 
                                (dwToggleKey & MASK_LCTRL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_RCTRL, 
                                (dwToggleKey & MASK_RCTRL) ? 1 : 0);

            CheckDlgButton(hDlg, IDC_LALT, 
                                (dwToggleKey & MASK_LALT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_RALT, 
                                (dwToggleKey & MASK_RALT) ? 1 : 0);

            break;

        case WM_CLOSE:
        case WM_DESTROY:
            EndDialog(hDlg, 0); 
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:

            switch(HIWORD(wParam)) {
            case BN_CLICKED:
                // Shift-Ctrl, Shift-Alt are reserved by Language HotKeys.
                // so check exclusively.
                switch(LOWORD(wParam)) {
                case IDC_LSHIFT:
                case IDC_RSHIFT:
                    hwndRadio = GetDlgItem (hDlg, IDC_LCTRL);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    hwndRadio = GetDlgItem (hDlg, IDC_RCTRL);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);

                    hwndRadio = GetDlgItem (hDlg, IDC_LALT);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    hwndRadio = GetDlgItem (hDlg, IDC_RALT);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                case IDC_LCTRL:
                case IDC_RCTRL:
                case IDC_LALT:
                case IDC_RALT:
                    hwndRadio = GetDlgItem (hDlg, IDC_LSHIFT);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    hwndRadio = GetDlgItem (hDlg, IDC_RSHIFT);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                }

                break;
            case CBN_SELCHANGE:
                switch(LOWORD(wParam)) {
                case IDC_TOGGLE:
                    if ((hwndRadio = GetDlgItem (hDlg, IDC_TOGGLE))) {
                        UINT ret;
                        ret = (UINT)SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                        if (ret != CB_ERR) {
                            if (ret == 29 || ret == 42 || ret == 54) {
                                // Ctrl, Shift, RShift
                                // 이 키들은 select할 수 없도록 한다.
                                SendMessage(hwndRadio, CB_SETCURSEL, 0, 0);
                            }
                            //SendMessage(hwndRadio, CB_SETCURSEL, ret, 0);
                            MyDebugPrint((TEXT(" >>>>>>> Key dialog : %d\n"), ret));
                        }
                        return TRUE;
                    }
                }
                break;
            }

            switch(wParam) {
            case 1:
                dwTemp = 0;
                if (dwToggleKey == 0)
                    dwToggleKey = MASK_LSHIFT | VK_SPACE;

                if (IsDlgButtonChecked(hDlg, IDC_LSHIFT))
                    dwTemp |= MASK_LSHIFT;
                if (IsDlgButtonChecked(hDlg, IDC_RSHIFT))
                    dwTemp |= MASK_RSHIFT;

                if (IsDlgButtonChecked(hDlg, IDC_LCTRL))
                    dwTemp |= MASK_LCTRL;
                if (IsDlgButtonChecked(hDlg, IDC_RCTRL))
                    dwTemp |= MASK_RCTRL;

                if (IsDlgButtonChecked(hDlg, IDC_LALT))
                    dwTemp |= MASK_LALT;
                if (IsDlgButtonChecked(hDlg, IDC_RALT))
                    dwTemp |= MASK_RALT;

                // Set the default modifier *SHIFT* for empty Modifier.
                if (!dwTemp)
                    dwTemp = MASK_LSHIFT;

                if ((hwndRadio = GetDlgItem (hDlg, IDC_TOGGLE))) {
                    UINT ret;
                    UINT vk;
                    ret = (UINT)SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                    if (ret == CB_ERR || ret <= 0 || ret == 29 || ret == 42 || ret >= 54) {
                        vk = VK_SPACE;
                    } else {
                        vk = MapVirtualKey(ret, 1);
                        //vk = MapVirtualKey(i, MAPVK_VSC_TO_VK);
                    }
                    MyDebugPrint((TEXT(" >>>>>== Key Apply dialog : vk=%d, sc=%d\n"), vk, ret));
                    dwTemp |= vk;
                }

                dwToggleKey = dwTemp;

                EndDialog(hDlg, wParam);
                return TRUE;

            case 2:
                dwToggleKey = MASK_LSHIFT | VK_SPACE;
                // reset by default.
                CheckDlgButton(hDlg, IDC_LSHIFT, 1);
                CheckDlgButton(hDlg, IDC_RSHIFT, 0);
                CheckDlgButton(hDlg, IDC_LCTRL, 0);
                CheckDlgButton(hDlg, IDC_RCTRL, 0);
                CheckDlgButton(hDlg, IDC_LALT, 0);
                CheckDlgButton(hDlg, IDC_RALT, 0);

                if ((hwndRadio = GetDlgItem (hDlg, IDC_TOGGLE)))
                    (UINT)SendMessage(hwndRadio, CB_SETCURSEL, 0, 0);

                return TRUE;
            }
            break;

        default:
            return FALSE;

    }
    return TRUE;
} 

/**********************************************************************/
/*                                                                    */
/*      GeneralConfigure()                                            */
/*                                                                    */
/**********************************************************************/
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    DWORD dwTemp;
    NMHDR FAR *lpnm;
    HWND  hwndRadio;
    HKL hCur;

    HKEY hKey;

    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    break;
        
                case PSN_KILLACTIVE:
                    break;
        
                case PSN_APPLY:
                    dwTemp = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_COMPOSITE_TYPING))
                        dwTemp |= COMPOSITE_TYPING;
                    if (IsDlgButtonChecked(hDlg, IDC_CONCURRENT_TYPING))
                        dwTemp |= CONCURRENT_TYPING;
                    if (IsDlgButtonChecked(hDlg, IDC_BACKSPACE_BY_JAMO))
                        dwTemp |= BACKSPACE_BY_JAMO;
                    if (IsDlgButtonChecked(hDlg, IDC_KSX1002_SUPPORT))
                        dwTemp |= KSX1002_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_KSX1001_SUPPORT))
                        dwTemp |= KSX1001_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_FULL_MULTIJOMO))
                        dwTemp |= FULL_MULTIJOMO;
                    if (IsDlgButtonChecked(hDlg, IDC_JAMOS))
                        dwTemp |= HANGUL_JAMOS;
                    if (IsDlgButtonChecked(hDlg, IDC_USE_SHIFT_SPACE))
                        dwTemp |= USE_SHIFT_SPACE;
                    if (IsDlgButtonChecked(hDlg, IDC_DVORAK_SUPPORT))
                        dwTemp |= DVORAK_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_QWERTY_HOTKEY_SUPPORT))
                        dwTemp |= QWERTY_HOTKEY_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_ESCENG_SUPPORT))
                        dwTemp |= ESCENG_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_SIMILAR_WORDS))
                        dwTemp |= SEARCH_SIMILAR_WORDS;
                    if (IsDlgButtonChecked(hDlg, IDC_HANJA_CAND_SPACE))
                        dwTemp |= HANJA_CAND_WITH_SPACE;
                    dwOptionFlag = dwTemp;

                    SetDwordToSetting(TEXT("OptionFlag"), dwOptionFlag);

                    // 사용자 키보드?
                    if ((hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_USER))) {
                        UINT ret;
                        ret = (UINT) SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                        if (ret != CB_ERR) {
                            SendMessage(hwndRadio, CB_SETCURSEL, ret, 0);
                            MyDebugPrint((TEXT(" ------- Keyboard dialog : %d\n"), ret));
                            dwLayoutFlag = LAYOUT_USER + ret;
                        }
                    }

                    dwTemp = 0;
                    if (dwLayoutFlag == 0)
                        dwLayoutFlag = LAYOUT_OLD2BUL;
                    if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_OLD2BUL))
                        dwTemp = LAYOUT_OLD2BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_3FIN))
                        dwTemp = LAYOUT_3FIN;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_390))
                        dwTemp = LAYOUT_390;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_NK_2BUL))
                        dwTemp = LAYOUT_NK_2BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_YET2BUL))
                        dwTemp = LAYOUT_YET2BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_YET3BUL))
                        dwTemp = LAYOUT_YET3BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_NEW2BUL))
                        dwTemp = LAYOUT_NEW2BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_NEW3BUL))
                        dwTemp = LAYOUT_NEW3BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_AHNMATAE))
                        dwTemp = LAYOUT_AHNMATAE;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_3SUN))
                        dwTemp = LAYOUT_3SUN;
                    //else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_USER))
                    //    dwTemp = LAYOUT_USER;
                    if (dwTemp != 0)
                        dwLayoutFlag = dwTemp;

                    SetDwordToSetting(TEXT("LayoutFlag"), dwLayoutFlag);

                    if (dwToggleKey)
                        SetDwordToSetting(TEXT("HangulToggle"), dwToggleKey);

                    set_keyboard(dwLayoutFlag);
                    
                    break;

                case PSN_RESET:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR)lParam);
            lpPropSheet = (LPPROPSHEETPAGE)lParam;

#if 0
            if (hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_NEW2BUL) )
                SendMessage (hwndRadio, BM_SETCHECK, BST_CHECKED, 0) ;
            else if (hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_NEW3BUL) )
                SendMessage (hwndRadio, BM_SETCHECK, BST_CHECKED, 0) ;
            else if (hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_OLD2BUL) )
                SendMessage (hwndRadio, BM_SETCHECK, BST_CHECKED, 0) ;
            else if (hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_3BULFINAL) )
                SendMessage (hwndRadio, BM_SETCHECK, BST_CHECKED, 0) ;
            else if (hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_390) )
                SendMessage (hwndRadio, BM_SETCHECK, BST_CHECKED, 0) ;
            else if (hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_AHNMATAE) )
                SendMessage (hwndRadio, BM_SETCHECK, BST_CHECKED, 0) ;
#endif


            if ((hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_USER)) &&
                    GetRegKeyHandle(TEXT("\\Keyboard"), &hKey)) {
                int     i, id;
                int     ret;
                DWORD   dwFlag;
                LPCWSTR wstrDesc;
                ULONG   nstrDesc;
                id = IDC_LAYOUT_USER;

                for (i=0;i<10;i++)
                {
                    WCHAR achValue[256]; 
                    DWORD cchValue = 256;
                    DWORD retCode;

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
                        break;
                        MyDebugPrint((TEXT("(%d) %s\n"), i+1, achValue));
                    }     
                    wstrDesc = achValue;
                    nstrDesc = wcslen (wstrDesc);
    
                    SendMessage(hwndRadio, CB_ADDSTRING,
                        0,
                        (LPARAM)wstrDesc);
                    
                }

                //ret = SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                ret = dwLayoutFlag - LAYOUT_USER;
                SendMessage(hwndRadio, CB_SETCURSEL, ret, 0);
                //MyDebugPrint((TEXT("User Keyboard dialog : %d\n"), ret));

                RegCloseKey(hKey);
            }

            /* Global options */
            CheckDlgButton(hDlg, IDC_CONCURRENT_TYPING, 
                                (dwOptionFlag & CONCURRENT_TYPING) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_COMPOSITE_TYPING, 
                                (dwOptionFlag & COMPOSITE_TYPING) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_KSX1001_SUPPORT, 
                                (dwOptionFlag & KSX1001_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_KSX1002_SUPPORT, 
                                (dwOptionFlag & KSX1002_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_BACKSPACE_BY_JAMO, 
                                (dwOptionFlag & BACKSPACE_BY_JAMO) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_FULL_MULTIJOMO, 
                                (dwOptionFlag & FULL_MULTIJOMO) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_JAMOS, 
                                (dwOptionFlag & HANGUL_JAMOS) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_DVORAK_SUPPORT, 
                                (dwOptionFlag & DVORAK_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_QWERTY_HOTKEY_SUPPORT, 
                                (dwOptionFlag & QWERTY_HOTKEY_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_ESCENG_SUPPORT, 
                                (dwOptionFlag & ESCENG_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_USE_SHIFT_SPACE, 
                                (dwOptionFlag & USE_SHIFT_SPACE) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_SIMILAR_WORDS, 
                                (dwOptionFlag & SEARCH_SIMILAR_WORDS) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_HANJA_CAND_SPACE, 
                                (dwOptionFlag & HANJA_CAND_WITH_SPACE) ? 1 : 0);

            if (dwToggleKey) {
                TCHAR szName[128];
                getToggleKeyName(szName, 128);
                if ((hwndRadio = GetDlgItem (hDlg, IDC_USE_SHIFT_SPACE))) {
                    LPTSTR lpDesc = szName + Mylstrlen(szName);
                    *lpDesc = TEXT(' ');
                    lpDesc ++;
                    LoadString(hInst, IDS_HANGUL_TOGGLE, lpDesc, 128);

                    SendMessage(hwndRadio, WM_SETTEXT, 0, (LPARAM) szName);
                    MyDebugPrint((TEXT(" >>>>>>> Toggle Key Name ='%s'\n"), szName));
                }
            }

            /* check dvorak layout */
            hCur= GetKeyboardLayout(0);
            if (hCur != LongToHandle(0xE0120412)) {
                hwndRadio = GetDlgItem (hDlg, IDC_DVORAK_SUPPORT);
                EnableWindow (hwndRadio, FALSE);
                hwndRadio = GetDlgItem (hDlg, IDC_QWERTY_HOTKEY_SUPPORT);
                EnableWindow (hwndRadio, FALSE);
            } else if (!(dwOptionFlag & DVORAK_SUPPORT)) {
                hwndRadio = GetDlgItem (hDlg, IDC_QWERTY_HOTKEY_SUPPORT);
                EnableWindow (hwndRadio, FALSE);
            }

            /* check shift-space */
            if (!(dwOptionFlag & USE_SHIFT_SPACE)) {
                hwndRadio = GetDlgItem (hDlg, IDC_HANGUL_TOGGLE);
                EnableWindow (hwndRadio, FALSE);
            }

            /* Layout */
            if (dwLayoutFlag == 0)
                dwLayoutFlag = LAYOUT_OLD2BUL;
            CheckDlgButton(hDlg, IDC_LAYOUT_OLD2BUL, 
                                (dwLayoutFlag == LAYOUT_OLD2BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_3FIN, 
                                (dwLayoutFlag == LAYOUT_3FIN) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_390, 
                                (dwLayoutFlag == LAYOUT_390) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_NK_2BUL, 
                                (dwLayoutFlag == LAYOUT_NK_2BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_YET2BUL, 
                                (dwLayoutFlag == LAYOUT_YET2BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_YET3BUL, 
                                (dwLayoutFlag == LAYOUT_YET3BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_NEW2BUL, 
                                (dwLayoutFlag == LAYOUT_NEW2BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_NEW3BUL, 
                                (dwLayoutFlag == LAYOUT_NEW3BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_AHNMATAE, 
                                (dwLayoutFlag == LAYOUT_AHNMATAE) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_3SUN, 
                                (dwLayoutFlag == LAYOUT_3SUN) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_USER, 
                                (dwLayoutFlag >= LAYOUT_USER) ? 1 : 0);

            if (GetRegKeyHandle(TEXT("\\Keyboard"), &hKey)) {
                int i;
                for (i = 0; i <= 5; i++) {
		    LPTSTR lpDesc;
		    TCHAR  szDesc[128];

		    lpDesc = (LPTSTR)&szDesc;
		    LoadString(hInst, IDS_KEY_NEW2BUL + i, lpDesc, 128);
		    // get IDS_KEY_XXX string

		    // Query if the user-defined keyboard exists or not. 
		    if (RegQueryValueEx(hKey, lpDesc, 0, NULL, NULL, NULL) != ERROR_SUCCESS) {
                        hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_NEW2BUL + i);
                        EnableWindow (hwndRadio, FALSE);
                    }
                }
                RegCloseKey(hKey);
            }
            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            //dwTemp = LAYOUT_NEW2BUL;
            //SetDwordToSetting(TEXT("LayoutFlag"), dwLayoutFlag);
            switch(HIWORD(wParam)) {
            case BN_CLICKED:
                {
                    UINT i = LOWORD(wParam);
                    if ( i >= IDC_LAYOUT_OLD2BUL && i < IDC_LAYOUT_USER) {
                        hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_USER);
                        SendMessage(hwndRadio, CB_SETCURSEL, -1, 0); // clear User defined KBD
                        break;
                    }
                }
                switch(LOWORD(wParam)) {
                case IDC_KSX1001_SUPPORT:
                    hwndRadio = GetDlgItem (hDlg, IDC_JAMOS);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                case IDC_JAMOS:
                    hwndRadio = GetDlgItem (hDlg, IDC_KSX1001_SUPPORT);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);

                    hwndRadio = GetDlgItem (hDlg, IDC_FULL_MULTIJOMO);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                case IDC_FULL_MULTIJOMO:
                    hwndRadio = GetDlgItem (hDlg, IDC_JAMOS);
                    SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                    break;
                case IDC_DVORAK_SUPPORT:
                    hwndRadio = GetDlgItem (hDlg, IDC_QWERTY_HOTKEY_SUPPORT);
                    if (IsDlgButtonChecked(hDlg, IDC_DVORAK_SUPPORT)) {
                        EnableWindow (hwndRadio, TRUE);
                    } else {
                        EnableWindow (hwndRadio, FALSE);
                    }
                    break;
                case IDC_HANGUL_TOGGLE:
                    if (DialogBox(hInst, MAKEINTRESOURCE(DLG_TOGGLE_KEY), hDlg, SelectToggleDlgProc)) {
                        if (dwToggleKey) {
                            TCHAR szName[128];
                            getToggleKeyName(szName, 128);
                            if ((hwndRadio = GetDlgItem (hDlg, IDC_USE_SHIFT_SPACE))) {
                                LPTSTR lpDesc = szName + Mylstrlen(szName);
                                *lpDesc = TEXT(' ');
                                lpDesc ++;
                                LoadString(hInst, IDS_HANGUL_TOGGLE, lpDesc, 128);

                                SendMessage(hwndRadio, WM_SETTEXT, 0, (LPARAM) szName);
                                MyDebugPrint((TEXT(" >>>>>>> Toggle Key Name ='%s'\n"), szName));
                            }
                        }
                    }
                    break;
                case IDC_USE_SHIFT_SPACE:
                    hwndRadio = GetDlgItem (hDlg, IDC_HANGUL_TOGGLE);
                    if (IsDlgButtonChecked(hDlg, IDC_USE_SHIFT_SPACE)) {
                        EnableWindow (hwndRadio, TRUE);
                    } else {
                        EnableWindow (hwndRadio, FALSE);
                    }
                    break;
                default:
                    break;
                }
                break;
            case CBN_CLOSEUP:
                switch(LOWORD(wParam)) {
                case IDC_LAYOUT_USER:
                    if ((hwndRadio = GetDlgItem (hDlg, IDC_LAYOUT_USER))) {
                        UINT ret;
                        UINT i;
                        ret = (UINT)SendMessage(hwndRadio, CB_GETCURSEL, 0, 0);
                        if (ret != CB_ERR) {
                            SendMessage(hwndRadio, CB_SETCURSEL, ret, 0);
                            MyDebugPrint((TEXT(" >>>>>>> Keyboard dialog : %d\n"), ret));
                            //dwLayoutFlag = LAYOUT_USER + ret;
                            //SetDwordToSetting(TEXT("LayoutFlag"), dwLayoutFlag);
                        }

                        for (i = IDC_LAYOUT_OLD2BUL; i < IDC_LAYOUT_USER; i++) {
                            if (!(hwndRadio = GetDlgItem (hDlg, i)))
                                break;
                            if (IsDlgButtonChecked(hDlg, i)) {
                                SendMessage(hwndRadio, BM_SETCHECK, BST_UNCHECKED, 0);
                                break;
                            }
                        }
                    }
                    break;
                }
                break;
            }

            break;

        default:
            return FALSE;

    }
    return TRUE;
} 

/**********************************************************************/
/*                                                                    */
/*      DebugOptionDlgProc()                                          */
/*                                                                    */
/**********************************************************************/
INT_PTR CALLBACK DebugOptionDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam)
{
    DWORD dwTemp;
    TCHAR szBuf[128];
    NMHDR FAR *lpnm;
    LPPROPSHEETPAGE lpPropSheet = (LPPROPSHEETPAGE)(GetWindowLongPtr(hDlg, DWLP_USER));

    switch(message)
    {
        case WM_NOTIFY:
            lpnm = (NMHDR FAR *)lParam;
            switch(lpnm->code)
            {
                case PSN_SETACTIVE:
                    break;
        
                case PSN_KILLACTIVE:
                    break;
        
                case PSN_APPLY:
#ifdef DEBUG
                    dwTemp = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_LOGF_ENTRY))
                        dwTemp |= LOGF_ENTRY;
                    if (IsDlgButtonChecked(hDlg, IDC_LOGF_API))
                        dwTemp |= LOGF_API;
                    if (IsDlgButtonChecked(hDlg, IDC_LOGF_KEY))
                        dwTemp |= LOGF_KEY;
                    dwLogFlag = dwTemp;

                    dwTemp = 0;
                    if (IsDlgButtonChecked(hDlg, IDC_DEBF_THREADID))
                        dwTemp |= DEBF_THREADID;
                    if (IsDlgButtonChecked(hDlg, IDC_DEBF_GUIDELINE))
                        dwTemp |= DEBF_GUIDELINE;
                    dwDebugFlag = dwTemp;

                    SetDwordToSetting(TEXT("LogFlag"), dwLogFlag);
                    SetDwordToSetting(TEXT("DebugFlag"), dwDebugFlag);
#endif

                    break;

                case PSN_RESET:
                    break;

                case PSN_HELP:
                    break;

                default:
                    return FALSE;
            }
            break;

        case WM_INITDIALOG:
            SetWindowLongPtr(hDlg, DWLP_USER, (LONG_PTR) lParam);
            lpPropSheet = (LPPROPSHEETPAGE)lParam;

#ifdef DEBUG
            CheckDlgButton(hDlg, IDC_LOGF_ENTRY, 
                                (dwLogFlag & LOGF_ENTRY) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LOGF_API, 
                                (dwLogFlag & LOGF_API) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LOGF_KEY, 
                                (dwLogFlag & LOGF_KEY) ? 1 : 0);
            
            CheckDlgButton(hDlg, IDC_DEBF_THREADID, 
                                (dwDebugFlag & DEBF_THREADID) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_DEBF_GUIDELINE, 
                                (dwDebugFlag & DEBF_GUIDELINE) ? 1 : 0);
#endif
            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            break;

        default:
            return FALSE;

    }
    return TRUE;
} 

#if 1
#define SAENARU_KEYBOARD_2SET 2
#define SAENARU_KEYBOARD_3SET 3
#define SAENARU_KEYBOARD_RAW  4

#define SAENARU_COMPOSE_DEFAULT  0
#define SAENARU_COMPOSE_2SET     2
#define SAENARU_COMPOSE_3SET     0
#define SAENARU_COMPOSE_AHN      3
#define SAENARU_COMPOSE_YET      4

static
DWORD string_to_hex(TCHAR* p)
{
    DWORD ret = 0;
    UINT remain = 0;

    if (*p == 'U')
        p++;

    while (*p != '\0') {
        if (*p >= '0' && *p <= '9')
            remain = *p - '0';
        else if (*p >= 'a' && *p <= 'f')
            remain = *p - 'a' + 10;
        else if (*p >= 'A' && *p <= 'F')
            remain = *p - 'A' + 10;
        else
            return 0;

        ret = ret * 16 + remain;
        p++;
    }
    return ret;
}

UINT
load_keyboard_map_from_reg(LPCTSTR lpszKeyboard, UINT nKeyboard, WCHAR *keyboard_map)
{
    int i;
    WCHAR *line, *p, *saved_position;
    WCHAR *save;
    TCHAR buf[256];
    //FILE* file;
    DWORD key, value;
    UINT type, ctype;
    INT sz, len;
    WCHAR name[256];
    //LPTSTR kbuf=NULL;
    LPTSTR kbuf=NULL;
    WCHAR _map[94];

    WCHAR achValue[256]; 

    //NabiKeyboardMap *keyboard_map;
    //
    if (lpszKeyboard == NULL && nKeyboard <10)
    {
        DWORD cchValue = 256;
        DWORD retCode;
        HKEY hKey;

        if (!GetRegKeyHandle(TEXT("\\Keyboard"), &hKey))
            return 0;

        achValue[0] = '\0'; 
        retCode = RegEnumValue(hKey, nKeyboard, 
            achValue, 
            &cchValue, 
            NULL, 
            NULL,
            NULL,
            NULL);

        if (retCode != ERROR_SUCCESS ) 
        { 
            MyDebugPrint((TEXT("(%d) %s\n"), nKeyboard, achValue));
            return 0;
        }
        lpszKeyboard = (LPCTSTR) &achValue;
    }

    sz= GetRegMultiStringValue(TEXT("\\Keyboard"),lpszKeyboard,NULL);
    if (sz <= 0) {
            MyDebugPrint((TEXT("Saenaru: Keyboard not found\n")));
            return 0;
    }

    MyDebugPrint((TEXT("Saenaru: reg size %d\n"), sz));
    kbuf=(LPTSTR) MEMALLOC(sz);
    //
    if (kbuf == (LPTSTR)NULL) {
            MyDebugPrint((TEXT("Saenaru: Can't read keyboard registry\n")));
            return 0;
    }

    GetRegMultiStringValue(TEXT("\\Keyboard"),lpszKeyboard,kbuf);
    //keyboard_map = g_malloc(sizeof(NabiKeyboardMap));

    /* init */
    type = SAENARU_KEYBOARD_3SET;
    ctype = SAENARU_COMPOSE_DEFAULT;
    //keyboard_map->filename = g_strdup(filename);
    //name = NULL;
    //MyDebugPrint((TEXT("Saenaru: %s\n"),kbuf));

    for (i = 0; i < 94; i++)
            _map[i] = i + '!';
            //_map[i] = 0;

    for (line = Mystrtok(kbuf, TEXT("\0"), &save);
         ;
         line = Mystrtok(saved_position, TEXT("\0"), &save)) {
        len=Mylstrlen(line);
        saved_position=line+len+1;

        //MyDebugPrint((TEXT("Saenaru: %s:%d\n"),line,len));
        if (len==0) break;

        p = Mystrtok(line, TEXT(" \t\0"), &save);
        MyDebugPrint((TEXT("tok: %s\n"),p));
        /* comment */
        if (p == NULL || p[0] == '#')
            continue;

        if (Mylstrcmp(p, TEXT("Name:")) == 0) {
            p = Mystrtok(NULL, TEXT(" \t\0"), &save);
            if (p == NULL)
                continue;
            //name = g_strdup(p);
            continue;
        } else if (Mylstrcmp(p, TEXT("Compose:")) == 0) {
            // Set default compose map
            p = Mystrtok(NULL, TEXT(" \t\0"), &save);
            if (p == NULL)
                continue;
            if (Mylstrcmp(p, TEXT("yet")) == 0) {
                ctype = SAENARU_COMPOSE_YET;
            } else if (Mylstrcmp(p, TEXT("full")) == 0) {
                ctype = SAENARU_COMPOSE_YET;
            } else if (Mylstrcmp(p, TEXT("2set")) == 0) {
                ctype = SAENARU_COMPOSE_2SET;
            } else if (Mylstrcmp(p, TEXT("3set")) == 0) {
                ctype = SAENARU_COMPOSE_3SET;
            } else if (Mylstrcmp(p, TEXT("ahn")) == 0) {
                ctype = SAENARU_COMPOSE_AHN;
            }
            //name = g_strdup(p);
            continue;
        } else if (Mylstrcmp(p, TEXT("Type2")) == 0) {
            type = SAENARU_KEYBOARD_2SET;
        } else {
            key = string_to_hex(p);
            if (key == 0)
                continue;

            p = Mystrtok(NULL, TEXT(" \t"), &save);
            if (p == NULL)
                continue;
            value = string_to_hex(p);
            if (value == 0)
                continue;

            if (key < '!' || key > '~')
                continue;

            MyDebugPrint((TEXT("REG %x=%x\n"),(DWORD)key,(DWORD)value));

            _map[key - '!'] = (WCHAR)value;
        }
    }
    MEMFREE(kbuf);

    if (keyboard_map != NULL)
        for (i=0;i<94;i++) keyboard_map[i]=_map[i];        

    //if (name == NULL)
    //        name = g_path_get_basename(keyboard_map->filename);

    return type | (ctype << 8);
}
#endif

#define SAENARU_AUTOMATA_NONE 1
#define SAENARU_AUTOMATA_2SET 2
#define SAENARU_AUTOMATA_3SET 3
#define SAENARU_AUTOMATA_RAW  4
UINT
load_compose_map_from_reg(LPCTSTR lpszCompose, UINT nCompose, HangulCompose *compose_map)
{
    WCHAR *line, *p, *saved_position;
    WCHAR *save;
    TCHAR buf[256];
    //FILE* file;
    UINT key1, key2;
    DWORD value;
    //NabiComposeItem *citem;
    UINT map_size;
    //GSList *list = NULL;
    INT sz, len;
    WCHAR name[256];
    //LPTSTR kbuf=NULL;
    LPTSTR kbuf=NULL;
    UINT _key[256];
    WCHAR _code[256];
    UINT id=0,i;
    UINT type=SAENARU_AUTOMATA_NONE;

    WCHAR achValue[256];

    if (lpszCompose == NULL && nCompose <10)
    {
        DWORD cchValue = 256;
        DWORD retCode;
        HKEY hKey;

        if (!GetRegKeyHandle(TEXT("\\Compose"), &hKey))
            return 0;

        achValue[0] = '\0'; 
        retCode = RegEnumValue(hKey, nCompose, 
            achValue, 
            &cchValue, 
            NULL, 
            NULL,
            NULL,
            NULL);

        if (retCode != ERROR_SUCCESS ) 
        { 
            MyDebugPrint((TEXT("(%d) %s\n"), nCompose, achValue));
            return 0;
        }
        lpszCompose = (LPCTSTR) &achValue;
    }

    sz= GetRegMultiStringValue(TEXT("\\Compose"),lpszCompose,NULL);
    if (sz <= 0) {
            MyDebugPrint((TEXT("Saenaru: ComposeMap not found\n")));
            return 0;
    }

    MyDebugPrint((TEXT("Saenaru: reg size %d\n"), sz));
    kbuf=(LPTSTR) MEMALLOC(sz);
    //
    if (kbuf == (LPTSTR)NULL) {
            MyDebugPrint((TEXT("Saenaru: Can't read compose map registry\n")));
            return 0;
    }

    GetRegMultiStringValue(TEXT("\\Compose"),lpszCompose,kbuf);
    /* init */
    //compose_map->name = NULL;
    //compose_map->map = NULL;
    //compose_map->size = 0;

    for (line = Mystrtok(kbuf, TEXT("\0"), &save);id<256
	 ;
	 line = Mystrtok(saved_position, TEXT("\0"), &save)) {
        len=Mylstrlen(line);
        saved_position=line+len+1;

        if (len==0) break;

	p = Mystrtok(line, TEXT(" \t\0"), &save);
        MyDebugPrint((TEXT("tok: %s\n"),p));
	/* comment */
	if (p == NULL || p[0] == '#')
	    continue;

	if (Mylstrcmp(p, TEXT("Name:")) == 0) {
	    p = Mystrtok(NULL, TEXT(" \t\0"), &save);
	    if (p == NULL)
		continue;
	    //compose_map->name = g_strdup(p);
	    continue;
        } else if (Mylstrcmp(p, TEXT("Type0")) == 0) {
            type = SAENARU_AUTOMATA_RAW;
	    //compose_map->name = g_strdup(p);
	    continue;
	} else {
	    key1 = string_to_hex(p);
	    if (key1 == 0)
		continue;

	    p = Mystrtok(NULL, TEXT(" \t"), &save);
	    if (p == NULL)
		continue;
	    key2 = string_to_hex(p);
	    if (key2 == 0)
		continue;

	    p = Mystrtok(NULL, TEXT(" \t"), &save);
	    if (p == NULL)
		continue;
	    value = string_to_hex(p);
	    if (value == 0)
		continue;

	    _key[id] = key1 << 16 | key2;
	    _code[id] = (WCHAR)value;
            id++;
	}
    }
    MEMFREE(kbuf);
    map_size=(UINT)id;

    //if (compose_map == NULL) {
	/* on error free the list */
//	while (list != NULL) {
//	    g_free(list->data);
//	    list = list->next;
//	}
//	g_slist_free(list);
//
//	return FALSE;
//    }

    /* sort compose map */
//    list = g_slist_reverse(list);
//    list = g_slist_sort(list, compose_item_compare);

    /* move data to map */
//    map_size = g_slist_length(list);
//    compose_map->map = (NabiComposeItem**)
//		g_malloc(map_size * sizeof(NabiComposeItem*));
    for (i = 0; i < map_size; i++) {
	compose_map[i].key = _key[i];
	compose_map[i].code = _code[i];
    }
    compose_map[map_size].key=(UINT)-1;

    /* free the list */
    //g_slist_free(list);

    return type;
}


/*
 * ex: ts=8 sts=4 sw=4 et
 */
