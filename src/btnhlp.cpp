/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 2006 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2006 Won-Kyu Park <wkpark@kldp.org>.
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
#include "windows.h"
#include "tchar.h"
#include "immdev.h"
#include <htmlhelp.h>

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

#define LANGBAR_ITEM_DESC	L"µµ¿ò¸»" // max 32 chars

static	void	_Menu_SaenaruHelp(void);
static	void	_Menu_About(void);

static	const TSFLBMENUINFO c_rgMenuItems[] = {
#if 1
	{ IDS_ABOUT, _Menu_About},
	{ IDS_HELP_DESC, _Menu_SaenaruHelp},
#endif
};

/*	CLangBarItemHelpMenu: Help Menu Button
 */
class	CLangBarItemHelpMenu : public ITfSystemLangBarItemSink
{
public:
	CLangBarItemHelpMenu ();
	~CLangBarItemHelpMenu ();

	// IUnknown
	STDMETHODIMP QueryInterface (REFIID riid, void **ppvObj);
	STDMETHODIMP_(ULONG) AddRef (void);
	STDMETHODIMP_(ULONG) Release (void);
	
	// ITfSystemLangBarItemSink
	STDMETHODIMP InitMenu (ITfMenu *pMenu);
	STDMETHODIMP OnMenuSelect (UINT wID);

private:
	ITfLangBarItemSink*		_pLangBarItemSink;
	TF_LANGBARITEMINFO		_tfLangBarItemInfo;
	LONG				_cRef;
};

/*
 */
CLangBarItemHelpMenu::CLangBarItemHelpMenu ()
{
	//DllAddRef ();
	//_tfLangBarItemInfo.clsidService	= c_clsidSaenaruTextService;
	_tfLangBarItemInfo.clsidService	= CLSID_NULL;
	_tfLangBarItemInfo.guidItem	= GUID_NULL;
	//_tfLangBarItemInfo.guidItem		= c_guidItemButtonMyHelp;
	_tfLangBarItemInfo.dwStyle              = TF_LBI_STYLE_BTN_MENU
                | TF_LBI_STYLE_SHOWNINTRAY
	      ;
	_tfLangBarItemInfo.ulSort	= 1;
	SafeStringCopy (_tfLangBarItemInfo.szDescription, ARRAYSIZE (_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);
	_pLangBarItemSink	= NULL;
	_cRef			= 1;
	return;
}

CLangBarItemHelpMenu::~CLangBarItemHelpMenu ()
{
	HRESULT     hr = S_OK;
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemHelpMenu::~CLangBarItemHelpMenu (this:%p)\n"), this));
	//DllRelease ();
	//
	return;
}

STDAPI
CLangBarItemHelpMenu::QueryInterface (
	REFIID			riid,
	void**			ppvObj)
{
	if (ppvObj == NULL)
		return	E_INVALIDARG;

	*ppvObj	= NULL;
	if (IsEqualIID (riid, IID_IUnknown)) {
		*ppvObj	= (ITfLangBarItemButton *)this;
	} else if (IsEqualIID (riid, IID_ITfSource)) {
		*ppvObj	= (ITfSource *)this;
	} else if (IsEqualIID (riid, IID_ITfSystemLangBarItemSink)) {
		*ppvObj	= (ITfSystemLangBarItemSink *)this;
	}
	if (*ppvObj != NULL) {
		AddRef ();
		return	S_OK;
	}
	return	E_NOINTERFACE;
}

STDAPI_(ULONG)
CLangBarItemHelpMenu::AddRef ()
{
	return	++ _cRef;
}

STDAPI_(ULONG)
CLangBarItemHelpMenu::Release ()
{
	LONG	cr	= -- _cRef;

	if (_cRef == 0) {
		delete	this;
	}
	return	cr;
}

/*	ITfLangBarItemButton::InitMenu
 *
 */
STDAPI
CLangBarItemHelpMenu::InitMenu (
	ITfMenu* pMenu)
{
	register int		i;
	register DWORD		dwFlag;
	register LPCWSTR	wstrDesc;
	register ULONG		nstrDesc;

        DEBUGPRINTFEX (100, (TEXT ("CLangBarItemHelpMenu::InitMenu (ITfMenu:%p)\n"), pMenu)) ;
	if (pMenu == NULL)
		return	E_INVALIDARG ;

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
			dwFlag		= 0;
		} else {
			nstrDesc	= 0 ;
			dwFlag		= TF_LBMENUF_SEPARATOR ;
		}
		pMenu->AddMenuItem (i, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL) ;
	}
	return	S_OK ;
}

STDAPI
CLangBarItemHelpMenu::OnMenuSelect (
	UINT wID)
{
	if (wID >= ARRAYSIZE (c_rgMenuItems))
		return	E_FAIL;

	if (c_rgMenuItems [wID].pfnHandler != NULL) {
		c_rgMenuItems [wID].pfnHandler();
	}
	return	S_OK;
}

/*========================================================================*
 *	private functions
 */
void
_Menu_SaenaruHelp (void)
{
    LPTSTR lpHelp;
    TCHAR szHelp[256];

    lpHelp=(LPTSTR)&szHelp;

    lpHelp += GetSaenaruDirectory(lpHelp,256);
    if (*(lpHelp-1) != TEXT('\\')) *lpHelp++ = TEXT('\\');
    //LoadString(hInst,c_rgMenuItems [i].chDesc,lpDesc,128);
    lstrcpy (lpHelp, TEXT("help\\saenaru.chm"));

    HtmlHelp(0, szHelp, HH_DISPLAY_TOPIC, NULL);
    return;
}

void
_Menu_About(void)
{
    HWND hWnd= GetFocus();
    DialogBox(hInst, MAKEINTRESOURCE(DLG_ABOUT), hWnd, AboutDlgProc);
    return;
}


/*========================================================================*
 *	public function interface
 */
BOOL	PASCAL
CreateItemHelp (
	register ITfSystemLangBarItemSink**   ppLangBarSink)
{
	if (ppLangBarSink == NULL)
		return	FALSE;
	*ppLangBarSink	= new CLangBarItemHelpMenu;
	return	(*ppLangBarSink != NULL);
}

#endif

/*
 * ex: ts=8 sts=4 sw=4 et
 */
