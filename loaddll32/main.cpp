#include <windows.h>
#include "ipcvst.h"

IpcVst ipc;

int main() {


	CreateMutex(NULL, TRUE, L"singleInstanceVst32");
	if (GetLastError() == ERROR_ALREADY_EXISTS) {
		return FALSE;
	}

	//ipc = new IpcVst();
	//ipc->Loop();
	//delete ipc;
	ipc.Loop();
	return 0;
}
