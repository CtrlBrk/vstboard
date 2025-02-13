#include <iostream>
#include "VstPlugin.h"
#include "VstHost.h"
#include "vstwindow.h"
#include "ipcvst.h"

CVSTHost host;
using namespace std;

IpcVst::IpcVst() :
	ipcMutex(0),
	ipcSemStart(0),
	ipcSemEnd(0),
	hMapFile(0),
	map(0)
{
}

void IpcVst::Loop() {

	TCHAR szName[] = TEXT("Local\\vstboard");
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ipc32), szName);
	if (!hMapFile)
	{
		MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "VstBoard", MB_OK);
		return;
	}
	map = (ipc32*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ipc32));
	if (!map)
	{
		MessageBoxA(GetActiveWindow(), "MapViewOfFile failed", "VstBoard", MB_OK);
		return;
	}

	ipcMutex = CreateMutex(NULL, FALSE, L"vstboardLock");
	if (!ipcMutex) {
		return;
	}

	ipcSemStart = CreateSemaphore(NULL, 0, 1, L"vstboardSemStart");
	if (!ipcSemStart) {
		return;
	}
	ipcSemEnd = CreateSemaphore(NULL, 0, 1, L"vstboardSemEnd");
	if (!ipcSemEnd) {
		return;
	}



	MSG msg;
	while (1) {
		
		//if (PeekMessage(&msg, hWin, 0, 0, PM_REMOVE))
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
		{
			if (msg.message == WM_QUIT)
			{
				break;
			}

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		//WaitForSingleObject(ipcSemStart, INFINITE);

		if (WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
			return;
		}

		if (map) {
			if (plugins.count(map->pluginId)) {
				plugins[map->pluginId]->MsgLoop(map);
			}

			switch (map->function) {
			case ipc32::Function::None:
				break;
			case ipc32::Function::LoadDll:
				cout << "load " << map->pluginId << endl;
				plugins[map->pluginId] = new VstPlugin(this, map->pluginId);
				plugins[map->pluginId]->Load(map->name);
				break;
			case ipc32::Function::UnloadDll:
				cout << "unload " << map->pluginId << endl;
				if (plugins.count(map->pluginId)) {
					delete plugins[map->pluginId];
					plugins.erase(map->pluginId);
					cout << "unload" << endl;
				}
				break;
			}

			map->function = ipc32::Function::None;

		}
		//maybe try to flush ?
		//FlushViewOfFile(mapFileBuffer, 0);

		ReleaseMutex(ipcMutex);
		ReleaseSemaphore(ipcSemEnd, 1, NULL);

	}

	UnmapViewOfFile(map);
	CloseHandle(hMapFile);
}

VstIntPtr IpcVst::onCallback(int pluginId,long opcode, long index, long value, void* ptr, float opt, long currentReturnCode)
{
	switch (opcode) {
	case audioMasterAutomate: //0
		if (WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
			return currentReturnCode;
		}

		if (map) {
			map->cbPluginId = pluginId;
			map->callback = ipc32::Function::SetParam;
			map->cbOpcode = opcode;
			map->cbIndex = index;
			map->cbValue = value;
			map->cbOpt = opt;
		//	cout << "callback " << map->cbPluginId << " " << opcode << " " << map->cbOpcode << " " << map->cbIndex << " " << map->cbValue << " " << map->cbOpt << endl;
		}
		
		ReleaseMutex(ipcMutex);
		break;
	
	}


	return currentReturnCode;
}