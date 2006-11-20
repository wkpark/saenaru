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
 * $Saenaru: saenaru/src/config.c,v 1.7 2006/11/09 11:45:33 wkpark Exp $
 */

#include "windows.h"
#include "immdev.h"
#include "saenaru.h"
#include "prsht.h"
#include "resource.h"
#include "malloc.h"

#define MAX_PAGES 3



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
    psh.dwFlags = PSH_PROPTITLE;
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
            AddPage(&psh, DLG_ABOUT, AboutDlgProc);
#ifdef DEBUG
            AddPage(&psh, DLG_DEBUG, DebugOptionDlgProc);
#endif
            PropertySheet(&psh);
            break;

        default:
            break;
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
                    if (IsDlgButtonChecked(hDlg, IDC_USE_SHIFT_SPACE))
                        dwTemp |= USE_SHIFT_SPACE;
                    if (IsDlgButtonChecked(hDlg, IDC_DVORAK_SUPPORT))
                        dwTemp |= DVORAK_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_QWERTY_HOTKEY_SUPPORT))
                        dwTemp |= QWERTY_HOTKEY_SUPPORT;
                    if (IsDlgButtonChecked(hDlg, IDC_ESCENG_SUPPORT))
                        dwTemp |= ESCENG_SUPPORT;
                    dwOptionFlag = dwTemp;

                    SetDwordToSetting(TEXT("OptionFlag"), dwOptionFlag);

                    dwTemp = 0;
                    if (dwLayoutFlag == 0)
                        dwLayoutFlag = LAYOUT_OLD2BUL;
                    if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_OLD2BUL))
                        dwTemp = LAYOUT_OLD2BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_3FIN))
                        dwTemp = LAYOUT_3FIN;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_390))
                        dwTemp = LAYOUT_390;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_NEW2BUL))
                        dwTemp = LAYOUT_NEW2BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_NEW3BUL))
                        dwTemp = LAYOUT_NEW3BUL;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_AHNMATAE))
                        dwTemp = LAYOUT_AHNMATAE;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_3SUN))
                        dwTemp = LAYOUT_3SUN;
                    else if (IsDlgButtonChecked(hDlg, IDC_LAYOUT_USER))
                        dwTemp = LAYOUT_USER;
                    dwLayoutFlag = dwTemp;

                    SetDwordToSetting(TEXT("LayoutFlag"), dwLayoutFlag);

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
            CheckDlgButton(hDlg, IDC_DVORAK_SUPPORT, 
                                (dwOptionFlag & DVORAK_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_QWERTY_HOTKEY_SUPPORT, 
                                (dwOptionFlag & QWERTY_HOTKEY_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_ESCENG_SUPPORT, 
                                (dwOptionFlag & ESCENG_SUPPORT) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_USE_SHIFT_SPACE, 
                                (dwOptionFlag & USE_SHIFT_SPACE) ? 1 : 0);

            /* Layout */
            if (dwLayoutFlag == 0)
                dwLayoutFlag = LAYOUT_OLD2BUL;
            CheckDlgButton(hDlg, IDC_LAYOUT_OLD2BUL, 
                                (dwLayoutFlag == LAYOUT_OLD2BUL) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_3FIN, 
                                (dwLayoutFlag == LAYOUT_3FIN) ? 1 : 0);
            CheckDlgButton(hDlg, IDC_LAYOUT_390, 
                                (dwLayoutFlag == LAYOUT_390) ? 1 : 0);
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
            break;

        case WM_DESTROY:
            break;

        case WM_HELP:
            break;

        case WM_CONTEXTMENU:   // right mouse click
            break;

        case WM_COMMAND:
            dwTemp = LAYOUT_NEW2BUL;
            //SetDwordToSetting(TEXT("Layout"), dwLayoutFlag);
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
    TCHAR buf[256];
    //FILE* file;
    DWORD key, value;
    UINT type;
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
    kbuf=(LPTSTR) malloc(sz);
    //
    if (kbuf == (LPTSTR)NULL) {
            MyDebugPrint((TEXT("Saenaru: Can't read keyboard registry\n")));
            return 0;
    }

    GetRegMultiStringValue(TEXT("\\Keyboard"),lpszKeyboard,kbuf);
    //keyboard_map = g_malloc(sizeof(NabiKeyboardMap));

    /* init */
    type = SAENARU_KEYBOARD_3SET;
    //keyboard_map->filename = g_strdup(filename);
    //name = NULL;
    //MyDebugPrint((TEXT("Saenaru: %s\n"),kbuf));

    for (i = 0; i < 94; i++)
            _map[i] = i + '!';
            //_map[i] = 0;

    for (line = Mystrtok(kbuf, TEXT("\0"));
         ;
         line = Mystrtok(saved_position, TEXT("\0"))) {
        len=Mylstrlen(line);
        saved_position=line+len+1;

        //MyDebugPrint((TEXT("Saenaru: %s:%d\n"),line,len));
        if (len==0) break;

        p = Mystrtok(line, TEXT(" \t\0"));
        MyDebugPrint((TEXT("tok: %s\n"),p));
        /* comment */
        if (p == NULL || p[0] == '#')
            continue;

        if (Mylstrcmp(p, TEXT("Name:")) == 0) {
            p = Mystrtok(NULL, TEXT("\0"));
            if (p == NULL)
                continue;
            //name = g_strdup(p);
            continue;
        } else if (Mylstrcmp(p, TEXT("Type2")) == 0) {
            type = SAENARU_KEYBOARD_2SET;
        } else {
            key = string_to_hex(p);
            if (key == 0)
                continue;

            p = Mystrtok(NULL, TEXT(" \t"));
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
    free(kbuf);

    if (keyboard_map != NULL)
        for (i=0;i<94;i++) keyboard_map[i]=_map[i];        

    //if (name == NULL)
    //        name = g_path_get_basename(keyboard_map->filename);

    return type;
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
    kbuf=(LPTSTR) malloc(sz);
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

    for (line = Mystrtok(kbuf, TEXT("\0"));id<256
	 ;
	 line = Mystrtok(saved_position, TEXT("\0"))) {
        len=Mylstrlen(line);
        saved_position=line+len+1;

        if (len==0) break;

	p = Mystrtok(line, TEXT(" \t\0"));
        MyDebugPrint((TEXT("tok: %s\n"),p));
	/* comment */
	if (p == NULL || p[0] == '#')
	    continue;

	if (Mylstrcmp(p, TEXT("Name:")) == 0) {
	    p = Mystrtok(NULL, TEXT("\0"));
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

	    p = Mystrtok(NULL, TEXT(" \t"));
	    if (p == NULL)
		continue;
	    key2 = string_to_hex(p);
	    if (key2 == 0)
		continue;

	    p = Mystrtok(NULL, TEXT(" \t"));
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
    free(kbuf);
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
