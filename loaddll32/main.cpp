#include <windows.h>
#include <string>
#include <iostream>
#include "ipc32.h"
#include "VstPlugin.h"
#include "VstHost.h"

HMODULE HpluginDll = 0;

using namespace std;

bool InitModule(const wstring & filename)
{
	HpluginDll = LoadLibrary((filename).c_str());
	if (!HpluginDll) {
		MessageBox(NULL, L"dll not loaded", L"VstBoard", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

bool DeinitModule()
{
	if (HpluginDll) FreeLibrary(HpluginDll);
	HpluginDll = 0;
	return true;
}


HANDLE ipcMutex;
HANDLE ipcSemStart;
HANDLE ipcSemEnd;
HANDLE hMapFile;
unsigned char* mapFileBuffer;

using namespace std;

VstPlugin* plugin = 0;
CVSTHost host;

void main() {
	
	TCHAR szName[] = TEXT("Local\\vstboard");
	hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(ipc32), szName);
	if (!hMapFile)
	{
		MessageBoxA(GetActiveWindow(), "CreateFileMapping failed", "VstBoard", MB_OK);
		return;
	}
	mapFileBuffer = (unsigned char*)MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(ipc32));
	if (!mapFileBuffer)
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
	//OpenSemaphore(SEMAPHORE_ALL_ACCESS, TRUE, L"vstboardLockBack");
	if (!ipcSemEnd) {
		return;
	}


	float** ins = 0;
	float** outs = 0;
	float* tmp = 0;

	void* chunk = 0;
	
	while (true)
	{
		WaitForSingleObject(ipcSemStart, INFINITE);

		if (WaitForSingleObject(ipcMutex, 10000) == WAIT_TIMEOUT) {
			return;
		}
		
		ipc32* pf = (ipc32*)mapFileBuffer;
		if (pf) {
			switch (pf->function) {
			case ipc32::Function::None:
				break;
			case ipc32::Function::LoadDll :
				if (!plugin) {
					cout << "load " << endl;
					plugin = new VstPlugin();
					plugin->Load(pf->name);
				}
				break;
			case ipc32::Function::UnloadDll :
				if (plugin) {
					cout << "unload" << endl;
					plugin->Unload();
					plugin = 0;
				}
				break;
			case ipc32::Function::GetAEffect:
				if (plugin && plugin->pEffect) {
					pf->flags = plugin->pEffect->flags;
					pf->numInputs = plugin->pEffect->numInputs;
					pf->numOutputs = plugin->pEffect->numOutputs;
					pf->initialDelay = plugin->pEffect->initialDelay;
					pf->numParams = plugin->pEffect->numParams;
				}
				break;
			case ipc32::Function::EditOpen:

				break;
			case ipc32::Function::GetParam:
				if (plugin && plugin->pEffect) {
					pf->opt = plugin->pEffect->getParameter(plugin->pEffect, pf->index);
				}
				break;
			case ipc32::Function::SetParam:
				if (plugin && plugin->pEffect) {
					plugin->pEffect->setParameter(plugin->pEffect, pf->index, pf->opt);
				}
				break;
			case ipc32::Function::Process:
			case ipc32::Function::ProcessReplace:
				if (plugin && plugin->pEffect) {
					ins = new float* [plugin->pEffect->numInputs];
					outs = new float* [plugin->pEffect->numOutputs];

					tmp = (float*)&pf->buffersIn;
					for (int i = 0; i < plugin->pEffect->numInputs; i++) {
						ins[i] = tmp;
						tmp += sizeof(float) * pf->dataSize;
					}
					tmp = (float*)&pf->buffersOut;
					for (int i = 0; i < plugin->pEffect->numOutputs; i++) {
						outs[i] = tmp;
						tmp += sizeof(float) * pf->dataSize;
					}

					if (pf->function == ipc32::Function::Process) {
						plugin->pEffect->process(plugin->pEffect, ins, outs, pf->dataSize);
					}
					if (pf->function == ipc32::Function::ProcessReplace) {
						plugin->pEffect->processReplacing(plugin->pEffect, ins, outs, pf->dataSize);
					}
					/*
									cout << &outs[0][0] << ":";
									for (int a = 0; a < pf->dataSize; a++) {
										cout << outs[0][a];
									}
									cout << endl;
						*/
					delete[] ins;
					delete[] outs;
				}
				break;
			case ipc32::Function::ProcessDouble:
				
				break;

			case ipc32::Function::GetChunk:
				if (plugin && plugin->pEffect) {
					pf->dataSize = plugin->EffGetChunk(&chunk, false);
					cout << "get chunk size:" << pf->dataSize << endl;
				}
				break;
			case ipc32::Function::GetChunkSegment:
				cout << "get segment start:" << pf->value << " size:" << pf->dataSize  << endl;
				memcpy_s(pf->data, IPC_CHUNK_SIZE, (char*)chunk + pf->value, pf->dataSize);
				
				break;
			case ipc32::Function::SetChunk:
				cout << "set chunk size:" << pf->dataSize << endl;
				chunk = new char[pf->dataSize];
				break;
			case ipc32::Function::SetChunkSegment:
				cout << "set segment start:" << pf->value << " size:" << pf->dataSize << endl;
				memcpy_s((char*)chunk + pf->value, IPC_CHUNK_SIZE, pf->data, min(pf->dataSize, IPC_CHUNK_SIZE));
				break;
			case ipc32::Function::DeleteChunk:
				if (chunk) {
					delete chunk;
					chunk = 0;
				}
				break;
			case ipc32::Function::Dispatch :
				if (plugin) {

					//we have segemented data, use it
					if (chunk) {
						cout << "use chunk" << endl;
						pf->dispatchReturn = plugin->EffDispatch(pf->opCode, pf->index, pf->value, chunk, pf->opt);
						//delete chunk;
						//chunk = 0;
					}
					else {
						pf->dispatchReturn = plugin->EffDispatch(pf->opCode, pf->index, pf->value, pf->data, pf->opt);
					}
				}
			}
			pf->function = ipc32::Function::None;
			
		}
		ReleaseMutex(ipcMutex);
		ReleaseSemaphore(ipcSemEnd, 1, NULL);
		
	}
	
	UnmapViewOfFile(mapFileBuffer);
	CloseHandle(hMapFile);
}
