/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 1990-1998 Microsoft Corporation.
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
 * $Saenaru: saenaru/src/data.c,v 1.5 2006/10/08 09:16:02 wkpark Exp $
 */

#include "windows.h"
#include "immdev.h"
#define _NO_EXTERN_
#include "saenaru.h"
#include "resource.h"

HINSTANCE   hInst;
HANDLE      hMutex = NULL;
HKL hMyKL = 0;

/* for Translat */
LPTRANSMSGLIST lpCurTransKey= NULL;
UINT     uNumTransKey;
BOOL     fOverTransKey = FALSE;

/* for UI */
TCHAR    szUIClassName[]      = TEXT("SAENARUUI");
TCHAR    szCompStrClassName[] = TEXT("SAENARUCompStr");
TCHAR    szCandClassName[]    = TEXT("SAENARUCand");
TCHAR    szStatusClassName[]  = TEXT("SAENARUStatus");
TCHAR    szGuideClassName[]   = TEXT("SAENARUGuide");

MYGUIDELINE glTable[] = {
        {GL_LEVEL_ERROR,   GL_ID_NODICTIONARY, IDS_GL_NODICTIONARY, 0},
        {GL_LEVEL_WARNING, GL_ID_TYPINGERROR,  IDS_GL_TYPINGERROR, 0},
        {GL_LEVEL_WARNING, GL_ID_PRIVATE_FIRST,IDS_GL_TESTGUIDELINESTR, IDS_GL_TESTGUIDELINEPRIVATE}
        };

/* for DIC */
TCHAR    szDicFileName[256];         /* Dictionary file name stored buffer */



#pragma data_seg("SHAREDDATA")
#ifdef DEBUG
/* for DebugOptions */
DWORD dwLogFlag = 0L;
DWORD dwDebugFlag = 0L;
#endif
DWORD dwLayoutFlag = 0L;
DWORD dwComposeFlag = 0L;
DWORD dwOptionFlag = BACKSPACE_BY_JAMO;
DWORD dwHanjaMode = 0L;
DWORD dwScanCodeBased = 0L;
DWORD dwToggleKey = 0L;

BOOL gfSaenaruSecure = FALSE;

/* local flags */
DWORD dwImeFlag = SAENARU_ONTHESPOT;
#pragma data_seg()

/*
 * ex: ts=8 sts=4 sw=4 et
 */
