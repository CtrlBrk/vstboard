#pragma once
#include <windows.h>

class VstPlugin;

class VstWin
{
public:
	VstWin(VstPlugin *vst);
	~VstWin();

public:
	HWND CrtWindow(HWND parent=nullptr);
	void resizeEditor(const RECT& clientRc) const;

	static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
	LRESULT _WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

	HWND hWin;
	VstPlugin* plugin;
};

