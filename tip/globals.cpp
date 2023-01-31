//
// globals.cpp
//
// Global variables.
//

#include "globals.h"

HINSTANCE g_hInst;

extern "C" {
#pragma data_seg("SHAREDDATA")
#ifdef DEBUG
/* for DebugOptions */
DWORD dwLogFlag = 0L;
DWORD dwDebugFlag = 0L;
#endif
DWORD dwLayoutFlag = 0L;
DWORD dwComposeFlag = 0L;
DWORD dwOptionFlag = 0L;
DWORD dwHanjaMode = 0L;
DWORD dwScanCodeBased = 0L;
DWORD dwToggleKey = 0L;

/* local flags */
DWORD dwImeFlag = 0L /* SAENARU_ONTHESPOT */;
#pragma data_seg()

}

LONG g_cRefDll = -1; // -1 /w no refs, for win95 InterlockedIncrement/Decrement compat

CRITICAL_SECTION g_cs;

/* 180a826c-1d6b-49ea-86c3-2451de276b71 */
//const CLSID c_clsidSaenaruTextService = { 0x180a826c, 0x1d6b, 0x49ea, {0x86, 0xc3, 0x24, 0x51, 0xde, 0x27, 0x6b, 0x71} };
const CLSID c_clsidSaenaruTextService = { 0x766a2c15, 0xb226, 0x4fd6, {0xb5, 0x2a, 0x86, 0x7b, 0x3E, 0xbf, 0x38, 0xd2} };

/* 930c0cdc-a91f-4d7c-b23a-67749d4ebfcd */
const GUID c_guidSaenaruProfile = { 0x930c0cdc, 0xa91f, 0x4d7c, {0xb2, 0x3a, 0x67, 0x74, 0x9d, 0x4e, 0xbf, 0xcd} };

/* c74a88d5-6614-439b-8880-2dd8e6cd91a7 */
const GUID c_guidLangBarItemButton = { 0xc74a88d5, 0x6614, 0x439b, {0x88, 0x80, 0x2d, 0xd8, 0xe6, 0xcd, 0x91, 0xa7} };

/* c84a88d5-6614-439b-8880-2dd8e6cd91a7 */
const GUID c_guidLangBarItemButtonShape = { 0xc84a88d5, 0x6614, 0x439b, {0x88, 0x80, 0x2d, 0xd8, 0xe6, 0xcd, 0x91, 0xa7} };

/* d81face6-845c-45e7-a2af-1c5fc7adf667 */
const GUID c_guidSaenaruDisplayAttribute = { 0xd81face6, 0x845c, 0x45e7, {0xa2, 0xaf, 0x1c, 0x5f, 0xc7, 0xad, 0xf6, 0x67} };

/* eadc084c-6130-4222-82e5-c528c1f4abbb */
const GUID c_guidSaenaruContextCompartment = { 0xeadc084c, 0x6130, 0x4222, {0x82, 0xe5, 0xc5, 0x28, 0xc1, 0xf4, 0xab, 0xbb} };

/* 947d9d1c-7a4c-4392-b37c-34017c6c7fe1 */
const GUID c_guidSaenaruGlobalCompartment = { 0x947d9d1c, 0x7a4c, 0x4392, {0xb3, 0x7c, 0x34, 0x01, 0x7c, 0x6c, 0x7f, 0xe1} };

/* 3042ae6a-4697-4f7d-acdf-20a972fee027 */
const GUID c_guidCaseProperty = { 0x3042ae6a, 0x4697, 0x4f7d, {0xac, 0xdf, 0x20, 0xa9, 0x72, 0xfe, 0xe0, 0x27} };

/* d05a182a-7782-4e61-a2ea-6a4794ab7aaa */
const GUID c_guidCustomProperty = { 0xd05a182a, 0x7782, 0x4e61, {0xa2, 0xea, 0x6a, 0x47, 0x94, 0xab, 0x7a, 0xaa} };

//+---------------------------------------------------------------------------
//
// AdviseSink
//
//----------------------------------------------------------------------------

BOOL AdviseSink(IUnknown *pSourceIn, IUnknown *pSink, REFIID riid, DWORD *pdwCookie)
{
    ITfSource *pSource;
    HRESULT hr;

    if (pSourceIn->QueryInterface(IID_ITfSource, (void **)&pSource) != S_OK)
        return FALSE;

    hr = pSource->AdviseSink(riid, pSink, pdwCookie);

    pSource->Release();

    if (hr != S_OK)
    {
        // make sure we don't try to Unadvise pdwCookie later
        *pdwCookie = TF_INVALID_COOKIE;
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// UnadviseSink
//
//----------------------------------------------------------------------------

void UnadviseSink(IUnknown *pSourceIn, DWORD *pdwCookie)
{
    ITfSource *pSource;

    if (*pdwCookie == TF_INVALID_COOKIE)
        return; // never Advised

    if (pSourceIn->QueryInterface(IID_ITfSource, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSink(*pdwCookie);
        pSource->Release();
    }

    *pdwCookie = TF_INVALID_COOKIE;
}

//+---------------------------------------------------------------------------
//
// AdviseSingleSink
//
//----------------------------------------------------------------------------

BOOL AdviseSingleSink(TfClientId tfClientId, IUnknown *pSourceIn, IUnknown *pSink, REFIID riid)
{
    ITfSourceSingle *pSource;
    HRESULT hr;

    if (pSourceIn->QueryInterface(IID_ITfSourceSingle, (void **)&pSource) != S_OK)
        return FALSE;

    hr = pSource->AdviseSingleSink(tfClientId, riid, pSink);

    pSource->Release();

    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// UnadviseSingleSink
//
//----------------------------------------------------------------------------

void UnadviseSingleSink(TfClientId tfClientId, IUnknown *pSourceIn, REFIID riid)
{
    ITfSourceSingle *pSource;

    if (pSourceIn->QueryInterface(IID_ITfSourceSingle, (void **)&pSource) == S_OK)
    {
        pSource->UnadviseSingleSink(tfClientId, riid);
        pSource->Release();
    }
}

//+---------------------------------------------------------------------------
//
// IsRangeCovered
//
// Returns TRUE if pRangeTest is entirely contained within pRangeCover.
//----------------------------------------------------------------------------

BOOL IsRangeCovered(TfEditCookie ec, ITfRange *pRangeTest, ITfRange *pRangeCover)
{
    LONG lResult;

    if (pRangeCover->CompareStart(ec, pRangeTest, TF_ANCHOR_START, &lResult) != S_OK ||
        lResult > 0)
    {
        return FALSE;
    }

    if (pRangeCover->CompareEnd(ec, pRangeTest, TF_ANCHOR_END, &lResult) != S_OK ||
        lResult < 0)
    {
        return FALSE;
    }

    return TRUE;
}

//+---------------------------------------------------------------------------
//
// IsEqualUnknown
//
// Returns TRUE if punk1 and punk2 refer to the same object.  We must QI for
// IUnknown to be guarenteed a reliable test, per the COM rules.
//----------------------------------------------------------------------------

BOOL IsEqualUnknown(IUnknown *interface1, IUnknown *interface2)
{
    IUnknown *punk1;
    IUnknown *punk2;

    if (interface1->QueryInterface(IID_IUnknown, (void **)&punk1) != S_OK)
        return FALSE;

    punk1->Release(); // we don't actually need to dereference these guys, just want to compare them

    if (interface2->QueryInterface(IID_IUnknown, (void **)&punk2) != S_OK)
        return FALSE;

    punk2->Release();

    return (punk1 == punk2);
}

/*
 * ex: ts=4 sts=4 sw=4 et
 */
