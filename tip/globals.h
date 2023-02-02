//
// globals.h
//
// Global variable declarations.
//

#ifndef GLOBALS_H
#define GLOBALS_H

#include <windows.h>
#include <ole2.h>
#include <olectl.h>
#include <assert.h>
#include "msctf.h"
#include "ctffunc.h"

extern "C" {
#include "debug.h"
}

LONG DllAddRef();
LONG DllRelease();

BOOL AdviseSink(IUnknown *pSource, IUnknown *pSink, REFIID riid, DWORD *pdwCookie);
void UnadviseSink(IUnknown *pSource, DWORD *pdwCookie);

BOOL AdviseSingleSink(TfClientId tfClientId, IUnknown *pSource, IUnknown *pSink, REFIID riid);
void UnadviseSingleSink(TfClientId tfClientId, IUnknown *pSource, REFIID riid);

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover);

BOOL IsEqualUnknown(IUnknown *interface1, IUnknown *interface2);

#ifndef ARRAYSIZE
#define ARRAYSIZE(a) (sizeof(a)/sizeof(a[0]))
#endif

#define SAENARU_LANGID    MAKELANGID(LANG_KOREAN, SUBLANG_DEFAULT)

#define SAENARU_DESC    L"Saenaru Hangul TIP"
#define SAENARU_DESC_A   "Saenaru Hangul TIP"
#define SAENARU_MODEL   TEXT("Apartment")

#define LANGBAR_ITEM_DESC L"Saenaru Menu" // max 32 chars!

#define SAENARU_ICON_INDEX  0

#define SafeRelease(punk)       \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
    }                           \
}

#define SafeReleaseClear(punk)  \
{                               \
    if ((punk) != NULL)         \
    {                           \
        (punk)->Release();      \
        (punk) = NULL;          \
    }                           \
}

//+---------------------------------------------------------------------------
//
// SafeStringCopy
//
// Copies a string from one buffer to another.  wcsncpy does not always
// null-terminate the destination buffer; this function does.
//----------------------------------------------------------------------------

inline void SafeStringCopy(WCHAR *pchDst, ULONG cchMax, const WCHAR *pchSrc)
{
    if (cchMax > 0)
    {
        //wcsncpy(pchDst, pchSrc, cchMax);
        wcsncpy_s(pchDst, cchMax, pchSrc, cchMax);
        pchDst[cchMax-1] = '\0';
    }
}

extern HINSTANCE g_hInst;

extern LONG g_cRefDll;

extern CRITICAL_SECTION g_cs;

#ifdef __cplusplus
extern "C" {
#endif

#ifdef DEBUG
/* for DebugOptions */
extern DWORD dwLogFlag;
extern DWORD dwDebugFlag;
#endif

/* Hangul options */
extern DWORD dwLayoutFlag;
extern DWORD dwComposeFlag;
extern DWORD dwOptionFlag;
extern DWORD dwHanjaMode;
extern DWORD dwScanCodeBased;
extern DWORD dwToggleKey;

extern DWORD dwImeFlag;

#ifdef __cplusplus
}
#endif

/* consts */
extern const CLSID c_clsidSaenaruTextService;
extern const GUID c_guidSaenaruProfile;
extern const GUID c_guidLangBarItemButton;
extern const GUID c_guidLangBarItemButtonShape;
extern const GUID c_guidLangBarItemButtonIME;
extern const GUID c_guidSaenaruContextCompartment;
extern const GUID c_guidSaenaruGlobalCompartment;
extern const GUID c_guidCaseProperty;
extern const GUID c_guidCustomProperty;

extern const GUID c_guidSaenaruDisplayAttributeInput;
extern const GUID c_guidSaenaruDisplayAttributeConverted;

#endif // GLOBALS_H

/*
 * ex: ts=4 sts=4 sw=4 et
 */
