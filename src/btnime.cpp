/* imported from the skkime and modified by wkpark */
#if !defined (NO_TSF)
/*    LanguageBar 궻걏볺쀍뺴렜갎걐??깛귩럌귡갃
 */
#include "windows.h"
#include "tchar.h"
#include "immdev.h"

extern "C" {
#include "saenaru.h"
#include "resource.h"
}
/*    Platform SDK 궸댨뫔궥귡븫빁갃Platform SDK 궸 include path
 *    귩믅궥궻궕쀇궋궻궔붬궔갃뢯뿀귢궽 DDK 궸궞귢귞궻 header 귩
 *    뱊궢궲궘귢귡궴쀇궋궻궬궕갃
 */
#include "c:\Program Files\Microsoft SDK\include\msctf.h"
#include "olectl.h"
#if !defined (TARGET_WIN2K)
#include "c:\Program Files\Microsoft SDK\include\uxtheme.h"
#endif
#include "tsf.h"

#define LANGBAR_ITEM_DESC    L"글쇠/입력기 설정" // max 32 chars!
#define SAENARU_LANGBARITEMSINK_COOKIE    0x0fab0fac

static void  _Menu_Help (UINT);
static void  _Menu_Property (UINT);
static void  _Menu_Reconversion (UINT);
static void  _Menu_ToggleShowKeyboard (UINT);
static void  _Menu_ToggleDvorak (UINT);
static void  _Menu_ToggleOnTheSpot (UINT);
static void  _Menu_SelectKeyboard (UINT);
static DWORD _MenuItem_GetNormalFlag (UINT);
static DWORD _MenuItem_GetKeyboardFlag (UINT);
static DWORD _MenuItem_GetToggleKeyboardFlag (UINT);
static DWORD _MenuItem_GetToggleDvorakFlag (UINT);
static DWORD _MenuItem_GetToggleOnTheSpotFlag (UINT);
static DWORD _UserKeyboardMenu_GetKeyboardFlag (UINT);

typedef struct {
    const WCHAR* pchDesc;
    DWORD        (*pfnGetFlag)(UINT);
    void         (*pfnHandler)(UINT);
} TSFLBMENUINFOEX;

static const TSFLBMENUINFOEX c_rgMenuItems[] = {
//  { L"도움말(&H)",   _MenuItem_GetNormalFlag, _Menu_Help },
    { L"환경설정(&R)", _MenuItem_GetNormalFlag, _Menu_Property },
//  { L"재설정(&C)",   _MenuItem_GetNormalFlag, _Menu_Reconversion },
    { NULL, NULL, NULL },
    { L"드보락(&V)",   _MenuItem_GetToggleDvorakFlag,   _Menu_ToggleDvorak },
    { L"단어단위 편집(&W)", _MenuItem_GetToggleOnTheSpotFlag,_Menu_ToggleOnTheSpot },
    { NULL, NULL, NULL },
    { L"키보드 보기",  _MenuItem_GetToggleKeyboardFlag, _Menu_ToggleShowKeyboard },
    { NULL, NULL, NULL },
    { L"취소", NULL, NULL }
};

typedef struct {
    const WCHAR* pchDesc;
    DWORD        dwFlag;
} TSFLBKEYBOARDINFOEX;

static const TSFLBKEYBOARDINFOEX c_rgKeyboardItems[]= {
    { L"두벌식(&2)",        NULL },
    { L"세벌식(&3)",        NULL },
    { L"세벌식390(&9)",     NULL },
    { L"새두벌(&D)",        NULL },
    { L"새세벌(&S)",        NULL },
    { L"안마태(&A)",        NULL },
    { L"세벌식 순아래(&N)", NULL },
    { L"사용자 자판(&U)",   TF_LBMENUF_SUBMENU },
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

/*    갷??깛궻릶궬궚 class 궕뷠뾴궬궴뛩궑귡궻궕쀇궠궩궎궬갃
 *    뚣궔귞믁돿궢궶궋궴궋궚궶궋궔궶갃
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
    _tfLangBarItemInfo.clsidService   = c_clsidSaenaruTextService;
    //_tfLangBarItemInfo.clsidService = CLSID_NULL;
    _tfLangBarItemInfo.guidItem       = c_guidItemButtonIME;
    _tfLangBarItemInfo.dwStyle        = TF_LBI_STYLE_BTN_MENU
        | TF_LBI_STYLE_SHOWNINTRAY
#if 0
        | TF_LBI_STYLE_HIDDENSTATUSCONTROL
#endif
        ;
    _tfLangBarItemInfo.ulSort = 1;
    SafeStringCopy (_tfLangBarItemInfo.szDescription, ARRAYSIZE (_tfLangBarItemInfo.szDescription), LANGBAR_ITEM_DESC);
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

/*    Button 궻 tooltip 귩뺅궥갃뺅궥뭠궼 SysAllocString 궸귝궯궲
 *    둴뺎궢궫쀌덃궸룕궔귢귡뷠뾴궕궇귡갃궞귢귩 SysFreeString 궥
 *    귡궻궼갂뚁귂뢯궢궫뫀궻먖봀궳궇귡갃
 */
STDAPI
CLangBarItemImeButton::GetTooltipString (
    BSTR* pbstrToolTip)
{
    if (pbstrToolTip == NULL)
        return E_INVALIDARG;

    *pbstrToolTip = SysAllocString (LANGBAR_ITEM_DESC);
    return (*pbstrToolTip == NULL)? E_OUTOFMEMORY : S_OK;
}

/*    ITfLangBarItemButton::OnClick
 *
 *    궞궻 method 궼깇?긗궕뙻뚭긫?궻 TF_LBI_STYLE_BTN_BUTTON 귏궫
 *    궼 TF_LBI_STYLE_BTN_TOGGLE 긚?귽깑귩렃궯궲궋귡??깛궻뤵궳?
 *    긂긚귩긏깏긞긏궢궫렄궸뚁귂뢯궠귢귡갃
 *    귖궢??깛 item 궕 TF_LBI_STYLE_BTN_BUTTON 긚?귽깑귩렃궫궶궋
 *    궻궶귞갂궞궻 method 럊귦귢궶궋갃
 *(*)
 *    뜞궻륉떟궳궼벫궸돺귖궥귡뷠뾴궼궶궋궻궳갂S_OK 귩뫂뺅궥갃
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
 *    궞궻 method 궼 TF_LBI_STYLE_BTN_MENU 긚?귽깑귩렃궯궫뙻뚭긫?궻??깛
 *    귩뙻뚭긫?궕??깛궸뫮궢궲?렑궥귡 menu item 귩믁돿궢궲뾎뚼궸궥귡궫귕
 *    궸뚁귂뢯궠귢귡갃
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

    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::InitMenu (ITfMenu:%p)\n"), pMenu));

    if (pMenu == NULL)
        return E_INVALIDARG;

    id = 0;
    for (i = 0; i < 2; i ++) {
        wstrDesc = c_rgMenuItems [i].pchDesc;
        if (wstrDesc != NULL) {
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
    for (i = 0; i < ARRAYSIZE (c_rgKeyboardItems); i++)
    {
        wstrDesc = c_rgKeyboardItems [i].pchDesc;
        nstrDesc = wcslen (wstrDesc);
        dwFlag = c_rgKeyboardItems [i].dwFlag;
        if (dwFlag == NULL)
        {
            dwFlag = _MenuItem_GetKeyboardFlag(id);
            pMenu->AddMenuItem (id++,
                   dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
        } else {
            idUserKeyboardMenu = id;
            pMenu->AddMenuItem (id++,
                   dwFlag, NULL, NULL, wstrDesc, nstrDesc, &pUserKeyboardMenu);
        }
    }
    // add seperator
    if (i)
        pMenu->AddMenuItem (id++,
                        TF_LBMENUF_SEPARATOR , NULL, NULL, NULL, 0, NULL);

    // add extra config menus
    id=20;
    for (i = 2; i < ARRAYSIZE (c_rgMenuItems); i++)
    {
        wstrDesc = c_rgMenuItems [i].pchDesc;
        if (wstrDesc != NULL) {
            nstrDesc = wcslen (wstrDesc);
            dwFlag = (c_rgMenuItems [i].pfnGetFlag != NULL)? (c_rgMenuItems [i].pfnGetFlag)(i) : 0;
        } else {
            nstrDesc = 0;
            dwFlag = TF_LBMENUF_SEPARATOR;
        }
        pMenu->AddMenuItem (id++, dwFlag, NULL, NULL, wstrDesc, nstrDesc, NULL);
    }
    
    // addd User defined Keyboads (SubMenu)
    if (idUserKeyboardMenu) {
        HKEY hKey;

        id = idUserKeyboardMenu;

        if (!GetRegKeyHandle(TEXT("\\Keyboard"), &hKey))
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
    return S_OK;
}

STDAPI
CLangBarItemImeButton::OnMenuSelect (
    UINT wID)
{
//    if (wID >= ARRAYSIZE (c_rgMenuItems))
//        return    E_FAIL;
    MyDebugPrint((TEXT("MenuSelect:%x\n"), wID));

    if (wID == 0 || wID >= 20 ){
        if (wID >= 20) wID-= 18;
        if (c_rgMenuItems [wID].pfnHandler != NULL) {
            c_rgMenuItems [wID].pfnHandler (wID);
            //UpdateLanguageBar ();
        }
    } else {
        _Menu_SelectKeyboard (wID);
    }
    return    S_OK;
}

STDAPI
CLangBarItemImeButton::GetIcon (
    HICON*                    phIcon)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::GetIcon(%p)\n"), phIcon));

    if (phIcon == NULL)
        return    E_INVALIDARG;

    *phIcon    = (HICON)LoadImage (hInst, TEXT ("INDIC_KEY"), IMAGE_ICON, 16, 16, 0);
    return (*phIcon != NULL) ? S_OK : E_FAIL;
}

STDAPI
CLangBarItemImeButton::GetText (
    BSTR*            pbstrText)
{
    DEBUGPRINTFEX (100, (TEXT ("CLangBarItemImeButton::GetText ()\n")));

    if (pbstrText == NULL)
        return    E_INVALIDARG;

    *pbstrText    = SysAllocString (LANGBAR_ITEM_DESC);
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
    /*    궎?귪갂HELP 궼띿궯궲궶궋갷갃*/
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

            /*    빾듂긾?긤귩떗맕밒궸돹뼹볺쀍긾?긤궸먠믦궥귡갃*/
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

void
_Menu_ToggleShowKeyboard (UINT wID)
{
    HIMC    hIMC;
    DWORD    dwConversion, dwSentence;

    hIMC    = _GetCurrentHIMC ();
    if (hIMC == NULL)
        return;

    if (ImmGetOpenStatus (hIMC) && ImmGetConversionStatus (hIMC, &dwConversion, &dwSentence))
    {
        if (dwConversion & IME_CMODE_SOFTKBD)
            dwConversion &= ~IME_CMODE_SOFTKBD;
        else
            dwConversion |= IME_CMODE_SOFTKBD;
    }
    ImmSetConversionStatus (hIMC, dwConversion, 0);
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
    //static HKL hkl=NULL;
    if (dwOptionFlag & DVORAK_SUPPORT)
    {
        dwOptionFlag &= ~DVORAK_SUPPORT;
	//if (hkl)
	//    UnloadKeyboardLayout(hkl);
	//hkl=NULL;
    }
    else
    {
        dwOptionFlag |= DVORAK_SUPPORT;
	//hkl=LoadKeyboardLayout(TEXT("00010409"),KLF_ACTIVATE);
    }
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
    return (dwOptionFlag & DVORAK_SUPPORT) ? 1 : 0;
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

    if (ImmGetOpenStatus (hIMC) && ImmGetConversionStatus (hIMC, &dwConversion, &dwSentence))
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
