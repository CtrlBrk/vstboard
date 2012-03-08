Name "VstBoard"
RequestExecutionLevel admin

!include LogicLib.nsh

OutFile "VstBoard_${VERSION}_${ARCH}.exe"
SetDateSave on
SetDatablockOptimize on
CRCCheck on
XPStyle on
SetCompressor lzma

!define START_LINK_DIR "$SMPROGRAMS\VstBoard${ARCH}"
!define START_LINK_RUN "$SMPROGRAMS\VstBoard${ARCH}\VstBoard ${ARCH}.lnk"
!define START_LINK_UNINSTALLER "$SMPROGRAMS\VstBoard${ARCH}\Uninstall VstBoard ${ARCH}.lnk"
!define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\VstBoard${ARCH}"
!define REG_SETTINGS "Software\CtrlBrk\VstBoard\${ARCH}"
!define UNINSTALLER_NAME "uninst.exe"
!define WEBSITE_LINK "http://vstboard.blogspot.com/"

LicenseText "License Agreeemt. Please review the license terms before installing VstBoard. If you accept the term of the agreement, click I Agree to continue. You must accept the agreement to install VstBoard"
LicenseData "license.txt"

Var InstFolder
Var VstDir
Var Vst3Dir

Page license
Page components

PageEx directory
    PageCallbacks defaultInstDir "" getInstDir
    Caption ": Installation Folder"
PageExEnd

PageEx directory
    PageCallbacks defaultVstDir "" getVstDir
    Caption ": VST Plugins Folder"
PageExEnd

PageEx directory
    PageCallbacks defaultVst3Dir "" getVst3Dir
    Caption ": VST3 Plugins Folder"
PageExEnd

Page instfiles

UninstPage uninstConfirm
UninstPage instfiles

Function defaultInstDir
    StrCpy $INSTDIR "$LOCALAPPDATA\VstBoard${ARCH}\"
FunctionEnd

Function getInstDir
    StrCpy $InstFolder $INSTDIR
FunctionEnd

Function defaultVstDir
    ${If} ${ARCH} == "x64"
        SetRegView 64
    ${Else}
        SetRegView 32
    ${EndIf}

    ReadRegStr $INSTDIR HKLM "SOFTWARE\VST" "VSTPluginsPath"
    ${If} $INSTDIR == ""
        ${If} ${ARCH} == "x64"
            StrCpy $INSTDIR "$PROGRAMFILES64\Vstplugins\"
        ${Else}
            StrCpy $INSTDIR "$PROGRAMFILES\Vstplugins\"
        ${EndIf}
    ${EndIf}
FunctionEnd

Function getVstDir
    StrCpy $VstDir "$INSTDIR\"
    ReadRegStr $1 HKLM "SOFTWARE\VST" "VSTPluginsPath"
FunctionEnd

Function defaultVst3Dir
    ${If} ${ARCH} == "x64"
        StrCpy $INSTDIR "$COMMONFILES64\VST3"
    ${Else}
        StrCpy $INSTDIR "$COMMONFILES\VST3"
    ${EndIf}
FunctionEnd

Function getVst3Dir
    StrCpy $Vst3Dir "$INSTDIR\"
FunctionEnd

Section "VstBoard (required)"
    SetShellVarContext current
    SectionIn RO
    SetOutPath $InstFolder
    File "QtCore4.dll"
    File "QtGui4.dll"
    File "QtScript4.dll"
    File "QtSolutions_MFCMigrationFramework-head.dll"
    File "VstBoard.exe"
    File "VstBoardPlugin.dll"
    File "license.txt"
    File "GPL.txt"
    File "LGPL.txt"

    WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayName" "VstBoard ${ARCH}"
    WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayIcon" "$\"$InstFolder\VstBoard.exe$\""
    WriteRegStr HKCU "${REG_UNINSTALL}" "Publisher" "CtrlBrk"
    WriteRegStr HKCU "${REG_UNINSTALL}" "DisplayVersion" "${VERSION} ${ARCH}"
    WriteRegDWord HKCU "${REG_UNINSTALL}" "EstimatedSize" 15000 ;KB
    WriteRegStr HKCU "${REG_UNINSTALL}" "HelpLink" "${WEBSITE_LINK}"
    WriteRegStr HKCU "${REG_UNINSTALL}" "URLInfoAbout" "${WEBSITE_LINK}"
    WriteRegStr HKCU "${REG_UNINSTALL}" "InstallLocation" "$\"$InstFolder$\""
    WriteRegStr HKCU "${REG_UNINSTALL}" "InstallSource" "$\"$EXEDIR$\""
    WriteRegDWord HKCU "${REG_UNINSTALL}" "NoModify" 1
    WriteRegDWord HKCU "${REG_UNINSTALL}" "NoRepair" 1
    WriteRegStr HKCU "${REG_UNINSTALL}" "UninstallString" "$\"$InstFolder\${UNINSTALLER_NAME}$\""
    WriteRegStr HKCU "${REG_UNINSTALL}" "Comments" "Uninstalls VstBoard ${ARCH}."
    WriteRegStr HKCU "${REG_SETTINGS}" "InstallLocation" "$\"$InstFolder$\""
    WriteUninstaller $InstFolder\uninst.exe

    SetOutPath $VstDir
    WriteRegStr HKCU "${REG_SETTINGS}" "pluginInstallDir" $VstDir
    WriteRegStr HKCU "${REG_SETTINGS}" "lastVstPath" $VstDir
    WriteRegStr HKCU "${REG_SETTINGS}\plugin" "lastVstPath" $VstDir
    File "VstBoardLoader.dll"

    SetOutPath $Vst3Dir
    WriteRegStr HKCU "${REG_SETTINGS}" "pluginVst3InstallDir" $Vst3Dir
    File "VstBoardLoader.vst3"
SectionEnd

Section "Start Menu Shortcuts"
    SetShellVarContext current
    SetOutPath $InstFolder
    CreateDirectory "${START_LINK_DIR}"
    CreateShortCut "${START_LINK_RUN}" "$InstFolder\VstBoard.exe"
    CreateShortCut "${START_LINK_UNINSTALLER}" "$InstFolder\uninst.exe"
SectionEnd

Section "-VcRedist"
    SetOutPath "$TEMP"
    SetOverwrite on
    File vcredist_${ARCH}.exe
    ExecWait '"$TEMP\vcredist_${ARCH}.exe" /passive /showfinalerror'
    Delete "$TEMP\vcredist_${ARCH}.exe"
SectionEnd

Section "Uninstall"
    SetShellVarContext current

    ReadRegStr $VstDir HKCU "${REG_SETTINGS}" "pluginInstallDir"
    Delete "$VstDir\VstBoardLoader.dll"

    ReadRegStr $Vst3Dir HKCU "${REG_SETTINGS}" "pluginVst3InstallDir"
    Delete "$Vst3Dir\VstBoardLoader.vst3"

    DeleteRegKey HKCU "${REG_UNINSTALL}"
    DeleteRegKey HKCU "${REG_SETTINGS}"
    Delete "$INSTDIR\QtCore4.dll"
    Delete "$INSTDIR\QtGui4.dll"
    Delete "$INSTDIR\QtScript4.dll"
    Delete "$INSTDIR\QtSolutions_MFCMigrationFramework-head.dll"
    Delete "$INSTDIR\VstBoard.exe"
    Delete "$INSTDIR\VstBoardPlugin.dll"
    Delete "$INSTDIR\license.txt"
    Delete "$INSTDIR\GPL.txt"
    Delete "$INSTDIR\LGPL.txt"
    Delete "$INSTDIR\uninst.exe"
    RMDir "$INSTDIR"
    Delete "${START_LINK_RUN}"
    Delete "${START_LINK_UNINSTALLER}"
    RMDir "${START_LINK_DIR}"
SectionEnd

