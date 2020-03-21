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
#pragma warning ( push, 1 )
#include "pluginterfaces/base/ftypes.h"
#pragma warning ( pop )
#include "myvst2wrapper.h"
#include "ids.h"
#include "vst2shell.h"
#include <windows.h>
#include <QMfcApp>

#if defined (_MSC_VER) && defined (DEVELOPMENT)
        #include <crtdbg.h>
#endif

#ifdef UNICODE
#define tstrrchr wcsrchr
#else
#define tstrrchr strrchr
#endif

AudioEffect *createShell(audioMasterCallback audioMaster);

::AudioEffect* createEffectInstance(audioMasterCallback audioMaster)
{
    long id = audioMaster(0,audioMasterCurrentId,0,0,0,0);

    switch(id) {
    case uniqueIDEffect:
        return MyVst2Wrapper::crt (GetPluginFactory (), VstBoardProcessorUID, uniqueIDEffect, audioMaster);
    case uniqueIDInstrument:
        return MyVst2Wrapper::crt (GetPluginFactory (), VstBoardInstProcessorUID, uniqueIDInstrument, audioMaster);
    default:
        return createShell(audioMaster);
    }


}


//------------------------------------------------------------------------
HINSTANCE ghInst = 0;
void* moduleHandle = 0;
Steinberg::tchar gPath[MAX_PATH] = {0};

//------------------------------------------------------------------------
#define DllExport __declspec( dllexport )

//------------------------------------------------------------------------
extern bool InitModule ();		///< must be provided by Plug-in: called when the library is loaded
extern bool DeinitModule ();	///< must be provided by Plug-in: called when the library is unloaded

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
    static bool ownApplication = FALSE;

    if (dwReason == DLL_PROCESS_ATTACH)
    {
        ownApplication = QMfcApp::pluginInstance( 0 );

//    #if defined (_MSC_VER) && defined (DEVELOPMENT)
//        _CrtSetReportMode ( _CRT_WARN, _CRTDBG_MODE_DEBUG );
//        _CrtSetReportMode ( _CRT_ERROR, _CRTDBG_MODE_DEBUG );
//        _CrtSetReportMode ( _CRT_ASSERT, _CRTDBG_MODE_DEBUG );
//        int flag = _CrtSetDbgFlag (_CRTDBG_REPORT_FLAG);
//        _CrtSetDbgFlag (flag | _CRTDBG_LEAK_CHECK_DF);
//    #endif

        moduleHandle = ghInst = hInst;

        // gets the path of the component
        GetModuleFileName (ghInst, gPath, MAX_PATH);
        Steinberg::tchar* bkslash = tstrrchr (gPath, TEXT ('\\'));
        if (bkslash)
            gPath[bkslash - gPath + 1] = 0;
    }

    //there's no way to know if another plugin is using our qapp
//        if ( dwReason == DLL_PROCESS_DETACH && ownApplication ) {
//            if(qApp)
//                delete qApp;
//        }

    return TRUE;
}

/*
#include "vst.h"

extern AudioEffect* createEffectInstance (audioMasterCallback audioMaster, bool asInstrument);

extern "C" {

//#if defined (__GNUC__) && ((__GNUC__ >= 4) || ((__GNUC__ == 3) && (__GNUC_MINOR__ >= 1)))
//#define VST_EXPORT	__attribute__ ((visibility ("default")))
//#else
#define VST_EXPORT _declspec(dllexport)
//#endif

    VST_EXPORT AEffect* VSTEffectMain (audioMasterCallback audioMaster)
    {
        // Get VST Version of the Host
        if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
            return 0;  // old version

        // Create the AudioEffect
        asIntrument=false;
        AudioEffect* effect = createEffectInstance (audioMaster);
        if (!effect)
            return 0;

        // Return the VST AEffect structur
        return effect->getAeffect ();
    }

    VST_EXPORT AEffect* VSTInstrumentMain (audioMasterCallback audioMaster)
    {
        // Get VST Version of the Host
        if (!audioMaster (0, audioMasterVersion, 0, 0, 0, 0))
            return 0;  // old version

        // Create the AudioEffect
        asIntrument=true;
        AudioEffect* effect = createEffectInstance (audioMaster);
        if (!effect)
            return 0;

        // Return the VST AEffect structur
        return effect->getAeffect ();
    }

} // extern "C"

//------------------------------------------------------------------------
#if WIN32
#include <windows.h>
#include <QMfcApp>

extern "C" {
BOOL WINAPI DllMain( HINSTANCE hInst, DWORD dwReason, LPVOID )
{
    static bool ownApplication = FALSE;
     if ( dwReason == DLL_PROCESS_ATTACH )
         ownApplication = QMfcApp::pluginInstance( hInst );
     if ( dwReason == DLL_PROCESS_DETACH && ownApplication ) {
         delete qApp;
     }

    return TRUE;
}
}// extern "C"

#endif
*/
