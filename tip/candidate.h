//
// candidate.h
//
// CCandidateList declaration.
//

#ifndef CANDIDATE_H
#define CANDIDATE_H

// same limits as the IMM IME (MAXCANDPAGESIZE, MAXCANDSTRNUM)
#define CAND_PAGESIZE   9
#define CAND_MAXNUM     300
#define CAND_BUFSIZE    1024
#define CAND_STRSIZE    128

class CSaenaruTextService;

class CCandidateList : public ITfCandidateListUIElementBehavior
{
public:
    CCandidateList(CSaenaruTextService *pSaenaru);
    ~CCandidateList();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfUIElement
    STDMETHODIMP GetDescription(BSTR *pbstrDescription);
    STDMETHODIMP GetGUID(GUID *pguid);
    STDMETHODIMP Show(BOOL bShow);
    STDMETHODIMP IsShown(BOOL *pbShow);

    // ITfCandidateListUIElement
    STDMETHODIMP GetUpdatedFlags(DWORD *pdwFlags);
    STDMETHODIMP GetDocumentMgr(ITfDocumentMgr **ppdim);
    STDMETHODIMP GetCount(UINT *puCount);
    STDMETHODIMP GetSelection(UINT *puIndex);
    STDMETHODIMP GetString(UINT uIndex, BSTR *pstr);
    STDMETHODIMP GetPageIndex(UINT *pIndex, UINT uSize, UINT *puPageCnt);
    STDMETHODIMP SetPageIndex(UINT *pIndex, UINT uPageCnt);
    STDMETHODIMP GetCurrentPage(UINT *puPage);

    // ITfCandidateListUIElementBehavior
    STDMETHODIMP SetSelection(UINT nIndex);
    STDMETHODIMP Finalize();
    STDMETHODIMP Abort();

    // pszList is a double null terminated list. the first item is the hangul.
    BOOL _Open(ITfThreadMgr *pThreadMgr, ITfContext *pContext, LPCWSTR pszList, BOOL fCodeStyle);
    void _Close();
    BOOL _IsOpen()
    {
        return _pContext != NULL;
    }
    BOOL _IsCodeStyle()
    {
        return _fCodeStyle;
    }
    ITfContext *_GetContext()
    {
        return _pContext;
    }
    UINT _GetSelection()
    {
        return _uSelection;
    }
    void _Move(int offset);
    void _SelectInPage(int nth);
    void _SetSelection(UINT uIndex);
    void _GetCandidate(UINT uIndex, LPWSTR psz, UINT cch);
    void _SetTextRect(const RECT *prc);

private:
    void _GetDisplayString(UINT uIndex, LPWSTR psz, UINT cch);
    UINT _GetPageStart()
    {
        return (_uSelection / _uPageSize) * _uPageSize;
    }
    void _Update(DWORD dwFlags);
    BOOL _CreateWindow();
    void _DestroyWindow();
    void _Layout();
    void _OnPaint(HWND hWnd);
    void _OnClick(int y);
    static LRESULT CALLBACK _WndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

    CSaenaruTextService *_pSaenaru;
    ITfThreadMgr *_pThreadMgr;
    ITfContext *_pContext;
    DWORD _dwUIElementId;
    BOOL _fShown;
    BOOL _fCodeStyle;
    DWORD _dwUpdatedFlags;

    WCHAR _achList[CAND_BUFSIZE + 2];
    UINT _rgOffset[CAND_MAXNUM];
    UINT _uCount;
    UINT _uSelection;
    UINT _uPageSize;

    HWND _hWnd;
    HFONT _hFont;
    HFONT _hNumFont;
    RECT _rcText;
    int _cxNum;
    int _cyItem;
    int _cyStatus;

    LONG _cRef;     // COM ref count
};

#endif // CANDIDATE_H

/*
 * vim: et sts=4 sw=4
 */
