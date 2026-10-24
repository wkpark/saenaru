//
// candidate.cpp
//
// Hanja candidate list. ITfCandidateListUIElement implementation.
// It follows ConvHanja() and DicKeydownHandler() of the IMM IME.
//

#include "globals.h"
#include <commctrl.h>
#include "saenarutip.h"
#include "editsess.h"
#include "candidate.h"

extern "C" {
#include "hangul.h"
int GetCandidateStringsFromDictionary(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
int PASCAL GetSaenaruDirectory(LPTSTR lpDest, int max);
int GetHangulFromHanjaIndex(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
long PASCAL GetRegStringValue (LPCTSTR,LPCTSTR,LPTSTR);
UINT PASCAL is_ksx1002(UINT);
}

// {03E8748F-B232-4E6D-B3B1-F3F266B019B0}
static const GUID c_guidCandUIElement =
{ 0x03e8748f, 0xb232, 0x4e6d, { 0xb3, 0xb1, 0xf3, 0xf2, 0x66, 0xb0, 0x19, 0xb0 } };

static const WCHAR c_szCandWndClass[] = L"SaenaruTipCandidate";
static const WCHAR c_szCandDesc[] = L"Saenaru Hanja Candidate";

#define CAND_WS         (WS_POPUP | WS_BORDER)
#define CAND_WS_EX      (WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE)

// toggled by VK_TAB and kept like as dwCandStyle of the IMM IME
static BOOL s_fCodeStyle = FALSE;

class CCandidateEditSession : public CEditSessionBase
{
public:
    CCandidateEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext, int action, LPCWSTR psz) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
        _action = action;
        _ach[0] = L'\0';
        if (psz != NULL)
            lstrcpynW(_ach, psz, ARRAYSIZE(_ach));
    }
    ~CCandidateEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        return _pSaenaru->_DoCandidateEditSession(ec, _pContext, _action, _ach);
    }

private:
    CSaenaruTextService *_pSaenaru;
    int _action;
    WCHAR _ach[CAND_STRSIZE];
};

//+---------------------------------------------------------------------------
//
// GetDicFileName
//
// Same lookup as ConvHanja(). Use the main dictionary if lpKey is not set.
//----------------------------------------------------------------------------

static void GetDicFileName(LPCTSTR lpKey, LPTSTR lpPath, int cch)
{
    TCHAR szName[MAX_PATH];
    LPTSTR lpDic = lpPath;
    long sz;

    lpDic += GetSaenaruDirectory(lpDic, cch);
    if (*(lpDic - 1) != TEXT('\\'))
        *lpDic++ = TEXT('\\');
    *lpDic = TEXT('\0');

    sz = GetRegStringValue(TEXT("\\Dictionary"), lpKey, NULL);
    if (sz <= 2 && *lpKey != TEXT('\0')) {
        lpKey = TEXT("");
        sz = GetRegStringValue(TEXT("\\Dictionary"), lpKey, NULL);
    }

    if (sz > 2 && sz <= (long)sizeof(szName) &&
            GetRegStringValue(TEXT("\\Dictionary"), lpKey, szName) > 2)
        lstrcpyn(lpDic, szName, cch - (int)(lpDic - lpPath));
    else
        lstrcpyn(lpDic, TEXT("SAENARU.DIC"), cch - (int)(lpDic - lpPath));

    DEBUGPRINTFEX(100, (TEXT("GetDicFileName() %s\n"), lpPath));
}

//+---------------------------------------------------------------------------
//
// GetHangulFromHanja
//
// The hangul reading of a hanja word with the HanjaIndex like as ConvHanja().
//----------------------------------------------------------------------------

static BOOL GetHangulFromHanja(LPCWSTR pszRead, LPWSTR pszHan, int cch)
{
    TCHAR szIdx[MAX_PATH];
    BOOL fHanja = FALSE;
    int i;

    // no default for the HanjaIndex
    if (GetRegStringValue(TEXT("\\Dictionary"), TEXT("HanjaIndex"), NULL) <= 2)
        return FALSE;
    GetDicFileName(TEXT("HanjaIndex"), szIdx, ARRAYSIZE(szIdx));

    for (i = 0; pszRead[i] != L'\0' && i < cch - 1; i++)
    {
        WCHAR c = pszRead[i];

        if (c < 0x3400 || c >= 0xfaff || (c >= 0xa000 && c <= 0xf8ff))
            pszHan[i] = c; // not a hanja
        else if (GetHangulFromHanjaIndex((LPWSTR)&pszRead[i], &pszHan[i], 2, szIdx) < 1)
            return FALSE; // not found in the hanja index
        else
            fHanja = TRUE;
    }
    pszHan[i] = L'\0';
    return fHanja;
}

//+---------------------------------------------------------------------------
//
// GetExtHanjaType
//
// The hanja candidate not in the CP949 like as PaintCandWindow() of the IMM
// IME. 0: not extended, 1: an extended hanja of the KS X 1002, 2: others.
//----------------------------------------------------------------------------

static int GetExtHanjaType(LPCWSTR psz)
{
    WCHAR c = psz[0];
    char mb[8];
    BOOL fUsedDefault = FALSE;
    int len;

    if (!(dwOptionFlag & KSX1002_SUPPORT))
        return 0;

    // one hanja. a surrogate pair is in the extension B or later.
    len = (c >= 0xd800 && c <= 0xdbff) ? 2 : 1;
    if (lstrlenW(psz) != len)
        return 0;
    if (len == 1 && (c < 0x3400 || (c >= 0xa000 && c < 0xf900) || c > 0xfaff))
        return 0;

    WideCharToMultiByte(949, WC_NO_BEST_FIT_CHARS, psz, len, mb, sizeof(mb), NULL, &fUsedDefault);
    if (!fUsedDefault)
        return 0;
    return is_ksx1002(c) ? 1 : 2;
}

//+---------------------------------------------------------------------------
//
// MakeCandidateList
//
// Look up the word, symbol or hanja dictionary like as ConvHanja().
// The first candidate is the reading itself. A hanja word not found is
// converted to the hangul, and *puSelect is the candidate of the hanja.
// pszBuf has CAND_STRSIZE + CAND_BUFSIZE + 2 chars.
//----------------------------------------------------------------------------

static BOOL MakeCandidateList(LPCWSTR pszRead, LPWSTR pszBuf, BOOL *pfSymbol, UINT *puSelect)
{
    TCHAR szDic[MAX_PATH];
    WCHAR achHan[CAND_STRSIZE];
    LPCWSTR psz;
    UINT i;
    int len, nBufLen;

    len = lstrlenW(pszRead);
    *puSelect = 0;

    // a jamo or an ascii char is converted to symbols
    *pfSymbol = (len == 1 &&
            (pszRead[0] < 0x7f || (pszRead[0] >= 0x3131 && pszRead[0] <= 0x314e)));
    GetDicFileName((len > 1) ? TEXT("Word") : (*pfSymbol ? TEXT("Symbol") : TEXT("")),
            szDic, ARRAYSIZE(szDic));

    lstrcpynW(pszBuf, pszRead, CAND_STRSIZE);
    len = lstrlenW(pszBuf);
    pszBuf[len + 1] = L'\0';

    nBufLen = GetCandidateStringsFromDictionary(pszBuf, pszBuf + len + 1, CAND_BUFSIZE, szDic);
    if (nBufLen > 1)
        return TRUE;

    // not found. look up the hangul of the hanja word
    if (nBufLen == 1 || !GetHangulFromHanja(pszBuf, achHan, ARRAYSIZE(achHan)))
        return FALSE;

    // the hangul first. it may have no hanja candidate.
    lstrcpyW(pszBuf, achHan);
    pszBuf[len + 1] = L'\0';
    GetCandidateStringsFromDictionary(achHan, pszBuf + len + 1, CAND_BUFSIZE, szDic);

    // select the candidate of the hanja
    for (i = 1, psz = pszBuf + len + 1; *psz != L'\0'; i++, psz += lstrlenW(psz) + 1)
    {
        if (*psz == pszRead[0])
        {
            *puSelect = i;
            break;
        }
    }
    return TRUE;
}

//+---------------------------------------------------------------------------
//
// ctor/dtor
//
//----------------------------------------------------------------------------

CCandidateList::CCandidateList(CSaenaruTextService *pSaenaru)
{
    // the text service owns this object, so do not AddRef() it.
    _pSaenaru = pSaenaru;
    _pThreadMgr = NULL;
    _pContext = NULL;
    _dwUIElementId = (DWORD)-1;
    _fShown = FALSE;
    _fSymbol = FALSE;
    _fWord = FALSE;
    _fCodeStyle = FALSE;
    _dwUpdatedFlags = 0;

    _achList[0] = L'\0';
    _uCount = 0;
    _uSelection = 0;
    _uPageSize = CAND_PAGESIZE;

    _hWnd = NULL;
    _hWndTip = NULL;
    _ptMouse.x = _ptMouse.y = 0;
    _hFont = NULL;
    _hNumFont = NULL;
    SetRectEmpty(&_rcText);
    _cxNum = 0;
    _cxCol = 0;
    _cyItem = 0;
    _cyStatus = 0;

    _cRef = 1;
}

CCandidateList::~CCandidateList()
{
    _Close();
}

//+---------------------------------------------------------------------------
//
// IUnknown
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfUIElement) ||
        IsEqualIID(riid, IID_ITfCandidateListUIElement) ||
        IsEqualIID(riid, IID_ITfCandidateListUIElementBehavior))
    {
        *ppvObj = (ITfCandidateListUIElementBehavior *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CCandidateList::AddRef()
{
    return ++_cRef;
}

STDAPI_(ULONG) CCandidateList::Release()
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
// ITfUIElement
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::GetDescription(BSTR *pbstrDescription)
{
    if (pbstrDescription == NULL)
        return E_INVALIDARG;

    *pbstrDescription = SysAllocString(c_szCandDesc);
    return (*pbstrDescription != NULL) ? S_OK : E_OUTOFMEMORY;
}

STDAPI CCandidateList::GetGUID(GUID *pguid)
{
    if (pguid == NULL)
        return E_INVALIDARG;

    *pguid = c_guidCandUIElement;
    return S_OK;
}

STDAPI CCandidateList::Show(BOOL bShow)
{
    _fShown = bShow;

    if (bShow)
    {
        if (_IsOpen() && _CreateWindow())
            _Layout();
    }
    else if (_hWnd != NULL)
    {
        ShowWindow(_hWnd, SW_HIDE);
        _UpdateToolTip();
    }
    return S_OK;
}

STDAPI CCandidateList::IsShown(BOOL *pbShow)
{
    if (pbShow == NULL)
        return E_INVALIDARG;

    *pbShow = _fShown;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElement
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::GetUpdatedFlags(DWORD *pdwFlags)
{
    if (pdwFlags == NULL)
        return E_INVALIDARG;

    *pdwFlags = _dwUpdatedFlags;
    return S_OK;
}

STDAPI CCandidateList::GetDocumentMgr(ITfDocumentMgr **ppdim)
{
    if (ppdim == NULL)
        return E_INVALIDARG;

    *ppdim = NULL;
    if (_pContext == NULL)
        return E_FAIL;

    return _pContext->GetDocumentMgr(ppdim);
}

STDAPI CCandidateList::GetCount(UINT *puCount)
{
    if (puCount == NULL)
        return E_INVALIDARG;

    *puCount = _uCount;
    return S_OK;
}

STDAPI CCandidateList::GetSelection(UINT *puIndex)
{
    if (puIndex == NULL)
        return E_INVALIDARG;

    *puIndex = _uSelection;
    return S_OK;
}

STDAPI CCandidateList::GetString(UINT uIndex, BSTR *pstr)
{
    WCHAR ach[CAND_STRSIZE];

    if (pstr == NULL)
        return E_INVALIDARG;

    *pstr = NULL;
    if (uIndex >= _uCount)
        return E_INVALIDARG;

    _GetDisplayString(uIndex, ach, ARRAYSIZE(ach));
    *pstr = SysAllocString(ach);
    return (*pstr != NULL) ? S_OK : E_OUTOFMEMORY;
}

STDAPI CCandidateList::GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt)
{
    UINT i, uPages;

    if (puPageCnt == NULL)
        return E_INVALIDARG;

    uPages = (_uCount + _uPageSize - 1) / _uPageSize;
    *puPageCnt = uPages;

    if (pIndex != NULL)
    {
        for (i = 0; i < uPages && i < uSize; i++)
            pIndex[i] = i * _uPageSize;
    }
    return S_OK;
}

STDAPI CCandidateList::SetPageIndex(UINT *pIndex, UINT uPageCnt)
{
    return E_NOTIMPL;
}

STDAPI CCandidateList::GetCurrentPage(UINT *puPage)
{
    if (puPage == NULL)
        return E_INVALIDARG;

    *puPage = _uSelection / _uPageSize;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfCandidateListUIElementBehavior
//
//----------------------------------------------------------------------------

STDAPI CCandidateList::SetSelection(UINT nIndex)
{
    if (!_IsOpen() || nIndex >= _uCount)
        return E_INVALIDARG;

    _SetSelection(nIndex);
    _pSaenaru->_UpdateCandidatePreview(_pContext, FALSE);
    return S_OK;
}

STDAPI CCandidateList::Finalize()
{
    if (!_IsOpen())
        return E_FAIL;

    _pSaenaru->_CommitCandidate(_pContext, FALSE);
    return S_OK;
}

STDAPI CCandidateList::Abort()
{
    if (!_IsOpen())
        return E_FAIL;

    // commit the hangul like as VK_ESCAPE
    _SetSelection(0);
    _pSaenaru->_CommitCandidate(_pContext, FALSE);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _Open
//
//----------------------------------------------------------------------------

BOOL CCandidateList::_Open(ITfThreadMgr *pThreadMgr, ITfContext *pContext, LPCWSTR pszList, BOOL fSymbol, BOOL fWord)
{
    ITfUIElementMgr *pUIElementMgr;
    LPCWSTR psz;
    UINT off, len;
    BOOL fShow;

    _Close();

    // copy the double null terminated list
    psz = pszList;
    off = 0;
    while (*psz != L'\0' && _uCount < CAND_MAXNUM)
    {
        len = lstrlenW(psz);
        if (off + len + 2 > ARRAYSIZE(_achList))
            break;
        CopyMemory(_achList + off, psz, (len + 1) * sizeof(WCHAR));
        _rgOffset[_uCount++] = off;
        off += len + 1;
        psz += len + 1;
    }
    _achList[off] = L'\0';

    if (_uCount == 0)
        return FALSE;

    _uSelection = 0;
    _uPageSize = (_uCount < CAND_PAGESIZE) ? _uCount : CAND_PAGESIZE;
    _fSymbol = fSymbol;
    _fWord = fWord;
    _fCodeStyle = fSymbol || (!fWord && s_fCodeStyle);

    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();
    _pContext = pContext;
    _pContext->AddRef();

    _dwUpdatedFlags = TF_CLUIE_DOCUMENTMGR | TF_CLUIE_COUNT | TF_CLUIE_SELECTION |
                      TF_CLUIE_STRING | TF_CLUIE_PAGEINDEX | TF_CLUIE_CURRENTPAGE;

    // the application may draw the candidate list itself (UILess mode)
    fShow = TRUE;
    if (_pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
    {
        if (pUIElementMgr->BeginUIElement(this, &fShow, &_dwUIElementId) != S_OK)
        {
            _dwUIElementId = (DWORD)-1;
            fShow = TRUE;
        }
        pUIElementMgr->Release();
    }

    _fShown = fShow;
    if (_fShown)
        _CreateWindow(); // shown by _SetTextRect()

    DEBUGPRINTFEX(100, (TEXT("CCandidateList::_Open() count = %d, show = %d\n"), _uCount, _fShown));
    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _Close
//
//----------------------------------------------------------------------------

void CCandidateList::_Close()
{
    ITfUIElementMgr *pUIElementMgr;

    if (_dwUIElementId != (DWORD)-1 && _pThreadMgr != NULL)
    {
        if (_pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
        {
            pUIElementMgr->EndUIElement(_dwUIElementId);
            pUIElementMgr->Release();
        }
    }
    _dwUIElementId = (DWORD)-1;

    _DestroyWindow();

    _fShown = FALSE;
    _uCount = 0;
    _uSelection = 0;

    SafeReleaseClear(_pContext);
    SafeReleaseClear(_pThreadMgr);
}

//+---------------------------------------------------------------------------
//
// _Move
//
// Same offset rules as ConvHanja(): +-1 item, +-2 page, > 2 end, < -2 home.
//----------------------------------------------------------------------------

void CCandidateList::_Move(int offset)
{
    int sel = (int)_uSelection;
    int count = (int)_uCount;

    if (count == 0)
        return;

    if (offset > 2)
    {
        sel = count - 1;
    }
    else if (offset < -2)
    {
        sel = 0;
    }
    else
    {
        // PgUp/PgDn
        if (offset == 2) offset = _uPageSize;
        else if (offset == -2) offset = -(int)_uPageSize;

        sel += offset;
        if (sel < 0)
            sel = 0;
        else if (offset > 1 && sel >= count)
            sel = count - 1;
        else if (sel >= count)
            sel = 0; // last + 1 goes to the first
    }

    _SetSelection((UINT)sel);
}

void CCandidateList::_SelectInPage(int nth)
{
    UINT uIndex = _GetPageStart() + nth - 1;

    if (uIndex >= _uCount)
        uIndex = _uCount - 1;
    _SetSelection(uIndex);
}

void CCandidateList::_SetSelection(UINT uIndex)
{
    if (uIndex >= _uCount)
        return;

    _uSelection = uIndex;
    _Update(TF_CLUIE_SELECTION | TF_CLUIE_CURRENTPAGE);
}

//+---------------------------------------------------------------------------
//
// _ToggleStyle
//
// VK_TAB toggles the read style (one page) and the code style (all pages).
// Symbols always use the code style, words the read style like as ConvHanja().
//----------------------------------------------------------------------------

void CCandidateList::_ToggleStyle()
{
    s_fCodeStyle = !s_fCodeStyle;
    _fCodeStyle = _fSymbol || (!_fWord && s_fCodeStyle);
    _Update(TF_CLUIE_SELECTION | TF_CLUIE_CURRENTPAGE);
}

//+---------------------------------------------------------------------------
//
// _GetCandidate
//
// The candidate string without its meaning. ("漢:한나라 한" -> "漢")
//----------------------------------------------------------------------------

void CCandidateList::_GetCandidate(UINT uIndex, LPWSTR psz, UINT cch)
{
    LPWSTR lpTemp;

    psz[0] = L'\0';
    if (uIndex >= _uCount)
        return;

    lstrcpynW(psz, _achList + _rgOffset[uIndex], cch);
    if (psz[0] != L'\0' && (lpTemp = wcschr(psz + 1, L':')) != NULL)
        *lpTemp = L'\0';
}

void CCandidateList::_GetDisplayString(UINT uIndex, LPWSTR psz, UINT cch)
{
    LPWSTR lpTemp;

    psz[0] = L'\0';
    if (uIndex >= _uCount)
        return;

    lstrcpynW(psz, _achList + _rgOffset[uIndex], cch);
    if (psz[0] != L'\0' && (lpTemp = wcschr(psz + 1, L':')) != NULL)
        *lpTemp = L' '; // with meaning
}

//+---------------------------------------------------------------------------
//
// _GetMeaning
//
// The meaning, or the code point of a hanja like as the IMM tooltip.
//----------------------------------------------------------------------------

void CCandidateList::_GetMeaning(UINT uIndex, LPWSTR psz, UINT cch)
{
    LPCWSTR lpstr, lpTemp;

    psz[0] = L'\0';
    if (uIndex >= _uCount)
        return;

    lpstr = _achList + _rgOffset[uIndex];
    if (lpstr[0] != L'\0' && (lpTemp = wcschr(lpstr + 1, L':')) != NULL)
        lstrcpynW(psz, lpTemp + 1, cch);
    else if (uIndex != 0 && lpstr[0] >= 0x3400 && cch > 6)
        wsprintfW(psz, L"U+%04X", (UINT)lpstr[0]);
}

void CCandidateList::_SetTextRect(const RECT *prc)
{
    _rcText = *prc;
    _Layout();
}

void CCandidateList::_Update(DWORD dwFlags)
{
    ITfUIElementMgr *pUIElementMgr;

    _dwUpdatedFlags = dwFlags;

    if (_dwUIElementId != (DWORD)-1 && _pThreadMgr != NULL)
    {
        if (_pThreadMgr->QueryInterface(IID_ITfUIElementMgr, (void **)&pUIElementMgr) == S_OK)
        {
            pUIElementMgr->UpdateUIElement(_dwUIElementId);
            pUIElementMgr->Release();
        }
    }

    if (_hWnd != NULL)
    {
        _Layout();
        InvalidateRect(_hWnd, NULL, FALSE);
    }
}

//+---------------------------------------------------------------------------
//
// candidate window
//
//----------------------------------------------------------------------------

BOOL CCandidateList::_CreateWindow()
{
    WNDCLASSEXW wc;
    INITCOMMONCONTROLSEX icc;
    TOOLINFOW ti;
    LOGFONTW lf;
    HDC hdc;
    int dpi;

    if (_hWnd != NULL)
        return TRUE;

    wc.cbSize = sizeof(wc);
    if (!GetClassInfoExW(g_hInst, c_szCandWndClass, &wc))
    {
        ZeroMemory(&wc, sizeof(wc));
        wc.cbSize = sizeof(wc);
        wc.style = CS_IME | CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = _WndProc;
        wc.hInstance = g_hInst;
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = c_szCandWndClass;
        if (!RegisterClassExW(&wc))
            return FALSE;
    }

    _hWnd = CreateWindowExW(CAND_WS_EX, c_szCandWndClass, NULL, CAND_WS,
                            0, 0, 0, 0, NULL, NULL, g_hInst, this);
    if (_hWnd == NULL)
        return FALSE;

    hdc = GetDC(NULL);
    dpi = GetDeviceCaps(hdc, LOGPIXELSY);
    ReleaseDC(NULL, hdc);

    ZeroMemory(&lf, sizeof(lf));
    lf.lfHeight = -MulDiv(12, dpi, 72);
    lf.lfWeight = FW_NORMAL;
    lf.lfCharSet = DEFAULT_CHARSET;
    lstrcpyW(lf.lfFaceName, L"Malgun Gothic");
    _hFont = CreateFontIndirectW(&lf);

    // number font like as the IMM candidate window
    lf.lfHeight = -MulDiv(8, dpi, 72);
    lf.lfWeight = FW_BOLD;
    lstrcpyW(lf.lfFaceName, L"Verdana");
    _hNumFont = CreateFontIndirectW(&lf);

    // the tooltip of the meaning like as the IMM candidate window
    icc.dwSize = sizeof(icc);
    icc.dwICC = ICC_BAR_CLASSES;
    InitCommonControlsEx(&icc);
    _hWndTip = CreateWindowExW(WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE, TOOLTIPS_CLASSW, NULL,
                               WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                               CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
                               _hWnd, NULL, g_hInst, NULL);
    if (_hWndTip != NULL)
    {
        ZeroMemory(&ti, sizeof(ti));
        ti.cbSize = TTTOOLINFOW_V2_SIZE; // comctl32 v5 does not know the full size
        // the mouse goes through the tooltip to the candidates under it
        ti.uFlags = TTF_TRACK | TTF_ABSOLUTE | TTF_TRANSPARENT;
        ti.hwnd = _hWnd;
        ti.lpszText = (LPWSTR)L"";
        SendMessageW(_hWndTip, TTM_ADDTOOLW, 0, (LPARAM)&ti);
        SendMessageW(_hWndTip, WM_SETFONT, (WPARAM)_hFont, FALSE);
    }

    // ignore the mouse until it moves
    GetCursorPos(&_ptMouse);

    return TRUE;
}

void CCandidateList::_DestroyWindow()
{
    if (_hWndTip != NULL)
    {
        DestroyWindow(_hWndTip);
        _hWndTip = NULL;
    }
    if (_hWnd != NULL)
    {
        DestroyWindow(_hWnd);
        _hWnd = NULL;
        // the dll could be unloaded. fails if other windows still use it.
        UnregisterClassW(c_szCandWndClass, g_hInst);
    }
    if (_hFont != NULL)
    {
        DeleteObject(_hFont);
        _hFont = NULL;
    }
    if (_hNumFont != NULL)
    {
        DeleteObject(_hNumFont);
        _hNumFont = NULL;
    }
}

void CCandidateList::_Layout()
{
    HDC hdc;
    HFONT hOldFont;
    TEXTMETRICW tm;
    SIZE sz;
    RECT rc;
    MONITORINFO mi;
    WCHAR ach[CAND_STRSIZE];
    UINT i, uStart, uEnd, uPages;
    int cx, cxStatus, cxWnd, cyWnd, x, y;

    if (_hWnd == NULL || _uCount == 0)
        return;

    uPages = (_uCount + _uPageSize - 1) / _uPageSize;

    hdc = GetDC(_hWnd);
    hOldFont = (HFONT)SelectObject(hdc, _hNumFont);

    GetTextExtentPoint32W(hdc, L"9", 1, &sz);
    _cxNum = sz.cx + 15;
    _cyStatus = sz.cy + 4;
    GetTextExtentPoint32W(hdc, L"300/300", 7, &sz);
    cxStatus = sz.cx + 10;

    SelectObject(hdc, _hFont);
    GetTextMetricsW(hdc, &tm);
    _cyItem = tm.tmHeight + 4; // 4 is line space

    if (_fCodeStyle)
    {
        // all pages in columns. the tooltip shows the meaning.
        cx = 0;
        for (i = 0; i < _uCount; i++)
        {
            _GetCandidate(i, ach, ARRAYSIZE(ach));
            GetTextExtentPoint32W(hdc, ach, lstrlenW(ach), &sz);
            if (sz.cx > cx)
                cx = sz.cx;
        }
        _cxCol = _cxNum + cx + 8;
        cx = _cxCol * (int)uPages;
    }
    else
    {
        uStart = _GetPageStart();
        uEnd = uStart + _uPageSize;
        if (uEnd > _uCount)
            uEnd = _uCount;

        cx = 0;
        for (i = uStart; i < uEnd; i++)
        {
            _GetDisplayString(i, ach, ARRAYSIZE(ach));
            GetTextExtentPoint32W(hdc, ach, lstrlenW(ach), &sz);
            if (sz.cx > cx)
                cx = sz.cx;
        }
        cx += _cxNum + 10;
        _cxCol = cx;
    }
    if (cx < cxStatus)
        cx = cxStatus;

    SelectObject(hdc, hOldFont);
    ReleaseDC(_hWnd, hdc);

    SetRect(&rc, 0, 0, cx, _cyItem * (int)_uPageSize + _cyStatus);
    AdjustWindowRectEx(&rc, CAND_WS, FALSE, CAND_WS_EX);
    cxWnd = rc.right - rc.left;
    cyWnd = rc.bottom - rc.top;

    // under the composition, or above it near the bottom of the screen
    x = _rcText.left;
    y = _rcText.bottom + 2;
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(MonitorFromRect(&_rcText, MONITOR_DEFAULTTONEAREST), &mi))
    {
        if (y + cyWnd > mi.rcWork.bottom)
            y = _rcText.top - cyWnd - 2;
        if (x + cxWnd > mi.rcWork.right)
            x = mi.rcWork.right - cxWnd;
        if (x < mi.rcWork.left)
            x = mi.rcWork.left;
        if (y < mi.rcWork.top)
            y = mi.rcWork.top;
    }

    SetWindowPos(_hWnd, HWND_TOPMOST, x, y, cxWnd, cyWnd,
                 SWP_NOACTIVATE | (_fShown ? SWP_SHOWWINDOW : 0));
    _UpdateToolTip();
}

//+---------------------------------------------------------------------------
//
// _UpdateToolTip
//
// Show the meaning under the selected candidate in the code style like as
// the IMM IME. The read style shows the meanings in the list.
//----------------------------------------------------------------------------

void CCandidateList::_UpdateToolTip()
{
    TOOLINFOW ti;
    WCHAR ach[CAND_STRSIZE];
    RECT rcWnd;
    MONITORINFO mi;
    POINT pt;
    LRESULT lSize;
    int cx, cy;

    if (_hWndTip == NULL)
        return;

    ZeroMemory(&ti, sizeof(ti));
    ti.cbSize = TTTOOLINFOW_V2_SIZE;
    ti.hwnd = _hWnd;

    ach[0] = L'\0';
    if (_fShown && _fCodeStyle && _uCount > 0 && IsWindowVisible(_hWnd))
        _GetMeaning(_uSelection, ach, ARRAYSIZE(ach));
    if (ach[0] == L'\0')
    {
        SendMessageW(_hWndTip, TTM_TRACKACTIVATE, FALSE, (LPARAM)&ti);
        return;
    }

    ti.lpszText = ach;
    SendMessageW(_hWndTip, TTM_UPDATETIPTEXTW, 0, (LPARAM)&ti);

    // under the text of the selected candidate
    pt.x = (int)(_uSelection / _uPageSize) * _cxCol + _cxNum + 4;
    pt.y = (int)(_uSelection % _uPageSize + 1) * _cyItem;
    ClientToScreen(_hWnd, &pt);

    lSize = SendMessageW(_hWndTip, TTM_GETBUBBLESIZE, 0, (LPARAM)&ti);
    cx = LOWORD(lSize);
    cy = HIWORD(lSize);
    GetWindowRect(_hWnd, &rcWnd);
    mi.cbSize = sizeof(mi);
    if (GetMonitorInfoW(MonitorFromRect(&rcWnd, MONITOR_DEFAULTTONEAREST), &mi))
    {
        if (pt.x + cx > mi.rcWork.right)
            pt.x = mi.rcWork.right - cx;
        if (pt.x < mi.rcWork.left)
            pt.x = mi.rcWork.left;
        if (pt.y + cy > mi.rcWork.bottom)
            pt.y -= _cyItem + cy; // above the candidate
        if (pt.y < mi.rcWork.top)
            pt.y = mi.rcWork.top;
    }

    SendMessageW(_hWndTip, TTM_TRACKPOSITION, 0, MAKELPARAM(pt.x, pt.y));
    SendMessageW(_hWndTip, TTM_TRACKACTIVATE, TRUE, (LPARAM)&ti);
}

void CCandidateList::_OnPaint(HWND hWnd)
{
    PAINTSTRUCT ps;
    HDC hdc, hMemDC;
    HBITMAP hBit, hOldBit;
    HFONT hOldFont;
    RECT rc, rcItem;
    SIZE sz;
    WCHAR ach[CAND_STRSIZE];
    WCHAR szNum[16];
    WCHAR achCand[CAND_STRSIZE];
    UINT i, uStart, uEnd, uPages;
    int x, y, xNum, cxCell, len, wtype;

    hdc = BeginPaint(hWnd, &ps);
    GetClientRect(hWnd, &rc);

    // double buffering
    hMemDC = CreateCompatibleDC(hdc);
    hBit = CreateCompatibleBitmap(hdc, rc.right, rc.bottom);
    hOldBit = (HBITMAP)SelectObject(hMemDC, hBit);
    hOldFont = (HFONT)GetCurrentObject(hMemDC, OBJ_FONT);

    // the code style shows all pages, the read style shows the current page.
    if (_fCodeStyle)
    {
        uStart = 0;
        uEnd = _uCount;
        xNum = (int)(_uSelection / _uPageSize) * _cxCol;
        cxCell = _cxCol;
    }
    else
    {
        uStart = _GetPageStart();
        uEnd = uStart + _uPageSize;
        if (uEnd > _uCount)
            uEnd = _uCount;
        xNum = 0;
        cxCell = rc.right;
    }

    // number area (of the current page) and page area use the button face color
    FillRect(hMemDC, &rc, GetSysColorBrush(COLOR_BTNFACE));
    SetRect(&rcItem, 0, 0, rc.right, rc.bottom - _cyStatus);
    FillRect(hMemDC, &rcItem, GetSysColorBrush(COLOR_WINDOW));
    SetRect(&rcItem, xNum, 0, xNum + _cxNum, rc.bottom - _cyStatus);
    FillRect(hMemDC, &rcItem, GetSysColorBrush(COLOR_BTNFACE));

    SetBkMode(hMemDC, TRANSPARENT);

    for (i = uStart; i < uEnd; i++)
    {
        x = _fCodeStyle ? (int)(i / _uPageSize) * _cxCol : 0;
        y = (int)(i % _uPageSize) * _cyItem;

        if (i == _uSelection)
        {
            SetRect(&rcItem, x, y, x + cxCell, y + _cyItem);
            FillRect(hMemDC, &rcItem, GetSysColorBrush(COLOR_HIGHLIGHT));
            SetTextColor(hMemDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
        }
        else
        {
            SetTextColor(hMemDC, GetSysColor(COLOR_WINDOWTEXT));
        }

        // numbers only for the current page
        if (x == xNum)
        {
            len = wsprintfW(szNum, L"%d", (int)(i % _uPageSize + 1));
            SelectObject(hMemDC, _hNumFont);
            GetTextExtentPoint32W(hMemDC, szNum, len, &sz);
            TextOutW(hMemDC, x + 5, y + (_cyItem - sz.cy) / 2, szNum, len);
        }

        if (_fCodeStyle)
            _GetCandidate(i, ach, ARRAYSIZE(ach));
        else
            _GetDisplayString(i, ach, ARRAYSIZE(ach));
        SelectObject(hMemDC, _hFont);

        // extended hanja with a shadow and colors like as the IMM IME
        _GetCandidate(i, achCand, ARRAYSIZE(achCand));
        wtype = GetExtHanjaType(achCand);
        if (wtype)
        {
            SetTextColor(hMemDC, (i == _uSelection) ? RGB(0, 0, 0) : RGB(220, 220, 220));
            TextOutW(hMemDC, x + _cxNum + 5, y + 3, ach, lstrlenW(ach));
            if (i == _uSelection)
                SetTextColor(hMemDC, GetSysColor(COLOR_HIGHLIGHTTEXT));
            else
                SetTextColor(hMemDC, (wtype == 1) ? RGB(16, 83, 239) : RGB(30, 150, 30));
        }
        TextOutW(hMemDC, x + _cxNum + 4, y + 2, ach, lstrlenW(ach));
    }

    // current page / total pages
    uPages = (_uCount + _uPageSize - 1) / _uPageSize;
    len = wsprintfW(szNum, L"%d/%d", (int)(_uSelection / _uPageSize + 1), (int)uPages);
    SelectObject(hMemDC, _hNumFont);
    SetTextColor(hMemDC, GetSysColor(COLOR_BTNTEXT));
    GetTextExtentPoint32W(hMemDC, szNum, len, &sz);
    TextOutW(hMemDC, 5, rc.bottom - _cyStatus + (_cyStatus - sz.cy) / 2, szNum, len);

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldFont);
    SelectObject(hMemDC, hOldBit);
    DeleteObject(hBit);
    DeleteDC(hMemDC);

    EndPaint(hWnd, &ps);
}

BOOL CCandidateList::_HitTest(int x, int y, UINT *puIndex)
{
    UINT uIndex, uRow;

    if (!_IsOpen() || _cyItem <= 0 || _cxCol <= 0 || x < 0 || y < 0)
        return FALSE;

    uRow = (UINT)(y / _cyItem);
    if (uRow >= _uPageSize)
        return FALSE;

    if (_fCodeStyle)
        uIndex = (UINT)(x / _cxCol) * _uPageSize + uRow;
    else
        uIndex = _GetPageStart() + uRow;
    if (uIndex >= _uCount)
        return FALSE;

    *puIndex = uIndex;
    return TRUE;
}

void CCandidateList::_OnClick(int x, int y)
{
    POINT pt;
    RECT rc;
    UINT uIndex;

    // a click on the tooltip is for the selected candidate, not the one under it
    pt.x = x;
    pt.y = y;
    ClientToScreen(_hWnd, &pt);
    if (_hWndTip != NULL && IsWindowVisible(_hWndTip) &&
            GetWindowRect(_hWndTip, &rc) && PtInRect(&rc, pt))
        uIndex = _uSelection;
    else if (!_HitTest(x, y, &uIndex))
        return;

    _SetSelection(uIndex);
    _pSaenaru->_CommitCandidate(_pContext, FALSE);
}

//+---------------------------------------------------------------------------
//
// _OnMouseMove
//
// Select the candidate under the mouse like as the IMM candidate window.
//----------------------------------------------------------------------------

void CCandidateList::_OnMouseMove(int x, int y)
{
    POINT pt;
    UINT uIndex;

    // ignore the fake move when the window is shown or moved under the mouse
    pt.x = x;
    pt.y = y;
    ClientToScreen(_hWnd, &pt);
    if (pt.x == _ptMouse.x && pt.y == _ptMouse.y)
        return;
    _ptMouse = pt;

    if (_HitTest(x, y, &uIndex) && uIndex != _uSelection)
        SetSelection(uIndex);
}

void CCandidateList::_OnMouseWheel(int delta)
{
    if (!_IsOpen())
        return;

    _Move((delta > 0) ? -1 : 1);
    _pSaenaru->_UpdateCandidatePreview(_pContext, FALSE);
}

/* static */
LRESULT CALLBACK CCandidateList::_WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    CCandidateList *_this;

    if (uMsg == WM_NCCREATE)
    {
        _this = (CCandidateList *)((LPCREATESTRUCT)lParam)->lpCreateParams;
        SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)_this);
    }

    _this = (CCandidateList *)GetWindowLongPtr(hWnd, GWLP_USERDATA);

    switch (uMsg)
    {
        case WM_PAINT:
            if (_this != NULL)
            {
                _this->_OnPaint(hWnd);
                return 0;
            }
            break;

        case WM_ERASEBKGND:
            return 1;

        case WM_MOUSEACTIVATE:
            return MA_NOACTIVATE;

        case WM_LBUTTONDOWN:
            if (_this != NULL)
                _this->_OnClick((short)LOWORD(lParam), (short)HIWORD(lParam));
            return 0;

        case WM_MOUSEMOVE:
            if (_this != NULL)
                _this->_OnMouseMove((short)LOWORD(lParam), (short)HIWORD(lParam));
            return 0;

        case WM_MOUSEWHEEL:
            if (_this != NULL)
                _this->_OnMouseWheel(GET_WHEEL_DELTA_WPARAM(wParam));
            return 0;
    }

    return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

//+---------------------------------------------------------------------------
//
// CSaenaruTextService candidate methods
//
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_IsCandidateOpen()
{
    return _pCandidateList != NULL && _pCandidateList->_IsOpen();
}

void CSaenaruTextService::_CloseCandidateList()
{
    if (_pCandidateList != NULL)
        _pCandidateList->_Close();
}

//+---------------------------------------------------------------------------
//
// _OpenCandidateList
//
// VK_HANJA handling. Look up the composing hangul like as ConvHanja().
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_OpenCandidateList(ITfContext *pContext)
{
    WCHAR achBuf[CAND_STRSIZE + CAND_BUFSIZE + 2];
    WCHAR achRead[2];
    BOOL fSymbol;
    UINT uSelect;

    if (!_IsComposing())
        return FALSE;

    achRead[0] = _HangulPreedit();
    achRead[1] = L'\0';
    if (achRead[0] == 0)
        return FALSE;

    if (!MakeCandidateList(achRead, achBuf, &fSymbol, &uSelect))
        return FALSE; // not found in the dictionary

    if (_pCandidateList == NULL && (_pCandidateList = new CCandidateList(this)) == NULL)
        return FALSE;

    if (!_pCandidateList->_Open(_pThreadMgr, pContext, achBuf, fSymbol, FALSE))
        return FALSE;
    if (uSelect)
        _pCandidateList->_SetSelection(uSelect);

    // place the candidate window
    _RequestCandidateEditSession(pContext, CAND_ES_LOCATE, NULL, TRUE);
    return TRUE;
}

HRESULT CSaenaruTextService::_RequestCandidateEditSession(ITfContext *pContext, int action, LPCWSTR psz, BOOL fSync)
{
    CCandidateEditSession *pEditSession;
    HRESULT hr = E_FAIL;

    if ((pEditSession = new CCandidateEditSession(this, pContext, action, psz)) != NULL)
    {
        if (!fSync ||
                pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr) != S_OK ||
                hr == TF_E_SYNCHRONOUS)
        {
            // a sync edit session is not granted everywhere. (OnPreservedKey etc.)
            pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        }
        DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_RequestCandidateEditSession(%d) hr = 0x%x\n"), action, hr));
        pEditSession->Release();
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// _UpdateCandidatePreview
//
// Show the selected candidate in the composition like as the IMM IME.
//----------------------------------------------------------------------------

void CSaenaruTextService::_UpdateCandidatePreview(ITfContext *pContext, BOOL fSync)
{
    WCHAR ach[CAND_STRSIZE];

    if (!_IsCandidateOpen())
        return;

    _pCandidateList->_GetCandidate(_pCandidateList->_GetSelection(), ach, ARRAYSIZE(ach));
    _RequestCandidateEditSession(pContext, CAND_ES_PREVIEW, ach, fSync);
}

//+---------------------------------------------------------------------------
//
// _CommitCandidate
//
// Commit the selected candidate and close the list.
// pContext == NULL means the context of the candidate list.
//----------------------------------------------------------------------------

void CSaenaruTextService::_CommitCandidate(ITfContext *pContext, BOOL fSync)
{
    WCHAR ach[CAND_STRSIZE];

    if (!_IsCandidateOpen())
        return;

    if (pContext == NULL)
        pContext = _pCandidateList->_GetContext();

    _pCandidateList->_GetCandidate(_pCandidateList->_GetSelection(), ach, ARRAYSIZE(ach));
    _RequestCandidateEditSession(pContext, CAND_ES_COMMIT, ach, fSync);

    // the edit session closes it too
    _CloseCandidateList();
    hangul_ic_init(&ic);
}

//+---------------------------------------------------------------------------
//
// _CancelCandidateList
//
// Restore the hangul and close the list. (VK_BACK)
//----------------------------------------------------------------------------

void CSaenaruTextService::_CancelCandidateList(ITfContext *pContext)
{
    if (!_IsCandidateOpen())
        return;

    _pCandidateList->_SetSelection(0);
    _UpdateCandidatePreview(pContext);
    _CloseCandidateList();
    _fCandidateReopen = TRUE;
}

//+---------------------------------------------------------------------------
//
// _HandleCandidateKey
//
// Candidate keys like as DicKeydownHandler().
// Returns TRUE if the key is handled by the candidate list.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_HandleCandidateKey(ITfContext *pContext, WPARAM wParam, BOOL fTest)
{
    int next = 0, select = 0;
    BOOL fCommit = FALSE, fChanged = FALSE;
    WCHAR ch, hkey;

    if (!_IsCandidateOpen())
        return FALSE;

    switch (wParam)
    {
        case VK_UP:
            next = -1;
            break;
        case VK_DOWN:
        case VK_HANJA:
            next = 1;
            break;
        case VK_PRIOR:
            next = -2;
            break;
        case VK_NEXT:
            next = 2;
            break;
        case VK_HOME:
            next = -100;
            break;
        case VK_END:
            next = 100;
            break;
        case VK_LEFT:
            if (_pCandidateList->_IsCodeStyle())
                next = -2;
            break;
        case VK_RIGHT:
            if (_pCandidateList->_IsCodeStyle())
                next = 2;
            break;
        case VK_SPACE:
            if (dwOptionFlag & HANJA_CAND_WITH_SPACE)
                next = 1;
            break;
        case VK_ESCAPE:
            select = -999; // commit the hangul
            break;
        case VK_RETURN:
            fCommit = TRUE;
            break;
        case VK_TAB:
            fChanged = TRUE; // toggle the candidate style
            break;
        case VK_NUMPAD1:
        case VK_NUMPAD2:
        case VK_NUMPAD3:
        case VK_NUMPAD4:
        case VK_NUMPAD5:
        case VK_NUMPAD6:
        case VK_NUMPAD7:
        case VK_NUMPAD8:
        case VK_NUMPAD9:
            select = (int)(wParam - VK_NUMPAD1) + 1;
            break;
        default:
            // hanja selection key. hangul jamo keys are not selection keys.
            ch = _ConvertVKey((UINT)wParam);
            hkey = (GetConversionStatus() & CMODE_HANGUL) ? keyToHangulKey(ch) : ch;
            if (hkey >= L'1' && hkey <= L'9')
                select = hkey - L'0';
            break;
    }

    if (!next && !select && !fCommit && !fChanged)
        return FALSE;

    if (fTest)
        return TRUE;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleCandidateKey() next = %d, select = %d\n"), next, select));

    if (fChanged)
    {
        _pCandidateList->_ToggleStyle();
        return TRUE;
    }

    if (next)
    {
        _pCandidateList->_Move(next);
        _UpdateCandidatePreview(pContext);
        return TRUE;
    }

    if (select == -999)
        _pCandidateList->_SetSelection(0);
    else if (select)
        _pCandidateList->_SelectInPage(select);

    _CommitCandidate(pContext);
    return TRUE;
}

//+---------------------------------------------------------------------------
//
// _DoCandidateEditSession
//
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_DoCandidateEditSession(TfEditCookie ec, ITfContext *pContext, int action, LPCWSTR psz)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    LONG cch;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoCandidateEditSession(%d)\n"), action));

    if (action == CAND_ES_RECONVERT)
        return _DoReconvert(ec, pContext);

    if (_pComposition == NULL)
        return S_OK; // already terminated

    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return E_FAIL;

    switch (action)
    {
        case CAND_ES_LOCATE:
            _LocateCandidateList(ec, pContext, pRangeComposition);
            break;

        case CAND_ES_PREVIEW:
        case CAND_ES_COMMIT:
            cch = lstrlenW(psz);
            if (cch == 0 || pRangeComposition->SetText(ec, (action == CAND_ES_PREVIEW) ? TF_ST_CORRECTION : 0, psz, cch) != S_OK)
                break;

            tfSelection.range = pRangeComposition;
            tfSelection.style.ase = TF_AE_NONE;
            tfSelection.style.fInterimChar = (action == CAND_ES_PREVIEW && cch == 1);
            // the caret at the end for a word not to select it
            if (action == CAND_ES_COMMIT || cch > 1)
                pRangeComposition->Collapse(ec, TF_ANCHOR_END);
            pContext->SetSelection(ec, 1, &tfSelection);

            if (action == CAND_ES_PREVIEW)
                _SetCompositionDisplayAttributes(ec);
            else
                _TerminateComposition(ec);
            break;
    }

    pRangeComposition->Release();
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _LocateCandidateList
//
// Place the candidate window under the range.
//----------------------------------------------------------------------------

void CSaenaruTextService::_LocateCandidateList(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange)
{
    ITfContextView *pContextView;
    GUITHREADINFO gti;
    RECT rc;
    BOOL fClipped, fFound;

    fFound = FALSE;
    if (pContext->GetActiveView(&pContextView) == S_OK)
    {
        if (pContextView->GetTextExt(ec, pRange, &rc, &fClipped) == S_OK &&
                (rc.right != 0 || rc.bottom != 0))
            fFound = TRUE;
        pContextView->Release();
    }
    if (!fFound)
    {
        // no layout. use the caret position
        gti.cbSize = sizeof(gti);
        if (GetGUIThreadInfo(0, &gti) && gti.hwndCaret != NULL)
        {
            rc = gti.rcCaret;
            MapWindowPoints(gti.hwndCaret, NULL, (LPPOINT)&rc, 2);
        }
        else
        {
            POINT pt;
            GetCursorPos(&pt);
            SetRect(&rc, pt.x, pt.y, pt.x, pt.y);
        }
    }
    if (_IsCandidateOpen())
        _pCandidateList->_SetTextRect(&rc);
}

//+---------------------------------------------------------------------------
//
// _DoReconvert
//
// VK_HANJA without the composition. Convert the first word of the selection,
// or the word after the caret like as the reconversion of the IMM IME.
// Returns S_OK if the candidate list is opened.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_DoReconvert(TfEditCookie ec, ITfContext *pContext)
{
    static const WCHAR c_szSep[] = L" \r\n\t";
    WCHAR achBuf[CAND_STRSIZE + CAND_BUFSIZE + 2];
    WCHAR ach[CAND_STRSIZE];
    TF_SELECTION tfSelection;
    ITfContextComposition *pContextComposition;
    ITfComposition *pComposition;
    ITfRange *pRange;
    ULONG cFetched, cch;
    LONG cchShift;
    BOOL fEmpty, fSymbol;
    UINT uSelect;
    int start, len;
    HRESULT hr = S_FALSE;

    if (_IsComposing())
        return S_FALSE;

    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;
    pRange = tfSelection.range;

    // no selection. the word after the caret
    if (pRange->IsEmpty(ec, &fEmpty) == S_OK && fEmpty)
        pRange->ShiftEnd(ec, ARRAYSIZE(ach) - 1, &cchShift, NULL);

    if (pRange->GetText(ec, 0, ach, ARRAYSIZE(ach) - 1, &cch) != S_OK)
        goto Exit;
    ach[cch] = L'\0';

    // the first word like as strtok()
    start = (int)wcsspn(ach, c_szSep);
    len = (int)wcscspn(ach + start, c_szSep);
    if (len == 0)
        goto Exit;
    ach[start + len] = L'\0';

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoReconvert() %s\n"), ach + start));
    if (!MakeCandidateList(ach + start, achBuf, &fSymbol, &uSelect))
        goto Exit; // not found in the dictionary

    // the range of the word
    pRange->Collapse(ec, TF_ANCHOR_START);
    pRange->ShiftEnd(ec, start + len, &cchShift, NULL);
    pRange->ShiftStart(ec, start, &cchShift, NULL);

    if (pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
        goto Exit;

    pComposition = NULL;
    if (pContextComposition->StartComposition(ec, pRange, this, &pComposition) == S_OK && pComposition != NULL)
    {
        _SetComposition(pComposition);
        hangul_ic_init(&ic);
        _SetCompositionDisplayAttributes(ec);

        if ((_pCandidateList != NULL || (_pCandidateList = new CCandidateList(this)) != NULL) &&
                _pCandidateList->_Open(_pThreadMgr, pContext, achBuf, fSymbol, len > 1))
        {
            if (uSelect)
                _pCandidateList->_SetSelection(uSelect);
            _LocateCandidateList(ec, pContext, pRange);
            hr = S_OK;
        }
        else
        {
            _TerminateComposition(ec);
        }

        // the caret at the end of the word
        pRange->Collapse(ec, TF_ANCHOR_END);
        tfSelection.style.ase = TF_AE_NONE;
        tfSelection.style.fInterimChar = FALSE;
        pContext->SetSelection(ec, 1, &tfSelection);
    }
    pContextComposition->Release();

Exit:
    pRange->Release();
    return hr;
}

/*
 * vim: et sts=4 sw=4
 */
