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

#include "studio_brush_trail.h"
#include "viewer.h"
#include "common.h"
#include "iface.h"
#include "console.h"


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
    // Class Trail - Trail editing brush
    //

    //
    // Constructor
    //
    Trail::Trail(const char *name) : Base(name), 
      updateRequired(FALSE), 
      mode(TrailObj::MODE_TWOWAY)
    {
      // Create interface vars
      varSelected = CreateString("selected", "");
      varEdit = CreateString("edit", "");
      varMode = CreateString("mode", "TwoWay");

      // Find the trail list
      listBox = IFace::Find<ICListBox>("List", config);

      if (!listBox.Alive())
      {
        ERR_FATAL(("Trail brush requires ListBox child named 'List'"));
      }
    }


    //
    // Destructor
    //
    Trail::~Trail()
    {
      list.DisposeAll();
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Trail::HasProperty(U32 property)
    {
      return (Base::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Trail::NotifyVar(IFaceVar *var)
    {
      // The trail mode var
      if (var == varMode)
      {
        // Get the new mode
        mode = TrailObj::StringToMode(varMode->GetStringValue());

        // Do we need to modify the selected trail
        if (selectedTrail.Alive() && selectedTrail->GetMode() != mode)
        {        
          selectedTrail->SetMode(mode);
        }
      }
      else
      
      if (var == varSelected)
      {
        // Find this trail
        selectedTrail = TrailObj::Find(varSelected->GetStringValue(), data.team);

        // Do we need to modify the selected mode
        if (selectedTrail.Alive())
        {
          // Ensure mode is correct
          varMode->SetStringValue(TrailObj::ModeToString(selectedTrail->GetMode()));
        }
      }

      // Pass notification down
      Base::NotifyVar(var);
    }


    //
    // GenerateList
    //
    // Build the trail instance list
    //
    void Trail::GenerateList()
    {
      // Remember the selected item
      GameString previous(varSelected->GetStringValue());

      // Clear the list
      listBox->DeleteAllItems();

      // Fill with trails for current team
      for (NList<TrailObj>::Iterator i(&TrailObj::GetTrails()); *i; i++)
      {
        // Get the trail
        TrailObj *t = *i;

        // Does the team match
        if (t->GetTeam() == data.team)
        {
          listBox->AddTextItem(t->GetName(), NULL);
        }
      } 

      // Sort the list using the name
      listBox->Sort();

      // Set the selected item
      varSelected->SetStringValue(previous.str);

      // Set the new team name
      teamName = data.team ? data.team->GetName() : "";
    }


    //
    // ClearPoints
    //
    // Clear current points
    //
    void Trail::ClearPoints()
    {
      list.DisposeAll();
    }

   
    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Trail::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x457659C0: // "System::PostDeletionPoll"
        {
          // Does the trail data need to be updated
          if (updateRequired)
          {
            // Rebuild the trail list
            GenerateList();

            // Clear update flag
            updateRequired = FALSE;
          }
          
          // Always pass system events down
          break;
        }

        case 0x8C6EE5DF: // "System::PostEventPoll"
        {
          // Get the team we currently reflect
          Team *team = Team::Name2Team(teamName.str);

          // If the team has been deleted or team has changed
          if ((!teamName.Null() && !team) || (team != data.team))
          {
            updateRequired = TRUE;
          }        

          // Always pass system events down
          break;
        }

        case 0x5AB0F116: // "System::Activated"
        {
          // Update on activation
          updateRequired = TRUE;

          // Always pass system events down
          break;
        }

        case 0x1E05A1B9: // "Brush::Trail::Message::Activate"
        {
          if (selectedTrail.Alive())
          {
            list.Set(selectedTrail->GetList());
          }
          return;
        }

        case 0xC6E3C9D5: // "Brush::Trail::Message::Create"
        {
          // Get the new trail name
          const char *name = varEdit->GetStringValue();

          // Try and create the trail
          TrailObj *trail = TrailObj::Create(data.team, (Utils::Strlen(name) > 0) ? name : NULL);

          if (trail)
          {
            // Set the point list
            trail->GetList().Set(list);

            // Set the mode
            trail->SetMode(mode);
            
            // Clear local points
            ClearPoints();

            // Set the selected item
            varSelected->SetStringValue(name);

            // Clear the edit box
            varEdit->SetStringValue("");

            // Data update is now required
            updateRequired = TRUE;
          }
          return;
        }

        case 0xDA2656D6: // "Command::Execute"
        case 0xECDA1D6E: // "Brush::Trail::Message::Apply"
        {
          // Do we have a selected trail and some local points
          if (selectedTrail.Alive() && list.GetCount())
          {
            selectedTrail->GetList().Set(list);
            ClearPoints();
          }
          return;
        }

        case 0x008CF4B8: // "Command::Delete"
        case 0xDDEF7E04: // "Brush::Trail::Message::Delete"
        {
          if (list.GetCount())
          {
            ClearPoints();
          }
          else

          if (selectedTrail.Alive())
          {
            // Delete the object
            selectedTrail->MarkForDeletion();

            // Data update is now required
            updateRequired = TRUE;
          }
          return;
        }

        case 0x27546BF9: // "Brush::LeftMouseClick"
        {
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            U32 x = data.cInfo.mTerrain.cellX;
            U32 z = data.cInfo.mTerrain.cellZ;

            // Find any existing trail at this cell
            TrailObj *trail = TrailObj::Find(x, z, data.team);

            // Clicked on a trail and no local points
            if (trail)
            {
              if (!list.GetCount())
              {
                varSelected->SetStringValue(trail->GetName());
              }
            }
            else

            if (!list.Find(x, z))
            {
              list.AppendPoint(x, z);
            }
          }
          return;
        }

        case 0x39E83C58: // "Command::Cancel"
        {
          ClearPoints();
          listBox->ClearSelected();
          return;
        }

        case 0x0ACEA1BD: // "Brush::RightMouseClick"
        {
          // Unlink the last point
          TrailObj::WayPoint *p = list.UnlinkTail();

          if (p)
          {
            delete p;
          }         
          else
          {
            listBox->ClearSelected();
          }
          return;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          // Get the currently selected trail
          TrailObj *selected = selectedTrail.Alive() ? selectedTrail.GetData() : NULL;

          // Setup the normal and selected colors
          Color normal(0.0F, 0.8F, 0.0F, 0.6F);
          Color select(0.0F, 0.0F, 0.8F, 0.6F);

          // Draw all current trails
          for (NList<TrailObj>::Iterator i(&TrailObj::GetTrails()); *i; i++)
          {
            // Get this trail
            TrailObj *trail = *i;

            if (trail->GetTeam() == data.team)
            {
              Common::Display::WayPoints
              (
                trail->GetList(), (trail == selected) ? select : normal, trail->GetMode()
              );
            }           
          }

          // Draw the edit points
          Common::Display::WayPoints(list, Color(0.8F, 0.0F, 0.0F, 0.6F), mode);

          // Always pass system events down
          break;
        }
      }
      
      // Not blocked at this level
      Base::Notification(crc, e);
    }
  }
}
