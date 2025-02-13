#include <windows.h>
#include "vstwindow.h"
#include "ipcvst.h"

IpcVst* ipc=0;

void Loop() {
	ipc->Loop();
}

int main() {

	ipc = new IpcVst();
	ipc->Loop();
	//std::thread vstloop(Loop);

	
	
	
	delete ipc;
	return 0;
}
