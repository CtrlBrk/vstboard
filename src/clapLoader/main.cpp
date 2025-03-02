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
#include <string>

#include "../dllLoader/loaderhelpers.h"

#include <clap/helpers/plugin.hh>

HMODULE HpluginDll = 0;
#define DllExport __declspec( dllexport )
#define DLLNAME L"ClapBlib.dllclap."
bool InitModule()
{
	//add install path
	AddDllPath();

    HpluginDll=LoadLibrary(DLLNAME);
    if(!HpluginDll) {

	//dlls not found, ask for a path

		MessageBox(NULL, 
            L"Some VstBoard files are missing, please select the installation path",
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

        if(!RegSetString(HKEY_CURRENT_USER, regBaseKey, installKey, newPath)) {
            //can't write registry ?
        }

        AddDllPath();
        HpluginDll=LoadLibrary(DLLNAME);
	}

	return true;
}

bool DeinitModule()
{
	if(HpluginDll) FreeLibrary(HpluginDll);
    HpluginDll=0;
//    FreeLibrary(HwinMigrate);
//    HwinMigrate=0;
//    FreeLibrary(Hgui);
//    Hgui=0;
//    FreeLibrary(Hcore);
//    Hcore=0;
    return true;
}

static const void *get_factory(const char *factory_id)
{
    auto entry = GetProcAddress(HpluginDll, "clap_entry");
    clap_plugin_entry_t * clapentry = (clap_plugin_entry_t *)entry;
    auto factory = clapentry->get_factory(CLAP_PLUGIN_FACTORY_ID);
    return factory;
    /*
    const struct clap_plugin_entry * _pluginEntry = reinterpret_cast<const struct clap_plugin_entry *>(::GetProcAddress (HpluginDll, "get_factory"));
    if (!_pluginEntry) {
        MessageBox(NULL,L"clap is not valid",L"VstBoard", MB_OK | MB_ICONERROR);
        return nullptr;
    }

    const clap_plugin_factory * _pluginFactory = static_cast<const clap_plugin_factory *>(_pluginEntry->get_factory(CLAP_PLUGIN_FACTORY_ID));
    return _pluginFactory;
*/
}



bool clap_init(const char *p) {
    if(!InitModule()) {
        DeinitModule();
        return false;
    }
    return true;
}
void clap_deinit() {
    DeinitModule();
}

extern "C" {

const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
    CLAP_VERSION,
    clap_init,
    clap_deinit,
    get_factory
};

} // extern "C"
