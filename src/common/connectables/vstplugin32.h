#ifndef VSTPLUGIN32_H
#define VSTPLUGIN32_H

#include "vstplugin.h"
#include "../../loaddll32/ipc32.h"
#include "../../loaddll32/ipc.h"

namespace Connectables {

class VstPlugin32 : public VstPlugin
{
    Q_OBJECT

public:
    VstPlugin32(MainHost *myHost,int index, const ObjectInfo & info);
    virtual ~VstPlugin32();

    bool Close();
    bool Load(const std::wstring &name) override;
    bool Unload();
    long EffDispatch(VstInt32 opCode, VstInt32 index=0, VstIntPtr value=0, void *ptr=0, float opt=0., VstInt32 size=0) override;
   // long EffEditOpen(void *ptr) override;
    void CreateEditorWindow() override;
    long EffEditGetRect(ERect **ptr) override;
    float EffGetParameter(long index) override;
    void EffSetParameter(long index, float parameter) override;
    void EffProcess(float **inputs, float **outputs, long sampleframes) override;
    void EffProcessReplacing(float **inputs, float **outputs, long sampleframes) override;
    void EffProcessDoubleReplacing(double **inputs, double **outputs, long sampleFrames) override;
    long EffGetChunk(void **ptr, bool isPreset = false)  override;

protected:
    bool initPlugin() override;
    bool GetChunkSegment(char *ptr, VstInt32 chunkSize);
    bool SendChunkSegment(char *ptr, VstInt32 chunkSize);
/*
    void Lock();
    void Process();
    void ProcessAndWaitResult();
    void UnlockAfterResult();
*/

private:
    static Ipc ipcTo32;
    static structTo32* dataTo32;
    static Ipc ipcFrom32;
    static structFrom32* dataFrom32;

    static HANDLE hMapFile;
    static HANDLE ipcMutex;
    static HANDLE ipcSemStart;
    static HANDLE ipcSemEnd;
    // static QByteArray ipcData;
    static char* chunkData;

public slots:
    void OnShowEditor() override;
    void OnHideEditor() override;
};

}
#endif // VSTPLUGIN32_H
