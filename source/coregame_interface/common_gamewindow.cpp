///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "common_gamewindow.h"
#include "mapobjctrl.h"
#include "promote.h"
#include "unitobj.h"
#include "meshent.h"
#include "ray.h"
#include "terrain.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//

namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class GameWindow
  //

  
  // Static data
  GameWindow::EventCallBack *GameWindow::handler = NULL;

  //
  // Constructor
  //
  GameWindow::GameWindow(IControl *parent) : IControl(parent)
  {
  }


  //
  // RegisterHandler
  //
  // Register the game window event handler
  //
  GameWindow::EventCallBack *GameWindow::RegisterHandler(EventCallBack *function)
  {
    EventCallBack *old = handler;

    handler = function;
    return (old);
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 GameWindow::HandleEvent(Event &e)
  {
    #ifndef DEMO
      // Do we have a registered handler
      if (handler)
      {
        if (handler(e, this))
        {
          return (TRUE);
        }
      }
    #endif

    // Chain to base class
    return (IControl::HandleEvent(e));
  }


  //
  // AddToSelected
  //
  // Add a unit object to the given list (TRUE if added)
  //
  Bool GameWindow::AddToSelected(UnitObj *obj, UnitObjList &list, Team *team, Bool allowFoot, Bool selectParent)
  {
    ASSERT(obj);

    // Is the object on the map
    if (obj->OnMap())
    {
      // Should we select base unit
      if (selectParent)
      {
        while (obj->GetParent())
        {
          UnitObj *p = Promote::Object<UnitObjType, UnitObj>(obj->GetParent());

          if (p && p->UnitType()->IsSelectable())
          {
            obj = p;
          }
          else
          {
            break;
          }
        }
      }

      // Can we select this object
      if (obj && obj->UnitType()->IsSelectable() && (allowFoot || !obj->GetFootInstance()))
      {
        // No team, or it matches the object
        if (!team || (team == obj->GetTeam()))
        {
          // Add to the start of the list
          list.PrependNoDup(obj);
          return (TRUE);
        }
      }
    }

    return (FALSE);
  }


  //
  // PickObject
  //
  // Find an object at the screen pixel position x,y
  //
  MapObj * GameWindow::PickObject(S32 x, S32 y, SelectFilter *filter)
  {
    Vector pos;
    Vid::ScreenToCamera(pos, x, y);
    pos *= Vid::Math::farPlane;
    const Matrix &camMat = Vid::CurCamera().WorldMatrix();
    camMat.Transform(pos);

    BinTree<MapObj, F32> matches;
    F32 dist;

    for (NList<MapObj>::Iterator li(&MapObjCtrl::GetDisplayList()); *li; li++)
    {
      MapObj *obj = *li;

      // Do a poly check if it's not a unit or not selectable
      if (Ray::Test(obj, camMat.posit, pos, dist, obj->MapType()->GetRayTestFlags()))
      {
        // Filter the object
        if (filter)
        {
          obj = filter(obj);
        }

        // This object passed, add it to the tree
        if (obj)
        {
          matches.Add(dist, obj);
        }
      }
    }

    // The left most element of the tree is the closest object
    MapObj *o = matches.GetFirst();
    matches.UnlinkAll();

    return (o);
  }


  //
  // RectTest
  //
  // Returns TRUE if the given position is within the given rect
  //
  Bool GameWindow::RectTest(const Vector &position, const Area<S32> &rect, Vector &scr)
  {
    Vid::TransformFromWorld(scr, position);

    if (scr.z >= Vid::Math::nearPlane)
    {
      Vid::ProjectFromCamera(scr);
      
      if (scr.x >= rect.p0.x && scr.x < rect.p1.x && scr.y >= rect.p0.y && scr.y < rect.p1.y)
      {
        return (TRUE);
      }
    }
    
    return (FALSE);
  }


  //
  // SelectGroup
  //
  // Do a group object selection using the given screen rectangle
  //
  Bool GameWindow::SelectGroup(const Area<S32> &rect, MapObjList &list, SelectFilter *filter)
  {
    // Check all visible objects
    for (NList<MapObj>::Iterator li(&MapObjCtrl::GetDisplayList()); *li; li++)
    {
      Vector scr;
      if (RectTest((*li)->Position(), rect, scr))
      {
        if (!filter || (filter && filter(*li)))
        {
          list.PrependNoDup(*li);
        }
      }
    }

    return (list.GetCount() ? TRUE : FALSE);
  }


  //
  // SelectGroup
  //
  // Do a group object selection using the given screen rectangle
  //
  Bool GameWindow::SelectGroup(const Area<S32> &rect, UnitObjList &list, Team *team)
  {
    // Check all visible objects
    for (NList<MapObj>::Iterator li(&MapObjCtrl::GetDisplayList()); *li; li++)
    {
      // Promote to a unit
      UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(*li);

      if (unitObj)
      {
        Vector scr;

        if (RectTest(unitObj->Position(), rect, scr))
        {
          AddToSelected(unitObj, list, team, FALSE);
        }
      }
    }

    return (list.GetCount() ? TRUE : FALSE);
  }


  //
  // FindClosest
  //
  // Returns the closest object to the centre of, and within, 'rect'
  //
  MapObj * GameWindow::FindClosest(const Area<S32> &rect)
  {
    MapObj *closest = NULL;
    F32 distance = 0.0F;

    // Check all visible objects
    for (NList<MapObj>::Iterator li(&MapObjCtrl::GetDisplayList()); *li; li++)
    {
      // Promote to a unit
      UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(*li);

      // Only consider selectable objects
      if (unitObj && unitObj->UnitType()->IsSelectable())
      {
        // Ignore if on the display team
        if (unitObj->GetTeam() != Team::GetDisplayTeam())
        {
          Vector scr;
          if (RectTest(unitObj->Position(), rect, scr))
          {
            F32 deltaX = (F32)fabs(((rect.p0.x + rect.p1.x) >> 1) - scr.x);
            F32 deltaY = (F32)fabs(((rect.p0.y + rect.p1.y) >> 1) - scr.y);
            F32 newDistance = deltaX * deltaX + deltaY * deltaY;

            // Is it the first found OR the closest so far
            if (!closest || (newDistance < distance))
            {
              closest = unitObj;
              distance = newDistance;
            }
          }
        }
      }
    }

    return (closest);
  }


  //
  // TerrainPosition
  //
  // Returns the terrain cell and metre position at screen x,y (or NULL)
  //
  Bool GameWindow::TerrainPosition(S32 x, S32 y, Vector &pos)
  {
    // Just call the terrain system
    return (TerrainData::ScreenToTerrainWithWater(x, y, pos));
  }


  //
  // HasProperty
  //
  // Returns TRUE if this game window has the specified property
  //
  Bool GameWindow::HasProperty(U32 propertyCrc)
  {
    switch (propertyCrc)
    {
      case 0xB665088B: // "Construction"
        return (TRUE);

      default :
        return (FALSE);
    }
  }
}
