#pragma once
#include <windows.h>



class VstWin
{
public:
	VstWin();

public:
	HWND CrtWindow();
	void OnSize(HWND hwnd, UINT flag, int width, int height);

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

