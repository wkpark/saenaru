; Saenaru Installation Script
; Written by Hye-Shik Chang <perky@i18n.org>
; $Saenaru$

!define RELVERSION      "031226"
!define REGISTRY_PATH   "Software\OpenHangulProject\Saenaru"
!define DDKBUILDDIR     "..\src\objfre\i386"
!define RESOURCEDIR     "..\resource"
!define SRCROOTDIR      ".."
!define SMPATH          "$SMPROGRAMS\새나루"

;---------------------
;Include Modern UI

  !include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  Name "새나루 시험판 ${RELVERSION}"
  OutFile "Saenaru-${RELVERSION}.exe"

  ;Folder selection page
  InstallDir "$WINDIR\IME\Saenaru"
  
  ;Get install folder from registry if available
  InstallDirRegKey HKLM "${REGISTRY_PATH}" ""

;--------------------------------
;Pages

  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_INSTFILES
  
  !insertmacro MUI_UNPAGE_CONFIRM
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
  File "${DDKBUILDDIR}\saenaru.ime"
  SetOutPath "$WINDIR"
  File "${RESOURCEDIR}\saenaru.dic"
  ; XXX 나중에 saenaru.dic은 WINDIR\ime\Saenaru로 옮겨야함.
  SetOutPath "$INSTDIR"
  File /oname=saenaru.ico "${RESOURCEDIR}\about.ico"
  
  ;Store install folder
  WriteRegStr HKLM "${REGISTRY_PATH}" "" $INSTDIR

  ;IME keys
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout file" "kbdkor.kbd"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout text" "새나루 한글 입력기"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout display name" "한글 입력기 (새나루)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "IME file" "SAENARU.IME"
  
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

;--------------------------------
;Descriptions

  LangString DESC_SecBody ${LANG_KOREAN} "새나루 입력기를 위한 기본적인 파일을 설치합니다."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecBody} $(DESC_SecBody)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$SYSDIR\saenaru.ime"
  Delete "$WINDIR\saenaru.dic"

  Delete "$INSTDIR\Source\DIRS"
  Delete "$INSTDIR\Source\LICENSE"
  Delete "$INSTDIR\Source\LICENSE.MICROSOFT"
  Delete "$INSTDIR\Source\Makefile"
  Delete "$INSTDIR\Source\doc\saenaru.htm"
  Delete "$INSTDIR\Source\resource\2set3set.reg"
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
  Delete "$INSTDIR\Source\resource\half_old.ico"
  Delete "$INSTDIR\Source\resource\keyboard.ico"
  Delete "$INSTDIR\Source\resource\penindic.ico"
  Delete "$INSTDIR\Source\resource\saenaru.dic"
  Delete "$INSTDIR\Source\resource\saenaru.dlg"
  Delete "$INSTDIR\Source\resource\saenaru.ico"
  Delete "$INSTDIR\Source\resource\saenaru.rcv"
  Delete "$INSTDIR\Source\resource\saenaru0.dic"
  Delete "$INSTDIR\Source\resource\status.bmp"
  Delete "$INSTDIR\Source\resource\status_old.bmp"
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
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru"
  DeleteRegKey /ifempty HKLM "${REGISTRY_PATH}"

SectionEnd

; ex: ts=8 sts=2 sw=2 et
