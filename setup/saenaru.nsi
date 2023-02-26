; Saenaru Installation Script
; by Hye-Shik Chang <perky@i18n.org>

Unicode true

!define RELVERSION      "1.2.0-GIT"
!include /NONFATAL ".\ver.nsi"
!ifndef DEVREL
  !define APPNAME         "새나루 ${RELVERSION}"
!else
  !define APPNAME         "새나루 스냅샷 ${RELVERSION}"
!endif

!define REGISTRY_PATH_ROOT   "Software\OpenHangulProject\Saenaru"
!define REGISTRY_PATH   "Software\OpenHangulProject\Saenaru"
!define DDKBUILDDIR     "..\Release"
!define DDK64BUILDDIR   "..\x64\Release"
!define DVBUILDDIR      "..\prebuilt\kbddvk\x86"
!define DV64BUILDDIR    "..\prebuilt\kbddvk\x64"
!define CMBUILDDIR      "..\prebuilt\colemak\x86"
!define CM64BUILDDIR    "..\prebuilt\colemak\x64"
!define RESOURCEDIR     "..\resource"
!define HELPDIR         "..\prebuilt"
!define SRCROOTDIR      ".."
!define SMPATH          "$SMPROGRAMS\새나루"

;; custom fixes
!include /NONFATAL ".\local.nsi"

!include "x64.nsh"
;---------------------
;Include Modern UI
!include "MUI2.nsh"
;Include Radio buttons
!include "Sections.nsh"
;Include Library
!include "Library.nsh"

SetCompressor /SOLID lzma

BrandingText "새나루 인스톨러"

;--------------------------------
;Configuration

  ;General
  Name "${APPNAME}"
  !ifndef OUTPUTFILE
    OutFile "Saenaru-${RELVERSION}.exe"
  !else
    OutFile "${OUTPUTFILE}"
  !endif

  !define MUI_ICON "install.ico"
  !define MUI_UNICON "install.ico"

  ;Folder selection page
  InstallDir "$WINDIR\IME\Saenaru"
  
  ;Get install folder from registry if available
  ;;InstallDirRegKey HKLM "${REGISTRY_PATH}" ""
  ;InstallDirRegKey HKCU "${REGISTRY_PATH}" ""

;--------------------------------
;Pages

  !ifndef DEVREL
    !insertmacro MUI_DEFAULT MUI_WELCOMEFINISHPAGE_BITMAP "setup.bmp"
  !else
    !insertmacro MUI_DEFAULT MUI_WELCOMEFINISHPAGE_BITMAP "dev.bmp"
  !endif

  !insertmacro MUI_PAGE_WELCOME 
  !insertmacro MUI_PAGE_LICENSE "LICENSE.txt"
  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_INSTFILES
  Page custom OpenEnableIme EnableImeSettings
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

  LangString DESC_VCRUNTIME_MSG ${LANG_KOREAN} "VC런타임이 필요합니다.$\r$\n설치를 계속하기 위해 VC런타임을 설치하시겠습니까?"
  LangString DESC_END_INSTALL ${LANG_KOREAN} "설치를 종료합니다"
  LangString DESC_VCREDIST_DOWNLOAD ${LANG_KOREAN} "VC++ Redist를 다운로드합니다..."
  LangString DESC_VCREDIST86_DOWNLOAD ${LANG_KOREAN} "VC++ Redist(x86)를 다운로드합니다..."
  LangString DESC_VCREDIST_INSTALL ${LANG_KOREAN} "VC런타임을 설치합니다..."
  LangString DESC_VCREDIST86_INSTALL ${LANG_KOREAN} "VC런타임(x86)을 설치합니다..."
  LangString DESC_INSTALL_WITHOUT_VCREDIST ${LANG_KOREAN} "VC런타임없이 설치를 진행합니다.$\r$\n설치후 실행이 제대로 안될 수 있습니다."

;--------------------------------
;check vcruntime140.dll
Section "VC런타임 점검" SecPrep
  SectionIn RO

  IfFileExists $SYSDIR\vcruntime140.dll 0 +2
  Goto VCRuntimeNext
  MessageBox MB_YESNOCANCEL|MB_ICONINFORMATION "$(DESC_VCRUNTIME_MSG)" IDYES +4 IDNO +3
  MessageBox MB_OK|MB_ICONSTOP "$(DESC_END_INSTALL)"
  Quit
  Goto VCRuntimeNo
  DetailPrint "$(DESC_VCREDIST86_DOWNLOAD)"
  NSISdl::download "https://aka.ms/vs/17/release/vc_redist.x86.exe" "$Temp\vc_redist.x86.exe"
  DetailPrint "$(DESC_VCREDIST86_INSTALL)"
  ExecWait '"$Temp\vc_redist.x86.exe" /norestart'
  VCRuntimeNext:
  ${If} ${RunningX64}
    IfFileExists $SYSDIR\vcruntime140.dll 0 +2
    Goto VCRuntimePass
    MessageBox MB_YESNOCANCEL|MB_ICONINFORMATION "$(DESC_VCRUNTIME_MSG)" IDYES +4 IDNO +3
    MessageBox MB_OK|MB_ICONSTOP "$(DESC_END_INSTALL)"
    Quit
    Goto VCRuntimeNo
    DetailPrint "$(DESC_VCREDIST_DOWNLOAD)"
    NSISdl::download "https://aka.ms/vs/17/release/vc_redist.x64.exe" "$Temp\vc_redist.x64.exe"
    DetailPrint "$(DESC_VCREDIST_INSTALL)"
    ExecWait '"$Temp\vc_redist.x64.exe" /norestart'
  ${EndIf}

  Goto VCRuntimePass
  VCRuntimeNo:
  MessageBox MB_OK|MB_ICONEXCLAMATION "$(DESC_INSTALL_WITHOUT_VCREDIST)"

  VCRuntimePass:
SectionEnd

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

  File "${DDKBUILDDIR}\SaenaruTip.dll"
  IfErrors 0 SaenaruTipDone

  GetTempFileName $3
  File /oname=$3 "${DDKBUILDDIR}\SaenaruTip.dll"
  Rename /REBOOTOK $3 $SYSDIR\SaenaruTip.dll
  SaenaruTipDone:

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

    !insertmacro InstallLib REGDLL NOTSHARED REBOOT_PROTECTED "${DDK64BUILDDIR}\SaenaruTip.dll" "$SYSDIR\SaenaruTip.dll" $SYSDIR

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
  ;; custom fix
  !include /NONFATAL ".\userkbd\file.nsi"
  ;;
  File /oname=saenaru.ico "${RESOURCEDIR}\about.ico"
  
  ;Store install folder
  WriteRegStr HKLM "${REGISTRY_PATH}" "" $INSTDIR

  ;IME keys
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout File" "kbdkor.dll"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout Text" "새나루 한글 입력기"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "Layout Display Name" "새나루 한글 입력기"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412" "IME File" "SAENARU.IME"

  ; dvorak driver support
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "Layout File" "kbddvk.dll"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "Layout Text" "새나루 한글 입력기 (드보락)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "Layout Display Name" "새나루 한글 입력기 (드보락)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412" "IME File" "SAENARU.IME"

  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "" "nabi.dic"
  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "Symbol" "symwin.dic"
  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "Word" "word.dic"
  WriteRegStr HKCU "${REGISTRY_PATH}\Dictionary" "HanjaIndex" "jinsuk.dic"

  ReadRegDWORD $1 HKCU "${REGISTRY_PATH}" "OptionFlag" 
  IfErrors 0 regDone
  WriteRegDWORD HKCU "${REGISTRY_PATH}" "OptionFlag" 41
  regDone:
  ReadRegDWORD $1 HKCU "${REGISTRY_PATH}" "HangulToggle" 
  IfErrors 0 regDone2
  WriteRegDWORD HKCU "${REGISTRY_PATH}" "HangulToggle" 65568
  regDone2:
  ;WriteRegDWORD HKCU "${REGISTRY_PATH}" "LayoutFlag" "1"
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  ;Write uninstallation info
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "DisplayName" "새나루 한글 입력기"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "UninstallString" "$INSTDIR\Uninstall.exe"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "InstallLocation" "$INSTDIR"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "DisplayIcon" "$INSTDIR\saenaru.ico,-0"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "DisplayVersion" "${RELVERSION}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "Publisher" "Open Hangul Project"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "HelpLink" "https://github.com/wkpark/saenaru"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru" "URLInfoAbout" "https://github.com/wkpark/saenaru"

  ;Create short cuts
  CreateDirectory "${SMPATH}"
  SetOutPath "${SMPATH}"
  File "/oname=새나루 프로젝트.url" "httplink-saenaru.url"
  File "/oname=열린 한글 프로젝트.url" "httplink-hangul.url"
  CreateShortCut "${SMPATH}\새나루 Uninstall.lnk" "$INSTDIR\uninstall.exe" "" "$INSTDIR\uninstall.exe" 0

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
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "Layout File" "kbdcmk.dll"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "Layout Text" "새나루 한글 입력기 (콜맥)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "Layout Display Name" "새나루 한글 입력기 (콜맥)"
  WriteRegStr HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0140412" "IME File" "SAENARU.IME"

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
  File ${SRCROOTDIR}\Saenaru.sln

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
  File ${SRCROOTDIR}\src\*.vcxproj
  File ${SRCROOTDIR}\src\*.vcxproj.*
  File ${SRCROOTDIR}\src\makefile
  File ${SRCROOTDIR}\src\makefile.inc
  File ${SRCROOTDIR}\src\saenaru.def
  File ${SRCROOTDIR}\src\sources
  File ${SRCROOTDIR}\src\*.c
  File ${SRCROOTDIR}\src\*.h
  File ${SRCROOTDIR}\src\*.cpp

  SetOutPath "$INSTDIR\Source\tip"
  File ${SRCROOTDIR}\tip\*.rc
  File ${SRCROOTDIR}\tip\*.vcxproj
  File ${SRCROOTDIR}\tip\*.vcxproj.*
  File ${SRCROOTDIR}\tip\saenarutip.def
  File ${SRCROOTDIR}\tip\*.h
  File ${SRCROOTDIR}\tip\*.cpp

SectionEnd

Section /o "기본 한글입력기로 지정" SecDefault
  ; save current IME
  ReadRegStr $0 HKCU "Keyboard Layout\Preload" "1"
  StrCmp $0 "e0120412" exit # saenaru
  StrCmp $0 "e0130412" exit # saenaru dvorak
  StrCmp $0 "e0140412" exit # saenaru colemak
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

  LangString DESC_IME_DEFAULT_TITLE ${LANG_KOREAN} "IME 입력기 설정"
  LangString DESC_IME_DEFAULT_SUBTITLE ${LANG_KOREAN} "구형 IME 입력기를 기본으로 변경합니다."
  LangString DESC_IME_DEFAULT_HEADER ${LANG_KOREAN} "윈도우10/11에서 IME입력기를 강제로 활성화시킵니다.$\r$\n상당수의 어플에서 구형 IME를 사용하실 수 있습니다.(단, 닷넷 앱은 사용이 불가능합니다)"
  LangString DESC_IME_DEFAULT_BUTTON ${LANG_KOREAN} "새나루 입력기 사용"
  LangString DESC_IME_DVORAK_BUTTON ${LANG_KOREAN} "새나루 입력기(드보락) 사용"
  LangString DESC_IME_COLEMAK_BUTTON ${LANG_KOREAN} "새나루 입력기(콜맥) 사용"

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

;; custom fix
!include /NONFATAL ".\userkbd\extra.nsi"

Var default_ime
Var dvorak_ime
Var colemak_ime
Var ime_dialog
Function OpenEnableIme
	nsDialogs::Create 1018
        Pop $ime_dialog
	!insertmacro MUI_HEADER_TEXT "$(DESC_IME_DEFAULT_TITLE)" "$(DESC_IME_DEFAULT_SUBTITLE)"
	${NSD_CreateLabel} 0 0 100% 103 "$(DESC_IME_DEFAULT_HEADER)"
	${NSD_CreateCheckBox} 0 104 110u 17u "$(DESC_IME_DEFAULT_BUTTON)"
	Pop $default_ime
	${NSD_CreateCheckBox} 120u 104 110u 17u "$(DESC_IME_DVORAK_BUTTON)"
	Pop $dvorak_ime
	${NSD_CreateCheckBox} 0 144 110u 17u "$(DESC_IME_COLEMAK_BUTTON)"
	Pop $colemak_ime

        ${NSD_SetState} $default_ime ${BST_CHECKED}

	nsDialogs::Show
FunctionEnd

Function EnableImeSettings
        ${NSD_GetState} $default_ime $0
        ${If} $0 == 1
	  Exec "RunDll32.exe saenaru.ime,InstallLayout 0,1,1"
        ${Else}
	  Exec "RunDll32.exe saenaru.ime,InstallLayout 0,0,1"
        ${EndIf}

        ${NSD_GetState} $dvorak_ime $0
        ${If} $0 == 1
	  Exec "RunDll32.exe saenaru.ime,InstallLayout 1,1,1"
        ${Else}
	  Exec "RunDll32.exe saenaru.ime,InstallLayout 1,0,1"
        ${EndIf}

        ${NSD_GetState} $colemak_ime $0
        ${If} $0 == 1
	  Exec "RunDll32.exe saenaru.ime,InstallLayout 2,1,1"
        ${Else}
	  Exec "RunDll32.exe saenaru.ime,InstallLayout 2,0,1"
        ${EndIf}
FunctionEnd

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

        ;; custom fix
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

  LangString DESC_SecBody ${LANG_KOREAN} "새나루 입력기를 위한 기본적인 파일을 설치합니다.$\r$\n구형 IME 및 신형 TSF지원 입력기가 함께 설치됩니다."
  LangString DESC_SecPrep ${LANG_KOREAN} "VC런타임 파일을 점검/설치합니다.$\r$\nVC런타임이 설치되어있는지 확인하고 필요한 경우 설치합니다."
  LangString DESC_SecSource ${LANG_KOREAN} "새나루 소스를 설치합니다. 새나루는 모든 소스코드가 공개된 자유 소프트웨어입니다.$\r$\n소스코드는 https://github.com/wkpark/saenaru 사이트에서 직접 받으실 수 있습니다."
  LangString DESC_SecDefault ${LANG_KOREAN} "새나루를 기본 입력기로 지정합니다.$\r$\n로그오프 후에 다시 로그인을 하거나 재부팅을 하셔야 설정이 반영됩니다.$\r$\n$\r$\n(윈도우 10 이상에서는 제대로 반영이 안될 수 있습니다)"
  LangString DESC_SecAdd ${LANG_KOREAN} "새나루를 한글 입력기 목록에 추가합니다.$\r$\n이 경우 입력기 상태바에서 새나루를 선택하실 수 있게 됩니다.$\r$\n$\r$\n(윈도우 10 이상에서는 제대로 반영이 안될 수 있습니다)"
  LangString DESC_SecColemak ${LANG_KOREAN} "새나루 콜맥(Colemak)자판 지원을 위한 콜맥 키보드 레이아웃 드라이버 설치 및 설정을 합니다."

  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecPrep} $(DESC_SecPrep)
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

  UnregDll "$SYSDIR\SaenaruTip.dll"

  ${If} ${RunningX64}
    ${DisableX64FSRedirection}
    Delete /REBOOTOK "$SYSDIR\saenaru.ime"
    Delete /REBOOTOK "$SYSDIR\SaenaruTip.dll"
    Delete /REBOOTOK "$SYSDIR\kbddvk.dll"
    Delete /REBOOTOK "$SYSDIR\kbdcmk.dll"
    ${EnableX64FSRedirection}
  ${EndIf}

  Delete /REBOOTOK "$SYSDIR\saenaru.ime"
  Delete /REBOOTOK "$SYSDIR\SaenaruTip.dll"
  Delete /REBOOTOK "$SYSDIR\kbddvk.dll"
  Delete /REBOOTOK "$SYSDIR\kbdcmk.dll"

  Delete /REBOOTOK "$INSTDIR\saenaru.dic"
  Delete /REBOOTOK "$INSTDIR\nabi.dic"
  Delete /REBOOTOK "$INSTDIR\symwin.dic"
  Delete /REBOOTOK "$INSTDIR\word.dic"
  Delete /REBOOTOK "$INSTDIR\jinsuk.dic"
  Delete "$INSTDIR\2set3set.reg"
  Delete "$INSTDIR\ahnmatae.reg"
  Delete "$INSTDIR\comp_default.reg"
  Delete "$INSTDIR\nk2set.reg"
  Delete "$INSTDIR\old2set3set.reg"
  Delete "$INSTDIR\help\saenaru.chm"

  Delete "$INSTDIR\Source\DIRS"
  Delete "$INSTDIR\Source\LICENSE"
  Delete "$INSTDIR\Source\LICENSE.MICROSOFT"
  Delete "$INSTDIR\Source\Makefile"
  Delete "$INSTDIR\Source\Saenaru.sln"
  Delete "$INSTDIR\Source\doc\saenaru.htm"
  Delete "$INSTDIR\Source\resource\2set3set.reg"
  Delete "$INSTDIR\Source\resource\2set.ico"
  Delete "$INSTDIR\Source\resource\3set.ico"
  Delete "$INSTDIR\Source\resource\ahnmatae.reg"
  Delete "$INSTDIR\Source\resource\ARWDWN.ico"
  Delete "$INSTDIR\Source\resource\ARWUP.ico"
  Delete "$INSTDIR\Source\resource\about.ico"
  Delete "$INSTDIR\Source\resource\button\engkey.ico"
  Delete "$INSTDIR\Source\resource\button\hanjakey.ico"
  Delete "$INSTDIR\Source\resource\button\hankey.ico"
  Delete "$INSTDIR\Source\resource\check.bmp"
  Delete "$INSTDIR\Source\resource\close.bmp"
  Delete "$INSTDIR\Source\resource\comp_default.reg"
  Delete "$INSTDIR\Source\resource\face.bmp"
  Delete "$INSTDIR\Source\resource\full.ico"
  Delete "$INSTDIR\Source\resource\general.ico"
  Delete "$INSTDIR\Source\resource\half.ico"
  Delete "$INSTDIR\Source\resource\keyboard.ico"
  Delete "$INSTDIR\Source\resource\hjcheck.bmp"
  Delete "$INSTDIR\Source\resource\hjmode.bmp"
  Delete "$INSTDIR\Source\resource\imepad.ico"
  Delete "$INSTDIR\Source\resource\nk2set.reg"
  Delete "$INSTDIR\Source\resource\old2set3set.reg"
  Delete "$INSTDIR\Source\resource\penindic.ico"
  Delete "$INSTDIR\Source\resource\saenaru.dic"
  Delete "$INSTDIR\Source\resource\symwin.dic"
  Delete "$INSTDIR\Source\resource\nabi.dic"
  Delete "$INSTDIR\Source\resource\word.dic"
  Delete "$INSTDIR\Source\resource\jinsuk.dic"
  Delete "$INSTDIR\Source\resource\saenaru.dlg"
  Delete "$INSTDIR\Source\resource\saenaru.ico"
  Delete "$INSTDIR\Source\resource\saenaru.rcv"
  Delete "$INSTDIR\Source\resource\status.bmp"
  Delete "$INSTDIR\Source\resource\text\engkey.ico"
  Delete "$INSTDIR\Source\resource\text\han2key.ico"
  Delete "$INSTDIR\Source\resource\text\hanjakey.ico"
  Delete "$INSTDIR\Source\resource\text\hankey.ico"
  Delete "$INSTDIR\Source\resource\uncheck.bmp"
  Delete "$INSTDIR\Source\setup\LICENSE.txt"
  Delete "$INSTDIR\Source\setup\httplink-hangul.url"
  Delete "$INSTDIR\Source\setup\httplink-saenaru.url"
  Delete "$INSTDIR\Source\setup\saenaru.inf"
  Delete "$INSTDIR\Source\setup\saenaru.nsi"
  Delete "$INSTDIR\Source\src\btncmd.cpp"
  Delete "$INSTDIR\Source\src\btnhlp.cpp"
  Delete "$INSTDIR\Source\src\btnpad.cpp"
  Delete "$INSTDIR\Source\src\btnext.cpp"
  Delete "$INSTDIR\Source\src\btnime.cpp"
  Delete "$INSTDIR\Source\src\config.c"
  Delete "$INSTDIR\Source\src\data.c"
  Delete "$INSTDIR\Source\src\dic.c"
  Delete "$INSTDIR\Source\src\dic2.c"
  Delete "$INSTDIR\Source\src\fdebug.c"
  Delete "$INSTDIR\Source\src\debug.h"
  Delete "$INSTDIR\Source\src\hangul.h"
  Delete "$INSTDIR\Source\src\hangul.c"
  Delete "$INSTDIR\Source\src\hansub.c"
  Delete "$INSTDIR\Source\src\hanja.c"
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
  Delete "$INSTDIR\Source\src\regconf.c"
  Delete "$INSTDIR\Source\src\regime.c"
  Delete "$INSTDIR\Source\src\regword.c"
  Delete "$INSTDIR\Source\src\resource.h"
  Delete "$INSTDIR\Source\src\saenaru.c"
  Delete "$INSTDIR\Source\src\saenaru.def"
  Delete "$INSTDIR\Source\src\saenaru.h"
  Delete "$INSTDIR\Source\src\saenaru.rc"
  Delete "$INSTDIR\Source\src\sources"
  Delete "$INSTDIR\Source\src\subs.c"
  Delete "$INSTDIR\Source\src\strsubs.c"
  Delete "$INSTDIR\Source\src\toascii.c"
  Delete "$INSTDIR\Source\src\tsf.cpp"
  Delete "$INSTDIR\Source\src\tsf.h"
  Delete "$INSTDIR\Source\src\ui.c"
  Delete "$INSTDIR\Source\src\uicand.c"
  Delete "$INSTDIR\Source\src\uicomp.c"
  Delete "$INSTDIR\Source\src\uiguide.c"
  Delete "$INSTDIR\Source\src\uistate.c"
  Delete "$INSTDIR\Source\src\vksub.h"
  Delete "$INSTDIR\Source\src\unicode.h"
  Delete "$INSTDIR\Source\src\version.h"
  Delete "$INSTDIR\Source\src\Saenaru.vcxproj"
  Delete "$INSTDIR\Source\src\Saenaru.vcxproj.filters"
  Delete "$INSTDIR\Source\tip\btnime.cpp"
  Delete "$INSTDIR\Source\tip\btnshape.cpp"
  Delete "$INSTDIR\Source\tip\cleanup.cpp"
  Delete "$INSTDIR\Source\tip\compart.cpp"
  Delete "$INSTDIR\Source\tip\compose.cpp"
  Delete "$INSTDIR\Source\tip\config.cpp"
  Delete "$INSTDIR\Source\tip\dap.cpp"
  Delete "$INSTDIR\Source\tip\dllmain.cpp"
  Delete "$INSTDIR\Source\tip\editsink.cpp"
  Delete "$INSTDIR\Source\tip\globals.cpp"
  Delete "$INSTDIR\Source\tip\keys.cpp"
  Delete "$INSTDIR\Source\tip\langbar.cpp"
  Delete "$INSTDIR\Source\tip\precomp.cpp"
  Delete "$INSTDIR\Source\tip\property.cpp"
  Delete "$INSTDIR\Source\tip\pstore.cpp"
  Delete "$INSTDIR\Source\tip\register.cpp"
  Delete "$INSTDIR\Source\tip\saenarutip.cpp"
  Delete "$INSTDIR\Source\tip\server.cpp"
  Delete "$INSTDIR\Source\tip\tmgrsink.cpp"
  Delete "$INSTDIR\Source\tip\btnime.h"
  Delete "$INSTDIR\Source\tip\btnshape.h"
  Delete "$INSTDIR\Source\tip\editsess.h"
  Delete "$INSTDIR\Source\tip\globals.h"
  Delete "$INSTDIR\Source\tip\langbar.h"
  Delete "$INSTDIR\Source\tip\pstore.h"
  Delete "$INSTDIR\Source\tip\resource.h"
  Delete "$INSTDIR\Source\tip\saenarutip.h"
  Delete "$INSTDIR\Source\tip\version.h"
  Delete "$INSTDIR\Source\tip\SaenaruTip.vcxproj"
  Delete "$INSTDIR\Source\tip\SaenaruTip.vcxproj.filters"
  Delete "$INSTDIR\Source\tip\saenarutip.rc"
  Delete "$INSTDIR\Source\tip\saenarutip.def"
  Delete "$INSTDIR\saenaru.ico"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR\Source\src"
  RMDir "$INSTDIR\Source\tip"
  RMDir "$INSTDIR\Source\setup"
  RMDir "$INSTDIR\Source\resource\text"
  RMDir "$INSTDIR\Source\resource\button"
  RMDir "$INSTDIR\Source\resource"
  RMDir "$INSTDIR\Source\doc"
  RMDir "$INSTDIR\Source"
  RMDir "$INSTDIR\help"
  RMDir "$INSTDIR"

  Delete "${SMPATH}\*.*"
  RMDir "${SMPATH}"

  DeleteRegKey HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0120412"
  DeleteRegKey HKLM "System\CurrentControlSet\Control\Keyboard Layouts\E0130412"
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Saenaru"
  DeleteRegKey /ifempty HKLM "${REGISTRY_PATH}"

SectionEnd

; ex: ts=8 sts=2 sw=2 et
