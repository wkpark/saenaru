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
    CEngTypoEditSession(CSaenaruTextService *pSaenaru, ITfContext *pContext, BOOL fUndo) : CEditSessionBase(pContext)
    {
        _pSaenaru = pSaenaru;
        _pSaenaru->AddRef();
        _fUndo = fUndo;
    }
    ~CEngTypoEditSession()
    {
        _pSaenaru->Release();
    }

    // ITfEditSession
    STDMETHODIMP DoEditSession(TfEditCookie ec)
    {
        return _pSaenaru->_DoEngTypo(ec, _pContext, _fUndo);
    }

private:
    CSaenaruTextService *_pSaenaru;
    BOOL _fUndo;
};

static BOOL IsCtrlOrAlt()
{
    return (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000);
}

//+---------------------------------------------------------------------------
//
// LoadEngWords
//
// engwords.dic for the hangul mode, loaded at the first use.
//----------------------------------------------------------------------------

static void LoadEngWords()
{
    static BOOL s_fLoaded = FALSE;
    TCHAR szPath[MAX_PATH];
    int len;

    if (s_fLoaded)
        return;

    EnterCriticalSection(&g_cs);
    if (!s_fLoaded)
    {
        len = GetSaenaruDirectory(szPath, MAX_PATH - 16);
        if (len > 0 && szPath[len - 1] != TEXT('\\'))
            szPath[len++] = TEXT('\\');
        lstrcpy(szPath + len, TEXT("engwords.dic"));
        if (hangul_load_eng_words(szPath))
        {
            DEBUGPRINTFEX(100, (TEXT("LoadEngWords() %s loaded\n"), szPath));
        }
        else
        {
            DEBUGPRINTFEX(100, (TEXT("LoadEngWords() %s not loaded\n"), szPath));
        }
        s_fLoaded = TRUE;
    }
    LeaveCriticalSection(&g_cs);
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
                    case IS_NUMERIC_PASSWORD:
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

    if (!(dwOptionFlag & HANENG_AUTOCORRECT))
        return FALSE;

    fHangul = (GetConversionStatus() & CMODE_HANGUL) != 0;
    if (fHangul && ENGTYPO_HAN_TO_ENG)
        LoadEngWords();

#ifdef DEBUG
    if (wParam == VK_SPACE)
    {
        WCHAR achHan[ENGTYPO_MAXLEN + 1];

        hangul_typo_check(&_typo, fHangul, achHan, ARRAYSIZE(achHan));
        DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo(%d) <SPACE> hangul = %d, word = %s, off = %d, typo = %s\n"),
                            fTest, fHangul, _typo.word, _typo.off, achHan));
    }
#endif

    action = hangul_typo_key(&_typo, (UINT)wParam, ch, fHangul, _IsComposing(),
                             IsCtrlOrAlt(), (GetKeyState(VK_SHIFT) & 0x8000) != 0, fTest);
    if (action == HANGUL_TYPO_NONE)
        return FALSE;
    if (fTest)
        return TRUE;

    if (action == HANGUL_TYPO_UNDO)
    {
        hr = _RequestEngTypoEditSession(pContext, TRUE);
        DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() undo hr = 0x%x\n"), hr));
        if (hr != S_OK)
            return FALSE; // not the same text. the application deletes the char.
        _SetHangulMode(_typo.toEng);
        return TRUE;
    }

    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() convert %s\n"), _typo.word));
    hr = _RequestEngTypoEditSession(pContext, FALSE);
    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() convert hr = 0x%x\n"), hr));
    if (hr != S_OK)
        hangul_typo_end(&_typo, fHangul, NULL);
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

HRESULT CSaenaruTextService::_RequestEngTypoEditSession(ITfContext *pContext, BOOL fUndo)
{
    CEngTypoEditSession *pEditSession;
    HRESULT hr = E_FAIL;

    if ((pEditSession = new CEngTypoEditSession(this, pContext, fUndo)) == NULL)
        return E_FAIL;
    if (pContext->RequestEditSession(_tfClientId, pEditSession, TF_ES_SYNC | TF_ES_READWRITE, &hr) != S_OK)
        hr = E_FAIL;
    pEditSession->Release();
    return hr;
}

//+---------------------------------------------------------------------------
//
// _DoEngTypo
//
// Convert the word before the caret, or restore it. If the text is not the
// same, insert the space for the conversion, and nothing for the undo.
// Returns S_OK if converted, S_FALSE if not.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_DoEngTypo(TfEditCookie ec, ITfContext *pContext, BOOL fUndo)
{
    WCHAR ach[ENGTYPO_MAXLEN + 3];
    WCHAR achHan[ENGTYPO_MAXLEN + 1];
    WCHAR achNew[ENGTYPO_MAXLEN + 2];
    TF_SELECTION tfSelection;
    ITfRange *pRange, *pRangeComposition = NULL;
    LPCWSTR pszTyped, pszNew;
    ULONG cFetched;
    LONG cchShift;
    BOOL fHangul, fEmpty, fConvert = FALSE;
    int len;
    HRESULT hr = E_FAIL;

    fHangul = (GetConversionStatus() & CMODE_HANGUL) != 0;

    // the syllable composing is a part of the word
    if (_IsComposing())
    {
        if (_pComposition->GetRange(&pRangeComposition) != S_OK)
            pRangeComposition = NULL;
        _TerminateComposition(ec);
        hangul_ic_init(&ic);
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

    if (pRange->IsEmpty(ec, &fEmpty) != S_OK)
        goto Exit;

    // the text typed before the caret and the new text
    pszTyped = pszNew = NULL;
    if (fUndo)
    {
        pszTyped = _typo.toEng ? _typo.eng : _typo.han;
        pszNew = _typo.toEng ? _typo.han : _typo.eng;
    }
    else if (hangul_typo_check(&_typo, fHangul, achHan, ARRAYSIZE(achHan)) > 0)
    {
        pszTyped = fHangul ? achHan : _typo.word;
        pszNew = fHangul ? _typo.word : achHan;
    }

    // one more char before the word: the space of the undo, or the start of
    // the word. the number of the chars is checked by GetText(), not by the shift.
    if (fEmpty && pszTyped != NULL)
    {
        len = lstrlenW(pszTyped);
        if (pRange->ShiftStart(ec, -(len + 1), &cchShift, NULL) == S_OK &&
                pRange->GetText(ec, 0, ach, ARRAYSIZE(ach) - 1, &cFetched) == S_OK)
        {
            ach[cFetched] = L'\0';
            DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoEngTypo(%d) text = %s, typed = %s\n"), fUndo, ach, pszTyped));
            fConvert = hangul_typo_match(ach, pszTyped, fUndo) &&
                    (fUndo || (pRange->ShiftStart(ec, (LONG)cFetched - len, &cchShift, NULL) == S_OK &&
                               (fHangul || !IsEngInputScope(ec, pContext, pRange))));
        }
    }

    if (fConvert)
    {
        lstrcpyW(achNew, pszNew);
        lstrcatW(achNew, L" ");
        if (pRange->SetText(ec, 0, achNew, lstrlenW(achNew)) != S_OK)
            goto Exit;
        if (!fUndo)
            hangul_typo_end(&_typo, fHangul, achHan);
        hr = S_OK;
    }
    else if (fUndo)
    {
        // the application deletes the char
        hr = S_FALSE;
        goto Exit;
    }
    else
    {
        // not the same text. insert the space for the key.
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
