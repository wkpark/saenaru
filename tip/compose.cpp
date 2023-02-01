//
// compose.cpp
//
// Composition code.
//

#include "globals.h"
#include "saenarutip.h"
#include "editsess.h"

extern "C" {
#include "hangul.h"
}

class CCompositionEditSession : public CEditSessionBase
{
public:
    CCompositionEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
    }
    ~CCompositionEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CSaenaruTextService *_pSaenaru;
};

class CStartCompositionEditSession : public CEditSessionBase
{
public:
    CStartCompositionEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
        _wParam = wParam;
    }
    ~CStartCompositionEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);
private:
    CSaenaruTextService *_pSaenaru;
    WPARAM _wParam;
};

class CEndCompositionEditSession : public CEditSessionBase
{
public:
    CEndCompositionEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
        _wParam = wParam;
    }
    ~CEndCompositionEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CSaenaruTextService *_pSaenaru;
    WPARAM _wParam;
};

//+---------------------------------------------------------------------------
//
// _StartCompositionInContext
//
//----------------------------------------------------------------------------
void CSaenaruTextService::_StartCompositionInContext(ITfContext *pContext, WPARAM wParam)
{
    CStartCompositionEditSession *pEditSession;
    HRESULT hr;
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_StartComposition(%p)\n"), this));

    if (pEditSession = new CStartCompositionEditSession(this, pContext, wParam))
    {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
        //pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _EndCompositionInContext
//
//----------------------------------------------------------------------------

void CSaenaruTextService::_EndCompositionInContext(ITfContext *pContext, WPARAM wParam)
{
    CEndCompositionEditSession *pEditSession;
    HRESULT hr;
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_EndCompositionInContext(%p)\n"), this));

    if (pEditSession = new CEndCompositionEditSession(this, pContext, wParam))
    {
        pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr);
        //pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_ASYNCDONTCARE | TF_ES_READWRITE, &hr);
        pEditSession->Release();
    }
}

//+---------------------------------------------------------------------------
//
// _Menu_OnComposition
//
// Callback for the "Start/End Composition" menu item.
// If we have a composition, end it.  Otherwise start a new composition over
// the selection of the current focus context.
//----------------------------------------------------------------------------

/* static */
void CSaenaruTextService::_Menu_OnComposition(CSaenaruTextService *_this)
{
    ITfDocumentMgr *pFocusDoc;
    ITfContext *pContext;
    CCompositionEditSession *pCompositionEditSession;
    HRESULT hr;

    // get the focus document
    if (_this->_pThreadMgr->GetFocus(&pFocusDoc) != S_OK)
        return;

    // we want the topmost context, since the main doc context could be
    // superceded by a modal tip context
    if (pFocusDoc->GetTop(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    if (pCompositionEditSession = new CCompositionEditSession(_this, pContext))
    {
        // we need a document write lock
        // the CCompositionEditSession will do all the work when the
        // CCompositionEditSession::DoEditSession method is called by the context
        pContext->RequestEditSession(_this->_tfClientId, pCompositionEditSession, TF_ES_READWRITE | TF_ES_ASYNCDONTCARE, &hr);

        pCompositionEditSession->Release();
    }

Exit:
    SafeRelease(pContext);
    pFocusDoc->Release();
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------
STDAPI CStartCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection *pInsertAtSelection = NULL;
    ITfRange *pRangeInsert = NULL;
    ITfContextComposition *pContextComposition = NULL;
    ITfComposition *pComposition = NULL;
    HRESULT hr = E_FAIL;

    // insert the text
    WCHAR ch = (WCHAR)_wParam;

    DEBUGPRINTFEX(100, (TEXT("CStartCompositionEditSession::DoEditSession(%lu)\n"), ec));

    // let's start a new composition over the current selection
    // this is totally contrived, a real text service would have
    // some meaningful logic to trigger this

    // first, test where a keystroke would go in the document if we did an insert
    // we need a special interface to insert text at the selection
    if (_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
    {
        pInsertAtSelection = NULL;
        goto Exit;
    }

    if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, ch ? &ch : NULL, ch ? ch : 0, &pRangeInsert) != S_OK)
        goto Exit;

    // get an interface on the context to deal with compositions
    if (_pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
    {
        goto Exit;
    }

    // start the composition
    if (pContextComposition->StartComposition(ec, pRangeInsert, _pSaenaru, &pComposition) != S_OK)
    {
        pComposition = NULL;
        DEBUGPRINTFEX(100, (TEXT("\tStartComposition() failed??\n")));
    }

    // _pComposition may be NULL even if StartComposition return S_OK, this mean the application
    // rejected the composition

    // insert the text
    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    if (ch != NULL && pRangeInsert->SetText(ec, TF_ST_CORRECTION, &ch, 1) != S_OK)
    {
        goto Exit;
    }

    if (pComposition != NULL)
    {
        DEBUGPRINTFEX(100, (TEXT("\tSetComposition\n"), ec));
        _pSaenaru->_SetComposition(pComposition);
        // underline the composition text to give the user some feedback UI
        _pSaenaru->_SetCompositionDisplayAttributes(ec);

        // set selection to the adjusted range
        TF_SELECTION tfSelection;
        tfSelection.range = pRangeInsert;
        tfSelection.style.ase = TF_AE_NONE;
        tfSelection.style.fInterimChar = TRUE;

        _pContext->SetSelection(ec, 1, &tfSelection);
    }

    // if we make it here, we've succeeded
    hr = S_OK;

Exit:
    SafeRelease(pRangeInsert);

    SafeRelease(pInsertAtSelection);
    SafeRelease(pContextComposition);

    return hr;
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------
STDAPI CEndCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection *pInsertAtSelection;
    ITfRange *pRangeInsert;
    ITfRange *pRangeComposition;
    ITfContext *pCompositionContext;
    HRESULT hr;
    BOOL fEqualContexts;

    DEBUGPRINTFEX(100, (TEXT("CEndCompositionEditSession::DoEditSession(%lu) "), ec));
    if (_wParam)
        DEBUGPRINTFEX(100, (TEXT("wParam = '%c'"), _wParam));
    DEBUGPRINTFEX(100, (TEXT("\r\n")));

    if (_wParam) {
        // let's start a new composition over the current selection
        // this is totally contrived, a real text service would have
        // some meaningful logic to trigger this
        WCHAR ch = (WCHAR) _wParam;

        // first, test where a keystroke would go in the document if we did an insert
        // we need a special interface to insert text at the selection
        if (_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) == S_OK)
        {
            if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, ch ? &ch :NULL, ch ? ch : 0, &pRangeInsert) == S_OK)
            {
                DEBUGPRINTFEX(100, (TEXT("\tANCHOR_END SetSelection()\n")));

                // double check empty range case. (firefox/chrome etc.)
                BOOL isEmpty;
                ITfRange *range = NULL;
                if (ch != NULL && pRangeInsert->IsEmpty(ec, &isEmpty) == S_OK && pRangeInsert->Clone(&range) == S_OK)
                {
                    BOOL isEqual;
                    range->Collapse(ec, TF_ANCHOR_END);
                    pRangeInsert->IsEqualStart(ec, range, TF_ANCHOR_START, &isEqual);
                    DEBUGPRINTFEX(100, (TEXT("\tis Eqaul Start = %d\n"), isEqual));
                    if (isEqual)
                    {
                        LONG cch;
                        pRangeInsert->ShiftStart(ec, -1, &cch, NULL);
                    }

                    range->Release();
                }

                if (pRangeInsert->SetText(ec, 0, &ch, 1) == S_OK)
                {
                    // update the selection
                    TF_SELECTION tfSelection;
                    tfSelection.range = pRangeInsert;
                    tfSelection.style.ase = TF_AE_END;
                    tfSelection.style.fInterimChar = FALSE; // not a intermidate char
                    tfSelection.range->Collapse(ec, TF_ANCHOR_END);

                    _pContext->SetSelection(ec, 1, &tfSelection);

                    pRangeInsert->Release();
                }
            }
            SafeRelease(pInsertAtSelection);
        }
    }
    hr = E_FAIL;

#if 0
    // we have a composition, let's terminate it

    // it's possible our current composition is in another context...let's find out
    fEqualContexts = TRUE;
    if (_pSaenaru->_GetComposition()->GetRange(&pRangeComposition) == S_OK)
    {
        if (pRangeComposition->GetContext(&pCompositionContext) == S_OK)
        {
            fEqualContexts = IsEqualUnknown(pCompositionContext, _pContext);
            if (!fEqualContexts)
            {
                // need an edit session in the composition context
                _pSaenaru->_EndCompositionInContext(pCompositionContext, 0);
            }
            pCompositionContext->Release();
        }
        pRangeComposition->Release();
    }

    // if the composition is in pContext, we already have an edit cookie
    if (fEqualContexts)
    {
        _pSaenaru->_TerminateComposition(ec);
        DEBUGPRINTFEX(100, (TEXT("\tTerminateCompositon !!\n")));
    }
#else
    _pSaenaru->_TerminateComposition(ec);
    DEBUGPRINTFEX(100, (TEXT("\t_TerminateCompositon() !!\n")));
#endif

    hr = S_OK;

    return hr;
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CCompositionEditSession::DoEditSession(TfEditCookie ec)
{
    ITfInsertAtSelection *pInsertAtSelection;
    ITfContextComposition *pContextComposition;
    ITfComposition *pComposition;
    ITfRange *pRangeComposition;
    ITfRange *pRangeInsert;
    ITfContext *pCompositionContext;
    HRESULT hr;
    BOOL fEqualContexts;

    DEBUGPRINTFEX(100, (TEXT("CCompositionEditSession::DoEditSession(%lu)\n"), ec));

    // get an interface on the context we can use to deal with compositions
    if (_pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
        return E_FAIL;

    hr = E_FAIL;

    pInsertAtSelection = NULL;

    if (_pSaenaru->_IsComposing())
    {
        // we have a composition, let's terminate it

        // it's possible our current composition is in another context...let's find out
        fEqualContexts = TRUE;
        if (_pSaenaru->_GetComposition()->GetRange(&pRangeComposition) == S_OK)
        {
            if (pRangeComposition->GetContext(&pCompositionContext) == S_OK)
            {
                fEqualContexts = IsEqualUnknown(pCompositionContext, _pContext);
                if (!fEqualContexts)
                {
                    // need an edit session in the composition context
                    _pSaenaru->_EndCompositionInContext(pCompositionContext, 0);
                }
                pCompositionContext->Release();
            }
            pRangeComposition->Release();
        }

        // if the composition is in pContext, we already have an edit cookie
        if (fEqualContexts)
        {
            _pSaenaru->_TerminateComposition(ec);
        }
    }
    else
    {
        // let's start a new composition over the current selection
        // this is totally contrived, a real text service would have
        // some meaningful logic to trigger this

        // first, test where a keystroke would go in the document if we did an insert
        // we need a special interface to insert text at the selection
        if (_pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) != S_OK)
        {
            pInsertAtSelection = NULL;
            goto Exit;
        }

        if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert) != S_OK)
            goto Exit;

        // start the composition
        if (pContextComposition->StartComposition(ec, pRangeInsert, _pSaenaru, &pComposition) != S_OK)
        {
            pComposition = NULL;
        }

        pRangeInsert->Release();

        // _pComposition may be NULL even if StartComposition return S_OK, this mean the application
        // rejected the composition

        if (pComposition != NULL)
        {
            _pSaenaru->_SetComposition(pComposition);
            // underline the composition text to give the user some feedback UI
            _pSaenaru->_SetCompositionDisplayAttributes(ec);
        }
    }

    // if we make it here, we've succeeded
    hr = S_OK;

Exit:
    SafeRelease(pInsertAtSelection);
    pContextComposition->Release();

    return hr;
}

//+---------------------------------------------------------------------------
//
// OnCompositionTerminated
//
// Callback for ITfCompositionSink.  The system calls this method whenever
// someone other than this service ends a composition.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnCompositionTerminated(TfEditCookie ecWrite, ITfComposition *pComposition)
{
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::DoCompositionTerminated(%lu)\n"), ecWrite));
    // we already have the composition cached, so we can ignore pComposition...
    if (_IsComposing()) {
        ITfRange *pRangeComposition;
        ITfContext *pContext;
        ITfProperty *pDisplayAttributeProperty;

        // we need a range and the context it lives in
        if (_pComposition->GetRange(&pRangeComposition) != S_OK)
            return S_OK;

        if (pRangeComposition->GetContext(&pContext) != S_OK)
        {
            pRangeComposition->Release();
            pContext = NULL;
            return S_OK;
        }
        DEBUGPRINTFEX(100, (TEXT("\tCall TerminateComposition\r\n")));
        _EndCompositionInContext(pContext, 0);
        // init hangul ic
        hangul_ic_init(&ic);

        pRangeComposition->Release();
        pContext->Release();
    }

    // all this service wants to do is clear the display property
    //_ClearCompositionDisplayAttributes(ecWrite);

    // releae our cached composition
    SafeReleaseClear(_pComposition);

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _ClearCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

void CSaenaruTextService::_ClearCompositionDisplayAttributes(TfEditCookie ec)
{
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    ITfProperty *pDisplayAttributeProperty;

    // we need a range and the context it lives in
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return;

    if (pRangeComposition->GetContext(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    // get our the display attribute property
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) != S_OK)
        goto Exit;

    // clear the value over the range
    pDisplayAttributeProperty->Clear(ec, pRangeComposition);

    pDisplayAttributeProperty->Release();

Exit:
    pRangeComposition->Release();
    SafeRelease(pContext);
}

//+---------------------------------------------------------------------------
//
// _SetCompositionDisplayAttributes
//
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_SetCompositionDisplayAttributes(TfEditCookie ec)
{
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    ITfProperty *pDisplayAttributeProperty;
    VARIANT var;
    HRESULT hr;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_SetCompositionDisplayAttributes()\n")));
    // we need a range and the context it lives in
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pRangeComposition->GetContext(&pContext) != S_OK)
    {
        pContext = NULL;
        goto Exit;
    }

    // get our the display attribute property
    if (pContext->GetProperty(GUID_PROP_ATTRIBUTE, &pDisplayAttributeProperty) != S_OK)
        goto Exit;

    // set the value over the range
    // the application will use this guid atom to lookup the acutal rendering information
    var.vt = VT_I4; // we're going to set a TfGuidAtom
    var.lVal = _gaDisplayAttribute; // our cached guid atom for c_guidSaenaruDisplayAttribute

    hr = pDisplayAttributeProperty->SetValue(ec, pRangeComposition, &var);

    pDisplayAttributeProperty->Release();
    DEBUGPRINTFEX(100, (TEXT("\tOK!!\n")));

Exit:
    pRangeComposition->Release();
    SafeRelease(pContext);
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _InitDisplayAttributeGuidAtom
//
// Because it's expensive to map our display attribute GUID to a TSF
// TfGuidAtom, we do it once when Activate is called.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_InitDisplayAttributeGuidAtom()
{
    ITfCategoryMgr *pCategoryMgr;
    HRESULT hr;

    if (CoCreateInstance(CLSID_TF_CategoryMgr,
                         NULL,
                         CLSCTX_INPROC_SERVER,
                         IID_ITfCategoryMgr,
                         (void**)&pCategoryMgr) != S_OK)
    {
        return FALSE;
    }

    hr = pCategoryMgr->RegisterGUID(c_guidSaenaruDisplayAttribute, &_gaDisplayAttribute);

    pCategoryMgr->Release();

    return (hr == S_OK);
}

/*
 * vim: et sts=4 sw=4
 */
