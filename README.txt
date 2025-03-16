Disclaimer:
I am not a developer; this is a hobby project and I don't know what I am doing. Please don't use my code as a reference.


As a standalone application, VstBoard can host 64-bit CLAP, VST3, VST and 32-bit VST.
VstBoard can also be loaded as a 64-bit CLAP, VST3, VST plugin.




How to build VstBoard:
================

Steinberg SDKs:
================
"VST SDK" and "ASIO SDK" are available at https://www.steinberg.net/developers/
Copy the VST SDK in ./libs/vst3sdk
Copy the Asio SDK in ./libs/portaudio/src/hostapi/asio/ASIOSDK (see README.txt in that folder)

Vst2.4 is no longer supported; compatibility is still provided by vestige Copyright (c) 2006 Javier Serrano Polo <jasp00/at/users.sourceforge.net>

Qt:
================
Download and install "Qt Community Edition" for MSVC from https://www.qt.io

QwinMirgate:
================
Found in the libs directory
Building it as a DLL is not straightforward; it needs some convincing... I don't remember what worked in the end

build64.bat:
================
Open buildall.pro in QtCreator,
or user the build64.bat script :
Use the MSVC "x64 Native Tools Command Prompt for VS 2022" prompt
Launch build64.bat from the source dir, it should build the release version, and the installer with NSIS (https://nsis.sourceforge.io/Download)

