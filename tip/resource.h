//
// resource.h
//
// Resource declarations.
//

#ifndef RESOURCE_H
#define RESOURCE_H

//#define INDIC_PENINDIC   0
//#define INDIC_SAENARU    1
//#define INDIC_HAN        2
//#define INDIC_ENG        3

/* string tables */
#define IDS_SAENARU_DESC 1000
#define IDS_HELP_DESC    1001
#define IDS_ABOUT        1002

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
