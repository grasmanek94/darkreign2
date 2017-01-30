///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_teamlist.h"
#include "studio_teameditor.h"
#include "studio_private.h"
#include "iface_util.h"
#include "ifvar.h"
#include "icslider.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TeamList
  //

  //
  // Constructor
  //
  TeamList::TeamList(IControl *parent) : 
    ICWindow(parent)
  {
    currentTeam = new IFaceVar(this, CreateString("currentTeam", ""));
    createTeam = new IFaceVar(this, CreateString("createTeam", ""));
  }


  //
  // Destructor
  //
  TeamList::~TeamList()
  {
    delete currentTeam;
    delete createTeam;
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 TeamList::HandleEvent(::Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case 0x23C19271: // "Edit"
            {
              if (data.team)
              {
                // Promote
                if (TeamEditor *editor = IFace::Find<TeamEditor>("TeamEditor"))
                {
                  // Set the color var to our own
                  editor->SetTeam(data.team);

                  // Activate it
                  IFace::Activate(editor);
                }
              }
              break;
            }

            case 0xC9FCFE2A: // "Clear"
            {
              data.team = NULL;
              break;
            }

            case 0x5989306C: // "Change"
            {
              data.team = Team::Name2Team(currentTeam->GetStringValue());
              break;
            }

            case 0xB884B9E8: // "CreateTeam"
            {
              // Ignore if already exists
              if (!Team::Name2Team(createTeam->GetStringValue()))
              {
                // Create a new team
                new Team(createTeam->GetStringValue(), Team::NewId());

                // Rebuild team list
                BuildList();
              }

              break;
            }

            case 0x74EF9BE3: // "Destroy"
            {
              // Delete the current team
              if (data.team)
              {
                // Transfer all units to another list because of upgrades
                UnitObjList list;

                for (NList<UnitObj>::Iterator u(&data.team->GetUnitObjects()); *u; ++u)
                {
                  list.Append(*u);
                }

                // Now delete each member of the team
                for (UnitObjList::Iterator i(&list); *i; ++i)
                {
                  (**i)->SetTeam(NULL);
                  (**i)->MarkForDeletion();
                }
                
                list.Clear();

                // Now it is safe to delete the team
                delete data.team;
                data.team = NULL;

                // Rebuild team list
                BuildList();
              }
              break;
            }

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool TeamList::Activate()
  {
    if (ICWindow::Activate())
    {
      // Find all the important controls
      teamList = IFace::Find<ICListBox>("TeamList", this);
      if (!teamList.Alive())
      {
        ERR_CONFIG(("Could not find 'TeamList' in the TeamList"))
      }

      // Build the list of teams
      BuildList();

      // Activate vars
      ActivateVar(currentTeam);
      ActivateVar(createTeam);
 
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool TeamList::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      currentTeam->Deactivate();
      createTeam->Deactivate();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // TeamList::BuildList
  //
  void TeamList::BuildList()
  {
    ASSERT(teamList.Alive())

    // Clear the list
    teamList->DeleteAllItems();

    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      if (Team *team = Team::Id2Team(i))
      {
        teamList->AddTextItem(team->GetName(), NULL);

        // If this was the display team select it
        if (team == data.team)
        {
          teamList->SetSelectedItem(team->GetName());
        }
      }
    } 
  }

}
