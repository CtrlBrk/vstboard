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


HANDLE hMutex;
HANDLE hMutexStart;
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

	hMutex = CreateMutex(NULL, FALSE, L"vstboardLock");
	if (!hMutex) {
		return;
	}
	hMutexStart = CreateMutex(NULL, FALSE, L"vstboardLockStart");
	if (!hMutexStart) {
		return;
	}

	float** ins = 0;
	float** outs = 0;
	float* tmp = 0;

	while (true)
	{
		//if (WaitForSingleObject(hMutexStart, 10000) == WAIT_TIMEOUT) {
		//	return;
		//}
		
		if (WaitForSingleObject(hMutex, 10000) == WAIT_TIMEOUT) {
			return;
		}
		
		ipc32* pf = (ipc32*)mapFileBuffer;
		if (pf) {
			switch (pf->function) {
			case ipc32::Function::None:
				break;
			case ipc32::Function::LoadDll :
				if (!plugin) {
					plugin = new VstPlugin();
					plugin->Load(pf->name);
				}
				break;
			case ipc32::Function::UnloadDll :
				if (plugin) {
					plugin->Unload();
					plugin = 0;
				}
				break;
			case ipc32::Function::GetAEffect:
				pf->flags = plugin->pEffect->flags;
				pf->numInputs = plugin->pEffect->numInputs;
				pf->numOutputs = plugin->pEffect->numOutputs;
				pf->initialDelay = plugin->pEffect->initialDelay;
				pf->numParams = plugin->pEffect->numParams;
				break;
			case ipc32::Function::EditOpen:

				break;
			case ipc32::Function::GetParam:
				pf->opt =  plugin->pEffect->getParameter(plugin->pEffect, pf->index);
				break;
			case ipc32::Function::SetParam:
				plugin->pEffect->setParameter(plugin->pEffect, pf->index, pf->opt);
				break;
			case ipc32::Function::Process:
			case ipc32::Function::ProcessReplace:
				
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
				plugin->pEffect->process(plugin->pEffect, ins, outs, pf->dataSize);

				delete[] ins;
				delete[] outs;
				
				break;
			case ipc32::Function::ProcessDouble:
				
				break;
			case ipc32::Function::Dispatch :
				if (plugin) {
					pf->dispatchReturn = plugin->EffDispatch(pf->opCode, pf->index, pf->value, pf->data, pf->opt);
				}
			}
			pf->function = ipc32::Function::None;
			
		}
		//ReleaseMutex(hMutexStart);
		ReleaseMutex(hMutex);
	}
	
	UnmapViewOfFile(mapFileBuffer);
	CloseHandle(hMapFile);
}
