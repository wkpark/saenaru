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
 * $Saenaru: saenaru/src/tsf.cpp,v 1.2 2003/12/26 09:26:33 perky Exp $
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

static    HMODULE            shMSCTF                = NULL;
static    TfClientId        g_tfClientID;
static    BOOL            g_fInitTSF            = FALSE;

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
#if 0
    if (gfSaenaruSecure) {
        DEBUGPRINTF ((TEXT ("Leave::InitLanguageBar - security\n")));
        return    FALSE;
    }
#endif
#if defined (TSF_NEED_MUTEX)
    hMutex = saenaruCreateMutex (SKKIME_MUTEX_NAME);
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
    register BOOL   fShowKeyboardIcon, fShowIMEIcon, fShowInputModeIcon, fShowShapeIcon;

#if 0
    if (gfSaenaruSecure)
        return    FALSE;
#endif

#if defined (TSF_NEED_MUTEX)
    hMutex    = saenaruCreateMutex (SKKIME_MUTEX_NAME);
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
    fShowKeyboardIcon = fShowIMEIcon = fShowInputModeIcon = fShowShapeIcon = TRUE;
#if 0
#if !defined (NO_TOUCH_REGISTRY)
    if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), &dwValue))
        fShowKeyboardIcon    = (BOOL) dwValue;
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
#if 1
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
#endif
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
        pLangBarItemMgr->Release ();
    }
#if 0
    _ShowKeyboardIcon (fShowKeyboardIcon);
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
    register BOOL                fShowKeyboardIcon, fShowIMEIcon, fShowInputModeIcon ,fShowShapeIcon;
    
    TF_LANGBARITEMINFO MypInfo;
#if 0
    if (gfSaenaruSecure)
        return;
#endif

#if defined (TSF_NEED_MUTEX)
    hMutex    = saenaruCreateMutex (SKKIME_MUTEX_NAME);
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
    fShowKeyboardIcon = fShowIMEIcon = fShowInputModeIcon = fShowShapeIcon = TRUE;
    if (fSelect) {
#if 0
#if !defined (NO_TOUCH_REGISTRY)
        if (GetRegDwordValue (TEXT ("\\CICERO"), TEXT(REGKEY_SHOWKEYBRDICON), &dwValue))
            fShowKeyboardIcon    = (BOOL) dwValue;
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
#if 1
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
#endif

#if 1
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
#endif

#if 1
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
#endif
        pLangBarItemMgr->Release ();
    }
#if 0
    if (fSelect)
        _ShowKeyboardIcon (fShowKeyboardIcon);
#endif
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

#if 0    
    if (gfSaenaruSecure) {
        return;
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

/*    «­???«É«¢«¤«³«ó(åëåÞ«Ð?ªÎ)ªÎ?ãÆ/Þª?ãÆªòï·ªêôðª¨ªë??¡£
 *¡Ø
 *    Text Service Framework ªÈ÷ÖùêªµªìªÆª¤ªëíÞùêªËªÏ«­???«ÉªÎ«¢«¤«³«óª¬
 *    ?ãÆªµªìªëª¬¡¢ª½ª¦ªÇªÊª¤íÞùêªËªÏ IME ICON ª¬××éÄªµªìªëªèª¦ªÇª¢ªë¡£
 */
BOOL
_ShowKeyboardIcon (
    register BOOL        fShow)
{
    register ITfLangBarItemMgr*    pLangBarItemMgr            = NULL;
    ITfLangBarItem*                    pItem;

    pLangBarItemMgr    = _QueryLangBarItemMgr ();
    if (pLangBarItemMgr == NULL) 
        return    FALSE;

    if (SUCCEEDED (pLangBarItemMgr->GetItem (c_guidKeyboardItemButton, &pItem))) {
        if (fShow) {
            ITfSystemLangBarItem*            pSysItem;
            ITfSystemDeviceTypeLangBarItem*    pSysDevItem;

            if (SUCCEEDED (pItem->QueryInterface (IID_ITfSystemDeviceTypeLangBarItem, (void**)&pSysDevItem)) && pSysDevItem != NULL) {
                pSysDevItem->SetIconMode (0);
                pSysDevItem->Release ();
            }

            /*    Text Service Framework ªÈ IME ª¬÷ÖùêªµªìªÆª¤ªÊª¤íÞùêªËªÏ¡¢
             *    IME Icon ª¬?ãÆªµªìªÆª·ªÞª¤¡¢ª½ªìª¬ Button ªÎ1ªÄªÈñìªÊªë
             *    ª¿ªáªË¡¢«­???«É«¢«¤«³«óªò?ÌÚª¹ªë¡£
             */
#if defined (not_integrate_tsf) || 0
            if (SUCCEEDED (pItem->QueryInterface (IID_ITfSystemLangBarItem, (void**)&pSysItem)) && pSysItem != NULL) {
                register HICON    hIcon;
                hIcon    = (HICON) LoadImage (hInst, TEXT ("INDICOPENICON"), IMAGE_ICON, 16, 16, 0);
                if (hIcon != NULL) {
                    pSysItem->SetIcon (hIcon);
                }
                pSysItem->Release ();
            }
#endif
        }
        pItem->Show (fShow);
        pItem->Release ();
    }
    pLangBarItemMgr->Release ();
    return    TRUE;
}

#endif

/*
 * ex: ts=8 sts=4 sw=4 et
 */
