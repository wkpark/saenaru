//
// dllmain.cpp
//
// DllMain module entry point.
//

#include "globals.h"

extern "C" {
void SetDwordToSetting(LPCTSTR lpszFlag, DWORD dwFlag);
DWORD PASCAL GetDwordFromSetting(LPTSTR lpszFlag);
long PASCAL GetRegMultiStringValue (LPCTSTR,LPCTSTR,LPTSTR);
void SetRegMultiString(LPCTSTR lpszFlag, DWORD dwFlag);
long PASCAL GetRegStringValue (LPCTSTR,LPCTSTR,LPTSTR);
}

void PASCAL SetGlobalFlags()
{
    DWORD tmp;
#ifdef DEBUG
    dwLogFlag = GetDwordFromSetting(TEXT("LogFlag"));
    dwDebugFlag = GetDwordFromSetting(TEXT("DebugFlag"));
#endif

    dwOptionFlag = GetDwordFromSetting(TEXT("OptionFlag"));
    dwLayoutFlag = GetDwordFromSetting(TEXT("LayoutFlag"));

    // get ScanCode based setting.
    dwScanCodeBased = GetDwordFromSetting(TEXT("ScanCodeBased"));

    // HangulToggle Key
    dwToggleKey = GetDwordFromSetting(TEXT("HangulToggle"));

    tmp = GetDwordFromSetting(TEXT("ImeFlag"));
    if (tmp) {
        dwImeFlag = tmp;
    }
}

//+---------------------------------------------------------------------------
//
// DllMain
//
//----------------------------------------------------------------------------

BOOL WINAPI DllMain(HINSTANCE hInstance, DWORD dwReason, LPVOID pvReserved)
{
    switch (dwReason)
    {
        case DLL_PROCESS_ATTACH:

            g_hInst = hInstance;

            if (!InitializeCriticalSectionAndSpinCount(&g_cs, 0))
                return FALSE;

	    // init global option variables
	    SetGlobalFlags();

            break;

        case DLL_PROCESS_DETACH:

            DeleteCriticalSection(&g_cs);

            break;
    }

    return TRUE;
}

/*
 * ex: ts=4 sts=4 sw=4 et
 */
