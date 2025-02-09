#ifndef VSTPLUGIN32_H
#define VSTPLUGIN32_H

#include "vstplugin.h"

namespace Connectables {


class VstPlugin32 : public VstPlugin
{
    Q_OBJECT

public:
    VstPlugin32(MainHost *myHost,int index, const ObjectInfo & info);
    virtual ~VstPlugin32();
    bool Open() override;

    bool Load(const std::wstring &name) override;
    bool Unload() override;
    long EffDispatch(VstInt32 opCode, VstInt32 index=0, VstIntPtr value=0, void *ptr=0, float opt=0., VstInt32 size=0) override;
    long EffEditOpen(void *ptr) override;
    void CreateEditorWindow() override;
    long EffEditGetRect(ERect **ptr) override;
    float EffGetParameter(long index) override;
    void EffSetParameter(long index, float parameter) override;
    void EffProcess(float **inputs, float **outputs, long sampleframes) override;
    void EffProcessReplacing(float **inputs, float **outputs, long sampleframes) override;
    void EffProcessDoubleReplacing(double **inputs, double **outputs, long sampleFrames) override;
protected:
    bool initPlugin() override;

private:

    static bool ProcessInit();
    static HANDLE hMapFile;
    static unsigned char* mapFileBuffer;
    static HANDLE ipcMutex;
    HANDLE ipcMutexStartProcess;
    // static QByteArray ipcData;
};

}
#endif // VSTPLUGIN32_H
