;NSIS Modern User Interface version 1.63
;Basic Example Script
;Written by Joost Verburg

!define MUI_PRODUCT "KeyboardAccess" ;Define your own software name here
!define MUI_VERSION "0.10" ;Define your own software version here

!include "MUI.nsh"

;--------------------------------
;Configuration

  ;General
  OutFile "KeyboardAccess-${MUI_VERSION}.exe"

  ;Folder selection page
  InstallDir "$PROGRAMFILES\${MUI_PRODUCT}"
  
  ;Remember install folder
  InstallDirRegKey HKCU "Software\Gnobal\${MUI_PRODUCT}\InstallFolder" ""

;--------------------------------
;Modern UI Configuration

  !define MUI_LICENSEPAGE
  !define MUI_DIRECTORYPAGE
  
  !define MUI_ABORTWARNING
  
  !define MUI_UNINSTALLER
  !define MUI_UNCONFIRMPAGE
  
;--------------------------------
;Languages
 
  !insertmacro MUI_LANGUAGE "English"
  
;--------------------------------
;Language Strings

  ;Description
  LangString DESC_SecCopyUI ${LANG_ENGLISH} "Copy the KeyboardAccess.dll file to the application folder."

;--------------------------------
;Data
  
  LicenseData "..\LICENSE"

;--------------------------------
;Installer Sections

Section "" SecCopyUI

  SetOutPath "$INSTDIR"
  File "..\bin\KeyboardAccess.dll"
  File "KeyboardAccess.reg"
  File "..\docs\KeyboardAccessDocs.html"
  File "..\CHANGELOG"
  File "..\LICENSE"

  RegDLL "$INSTDIR\KeyboardAccess.dll"
  IfErrors onError

  ExecWait "$\"$WINDIR\regedit.exe$\" /s $\"$INSTDIR\KeyboardAccess.reg$\""
  IfErrors onError

  Delete "$INSTDIR\KeyboardAccess.reg"

  ;Store install folder
  WriteRegStr HKLM "Software\Gnobal\${MUI_PRODUCT}\InstallFolder" "" $INSTDIR
  IfErrors onError
  
  ;Create uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
  IfErrors onError

  ;Create entry in Add/Remove programs
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "DisplayName" "${MUI_PRODUCT} ${MUI_VERSION} (remove only)"
  IfErrors onError

  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}" "UninstallString" "$INSTDIR\Uninstall.exe"
  IfErrors onError

  Goto endNoError

  onError:
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}"
  DeleteRegKey HKLM "Software\Gnobal\${MUI_PRODUCT}"
  UnRegDLL "$INSTDIR\KeyboardAccess.dll"
  RMDir /r "$INSTDIR"
  MessageBox MB_OK "An error occured while trying to install KeyboardAccess. KeyboardAccess must be installed from an administrator account."
  Abort

  endNoError:
SectionEnd

;Display the Finish header
;Insert this macro after the sections if you are not using a finish page
!insertmacro MUI_SECTIONS_FINISHHEADER

;--------------------------------
;Descriptions

!insertmacro MUI_FUNCTIONS_DESCRIPTION_BEGIN
  !insertmacro MUI_DESCRIPTION_TEXT ${SecCopyUI} $(DESC_SecCopyUI)
!insertmacro MUI_FUNCTIONS_DESCRIPTION_END
 
;--------------------------------
;Uninstaller Section

Section "Uninstall"

  Delete "$INSTDIR\CHANGELOG"
  Delete "$INSTDIR\LICENSE"
  Delete "$INSTDIR\KeyboardAccessDocs.html"
  UnRegDLL "$INSTDIR\KeyboardAccess.dll"
  Delete /REBOOTOK "$INSTDIR\KeyboardAccess.dll"
  Delete "$INSTDIR\Uninstall.exe"

  RMDir "$INSTDIR"

  DeleteRegKey HKLM "Software\Gnobal\${MUI_PRODUCT}"

  ;Display the Finish header
  !insertmacro MUI_UNFINISHHEADER

SectionEnd

;--------------------------------
;Functions
Function .onInstSuccess
  MessageBox MB_YESNO "KeyboardAccess will take effect the next time you open a browser window.$\n$\nIf this is your first time using KeyboardAccess, you must read the section$\n$\"How To Use KeyboardAccess$\" in the documentation.$\n$\nWould you like to read the documentation now?" IDNO noDocs
    Exec "explorer $\"file://$INSTDIR\KeyboardAccessDocs.html$\""
  noDocs:
FunctionEnd

Function un.onUninstSuccess
   DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${MUI_PRODUCT}"

   IfRebootFlag showMessage end
   showMessage:
   MessageBox MB_OK "Some files could not be removed. They will be removed during the next reboot. If you are upgrading, reboot first!"
   end:
FunctionEnd
