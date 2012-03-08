#include "vst2shell.h"

AudioEffect *createShell(audioMasterCallback audioMaster)
{
    return new Vst2Shell(audioMaster);
}

Vst2Shell::Vst2Shell(audioMasterCallback audioMaster) :
    AudioEffectX(audioMaster,0,0),
    currentShellPlugin(0L)
{
}

VstPlugCategory Vst2Shell::getPlugCategory()
{
    return kPlugCategShell;
}

VstInt32 Vst2Shell::getNextShellPlugin(char *name)
{
    switch(currentShellPlugin++) {
    case 0:
        strcpy(name,"VstBoard Effect");
        return uniqueIDEffect;
    case 1:
        strcpy(name,"VstBoard Instrument");
        return uniqueIDInstrument;
    }
    return 0;
}

void Vst2Shell::processReplacing (float** , float** , VstInt32 )
{
}
