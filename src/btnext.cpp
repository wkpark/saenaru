/* imported from skkime by wkpark */
#if !defined (NO_TSF)
/*	LanguageBar の「入力モ?ド」??ンを司る。
 */
#include "windows.h"
#include "tchar.h"
#include "immdev.h"

#include "objbase.h"
extern "C" {
#include "saenaru.h"
#include "resource.h"
}
/*	Platform SDK に依存する部分。Platform SDK に include path
 *	を通すのが良いのか否か。出来れば DDK にこれらの header を
 *	渡してくれると良いのだが。
 */
#include "c:\Program Files\Microsoft SDK\include\msctf.h"
#include "olectl.h"
#if !defined (TARGET_WIN2K)
#include "c:\Program Files\Microsoft SDK\include\uxtheme.h"
#endif
#include "tsf.h"

#define LANGBAR_ITEM_DESC	L"ﾀ�ｰ｢/ｹﾝｰ｢ ﾀ�ﾈｯ" // max 32 chars

#if 0
static	void	_Menu_ToHangul (void) ;
static	void	_Menu_ToHanja  (void) ;
static	void	_Menu_ToAscII  (void) ;
static	void	_Menu_ToCMode  (DWORD fdwConversion) ;
static	int	_GetConversionMode (HIMC hIMC) ;

static	const TSFLBMENUINFO	c_rgMenuItems []	= {
	{ L"ﾇﾑｱﾛ",	_Menu_ToHangul },
	{ L"ｿｵｹｮ",	_Menu_ToAscII },
	{ L"ﾇﾑﾀﾚ",	_Menu_ToHanja },
	{ NULL,		NULL },
	{ L"ﾃ�ｼﾒ",	NULL }
} ;
enum {
	MENU_ITEM_INDEX_CANCEL	= -1,
	MENU_ITEM_INDEX_HANGUL	= 0,
	MENU_ITEM_INDEX_ASCII,
	MENU_ITEM_INDEX_HANJA,
} ;
#endif

#define	SAENARU_LANGBARITEMSINK_COOKIE	0x0fab0fab

/*	CModeItemButton: Conversion Mode Language Bar Item Button
 */
class	CLangBarItemShapeButton : public ITfLangBarItemButton,
						   public ITfSource
{
public:
	CLangBarItemShapeButton () ;
	~CLangBarItemShapeButton () ;

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
CLangBarItemShapeButton::CLangBarItemShapeButton ()
{
	//DllAddRef () ;
#if 1
	_tfLangBarItemInfo.clsidService	= c_clsidSaenaruTextService ;
	//_tfLangBarItemInfo.clsidService	= CLSID_NULL ;
	_tfLangBarItemInfo.guidItem		= c_guidItemButtonShape ;
	_tfLangBarItemInfo.dwStyle		= TF_LBI_STYLE_BTN_BUTTON
	       | TF_LBI_STYLE_SHOWNINTRAY
//	       | TF_LBI_STYLE_HIDDENSTATUSCONTROL
	       | TF_LBI_STYLE_TEXTCOLORICON
	       ;
	_tfLangBarItemInfo.ulSort		= 1 ;
	SafeStringCopy (_tfLangBarItemInfo.szDescription, ARRAYSIZE (_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC) ;
	_pLangBarItemSink	= NULL ;
	_cRef				= 1 ;
#endif
	return ;
}

CLangBarItemShapeButton::~CLangBarItemShapeButton ()
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::~CLangBarItemShapeButton (this:%p)\n"), this)) ;
	//DllRelease () ;
	return ;
}

STDAPI
CLangBarItemShapeButton::QueryInterface (
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
CLangBarItemShapeButton::AddRef ()
{
	return	++ _cRef ;
}

STDAPI_(ULONG)
CLangBarItemShapeButton::Release ()
{
	LONG	cr	= -- _cRef ;

	if (_cRef == 0) {
		delete	this ;
	}
	return	cr ;
}

STDAPI
CLangBarItemShapeButton::GetInfo (
	TF_LANGBARITEMINFO*		pInfo)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::GetInfo (this:%p)\n"), this)) ;

	if (pInfo == NULL)
		return	E_INVALIDARG ;

	*pInfo	= _tfLangBarItemInfo ;
	return	S_OK ;
}

STDAPI
CLangBarItemShapeButton::Show (
	BOOL					fShow)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::Show (BOOL:%d)\n"), fShow)) ;
    return	E_NOTIMPL ;
}

STDAPI
CLangBarItemShapeButton::GetStatus (
	DWORD*					pdwStatus)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::Show (DWORD*:%d)\n"), pdwStatus)) ;

	if (pdwStatus == NULL)
		return	E_INVALIDARG ;

	*pdwStatus	= (_GetCurrentHIMC () != NULL)? 0 : TF_LBI_STATUS_DISABLED ;
	return	S_OK ;
}

/*	Button の tooltip を返す。返す値は SysAllocString によって
 *	確保した領域に書かれる必要がある。これを SysFreeString す
 *	るのは、呼び出した側の責任である。
 */
STDAPI
CLangBarItemShapeButton::GetTooltipString (
	BSTR*					pbstrToolTip)
{
	if (pbstrToolTip == NULL)
		return	E_INVALIDARG ;

	*pbstrToolTip	= SysAllocString (LANGBAR_ITEM_DESC) ;
	return	(*pbstrToolTip == NULL)? E_OUTOFMEMORY : S_OK ;
}

/*	ITfLangBarItemButton::OnClick
 *
 *	この method はユ?ザが言語バ?の TF_LBI_STYLE_BTN_BUTTON また
 *	は TF_LBI_STYLE_BTN_TOGGLE ス?イルを持っている??ンの上で?
 *	ウスをクリックした時に呼び出される。
 *	もし??ン item が TF_LBI_STYLE_BTN_BUTTON ス?イルを持たない
 *	のなら、この method 使われない。
 *(*)
 *	今の状況では特に何もする必要はないので、S_OK を即返す。
 */
STDAPI
CLangBarItemShapeButton::OnClick (
	TfLBIClick				click,
	POINT					pt,
	const RECT*				prcArea)
{
#if 1
	register HIMC		hIMC ;
	DWORD	dwConversion, dwSentense ;

	hIMC	= _GetCurrentHIMC () ;
	if (hIMC == NULL) return	S_OK ;

	if (!ImmGetOpenStatus (hIMC)) 
		return	S_OK ;
	//	ImmSetOpenStatus (hIMC, TRUE) ;
	if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentense))
	{
		if (dwConversion & IME_CMODE_FULLSHAPE)
			dwConversion &= ~IME_CMODE_FULLSHAPE;
		else
			dwConversion |= IME_CMODE_FULLSHAPE;
		ImmSetConversionStatus (hIMC, dwConversion, 0) ;
	}
	UpdateLanguageBar () ;
#endif
	return	S_OK ;
}

/*	ITfLangBarItemButton::InitMenu
 *
 *	この method は TF_LBI_STYLE_BTN_MENU ス?イルを持った言語バ?の??ン
 *	を言語バ?が??ンに対して?示する menu item を追加して有効にするため
 *	に呼び出される。
 */
STDAPI
CLangBarItemShapeButton::InitMenu (
	ITfMenu*				pMenu)
{
	register HIMC		hIMC ;
	register int		i, nCMode ;
	register DWORD		dwFlag ;
	register LPCWSTR	wstrDesc ;
	register ULONG		nstrDesc ;

	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::InitMenu (ITfMenu:%p)\n"), pMenu)) ;

	if (pMenu == NULL)
		return	E_INVALIDARG ;
#if 0
	hIMC	= _GetCurrentHIMC () ;
	if (hIMC != NULL) {
		/*	というわけで??ンが押された時に?示されるメニュ?の
		 *	登?を行う。
		 */
		nCMode	= _GetConversionMode (hIMC) ;
		for (i = 0 ; i < ARRAYSIZE (c_rgMenuItems) ; i ++) {
			wstrDesc		= c_rgMenuItems [i].pchDesc ;
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
#endif
	return	S_OK ;
}

STDAPI
CLangBarItemShapeButton::OnMenuSelect (
	UINT					wID)
{
#if 0
	if (wID >= ARRAYSIZE (c_rgMenuItems))
		return	E_FAIL ;

	/*	NULL の場合は Cancel だと思うことにする。*/
	if (c_rgMenuItems [wID].pfnHandler != NULL) {
		c_rgMenuItems [wID].pfnHandler () ;
		UpdateLanguageBar () ;
	}
#endif
	return	S_OK ;
}

STDAPI
CLangBarItemShapeButton::GetIcon (
	HICON*					phIcon)
{
	HIMC	hIMC ;
	DWORD	dwConversion, dwSentence ;
	register LPCTSTR	str	= NULL ;

	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::GetIcon(%p)\n"), phIcon)) ;

	if (phIcon == NULL)
		return	E_INVALIDARG ;

	hIMC	= _GetCurrentHIMC () ;
	if (hIMC == NULL) 
		goto	Skip ;

	if (ImmGetOpenStatus (hIMC)) {
		if (ImmGetConversionStatus (hIMC, &dwConversion, &dwSentence)) {
			if (dwConversion & IME_CMODE_FULLSHAPE){
				str	= TEXT ("INDIC_FULL") ;
			} else {
				str	= TEXT ("INDIC_HALF") ;
			}
		}
	} else {
		str	= TEXT ("INDIC_HALF") ;
	}
Skip:
	if (str == NULL) {
		str	= TEXT ("INDIC_HALF") ;
	}
	*phIcon	= (HICON)LoadImage (hInst, str, IMAGE_ICON, 16, 16, 0);
	return (*phIcon != NULL) ? S_OK : E_FAIL ;
}

STDAPI
CLangBarItemShapeButton::GetText (
	BSTR*			pbstrText)
{
	if (pbstrText == NULL)
		return	E_INVALIDARG ;

	*pbstrText	= SysAllocString (LANGBAR_ITEM_DESC) ;
	return	(*pbstrText == NULL)? E_OUTOFMEMORY : S_OK ;
}

STDAPI
CLangBarItemShapeButton::AdviseSink (
	REFIID			riid,
	IUnknown*		punk,
	DWORD*			pdwCookie)
{
	DEBUGPRINTFEX (100, (TEXT ("CLangBarItemShapeButton::AdviseSink (this:%p)\n"), this)) ;

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
CLangBarItemShapeButton::UnadviseSink (
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
CreateItemButtonShape (
	register ITfLangBarItem** 	ppLangBarItem)
{
	if (ppLangBarItem == NULL)
		return	FALSE ;
	*ppLangBarItem	= new CLangBarItemShapeButton ;
	return	(*ppLangBarItem != NULL) ;
}

#if 0
/*========================================================================*
 *	private functions
 */
void
_Menu_ToHangul (void)
{
	_Menu_ToCMode (IME_CMODE_HANGUL | IME_CMODE_LANGUAGE) ;
}

void
_Menu_ToHanja    (void)
{
	_Menu_ToCMode (IME_CMODE_HANGUL | IME_CMODE_FULLSHAPE) ;
}

void
_Menu_ToAscII    (void)
{
	register HIMC	hIMC	= _GetCurrentHIMC () ;
	DWORD conv = 0;
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

	if (dwConversion & IME_CMODE_FULLSHAPE){
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

#endif

