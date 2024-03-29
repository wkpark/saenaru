//
// resource.h
//
// Resource declarations.
//

#ifndef RESOURCE_H
#define RESOURCE_H

/* Property Sheet DLG ID */
#define DLG_GENERAL             125
#define DLG_ABOUT               126
#define DLG_DEBUG               127
#define DLG_REGISTERWORD        128
#define DLG_SELECTDICTIONARY    129
#define DLG_TOGGLE_KEY          130

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

#define IDC_LSHIFT              401
#define IDC_RSHIFT              402
#define IDC_LCTRL               403
#define IDC_RCTRL               404
#define IDC_LALT                405
#define IDC_RALT                406
#define IDC_TOGGLE              407

#define IDI_GENERAL             102
#define IDI_ABOUT               103

#define IDC_LAYOUT_OLD2BUL      1001
#define IDC_LAYOUT_3FIN         1002
#define IDC_LAYOUT_390          1003
#define IDC_LAYOUT_3SUN         1004
#define IDC_LAYOUT_NEW2BUL      1005
#define IDC_LAYOUT_NEW3BUL      1006
#define IDC_LAYOUT_AHNMATAE     1007
#define IDC_LAYOUT_YET2BUL      1008
#define IDC_LAYOUT_YET3BUL      1009
#define IDC_LAYOUT_NK_2BUL      1010
#define IDC_LAYOUT_USER         1011

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
#define IDC_HANJA_CAND_SPACE    2011
#define IDC_SIMILAR_WORDS       2012
#define IDC_HANGUL_TOGGLE       2013

//#define INDIC_PENINDIC   0
//#define INDIC_SAENARU    1
//#define INDIC_HAN        2
//#define INDIC_ENG        3

/* string tables */
#define IDS_SAENARU_DESC 1000
#define IDS_HELP_DESC    1001
#define IDS_ABOUT        1002
#define IDS_CONFIGNAME   1003

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

#define IDS_HANGUL_TOGGLE           1100
#define IDS_HANJA_CONVERT           1101

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
#define IDS_MENU_3SOON              2104
#define IDS_MENU_NEW2BUL            2105
#define IDS_MENU_NEW3BUL            2106
#define IDS_MENU_AHNMATAE           2107
#define IDS_MENU_YET2BUL            2108
#define IDS_MENU_YET3BUL            2109
#define IDS_MENU_NK_2BUL            2110
#define IDS_MENU_USER               2111

#if 1
#define IDS_START_END_COMPOSITION   9000
#define IDS_GET_GLOBAL_COMPART      9001
#define IDS_SET_CASE_PROP           9002
#define IDS_SET_CUSTOM_PROP         9003
#define IDS_GET_CASE_PROP           9004
#define IDS_GET_CUSTOM_PROP         9005
#endif

#define IDS_TIP_ASCII               3999
#define IDS_TIP_HANGUL              4000
#define IDS_TIP_OLD2BUL             4001
#define IDS_TIP_3FIN                4002
#define IDS_TIP_390                 4003
#define IDS_TIP_3SOON               4004
#define IDS_TIP_YET2BUL             4005
#define IDS_TIP_YET3BUL             4006
#define IDS_TIP_NK_2BUL             4007
#define IDS_TIP_NEW2BUL             4008
#define IDS_TIP_NEW3BUL             4009
#define IDS_TIP_AHNMATAE            4010
#define IDS_TIP_USER                4011

#define IDS_MENU_USER_COMP          2120
#define IDS_MENU_ENV                2131
#define IDS_MENU_DVORAK             2132
#define IDS_MENU_ESC_ASCII          2133
#define IDS_MENU_EDIT_WORD_UNIT     2134
#define IDS_MENU_SHOW_KEYBOARD      2135
#define IDS_MENU_HANGUL_TOGGLE      2136

#define IDC_STATIC                    -1

#define IDS_KEY_NEW2BUL             3105
#define IDS_KEY_NEW3BUL             3106
#define IDS_KEY_AHNMATAE            3107
#define IDS_KEY_YET2BUL             3108
#define IDS_KEY_YET3BUL             3109
#define IDS_KEY_NK_2BUL             3110

#define IDM_CONFIG     		40000
#define IDM_OLD2BUL		40001
#define IDM_3BUL		40002
#define IDM_390			40003
#define IDM_3SOON		40004
#define IDM_NEW2BUL		40005
#define IDM_NEW3BUL		40006
#define IDM_AHNMATAE		40007
#define IDM_YET2BUL		40008
#define IDM_YET3BUL		40009
#define IDM_NK_2BUL 		40010

#define IDM_USER		40011
#define IDM_USER_COMP		40020
#define IDM_DVORAK		40031
#define IDM_ESC_ASCII		40032
#define IDM_WORD_UNIT		40033
#define IDM_SHOW_KEYBOARD	40041
#define IDM_HANGUL_TOGGLE       40042
#define IDM_HANGUL		40051
#define IDM_ENG			40052
#define IDM_HANJA		40053
#define IDM_RECONVERT		40061
#define IDM_ABOUT      		40062

#endif // RESOURCE_H

/*
 * vim: et sts=4 sw=4
 */
