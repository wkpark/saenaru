//
// compart.cpp
//
// Compartment example.
//

#include "globals.h"
#include "saenarutip.h"
extern "C" {
#include "hangul.h"
}

//+---------------------------------------------------------------------------
//
// _InitCompartment
//
// Initialize a compartment on a particular context.
//
// The Mark sample doesn't really do any with its context compartment, this
// code is purely for demonstration purposes.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_InitContextCompartment(ITfContext *pContext)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT varValue;
    HRESULT hr;

    // we want the mgr associated with pContext
    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return FALSE;

    hr = E_FAIL;

    if (pCompartmentMgr->GetCompartment(c_guidSaenaruContextCompartment, &pCompartment) != S_OK)
        goto Exit;

    // if we don't initialize the value, it will be VT_EMPTY
    // but let's initialize it to 0

    // NB: to keep things simple, we use a VT_I4
    // but you could use VT_UNKNOWN and store a pointer to anything
    varValue.vt = VT_I4;
    varValue.lVal = 0; // arbitrary value

    hr = pCompartment->SetValue(_tfClientId, &varValue);

    pCompartment->Release();

Exit:
    pCompartmentMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitCompartment
//
// Uninitialize a compartment on a particular context.
//
// The Mark sample doesn't really do any with its context compartment, this
// code is purely for demonstration purposes.
//----------------------------------------------------------------------------

void CSaenaruTextService::_UninitCompartment(ITfContext *pContext)
{
    ITfCompartmentMgr *pCompartmentMgr;

    // we want the mgr associated with pContext
    if (pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return;

    pCompartmentMgr->ClearCompartment(_tfClientId, c_guidSaenaruContextCompartment);

    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// _Menu_OnSetGlobalCompartment
//
// Callback for the "Set Global Compartment" menu item.
// Set the value of our global compartment.  This will trigger a callback
// on our compartment change sinks in every thread/instance of this service.
//----------------------------------------------------------------------------

/* static */
void CSaenaruTextService::_Menu_OnSetGlobalCompartment(CSaenaruTextService *_this)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT varValue;

    // we want the global mgr
    if (_this->_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return;

    if (pCompartmentMgr->GetCompartment(c_guidSaenaruGlobalCompartment, &pCompartment) != S_OK)
    {
        pCompartment = NULL;
        goto Exit;
    }

    // let's toggle the value
    // notice that global compartments are persisted, unlike all others
    if (FAILED(pCompartment->GetValue(&varValue))) // will return S_FALSE if varValue.vt == VT_EMPTY
        goto Exit;

    if (varValue.vt == VT_EMPTY)
    {
        // if we get here, the compartment has never been initialized
        varValue.vt = VT_I4;
        varValue.lVal = 0;
    }

    // toggle value
    varValue.lVal = ~varValue.lVal;

    pCompartment->SetValue(_this->_tfClientId, &varValue);

Exit:
    SafeRelease(pCompartment);
    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// _InitGlobalCompartment
//
// Ininit a change sink on our global compartment.  The system will call us
// back anytime the compartment is modified from any thread in the desktop.
//
// NB: ITfCompartmentEventSink's attached to thread local compartments will
// only get callbacks to changes that occur within a single thread.  Global
// compartments are different.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_InitGlobalCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    BOOL fRet;

    // we want the global mgr
    if (_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return FALSE;

    fRet = FALSE;

    if (pCompartmentMgr->GetCompartment(c_guidSaenaruGlobalCompartment, &pCompartment) != S_OK)
        goto Exit;

    fRet = AdviseSink(pCompartment, (ITfCompartmentEventSink *)this,
                      IID_ITfCompartmentEventSink, &_dwGlobalCompartmentEventSinkCookie);

    pCompartment->Release();

    if (!fRet)
    {
        // don't try to unadvise a bogus cookie later
        _dwGlobalCompartmentEventSinkCookie = TF_INVALID_COOKIE;
    }

Exit:
    pCompartmentMgr->Release();
    return fRet;
}

//+---------------------------------------------------------------------------
//
// _UninitCompartment
//
// Unitialize the global compartment if we have previously accessed it.
// This method only frees resources the system has allocated in this thread.
// Other threads can still access the global compartment, and the value (which
// is persisted across the desktop) does not change.
//
// Also, uninit the change sink we attached to the compartment.
//----------------------------------------------------------------------------

void CSaenaruTextService::_UninitGlobalCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;

    // we want the global mgr
    if (_pThreadMgr->GetGlobalCompartment(&pCompartmentMgr) != S_OK)
        return;

    // unadvise our event sink
    if (pCompartmentMgr->GetCompartment(c_guidSaenaruGlobalCompartment, &pCompartment) == S_OK)
    {
        UnadviseSink(pCompartment, &_dwGlobalCompartmentEventSinkCookie);
        pCompartment->Release();
    }

    // let the system free resources associated with the compartment on this
    // thread
    pCompartmentMgr->ClearCompartment(_tfClientId, c_guidSaenaruGlobalCompartment);

    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// The keyboard compartments
//
// ImmGetOpenStatus() and ImmGetConversionStatus() of the IMM applications are
// the keyboard compartments of the thread. Like as the IMM IME, the hangul
// mode is open with the native mode, and the english mode is closed. gvim
// changes them to the english at <ESC>, and back to the mode saved at the
// insert mode.
//----------------------------------------------------------------------------

static const GUID *c_rgKeyboardCompartment[] = {
    &GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
    &GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION,
};

static DWORD GetCompartmentDword(ITfCompartmentMgr *pCompartmentMgr, REFGUID rguid)
{
    ITfCompartment *pCompartment;
    VARIANT var;
    DWORD dw = 0;

    if (pCompartmentMgr->GetCompartment(rguid, &pCompartment) != S_OK)
        return 0;
    VariantInit(&var);
    if (pCompartment->GetValue(&var) == S_OK && var.vt == VT_I4)
        dw = (DWORD)var.lVal;
    VariantClear(&var);
    pCompartment->Release();
    return dw;
}

static void SetCompartmentDword(ITfCompartmentMgr *pCompartmentMgr, TfClientId tfClientId, REFGUID rguid, DWORD dw)
{
    ITfCompartment *pCompartment;
    VARIANT var;

    if (pCompartmentMgr->GetCompartment(rguid, &pCompartment) != S_OK)
        return;
    var.vt = VT_I4;
    var.lVal = (LONG)dw;
    pCompartment->SetValue(tfClientId, &var);
    pCompartment->Release();
}

void CSaenaruTextService::_InitKeyboardCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    int i;

    static_assert(ARRAYSIZE(c_rgKeyboardCompartment) == ARRAYSIZE(_dwKeyboardCompartmentSinkCookie),
                  "a cookie for each compartment");

    if (_pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return;
    for (i = 0; i < ARRAYSIZE(c_rgKeyboardCompartment); i++)
    {
        if (pCompartmentMgr->GetCompartment(*c_rgKeyboardCompartment[i], &pCompartment) != S_OK)
            continue;
        AdviseSink(pCompartment, (ITfCompartmentEventSink *)this,
                   IID_ITfCompartmentEventSink, &_dwKeyboardCompartmentSinkCookie[i]);
        pCompartment->Release();
    }
    pCompartmentMgr->Release();

    _SetKeyboardCompartment(GetConversionStatus());
}

void CSaenaruTextService::_UninitKeyboardCompartment()
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    int i;

    if (_pThreadMgr == NULL || _pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return;
    for (i = 0; i < ARRAYSIZE(c_rgKeyboardCompartment); i++)
    {
        if (pCompartmentMgr->GetCompartment(*c_rgKeyboardCompartment[i], &pCompartment) != S_OK)
            continue;
        UnadviseSink(pCompartment, &_dwKeyboardCompartmentSinkCookie[i]);
        pCompartment->Release();
    }
    pCompartmentMgr->Release();
}

// like as the hangul key of the IMM IME, the english mode is closed without
// the native mode
void CSaenaruTextService::_SetKeyboardCompartment(DWORD status)
{
    ITfCompartmentMgr *pCompartmentMgr;
    DWORD dwConversion, dwNew;
    BOOL fHangul = (status & CMODE_HANGUL) != 0;

    if (_pThreadMgr == NULL || _pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return;

    _fSettingKeyboardCompartment = TRUE;
    if ((GetCompartmentDword(pCompartmentMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) != 0) != fHangul)
        SetCompartmentDword(pCompartmentMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, fHangul);
    dwConversion = GetCompartmentDword(pCompartmentMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
    if (fHangul)
        dwNew = dwConversion | TF_CONVERSIONMODE_NATIVE;
    else
        dwNew = dwConversion & ~TF_CONVERSIONMODE_NATIVE;
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_SetKeyboardCompartment(0x%x) tid = %d, conversion = 0x%x -> 0x%x\n"),
                        status, GetCurrentThreadId(), dwConversion, dwNew));
    if (dwNew != dwConversion)
        SetCompartmentDword(pCompartmentMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, dwNew);
    _fSettingKeyboardCompartment = FALSE;

    pCompartmentMgr->Release();
}

//+---------------------------------------------------------------------------
//
// ITfCompartmentEventSink::OnChange
//
// The keyboard compartments changed by the application, like as
// ImmSetConversionStatus() and ImmSetOpenStatus() of gvim.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnChange(REFGUID rguidCompartment)
{
    ITfCompartmentMgr *pCompartmentMgr;
    ITfRange *pRangeComposition;
    ITfContext *pContext;
    DWORD dwOpen, dwConversion;
    BOOL fHangul;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::OnChange(openclose = %d) tid = %d, by the TIP = %d\n"),
                        IsEqualGUID(rguidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE),
                        GetCurrentThreadId(), _fSettingKeyboardCompartment));
    if (_fSettingKeyboardCompartment || _pThreadMgr == NULL ||
        !(IsEqualGUID(rguidCompartment, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE) ||
          IsEqualGUID(rguidCompartment, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION)))
        return S_OK;
    if (_pThreadMgr->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return S_OK;
    dwOpen = GetCompartmentDword(pCompartmentMgr, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    dwConversion = GetCompartmentDword(pCompartmentMgr, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION);
    pCompartmentMgr->Release();
    fHangul = dwOpen && (dwConversion & TF_CONVERSIONMODE_NATIVE);
    DEBUGPRINTFEX(100, (TEXT("\topen = %d, conversion = 0x%x, status = 0x%x, composing = %d\n"),
                        dwOpen, dwConversion, GetConversionStatus(), _IsComposing()));

    if (fHangul == ((GetConversionStatus() & CMODE_HANGUL) != 0))
        return S_OK;

    // commit the hangul like as the hangul key
    _CommitCandidate(NULL);
    _ResetEngTypo();
    if (_IsComposing())
    {
        if (_GetComposition()->GetRange(&pRangeComposition) == S_OK)
        {
            if (pRangeComposition->GetContext(&pContext) == S_OK)
            {
                WCHAR ch = _HangulCommit();
                _EndCompositionInContext(pContext, ch);
                pContext->Release();
            }
            pRangeComposition->Release();
        }
        // ic is shared with the other threads. only for the composition here
        hangul_ic_init(&ic);
    }

    // not by SetConversionStatus(). the native mode is kept while closed.
    if (fHangul)
        _dwConversionStatus |= CMODE_HANGUL;
    else
        _dwConversionStatus &= ~CMODE_HANGUL;
    _UpdateLanguageBar(TF_LBI_STATUS | TF_LBI_ICON);
    return S_OK;
}

/*
 * ex: ts=4 sts=4 sw=4 et
 */
