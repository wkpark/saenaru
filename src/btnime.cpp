/*
 * This file is part of Saenaru.
 *
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

#if !defined (NO_TSF)
/*    LanguageBar の「入力方式？」??ンを司る。
 */
#include "windows.h"
#include "tchar.h"
#include "immdev.h"

extern "C" {
#include "saenaru.h"
#include "resource.h"
}
/*    Platform SDK に依存する部分。Platform SDK に include path
 *    を通すのが良いのか否か。出?れば DDK にこれらの header を
 *    渡してくれると良いのだが。
 */
//#include "c:\Program Files\Microsoft SDK\include\msctf.h"
#include "msctf.h"
#include "olectl.h"
#if !defined (TARGET_WIN2K)
//#include "c:\Program Files\Microsoft SDK\include\uxtheme.h"
#include "uxtheme.h"
#endif
#include "tsf.h"

//#define LANGBAR_ITEM_DESC    L"글쇠/입력기 설정" // max 32 chars!
#define SAENARU_LANGBARITEMSINK_COOKIE    0x0fab0fac

static void  _Menu_Help (UINT);
static void  _Menu_Property (UINT);
static void  _Menu_Reconversion (UINT);
static void  _Menu_ToggleShowKeyboard (UINT);
static void  _Menu_ToggleDvorak (UINT);
static void  _Menu_ToggleEscEng (UINT);
static void  _Menu_ToggleHanEng (UINT);
static void  _Menu_ToggleOnTheSpot (UINT);
static void  _Menu_SelectKeyboard (UINT);
static void  _Menu_SelectCompose (UINT);
static DWORD _MenuItem_GetNormalFlag (UINT);
static DWORD _MenuItem_GetKeyboardFlag (UINT);
static DWORD _MenuItem_GetComposeFlag (UINT);
static DWORD _MenuItem_GetToggleKeyboardFlag (UINT);
static DWORD _MenuItem_GetToggleComposeFlag (UINT);
static DWORD _MenuItem_GetToggleDvorakFlag (UINT);
static DWORD _MenuItem_GetToggleEscEngFlag (UINT);
static DWORD _MenuItem_GetToggleHanEngFlag (UINT);
static DWORD _MenuItem_GetToggleOnTheSpotFlag (UINT);
static DWORD _UserKeyboardMenu_GetKeyboardFlag (UINT);

typedef struct {
    //const WCHAR* pchDesc;
    UINT	 chDesc;
    DWORD        (*pfnGetFlag)(UINT);
    void         (*pfnHandler)(UINT);
} TSFLBMENUINFOEX;

static const TSFLBMENUINFOEX c_rgMenuItems[] = {
#if 0
//  { L"도움말(&H)",   _MenuItem_GetNormalFlag, _Menu_Help },
    { L"환경설정(&R)", _MenuItem_GetNormalFlag, _Menu_Property },
//  { L"재설정(&C)",   _MenuItem_GetNormalFlag, _Menu_Reconversion },
    { NULL, NULL, NULL },
    { L"드보락(&V)",   _MenuItem_GetToggleDvorakFlag,   _Menu_ToggleDvorak },
    { L"ESC 영문전환(&E)", _MenuItem_GetToggleEscEngFlag, _Menu_ToggleEscEng },
    { L"단어단위 편집(&W)", _MenuItem_GetToggleOnTheSpotFlag,_Menu_ToggleOnTheSpot },
    { NULL, NULL, NULL },
    { L"키보드 보기",  _MenuItem_GetToggleKeyboardFlag, _Menu_ToggleShowKeyboard },
    { NULL, NULL, NULL },
    { L"취소", NULL, NULL }
#endif
//  { L"도움말(&H)",   _MenuItem_GetNormalFlag, _Menu_Help },
    { IDS_MENU_ENV, _MenuItem_GetNormalFlag, _Menu_Property },
//  { L"재설정(&C)",   _MenuItem_GetNormalFlag, _Menu_Reconversion },
    { NULL, NULL, NULL },
    { IDS_MENU_DVORAK,   _MenuItem_GetToggleDvorakFlag,   _Menu_ToggleDvorak },
    { IDS_MENU_ESC_ASCII, _MenuItem_GetToggleEscEngFlag, _Menu_ToggleEscEng },
    { IDS_MENU_HANGUL_TOGGLE, _MenuItem_GetToggleHanEngFlag,_Menu_ToggleHanEng },
    { IDS_MENU_EDIT_WORD_UNIT, _MenuItem_GetToggleOnTheSpotFlag,_Menu_ToggleOnTheSpot },
    { NULL, NULL, NULL },
    { IDS_MENU_SHOW_KEYBOARD,  _MenuItem_GetToggleKeyboardFlag, _Menu_ToggleShowKeyboard },
    { NULL, NULL, NULL },
    { IDS_MENU_CANCEL, NULL, NULL }
};

typedef struct {
    //const WCHAR* pchDesc;
    UINT         chDesc;
    DWORD        dwFlag;
} TSFLBKEYBOARDINFOEX;

static const TSFLBKEYBOARDINFOEX c_rgKeyboardItems[]= {
#if 0
    { L"두벌식(&2)",        NULL },
    { L"세벌식(&3)",        NULL },
    { L"세벌식390(&9)",     NULL },
    { L"새두벌(&D)",        NULL },
    { L"새세벌(&S)",        NULL },
    { L"안마태(&A)",        NULL },
    { L"세벌식 순아래(&N)", NULL },
    { L"사용자 자판(&U)",   TF_LBMENUF_SUBMENU },
    { L"사용자 조합(&C)",   TF_LBMENUF_SUBMENU },
#endif
    { IDS_MENU_OLD2BUL  ,   NULL },
    { IDS_MENU_3FIN     ,   NULL },
    { IDS_MENU_3SOON    ,   NULL },
    { IDS_MENU_390      ,   NULL },
    { IDS_MENU_NK_2BUL  ,   NULL },
    { IDS_MENU_YET2BUL  ,   NULL },
    { IDS_MENU_YET3BUL  ,   NULL },
    { IDS_MENU_NEW2BUL  ,   NULL },
    { IDS_MENU_NEW3BUL  ,   NULL },
    { IDS_MENU_AHNMATAE ,   NULL },
    { IDS_MENU_USER     ,   TF_LBMENUF_SUBMENU },
    { IDS_MENU_USER_COMP,   TF_LBMENUF_SUBMENU },
};

enum {
    MENU_ITEM_INDEX_CANCEL        = -1,
    //MENU_ITEM_INDEX_HELP,
    MENU_ITEM_INDEX_PROPERTY,
    //MENU_ITEM_INDEX_RECONVRESION,
    MENU_ITEM_INDEX_OLD2BUL = 2,
    MENU_ITEM_INDEX_OLD3BUL,
    MENU_ITEM_INDEX_3FIN,
};

/*    …??ンの?だけ class が必要だと考えるのが良さそうだ。
 *    後から追加しないといけないかな。
 */
class CLangBarItemImeButton : public ITfLangBarItemButton,
                             public ITfSource
{
public:
    CLangBarItemImeButton ();
    ~CLangBarItemImeButton ();

    // IUnknown
    STDMETHODIMP QueryInterface (REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef (void);
    STDMETHODIMP_(ULONG) Release (void);
    
    // ITfLangBarItem
    STDMETHODIMP GetInfo (TF_LANGBARITEMINFO *pInfo);
    STDMETHODIMP GetStatus (DWORD *pdwStatus);
    STDMETHODIMP Show (BOOL fShow);
    STDMETHODIMP GetTooltipString (BSTR *pbstrToolTip);
    
    // ITfLangBarItemButton
    STDMETHODIMP OnClick (TfLBIClick click, POINT pt, const RECT *prcArea);
    STDMETHODIMP InitMenu (ITfMenu *pMenu);
    STDMETHODIMP OnMenuSelect (UINT wID);
    STDMETHODIMP GetIcon (HICON *phIcon);
    STDMETHODIMP GetText (BSTR *pbstrText);
    
    // ITfSource
    STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
    STDMETHODIMP UnadviseSink(DWORD dwCookie);

private:
    ITfLangBarItemSink* _pLangBarItemSink;
    TF_LANGBARITEMINFO  _tfLangBarItemInfo;
    LONG                _cRef;
};

CLangBarItemImeButton::CLangBarItemImeButton ()
{
    //DllAddRef ();
    LPTSTR lpDesc;

    _tfLangBarItemInfo.clsidService   = c_clsidSaenaruTextService;
    //_tfLangBarItemInfo.clsidService = CLSID_NULL;
    _tfLangBarItemInfo.guidItem       = c_guidItemButtonIME;
    _tfLangBarItemInfo.dwStyle        = TF_LBI_STYLE_BTN_MENU
        | TF_LBI_STYLE_SHOWNINTRAY
#if 0
        | TF_LBI_STYLE_HIDDENSTATUSCONTROL
#endif
        ;
    lpDesc=(LPTSTR)&_tfLangBarItemInfo.szDescription;
    _tfLangBarItemInfo.ulSort = 1;
    LoadString(hInst,IDS_KEYBOARD_OPTION_DESC,lpDesc,ARRAYSIZE (_tfLangBarItemInfo.szDescription));

    //SafeStringCopy (_tfLangBarItemInfo.szDescription, ARRAYSIZE (_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);
    _pLangBarItemSink = NULL;
    _cRef             = 1;
    return;
}

CLangBarItemImeButton::~CLangBarItemImeButton ()
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::~CLangBarItemImeButton (this:%p)\n"), this));
    //DllRelease ();
    return;
}

STDAPI
CLangBarItemImeButton::QueryInterface (
    REFIID riid,
    void** ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;
    if (IsEqualIID (riid, IID_IUnknown) ||
        IsEqualIID (riid, IID_ITfLangBarItem) ||
        IsEqualIID (riid, IID_ITfLangBarItemButton)) {
        *ppvObj = (ITfLangBarItemButton *)this;
    } else if (IsEqualIID (riid, IID_ITfSource)) {
        *ppvObj = (ITfSource *)this;
    }
    if (*ppvObj != NULL) {
        AddRef ();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDAPI_(ULONG)
CLangBarItemImeButton::AddRef ()
{
    return ++_cRef;
}

STDAPI_(ULONG)
CLangBarItemImeButton::Release ()
{
    LONG cr = --_cRef;

    if (_cRef == 0) {
        delete this;
    }
    return cr;
}

STDAPI
CLangBarItemImeButton::GetInfo (
    TF_LANGBARITEMINFO* pInfo)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::GetInfo (this:%p)\n"), this));
    
    if (pInfo == NULL)
        return E_INVALIDARG;

    *pInfo = _tfLangBarItemInfo;
    return S_OK;
}

STDAPI
CLangBarItemImeButton::Show (
    BOOL fShow)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::Show (BOOL:%d)\n"), fShow));
    return E_NOTIMPL;
}

STDAPI
CLangBarItemImeButton::GetStatus (
    DWORD* pdwStatus)
{
    if (pdwStatus == NULL)
        return E_INVALIDARG;
#if 0
    *pdwStatus = (_GetCurrentHIMC () != NULL)? 0 : TF_LBI_STATUS_DISABLED;
#endif
    *pdwStatus = 0;
    return S_OK;
}

/*    Button の tooltip を返す。返す値は SysAllocString によって
 *    確保した領域に書かれる必要がある。これを SysFreeString す
 *    るのは、呼び出した側の責任である。
 */
STDAPI
CLangBarItemImeButton::GetTooltipString (
    BSTR* pbstrToolTip)
{
    if (pbstrToolTip == NULL)
        return E_INVALIDARG;

    *pbstrToolTip = SysAllocString (_tfLangBarItemInfo.szDescription);
    return (*pbstrToolTip == NULL)? E_OUTOFMEMORY : S_OK;
}

/*    ITfLangBarItemButton::OnClick
 *
 *    この method はユ?ザが言語バ?の TF_LBI_STYLE_BTN_BUTTON また
 *    は TF_LBI_STYLE_BTN_TOGGLE ス?イルを持っている??ンの上で?
 *    ウスをクリックした時に呼び出される。
 *    もし??ン item が TF_LBI_STYLE_BTN_BUTTON ス?イルを持たない
 *    のなら、この method 使われない。
 *(*)
 *    今の?況では特に何もする必要はないので、S_OK を?返す。
 */
STDAPI
CLangBarItemImeButton::OnClick (
    TfLBIClick click,
    POINT pt,
    const RECT* prcArea)
{
    return S_OK;
}

/*    ITfLangBarItemButton::InitMenu
 *
 *    この method は TF_LBI_STYLE_BTN_MENU ス?イルを持った言語バ?の??ン
 *    を言語バ?が??ンに?して?示する menu item を追加して有?にするため
 *    に呼び出される。
 */
STDAPI
CLangBarItemImeButton::InitMenu (
    ITfMenu* pMenu)
{
    register int     i, id;
    register DWORD   dwFlag;
    register LPCWSTR wstrDesc;
    register ULONG   nstrDesc;
    ITfMenu *pUserKeyboardMenu;
    int     idUserKeyboardMenu=0;

    ITfMenu *pUserComposeMenu;
    int     idUserComposeMenu=0;
    HKEY    hKey;

    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::InitMenu (ITfMenu:%p)\n"), pMenu));

    if (pMenu == NULL)
        return E_INVALIDARG;

    GetRegKeyHandle(TEXT("\\Keyboard"), &hKey);

    id = 0;
    for (i = 0; i < 2; i ++) {
	LPTSTR lpDesc;
	TCHAR  szDesc[128];
	lpDesc=(LPTSTR)&szDesc;

	LoadString(hInst,c_rgMenuItems [i].chDesc,lpDesc,128);
        wstrDesc = (LPCWSTR)lpDesc;
        //wstrDesc = c_rgMenuItems [i].pchDesc;
        if (c_rgMenuItems [i].chDesc != NULL) {
            nstrDesc = wcslen (wstrDesc);
            dwFlag = 0;
            if (c_rgMenuItems [i].pfnGetFlag != NULL)
                dwFlag = (c_rgMenuItems [i].pfnGetFlag)(i);
        } else {
            nstrDesc = 0;
            dwFlag = TF_LBMENUF_SEPARATOR;
        }
        pMenu->AddMenuItem (id++,
                        dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
    }

    // Add default Keyboard lists
    pUserKeyboardMenu=NULL;
    pUserComposeMenu=NULL;
    for (i = 0; i < ARRAYSIZE (c_rgKeyboardItems); i++)
    {
	LPTSTR lpDesc;
	TCHAR  szDesc[128];
	DWORD  uFlag = TRUE;
	lpDesc=(LPTSTR)&szDesc;

	//if (c_rgKeyboardItems [i].chDesc == NULL)
	//	lpDesc=NULL;
	//else
	LoadString(hInst,c_rgKeyboardItems [i].chDesc,lpDesc,128);
        wstrDesc = (LPCWSTR)lpDesc;
        //wstrDesc = c_rgKeyboardItems [i].pchDesc;
        nstrDesc = wcslen (wstrDesc);
        dwFlag = c_rgKeyboardItems [i].dwFlag;

        if (dwFlag == NULL)
        {
	    // make the Keyboard WID from the IDS.
	    UINT ii = c_rgKeyboardItems [i].chDesc - 2100 + 1;

            dwFlag = _MenuItem_GetKeyboardFlag(ii);
	    if (dwFlag)
		    uFlag = FALSE;

	    if (hKey && ii > 5) {
		LPTSTR lpDesc;
		TCHAR  szDesc[128];

		lpDesc=(LPTSTR)&szDesc;
		LoadString(hInst,c_rgKeyboardItems [i].chDesc + 1000, lpDesc, 128);
		// IDS_MENU_XXX => IDS_KEY_XXX

		// Query if the user-defined keyboard exists or not. 
		if (RegQueryValueEx(hKey, lpDesc, 0, NULL, NULL, NULL) != ERROR_SUCCESS)
		    continue;
	    }

            pMenu->AddMenuItem (ii,
                   dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
	    id++;
        } else if (idUserKeyboardMenu == 0) {
	    UINT ii = c_rgKeyboardItems [i].chDesc - 2100 + 1;
	    if (!uFlag)
		    dwFlag |= TF_LBMENUF_RADIOCHECKED; // NOT WORK

            idUserKeyboardMenu = ii;
            pMenu->AddMenuItem (ii,
                   dwFlag, NULL, NULL, wstrDesc, nstrDesc, &pUserKeyboardMenu);
	    id++;
        } else {
	    id=20;
            idUserComposeMenu = id;
            pMenu->AddMenuItem (id++,
                   dwFlag, NULL, NULL, wstrDesc, nstrDesc, &pUserComposeMenu);
        }
    }
    // add seperator
    if (i)
        pMenu->AddMenuItem (id++,
                        TF_LBMENUF_SEPARATOR , NULL, NULL, NULL, 0, NULL);

    // add extra config menus
    id=30;
    for (i = 2; i < ARRAYSIZE (c_rgMenuItems); i++)
    {
	LPTSTR lpDesc;
	TCHAR  szDesc[128];
	lpDesc=(LPTSTR)&szDesc;

	if (c_rgMenuItems [i].chDesc == NULL)
		lpDesc=NULL;
	else
		LoadString(hInst,c_rgMenuItems [i].chDesc,lpDesc,128);
        wstrDesc = (LPCWSTR)lpDesc;
        //wstrDesc = c_rgMenuItems [i].pchDesc;
        if (wstrDesc != NULL) {
            nstrDesc = wcslen (wstrDesc);
            dwFlag = (c_rgMenuItems [i].pfnGetFlag != NULL)? (c_rgMenuItems [i].pfnGetFlag)(i) : 0;
        } else {
            nstrDesc = 0;
            dwFlag = TF_LBMENUF_SEPARATOR;
        }
        pMenu->AddMenuItem (id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
    }
    
    // add User defined Keyboads (SubMenu)
    if (idUserKeyboardMenu) {
        id = idUserKeyboardMenu;

        if (hKey == NULL)
            return S_OK;

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
            dwFlag = _MenuItem_GetKeyboardFlag(id);
            pUserKeyboardMenu->AddMenuItem (id++,
            //pUserKeyboardMenu->AddMenuItem (idUserKeyboardMenu,
                            dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
        }
        pUserKeyboardMenu->Release();
    }
    if (hKey)
	RegCloseKey(hKey);

    // add User defined Composes (SubMenu)
    if (idUserComposeMenu) {
        HKEY hKey;

        id = idUserComposeMenu;

        if (!GetRegKeyHandle(TEXT("\\Compose"), &hKey))
            return S_OK;
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
            dwFlag = _MenuItem_GetComposeFlag(id);
            pUserComposeMenu->AddMenuItem (id++,
            //pUserKeyboardMenu->AddMenuItem (idUserKeyboardMenu,
                            dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
        }
	RegCloseKey(hKey);

        pUserComposeMenu->Release();
    }
    return S_OK;
}

STDAPI
CLangBarItemImeButton::OnMenuSelect (
    UINT wID)
{
//    if (wID >= ARRAYSIZE (c_rgMenuItems))
//        return    E_FAIL;
    MyDebugPrint((TEXT("MenuSelect:%x\n"), wID));

    if (wID == 0 || wID >= 30 ){
        if (wID >= 30) wID-= 28;
        if (c_rgMenuItems [wID].pfnHandler != NULL) {
            c_rgMenuItems [wID].pfnHandler (wID);
            //UpdateLanguageBar ();
        }
    } else if (wID < 20) {
        _Menu_SelectKeyboard (wID);
    } else {
        _Menu_SelectCompose (wID);
    }

    UpdateLanguageBar();
    return    S_OK;
}

STDAPI
CLangBarItemImeButton::GetIcon (
    HICON*                    phIcon)
{
    LPCTSTR str = NULL;
    HIMC hIMC;
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::GetIcon(%p)\n"), phIcon));

    if (phIcon == NULL)
        return    E_INVALIDARG;


    hIMC= _GetCurrentHIMC();
    str = TEXT("INDIC_KEY");
    if (hIMC != NULL) {
        if (dwImeFlag & AUTOMATA_3SET)
	    str = TEXT("INDIC_3SET");
	else if (dwImeFlag & AUTOMATA_2SET)
	    str = TEXT("INDIC_2SET");
    }

    *phIcon    = (HICON)LoadImage (hInst, str, IMAGE_ICON, 16, 16, 0);
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI
CLangBarItemImeButton::GetText (
    BSTR*            pbstrText)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::GetText ()\n")));

    if (pbstrText == NULL)
        return    E_INVALIDARG;

    *pbstrText    = SysAllocString (_tfLangBarItemInfo.szDescription);
    return    (*pbstrText == NULL)? E_OUTOFMEMORY : S_OK;
}

STDAPI
CLangBarItemImeButton::AdviseSink (
    REFIID            riid,
    IUnknown*        punk,
    DWORD*            pdwCookie)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::AdviseSink (this:%p)\n"), this));

    if (!IsEqualIID (IID_ITfLangBarItemSink, riid)) {
        DEBUGPRINTFEX (100, (TEXT ("CONNECT_E_CANNOTCONNECT\n")));
        return    CONNECT_E_CANNOTCONNECT;
    }

    if (_pLangBarItemSink != NULL) {
        DEBUGPRINTFEX (100, (TEXT ("CONNECT_E_ADVISELIMIT\n")));
        return    CONNECT_E_ADVISELIMIT;
    }

    if (punk->QueryInterface (IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK) {
        DEBUGPRINTFEX (100, (TEXT ("E_NOINTERFACE\n")));
        _pLangBarItemSink    = NULL;
        return    E_NOINTERFACE;
    }

    *pdwCookie    = SAENARU_LANGBARITEMSINK_COOKIE;
    return    S_OK;
}

STDAPI
CLangBarItemImeButton::UnadviseSink (
    DWORD            dwCookie)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::UnadviseSink ()\n")));

    if (dwCookie != SAENARU_LANGBARITEMSINK_COOKIE)
        return    CONNECT_E_NOCONNECTION;

    if (_pLangBarItemSink == NULL)
        return    CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release ();
    _pLangBarItemSink    = NULL;

    return    S_OK;
}

/*========================================================================*
 *    public function interface
 */
BOOL    PASCAL
CreateItemButtonIME (
    register ITfLangBarItem**    ppLangBarItem)
{
    if (ppLangBarItem == NULL)
        return    FALSE;
    *ppLangBarItem        = new CLangBarItemImeButton ();
    return    (*ppLangBarItem != NULL);
}

/*========================================================================*
 *    private functions
 */
void
_Menu_Help (UINT wID)
{
    /*    う?ん、HELP は作ってない…。*/
    return;
}

void
_Menu_Property (UINT wID)
{
    register HKL    hKL        = GetMyHKL ();
    register HWND    hWnd    = GetFocus ();

    if (hKL == NULL)
        return;
    (void) ImeConfigure (hKL, hWnd, IME_CONFIG_GENERAL, NULL);
    return;
}

void
_Menu_Reconversion (UINT wID)
{
    register HIMC                hIMC;
    register DWORD                dwSize;
    register LPRECONVERTSTRING    lpRS;

    hIMC    = _GetCurrentHIMC ();
    if (hIMC == NULL)
        return;

    dwSize    = (DWORD) MyImmRequestMessage (hIMC, IMR_RECONVERTSTRING, 0);
    if (! dwSize) 
        return;

    lpRS            = (LPRECONVERTSTRING) GlobalAlloc (GPTR, dwSize);
    lpRS->dwSize    = dwSize;
    if (dwSize = (DWORD) MyImmRequestMessage (hIMC, IMR_RECONVERTSTRING, (LPARAM) lpRS)) {
        register LPINPUTCONTEXT            lpIMC;
            
        lpIMC    = ImmLockIMC (hIMC);
        if (lpIMC != NULL) {
            register LPCOMPOSITIONSTRING    lpCompStr;

            if (ImmGetIMCCSize (lpIMC->hCompStr) < sizeof (MYCOMPSTR))
                goto    pass_1;

            /*    ?換モ?ドを?制的に?名入力モ?ドに設定する。*/
            lpCompStr    = (LPCOMPOSITIONSTRING)ImmLockIMCC (lpIMC->hCompStr);
            if (lpCompStr != NULL) {
                //SKKSetReconvertStr (hIMC, lpIMC, lpCompStr, lpRS, TRUE); // ?
                ImmUnlockIMCC (lpIMC->hCompStr);
            }
        pass_1:
            ImmUnlockIMC (hIMC);
        }
        MyImmRequestMessage(hIMC, IMR_CONFIRMRECONVERTSTRING, (LPARAM)lpRS);
    } else {
        GlobalFree((HANDLE)lpRS);
    }
    return;
}

void
_Menu_SelectKeyboard (UINT wID)
{
    wID--;
    //dwLayoutFlag &= 0xffff0000;
    //dwLayoutFlag |= wID;
    dwLayoutFlag = wID;
    set_keyboard(wID);
    return;
}

void
_Menu_SelectCompose (UINT wID)
{
    wID-=16;
    //dwLayoutFlag &= 0xffff0000;
    //dwLayoutFlag |= wID;
    dwComposeFlag = wID;
    set_compose(wID);
    return;
}

DWORD
_MenuItem_GetKeyboardFlag (UINT wID)
{
    wID--;
    DEBUGPRINTFEX (100, (TEXT ("GetKeyboardFlag:%d)\n"), wID));
    //if ( flag & dwLayoutFlag)
    if ( wID == dwLayoutFlag)
        return 1;
    return    0;
}

DWORD
_MenuItem_GetComposeFlag (UINT wID)
{
    wID-=16;
    DEBUGPRINTFEX (100, (TEXT ("GetComposeFlag:%d)\n"), wID));
    //if ( flag & dwLayoutFlag)
    if ( wID == dwComposeFlag)
        return 1;
    return    0;
}

void
_Menu_ToggleShowKeyboard (UINT wID)
{
    HIMC    hIMC;
    DWORD    dwConversion, dwSentence;
    LPINPUTCONTEXT lpIMC;

    hIMC    = _GetCurrentHIMC ();
    if (hIMC == NULL)
        return;

    lpIMC = ImmLockIMC (hIMC);
    if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentence))
    {
        if (dwConversion & IME_CMODE_SOFTKBD)
            dwConversion &= ~IME_CMODE_SOFTKBD;
        else
            dwConversion |= IME_CMODE_SOFTKBD;
    }
    ImmSetConversionStatus (hIMC, dwConversion, 0);
    ImmUnlockIMC (hIMC);
#if 0
    register BOOL    fShowKeyboardIcon    = TRUE;
    DWORD            dwValue;

    if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), &dwValue)) 
        fShowKeyboardIcon    = (BOOL) dwValue;
    SetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), !fShowKeyboardIcon);
#endif
    return;
}

void
_Menu_ToggleDvorak (UINT wID)
{
    if (dwOptionFlag & DVORAK_SUPPORT)
        dwOptionFlag &= ~DVORAK_SUPPORT;
    else
        dwOptionFlag |= DVORAK_SUPPORT;
    return;
}

void
_Menu_ToggleHanEng (UINT wID)
{
    if (dwOptionFlag & USE_SHIFT_SPACE)
        dwOptionFlag &= ~USE_SHIFT_SPACE;
    else
        dwOptionFlag |= USE_SHIFT_SPACE;
    return;
}

void
_Menu_ToggleEscEng (UINT wID)
{
    if (dwOptionFlag & ESCENG_SUPPORT)
        dwOptionFlag &= ~ESCENG_SUPPORT;
    else
        dwOptionFlag |= ESCENG_SUPPORT;
    return;
}

void
_Menu_ToggleOnTheSpot (UINT wID)
{
    if (dwImeFlag & SAENARU_ONTHESPOT)
        dwImeFlag &= ~SAENARU_ONTHESPOT;
    else
        dwImeFlag |= SAENARU_ONTHESPOT;
    return;
}

DWORD
_MenuItem_GetNormalFlag (UINT wID)
{
    return    0;
}

DWORD
_MenuItem_GetToggleDvorakFlag (UINT wID)
{
    HKL hcur;
    hcur= GetKeyboardLayout(0);
    if (hcur != LongToHandle(0xE0120412))
        return TF_LBI_STATUS_DISABLED;

    return (dwOptionFlag & DVORAK_SUPPORT) ? 1 : 0;
}

DWORD
_MenuItem_GetToggleHanEngFlag (UINT wID)
{
    return (dwOptionFlag & USE_SHIFT_SPACE) ? 1 : 0;
}

DWORD
_MenuItem_GetToggleEscEngFlag (UINT wID)
{
    return (dwOptionFlag & ESCENG_SUPPORT) ? 1 : 0;
}

DWORD
_MenuItem_GetToggleOnTheSpotFlag (UINT wID)
{
    return (dwImeFlag & SAENARU_ONTHESPOT) ? 0 : 1;
}

DWORD
_MenuItem_GetToggleKeyboardFlag (UINT wID)
{
    HIMC    hIMC;
    DWORD    dwConversion, dwSentence;

    hIMC    = _GetCurrentHIMC ();
    if (hIMC == NULL)
        return TF_LBI_STATUS_DISABLED;

    if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentence))
        return (dwConversion & IME_CMODE_SOFTKBD) ? TF_LBMENUF_CHECKED: 0;
    else
        //return TF_LBI_STATUS_DISABLED;
        return 0;
#if 0    
    register BOOL    fShowKeyboardIcon    = TRUE;
    DWORD            dwValue;

    if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), &dwValue)) 
        fShowKeyboardIcon    = (BOOL) dwValue;
    return    (fShowKeyboardIcon)? TF_LBMENUF_CHECKED : 0;
#endif
}

#endif

/*
 * ex: ts=8 sts=8 sw=8 noet
 */
