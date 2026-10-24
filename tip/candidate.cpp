//
// candidate.cpp
//
// Hanja candidate list. ITfCandidateListUIElement implementation.
// It follows ConvHanja() and DicKeydownHandler() of the IMM IME.
//

#include "globals.h"
#include "saenarutip.h"
#include "editsess.h"
#include "candidate.h"

extern "C" {
#include "hangul.h"
int GetCandidateStringsFromDictionary(LPWSTR lpRead, LPWSTR lpBuf, DWORD dwBufLen, LPTSTR lpFilename);
int PASCAL GetSaenaruDirectory(LPTSTR lpDest, int max);
long PASCAL GetRegStringValue (LPCTSTR,LPCTSTR,LPTSTR);
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
    _fCodeStyle = FALSE;
    _dwUpdatedFlags = 0;

    _achList[0] = L'\0';
    _uCount = 0;
    _uSelection = 0;
    _uPageSize = CAND_PAGESIZE;

    _hWnd = NULL;
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

BOOL CCandidateList::_Open(ITfThreadMgr *pThreadMgr, ITfContext *pContext, LPCWSTR pszList, BOOL fSymbol)
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
    _fCodeStyle = fSymbol || s_fCodeStyle;

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
// Symbols always use the code style like as ConvHanja().
//----------------------------------------------------------------------------

void CCandidateList::_ToggleStyle()
{
    s_fCodeStyle = !s_fCodeStyle;
    _fCodeStyle = _fSymbol || s_fCodeStyle;
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

    return TRUE;
}

void CCandidateList::_DestroyWindow()
{
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
    int cx, cxStatus, cxMeaning, cxWnd, cyWnd, x, y;

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
        // all pages in columns. the status line shows the meaning.
        cx = 0;
        cxMeaning = 0;
        for (i = 0; i < _uCount; i++)
        {
            _GetCandidate(i, ach, ARRAYSIZE(ach));
            GetTextExtentPoint32W(hdc, ach, lstrlenW(ach), &sz);
            if (sz.cx > cx)
                cx = sz.cx;
            // the longest meaning, not to resize the window on every move
            _GetMeaning(i, ach, ARRAYSIZE(ach));
            GetTextExtentPoint32W(hdc, ach, lstrlenW(ach), &sz);
            if (sz.cx > cxMeaning)
                cxMeaning = sz.cx;
        }
        _cxCol = _cxNum + cx + 8;
        cx = _cxCol * (int)uPages;
        cxStatus += cxMeaning + 10;
        if (_cyStatus < _cyItem)
            _cyStatus = _cyItem;
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
    UINT i, uStart, uEnd, uPages;
    int x, y, xNum, cxCell, len;

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
        TextOutW(hMemDC, x + _cxNum + 4, y + 2, ach, lstrlenW(ach));
    }

    // current page / total pages
    uPages = (_uCount + _uPageSize - 1) / _uPageSize;
    len = wsprintfW(szNum, L"%d/%d", (int)(_uSelection / _uPageSize + 1), (int)uPages);
    SelectObject(hMemDC, _hNumFont);
    SetTextColor(hMemDC, GetSysColor(COLOR_BTNTEXT));
    GetTextExtentPoint32W(hMemDC, szNum, len, &sz);
    TextOutW(hMemDC, 5, rc.bottom - _cyStatus + (_cyStatus - sz.cy) / 2, szNum, len);

    // the meaning of the selected candidate. (a tooltip in the IMM IME)
    if (_fCodeStyle)
    {
        x = 5 + sz.cx + 10;
        _GetMeaning(_uSelection, ach, ARRAYSIZE(ach));
        SelectObject(hMemDC, _hFont);
        TextOutW(hMemDC, x, rc.bottom - _cyStatus + 2, ach, lstrlenW(ach));
    }

    BitBlt(hdc, 0, 0, rc.right, rc.bottom, hMemDC, 0, 0, SRCCOPY);

    SelectObject(hMemDC, hOldFont);
    SelectObject(hMemDC, hOldBit);
    DeleteObject(hBit);
    DeleteDC(hMemDC);

    EndPaint(hWnd, &ps);
}

void CCandidateList::_OnClick(int x, int y)
{
    UINT uIndex, uRow;

    if (!_IsOpen() || _cyItem <= 0 || _cxCol <= 0 || x < 0 || y < 0)
        return;

    uRow = (UINT)(y / _cyItem);
    if (uRow >= _uPageSize)
        return;

    if (_fCodeStyle)
        uIndex = (UINT)(x / _cxCol) * _uPageSize + uRow;
    else
        uIndex = _GetPageStart() + uRow;
    if (uIndex >= _uCount)
        return;

    _SetSelection(uIndex);
    _pSaenaru->_CommitCandidate(_pContext, FALSE);
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
    WCHAR achBuf[CAND_BUFSIZE + 2];
    WCHAR achRead[2];
    TCHAR szDic[MAX_PATH];
    BOOL fSymbol;
    int nBufLen;

    if (!_IsComposing())
        return FALSE;

    achRead[0] = _HangulPreedit();
    achRead[1] = L'\0';
    if (achRead[0] == 0)
        return FALSE;

    // a jamo is converted to symbols
    fSymbol = (achRead[0] >= 0x3131 && achRead[0] <= 0x314e);
    GetDicFileName(fSymbol ? TEXT("Symbol") : TEXT(""), szDic, ARRAYSIZE(szDic));

    // the first candidate is the hangul itself
    achBuf[0] = achRead[0];
    achBuf[1] = L'\0';
    achBuf[2] = L'\0';

    nBufLen = GetCandidateStringsFromDictionary(achRead, achBuf + 2, CAND_BUFSIZE, szDic);
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_OpenCandidateList() nBufLen = %d\n"), nBufLen));
    if (nBufLen <= 1)
        return FALSE; // not found in the dictionary

    if (_pCandidateList == NULL && (_pCandidateList = new CCandidateList(this)) == NULL)
        return FALSE;

    if (!_pCandidateList->_Open(_pThreadMgr, pContext, achBuf, fSymbol))
        return FALSE;

    // place the candidate window
    _RequestCandidateEditSession(pContext, CAND_ES_LOCATE, NULL, TRUE);
    return TRUE;
}

void CSaenaruTextService::_RequestCandidateEditSession(ITfContext *pContext, int action, LPCWSTR psz, BOOL fSync)
{
    CCandidateEditSession *pEditSession;
    HRESULT hr;

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
    ITfContextView *pContextView;
    TF_SELECTION tfSelection;
    GUITHREADINFO gti;
    RECT rc;
    BOOL fClipped, fFound;
    LONG cch;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoCandidateEditSession(%d)\n"), action));

    if (_pComposition == NULL)
        return S_OK; // already terminated

    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return E_FAIL;

    switch (action)
    {
        case CAND_ES_LOCATE:
            fFound = FALSE;
            if (pContext->GetActiveView(&pContextView) == S_OK)
            {
                if (pContextView->GetTextExt(ec, pRangeComposition, &rc, &fClipped) == S_OK &&
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
            break;

        case CAND_ES_PREVIEW:
        case CAND_ES_COMMIT:
            cch = lstrlenW(psz);
            if (cch == 0 || pRangeComposition->SetText(ec, (action == CAND_ES_PREVIEW) ? TF_ST_CORRECTION : 0, psz, cch) != S_OK)
                break;

            tfSelection.range = pRangeComposition;
            tfSelection.style.ase = TF_AE_NONE;
            tfSelection.style.fInterimChar = (action == CAND_ES_PREVIEW && cch == 1);
            if (action == CAND_ES_COMMIT)
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

/*
 * vim: et sts=4 sw=4
 */
