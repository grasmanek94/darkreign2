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

#include "studio_brush_area.h"
#include "terrain.h"
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
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Area - Base for all terrain area-type brushes
    //

    //
    // Constructor
    //
    AreaBase::AreaBase(const char *name) 
    : Base(name),
      valid(FALSE)
    {
      captureThreshold[MOUSE_LEFT].Set(0, 0);
    }


    //
    // Destructor
    //
    AreaBase::~AreaBase()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool AreaBase::HasProperty(U32 property)
    {
      return (Base::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void AreaBase::NotifyVar(IFaceVar *var)
    {
      // Pass notification down
      Base::NotifyVar(var);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void AreaBase::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0xFEA6C563: // "Brush::LeftMouseCaptureStart"
        {
          Vector pos;

          if (TerrainData::ScreenToTerrain(e->input.mouseX, e->input.mouseY, pos))
          {
            // Metre threshold for selecting a corner point
            const F32 THRESHOLD = 4.0F;
            const F32 NOT_SET = F32_MAX;

            Point<F32> anchor(NOT_SET, NOT_SET);

            // Clamp to world limits
            WorldCtrl::ClampMetreMapX(pos.x);
            WorldCtrl::ClampMetreMapZ(pos.z);

            Point<F32> p(pos.x, pos.z);

            // Is it near a corner? 
            if (valid)
            {
              if (fabs(p.x - metreArea.p0.x) < THRESHOLD)
              {
                anchor.x = metreArea.p1.x;
              }
              else

              if (fabs(p.x - metreArea.p1.x) < THRESHOLD)
              {
                anchor.x = metreArea.p0.x;
              }

              if (fabs(p.z - metreArea.p0.z) < THRESHOLD)
              {
                anchor.z = metreArea.p1.z;
              }
              else

              if (fabs(p.z - metreArea.p1.z) < THRESHOLD)
              {
                anchor.z = metreArea.p0.z;
              }
            }

            // Use anchor and new end point
            if (anchor.x != NOT_SET && anchor.z != NOT_SET)
            {
              metreArea.p0 = anchor;
              metreArea.p1 = p;
            }
            else
            {
              // Start a new region
              metreArea.p0 = p;
              metreArea.p1 = p;
            }
            valid = TRUE;
          }
          else
          {
            valid = FALSE;
          }

          // Pass event on
          return;
        }

        case 0xE6428DAD: // "Brush::LeftMouseCaptureMove"
        {
          if (valid)
          {
            Vector pos;

            if (TerrainData::ScreenToTerrain(e->input.mouseX, e->input.mouseY, pos))
            {
              // Clamp to world limits
              WorldCtrl::ClampMetreMapX(pos.x);
              WorldCtrl::ClampMetreMapZ(pos.z);

              // Set end position
              metreArea.p1.Set(pos.x, pos.z);
            }
          }

          // Block this event
          return;
        }

        case 0x252BA28C: // "Brush::LeftMouseCaptureEnd"
        {
          if (valid)
          {
            // Generate a notification
            Notify("Brush::Area::Changed");
          }
          
          // Block this event
          return;
        }

        case 0x0ACEA1BD: // "Brush::RightMouseClick"
        {
          // Deselect region
          valid = FALSE;

          // Generate a notification
          Notify("Brush::Area::Deselected");

          // Block this event
          return;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          // Should we display the terrain brush
          if (valid)
          {
            // MarkAreaMetre requires a sorted area
            Area<F32> f = metreArea;
            f.Sort();

            // Draw the brush
            Common::Display::MarkAreaMetre(f);
          }

          // Always pass system events down
          break;
        }
      }

      // Not blocked at this level
      Base::Notification(crc, e);
    }
  }
}
