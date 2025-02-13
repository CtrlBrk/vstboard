#include <iostream>
#include "VstPlugin.h"
#include "VstHost.h"

#include "ipc32.h"
#include "vstwindow.h"
#include "ipcvst.h"

CVSTHost host;
using namespace std;

IpcVst::IpcVst() :
	HpluginDll(0),
	ipcMutex(0),
	ipcSemStart(0),
	ipcSemEnd(0),
	hMapFile(0),
	mapFileBuffer(0),
	plugin(0),
	hWin(nullptr)
{
}


bool IpcVst::InitModule(const wstring& filename)
{
	HpluginDll = LoadLibrary((filename).c_str());
	if (!HpluginDll) {
		MessageBox(NULL, L"dll not loaded", L"VstBoard", MB_OK | MB_ICONERROR);
		return false;
	}

	return true;
}

bool IpcVst::DeinitModule()
{
	if (HpluginDll) FreeLibrary(HpluginDll);
	HpluginDll = 0;
	return true;
}

VstWin* win = 0;

std::thread *th;
void CrtVstWin(IpcVst *vst) {
	/*
	win = new VstWin();
	vst->hWin = win->CrtWindow();

	
	
	//while (GetMessage(&msg, vst->hWin, 0, 0) != 0)
	//{
		//TranslateMessage(&msg);
		//DispatchMessage(&msg);
	//}
	*/
	WNDCLASSEX wcex{ sizeof(wcex) };
	wcex.lpfnWndProc = DefWindowProc;
	wcex.hInstance = GetModuleHandle(0);
	wcex.lpszClassName = L"from github.com/t-mat/";
	RegisterClassEx(&wcex);
	const auto style = WS_CAPTION | WS_THICKFRAME | WS_OVERLAPPEDWINDOW;
	vst->hWin = CreateWindow(
		wcex.lpszClassName, L"Vst32", style
		, 0, 0, 0, 0, NULL, 0, 0, 0
	);
	/*
	MSG msg;
	while (BOOL b = GetMessage(&msg, 0, 0, 0)) {
		if (b == -1) {
			//run = false;
			break;
		}
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
	*/
}

void IpcVst::resizeEditor(const RECT& clientRc) const {
	if (hWin) {
		auto rc = clientRc;
		const auto style = GetWindowLongPtr(hWin, GWL_STYLE);
		const auto exStyle = GetWindowLongPtr(hWin, GWL_EXSTYLE);
		const BOOL fMenu = GetMenu(hWin) != nullptr;
		AdjustWindowRectEx(&rc, style, fMenu, exStyle);
		MoveWindow(hWin, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}
}

bool IpcVst::EditOpen() {
	//std::thread vstwin(CrtVstWin,this);
	//vstwin.detach();
	//while (hWin == 0) {};
	CrtVstWin(this);

	//return true;
	//VstWin* win = new VstWin();
	//HWND hWin = win->CrtWindow();

	//bool windowOk = false;
	//ERect* rect = 0;

	if (hWin) {
		ShowWindow(hWin, SW_SHOW);
	}
	ERect* erc = nullptr;
	plugin->EffDispatch(effEditGetRect, 0, 0, &erc);
	if (plugin->EffDispatch(effEditOpen, 0, 0, hWin) == 1L) {
		plugin->EffDispatch(effEditGetRect, 0, 0, &erc);
		RECT rc{};
		if (erc) {
			rc.left = erc->left;
			rc.top = erc->top;
			rc.right = erc->right;
			rc.bottom = erc->bottom;
		}
		resizeEditor(rc);
		
	}
	/*
	if (plugin->EffEditGetRect(&rect) == 1L) windowOk = true;
	if (plugin->EffEditOpen(hWin) == 1L) windowOk = true;
	if (plugin->EffEditGetRect(&rect) == 1L) windowOk = true;	//try to get rect again
	if (!windowOk) return false;
	*/
	//SetWindowSize(rect->right, rect->bottom);
	return true;
}

void IpcVst::Loop() {

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
	if (!ipcSemEnd) {
		return;
	}


	float** ins = 0;
	float** outs = 0;
	float* tmp = 0;

	void* chunk = 0;
	MSG msg;
	LONG valLock = 0;
	while (1) {
		
		if (PeekMessage(&msg, hWin, 0, 0, PM_REMOVE))
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

		ipc32* pf = (ipc32*)mapFileBuffer;
		if (pf) {
			switch (pf->function) {
			case ipc32::Function::None:
				break;
			case ipc32::Function::LoadDll:
				if (!plugin) {
					cout << "load " << endl;
					plugin = new VstPlugin(this);
					plugin->Load(pf->name);
				}
				break;
			case ipc32::Function::UnloadDll:
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
				EditOpen();
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
				cout << "get segment start:" << pf->value << " size:" << pf->dataSize << endl;
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
			case ipc32::Function::Dispatch:
				if (plugin) {
				//	cout << "optcode " << pf->opCode << endl;
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
		//maybe try to flush ?
		//FlushViewOfFile(mapFileBuffer, 0);

		ReleaseMutex(ipcMutex);
		ReleaseSemaphore(ipcSemEnd, 1, NULL);

	}

	UnmapViewOfFile(mapFileBuffer);
	CloseHandle(hMapFile);
}

VstIntPtr IpcVst::onCallback(long opcode, long index, long value, void* ptr, float opt, long currentReturnCode)
{
	switch (opcode) {
	case audioMasterAutomate: //0
		if (WaitForSingleObject(ipcMutex, IPC_TIMEOUT) == WAIT_TIMEOUT) {
			return currentReturnCode;
		}

		ipc32* pf = (ipc32*)mapFileBuffer;
		if (pf) {
			pf->callback = ipc32::Function::SetParam;
			pf->cbOpcode = opcode;
			pf->cbIndex = index;
			pf->cbValue = value;
			pf->cbOpt = opt;
			cout << "callback " << opcode << " " << pf->cbOpcode << " " << pf->cbIndex << " " << pf->cbValue << " " << pf->cbOpt << endl;
		}
		
		ReleaseMutex(ipcMutex);
		break;
	
	}


	return currentReturnCode;
}