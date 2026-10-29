//
// typoconv.cpp
//
// The hangul typed in the english mode, and the english typed in the hangul
// mode. At <SPACE>, the word before the caret like as "dkssud" is converted
// to "안녕" and the hangul mode is turned on, or "ㅗ듸ㅐ" to "hello" and the
// english mode. <BACKSPACE> just after it restores the word and the mode.
// The state of the word is hangul_typo_key() of engtypo.c.
//

#include "globals.h"
#include "saenarutip.h"
#include "editsess.h"
#include <initguid.h>
#include <InputScope.h>

extern "C" {
#include "hangul.h"
int PASCAL GetSaenaruDirectory(LPTSTR lpDest, int max);
}

class CEngTypoEditSession : public CEditSessionBase
{
public:
    CEngTypoEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext, BOOL fUndo, BOOL fCheck) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
        _fUndo = fUndo;
        _fCheck = fCheck;
    }
    ~CEngTypoEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        return _pSaenaru->_DoEngTypo(ec, _pContext, _fUndo, _fCheck);
    }

private:
    CSaenaruTextService *_pSaenaru;
    BOOL _fUndo;
    BOOL _fCheck;
};

static BOOL IsCtrlOrAlt()
{
    return (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000);
}

//+---------------------------------------------------------------------------
//
// LoadEngWords
//
// engwords.dic for both of the directions, mapped at the first <SPACE>.
// If it is not loaded, it is tried again a minute later, not at every <SPACE>.
//----------------------------------------------------------------------------

static void LoadEngWords()
{
    static BOOL s_fLoaded = FALSE;
    static DWORD s_dwTried = 0; // the tick of the last try, 0 for none
    TCHAR szPath[MAX_PATH + 32]; // GetSaenaruDirectory() does not check the size
    int len;

    if (s_fLoaded || (s_dwTried != 0 && GetTickCount() - s_dwTried < 60 * 1000))
        return;

    EnterCriticalSection(&g_cs);
    if (!s_fLoaded && (s_dwTried == 0 || GetTickCount() - s_dwTried >= 60 * 1000))
    {
        s_dwTried = GetTickCount() | 1;
        len = GetSaenaruDirectory(szPath, MAX_PATH);
        if (len > 0 && len < MAX_PATH)
        {
            if (szPath[len - 1] != TEXT('\\'))
                szPath[len++] = TEXT('\\');
            lstrcpy(szPath + len, TEXT("engwords.dic"));
            s_fLoaded = hangul_load_eng_words(szPath);
            DEBUGPRINTFEX(100, (TEXT("LoadEngWords() %s loaded = %d\n"), szPath, s_fLoaded));
        }
    }
    LeaveCriticalSection(&g_cs);
}

//+---------------------------------------------------------------------------
//
// IsKeyboardDisabled
//
// The keyboard disabled by the application, like as ImmAssociateContext(NULL).
// The open/close compartment is not checked. It is closed in the english mode.
//----------------------------------------------------------------------------

static BOOL IsKeyboardDisabled(ITfContext *pContext)
{
    static const GUID *c_rgGuid[] = {&GUID_COMPARTMENT_KEYBOARD_DISABLED, &GUID_COMPARTMENT_EMPTYCONTEXT};
    ITfCompartmentMgr *pCompartmentMgr;
    ITfCompartment *pCompartment;
    VARIANT var;
    BOOL fDisabled = FALSE;
    UINT i;

    if (pContext == NULL || pContext->QueryInterface(IID_ITfCompartmentMgr, (void **)&pCompartmentMgr) != S_OK)
        return FALSE;
    for (i = 0; i < ARRAYSIZE(c_rgGuid) && !fDisabled; i++)
    {
        if (pCompartmentMgr->GetCompartment(*c_rgGuid[i], &pCompartment) != S_OK)
            continue;
        VariantInit(&var);
        if (pCompartment->GetValue(&var) == S_OK && var.vt == VT_I4 && var.lVal != 0)
            fDisabled = TRUE;
        VariantClear(&var);
        pCompartment->Release();
    }
    pCompartmentMgr->Release();
    return fDisabled;
}

//+---------------------------------------------------------------------------
//
// IsEngInputScope
//
// URL, mail address, password and so on are in english.
//----------------------------------------------------------------------------

static BOOL IsEngInputScope(TfEditCookie ec, ITfContext *pContext, ITfRange *pRange)
{
    ITfProperty *pProperty;
    ITfInputScope *pInputScope;
    InputScope *pScopes;
    VARIANT var;
    UINT i, uCount;
    BOOL fEng = FALSE;

    if (pContext->GetProperty(GUID_PROP_INPUTSCOPE, &pProperty) != S_OK)
        return FALSE;

    VariantInit(&var);
    if (pProperty->GetValue(ec, pRange, &var) == S_OK && var.vt == VT_UNKNOWN && var.punkVal != NULL &&
            var.punkVal->QueryInterface(IID_ITfInputScope, (void **)&pInputScope) == S_OK)
    {
        if (pInputScope->GetInputScopes(&pScopes, &uCount) == S_OK)
        {
            for (i = 0; i < uCount; i++)
            {
                switch (pScopes[i])
                {
                    case IS_URL:
                    case IS_FILE_FULLFILEPATH:
                    case IS_FILE_FILENAME:
                    case IS_EMAIL_USERNAME:
                    case IS_EMAIL_SMTPEMAILADDRESS:
                    case IS_LOGINNAME:
                    case IS_PASSWORD:
                    case IS_ALPHANUMERIC_HALFWIDTH:
                    case IS_ALPHANUMERIC_FULLWIDTH:
                    case IS_NUMERIC_PASSWORD:
                    case IS_EMAILNAME_OR_ADDRESS:
                    case IS_ALPHANUMERIC_PIN:
                    case IS_ALPHANUMERIC_PIN_SET:
                        fEng = TRUE;
                        break;
                    default:
                        break;
                }
            }
            CoTaskMemFree(pScopes);
        }
        pInputScope->Release();
    }
    VariantClear(&var);
    pProperty->Release();
    return fEng;
}

//+---------------------------------------------------------------------------
//
// _HandleEngTypo
//
// Called by OnTestKeyDown() (fTest) and OnKeyDown(). Returns TRUE to eat the key.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_HandleEngTypo(ITfContext *pContext, WPARAM wParam, WCHAR ch, BOOL fTest)
{
    BOOL fHangul;
    HRESULT hr;
    int action;

    // only <SPACE> and <BACKSPACE> are eaten. no edit session without a context.
    if (pContext == NULL || !(dwOptionFlag & HANENG_AUTOCORRECT) ||
            ((wParam == VK_SPACE || wParam == VK_BACK) && IsKeyboardDisabled(pContext)))
    {
        // a new word when it is turned on again, also by the option dialog
        _ResetEngTypo();
        return FALSE;
    }

    fHangul = (GetConversionStatus() & CMODE_HANGUL) != 0;
    if (wParam == VK_SPACE)
        LoadEngWords();

    // not the letters typed, they may be a password
    if (wParam == VK_SPACE)
        DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo(%d) <SPACE> hangul = %d, len = %d, off = %d\n"),
                            fTest, fHangul, _typo.len, _typo.off));

    action = hangul_typo_key(&_typo, (UINT)wParam, ch, fHangul, _IsComposing(),
                             IsCtrlOrAlt(), (GetKeyState(VK_SHIFT) & 0x8000) != 0, fTest);
    if (action == HANGUL_TYPO_NONE)
        return FALSE;
    if (fTest)
    {
        // eaten only for the text before the caret. the caret may be moved
        // by the mouse, and the field may be in english. not eaten without
        // the check, like as _IsReconvertKey().
        if (_RequestEngTypoEditSession(pContext, action == HANGUL_TYPO_UNDO, TRUE) == S_OK)
            return TRUE;
        if (action == HANGUL_TYPO_UNDO)
        {
            // not the undo, the <BACKSPACE> of the word
            _typo.undo = FALSE;
            hangul_typo_key(&_typo, (UINT)wParam, ch, fHangul, FALSE, FALSE, FALSE, fTest);
        }
        else
        {
            // not converted, and the application gets the <SPACE>
            hangul_typo_end(&_typo, FALSE);
        }
        return FALSE;
    }

    if (action == HANGUL_TYPO_UNDO)
    {
        hr = _RequestEngTypoEditSession(pContext, TRUE, FALSE);
        DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() undo hr = 0x%x\n"), hr));
        // S_FALSE: not the same text, and the application deletes the char
        if (hr == S_OK)
            _SetHangulMode(_typo.toEng);
        else
            hangul_typo_key(&_typo, (UINT)wParam, ch, fHangul, FALSE, FALSE, FALSE, fTest);
        return hr == S_OK;
    }

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() convert len = %d\n"), _typo.len));
    hr = _RequestEngTypoEditSession(pContext, FALSE, FALSE);
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() convert hr = 0x%x\n"), hr));
    if (hr != S_OK)
        hangul_typo_end(&_typo, FALSE);
    else
        _SetHangulMode(!fHangul);
    return hr == S_OK || hr == S_FALSE;
}

void CSaenaruTextService::_ResetEngTypo()
{
    hangul_typo_init(&_typo);
}

void CSaenaruTextService::_SetHangulMode(BOOL fHangul)
{
    DWORD status = GetConversionStatus();

    if (fHangul)
        status |= CMODE_HANGUL;
    else
        status &= ~CMODE_HANGUL;
    SetConversionStatus(status);
    hangul_ic_init(&ic);
    _UpdateLanguageBar(TF_LBI_STATUS | TF_LBI_ICON);
}

HRESULT CSaenaruTextService::_RequestEngTypoEditSession(ITfContext *pContext, BOOL fUndo, BOOL fCheck)
{
    CEngTypoEditSession *pEditSession;
    HRESULT hr = E_FAIL;

    if ((pEditSession = new CEngTypoEditSession(this, pContext, fUndo, fCheck)) == NULL)
        return E_FAIL;
    if (pContext->RequestEditSession(_tfClientId, pEditSession,
                                     TF_ES_SYNC | (fCheck ? TF_ES_READ : TF_ES_READWRITE), &hr) != S_OK)
        hr = E_FAIL;
    pEditSession->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// _DoEngTypo
//
// Convert the word before the caret, or restore it. fCheck only checks the
// text in the read only session for OnTestKeyDown() to eat the key. If the
// text is not the same after it, do the key eaten like as the application:
// insert the space for the conversion. The key of the undo is not eaten.
// Returns S_OK if converted, S_FALSE if not.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_DoEngTypo(TfEditCookie ec, ITfContext *pContext, BOOL fUndo, BOOL fCheck)
{
    WCHAR ach[ENGTYPO_MAXLEN + 3];
    WCHAR achNew[ENGTYPO_MAXLEN + 2];
    TF_SELECTION tfSelection;
    ITfRange *pRange, *pRangeComposition = NULL;
    LPCWSTR pszTyped, pszNew;
    ULONG cFetched;
    LONG cchShift;
    BOOL fHangul, fComposing, fEmpty, fConvert = FALSE;
    int len;
    HRESULT hr = E_FAIL;

    fHangul = (GetConversionStatus() & CMODE_HANGUL) != 0;

    // the syllable composing is a part of the word
    fComposing = _IsComposing();
    if (fComposing)
    {
        if (_pComposition->GetRange(&pRangeComposition) != S_OK)
            pRangeComposition = NULL;
        if (!fCheck)
        {
            _TerminateComposition(ec);
            hangul_ic_init(&ic);
        }
    }

    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
    {
        SafeRelease(pRangeComposition);
        return E_FAIL;
    }
    pRange = tfSelection.range;

    // the word ends at the syllable composed. the selection may cover it
    // (fInterimChar) or may be the caret before it.
    if (pRangeComposition != NULL)
    {
        pRange->ShiftEndToRange(ec, pRangeComposition, TF_ANCHOR_END);
        pRange->Collapse(ec, TF_ANCHOR_END);
        pRangeComposition->Release();
    }
    else if (fComposing)
    {
        // no range of the composition. the selection covering it.
        pRange->Collapse(ec, TF_ANCHOR_END);
    }

    if (pRange->IsEmpty(ec, &fEmpty) != S_OK)
        goto Exit;

    // the text typed before the caret and the new text. the word is checked
    // at <SPACE> by hangul_typo_key() just before it.
    if (fUndo)
    {
        pszTyped = _typo.toEng ? _typo.eng : _typo.han;
        pszNew = _typo.toEng ? _typo.han : _typo.eng;
    }
    else
    {
        pszTyped = fHangul ? _typo.typo : _typo.word;
        pszNew = fHangul ? _typo.word : _typo.typo;
    }

    // one more char before the word: the space of the undo, or the start of
    // the word. the number of the chars is checked by GetText(), not by the shift.
    if (fEmpty)
    {
        len = lstrlenW(pszTyped);
        if (pRange->ShiftStart(ec, -(len + 1), &cchShift, NULL) == S_OK &&
                pRange->GetText(ec, 0, ach, ARRAYSIZE(ach) - 1, &cFetched) == S_OK)
        {
            ach[cFetched] = L'\0';
            DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoEngTypo(%d) text = %d, typed = %d chars\n"), fUndo, cFetched, len));
            fConvert = hangul_typo_match(ach, pszTyped, fUndo) &&
                    (fUndo || (pRange->ShiftStart(ec, (LONG)cFetched - len, &cchShift, NULL) == S_OK &&
                               (fHangul || !IsEngInputScope(ec, pContext, pRange))));
        }
    }

    // nothing is changed by the check. the undo not the same is not eaten,
    // and the application deletes the char.
    if (fCheck || (fUndo && !fConvert))
    {
        hr = fConvert ? S_OK : S_FALSE;
        goto Exit;
    }

    if (fConvert)
    {
        lstrcpyW(achNew, pszNew);
        lstrcatW(achNew, L" ");
        if (pRange->SetText(ec, 0, achNew, lstrlenW(achNew)) != S_OK)
            goto Exit;
        if (!fUndo)
            hangul_typo_end(&_typo, TRUE);
        hr = S_OK;
    }
    else
    {
        // not the same text after the check. do the key eaten: insert the space.
        if (fEmpty)
            pRange->Collapse(ec, TF_ANCHOR_END);
        if (pRange->SetText(ec, 0, L" ", 1) != S_OK)
            goto Exit;
        hr = S_FALSE;
    }

    pRange->Collapse(ec, TF_ANCHOR_END);
    tfSelection.style.ase = TF_AE_NONE;
    tfSelection.style.fInterimChar = FALSE;
    pContext->SetSelection(ec, 1, &tfSelection);

Exit:
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoEngTypo(%d) hr = 0x%x\n"), fUndo, hr));
    pRange->Release();
    return hr;
}

/*
 * vim: et sts=4 sw=4
 */
