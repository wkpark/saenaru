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
 * $Saenaru: saenaru/src/btncmd.cpp,v 1.4 2006/10/11 03:46:34 wkpark Exp $
 */

#if !defined (NO_TSF)
/*	LanguageBar ªÎ¡¸ìýÕô«â?«É¡¹??«óªòÞÉªë¡£
 */
#include "windows.h"
#include "tchar.h"
#include "immdev.h"

#include "objbase.h"
extern "C" {
#include "saenaru.h"
#include "resource.h"
}
/*	Platform SDK ªËëîðíª¹ªëÝ»ÝÂ¡£Platform SDK ªË include path
 *	ªò÷×ª¹ªÎª¬ÕÞª¤ªÎª«Üúª«¡£õó?ªìªÐ DDK ªËª³ªìªéªÎ header ªò
 *	Ô¤ª·ªÆª¯ªìªëªÈÕÞª¤ªÎªÀª¬¡£
 */
//#include "c:\Program Files\Microsoft SDK\include\msctf.h"
#include "msctf.h"
#include "olectl.h"
#if !defined (TARGET_WIN2K)
//#include "c:\Program Files\Microsoft SDK\include\uxtheme.h"
#include "uxtheme.h"
#endif
#include "tsf.h"

//#define LANGBAR_ITEM_DESC	L"ÀÔ·Â¹æ½Ä" // max 32 chars

static	void	_Menu_ToHangul (void) ;
static	void	_Menu_ToHanja  (void) ;
static	void	_Menu_ToAscII  (void) ;
static	void	_Menu_ToCMode  (DWORD fdwConversion) ;
static	int	_GetConversionMode (HIMC hIMC) ;

static	const TSFLBMENUINFO	c_rgMenuItems []	= {
#if 0
	{ L"ÇÑ±Û",	_Menu_ToHangul },
	{ L"¿µ¹®",	_Menu_ToAscII },
	{ L"ÇÑÀÚ",	_Menu_ToHanja },
	{ NULL,		NULL },
	{ L"Ãë¼Ò",	NULL }
#endif
	{ IDS_MENU_HANGUL,	_Menu_ToHangul },
	{ IDS_MENU_ASCII,	_Menu_ToAscII },
	{ IDS_MENU_HANJA,	_Menu_ToHanja },
	{ NULL,		NULL },
	{ IDS_MENU_CANCEL,	NULL }
} ;

#define	SAENARU_LANGBARITEMSINK_COOKIE	0x0fab0faa

enum {
	MENU_ITEM_INDEX_CANCEL	= -1,
	MENU_ITEM_INDEX_HANGUL	= 0,
	MENU_ITEM_INDEX_ASCII,
	MENU_ITEM_INDEX_HANJA,
} ;

/*	CModeItemButton: Conversion Mode Language Bar Item Button
 */
class	CLangBarItemCModeButton : public ITfLangBarItemButton,
						   public ITfSource
{
public:
	CLangBarItemCModeButton () ;
	~CLangBarItemCModeButton () ;

	// IUnknown
	STDMETHODIMP QueryInterface (REFIID riid, void **ppvObj) ;
	STDMETHODIMP_(ULONG) AddRef (void) ;
	STDMETHODIMP_(ULONG) Release (void) ;
	
	// ITfLangBarItem
	STDMETHODIMP GetInfo (TF_LANGBARITEMINFO *pInfo) ;
	STDMETHODIMP GetStatus (DWORD *pdwStatus) ;
	STDMETHODIMP Show (BOOL fShow) ;
	STDMETHODIMP GetTooltipString (BSTR *pbstrToolTip) ;
	
	// ITfLangBarItemButton
	STDMETHODIMP OnClick (TfLBIClick click, POINT pt, const RECT *prcArea) ;
	STDMETHODIMP InitMenu (ITfMenu *pMenu) ;
	STDMETHODIMP OnMenuSelect (UINT wID) ;
	STDMETHODIMP GetIcon (HICON *phIcon) ;
	STDMETHODIMP GetText (BSTR *pbstrText) ;

	// ITfSource
	STDMETHODIMP AdviseSink(REFIID riid, IUnknown *punk, DWORD *pdwCookie);
	STDMETHODIMP UnadviseSink(DWORD dwCookie);

private:
	ITfLangBarItemSink*		_pLangBarItemSink ;
	TF_LANGBARITEMINFO		_tfLangBarItemInfo ;
	LONG					_cRef ;
} ;

/*
 */
CLangBarItemCModeButton::CLangBarItemCModeButton ()
{
	//DllAddRef () ;
	LPTSTR lpDesc;
#if 1
	_tfLangBarItemInfo.clsidService	= c_clsidSaenaruTextService ;
	//_tfLangBarItemInfo.clsidService	= CLSID_NULL ;
	_tfLangBarItemInfo.guidItem		= c_guidItemButtonCMode ;
	_tfLangBarItemInfo.dwStyle		= TF_LBI_STYLE_BTN_MENU
	       | TF_LBI_STYLE_SHOWNINTRAY
	       | TF_LBI_STYLE_HIDDENSTATUSCONTROL
#if 1
	       | TF_LBI_STYLE_TEXTCOLORICON
#endif
	       ;
	_tfLangBarItemInfo.ulSort		= 1 ;

	lpDesc=(LPTSTR)&_tfLangBarItemInfo.szDescription;
	LoadString(hInst,IDS_INPUT_CMODE_DESC, lpDesc,ARRAYSIZE (_tfLangBarItemInfo.szDescription));

	// SafeStringCopy (_tfLangBarItemInfo.szDescription, ARRAYSIZE (_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC) ;
	_pLangBarItemSink	= NULL ;
	_cRef				= 1 ;
#endif
	return ;
}

CLangBarItemCModeButton::~CLangBarItemCModeButton ()
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::~CLangBarItemCModeButton (this:%p)\n"), this)) ;
	//DllRelease () ;
	return ;
}

STDAPI
CLangBarItemCModeButton::QueryInterface (
	REFIID			riid,
	void**			ppvObj)
{
	if (ppvObj == NULL)
		return	E_INVALIDARG ;

	*ppvObj	= NULL ;
	if (IsEqualIID (riid, IID_IUnknown) ||
		IsEqualIID (riid, IID_ITfLangBarItem) ||
		IsEqualIID (riid, IID_ITfLangBarItemButton)) {
		*ppvObj	= (ITfLangBarItemButton *)this ;
	} else if (IsEqualIID (riid, IID_ITfSource)) {
		*ppvObj	= (ITfSource *)this ;
	}
	if (*ppvObj != NULL) {
		AddRef () ;
		return	S_OK ;
	}
	return	E_NOINTERFACE ;
}

STDAPI_(ULONG)
CLangBarItemCModeButton::AddRef ()
{
	return	++ _cRef ;
}

STDAPI_(ULONG)
CLangBarItemCModeButton::Release ()
{
	LONG	cr	= -- _cRef ;

	if (_cRef == 0) {
		delete	this ;
	}
	return	cr ;
}

STDAPI
CLangBarItemCModeButton::GetInfo (
	TF_LANGBARITEMINFO*		pInfo)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::GetInfo (this:%p)\n"), this)) ;

	if (pInfo == NULL)
		return	E_INVALIDARG ;

	*pInfo	= _tfLangBarItemInfo ;
	return	S_OK ;
}

STDAPI
CLangBarItemCModeButton::Show (
	BOOL					fShow)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::Show (BOOL:%d)\n"), fShow)) ;
    return	E_NOTIMPL ;
}

STDAPI
CLangBarItemCModeButton::GetStatus (
	DWORD*					pdwStatus)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::Show (DWORD*:%d)\n"), pdwStatus)) ;

	if (pdwStatus == NULL)
		return	E_INVALIDARG ;

	*pdwStatus	= (_GetCurrentHIMC () != NULL)? 0 : TF_LBI_STATUS_DISABLED ;
	return	S_OK ;
}

/*	Button ªÎ tooltip ªòÚ÷ª¹¡£Ú÷ª¹ö·ªÏ SysAllocString ªËªèªÃªÆ
 *	ü¬ÜÁª·ª¿ÖÅæ´ªËßöª«ªìªëù±é©ª¬ª¢ªë¡£ª³ªìªò SysFreeString ª¹
 *	ªëªÎªÏ¡¢û¼ªÓõóª·ª¿ö°ªÎô¡ìòªÇª¢ªë¡£
 */
STDAPI
CLangBarItemCModeButton::GetTooltipString (
	BSTR*					pbstrToolTip)
{
	if (pbstrToolTip == NULL)
		return	E_INVALIDARG ;

	*pbstrToolTip	= SysAllocString (_tfLangBarItemInfo.szDescription) ;
	return	(*pbstrToolTip == NULL)? E_OUTOFMEMORY : S_OK ;
}

/*	ITfLangBarItemButton::OnClick
 *
 *	ª³ªÎ method ªÏ«æ?«¶ª¬åëåÞ«Ð?ªÎ TF_LBI_STYLE_BTN_BUTTON ªÞª¿
 *	ªÏ TF_LBI_STYLE_BTN_TOGGLE «¹?«¤«ëªòò¥ªÃªÆª¤ªë??«óªÎß¾ªÇ?
 *	«¦«¹ªò«¯«ê«Ã«¯ª·ª¿ãÁªËû¼ªÓõóªµªìªë¡£
 *	ªâª·??«ó item ª¬ TF_LBI_STYLE_BTN_BUTTON «¹?«¤«ëªòò¥ª¿ªÊª¤
 *	ªÎªÊªé¡¢ª³ªÎ method ÞÅªïªìªÊª¤¡£
 *(*)
 *	ÐÑªÎ?üÏªÇªÏ÷åªËù¼ªâª¹ªëù±é©ªÏªÊª¤ªÎªÇ¡¢S_OK ªò?Ú÷ª¹¡£
 */
STDAPI
CLangBarItemCModeButton::OnClick (
	TfLBIClick				click,
	POINT					pt,
	const RECT*				prcArea)
{
	return	S_OK ;
}

/*	ITfLangBarItemButton::InitMenu
 *
 *	ª³ªÎ method ªÏ TF_LBI_STYLE_BTN_MENU «¹?«¤«ëªòò¥ªÃª¿åëåÞ«Ð?ªÎ??«ó
 *	ªòåëåÞ«Ð?ª¬??«óªË?ª·ªÆ?ãÆª¹ªë menu item ªòõÚÊ¥ª·ªÆêó?ªËª¹ªëª¿ªá
 *	ªËû¼ªÓõóªµªìªë¡£
 */
STDAPI
CLangBarItemCModeButton::InitMenu (
	ITfMenu*				pMenu)
{
	register HIMC		hIMC ;
	register int		i, nCMode ;
	register DWORD		dwFlag ;
	register LPCWSTR	wstrDesc ;
	register ULONG		nstrDesc ;

	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::InitMenu (ITfMenu:%p)\n"), pMenu)) ;

	if (pMenu == NULL)
		return	E_INVALIDARG ;

	hIMC	= _GetCurrentHIMC () ;
	if (hIMC != NULL) {
		/*	ªÈª¤ª¦ªïª±ªÇ??«óª¬äãªµªìª¿ãÁªË?ãÆªµªìªë«á«Ë«å?ªÎ
		 *	Ôô?ªòú¼ª¦¡£
		 */
		nCMode	= _GetConversionMode (hIMC) ;
		for (i = 0 ; i < ARRAYSIZE (c_rgMenuItems) ; i ++) {
			LPTSTR lpDesc;
			TCHAR szDesc[128];

			lpDesc=(LPTSTR)&szDesc;
			if (c_rgMenuItems [i].chDesc!=NULL)
				LoadString(hInst,c_rgMenuItems [i].chDesc,lpDesc,128);
			else
				lpDesc=NULL;
			wstrDesc		= (LPCWSTR)lpDesc;
			//wstrDesc		= c_rgMenuItems [i].pchDesc ;
			if (wstrDesc != NULL) {
				nstrDesc	= wcslen (wstrDesc) ;
				dwFlag		= (i == nCMode)? TF_LBMENUF_CHECKED : 0 ;
			} else {
				nstrDesc	= 0 ;
				dwFlag		= TF_LBMENUF_SEPARATOR ;
			}
			pMenu->AddMenuItem (i, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL) ;
		}
	}
	return	S_OK ;
}

STDAPI
CLangBarItemCModeButton::OnMenuSelect (
	UINT					wID)
{
	if (wID >= ARRAYSIZE (c_rgMenuItems))
		return	E_FAIL ;

	/*	NULL ªÎíÞùêªÏ Cancel ªÀªÈÞÖª¦ª³ªÈªËª¹ªë¡£*/
	if (c_rgMenuItems [wID].pfnHandler != NULL) {
		c_rgMenuItems [wID].pfnHandler () ;
		UpdateLanguageBar () ;
	}
	return	S_OK ;
}

STDAPI
CLangBarItemCModeButton::GetIcon (
	HICON*					phIcon)
{
	HIMC	hIMC ;
	DWORD	dwConversion, dwSentence ;
	register LPCTSTR	str	= NULL ;

	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::GetIcon(%p)\n"), phIcon)) ;

	if (phIcon == NULL)
		return	E_INVALIDARG ;

	hIMC	= _GetCurrentHIMC () ;
	if (hIMC == NULL) 
		goto	Skip ;

	if (ImmGetOpenStatus (hIMC)) {
		if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentence)) {
			if (dwConversion & IME_CMODE_HANJACONVERT){
				if (dwConversion & IME_CMODE_NATIVE)
				{
					str	= TEXT ("INDIC_HANJA") ;
				} else {
					str	= TEXT ("INDIC_ENG") ;
				}
			} else {
				if (dwConversion & IME_CMODE_NATIVE){
					str	= TEXT ("INDIC_HAN") ;
				} else {
					str	= TEXT ("INDIC_ENG") ;
				}
			}
		}
	} else {
		str	= TEXT ("INDIC_ENG") ;
	}
Skip:
	if (str == NULL)
		str	= TEXT ("INDIC_ENG") ;

    *phIcon	= (HICON)LoadImage (hInst, str, IMAGE_ICON, 16, 16, 0);
    return (*phIcon != NULL) ? S_OK : E_FAIL ;
}

STDAPI
CLangBarItemCModeButton::GetText (
	BSTR*			pbstrText)
{
	if (pbstrText == NULL)
		return	E_INVALIDARG ;

	*pbstrText	= SysAllocString (_tfLangBarItemInfo.szDescription) ;
	return	(*pbstrText == NULL)? E_OUTOFMEMORY : S_OK ;
}

STDAPI
CLangBarItemCModeButton::AdviseSink (
	REFIID			riid,
	IUnknown*		punk,
	DWORD*			pdwCookie)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemCModeButton::AdviseSink (this:%p)\n"), this)) ;

    if (!IsEqualIID (IID_ITfLangBarItemSink, riid)) {
		DEBUGPRINTFEX (100, (TEXT ("CONNECT_E_CANNOTCONNECT\n"))) ;
		return	CONNECT_E_CANNOTCONNECT ;
	}

    if (_pLangBarItemSink != NULL) {
		DEBUGPRINTFEX (100, (TEXT ("CONNECT_E_ADVISELIMIT\n"))) ;
        return	CONNECT_E_ADVISELIMIT ;
	}

    if (punk->QueryInterface (IID_ITfLangBarItemSink, (void **)&_pLangBarItemSink) != S_OK) {
		DEBUGPRINTFEX (100, (TEXT ("E_NOINTERFACE\n"))) ;
        _pLangBarItemSink	= NULL ;
        return	E_NOINTERFACE ;
    }

    *pdwCookie	= SAENARU_LANGBARITEMSINK_COOKIE ;
    return	S_OK ;
}

STDAPI
CLangBarItemCModeButton::UnadviseSink (
	DWORD			dwCookie)
{
    if (dwCookie != SAENARU_LANGBARITEMSINK_COOKIE)
        return	CONNECT_E_NOCONNECTION ;

    if (_pLangBarItemSink == NULL)
        return	CONNECT_E_NOCONNECTION ;

    _pLangBarItemSink->Release () ;
    _pLangBarItemSink	= NULL ;

    return	S_OK ;
}

/*========================================================================*
 *	public function interface
 */
BOOL	PASCAL
CreateItemButtonCMode (
	register ITfLangBarItem** 	ppLangBarItem)
{
	if (ppLangBarItem == NULL)
		return	FALSE ;
	*ppLangBarItem	= new CLangBarItemCModeButton ;
	return	(*ppLangBarItem != NULL) ;
}

/*========================================================================*
 *	private functions
 */
void
_Menu_ToHangul (void)
{
	_Menu_ToCMode (IME_CMODE_NATIVE) ;
}

void
_Menu_ToHanja    (void)
{
	_Menu_ToCMode (IME_CMODE_NATIVE | IME_CMODE_HANJACONVERT) ;
}

void
_Menu_ToAscII    (void)
{
	// XXX
	register HIMC	hIMC	= _GetCurrentHIMC () ;
	if (! hIMC)
		return ;
	_Menu_ToCMode (IME_CMODE_ROMAN) ;
	ImmSetOpenStatus (hIMC, FALSE) ;
}

void
_Menu_ToCMode (
	register DWORD		fdwConversion)
{
	register HIMC	hIMC	= _GetCurrentHIMC () ;
	if (! hIMC)
		return ;

	if (! ImmGetOpenStatus (hIMC)) 
		ImmSetOpenStatus (hIMC, TRUE) ;
	ImmSetConversionStatus (hIMC, fdwConversion, 0) ;
	return ;
}

int
_GetConversionMode (
	register HIMC	hIMC)
{
	DWORD	dwConversion, dwSentense ;

	if (! hIMC)
		return	MENU_ITEM_INDEX_ASCII ;

	if (! ImmGetOpenStatus (hIMC)) 
		return	MENU_ITEM_INDEX_ASCII ;

	if (! ImmGetConversionStatus (hIMC, &dwConversion, &dwSentense))
		return	MENU_ITEM_INDEX_CANCEL ;

	if (dwConversion & IME_CMODE_HANJACONVERT){
		if (dwConversion & IME_CMODE_NATIVE){
			return	MENU_ITEM_INDEX_HANJA ;
		} else {
			return	MENU_ITEM_INDEX_ASCII ;
		}
	} else {
		if (dwConversion & IME_CMODE_NATIVE){
			return	MENU_ITEM_INDEX_HANGUL ;
		} else {
			return	MENU_ITEM_INDEX_ASCII ;
		}
	}
}

#endif

/*
 * ex: ts=8 sts=8 sw=8 noet
 */
