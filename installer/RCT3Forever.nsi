; RCT3 Forever installer (NSIS 3). build.py makes it: dist\RCT3Forever-<version>-setup.exe
; makensis /DVERSION=0.1 /DOUT=<dir> /DFILES=<dir with RCT3Forever.exe, forever.dll, RCT3Forever.default.ini,
;          README.md, LICENSES.txt> [/DPERUSER] RCT3Forever.nsi
; PERUSER installs for the current user only (into %LOCALAPPDATA%\Programs, no administrator rights): the tests use it.
;
; The installed folder has no RCT3Forever.ini, so RCT3 Forever keeps its settings and logs in %APPDATA%\RCT3 Forever,
; and copies RCT3Forever.default.ini there on the first start. Uninstalling leaves those settings alone.

Unicode true
!include "MUI2.nsh"

!define APP "RCT3 Forever"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\RCT3 Forever"

Name "${APP}"
OutFile "${OUT}\RCT3Forever-${VERSION}-setup.exe"
SetCompressor /SOLID lzma
BrandingText "${APP} ${VERSION}"

!ifdef PERUSER
  RequestExecutionLevel user
  InstallDir "$LOCALAPPDATA\Programs\${APP}"
  InstallDirRegKey HKCU "Software\${APP}" "Install"
  !define ROOT HKCU
!else
  RequestExecutionLevel admin
  InstallDir "$PROGRAMFILES\${APP}"
  InstallDirRegKey HKLM "Software\${APP}" "Install"
  !define ROOT HKLM
!endif

VIProductVersion "${VERSION}.0.0"
VIAddVersionKey "ProductName" "${APP}"
VIAddVersionKey "FileDescription" "${APP} installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "LegalCopyright" "MIT licence"

!define MUI_ABORTWARNING
!define MUI_WELCOMEPAGE_TEXT "This installs ${APP} ${VERSION}, which runs RollerCoaster Tycoon 3 with its rewritten code and the Cheats Menu.$\r$\n$\r$\nYou need RollerCoaster Tycoon 3 Platinum / Gold (with Soaked! and Wild!) installed. The first time ${APP} starts it finds the game in your Steam libraries, or asks you to select its folder.$\r$\n$\r$\nNothing in the game's own folder is changed."
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "${FILES}\LICENSES.txt"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN
!define MUI_FINISHPAGE_RUN_TEXT "Start ${APP} now"
!define MUI_FINISHPAGE_RUN_FUNCTION StartForever
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\README.md"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "Show the readme"
!define MUI_FINISHPAGE_SHOWREADME_NOTCHECKED
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES

!insertmacro MUI_LANGUAGE "English"

; From an installer running as administrator, Explorer starts the game as the user (not as administrator)
Function StartForever
  Exec '"$WINDIR\explorer.exe" "$INSTDIR\RCT3Forever.exe"'
FunctionEnd

Section "${APP}" SecMain
  SectionIn RO
  !ifdef PERUSER
    SetShellVarContext current
  !else
    SetShellVarContext all
  !endif
  SetOutPath "$INSTDIR"
  ; a settings file in the program folder would make it keep its settings there, which isn't writable when installed
  Delete "$INSTDIR\RCT3Forever.ini"
  File "${FILES}\RCT3Forever.exe"
  File "${FILES}\forever.dll"
  File "${FILES}\RCT3Forever.default.ini"
  File "${FILES}\README.md"
  File "${FILES}\LICENSES.txt"
  WriteUninstaller "$INSTDIR\Uninstall.exe"

  CreateDirectory "$SMPROGRAMS\${APP}"
  CreateShortcut "$SMPROGRAMS\${APP}\${APP}.lnk" "$INSTDIR\RCT3Forever.exe"
  CreateShortcut "$SMPROGRAMS\${APP}\${APP} (choose the game folder).lnk" "$INSTDIR\RCT3Forever.exe" "--choose-game"
  CreateShortcut "$SMPROGRAMS\${APP}\Uninstall ${APP}.lnk" "$INSTDIR\Uninstall.exe"

  WriteRegStr ${ROOT} "Software\${APP}" "Install" "$INSTDIR"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "DisplayName" "${APP}"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "DisplayVersion" "${VERSION}"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "Publisher" "RCT3 Forever"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "URLInfoAbout" "https://github.com/K33rn/rct3-forever"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\RCT3Forever.exe"
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
  WriteRegStr ${ROOT} "${UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
  WriteRegDWORD ${ROOT} "${UNINSTALL_KEY}" "NoModify" 1
  WriteRegDWORD ${ROOT} "${UNINSTALL_KEY}" "NoRepair" 1
  WriteRegDWORD ${ROOT} "${UNINSTALL_KEY}" "EstimatedSize" 4096
SectionEnd

Section "Desktop shortcut" SecDesktop
  !ifdef PERUSER
    SetShellVarContext current
  !else
    SetShellVarContext all
  !endif
  CreateShortcut "$DESKTOP\${APP}.lnk" "$INSTDIR\RCT3Forever.exe"
SectionEnd

!insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecMain} "RCT3Forever.exe, forever.dll and Start Menu shortcuts."
  !insertmacro MUI_DESCRIPTION_TEXT ${SecDesktop} "A shortcut on the desktop."
!insertmacro MUI_FUNCTION_DESCRIPTION_END

Section "Uninstall"
  !ifdef PERUSER
    SetShellVarContext current
  !else
    SetShellVarContext all
  !endif
  Delete "$INSTDIR\RCT3Forever.exe"
  Delete "$INSTDIR\forever.dll"
  Delete "$INSTDIR\RCT3Forever.default.ini"
  Delete "$INSTDIR\README.md"
  Delete "$INSTDIR\LICENSES.txt"
  Delete "$INSTDIR\Uninstall.exe"
  RMDir "$INSTDIR"
  Delete "$SMPROGRAMS\${APP}\${APP}.lnk"
  Delete "$SMPROGRAMS\${APP}\${APP} (choose the game folder).lnk"
  Delete "$SMPROGRAMS\${APP}\Uninstall ${APP}.lnk"
  RMDir "$SMPROGRAMS\${APP}"
  Delete "$DESKTOP\${APP}.lnk"
  DeleteRegKey ${ROOT} "${UNINSTALL_KEY}"
  DeleteRegKey ${ROOT} "Software\${APP}"
SectionEnd
