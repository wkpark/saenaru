/*
 * This file is part of Saenaru.
 *
 * Copyright(c) 2003 Hye-Shik Chang <perky@i18n.org>.
 * Copyright(c) 2003 Won-Kyu Park <wkpark@kldp.org>.
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
 * DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */

#include "globals.h"
#include "saenarutip.h"
#include "resource.h"
#include "btnshape.h"

#define SAENARU_LANGBARITEMSINK_COOKIE  0x0fab0fac

/*
 */
CLangBarItemShapeButton::CLangBarItemShapeButton(CSaenaruTextService *pSaenaru)
{
    DllAddRef();

    LPTSTR lpDesc;

    _tfLangBarItemInfo.clsidService = c_clsidSaenaruTextService;
    _tfLangBarItemInfo.guidItem = c_guidLangBarItemButtonShape;
    _tfLangBarItemInfo.dwStyle = TF_LBI_STYLE_BTN_BUTTON
            | TF_LBI_STYLE_SHOWNINTRAY
            | TF_LBI_STYLE_TEXTCOLORICON
            ;
    _tfLangBarItemInfo.ulSort = 1;

    lpDesc = (LPTSTR)& _tfLangBarItemInfo.szDescription;
    LoadString(g_hInst, IDS_TOGGLE_HALFFULL_DESC, lpDesc, ARRAYSIZE(_tfLangBarItemInfo.szDescription));

    _pSaenaru = pSaenaru;
    _pSaenaru->AddRef();
    _pLangBarItemSink = NULL;
    _cRef = 1;
    return;
}

CLangBarItemShapeButton::~CLangBarItemShapeButton()
{
    DllRelease();
    _pSaenaru->Release();
    return;
}

STDAPI
CLangBarItemShapeButton::QueryInterface(
    REFIID riid,
    void** ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;
    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfLangBarItem) ||
        IsEqualIID(riid, IID_ITfLangBarItemButton)) {
        *ppvObj = (ITfLangBarItemButton *)this;
    } else if (IsEqualIID(riid, IID_ITfSource)) {
        *ppvObj = (ITfSource *)this;
    }
    if (*ppvObj != NULL) {
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDAPI_(ULONG)
CLangBarItemShapeButton::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG)
CLangBarItemShapeButton::Release()
{
    LONG cr = --_cRef;

    if (_cRef == 0) {
        delete  this;
    }
    return cr;
}

STDAPI
CLangBarItemShapeButton::GetInfo(
    TF_LANGBARITEMINFO* pInfo)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemShapeButton::GetInfo(this:%p)\n"), this));

    if (pInfo == NULL)
        return E_INVALIDARG;

    *pInfo  = _tfLangBarItemInfo;
    return S_OK;
}

STDAPI
CLangBarItemShapeButton::Show(
    BOOL fShow)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemShapeButton::Show(BOOL:%d)\n"), fShow));
    return E_NOTIMPL;
}

STDAPI
CLangBarItemShapeButton::GetStatus(
    DWORD* pdwStatus)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemShapeButton::Show(DWORD*:%d)\n"), pdwStatus));

    if (pdwStatus == NULL)
        return E_INVALIDARG;

    *pdwStatus = 0 ; // TF_LBI_STATUS_DISABLED;
    return S_OK;
}

STDAPI
CLangBarItemShapeButton::GetTooltipString(
    BSTR*                   pbstrToolTip)
{
    if (pbstrToolTip == NULL)
        return E_INVALIDARG;

    *pbstrToolTip = SysAllocString(_tfLangBarItemInfo.szDescription);
    return (*pbstrToolTip == NULL)? E_OUTOFMEMORY : S_OK;
}

STDAPI
CLangBarItemShapeButton::OnClick(
    TfLBIClick              click,
    POINT                   pt,
    const RECT*             prcArea)
{
    DWORD dwConversion = 0;
    switch (click)
    {
        case TF_LBI_CLK_LEFT:
            dwConversion = _pSaenaru->GetConversionStatus();
            dwConversion ^= CMODE_FULLSHAPE;
            _pSaenaru->SetConversionStatus(dwConversion);

            _pLangBarItemSink->OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
            return S_OK;
        case TF_LBI_CLK_RIGHT:
            return S_OK;
    }

    return S_OK;
}

/*  ITfLangBarItemButton::InitMenu
 */
STDAPI
CLangBarItemShapeButton::InitMenu(
    ITfMenu*                pMenu)
{
    register HIMC       hIMC;
    register int        i, nCMode;
    register DWORD      dwFlag;
    register LPCWSTR    wstrDesc;
    register ULONG      nstrDesc;

    DEBUGPRINTFEX(100, (TEXT("CLangBarItemShapeButton::InitMenu(ITfMenu:%p)\n"), pMenu));

    if (pMenu == NULL)
        return E_INVALIDARG;

    return S_OK;
}

STDAPI
CLangBarItemShapeButton::OnMenuSelect(
    UINT wID)
{
    return S_OK;
}

STDAPI
CLangBarItemShapeButton::GetIcon(
    HICON* phIcon)
{
    DWORD dwConversion;
    register LPCTSTR str = NULL;

    DEBUGPRINTFEX(100, (TEXT("CLangBarItemShapeButton::GetIcon(%p)\n"), phIcon));

    if (phIcon == NULL)
        return E_INVALIDARG;

    dwConversion = _pSaenaru->GetConversionStatus();
    if (dwConversion & CMODE_FULLSHAPE) {
        str = TEXT("INDIC_FULL");
    } else {
        str = TEXT("INDIC_HALF");
    }
    *phIcon = (HICON)LoadImage(g_hInst, str, IMAGE_ICON, 16, 16, 0);
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI
CLangBarItemShapeButton::GetText(
    BSTR* pbstrText)
{
    if (pbstrText == NULL)
        return E_INVALIDARG;

    *pbstrText  = SysAllocString(_tfLangBarItemInfo.szDescription);
    return (*pbstrText == NULL)? E_OUTOFMEMORY : S_OK;
}

STDAPI
CLangBarItemShapeButton::AdviseSink(
    REFIID riid,
    IUnknown* punk,
    DWORD* pdwCookie)
{
    DEBUGPRINTFEX(100, (TEXT("CLangBarItemShapeButton::AdviseSink(this:%p)\n"), this));

    if (!IsEqualIID(IID_ITfLangBarItemSink, riid)) {
        DEBUGPRINTFEX(100, (TEXT("CONNECT_E_CANNOTCONNECT\n")));
        return CONNECT_E_CANNOTCONNECT;
    }

    if (_pLangBarItemSink != NULL) {
        DEBUGPRINTFEX(100, (TEXT("CONNECT_E_ADVISELIMIT\n")));
        return CONNECT_E_ADVISELIMIT;
    }

    if (punk->QueryInterface(IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK) {
        DEBUGPRINTFEX(100, (TEXT("E_NOINTERFACE\n")));
        _pLangBarItemSink = NULL;
        return E_NOINTERFACE;
    }

    *pdwCookie = SAENARU_LANGBARITEMSINK_COOKIE;
    return S_OK;
}

STDAPI
CLangBarItemShapeButton::UnadviseSink(
    DWORD dwCookie)
{
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
void CLangBarItemShapeButton::_OnUpdate(DWORD dwFlags)
{
    if (_pLangBarItemSink)
        _pLangBarItemSink->OnUpdate(dwFlags);
}

/*
 * ex: ts=4 sts=4 sw=4 et
 */
