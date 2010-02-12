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
 * $Saenaru: saenaru/src/tsf.h,v 1.6 2006/11/20 08:44:02 wkpark Exp $
 */

#if !defined (tsf_h)
#define tsf_h

#ifndef ARRAYSIZE
#define ARRAYSIZE(array)    (sizeof (array) / sizeof (array[0]))
#endif

inline void SafeStringCopy (WCHAR *pchDst, ULONG cchMax, const WCHAR *pchSrc)
{
    if (cchMax > 0)
    {
        wcsncpy(pchDst, pchSrc, cchMax);
        pchDst[cchMax-1] = '\0';
    }
}

typedef HRESULT (WINAPI *PTF_CREATETHREADMGR)(ITfThreadMgr**) ;
typedef HRESULT (WINAPI *PTF_CREATELANGBARITEMMGR)(ITfLangBarItemMgr**);
typedef struct {
    //const WCHAR*    pchDesc ;
    UINT      chDesc ;
    void    (*pfnHandler)(void) ;
} TSFLBMENUINFO ;

extern const CLSID c_clsidSaenaruTextService ;
extern const GUID c_guidSaenaruProfile ;
extern const GUID c_guidItemButtonCMode ;
extern const GUID c_guidItemButtonIME ;
extern const GUID c_guidItemButtonShape ;
extern const GUID c_guidItemButtonPad ;

/* prototypes */
HIMC _GetCurrentHIMC(void);
BOOL CreateItemButtonCMode(ITfLangBarItem** ppLangBarItem);
BOOL CreateItemButtonIME(ITfLangBarItem** ppLangBarItem);
BOOL CreateItemButtonShape(ITfLangBarItem** ppLangBarItem);
BOOL CreateItemButtonPad(ITfLangBarItem** ppLangBarItem);
BOOL CreateItemHelp(ITfSystemLangBarItemSink** ppLangBarItemSink);
BOOL IsLangBarEnabled(VOID);

LONG DllAddRef(void);
LONG DllRelease(void);

#endif

/*
 * ex: ts=8 sts=4 sw=4 et
 */
