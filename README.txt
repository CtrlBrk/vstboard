Disclaimer :
If it's not obvious, if you're currently learning to code and don't know better yet : please don't use my code as a reference
I am not a developer, this is a hobby project and I don't know what I am doing.


As a standalone, VstBoard can host 64bits CLAP,VST3,VST and 32bits VST 
VstBoard can also be loaded as a 64bits CLAP,VST3,VST plugin




How to build VstBoard :
================

Steinberg SDKs :
================
"VST SDK" and "ASIO SDK" are available at https://www.steinberg.net/developers/
Copy the VST SDK in ./libs/vst3sdk
Copy the Asio SDK in ./libs/portaudio/src/hostapi/asio/ASIOSDK (see README.txt in that folder)

Vst2.4 is no longer supported, a compatibility is still provided by vestige Copyright (c) 2006 Javier Serrano Polo <jasp00/at/users.sourceforge.net>

Qt :
================
Download and install "Qt Community Edition" for MSVC from https://www.qt.io

QwinMirgate :
================
found in the libs directory
building it as a dll is not straight forward, it need some convincing... I don't remember what worked in the end

build64.bat :
================
use the MSVC "x64 Native Tools Command Prompt for VS 2022" prompt
launch build64.bat from the source dir, it should build the release version, and the installer with NSIS (https://nsis.sourceforge.io/Download)

