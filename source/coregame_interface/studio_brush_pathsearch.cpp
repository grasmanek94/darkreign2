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
#include "studio_brush_pathsearch.h"
#include "gameobjctrl.h"
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
    // Class PathSearch - Brush for path search testing
    //

    //
    // Constructor
    //
    PathSearch::PathSearch(const char *name) 
    : Base(name), 
      start(-1, -1), 
      end(-1, -1),
      rescan(FALSE)
    {
      // Create interface vars
      varType = CreateString("type", "");
      varOptimize = CreateInteger("optimize", FALSE, 0, 1);
    }


    //
    // Destructor
    //
    PathSearch::~PathSearch()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool PathSearch::HasProperty(U32 property)
    {
      return (Base::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void PathSearch::NotifyVar(IFaceVar *var)
    {
      if (var == varType)
      {
        // Change the current object type
        objType = GameObjCtrl::FindType<UnitObjType>(var->GetStringValue());
        rescan = TRUE;
      }
      else

      if (var == varOptimize)
      {
        rescan = TRUE;
      }

      // Pass notification down
      Base::NotifyVar(var);
    }

   
    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void PathSearch::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x281C22F7: // "System::DoneSimulation"
        {
          // Forget the path
          finder.ForgetPath();

          // Always pass system events down
          break;
        }

        case 0x27546BF9: // "Brush::LeftMouseClick"
        {
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            start.Set(data.cInfo.mTerrain.cellX, data.cInfo.mTerrain.cellZ);
            rescan = TRUE;
          }

          // Block this event
          return;
        }

        case 0x0ACEA1BD: // "Brush::RightMouseClick"
        {
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            end.Set(data.cInfo.mTerrain.cellX, data.cInfo.mTerrain.cellZ);
            rescan = TRUE;
          }

          // Block this event
          return;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Get the object under the cursor
          MapObj * obj = ObjectUnderCursor();

          if (obj)
          {
            varType->SetStringValue(obj->TypeName());
          }

          // Block this event
          return;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          if (rescan)
          {
            if (objType.Alive())
            {
              U32 flags = 0;
            
              if (varOptimize->GetIntegerValue())
              {
                flags |= ::PathSearch::Finder::RF_OPTIMIZE;
              }

              if (WorldCtrl::CellOnMap(start.x, start.z) && WorldCtrl::CellOnMap(end.x, end.z))
              {
                UnitObjType *unitType = Promote::Type<UnitObjType>(objType);

                if (unitType)
                {
                  finder.RequestPath
                  (
                    start.x, start.z, end.x, end.z, 
                    unitType->GetTractionIndex(unitType->GetDefaultLayer()),
                    NULL,
                    (::PathSearch::SearchType)unitType->GetMovementModel().GetPathingMethod(unitType->GetDefaultLayer()),
                    flags
                  );
                }
              }
            }
            rescan = FALSE;
          }

          if (finder.State() == ::PathSearch::FS_FOUND || finder.State() == ::PathSearch::FS_CLOSEST || finder.State() == ::PathSearch::FS_DIRECT)
          {
            Color g(0L, 255L, 0L, 255L);
            Color r(255L, 0L, 0L, 255L);

            Common::Display::MarkCell(start.x, start.z, g);
            Common::Display::MarkCell(end.x, end.z, r);

            for (::PathSearch::PointList::Iterator i(&finder.GetPointList()); *i; i++)
            {
              Area<S32> rc;
              rc.SetSize( 
                (*i)->x, (*i)->z,
                1, 1);

              // Draw the brush
              ::Terrain::RenderCellRect(rc, g, TRUE, FALSE);
            }
          }

          // Pass system events down
          break;
        }
      }

      // Not blocked at this level
      Base::Notification(crc, e);
    }
  }
}
