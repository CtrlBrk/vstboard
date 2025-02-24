#ifndef IPCVST
#define IPCVST

#include <windows.h>
#include <string>
#include <map>
#include "ipc32.h"
#include "ipc.h"

using namespace std;

class VstPlugin;

class IpcVst {
	
public:
	IpcVst();
	void Loop();
	int onCallback(int pluginId, long opcode, long index, long value, void* ptr, float opt, long currentReturnCode);
	
	structPilot* dataIn;
	structFrom32* dataOut;
	Ipc ipcIn;
	Ipc ipcOut;


	std::map<int,VstPlugin*> plugins;
	
};

#endif