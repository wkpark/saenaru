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
 * $Id$
 */

#include "indicml.h"
/**********************************************************************/
/*                                                                    */
/*      DebugOptions                                                  */
/*                                                                    */
/**********************************************************************/
#define DEBF_THREADID       0x00000001
#define DEBF_GUIDELINE      0x00000002

#define LOGF_ENTRY          0x00000001
#define LOGF_API            0x00000002
#define LOGF_APIOUT         0x00000004
#define LOGF_KEY            0x00000008

/**********************************************************************/
/*                                                                    */
/*      Define                                                        */
/*                                                                    */
/**********************************************************************/

#define SAENARU_MUTEX_NAME  TEXT("SaeNaru_Mutex")

#define MASK_LSHIFT         0x00010000
#define MASK_LCTRL          0x00020000
#define MASK_LALT           0x00040000
#define MASK_RSHIFT         0x00100000
#define MASK_RCTRL          0x00200000
#define MASK_RALT           0x00400000
#define MASK_SHIFT          0x00110000
#define MASK_CTRL           0x00220000
#define MASK_ALT            0x00440000

#include "unicode.h"

#define MyFileName        MYTEXT("saenaru.ime")

/* for limit of SAENARU */
#define MAXCOMPWND              10
#define MAXCOMPSIZE             128
#define MAXCLAUSESIZE           16
#define MAXCANDPAGESIZE         9
#define MAXCANDSTRSIZE          16
#define MAXGLCHAR               32
//#define MAXCANDSTRNUM           32
#define MAXCANDSTRNUM           300


/* for GlobalAlloc */
#define GHIME (GHND | GMEM_SHARE)

/* special messages */
#define WM_UI_UPDATE      (WM_USER+500)
#define WM_UI_HIDE        (WM_USER+501)

#define WM_UI_STATEMOVE   (WM_USER+601)
#define WM_UI_DEFCOMPMOVE (WM_USER+602)
#define WM_UI_CANDMOVE    (WM_USER+603)
#define WM_UI_GUIDEMOVE   (WM_USER+604)
#define WM_UI_STATEHIDE   (WM_USER+605)
#define WM_UI_CARET       (WM_USER+606)

/* Escape Functions */
#define IME_ESC_PRI_GETDWORDTEST (IME_ESC_PRIVATE_FIRST + 0)

/* special style */
#define WS_COMPDEFAULT (WS_DISABLED | WS_POPUP)
#define WS_COMPNODEFAULT (WS_DISABLED | WS_POPUP)

/* macro for me! */
#define IsCTLPushed(x)          ((x)[VK_CONTROL] & 0x80)
#define IsSHFTPushed(x)         ((x)[VK_SHIFT] & 0x80)
#define IsALTPushed(x)          ((x)[VK_ALT] & 0x80)

/* update context */
#define UPDATE_MODE             0x01
#define UPDATE_SYSTEM           0x02
#define UPDATE_TEXT             0x04
#define UPDATE_FORCE            0x08
#define UPDATE_ALL              ( UPDATE_MODE | UPDATE_SYSTEM | UPDATE_TEXT )

/* advise context */
#define ADVISE_MODE             0x0001  /* advise about Mode requested   */
#define ADVISE_ISOPEN           0x0002  /* advise about IsOpen requested */

/* key state context */
#define KS_SHIFT                0x01
#define KS_CONTROL              0x02



/* ID of guideline table */
#define MYGL_NODICTIONARY     0
#define MYGL_TYPINGERROR      1
#define MYGL_TESTGUIDELINE    2

/* Change Mode index */
#define TO_CMODE_ALPHANUMERIC  0x0001
#define TO_CMODE_HANGUL        0x0002
#define TO_CMODE_HIRAGANA      0x0003
#define TO_CMODE_FULLSHAPE     0x0008
#define TO_CMODE_ROMAN         0x0010
#define TO_CMODE_CHARCODE      0x0020
#define TO_CMODE_TOOLBAR       0x0100

/* WndExtra of child UI windows */
#define FIGWL_MOUSE         0
#define FIGWL_SVRWND       (FIGWL_MOUSE+sizeof(LONG)) 
#define FIGWL_FONT         (FIGWL_SVRWND+sizeof(LONG_PTR))
#define FIGWL_COMPSTARTSTR (FIGWL_FONT+sizeof(LONG_PTR))
#define FIGWL_COMPSTARTNUM (FIGWL_COMPSTARTSTR+sizeof(LONG))
#define FIGWL_STATUSBMP    (FIGWL_COMPSTARTNUM+sizeof(LONG))
#define FIGWL_CLOSEBMP     (FIGWL_STATUSBMP+sizeof(LONG_PTR))
#define FIGWL_HANJA_MODEBMP  (FIGWL_CLOSEBMP+sizeof(LONG_PTR))
#define FIGWL_HANJA_CHECKBMP (FIGWL_HANJA_MODEBMP+sizeof(LONG_PTR))
#define FIGWL_PUSHSTATUS     (FIGWL_HANJA_CHECKBMP+sizeof(LONG_PTR))
#define FIGWL_CHILDWND     (FIGWL_PUSHSTATUS+sizeof(LONG))

//#define FIGWL_PREVCURSOR   (FIGWL_COMPSTARTNUM+sizeof(LONG))


#define UIEXTRASIZE        (FIGWL_CHILDWND+sizeof(LONG_PTR))

/* The flags of FIGWL_MOUSE */
#define FIM_CAPUTURED 0x01
#define FIM_MOVED     0x02

/* The flags of the button of Status Window */
#define PUSHED_STATUS_HDR   0x01
#define PUSHED_STATUS_MODE  0x02
#define PUSHED_STATUS_ROMAN 0x04
#define PUSHED_STATUS_CLOSE 0x08

/* Status Button Pos */
#define BTX       20
#define BTY       20
#define BTFHIRA   20
#define BTFKATA   40
#define BTFALPH   60
#define BTHKATA   80
#define BTHALPH  100
#define BTROMA   120
#define BTEMPT   140

/* Statue Close Button */
#define STCLBT_X   (BTX*2+3)
#define STCLBT_Y   1
#define STCLBT_DX  12
#define STCLBT_DY  12

/* define Shift Arrow right-left */
#define ARR_RIGHT 1
#define ARR_LEFT  2

/* Init or Clear Structure Flag */
#define CLR_RESULT 1
#define CLR_UNDET  2
#define CLR_RESULT_AND_UNDET  3


/* define GET LP for COMPOSITIONSTRING members. */
#define GETLPCOMPREADATTR(lpcs) (LPBYTE)((LPBYTE)(lpcs) + (lpcs)->dwCompReadAttrOffset)
#define GETLPCOMPREADCLAUSE(lpcs) (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwCompReadClauseOffset)
#define GETLPCOMPREADSTR(lpcs) (LPMYSTR)((LPBYTE)(lpcs) + (lpcs)->dwCompReadStrOffset)
#define GETLPCOMPATTR(lpcs) (LPBYTE)((LPBYTE)(lpcs) + (lpcs)->dwCompAttrOffset)
#define GETLPCOMPCLAUSE(lpcs) (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwCompClauseOffset)
#define GETLPCOMPSTR(lpcs) (LPMYSTR)((LPBYTE)(lpcs) + (lpcs)->dwCompStrOffset)
#define GETLPRESULTREADCLAUSE(lpcs) (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwResultReadClauseOffset)
#define GETLPRESULTREADSTR(lpcs) (LPMYSTR)((LPBYTE)(lpcs) + (lpcs)->dwResultReadStrOffset)
#define GETLPRESULTCLAUSE(lpcs) (LPDWORD)((LPBYTE)(lpcs) + (lpcs)->dwResultClauseOffset)
#define GETLPRESULTSTR(lpcs) (LPMYSTR)((LPBYTE)(lpcs) + (lpcs)->dwResultStrOffset)


#define SetClause(lpdw,num)   {*((LPDWORD)(lpdw)) = 0;*((LPDWORD)(lpdw)+1) = num;}

#define GCS_COMPALL ( GCS_COMPSTR | GCS_COMPATTR | GCS_COMPREADSTR | GCS_COMPREADATTR | GCS_COMPCLAUSE | GCS_COMPREADCLAUSE)
#define GCS_RESULTALL ( GCS_RESULTSTR | GCS_RESULTREADSTR | GCS_RESULTCLAUSE | GCS_RESULTREADCLAUSE)


// ImeMenu Define
#define NUM_ROOT_MENU_L 3
#define NUM_ROOT_MENU_R 3
#define NUM_SUB_MENU_L 2
#define NUM_SUB_MENU_R 2

#define IDIM_ROOT_ML_1       0x10
#define IDIM_ROOT_ML_2       0x11
#define IDIM_ROOT_ML_3       0x12
#define IDIM_SUB_ML_1        0x20
#define IDIM_SUB_ML_2        0x21
#define IDIM_ROOT_MR_1       0x30
#define IDIM_ROOT_MR_2       0x31
#define IDIM_ROOT_MR_3       0x32
#define IDIM_SUB_MR_1        0x40
#define IDIM_SUB_MR_2        0x41

#define NATIVE_CHARSET HANGUL_CHARSET

/**********************************************************************/
/*                                                                    */
/*      Structures                                                    */
/*                                                                    */
/**********************************************************************/
typedef struct _tagMYCOMPSTR{
    COMPOSITIONSTRING cs;
    MYCHAR         szCompReadStr[MAXCOMPSIZE];
    BYTE           bCompReadAttr[MAXCOMPSIZE];
    DWORD          dwCompReadClause[MAXCLAUSESIZE];
    MYCHAR         szCompStr[MAXCOMPSIZE];
    BYTE           bCompAttr[MAXCOMPSIZE];
    DWORD          dwCompClause[MAXCLAUSESIZE];
    char           szTypeInfo[MAXCOMPSIZE];
    MYCHAR         szResultReadStr[MAXCOMPSIZE];
    DWORD          dwResultReadClause[MAXCOMPSIZE];
    MYCHAR         szResultStr[MAXCOMPSIZE];
    DWORD          dwResultClause[MAXCOMPSIZE];
} MYCOMPSTR, NEAR *PMYCOMPSTR, FAR *LPMYCOMPSTR;

typedef struct _tagMYCAND{
    CANDIDATEINFO  ci;
    CANDIDATELIST  cl;
    DWORD          offset[MAXCANDSTRNUM];
    TCHAR          szCand[MAXCANDSTRNUM][MAXCANDSTRSIZE];
} MYCAND, NEAR *PMYCAND, FAR *LPMYCAND;


typedef struct _tagUICHILD{
    HWND    hWnd;
    BOOL    bShow;
    POINT   pt;
} UICHILD, NEAR *PUICHILD, FAR *LPUICHILD;

typedef struct _tagUICHILD2{
    HWND    hWnd;
    BOOL    bShow;
    RECT    rc;
} UICHILD2, NEAR *PUICHILD2, FAR *LPUICHILD2;

typedef struct _tagUIEXTRA{
    HIMC     hIMC;
    UICHILD  uiStatus;
    UICHILD  uiCand;
#if 1
    UICHILD  uiSoftKbd;
    DWORD    nShowSoftKbdCmd;
#endif
    DWORD    dwCompStyle;
    HFONT    hFont;
    BOOL     bVertical;
    UICHILD  uiDefComp;
    UICHILD2 uiComp[MAXCOMPWND];
    UICHILD  uiGuide;
} UIEXTRA, NEAR *PUIEXTRA, FAR *LPUIEXTRA;

typedef struct _tagMYGUIDELINE{
    DWORD dwLevel;
    DWORD dwIndex;
    DWORD dwStrID;
    DWORD dwPrivateID;
} MYGUIDELINE, NEAR *PMYGUIDELINE, FAR *LPMYGUIDELINE;

/**********************************************************************/
/*                                                                    */
/*      Externs                                                       */
/*                                                                    */
/**********************************************************************/
#ifndef _NO_EXTERN_
extern HINSTANCE  hInst;
extern HKL hMyKL;
extern LPTRANSMSGLIST lpCurTransKey;
extern UINT uNumTransKey;
extern BOOL fOverTransKey;
extern TCHAR szUIClassName[];
extern TCHAR szCompStrClassName[];
extern TCHAR szCandClassName[];
extern TCHAR szStatusClassName[];
extern TCHAR szGuideClassName[];
extern MYGUIDELINE glTable[];
extern TCHAR szDicFileName[];
extern BYTE bComp[];
extern BYTE bCompCtl[];
extern BYTE bCompSht[];
extern BYTE bCompAlt[];
extern BYTE bNoComp[];
extern BYTE bNoCompCtl[];
extern BYTE bNoCompSht[];
extern BYTE bNoCompAlt[];

#if DEBUG
extern DWORD dwLogFlag;
extern DWORD dwDebugFlag;
#endif

extern DWORD dwLayoutFlag;
extern DWORD dwComposeFlag;
extern DWORD dwOptionFlag;
extern DWORD dwHanjaMode;
extern DWORD dwScanCodeBased;
extern DWORD dwToggleKey;

extern DWORD dwImeFlag;

extern BOOL gfSaenaruSecure;

extern UINT WM_MSIME_QUERYPOSITION;
#endif //_NO_EXTERN_

/**********************************************************************/
/*                                                                    */
/*      Functions                                                     */
/*                                                                    */
/**********************************************************************/
/*   saenaru.c     */
int PASCAL Init(void);

/*   subs.c     */
void PASCAL InitCompStr(LPCOMPOSITIONSTRING lpCompStr,DWORD dwClrFlag);
void PASCAL ClearCompStr(LPCOMPOSITIONSTRING lpCompStr,DWORD dwClrFlag);
void PASCAL ClearCandidate(LPCANDIDATEINFO lpCandInfo);
void PASCAL ChangeMode(HIMC hIMC,DWORD dwToMode);
void PASCAL ChangeCompStr(HIMC hIMC,DWORD dwToMode);
HKL  PASCAL GetMyHKL (void) ;
BOOL PASCAL IsCompStr(HIMC hIMC);
BOOL PASCAL IsConvertedCompStr(HIMC hIMC);
BOOL PASCAL IsCandidate(LPINPUTCONTEXT lpIMC);
void PASCAL UpdateIndicIcon(HIMC hIMC);
void PASCAL lmemset(LPBYTE,BYTE,UINT);

LPWSTR PASCAL MyCharPrevW(LPWSTR lpStart, LPWSTR lpCur);
LPWSTR PASCAL MyCharNextW(LPWSTR lp);
HFONT CheckNativeCharset(HDC hDC);

int PASCAL GetSaenaruDirectory(LPTSTR lpDest, int max);

/*   toascii.c   */
BOOL PASCAL GenerateMessageToTransKey(LPTRANSMSGLIST lpTrabsKey,LPTRANSMSG lpGeneMsg);
BOOL PASCAL GenerateOverFlowMessage(LPTRANSMSGLIST lpTransKey);

/*   notify.c    */
BOOL PASCAL NotifyUCSetOpen(HIMC hIMC);
BOOL PASCAL NotifyUCConversionMode(HIMC hIMC);
BOOL PASCAL NotifyUCSetCompositionWindow(HIMC hIMC);

/*   input.c     */
BOOL PASCAL IMEKeydownHandler(HIMC,WPARAM,LPARAM,CONST LPBYTE);
BOOL PASCAL IMEKeyupHandler(HIMC,WPARAM,LPARAM,CONST LPBYTE);

/*   ui.c        */
BOOL IMERegisterClass(HANDLE hInstance);
LRESULT CALLBACK SAENARUWndProc(HWND,UINT,WPARAM,LPARAM);
LONG PASCAL NotifyCommand(HIMC hUICurIMC, HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
LONG PASCAL ControlCommand(HIMC hUICurIMC, HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam);
void PASCAL DrawUIBorder( LPRECT lprc );
void PASCAL DragUI(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

/*   uistate.c   */
LRESULT CALLBACK StatusWndProc(HWND,UINT,WPARAM,LPARAM);
void PASCAL PaintStatus(HWND hStatusWnd , HDC hDC, LPPOINT lppt,DWORD dwPushedStatus);
void PASCAL ButtonStatus(HWND hWnd, UINT message,WPARAM wParam,LPARAM lParam);
BOOL PASCAL MyIsIMEMessage(UINT message);
void PASCAL UpdateStatusWindow(LPUIEXTRA lpUIExtra);

/* uicand.c      */
LRESULT CALLBACK CandWndProc(HWND,UINT,WPARAM,LPARAM);
void PASCAL PaintCandWindow(HWND hCandWnd);
BOOL PASCAL GetCandPosFromComp(LPINPUTCONTEXT lpIMC, LPUIEXTRA lpUIExtra,LPPOINT lppt);
void PASCAL CreateCandWindow(HWND hUIWnd,LPUIEXTRA lpUIExtra, LPINPUTCONTEXT lpIMC);
void PASCAL ResizeCandWindow(LPUIEXTRA lpUIExtra,LPINPUTCONTEXT lpIMC);
void PASCAL HideCandWindow( LPUIEXTRA lpUIExtra);
void PASCAL MoveCandWindow(HWND hUIWnd, LPINPUTCONTEXT lpIMC, LPUIEXTRA lpUIExtra, BOOL fForceComp);

/* uicomp.c      */
LRESULT CALLBACK CompStrWndProc(HWND,UINT,WPARAM,LPARAM);
void PASCAL PaintCompWindow(HWND hCompWnd);
void PASCAL CreateCompWindow(HWND hUIWnd, LPUIEXTRA lpUIExtra,LPINPUTCONTEXT lpIMC);
void PASCAL MoveCompWindow(LPUIEXTRA lpUIExtra,LPINPUTCONTEXT lpIMC);
void PASCAL HideCompWindow(LPUIEXTRA lpUIExtra);
void PASCAL SetFontCompWindow(LPUIEXTRA lpUIExtra);

int PASCAL DrawCompCaret(HWND hCompWnd);

/* uisoft.c      */
void PASCAL ShowSoftKbd(HWND, int);
void PASCAL SetSoftKbdData(HWND);
void PASCAL UpdateSoftKbd(HWND);
void PASCAL SoftKbdDestroyed(HWND);

/*   uiguide.c   */
LRESULT CALLBACK GuideWndProc(HWND,UINT,WPARAM,LPARAM);
void PASCAL PaintGuide(HWND hGuideWnd , HDC hDC, LPPOINT lppt,DWORD dwPushedGuide);
void PASCAL ButtonGuide(HWND hWnd, UINT message,WPARAM wParam,LPARAM lParam);
void PASCAL UpdateGuideWindow(LPUIEXTRA lpUIExtra);
LRESULT CALLBACK LineWndProc(HWND,UINT,WPARAM,LPARAM);

/* config.c      */
INT_PTR  CALLBACK RegWordDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK SelectDictionaryDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK AboutDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK GeneralDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK DebugOptionDlgProc(HWND hDlg, UINT message , WPARAM wParam, LPARAM lParam);

void CALLBACK InstallLayout(HWND hwnd, HINSTANCE hinst, LPSTR lpszCmdLine, int nCmdShow);

/* regconf.c */
UINT load_keyboard_map_from_reg(LPCTSTR, UINT, WCHAR *);
UINT load_compose_map_from_reg(LPCTSTR, UINT, void *);

/* DIC.C         */
BOOL PASCAL IsEat(WORD);
BOOL PASCAL DicKeydownHandler(HIMC,UINT,LPARAM,CONST LPBYTE);
void PASCAL DeleteChar( HIMC hIMC ,UINT uVKey);
void PASCAL FlushText();
void PASCAL RevertText(HIMC hIMC);

void PASCAL AddChar(HIMC,WORD);
DWORD PASCAL ConvJunja(DWORD, WORD);

BOOL PASCAL ConvHanja(HIMC, int, int);
BOOL WINAPI MakeResultString(HIMC,BOOL);
BOOL PASCAL MakeGuideLine(HIMC, DWORD);
BOOL PASCAL GenerateMessage(HIMC,LPINPUTCONTEXT,LPTRANSMSGLIST,LPTRANSMSG);
BOOL PASCAL CheckAttr( LPCOMPOSITIONSTRING lpCompStr);
void PASCAL MakeAttrClause( LPCOMPOSITIONSTRING lpCompStr);
void PASCAL HandleShiftArrow( HIMC hIMC, BOOL fArrow);

/* hansub.c       */
DWORD PASCAL checkHangulKey(HIMC,UINT,LPARAM,LPBYTE);
void PASCAL hangulKeyHandler(HIMC,WPARAM,LPARAM,CONST LPBYTE);

/* hanjaidx.c       */
UINT PASCAL is_ksx1002(UINT);
int GetHangulFromHanjaIndex(LPMYSTR, LPMYSTR, DWORD, LPTSTR);

/* DIC2.C        */
BOOL OneCharZenToHan(WCHAR ,WCHAR* ,WCHAR* );
WORD PASCAL HanToZen(WORD,WORD,DWORD);
WORD PASCAL ZenToHan(WORD);
int  PASCAL IsFirst(WORD);
int  PASCAL IsSecond(WORD);
WORD PASCAL ConvChar(HIMC,WORD,WORD);
BOOL PASCAL IsTenten(WORD);
WORD PASCAL ConvTenten(WORD);
BOOL PASCAL IsMaru(WORD);
WORD PASCAL ConvMaru(WORD);
WORD PASCAL HiraToKata(WORD);
WORD PASCAL KataToHira(WORD);
void PASCAL lZenToHan(LPMYSTR,LPMYSTR);
void PASCAL lHanToZen(LPMYSTR,LPMYSTR,DWORD);

/* DIC3.C        */
WORD PASCAL AddWord(LPSTR,LPSTR);
WORD PASCAL GetWord(LPSTR,LPSTR,WORD,WORD);
WORD PASCAL GetWordCount(LPSTR);
WORD PASCAL GetWordNum(LPSTR);
WORD PASCAL DeleteWord(LPSTR,LPSTR);

/* reg.C      */
void SetDwordToSetting(LPCTSTR lpszFlag, DWORD dwFlag);
DWORD PASCAL GetDwordFromSetting(LPTSTR lpszFlag);
long PASCAL GetRegMultiStringValue (LPCTSTR,LPCTSTR,LPTSTR);
void SetRegMultiString(LPCTSTR lpszFlag, DWORD dwFlag);
long PASCAL GetRegStringValue (LPCTSTR,LPCTSTR,LPTSTR);
void PASCAL SetGlobalFlags();
void GetRegKeyList(LPCTSTR lpszSubDir);
BOOL GetRegKeyHandle(LPCTSTR lpszSubKey, HKEY *hKey);
void PASCAL ImeLog(DWORD dwFlag, LPTSTR lpStr);
#include "debug.h"

/* tsf.cpp */
#if !defined (NO_TSF)
BOOL PASCAL InitLanguageBar(void);
void PASCAL UninitLanguageBar(void);
BOOL PASCAL UpdateLanguageBarIfSelected(void);
BOOL PASCAL UpdateLanguageBar(void);
void PASCAL ActivateLanguageBar(BOOL);
BOOL PASCAL IsTSFEnabled(void);
BOOL PASCAL IsLangBarEnabled(void);
#endif

BOOL IsWindowsVersionOrLater(DWORD dwMajorVersion, DWORD dwMinorVersion, DWORD dwBuildNumber);

/*
 * ex: ts=8 sts=4 sw=4 et
 */
