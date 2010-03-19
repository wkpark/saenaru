/***************************************************************************\
* Module Name: kbdcmk.c
*
* keyboard layout
*
* Copyright (c) 2006 Shai Coleman colemak.com. Public Domain
*
* History:
*  - Hanja/Hangul key added by wkpark@kldp.org 2010/03/15
*  - KBDTOOL v3.40 - Created  Sat Mar 13 01:25:57 2010
\***************************************************************************/

#include <windows.h>
#include "kbdcmk.h"

#if defined(_M_IA64)
#pragma section(".data")
#define ALLOC_SECTION_LDATA __declspec(allocate(".data"))
#else
#pragma data_seg(".data")
#define ALLOC_SECTION_LDATA
#endif

/***************************************************************************\
* ausVK[] - Virtual Scan Code to Virtual Key conversion table
\***************************************************************************/

static ALLOC_SECTION_LDATA USHORT ausVK[] = {
    T00, T01, T02, T03, T04, T05, T06, T07,
    T08, T09, T0A, T0B, T0C, T0D, T0E, T0F,
    T10, T11, T12, T13, T14, T15, T16, T17,
    T18, T19, T1A, T1B, T1C, T1D, T1E, T1F,
    T20, T21, T22, T23, T24, T25, T26, T27,
    T28, T29, T2A, T2B, T2C, T2D, T2E, T2F,
    T30, T31, T32, T33, T34, T35,

    /*
     * Right-hand Shift key must have KBDEXT bit set.
     */
    T36 | KBDEXT,

    T37 | KBDMULTIVK,               // numpad_* + Shift/Alt -> SnapShot

    T38, T39, T3A, T3B, T3C, T3D, T3E,
    T3F, T40, T41, T42, T43, T44,

    /*
     * NumLock Key:
     *     KBDEXT     - VK_NUMLOCK is an Extended key
     *     KBDMULTIVK - VK_NUMLOCK or VK_PAUSE (without or with CTRL)
     */
    T45 | KBDEXT | KBDMULTIVK,

    T46 | KBDMULTIVK,

    /*
     * Number Pad keys:
     *     KBDNUMPAD  - digits 0-9 and decimal point.
     *     KBDSPECIAL - require special processing by Windows
     */
    T47 | KBDNUMPAD | KBDSPECIAL,   // Numpad 7 (Home)
    T48 | KBDNUMPAD | KBDSPECIAL,   // Numpad 8 (Up),
    T49 | KBDNUMPAD | KBDSPECIAL,   // Numpad 9 (PgUp),
    T4A,
    T4B | KBDNUMPAD | KBDSPECIAL,   // Numpad 4 (Left),
    T4C | KBDNUMPAD | KBDSPECIAL,   // Numpad 5 (Clear),
    T4D | KBDNUMPAD | KBDSPECIAL,   // Numpad 6 (Right),
    T4E,
    T4F | KBDNUMPAD | KBDSPECIAL,   // Numpad 1 (End),
    T50 | KBDNUMPAD | KBDSPECIAL,   // Numpad 2 (Down),
    T51 | KBDNUMPAD | KBDSPECIAL,   // Numpad 3 (PgDn),
    T52 | KBDNUMPAD | KBDSPECIAL,   // Numpad 0 (Ins),
    T53 | KBDNUMPAD | KBDSPECIAL,   // Numpad . (Del),

    T54, T55, T56, T57, T58, T59, T5A, T5B,
    T5C, T5D, T5E, T5F, T60, T61, T62, T63,
    T64, T65, T66, T67, T68, T69, T6A, T6B,
    T6C, T6D, T6E, T6F, T70, T71, T72, T73,
    T74, T75, T76, T77, T78, T79, T7A, T7B,
    T7C, T7D, T7E

};

static ALLOC_SECTION_LDATA VSC_VK aE0VscToVk[] = {
        { 0x10, X10 | KBDEXT              },  // Speedracer: Previous Track
        { 0x19, X19 | KBDEXT              },  // Speedracer: Next Track
        { 0x1D, X1D | KBDEXT              },  // RControl
        { 0x20, X20 | KBDEXT              },  // Speedracer: Volume Mute
        { 0x21, X21 | KBDEXT              },  // Speedracer: Launch App 2
        { 0x22, X22 | KBDEXT              },  // Speedracer: Media Play/Pause
        { 0x24, X24 | KBDEXT              },  // Speedracer: Media Stop
        { 0x2E, X2E | KBDEXT              },  // Speedracer: Volume Down
        { 0x30, X30 | KBDEXT              },  // Speedracer: Volume Up
        { 0x32, X32 | KBDEXT              },  // Speedracer: Browser Home
        { 0x35, X35 | KBDEXT              },  // Numpad Divide
        { 0x37, X37 | KBDEXT              },  // Snapshot
        { 0x38, X38 | KBDEXT              },  // RMenu
        { 0x47, X47 | KBDEXT              },  // Home
        { 0x48, X48 | KBDEXT              },  // Up
        { 0x49, X49 | KBDEXT              },  // Prior
        { 0x4B, X4B | KBDEXT              },  // Left
        { 0x4D, X4D | KBDEXT              },  // Right
        { 0x4F, X4F | KBDEXT              },  // End
        { 0x50, X50 | KBDEXT              },  // Down
        { 0x51, X51 | KBDEXT              },  // Next
        { 0x52, X52 | KBDEXT              },  // Insert
        { 0x53, X53 | KBDEXT              },  // Delete
        { 0x5B, X5B | KBDEXT              },  // Left Win
        { 0x5C, X5C | KBDEXT              },  // Right Win
        { 0x5D, X5D | KBDEXT              },  // Application
        { 0x5F, X5F | KBDEXT              },  // Speedracer: Sleep
        { 0x65, X65 | KBDEXT              },  // Speedracer: Browser Search
        { 0x66, X66 | KBDEXT              },  // Speedracer: Browser Favorites
        { 0x67, X67 | KBDEXT              },  // Speedracer: Browser Refresh
        { 0x68, X68 | KBDEXT              },  // Speedracer: Browser Stop
        { 0x69, X69 | KBDEXT              },  // Speedracer: Browser Forward
        { 0x6A, X6A | KBDEXT              },  // Speedracer: Browser Back
        { 0x6B, X6B | KBDEXT              },  // Speedracer: Launch App 1
        { 0x6C, X6C | KBDEXT              },  // Speedracer: Launch Mail
        { 0x6D, X6D | KBDEXT              },  // Speedracer: Launch Media Selector
        { 0x1C, X1C | KBDEXT              },  // Numpad Enter
        { 0x46, X46 | KBDEXT              },  // Break (Ctrl + Pause)
        { 0xF1, XF1 | KBDEXT              },  // Hanja key
        { 0xF2, XF2 | KBDEXT              },  // Hangul key
        { 0,      0                       }
};

static ALLOC_SECTION_LDATA VSC_VK aE1VscToVk[] = {
        { 0x1D, Y1D                       },  // Pause
        { 0   ,   0                       }
};

/***************************************************************************\
* aVkToBits[]  - map Virtual Keys to Modifier Bits
*
* See kbd.h for a full description.
*
* The keyboard has only three shifter keys:
*     SHIFT (L & R) affects alphabnumeric keys,
*     CTRL  (L & R) is used to generate control characters
*     ALT   (L & R) used for generating characters by number with numpad
\***************************************************************************/
static ALLOC_SECTION_LDATA VK_TO_BIT aVkToBits[] = {
    { VK_SHIFT    ,   KBDSHIFT     },
    { VK_CONTROL  ,   KBDCTRL      },
    { VK_MENU     ,   KBDALT       },
    { 0           ,   0           }
};

/***************************************************************************\
* aModification[]  - map character modifier bits to modification number
*
* See kbd.h for a full description.
*
\***************************************************************************/

static ALLOC_SECTION_LDATA MODIFIERS CharModifiers = {
    &aVkToBits[0],
    7,
    {
    //  Modification# //  Keys Pressed
    //  ============= // =============
        0,            // 
        1,            // Shift 
        2,            // Control 
        SHFT_INVALID, // Shift + Control 
        SHFT_INVALID, // Menu 
        SHFT_INVALID, // Shift + Menu 
        3,            // Control + Menu 
        4             // Shift + Control + Menu 
     }
};

/***************************************************************************\
*
* aVkToWch2[]  - Virtual Key to WCHAR translation for 2 shift states
* aVkToWch3[]  - Virtual Key to WCHAR translation for 3 shift states
* aVkToWch4[]  - Virtual Key to WCHAR translation for 4 shift states
* aVkToWch5[]  - Virtual Key to WCHAR translation for 5 shift states
* aVkToWch6[]  - Virtual Key to WCHAR translation for 6 shift states
*
* Table attributes: Unordered Scan, null-terminated
*
* Search this table for an entry with a matching Virtual Key to find the
* corresponding unshifted and shifted WCHAR characters.
*
* Special values for VirtualKey (column 1)
*     0xff          - dead chars for the previous entry
*     0             - terminate the list
*
* Special values for Attributes (column 2)
*     CAPLOK bit    - CAPS-LOCK affect this key like SHIFT
*
* Special values for wch[*] (column 3 & 4)
*     WCH_NONE      - No character
*     WCH_DEAD      - Dead Key (diaresis) or invalid (US keyboard has none)
*     WCH_LGTR      - Ligature (generates multiple characters)
*
\***************************************************************************/

static ALLOC_SECTION_LDATA VK_TO_WCHARS2 aVkToWch2[] = {
//                      |         |  Shift  |
//                      |=========|=========|
  {VK_TAB       ,0      ,'\t'     ,'\t'     },
  {VK_ADD       ,0      ,'+'      ,'+'      },
  {VK_DIVIDE    ,0      ,'/'      ,'/'      },
  {VK_MULTIPLY  ,0      ,'*'      ,'*'      },
  {VK_SUBTRACT  ,0      ,'-'      ,'-'      },
  {0            ,0      ,0        ,0        }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS3 aVkToWch3[] = {
//                      |         |  Shift  |  Ctrl   |
//                      |=========|=========|=========|
  {VK_BACK      ,0      ,'\b'     ,'\b'     ,0x007f   },
  {VK_ESCAPE    ,0      ,0x001b   ,0x001b   ,0x001b   },
  {VK_RETURN    ,0      ,'\r'     ,'\r'     ,'\n'     },
  {VK_CANCEL    ,0      ,0x0003   ,0x0003   ,0x0003   },
  {0            ,0      ,0        ,0        ,0        }
};

static ALLOC_SECTION_LDATA VK_TO_WCHARS5 aVkToWch5[] = {
//                      |         |  Shift  |  Ctrl   |  Ctl+Alt|S+Ctl+Alt|
//                      |=========|=========|=========|=========|=========|
  {'1'          ,0      ,'1'      ,'!'      ,WCH_NONE ,0x00a1   ,0x00b9   },
  {'2'          ,0      ,'2'      ,'@'      ,WCH_NONE ,0x00ba   ,0x00b2   },
  {'3'          ,0      ,'3'      ,'#'      ,WCH_NONE ,0x00aa   ,0x00b3   },
  {'4'          ,0      ,'4'      ,'$'      ,WCH_NONE ,0x00a2   ,0x00a3   },
  {'5'          ,0      ,'5'      ,'%'      ,WCH_NONE ,0x20ac   ,0x00a5   },
  {'6'          ,CAPLOKALTGR,'6'      ,'^'      ,WCH_NONE ,0x0127   ,0x0126   },
  {'7'          ,CAPLOKALTGR,'7'      ,'&'      ,WCH_NONE ,0x00f0   ,0x00d0   },
  {'8'          ,CAPLOKALTGR,'8'      ,'*'      ,WCH_NONE ,0x00fe   ,0x00de   },
  {'9'          ,0      ,'9'      ,'('      ,WCH_NONE ,0x2018   ,0x201c   },
  {'0'          ,0      ,'0'      ,')'      ,WCH_NONE ,0x2019   ,0x201d   },
  {VK_OEM_MINUS ,0      ,'-'      ,'_'      ,WCH_NONE ,0x2013   ,0x2014   },
  {VK_OEM_PLUS  ,0      ,'='      ,'+'      ,WCH_NONE ,0x00d7   ,0x00f7   },
  {'Q'          ,CAPLOK | CAPLOKALTGR,'q'      ,'Q'      ,WCH_NONE ,0x00e4   ,0x00c4   },
  {'W'          ,CAPLOK | CAPLOKALTGR,'w'      ,'W'      ,WCH_NONE ,0x00e5   ,0x00c5   },
  {'F'          ,CAPLOK | CAPLOKALTGR,'f'      ,'F'      ,WCH_NONE ,0x00e3   ,0x00c3   },
  {'P'          ,CAPLOK | CAPLOKALTGR,'p'      ,'P'      ,WCH_NONE ,0x00f8   ,0x00d8   },
  {'G'          ,CAPLOK ,'g'      ,'G'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x02db   ,WCH_NONE },
  {'J'          ,CAPLOK | CAPLOKALTGR,'j'      ,'J'      ,WCH_NONE ,0x0111   ,0x0110   },
  {'L'          ,CAPLOK | CAPLOKALTGR,'l'      ,'L'      ,WCH_NONE ,0x0142   ,0x0141   },
  {'U'          ,CAPLOK | CAPLOKALTGR,'u'      ,'U'      ,WCH_NONE ,0x00fa   ,0x00da   },
  {'Y'          ,CAPLOK | CAPLOKALTGR,'y'      ,'Y'      ,WCH_NONE ,0x00fc   ,0x00dc   },
  {VK_OEM_1     ,CAPLOKALTGR,';'      ,':'      ,WCH_NONE ,0x00f6   ,0x00d6   },
  {VK_OEM_4     ,0      ,'['      ,'{'      ,WCH_NONE ,0x00ab   ,0x2039   },
  {VK_OEM_6     ,0      ,']'      ,'}'      ,WCH_NONE ,0x00bb   ,0x203a   },
  {'A'          ,CAPLOK | CAPLOKALTGR,'a'      ,'A'      ,WCH_NONE ,0x00e1   ,0x00c1   },
  {'R'          ,CAPLOK ,'r'      ,'R'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,'`'      ,WCH_NONE },
  {'S'          ,CAPLOK | CAPLOKALTGR,'s'      ,'S'      ,WCH_NONE ,0x00df   ,'~'      },
  {'T'          ,CAPLOK | CAPLOKALTGR,'t'      ,'T'      ,WCH_NONE ,WCH_DEAD ,WCH_DEAD },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x00b4   ,0x02dd   },
  {'D'          ,CAPLOK ,'d'      ,'D'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x00a8   ,WCH_NONE },
  {'H'          ,CAPLOK ,'h'      ,'H'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x02c7   ,WCH_NONE },
  {'N'          ,CAPLOK | CAPLOKALTGR,'n'      ,'N'      ,WCH_NONE ,0x00f1   ,0x00d1   },
  {'E'          ,CAPLOK | CAPLOKALTGR,'e'      ,'E'      ,WCH_NONE ,0x00e9   ,0x00c9   },
  {'I'          ,CAPLOK | CAPLOKALTGR,'i'      ,'I'      ,WCH_NONE ,0x00ed   ,0x00cd   },
  {'O'          ,CAPLOK | CAPLOKALTGR,'o'      ,'O'      ,WCH_NONE ,0x00f3   ,0x00d3   },
  {VK_OEM_7     ,CAPLOKALTGR,'\''     ,'\"'     ,WCH_NONE ,0x00f5   ,0x00d5   },
  {VK_OEM_3     ,0      ,'`'      ,'~'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,'~'      ,WCH_NONE },
  {VK_OEM_5     ,0      ,'\\'     ,'|'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,'*'      ,WCH_NONE },
  {'Z'          ,CAPLOK | CAPLOKALTGR,'z'      ,'Z'      ,WCH_NONE ,0x00e6   ,0x00c6   },
  {'X'          ,CAPLOK ,'x'      ,'X'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,'^'      ,WCH_NONE },
  {'C'          ,CAPLOK | CAPLOKALTGR,'c'      ,'C'      ,WCH_NONE ,0x00e7   ,0x00c7   },
  {'V'          ,CAPLOK | CAPLOKALTGR,'v'      ,'V'      ,WCH_NONE ,0x0153   ,0x0152   },
  {'B'          ,CAPLOK ,'b'      ,'B'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x02d8   ,WCH_NONE },
  {'K'          ,CAPLOK ,'k'      ,'K'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x02da   ,WCH_NONE },
  {'M'          ,CAPLOK | CAPLOKALTGR,'m'      ,'M'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x00af   ,WCH_NONE },
  {VK_OEM_COMMA ,CAPLOKALTGR,','      ,'<'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x00b8   ,WCH_NONE },
  {VK_OEM_PERIOD,CAPLOKALTGR,'.'      ,'>'      ,WCH_NONE ,WCH_DEAD ,'~'      },
  {0xff         ,0      ,WCH_NONE ,WCH_NONE ,WCH_NONE ,0x02d9   ,WCH_NONE },
  {VK_OEM_2     ,0      ,'/'      ,'?'      ,WCH_NONE ,0x00bf   ,'~'      },
  {VK_SPACE     ,0      ,' '      ,' '      ,WCH_NONE ,' '      ,0x00a0   },
  {VK_OEM_102   ,0      ,'-'      ,'_'      ,WCH_NONE ,0x2013   ,0x2014   },
  {VK_DECIMAL   ,0      ,'.'      ,'.'      ,WCH_NONE ,WCH_NONE ,WCH_NONE },
  {0            ,0      ,0        ,0        ,0        ,0        ,0        }
};

// Put this last so that VkKeyScan interprets number characters
// as coming from the main section of the kbd (aVkToWch2 and
// aVkToWch5) before considering the numpad (aVkToWch1).

static ALLOC_SECTION_LDATA VK_TO_WCHARS1 aVkToWch1[] = {
    { VK_NUMPAD0   , 0      ,  '0'   },
    { VK_NUMPAD1   , 0      ,  '1'   },
    { VK_NUMPAD2   , 0      ,  '2'   },
    { VK_NUMPAD3   , 0      ,  '3'   },
    { VK_NUMPAD4   , 0      ,  '4'   },
    { VK_NUMPAD5   , 0      ,  '5'   },
    { VK_NUMPAD6   , 0      ,  '6'   },
    { VK_NUMPAD7   , 0      ,  '7'   },
    { VK_NUMPAD8   , 0      ,  '8'   },
    { VK_NUMPAD9   , 0      ,  '9'   },
    { 0            , 0      ,  '\0'  }
};

static ALLOC_SECTION_LDATA VK_TO_WCHAR_TABLE aVkToWcharTable[] = {
    {  (PVK_TO_WCHARS1)aVkToWch3, 3, sizeof(aVkToWch3[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch5, 5, sizeof(aVkToWch5[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch2, 2, sizeof(aVkToWch2[0]) },
    {  (PVK_TO_WCHARS1)aVkToWch1, 1, sizeof(aVkToWch1[0]) },
    {                       NULL, 0, 0                    },
};

/***************************************************************************\
* aKeyNames[], aKeyNamesExt[]  - Virtual Scancode to Key Name tables
*
* Table attributes: Ordered Scan (by scancode), null-terminated
*
* Only the names of Extended, NumPad, Dead and Non-Printable keys are here.
* (Keys producing printable characters are named by that character)
\***************************************************************************/

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNames[] = {
    0x01,    L"Esc",
    0x0e,    L"Backspace",
    0x0f,    L"Tab",
    0x1c,    L"Enter",
    0x1d,    L"Ctrl",
    0x2a,    L"Shift",
    0x36,    L"Right Shift",
    0x37,    L"Num *",
    0x38,    L"Alt",
    0x39,    L"Space",
    0x3a,    L"Caps Lock",
    0x3b,    L"F1",
    0x3c,    L"F2",
    0x3d,    L"F3",
    0x3e,    L"F4",
    0x3f,    L"F5",
    0x40,    L"F6",
    0x41,    L"F7",
    0x42,    L"F8",
    0x43,    L"F9",
    0x44,    L"F10",
    0x45,    L"Pause",
    0x46,    L"Scroll Lock",
    0x47,    L"Num 7",
    0x48,    L"Num 8",
    0x49,    L"Num 9",
    0x4a,    L"Num -",
    0x4b,    L"Num 4",
    0x4c,    L"Num 5",
    0x4d,    L"Num 6",
    0x4e,    L"Num +",
    0x4f,    L"Num 1",
    0x50,    L"Num 2",
    0x51,    L"Num 3",
    0x52,    L"Num 0",
    0x53,    L"Num Del",
    0x54,    L"Sys Req",
    0x57,    L"F11",
    0x58,    L"F12",
    0x7c,    L"F13",
    0x7d,    L"F14",
    0x7e,    L"F15",
    0x7f,    L"F16",
    0x80,    L"F17",
    0x81,    L"F18",
    0x82,    L"F19",
    0x83,    L"F20",
    0x84,    L"F21",
    0x85,    L"F22",
    0x86,    L"F23",
    0x87,    L"F24",
    0   ,    NULL
};

static ALLOC_SECTION_LDATA VSC_LPWSTR aKeyNamesExt[] = {
    0x1c,    L"Num Enter",
    0x1d,    L"Right Ctrl",
    0x35,    L"Num /",
    0x37,    L"Prnt Scrn",
    0x38,    L"Right Alt",
    0x45,    L"Num Lock",
    0x46,    L"Break",
    0x47,    L"Home",
    0x48,    L"Up",
    0x49,    L"Page Up",
    0x4b,    L"Left",
    0x4d,    L"Right",
    0x4f,    L"End",
    0x50,    L"Down",
    0x51,    L"Page Down",
    0x52,    L"Insert",
    0x53,    L"Delete",
    0x54,    L"<00>",
    0x56,    L"Help",
    0x5b,    L"Left Windows",
    0x5c,    L"Right Windows",
    0x5d,    L"Application",
    0xf1,    L"Hanguel",
    0xf1,    L"Hanja",
    0   ,    NULL
};

static ALLOC_SECTION_LDATA DEADKEY_LPWSTR aKeyNamesDead[] = {
    L"\x02db"	L"OGONEK",
    L"`"	L"GRAVE ACCENT",
    L"\x00b4"	L"ACUTE ACCENT",
    L"\x02dd"	L"DOUBLE ACUTE ACCENT",
    L"\x00a8"	L"DIAERESIS",
    L"\x02c7"	L"CARON",
    L"~"	L"TILDE",
    L"*"	L"SPECIAL",
    L"^"	L"CIRCUMFLEX ACCENT",
    L"\x02d8"	L"BREVE",
    L"\x02da"	L"RING ABOVE",
    L"\x00af"	L"MACRON",
    L"\x00b8"	L"CEDILLA",
    L"\x02d9"	L"DOT ABOVE",
    NULL
};

static ALLOC_SECTION_LDATA DEADKEY aDeadKey[] = {
    DEADTRANS( L'a'   , 0x02db , 0x0105 , 0x0000),
    DEADTRANS( L'e'   , 0x02db , 0x0119 , 0x0000),
    DEADTRANS( L'i'   , 0x02db , 0x012f , 0x0000),
    DEADTRANS( L'o'   , 0x02db , 0x01eb , 0x0000),
    DEADTRANS( L'u'   , 0x02db , 0x0173 , 0x0000),
    DEADTRANS( 0x014d , 0x02db , 0x01ed , 0x0000),
    DEADTRANS( L'A'   , 0x02db , 0x0104 , 0x0000),
    DEADTRANS( L'E'   , 0x02db , 0x0118 , 0x0000),
    DEADTRANS( L'I'   , 0x02db , 0x012e , 0x0000),
    DEADTRANS( L'O'   , 0x02db , 0x01ea , 0x0000),
    DEADTRANS( L'U'   , 0x02db , 0x0172 , 0x0000),
    DEADTRANS( 0x014c , 0x02db , 0x01ec , 0x0000),

    DEADTRANS( L'a'   , L'`'   , 0x00e0 , 0x0000),
    DEADTRANS( L'e'   , L'`'   , 0x00e8 , 0x0000),
    DEADTRANS( L'i'   , L'`'   , 0x00ec , 0x0000),
    DEADTRANS( L'n'   , L'`'   , 0x01f9 , 0x0000),
    DEADTRANS( L'o'   , L'`'   , 0x00f2 , 0x0000),
    DEADTRANS( L'u'   , L'`'   , 0x00f9 , 0x0000),
    DEADTRANS( L'w'   , L'`'   , 0x1e81 , 0x0000),
    DEADTRANS( L'y'   , L'`'   , 0x1ef3 , 0x0000),
    DEADTRANS( 0x0103 , L'`'   , 0x1eb1 , 0x0000),
    DEADTRANS( 0x00e2 , L'`'   , 0x1ea7 , 0x0000),
    DEADTRANS( 0x00ea , L'`'   , 0x1ec1 , 0x0000),
    DEADTRANS( 0x0113 , L'`'   , 0x1e15 , 0x0000),
    DEADTRANS( 0x00f4 , L'`'   , 0x1ed3 , 0x0000),
    DEADTRANS( 0x014d , L'`'   , 0x1e51 , 0x0000),
    DEADTRANS( 0x00fc , L'`'   , 0x01dc , 0x0000),
    DEADTRANS( L'A'   , L'`'   , 0x00c0 , 0x0000),
    DEADTRANS( L'E'   , L'`'   , 0x00c8 , 0x0000),
    DEADTRANS( L'I'   , L'`'   , 0x00cc , 0x0000),
    DEADTRANS( L'N'   , L'`'   , 0x01f8 , 0x0000),
    DEADTRANS( L'O'   , L'`'   , 0x00d2 , 0x0000),
    DEADTRANS( L'U'   , L'`'   , 0x00d9 , 0x0000),
    DEADTRANS( L'W'   , L'`'   , 0x1e80 , 0x0000),
    DEADTRANS( L'Y'   , L'`'   , 0x1ef2 , 0x0000),
    DEADTRANS( 0x0102 , L'`'   , 0x1eb0 , 0x0000),
    DEADTRANS( 0x00c2 , L'`'   , 0x1ea6 , 0x0000),
    DEADTRANS( 0x00ca , L'`'   , 0x1ec0 , 0x0000),
    DEADTRANS( 0x0112 , L'`'   , 0x1e14 , 0x0000),
    DEADTRANS( 0x00d4 , L'`'   , 0x1ed2 , 0x0000),
    DEADTRANS( 0x014c , L'`'   , 0x1e50 , 0x0000),
    DEADTRANS( 0x00dc , L'`'   , 0x01db , 0x0000),

    DEADTRANS( L'a'   , 0x00b4 , 0x00e1 , 0x0000),
    DEADTRANS( 0x00e6 , 0x00b4 , 0x01fd , 0x0000),
    DEADTRANS( L'c'   , 0x00b4 , 0x0107 , 0x0000),
    DEADTRANS( L'e'   , 0x00b4 , 0x00e9 , 0x0000),
    DEADTRANS( L'g'   , 0x00b4 , 0x01f5 , 0x0000),
    DEADTRANS( L'i'   , 0x00b4 , 0x00ed , 0x0000),
    DEADTRANS( L'k'   , 0x00b4 , 0x1e31 , 0x0000),
    DEADTRANS( L'l'   , 0x00b4 , 0x013a , 0x0000),
    DEADTRANS( L'm'   , 0x00b4 , 0x1e3f , 0x0000),
    DEADTRANS( L'n'   , 0x00b4 , 0x0144 , 0x0000),
    DEADTRANS( L'o'   , 0x00b4 , 0x00f3 , 0x0000),
    DEADTRANS( L'p'   , 0x00b4 , 0x1e55 , 0x0000),
    DEADTRANS( L'r'   , 0x00b4 , 0x0155 , 0x0000),
    DEADTRANS( L's'   , 0x00b4 , 0x015b , 0x0000),
    DEADTRANS( L'u'   , 0x00b4 , 0x00fa , 0x0000),
    DEADTRANS( L'w'   , 0x00b4 , 0x1e83 , 0x0000),
    DEADTRANS( L'y'   , 0x00b4 , 0x00fd , 0x0000),
    DEADTRANS( L'z'   , 0x00b4 , 0x017a , 0x0000),
    DEADTRANS( 0x0103 , 0x00b4 , 0x1eaf , 0x0000),
    DEADTRANS( 0x00e2 , 0x00b4 , 0x1ea5 , 0x0000),
    DEADTRANS( 0x00e5 , 0x00b4 , 0x01fb , 0x0000),
    DEADTRANS( 0x00e7 , 0x00b4 , 0x1e09 , 0x0000),
    DEADTRANS( 0x00ea , 0x00b4 , 0x1ebf , 0x0000),
    DEADTRANS( 0x0113 , 0x00b4 , 0x1e17 , 0x0000),
    DEADTRANS( 0x00ef , 0x00b4 , 0x1e2f , 0x0000),
    DEADTRANS( 0x00f4 , 0x00b4 , 0x1ed1 , 0x0000),
    DEADTRANS( 0x014d , 0x00b4 , 0x1e53 , 0x0000),
    DEADTRANS( 0x00f8 , 0x00b4 , 0x01ff , 0x0000),
    DEADTRANS( 0x00f5 , 0x00b4 , 0x1e4d , 0x0000),
    DEADTRANS( 0x00fc , 0x00b4 , 0x01d8 , 0x0000),
    DEADTRANS( 0x0169 , 0x00b4 , 0x1e79 , 0x0000),
    DEADTRANS( 0x1e61 , 0x00b4 , 0x1e65 , 0x0000),
    DEADTRANS( L'A'   , 0x00b4 , 0x00c1 , 0x0000),
    DEADTRANS( 0x00c6 , 0x00b4 , 0x01fc , 0x0000),
    DEADTRANS( L'C'   , 0x00b4 , 0x0106 , 0x0000),
    DEADTRANS( L'E'   , 0x00b4 , 0x00c9 , 0x0000),
    DEADTRANS( L'G'   , 0x00b4 , 0x01f4 , 0x0000),
    DEADTRANS( L'I'   , 0x00b4 , 0x00cd , 0x0000),
    DEADTRANS( L'K'   , 0x00b4 , 0x1e30 , 0x0000),
    DEADTRANS( L'L'   , 0x00b4 , 0x0139 , 0x0000),
    DEADTRANS( L'M'   , 0x00b4 , 0x1e3e , 0x0000),
    DEADTRANS( L'N'   , 0x00b4 , 0x0143 , 0x0000),
    DEADTRANS( L'O'   , 0x00b4 , 0x00d3 , 0x0000),
    DEADTRANS( L'P'   , 0x00b4 , 0x1e54 , 0x0000),
    DEADTRANS( L'R'   , 0x00b4 , 0x0154 , 0x0000),
    DEADTRANS( L'S'   , 0x00b4 , 0x015a , 0x0000),
    DEADTRANS( L'U'   , 0x00b4 , 0x00da , 0x0000),
    DEADTRANS( L'W'   , 0x00b4 , 0x1e82 , 0x0000),
    DEADTRANS( L'Y'   , 0x00b4 , 0x00dd , 0x0000),
    DEADTRANS( L'Z'   , 0x00b4 , 0x0179 , 0x0000),
    DEADTRANS( 0x0102 , 0x00b4 , 0x1eae , 0x0000),
    DEADTRANS( 0x00c2 , 0x00b4 , 0x1ea4 , 0x0000),
    DEADTRANS( 0x00c5 , 0x00b4 , 0x01fa , 0x0000),
    DEADTRANS( 0x00c7 , 0x00b4 , 0x1e08 , 0x0000),
    DEADTRANS( 0x00ca , 0x00b4 , 0x1ebe , 0x0000),
    DEADTRANS( 0x0112 , 0x00b4 , 0x1e16 , 0x0000),
    DEADTRANS( 0x00cf , 0x00b4 , 0x1e2e , 0x0000),
    DEADTRANS( 0x00d4 , 0x00b4 , 0x1ed0 , 0x0000),
    DEADTRANS( 0x014c , 0x00b4 , 0x1e52 , 0x0000),
    DEADTRANS( 0x00d8 , 0x00b4 , 0x01fe , 0x0000),
    DEADTRANS( 0x00d5 , 0x00b4 , 0x1e4c , 0x0000),
    DEADTRANS( 0x00dc , 0x00b4 , 0x01d7 , 0x0000),
    DEADTRANS( 0x0168 , 0x00b4 , 0x1e78 , 0x0000),
    DEADTRANS( 0x1e60 , 0x00b4 , 0x1e64 , 0x0000),

    DEADTRANS( L'o'   , 0x02dd , 0x0151 , 0x0000),
    DEADTRANS( L'u'   , 0x02dd , 0x0171 , 0x0000),
    DEADTRANS( L'O'   , 0x02dd , 0x0150 , 0x0000),
    DEADTRANS( L'U'   , 0x02dd , 0x0170 , 0x0000),

    DEADTRANS( L'a'   , 0x00a8 , 0x00e4 , 0x0000),
    DEADTRANS( L'e'   , 0x00a8 , 0x00eb , 0x0000),
    DEADTRANS( L'h'   , 0x00a8 , 0x1e27 , 0x0000),
    DEADTRANS( L'i'   , 0x00a8 , 0x00ef , 0x0000),
    DEADTRANS( L'o'   , 0x00a8 , 0x00f6 , 0x0000),
    DEADTRANS( L't'   , 0x00a8 , 0x1e97 , 0x0000),
    DEADTRANS( L'u'   , 0x00a8 , 0x00fc , 0x0000),
    DEADTRANS( L'w'   , 0x00a8 , 0x1e85 , 0x0000),
    DEADTRANS( L'x'   , 0x00a8 , 0x1e8d , 0x0000),
    DEADTRANS( L'y'   , 0x00a8 , 0x00ff , 0x0000),
    DEADTRANS( 0x00ed , 0x00a8 , 0x1e2f , 0x0000),
    DEADTRANS( 0x00fa , 0x00a8 , 0x01d8 , 0x0000),
    DEADTRANS( 0x01d4 , 0x00a8 , 0x01da , 0x0000),
    DEADTRANS( 0x00f5 , 0x00a8 , 0x1e4f , 0x0000),
    DEADTRANS( 0x00f9 , 0x00a8 , 0x01dc , 0x0000),
    DEADTRANS( 0x0101 , 0x00a8 , 0x01df , 0x0000),
    DEADTRANS( 0x014d , 0x00a8 , 0x022b , 0x0000),
    DEADTRANS( 0x016b , 0x00a8 , 0x01d6 , 0x0000),
    DEADTRANS( L'A'   , 0x00a8 , 0x00c4 , 0x0000),
    DEADTRANS( L'E'   , 0x00a8 , 0x00cb , 0x0000),
    DEADTRANS( L'H'   , 0x00a8 , 0x1e26 , 0x0000),
    DEADTRANS( L'I'   , 0x00a8 , 0x00cf , 0x0000),
    DEADTRANS( L'O'   , 0x00a8 , 0x00d6 , 0x0000),
    DEADTRANS( L'U'   , 0x00a8 , 0x00dc , 0x0000),
    DEADTRANS( L'W'   , 0x00a8 , 0x1e84 , 0x0000),
    DEADTRANS( L'X'   , 0x00a8 , 0x1e8c , 0x0000),
    DEADTRANS( L'Y'   , 0x00a8 , 0x0178 , 0x0000),
    DEADTRANS( 0x00cd , 0x00a8 , 0x1e2e , 0x0000),
    DEADTRANS( 0x00da , 0x00a8 , 0x01d7 , 0x0000),
    DEADTRANS( 0x01d3 , 0x00a8 , 0x01d9 , 0x0000),
    DEADTRANS( 0x00d5 , 0x00a8 , 0x1e4e , 0x0000),
    DEADTRANS( 0x00d9 , 0x00a8 , 0x01db , 0x0000),
    DEADTRANS( 0x0100 , 0x00a8 , 0x01de , 0x0000),
    DEADTRANS( 0x014c , 0x00a8 , 0x022a , 0x0000),
    DEADTRANS( 0x016a , 0x00a8 , 0x01d5 , 0x0000),

    DEADTRANS( L'a'   , 0x02c7 , 0x01ce , 0x0000),
    DEADTRANS( L'c'   , 0x02c7 , 0x010d , 0x0000),
    DEADTRANS( L'd'   , 0x02c7 , 0x010f , 0x0000),
    DEADTRANS( 0x01f3 , 0x02c7 , 0x01c6 , 0x0000),
    DEADTRANS( L'e'   , 0x02c7 , 0x011b , 0x0000),
    DEADTRANS( 0x0292 , 0x02c7 , 0x01ef , 0x0000),
    DEADTRANS( L'g'   , 0x02c7 , 0x01e7 , 0x0000),
    DEADTRANS( L'h'   , 0x02c7 , 0x021f , 0x0000),
    DEADTRANS( L'i'   , 0x02c7 , 0x01d0 , 0x0000),
    DEADTRANS( L'j'   , 0x02c7 , 0x01f0 , 0x0000),
    DEADTRANS( L'k'   , 0x02c7 , 0x01e9 , 0x0000),
    DEADTRANS( L'l'   , 0x02c7 , 0x013e , 0x0000),
    DEADTRANS( L'n'   , 0x02c7 , 0x0148 , 0x0000),
    DEADTRANS( L'o'   , 0x02c7 , 0x01d2 , 0x0000),
    DEADTRANS( L'r'   , 0x02c7 , 0x0159 , 0x0000),
    DEADTRANS( L's'   , 0x02c7 , 0x0161 , 0x0000),
    DEADTRANS( L't'   , 0x02c7 , 0x0165 , 0x0000),
    DEADTRANS( L'u'   , 0x02c7 , 0x01d4 , 0x0000),
    DEADTRANS( L'z'   , 0x02c7 , 0x017e , 0x0000),
    DEADTRANS( 0x00fc , 0x02c7 , 0x01da , 0x0000),
    DEADTRANS( 0x1e61 , 0x02c7 , 0x1e67 , 0x0000),
    DEADTRANS( L'A'   , 0x02c7 , 0x01cd , 0x0000),
    DEADTRANS( L'C'   , 0x02c7 , 0x010c , 0x0000),
    DEADTRANS( L'D'   , 0x02c7 , 0x010e , 0x0000),
    DEADTRANS( 0x01f2 , 0x02c7 , 0x01c5 , 0x0000),
    DEADTRANS( 0x01f1 , 0x02c7 , 0x01c4 , 0x0000),
    DEADTRANS( L'E'   , 0x02c7 , 0x011a , 0x0000),
    DEADTRANS( 0x01b7 , 0x02c7 , 0x01ee , 0x0000),
    DEADTRANS( L'G'   , 0x02c7 , 0x01e6 , 0x0000),
    DEADTRANS( L'H'   , 0x02c7 , 0x021e , 0x0000),
    DEADTRANS( L'I'   , 0x02c7 , 0x01cf , 0x0000),
    DEADTRANS( L'K'   , 0x02c7 , 0x01e8 , 0x0000),
    DEADTRANS( L'L'   , 0x02c7 , 0x013d , 0x0000),
    DEADTRANS( L'N'   , 0x02c7 , 0x0147 , 0x0000),
    DEADTRANS( L'O'   , 0x02c7 , 0x01d1 , 0x0000),
    DEADTRANS( L'R'   , 0x02c7 , 0x0158 , 0x0000),
    DEADTRANS( L'S'   , 0x02c7 , 0x0160 , 0x0000),
    DEADTRANS( L'T'   , 0x02c7 , 0x0164 , 0x0000),
    DEADTRANS( L'U'   , 0x02c7 , 0x01d3 , 0x0000),
    DEADTRANS( L'Z'   , 0x02c7 , 0x017d , 0x0000),
    DEADTRANS( 0x00dc , 0x02c7 , 0x01d9 , 0x0000),
    DEADTRANS( 0x1e60 , 0x02c7 , 0x1e66 , 0x0000),

    DEADTRANS( L'a'   , L'~'   , 0x00e3 , 0x0000),
    DEADTRANS( L'e'   , L'~'   , 0x1ebd , 0x0000),
    DEADTRANS( L'i'   , L'~'   , 0x0129 , 0x0000),
    DEADTRANS( L'n'   , L'~'   , 0x00f1 , 0x0000),
    DEADTRANS( L'o'   , L'~'   , 0x00f5 , 0x0000),
    DEADTRANS( L'u'   , L'~'   , 0x0169 , 0x0000),
    DEADTRANS( L'v'   , L'~'   , 0x1e7d , 0x0000),
    DEADTRANS( L'y'   , L'~'   , 0x1ef9 , 0x0000),
    DEADTRANS( 0x00f3 , L'~'   , 0x1e4d , 0x0000),
    DEADTRANS( 0x00fa , L'~'   , 0x1e79 , 0x0000),
    DEADTRANS( 0x00f6 , L'~'   , 0x1e4f , 0x0000),
    DEADTRANS( 0x014d , L'~'   , 0x022d , 0x0000),
    DEADTRANS( 0x0103 , L'~'   , 0x1eb5 , 0x0000),
    DEADTRANS( 0x00e2 , L'~'   , 0x1eab , 0x0000),
    DEADTRANS( 0x00ea , L'~'   , 0x1ec5 , 0x0000),
    DEADTRANS( 0x00f4 , L'~'   , 0x1ed7 , 0x0000),
    DEADTRANS( L'A'   , L'~'   , 0x00c3 , 0x0000),
    DEADTRANS( L'E'   , L'~'   , 0x1ebc , 0x0000),
    DEADTRANS( L'I'   , L'~'   , 0x0128 , 0x0000),
    DEADTRANS( L'N'   , L'~'   , 0x00d1 , 0x0000),
    DEADTRANS( L'O'   , L'~'   , 0x00d5 , 0x0000),
    DEADTRANS( L'U'   , L'~'   , 0x0168 , 0x0000),
    DEADTRANS( L'V'   , L'~'   , 0x1e7c , 0x0000),
    DEADTRANS( L'Y'   , L'~'   , 0x1ef8 , 0x0000),
    DEADTRANS( 0x00d3 , L'~'   , 0x1e4c , 0x0000),
    DEADTRANS( 0x00da , L'~'   , 0x1e78 , 0x0000),
    DEADTRANS( 0x00d6 , L'~'   , 0x1e4e , 0x0000),
    DEADTRANS( 0x014c , L'~'   , 0x022c , 0x0000),
    DEADTRANS( 0x0102 , L'~'   , 0x1eb4 , 0x0000),
    DEADTRANS( 0x00c2 , L'~'   , 0x1eaa , 0x0000),
    DEADTRANS( 0x00ca , L'~'   , 0x1ec4 , 0x0000),
    DEADTRANS( 0x00d4 , L'~'   , 0x1ed6 , 0x0000),

    DEADTRANS( L'~'   , L'*'   , 0x2248 , 0x0000),
    DEADTRANS( L'c'   , L'*'   , 0x00a9 , 0x0000),
    DEADTRANS( L'r'   , L'*'   , 0x00ae , 0x0000),
    DEADTRANS( L'n'   , L'*'   , 0x2116 , 0x0000),
    DEADTRANS( L'd'   , L'*'   , 0x00b0 , 0x0000),
    DEADTRANS( L't'   , L'*'   , 0x2122 , 0x0000),
    DEADTRANS( L'm'   , L'*'   , 0x00b5 , 0x0000),
    DEADTRANS( L'p'   , L'*'   , 0x00b6 , 0x0000),
    DEADTRANS( L'.'   , L'*'   , 0x2026 , 0x0000),
    DEADTRANS( L'%'   , L'*'   , 0x2030 , 0x0000),
    DEADTRANS( L'+'   , L'*'   , 0x00b1 , 0x0000),
    DEADTRANS( L'<'   , L'*'   , 0x2264 , 0x0000),
    DEADTRANS( L'>'   , L'*'   , 0x2265 , 0x0000),
    DEADTRANS( L'='   , L'*'   , 0x2260 , 0x0000),
    DEADTRANS( L'|'   , L'*'   , 0x00a6 , 0x0000),
    DEADTRANS( L's'   , L'*'   , 0x00a7 , 0x0000),

    DEADTRANS( L'a'   , L'^'   , 0x00e2 , 0x0000),
    DEADTRANS( L'c'   , L'^'   , 0x0109 , 0x0000),
    DEADTRANS( L'e'   , L'^'   , 0x00ea , 0x0000),
    DEADTRANS( L'g'   , L'^'   , 0x011d , 0x0000),
    DEADTRANS( L'h'   , L'^'   , 0x0125 , 0x0000),
    DEADTRANS( L'i'   , L'^'   , 0x00ee , 0x0000),
    DEADTRANS( L'j'   , L'^'   , 0x0135 , 0x0000),
    DEADTRANS( L'o'   , L'^'   , 0x00f4 , 0x0000),
    DEADTRANS( L's'   , L'^'   , 0x015d , 0x0000),
    DEADTRANS( L'u'   , L'^'   , 0x00fb , 0x0000),
    DEADTRANS( L'w'   , L'^'   , 0x0175 , 0x0000),
    DEADTRANS( L'y'   , L'^'   , 0x0177 , 0x0000),
    DEADTRANS( L'z'   , L'^'   , 0x1e91 , 0x0000),
    DEADTRANS( 0x00e1 , L'^'   , 0x1ea5 , 0x0000),
    DEADTRANS( 0x00e9 , L'^'   , 0x1ebf , 0x0000),
    DEADTRANS( 0x00f3 , L'^'   , 0x1ed1 , 0x0000),
    DEADTRANS( 0x00e0 , L'^'   , 0x1ea7 , 0x0000),
    DEADTRANS( 0x00e8 , L'^'   , 0x1ec1 , 0x0000),
    DEADTRANS( 0x00f2 , L'^'   , 0x1ed3 , 0x0000),
    DEADTRANS( 0x00e3 , L'^'   , 0x1eab , 0x0000),
    DEADTRANS( 0x1ebd , L'^'   , 0x1ec5 , 0x0000),
    DEADTRANS( 0x00f5 , L'^'   , 0x1ed7 , 0x0000),
    DEADTRANS( L'A'   , L'^'   , 0x00c2 , 0x0000),
    DEADTRANS( L'C'   , L'^'   , 0x0108 , 0x0000),
    DEADTRANS( L'E'   , L'^'   , 0x00ca , 0x0000),
    DEADTRANS( L'G'   , L'^'   , 0x011c , 0x0000),
    DEADTRANS( L'H'   , L'^'   , 0x0124 , 0x0000),
    DEADTRANS( L'I'   , L'^'   , 0x00ce , 0x0000),
    DEADTRANS( L'J'   , L'^'   , 0x0134 , 0x0000),
    DEADTRANS( L'O'   , L'^'   , 0x00d4 , 0x0000),
    DEADTRANS( L'S'   , L'^'   , 0x015c , 0x0000),
    DEADTRANS( L'U'   , L'^'   , 0x00db , 0x0000),
    DEADTRANS( L'W'   , L'^'   , 0x0174 , 0x0000),
    DEADTRANS( L'Y'   , L'^'   , 0x0176 , 0x0000),
    DEADTRANS( L'Z'   , L'^'   , 0x1e90 , 0x0000),
    DEADTRANS( 0x00c1 , L'^'   , 0x1ea4 , 0x0000),
    DEADTRANS( 0x00c9 , L'^'   , 0x1ebe , 0x0000),
    DEADTRANS( 0x00d3 , L'^'   , 0x1ed0 , 0x0000),
    DEADTRANS( 0x00c0 , L'^'   , 0x1ea6 , 0x0000),
    DEADTRANS( 0x00c8 , L'^'   , 0x1ec0 , 0x0000),
    DEADTRANS( 0x00d2 , L'^'   , 0x1ed2 , 0x0000),
    DEADTRANS( 0x00c3 , L'^'   , 0x1eaa , 0x0000),
    DEADTRANS( 0x1ebc , L'^'   , 0x1ec4 , 0x0000),
    DEADTRANS( 0x00d5 , L'^'   , 0x1ed6 , 0x0000),

    DEADTRANS( L'a'   , 0x02d8 , 0x0103 , 0x0000),
    DEADTRANS( L'e'   , 0x02d8 , 0x0115 , 0x0000),
    DEADTRANS( L'g'   , 0x02d8 , 0x011f , 0x0000),
    DEADTRANS( L'i'   , 0x02d8 , 0x012d , 0x0000),
    DEADTRANS( L'o'   , 0x02d8 , 0x014f , 0x0000),
    DEADTRANS( L'u'   , 0x02d8 , 0x016d , 0x0000),
    DEADTRANS( 0x00e1 , 0x02d8 , 0x1eaf , 0x0000),
    DEADTRANS( 0x0229 , 0x02d8 , 0x1e1d , 0x0000),
    DEADTRANS( 0x00e0 , 0x02d8 , 0x1eb1 , 0x0000),
    DEADTRANS( 0x00e3 , 0x02d8 , 0x1eb5 , 0x0000),
    DEADTRANS( L'A'   , 0x02d8 , 0x0102 , 0x0000),
    DEADTRANS( L'E'   , 0x02d8 , 0x0114 , 0x0000),
    DEADTRANS( L'G'   , 0x02d8 , 0x011e , 0x0000),
    DEADTRANS( L'I'   , 0x02d8 , 0x012c , 0x0000),
    DEADTRANS( L'O'   , 0x02d8 , 0x014e , 0x0000),
    DEADTRANS( L'U'   , 0x02d8 , 0x016c , 0x0000),
    DEADTRANS( 0x00c1 , 0x02d8 , 0x1eae , 0x0000),
    DEADTRANS( 0x0228 , 0x02d8 , 0x1e1c , 0x0000),
    DEADTRANS( 0x00c0 , 0x02d8 , 0x1eb0 , 0x0000),
    DEADTRANS( 0x00c3 , 0x02d8 , 0x1eb4 , 0x0000),

    DEADTRANS( L'a'   , 0x02da , 0x00e5 , 0x0000),
    DEADTRANS( L'u'   , 0x02da , 0x016f , 0x0000),
    DEADTRANS( L'w'   , 0x02da , 0x1e98 , 0x0000),
    DEADTRANS( L'y'   , 0x02da , 0x1e99 , 0x0000),
    DEADTRANS( 0x00e1 , 0x02da , 0x01fb , 0x0000),
    DEADTRANS( L'A'   , 0x02da , 0x00c5 , 0x0000),
    DEADTRANS( L'U'   , 0x02da , 0x016e , 0x0000),
    DEADTRANS( 0x00c1 , 0x02da , 0x01fa , 0x0000),

    DEADTRANS( L'a'   , 0x00af , 0x0101 , 0x0000),
    DEADTRANS( 0x00e6 , 0x00af , 0x01e3 , 0x0000),
    DEADTRANS( L'e'   , 0x00af , 0x0113 , 0x0000),
    DEADTRANS( L'g'   , 0x00af , 0x1e21 , 0x0000),
    DEADTRANS( L'i'   , 0x00af , 0x012b , 0x0000),
    DEADTRANS( L'o'   , 0x00af , 0x014d , 0x0000),
    DEADTRANS( L'u'   , 0x00af , 0x016b , 0x0000),
    DEADTRANS( L'y'   , 0x00af , 0x0233 , 0x0000),
    DEADTRANS( 0x00e9 , 0x00af , 0x1e17 , 0x0000),
    DEADTRANS( 0x00f3 , 0x00af , 0x1e53 , 0x0000),
    DEADTRANS( 0x00e8 , 0x00af , 0x1e15 , 0x0000),
    DEADTRANS( 0x00f2 , 0x00af , 0x1e51 , 0x0000),
    DEADTRANS( 0x00e4 , 0x00af , 0x01df , 0x0000),
    DEADTRANS( 0x0227 , 0x00af , 0x01e1 , 0x0000),
    DEADTRANS( 0x00f6 , 0x00af , 0x022b , 0x0000),
    DEADTRANS( 0x022f , 0x00af , 0x0231 , 0x0000),
    DEADTRANS( 0x01eb , 0x00af , 0x01ed , 0x0000),
    DEADTRANS( 0x00f5 , 0x00af , 0x022d , 0x0000),
    DEADTRANS( 0x00fc , 0x00af , 0x01d6 , 0x0000),
    DEADTRANS( L'A'   , 0x00af , 0x0100 , 0x0000),
    DEADTRANS( 0x00c6 , 0x00af , 0x01e2 , 0x0000),
    DEADTRANS( L'E'   , 0x00af , 0x0112 , 0x0000),
    DEADTRANS( L'G'   , 0x00af , 0x1e20 , 0x0000),
    DEADTRANS( L'I'   , 0x00af , 0x012a , 0x0000),
    DEADTRANS( L'O'   , 0x00af , 0x014c , 0x0000),
    DEADTRANS( L'U'   , 0x00af , 0x016a , 0x0000),
    DEADTRANS( L'Y'   , 0x00af , 0x0232 , 0x0000),
    DEADTRANS( 0x00c9 , 0x00af , 0x1e16 , 0x0000),
    DEADTRANS( 0x00d3 , 0x00af , 0x1e52 , 0x0000),
    DEADTRANS( 0x00c8 , 0x00af , 0x1e14 , 0x0000),
    DEADTRANS( 0x00d2 , 0x00af , 0x1e50 , 0x0000),
    DEADTRANS( 0x00c4 , 0x00af , 0x01de , 0x0000),
    DEADTRANS( 0x0226 , 0x00af , 0x01e0 , 0x0000),
    DEADTRANS( 0x00d6 , 0x00af , 0x022a , 0x0000),
    DEADTRANS( 0x022e , 0x00af , 0x0230 , 0x0000),
    DEADTRANS( 0x01ea , 0x00af , 0x01ec , 0x0000),
    DEADTRANS( 0x00d5 , 0x00af , 0x022c , 0x0000),
    DEADTRANS( 0x00dc , 0x00af , 0x01d5 , 0x0000),

    DEADTRANS( L'c'   , 0x00b8 , 0x00e7 , 0x0000),
    DEADTRANS( L'd'   , 0x00b8 , 0x1e11 , 0x0000),
    DEADTRANS( L'e'   , 0x00b8 , 0x0229 , 0x0000),
    DEADTRANS( L'g'   , 0x00b8 , 0x0123 , 0x0000),
    DEADTRANS( L'h'   , 0x00b8 , 0x1e29 , 0x0000),
    DEADTRANS( L'k'   , 0x00b8 , 0x0137 , 0x0000),
    DEADTRANS( L'l'   , 0x00b8 , 0x013c , 0x0000),
    DEADTRANS( L'n'   , 0x00b8 , 0x0146 , 0x0000),
    DEADTRANS( L'r'   , 0x00b8 , 0x0157 , 0x0000),
    DEADTRANS( L's'   , 0x00b8 , 0x015f , 0x0000),
    DEADTRANS( L't'   , 0x00b8 , 0x0163 , 0x0000),
    DEADTRANS( 0x0107 , 0x00b8 , 0x1e09 , 0x0000),
    DEADTRANS( 0x0115 , 0x00b8 , 0x1e1d , 0x0000),
    DEADTRANS( L'C'   , 0x00b8 , 0x00c7 , 0x0000),
    DEADTRANS( L'D'   , 0x00b8 , 0x1e10 , 0x0000),
    DEADTRANS( L'E'   , 0x00b8 , 0x0228 , 0x0000),
    DEADTRANS( L'G'   , 0x00b8 , 0x0122 , 0x0000),
    DEADTRANS( L'H'   , 0x00b8 , 0x1e28 , 0x0000),
    DEADTRANS( L'K'   , 0x00b8 , 0x0136 , 0x0000),
    DEADTRANS( L'L'   , 0x00b8 , 0x013b , 0x0000),
    DEADTRANS( L'N'   , 0x00b8 , 0x0145 , 0x0000),
    DEADTRANS( L'R'   , 0x00b8 , 0x0156 , 0x0000),
    DEADTRANS( L'S'   , 0x00b8 , 0x015e , 0x0000),
    DEADTRANS( L'T'   , 0x00b8 , 0x0162 , 0x0000),
    DEADTRANS( 0x0106 , 0x00b8 , 0x1e08 , 0x0000),
    DEADTRANS( 0x0114 , 0x00b8 , 0x1e1c , 0x0000),

    DEADTRANS( L'a'   , 0x02d9 , 0x0227 , 0x0000),
    DEADTRANS( L'b'   , 0x02d9 , 0x1e03 , 0x0000),
    DEADTRANS( L'c'   , 0x02d9 , 0x010b , 0x0000),
    DEADTRANS( L'd'   , 0x02d9 , 0x1e0b , 0x0000),
    DEADTRANS( L'e'   , 0x02d9 , 0x0117 , 0x0000),
    DEADTRANS( L'f'   , 0x02d9 , 0x1e1f , 0x0000),
    DEADTRANS( L'g'   , 0x02d9 , 0x0121 , 0x0000),
    DEADTRANS( L'h'   , 0x02d9 , 0x1e23 , 0x0000),
    DEADTRANS( 0x017f , 0x02d9 , 0x1e9b , 0x0000),
    DEADTRANS( L'm'   , 0x02d9 , 0x1e41 , 0x0000),
    DEADTRANS( L'n'   , 0x02d9 , 0x1e45 , 0x0000),
    DEADTRANS( L'o'   , 0x02d9 , 0x022f , 0x0000),
    DEADTRANS( L'p'   , 0x02d9 , 0x1e57 , 0x0000),
    DEADTRANS( L'r'   , 0x02d9 , 0x1e59 , 0x0000),
    DEADTRANS( L's'   , 0x02d9 , 0x1e61 , 0x0000),
    DEADTRANS( L't'   , 0x02d9 , 0x1e6b , 0x0000),
    DEADTRANS( L'w'   , 0x02d9 , 0x1e87 , 0x0000),
    DEADTRANS( L'x'   , 0x02d9 , 0x1e8b , 0x0000),
    DEADTRANS( L'y'   , 0x02d9 , 0x1e8f , 0x0000),
    DEADTRANS( L'z'   , 0x02d9 , 0x017c , 0x0000),
    DEADTRANS( 0x015b , 0x02d9 , 0x1e65 , 0x0000),
    DEADTRANS( 0x0161 , 0x02d9 , 0x1e67 , 0x0000),
    DEADTRANS( 0x0101 , 0x02d9 , 0x01e1 , 0x0000),
    DEADTRANS( 0x014d , 0x02d9 , 0x0231 , 0x0000),
    DEADTRANS( L'A'   , 0x02d9 , 0x0226 , 0x0000),
    DEADTRANS( L'B'   , 0x02d9 , 0x1e02 , 0x0000),
    DEADTRANS( L'C'   , 0x02d9 , 0x010a , 0x0000),
    DEADTRANS( L'D'   , 0x02d9 , 0x1e0a , 0x0000),
    DEADTRANS( L'E'   , 0x02d9 , 0x0116 , 0x0000),
    DEADTRANS( L'F'   , 0x02d9 , 0x1e1e , 0x0000),
    DEADTRANS( L'G'   , 0x02d9 , 0x0120 , 0x0000),
    DEADTRANS( L'H'   , 0x02d9 , 0x1e22 , 0x0000),
    DEADTRANS( L'I'   , 0x02d9 , 0x0130 , 0x0000),
    DEADTRANS( L'M'   , 0x02d9 , 0x1e40 , 0x0000),
    DEADTRANS( L'N'   , 0x02d9 , 0x1e44 , 0x0000),
    DEADTRANS( L'O'   , 0x02d9 , 0x022e , 0x0000),
    DEADTRANS( L'P'   , 0x02d9 , 0x1e56 , 0x0000),
    DEADTRANS( L'R'   , 0x02d9 , 0x1e58 , 0x0000),
    DEADTRANS( L'S'   , 0x02d9 , 0x1e60 , 0x0000),
    DEADTRANS( L'T'   , 0x02d9 , 0x1e6a , 0x0000),
    DEADTRANS( L'W'   , 0x02d9 , 0x1e86 , 0x0000),
    DEADTRANS( L'X'   , 0x02d9 , 0x1e8a , 0x0000),
    DEADTRANS( L'Y'   , 0x02d9 , 0x1e8e , 0x0000),
    DEADTRANS( L'Z'   , 0x02d9 , 0x017b , 0x0000),
    DEADTRANS( 0x015a , 0x02d9 , 0x1e64 , 0x0000),
    DEADTRANS( 0x0160 , 0x02d9 , 0x1e66 , 0x0000),
    DEADTRANS( 0x0100 , 0x02d9 , 0x01e0 , 0x0000),
    DEADTRANS( 0x014c , 0x02d9 , 0x0230 , 0x0000),
    DEADTRANS( L'i'   , 0x02d9 , 0x0131 , 0x0000),
    DEADTRANS( L'l'   , 0x02d9 , 0x0140 , 0x0000),
    DEADTRANS( L'L'   , 0x02d9 , 0x013f , 0x0000),

    0, 0
};

static ALLOC_SECTION_LDATA KBDTABLES KbdTables = {
    /*
     * Modifier keys
     */
    &CharModifiers,

    /*
     * Characters tables
     */
    aVkToWcharTable,

    /*
     * Diacritics
     */
    aDeadKey,

    /*
     * Names of Keys
     */
    aKeyNames,
    aKeyNamesExt,
    aKeyNamesDead,

    /*
     * Scan codes to Virtual Keys
     */
    ausVK,
    sizeof(ausVK) / sizeof(ausVK[0]),
    aE0VscToVk,
    aE1VscToVk,

    /*
     * Locale-specific special processing
     */
    MAKELONG(KLLF_ALTGR, KBD_VERSION),

    /*
     * Ligatures
     */
    0,
    0,
    NULL
};

PKBDTABLES KbdLayerDescriptor(VOID)
{
    return &KbdTables;
}
