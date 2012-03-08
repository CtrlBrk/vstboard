call version.bat

set SRC_PATH=%CD%
set NSIS_PATH="c:\Program Files (x86)\NSIS\makensis.exe"
set BUILD_PATH=..\vstboard.build\Qt_4_8_0_msvc10_32_Release
set JOM_PATH="E:\Qt\qtcreator-2.4.0\bin\jom"

set QTDIR=e:\Qt\4.8.0_msvc10_32
set QMAKESPEC=win32-msvc2010
call "C:\Program Files (x86)\Microsoft Visual Studio 10.0\VC\vcvarsall.bat"
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /xp /x86 /Release

mkdir "%BUILD_PATH%/installer"
copy /y "*.txt" "%BUILD_PATH%/installer"
copy /y ".\tools\nsis.nsi" "%BUILD_PATH%/installer"

pushd "libs\qtwinmigrate\buildlib"
%QTDIR%\bin\qmake -r buildlib.pro
%JOM_PATH% clean
%JOM_PATH% release
popd

cd "%BUILD_PATH%"
%QTDIR%\bin\qmake -r %SRC_PATH%\src\buildall.pro
%JOM_PATH% clean
%JOM_PATH% release

pause

copy /y ".\dllLoader\release\VstBoardLoader.dll" ".\installer\VstBoardLoader.dll"
copy /y ".\dllLoader\release\VstBoardLoader.dll" ".\installer\VstBoardLoader.vst3"
copy /y ".\vstboard\release\vstboard.exe" ".\installer"
copy /y ".\vstdll\release\VstBoardPlugin.dll" ".\installer"
copy /y "%QTDIR%\bin\QtCore4.dll" ".\installer"
copy /y "%QTDIR%\bin\QtGui4.dll" ".\installer"
copy /y "%QTDIR%\bin\QtScript4.dll" ".\installer"
copy /y "%QTDIR%\bin\QtSolutions_MFCMigrationFramework-head.dll" ".\installer"
rem copy /y "%WindowsSDKDir%\Redist\VC\vcredist_x86.exe" ".\installer"

cd "installer"
%NSIS_PATH% /DARCH=x86 /DVERSION="%APP_VERSION_MAJOR%.%APP_VERSION_MINOR%.%APP_VERSION_BUILD%" nsis.nsi

pause
