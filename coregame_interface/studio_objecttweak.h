///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// ObjectTweak
//
// 15-SEP-1999
//


#ifndef __STUDIO_OBJECTTWEAK_H
#define __STUDIO_OBJECTTWEAK_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ObjectTweak
  //
  namespace ObjectTweak
  {


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class BaseObj
    //
    class BaseObj : public IControl
    {
      PROMOTE_LINK(ObjectTweak::BaseObj, IControl, 0x2CFA6E13); // "ObjectTweak::BaseObj"

    public:

      // Constructor
      BaseObj(IControl *parent);
      ~BaseObj();

      // Draw the control
      void DrawSelf(PaintInfo &pi);

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class MapObj
    //
    class MapObj : public BaseObj
    {
      PROMOTE_LINK(MapObj, BaseObj, 0x8CC5AE1A); // "ObjectTweak::MapObj"

    protected:

      // Hitpoints
      IFaceVar *hitPoints;

      // Armour
      IFaceVar *armour;

    protected:

      // Notification that a local var has changed value
      void Notify(IFaceVar *var);

    public:

      // Constructor
      MapObj(IControl *parent);
      ~MapObj();

      // Activate and deactivate this control
      Bool Activate();
      Bool Deactivate();

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class UnitObj
    //
    class UnitObj : public BaseObj
    {
      PROMOTE_LINK(UnitObj, BaseObj, 0xC12DA4B3); // "ObjectTweak::UnitObj"

    protected:

      // Hitpoints
      IFaceVar *team;

    protected:

      // Notification that a local var has changed value
      void Notify(IFaceVar *var);

    public:

      // Constructor
      UnitObj(IControl *parent);
      ~UnitObj();

      // Activate and deactivate this control
      Bool Activate();
      Bool Deactivate();

    };


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class ResourceObj
    //
    class ResourceObj : public BaseObj
    {
      PROMOTE_LINK(ResourceObj, BaseObj, 0x49D041A5); // "ObjectTweak::ResourceObj"

    protected:

      // Resource
      IFaceVar *resource;

    protected:

      // Notification that a local var has changed value
      void Notify(IFaceVar *var);

    public:

      // Constructor
      ResourceObj(IControl *parent);
      ~ResourceObj();

      // Activate and deactivate this control
      Bool Activate();
      Bool Deactivate();

    };

  }

}

#endif
