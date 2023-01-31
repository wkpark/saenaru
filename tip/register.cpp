//
// register.cpp
//
// Server registration code.
//

#include <windows.h>
#include <ole2.h>
#include "msctf.h"
#include "globals.h"
#include "saenarutip.h"
#include "resource.h"

#define CLSID_STRLEN 38  // strlen("{xxxxxxxxx-xxxx-xxxx-xxxx-xxxxxxxxxxx}")

#if defined (_UNICODE) || defined (UNICODE)
#define		CLSIDToString(refGUID,pch)	CLSIDToStringW((refGUID),(pch))
#else
#define		CLSIDToString(refGUID,pch)	CLSIDToStringA((refGUID),(pch))
#endif

const struct
{
    const GUID *pguidCategory;
    const GUID *pguid;
}
c_rgCategories[] =

{
    { &GUID_TFCAT_TIP_KEYBOARD, &c_clsidSaenaruTextService },
    { &GUID_TFCAT_TIPCAP_IMMERSIVESUPPORT, &c_clsidSaenaruTextService },
    { &GUID_TFCAT_TIPCAP_SYSTRAYSUPPORT, &c_clsidSaenaruTextService }, /* not needed but useful for debuging purpose */
/*    { &GUID_TFCAT_DISPLAYATTRIBUTEPROVIDER, &c_clsidSaenaruTextService }, */
/*   { &GUID_TFCAT_PROPSTYLE_STATICCOMPACT, &c_guidCaseProperty }, */
    { &GUID_TFCAT_PROPSTYLE_CUSTOM, &c_guidCustomProperty },
};

static const TCHAR c_szInfoKeyPrefix[] = TEXT("CLSID\\");
static const TCHAR c_szInProcSvr32[] = TEXT("InProcServer32");
static const TCHAR c_szModelName[] = TEXT("ThreadingModel");

//+---------------------------------------------------------------------------
//
//  RegisterProfiles
//
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::RegisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    WCHAR achIconFile[MAX_PATH];
    char achFileNameA[MAX_PATH];
    DWORD cchA;
    int cchIconFile;
    HRESULT hr;

    LPTSTR lpDesc;
    TCHAR  szDesc[128];
    lpDesc = (LPTSTR)szDesc;

    LoadString(g_hInst, IDS_SAENARU_DESC, lpDesc, 128);

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return E_FAIL;

    hr = pInputProcessProfiles->Register(c_clsidSaenaruTextService);

    if (hr != S_OK)
        goto Exit;

    cchA = GetModuleFileNameA(g_hInst, achFileNameA, ARRAYSIZE(achFileNameA));

    cchIconFile = MultiByteToWideChar(CP_ACP, 0, achFileNameA, cchA, achIconFile, ARRAYSIZE(achIconFile)-1);
    achIconFile[cchIconFile] = '\0';

    hr = pInputProcessProfiles->AddLanguageProfile(c_clsidSaenaruTextService,
                                  SAENARU_LANGID,
                                  c_guidSaenaruProfile,
                                  lpDesc,
                                  (ULONG)wcslen(lpDesc),
                                  achIconFile,
                                  cchIconFile,
                                  SAENARU_ICON_INDEX);

Exit:
    pInputProcessProfiles->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
//  UnregisterProfiles
//
//----------------------------------------------------------------------------

void CSaenaruTextService::UnregisterProfiles()
{
    ITfInputProcessorProfiles *pInputProcessProfiles;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfInputProcessorProfiles, (void**)&pInputProcessProfiles);

    if (hr != S_OK)
        return;

    pInputProcessProfiles->Unregister(c_clsidSaenaruTextService);
    pInputProcessProfiles->Release();
}

//+---------------------------------------------------------------------------
//
//  RegisterCategories
//
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::RegisterCategories(BOOL fRegister)
{
    ITfCategoryMgr *pCategoryMgr;
    int i;
    HRESULT hr;

    hr = CoCreateInstance(CLSID_TF_CategoryMgr, NULL, CLSCTX_INPROC_SERVER,
                          IID_ITfCategoryMgr, (void**)&pCategoryMgr);

    if (hr != S_OK)
        return FALSE;

    for (i=0; i<ARRAYSIZE(c_rgCategories); i++)
    {
        if (fRegister)
        {
            hr = pCategoryMgr->RegisterCategory(c_clsidSaenaruTextService,
                   *c_rgCategories[i].pguidCategory, *c_rgCategories[i].pguid);
        }
        else
        {
            hr = pCategoryMgr->UnregisterCategory(c_clsidSaenaruTextService,
                   *c_rgCategories[i].pguidCategory, *c_rgCategories[i].pguid);
        }

        if (hr != S_OK)
            break;
    }

    pCategoryMgr->Release();
    return (hr == S_OK);
}

//+---------------------------------------------------------------------------
//
// CLSIDToStringA
//
//----------------------------------------------------------------------------

BOOL CLSIDToStringA(REFGUID refGUID, char *pchA)
{
    static const BYTE GuidMap[] = {3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-',
                                   8, 9, '-', 10, 11, 12, 13, 14, 15};

    static const char szDigits[] = "0123456789ABCDEF";

    int i;
    char *p = pchA;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = '{';
    for (i = 0; i < sizeof(GuidMap); i++)
    {
        if (GuidMap[i] == '-')
        {
            *p++ = '-';
        }
        else
        {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }

    *p++ = '}';
    *p   = '\0';

    return TRUE;
}

BOOL
CLSIDToStringW (
	REFGUID			refGUID,
	WCHAR*			pchW)
{
    static const BYTE GuidMap[] = {
		3, 2, 1, 0, '-', 5, 4, '-', 7, 6, '-', 8, 9, '-', 10, 11, 12, 13, 14, 15
	} ;
    static const WCHAR szDigits[] = L"0123456789ABCDEF";

    int		i ;
    WCHAR*	p = pchW ;

    const BYTE * pBytes = (const BYTE *) &refGUID;

    *p++ = L'{';
    for (i = 0; i < sizeof(GuidMap); i++) {
        if (GuidMap[i] == '-') {
            *p++ = L'-';
        } else {
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0xF0) >> 4 ];
            *p++ = szDigits[ (pBytes[GuidMap[i]] & 0x0F) ];
        }
    }
	
    *p++ = L'}';
    *p   = L'\0';

    return	TRUE ;
}

//+---------------------------------------------------------------------------
//
// RecurseDeleteKey
//
// RecurseDeleteKey is necessary because on NT RegDeleteKey doesn't work if the
// specified key has subkeys
//----------------------------------------------------------------------------
LONG RecurseDeleteKey(HKEY hParentKey, LPCTSTR lpszKey)
{
    HKEY hKey;
    LONG lRes;
    FILETIME time;
    TCHAR szBuffer[256];
    DWORD dwSize = ARRAYSIZE(szBuffer);

    if (RegOpenKey(hParentKey, lpszKey, &hKey) != ERROR_SUCCESS)
        return ERROR_SUCCESS; // let's assume we couldn't open it because it's not there

    lRes = ERROR_SUCCESS;
    while (RegEnumKeyEx(hKey, 0, szBuffer, &dwSize, NULL, NULL, NULL, &time)==ERROR_SUCCESS)
    {
        szBuffer[ARRAYSIZE(szBuffer)-1] = '\0';
        lRes = RecurseDeleteKey(hKey, szBuffer);
        if (lRes != ERROR_SUCCESS)
            break;
        dwSize = ARRAYSIZE(szBuffer);
    }
    RegCloseKey(hKey);

    return lRes == ERROR_SUCCESS ? RegDeleteKey(hParentKey, lpszKey) : lRes;
}

//+---------------------------------------------------------------------------
//
//  RegisterServer
//
//----------------------------------------------------------------------------

BOOL CSaenaruTextService::RegisterServer()
{
    DWORD dw;
    HKEY hKey;
    HKEY hSubKey;
    BOOL fRet;
    TCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];
    TCHAR achFileName[MAX_PATH];

    LPTSTR lpDesc;
    TCHAR  szDesc[128];
    lpDesc = (LPTSTR)szDesc;

    LoadString(g_hInst, IDS_SAENARU_DESC, lpDesc, 128);

    if (!CLSIDToString(c_clsidSaenaruTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return FALSE;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(TCHAR));

    if (fRet = RegCreateKeyEx(HKEY_CLASSES_ROOT, achIMEKey, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, &dw)
            == ERROR_SUCCESS)
    {
        fRet &= RegSetValueEx(hKey, NULL, 0, REG_SZ, (BYTE *)lpDesc, (lstrlen(lpDesc)+1)*sizeof(TCHAR))
            == ERROR_SUCCESS;

        if (fRet &= RegCreateKeyEx(hKey, c_szInProcSvr32, 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hSubKey, &dw)
            == ERROR_SUCCESS)
        {
            dw = GetModuleFileName(g_hInst, achFileName, ARRAYSIZE(achFileName));

            fRet &= RegSetValueEx(hSubKey, NULL, 0, REG_SZ, (BYTE *)achFileName, (lstrlen(achFileName)+1)*sizeof(TCHAR)) == ERROR_SUCCESS;
            fRet &= RegSetValueEx(hSubKey, c_szModelName, 0, REG_SZ, (BYTE *)SAENARU_MODEL, (lstrlen(SAENARU_MODEL)+1)*sizeof(TCHAR)) == ERROR_SUCCESS;
            RegCloseKey(hSubKey);
        }
        RegCloseKey(hKey);
    }

    return fRet;
}

//+---------------------------------------------------------------------------
//
//  UnregisterServer
//
//----------------------------------------------------------------------------

void CSaenaruTextService::UnregisterServer()
{
    TCHAR achIMEKey[ARRAYSIZE(c_szInfoKeyPrefix) + CLSID_STRLEN];

    if (!CLSIDToString(c_clsidSaenaruTextService, achIMEKey + ARRAYSIZE(c_szInfoKeyPrefix) - 1))
        return;
    memcpy(achIMEKey, c_szInfoKeyPrefix, sizeof(c_szInfoKeyPrefix)-sizeof(TCHAR));

    RecurseDeleteKey(HKEY_CLASSES_ROOT, achIMEKey);
}

/*
 * vim: et sts=4 sw=4
 */
