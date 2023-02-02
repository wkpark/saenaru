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
 */

#include "globals.h"
#include "saenarutip.h"
#include "resource.h"
#include "btnime.h"

extern "C"
{
#include "hangul.h"
    BOOL GetRegKeyHandle(LPCTSTR, HKEY *);
}

#define SAENARU_LANGBARITEMSINK_COOKIE 0x0fab0fac

#define UNUSED(x) (void)(x)

const struct
{
    UINT chDesc;
    DWORD (*pfnGetFlag)(CLangBarItemImeButton *, UINT);
    void (*pfnHandler)(CLangBarItemImeButton *, UINT);
} c_rgMenuItems[] = {
    {IDS_MENU_ENV, CLangBarItemImeButton::_MenuItem_GetNormalFlag, CLangBarItemImeButton::_Menu_Property},
    {NULL, NULL, NULL},
    {IDS_MENU_DVORAK, CLangBarItemImeButton::_MenuItem_GetToggleDvorakFlag, CLangBarItemImeButton::_Menu_ToggleDvorak},
    {IDS_MENU_ESC_ASCII,
     CLangBarItemImeButton::_MenuItem_GetToggleEscEngFlag,
     CLangBarItemImeButton::_Menu_ToggleEscEng},
    {IDS_MENU_HANGUL_TOGGLE,
     CLangBarItemImeButton::_MenuItem_GetToggleHanEngFlag,
     CLangBarItemImeButton::_Menu_ToggleHanEng},
    {IDS_MENU_EDIT_WORD_UNIT,
     CLangBarItemImeButton::_MenuItem_GetToggleOnTheSpotFlag,
     CLangBarItemImeButton::_Menu_ToggleOnTheSpot},
    {NULL, NULL, NULL},
    /*
    { IDS_MENU_SHOW_KEYBOARD,
        CLangBarItemImeButton::_MenuItem_GetToggleKeyboardFlag,
        CLangBarItemImeButton::_Menu_ToggleShowKeyboard
    }, */
    {NULL, NULL, NULL},
    {IDS_MENU_CANCEL, NULL, NULL}};

const struct
{
    UINT chDesc;
    DWORD dwFlag;
} c_rgKeyboardItems[] = {
    {IDS_MENU_OLD2BUL, NULL},
    {IDS_MENU_3FIN, NULL},
    {IDS_MENU_3SOON, NULL},
    {IDS_MENU_390, NULL},
    {IDS_MENU_NK_2BUL, NULL},
    {IDS_MENU_YET2BUL, NULL},
    {IDS_MENU_YET3BUL, NULL},
    {IDS_MENU_NEW2BUL, NULL},
    {IDS_MENU_NEW3BUL, NULL},
    {IDS_MENU_AHNMATAE, NULL},
    {IDS_MENU_USER, TF_LBMENUF_SUBMENU},
    {IDS_MENU_USER_COMP, TF_LBMENUF_SUBMENU},
};

enum
{
    MENU_ITEM_INDEX_CANCEL = -1,
    // MENU_ITEM_INDEX_HELP,
    MENU_ITEM_INDEX_PROPERTY,
    // MENU_ITEM_INDEX_RECONVRESION,
    MENU_ITEM_INDEX_OLD2BUL = 2,
    MENU_ITEM_INDEX_OLD3BUL,
    MENU_ITEM_INDEX_3FIN,
};

CLangBarItemImeButton::CLangBarItemImeButton(CSaenaruTextService *pSaenaru)
{
    DllAddRef();
    LPTSTR lpDesc;

    _tfLangBarItemInfo.clsidService = c_clsidSaenaruTextService;
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButtonIME;
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_MENU | TF_LBI_STYLE_SHOWNINTRAY;
    lpDesc = (LPTSTR)&_tfLangBarItemInfo.szDescription;
    _tfLangBarItemInfo.ulSort = 1;
    LoadString(g_hInst, IDS_KEYBOARD_OPTION_DESC, lpDesc, ARRAYSIZE(_tfLangBarItemInfo.szDescription));

    _pSaenaru = pSaenaru;
    _pSaenaru->AddRef();
    _pLangBarItemSink = NULL;

    _cRef = 1;
    return;
}

CLangBarItemImeButton::~CLangBarItemImeButton()
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::~CLangBarItemImeButton (this:%p)\n"), this));
    DllRelease();
    _pSaenaru->Release();
    return;
}

STDAPI
CLangBarItemImeButton::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfLangBarItem)
        || IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource *)this;
    }
    if (*ppvObj != NULL)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG)
CLangBarItemImeButton::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG)
CLangBarItemImeButton::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }
    return cr;
}

STDAPI
CLangBarItemImeButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::GetInfo(this:%p)\n"), this));

    if (pInfo == NULL)
        return E_INVALIDARG;

    *pInfo = _tfLangBarItemInfo;
    return S_OK;
}

STDAPI
CLangBarItemImeButton::Show(BOOL fShow)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::Show(BOOL:%d)\n"), fShow));
    return E_NOTIMPL;
}

STDAPI
CLangBarItemImeButton::GetStatus(DWORD *pdwStatus)
{
    if (pdwStatus == NULL)
        return E_INVALIDARG;

    *pdwStatus = 0;
    return S_OK;
}

STDAPI
CLangBarItemImeButton::GetTooltipString(BSTR *pbstrToolTip)
{
    if (pbstrToolTip == NULL)
        return E_INVALIDARG;

    *pbstrToolTip = SysAllocString(_tfLangBarItemInfo.szDescription);
    return (*pbstrToolTip == NULL) ? E_OUTOFMEMORY : S_OK;
}

STDAPI
CLangBarItemImeButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
    return S_OK;
}

STDAPI
CLangBarItemImeButton::InitMenu(ITfMenu *pMenu)
{
    int i, id;
    DWORD dwFlag;
    LPCWSTR wstrDesc;
    ULONG nstrDesc;
    ITfMenu *pUserKeyboardMenu;
    int idUserKeyboardMenu = 0;

    ITfMenu *pUserComposeMenu;
    int idUserComposeMenu = 0;
    HKEY hKey;

    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::InitMenu(ITfMenu:%p)\n"), pMenu));

    if (pMenu == NULL)
        return E_INVALIDARG;

    GetRegKeyHandle(TEXT("\\Keyboard"), &hKey);

    id = 0;
    for (i = 0; i < 2; i++)
    {
        LPTSTR lpDesc;
        TCHAR szDesc[128];
        lpDesc = (LPTSTR)&szDesc;

        LoadString(g_hInst, c_rgMenuItems[i].chDesc, lpDesc, 128);
        wstrDesc = (LPCWSTR)lpDesc;
        if (c_rgMenuItems[i].chDesc != NULL)
        {
            nstrDesc = (ULONG)wcslen(wstrDesc);
            dwFlag = 0;
            if (c_rgMenuItems[i].pfnGetFlag != NULL)
                dwFlag = (c_rgMenuItems[i].pfnGetFlag)(this, i);
        }
        else
        {
            nstrDesc = 0;
            dwFlag = TF_LBMENUF_SEPARATOR;
        }
        pMenu->AddMenuItem(id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
    }

    // Add default Keyboard lists
    pUserKeyboardMenu = NULL;
    pUserComposeMenu = NULL;
    for (i = 0; i < ARRAYSIZE(c_rgKeyboardItems); i++)
    {
        LPTSTR lpDesc;
        TCHAR szDesc[128];
        DWORD uFlag = TRUE;
        lpDesc = (LPTSTR)&szDesc;

        LoadString(g_hInst, c_rgKeyboardItems[i].chDesc, lpDesc, 128);
        wstrDesc = (LPCWSTR)lpDesc;
        nstrDesc = (ULONG)wcslen(wstrDesc);
        dwFlag = c_rgKeyboardItems[i].dwFlag;

        if (dwFlag == NULL)
        {
            // make the Keyboard WID from the IDS. (IDS_MENU_OLD2BUL == 2101)
            UINT ii = c_rgKeyboardItems[i].chDesc - 2100 + 1;

            dwFlag = _MenuItem_GetKeyboardFlag(this, ii);
            if (dwFlag)
                uFlag = FALSE;

            if (hKey && ii > 5)
            {
                LPTSTR lpDesc;
                TCHAR szDesc[128];

                lpDesc = (LPTSTR)&szDesc;
                LoadString(g_hInst, c_rgKeyboardItems[i].chDesc + 1000, lpDesc, 128);
                // IDS_MENU_XXX => IDS_KEY_XXX

                // Query if the user-defined keyboard exists or not.
                if (RegQueryValueEx(hKey, lpDesc, 0, NULL, NULL, NULL) != ERROR_SUCCESS)
                    dwFlag = TF_LBI_STATUS_DISABLED;
                //   continue;
            }

            pMenu->AddMenuItem(ii, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
            id++;
        }
        else if (idUserKeyboardMenu == 0)
        {
            UINT ii = c_rgKeyboardItems[i].chDesc - 2100 + 1;
            if (!uFlag)
                dwFlag |= TF_LBMENUF_RADIOCHECKED; // Not work

            idUserKeyboardMenu = ii;
            pMenu->AddMenuItem(ii, dwFlag, NULL, NULL, wstrDesc, nstrDesc, &pUserKeyboardMenu);
            id++;
        }
        else
        {
            id = 20;
            idUserComposeMenu = id;
            pMenu->AddMenuItem(id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, &pUserComposeMenu);
        }
    }
    // add seperator
    if (i)
        pMenu->AddMenuItem(id++, TF_LBMENUF_SEPARATOR, NULL, NULL, NULL, 0, NULL);

    // add extra config menus
    id = 30;
    for (i = 2; i < ARRAYSIZE(c_rgMenuItems); i++)
    {
        LPTSTR lpDesc;
        TCHAR szDesc[128];
        lpDesc = (LPTSTR)&szDesc;

        if (c_rgMenuItems[i].chDesc == NULL)
            lpDesc = NULL;
        else
            LoadString(g_hInst, c_rgMenuItems[i].chDesc, lpDesc, 128);
        wstrDesc = (LPCWSTR)lpDesc;
        if (wstrDesc != NULL)
        {
            nstrDesc = (ULONG)wcslen(wstrDesc);
            dwFlag = (c_rgMenuItems[i].pfnGetFlag != NULL) ? (c_rgMenuItems[i].pfnGetFlag)(this, i) : 0;
        }
        else
        {
            nstrDesc = 0;
            dwFlag = TF_LBMENUF_SEPARATOR;
        }
        pMenu->AddMenuItem(id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
    }

    // add User defined Keyboards (SubMenu)
    if (idUserKeyboardMenu)
    {
        id = idUserKeyboardMenu;

        if (hKey == NULL)
            return S_OK;

        for (i = 0; i < 10; i++)
        {
            WCHAR achValue[256];
            DWORD cchValue = 256;
            DWORD retCode;

            achValue[0] = '\0';
            retCode = RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL);

            if (retCode != ERROR_SUCCESS)
            {
                break;
                MyDebugPrint((TEXT("(%d) %s\n"), i + 1, achValue));
            }
            wstrDesc = achValue;
            nstrDesc = (ULONG)wcslen(wstrDesc);
            dwFlag = _MenuItem_GetKeyboardFlag(this, id);
            pUserKeyboardMenu->AddMenuItem(id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
        }
        pUserKeyboardMenu->Release();
    }
    if (hKey)
        RegCloseKey(hKey);

    // add User defined Composes (SubMenu)
    if (idUserComposeMenu)
    {
        HKEY hKey;

        id = idUserComposeMenu;

        if (!GetRegKeyHandle(TEXT("\\Compose"), &hKey))
            return S_OK;

        for (i = 0; i < 10; i++)
        {
            WCHAR achValue[256];
            DWORD cchValue = 256;
            DWORD retCode;

            achValue[0] = '\0';
            retCode = RegEnumValue(hKey, i, achValue, &cchValue, NULL, NULL, NULL, NULL);

            if (retCode != ERROR_SUCCESS)
            {
                break;
                MyDebugPrint((TEXT("(%d) %s\n"), i + 1, achValue));
            }
            wstrDesc = achValue;
            nstrDesc = (ULONG)wcslen(wstrDesc);
            dwFlag = _MenuItem_GetComposeFlag(this, id);
            pUserComposeMenu->AddMenuItem(id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
        }
        RegCloseKey(hKey);

        pUserComposeMenu->Release();
    }
    return S_OK;
}

STDAPI
CLangBarItemImeButton::OnMenuSelect(UINT wID)
{
    MyDebugPrint((TEXT("MenuSelect(%d)\n"), wID));

    if (wID == 0 || wID >= 30)
    {
        if (wID >= 30)
            wID -= 28;
        if (c_rgMenuItems[wID].pfnHandler != NULL)
        {
            c_rgMenuItems[wID].pfnHandler(this, wID);
        }
    }
    else if (wID < 20)
    {
        _Menu_SelectKeyboard(this, wID);
    }
    else
    {
        _Menu_SelectCompose(this, wID);
    }

    // update all LangBarItemButtons
    _pSaenaru->_UpdateLanguageBar(TF_LBI_STATUS | TF_LBI_ICON);
    return S_OK;
}

STDAPI
CLangBarItemImeButton::GetIcon(HICON *phIcon)
{
    LPCTSTR str = NULL;
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::GetIcon(%p)\n"), phIcon));

    if (phIcon == NULL)
        return E_INVALIDARG;

    str = TEXT("INDIC_KEY");
    if (dwImeFlag & AUTOMATA_3SET)
        str = TEXT("INDIC_3SET");
    else if (dwImeFlag & AUTOMATA_2SET)
        str = TEXT("INDIC_2SET");

    *phIcon = (HICON)LoadImage(g_hInst, str, IMAGE_ICON, 16, 16, 0);
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI
CLangBarItemImeButton::GetText(BSTR *pbstrText)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::GetText()\n")));

    if (pbstrText == NULL)
        return E_INVALIDARG;

    *pbstrText = SysAllocString(_tfLangBarItemInfo.szDescription);
    return (*pbstrText == NULL) ? E_OUTOFMEMORY : S_OK;
}

STDAPI
CLangBarItemImeButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::AdviseSink(this:%p)\n"), this));

    if (!IsEqualIID(IID_ITfLangBarItemSink, riid))
    {
        DEBUGPRINTFEX(100, (TEXT("CONNECT_E_CANNOTCONNECT\n")));
        return CONNECT_E_CANNOTCONNECT;
    }

    if (_pLangBarItemSink != NULL)
    {
        DEBUGPRINTFEX(100, (TEXT("CONNECT_E_ADVISELIMIT\n")));
        return CONNECT_E_ADVISELIMIT;
    }

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        DEBUGPRINTFEX(100, (TEXT("E_NOINTERFACE\n")));
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    *pdwCookie = SAENARU_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

STDAPI
CLangBarItemImeButton::UnadviseSink(DWORD dwCookie)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemImeButton::UnadviseSink()\n")));

    if (dwCookie != SAENARU_LANGBARITEMSINK_COOKIE)
        return CONNECT_E_NOCONNECTION;

    if (_pLangBarItemSink == NULL)
        return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = NULL;

    return S_OK;
}

//
// _OnUpdate
//
void CLangBarItemImeButton::_OnUpdate(DWORD dwFlags)
{
    if (_pLangBarItemSink)
        _pLangBarItemSink->OnUpdate(dwFlags);
}

//
// private functions
//
void CLangBarItemImeButton::_Menu_Help(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    return;
}

void CLangBarItemImeButton::_Menu_Property(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);

    // HKL hKL = GetMyHKL();
    HWND hWnd = GetFocus();

    // if (hKL == NULL)
    //     return;
    //(void) ImeConfigure(hKL, hWnd, IME_CONFIG_GENERAL, NULL);
    return;
}

#if 0
void
CLangBarItemImeButton::_Menu_Reconversion(UINT wID)
{
    UNUSED(_this);
    HIMC hIMC;
    DWORD dwSize;
    LPRECONVERTSTRING lpRS;

    dwSize = (DWORD) MyImmRequestMessage (hIMC, IMR_RECONVERTSTRING, 0);
    if (! dwSize)
        return;

    lpRS = (LPRECONVERTSTRING) GlobalAlloc (GPTR, dwSize);
    lpRS->dwSize = dwSize;
    if (dwSize = (DWORD) MyImmRequestMessage (hIMC, IMR_RECONVERTSTRING, (LPARAM) lpRS)) {
        LPINPUTCONTEXT lpIMC;

        lpIMC = ImmLockIMC(hIMC);
        if (lpIMC != NULL) {
            LPCOMPOSITIONSTRING lpCompStr;

            if (ImmGetIMCCSize (lpIMC->hCompStr) < sizeof (MYCOMPSTR))
                goto    pass_1;

            lpCompStr    = (LPCOMPOSITIONSTRING)ImmLockIMCC (lpIMC->hCompStr);
            if (lpCompStr != NULL) {
                //SKKSetReconvertStr (hIMC, lpIMC, lpCompStr, lpRS, TRUE); // ?
                ImmUnlockIMCC (lpIMC->hCompStr);
            }
        pass_1:
            ImmUnlockIMC (hIMC);
        }
        MyImmRequestMessage(hIMC, IMR_CONFIRMRECONVERTSTRING, (LPARAM)lpRS);
    }
    else
    {
        GlobalFree((HANDLE)lpRS);
    }
    return;
}
#endif

void CLangBarItemImeButton::_Menu_SelectKeyboard(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    wID--;
    dwLayoutFlag = wID;
    set_keyboard(wID);
    return;
}

void CLangBarItemImeButton::_Menu_SelectCompose(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    wID -= 16;
    dwComposeFlag = wID;
    set_compose(wID);
    return;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetKeyboardFlag(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    wID--;
    DEBUGPRINTFEX(100, (TEXT("GetKeyboardFlag(%d)\n"), wID));
    if (wID == dwLayoutFlag)
        return TF_LBMENUF_RADIOCHECKED;
    return 0;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetComposeFlag(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    wID -= 16;
    DEBUGPRINTFEX(100, (TEXT("GetComposeFlag:%d)\n"), wID));
    if (wID == dwComposeFlag)
        return 1;
    return 0;
}

#if 0
void
CLangBarItemImeButton::_Menu_ToggleShowKeyboard (UINT wID)
{
    DWORD dwConversion, dwSentence;
    LPINPUTCONTEXT lpIMC;

    lpIMC = ImmLockIMC (hIMC);
    if (ImmGetConversionStatus(hIMC, &dwConversion, &dwSentence))
    {
        if (dwConversion & IME_CMODE_SOFTKBD)
            dwConversion &= ~IME_CMODE_SOFTKBD;
        else
            dwConversion |= IME_CMODE_SOFTKBD;
    }
    ImmSetConversionStatus(hIMC, dwConversion, 0);
    ImmUnlockIMC (hIMC);
#if 0
    register BOOL    fShowKeyboardIcon    = TRUE;
    DWORD            dwValue;

    if (GetRegDwordValue(TEXT("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), &dwValue))
        fShowKeyboardIcon    = (BOOL) dwValue;
    SetRegDwordValue(TEXT("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), !fShowKeyboardIcon);
#endif
    return;
}
#endif

void CLangBarItemImeButton::_Menu_ToggleDvorak(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    if (dwOptionFlag & DVORAK_SUPPORT)
        dwOptionFlag &= ~DVORAK_SUPPORT;
    else
        dwOptionFlag |= DVORAK_SUPPORT;
    return;
}

void CLangBarItemImeButton::_Menu_ToggleHanEng(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    if (dwOptionFlag & USE_SHIFT_SPACE)
        dwOptionFlag &= ~USE_SHIFT_SPACE;
    else
        dwOptionFlag |= USE_SHIFT_SPACE;
    return;
}

void CLangBarItemImeButton::_Menu_ToggleEscEng(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    if (dwOptionFlag & ESCENG_SUPPORT)
        dwOptionFlag &= ~ESCENG_SUPPORT;
    else
        dwOptionFlag |= ESCENG_SUPPORT;
    return;
}

void CLangBarItemImeButton::_Menu_ToggleOnTheSpot(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    if (dwImeFlag & SAENARU_ONTHESPOT)
        dwImeFlag &= ~SAENARU_ONTHESPOT;
    else
        dwImeFlag |= SAENARU_ONTHESPOT;
    return;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetNormalFlag(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    return 0;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetToggleDvorakFlag(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    // HKL hcur;
    // hcur = GetKeyboardLayout(0);
    // if (hcur != LongToHandle(0xE0120412))
    return TF_LBI_STATUS_DISABLED;

    // return (dwOptionFlag & DVORAK_SUPPORT) ? 1 : 0;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetToggleHanEngFlag(CLangBarItemImeButton *_this, UINT wID)
{
    return (dwOptionFlag & USE_SHIFT_SPACE) ? 1 : 0;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetToggleEscEngFlag(CLangBarItemImeButton *_this, UINT wID)
{
    UNUSED(_this);
    return (dwOptionFlag & ESCENG_SUPPORT) ? 1 : 0;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetToggleOnTheSpotFlag(CLangBarItemImeButton *_this, UINT wID)
{
    return (dwImeFlag & SAENARU_ONTHESPOT) ? 0 : 1;
}

DWORD
CLangBarItemImeButton::_MenuItem_GetToggleKeyboardFlag(CLangBarItemImeButton *_this, UINT wID)
{
    DWORD dwConversion = _this->_pSaenaru->GetConversionStatus();
    return (dwConversion & IME_CMODE_SOFTKBD) ? TF_LBMENUF_CHECKED : 0;

#if 0
    register BOOL    fShowKeyboardIcon    = TRUE;
    DWORD            dwValue;

    if (GetRegDwordValue(TEXT("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), &dwValue))
        fShowKeyboardIcon    = (BOOL) dwValue;
    return    (fShowKeyboardIcon)? TF_LBMENUF_CHECKED : 0;
#endif
}

/*
 * ex: ts=4 sts=4 sw=4 et
 */
