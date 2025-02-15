#include "ipc.h"
#include <iostream>
#include <thread>
using namespace std;

Ipc::Ipc(std::wstring n,void** dataPtr, DWORD s) :
	mutexData(0),
	semaphStart(0),
	semaphReady(0),
	hMapFile(0),
	name(n),
	size(s)
{
    Init(dataPtr);
	map = *dataPtr;
}

Ipc::~Ipc()
{
    if(map) UnmapViewOfFile(map);
    if(hMapFile) CloseHandle(hMapFile);
    if(mutexData) CloseHandle(mutexData);
    if(semaphStart) CloseHandle(semaphStart);
    if(semaphReady) CloseHandle(semaphReady);
}

bool Ipc::Init(void** dataPtr)
{
	std::wstring s(L"Local\\vstboard");
    hMapFile = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, size, (s + name).c_str());
    //if( GetLastError() == ERROR_ALREADY_EXISTS) {
    //    cout << "file map already exists" << endl;
    //}
    if (!hMapFile)
	{
		return false;
	}
    map = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, size);
	if (!map)
	{
		return false;
	}
	*dataPtr = map;
	
	s = L"data";
	mutexData = CreateMutex(NULL, FALSE, (s + name).c_str()   );
	if (!mutexData) {
		return false;
	}

	s = L"start";
	semaphStart = CreateSemaphore(NULL, 0, 1, (s + name).c_str());
	if (!semaphStart) {
		return false;
	}


	s = L"ready";
	semaphReady = CreateSemaphore(NULL, 0, 1, (s + name).c_str());
	if (!semaphReady) {
		return false;
	}

	return true;
}
void Ipc::LockData()
{
	WaitForSingleObject(mutexData, IPC_TIMEOUT);
}

void Ipc::SignalStart()
{
    SignalObjectAndWait(semaphStart,semaphReady,IPC_TIMEOUT,FALSE);
}

void Ipc::SignalStartAndRelease()
{
    SignalObjectAndWait(semaphStart,semaphReady,IPC_TIMEOUT,FALSE);
    ReleaseMutex(mutexData);
}

bool Ipc::IfWaitingToStart()
{

    DWORD r = WaitForSingleObject(semaphStart, 0);
	switch (r) {
	case WAIT_TIMEOUT:
	case WAIT_ABANDONED:
		return false;
	case WAIT_OBJECT_0:
		return true;
	}
	return false;

}

void Ipc::SignalResultReady()
{
	ReleaseSemaphore(semaphReady, 1, NULL);
}

void Ipc::Unlockdata()
{
	ReleaseMutex(mutexData);
}
