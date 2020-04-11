#ifndef VST3CONTEXTMENU_H
#define VST3CONTEXTMENU_H


#include <QWidget>
//#include <pluginterfaces/vst/ivstcontextmenu.h>

//namespace View {


//using namespace Steinberg;


//    class Vst3ContextMenu :
//            public QWidget,
//            public Vst::IContextMenu
//    {
//        Q_OBJECT

//    public:
//        Vst3ContextMenu(QWidget *parent = 0);

//        /** Gets the number of menu items. */
//        int32 PLUGIN_API getItemCount ();

//        /** Gets a menu item and its target (target could be not assigned). */
//        tresult PLUGIN_API getItem (int32 index, Item& item /*out*/, Vst::IContextMenuTarget** target /*out*/);

//        /** Adds a menu item and its target. */
//        tresult PLUGIN_API addItem (const Item& item, Vst::IContextMenuTarget* target);

//        /** Removes a menu item. */
//        tresult PLUGIN_API removeItem (const Item& item, Vst::IContextMenuTarget* target);

//        /** Pop-ups the menu. Coordinates are relative to the top-left position of the Plug-ins view. */
//        tresult PLUGIN_API popup (UCoord x, UCoord y) ;

//        /** Query for a pointer to the specified interface.
//        Returns kResultOk on success or kNoInterface if the object does not implement the interface.
//        The object has to call addRef when returning an interface.
//        \param _iid : (in) 16 Byte interface identifier (-> FUID)
//        \param obj : (out) On return, *obj points to the requested interface */
//        tresult PLUGIN_API queryInterface (const TUID _iid, void** obj);

//        /** Adds a reference and return the new reference count.
//        \par Remarks:
//            The initial reference count after creating an object is 1. */
//        uint32 PLUGIN_API addRef ();

//        /** Releases a reference and return the new reference count.
//        If the reference count reaches zero, the object will be destroyed in memory. */
//        uint32 PLUGIN_API release ();
//    };
//}

#endif // VST3CONTEXTMENU_H
