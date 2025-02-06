!include MUI2.nsh
!include "nsis_variables.nsh"

Name "${LNK_APP_NAME}-${APP_VERSION}"
OutFile "../${INSTALLER_APP_NAME}-${APP_VERSION}.exe"

InstallDir "$PROGRAMFILES\${APP_NAME}"

LicenseText "Please read the following license agreement."
LicenseData "..\build\release\LICENSE"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "..\build\release\LICENSE"
!insertmacro MUI_PAGE_COMPONENTS
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES

; TODO Changer pour launch.vbs mais ca marche pas
!define MUI_FINISHPAGE_RUN "$INSTDIR\start.bat"
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"


Section "Installer" SEC00
    DetailPrint "Installing ${APP_NAME} version ${APP_VERSION}"
    SectionIn RO
    SetOutPath $INSTDIR

    RMDir /r $INSTDIR

    File /r "..\build\release\*.*"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
    
    ; Add registry keys
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "DisplayName" "${LNK_APP_NAME}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "DisplayIcon" "$INSTDIR\icon.ico"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "Publisher" "Open Image Editor Team (i am alone ;)"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "DisplayVersion" "${APP_VERSION}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}" "NoRepair" 1
SectionEnd


Section "Create Shortcuts" SEC01
    CreateShortcut "$DESKTOP\${LNK_APP_NAME}.lnk" "$INSTDIR\launch.vbs" "$INSTDIR\launch.vbs" "$INSTDIR\icon.ico"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\*.*"
    RMDir /r "$INSTDIR"
    
    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${LNK_APP_NAME}"
    
    ; Remove uninstaller shortcut
    Delete "$SMPROGRAMS\${APP_NAME}\Uninstall.lnk"

    ; Remove shortcuts
    Delete "$DESKTOP\${LNK_APP_NAME}.lnk"

    ; Remove user-specific directory
    RMDir /r "$APPDATA\.${APP_NAME}"
SectionEnd