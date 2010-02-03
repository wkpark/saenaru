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
 * $Saenaru: saenaru/src/resource.h,v 1.11 2006/11/20 08:42:28 wkpark Exp $
 */

//{{NO_DEPENDENCIES}}
// Microsoft Developer Studio generated include file.
// Used by saenaru.rc
//
#define VER_DEBUG                       0

/* Property Sheet DLG ID */
#define DLG_GENERAL             125
#define DLG_ABOUT               126
#define DLG_DEBUG               127

/* for dialog box */
#define ID_WR_READING           200
#define ID_WR_STRING            201
#define ID_WR_STYLE             202

#define ID_SD_SELDIC            301
#define ID_SD_DICPATH           302

#define IDC_LOGF_ENTRY          310
#define IDC_LOGF_API            311
#define IDC_LOGF_KEY            312
#define IDC_DEBF_THREADID       313
#define IDC_DEBF_GUIDELINE      314

#define IDI_GENERAL             102
#define IDI_ABOUT               103

#define IDC_LAYOUT_OLD2BUL      1001
#define IDC_LAYOUT_3FIN         1002
#define IDC_LAYOUT_390          1003
#define IDC_LAYOUT_NEW2BUL      1004
#define IDC_LAYOUT_NEW3BUL      1005
#define IDC_LAYOUT_AHNMATAE     1006
#define IDC_LAYOUT_3SUN         1007
#define IDC_LAYOUT_USER         1008

#define IDC_CONCURRENT_TYPING   2000
#define IDC_COMPOSITE_TYPING    2001
#define IDC_KSX1001_SUPPORT     2002
#define IDC_KSX1002_SUPPORT     2003
#define IDC_DVORAK_SUPPORT      2004
#define IDC_BACKSPACE_BY_JAMO   2005
#define IDC_FULL_MULTIJOMO      2006
#define IDC_USE_SHIFT_SPACE     2007
#define IDC_ESCENG_SUPPORT      2008
#define IDC_QWERTY_HOTKEY_SUPPORT 2009
#define IDC_JAMOS               2010

/* string tables */
#define IDS_CONFIGNAME              1000
#define IDS_HELP_DESC               1001
#define IDS_ABOUT                   1002
#define IDS_DICFILENAME             1003
#define IDS_DIC_KEY                 1005
#define IDS_SYM_KEY                 1006
#define IDS_WORD_KEY                1007
#define IDS_HIDX_KEY                1008

#define IDS_GL_NODICTIONARY         1010
#define IDS_GL_TYPINGERROR          1011
#define IDS_GL_TESTGUIDELINESTR     1012
#define IDS_GL_TESTGUIDELINEPRIVATE 1013
#define IDS_NOREADING               1020
#define IDS_NOSTRING                1021
#define IDS_NOMEMORY                1022
#define IDS_REGWORDRET              1023

#define IDS_INPUT_CMODE_DESC        2000
#define IDS_TOGGLE_HALFFULL_DESC    2001
#define IDS_KEYBOARD_OPTION_DESC    2100
#define IDS_MENU_HANGUL             2002
#define IDS_MENU_ASCII              2003
#define IDS_MENU_HANJA              2004
#define IDS_MENU_CANCEL             2005
#define IDS_MENU_OLD2BUL            2101
#define IDS_MENU_3FIN               2102
#define IDS_MENU_390                2103
#define IDS_MENU_NEW2BUL            2104
#define IDS_MENU_NEW3BUL            2105
#define IDS_MENU_AHNMATAE           2106
#define IDS_MENU_3SOON              2107
#define IDS_MENU_USER               2108
#define IDS_MENU_USER_COMP          2109

#define IDS_MENU_ENV                2111
#define IDS_MENU_DVORAK             2112
#define IDS_MENU_ESC_ASCII          2113
#define IDS_MENU_EDIT_WORD_UNIT     2114
#define IDS_MENU_SHOW_KEYBOARD      2115

#define IDC_STATIC                      -1

#define IDM_CONFIG     		40000
#define IDM_OLD2BUL		40001
#define IDM_3BUL		40002
#define IDM_390			40003
#define IDM_NEW2BUL		40004
#define IDM_NEW3BUL		40005
#define IDM_AHNMATAE		40006
#define IDM_3SOON		40007
#define IDM_USER		40010
#define IDM_USER_COMP		40020
#define IDM_DVORAK		40031
#define IDM_ESC_ASCII		40032
#define IDM_WORD_UNIT		40033
#define IDM_SHOW_KEYBOARD	40041
#define IDM_HANGUL		40051
#define IDM_ENG			40052
#define IDM_HANJA		40053
#define IDM_RECONVERT		40061
#define IDM_ABOUT      		40062

#define IDM_SKL1                0x0500
#define IDM_SKL2                0x0501
#define IDM_SKL3                0x0502
#define IDM_SKL4                0x0503
#define IDM_SKL5                0x0504
#define IDM_SKL6                0x0505
#define IDM_SKL7                0x0506
#define IDM_SKL8                0x0507
#define IDM_SKL9                0x0508
#define IDM_SKL10               0x0509
#define IDM_SKL11               0x050a
#define IDM_SKL12               0x050b
#define IDM_SKL13               0x050c

// Next default values for new objects
// 
#ifdef APSTUDIO_INVOKED
#ifndef APSTUDIO_READONLY_SYMBOLS
#define _APS_NEXT_RESOURCE_VALUE        104
#define _APS_NEXT_COMMAND_VALUE         40001
#define _APS_NEXT_CONTROL_VALUE         1011
#define _APS_NEXT_SYMED_VALUE           101
#endif
#endif

