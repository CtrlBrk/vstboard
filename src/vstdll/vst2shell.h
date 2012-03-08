#ifndef VST2SHELL_H
#define VST2SHELL_H

#include "audioeffectx.h"
#include "ids.h"

AudioEffect *createShell(audioMasterCallback audioMaster);

class Vst2Shell : public AudioEffectX
{
public:
    Vst2Shell(audioMasterCallback audioMaster);
    VstPlugCategory getPlugCategory();
    VstInt32 getNextShellPlugin(char *name);
    void processReplacing (float** /*inputs*/, float** /*outputs*/, VstInt32 /*sampleFrames*/);

private:
    long currentShellPlugin;
};

#endif // VST2SHELL_H
