/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 2003 Hye-Shik Chang <perky@i18n.org>.
 * Copyright (c) 2003 Won-Kyu Park <wkpark@kldp.org>.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $Saenaru: saenaru/src/tsf.cpp,v 1.7 2006/11/21 15:21:24 wkpark Exp $
 */

#if !defined (NO_TSF)
#include "windows.h"
#include "tchar.h"
#include "immdev.h"
extern "C" {
#include "saenaru.h"
#include "resource.h"
}
//#include "c:\Program Files\Microsoft SDK\include\msctf.h"
#include "msctf.h"
#include "olectl.h"
#include "tsf.h"

#define    NUM_LANGBAR_ITEM_BUTTON    (2)
#define    NUM_TSF_GETITEMS        (15)
#define    TSF_WAITTIME            (3 * 1000)

static    ITfThreadMgr*        _QueryThreadMgr (void);
static    ITfLangBarItemMgr*    _QueryLangBarItemMgr (void);
static    BOOL            InitOtherLangBarItem (void);
static    void		  _DumpLangBarItem(void);

static    HMODULE       shMSCTF            = NULL;
static    TfClientId    g_tfClientID;
static    BOOL          g_fInitTSF         = FALSE;
static    DWORD         g_dwHelpButtonCookie = NULL;

LONG DllAddRef(void)
{
    return TRUE;
}

//+---------------------------------------------------------------------------
//
//  DllRelease
//
//----------------------------------------------------------------------------

LONG DllRelease(void)
{
    return TRUE;
}


// {830DE831-E04D-40bb-8683-1E179A5F1A4F}
/*
 *    ª³ªÎ class id ªÏ skimic.dll ªÎ××éÄª·ªÆª¤ªëªâªÎªÀª¬¡¢ª³ªìªòÞÅªÃªÆªÏª¤ª±ªÊª¤
 *    ªèª¦ªÀ¡£?ð·ªËÞÅª¦ class id ªËªÄª¤ªÆªÏ¡¦
 *
 */
#if 0
const CLSID    c_clsidSaenaruTextService    = {
    0x830de831, 0xe04d, 0x40bb, { 0x86, 0x83, 0x1e, 0x17, 0x9a, 0x5f, 0x1a, 0x4f }
};
#endif
/*    ª³ªÎ IME ªÈª¤ª¦??«óªÈÔÒª¸ CLSID ªËàâïÒª¹ªëù±é©ª¬ª¢ªëªèª¦ªÀ¡£MS-IME2002 ªÎ
 *    ?ÔÑªò?ðÎª¹ªëªË¡£
 */
#if 1
const CLSID c_clsidSaenaruTextService    = {
    //0x77B34286, 0xED57, 0x45E2, { 0x97, 0x66, 0xD8, 0xC5, 0xFE, 0x3D, 0xFB, 0x2C }
    0x766a2c15, 0xb226, 0x4fd6, { 0xb5, 0x2a, 0x86, 0x7b, 0x3E, 0xbf, 0x38, 0xd2 }
};
#endif

// {930C0CDC-A91F-4d7c-B23A-67749D4EBFCD}
const GUID c_guidSaenaruProfile = {
    0x930c0cdc, 0xa91f, 0x4d7c, { 0xb2, 0x3a, 0x67, 0x74, 0x9d, 0x4e, 0xbf, 0xcd }
};

// {D97B240A-2C61-4b9b-AECF-D5C3CFDDFCBB}
const GUID c_guidItemButtonCMode = {
    //0xd97b240b, 0x2c61, 0x4b9b, { 0xae, 0xcf, 0xd5, 0xc3, 0xcf, 0xdd, 0xfc, 0xbb }
    0xd97b240a, 0x2c61, 0x4b9b, { 0xae, 0xcf, 0xd5, 0xc3, 0xcf, 0xdd, 0xfc, 0xbb }
};

// {68D148D7-E134-45ef-B596-C5EE7915F819}
const GUID c_guidItemButtonIME    = {
    //0x68d148d8, 0xe134, 0x45ef, { 0xb5, 0x96, 0xc5, 0xee, 0x79, 0x15, 0xf8, 0x19 }
    0x68d148d7, 0xe134, 0x45ef, { 0xb5, 0x96, 0xc5, 0xee, 0x79, 0x15, 0xf8, 0x19 }
};

// {D97B240A-2C61-4b9b-AECF-D5C3CFDDFCBB}
const GUID c_guidItemButtonShape = {
    0xd97b240b, 0x2c61, 0x4b9b, { 0xae, 0xcf, 0xd5, 0xc3, 0xcf, 0xdd, 0xfc, 0xbb }
};

// {4d970333-cad0-42d0-ab1f-a9315b6cad38}
const GUID c_guidItemButtonPad = {
    0x4d970333, 0xcad0, 0x42d0, { 0xab, 0x1f, 0xa9, 0x31, 0x5b, 0x6c, 0xad, 0x38 }
};

/*    ÔÒª¸ button ªËÌ¸ª¨ªëª¬¡¢skimic.dll ªÈÔÒª¸ªâªÎªòÞÅª¦ªÎªÏù­ª±ªÊª±ªìªÐªÊªéªÊ
 *    ª¤ªèª¦ªÀ¡£ÔÒª¸ªËª·ªÆª¤ªëªÎªÀªÈª¤ª¦à»ìý?ª¬ã÷ø¨ªÀªÃª¿¡¦¡£
 */
#if 0
const GUID    c_guidItemButtonCMode = {
    0x82fd14c8, 0xa99a, 0x4882, { 0xa6, 0xe5, 0x74, 0x93, 0xca, 0x67, 0x86, 0x6a }
};

// {EDA78BBB-65F7-4a1b-9551-309D9FE6200C}
const GUID    c_guidItemButtonIME    = {
    0xeda78bbb, 0x65f7, 0x4a1b, { 0x95, 0x51, 0x30, 0x9d, 0x9f, 0xe6, 0x20, 0x0c }
};
#endif

/*    «­???«É??«óªÎ GUID¡£Íðªéª¯ªÏª³ªÎö·ªÇìéëòªËÌ½ïÒª¹ªë?¡£
 */
#if 1
const GUID c_guidKeyboardItemButton    = {
    0x34745C63, 0xB2F0, 0x4784, { 0x8B, 0x67, 0x5E, 0x12, 0xC8, 0x70, 0x1A, 0x31 }
};
#endif

// from _DumpLangBarItem()
// {50549ded e6eb 5542 82 89 f8 a3 1e 68 72 28}
const GUID c_guidItemButtonHelp = {
    0xed9d5450, 0xebe6, 0x4255, { 0x82, 0x89, 0xf8, 0xa3, 0x1e, 0x68, 0x72, 0x28 }
};

BOOL PASCAL
InitLanguageBar (void)
{
    register ITfLangBarItemMgr*    pLangBarItemMgr            = NULL;
#if defined (TSF_NEED_MUTEX)
    register HANDLE  hMutex;
    register DWORD   dwValue;
#endif
    register BOOL    fRet = FALSE;
    register HRESULT hr;
    register int     i;
        
    DEBUGPRINTF ((TEXT ("Enter::InitLanguageBar\n")));

    /* Logon ãÁªËªÏ TSF ªÏû¼ªÓõóªµªÊª¤¡£*/
#if 1
    if (gfSaenaruSecure) {
        DEBUGPRINTF ((TEXT ("Leave::InitLanguageBar - security\n")));
        return    FALSE;
    }
#endif
#if defined (TSF_NEED_MUTEX)
    hMutex = SaenaruCreateMutex (SAENARU_MUTEX_NAME);
    if (!hMutex) 
        return FALSE;

    dwValue = WaitForSingleObject (hMutex, TSF_WAITTIME);
    if (dwValue == WAIT_FAILED || dwValue == WAIT_TIMEOUT) 
        goto Exit_Func_1;
#endif    
    if (g_fInitTSF) {
        fRet = TRUE;
        goto Exit_Func;
    }

    shMSCTF = LoadLibrary (TEXT ("msctf.dll"));
    if (shMSCTF == NULL) {
        DEBUGPRINTF ((TEXT ("Leave::InitLanguageBar - mstcf.dll not found\n")));
        goto Exit_Func;
    }
    fRet = TRUE;
    g_fInitTSF = TRUE;

#define USE_HELPSINK
#ifdef USE_HELPSINK
    // http://msdn2.microsoft.com/en-us/library/ms628993.aspx
    pLangBarItemMgr = _QueryLangBarItemMgr();
    if (pLangBarItemMgr != NULL && g_dwHelpButtonCookie == NULL) {
        ITfLangBarItem* pItem;
        ITfSource* pSource;
        
        if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonHelp, &pItem)) &&
            pItem != NULL) {
            if (SUCCEEDED (pItem->QueryInterface (IID_ITfSource,
                        (void**)&pSource)) && pSource != NULL) {
                ITfSystemLangBarItemSink *pHelpSink;
                HRESULT hr;

                hr= CreateItemHelp(&pHelpSink);
                if (SUCCEEDED(hr)) {
                    hr= pSource->AdviseSink(IID_ITfSystemLangBarItemSink,
                                        pHelpSink,&g_dwHelpButtonCookie);
                    pHelpSink->Release();
                }
                pSource->Release();
            }
            pItem->Release();
        }
        pLangBarItemMgr->Release();
    }
#endif

 Exit_Func:
    DEBUGPRINTFEX (99, (TEXT ("InitLanguageBar () = %d\n"), fRet));
    if (! fRet)
    {
        DEBUGPRINTFEX (99, (TEXT ("InitLanguageBar failed.\n")));
        if (shMSCTF != NULL)
        {
            FreeLibrary (shMSCTF);
            shMSCTF = NULL;
        }
    }
#if defined (TSF_NEED_MUTEX)
    ReleaseMutex (hMutex);
  Exit_Func_1:
    CloseHandle (hMutex);
#endif
    DEBUGPRINTF ((TEXT ("Leave::InitLanguageBar - %d\n"), fRet));
    return fRet;
}

BOOL    PASCAL
IsTSFEnabled (void)
{
    register HMODULE hMSCTF;
    register BOOL fRetval;

    if (g_fInitTSF)
        return (shMSCTF != NULL);

    hMSCTF = LoadLibrary (TEXT ("msctf.dll"));
    if (hMSCTF != NULL) {
        FreeLibrary (hMSCTF);
        fRetval = TRUE;
    } else {
        fRetval = FALSE;
    }
    return fRetval;
}

BOOL    PASCAL
UpdateLanguageBarIfSelected (void)
{
    TCHAR    szFile [32];
    HKL        hKL;

    hKL    = GetKeyboardLayout (0);
    ImmGetIMEFileName (hKL, szFile, sizeof (szFile) / sizeof (TCHAR));
    if (!lstrcmpi (szFile, MyFileName)) 
        return    UpdateLanguageBar ();
    return    FALSE;
}

BOOL    PASCAL
UpdateLanguageBar (void)
{
    register ITfLangBarItemMgr* pLangBarItemMgr = NULL;
    register ITfThreadMgr*      pThreadMgr;
#if defined (TSF_NEED_MUTEX)
    register HANDLE hMutex;
#endif
    register DWORD  dwValue;
    register BOOL   fRetval = FALSE;
    register BOOL   fShowIMEIcon, fShowInputModeIcon, fShowShapeIcon, fShowPadIcon;

#if 1
    if (gfSaenaruSecure)
        return    FALSE;
#endif

#if defined (TSF_NEED_MUTEX)
    hMutex    = SaenaruCreateMutex (SAENARU_MUTEX_NAME);
    if (!hMutex) 
        return    FALSE;

    dwValue    = WaitForSingleObject (hMutex, TSF_WAITTIME);
    if (dwValue == WAIT_FAILED || dwValue == WAIT_TIMEOUT) 
        goto    exit_func_1;
#endif
    if (shMSCTF == NULL)
        goto    exit_func;

    DEBUGPRINTFEX (100, (TEXT ("UpdateLanguageBar ()\n")));

    /*    ?üÞ«ì«¸«¹«È«êªò?ðÎª¹ªëªÎªÏïáª·ª¤ªÎªÀªíª¦ª«£¿ */
    fShowIMEIcon = fShowInputModeIcon = fShowShapeIcon = TRUE;
    fShowPadIcon = TRUE;
#if 0
#if !defined (NO_TOUCH_REGISTRY)
    if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWIMEICON), &dwValue))
        fShowIMEIcon        = (BOOL) dwValue;
    if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWINPUTMODEICON), &dwValue))
        fShowInputModeIcon    = (BOOL) dwValue;
    // XXX
    if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWTOGGLEICON), &dwValue))
        fShowInputModeIcon    = (BOOL) dwValue;
#endif
#endif
    pLangBarItemMgr    = _QueryLangBarItemMgr ();
    if (pLangBarItemMgr != NULL) {
        ITfLangBarItem*        pItem;
        ITfLangBarItem*        pNewItem;
        
        if (fShowInputModeIcon) {
            pItem    = NULL;
            if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonCMode, &pItem)) &&
                pItem != NULL) {
                pLangBarItemMgr->RemoveItem (pItem);
                if (fShowInputModeIcon) 
                    pLangBarItemMgr->AddItem (pItem);
                pItem->Release ();
            }
        }

        if (fShowShapeIcon) {
            pItem    = NULL;
            if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonShape, &pItem)) &&
                pItem != NULL) {
                pLangBarItemMgr->RemoveItem (pItem);
                if (fShowShapeIcon) 
                    pLangBarItemMgr->AddItem (pItem);
                pItem->Release ();
            }
        }

        if (fShowIMEIcon) {
            pItem    = NULL;
            if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonIME, &pItem)) &&
                pItem != NULL) {
                pLangBarItemMgr->RemoveItem (pItem);
                if (fShowIMEIcon) 
                    pLangBarItemMgr->AddItem (pItem);
                pItem->Release ();
            }
        }

        if (fShowPadIcon) {
            pItem    = NULL;
            if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonPad, &pItem)) &&
                pItem != NULL) {
                pLangBarItemMgr->RemoveItem (pItem);
                if (fShowPadIcon) 
                    pLangBarItemMgr->AddItem (pItem);
                pItem->Release ();
            }
        }

        pLangBarItemMgr->Release ();
    }
#if DEBUG
    _DumpLangBarItem();
#endif
  exit_func:
#if defined (TSF_NEED_MUTEX)
    ReleaseMutex (hMutex);
  exit_func_1:
    CloseHandle (hMutex);
#endif
    return    fRetval;
}

/*    åëåÞ«Ð?ªË«¢«¤«Æ?ªòõÚÊ¥/Þûð¶ª¹ªë¡£
 */
void    PASCAL
ActivateLanguageBar (
    register BOOL    fSelect)
{
    register ITfLangBarItemMgr*    pLangBarItemMgr            = NULL;
    register ITfLangBarItem*    pLangBarItem            = NULL;
#if defined (TSF_NEED_MUTEX)
    register HANDLE                hMutex;
#endif
    register DWORD                dwValue;
    register BOOL               fShowIMEIcon, fShowInputModeIcon ,fShowShapeIcon, fShowPadIcon;
    
    TF_LANGBARITEMINFO MypInfo;
#if 1
    if (gfSaenaruSecure)
        return;
#endif

#if defined (TSF_NEED_MUTEX)
    hMutex    = SaenaruCreateMutex (SAENARU_MUTEX_NAME);
    if (!hMutex) 
        return;

    dwValue    = WaitForSingleObject (hMutex, TSF_WAITTIME);
    if (dwValue == WAIT_FAILED || dwValue == WAIT_TIMEOUT) {
        goto    Exit_Func_1;
    }
#endif
    if (shMSCTF == NULL)
        goto    Exit_Func;

    DEBUGPRINTFEX (100, (TEXT ("ActivateLanguageBar (Select:%d)\n"), fSelect));

    /*    ?üÞ«ì«¸«¹«È«êªò?ðÎª¹ªëªÎªÏïáª·ª¤ªÎªÀªíª¦ª«£¿ */
    fShowIMEIcon = fShowInputModeIcon = fShowShapeIcon = TRUE;
    fShowPadIcon = FALSE;
    if (fSelect) {
#if 0
#if !defined (NO_TOUCH_REGISTRY)
        if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWIMEICON), &dwValue))
            fShowIMEIcon        = (BOOL) dwValue;
        if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWINPUTMODEICON), &dwValue))
            fShowInputModeIcon    = (BOOL) dwValue;
#endif
#endif
    }

    pLangBarItemMgr    = _QueryLangBarItemMgr ();
    if (pLangBarItemMgr != NULL) {
        ITfLangBarItem*        pItem;
        ITfLangBarItem*        pNewItem;
        BOOL                fRemove;

        pItem    = NULL;
        fRemove    = FALSE;

        if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonCMode, &pItem)) &&
            pItem != NULL) {
            fRemove    = SUCCEEDED (pLangBarItemMgr->RemoveItem (pItem));
            pItem->Release ();
        }

        if ((pItem == NULL || fRemove) && fSelect && fShowInputModeIcon) {
            pNewItem    = NULL;
            if (CreateItemButtonCMode (&pNewItem) && pNewItem != NULL) {
                pLangBarItemMgr->AddItem (pNewItem);
                pNewItem->Release ();
            }
        }
        pItem    = NULL;
        fRemove    = FALSE;

        if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonShape, &pItem)) &&
            pItem != NULL) {
            fRemove    = SUCCEEDED (pLangBarItemMgr->RemoveItem (pItem));
            pItem->Release ();
        }

        if ((pItem == NULL || fRemove) && fSelect && fShowShapeIcon) {
            pNewItem    = NULL;
            if (CreateItemButtonShape (&pNewItem) && pNewItem != NULL) {
                pLangBarItemMgr->AddItem (pNewItem);
                pNewItem->Release ();
            }
        }
        pItem    = NULL;
        fRemove    = FALSE;


        if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonIME, &pItem)) &&
            pItem != NULL) {
            fRemove    = SUCCEEDED (pLangBarItemMgr->RemoveItem (pItem));
            pItem->Release ();
        }
        if ((pItem == NULL || fRemove) && fSelect && fShowIMEIcon) {
            pNewItem    = NULL;
            if (CreateItemButtonIME (&pNewItem) && pNewItem != NULL) {
                pLangBarItemMgr->AddItem (pNewItem);
                pNewItem->Release ();
            }
        }
        pItem    = NULL;
        fRemove    = FALSE;
#if 1
        if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonPad, &pItem)) &&
            pItem != NULL) {
            fRemove    = SUCCEEDED (pLangBarItemMgr->RemoveItem (pItem));
            pItem->Release ();
        }

        if ((pItem == NULL || fRemove) && fSelect && fShowPadIcon) {
            pNewItem    = NULL;
            if (CreateItemButtonPad (&pNewItem) && pNewItem != NULL) {
                pLangBarItemMgr->AddItem (pNewItem);
                pNewItem->Release ();
            }
        }
        pItem    = NULL;
        fRemove    = FALSE;
#endif
        pLangBarItemMgr->Release ();
    }
  Exit_Func:
#if defined (TSF_NEED_MUTEX)
    ReleaseMutex (hMutex);
  Exit_Func_1:
    CloseHandle (hMutex);
#endif
    return;
}

void    PASCAL
UninitLanguageBar (void)
{
    register ITfLangBarItemMgr*    pLangBarItemMgr            = NULL;
    register int    i;

#if 1
    if (gfSaenaruSecure) {
        return;
    }
#endif

#ifdef USE_HELPSINK
    DEBUGPRINTF ((TEXT ("Enter::UninitLanguageBar\n")));
    // http://msdn2.microsoft.com/en-us/library/ms628993.aspx
    pLangBarItemMgr = _QueryLangBarItemMgr();
    if (pLangBarItemMgr != NULL) {
        ITfLangBarItem* pItem;
        ITfSource* pSource;
        
        if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidItemButtonHelp, &pItem)) &&
            pItem != NULL) {
            if (SUCCEEDED (pItem->QueryInterface (IID_ITfSource,
                        (void**)&pSource)) && pSource != NULL) {
                pSource->UnadviseSink(g_dwHelpButtonCookie);
                pSource->Release();
                g_dwHelpButtonCookie= NULL;
                DEBUGPRINTF ((TEXT ("OK ? ::UninitLanguageBar\n")));
            }
            pItem->Release();
        }
        pLangBarItemMgr->Release();
    }
#endif
    if (shMSCTF != NULL) {
        FreeLibrary (shMSCTF);
        shMSCTF    = NULL;
    }
    g_fInitTSF    = FALSE;
    return;
}

ITfThreadMgr*
_QueryThreadMgr (void)
{
    PTF_CREATETHREADMGR    pfnCreateThreadMgr    = NULL;
    ITfThreadMgr*        pThreadMgr            = NULL;
    HRESULT                hr;

    if (shMSCTF == NULL)
        return    NULL;

    pfnCreateThreadMgr = (PTF_CREATETHREADMGR)GetProcAddress (shMSCTF, "TF_CreateThreadMgr");
    if(pfnCreateThreadMgr) {
        hr = (*pfnCreateThreadMgr)(&pThreadMgr);
        if (SUCCEEDED (hr) && pThreadMgr != NULL) 
            return    pThreadMgr;
    }
    return    NULL;
}

ITfLangBarItemMgr*
_QueryLangBarItemMgr (void)
{
    PTF_CREATELANGBARITEMMGR    pfnCreateLangBarItemMgr    = NULL;
    ITfLangBarItemMgr*            pLangBarItemMgr            = NULL;
    HRESULT                hr;

    if (shMSCTF == NULL)
        return    NULL;

    pfnCreateLangBarItemMgr    = (PTF_CREATELANGBARITEMMGR)GetProcAddress (shMSCTF, "TF_CreateLangBarItemMgr");
    if (pfnCreateLangBarItemMgr == NULL) 
        return    NULL;

    hr = (*pfnCreateLangBarItemMgr)(&pLangBarItemMgr);
    if (SUCCEEDED (hr)) 
        return    pLangBarItemMgr;
    return    NULL;
}

HIMC
_GetCurrentHIMC (void)
{
    register HIMC    hIMC    = NULL;

    register HWND    hwnd    = GetFocus ();
    if (hwnd != NULL)
        hIMC    = ImmGetContext (hwnd);
    return    hIMC;
}

BOOL PASCAL
IsLangBarEnabled(void)
{
    HKEY    hKey;
    DWORD   dwRegType, dwData, dwDataSize, dwRet;
    BOOL    fRet = TRUE;

    if (gfSaenaruSecure)
        return FALSE;

    dwData = 0;
    dwDataSize = sizeof(DWORD);
    if (RegOpenKeyEx(HKEY_CURRENT_USER, TEXT("Software\\Microsoft\\CTF"),
                0, KEY_READ,&hKey) == ERROR_SUCCESS) {
        dwRet = RegQueryValueEx(hKey, TEXT("Disable Thread Input Manager"),
                NULL, &dwRegType, (LPBYTE)&dwData, &dwDataSize);
        RegCloseKey(hKey);
        if (dwRet == ERROR_SUCCESS && dwRegType == REG_DWORD)
             fRet = (dwData == 0);
    }
    return  fRet;
    /*
    ITfLangBarItemMgr* pLangBarItemMgr = NULL;
    ITfLangBarItem* pItem;
    BOOL fRet = FALSE;

    pLangBarItemMgr = _QueryLangBarItemMgr();
    if (pLangBarItemMgr == NULL) 
        return FALSE;

    if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidKeyboardItemButton, &pItem))) {
        pItem->Release();
        fRet = TRUE;
    }
    pLangBarItemMgr->Release();
    */
}


#if DEBUG
void
_DumpLangBarItem (void)
{
    register ITfLangBarItemMgr*	pLangBarItemMgr	= NULL;
    ITfLangBarItem* pItem;
    IEnumTfLangBarItems* pEnum;

    pLangBarItemMgr = _QueryLangBarItemMgr();
    if (pLangBarItemMgr == NULL) 
	return ;

    MyDebugPrint((TEXT("--[Enter: _DumpLangBarItem (%d)]--\n"), (int) GetTickCount())); 
    if (SUCCEEDED (pLangBarItemMgr->EnumItems(&pEnum))) {
	ITfLangBarItem*	rpItems[16]= { NULL };
	ULONG cItems = 0 ;
	if (SUCCEEDED (pEnum->Next (16, rpItems, &cItems))) {
	    ULONG i;
	    for (i = 0; i < cItems; i ++) {
		TF_LANGBARITEMINFO info;
		if (SUCCEEDED (rpItems [i]->GetInfo (&info))) {
		    const BYTE*	pBytes = (const BYTE *) &(info.guidItem);
		    MyDebugPrint((TEXT("(%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x)\n"),
			pBytes [0], pBytes [1], pBytes [2], pBytes [3],
			pBytes [4], pBytes [5], pBytes [6], pBytes [7], 
			pBytes [8], pBytes [9], pBytes [10], pBytes [11],
			pBytes [12], pBytes [13], pBytes [14], pBytes [15]));
                    MyDebugPrint((TEXT("Style: %0x\n"),info.dwStyle));
                    MyDebugPrint((TEXT("Desc: %s\n"),info.szDescription));
		}
	    }
	    for (i = 0 ; i < cItems ; i ++) {
		rpItems [i]->Release ();
		rpItems [i] = NULL;
	    }
	}
	pEnum->Release ();
    }
    return;
}
#endif

#endif

/*
 * ex: ts=8 sts=4 sw=4 et
 */
