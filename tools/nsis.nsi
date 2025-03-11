Name "VstBoard"
RequestExecutionLevel admin

!include LogicLib.nsh

OutFile "VstBoard_${VERSION}.exe"
SetDateSave on
SetDatablockOptimize on
CRCCheck on
XPStyle on
SetCompressor lzma

!define START_LINK_DIR "$SMPROGRAMS\VstBoard"
!define START_LINK_RUN "$SMPROGRAMS\VstBoard\VstBoard.lnk"
!define START_LINK_UNINSTALLER "$SMPROGRAMS\VstBoard\Uninstall VstBoard.lnk"
!define REG_UNINSTALL "Software\Microsoft\Windows\CurrentVersion\Uninstall\VstBoard"
!define REG_SETTINGS "Software\CtrlBrk\VstBoard"
!define UNINSTALLER_NAME "uninstall.exe"
!define WEBSITE_LINK "http://vstboard.blogspot.com/"

LicenseText "License Agreeemt. Please review the license terms before installing VstBoard. If you accept the term of the agreement, click I Agree to continue. You must accept the agreement to install VstBoard"
LicenseData "license.txt"

Var InstFolder
Var VstDir
Var Vst3Dir
Var ClapDir

Page license
Page components

PageEx directory
    PageCallbacks defaultInstDir "" getInstDir
    Caption ": Installation Folder"
PageExEnd

PageEx directory
    PageCallbacks defaultClapDir "" getClapDir
    Caption ": CLAP 64bits Plugins Folder"
PageExEnd

PageEx directory
    PageCallbacks defaultVstDir "" getVstDir
    Caption ": VST2.4 64bits Plugins Folder"
PageExEnd

PageEx directory
    PageCallbacks defaultVst3Dir "" getVst3Dir
    Caption ": VST3 64bits Plugins Folder"
PageExEnd

Page instfiles

UninstPage uninstConfirm
UninstPage instfiles


Function defaultInstDir
    StrCpy $INSTDIR "$PROGRAMFILES64\VstBoard"
FunctionEnd

Function getInstDir
    StrCpy $InstFolder $INSTDIR
FunctionEnd


Function defaultClapDir
	StrCpy $INSTDIR "$COMMONFILES64\CLAP"
FunctionEnd

Function getClapDir
    StrCpy $ClapDir $INSTDIR
FunctionEnd


Function defaultVstDir
    SetRegView 64

    ReadRegStr $INSTDIR HKLM "SOFTWARE\VST" "VSTPluginsPath"
    ${If} $INSTDIR == ""
		StrCpy $INSTDIR "$PROGRAMFILES64\Vstplugins"
    ${EndIf}
FunctionEnd

Function getVstDir
	SetRegView 64
	
    StrCpy $VstDir "$INSTDIR\"
    ReadRegStr $1 HKLM "SOFTWARE\VST" "VSTPluginsPath"
FunctionEnd


Function defaultVst3Dir
	StrCpy $INSTDIR "$COMMONFILES64\VST3"
FunctionEnd

Function getVst3Dir
    StrCpy $Vst3Dir "$INSTDIR\"
FunctionEnd


Section "VstBoard (required)"
	SetRegView 64
	
    SetShellVarContext all
    SectionIn RO
    SetOutPath $InstFolder
		
	File "VstBoard.exe"
	File "VstBoardHost.exe"
    File "loaddll32.exe"
	File "VstBoardVst.dat"
	File "VstBoardClap.dat"
		
	File "license.txt"
    File "GPL.txt"
    File "LGPL.txt"
	
	SetOutPath "$InstFolder\Qt"
    File "Qt6Core.dll"
    File "Qt6Gui.dll"
    File "Qt6Svg.dll"
	File "Qt6Widgets.dll"
    File "QtSolutions_MFCMigrationFramework-head.dll"
	
	SetOutPath "$InstFolder\plugins\iconengines"
	File "qsvgicon.dll"
	SetOutPath "$InstFolder\plugins\imageformats"
	File "qsvg.dll"
	SetOutPath "$InstFolder\plugins\platforms"
	File "qdirect2d.dll"
	File "qminimal.dll"
	File "qoffscreen.dll"
	File "qwindows.dll"
	SetOutPath "$InstFolder\plugins\styles"
	File "qmodernwindowsstyle.dll"
	
   
	SetOutPath $ClapDir
	WriteRegStr HKLM "${REG_SETTINGS}" "clapInstallDir" $ClapDir
	File "VstBoard.clap"

    SetOutPath $VstDir
    WriteRegStr HKLM "${REG_SETTINGS}" "pluginInstallDir" $VstDir
    WriteRegStr HKCU "${REG_SETTINGS}" "lastVstPath" $VstDir
    WriteRegStr HKCU "${REG_SETTINGS}\plugin" "lastVstPath" $VstDir
	File "VstBoard.dll"
	File "VstBoardInst.dll"

    SetOutPath $Vst3Dir
    WriteRegStr HKLM "${REG_SETTINGS}" "pluginVst3InstallDir" $Vst3Dir
    File "VstBoard.vst3"

    WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayName" "VstBoard"
    WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayIcon" "$\"$InstFolder\VstBoard.exe$\""
    WriteRegStr HKLM "${REG_UNINSTALL}" "Publisher" "CtrlBrk"
    WriteRegStr HKLM "${REG_UNINSTALL}" "DisplayVersion" "${VERSION}"
    WriteRegDWord HKLM "${REG_UNINSTALL}" "EstimatedSize" 30000 ;KB
    WriteRegStr HKLM "${REG_UNINSTALL}" "HelpLink" "${WEBSITE_LINK}"
    WriteRegStr HKLM "${REG_UNINSTALL}" "URLInfoAbout" "${WEBSITE_LINK}"
    WriteRegStr HKLM "${REG_UNINSTALL}" "InstallLocation" "$\"$InstFolder$\""
    WriteRegStr HKLM "${REG_UNINSTALL}" "InstallSource" "$\"$EXEDIR$\""
    WriteRegDWord HKLM "${REG_UNINSTALL}" "NoModify" 1
    WriteRegDWord HKLM "${REG_UNINSTALL}" "NoRepair" 1
    WriteRegStr HKLM "${REG_UNINSTALL}" "UninstallString" "$\"$InstFolder\${UNINSTALLER_NAME}$\""
    WriteRegStr HKLM "${REG_UNINSTALL}" "Comments" "Uninstalls VstBoard"
	
	WriteRegStr HKLM "${REG_SETTINGS}" "Location" $InstFolder
	
    WriteUninstaller $InstFolder\uninstall.exe

SectionEnd

Section "Start Menu Shortcuts"
    SetShellVarContext all
    SetOutPath $InstFolder
    CreateDirectory "${START_LINK_DIR}"
    CreateShortCut "${START_LINK_RUN}" "$InstFolder\VstBoard.exe"
    CreateShortCut "${START_LINK_UNINSTALLER}" "$InstFolder\uninstall.exe"
SectionEnd

Section "Uninstall"
    SetShellVarContext all
	SetRegView 64

    ReadRegStr $VstDir HKLM "${REG_SETTINGS}" "pluginInstallDir"
    Delete "$VstDir\VstBoard.dll"
	Delete "$VstDir\VstBoardInst.dll"

    ReadRegStr $Vst3Dir HKLM "${REG_SETTINGS}" "pluginVst3InstallDir"
    Delete "$Vst3Dir\VstBoard.vst3"

	ReadRegStr $ClapDir HKLM "${REG_SETTINGS}" "clapInstallDir"
    Delete "$ClapDir\VstBoard.clap"
	
	Delete "$INSTDIR\VstBoard.exe"
	Delete "$INSTDIR\VstBoardHost.exe"
    Delete "$INSTDIR\loaddll32.exe"
	Delete "$INSTDIR\VstBoardVst.dat"
	Delete "$INSTDIR\VstBoardClap.dat"
		
	Delete "$INSTDIR\license.txt"
    Delete "$INSTDIR\GPL.txt"
    Delete "$INSTDIR\LGPL.txt"
		
	
	Delete "$INSTDIR\Qt\Qt6Core.dll"
    Delete "$INSTDIR\Qt\Qt6Gui.dll"
    Delete "$INSTDIR\Qt\Qt6Svg.dll"
	Delete "$INSTDIR\Qt\Qt6Widgets.dll"
    Delete "$INSTDIR\Qt\QtSolutions_MFCMigrationFramework-head.dll"

	RMDir "$INSTDIR\Qt"	
	
	Delete "$INSTDIR\plugins\iconengines\qsvgicon.dll"
	RMDir "$INSTDIR\plugins\iconengines"
	
	Delete "$INSTDIR\plugins\imageformats\qsvg.dll"
	RMDir "$INSTDIR\plugins\imageformats"
	
	Delete "$INSTDIR\plugins\platforms\qdirect2d.dll"
	Delete "$INSTDIR\plugins\platforms\qminimal.dll"
	Delete "$INSTDIR\plugins\platforms\qoffscreen.dll"
	Delete "$INSTDIR\plugins\platforms\qwindows.dll"
	RMDir "$INSTDIR\plugins\platforms"
	
	Delete "$INSTDIR\plugins\styles\qmodernwindowsstyle.dll"
	RMDir "$INSTDIR\plugins\styles"
	
	RMDir "$INSTDIR\plugins"
	
    
    Delete "${START_LINK_RUN}"
    Delete "${START_LINK_UNINSTALLER}"
    RMDir "${START_LINK_DIR}"
		
    
    Delete "$INSTDIR\uninstall.exe"
	RMDir "$INSTDIR"
	
	DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\VstBoard"
    DeleteRegKey HKLM "${REG_SETTINGS}"

SectionEnd

