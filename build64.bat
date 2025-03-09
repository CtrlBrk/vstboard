set VERSION="0.6.0"

set SRC_PATH=%CD%
set NSIS_PATH="C:\Program Files (x86)\NSIS\makensis.exe"
set BUILD_PATH=src\build\64bit-Release
set JOM_PATH="C:\Qt\Tools\QtCreator\bin\jom\jom.exe"
set QTDIR=C:\Qt\6.8.2\msvc2022_64
set QMAKESPEC=win32-msvc

rem set PATH=%QTDIR%\bin;%PATH%

rem call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvars64.bat"
rem call "C:\Program Files\Microsoft SDKs\Windows\v7.1\Bin\SetEnv.cmd" /xp /x64 /Release



mkdir "%BUILD_PATH%/installer"
copy /y "*.txt" "%BUILD_PATH%/installer/"
copy /y ".\tools\nsis.nsi" "%BUILD_PATH%/installer/"

rem =====================build qwinmigrate as dll (needs tinkering, need to document the process)
rem 
rem pushd "libs\qtwinmigrate\buildlib"
rem 	some moc files are not generated for some reasons :
rem 		moc -o qwinhost.moc qwinhost.h 
rem			moc -o qwinwidget.moc qwinwidget.h
rem 		and copy in release dir
rem %QTDIR%\bin\qmake -r buildlib.pro
rem %JOM_PATH% clean
rem %JOM_PATH% release
rem popd

rem =================== build loaddll32 with msvc
msbuild loaddll32\loaddll32.sln /p:Configuration=Release /property:Platform=x86

rem ================== buildall
pushd "%BUILD_PATH%/"
%QTDIR%\bin\qmake -makefile %SRC_PATH%\src\buildall.pro

rem %JOM_PATH% clean
%JOM_PATH% release


pause

copy /y "loaddll32.exe" ".\installer\" 
copy /y "VstBlib.dll" ".\installer\"

copy /y "VstBoardInst.dll" ".\installer\"
copy /y "VstBoard.dll" ".\installer\"
copy /y "VstBoard.dll" ".\installer\VstBoard.vst3"

copy /y "VstBoardClap.clap" ".\installer\VstBoard.clap"
copy /y "ClapBlib.dllclap" ".\installer\"

copy /y "VstBoardHost.exe" ".\installer\VstBoard.exe"


copy /y "%QTDIR%\bin\Qt6Core.dll" ".\installer"
copy /y "%QTDIR%\bin\Qt6Gui.dll" ".\installer"
copy /y "%QTDIR%\bin\Qt6Svg.dll" ".\installer"
copy /y "%QTDIR%\bin\Qt6Widgets.dll" ".\installer"
copy /y "..\..\..\libs\qtwinmigrate\lib\QtSolutions_MFCMigrationFramework-head.dll" ".\installer"

copy /y "%QTDIR%\plugins\iconengines\qsvgicon.dll" ".\installer"
copy /y "%QTDIR%\plugins\imageformats\qsvg.dll" ".\installer"
copy /y "%QTDIR%\plugins\platforms\qdirect2d.dll" ".\installer"
copy /y "%QTDIR%\plugins\platforms\qminimal.dll" ".\installer"
copy /y "%QTDIR%\plugins\platforms\qoffscreen.dll" ".\installer"
copy /y "%QTDIR%\plugins\platforms\qwindows.dll" ".\installer"
copy /y "%QTDIR%\plugins\styles\qmodernwindowsstyle.dll" ".\installer"

cd "installer"
%NSIS_PATH% /DARCH=x64 /DVERSION="%VERSION%" nsis.nsi

pause
popd