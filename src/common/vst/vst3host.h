#ifndef VST3HOST_H
#define VST3HOST_H

//#include "../precomp.h"
//#include "const.h"


#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif
// #include "public.sdk/source/vst/hosting/hostclasses.h"

#include "public.sdk/source/vst/hosting/pluginterfacesupport.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/vsttypes.h"
#ifdef VST24SDK
    #include "pluginterfaces/vst2.x/aeffectx.h"
#endif
#ifdef _MSC_VER
#pragma warning( pop )
#endif




using namespace Steinberg;
using namespace Vst;

class Vst3Host : public QObject
    , public IHostApplication
    , public IPlugInterfaceSupport
{
    Q_OBJECT

public:
    Vst3Host(QObject *parent);
    virtual ~Vst3Host () noexcept {FUNKNOWN_DTOR}

    tresult PLUGIN_API getName (String128 name) override;
    tresult PLUGIN_API createInstance (TUID cid, TUID _iid, void** obj) override;

    DECLARE_FUNKNOWN_METHODS

    PlugInterfaceSupport* getPlugInterfaceSupport () const;

    void UpdateTime(long buffSize, float sampleRate);
    float GetCurrentBarTic();
    void SetTempo(int tempo, int sign1, int sign2);
    void GetTempo(int &tempo, int &sign1, int &sign2);
    void SetTimeInfo(const Vst::ProcessContext *info);
//    void GetTimeInfo(VstTimeInfo *info);

    Vst::ProcessContext processContext;
  //  VstTimeInfo vst2TimeInfo;
    int currentBar;
    int loopLenght;

    tresult	isPlugInterfaceSupported (const TUID _iid) override;
    void addPlugInterfaceSupported (const TUID _iid);
    bool removePlugInterfaceSupported (const TUID _iid);

private:
    IPtr<PlugInterfaceSupport> mPlugInterfaceSupport;
    std::vector<FUID> mFUIDArray;

public slots:
    void SetSampleRate(float rate=44100.0);
};

class HostAttributeList final : public Vst::IAttributeList
{
public:
    /** make a new attribute list instance */
    static IPtr<Vst::IAttributeList> make ();

    tresult PLUGIN_API setInt (AttrID aid, int64 value) override;
    tresult PLUGIN_API getInt (AttrID aid, int64& value) override;
    tresult PLUGIN_API setFloat (AttrID aid, double value) override;
    tresult PLUGIN_API getFloat (AttrID aid, double& value) override;
    tresult PLUGIN_API setString (AttrID aid, const Vst::TChar* string) override;
    tresult PLUGIN_API getString (AttrID aid, Vst::TChar* string, uint32 sizeInBytes) override;
    tresult PLUGIN_API setBinary (AttrID aid, const void* data, uint32 sizeInBytes) override;
    tresult PLUGIN_API getBinary (AttrID aid, const void*& data, uint32& sizeInBytes) override;

    virtual ~HostAttributeList () noexcept;
DECLARE_FUNKNOWN_METHODS
    private:
              HostAttributeList ();

    struct Attribute;
    std::map<std::string, Attribute> list;
};


class HostMessage final : public Vst::IMessage
{
public:
    HostMessage ();
    virtual ~HostMessage () noexcept;

    const char* PLUGIN_API getMessageID () override;
    void PLUGIN_API setMessageID (const char* messageID) override;
    Vst::IAttributeList* PLUGIN_API getAttributes () override;

DECLARE_FUNKNOWN_METHODS
    private:
              char* messageId {nullptr};
    IPtr<Vst::IAttributeList> attributeList;
};

#endif // VST3HOST_H
