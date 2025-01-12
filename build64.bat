call version.bat

set SRC_PATH=%CD%
set NSIS_PATH="c:\Program Files (x86)\NSIS\makensis.exe"
set BUILD_PATH=..\vstboard.build\Qt_4_8_0_msvc10_64_Release
set JOM_PATH="C:\Qt\Tools\QtCreator\bin\jom\jom.exe"

set QTDIR=c:\Qt\6.8.1\msvc2022_64
set QMAKESPEC=win32-msvc2010
call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /xp /x64 /Release

mkdir "%BUILD_PATH%/installer"
copy /y "*.txt" "%BUILD_PATH%/installer"
copy /y ".\tools\nsis.nsi" "%BUILD_PATH%/installer"

pushd "libs\qtwinmigrate\buildlib"

rem 		moc -o qwinhost.moc qwinhost.h 
rem			moc -o qwinwidget.moc qwinwidget.h
rem 		and copy in release dir

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
rem copy /y "%WindowsSDKDir%\Redist\VC\vcredist_x64.exe" ".\installer"

cd "installer"
%NSIS_PATH% /DARCH=x64 /DVERSION="0.6.0" nsis.nsi

pause
