///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_brush_tag.h"
#include "tagobj.h"
#include "gameobjctrl.h"
#include "viewer.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Tag - Brush for managing tags
    //

    //
    // Constructor
    //
    Tag::Tag(const char *name) 
    : Objects(name)
    {
      // Create interface vars
      varTag = CreateString("tag", "");
      varTagEdit = CreateString("tagedit", "");

      // Last Count
      lastCount = 0;
    }


    //
    // Destructor
    //
    Tag::~Tag()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Tag::HasProperty(U32 property)
    {
      switch (property)
      {
        case 0x86405F56: // "DisplaySelected"
          return (TRUE);
      }

      return (Objects::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Tag::NotifyVar(IFaceVar *var)
    {
      // Pass notification down
      Objects::NotifyVar(var);
    }


    
    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Tag::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x44D1C3B4: // "System::PostCycleInfoPoll"
          if (data.sList.GetCount() == lastCount)
          {
            break;
          }
          // Intentional fallthrough

        case 0x5AB0F116: // "System::Activated"
          lastCount = data.sList.GetCount();
          if (lastCount)
          {
            config->SendNotify(config, Crc::CalcStr("Brush::Tag::Notify::Valid"));
          }
          else
          {
            config->SendNotify(config, Crc::CalcStr("Brush::Tag::Notify::Invalid"));
          }
          break;

        case 0x247D95B7: // "Brush::Tag::Message::Create"
          Create(varTagEdit->GetStringValue());
          return;

        case 0x3F712266: // "Brush::Tag::Message::Delete"
          Delete();
          return;

        case 0x19A6FEA4: // "Brush::Tag::Message::Apply"
          Create(varTag->GetStringValue());
          return;

        case 0xA7A6E997: // "Brush::Tag::Message::Select"
        {
          TagObj *obj = TagObj::FindTag(varTag->GetStringValue());

          if (obj)
          {
            data.sList.Dup(obj->list);
          }
          return;
        }

        case 0xD4B976BF: // "Brush::Tag::Message::Clear"
        {
          data.sList.Clear();
          return;
        }
      }

      // Not blocked at this level
      Objects::Notification(crc, e);
    }


    //
    // Tag::Clear
    //
    void Tag::Clear()
    {
      if (config.Alive())
      {
      }
    }


    //
    // Tag::Create
    //
    void Tag::Create(const char *name)
    {
      if (Utils::Strlen(name) > 0)
      {
        if (TagObj::CreateTag(name, data.sList))
        {
          if (config.Alive())
          {
            config->SendNotify(config, Crc::CalcStr("Brush::Tag::Notify::Created"));
          }
        }
      }
    }


    //
    // Tag::Delete
    //
    void Tag::Delete()
    {
      // Find the tag
      TagObj *obj = TagObj::FindTag(varTag->GetStringValue());

      if (obj)
      {
        GameObjCtrl::MarkForDeletion(obj);

        if (config.Alive())
        {
          config->SendNotify(config, Crc::CalcStr("Brush::Tag::Notify::Deleted"));
        }
      }
    }

  }
}
