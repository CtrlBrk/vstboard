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
#ifndef VST3HOST_H
#define VST3HOST_H

#ifdef _MSC_VER
#pragma warning( push, 1 )
#endif
// #include "public.sdk/source/vst/hosting/hostclasses.h"

#include "public.sdk/source/vst/hosting/pluginterfacesupport.h"
#include "pluginterfaces/vst/ivsthostapplication.h"
#include "pluginterfaces/vst/ivstprocesscontext.h"
#include "pluginterfaces/vst/vsttypes.h"
//#include "pluginterfaces/vst/ivsteditcontroller.h"

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

    std::atomic<float> barTic{0};
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
