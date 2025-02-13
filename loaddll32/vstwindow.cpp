#include "vstwindow.h"


VstWin::VstWin() {

}

HWND VstWin::CrtWindow() {
	const wchar_t CLASS_NAME[] = L"Vst32";

	WNDCLASS wc = { };

	wc.lpfnWndProc = DefWindowProc;// WindowProc;
	wc.hInstance = NULL;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);

	HWND hwnd = CreateWindowEx(
		0,                              // Optional window styles.
		CLASS_NAME,                     // Window class
		L"Learn to Program Windows",    // Window text
		WS_OVERLAPPEDWINDOW,            // Window style

		// Size and position
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,

		NULL,       // Parent window    
		NULL,       // Menu
		wc.hInstance,  // Instance handle
		NULL        // Additional application data
	);

	if (hwnd == NULL)
	{
		return 0;
	}

	ShowWindow(hwnd, SW_SHOWDEFAULT);

	return hwnd;
}

void VstWin::OnSize(HWND hwnd, UINT flag, int width, int height)
{
	// Handle resizing
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
	
	switch (uMsg)
	{
	case WM_DESTROY:
		PostQuitMessage(S_OK);
		break;
	case WM_SIZE:
	{
		int width = LOWORD(lParam); 
		int height = HIWORD(lParam);

		// Respond to the message:
		OnSize(hwnd, (UINT)wParam, width, height);
	}
	break;
	default:
		return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}


	return 0;
}