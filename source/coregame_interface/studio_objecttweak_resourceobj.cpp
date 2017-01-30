///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// ObjectTweak - ResourceObj
//
// 15-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_objecttweak.h"
#include "studio_private.h"
#include "promote.h"
#include "resourceobj.h"


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
    // Class ResourceObj
    //


    //
    // ResourceObj::ResourceObj
    //
    ResourceObj::ResourceObj(IControl *parent)
    : BaseObj(parent)
    {
      // Create IFace Vars
      resource = new IFaceVar(this, CreateFloat("resource", 0.0f, 0.0f, 1.0f));
    }


    //
    // ResourceObj::~ResourceObj
    //
    ResourceObj::~ResourceObj()
    {
      // Delete IFace Vars
      delete resource;
    }


    //
    // ResourceObj::Notify
    //
    void ResourceObj::Notify(IFaceVar *var)
    {
      if (var == resource)
      {
        // Iterate the selected objects
        for (MapObjList::Iterator i(&data.sList); *i; i++)
        {
          if ((*i)->Alive())
          {
            ::ResourceObj *resourceObj = Promote::Object<::ResourceObjType, ::ResourceObj>(**i);

            if (resourceObj)
            {
              // Set the resource percentage
              resourceObj->SetResource(resource->GetFloatValue());
            }
          }
        }
      }
      else
      {
        IControl::Notify(var);
      }
    }


    //
    // ResourceObj::Activate
    //
    Bool ResourceObj::Activate()
    {
      if (IControl::Activate())
      {
        ActivateVar(resource);
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // ResourceObj::Deactivate
    //
    Bool ResourceObj::Deactivate()
    {
      if (IControl::Deactivate())
      {
        resource->Deactivate();
        return (TRUE);
      }
      else
      {
        return (FALSE);
      }
    }

  }

}
