!include MUI2.nsh

Name "Image Editor"
OutFile "..\ImageEditorInstaller.exe"
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
    SectionIn RO
    SetOutPath $INSTDIR

    File /r "..\build\release\*.*"
SectionEnd


Section "Create Shortcuts" SEC01
    CreateShortcut "$DESKTOP\Image Editor.lnk" "$INSTDIR\launch.vbs" "$INSTDIR\launch.vbs" "$INSTDIR\icon.ico"
SectionEnd