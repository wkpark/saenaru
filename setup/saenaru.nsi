; Saenaru Installation Script
; Written by Hye-Shik Chang <perky@i18n.org>
; $Id$

!define RELVERSION      "1.1.0cvs-snapshot"
!define APPNAME         "새나루 스냅샷 ${RELVERSION}"

!define REGISTRY_PATH_ROOT   "Software\OpenHangulProject\Saenaru"
!define REGISTRY_PATH   "Software\OpenHangulProject\Saenaru"
!define DDKBUILDDIR     "..\src\objfre_wxp_x86\i386"
!define DDK64BUILDDIR   "..\src\objfre_wnet_amd64\amd64"
!define DVBUILDDIR      "..\kbddvk\objfre_wxp_x86\i386"
!define DV64BUILDDIR    "..\kbddvk\objfre_wnet_amd64\amd64"
!define CMBUILDDIR      "..\colemak\objfre_wxp_x86\i386"
!define CM64BUILDDIR    "..\colemak\objfre_wnet_amd64\amd64"
!define RESOURCEDIR     "..\resource"
!define HELPDIR         "..\help"
!define SRCROOTDIR      ".."
!define SMPATH          "$SMPROGRAMS\새나루"

!include /NONFATAL ".\local.nsi"

!include "x64.nsh"
;---------------------
;Include Modern UI
!include "MUI2.nsh"
;Include Radio buttons
!include "Sections.nsh"

SetCompressor /SOLID lzma

BrandingText "새나루 인스톨러"

;--------------------------------
;Configuration

  ;General
  Name "${APPNAME}"
  OutFile "Saenaru-${RELVERSION}.exe"
  !define MUI_ICON "install.ico"
  !define MUI_UNICON "install.ico"

  ;Folder selection page
  InstallDir "$WINDIR\IME\Saenaru"
  
  ;Get install folder from registry if available
  ;;InstallDirRegKey HKLM "${REGISTRY_PATH}" ""
  ;InstallDirRegKey HKCU "${REGISTRY_PATH}" ""

;--------------------------------
;Pages

  !insertmacro MUI_DEFAULT MUI_WELCOMEFINISHPAGE_BITMAP "setup.bmp"

  !insertmacro MUI_PAGE_WELCOME 
  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_INSTFILES
  Page custom OpenInputSetting
  Page custom OpenUserKeyboard OpenUserKbdSettings
  !insertmacro MUI_PAGE_FINISH 
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

  !insertmacro MUI_UNPAGE_INSTFILES
  
;--------------------------------
;Interface Settings

  !define MUI_ABORTWARNING
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "Korean"

;--------------------------------
;Installer Sections

Section "새나루 입력기" SecBody

  ;SectionIn RO

  SetOutPath "$SYSDIR"
  SetOverwrite try
  File "${DDKBUILDDIR}\saenaru.ime"
  IfErrors 0 SaenaruDone

  # Can't copy saenaru.ime, create it under another name and rename it on
  # next reboot.
  GetTempFileName $3
  File /oname=$3 "${DDKBUILDDIR}\saenaru.ime"
  Rename /REBOOTOK $3 $SYSDIR\saenaru.ime
  SaenaruDone:

  File "${DVBUILDDIR}\kbddvk.dll"
  IfErrors 0 kbddvkDone

  GetTempFileName $3
  File /oname=$3 "${DVBUILDDIR}\kbddvk.dll"
  Rename /REBOOTOK $3 $SYSDIR\kbddvk.dll
  kbddvkDone:

  #
  # 64bit install
  #
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    File "${DDK64BUILDDIR}\saenaru.ime"
    IfErrors 0 Saenaru64Done

    # Can't copy saenaru.ime, create it under another name and rename it on
    # next reboot.
    GetTempFileName $3
    File /oname=$3 "${DDK64BUILDDIR}\saenaru.ime"
    Rename /REBOOTOK $3 $SYSDIR\saenaru.ime
    Saenaru64Done:


    File "${DV64BUILDDIR}\kbddvk.dll"
    IfErrors 0 kbddvk64Done

    GetTempFileName $3
    File /oname=$3 "${DV64BUILDDIR}\kbddvk.dll"
    Rename /REBOOTOK $3 $SYSDIR\kbddvk.dll
    kbddvk64Done:
    ${EnableX64FSRedirection}
  ${EndIf}

  SetOverwrite lastused
  SetOutPath "$INSTDIR\help"
  File "${HELPDIR}\saenaru.chm"
  SetOutPath "$INSTDIR"
  File "${RESOURCEDIR}\saenaru.dic"
  File "${RESOURCEDIR}\word.dic"
  File "${RESOURCEDIR}\symwin.dic"
  File "${RESOURCEDIR}\nabi.dic"
  File "${RESOURCEDIR}\jinsuk.dic"
  File "${RESOURCEDIR}\2set3set.reg"
  File "${RESOURCEDIR}\ahnmatae.reg"
  File "${RESOURCEDIR}\old2set3set.reg"
  File "${RESOURCEDIR}\nk2set.reg"
  File "${RESOURCEDIR}\comp_default.reg"
  ;;
  !include /NONFATAL ".\userkbd\file.nsi"
  ;;
  File /oname=saenaru.ico "${RESOURCEDIR}\about.ico"
  
  ;Store install folder
  WriteRegStr HKLM "${REGISTRY_PATH}" "" $INSTDIR

  ;IME keys
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout file" "kbdkor.dll"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout text" "새나루 한글 입력기"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout display name" "한글 입력기 (새나루)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "IME file" "SAENARU.IME"

  ; dvorak driver support
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "Layout file" "kbddvk.dll"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "Layout text" "새나루 한글 입력기"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "Layout display name" "한글 입력기 (새나루 드보락)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "IME file" "SAENARU.IME"

  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "" "nabi.dic"
  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "Symbol" "symwin.dic"
  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "Word" "word.dic"
  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "HanjaIndex" "jinsuk.dic"

  WriteRegStr HKCU "${REGISTRY_PATH}" "OptionFlag" "9"
  ;WriteRegStr HKCU "${REGISTRY_PATH" "LayoutFlag" "1"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Write uninstallation info
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "DisplayName" "새나루 한글 입력기"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "DisplayIcon" "$INSTDIR\saenaru.ico,-0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "DisplayVersion" "${RELVERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "Publisher" "Open Hangul Project"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "HelpLink" "http://saenaru.i18n.org"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "URLInfoAbout" "http://kldp.net/projects/saenaru"

  ;Create short cuts
  CreateDirectory "${SMPATH}"
  SetOutPath "${SMPATH}"
  File "/oname=새나루 프로젝트.url" "httplink-saenaru.url"
  File "/oname=열린 한글 프로젝트.url" "httplink-hangul.url"
  CreateShortCut "${SMPATH}\Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

  IfRebootFlag 0 noreboot
    MessageBox MB_OK|MB_ICONINFORMATION "새나루가 사용중이기 때문에 설치/업데이트가 완료되지 못했습니다. 재부팅을 해야 설치가 완료됩니다."
  noreboot:
SectionEnd

Section /o "새나루 콜맥(Colemak)" SecColemak

  ;SectionIn RO

  SetOutPath "$SYSDIR"
  SetOverwrite try

  File "${CMBUILDDIR}\kbdcmk.dll"
  IfErrors 0 kbdcmkDone

  GetTempFileName $3
  File /oname=$3 "${CMBUILDDIR}\kbdcmk.dll"
  Rename /REBOOTOK $3 $SYSDIR\kbdcmk.dll
  kbdcmkDone:

  #
  # 64bit install
  #
  ${If} ${RunningX64}
    ${DisableX64FSRedirection}

    File "${CM64BUILDDIR}\kbdcmk.dll"
    IfErrors 0 kbdcmk64Done

    GetTempFileName $3
    File /oname=$3 "${CM64BUILDDIR}\kbdcmk.dll"
    Rename /REBOOTOK $3 $SYSDIR\kbdcmk.dll
    kbdcmk64Done:
    ${EnableX64FSRedirection}
  ${EndIf}

  ; Colemak driver support
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "Layout file" "kbdcmk.dll"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "Layout text" "새나루 한글 입력기"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "Layout display name" "한글 입력기 (새나루 콜맥)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "IME file" "SAENARU.IME"

  IfRebootFlag 0 noreboot
    MessageBox MB_OK|MB_ICONINFORMATION "새나루가 사용중이기 때문에 설치/업데이트가 완료되지 못했습니다. 재부팅을 해야 설치가 완료됩니다."
  noreboot:
SectionEnd

Section /o "새나루 소스 코드" SecSource

  SetOutPath "$INSTDIR\Source"
  File ${SRCROOTDIR}\DIRS
  File ${SRCROOTDIR}\LICENSE
  File ${SRCROOTDIR}\LICENSE.MICROSOFT
  File ${SRCROOTDIR}\Makefile

  SetOutPath "$INSTDIR\Source\doc"
  File ${SRCROOTDIR}\doc\saenaru.htm

  SetOutPath "$INSTDIR\Source\resource"
  File ${SRCROOTDIR}\resource\*.reg
  File ${SRCROOTDIR}\resource\*.bmp
  File ${SRCROOTDIR}\resource\*.ico
  File ${SRCROOTDIR}\resource\*.dlg
  File ${SRCROOTDIR}\resource\*.rcv
  File ${SRCROOTDIR}\resource\*.dic

  SetOutPath "$INSTDIR\Source\resource\button"
  File ${SRCROOTDIR}\resource\button\*.ico

  SetOutPath "$INSTDIR\Source\resource\text"
  File ${SRCROOTDIR}\resource\text\*.ico

  SetOutPath "$INSTDIR\Source\setup"
  File ${SRCROOTDIR}\setup\LICENSE.txt
  File ${SRCROOTDIR}\setup\saenaru.nsi
  File ${SRCROOTDIR}\setup\saenaru.inf
  File ${SRCROOTDIR}\setup\*.url

  SetOutPath "$INSTDIR\Source\src"
  File ${SRCROOTDIR}\src\*.rc
  File ${SRCROOTDIR}\src\makefile
  File ${SRCROOTDIR}\src\makefile.inc
  File ${SRCROOTDIR}\src\saenaru.def
  File ${SRCROOTDIR}\src\sources
  File ${SRCROOTDIR}\src\*.c
  File ${SRCROOTDIR}\src\*.h
  File ${SRCROOTDIR}\src\*.cpp
  File ${SRCROOTDIR}\src\*.def

SectionEnd

Section /o "기본 한글입력기로 지정" SecDefault
  ; save current IME
  ReadRegStr $0 HKCU "Keyboard Layout\Preload" "1"
  StrCmp $0 "e0120412" exit # saenaru
  StrCmp $0 "e0130412" exit # saenaru dvorak
  ; set Saenaru as the default IME
  ; with rundll32
  ;
  ; save default IME
  StrCpy $1 $0

  ; get last IME
  !tempfile FILE
  !appendfile "${FILE}" '[RegionalSettings]$\r$\n;LanguageGroup="1","2","3","7","8"$\r$\n;SystemLocale=0412$\r$\n'
  !appendfile "${FILE}" ';UserLocale=0412$\r$\n'
  !appendfile "${FILE}" 'InputLocale="0412:e0120412","0412:e0130412"$\r$\n'
  !appendfile "${FILE}" 'UserLocale_DefaultUser=0412$\r$\n'
  !appendfile "${FILE}" ';InputLocale_DefaultUser="0412:e0010412"$\r$\n'
  Exec 'RunDll32.exe shell32.dll,Control_RunDLL intl.cpl,,/f:"${FILE}"'
  !undef FILE
  exit:
SectionEnd


Section "한글입력기 목록에 추가" SecAdd
  ; save current IME
  ReadRegStr $0 HKCU "Keyboard Layout\Preload" "1"
  StrCmp $0 "e0120412" exit
  StrCmp $0 "e0130412" exit

  ; known hangul IME
  StrCmp $0 "e0010412" save ; IME 2002
  StrCmp $0 "e0200412" save ; IME 2003
  StrCmp $0 "e0220412" save ; 날개셋
  ; set Saenaru as the default IME
  ; with rundll32
  ;
  ; save default IME
  save:
  StrCpy $1 $0
  StrCpy $2 $0 "" -4 # lang code 0412 etc.

  ; get last IME
  !tempfile FILE
  !appendfile "${FILE}" '[RegionalSettings]$\r$\n;LanguageGroup="1","2","3","7","8"$\r$\n;SystemLocale="00000412"$\r$\n'
  !appendfile "${FILE}" ';UserLocale=0412$\r$\n'
  !appendfile "${FILE}" 'InputLocale="$2:$1","0412:e0120412","0412:e0130412"$\r$\n'
  !appendfile "${FILE}" 'UserLocale_DefaultUser=0412$\r$\n'
  !appendfile "${FILE}" ';InputLocale_DefaultUser="$2:$1"$\r$\n'
  Exec 'RunDll32.exe shell32.dll,Control_RunDLL intl.cpl,,/f:"${FILE}"'
  !undef FILE
  exit:
SectionEnd

  LangString DESC_IME_SETTING_TITLE ${LANG_KOREAN} "입력기 설정"
  LangString DESC_IME_SETTING_SUBTITLE ${LANG_KOREAN} "입력기 설정을 사용자가 직접 변경할 수 있습니다."
  LangString DESC_IME_SETTING_HEADER ${LANG_KOREAN} "제어판의 입력기 설정 변경창을 직접 열어 사용자가 입력기 설정을 추가 및 변경할 수 있습니다."
  LangString DESC_IME_SETTING_BUTTON ${LANG_KOREAN} "입력기 설정"

  LangString DESC_KBD_SETTING_TITLE ${LANG_KOREAN} "사용자 정의 키보드 설정"
  LangString DESC_KBD_SETTING_SUBTITLE ${LANG_KOREAN} "미리 정의된 사용자 정의 키보드 정보를 등록합니다."
  LangString DESC_KBD_SETTING_HEADER ${LANG_KOREAN} "미리 정의된 사용자 정의 키보드입니다. 아래 목록에 있는 사용자 정의 자판은 새나루 환경설정 메뉴에 나타나며 기본자판처럼 쓸 수 있습니다."
  LangString DESC_KBD_OLD_BUTTON ${LANG_KOREAN} "옛한글 두벌/세벌 자판 (&Y)"
  LangString DESC_KBD_NEW_BUTTON ${LANG_KOREAN} "새두벌/새세벌 자판 (&N)"
  LangString DESC_KBD_NK_BUTTON ${LANG_KOREAN} "북한 두벌식 (&K)"

!include /NONFATAL ".\userkbd\extra.nsi"

Function OpenInputSetting
	nsDialogs::Create 1018
	!insertmacro MUI_HEADER_TEXT "$(DESC_IME_SETTING_TITLE)" "$(DESC_IME_SETTING_SUBTITLE)"
	${NSD_CreateLabel} 0 0 100% 143 "$(DESC_IME_SETTING_HEADER)"
	${NSD_CreateButton} 0 144 90u 17u "$(DESC_IME_SETTING_BUTTON)"
	Pop $0
	${NSD_OnClick} $0 OpenKeyboardSettings
	
	nsDialogs::Show
FunctionEnd

Function OpenKeyboardSettings
	Exec "RunDll32.exe shell32.dll,Control_RunDLL input.dll"
FunctionEnd

Var old_kbd
Var new_kbd
Var nk_kbd
Var user1_kbd
Var dialog
Function OpenUserKeyboard
	nsDialogs::Create 1018
        Pop $dialog
	!insertmacro MUI_HEADER_TEXT "$(DESC_KBD_SETTING_TITLE)" "$(DESC_KBD_SETTING_SUBTITLE)"
	${NSD_CreateLabel} 0 0 100% 103 "$(DESC_KBD_SETTING_HEADER)"
	${NSD_CreateCheckBox} 0 104 110u 17u "$(DESC_KBD_OLD_BUTTON)"
	Pop $old_kbd
	${NSD_CreateCheckBox} 120u 104 110u 17u "$(DESC_KBD_NEW_BUTTON)"
	Pop $new_kbd
	${NSD_CreateCheckBox} 0 144 110u 17u "$(DESC_KBD_NK_BUTTON)"
	Pop $nk_kbd

        !include /NONFATAL "userkbd\ui.nsi"

	nsDialogs::Show
FunctionEnd

Function OpenUserKbdSettings
        ${NSD_GetState} $old_kbd $0
        ${If} $0 == 1
	  Exec "Regedit /s $INSTDIR\old2set3set.reg"
        ${EndIf}

        ${NSD_GetState} $new_kbd $0
        ${If} $0 == 1
	  Exec "Regedit /s $INSTDIR\2set3set.reg"
        ${EndIf}

        ${NSD_GetState} $nk_kbd $0
        ${If} $0 == 1
	  Exec "Regedit /s $INSTDIR\nk2set.reg"
        ${EndIf}

        ${NSD_GetState} $user1_kbd $0
        ${If} $0 == 1
	  Exec "Regedit /s $INSTDIR\user1.reg"
        ${EndIf}
FunctionEnd

;--------------------------------
;Descriptions

  LangString DESC_SecBody ${LANG_KOREAN} "새나루 입력기를 위한 기본적인 파일을 설치합니다."
  LangString DESC_SecSource ${LANG_KOREAN} "새나루 소스를 설치합니다.$\r$\n새나루는 모든 소스코드가 공개된 자유 소프트웨어입니다.$\r$\n소스코드는 http://kldp.net/projects/saenaru 사이트에서 직접 받으실 수 있습니다."
  LangString DESC_SecDefault ${LANG_KOREAN} "새나루를 기본 입력기로 지정합니다.$\r$\n로그오프 후에 다시 로그인을 하거나 재부팅을 하셔야 설정이 반영됩니다."
  LangString DESC_SecAdd ${LANG_KOREAN} "새나루를 한글 입력기 목록에 추가합니다.$\r$\n이 경우 입력기 상태바에서 새나루를 선택하실 수 있게 됩니다."
  LangString DESC_SecColemak ${LANG_KOREAN} "새나루 콜맥(Colemak)자판 지원을 위한 콜맥 키보드 레이아웃 드라이버 설치 및 설정을 합니다."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBody} $(DESC_SecBody)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecColemak} $(DESC_SecColemak)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecSource} $(DESC_SecSource)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecAdd} $(DESC_SecAdd)
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDefault} $(DESC_SecDefault)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

Function .onInit
  StrCpy $1 ${SecAdd}
FunctionEnd

Function .onSelChange
  !insertmacro StartRadioButtons $1
  !insertmacro RadioButton "${SecAdd}"
  !insertmacro RadioButton "${SecDefault}"
FunctionEnd

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    Delete "$SYSDIR\saenaru.ime"
    Delete "$SYSDIR\kbddvk.dll"
    Delete "$SYSDIR\kbdcmk.dll"
    ${EnableX64FSRedirection}
  ${EndIf}

  Delete "$SYSDIR\saenaru.ime"
  Delete "$SYSDIR\kbddvk.dll"
  Delete "$SYSDIR\kbdcmk.dll"

  Delete "$INSTDIR\saenaru.dic"
  Delete "$INSTDIR\nabi.dic"
  Delete "$INSTDIR\winsym.dic"
  Delete "$INSTDIR\word.dic"
  Delete "$INSTDIR\jinsuk.dic"
  Delete "$INSTDIR\help\saenaru.chm"

  Delete "$INSTDIR\Source\DIRS"
  Delete "$INSTDIR\Source\LICENSE"
  Delete "$INSTDIR\Source\LICENSE.MICROSOFT"
  Delete "$INSTDIR\Source\Makefile"
  Delete "$INSTDIR\Source\doc\saenaru.htm"
  Delete "$INSTDIR\Source\resource\2set3set.reg"
  Delete "$INSTDIR\Source\resource\ahnmatae.reg"
  Delete "$INSTDIR\Source\resource\ARWDWN.ico"
  Delete "$INSTDIR\Source\resource\ARWUP.ico"
  Delete "$INSTDIR\Source\resource\about.ico"
  Delete "$INSTDIR\Source\resource\button\engkey.ico"
  Delete "$INSTDIR\Source\resource\button\hanjakey.ico"
  Delete "$INSTDIR\Source\resource\button\hankey.ico"
  Delete "$INSTDIR\Source\resource\check.bmp"
  Delete "$INSTDIR\Source\resource\close.bmp"
  Delete "$INSTDIR\Source\resource\face.bmp"
  Delete "$INSTDIR\Source\resource\full.ico"
  Delete "$INSTDIR\Source\resource\full_old.ico"
  Delete "$INSTDIR\Source\resource\general.ico"
  Delete "$INSTDIR\Source\resource\half.ico"
  Delete "$INSTDIR\Source\resource\keyboard.ico"
  Delete "$INSTDIR\Source\resource\imepad.ico"
  Delete "$INSTDIR\Source\resource\penindic.ico"
  Delete "$INSTDIR\Source\resource\saenaru.dic"
  Delete "$INSTDIR\Source\resource\winsym.dic"
  Delete "$INSTDIR\Source\resource\nabi.dic"
  Delete "$INSTDIR\Source\resource\word.dic"
  Delete "$INSTDIR\Source\resource\jinsuk.dic"
  Delete "$INSTDIR\Source\resource\saenaru.dlg"
  Delete "$INSTDIR\Source\resource\saenaru.ico"
  Delete "$INSTDIR\Source\resource\saenaru.rcv"
  Delete "$INSTDIR\Source\resource\status.bmp"
  Delete "$INSTDIR\Source\resource\text\engkey.ico"
  Delete "$INSTDIR\Source\resource\text\hanjakey.ico"
  Delete "$INSTDIR\Source\resource\text\hankey.ico"
  Delete "$INSTDIR\Source\resource\uncheck.bmp"
  Delete "$INSTDIR\Source\setup\LICENSE.txt"
  Delete "$INSTDIR\Source\setup\httplink-hangul.url"
  Delete "$INSTDIR\Source\setup\httplink-saenaru.url"
  Delete "$INSTDIR\Source\setup\saenaru.inf"
  Delete "$INSTDIR\Source\setup\saenaru.nsi"
  Delete "$INSTDIR\Source\src\btncmd.cpp"
  Delete "$INSTDIR\Source\src\btnext.cpp"
  Delete "$INSTDIR\Source\src\btnime.cpp"
  Delete "$INSTDIR\Source\src\config.c"
  Delete "$INSTDIR\Source\src\data.c"
  Delete "$INSTDIR\Source\src\dic.c"
  Delete "$INSTDIR\Source\src\dic2.c"
  Delete "$INSTDIR\Source\src\fdebug.c"
  Delete "$INSTDIR\Source\src\hangul.c"
  Delete "$INSTDIR\Source\src\hanjaidx.c"
  Delete "$INSTDIR\Source\src\imm.c"
  Delete "$INSTDIR\Source\src\immsec.c"
  Delete "$INSTDIR\Source\src\immsec.h"
  Delete "$INSTDIR\Source\src\indicml.h"
  Delete "$INSTDIR\Source\src\input.c"
  Delete "$INSTDIR\Source\src\makefile"
  Delete "$INSTDIR\Source\src\makefile.inc"
  Delete "$INSTDIR\Source\src\process.c"
  Delete "$INSTDIR\Source\src\reg.c"
  Delete "$INSTDIR\Source\src\regword.c"
  Delete "$INSTDIR\Source\src\resource.h"
  Delete "$INSTDIR\Source\src\saenaru.c"
  Delete "$INSTDIR\Source\src\saenaru.def"
  Delete "$INSTDIR\Source\src\saenaru.h"
  Delete "$INSTDIR\Source\src\saenaru.rc"
  Delete "$INSTDIR\Source\src\sources"
  Delete "$INSTDIR\Source\src\subs.c"
  Delete "$INSTDIR\Source\src\toascii.c"
  Delete "$INSTDIR\Source\src\tsf.cpp"
  Delete "$INSTDIR\Source\src\tsf.h"
  Delete "$INSTDIR\Source\src\ui.c"
  Delete "$INSTDIR\Source\src\uicand.c"
  Delete "$INSTDIR\Source\src\uicomp.c"
  Delete "$INSTDIR\Source\src\uiguide.c"
  Delete "$INSTDIR\Source\src\uistate.c"
  Delete "$INSTDIR\Source\src\vksub.h"
  Delete "$INSTDIR\saenaru.ico"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR\Source\src"
  RMDir "$INSTDIR\Source\setup"
  RMDir "$INSTDIR\Source\resource\text"
  RMDir "$INSTDIR\Source\resource\button"
  RMDir "$INSTDIR\Source\resource"
  RMDir "$INSTDIR\Source\doc"
  RMDir "$INSTDIR\Source"
  RMDir "$INSTDIR"

  Delete "${SMPATH}\*.*"
  RMDir "${SMPATH}"

  DeleteRegKey HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412"
  DeleteRegKey HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru"
  DeleteRegKey /ifempty HKLM "${REGISTRY_PATH}"

SectionEnd

; ex: ts=8 sts=2 sw=2 et
