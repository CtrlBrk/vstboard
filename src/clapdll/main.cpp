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

#include "clapmainhost.h"
// #include <iostream>

#ifdef WIN32
    #include <windows.h>
#endif

#define DllExport __declspec( dllexport )

QApplication* createQapp() {
    if (qApp)
        return 0;

    int argc = 0;
    return new QApplication(argc, 0);
}

#ifdef WIN32


BOOL WINAPI DllMain (HINSTANCE /*hInst*/, DWORD dwReason, LPVOID /*lpvReserved*/)
{
    if ( dwReason == DLL_PROCESS_ATTACH)
    {
        createQapp();
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
#endif



namespace sst::clap_board::pluginentry
{

uint32_t clap_get_plugin_count(const clap_plugin_factory *f) { return 2; }
const clap_plugin_descriptor *clap_get_plugin_descriptor(const clap_plugin_factory *f, uint32_t w)
{
    if(w==1)
        return &ClapMainHost::descInst;

    return &ClapMainHost::desc;
}

static const clap_plugin *clap_create_plugin(const clap_plugin_factory *f, const clap_host *host, const char *plugin_id)
{
    if (!strcmp(plugin_id, ClapMainHost::desc.id))
    {
        // I know it looks like a leak right? but the clap-plugin-helpers basically
        // take ownership and destroy the wrapper when the host destroys the
        // underlying plugin (look at Plugin<h, l>::clapDestroy if you don't believe me!)

        auto p = new ClapMainHost(host, ClapMainHost::desc);
        return p->clapPlugin();
    }

    if (!strcmp(plugin_id, ClapMainHost::descInst.id))
    {
        auto p = new ClapMainHost(host, ClapMainHost::descInst);
        return p->clapPlugin();
    }

    return nullptr;
}

const struct clap_plugin_factory clap_board_factory = {
    sst::clap_board::pluginentry::clap_get_plugin_count,
    sst::clap_board::pluginentry::clap_get_plugin_descriptor,
    sst::clap_board::pluginentry::clap_create_plugin,
};
static const void *get_factory(const char *factory_id)
{
    return (!strcmp(factory_id, CLAP_PLUGIN_FACTORY_ID)) ? &clap_board_factory : nullptr;
}

// clap_init and clap_deinit are required to be fast, but we have nothing we need to do here
bool clap_init(const char *p) { return true; }
void clap_deinit() {}

} // namespace sst::clap_board::pluginentry

extern "C"
{
    // clang-format off
    const CLAP_EXPORT struct clap_plugin_entry clap_entry = {
        CLAP_VERSION,
        sst::clap_board::pluginentry::clap_init,
        sst::clap_board::pluginentry::clap_deinit,
        sst::clap_board::pluginentry::get_factory
    };
    // clang-format on
}
