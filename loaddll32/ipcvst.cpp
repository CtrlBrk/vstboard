#include <iostream>
#include "VstPlugin.h"
#include "VstHost.h"
#include "vstwindow.h"
#include "ipcvst.h"
#include <thread>

CVSTHost host;
using namespace std;

IpcVst::IpcVst() :
	dataIn(0),
	dataOut(0),
	ipcIn(L"to32", (void**)& dataIn, sizeof(structPilot)),
	ipcOut(L"from32", (void**)& dataOut, sizeof(structFrom32))
{

}

void IpcVst::Loop() {
	
	MSG msg;
	while (1) {
		
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
			//	break;
			}
			else {

				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		}
		
		map<int, VstPlugin*>::iterator it;
		for (it = plugins.begin(); it != plugins.end(); it++) {
			it->second->MsgLoop();
		}
				

		if (ipcIn.IfWaitingToStart()) {

			VstPlugin* p = 0;

			auto search = plugins.find(dataIn->pluginId);
			if(search != plugins.end()) {
				p = search->second;
			}
			
			switch (dataIn->function) {

			case IpcFunction::LoadDll:
				cout << "load " << dataIn->pluginId << endl;
				p = new VstPlugin(this, dataIn->pluginId);
				p->Load(dataIn->name,(float)dataIn->value,dataIn->index);
				plugins.insert({ dataIn->pluginId, p });
				break;

			case IpcFunction::UnloadDll:
				cout << "unload " << dataIn->pluginId << endl;
				if (p) {
					delete p;
					plugins.erase(dataIn->pluginId);
				}
				break;
			
			case IpcFunction::CloseHost:
				return;
			}
				
			dataIn->function = IpcFunction::None;
			ipcIn.SignalResultReady();
		}
	}

}

int IpcVst::onCallback(int pluginId,long opcode, long index, long value, void* ptr, float opt, long currentReturnCode)
{
	switch (opcode) {
	case audioMasterAutomate: //0
		
		ipcOut.LockData();

		if (dataOut) {
			dataOut->cbPluginId = pluginId;
			dataOut->callback = IpcFunction::SetParam;
			dataOut->cbOpcode = opcode;
			dataOut->cbIndex = index;
			dataOut->cbValue = value;
			dataOut->cbOpt = opt;
		//	cout << "callback " << dataOut->cbPluginId << " " << opcode << " " << dataOut->cbOpcode << " " << dataOut->cbIndex << " " << dataOut->cbValue << " " << dataOut->cbOpt << endl;
		}
		ipcOut.Unlockdata();
		
		break;
	
	}

	return currentReturnCode;
}