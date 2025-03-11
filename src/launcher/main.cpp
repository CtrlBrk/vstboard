#include <windows.h>
#include "../dllLoader/loaderhelpers.h"

int main() {
    AddDllPath();

    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory( &si, sizeof(si) );
    si.cb = sizeof(si);
    ZeroMemory( &pi, sizeof(pi) );
    CreateProcess( L"VstBoardHost.exe",   // the path
                  NULL,         // Command line
                  NULL,           // Process handle not inheritable
                  NULL,           // Thread handle not inheritable
                  FALSE,          // Set handle inheritance to FALSE
                  0,              // No creation flags
                  NULL,           // Use parent's environment block
                  NULL,           // Use parent's starting directory
                  &si,            // Pointer to STARTUPINFO structure
                  &pi             // Pointer to PROCESS_INFORMATION structure (removed extra parentheses)
                  );
    // Close process and thread handles.
    CloseHandle( pi.hProcess );
    CloseHandle( pi.hThread );

}
