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

extern "C" {
#include "hangul.h"
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

CLangBarItemButton::CLangBarItemButton(CSaenaruTextService *pSaenaru)
{
    DllAddRef();

    _tfLangBarItemInfo.clsidService = c_clsidSaenaruTextService;
    //_tfLangBarItemInfo.guidItem = GUID_LBI_INPUTMODE;
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButton;
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_SHOWNINTRAY
//        | TF_LBI_STYLE_BTN_BUTTON
        | TF_LBI_STYLE_BTN_MENU
        | TF_LBI_STYLE_TEXTCOLORICON
        ;
    _tfLangBarItemInfo.ulSort = 0;
    SafeStringCopy(_tfLangBarItemInfo.szDescription, ARRAYSIZE(_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);

    _pSaenaru = pSaenaru;
    _pSaenaru->AddRef();
    _pLangBarItemSink = NULL;

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
    *pdwStatus = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Show
//
//----------------------------------------------------------------------------

STDAPI CLangBarItemButton::Show(BOOL fShow)
{
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
    if (c_rgMenuItems[wID].pfnHandler != NULL) {
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
    DWORD status = _pSaenaru->GetConversionStatus();

    DEBUGPRINTFEX(100, (TEXT("CLangBarItemButton::GetIcon(status = %x)\n"), status));
    if (status & CMODE_HANGUL)
    {
        if (dwImeFlag & AUTOMATA_3SET)
            *phIcon = (HICON)LoadImage(g_hInst, TEXT("INDIC_HAN"),
                    IMAGE_ICON, 16, 16, 0);
        else
            *phIcon = (HICON)LoadImage(g_hInst, TEXT("INDIC_HAN2"),
                    IMAGE_ICON, 16, 16, 0);
    }
    else
    {
        *phIcon = (HICON)LoadImage(g_hInst, TEXT("INDIC_ENG"),
		    IMAGE_ICON, 16, 16, 0);
    }

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

    if ((_pLangBarItem = new CLangBarItemButton(this)) == NULL)
        goto Exit;

    if ((_pLangBarShapeItem = new CLangBarItemShapeButton(this)) == NULL)
        goto Exit;

    if ((_pLangBarImeItem = new CLangBarItemImeButton(this)) == NULL)
        goto Exit;

    if (pLangBarItemMgr->AddItem(_pLangBarItem) != S_OK)
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
