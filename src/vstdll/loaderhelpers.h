/**************************************************************************
#    Copyright 2010-2020 Raphaël François
#    Contact : ctrlbrk76@gmail.com
#
#    This file is part of VstBoard.
#
#    VstBoard is free software: you can redistribute it and/or modify
#    it under the terms of the under the terms of the GNU Lesser General Public License as published by
#    the Free Software Foundation, either version 3 of the License, or
#    (at your option) any later version.
#
#    VstBoard is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    under the terms of the GNU Lesser General Public License for more details.
#
#    You should have received a copy of the under the terms of the GNU Lesser General Public License
#    along with VstBoard.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

#ifndef LOADERHELPERS_H
#define LOADERHELPERS_H


#include <sys/types.h>
#include <wchar.h>

struct RegistryError {
    std::wstring errMsg;
    LONG errCode;
};
struct FileError {
    std::wstring errMsg;
    LONG errCode;
    const std::wstring filePath;
};

#ifndef QT_NO_DEBUG
std::wstring installKey(L"DebugLocation");
#else
std::wstring installKey(L"InstallLocation");
#endif

#if defined(_M_X64) || defined(__amd64__)
std::wstring regBaseKey(L"Software\\CtrlBrk\\VstBoard\\x64");
#else
std::wstring regBaseKey(L"Software\\CtrlBrk\\VstBoard\\x86");
#endif


std::wstring RegGetString(HKEY hKey, const std::wstring& subKey, const std::wstring& value) {
    DWORD dataSize{};
    LONG retCode = ::RegGetValue(
        hKey,
        subKey.c_str(),
        value.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        nullptr,
        &dataSize
    );
    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{ L"Cannot read string from registry", retCode };
    }

    std::wstring data;
    data.resize(dataSize / sizeof(wchar_t));

    retCode = ::RegGetValue(
        hKey,
        subKey.c_str(),
        value.c_str(),
        RRF_RT_REG_SZ,
        nullptr,
        &data[0],
        &dataSize
    );

    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{ L"Cannot read string from registry", retCode };
    }

    DWORD stringLengthInWchars = dataSize / sizeof(wchar_t);
    stringLengthInWchars--; // Exclude the NUL written by the Win32 API
    data.resize(stringLengthInWchars);
    return data;
}

void RegSetString(HKEY hKey, const std::wstring& subKey, std::wstring& value, const std::wstring& data)
{
    HKEY  handle;
    LONG retCode = ::RegOpenKeyEx(hKey, subKey.c_str(), 0, KEY_SET_VALUE, &handle);
    if (retCode != ERROR_SUCCESS) {
        throw RegistryError{ L"Cannot open registry key", retCode };
    }

    DWORD dataSize = (DWORD)data.length() * sizeof(WCHAR);

    retCode = ::RegSetValueEx(
        handle,
        value.c_str(),
        0,
        RRF_RT_REG_SZ,
        (BYTE *)data.c_str(),
        dataSize
    );

    if (retCode != ERROR_SUCCESS)
    {
        throw RegistryError{ L"Cannot set string in registry", retCode };
    }
}

HMODULE LoadDll(const std::wstring &dll) {

    HMODULE h = 0;

#ifndef QT_NO_DEBUG
    //try to load the debug version
    h = LoadLibrary((dll + L"d.dll").c_str());
    if (h) return h;

#endif
    h = LoadLibrary((dll + L".dll").c_str());
    if (h) return h;

    return 0;
}

std::wstring openfilename(HWND owner = NULL) {
    OPENFILENAME ofn;
    WCHAR fileName[MAX_PATH] = L"";
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(OPENFILENAME);
    ofn.hwndOwner = owner;
    ofn.lpstrFilter = L"Programs (.exe)\0*.exe\0All Files\0*.*\0\0";
    ofn.lpstrFile = fileName;
    ofn.nMaxFile = MAX_PATH;
    ofn.Flags = OFN_EXPLORER | OFN_FILEMUSTEXIST | OFN_HIDEREADONLY;
    ofn.lpstrDefExt = L"";
    std::wstring fileNameStr;
    if (GetOpenFileName(&ofn))
        fileNameStr = fileName;
    return fileNameStr;
}

std::wstring TestInstallPath(const std::wstring &currentPath) {

    std::list<std::wstring> instPaths = { currentPath };

    try {
        instPaths.push_back(RegGetString(HKEY_CURRENT_USER, regBaseKey, installKey));
    }
    catch (RegistryError &e) {}

    try {
        instPaths.push_back( RegGetString(HKEY_LOCAL_MACHINE, regBaseKey, installKey) );
    }
    catch (RegistryError &e) {}

    for (auto const& p : instPaths) {
        struct __stat64 buffer;
        if (_wstat64((p + L"\\Qt5Cored.dll").c_str(), &buffer) == 0) {
            return p;
        }
    }

    return L"";
}

#endif // LOADERHELPERS_H
