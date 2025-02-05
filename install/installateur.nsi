!include MUI2.nsh
!include "nsis_variables.nsh"

Name "Image Editor"
; OutFile "..\ImageEditorInstaller.exe"
OutFile "../${APP_NAME}-${APP_VERSION}.exe"
InstallDir "$PROGRAMFILES\Open_Image_Editor"

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

    File /r "..\build\release\*.*"

    ; Create uninstaller
    WriteUninstaller "$INSTDIR\uninstall.exe"
    
    ; Add registry keys
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "DisplayName" "Image Editor"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "InstallLocation" "$INSTDIR"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "DisplayIcon" "$INSTDIR\ImageEditor.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "Publisher" "Open Image Editor Team (i am alone ;)"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "DisplayVersion" "1.0"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor" "NoRepair" 1
SectionEnd


Section "Create Shortcuts" SEC01
    CreateShortcut "$DESKTOP\Image Editor.lnk" "$INSTDIR\launch.vbs" "$INSTDIR\launch.vbs" "$INSTDIR\icon.ico"
SectionEnd

Section "Uninstall"
    Delete "$INSTDIR\*.*"
    RMDir /r "$INSTDIR"
    
    ; Remove registry keys
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\Open_Image_Editor"
    
    ; Remove uninstaller shortcut
    Delete "$SMPROGRAMS\Open Image Editor\Uninstall.lnk"

    ; Remove shortcuts
    Delete "$DESKTOP\Image Editor.lnk"

    ; Remove user-specific directory
    RMDir /r "$APPDATA\.OpenImageEditor"
SectionEnd