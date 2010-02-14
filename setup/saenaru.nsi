; Saenaru Installation Script
; Written by Hye-Shik Chang <perky@i18n.org>
; $Saenaru: saenaru/setup/saenaru.nsi,v 1.9 2007/12/29 16:52:04 wkpark Exp $

!define RELVERSION      "1.1.0cvs-snapshot"
;!define RELVERSION      "1.1.0cvs-20100214"
!define REGISTRY_PATH_ROOT   "Software\OpenHangulProject\Saenaru"
!define REGISTRY_PATH   "Software\OpenHangulProject\Saenaru"
!define DDKBUILDDIR     "..\src\objfre_wxp_x86\i386"
!define DDK64BUILDDIR   "..\src\objfre_wnet_amd64\amd64"
!define DVBUILDDIR      "..\kbddvk\objfre_wxp_x86\i386"
!define DV64BUILDDIR    "..\kbddvk\objfre_wnet_amd64\amd64"
!define RESOURCEDIR     "..\resource"
!define HELPDIR         "..\help"
!define SRCROOTDIR      ".."
!define SMPATH          "$SMPROGRAMS\새나루"

!include "x64.nsh"
;---------------------
;Include Modern UI
!include "MUI.nsh"
;Include Radio buttons
!include "Sections.nsh"

BrandingText "새나루 인스톨러" 

;--------------------------------
;Configuration

  ;General
  Name "새나루 시험판 ${RELVERSION}"
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

  SectionIn RO

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
  File "${RESOURCEDIR}\comp_default.reg"
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
  StrCmp $0 "e0120412" exit
  ; set as default IME
  WriteRegStr HKCU "Keyboard Layout\Preload" "1" "e0120412"
  SetRebootFlag true

  ; set some Saenaru reg entries.

  ; get last IME
  StrCpy $1 1
  StrCpy $3 2
  loop:
    IntOp $1 $1 + 1
    ReadRegStr $2 HKCU "Keyboard Layout\Preload" "$1"
    IfErrors exit
    StrCmp $2 "e0120412" del loop1
  del:
    DeleteRegKey HKCU "Keyboard Layout\Preload\$1"
  loop1:
    IntOp $3 $3 + 1
    StrCpy $0 $2
    Goto loop
  exit:
SectionEnd

Section "한글입력기 목록에 추가" SecAdd
  ; get last IME
  StrCpy $1 0
  StrCpy $3 2
  StrCpy $4 1
  loop:
    IntOp $1 $1 + 1
    ReadRegStr $2 HKCU "Keyboard Layout\Preload" "$1"
    IfErrors save
    StrCmp $2 "e0120412" exit
    IntCmp $2 $4 loop loop inc
  inc:
    StrCpy $4 $1
    Goto loop

  save:
  ; save current IME
  ; set as default IME
  IntCmp $1 $4 save2 save2 save1
  save1:
  StrCpy $4 $1
  save2:
  WriteRegStr HKCU "Keyboard Layout\Preload" "$4" "e0120412"

  SetRebootFlag true
  exit:

SectionEnd

;--------------------------------
;Descriptions

  LangString DESC_SecBody ${LANG_KOREAN} "새나루 입력기를 위한 기본적인 파일을 설치합니다."
  LangString DESC_SecSource ${LANG_KOREAN} "새나루 소스를 설치합니다.$\r$\n새나루는 모든 소스코드가 공개된 자유 소프트웨어입니다.$\r$\n소스코드는 http://kldp.net/projects/saenaru 사이트에서 직접 받으실 수 있습니다."
  LangString DESC_SecDefault ${LANG_KOREAN} "새나루를 기본 입력기로 지정합니다.$\r$\n로그오프 후에 다시 로그인을 하거나 재부팅을 하셔야 설정이 반영됩니다."
  LangString DESC_SecAdd ${LANG_KOREAN} "새나루를 한글 입력기 목록에 추가합니다.$\r$\n이 경우 입력기 상태바에서 새나루를 선택하실 수 있게 됩니다."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBody} $(DESC_SecBody)
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

  Delete "$SYSDIR\saenaru.ime"
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
