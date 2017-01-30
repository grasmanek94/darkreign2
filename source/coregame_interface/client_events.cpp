///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Events
//
// 19-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_private.h"
#include "iface_types.h"
#include "worldctrl.h"
#include "unitobj.h"
#include "team.h"
#include "iface.h"
#include "input.h"
#include "icmenu.h"
#include "promote.h"
#include "viewer.h"
#include "resourceobj.h"
#include "unitobjlist.h"
#include "mesh.h"
#include "meshent.h"
#include "common.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "tasks_unitattack.h"
#include "orders_squad.h"
#include "squadobj.h"
#include "client_construction.h"
#include "tasks_unitconstructor.h"
#include "multiplayer.h"
#include "client_construction.h"
#include "client_unitcontext.h"
#include "sight.h"
#include "restoreobj.h"
#include "tasks_restorestatic.h"
#include "transportobj.h"
#include "tasks_unitboard.h"
#include "particlesystem.h"
#include "particle.h"
#include "random.h"
#include "sound.h"
#include "tasks_unitpowerdown.h"
#include "client_facility.h"
#include "tasks_transportpad.h"
#include "wallobj.h"
#include "spyobj.h"
#include "tasks_unitguard.h"
#include "message.h"



///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client - Controls client side interface
//

namespace Client
{
  namespace Events
  {
    // Cursors for each primary event
    static GameIdent eventCursors[PE_MAX];

    // List of selected objects when last order was issued
    static UnitObjList prevSelectList;

    // List of selected objects on the previous cycle
    static UnitObjList prevCycleList;



    //
    // RandomResponse
    //
    // Play a random response to the given event
    //
    static void RandomResponse(UnitObj *obj, U32 event)
    {
      // Get the sound effect file name
      if (const char *response = obj->UnitType()->RandomResponse(event))
      {
        Sound::Digital::Reserved::Submit("Response", response, Sound::Digital::Reserved::MODE4);
      }
    }


    //
    // TriggerResponse
    //
    // Trigger the given response event
    //
    void TriggerResponse(const UnitObjList &list, U32 event)
    {
      UnitObjList candidates;
    
      // Generate the list of candidates
      for (UnitObjList::Iterator i(&list); *i; i++)
      {
        if ((**i)->UnitType()->GetResponse(event))
        {
          candidates.Append(**i);
        }
      }

      // Pick a random candidate
      i.SetList(&candidates);
      i.GoTo(Random::nonSync.Integer(candidates.GetCount()));

      if (*i)
      {
        RandomResponse(**i, event);
      }

      candidates.Clear();
    }

  
    //
    // IsUnit
    //
    // Is the given object derived from unit object (obj can be NULL)
    //
    static UnitObj * IsUnit(MapObj *obj)
    {
      return (obj ? Promote::Object<UnitObjType, UnitObj>(obj) : NULL);
    }


    //
    // ClearSelected
    //
    // Clear currently selected list
    //
    void ClearSelected()
    {
      // Clear the list
      data.sList.Clear();
    }


    //
    // ValidateSelected
    //
    // Validate selected list and current object pointers
    //
    static void ValidateSelected()
    {
      // Ensure all selected objects are alive and on the map
      UnitObjListUtil::PurgeNonTeamOffMap(data.sList, Team::GetDisplayTeam());
    }
                            

    //
    // UpdatePreviousSelected
    //
    // Update the previous selected list
    //
    void UpdatePreviousSelected(Bool notify)
    {
      // Ensure all selected objects are alive and on the map
      UnitObjListUtil::PurgeNonTeamOffMap(prevCycleList, Team::GetDisplayTeam());

      // Compare the previous cycles list with this ones
      if (!data.sList.Identical(prevCycleList))
      {
        if (notify)
        {
          // If the selection has changed, then we're not selecting a squad
          data.squad = NULL;

          // Reset the squad tabs
          data.squadReset = !data.squadReset;
        }

        // Copy the current list into the previous cycle list
        prevCycleList.Dup(data.sList);
      }
    }


    //
    // TriggerDefaultMode
    //
    // Sets the current mode only if there is no other mode active
    //
    static void TriggerDefaultMode(ClientMode mode)
    {
      if (data.clientMode == CM_NONE)
      {
        TriggerClientMode(mode);
      }
    }


    //
    // SetupSelectedUnitInfo
    //
    // Update the selected unit information
    //
    static void SetupSelectedUnitInfo()
    {
      data.sListInfo->Reset();
      data.sListInfo->Add(data.sList);
      data.sListInfo->Done();
    }


    //
    // ViewUnit
    //
    // Move the camera to view the given unit
    //
    static void ViewUnit(UnitObj *unit)
    {
      ASSERT(unit)

      Viewer::GetCurrent()->LookAt
      (
        WorldCtrl::CellToMetresX(unit->GetCellX()),
        WorldCtrl::CellToMetresZ(unit->GetCellZ())
      );
    }


    //
    // UnitsSelected
    //
    // Called when the user selects units
    //
    void UnitsSelected()
    {
      // Update the selected unit info
      SetupSelectedUnitInfo();

      // Is there a single unit selected
      UnitObj *single = (data.sList.GetCount() == 1) ? data.sList.GetFirst() : NULL;

      // Is there a construction menu    
      if (Construction *ctrl = data.controls.construction.GetPointer())
      {
        // Should it be active
        if (single && single->HasProperty(0xDCDE71CD)) // "Ability::Construction"
        {
          // Either inactive or wrong constructor
          if (!ctrl->IsActive() || single != ctrl->GetConstructor())
          {
            if (ctrl->Generate(single))
            {
              ctrl->Activate();
            }
            else
            {
              ctrl->Deactivate();
            }
          }       
        }
        else
        {
          // Ensure it's inactive
          ctrl->Deactivate();
        }
      }

      // If not blocked by the orders menu, activate the context menus
      if (data.controls.orders.Dead() || !data.controls.orders->IsActive())
      {
  
        // Is there a unit context menu
        if (UnitContext *ctrl = data.controls.context.GetPointer())
        {
          ctrl->ChangeActiveState(TRUE);
        }
      }

      // Is there a single unit selected
      if (single) 
      {
        // Is this a telepad
        if (TaskCtrl::Promote<Tasks::TransportPad>(single))
        {
          // Enter unload mode
          TriggerDefaultMode(CM_UNLOADCARGO);
        }
      }
    }


    //
    // SelectUnit
    //
    // Select a single unit, using all modifier keys
    //
    void SelectUnit(UnitObj *unit)
    {
      ASSERT(unit)

      // Get the display team
      if (Team *team = Team::GetDisplayTeam())
      {
        // Are we adding/removing from selection
        if (Common::Input::GetModifierKey(3))
        {
          // Is this unit already in the selected list
          if (data.sList.Exists(unit))
          {
            // Remove it
            data.sList.Remove(unit);

            // My work here is done
            return;
          }
        }
        else
        {
          ClearSelected();
        }

        // Get the type of the unit
        UnitObjType *type = unit->UnitType();

        // Do we want to select everything on the map of this type
        if (Common::Input::GetModifierKey(1))
        {
          // Iterate each unit on the display team
          for (NList<UnitObj>::Iterator u(&team->GetUnitObjects()); *u; ++u)
          {
            // Is this unit the same type
            if ((*u)->UnitType() == type)
            {
              // Add to the selected list
              Common::GameWindow::AddToSelected(*u, data.sList, team);
            }           
          }
        }
        else

        // Do we want to select all visible units of this type
        if (Common::Input::GetModifierKey(2))
        {
          // Check all visible objects
          for (NList<MapObj>::Iterator u(&MapObjCtrl::GetDisplayList()); *u; ++u)
          {
            // Promote to a unit
            if (UnitObj *target = Promote::Object<UnitObjType, UnitObj>(*u))
            {
              // Is this unit the same type
              if (target->UnitType() == type)
              {
                // Add to the selected list
                Common::GameWindow::AddToSelected(target, data.sList, team);
              }
            }
          }
        }
        else

        // Just select this unit
        if (!Common::GameWindow::AddToSelected(unit, data.sList, team))
        {
          return;
        }

        // Generate a response
        RandomResponse(unit, 0x9FF22134); // "Select"

        // Let the system know that units have been selected
        UnitsSelected();
      }
    }


    //
    // StepUnitSelectionNext
    //
    // Get the next unit to test
    //
    static UnitObj * StepUnitSelectionNext(const NList<UnitObj> &list, UnitObj *unit, Bool next)
    {
      ASSERT(unit)
      ASSERT(unit->GetTeam())
      ASSERT(unit->teamNode.InUse())

      // Get the next or previous node in the list
      if (NList<UnitObj>::Node *node = next ? unit->teamNode.GetNext() : unit->teamNode.GetPrev())
      {
        return (node->GetData());
      }
      else
      {
        // Return either the first or the last unit
        return (next ? list.GetHead() : list.GetTail());
      }
    }


    //
    // StepUnitSelection
    //
    // Step to the prev/next unit
    //
    static Bool StepUnitSelection(Bool next, Bool filterType, U32 property = 0)
    {
      ASSERT(Team::GetDisplayTeam())

      // Get the team unit list
      const NList<UnitObj> &list = Team::GetDisplayTeam()->GetUnitObjects();

      // Get the first selected unit
      UnitObj *unit = data.sList.GetFirst();

      // Set the type now to cater for units not actively on the team
      UnitObjType *type = unit ? unit->UnitType() : NULL;

      // If nothing selected, or the unit isn't on a team
      if (!unit || !unit->teamNode.InUse())
      {
        // Grab either the oldest or the youngest unit
        unit = next ? list.GetHead() : list.GetTail();
      }
      else
      {
        // Not interested in the one that is already selected
        unit = StepUnitSelectionNext(list, unit, next);
      }

      // Did we find a starting point
      if (unit)
      {
        // Set the type if not set already
        if (!type)
        {
          type = unit->UnitType();
        }

        // Clear the current selection
        ClearSelected();

        // Only check each unit in the team list once
        for (U32 c = 0; unit && (c < list.GetCount()); ++c)
        {
          if
          (
            // Does it pass the type check
            (!filterType || (unit->UnitType() == type))

            &&

            // Does it pass the property check
            (!property || unit->HasProperty(property))
          )
          {
            // Can we select this unit
            if (Common::GameWindow::AddToSelected(unit, data.sList, NULL, TRUE, FALSE))
            {
              // Generate a response
              RandomResponse(unit, 0x9FF22134); // "Select"

              // Let the system know that units have been selected
              UnitsSelected();

              // Move the camera
              ViewUnit(unit);

              // Success
              return(TRUE);
            }
          }

          // Get the next unit to test
          unit = StepUnitSelectionNext(list, unit, next);
        }
      }

      // Failed
      return(FALSE);
    }

    
    // 
    // SelectList
    //
    // Set the current selection using the given list, optionally
    // scrolling to the first object
    //
    void SelectList(const UnitObjList &list, Bool scroll)
    {
      // Copy the list
      data.sList.Dup(list);

      // Get the first object
      UnitObj *first = FirstSelected();

      if (first)
      {
        Mesh::Manager::selEnt = &first->Mesh();

        // Should we scroll to the first object
        if (scroll)
        {
          ViewUnit(first);
        }
      }
    
      UnitsSelected();
    }


    //
    // SelectType
    //
    // 
    // 
    // SelectSquad
    //
    // Set the current selection using the given list, optionally
    // scrolling to the first object
    //
    void SelectSquad(SquadObj *squad, Bool scroll)
    {
      data.squad = squad;
      SelectList((const UnitObjList &) squad->GetList(), scroll);
    }


    //
    // PointThreshold
    //
    // Returns TRUE when the points are apart >= the threshold 
    //
    static Bool PointThreshold(const Point<S32> &a, const Point<S32> &b, S32 x, S32 y)
    {
      return (abs(a.x - b.x) >= x || abs(a.y - b.y) >= y);
    }


    //
    // HaveSelected
    //
    // Returns TRUE if we currently have at least one selected object
    //
    static Bool HaveSelected()
    {
      return (data.sList.GetCount() ? TRUE : FALSE);
    }


    //
    // UpdateSelectedLists
    //
    // Send the order to change the list of selected units if they have changed
    //
    void UpdateSelectedLists()
    {
      if (!data.sList.Identical(prevSelectList))
      {
        Orders::Game::ClearSelected::Generate(GetPlayer());
        Orders::Game::AddSelected::Generate(GetPlayer(), data.sList);

        // Copy the currently selected list
        prevSelectList.Dup(data.sList);
      }
    }


    //
    // CreateParticle
    //
    // Creates the given particle type at the given location
    //
    static Bool CreateParticle(const Vector &location, U32 typeCrc)
    {
      if (ParticleClass *p = ParticleSystem::FindType(typeCrc))
      {
			  Vector v(0.0f, 0.0f, 0.0f);

        Matrix m;
        m.ClearData();
        m.posit = location;

        ParticleSystem::New(p, m, v, v, v, 0.0F);
        return (TRUE);
      }

      return (FALSE);
    }


    //
    // HandleDiscreteEvent
    //
    // Responds to discrete event triggers.  These can be generated
    // through key bindings (such as 's' for 'stop') or through a
    // context menu.  There may or may not be objects selected 
    // when called.
    //
    // Returns TRUE if event was handled, otherwise FALSE.
    //
    Bool HandleDiscreteEvent(void *, U32 crc, U32 param1, U32)
    {
      switch (crc)
      {
        case 0xA918FBE3: // "de::selectall"
        {
          // Clear current selection
          ClearSelected();

          // Now select all units on the display team, including powered down ones
          for (NList<GameObj>::Iterator u(&GameObjCtrl::listAll); *u; ++u)
          {
            if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(*u))
            {
              if (unit->GetTeam() == Team::GetDisplayTeam())
              {
                Common::GameWindow::AddToSelected(unit, data.sList, NULL, TRUE, FALSE);
              }
            }
          }

          if (data.sList.GetCount())
          {
            UnitsSelected();
            TriggerResponse(data.sList, 0x9FF22134); // "Select"
          }

          break;
        }

        case 0x4953CCB5: // "de::nextunit"
          StepUnitSelection(TRUE, FALSE, param1);
          break;

        case 0x6986C532: // "de::prevunit"
          StepUnitSelection(FALSE, FALSE, param1);
          break;

        case 0x269CF607: // "de::nextunittype"
          StepUnitSelection(TRUE, TRUE);
          break;

        case 0x62223638: // "de::prevunittype"
          StepUnitSelection(FALSE, TRUE);
          break;

        case 0x0AE21E46: // "de::nextdir"
          data.dir = WorldCtrl::SlideCompassDir(data.dir);
          break;

        case 0xD8DAD23F: // "de::prevdir"
          data.dir = WorldCtrl::SlideCompassDir(data.dir, FALSE);
          break;

        // Clear currently selected objects
        case 0xE0FEDE24: // "de::deselect"
          ClearSelected();
          break;

        // Tell selected objects to stop
        case 0x92840F9F: // "de::stop"
          // Do we have a squad selected ?
          if (data.squad.Alive())
          {
            Orders::Squad::Stop::Generate(GetPlayer(), data.squad.Id());
          }

          // We may want to change this later, but for the 
          // time being stop all of the units in the squad too
          UpdateSelectedLists();
          Orders::Game::Stop::Generate(GetPlayer());
          break;


        case 0x49B46B05: // "de::explore"
          // Do we have a squad selected ?
          if (data.squad.Alive())
          {
            Orders::Squad::Explore::Generate(GetPlayer(), data.squad.Id(), Orders::FLUSH);
          }
          else
          {
            UpdateSelectedLists();
            Orders::Game::Explore::Generate(GetPlayer(), Orders::FLUSH);
          }
          break;

        // Tell selected objects to self destruct
        case 0x4CE501F9: // "de::selfdestruct"
          UpdateSelectedLists();
          Orders::Game::SelfDestruct::Generate(GetPlayer());
          break;

        // Tell selected objects to scatter
        case 0xEA42AC92: // "de::scatter"
          UpdateSelectedLists();
          Orders::Game::Scatter::Generate(GetPlayer());
          break;

        // Tell selected objects to recycle
        case 0x260BEC2F: // "de::recycle"
          UpdateSelectedLists();
          Orders::Game::Recycle::Generate(GetPlayer());
          ClearSelected();
          break;

        case 0x3F37A8FB: // "de::upgrade"
          UpdateSelectedLists();
          Orders::Game::Upgrade::Generate(GetPlayer());
          break;

        // Tell selected objects to search for resource
        case 0x7C3CC53C: // "de::collect"
          UpdateSelectedLists();
          Orders::Game::Collect::Generate(GetPlayer(), 0, TRUE, Orders::FLUSH);
          break;

        // Tell selected objects to store resources
        case 0x02F5EC48: // "de::store"
          UpdateSelectedLists();
          Orders::Game::Store::Generate(GetPlayer(), 0, TRUE, FALSE, Orders::FLUSH);
          break;

        case 0x8AC31D58: // "de::restore"
          UpdateSelectedLists();
          Orders::Game::Restore::Generate(GetPlayer(), Orders::FLUSH);
          break;

        case 0x736969ED: // "de::guard"
          TriggerClientMode(CM_GUARD); 
          break;

        case 0x3DF04D34: // "de::setrestore"
          TriggerClientMode(CM_SETRESTORE);
          break;

        case 0x4F4085F7: // "de::unloadcargo"
          TriggerClientMode(CM_UNLOADCARGO);
          break;

        case 0x284E48A2: // "de::setrally"
          TriggerClientMode(CM_SETRALLY);
          break;

        case 0x3A8C01AE: // "de::ejectspy"
          UpdateSelectedLists();
          Orders::Game::EjectSpy::Generate(GetPlayer());
          break;

        case 0x6A8521B0: // "de::clearrestore"
          UpdateSelectedLists();
          Orders::Game::SetRestore::Generate(GetPlayer(), 0, Orders::FLUSH);
          break;

        case 0x4A083FB9: // "de::powerdown"
          UpdateSelectedLists();
          Orders::Game::PowerDown::Generate(GetPlayer(), TRUE);
          Message::TriggerGameMessage(0xD9DBD912); // "Client::PowerDown"
          break;

        case 0x891EF46F: // "de::powerup"
          UpdateSelectedLists();
          Orders::Game::PowerDown::Generate(GetPlayer(), FALSE);
          Message::TriggerGameMessage(0xA9C26388); // "Client::PowerUp"
          break;

        case 0xE95CE417: // "de::formation"
          // Save the formation type into the client info
          data.formation = param1;

          // Force the client mode to formation
          TriggerClientMode(CM_FORMATION);
          break;

        case 0xD01F1226: // "de::trail::toggleactive"
          Trail::TriggerEvent(0x8744485A); // "Discrete::ToggleActive"
          break;

        case 0xF7D7AE6B: // "de::trail::deletelast"
          Trail::TriggerEvent(0x610FB9A8); // "Discrete::DeleteLast"
          break;

        case 0xCBEC9474: // "de::trail::deletelocal"
          Trail::TriggerEvent(0x2C602158); // "Discrete::DeleteLocal"
          break;

        case 0xE19A6BE7: // "de::trail::deleteselected"
          Trail::TriggerEvent(0x8EE282A4); // "Discrete::DeleteSelected"
          break;

        case 0x0F5DD774: // "de::trail::cancelconstruction"
          Trail::TriggerEvent(0x0D432C0B); // "Discrete::CancelConstruction"
          break;

        case 0x288ABCFE: // "de::trail::create"
          Trail::TriggerEvent(0x9012B6B2); // "Discrete::Create"
          break;

        case 0xF46FF3AB: // "de::trail::construct::oneway"
          Trail::StartConstruction(TrailObj::MODE_ONEWAY);
          break;

        case 0x78D09B80: // "de::trail::construct::twoway"
          Trail::StartConstruction(TrailObj::MODE_TWOWAY);
          break;

        case 0x08FFBCCF: // "de::trail::construct::loopin"
          Trail::StartConstruction(TrailObj::MODE_LOOPIN);
          break;

        // Add the selected items to the given squad
        case 0x01469E6B: // "de::squad::addselected"
          if (data.squad.Alive())
          {
            UpdateSelectedLists();
            Orders::Squad::AddSelected::Generate(GetPlayer(), data.squad.Id());
          }
          break;

        // Add the selected items to the given squad
        case 0x1D63ABE3: // "de::squad::removeselected"
          UpdateSelectedLists();
          Orders::Squad::RemoveSelected::Generate(GetPlayer());
          data.squad = NULL;
          data.squadReset = !data.squadReset;
          break;

        // Empty the squad
        case 0x5900F3B9: // "de::squad::empty"
          if (data.squad.Alive())
          {
            Orders::Squad::Empty::Generate(GetPlayer(), data.squad.Id());
          }
          break;

        case 0x8E589710: // "de::clearplayermarker"
          Orders::Game::PlayerMarker::Generate(GetPlayer(), S32_MAX, S32_MAX);
          break;

        // Tell the selected objects to pause
        case 0x36F3D487: // "de::generic::pause"
          UpdateSelectedLists();
          Orders::Game::PostEvent::Generate(GetPlayer(), TaskNotify::Pause);
          break;

        // Tell the selected objects to unpause
        case 0x1301D4D9: // "de::generic::unpause"
          UpdateSelectedLists();
          Orders::Game::PostEvent::Generate(GetPlayer(), TaskNotify::Unpause);
          break;

        // Toggle pause of the selected object
        case 0xB570D0D5: // "de::generic::togglepause"
          UpdateSelectedLists();
          Orders::Game::PostEvent::Generate(GetPlayer(), TaskNotify::TogglePause);
          break;

        // Cancel the task of the selected objects
        case 0xFD04A892: // "de::generic::cancel"
          UpdateSelectedLists();
          Orders::Game::PostEvent::Generate(GetPlayer(), TaskNotify::Cancel);
          break;

        // Unknown events should be ignored
        default :
          return (FALSE);
      }

      // Event was handled
      return (TRUE);
    }


    //
    // TriggerClientMode
    //
    // Sets the current forced client mode, or reverts to none
    //
    void TriggerClientMode(ClientMode mode)
    {
      ASSERT(mode >= 0 && mode < CM_MAX);

      // Set new mode or revert to none
      data.clientMode = (data.clientMode == mode) ? CM_NONE : mode;
    }

    
    //
    // ModifyTacticalSetting
    //
    // Modify tactical setting of selected squad/units
    //
    void ModifyTacticalSetting(U8 modifier, U8 setting)
    {
      if (data.squad.Alive())
      {
        Orders::Squad::Tactical::Generate(GetPlayer(), data.squad.Id(), modifier, setting);
      }
      else

      if (data.sList.GetCount())
      {
        Events::UpdateSelectedLists();
        Orders::Game::Tactical::Generate(GetPlayer(), modifier, setting);
      }
    }


    //
    // TriggerClientMode
    //
    // Find 'name' and call above method, FALSE if not found
    //
    Bool TriggerClientMode(const char *name)
    {
      switch (Crc::CalcStr(name))
      {
        case 0xC9EF9119: // "none"
          TriggerClientMode(CM_NONE); break;

        case 0x3381FB36: // "move"
          TriggerClientMode(CM_MOVE); break;

        case 0xA8FEF90A: // "attack"
          TriggerClientMode(CM_ATTACK); break;

        case 0xCEC593A1: // "moveattack"
          TriggerClientMode(CM_MOVEATTACK); break;

        case 0x64F10AE2: // "attacknomove"
          TriggerClientMode(CM_ATTACKNOMOVE); break;

        case 0x9F83CBA6: // "turn"
          TriggerClientMode(CM_TURN); break;    

        case 0xA2A4CB5F: // "jumpscroll"
          TriggerClientMode(CM_JUMPSCROLL); break;    

        case 0x7223612A: // "formation"
          TriggerClientMode(CM_FORMATION); break;

        case 0x4C2380CB: // "setrestore"
          TriggerClientMode(CM_SETRESTORE); break;

        case 0x78F87AA7: // "unloadcargo"
          TriggerClientMode(CM_UNLOADCARGO); break;

        case 0xA58DF641: // "setrally"
          TriggerClientMode(CM_SETRALLY); break;

        case 0xA30C4555: // "playermarker"
          TriggerClientMode(CM_PLAYERMARKER); break;
        
        default:
          return (FALSE);
      }

      return (TRUE);
    }


    //
    // FilterClientMode
    //
    // Apply filters to ensure mode is valid for the current state
    //
    static void FilterClientMode()
    {
      CycleInfo &i = data.cInfo;
      Bool invalid = FALSE;

      switch (data.clientMode)
      {
        // Move and turn are basically the same thing
        case CM_MOVE:
        case CM_TURN:
        case CM_MOVEATTACK:
        case CM_FORMATION:
        case CM_SETRESTORE:
        case CM_ATTACK:
        case CM_ATTACKNOMOVE:
        case CM_GUARD:
        {
          invalid = !HaveSelected();
          break;
        }

        case CM_UNLOADCARGO :
        {
          invalid = TRUE;
        
          for (UnitObjList::Iterator li(&data.sList); *li; li++)
          {
            if (Promote::Object<TransportObjType, TransportObj>(**li))
            {
              invalid = FALSE; 
              break;
            }
          }
          break;
        }

        case CM_SETRALLY:
        {
          invalid = TRUE;
        
          for (UnitObjList::Iterator li(&data.sList); *li; li++)
          {
            if ((**li)->CanUseRallyPoint())
            {
              invalid = FALSE;  
              break;
            }
          }
          break;
        }

        case CM_CONSTRUCT:
        {
          invalid = TRUE;

          // Is there a constructor set and active control
          if (data.constructType.Alive() && data.controls.construction->IsActive())
          {
            // Is the constructor the only selected unit
            if (i.oneUnit.Alive() && data.controls.construction->GetConstructor() == i.oneUnit)
            {
              invalid = FALSE;
            }
          }
          break;
        }
      }

      // If mode was invalid, reset to default
      if (invalid)
      {
        data.clientMode = CM_NONE;
      }
    }


    //
    // CheckConstruction
    //
    // Returns true if allowed to build in current location
    //
    static Bool CheckConstruction()
    {
      ASSERT(data.cInfo.gameWnd->HasProperty(0xB665088B)) // "Construction"
      ASSERT(data.cInfo.oneUnit->HasProperty(0xDCDE71CD)) // "Ability::Construction"
      ASSERT(data.cInfo.t.cell)
      ASSERT(data.constructType.Alive())

      // Does object have a footprint
      if (FootPrint::Type *foot = data.constructType->GetFootPrintType())
      {
        // Setup the placement object
        data.placeFoot.Init(foot, Team::GetDisplayTeam());

        // Setup matrix using the position and direction
        WorldCtrl::SetupWorldMatrix(data.placeLocation, data.cInfo.t.pos, data.dir);

        // Snap to the nearest cell
        data.placeFoot.AdjustLocation(data.placeLocation);

        // Do the placement check
        FootPrint::Placement::Result r = data.placeFoot.Check
        (
          data.placeLocation, FootPrint::Placement::CHECK_IGNOREMOBILE
        );

        // Apply thump-to height to matrix
        if (data.placeFoot.GetThumped().IsValid())
        {
          data.placeLocation.posit.y = data.placeFoot.GetThumpHeight();
        }

        // Can we build here
        return (r == FootPrint::Placement::PR_OK);
      }

      // Can not build non-footprinted objects
      return (FALSE);
    }


    //
    // GetOffMapObject
    //
    // Get the offmap object from the current selection
    //
    static OffMapObj * GetOffMapObject()
    {
      ASSERT(data.cInfo.gameWnd.Alive())

      // Is there a single unit selected
      if (UnitObj *one = data.cInfo.oneUnit.GetPointer())
      {
        // Is this a construction facility
        if (Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(one))
        {
          return (task->GetOffMapObject());
        }
      }

      return (NULL);
    }


    //
    // GetTerrainOrObjectPosition
    //
    // Returns mouse object position, or the terrain position, or NULL
    //
    static const Vector * GetTerrainOrObjectPosition()
    {
      ASSERT(data.cInfo.gameWnd.Alive())

      if (data.cInfo.o.map.Alive())
      {
        return (&data.cInfo.o.map->Position());
      }

      if (data.cInfo.t.cell)
      {
        return (&data.cInfo.t.pos);
      }
      
      return (NULL);
    }


    //
    // SetupPrimaryEvent
    //
    // Set the primary event type for this process cycle
    //
    static void SetupPrimaryEvent()
    {
      ASSERT(data.cInfo.gameWnd.Alive());

      // Grab a couple of shortcuts
      CycleInfo &i = data.cInfo;
      PrimaryEvent &e = i.pEvent;
      Team *clientTeam = Team::GetDisplayTeam();

      // Set default mode
      e = PE_NONE;

      // Set initial event from the client mode
      switch (data.clientMode)
      {
        case CM_MOVE:
          e = PE_MOVE;
          break;

        case CM_ATTACK:
          e = PE_ATTACK; 
          break;

        case CM_MOVEATTACK:
          e = PE_MOVEATTACK;
          break;

        case CM_ATTACKNOMOVE:
          e = PE_ATTACKNOMOVE;
          break;

        case CM_TURN:
          e = PE_TURN; 
          break;

        case CM_JUMPSCROLL:
          e = PE_JUMPSCROLL; 
          break;

        case CM_FORMATION:
          if (i.t.cell) 
          { 
            e = PE_FORMATION; 
          }
          break;

        case CM_SETRESTORE:
        {
          e = PE_NOSETRESTORE;

          RestoreObj * restore = i.o.unit.Alive() ? Promote::Object<RestoreObjType, RestoreObj>(i.o.unit) : NULL;

          if (restore)
          {
            for (UnitObjList::Iterator li(&data.sList); *li; li++)
            {
              if (restore->IsStatic() && restore->CanRestore(**li))
              {
                e = PE_SETRESTORE;
              }
            }
          }
          break;
        }
      
        case CM_UNLOADCARGO:
        {
          e = PE_NOUNLOADCARGO;

          if (i.t.cell)
          {
            for (UnitObjList::Iterator li(&data.sList); *li; li++)
            {
              if (TransportObj *t = Promote::Object<TransportObjType, TransportObj>(**li))
              {
                if (t->UnloadAvailable() && t->CheckUnload(i.t.cellX, i.t.cellZ))
                {
                  e = PE_UNLOADCARGO;
                }
              }
            }
          }
          break;   
        }

        case CM_SETRALLY:
        {
          // A rally point can be placed anywhere on the terrain
          e = (i.t.cell) ? PE_SETRALLY : PE_NOSETRALLY;
          break;   
        }

        case CM_CONSTRUCT:    
        {
          // Does the current window allow construction
          if (i.t.cell && i.gameWnd->HasProperty(0xB665088B)) // "Construction"
          {
            // Only set construction result after calling CheckConstruction
            e = CheckConstruction() ? PE_CONSTRUCT : PE_NOCONSTRUCT;
          }
          break;
        }

        case CM_GUARD:
        {
          // We need to have a cell or a unit under the cursor to guard
          e = (i.t.cell || i.o.map.Alive()) ? PE_GUARD : PE_NOGUARD;
          break;
        }

        case CM_PLAYERMARKER:
        {
          e = (i.t.cell) ? PE_PLAYERMARKER : PE_NOPLAYERMARKER;
          break;
        }
      }

      // Trail operations
      if (e == PE_NONE && Trail::Active())
      {
        e = Trail::GetPrimaryEvent();
      }

      // No event set, and single unit selected
      if (e == PE_NONE && i.oneUnit.Alive())
      {
        // Is there an offmap object available
        if (OffMapObj *offMap = GetOffMapObject())
        {
          // Get the object or terrain position
          const Vector *p = GetTerrainOrObjectPosition();

          // Check if we can trigger here
          e = (p && offMap->Check(*p)) ? PE_USEOFFMAP : PE_NOUSEOFFMAP;
        }
        else
      
        // Is there one of our units under the cursor
        if (i.o.unit.Alive() && i.o.unit->GetTeam() == clientTeam)
        {
          // Is it a transport
          TransportObj *transport = Promote::Object<TransportObjType, TransportObj>(i.oneUnit);

          if (transport && transport->CheckCargo(i.o.unit))
          {
            e = PE_TRANSPORT;
          }
        }
      }

      // No event set, and object under the mouse
      if (e == PE_NONE && i.o.map.Alive())
      {
        // Is it a unit under the mouse
        if (i.o.unit.Alive())
        {
          Team *objTeam = i.o.unit->GetTeam();

          // We have a team, and the object is on a team
          if (clientTeam && objTeam)
          {
            // If we have control of this unit then the default is select
            if (objTeam == clientTeam)
            {
              e = PE_SELECT;
            }
            else

            // If we have units selects and this unit is an enemy unit the the default is attack
            if (HaveSelected() && clientTeam->Team::TestUnitRelation(i.o.unit, Relation::ENEMY))
            {
              SpyObj *spy = i.oneUnit.Alive() ? Promote::Object<SpyObjType, SpyObj>(i.oneUnit) : NULL;

              // One unit selected and it is a spy
              if (spy)
              {
                e = PE_NOINFILTRATE;

                // Can unit under cursor be infiltrated
                if (i.o.unit->UnitType()->CanBeInfiltrated())
                {
                  e = PE_INFILTRATE;
                }
                else
                {
                  // Can object be morphed into
                  if (spy->CanMorph(i.o.unit))
                  {
                    e = PE_MORPH;
                  }
                  else
                  {
                    e = PE_NOMORPH;
                  }
                }
              }
              else
              {
                e = PE_ATTACK;
              }
            }
          }
          else
          {
            // Missing team, so ignore
            e = PE_NONE;
          }
        }
        else
        {
          // Should we attack this map object
          if (HaveSelected() && i.o.map->HasProperty(0x74FC64CC)) // "Client::AutoAttack"
          {
            e = PE_ATTACK;
          }
        }

        // Ignore rest if adding/removing from selection
        if ((e != PE_SELECT) || !Common::Input::GetModifierKey(3))
        {
          // Is the unit under the cursor a static restorer
          RestoreObj *restoreStatic = NULL;
      
          if (i.o.unit.Alive())
          {
            // Promote to a restore object
            restoreStatic = Promote::Object<RestoreObjType, RestoreObj>(i.o.unit);

            // Are we prevented from using this facility
            if (restoreStatic && (!restoreStatic->IsStatic() || !Team::TestRelation(clientTeam, restoreStatic->GetTeam(), Relation::ALLY)))
            {
              restoreStatic = NULL;
            }
          }

          // Is the unit under the cursor a transport
          TransportObj *transport = i.o.unit.Alive() ? 
            Promote::Object<TransportObjType, TransportObj>(i.o.unit) : NULL;

          // Is the unit under the cursor a wall
          WallObj *wall = i.o.unit.Alive() ? 
            Promote::Object<WallObjType, WallObj>(i.o.unit) : NULL;

          // Try to find an object that has a function with the object under the mouse
          for (UnitObjList::Iterator li(&data.sList); *li; li++)
          {
            UnitObj *unitObj = **li;

            if (unitObj->UnitType()->GetResourceTransport())
            {
              // If this object can transport resource and the cursor is over resource then
              if (ResourceObj *resourceObj = Promote::Object<ResourceObjType, ResourceObj>(i.o.map))
              {
                e = PE_COLLECT;
                break;
              }

              // If this object can transport resource and the cursor is over storage then
              if 
              (
                i.o.unit.Alive() &&
                i.o.unit->HasProperty(0xAE95DF36) && // "Ability::StoreResource"
                i.o.unit->GetTeam() == unitObj->GetTeam()
              )
              {
                e = PE_STORE;
                break;
              }
            }

            // Does this unit need to get restored by the unit under the cursor
            if (restoreStatic && restoreStatic->RestoreRequired(unitObj))
            {
              e = PE_RESTORESTATIC;
              break;
            }

            // Is there a unit under the cursor
            if (i.o.unit.Alive() && i.o.unit != unitObj)
            {
              // Can we board a unit transporter
              if (transport && transport->CheckCargo(unitObj))
              {
                e = PE_BOARD;
                break;
              }

              // Is the unit under the cursor an ally
              if (Team::TestRelation(i.o.unit->GetTeam(), clientTeam, Relation::ALLY))
              {
                // Is this a selected restore object
                if (RestoreObj *restore = Promote::Object<RestoreObjType, RestoreObj>(unitObj))
                {
                  // Does the unit require restoration
                  if (!restore->IsStatic() && restore->RestoreRequired(i.o.unit))
                  {
                    e = PE_RESTOREMOBILE;
                    break;
                  }
                }

                // Is there a wall under the cursor
                if (wall)
                {
                  // Is this a selected wall
                  if (WallObj *selectedWall = Promote::Object<WallObjType, WallObj>(unitObj))
                  {  
                    // Can we link to this wall
                    if (selectedWall->TestLink(wall) == WallObj::TR_SUCCESS)
                    {
                      e = PE_WALLACTIVATE;
                      break;
                    }
                    else

                    // Is the selected wall connected to this wall
                    if (selectedWall->FindLink(wall))
                    {
                      e = PE_WALLDEACTIVATE;
                      break;
                    }
                    else
                    {
                      // Unable to link, but maybe another selected can, so don't break
                      e = PE_WALLNOACTIVATE;
                    }
                  }
                  break;
                }
              }
            }
          }
        }
      }

      // No event set, and over terrain
      if (e == PE_NONE && i.t.cell)
      {
        // Do we have guys selected
        if (HaveSelected())
        {
          e = PE_MOVE;
        }
        else

        // Should we jumpscroll to location
        if (i.gameWnd->HasProperty("JumpScroll"))
        {
          e = PE_JUMPSCROLL;
        }
      }

      // Now do restriction modifiers
      switch (e)
      {
        case PE_MOVE:
        case PE_MOVEATTACK:
        {
          PrimaryEvent p = e;

          // Assume none of the objects can ever move
          e = PE_NONE;

          if (HaveSelected() && i.t.cell)
          {
            // Try and find an object that can move to the location
            for (UnitObjList::Iterator li(&data.sList); *li; li++)
            {
              // Get a pointer to the object
              UnitObj *obj = **li;

              // Can we ever move
              if (obj->CanEverMove())
              {
                // We can now correctly set the no move event
                e = PE_NOMOVE;

                // Now check if this object can move to the specified location
                if
                (
                  // Do not have a parent
                  !obj->GetParent() && 
                
                  // Not currently visible, or we can move there
                  (
                    !i.t.visible || PathSearch::CanMoveToCell
                    (
                      obj->UnitType()->GetTractionIndex(obj->UnitType()->GetDefaultLayer()), *i.t.dataCell
                    )
                  )
                )
                {
                  e = p;
                  break;
                }
              }
            }
          }
          break;
        }

        case PE_ATTACK:
        case PE_ATTACKNOMOVE:
        {
          ASSERT(HaveSelected());

          PrimaryEvent p = e;

          // Assume we can't attack
          e = PE_NOATTACK;

          // Are we targetting an object
          MapObj *target = i.o.map.GetPointer();

          // Try and find an object that can
          for (UnitObjList::Iterator li(&data.sList); *li; li++)
          {
            // Get the unit
            UnitObj *unit = **li;

            // If targetting an object, can we do damage
            if (unit->CanEverFire() && (!target || unit->CanDamageNow(target)))
            {
              // Re-activate the original primary event
              e = p;

              // At least one unit can attack, so stop checking
              break;
            }
          }
          break;
        }

        case PE_GUARD:
        {
          ASSERT(HaveSelected());

          // Assume we can't guard
          e = PE_NOGUARD;

          // Try and find an object that can
          for (UnitObjList::Iterator li(&data.sList); *li; li++)
          {
            // Can we use the attack task
            if (Tasks::UnitGuard::CanGuard(**li))
            {         
              e = PE_GUARD;
              break;
            }
          }
          break;
        }
      }
    }


    //
    // SetupCycleDisplay
    //
    // Displays all items that depend on the current cycle information
    //
    void SetupCycleDisplay()
    {
      // Grab a shortcut
      CycleInfo &i = data.cInfo;

      // Are we over a game window
      if (i.gameWnd.Alive())
      {
        // Set the cursor for this game window based on the primary event
        i.gameWnd->SetCursor
        (
          Crc::CalcStr(eventCursors[data.cInfo.pEvent].str)
        );
      }
    }

  
    //
    // GetTrackedObject
    //
    // Returns an object to track
    //
    MapObj * GetTrackedObject(U32 distance)
    {
      ASSERT(data.cInfo.gameWnd.Alive());

      if (data.trackObject)
      {
        Area<S32> a;

        // Adjust distances for mode
        U32 dx = (distance * Input::GetModeRatio().x) >> 16;
        U32 dy = (distance * Input::GetModeRatio().y) >> 16;

        // Setup selection area
        a.p0.x = data.cInfo.mouse.x - dx;
        a.p0.y = data.cInfo.mouse.y - dy;
        a.p1.x = data.cInfo.mouse.x + dx;
        a.p1.y = data.cInfo.mouse.y + dy;

        // Find the closest object
        return (data.cInfo.gameWnd->FindClosest(a));
      }
      else
      {
        return (NULL);
      }
    }


    //
    // GetGameWindow
    //
    // Returns pointer to the game window the mouse is currently over, or NULL
    //
    Common::GameWindow * GetGameWindow()
    {
      // Always return NULL if a modal window is active
      if (IFace::GetModal())
      {
        return (NULL);
      }

      // Get the appropriate window
      IControl *ctrl = data.captureHandler ? IFace::GetCapture() : IFace::GetMouseOver();

      // Promote to a game window
      return (ctrl ? IFace::Promote<Common::GameWindow>(ctrl) : NULL);
    }


    //
    // SetupCommonCycle
    //
    // Setup common cycle info
    //
    static void SetupCommonCycle()
    {
      // Start the common cycle
      Common::Cycle::Start();

      // Clear the current selection
      Common::Cycle::ClearSelected();

      // Add each selected object
      for (UnitObjList::Iterator i(&data.sList); *i; i++)
      {
        Common::Cycle::AddSelected(**i);
      }

      // Are we over a game window
      if (data.cInfo.gameWnd.Alive())
      {
        // Set the object under the mouse
        Common::Cycle::SetMouseOverObject(data.cInfo.o.map.Alive() ? data.cInfo.o.map.GetData() : NULL);
      }

      // End the common cycle
      Common::Cycle::End();
    }


    //
    // MouseOverFilter
    //
    // Selection filter for object selection
    //
    static MapObj * MouseOverFilter(MapObj *obj)
    {
      ASSERT(obj)

      // Always select base unit
      while (obj->GetParent())
      {
        obj = obj->GetParent();
      }

      return (obj);
    }


    //
    // SetupCycleInfo
    //
    // Generate info for this processing cycle
    //
    static void SetupCycleInfo()
    {
      // Grab a shortcut
      CycleInfo &i = data.cInfo;

      // Does the current info need to be updated
      ASSERT(data.cycleCount != data.lastUpdate)

      // Save the current cycle
      data.lastUpdate = data.cycleCount;

      // Validate the currently selected list
      ValidateSelected();

      // Update the selected unit info
      SetupSelectedUnitInfo();

      // Update the previous selected list
      UpdatePreviousSelected(TRUE);

      // Get the current screen position of the mouse
      i.mouse = Input::MousePos();

      // Save the number of objects in the display list
      i.displayListCount = MapObjCtrl::GetDisplayList().GetCount();

      // Do we have a single unit selected
      i.oneUnit = (data.sList.GetCount() == 1) ? IsUnit(data.sList.GetFirst()) : NULL;

      // Get the game window the mouse is over, or NULL
      i.gameWnd = GetGameWindow();

       // Setup information if over a game window and no capture event
      if (data.processing && i.gameWnd.Alive() && !data.captureHandler)
      {
        #ifndef DEVELOPMENT
        // Pretend nothing under mouse if paused
        if (GameTime::Paused())
        {
          i.o.map = NULL;
          i.o.unit = NULL;
          i.t.cell = NULL;
        }
        else
        {
        #endif
          // See if mouse is over an object
          i.o.map = i.gameWnd->PickObject(i.mouse.x, i.mouse.y, MouseOverFilter);

          // If not, try and find an object near the cursor
          if (!i.o.map.Alive())
          {
            // Try and find an object that can fire
            for (UnitObjList::Iterator li(&data.sList); *li; ++li)
            {
              if ((**li)->CanEverFire())
              {
                i.o.map = GetTrackedObject(data.trackDistance);
                break;
              }
            }
          }

          // Promote to a unit
          i.o.unit = i.o.map.Alive() ? IsUnit(i.o.map) : NULL;
     
          if (i.o.unit.Alive())
          {
            Mesh::Manager::curEnt = &i.o.map->Mesh();
            Mesh::Manager::curParent = Mesh::Manager::curEnt->FindMeshEnt();
          }

          // See if mouse is over the terrain
          if (i.gameWnd->TerrainPosition(i.mouse.x, i.mouse.y, i.t.pos))
          {
            ASSERT(WorldCtrl::MetreOnMap(i.t.pos.x, i.t.pos.z));
      
            // Calculate the cell position on the terrain
            i.t.cellX = WorldCtrl::MetresToCellX(i.t.pos.x);
            i.t.cellZ = WorldCtrl::MetresToCellZ(i.t.pos.z);
            i.t.cell = Terrain::GetCell(i.t.cellX, i.t.cellZ);

            // And get a pointer to the actual map cell
            i.t.dataCell = &TerrainData::GetCell(i.t.cellX, i.t.cellZ);

            // Store seen and visible flags
            Sight::SeenVisible
            (
              i.t.cellX, i.t.cellZ, Team::GetDisplayTeam(), i.t.seen, i.t.visible
            );
          }
          else
          {
            // Otherwise clear the pointers
            i.t.cell = NULL;
          }
        #ifndef DEVELOPMENT
        }
        #endif

        // Make sure client mode is valid
        FilterClientMode();

        // Setup primary event based on this information
        SetupPrimaryEvent();
      }
      else
      {
        // Information was not setup, so clear game window pointer
        i.gameWnd = NULL;
      }

      // Setup per-cycle display
      SetupCycleDisplay();
    }


    //
    // TriggerPrimaryEvent
    //
    // Triggers the current primary event using the current state info
    //
    void TriggerPrimaryEvent()
    {
      ASSERT(Player::GetCurrentPlayer());

      // Grab a shortcut
      CycleInfo &i = data.cInfo;

      // Ignore event if window changes during one cycle
      if (!i.gameWnd.Alive())
      {
        //LOG_DIAG(("Mouse not over a game window, but primary event triggered"));
        return;
      }

      // Set the default order modifier
      Orders::Modifier modifier = Orders::FLUSH;
    
      // Now change the modifier based on the user's input
      if (Common::Input::GetModifierKey(1))
      {
        // If control is held, append all orders
        modifier = Orders::APPEND;
      }
      else

      // If shift is held, prepend all orders
      if (Common::Input::GetModifierKey(3))
      {
        modifier = Orders::PREPEND;
      }

      // Process the current primary event
      switch (i.pEvent)
      {
        // Select a single object
        case PE_SELECT:
        {
          SelectUnit(i.o.unit);         
          break;
        }

        // Order units to move
        case PE_MOVE:
        case PE_MOVEATTACK:
        {
          ASSERT(i.t.cell);

          // Do we have a squad selected ?
          if (data.squad.Alive())
          {
            Orders::Squad::Move::Generate(
              GetPlayer(), 
              data.squad.Id(), 
              i.t.pos, 
              (i.pEvent == PE_MOVEATTACK) ? TRUE : FALSE, FALSE,
              modifier);
          }
          else
          {
            UpdateSelectedLists();
            Orders::Game::Move::Generate(
              GetPlayer(), 
              i.t.pos, 
              (i.pEvent == PE_MOVEATTACK) ? TRUE : FALSE, 
              modifier);
          }

          CreateParticle(i.t.pos, 0x12EA8886); // "Client::Move"
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          TriggerClientMode(CM_NONE);

          break;
        }

        // Order units to attack
        case PE_ATTACK:
        case PE_ATTACKNOMOVE:
        { 
          UpdateSelectedLists();

          // Attack a specific object
          if (i.o.map.Alive())
          {
            // Do we have a squad selected ?
            if (data.squad.Alive())
            {
              Orders::Squad::Attack::Generate(
                GetPlayer(), 
                data.squad.Id(), 
                i.o.map->Id(), 
                (i.pEvent == PE_ATTACKNOMOVE) ? FALSE : TRUE,
                modifier);
            }
            else
            {
              Orders::Game::Attack::Generate(
                GetPlayer(), 
                i.o.map->Id(), 
                (i.pEvent == PE_ATTACKNOMOVE) ? FALSE : TRUE,
                modifier);
            }

            TriggerResponse(data.sList, 0xA8FEF90A); // "Attack"
          }
          else

          // Attack the terrain
          if (i.t.cell)
          {   
            // Do we have a squad selected ?
            if (data.squad.Alive())
            {
              Orders::Squad::Attack::Generate(
                GetPlayer(), 
                data.squad.Id(), 
                i.t.pos, 
                (i.pEvent == PE_ATTACKNOMOVE) ? FALSE : TRUE,
                modifier);
            }
            else
            {
              Orders::Game::Attack::Generate(
                GetPlayer(), 
                i.t.pos, 
                (i.pEvent == PE_ATTACKNOMOVE) ? FALSE : TRUE,
                modifier);
            }

            CreateParticle(i.t.pos, 0xAE1A0306); // "Client::Attack"
            TriggerResponse(data.sList, 0xA8FEF90A); // "Attack"
          }

          TriggerClientMode(CM_NONE);
          break;
        }

        // Order units to guard
        case PE_GUARD:
        { 
          UpdateSelectedLists();

          // Attack a specific object
          if (i.o.map.Alive())
          {
            // Do we have a squad selected ?
            if (data.squad.Alive())
            {
              Orders::Squad::Guard::Generate(GetPlayer(), data.squad.Id(), i.o.map->Id(), modifier);
            }
            else
            {
              Orders::Game::Guard::Generate(GetPlayer(), i.o.map->Id(), modifier);
            }

            TriggerResponse(data.sList, 0x3381FB36); // "Move"
          }
          else

          // Attack the terrain
          if (i.t.cell)
          {   
            // Do we have a squad selected ?
            if (data.squad.Alive())
            {
              Orders::Squad::Guard::Generate(GetPlayer(), data.squad.Id(), i.t.pos, modifier);
            }
            else
            {
              Orders::Game::Guard::Generate(GetPlayer(), i.t.pos, modifier);
            }

            TriggerResponse(data.sList, 0x3381FB36); // "Move"
          }

          TriggerClientMode(CM_NONE);
          break;
        }

        // Order units to turn
        case PE_TURN:
        { 
          UpdateSelectedLists();
          Orders::Game::Turn::Generate(GetPlayer(), i.t.pos, modifier);
          TriggerClientMode(CM_NONE);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        // Order units to collect resource
        case PE_COLLECT:
        {
          UpdateSelectedLists();
          Orders::Game::Collect::Generate(GetPlayer(), i.o.map->Id(), FALSE, modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        // Order units to store resource
        case PE_STORE:
        {
          UpdateSelectedLists();
          Orders::Game::Store::Generate(GetPlayer(), i.o.map->Id(), FALSE, FALSE, modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        // Jump scroll to a terrain location
        case PE_JUMPSCROLL:
        {
          Viewer::GetCurrent()->LookAt(i.t.pos.x, i.t.pos.z);
          TriggerClientMode(CM_NONE);
          break;
        }

        // Construct an object
        case PE_CONSTRUCT:
        {
          // Generate the order
          UpdateSelectedLists();
          Orders::Game::Build::Generate(GetPlayer(), data.constructType, i.t.pos, data.dir, modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"

          // Clear the selection
          if (!Common::Input::GetModifierKey(1))
          {
            ClearSelected();
          }
          break;
        }

        // Set the restore facility of an object
        case PE_SETRESTORE:
        {
          UpdateSelectedLists();
          Orders::Game::SetRestore::Generate(GetPlayer(), i.o.map->Id(), modifier);
          TriggerClientMode(CM_NONE);
          break;
        }

        // Order units to make a formation
        case PE_FORMATION:
        {
          // Do we have a squad selected
          if (data.squad.Alive())
          {     
            Orders::Squad::Formation::Generate(GetPlayer(), data.squad.Id(), data.formation, i.t.pos, PIBY2 - WorldCtrl::GetCompassAngle(data.dir), modifier);
          }
          else
          {
            UpdateSelectedLists();

            Orders::Game::Formation::Generate(GetPlayer(), data.formation, i.t.pos, PIBY2 - WorldCtrl::GetCompassAngle(data.dir));
          }
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          TriggerClientMode(CM_NONE);
          break;
        }

        // Use an offmap object
        case PE_USEOFFMAP:
        {
          ASSERT(GetOffMapObject())

          // Get the object or terrain position
          const Vector *p = GetTerrainOrObjectPosition();

          ASSERT(p)

          // "Trigger::Positional"
          Orders::Game::OffMap::Generate(GetPlayer(), GetOffMapObject()->Id(), 0x63417A92, p);

          // Create attack particle
          if (GetOffMapObject()->HasProperty(0x3ACCA5E6)) // "Client::OffMapAttack"
          {
            CreateParticle(*p, 0xAE1A0306); // "Client::Attack"
          }

          break;
        }

        case PE_RESTOREMOBILE:
        {
          UpdateSelectedLists();
          Orders::Game::RestoreMobile::Generate(GetPlayer(), i.o.unit->Id(), modifier);
          TriggerResponse(data.sList, 0x5463CB0D); // "Restore"
          break;
        }

        case PE_RESTORESTATIC:
        {
          UpdateSelectedLists();
          Orders::Game::RestoreStatic::Generate(GetPlayer(), i.o.unit->Id(), modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        case PE_BOARD:
        {
          UpdateSelectedLists();
          Orders::Game::Board::Generate(GetPlayer(), i.o.unit->Id(), modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        case PE_TRANSPORT:
        {
          UpdateSelectedLists();
          Orders::Game::Transport::Generate(GetPlayer(), i.o.unit->Id(), modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        case PE_UNLOADCARGO:
        {
          ASSERT(i.t.cell)
          UpdateSelectedLists();
          Orders::Game::Unload::Generate(GetPlayer(), i.t.pos, modifier);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          ClearSelected();
          TriggerClientMode(CM_NONE);
          break;
        }

        case PE_SETRALLY:
        {
          ASSERT(i.t.cell)
          UpdateSelectedLists();
          Orders::Game::SetRally::Generate(GetPlayer(), i.t.cellX, i.t.cellZ);
          TriggerClientMode(CM_NONE);
          break;
        }

        case PE_TRAILSELECT:
          Trail::TriggerEvent(0x21C3C89F); // "Primary::Select"
          break;

        case PE_TRAILPOINT:
          Trail::TriggerEvent(0x1B71FF8E); // "Primary::Point"
          break;

        case PE_TRAILNOPOINT:
          break;

        case PE_TRAILAPPLY:
          Trail::TriggerEvent(0x407AC5DE, modifier); // "Primary::Apply"
          break;

        case PE_WALLACTIVATE:
        case PE_WALLDEACTIVATE:
        {
          UpdateSelectedLists();
          Orders::Game::Wall::Generate(GetPlayer(), i.o.unit->Id());

          ASSERT(i.o.unit.Alive());

          ClearSelected();
          break;
        }

        case PE_INFILTRATE:
        {
          UpdateSelectedLists();
          Orders::Game::Infiltrate::Generate(GetPlayer(), i.o.unit->Id());
          TriggerClientMode(CM_NONE);
          TriggerResponse(data.sList, 0xA54F900C); // "Infiltrate"
          break;
        }

        case PE_MORPH:
        {
          UpdateSelectedLists();
          Orders::Game::Morph::Generate(GetPlayer(), i.o.unit->Id());
          TriggerClientMode(CM_NONE);
          TriggerResponse(data.sList, 0x3381FB36); // "Move"
          break;
        }

        case PE_PLAYERMARKER:
        {
          Orders::Game::PlayerMarker::Generate(GetPlayer(), i.t.cellX, i.t.cellZ);
          TriggerClientMode(CM_NONE);
          break;
        }
      }
    }


    //
    // TriggerSecondaryEvent
    //
    // Triggers a secondary event using the current state info
    //
    void TriggerSecondaryEvent()
    {
      // Ignore event if window changes during one cycle
      if (!data.cInfo.gameWnd.Alive())
      {
        //LOG_DIAG(("Mouse not over a game window, but secondary event triggered"));
        return;
      }

      // Do actions based on the primary event
      switch (data.cInfo.pEvent)
      {
        // Switch from construction to move mode
        case PE_CONSTRUCT:
        case PE_NOCONSTRUCT:
          TriggerClientMode(CM_NONE); 
          return;
      }

      // Does the trail system want this click
      if (Trail::TriggerEvent(0xDA91B0DB)) // "Secondary::Offer"
      {
        return;
      }

      // Deselect any current objects
      if (data.sList.GetCount())
      {
        ClearSelected();
      }
      else
      {
        // Deactivate the orders menu
        if (data.controls.orders.Alive())
        {
          data.controls.orders->Deactivate();
        }
      }
    }


    //
    // CaptureDragSelect
    //
    // Callback for drag selection
    //
    void CaptureDragSelect(CaptureEvent event)
    {
      ASSERT(event >= 0 && event < CSE_MAX);

      switch (event)
      {
        case CSE_INIT :
        {
          // Get the current game window
          Common::GameWindow *ctrl = GetGameWindow();

          if (!ctrl)
          {
            ERR_FATAL(("Expected to find a game window"));
          }

          // Hide the cursor
          Input::ShowCursor(FALSE);

          // Set the selection mode
          data.selectMode = ctrl->HasProperty("PostDrawSelect") ? SM_POSTDRAW : SM_PREDRAW;

          break;
        }

        case CSE_MOUSEMOVE :
        {
          // Update the select box for display
          data.mouseRect.Set(data.cInfo.mouse, data.mouseStart);
          data.mouseRect.Sort();
          break;
        }

        case CSE_DONE :
        {
          // Do we need to clear the current selection
          if (!Common::Input::GetModifierKey(3))
          {
            ClearSelected();
          }

          // Get the current game window
          Common::GameWindow *ctrl = GetGameWindow();

          if (!ctrl)
          {
            ERR_FATAL(("Expected to find a game window"));
          }

          // Do the group selection
          ctrl->SelectGroup(data.mouseRect, data.sList, Team::GetDisplayTeam());

          if (data.sList.GetCount())
          {
            UnitsSelected();
            TriggerResponse(data.sList, 0x9FF22134); // "Select"
          }

          // Intentional fall-through
        }
      
        case CSE_ABORT :
        {
          // Restore normal conditions
          Input::ShowCursor(TRUE);
          data.selectMode = SM_INACTIVE;
          break;
        }
      }
    }


    //
    // CaptureRightScroll
    //
    // Callback for drag selection
    //
    void CaptureRightScroll(CaptureEvent event)
    {
      ASSERT(event >= 0 && event < CSE_MAX);

      switch (event)
      {
        case CSE_INIT :
          Viewer::GetCurrent()->Notify(Viewer::VN_RBUTTON, TRUE);
          Input::ShowCursor(FALSE);
          break;

        case CSE_MOUSEMOVE :
          Viewer::GetCurrent()->Notify(Viewer::VN_MOUSEMOVE, Input::MouseDelta().x, Input::MouseDelta().y);
          break;

        case CSE_DONE :    
        case CSE_ABORT :
          Viewer::GetCurrent()->Notify(Viewer::VN_RBUTTON, FALSE);
          Input::SetMousePos(data.mouseStart.x, data.mouseStart.y);
          Input::ShowCursor(TRUE);
          break;
      }
    }


    //
    // CaptureMidScroll
    //
    // Callback for middle mouse button
    //
    void CaptureMidScroll(CaptureEvent event)
    {
      ASSERT(event >= 0 && event < CSE_MAX);

      switch (event)
      {
        case CSE_INIT :
          Viewer::GetCurrent()->Notify(Viewer::VN_MBUTTON, TRUE);
          Input::ShowCursor(FALSE);
          break;

        case CSE_MOUSEMOVE :
          Viewer::GetCurrent()->Notify(Viewer::VN_MOUSEMOVE, Input::MouseDelta().x, Input::MouseDelta().y);
          break;

        case CSE_DONE :    
        case CSE_ABORT :
          Viewer::GetCurrent()->Notify(Viewer::VN_MBUTTON, FALSE);
          Input::SetMousePos(data.mouseStart.x, data.mouseStart.y);
          Input::ShowCursor(TRUE);
          break;
      }
    }


    //
    // TriggerCaptureEvent
    //
    // Generates a capture event
    //
    void TriggerCaptureEvent(CaptureEvent event)
    {
      // Does a handler have the capture
      if (data.captureHandler)
      {
        switch (event)
        {
          // Non-terminal events
          case CSE_INIT :
          case CSE_MOUSEMOVE :
            data.captureHandler(event);
            break;

          // Terminal events
          case CSE_DONE :
          case CSE_ABORT :
            data.captureHandler(event);
            data.captureHandler = NULL;
            break;
        }
      }
    }


    //
    // InitCaptureHandler
    //
    // Sets the current capture handler and generates the CSE_INIT event
    //
    void InitCaptureHandler(CaptureCallBack *func)
    {
      ASSERT(func);

      // Abort any current capture
      TriggerCaptureEvent(CSE_ABORT);

      // Set the new handler
      data.captureHandler = func;

      // And initialize it
      TriggerCaptureEvent(CSE_INIT);
    }


    //
    // GameWindowHandler
    //
    // Event handler for any active game window
    //
    Bool FASTCALL GameWindowHandler(Event &e, Common::GameWindow *gameWindow)
    {
      ASSERT(gameWindow);

      // Should we ignore events
      if (!data.processing)
      {
        return (TRUE);
      }

      // Input events
      if (e.type == Input::EventID())
      {
        switch (e.subType)
        {
          // A mouse button has been pressed
          case Input::MOUSEBUTTONDOWN :
          {
            // If we do not already have mouse capture
            if (gameWindow != IFace::GetCapture())
            {
              ASSERT(!data.captureHandler);

              // Get mouse capture
              gameWindow->GetMouseCapture();

              // Save mouse code
              data.captureCode = e.input.code;

              // Save starting position
              data.mouseStart = Point<S32>(e.input.mouseX, e.input.mouseY);
            }

            return (TRUE);
          }

          // The mouse has moved
          case Input::MOUSEMOVE :
          {
            // Do we have capture
            if (gameWindow == IFace::GetCapture())
            {
              // If we don't yet have a handler
              if (!data.captureHandler)
              {
                // Is the left mouse down
                if (data.captureCode == Input::LeftButtonCode())
                {
                  // Have we moved far enough
                  if (PointThreshold(data.mouseStart, Point<S32>(e.input.mouseX, e.input.mouseY), 15, 15))
                  {
                    // Start the left mouse drag select
                    InitCaptureHandler(CaptureDragSelect);
                  }
                }
                else

                // Is the right mouse down
                if (data.captureCode == Input::RightButtonCode())
                {
                  // Have we moved far enough
                  if (PointThreshold(data.mouseStart, Point<S32>(e.input.mouseX, e.input.mouseY), 10, 10))
                  {
                    // Start the right mouse button scroll
                    InitCaptureHandler(CaptureRightScroll);
                  }
                }

                // Is the middle mouse down
                if (data.captureCode == Input::MidButtonCode())
                {
                  // Have we moved far enough
                  if (PointThreshold(data.mouseStart, Point<S32>(e.input.mouseX, e.input.mouseY), 10, 10))
                  {
                    // Start the right mouse button scroll
                    InitCaptureHandler(CaptureMidScroll);
                  }
                }
              }

              // If we have a handler (set above - do not use 'else')
              if (data.captureHandler)
              {
                // Notify that the mouse has moved
                TriggerCaptureEvent(CSE_MOUSEMOVE);
              }
            }
          
            return (TRUE);
          }

          // A mouse button has been released
          case Input::MOUSEBUTTONUP :
          {
            // We have capture and it's the button that got the capture
            if (gameWindow == IFace::GetCapture() && (e.input.code == data.captureCode))
            {
              // Did something have event capture
              if (data.captureHandler)
              {
                // Send the done message
                TriggerCaptureEvent(CSE_DONE);
              
                // Release mouse capture (order is important!)
                gameWindow->ReleaseMouseCapture();
              }
              else
              {
                // Release mouse capture (order is important!)
                gameWindow->ReleaseMouseCapture();

                // Left mouse button released
                if (e.input.code == Input::LeftButtonCode())
                {
                  TriggerPrimaryEvent();
                }
                else

                // Right mouse button released
                if (e.input.code == Input::RightButtonCode())
                {
                  TriggerSecondaryEvent();
                }
              }
            }

            return (TRUE);
          }

          // Mouse-wheel axis has changed
          case Input::MOUSEAXIS:
          {
            // Tell the camera
            Viewer::GetCurrent()->Notify(Viewer::VN_MOUSEAXIS, S32(e.input.ch) / 120);
            return (TRUE);
          }
        }
      }
      else 
    
      // Interface events
      if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::LOSTCAPTURE:
          {
            // Abort any capture operation
            TriggerCaptureEvent(CSE_ABORT);
            return (TRUE);
          }
        }
      }

      return FALSE;
    }


    //
    // PostEventProcessing
    //
    // Do post-event processing
    //
    static void PostEventProcessing()
    {
    }


    //
    // SetProcessing
    //
    // Set event processing state
    //
    void SetProcessing(Bool state)
    {
      if (state)
      {
        // Turning event processing on
      }
      else
      {
        // Turning event processing off
      
        // Take capture from game window
        if (data.captureHandler && IFace::GetCapture())
        {
          IFace::SetMouseCapture(NULL);
        }
      }
      data.processing = state;
    }


    //
    // Process
    //
    // Called before the render methods
    //
    void Process()
    {
      // Increment the cycle counter
      data.cycleCount++;

      // Process input and interface events if the app window is active
      if (Main::active)
      {
        IFace::Process(TRUE, SetupCycleInfo);

        #ifdef DEMO
          #pragma message("Client disabled")
        #else
          // Setup common cycle info
          SetupCommonCycle();
        #endif
      }

      // Do post-event processing
      PostEventProcessing();

      // Process network messages
      MultiPlayer::Process();
    }


    //
    // SetPrimaryCursorName
    //
    // Sets the name of a primary event cursor
    //
    static void SetPrimaryCursorName(U32 event, const char *cursor)
    {
      ASSERT(event >= 0 && event < PE_MAX);
      eventCursors[event] = cursor;
    }

    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      // Reset base cycle data
      data.cInfo.mouse.Set(0, 0);
      data.cInfo.displayListCount = 0;
      data.cInfo.oneUnit = NULL;
      data.cInfo.gameWnd = NULL;

      // Setup event system data
      data.processing = TRUE;
      data.cycleCount = 0;
      data.lastUpdate = 0;
      data.clientMode = CM_NONE;
      data.selectMode = SM_INACTIVE;
      data.captureHandler = NULL;
      data.mouseStart.Set(0, 0);
      data.mouseRect.Set(0, 0, 0, 0);
      data.sListInfo = new UnitObjInfo;
      data.squad = NULL;
      data.constructType = NULL;
      data.hiliteConstructType = NULL;
      data.dir = WorldCtrl::SOUTH;

      data.formation = 0;

      // Interface controls
      data.controls.construction = NULL;
      data.controls.context = NULL;
      data.controls.orders = NULL;
      data.controls.facility = NULL;
      data.controls.trails = NULL;

      // Ensure all cursor names are initialized
      for (U32 i = 0; i < PE_MAX; i++)
      {
        SetPrimaryCursorName(i, "ClientNone");
      }

      // Now initialize actual cursor names
      SetPrimaryCursorName(PE_NONE, "ClientNone");
      SetPrimaryCursorName(PE_SELECT, "ClientSelect");
      SetPrimaryCursorName(PE_MOVE, "ClientMove");
      SetPrimaryCursorName(PE_NOMOVE, "ClientNoMove");
      SetPrimaryCursorName(PE_ATTACK, "ClientAttack");
      SetPrimaryCursorName(PE_NOATTACK, "ClientNoAttack");
      SetPrimaryCursorName(PE_MOVEATTACK, "ClientMoveAttack");
      SetPrimaryCursorName(PE_ATTACKNOMOVE, "ClientAttackNoMove");
      SetPrimaryCursorName(PE_TURN, "ClientTurn");
      SetPrimaryCursorName(PE_COLLECT, "ClientCollect");
      SetPrimaryCursorName(PE_STORE, "ClientStore");
      SetPrimaryCursorName(PE_JUMPSCROLL, "ClientJumpScroll");
      SetPrimaryCursorName(PE_CONSTRUCT, "ClientConstruct");
      SetPrimaryCursorName(PE_NOCONSTRUCT, "ClientNoConstruct");
      SetPrimaryCursorName(PE_GUARD, "ClientGuard");
      SetPrimaryCursorName(PE_NOGUARD, "ClientNoGuard");
      SetPrimaryCursorName(PE_FORMATION, "ClientFormation");
      SetPrimaryCursorName(PE_SETRESTORE, "ClientSetRestore");
      SetPrimaryCursorName(PE_NOSETRESTORE, "ClientNoSetRestore");
      SetPrimaryCursorName(PE_USEOFFMAP, "ClientUseOffMap");
      SetPrimaryCursorName(PE_NOUSEOFFMAP, "ClientNoUseOffMap");
      SetPrimaryCursorName(PE_RESTOREMOBILE, "ClientRestoreMobile");
      SetPrimaryCursorName(PE_RESTORESTATIC, "ClientRestoreStatic");
      SetPrimaryCursorName(PE_BOARD, "ClientBoard");
      SetPrimaryCursorName(PE_TRANSPORT, "ClientTransport");
      SetPrimaryCursorName(PE_UNLOADCARGO, "ClientUnloadCargo");
      SetPrimaryCursorName(PE_NOUNLOADCARGO, "ClientNoUnloadCargo");
      SetPrimaryCursorName(PE_SETRALLY, "ClientSetRally");
      SetPrimaryCursorName(PE_NOSETRALLY, "ClientNoSetRally");
      SetPrimaryCursorName(PE_TRAILSELECT, "ClientTrailSelect");
      SetPrimaryCursorName(PE_TRAILPOINT, "ClientTrailPoint");
      SetPrimaryCursorName(PE_TRAILNOPOINT, "ClientTrailNoPoint");
      SetPrimaryCursorName(PE_TRAILAPPLY, "ClientTrailApply");
      SetPrimaryCursorName(PE_WALLACTIVATE, "ClientWallActivate");
      SetPrimaryCursorName(PE_WALLNOACTIVATE, "ClientWallNoActivate");
      SetPrimaryCursorName(PE_WALLDEACTIVATE, "ClientWallDeactivate");
      SetPrimaryCursorName(PE_MORPH, "ClientMorph");
      SetPrimaryCursorName(PE_NOMORPH, "ClientNoMorph");
      SetPrimaryCursorName(PE_INFILTRATE, "ClientInfiltrate");
      SetPrimaryCursorName(PE_NOINFILTRATE, "ClientNoInfiltrate");
      SetPrimaryCursorName(PE_PLAYERMARKER, "ClientPlayerMarker");
      SetPrimaryCursorName(PE_NOPLAYERMARKER, "ClientNOPlayerMarker");
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      prevSelectList.Clear();
      prevCycleList.Clear();
      data.sList.Clear();
      data.rigs.Clear();
      data.placeFoot.Done();
      delete data.sListInfo;
    }
  }
}
