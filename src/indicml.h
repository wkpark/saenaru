/*
 * This file is part of Saenaru.
 *
 * Copyright (c) 1993-1997  Microsoft Corporation.
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
 * $Saenaru: saenaru/src/indicml.h,v 1.2 2003/12/26 08:28:43 perky Exp $
 */

#ifndef _INDICML_
#define _INDICML_        // defined if INDICML.H has been included

#ifdef __cplusplus
extern "C" {
#endif

//---------------------------------------------------------------------
//
// The messages for Indicator Window.
//
//---------------------------------------------------------------------
#define INDICM_SETIMEICON                 (WM_USER+100)
#define INDICM_SETIMETOOLTIPS             (WM_USER+101)
#define INDICM_REMOVEDEFAULTMENUITEMS     (WM_USER+102)

//---------------------------------------------------------------------
//
// The wParam for INDICM_REMOVEDEFAULTMEUITEMS
//
//---------------------------------------------------------------------
#define RDMI_LEFT         0x0001
#define RDMI_RIGHT        0x0002

//---------------------------------------------------------------------
//
// INDICATOR_WND will be used by the IME to find indicator window.
// IME should call FindWindow(INDICATOR_WND) to get it.
//
//---------------------------------------------------------------------
#ifdef _WIN32

#define INDICATOR_CLASSW         L"Indicator"
#define INDICATOR_CLASSA         "Indicator"

#ifdef UNICODE
#define INDICATOR_CLASS          INDICATOR_CLASSW
#else
#define INDICATOR_CLASS          INDICATOR_CLASSA
#endif

#else
#define INDICATOR_CLASS          "Indicator"
#endif

#ifdef __cplusplus
}
#endif

#endif  // _INDICML_

/*
 * ex: ts=8 sts=4 sw=4 et
 */
