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
	VstIntPtr onCallback(int pluginId, long opcode, long index, long value, void* ptr, float opt, long currentReturnCode);
	


	structTo32* dataIn;
	structFrom32* dataOut;
	Ipc ipcIn;
	Ipc ipcOut;
private:

	std::map<int,VstPlugin*> plugins;
	
};

#endif