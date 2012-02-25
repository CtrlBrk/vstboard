/**************************************************************************
#    Copyright 2010-2011 Raphaël François
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
    HKEY  hKey;
    if(::RegOpenKeyEx(HKEY_CURRENT_USER, L"Software\\CtrlBrk\\VstBoard", 0, KEY_QUERY_VALUE, &hKey) != ERROR_SUCCESS) {
        MessageBox(NULL,L"Can't open HKCU\\Software\\CtrlBrk\\VstBoard",L"VstBoard", MB_OK | MB_ICONERROR);
        return 0;
    }
    DWORD dwSize     = 1000;
    DWORD dwDataType = 0;

    BYTE value[1000];
#ifndef QT_NO_DEBUG
    if(::RegQueryValueEx(hKey, L"DebugLocation", 0, &dwDataType, (LPBYTE)value, &dwSize) != ERROR_SUCCESS) {
        ::RegCloseKey(hKey);
        MessageBox(NULL,L"Can't read HKCU\\Software\\CtrlBrk\\VstBoard\\DebugLocation",L"VstBoard", MB_OK | MB_ICONERROR);
        return 0;
    }
#else
    if(::RegQueryValueEx(hKey, L"InstallLocation", 0, &dwDataType, (LPBYTE)value, &dwSize) != ERROR_SUCCESS) {
        ::RegCloseKey(hKey);
        MessageBox(NULL,L"Can't read HKCU\\Software\\CtrlBrk\\VstBoard\\InstallLocation",L"VstBoard", MB_OK | MB_ICONERROR);
        return 0;
    }
#endif
    ::RegCloseKey(hKey);
    std::wstring instDir((TCHAR*)value);
    instDir.erase(
        remove( instDir.begin(), instDir.end(), '\"' ),
        instDir.end()
        );

    if(GetFileAttributes((instDir).c_str()) == 0xffffffff)
    {
      MessageBox(NULL,(L"The path \""+instDir+L"\" defined in HKCU\\Software\\CtrlBrk\\VstBoard\\InstallLocation is not valid").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
      return 0;
    }

    if(GetFileAttributes((instDir+L"\\VstBoardPlugin.dll").c_str()) == 0xffffffff)
    {
      MessageBox(NULL,(instDir+L"\\VstBoardPlugin.dll : file not found").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
      return false;
    }

#ifndef QT_NO_DEBUG
    Hcore = LoadLibrary((instDir+L"\\QtCored4.dll").c_str());
    Hgui = LoadLibrary((instDir+L"\\QtGuid4.dll").c_str());
    #ifdef SCRIPTENGINE
        Hscript = LoadLibrary((instDir+L"\\QtScriptd4.dll").c_str());
    #endif
    HwinMigrate = LoadLibrary((instDir+L"\\QtSolutions_MFCMigrationFramework-headd.dll").c_str());
    Hplugin = LoadLibrary((instDir+L"\\VstBoardPlugin.dll").c_str());
#else
    Hcore = LoadLibrary((instDir+L"\\QtCore4.dll").c_str());
    Hgui = LoadLibrary((instDir+L"\\QtGui4.dll").c_str());
    #ifdef SCRIPTENGINE
        Hscript = LoadLibrary((instDir+L"\\QtScript4.dll").c_str());
    #endif
    HwinMigrate = LoadLibrary((instDir+L"\\QtSolutions_MFCMigrationFramework-head.dll").c_str());
    Hplugin = LoadLibrary((instDir+L"\\VstBoardPlugin.dll").c_str());
#endif

    if(!Hplugin) {
        FreeLibrary(Hplugin);
        FreeLibrary(HwinMigrate);
    #ifdef SCRIPTENGINE
        FreeLibrary(Hscript);
    #endif
        FreeLibrary(Hgui);
        FreeLibrary(Hcore);
        MessageBox(NULL,(L"Error while loading "+instDir+L"\\VstBoardPlugin.dll").c_str(),L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

    return true;
}

bool DeinitModule()
{
    FreeLibrary(Hplugin);
    FreeLibrary(HwinMigrate);
#ifdef SCRIPTENGINE
    FreeLibrary(Hscript);
#endif
    FreeLibrary(Hgui);
    FreeLibrary(Hcore);
    return true;
}

#ifdef __cplusplus
extern "C" {
#endif
bool DllExport InitDll ()
{
    if(Hplugin)
        return true;

    return InitModule();
}
bool DllExport ExitDll ()
{
    return DeinitModule ();
}
#ifdef __cplusplus
} // extern "C"
#endif



namespace Steinberg {
EXPORT_FACTORY IPluginFactory* PLUGIN_API GetPluginFactory ()
{
    if(!Hplugin)
        InitModule();
    if(!Hplugin)
        return 0;

    GetFactoryProc entryPoint = (GetFactoryProc)::GetProcAddress (Hplugin, "GetPluginFactory");

    if(!entryPoint) {
        FreeLibrary(Hplugin);
        FreeLibrary(HwinMigrate);
#ifdef SCRIPTENGINE
        FreeLibrary(Hscript);
#endif
        FreeLibrary(Hgui);
        FreeLibrary(Hcore);
        MessageBox(NULL,L"VstBoardPlugin.dll is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
        return 0;
    }

    FreeLibrary(HwinMigrate);
#ifdef SCRIPTENGINE
    FreeLibrary(Hscript);
#endif
    FreeLibrary(Hgui);
    FreeLibrary(Hcore);

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
        if(InitModule()) {
            DeinitModule();
            return 0;
        }

#ifdef AS_INSTRUMENT
        vstPluginFuncPtr entryPoint = (vstPluginFuncPtr)GetProcAddress(Hplugin, "VSTInstrumentMain");
#else
        vstPluginFuncPtr entryPoint = (vstPluginFuncPtr)GetProcAddress(Hplugin, "VSTEffectMain");
#endif
        if(!entryPoint) {
            DeinitModule();
            MessageBox(NULL,L"\\VstBoardPlugin.dll is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
            return 0;
        }

        FreeLibrary(HwinMigrate);
#ifdef SCRIPTENGINE
        FreeLibrary(Hscript);
#endif
        FreeLibrary(Hgui);
        FreeLibrary(Hcore);

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
    if(dwReason==DLL_PROCESS_DETACH) {
        HMODULE Hplugin = GetModuleHandle(L"VstBoardPlugin");
        if(Hplugin!=NULL)
            FreeLibrary(Hplugin);
    }
    return TRUE;
}
}// extern "C"


