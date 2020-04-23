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

#include <windows.h>
//#include <QMfcApp>

#pragma warning ( push, 1 )
#include "pluginterfaces/base/ftypes.h"
#include "public.sdk/source/vst/vst2wrapper/vst2wrapper.h"
#pragma warning ( pop )

#include "ids.h"
#include "vst2shell.h"
#include "loaderhelpers.h"


using namespace Steinberg;

#define DllExport __declspec( dllexport )
extern bool InitModule();
extern bool DeinitModule();

//the vst2.4 factory creates kFx & kInstrument classes, vst3 creates kFxInstrument
extern IPluginFactory* PLUGIN_API GetPluginFactoryVst24();



//#if defined (_MSC_VER) && defined (DEVELOPMENT)
//	#define _CRTDBG_MAP_ALLOC
//	#include <crtdbg.h>
//#endif

//
//#ifdef UNICODE
//#define tstrrchr wcsrchr
//#else
//#define tstrrchr strrchr
//#endif

const std::wstring GetCurrentDllPath(HINSTANCE hInst)
{
	WCHAR buffer[MAX_PATH];
	
	if (GetModuleFileName(hInst, buffer, sizeof(buffer)) == 0)
		return L"";

	std::wstring path(buffer);
	const size_t last_slash_idx = path.rfind('\\');
	if (std::wstring::npos != last_slash_idx)
		path = path.substr(0, last_slash_idx);

	return path;
}



AudioEffect *createShell(audioMasterCallback audioMaster);

::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    long id = audioMaster(0,audioMasterCurrentId,0,0,0,0);

    switch(id) {
    case uniqueIDEffect:
        return Vst::Vst2Wrapper::create(GetPluginFactoryVst24(), VstBoardProcessorUID, uniqueIDEffect, audioMaster);
    case uniqueIDInstrument:
        return Vst::Vst2Wrapper::create(GetPluginFactoryVst24(), VstBoardInstProcessorUID, uniqueIDInstrument, audioMaster);
    default:
        return createShell(audioMaster);
    }
}

QApplication* createQapp() {
	if (qApp)
		return FALSE;

	int argc = 0;
	return new QApplication(argc, 0);
}

#ifdef __cplusplus
extern "C" {
#endif

    bool DllExport InitDll () ///< must be called from host right after loading dll
    {
            return InitModule ();
    }
    bool DllExport ExitDll ()  ///< must be called from host right before unloading dll
    {
            return DeinitModule ();
    }
#ifdef __cplusplus
} // extern "C"
#endif

//------------------------------------------------------------------------
BOOL WINAPI DllMain (HINSTANCE hInst, DWORD dwReason, LPVOID /*lpvReserved*/)
{
    if ( dwReason == DLL_PROCESS_ATTACH)
    {
		//custom Qt plugin path : Qt will not find the "platforms" directory in the host's dir
		std::wstring libPath = TestInstallPath(GetCurrentDllPath(hInst));
		if (libPath == L"") {
			MessageBox(NULL, L"Qt dll not found", L"VstBoard", MB_OK | MB_ICONERROR);
			return FALSE;
		}

		QString p = QString("%1").arg(libPath);
		QCoreApplication::addLibraryPath(p);

		createQapp();

	//	QtAppCreated = QMfcApp::pluginInstance( 0 );

    //#if defined (_MSC_VER) && defined (DEVELOPMENT)
    //    _CrtSetReportMode ( _CRT_WARN, _CRTDBG_MODE_DEBUG );
    //    _CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
    //    _CrtSetReportMode ( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
    //    int flag = _CrtSetDbgFlag (_CRTDBG_REPORT_FLAG);
    //    _CrtSetDbgFlag (flag | _CRTDBG_LEAK_CHECK_DF);
    //#endif

    }
	
	//do not delete the app
	//there's no way to know if another plugin is using our qapp
	/*if (dwReason == DLL_PROCESS_DETACH) {
		if(QtAppCreated && qApp) {
		        qApp->exit(0);
		}
	}*/
    return TRUE;
}
