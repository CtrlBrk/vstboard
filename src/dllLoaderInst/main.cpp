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
// #include <algorithm>
#include <windows.h>
#include <string>
// #include <list>

#include "../dllLoader/loaderhelpers.h"

#ifndef __ipluginbase__
#include "pluginterfaces/base/ipluginbase.h"
#endif

#if defined(__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
        #define EXPORT_FACTORY	__attribute__ ((visibility ("default")))
#else
        #define EXPORT_FACTORY
#endif

#define DllExport __declspec( dllexport )



HMODULE HpluginDll = 0;



bool InitModule()
{
	//add install path
	AddDllPath();

	//load requiered dlls
	try {
		LoadRequiredDlls();
	}

	//dlls not found, ask for a path
	catch (FileError &e) {
		MessageBox(NULL, 
            (e.errMsg + L" " + e.filePath + L"\nSome VstBoard files are missing, please select the installation path").c_str(),
			L"VstBoard", 
			MB_OK | MB_ICONERROR);
		
		std::wstring newPath = openfilename();
		if (newPath == L"")
			return false;

		const size_t last_slash_idx = newPath.rfind('\\');
		if (std::wstring::npos != last_slash_idx)
		{
			newPath = newPath.substr(0, last_slash_idx);
		}

		//save path in registry & load dlls
		try {
			RegSetString(HKEY_CURRENT_USER, regBaseKey, installKey, newPath);
			AddDllPath();
			LoadRequiredDlls();
		}
		catch (RegistryError &r) {
			MessageBox(NULL,
				r.errMsg.c_str(),
				L"VstBoard",
				MB_OK | MB_ICONERROR);

			return false;
		}
		catch (FileError &fe) {
			MessageBox(NULL,
				(fe.errMsg + L" " + fe.filePath).c_str(),
				L"VstBoard",
				MB_OK | MB_ICONERROR);

			return false;
		}
		
	}

    //load the plugin dll
    HpluginDll = LoadLibrary( L"VstBoardVst.dat" );
    if (HpluginDll==NULL) {
        auto err = GetLastError();
        MessageBox(NULL, std::to_wstring(err).c_str(), L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

    if (!HpluginDll) {
        MessageBox(NULL, L"VstBoardVst.dat : not loaded", L"VstBoard", MB_OK | MB_ICONERROR);
        return false;
    }

	return true;
}

bool DeinitModule()
{
    //don't unload, avoid vrash maybe related to winmigrate
    // if(HpluginDll) FreeLibrary(HpluginDll);
    // HpluginDll=0;

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
    // if(!InitModule()) {
    //     DeinitModule();
    //     return 0;
    // }

    GetFactoryProc entryPoint = (GetFactoryProc)::GetProcAddress (HpluginDll, "GetPluginFactoryInst");

    if(!entryPoint) {
        MessageBox(NULL,L"VstBoardVst.dat is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
        //DeinitModule();
        return 0;
    }

    return entryPoint();
}
}
#ifdef VST24SDK
#include "pluginterfaces/vst2.x/aeffect.h"
#else
#include "../vestige.h"
#endif

typedef AEffect *(*vstPluginFuncPtr)(audioMasterCallback host);

extern "C" {

    VST_EXPORT AEffect* VSTPluginMain (audioMasterCallback audioMaster)
    {
        if(!InitModule()) {
            return 0;
        }
        vstPluginFuncPtr entryPoint = (vstPluginFuncPtr)GetProcAddress(HpluginDll, "VSTPluginMainInst");

        if(!entryPoint) {
            MessageBox(NULL,L"VstBoardVst.dat is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
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


