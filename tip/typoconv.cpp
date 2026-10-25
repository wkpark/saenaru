//
// typoconv.cpp
//
// The hangul typed in the english mode, and the english typed in the hangul
// mode. At <SPACE>, the word before the caret like as "dkssud" is converted
// to "안녕" and the hangul mode is turned on, or "ㅗ듸ㅐ" to "hello" and the
// english mode. <BACKSPACE> just after it restores the word and the mode.
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

C_ASSERT(ENGTYPO_BUFSIZE == ENGTYPO_MAXLEN + 1);

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

static BOOL IsAsciiLetter(WCHAR c)
{
    return (c >= L'a' && c <= L'z') || (c >= L'A' && c <= L'Z');
}

static BOOL IsWordChar(WCHAR c)
{
    return IsAsciiLetter(c) || (c >= L'0' && c <= L'9') || c == L'_';
}

static BOOL IsCtrlOrAlt()
{
    return (GetKeyState(VK_CONTROL) & 0x8000) || (GetKeyState(VK_MENU) & 0x8000);
}

//+---------------------------------------------------------------------------
//
// HanTypo
//
// hangul_han_typo() with engwords.dic loaded at the first use.
//----------------------------------------------------------------------------

static int HanTypo(LPCWSTR pszEng, LPWSTR pszHan, int cch)
{
    static BOOL s_fLoaded = FALSE;
    TCHAR szPath[MAX_PATH];
    int len;

    if (!s_fLoaded)
    {
        EnterCriticalSection(&g_cs);
        if (!s_fLoaded)
        {
            len = GetSaenaruDirectory(szPath, MAX_PATH - 16);
            if (len > 0 && szPath[len - 1] != TEXT('\\'))
                szPath[len++] = TEXT('\\');
            lstrcpy(szPath + len, TEXT("engwords.dic"));
            if (!hangul_load_eng_words(szPath))
                DEBUGPRINTFEX(100, (TEXT("HanTypo() %s not loaded\n"), szPath));
            s_fLoaded = TRUE;
        }
        LeaveCriticalSection(&g_cs);
    }
    return hangul_han_typo(pszEng, pszHan, cch);
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
// Called by OnTestKeyDown() and OnKeyDown(). OnKeyDown() is called only for
// the keys eaten by OnTestKeyDown(). OnTestKeyDown() only tracks the letters
// of the word without an edit session, and gives the same answer if it is
// called again for the key. OnKeyDown() does the conversion.
// The letters are eaten in the hangul mode, so OnKeyDown() tracks them.
// Returns TRUE to eat the key.
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::_HandleEngTypo(ITfContext *pContext, WPARAM wParam, WCHAR ch, BOOL fTest)
{
    WCHAR achHan[ENGTYPO_BUFSIZE];
    BOOL fHangul;
    HRESULT hr;

    if (!(dwOptionFlag & HANENG_AUTOCORRECT))
        return FALSE;

    fHangul = (GetConversionStatus() & CMODE_HANGUL) != 0;

    // <BACKSPACE> just after the conversion restores the word and the mode
    if (wParam == VK_BACK && _fEngTypoUndo)
    {
        if (fHangul == _fEngTypoToEng || _IsComposing() || IsCtrlOrAlt() || (GetKeyState(VK_SHIFT) & 0x8000))
        {
            _fEngTypoUndo = FALSE;
            return FALSE;
        }
        if (fTest)
            return TRUE;

        _fEngTypoUndo = FALSE;
        hr = _RequestEngTypoEditSession(pContext, TRUE);
        DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() undo hr = 0x%x\n"), hr));
        if (hr == S_OK)
            _SetHangulMode(_fEngTypoToEng);
        return hr == S_OK || hr == S_FALSE;
    }
    if (wParam != VK_SPACE)
        _fEngTypoUndo = FALSE;

    if (IsCtrlOrAlt())
    {
        // shortcuts like as <CTRL>-<V>
        _cchEngTypo = 0;
        _achEngTypo[0] = L'\0';
        _fEngTypoOff = TRUE;
        return FALSE;
    }

    switch (wParam)
    {
        case VK_SPACE:
            if (fTest)
            {
                if (!_fEngTypoOff && _cchEngTypo > 0 &&
                        (fHangul ? HanTypo(_achEngTypo, achHan, ARRAYSIZE(achHan)) :
                         hangul_eng_typo(_achEngTypo, achHan, ARRAYSIZE(achHan))) > 0)
                {
                    DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() %s -> %s\n"), _achEngTypo, achHan));
                    return TRUE;
                }
                DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() not a typo: %s, off = %d\n"), _achEngTypo, _fEngTypoOff));
                // a new word
                _cchEngTypo = 0;
                _achEngTypo[0] = L'\0';
                _fEngTypoOff = FALSE;
                return FALSE;
            }

            hr = _RequestEngTypoEditSession(pContext, FALSE);
            DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_HandleEngTypo() convert hr = 0x%x\n"), hr));
            _cchEngTypo = 0;
            _achEngTypo[0] = L'\0';
            _fEngTypoOff = FALSE;
            if (hr == S_OK)
            {
                _fEngTypoUndo = TRUE;
                _SetHangulMode(!fHangul);
            }
            return hr == S_OK || hr == S_FALSE;

        case VK_RETURN:
        case VK_TAB:
            // a new word
            _cchEngTypo = 0;
            _achEngTypo[0] = L'\0';
            _fEngTypoOff = FALSE;
            break;

        case VK_BACK:
            if (fHangul)
            {
                // a jamo is deleted, not a key
                _cchEngTypo = 0;
                _achEngTypo[0] = L'\0';
                _fEngTypoOff = TRUE;
            }
            else if (fTest)
            {
                // a typo fixed in the word
                if (_cchEngTypo > 0)
                    _achEngTypo[--_cchEngTypo] = L'\0';
                else
                    _fEngTypoOff = TRUE; // back in the previous word
            }
            break;

        case VK_LEFT:
        case VK_RIGHT:
        case VK_UP:
        case VK_DOWN:
        case VK_HOME:
        case VK_END:
        case VK_PRIOR:
        case VK_NEXT:
        case VK_DELETE:
        case VK_INSERT:
        case VK_ESCAPE:
            // the user moved the caret on purpose. not this word.
            _cchEngTypo = 0;
            _achEngTypo[0] = L'\0';
            _fEngTypoOff = TRUE;
            break;

        default:
            if (IsAsciiLetter(ch))
            {
                // OnKeyDown() for the letters eaten in the hangul mode
                if (fTest == fHangul)
                    break;
                if (_cchEngTypo < ENGTYPO_MAXLEN)
                {
                    _achEngTypo[_cchEngTypo++] = ch;
                    _achEngTypo[_cchEngTypo] = L'\0';
                }
                else
                {
                    _fEngTypoOff = TRUE; // too long
                }
            }
            else if (ch != 0 && _cchEngTypo > 0)
            {
                _fEngTypoOff = TRUE; // digits, punctuations in the word
            }
            break;
    }
    return FALSE;
}

void CSaenaruTextService::_ResetEngTypo()
{
    _cchEngTypo = 0;
    _achEngTypo[0] = L'\0';
    _fEngTypoOff = FALSE;
    _fEngTypoUndo = FALSE;
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
// same, do the key itself: insert the space, or delete a char.
// Returns S_OK if converted, S_FALSE if the key is done.
//----------------------------------------------------------------------------

HRESULT CSaenaruTextService::_DoEngTypo(TfEditCookie ec, ITfContext *pContext, BOOL fUndo)
{
    WCHAR ach[ENGTYPO_BUFSIZE + 2];
    WCHAR achHan[ENGTYPO_BUFSIZE];
    WCHAR achNew[ENGTYPO_BUFSIZE + 1];
    TF_SELECTION tfSelection;
    ITfRange *pRange;
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
        _TerminateComposition(ec);
        hangul_ic_init(&ic);
    }

    if (pContext->GetSelection(ec, TF_DEFAULT_SELECTION, 1, &tfSelection, &cFetched) != S_OK || cFetched != 1)
        return E_FAIL;
    pRange = tfSelection.range;

    if (pRange->IsEmpty(ec, &fEmpty) != S_OK)
        goto Exit;

    // the text typed before the caret and the new text
    pszTyped = pszNew = NULL;
    if (fUndo)
    {
        pszTyped = _fEngTypoToEng ? _achEngTypoEng : _achEngTypoHan;
        pszNew = _fEngTypoToEng ? _achEngTypoHan : _achEngTypoEng;
    }
    else if (_cchEngTypo > 0 &&
            (fHangul ? HanTypo(_achEngTypo, achHan, ARRAYSIZE(achHan)) :
             hangul_eng_typo(_achEngTypo, achHan, ARRAYSIZE(achHan))) > 0)
    {
        pszTyped = fHangul ? achHan : _achEngTypo;
        pszNew = fHangul ? _achEngTypo : achHan;
    }

    // the number of the chars is checked by GetText(), not by the shift
    if (fEmpty && pszTyped != NULL)
    {
        if (fUndo)
        {
            // the word and the space inserted by the conversion
            len = lstrlenW(pszTyped) + 1;
            if (pRange->ShiftStart(ec, -len, &cchShift, NULL) == S_OK &&
                    pRange->GetText(ec, 0, ach, ARRAYSIZE(ach) - 1, &cFetched) == S_OK && cFetched == (ULONG)len)
            {
                ach[cFetched] = L'\0';
                DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoEngTypo() undo text = %s\n"), ach));
                fConvert = ach[len - 1] == L' ' && wcsncmp(ach, pszTyped, len - 1) == 0;
            }
        }
        else
        {
            // one more char for the start of the word
            len = lstrlenW(pszTyped);
            if (pRange->ShiftStart(ec, -(len + 1), &cchShift, NULL) == S_OK &&
                    pRange->GetText(ec, 0, ach, ARRAYSIZE(ach) - 1, &cFetched) == S_OK && cFetched >= (ULONG)len)
            {
                ach[cFetched] = L'\0';
                DEBUGPRINTFEX(100, (TEXT("CSaenaruTextService::_DoEngTypo() text = %s, typed = %s\n"), ach, pszTyped));
                fConvert = wcscmp(ach + cFetched - len, pszTyped) == 0 &&
                        (cFetched == (ULONG)len || !IsWordChar(ach[0])) &&
                        pRange->ShiftStart(ec, (LONG)cFetched - len, &cchShift, NULL) == S_OK &&
                        (fHangul || !IsEngInputScope(ec, pContext, pRange));
            }
        }
    }

    if (fConvert)
    {
        lstrcpyW(achNew, pszNew);
        lstrcatW(achNew, L" ");
        if (pRange->SetText(ec, 0, achNew, lstrlenW(achNew)) != S_OK)
            goto Exit;
        if (!fUndo)
        {
            lstrcpyW(_achEngTypoEng, _achEngTypo);
            lstrcpyW(_achEngTypoHan, achHan);
            _fEngTypoToEng = fHangul;
        }
        hr = S_OK;
    }
    else
    {
        // not the same text. do the key itself.
        if (fEmpty)
            pRange->Collapse(ec, TF_ANCHOR_END);
        if (fUndo)
        {
            // delete the selection or the char before the caret
            if (fEmpty && (pRange->ShiftStart(ec, -1, &cchShift, NULL) != S_OK ||
                    pRange->IsEmpty(ec, &fEmpty) != S_OK || fEmpty))
                goto Exit;
            if (pRange->SetText(ec, 0, L"", 0) != S_OK)
                goto Exit;
        }
        else if (pRange->SetText(ec, 0, L" ", 1) != S_OK)
        {
            goto Exit;
        }
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
