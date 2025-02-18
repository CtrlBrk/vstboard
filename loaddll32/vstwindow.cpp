#include "vstwindow.h"
#include "VstPlugin.h"

#define TIMERMOVE 1001

VstWin::VstWin(VstPlugin* vst) :
	hWin(0) ,
	plugin(vst)
{

}

VstWin::~VstWin() {
	if (hWin) {
		DestroyWindow(hWin);
	}
}
HWND VstWin::CrtWindow(HWND parent) {
/*
	const wchar_t CLASS_NAME[] = L"Vst32";
	WNDCLASS wc = { };
	wc.lpfnWndProc = DefWindowProc;// WindowProc;
	wc.hInstance = NULL;
	wc.lpszClassName = CLASS_NAME;
	RegisterClass(&wc);
	const auto style = WS_OVERLAPPEDWINDOW;
	hWin = CreateWindowEx(
		0,                              
		CLASS_NAME,                    
		L"Vst32",   
		style,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL,  
		NULL,  
		wc.hInstance, 
		NULL 
	);

*/
	


	WNDCLASSEX wcex{ sizeof(wcex) };
	wcex.lpfnWndProc = WindowProc;
	wcex.hInstance = GetModuleHandle(0);
	wcex.lpszClassName = L"Vst32";
	RegisterClassEx(&wcex);

	const auto style = WS_DLGFRAME;// WS_CAPTION | WS_BORDER | WS_OVERLAPPEDWINDOW;
	hWin = CreateWindow(
		wcex.lpszClassName, wcex.lpszClassName, style
		, 0, 0, 0, 0, parent, 0, 0, 0
	);
	return hWin;
}

void VstWin::resizeEditor(const RECT& clientRc) const {
	if (hWin) {
		auto rc = clientRc;
		const auto style = GetWindowLongPtr(hWin, GWL_STYLE);
		const auto exStyle = GetWindowLongPtr(hWin, GWL_EXSTYLE);
		const BOOL fMenu = GetMenu(hWin) != nullptr;
		AdjustWindowRectEx(&rc, style, fMenu, exStyle);
		MoveWindow(hWin, 0, 0, rc.right - rc.left, rc.bottom - rc.top, TRUE);
	}
}

LRESULT CALLBACK VstWin::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if (WM_NCCREATE == uMsg)
	{
		SetWindowLongPtr(hwnd, GWLP_USERDATA, (LONG_PTR)((CREATESTRUCT*)lParam)->lpCreateParams);
		return TRUE;

	}

	return ((VstWin*)GetWindowLongPtr(hwnd, GWLP_USERDATA))->_WindowProc(hwnd, uMsg, wParam, lParam);
	
}

LRESULT VstWin::_WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	UINT_PTR IDT_TIMER1=0;

	switch (uMsg)
	{
	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_ENTERSIZEMOVE:
		SetTimer(hwnd,           
			TIMERMOVE,
			10,              
			(TIMERPROC)NULL);
		break;
	case WM_EXITSIZEMOVE:
		KillTimer(hwnd, TIMERMOVE);
		break;
	case WM_TIMER:
		if (wParam == TIMERMOVE) {
			//crash...
			//plugin->MsgLoop();
		}
		break;

	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}