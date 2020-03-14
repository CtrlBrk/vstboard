/**************************************************************************
#    Copyright 2010-2020 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/
#include <algorithm>
#include <windows.h>
#include <string>

#ifndef __ipluginbase__
#include "pluginterfaces/base/ipluginbase.h"
#endif

#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
        #define EXPORT_FACTORY	__attribute__ ((visibility ("default")))
#else
        #define EXPORT_FACTORY
#endif

#define DllExport __declspec( dllexport )

HMODULE Hcore=0;
HMODULE Hgui=0;
HMODULE Hscript=0;
HMODULE HwinMigrate=0;
HMODULE Hplugin=0;

bool InitModule()
{
#if defined(_M_X64) || defined(__amd64__)
    std::wstring regBaseKey(L"Software\\CtrlBrk\\VstBoard\\x64");
#else
    std::wstring regBaseKey(L"Software\\CtrlBrk\\VstBoard\\x86");
#endif

    HKEY  hKey;
    if(::RegOpenKeyEx(HKEY_CURRENT_USER, regBaseKey.c_str(), 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        MessageBox(NULL,(L"Registry key not found : HKCU\\"+regBaseKey).c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

#ifndef QT_NO_DEBUG
    std::wstring installKey(L"DebugLocation");
#else
    std::wstring installKey(L"InstallLocation");
#endif

    DWORD dwSize     = 1000;
    DWORD dwDataType = 0;
    BYTE value[1000];
    if(::RegQueryValueEx(hKey, installKey.c_str(), 0, &dwDataType, (LPBYTE)value, &dwSize) != ERROR_SUCCESS) {
        ::RegCloseKey(hKey);
        MessageBox(NULL,(L"Registry key not found : HKCU\\"+regBaseKey+L"\\"+installKey).c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

    ::RegCloseKey(hKey);
    std::wstring instDir((TCHAR*)value);
    instDir.erase( remove( instDir.begin(), instDir.end(), '\"' ), instDir.end() );

    if(GetFileAttributes((instDir).c_str()) == 0xffffffff) {
        MessageBox(NULL,(L"The path \""+instDir+L"\" defined in \"HKCU\\"+regBaseKey+L"\\"+installKey+L"\" is not valid").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

    if(GetFileAttributes((instDir+L"\\VstBoardPlugin.dll").c_str()) == 0xffffffff) {
      MessageBox(NULL,(instDir+L"\\VstBoardPlugin.dll : file not found").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
      return false;
    }

#ifndef QT_NO_DEBUG
    std::wstring dbgSuffix(L"d");
#else
    std::wstring dbgSuffix(L"");
#endif

    if(!Hcore)
        Hcore = LoadLibrary((instDir+L"\\QtCore"+dbgSuffix+L"4.dll").c_str());
    if(!Hcore) {
        MessageBox(NULL,(instDir+L"\\QtCore"+dbgSuffix+L"4.dll : not loaded").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }
    if(!Hgui)
    Hgui = LoadLibrary((instDir+L"\\QtGui"+dbgSuffix+L"4.dll").c_str());
    if(!Hgui) {
        MessageBox(NULL,(instDir+L"\\QtGui"+dbgSuffix+L"4.dll : not loaded").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }
    #ifdef SCRIPTENGINE
        if(!Hscript)
        Hscript = LoadLibrary((instDir+L"\\QtScript"+dbgSuffix+L"4.dll").c_str());
        if(!Hscript) {
            MessageBox(NULL,(instDir+L"\\QtScript"+dbgSuffix+L"4.dll : not loaded").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
            return false;
        }
    #endif
    if(!HwinMigrate)
    HwinMigrate = LoadLibrary((instDir+L"\\QtSolutions_MFCMigrationFramework-head"+dbgSuffix+L".dll").c_str());
    if(!HwinMigrate) {
        MessageBox(NULL,(instDir+L"QtSolutions_MFCMigrationFramework-head"+dbgSuffix+L".dll : not loaded").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

    if(!Hplugin)
    Hplugin = LoadLibrary((instDir+L"\\VstBoardPlugin.dll").c_str());
    if(!Hplugin) {
        MessageBox(NULL,(instDir+L"\\VstBoardPlugin.dll : not loaded").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }
    return true;
}

bool DeinitModule()
{
//    FreeLibrary(Hplugin);
//    Hplugin=0;
//    FreeLibrary(HwinMigrate);
//    HwinMigrate=0;
//#ifdef SCRIPTENGINE
//    FreeLibrary(Hscript);
//    Hscript=0;
//#endif
//    FreeLibrary(Hgui);
//    Hgui=0;
//    FreeLibrary(Hcore);
//    Hcore=0;
    return true;
}

#ifdef __cplusplus
extern "C" {
#endif
bool DllExport InitDll ()
{
    if(!InitModule()) {
        DeinitModule();
        return false;
    }
    return true;
}
bool DllExport ExitDll ()
{
    DeinitModule ();
    return true;
}
#ifdef __cplusplus
} // extern "C"
#endif



namespace Steinberg {
EXPORT_FACTORY IPluginFactory* PLUGIN_API GetPluginFactory ()
{
    if(!InitModule()) {
        DeinitModule();
        return 0;
    }

    GetFactoryProc entryPoint = (GetFactoryProc)::GetProcAddress (Hplugin, "GetPluginFactory");

    if(!entryPoint) {
        MessageBox(NULL,L"VstBoardPlugin.dll is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
        DeinitModule();
        return 0;
    }

    return entryPoint();
}
}

#include "pluginterfaces/vst2.x/aeffect.h"

typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback host);

extern "C" {

//#if defined (__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
//#define VST_EXPORT	__attribute__ ((visibility ("default")))
//#else
#define VST_EXPORT _declspec(dllexport)
//#endif

    VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
    {
        if(!InitModule()) {
            return 0;
        }
        vstPluginFuncPtr entryPoint = (vstPluginFuncPtr)GetProcAddress(Hplugin, "VSTPluginMain");
        if(!entryPoint) {
            MessageBox(NULL,L"VstBoardPlugin.dll is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
            return 0;
        }
        return entryPoint(audioMaster);
    }

    // support for old hosts not looking for VSTPluginMain
#if (TARGET_API_MAC_CARBON && __ppc__)
    VST_EXPORT AEffect* main_macho (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif WIN32
    VST_EXPORT AEffect* MAIN (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#elif BEOS
    VST_EXPORT AEffect* main_plugin (audioMasterCallback audioMaster) { return VSTPluginMain (audioMaster); }
#endif

} // extern "C"

//------------------------------------------------------------------------

extern "C" {

BOOL WINAPI DllMain( HINSTANCE , DWORD dwReason, LPVOID )
{
    if ( dwReason == DLL_PROCESS_DETACH) {
        DeinitModule();
    }
    return TRUE;
}
}// extern "C"


