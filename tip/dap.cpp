//
// dap.cpp
//
// ITfDisplayAttributeProvider implementation.
//

#include "globals.h"
#include "saenarutip.h"

static const TCHAR c_szAttributeInfoKey[] = TEXT("Software\\Saenaru Text Service");
static const TCHAR c_szAttributeInfoValueName[] = TEXT("DisplayAttr");

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfo class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfo : public ITfDisplayAttributeInfo
{
public:
    CDisplayAttributeInfo();
    ~CDisplayAttributeInfo();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // ITfDisplayAttributeInfo
    STDMETHODIMP GetGUID(GUID *pguid);
    STDMETHODIMP GetDescription(BSTR *pbstrDesc);
    STDMETHODIMP GetAttributeInfo(TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP SetAttributeInfo(const TF_DISPLAYATTRIBUTE *ptfDisplayAttr);
    STDMETHODIMP Reset();

protected:
    const GUID* _pguid;
    const TF_DISPLAYATTRIBUTE* _pDisplayAttribute;
    const WCHAR* _pDescription;
    const WCHAR* _pValueName;

private:
    LONG _refCount; // COM ref count
};

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfoInput class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfoInput : public CDisplayAttributeInfo
{
public:
    CDisplayAttributeInfoInput()
    {
        _pguid = &c_guidSaenaruDisplayAttributeInput;
        _pDisplayAttribute = &_s_DisplayAttribute;
        _pDescription = _s_szDescription;
        _pValueName = _s_szValueName;
    }

    static const TF_DISPLAYATTRIBUTE _s_DisplayAttribute;
    static const WCHAR _s_szDescription[];
    static const WCHAR _s_szValueName[];
};

//+---------------------------------------------------------------------------
//
// CDisplayAttributeInfoConverted class
//
//----------------------------------------------------------------------------

class CDisplayAttributeInfoConverted : public CDisplayAttributeInfo
{
public:
    CDisplayAttributeInfoConverted()
    {
        _pguid = &c_guidSaenaruDisplayAttributeConverted;
        _pDisplayAttribute = &_s_DisplayAttribute;
        _pDescription = _s_szDescription;
        _pValueName = _s_szValueName;
    }

    static const TF_DISPLAYATTRIBUTE _s_DisplayAttribute;
    static const WCHAR _s_szDescription[];
    static const WCHAR _s_szValueName[];
};

// the registry values of the custmized display attributes
const WCHAR CDisplayAttributeInfoInput::_s_szValueName[] = TEXT("DisplayAttributeInput");
const WCHAR CDisplayAttributeInfoConverted::_s_szValueName[] = TEXT("DisplayAttributeConverted");

// The descriptions
const WCHAR CDisplayAttributeInfoInput::_s_szDescription[] = TEXT("Saenaru Display Attribute Input");
const WCHAR CDisplayAttributeInfoConverted::_s_szDescription[] = TEXT("Saenaru Display Attribute Converted");

const TF_DISPLAYATTRIBUTE CDisplayAttributeInfoInput::_s_DisplayAttribute =
{
    { TF_CT_COLORREF, RGB(0xff, 0xff, 0xff) }, // text color
    { TF_CT_COLORREF, RGB(0x00, 0xff, 0xff) }, // background color
    TF_LS_NONE,                             // underline style
    FALSE,                                  // underline boldness
    { TF_CT_NONE, 0 },                      // underline color
    TF_ATTR_INPUT                           // attribute info
};

const TF_DISPLAYATTRIBUTE CDisplayAttributeInfoConverted::_s_DisplayAttribute =
{
    { TF_CT_COLORREF, RGB(0xff, 0xff, 0xff) }, // text color
    { TF_CT_COLORREF, RGB(0x00, 0x6e, 0xde) }, // background color
    TF_LS_NONE,                             // underline style
    FALSE,                                  // underline boldness
    { TF_CT_NONE, 0 },                      // underline color
    TF_ATTR_TARGET_CONVERTED                // attribute info
};

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfDisplayAttributeInfo))
    {
        *ppvObj = (ITfDisplayAttributeInfo *)this;
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
// ctor
//
//----------------------------------------------------------------------------

CDisplayAttributeInfo::CDisplayAttributeInfo()
{
    DllAddRef();

    _pguid = nullptr;
    _pDisplayAttribute = nullptr;
    _pValueName = nullptr;

    _refCount = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CDisplayAttributeInfo::~CDisplayAttributeInfo()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

ULONG CDisplayAttributeInfo::AddRef(void)
{
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

ULONG CDisplayAttributeInfo::Release(void)
{
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

//+---------------------------------------------------------------------------
//
// GetGUID
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::GetGUID(GUID *pguid)
{
    if (pguid == NULL)
        return E_INVALIDARG;

    *pguid = *_pguid;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDescription
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::GetDescription(BSTR *pbstrDesc)
{
    BSTR bstrDesc;

    if (pbstrDesc == NULL)
        return E_INVALIDARG;

    *pbstrDesc = NULL;

    if ((bstrDesc = SysAllocString(_pDescription)) == NULL)
        return E_OUTOFMEMORY;

    *pbstrDesc = bstrDesc;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::GetAttributeInfo(TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
{
    HKEY hKeyAttributeInfo;
    LONG lResult;
    DWORD cbData;

    if (ptfDisplayAttr == NULL)
        return E_INVALIDARG;

#if 0
    lResult = E_FAIL;

    if (RegOpenKeyEx(HKEY_CURRENT_USER, c_szAttributeInfoKey, 0, KEY_READ, &hKeyAttributeInfo) == ERROR_SUCCESS)
    {
        cbData = sizeof(*ptfDisplayAttr);

        lResult = RegQueryValueEx(hKeyAttributeInfo, c_szAttributeInfoValueName,
                                  NULL, NULL,
                                  (LPBYTE)ptfDisplayAttr, &cbData);

        RegCloseKey(hKeyAttributeInfo);
    }

    if (lResult != ERROR_SUCCESS || cbData != sizeof(*ptfDisplayAttr))
    {
        // go with the defaults
        *ptfDisplayAttr = _c_DefaultDisplayAttribute;
        MyDebugPrint((TEXT("\t Default GetAttributeInfo()\n")));
    }
#endif
    // return the default display attribute.
    *ptfDisplayAttr = *_pDisplayAttribute;
    MyDebugPrint((TEXT("END GetAttributeInfo()\n")));

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// SetAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::SetAttributeInfo(const TF_DISPLAYATTRIBUTE *ptfDisplayAttr)
{
#if 0
    HKEY hKeyAttributeInfo;
    LONG lResult;

    lResult = RegCreateKeyEx(HKEY_CURRENT_USER, c_szAttributeInfoKey, 0, TEXT(""),
                             REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL,
                             &hKeyAttributeInfo, NULL);

    if (lResult != ERROR_SUCCESS)
        return E_FAIL;

    lResult = RegSetValueEx(hKeyAttributeInfo, c_szAttributeInfoValueName,
                            0, REG_BINARY, (const BYTE *)ptfDisplayAttr,
                            sizeof(*ptfDisplayAttr));

    RegCloseKey(hKeyAttributeInfo);

    return (lResult == ERROR_SUCCESS) ? S_OK : E_FAIL;
#endif
    MyDebugPrint((TEXT("\tEND SetAttributeInfo()\n")));
    ptfDisplayAttr;

    return E_NOTIMPL;
}

//+---------------------------------------------------------------------------
//
// Reset
//
//----------------------------------------------------------------------------

STDAPI CDisplayAttributeInfo::Reset()
{
    return SetAttributeInfo(_pDisplayAttribute);
}

class CEnumDisplayAttributeInfo : public IEnumTfDisplayAttributeInfo
{
public:
    CEnumDisplayAttributeInfo();
    ~CEnumDisplayAttributeInfo();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void **ppvObj);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

    // IEnumTfDisplayAttributeInfo
    STDMETHODIMP Clone(IEnumTfDisplayAttributeInfo **ppEnum);
    STDMETHODIMP Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched);
    STDMETHODIMP Reset();
    STDMETHODIMP Skip(ULONG ulCount);

private:
    LONG _iIndex; // next display attribute to enum
    LONG _cRef; // COM ref count
};

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CEnumDisplayAttributeInfo::CEnumDisplayAttributeInfo()
{
    DllAddRef();

    _iIndex = 0;
    _cRef = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CEnumDisplayAttributeInfo::~CEnumDisplayAttributeInfo()
{
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::QueryInterface(REFIID riid, void **ppvObj)
{
    if (ppvObj == NULL)
        return E_INVALIDARG;

    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_IEnumTfDisplayAttributeInfo))
    {
        *ppvObj = (IEnumTfDisplayAttributeInfo *)this;
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

STDAPI_(ULONG) CEnumDisplayAttributeInfo::AddRef()
{
    return ++_cRef;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CEnumDisplayAttributeInfo::Release()
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
// Clone
//
// Returns a copy of the object.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Clone(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pClone;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pClone = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    // the clone should match this object's state
    pClone->_iIndex = _iIndex;

    *ppEnum = pClone;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Next
//
// Returns an array of display attribute info objects supported by this service.
//----------------------------------------------------------------------------

const int MAX_DISPLAY_ATTRIBUTE_INFO = 2;

STDAPI CEnumDisplayAttributeInfo::Next(ULONG ulCount, ITfDisplayAttributeInfo **rgInfo, ULONG *pcFetched)
{
    ULONG cFetched;

    cFetched = 0;

    if (ulCount == 0)
        return S_OK;
    if (rgInfo == NULL)
    {
        return E_INVALIDARG;
    }

    while (cFetched < ulCount)
    {
        ITfDisplayAttributeInfo* pDisplayAttributeInfo = nullptr;

        if (_iIndex == 0)
        {
            pDisplayAttributeInfo = new CDisplayAttributeInfoInput();
            if ((pDisplayAttributeInfo) == NULL)
            {
                return E_OUTOFMEMORY;
            }
        }
        else if (_iIndex == 1)
        {
            pDisplayAttributeInfo = new CDisplayAttributeInfoConverted();
            if ((pDisplayAttributeInfo) == NULL)
            {
                return E_OUTOFMEMORY;
            }
        }
        else
        {
            break;
        }
        *rgInfo = pDisplayAttributeInfo;
        rgInfo++;
        cFetched++;
        _iIndex++;
    }

    if (pcFetched != NULL)
    {
        // technically this is only legal if ulCount == 1, but we won't check
        pcFetched = &cFetched;
    }

    return (cFetched == ulCount) ? S_OK : S_FALSE;
}

//+---------------------------------------------------------------------------
//
// Reset
//
// Resets the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Reset()
{
    _iIndex = 0;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// Skip
//
// Skips past objects in the enumeration.
//----------------------------------------------------------------------------

STDAPI CEnumDisplayAttributeInfo::Skip(ULONG ulCount)
{
    if ((ulCount + _iIndex ) > 0 && MAX_DISPLAY_ATTRIBUTE_INFO || (ulCount + _iIndex) < ulCount)
    {
        _iIndex = MAX_DISPLAY_ATTRIBUTE_INFO;
        return S_FALSE;
    }
    _iIndex += ulCount;
    return S_OK;
}

//+---------------------------------------------------------------------------
//
// EnumDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::EnumDisplayAttributeInfo(IEnumTfDisplayAttributeInfo **ppEnum)
{
    CEnumDisplayAttributeInfo *pAttributeEnum;

    if (ppEnum == NULL)
        return E_INVALIDARG;

    *ppEnum = NULL;

    if ((pAttributeEnum = new CEnumDisplayAttributeInfo) == NULL)
        return E_OUTOFMEMORY;

    *ppEnum = pAttributeEnum;

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// GetDisplayAttributeInfo
//
//----------------------------------------------------------------------------

STDAPI CSaenaruTextService::GetDisplayAttributeInfo(REFGUID guidInfo, ITfDisplayAttributeInfo **ppInfo)
{
    if (ppInfo == NULL)
        return E_INVALIDARG;

    *ppInfo = NULL;

    if (IsEqualGUID(guidInfo, c_guidSaenaruDisplayAttributeInput))
    {
        *ppInfo = new CDisplayAttributeInfoInput();
        if ((*ppInfo) == nullptr)
        {
            return E_OUTOFMEMORY;
        }
    }
    else if (IsEqualGUID(guidInfo, c_guidSaenaruDisplayAttributeConverted))
    {
        *ppInfo = new CDisplayAttributeInfoConverted();
        if ((*ppInfo) == nullptr)
        {
            return E_OUTOFMEMORY;
        }
    }
    else
    {
        return E_INVALIDARG;
    }

    return S_OK;
}

/*
 * ex: ts=4 sts=4 sw=4 et
 */
