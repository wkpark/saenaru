//
// keys.cpp
//
// ITfKeyEventSink implementation.
//

#include "globals.h"
#include "saenarutip.h"
#include "langbar.h"
#include "editsess.h"

extern "C" {
#include "hangul.h"
}

// from NavilIME
// {71186DC9-D6CB-0412-A256-6BAA84F0792F}
static const GUID cPreservedKey_GUID_ToggleHangul =
{ 0x71186DC9, 0xD6CB, 0x0412,{ 0xA2, 0x56, 0x6B, 0xAA, 0x84, 0xF0, 0x79, 0x2F } };

/* Set Shift+Space as on off key */
static const TF_PRESERVEDKEY cShiftSpace = { VK_SPACE, TF_MOD_SHIFT };
static const TF_PRESERVEDKEY cHangul = { VK_HANGUL, TF_MOD_IGNORE_ALL_MODIFIER };
static const WCHAR ToggleDesc[] = L"Hangul Toggle";

// {72186DC9-D6CB-0412-A256-6BAA84F0792F}
static const GUID cPreservedKey_GUID_Hanja =
{ 0x72186DC9, 0xD6CB, 0x0412,{ 0xA2, 0x56, 0x6B, 0xAA, 0x84, 0xF0, 0x79, 0x2F } };
static const TF_PRESERVEDKEY cHanja  = { VK_HANJA, TF_MOD_IGNORE_ALL_MODIFIER };
static const TF_PRESERVEDKEY cRControl = { VK_CONTROL, TF_MOD_RCONTROL };
static const WCHAR HanjaDesc[] = L"Hanja";

// shift without ctrl and alt
static BOOL IsShiftOnly()
{
    return (GetKeyState(VK_SHIFT) & 0x8000) &&
           !(GetKeyState(VK_CONTROL) & 0x8000) && !(GetKeyState(VK_MENU) & 0x8000);
}

class CReconvertEditSession : public CEditSessionBase
{
public:
    CReconvertEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
    }
    ~CReconvertEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        return _pSaenaru->_ReconvertSyllable(ec, _pContext, TRUE);
    }

private:
    CSaenaruTextService *_pSaenaru;
};

class CKeystrokeEditSession : public CEditSessionBase
{
public:
    CKeystrokeEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext, WPARAM wParam) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
        _wParam = wParam;
    }
    ~CKeystrokeEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec);

private:
    CSaenaruTextService *_pSaenaru;
    WPARAM _wParam;
};

// Conversion Mode
DWORD CSaenaruTextService::GetConversionStatus()
{
//    if (pContext == NULL)
//        return CMODE_ALPHANUMERIC;

    // FIXME
    return _dwConversionStatus;
}

DWORD CSaenaruTextService::SetConversionStatus(DWORD status)
{
//    if (pContext == NULL)
//        return 0;

    // FIXME
    _dwConversionStatus = status;
    return _dwConversionStatus;
}

//+---------------------------------------------------------------------------
//
// _HandleReturn
//
// Returns S_OK to eat the keystroke, S_FALSE otherwise.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_HandleReturn(TfEditCookie ec, ITfContext *pContext)
{
    // just terminate the composition
    _TerminateComposition(ec);
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _HandleArrowKey
//
// Update the selection within a composition.
// Returns S_OK to eat the keystroke, S_FALSE otherwise.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_HandleArrowKey(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    LONG cch;
    BOOL fEqual;
    TF_SELECTION tfSelection;
    ULONG cFetched;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleArrowKey(), wParam = %x\n"), wParam));
    if (_IsComposing())
    {
        DEBUGPRINTFEX(100, (TEXT("\tTerminateComposition\r\n")));
        _EndCompositionInContext(pContext);
        hangul_ic_init(&ic);
    }

    // get the selection
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK ||
        cFetched != 1)
    {
        // no selection?
        return S_OK; // eat the keystroke
    }

    // get the composition range
    if (_pComposition->GetRange(&pRangeComposition) != S_OK)
        goto Exit;

    // adjust the selection, we won't do anything fancy

    if (wParam == VK_LEFT)
    {
        if (tfSelection.range->IsEqualStart(ec, pRangeComposition, TF_ANCHOR_START, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftStart(ec, -1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_START);
    }
    else
    {
        // VK_RIGHT
        if (tfSelection.range->IsEqualEnd(ec, pRangeComposition, TF_ANCHOR_END, &fEqual) == S_OK &&
            !fEqual)
        {
            tfSelection.range->ShiftEnd(ec, +1, &cch, NULL);
        }
        tfSelection.range->Collapse(ec, TF_ANCHOR_END);
    }

    pContext->SetSelection(ec, 1, &tfSelection);

    pRangeComposition->Release();

Exit:
    tfSelection.range->Release();
    return S_OK; // eat the keystroke
}

//+---------------------------------------------------------------------------
//
// _HandleBackspace
//
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_HandleBackspace(TfEditCookie ec, ITfContext *pContext)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    HRESULT hr;
    WCHAR ch;
    BOOL fCovered;

    /* for hangul automata */
    TCHAR lcs[128];
    WCHAR cs = 0;
    int ncs;
    lcs[0] = 0;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleBackspace()\n")));
    if (!_IsComposing())
        return _ReconvertSyllable(ec, pContext, FALSE);

    DWORD status = GetConversionStatus();
    if (status & CMODE_HANGUL && _IsComposing()) {
        // shift + backspace deletes the syllable at once like as the IMM IME
        if (IsShiftOnly())
            hangul_ic_init(&ic);
        if (ic.len) {
            hangul_ic_backspace(&ic);
            if (ic.len > 0 && ic.syllable) {
                ncs = hangul_ic_get(&ic, 0, lcs);
                if (ncs >= 1) {
                    ch = lcs[ncs - 1];
                } else {
                    // empty preedit
                    hangul_ic_init(&ic);
                    ch = NULL;
                }
            } else if (ic.len == 0) {
                hangul_ic_init(&ic);
                ch = NULL;
            } else {
                // Middle-aged hangul
                // FIXME
            }
        } else {
            // empty preedit
            hangul_ic_init(&ic);
            ch = NULL;
        }
    }

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleBackspace() ch = %x\n"), ch));

    hr = S_OK; // return S_FALSE to NOT eat the key

    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1) {
        DEBUGPRINTFEX(100, (TEXT("\tGetSelection() failed?\n")));
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (_pComposition->GetRange(&pRangeComposition) == S_OK)
    {
        fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!fCovered)
        {
            hr = S_FALSE; // don't eat the key, it's outside our composition
            goto Exit;
        }
    }

    // insert the text
    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    if (tfSelection.range->SetText(ec, ch != NULL ? TF_ST_CORRECTION : 0, ch ? &ch : NULL, ch ? 1 : 0) != S_OK)
        goto Exit;

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    //tfSelection.range->Collapse(ec, TF_ANCHOR_END);

    pContext->SetSelection(ec, 1, &tfSelection);

    // apply our dislay attribute property to the inserted text
    // we need to apply it to the entire composition, since the
    // display attribute property is static, not static compact
    _SetCompositionDisplayAttributes(ec);

    // the composition is empty
    if (ch == NULL)
        _TerminateComposition(ec);

Exit:
    tfSelection.range->Release();
    DEBUGPRINTFEX(100, (TEXT("\tEND CSaenaruTextService::_HandleKeyDown()\n")));
    if (ch == NULL)
        return S_FALSE;
    return hr;
}

//+---------------------------------------------------------------------------
//
// _ReconvertSyllable
//
// Shift + VK_BACK without the composition. Compose the syllable before the
// caret again and delete the last jamo like as DicKeydownHandler() of the
// IMM IME. fTest only checks the syllable.
// Returns S_OK if the syllable is reconverted.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_ReconvertSyllable(TfEditCookie ec, ITfContext *pContext, BOOL fTest)
{
    TF_SELECTION tfSelection;
    ITfContextComposition *pContextComposition;
    ITfComposition *pComposition;
    ITfRange *pRange;
    ULONG cFetched, cch;
    LONG cchShift;
    BOOL fEmpty;
    TCHAR lcs[128];
    WCHAR ch;
    int ncs;
    HRESULT hr = S_FALSE;

    if (_IsComposing())
        return S_FALSE;

    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return S_FALSE;
    pRange = tfSelection.range;

    // the syllable before the caret
    if (pRange->IsEmpty(ec, &fEmpty) != S_OK || !fEmpty)
        goto Exit;
    if (pRange->ShiftStart(ec, -1, &cchShift, NULL) != S_OK || cchShift != -1)
        goto Exit;
    if (pRange->GetText(ec, 0, &ch, 1, &cch) != S_OK || cch != 1 || !hangul_is_syllable(ch))
        goto Exit;

    if (fTest)
    {
        hr = S_OK;
        goto Exit;
    }

    if (pContext->QueryInterface(IID_ITfContextComposition, (void **)&pContextComposition) != S_OK)
        goto Exit;

    pComposition = NULL;
    if (pContextComposition->StartComposition(ec, pRange, this, &pComposition) == S_OK && pComposition != NULL)
    {
        _SetComposition(pComposition);

        hangul_ic_set_syllable(&ic, ch);
        hangul_ic_backspace(&ic);
        ncs = hangul_ic_get(&ic, 0, lcs);
        ch = (ncs >= 1) ? lcs[ncs - 1] : 0;

        if (pRange->SetText(ec, TF_ST_CORRECTION, ch ? &ch : NULL, ch ? 1 : 0) == S_OK)
        {
            tfSelection.style.ase = TF_AE_NONE;
            tfSelection.style.fInterimChar = TRUE;
            pContext->SetSelection(ec, 1, &tfSelection);
            _SetCompositionDisplayAttributes(ec);
            hr = S_OK;
        }
        if (!ch || hr != S_OK)
        {
            hangul_ic_init(&ic);
            _TerminateComposition(ec);
        }
    }
    pContextComposition->Release();

Exit:
    pRange->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// _IsReconvertKey
//
// Shift + VK_BACK on the syllable before the caret.
// Without fTest, only check the key. The edit session checks the syllable.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_IsReconvertKey(ITfContext *pContext, WPARAM wParam, BOOL fTest)
{
    CReconvertEditSession *pEditSession;
    HRESULT hr = S_FALSE;

    if (wParam != VK_BACK || _IsComposing() || !IsShiftOnly())
        return FALSE;
    if (!fTest)
        return TRUE;

    if ((pEditSession = new CReconvertEditSession(this, pContext)) == NULL)
        return FALSE;
    if (pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READ, &hr) != S_OK)
        hr = S_FALSE;
    pEditSession->Release();

    return hr == S_OK;
}

//+---------------------------------------------------------------------------
//
// _HangulCommitComposition /* not used */
//
// commit the given preedit char and terminate composition
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_HangulCommitComposition(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    HRESULT hr;
    WCHAR ch = (WCHAR)wParam;
    BOOL fCovered;

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandlePreedit() ch = 0x%x, %d)\n"), wParam, wParam));
    hr = S_OK; // return S_FALSE to NOT eat the key

    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1) {
        DEBUGPRINTFEX(100, (TEXT("\tGetSelection() failed?\n")));
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (_pComposition->GetRange(&pRangeComposition) == S_OK)
    {
        fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!fCovered)
        {
            hr = S_FALSE; // don't eat the key, it's outside our composition
            goto Exit;
        }
    }

    // insert the text
    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    if (tfSelection.range->SetText(ec, TF_ST_CORRECTION, ch ? &ch : NULL, ch ? 1 : 0) != S_OK)
        goto Exit;

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    tfSelection.range->Collapse(ec, TF_ANCHOR_END);

    pContext->SetSelection(ec, 1, &tfSelection);

    // apply our dislay attribute property to the inserted text
    // we need to apply it to the entire composition, since the
    // display attribute property is static, not static compact
    //_SetCompositionDisplayAttributes(ec);

Exit:
    tfSelection.range->Release();
    return hr;
}

//
// _HangulCommit()
//
// it will destroy/init the hangulIC. (ic)
WCHAR CSaenaruTextService::_HangulCommit()
{
    TCHAR lcs[128];
    int ncs;
    ncs = hangul_ic_get(&ic, 1, lcs);
    if (ncs > 0)
        return lcs[ncs - 1];
    else
        return 0;
}

WCHAR CSaenaruTextService::_HangulPreedit()
{
    TCHAR lcs[128];
    int ncs;
    ncs = hangul_ic_get(&ic, 0, lcs);
    if (ncs > 0)
        return lcs[ncs - 1];
    else
        return 0;
}

//+---------------------------------------------------------------------------
//
// _HandleKeyDown
//
// If the keystroke happens within a composition, eat the key and return S_OK.
// Otherwise, do nothing and return S_FALSE.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_HandleKeyDown(TfEditCookie ec, ITfContext *pContext, WPARAM wParam)
{
    ITfRange *pRangeComposition;
    TF_SELECTION tfSelection;
    ULONG cFetched;
    HRESULT hr;
    WCHAR ch;
    BOOL fCovered;

    /* for hangul automata */
    DWORD hkey = 0;
    TCHAR lcs[128];
    WCHAR cs = 0;
    int ret, ncs;
    lcs[0] = 0;

    ch = _ConvertVKey((UINT)wParam);
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleKeydown() VK key ch = 0x%x, %d)\n"), ch, ch));

    DWORD status = GetConversionStatus();
    if (status & CMODE_HANGUL) {
        hkey = keyToHangulKey(ch);

        DEBUGPRINTFEX(100, (TEXT("\t hkey = %x, ch = %x\n"), hkey, ch));
    } else {
        return S_OK; // just eat the key if it's not in a range we know how to handle
    }

    if (hkey == 0 || hkey == ch) {
        DEBUGPRINTFEX(100, (TEXT("\tXXX same converted hkey or no converted %x = ch %x\n"), hkey, ch));
        if (_IsComposing()) {
            DEBUGPRINTFEX(100, (TEXT("\tTerminateComposition\r\n")));
            WCHAR commit;
            commit = _HangulCommit();
            _EndCompositionInContext(pContext, commit);
            //hangul_ic_init(&ic);
            return S_FALSE;
        }
        return S_FALSE;
    }
    ch = (WCHAR)hkey;

    // hangul_automata
    if (ch >= 0x100 && ch <= 0x11ff) {
        // a hangul char
        ret = hangul_automata(&ic, ch, lcs, &ncs);
        if (ncs)
            cs = lcs[ncs - 1];
        DEBUGPRINTFEX(100, (TEXT("\thangul_automata result : %d, %s\r\n"), ncs, lcs));
        ic.lastvkey = (UINT) wParam; //(UINT)VkKeyScan();

        if (ncs)
        {
            // not composable. emit a hangul syllable
            if (ncs == 1 && ic.syllable) {
                ch = cs;
            } else {
                ch = cs; // XXX
            }
            _EndCompositionInContext(pContext, ch);
        } else if (ncs > 0) {
            // Middle-aged Hangul
            DEBUGPRINTFEX(100, (TEXT("\tmiddle-aged hangul ?\r\n")));
        }
    } else {
        // not a hangul char
        if (_IsComposing()) {
            DEBUGPRINTFEX(100, (TEXT("\tTerminateComposition\r\n")));
            ch = _HangulCommit();
            _EndCompositionInContext(pContext, ch);
            hangul_ic_init(&ic);
            return S_OK;
        }
        return S_FALSE;
    }

    if (ret <= 0) {
        // preedit state
        int ncs;
        ncs = hangul_ic_get(&ic, 0, lcs);
        if (ncs > 0)
            ch = lcs[ncs - 1];
        DEBUGPRINTFEX(100, (TEXT("\thangul_automata preedit : %d, %s\r\n"), ncs, lcs));
    } else {
        // new composition
        int ncs;
        ncs = hangul_ic_get(&ic, 0, lcs);
        ch = lcs[0];
        DEBUGPRINTFEX(100, (TEXT("\thangul_automata new preedit : %d, %s\r\n"), ncs, lcs));
    }

    hr = S_OK; // return S_FALSE to NOT eat the key

    // Start the new compositon if there is no composition.
    if (!_IsComposing())
    {
        DEBUGPRINTFEX(100, (TEXT("\tNo composition! StartCompositionInContext()\n")));
        _StartCompositionInContext(pContext, ch);
        if (ch) {
            return S_OK;
        }
    } else {
        DEBUGPRINTFEX(100, (TEXT("\tIsComposing() OK\n")));
    }

#if 1
    ITfInsertAtSelection *pInsertAtSelection;
    ITfRange *pRangeInsert;

    // first, test where a keystroke would go in the document if we did an insert
    // we need a special interface to insert text at the selection
    if (pContext->QueryInterface(IID_ITfInsertAtSelection, (void **)&pInsertAtSelection) == S_OK)
    {
        if (pInsertAtSelection->InsertTextAtSelection(ec, TF_IAS_QUERYONLY, NULL, 0, &pRangeInsert) == S_OK)
        {

            if (pRangeInsert->SetText(ec, TF_ST_CORRECTION, &ch, 1) == S_OK)
            {
                // update the selection
                TF_SELECTION tfSelection;
                tfSelection.range = pRangeInsert;
                tfSelection.style.ase = TF_AE_NONE;
                tfSelection.style.fInterimChar = TRUE; // a intermidate char
                //tfSelection.range->Collapse(ec, TF_ANCHOR_END);

                pContext->SetSelection(ec, 1, &tfSelection);

                pRangeInsert->Release();
            }
            SafeRelease(pInsertAtSelection);
        }
    }

#else
    // first, test where a keystroke would go in the document if we did an insert
    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1) {
        DEBUGPRINTFEX(100, (TEXT("\tGetSelection() failed?\n")));
        return S_FALSE;
    }

    // is the insertion point covered by a composition?
    if (_pComposition->GetRange(&pRangeComposition) == S_OK)
    {
        fCovered = IsRangeCovered(ec, tfSelection.range, pRangeComposition);

        pRangeComposition->Release();

        if (!fCovered)
        {
            hr = S_FALSE; // don't eat the key, it's outside our composition
            goto Exit;
        }
    }

    // insert the text
    // we use SetText here instead of InsertTextAtSelection because we've already started a composition
    // we don't want to the app to adjust the insertion point inside our composition
    if (tfSelection.range->SetText(ec, TF_ST_CORRECTION, &ch, 1) != S_OK)
        goto Exit;

    // update the selection, we'll make it an insertion point just past
    // the inserted text.
    //tfSelection.range->Collapse(ec, TF_ANCHOR_END);

    pContext->SetSelection(ec, 1, &tfSelection);
    tfSelection.range->Release();
#endif

    if (ret > 0) {
        DEBUGPRINTFEX(100, (TEXT("\tOK commit hangul\n")));
        WCHAR ch = _HangulCommit();
        _EndCompositionInContext(pContext, ch);
        //hangul_ic_init(&ic);
        goto Exit;
    }

    // apply our dislay attribute property to the inserted text
    // we need to apply it to the entire composition, since the
    // display attribute property is static, not static compact
    _SetCompositionDisplayAttributes(ec);

Exit:
    DEBUGPRINTFEX(100, (TEXT("\tEND CSaenaruTextService::_HandleKeyDown()\n")));
    return hr;
}

//+---------------------------------------------------------------------------
//
// _InitKeystrokeSink
//
// Advise a keystroke sink.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_InitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return FALSE;

    hr = pKeystrokeMgr->AdviseKeyEventSink(_tfClientId, (ITfKeyEventSink *)this, TRUE);

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// _UninitKeystrokeSink
//
// Unadvise a keystroke sink.  Assumes we have advised one already.
//----------------------------------------------------------------------------

void CSaenaruTextService::_UninitKeystrokeSink()
{
    ITfKeystrokeMgr *pKeystrokeMgr;
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_UninitKeystrokeSink(%p)\n"), this));

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK)
        return;

    pKeystrokeMgr->UnadviseKeyEventSink(_tfClientId);
    pKeystrokeMgr->Release();
}

BOOL CSaenaruTextService::_InitPreservedKey()
{
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_InitPreservedKey(%p)\n"), this));

    ITfKeystrokeMgr *pKeystrokeMgr;
    HRESULT hr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK) {
        return false;
    }

    // register Hangul
    hr = pKeystrokeMgr->PreserveKey(_tfClientId,
            cPreservedKey_GUID_ToggleHangul,
            &cHangul,
            ToggleDesc,
            (ULONG)wcslen(ToggleDesc));

    if (hr == S_OK) {
        // register Shift + Space
        hr = pKeystrokeMgr->PreserveKey(_tfClientId,
                cPreservedKey_GUID_ToggleHangul,
                &cShiftSpace,
                ToggleDesc,
                (ULONG)wcslen(ToggleDesc));
    }

    if (hr == S_OK) {
        // register Hanja
        hr = pKeystrokeMgr->PreserveKey(_tfClientId,
                cPreservedKey_GUID_Hanja,
                &cHanja,
                HanjaDesc,
                (ULONG)wcslen(HanjaDesc));
    }

    if (hr == S_OK) {
        hr = pKeystrokeMgr->PreserveKey(_tfClientId,
                cPreservedKey_GUID_Hanja,
                &cRControl,
                HanjaDesc,
                (ULONG)wcslen(HanjaDesc));
    }

    pKeystrokeMgr->Release();

    return (hr == S_OK);
}

void CSaenaruTextService::_UninitPreservedKey()
{
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_UninitPreservedKey(%p)\n"), this));

    ITfKeystrokeMgr *pKeystrokeMgr;

    if (_pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void **)&pKeystrokeMgr) != S_OK) {
        return;
    }

    pKeystrokeMgr->UnpreserveKey(cPreservedKey_GUID_ToggleHangul, &cShiftSpace);
    pKeystrokeMgr->UnpreserveKey(cPreservedKey_GUID_ToggleHangul, &cHangul);
    pKeystrokeMgr->UnpreserveKey(cPreservedKey_GUID_Hanja, &cRControl);
    pKeystrokeMgr->UnpreserveKey(cPreservedKey_GUID_Hanja, &cHanja);

    pKeystrokeMgr->Release();
}

//+---------------------------------------------------------------------------
//
// OnSetFocus
//
// Called by the system whenever this service gets the keystroke device focus.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnSetFocus(BOOL fForeground)
{
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// _ConvertVKey
//
//----------------------------------------------------------------------------

WORD CSaenaruTextService::_ConvertVKey(UINT code)
{
    HKL hCur;
    hCur = GetKeyboardLayout(0);
    //
    // Map virtual key to scan code
    //
    UINT scanCode = 0;
    //scanCode = MapVirtualKey(code, 0);
    scanCode = (UINT)MapVirtualKeyEx(code, 2, hCur);

    //
    // Keyboard state
    //
    BYTE abKbdState[256] = {'\0'};
    if (!GetKeyboardState(abKbdState))
    {
        return 0;
    }

    //
    // Map virtual key to character code
    //
    WORD wch = '\0';
    if (ToAscii(code, scanCode, abKbdState, &wch, 0) == 1)
    {
        return wch;
    }

    return 0;
}

//+---------------------------------------------------------------------------
//
// OnTestKeyDown
//
// Called by the system to query if this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnTestKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::OnTestKeyDown(wParam = 0x%x, %d)\n"), wParam, wParam));
    DWORD hkey = 0;
    WCHAR ch;

    *pfEaten = FALSE;

    if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_RWIN || wParam == VK_LWIN || wParam == VK_MENU)
        return S_OK;

    _fCandidateReopen = FALSE;
    if (_IsCandidateOpen())
    {
        if (_HandleCandidateKey(pContext, wParam, TRUE))
        {
            *pfEaten = TRUE;
            return S_OK;
        }
        // other keys commit the selected candidate like as the IMM IME
        if (wParam == VK_BACK)
            _CancelCandidateList(pContext);
        else
            _CommitCandidate(pContext);
    }

    ch = _ConvertVKey((UINT)wParam);
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::OnTestKeyDown() VK key ch = 0x%x, %d)\n"), ch, ch));

    // the hangul typed in the english mode
    if (_HandleEngTypo(pContext, wParam, ch, TRUE))
    {
        *pfEaten = TRUE;
        return S_OK;
    }
    if (ch == 0 && (wParam != VK_LEFT && wParam != VK_RIGHT && wParam != VK_UP && wParam != VK_DOWN))
        return S_OK;

    DWORD status = GetConversionStatus();
    if (status & CMODE_HANGUL) {
        hkey = keyToHangulKey(ch);

        if (hkey == 0 || hkey == ch) {
            if (wParam == VK_BACK && _IsComposing() && (ic.len > 1 || hangul_ic_restorable(&ic))) {
                *pfEaten = TRUE;
            } else {
                *pfEaten = _IsReconvertKey(pContext, wParam, TRUE);
            }
        } else {
            *pfEaten = TRUE;
        }

        DEBUGPRINTFEX(100, (TEXT("\thkey = %x, ch = %x\n"), hkey, ch));
    } else {
        *pfEaten = FALSE;
    }

    if (!*pfEaten && _IsComposing())
    {
        // commit previous comitted chars.
        DEBUGPRINTFEX(100, (TEXT("\tXXX No eaten\n")));
        WCHAR ch = _HangulCommit();
        _EndCompositionInContext(pContext, ch);
        hangul_ic_init(&ic);
    }

    DEBUGPRINTFEX(100, (TEXT("\tEaten = %d\n"), *pfEaten));
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyDown
//
// Called by the system to offer this service a keystroke.  If *pfEaten == TRUE
// on exit, the application will not handle the keystroke.
//
// This text service is interested in handling keystrokes to demonstrate the
// use the compositions.  Some apps will cancel compositions if they receive
// keystrokes while a compositions is ongoing.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnKeyDown(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    CKeystrokeEditSession *pEditSession;
    HRESULT hr;
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::OnKeyDown(wParam = 0x%x, %d)\n"), wParam, wParam));

    hr = E_FAIL;
    *pfEaten = FALSE;

    DWORD hkey = 0;
    WCHAR ch;
    BOOL fReconvert = FALSE;

    if (wParam == VK_SHIFT || wParam == VK_CONTROL || wParam == VK_RWIN || wParam == VK_LWIN || wParam == VK_MENU)
        return S_OK;

    if (_IsCandidateOpen())
    {
        if (_HandleCandidateKey(pContext, wParam, FALSE))
        {
            *pfEaten = TRUE;
            return S_OK;
        }
        if (wParam == VK_BACK)
            _CancelCandidateList(pContext);
        else
            _CommitCandidate(pContext);
    }

    ch = _ConvertVKey((UINT)wParam);
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::OnKeyDown() VK key ch = 0x%x, %d)\n"), ch, ch));

    if (_HandleEngTypo(pContext, wParam, ch, FALSE))
    {
        *pfEaten = TRUE;
        return S_OK;
    }

    // FIXME
    if (ch == 0 && (wParam != VK_LEFT && wParam != VK_RIGHT && wParam != VK_UP && wParam != VK_DOWN))
        return S_OK;

    DWORD status = GetConversionStatus();
    if (status & CMODE_HANGUL) {
        if (wParam == VK_SHIFT)
            return S_OK;
        hkey = keyToHangulKey(ch);

        if (hkey == 0 || hkey == ch) {
            if (wParam == VK_BACK && _IsComposing() && (ic.len > 1 || hangul_ic_restorable(&ic))) {
                *pfEaten = TRUE;
            } else {
                *pfEaten = fReconvert = _IsReconvertKey(pContext, wParam, FALSE);
            }
        } else {
            *pfEaten = TRUE;
        }

        DEBUGPRINTFEX(100, (TEXT("\thkey = %x, ch = %x\n"), hkey, ch));
    } else {
        *pfEaten = FALSE;
    }

    if (*pfEaten) // only process keys while composing
    {
        // we'll insert a char ourselves in place of this keystroke
        if ((pEditSession = new CKeystrokeEditSession(this, pContext, wParam)) == NULL)
            goto Exit;

        // we need a lock to do our work
        // nb: this method is one of the few places where it is legal to use
        // the TF_ES_SYNC flag
        if (pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr) != S_OK)
        {
            hr = E_FAIL;
        }

        pEditSession->Release();
    } else {
        if (_IsComposing()) {
            DEBUGPRINTFEX(100, (TEXT("\tTerminate composition\n")));
            WCHAR ch = _HangulCommit();
            _EndCompositionInContext(pContext, ch);
            hangul_ic_init(&ic);
        } else {
            DEBUGPRINTFEX(100, (TEXT("\tNo composition\n")));
        }
    }

Exit:
    // if we made it all the way to the RequestEditSession, then hr is ultimately the
    // return code from CKeystrokeEditSession::DoEditSession.  Our DoEditSession method
    // return S_OK to signal that the keystroke should be eaten, S_FALSE otherwise.
    if (hr == S_OK)
    {
        *pfEaten = TRUE;
    }
    else if (fReconvert)
    {
        *pfEaten = FALSE; // not reconverted. the application deletes the char
    }

    // reopen the list for the new syllable like as DicKeydownHandler()
    if (_fCandidateReopen)
    {
        _fCandidateReopen = FALSE;
        if (wParam == VK_BACK && _IsComposing() && ic.len > 1)
            _OpenCandidateList(pContext);
    }
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// DoEditSession
//
//----------------------------------------------------------------------------

STDAPI CKeystrokeEditSession::DoEditSession(TfEditCookie ec)
{
    DEBUGPRINTFEX(100, (TEXT("CKeystrokeEditSession::DoEditSession()\n")));
    switch (_wParam)
    {
        case VK_LEFT:
        case VK_RIGHT:
            return _pSaenaru->_HandleArrowKey(ec, _pContext, _wParam);

        case VK_RETURN:
            return _pSaenaru->_HandleReturn(ec, _pContext);

        case VK_BACK:
            return _pSaenaru->_HandleBackspace(ec, _pContext);

        case VK_SPACE:
            return S_OK;
    }

    return _pSaenaru->_HandleKeyDown(ec, _pContext, _wParam);
}

//+---------------------------------------------------------------------------
//
// OnTestKeyUp
//
// Called by the system to query this service wants a potential keystroke.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnTestKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnKeyUp
//
// Called by the system to offer this service a keystroke.  If *pfEaten == TRUE
// on exit, the application will not handle the keystroke.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnKeyUp(ITfContext *pContext, WPARAM wParam, LPARAM lParam, BOOL *pfEaten)
{
    *pfEaten = FALSE;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// OnPreservedKey
//
// Called when a hotkey (registered by us, or by the system) is typed.
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::OnPreservedKey(ITfContext *pContext, REFGUID rguid, BOOL *pfEaten)
{
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::OnPreservedKey(%p)\n"), this));
    if (IsEqualGUID(rguid, cPreservedKey_GUID_ToggleHangul)) {
        DEBUGPRINTFEX(100, (TEXT("\tHangulKey\n")));

        if (pContext != NULL) {
            DWORD status = GetConversionStatus();
            status ^= CMODE_HANGUL;

            if (_pLangBarItem) {
                _pLangBarItem->_OnUpdate(TF_LBI_STATUS | TF_LBI_ICON);
            }

            // commit the selected hanja first
            _CommitCandidate(pContext);
            _ResetEngTypo();

            if (_IsComposing()) {
                DEBUGPRINTFEX(100, (TEXT("\tIsComposing\n")));
                WCHAR ch = _HangulCommit();
                _EndCompositionInContext(pContext, ch);
                DEBUGPRINTFEX(100, (TEXT("\tEndComposition\n")));
            }
            SetConversionStatus(status);

            if (status & CMODE_HANGUL) {
                hangul_ic_init(&ic);
                DEBUGPRINTFEX(100, (TEXT("\thangul_ic_init()\n")));
            }

            *pfEaten = TRUE;
        } else {
            DEBUGPRINTFEX(100, (TEXT("\tpContext == NULL ?\n")));
            *pfEaten = FALSE;
        }
    } else if (IsEqualGUID(rguid, cPreservedKey_GUID_Hanja)) {
        DEBUGPRINTFEX(100, (TEXT("\tHanjaKey\n")));

        *pfEaten = FALSE;
        if (pContext != NULL) {
            if (_IsCandidateOpen()) {
                // next candidate
                *pfEaten = _HandleCandidateKey(pContext, VK_HANJA, FALSE);
            } else if ((GetConversionStatus() & CMODE_HANGUL) && _IsComposing()) {
                *pfEaten = _OpenCandidateList(pContext);
            } else if (GetConversionStatus() & CMODE_HANGUL) {
                // convert the selected word like as the reconversion of the IMM IME
                HRESULT hr = _RequestCandidateEditSession(pContext, CAND_ES_RECONVERT, NULL, TRUE);
                *pfEaten = (hr == S_OK || hr == TF_S_ASYNC);
            }
        }
    } else {
        *pfEaten = FALSE;
    }
    return S_OK;
}

/*
 * vim: et sts=4 sw=4
 */
