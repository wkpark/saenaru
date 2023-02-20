//
// langbar.cpp
//
// Language bar ui code.
//

#include "globals.h"
#include "saenarutip.h"
#include "resource.h"

#include "btnshape.h"
#include "btnime.h"
#include "langbar.h"
#include "tchar.h"

extern "C" {
#include "hangul.h"
    BOOL WINAPI ImeConfigure(HKL hKL,HWND hWnd, DWORD dwMode, LPVOID lpData);
    BOOL IsWindowsVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion, DWORD dwBuildNumber);
}
#define SAENARU_LANGBARITEMSINK_COOKIE 0x0fab0fab

const struct
{
    //const WCHAR *pchDesc;
    UINT chDesc;
    void (*pfnHandler)(CSaenaruTextService *_this);
}
c_rgMenuItems[] =
{
    { IDS_MENU_HANGUL,	CSaenaruTextService::_Menu_ToHangul },
    { IDS_MENU_ASCII,	CSaenaruTextService::_Menu_ToAscII },
    { IDS_MENU_HANJA,	CSaenaruTextService::_Menu_ToHanja },
    { NULL,		NULL },
    { IDS_MENU_CANCEL,	NULL },
#if 1
    { NULL,		NULL },
    { IDS_START_END_COMPOSITION, CSaenaruTextService::_Menu_OnComposition },
/*   { IDS_GET_GLOBAL_COMPART, CSaenaruTextService::_Menu_OnSetGlobalCompartment }, */
/*    { IDS_SET_CASE_PROP, CSaenaruTextService::_Menu_OnSetCaseProperty }, */
    { IDS_SET_CUSTOM_PROP, CSaenaruTextService::_Menu_OnSetCustomProperty },
/*   { IDS_GET_CASE_PROP, CSaenaruTextService::_Menu_OnViewCaseProperty }, */
    { IDS_GET_CUSTOM_PROP, CSaenaruTextService::_Menu_OnViewCustomProperty },
#endif
};

enum {
    MENU_ITEM_INDEX_CANCEL = -1,
    MENU_ITEM_INDEX_HANGUL = 0,
    MENU_ITEM_INDEX_ASCII,
    MENU_ITEM_INDEX_HANJA,
};

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CLangBarItemButton::CLangBarItemButton(CSaenaruTextService *pSaenaru, REFGUID guid)
{
    DllAddRef();

    _tfLangBarItemInfo.clsidService = c_clsidSaenaruTextService;
    _tfLangBarItemInfo.guidItem = guid;
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_SHOWNINTRAY
        | (IsEqualGUID(guid, GUID_LBI_INPUTMODE) ? TF_LBI_STYLE_BTN_MENU : TF_LBI_STYLE_TEXTCOLORICON)
        | TF_LBI_STYLE_BTN_BUTTON
        ;
    _tfLangBarItemInfo.ulSort = 0;
    SafeStringCopy(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);

    _pSaenaru = pSaenaru;
    _pSaenaru->AddRef();
    _pLangBarItemSink = NULL;

    _status = 0;

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CLangBarItemButton::~CLangBarItemButton()
{
    DllRelease();
    _pSaenaru->Release();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem) ||
        IsEqualIID(riid, IID_ITfLangBarItemButton))
    {
        *ppvObj = (ITfLangBarItemButton *)this;
    }
    else if (IsEqualIID(riid, IID_ITfSource))
    {
        *ppvObj = (ITfSource *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}


//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CLangBarItemButton::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CLangBarItemButton::Release()
{
    LONG cr = --_cRef;

    assert(_cRef >= 0);

    if (_cRef == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// GetInfo
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetInfo(TF_LANGBARITEMINFO *pInfo)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::GetInfo(this:%p)\n"), this));

    if (pInfo == nullptr)
    {
        return E_INVALIDARG;
    }
    _tfLangBarItemInfo.dwStyle |= TF_LBI_STYLE_SHOWNINTRAY;
    *pInfo = _tfLangBarItemInfo;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetStatus
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetStatus(DWORD *pdwStatus)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::GetInfo(this:%p)\n"), this));

    if (pdwStatus == nullptr)
    {
        E_INVALIDARG;
    }

    *pdwStatus = _status;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// SetStatus
//
//----------------------------------------------------------------------------

void CLangBarItemButton::SetStatus(DWORD dwStatus, BOOL fSet)
{
    BOOL isChange = FALSE;

    if (fSet)
    {
        if (!(_status & dwStatus))
        {
            _status |= dwStatus;
            isChange = TRUE;
        }
    }
    else
    {
        if (_status & dwStatus)
        {
            _status &= ~dwStatus;
            isChange = TRUE;
        }
    }

    if (isChange && _pLangBarItemSink)
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
    }

    return;
}

//+---------------------------------------------------------------------------
//
// Show
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
    fShow;

    if (_pLangBarItemSink)
    {
        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS);
    }
    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// GetTooltipString
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetTooltipString(BSTR *pbstrToolTip)
{
    DWORD dwConversion = 0;
    LPTSTR lpDesc;
    TCHAR  szDesc[128];

    if (pbstrToolTip == NULL)
        return E_INVALIDARG;

    dwConversion = _pSaenaru->GetConversionStatus();
    lpDesc = (LPTSTR)szDesc;
    if (dwConversion & CMODE_HANGUL)
    {
        LoadString(g_hInst, IDS_TIP_HANGUL, lpDesc, 128);
    } else {
        LoadString(g_hInst, IDS_TIP_ASCII, lpDesc, 128);
    }

    *pbstrToolTip = SysAllocString(lpDesc);
    return (*pbstrToolTip == NULL) ? E_OUTOFMEMORY : S_OK;
}

//+---------------------------------------------------------------------------
//
// OnClick
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::OnClick(TfLBIClick click, POINT pt, const RECT *prcArea)
{
    DWORD dwConversion = 0;
    ITfRange *pRangeComposition;
    ITfContext *pCompositionContext;

    // HACK generate VK_ESCAPE to close the default IME popup menu.
    keybd_event(VK_ESCAPE, 0x0, 0, 0);

    switch (click)
    {
        case TF_LBI_CLK_LEFT:
            dwConversion = _pSaenaru->GetConversionStatus();
            dwConversion ^= CMODE_HANGUL;

            // commit hangul chars.
            if (_pSaenaru->_IsComposing()) {
                DEBUGPRINTFEX(100, (TEXT("\tIsComposing\n")));
                if (_pSaenaru->_GetComposition()->GetRange(&pRangeComposition) == S_OK)
                {
                    if (pRangeComposition->GetContext(&pCompositionContext) == S_OK)
                    {
                        WCHAR ch = _pSaenaru->_HangulCommit();
                        _pSaenaru->_EndCompositionInContext(pCompositionContext, ch);
                        pCompositionContext->Release();
                    }
                    pRangeComposition->Release();
                }
                DEBUGPRINTFEX(100, (TEXT("\tEndComposition\n")));
            }
            // update the conversion status
            _pSaenaru->SetConversionStatus(dwConversion);

            _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
            return S_OK;
        case TF_LBI_CLK_RIGHT:
            DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::OnClick()\n")));
            {
                UINT checked = 0;
                HMENU hMenu = CreatePopupMenu();
                if (hMenu)
                {
                    dwConversion = _pSaenaru->GetConversionStatus();

                    UINT selected = 0;
                    if (dwConversion & CMODE_HANGUL)
                    {
                        checked = IDM_HANGUL;
                        selected = 0;
                    }
                    else
                    {
                        checked = IDM_ENG;
                        selected = 1;
                    }

                    for (int i = 0 ; i < ARRAYSIZE(c_rgMenuItems); i++)
                    {
                        LPTSTR lpDesc;
                        TCHAR szDesc[128];
                        MENUITEMINFO mii;
                        UINT state = 0;

                        lpDesc = (LPTSTR)&szDesc;
                        if (c_rgMenuItems[i].chDesc != NULL)
                            LoadString(g_hInst, c_rgMenuItems[i].chDesc, lpDesc, 128);
                        else
                            lpDesc = NULL;

                        mii.cbSize = sizeof(MENUITEMINFO);

                        if (i == selected)
                            state = MFS_CHECKED;
                        else
                            state = 0;

                        mii.wID = i;
                        if (lpDesc == NULL)
                        {
                            mii.fMask = MIIM_ID | MIIM_FTYPE;
                            mii.fType = MFT_SEPARATOR;
                        }
                        else
                        {
                            mii.fMask = MIIM_ID | MIIM_STRING | MIIM_STATE | MIIM_FTYPE;
                            mii.fType = MFT_STRING | MFT_RADIOCHECK;
                            mii.dwTypeData = lpDesc;
                            mii.fState = state;
                            mii.cch = (UINT)wcslen(lpDesc);
                        }
                        InsertMenuItem(hMenu, i, TRUE, &mii);
                    }

                    CheckMenuRadioItem(hMenu, IDM_HANGUL, IDM_HANJA, checked, MF_BYCOMMAND);

                    {
                        TPMPARAMS tpm = {};
                        TPMPARAMS *ptpm = nullptr;
                        if (prcArea != nullptr)
                        {
                            tpm.cbSize = sizeof(tpm);
                            tpm.rcExclude = *prcArea;
                            ptpm = &tpm;
                        }
                        // open popup menu
                        UINT bRet = TrackPopupMenuEx(hMenu,
                                TPM_LEFTALIGN | TPM_TOPALIGN | TPM_NONOTIFY | TPM_RETURNCMD | TPM_LEFTBUTTON | TPM_VERTICAL,
                                pt.x, pt.y, GetFocus(), ptpm);
                        DEBUGPRINTFEX(100, (TEXT("\t - bRet = %d\n"), bRet));
                        this->OnMenuSelect(bRet);
                    }
                    DestroyMenu(hMenu);
                }
            }
            return S_OK;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// InitMenu
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::InitMenu(ITfMenu *pMenu)
{
    int i, nCMode = 0;
    DWORD dwFlag;
    register LPCWSTR wstrDesc;
    register ULONG nstrDesc;

    DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::InitMenu()\n")));
    DWORD dwConversion = _pSaenaru->GetConversionStatus();
    if (dwConversion & CMODE_HANGUL)
    {
        nCMode = MENU_ITEM_INDEX_HANGUL;
    }
    else
    {
        nCMode = MENU_ITEM_INDEX_ASCII;
    }

    for (i=0; i<ARRAYSIZE(c_rgMenuItems); i++)
    {
        LPTSTR lpDesc;
        TCHAR szDesc[128];

        lpDesc = (LPTSTR)&szDesc;
        if (c_rgMenuItems[i].chDesc != NULL)
            LoadString(g_hInst, c_rgMenuItems[i].chDesc, lpDesc, 128);
        else
            lpDesc = NULL;
        wstrDesc = (LPCWSTR)lpDesc;

        if (wstrDesc != NULL) {
            nstrDesc = (ULONG) wcslen(wstrDesc);
            dwFlag = (i == nCMode) ? TF_LBMENUF_RADIOCHECKED : 0;
        } else {
            nstrDesc = 0;
            dwFlag = TF_LBMENUF_SEPARATOR;
        }
        pMenu->AddMenuItem(i, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnMenuSelect
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::OnMenuSelect(UINT wID)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::OnMenuSelect(wID = %d)\n"), wID));
    if (wID >= IDM_CONFIG)
    {
        DWORD dwConversion;
        DEBUGPRINTFEX(100, (TEXT("\twID = %d\n"), wID));

        switch(wID)
        {
            case IDM_HANGUL:
                dwConversion = _pSaenaru->GetConversionStatus();
                dwConversion = CMODE_HANGUL;
                _pSaenaru->SetConversionStatus(dwConversion);
                break;
            case IDM_ENG:
                dwConversion = _pSaenaru->GetConversionStatus();
                dwConversion = CMODE_ALPHANUMERIC;
                _pSaenaru->SetConversionStatus(dwConversion);
                break;
            case IDM_DVORAK:
                DEBUGPRINTFEX(100, (TEXT("\tDVORAK = %d)\n"), dwOptionFlag));
                if (dwOptionFlag & DVORAK_SUPPORT)
                    dwOptionFlag &= ~DVORAK_SUPPORT;
                else
                    dwOptionFlag |= DVORAK_SUPPORT;
                break;
            case IDM_ESC_ASCII:
                DEBUGPRINTFEX(100, (TEXT("\tESC = %d)\n"), dwOptionFlag));
                if (dwOptionFlag & ESCENG_SUPPORT)
                    dwOptionFlag &= ~ESCENG_SUPPORT;
                else
                    dwOptionFlag |= ESCENG_SUPPORT;
                break;
            case IDM_WORD_UNIT:
                DEBUGPRINTFEX(100, (TEXT("\tWORD = %d)\n"), dwImeFlag));
                if (dwImeFlag & SAENARU_ONTHESPOT)
                    dwImeFlag &= ~SAENARU_ONTHESPOT;
                else
                    dwImeFlag |= SAENARU_ONTHESPOT;
                break;
            case IDM_HANGUL_TOGGLE:
                DEBUGPRINTFEX(100, (TEXT("\tTOGGLE = %d)\n"), dwOptionFlag));
                if (dwOptionFlag & USE_SHIFT_SPACE)
                    dwOptionFlag &= ~USE_SHIFT_SPACE;
                else
                    dwOptionFlag |= USE_SHIFT_SPACE;
                break;
            case IDM_CONFIG:
                {
                    HKL hKL = GetKeyboardLayout(0);
                    HWND hWnd = GetFocus();

                    if (hKL == NULL)
                        break;
                    (void) ImeConfigure(hKL, hWnd, IME_CONFIG_GENERAL, NULL);
                }
                break;
        }

        _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
        return S_OK;
    }

    if (wID < ARRAYSIZE(c_rgMenuItems) && c_rgMenuItems[wID].pfnHandler != NULL) {
        c_rgMenuItems[wID].pfnHandler(_pSaenaru);
    }

    _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetIcon
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetIcon(HICON *phIcon)
{
    LPTSTR lpDesc;
    TCHAR szDesc[128];
    lpDesc = (LPTSTR)szDesc;

    DWORD status = _pSaenaru->GetConversionStatus();

    DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::GetIcon(status = %x)\n"), status));
    if (status & CMODE_HANGUL)
    {
        if (dwImeFlag & AUTOMATA_3SET)
            lstrcpyn(lpDesc, TEXT("INDIC_HAN"), 10);
        else
            lstrcpyn(lpDesc, TEXT("INDIC_HAN2"), 11);
    }
    else
    {
        lstrcpyn(lpDesc, TEXT("INDIC_ENG"), 10);
    }

    if (!(_tfLangBarItemInfo.dwStyle & TF_LBI_STYLE_TEXTCOLORICON))
    {
        lpDesc[lstrlen(lpDesc) + 1] = TEXT('\0');
        lpDesc[lstrlen(lpDesc)] = TEXT('X'); // append 'X' to get the 32-bit icon with alpha channel.
    }
    DEBUGPRINTFEX(100, (TEXT("\tIcon = %s\n"), lpDesc));

    *phIcon = (HICON)LoadImage(g_hInst, lpDesc, IMAGE_ICON, 16, 16, 0);

    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

//+---------------------------------------------------------------------------
//
// GetText
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::GetText(BSTR *pbstrText)
{
    *pbstrText = SysAllocString(LANGBAR_ITEM_DESC);

    return (*pbstrText == NULL) ? E_OUTOFMEMORY : S_OK;
}

//+---------------------------------------------------------------------------
//
// AdviseSink
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie)
{
    if (!IsEqualIID(IID_ITfLangBarItemSink, riid))
        return CONNECT_E_CANNOTCONNECT;

    if (_pLangBarItemSink != NULL)
        return CONNECT_E_ADVISELIMIT;

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK)
    {
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    *pdwCookie = SAENARU_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

//
// _OnUpdate
//
void CLangBarItemButton::_OnUpdate(DWORD dwFlags)
{
    if (_pLangBarItemSink)
        _pLangBarItemSink->OnUpdate(dwFlags);
}

//+---------------------------------------------------------------------------
//
// UnadviseSink
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::UnadviseSink(DWORD dwCookie)
{
    if (dwCookie != SAENARU_LANGBARITEMSINK_COOKIE)
        return CONNECT_E_NOCONNECTION;

    if (_pLangBarItemSink == NULL)
        return CONNECT_E_NOCONNECTION;

    _pLangBarItemSink->Release();
    _pLangBarItemSink = NULL;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _InitLanguageBar
//
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_InitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;
    BOOL fRet;
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_InitLanguageBar()\n")));

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if ((_pLangBarModeItem = new CLangBarItemButton(this, c_guidLangBarItemButton)) == NULL)
        goto Exit;

    _pLangBarItem = NULL;
    if (IsWindowsVersionOrLater(6, 2, 0))
    {
        if ((_pLangBarItem = new CLangBarItemButton(this, GUID_LBI_INPUTMODE)) == NULL)
            goto Exit;
    }

    if ((_pLangBarShapeItem = new CLangBarItemShapeButton(this)) == NULL)
        goto Exit;

    if ((_pLangBarImeItem = new CLangBarItemImeButton(this)) == NULL)
        goto Exit;

    if (pLangBarItemMgr->AddItem(_pLangBarModeItem) != S_OK)
    {
        _pLangBarModeItem->Release();
        _pLangBarModeItem = NULL;
        goto Exit;
    }

    if (_pLangBarItem && pLangBarItemMgr->AddItem(_pLangBarItem) != S_OK)
    {
        _pLangBarItem->Release();
        _pLangBarItem = NULL;
        goto Exit;
    }

    if (pLangBarItemMgr->AddItem(_pLangBarShapeItem) != S_OK)
    {
        _pLangBarShapeItem->Release();
        _pLangBarShapeItem = NULL;
        goto Exit;
    }

    if (pLangBarItemMgr->AddItem(_pLangBarImeItem) != S_OK)
    {
        _pLangBarImeItem->Release();
        _pLangBarImeItem = NULL;
        goto Exit;
    }

    fRet = TRUE;

Exit:
    pLangBarItemMgr->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// _UninitLanguageBar
//
//----------------------------------------------------------------------------

void CSaenaruTextService::_UninitLanguageBar()
{
    ITfLangBarItemMgr *pLangBarItemMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfLangBarItemMgr, (void **)&pLangBarItemMgr) == S_OK)
    {
        if (_pLangBarItem)
        {
            pLangBarItemMgr->RemoveItem(_pLangBarItem);
            _pLangBarItem->Release();
            _pLangBarItem = NULL;
        }
        if (_pLangBarModeItem)
        {
            pLangBarItemMgr->RemoveItem(_pLangBarModeItem);
            _pLangBarModeItem->Release();
            _pLangBarModeItem = NULL;
        }
        if (_pLangBarShapeItem)
        {
            pLangBarItemMgr->RemoveItem(_pLangBarShapeItem);
            _pLangBarShapeItem->Release();
            _pLangBarShapeItem = NULL;
        }
        if (_pLangBarImeItem)
        {
            pLangBarItemMgr->RemoveItem(_pLangBarImeItem);
            _pLangBarImeItem->Release();
            _pLangBarImeItem = NULL;
        }
        pLangBarItemMgr->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _UpdateLanguageBar
//
//----------------------------------------------------------------------------

void CSaenaruTextService::_UpdateLanguageBar(DWORD dwFlags)
{
    if (_pLangBarItem)
    {
        _pLangBarItem->_OnUpdate(dwFlags);
    }
    if (_pLangBarModeItem)
    {
        _pLangBarModeItem->_OnUpdate(dwFlags);
    }
    if (_pLangBarShapeItem)
    {
        _pLangBarShapeItem->_OnUpdate(dwFlags);
    }
    if (_pLangBarImeItem)
    {
        _pLangBarImeItem->_OnUpdate(dwFlags);
    }
}

/*
 * vim: et sts=4 sw=4
 */
