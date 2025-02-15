#pragma once
#include <windows.h>
#include <string>

#define IPC_TIMEOUT INFINITE
//#define IPC_TIMEOUT 10000

class Ipc
{
public:
    Ipc(std::wstring name, void** dataPtr, DWORD size);
	~Ipc();
	bool Init(void** dataPtr);

	//client side
	void LockData();
		//write
	void SignalStart();
		//read response
		
	//start and don't wait for the response
	void SignalStartAndRelease();
	
	//server side
	//if true : got the semaphore, start process
	bool IfWaitingToStart();
		//read / write
	void SignalResultReady();

	//back to client side
	void Unlockdata();
	

private:
	HANDLE mutexData;
	HANDLE semaphStart;
	HANDLE semaphReady;
	HANDLE hMapFile;
	void* map;

	std::wstring name;
    DWORD size;
};
