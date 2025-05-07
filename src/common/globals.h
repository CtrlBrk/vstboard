/**************************************************************************
#    Copyright 2010-2025 Raphaël François
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

#ifndef GLOBALS_H
#define GLOBALS_H

//#include "precomp.h"

#define APP_VENDOR "CtrlBrk"
#define APP_NAME "VstBoard"

#define EMPTY_PROGRAM 0xffff
#define TEMP_PROGRAM 0xfffe

#define SETUP_FILE_EXTENSION "vstbsetup"
#define SETUP_JSON_FILE_EXTENSION "vbs.json"
#define SETUP_JSON_BINARY_FILE_EXTENSION "vbs.zjson"
#define PROJECT_FILE_EXTENSION "vstbproject"
#define PROJECT_JSON_FILE_EXTENSION "vbp.json"
#define PROJECT_JSON_BINARY_FILE_EXTENSION "vbp.zjson"
#define NB_RECENT_FILES 5
#define VST_BANK_FILE_EXTENSION "fxb"
#define VST_PROGRAM_FILE_EXTENSION "fxp"

#define MME_DFAULT_FLAGS paWinMmeUseLowLevelLatencyParameters | paWinMmeDontThrottleOverloadedProcessingThread
#define MME_DEFAULT_BUFFER_SIZE 512
#define MME_DEFAULT_BUFFER_COUNT 8

#define WASAPI_DEFAULT_FLAGS 0
#define WASAPI_DEFAULT_INLATENCY 0
#define WASAPI_DEFAULT_OUTLATENCY 0

#define DIRECTX_DFAULT_FLAGS paWinDirectSoundUseLowLevelLatencyParameters
#define DIRECTX_DEFAULT_BUFFER_SIZE 4096

namespace UserRoles {
    enum Enum {
        programGroup = Qt::UserRole+1,
        programNumber,
        shortcut,
        connectionInfo,
        value,
        stepSize,
        position,
        parking,
        objInfo,
        nodeType,
        editorVisible,
        editorImage,
        editorPos,
        editorSize,
        editorVScroll,
        editorHScroll,
        errorMessage,
        isDirty,
        type,
        bankFile,
        programFile,
        isDoublePrecision,
        enable
    };
}
namespace NodeType {
    enum Enum {
        ND,
        object,
        container,
        bridge,
        listPin,
        pin,
        cable,
        cursor,
        tempObj
    };
}

namespace ObjType {
    enum Enum {
        ND,
        AudioInterface,
        AudioInterfaceIn,
        AudioInterfaceOut,
        MidiInterface,
        VstPlugin,
        MidiToAutomation,
        MidiSender,
        HostController,
        Container,
        ParkingContainer,
        BridgeIn,
        BridgeOut,
        BridgeSend,
        BridgeReturn,
        listAudioIn,
        listAudioOut,
        listMidiIn,
        listMidiOut,
        listParamIn,
        listParamOut,
        listBridgeIn,
        listBridgeOut,
        dummy,
        VstAutomation,
        limitInMin,
        limitInMax,
        limitOutMin,
        limitOutMax,
        Script,
        Buffer,
        Vst3Plugin,
        VstPlugin32,
        ClapPlugin,
        MainWindow
    };
}

namespace FixedObjId {
    static const qint32 dynamicIdStart = 50;
    enum Enum {
        ND,
        mainContainer,
        hostContainer,
        hostContainerIn,
        hostContainerOut,
        hostContainerSend,
        hostContainerReturn,
        projectContainer,
        projectContainerIn,
        projectContainerOut,
        projectContainerSend,
        projectContainerReturn,
        programContainer,
        programContainerIn,
        programContainerOut,
        programContainerSend,
        programContainerReturn,
        groupContainer,
        groupContainerIn,
        groupContainerOut,
        groupContainerSend,
        groupContainerReturn,
        mainHost,
        mainWindow,
        programsManager,
        groupParking,
        programParking,
        audioDevices,
        midiDevices,
        tools,
        renderer,
        shellselect,
        noContainer=65535
    };
}

namespace PinType {
    enum Enum {
        ND,
        Audio,
        Midi,
        Parameter,
        Bridge
    };
}

namespace PinDirection {
    enum Enum {
        ND,
        Input,
        Output
    };
}

namespace PinMessage {
    enum Enum {
        ND,
        AudioBuffer,
        MidiMsg,
        ParameterValue,
        VstEvent,
        FadeIn,
        FadeOut
    };
}

#define FIXED_PIN_STARTINDEX 0xf000
namespace FixedPinNumber {
    enum Enum {
        editorVisible = FIXED_PIN_STARTINDEX,
        learningMode,
        vstProgNumber,
        numberOfPins,
        bypass
    };
}

#define MidiMsg(status, data1, data2) \
         ((((data2) << 16) & 0xFF0000) | \
          (((data1) << 8) & 0xFF00) | \
          ((status) & 0xFF))
#define MidiStatus(msg) ((msg) & 0xFF)
#define MidiData1(msg) (((msg) >> 8) & 0xFF)
#define MidiData2(msg) (((msg) >> 16) & 0xFF)

namespace MidiConst {
     enum Enum {
        codeMask = 0xf0,
        channelMask = 0x0f,

        noteOff =   0x80, //key, velocity
        noteOn =    0x90, //key, velocity
        aftertouch = 0xa0, //key, value
        ctrl =      0xb0, //ctrl, value
        prog =      0xc0, //prog, nd
        chanpressure = 0xd0, //value, nd
        pitchbend = 0xe0, //lsb, msb
        other = 0xf0 //nd,nd
    };
}

namespace DefaultLayers {
    enum Enum {
        host,
        project,
        program,
    };
}

namespace LearningMode {
    enum Enum {
        off,
        learn,
        unlearn
    };
}

//namespace Event {
//    const QEvent::Type ProgChange = QEvent::User;
//    const QEvent::Type UpdateSolver = QEvent::Type(QEvent::User+1);
//    const QEvent::Type PinMessage = QEvent::Type(QEvent::User+2);
//    const QEvent::Type Shortcut = QEvent::Type(QEvent::User+3);
//}

namespace InsertionType {
    enum Enum {
        NoInsertion,
        InsertBefore,
        InsertAfter,
        Replace,
        AddBefore,
        AddAfter
    };
}

namespace RemoveType {
    enum Enum {
        RemoveWithCables,
        BridgeCables
    };
}

namespace Connectables {
    class Object;
}
//typedef QHash<int, QWeakPointer<Connectables::Object> > hashObjects;
typedef QMap<int, QWeakPointer<Connectables::Object> > hashObjects;

#endif // GLOBALS_H
