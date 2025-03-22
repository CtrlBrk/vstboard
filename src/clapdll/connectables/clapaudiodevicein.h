#ifndef CLAPAUDIODEVICEIN_H
#define CLAPAUDIODEVICEIN_H

#include "precomp.h"
#include "connectables/object.h"
#include "connectables/objectinfo.h"

class AudioBuffer;
namespace Connectables {

class ClapAudioDeviceIn : public Object
{
    Q_OBJECT
public:
    ClapAudioDeviceIn(MainHost *myHost,int index, const ObjectInfo &info);
    ~ClapAudioDeviceIn();

    bool Open() override;
    bool Close() override;
    void Render() override;

    void SetBuffers(float **buf, int sampleFrames);
    void SetBuffersD(double **buf, int sampleFrames);

public slots:
    void SetBufferSize(qint32 size) override;

};

}
#endif // CLAPAUDIODEVICEIN_H
