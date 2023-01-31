//
// saenaru.cpp
//
// IUnknown, ITfTextInputProcessor implementation.
//

#include "globals.h"
#include "saenarutip.h"

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT CSaenaruTextService::CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj)
{
    CSaenaruTextService *pInst;
    HRESULT hr;

    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    if ((pInst = new CSaenaruTextService) == NULL)
        return E_OUTOFMEMORY;

    hr = pInst->QueryInterface(riid, ppvObj);

    pInst->Release(); // caller still holds ref if hr == S_OK

    return hr;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CSaenaruTextService::CSaenaruTextService()
{
    DllAddRef();

    _pThreadMgr = NULL;
    _tfClientId = TF_CLIENTID_NULL;

    _pComposition = NULL;

    _fCleaningUp = FALSE;

    //_gaDisplayAttribute = TF_DEFAULT_SELECTION;
    _gaDisplayAttribute = TF_INVALID_GUIDATOM;

    _pLangBarItem = NULL;

    _dwThreadMgrEventSinkCookie = TF_INVALID_COOKIE;
    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;
    _dwTextEditSinkCookie = TF_INVALID_COOKIE;
    _dwGlobalCompartmentEventSinkCookie = TF_INVALID_COOKIE;

    _pTextEditSinkContext = NULL;

    _hWorkerWnd = NULL;

    _dwConversionStatus = 0;

    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CSaenaruTextService::~CSaenaruTextService()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor *)this;
    }
    else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
    {
        *ppvObj = (ITfDisplayAttributeProvider *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCreatePropertyStore))
    {
        *ppvObj = (ITfCreatePropertyStore *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextSink))
    {
        *ppvObj = (ITfCleanupContextSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCleanupContextDurationSink))
    {
        *ppvObj = (ITfCleanupContextDurationSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCompartmentEventSink))
    {
        *ppvObj = (ITfCompartmentEventSink *)this;
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

STDAPI_(ULONG) CSaenaruTextService::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CSaenaruTextService::Release()
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
// Activate
//
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::Activate(ITfThreadMgr *pThreadMgr, TfClientId tfClientId)
{
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;

    if (!_InitLanguageBar())
        goto ExitError;

    if (!_InitThreadMgrSink())
        goto ExitError;

    if (!_InitPreservedKey())
    {
        goto ExitError;
    }

    if (!_InitDisplayAttributeGuidAtom())
        goto ExitError;

    /*
    if (!_InitCleanupContextDurationSink())
        goto ExitError;

    if (!_InitGlobalCompartment())
        goto ExitError;

    if (!_InitWorkerWnd())
        goto ExitError;
    */

    if (!_InitKeystrokeSink())
        goto ExitError;

    return S_OK;

ExitError:
    Deactivate(); // cleanup any half-finished init
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
// Deactivate
//
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::Deactivate()
{
    _UninitThreadMgrSink();
    _UninitLanguageBar();
    //_UninitCleanupContextDurationSink();
    //_UninitGlobalCompartment();
    //_UninitWorkerWnd();
    _UninitKeystrokeSink();
    _UninitPreservedKey();

    _InitTextEditSink(NULL);

    // we MUST release all refs to _pThreadMgr in Deactivate
    SafeReleaseClear(_pThreadMgr);

    _tfClientId = TF_CLIENTID_NULL;

    return S_OK;
}

/*
 * vim: et sts=4 sw=4
 */
