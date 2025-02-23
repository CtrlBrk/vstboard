#include <windows.h>
#include "ipcvst.h"

IpcVst* ipc=0;

void Loop() {
	ipc->Loop();
}

int main() {


	CreateMutex(NULL, TRUE, L"singleInstanceVst32");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return FALSE;
	}


	ipc = new IpcVst();
	ipc->Loop();
	//std::thread vstloop(Loop);

	
	
	
	delete ipc;
	return 0;
}
