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

#ifdef SCRIPTENGINE
#include "script.h"
#include "../mainhost.h"
#include "../mainwindow.h"

using namespace Connectables;

//scripts cannot be multithreaded ?
QMutex Script::mutexScript;

Script::Script(MainHost *host, int index, const ObjectInfo &info) :
    Object(host,index,info),
    editorWnd(0)
{
    setObjectName( QString("objScript%1").arg(index) );

    listParameterPinIn->AddPin(FixedPinNumber::editorVisible);

    connect(this, SIGNAL(_dspMsg(QString,QString)),
            this, SLOT(DspMsg(QString,QString)),
            Qt::QueuedConnection);

    editorWnd = new View::ScriptEditor(myHost->mainWindow);
    editorWnd->SetObject(this);
    connect(editorWnd, SIGNAL(Execute(QString)),
            this,SLOT(ReplaceScript(QString)));
    connect(this,SIGNAL(HideEditorWindow()),
            editorWnd,SLOT(hide()),
            Qt::QueuedConnection);
    connect(editorWnd, SIGNAL(Hide()),
            this, SLOT(OnEditorClosed()));
    connect(editorWnd, SIGNAL(destroyed()),
            this, SLOT(EditorDestroyed()));
    connect(this,SIGNAL(_addLog(QString)),
            editorWnd, SLOT(AddToLog(QString)));

    defaultScript = "({\n"
            "//called when the script is added to a panel, on program change\n"
            "open: function(){\n"
            "   this.listParameterPinIn.nbPins=1;\n"
            "   this.listParameterPinOut.nbPins=1;\n"
            "   this.listAudioPinIn.nbPins=1;\n"
            "   this.listAudioPinOut.nbPins=1;\n"
            "   this.listMidiPinIn.nbPins=1;\n"
            "   this.listMidiPinOut.nbPins=2;\n"
            "/*\n"
            "//load two plugins :\n"
            "   id1 = programContainer.AddObject('C:\\\\Program Files\\\\Common Files\\\\VST3\\\\mda-vst3.vst3','mda TestTone');\n"
            "   id2 = programContainer.AddObject('C:\\\\Program Files\\\\Common Files\\\\VST3\\\\adelay.vst3');\n"
            "//connect them together :\n"
            "   programContainer.ConnectObjects(id1,id2);\n"
            "//show some message :\n"
            "   this.alert(id1);\n"
            "//connect two pins :\n"
            "   programContainer.AddCable(this.ParamOut0, id1.Mode)\n"
            "*/\n"
            "},\n"
            "\n"
            "\n"
            "//called on every render loop\n"
            "render: function(){\n"
            "//copy values from input parameter to output :\n"
            "   this.ParamOut0.value=this.ParamIn0.value;\n"
            "//copy buffer content from in to out :\n"
            "   this.AudioOut0.buffer=this.AudioIn0.buffer;\n"
            "},\n"
            "\n"
            "\n"
            "//called on midi event\n"
            "midi_in: function(channel,command,val1,val2,commandname,raw){\n"
            "//add some information to the log :\n"
            "   //this.log(commandname + ' chan:' + channel + ' 1:' + val1 + ' 2:' + val2);\n"
            "//relay 'note on' to all outputs :\n"
            "   if(commandname=='noteon') {\n"
            "       this.SendMidiMsg(channel,command,val1,val2);\n"
            "   } else {\n"
            "//else, send the raw message to the first pin :\n"
            "       this.SendMidiMsg(raw,1);\n"
            "   }\n"
            "}\n"
            "})";

//    defaultScript = "({\n"
//            "open: function(){\n"
//            "},\n"
//            "render: function(){\n"
//            "},\n"
//            "midi_in: function(channel,command,val1,val2,commandname,raw){\n"
//            "}\n"
//            "})";
}

bool Script::Open()
{
//    CompileScrtipt();
    return true;
}

bool Script::CompileScrtipt()
{
    scriptThisObj = myHost->scriptEngine.newQObject(this);
//    QQmlEngine::setObjectOwnership(this, QQmlEngine::CppOwnership);

    {
        QMutexLocker ms(&mutexScript);

        if(scriptText=="")
            scriptText=defaultScript;

        objScript = myHost->scriptEngine.evaluate(scriptText);
        if(myHost->scriptEngine.hasUncaughtException()) {
            int line = myHost->scriptEngine.uncaughtExceptionLineNumber();
            log( tr("compile exception, line %1\n%2").arg(line).arg(objScript.toString()) );
            return false;
        }

        openScript = objScript.property("open");
        renderScript = objScript.property("render");
        midiinScript = objScript.property("midi_in");

        QScriptValue result = openScript.call(scriptThisObj);
        if(myHost->scriptEngine.hasUncaughtException()) {
            int line = myHost->scriptEngine.uncaughtExceptionLineNumber();
            log( tr("open exception, line %1\n%2").arg(line).arg(result.toString()) );
            return false;
        }
    }
    return true;
}

Script::~Script()
{
    Close();
    if(currentProgId!=EMPTY_PROGRAM)
        delete currentProgram;
}

bool Script::Close()
{
   if(editorWnd) {
        editorWnd->disconnect();
        editorWnd->SetObject(0);
        disconnect(editorWnd);
        QTimer::singleShot(0,editorWnd,SLOT(close()));
        editorWnd=0;
    }
    return true;
}

void Script::SendMidiMsg(long raw, int pin)
{
    if(pin!=-1) {
        listMidiPinOut->GetPin(pin)->SendMsg(PinMessage::MidiMsg,(void*)&raw);
    } else {

        foreach(Pin *pin,listMidiPinOut->listPins) {
            pin->SendMsg(PinMessage::MidiMsg,(void*)&raw);
        }
    }
}

void Script::SendMidiMsg(int chan, int command, int m1, int m2, int pin)
{
    SendMidiMsg( MidiMsg(command | chan, m1, m2), pin );
}

void Script::MidiMsgFromInput(long msg)
{
    if(!midiinScript.isError()) {

        quint8 command = MidiStatus(msg) & MidiConst::codeMask;
        qint8 chan = MidiStatus(msg) & MidiConst::channelMask;
        quint8 m1 = MidiData1(msg);
        quint8 m2 = MidiData2(msg);

        QString cmdType;
        switch(command) {
        case MidiConst::noteOff:
            cmdType="noteoff";
            break;
        case MidiConst::noteOn:
        cmdType="noteon";
        break;
        case MidiConst::aftertouch:
            cmdType="aftertouch";
            break;
        case MidiConst::ctrl:
            cmdType="control";
            break;
        case MidiConst::prog:
            cmdType="program";
            break;
        case MidiConst::chanpressure:
            cmdType="chanpressure";
            break;
        case MidiConst::pitchbend:
            cmdType="pitchbend";
            break;
        default:
            cmdType="other";
            break;
        }

        QScriptValueList args;
        args << chan;
        args << command;
        args << m1;
        args << m2;
        args << cmdType;

        QScriptValue result = midiinScript.call(scriptThisObj,args);
        if(myHost->scriptEngine.hasUncaughtException()) {
            int line = myHost->scriptEngine.uncaughtExceptionLineNumber();
            midiinScript = result;
            log( tr("midi_in exception, line %1\n%2").arg(line).arg(result.toString()) );
        }
    }
}

void Script::Render()
{
    QMutexLocker ms(&mutexScript);

    foreach(Pin *pin, listAudioPinIn->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->NewRenderLoop();
    }
    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->NewRenderLoop();
    }

    if(renderScript.isValid() && !renderScript.isError()) {
        QScriptValue result = renderScript.call(scriptThisObj);
        if(myHost->scriptEngine.hasUncaughtException()) {
            renderScript = result;

            int line = myHost->scriptEngine.uncaughtExceptionLineNumber();
            log( tr("render exception, line %1\n%2").arg(line).arg(result.toString()) );
        }
    }

    foreach(Pin *pin, listAudioPinOut->listPins) {
        static_cast<AudioPin*>(pin)->GetBuffer()->ConsumeStack();
        static_cast<AudioPin*>(pin)->SendAudioBuffer();
    }

}

void Script::log(const QString &str)
{
    emit _addLog(str);
}

void Script::alert(const QString &str)
{
    QThread *t = QThread::currentThread();
    QThread *maint = QApplication::instance()->thread();
    if(t != maint) {
        emit _addLog(str);
        return;
    }

    emit _dspMsg(objectName(),str);
}

void Script::DspMsg(const QString &title, const QString &str)
{
    QMessageBox msg(
        QMessageBox::Information,
        title,
        str
    );
    msg.exec();
}

void Script::ReplaceScript(const QString &str)
{
    scriptText = str;
    if(editorWnd)
        editorWnd->SetScript(scriptText);
    OnProgramDirty();
    CompileScrtipt();
}

Pin* Script::CreatePin(const ConnectionInfo &info)
{
    pinConstructArgs args(info);
    args.parent = this;

    if(info.type == PinType::Parameter) {
        if(info.direction == PinDirection::Input) {
            args.name += "ParamIn";
        } else {
            args.name += "ParamOut";
        }


        args.name = QString("%1%2").arg(args.name).arg(info.pinNumber);
        args.visible = true;
    }

    return Object::CreatePin(args);

//    Pin *newPin = Object::CreatePin(args);
//    if(newPin)
//        return newPin;

//    PinFactory::MakePin(args);

//    if(info.type == PinType::Parameter) {
//        switch(info.direction) {
//            case PinDirection::Input :
//                args.name = QString("ParamIn%1").arg(info.pinNumber);
//                return PinFactory::MakePin(args);
//            case PinDirection::Output :
//                args.name = QString("ParamOut%1").arg(info.pinNumber);
//                return PinFactory::MakePin(args);
//            default:
//                return 0;
//        }
//    }

//    return 0;
}

void Script::OnShowEditor()
{
    if(!editorWnd || editorWnd->isVisible())
        return;

    editorWnd->SetScript(scriptText);
    editorWnd->show();
}

void Script::OnHideEditor()
{
    if(!editorWnd || !editorWnd->isVisible())
        return;
    emit HideEditorWindow();
}

void Script::OnEditorClosed()
{
    ToggleEditor(false);
}

void Script::EditorDestroyed()
{
    editorWnd=0;
    ToggleEditor(false);
}

void Script::SaveProgram()
{
    if(!currentProgram || !currentProgram->IsDirty())
        return;

    Object::SaveProgram();

    currentProgram->listOtherValues.insert(0, scriptText);
}

void Script::LoadProgram(int prog)
{
    Object::LoadProgram(prog);
    if(!currentProgram)
        return;

    scriptText = currentProgram->listOtherValues.value(0,"").toString();
    CompileScrtipt();

    if(editorWnd && editorWnd->isVisible())
        editorWnd->SetScript(scriptText);

    //enable view update for the pins created by the script
    foreach(PinsList *lst, pinLists) {
        lst->EnableVuUpdates(true);
    }
}

#endif
