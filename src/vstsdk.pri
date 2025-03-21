SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/hostclasses.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/pluginterfacesupport.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/memorystream.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/processdata.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/parameterchanges.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module.cpp
win32 {
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_win32.cpp
}
#SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_linux.cpp
#SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/module_mac.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/stringconvert.cpp

SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/plugprovider.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/connectionproxy.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/hosting/eventlist.cpp
#SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vsteditcontroller.cpp

#LIBDEPS = sdk base pluginterfaces
#for(a, LIBDEPS) {
#    LIBS += -L$$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR} -l$${LIBPREFIX}$${a}
#    PRE_TARGETDEPS += $$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR}/$${LIBPREFIX}$${a}.$${LIBEXT}
#    DEPENDPATH += $$VST3SDK_PATH/$${VSTLIB}/lib/$${MSBUILDDIR}/$${a}
#}

#vst3 base
SOURCES += $$VST3SDK_PATH/base/source/baseiids.cpp
HEADERS += $$VST3SDK_PATH/base/source/classfactoryhelpers.h
SOURCES += $$VST3SDK_PATH/base/source/fbuffer.cpp
HEADERS += $$VST3SDK_PATH/base/source/fbuffer.h
HEADERS += $$VST3SDK_PATH/base/source/fcleanup.h
HEADERS += $$VST3SDK_PATH/base/source/fcommandline.h
SOURCES += $$VST3SDK_PATH/base/source/fdebug.cpp
HEADERS += $$VST3SDK_PATH/base/source/fdebug.h
SOURCES += $$VST3SDK_PATH/base/source/fdynlib.cpp
HEADERS += $$VST3SDK_PATH/base/source/fdynlib.h
SOURCES += $$VST3SDK_PATH/base/source/fobject.cpp
HEADERS += $$VST3SDK_PATH/base/source/fobject.h
SOURCES += $$VST3SDK_PATH/base/source/fstreamer.cpp
HEADERS += $$VST3SDK_PATH/base/source/fstreamer.h
SOURCES += $$VST3SDK_PATH/base/source/fstring.cpp
HEADERS += $$VST3SDK_PATH/base/source/fstring.h
SOURCES += $$VST3SDK_PATH/base/source/timer.cpp
HEADERS += $$VST3SDK_PATH/base/source/timer.h
SOURCES += $$VST3SDK_PATH/base/source/updatehandler.cpp
HEADERS += $$VST3SDK_PATH/base/source/updatehandler.h
HEADERS += $$VST3SDK_PATH/base/thread/include/fcondition.h
HEADERS += $$VST3SDK_PATH/base/thread/include/flock.h
SOURCES += $$VST3SDK_PATH/base/thread/source/fcondition.cpp
SOURCES += $$VST3SDK_PATH/base/thread/source/flock.cpp

#HEADERS += $$VST3SDK_PATH/base/source/basefwd.h
#SOURCES += $$VST3SDK_PATH/base/source/classfactory.cpp
#HEADERS += $$VST3SDK_PATH/base/source/classfactory.h
#SOURCES += $$VST3SDK_PATH/base/source/fbitset.cpp
#HEADERS += $$VST3SDK_PATH/base/source/fbitset.h
#HEADERS += $$VST3SDK_PATH/base/source/fcontainer.h
#SOURCES += $$VST3SDK_PATH/base/source/fcpu.cpp
#HEADERS += $$VST3SDK_PATH/base/source/fcpu.h
#SOURCES += $$VST3SDK_PATH/base/source/fcriticalperformance.cpp
#HEADERS += $$VST3SDK_PATH/base/source/fcriticalperformance.h
#SOURCES += $$VST3SDK_PATH/base/source/finitializer.cpp
#HEADERS += $$VST3SDK_PATH/base/source/finitializer.h
#SOURCES += $$VST3SDK_PATH/base/source/fmemory.cpp
#HEADERS += $$VST3SDK_PATH/base/source/fmemory.h
#SOURCES += $$VST3SDK_PATH/base/source/fpoint.cpp
#HEADERS += $$VST3SDK_PATH/base/source/fpoint.h
#SOURCES += $$VST3SDK_PATH/base/source/frect.cpp
#HEADERS += $$VST3SDK_PATH/base/source/frect.h
#SOURCES += $$VST3SDK_PATH/base/source/fregion.cpp
#HEADERS += $$VST3SDK_PATH/base/source/fregion.h
#HEADERS += $$VST3SDK_PATH/base/source/fstdmethods.h
#HEADERS += $$VST3SDK_PATH/base/source/fstringmethods.h
#HEADERS += $$VST3SDK_PATH/base/source/fstringstream.h
#HEADERS += $$VST3SDK_PATH/base/source/funknownfactory.h
#HEADERS += $$VST3SDK_PATH/base/source/hexbinary.h
#SOURCES += $$VST3SDK_PATH/base/source/istreamwrapper.cpp
#HEADERS += $$VST3SDK_PATH/base/source/istreamwrapper.h
#HEADERS += $$VST3SDK_PATH/base/source/tringbuffer.h
#HEADERS += $$VST3SDK_PATH/base/thread/include/fatomic.h
#HEADERS += $$VST3SDK_PATH/base/thread/include/forwarddeclarations.h
#HEADERS += $$VST3SDK_PATH/base/thread/include/frwlock_generic.h
#HEADERS += $$VST3SDK_PATH/base/thread/include/frwlock_macosx.h
#HEADERS += $$VST3SDK_PATH/base/thread/include/frwlock_windows.h
#HEADERS += $$VST3SDK_PATH/base/thread/include/fthread.h
#SOURCES += $$VST3SDK_PATH/base/thread/source/fatomic.cpp
#SOURCES += $$VST3SDK_PATH/base/thread/source/fthread.cpp

#vst3 sdk
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/commoniids.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/pluginview.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/common/pluginview.h
unix {
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/threadchecker_linux.cpp
}
win32 {
SOURCES += $$VST3SDK_PATH/public.sdk/source/common/threadchecker_win32.cpp
}
HEADERS += $$VST3SDK_PATH/public.sdk/source/common/threadchecker.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/main/pluginfactory.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/main/pluginfactory.h
#mac
#SOURCES += $$VST3SDK_PATH/public.sdk/source/common/threadchecker_mac.mm

HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/utility/ringbuffer.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstaudioeffect.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstaudioeffect.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstbus.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstbus.h
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstbypassprocessor.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstcomponent.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstcomponent.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstcomponentbase.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstcomponentbase.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vsteditcontroller.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vsteditcontroller.h
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vsteventshelper.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstinitiids.cpp
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstnoteexpressiontypes.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstnoteexpressiontypes.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstparameters.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstparameters.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstpresetfile.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstpresetfile.h
SOURCES += $$VST3SDK_PATH/public.sdk/source/vst/vstrepresentation.cpp
HEADERS += $$VST3SDK_PATH/public.sdk/source/vst/vstrepresentation.h

#vst3 pluginterfaces
SOURCES += $$VST3SDK_PATH/pluginterfaces/base/conststringtable.cpp
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/conststringtable.h
SOURCES += $$VST3SDK_PATH/pluginterfaces/base/coreiids.cpp
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/falignpop.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/falignpush.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/fplatform.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/fstrdefs.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ftypes.h
SOURCES += $$VST3SDK_PATH/pluginterfaces/base/funknown.cpp
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/funknown.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/futils.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/fvariant.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/geoconstants.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ibstream.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/icloneable.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ierrorcontext.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ipersistent.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ipluginbase.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/istringresult.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/iupdatehandler.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/keycodes.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/pluginbasefwd.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/smartpointer.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/typesizecheck.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ucolorspec.h
SOURCES += $$VST3SDK_PATH/pluginterfaces/base/ustring.cpp
HEADERS += $$VST3SDK_PATH/pluginterfaces/base/ustring.h

HEADERS += $$VST3SDK_PATH/pluginterfaces/gui/iplugview.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/gui/iplugviewcontentscalesupport.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstattributes.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstaudioprocessor.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstautomationstate.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstchannelcontextinfo.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstcomponent.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstcontextmenu.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivsteditcontroller.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstevents.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivsthostapplication.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstinterappaudio.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstmessage.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstmidicontrollers.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstmidilearn.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstnoteexpression.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstparameterchanges.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstphysicalui.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstplugview.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstprefetchablesupport.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstprocesscontext.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstpluginterfacesupport.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstrepresentation.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/ivstunits.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/vstpresetkeys.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/vstpshpack4.h
HEADERS += $$VST3SDK_PATH/pluginterfaces/vst/vsttypes.h
