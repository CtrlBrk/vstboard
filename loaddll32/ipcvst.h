#ifndef IPCVST
#define IPCVST

#include <windows.h>
#include <string>
//#include <thread>
#include <map>
//#include "public.sdk/source/vst2.x/audioeffectx.h"
#include "ipc32.h"

using namespace std;

class VstPlugin;
//class CVSTHost;


class IpcVst {
	
public:
	IpcVst();
	void Loop();
	VstIntPtr onCallback(int pluginId, long opcode, long index, long value, void* ptr, float opt, long currentReturnCode);
	

private:

	HANDLE ipcMutex;
	HANDLE ipcSemStart;
	HANDLE ipcSemEnd;
	HANDLE hMapFile;
	ipc32* map;

	
	std::map<int,VstPlugin*> plugins;
	//VstPlugin* plugin;
	//CVSTHost* host;


	void resizeEditor(const RECT& clientRc) const;
	bool InitModule(const wstring& filename);
	bool DeinitModule();
	bool EditOpen();
	

};

#endif