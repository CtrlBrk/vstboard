#ifndef IPCVST
#define IPCVST

#include <windows.h>
#include <string>
#include <thread>
#include "public.sdk/source/vst2.x/audioeffectx.h"

using namespace std;

class VstPlugin;
//class CVSTHost;


class IpcVst {
	
public:
	IpcVst();
	void Loop();
	VstIntPtr onCallback(long opcode, long index, long value, void* ptr, float opt, long currentReturnCode);
	HWND hWin;
	

private:
	HMODULE HpluginDll;

	HANDLE ipcMutex;
	HANDLE ipcSemStart;
	HANDLE ipcSemEnd;
	HANDLE hMapFile;
	unsigned char* mapFileBuffer;

	

	VstPlugin* plugin;
	//CVSTHost* host;


	void resizeEditor(const RECT& clientRc) const;
	bool InitModule(const wstring& filename);
	bool DeinitModule();
	bool EditOpen();
	

};

#endif