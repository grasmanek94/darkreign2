///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// General Client Feature Systems
//
// 10-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_private.h"
#include "trailobj.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client - Controls client side interface
//

namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Trail - Trail management
  //
  namespace Trail
  {
    // Is the system active
    static Bool active;

    // Is a trail currently being constructed
    static Bool constructing;

    // The mode of the trail under construction
    static TrailObj::Mode mode;

    // Local waypoint list
    static TrailObj::WayPointList localList;

    // The last trail the mouse was over
    static TrailObjPtr mouseOverTrail;

    // The index to the point the mouse is over
    static U32 mouseOverIndex;

    // Currently selected trail
    static TrailObjPtr selectedTrail;


    //
    // SelectedTrail
    //
    // Returns the currently selected trail, or NULL
    //
    static TrailObj * SelectedTrail()
    {
      return (selectedTrail.Alive() ? selectedTrail.GetData() : NULL);
    }


    //
    // Active
    //
    // Is the system currently active
    //
    Bool Active()
    {
      return (active);
    }


    //
    // GetPrimaryEvent
    //
    // Returns the primary event
    //
    PrimaryEvent GetPrimaryEvent()
    {
      ASSERT(Active())

      // Mouse not over an object, and over the terrain
      if (!data.cInfo.o.map.Alive() && data.cInfo.t.cell)
      {
        // Is the mouse over an existing trail
        mouseOverTrail = TrailObj::Find
        (
          data.cInfo.t.cellX, data.cInfo.t.cellZ, Team::GetDisplayTeam(), &mouseOverIndex
        );

        // Are we currently constructing a trail
        if (constructing)
        {
          // Do not allow creation of points on other points
          if (!mouseOverTrail.Alive() && !localList.Find(data.cInfo.t.cellX, data.cInfo.t.cellZ))
          {
            return (PE_TRAILPOINT);
          }
          else
          {
            return (PE_TRAILNOPOINT);
          }
        }
        else

        if (mouseOverTrail.Alive())
        {
          if (data.sList.GetCount())
          {
            return (PE_TRAILAPPLY);
          }
          else
          {
            return (PE_TRAILSELECT);
          }
        }
      }

      return (PE_NONE);
    }


    //
    // TriggerEvent
    //
    // Triggers the given event
    //
    Bool TriggerEvent(U32 event, U32 value)
    {
      switch (event)
      {
        case 0x8744485A: // "Discrete::ToggleActive"
        {
          // Flip the switch
          active = !active;

          if (data.controls.trails.Alive())
          {
            if (active)
            {
              data.controls.trails->Activate();
            }
            else
            {
              data.controls.trails->Deactivate();
            }
          }
          break;
        }

        case 0x610FB9A8: // "Discrete::DeleteLast"
        {
          if (Active())
          {
            // Unlink the last point
            TrailObj::WayPoint *p = localList.UnlinkTail();

            if (p)
            {
              delete p;
            }
          }
          break;
        }

        case 0x2C602158: // "Discrete::DeleteLocal"
        {
          if (Active())
          {
            localList.DisposeAll();
          }
          break;
        }

        case 0x8EE282A4: // "Discrete::DeleteSelected"
        {
          if (Active() && selectedTrail.Alive())
          {
            Orders::Game::Trail::Delete(GetPlayer(), selectedTrail->Id());           
            selectedTrail = NULL;
          }
          break;
        }

        case 0x0D432C0B: // "Discrete::CancelConstruction"
        {
          if (Active())
          {
            constructing = FALSE;
            localList.DisposeAll();
          }
          break;
        }

        case 0x9012B6B2: // "Discrete::Create"
        {
          if (Active() && constructing && localList.GetCount())
          {
            Orders::Game::Trail::Create(GetPlayer(), mode);
            Orders::Game::TrailPoints::Generate(GetPlayer(), localList);

            constructing = FALSE;
            localList.DisposeAll();
          }

          break;
        }

        case 0x21C3C89F: // "Primary::Select"
        {
          ASSERT(Active())
          ASSERT(mouseOverTrail.Alive())

          selectedTrail = mouseOverTrail;
          break;
        }

        case 0x1B71FF8E: // "Primary::Point"
        {
          ASSERT(Active())
          ASSERT(data.cInfo.t.cell)

          // Add the clicked cell to the list
          localList.AppendPoint(data.cInfo.t.cellX, data.cInfo.t.cellZ);
          break;
        }

        case 0x407AC5DE: // "Primary::Apply"
        {
          ASSERT(Active())
          ASSERT(mouseOverTrail.Alive())

          Events::UpdateSelectedLists();
          Orders::Game::Trail::Apply
          (
            GetPlayer(), mouseOverTrail->Id(), mouseOverIndex, TRUE, Orders::Modifier(value)
          );
          Events::TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        case 0xDA91B0DB: // "Secondary::Offer"
        {
          if (Active() && constructing)
          {
            if (localList.GetCount())
            {
              TriggerEvent(0x9012B6B2); // "Discrete::Create"
            }
            else
            {
              TriggerEvent(0x0D432C0B); // "Discrete::CancelConstruction"
            }
            return (TRUE);
          }
          else

          if (selectedTrail.Alive())
          {
            selectedTrail = NULL;
            return (TRUE);
          }
          return (FALSE);
        }
      }

      // Return value not used for this event
      return (FALSE);
    }

    
    //
    // StartConstruction
    //
    // Start trail construction
    //
    void StartConstruction(TrailObj::Mode m)
    {
      constructing = TRUE;
      selectedTrail = NULL;
      mode = m;
    }


    //
    // Render
    //
    // Display trails
    //
    void Render()
    {
      if (Active())
      {
        // Get the currently selected trail
        TrailObj *selected = SelectedTrail();

        // Setup the normal and selected colors
        Color normal(0.0F, 0.8F, 0.0F, 0.6F);
        Color select(0.0F, 0.0F, 0.8F, 0.6F);

        // Draw all current trails
        for (NList<TrailObj>::Iterator i(&TrailObj::GetTrails()); *i; i++)
        {
          // Get this trail
          TrailObj *trail = *i;

          if (trail->GetTeam() == Team::GetDisplayTeam())
          {
            Common::Display::WayPoints
            (
              trail->GetList(), (trail == selected) ? select : normal, trail->GetMode()
            );
          }
        }

        // Draw the edit points
        Common::Display::WayPoints(localList, Color(0.8F, 0.0F, 0.0F, 0.6F), mode);
      }
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      active = FALSE;
      constructing = FALSE;
      mouseOverTrail = NULL;
      selectedTrail = NULL;
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      localList.DisposeAll();
    }
  }
}
