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

#include "studio_brush_region.h"
#include "regionobj.h"
#include "gameobjctrl.h"
#include "viewer.h"
#include "common.h"

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

    // Color of non-selected regions
    static Color inertColors[3] =
    {
      Color(160L, 160L, 160L, 255L),
      Color(192L, 192L, 192L, 255L),
      Color(128L, 128L, 128L, 255L),
    };

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Region - Region brush
    //

    //
    // Constructor
    //
    Region::Region(const char *name) : AreaBase(name)
    {
      // Create interface vars
      varRegion = CreateString("region", "");
      varRegionEdit = CreateString("regionedit", "");
    }


    //
    // Destructor
    //
    Region::~Region()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Region::HasProperty(U32 property)
    {
      return (AreaBase::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Region::NotifyVar(IFaceVar *var)
    {
      // Pass notification down
      AreaBase::NotifyVar(var);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Region::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x1B69FE7A: // "Brush::Area::Deselected"
        {
          Clear();

          // Pass Area brush notifications down
          break;
        }

        case 0x9860FB16: // "Brush::Area::Changed"
        {
          if (config.Alive())
          {
            // Notify the config control that the area is now valid
            config->SendNotify(config, Crc::CalcStr("Brush::Area::Notify::Valid"));
          }

          // Pass Area brush notifications down
          break;
        }

        case 0xC87CF179: // "Brush::Region::Message::Clear"
          Clear();
          return;

        case 0x98E713A3: // "Brush::Region::Message::Create"
          Create(varRegionEdit->GetStringValue());
          return;

        case 0x83EBA472: // "Brush::Region::Message::Delete"
          Delete();
          return;

        case 0x05637962: // "Brush::Region::Message::Apply"
          Create(varRegion->GetStringValue());
          return;

        case 0x71BE135A: // "Brush::Region::Message::Display"
          Show(FALSE);
          return;

        case 0x6B5E7424: // "Brush::Region::Message::Jumpto"
          Show(TRUE);
          return;

        case 0x5AB0F116: // "System::Activated"
        {
          Clear();

          // Always pass system events down
          break;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          RegionObj *obj = RegionObj::FindRegion(varRegion->GetStringValue());

          // Draw all non-selected regions in grey
          for (NList<RegionObj>::Iterator i(&RegionObj::allRegions); *i; i++)
          {
            if (*i != obj)
            {
              // MarkAreaMetre requires a sorted area
              Area<F32> f = (*i)->GetArea();
              f.Sort();

              // Draw the brush
              Common::Display::MarkAreaMetre(f, inertColors);
            }
          }

          // Always pass system events down
          break;
        }
      }

      // Not blocked at this level
      AreaBase::Notification(crc, e);
    }


    //
    // Region::Clear
    //
    void Region::Clear()
    {
      valid = FALSE;

      if (config.Alive())
      {
        // Notify the config control that the area is now invalid
        config->SendNotify(config, Crc::CalcStr("Brush::Area::Notify::Invalid"));
      }
    }


    //
    // Region::Create
    //
    void Region::Create(const char *name)
    {
      if (valid)
      {
        if (Utils::Strlen(name) > 0)
        {
          if (RegionObj::CreateRegion(name, metreArea))
          {
            if (config.Alive())
            {
              // Notify the config control that a region was created
              config->SendNotify(config, Crc::CalcStr("Brush::Region::Notify::Created"));
            }
          }
        }
      }
    }


    //
    // Region::Delete
    //
    void Region::Delete()
    {
      RegionObj *obj = RegionObj::FindRegion(varRegion->GetStringValue());

      if (obj)
      {
        GameObjCtrl::MarkForDeletion(obj);

        if (config.Alive())
        {
          // Notify the config control that a region was deleted
          config->SendNotify(config, Crc::CalcStr("Brush::Region::Notify::Deleted"));
        }
      }
    }


    //
    // Region::Show
    //
    void Region::Show(Bool jump)
    {
      RegionObj *obj = RegionObj::FindRegion(varRegion->GetStringValue());

      if (obj)
      {
        // Setup area 
        metreArea = obj->GetArea();
        valid = TRUE;

        // Move camera
        if (jump)
        {
          Viewer::GetCurrent()->LookAt(obj->GetMidPoint().x, obj->GetMidPoint().z);
        }

        // Notify control
        if (config.Alive())
        {
          config->SendNotify(config, Crc::CalcStr("Brush::Area::Notify::Changed"));
        }
      }
    }
  }
}
