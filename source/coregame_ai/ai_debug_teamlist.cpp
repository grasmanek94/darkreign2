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
#include "ai_debug_teamlist.h"
#include "ai_debug_info.h"
#include "iface_util.h"
#include "ifvar.h"
#include "icslider.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "team.h"
#include "strategic_object.h"
#include "console_viewer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Debug
  //
  namespace Debug
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamList
    //

    //
    // Constructor
    //
    TeamList::TeamList(IControl *parent) 
    : ICWindow(parent)
    {
      currentTeam = new IFaceVar(this, CreateString("currentTeam", ""));
    }


    //
    // Destructor
    //
    TeamList::~TeamList()
    {
      delete currentTeam;
    }


    //
    // HandleEvent
    //
    // Pass any events to the registered handler
    //
    U32 TeamList::HandleEvent(Event &e)
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
              case 0x71775671: // "Info"
              {
                // Create a log viewer
                IControl *ctrl = IFace::CreateControl("AI::Debug::Information", e.iface.p2, NULL);

                AI::Debug::Info *info = IFace::Promote<AI::Debug::Info>(ctrl);
                if (!info)
                {
                  ERR_FATAL(("Debug information class could not be promoted."))
                }

                // Get the console viewer out of it
                ConsoleViewer *log = IFace::Find<ConsoleViewer>("Config.Log.Log", ctrl);
                if (!log)
                {
                  ERR_FATAL(("Could not find ConsoleViewer named 'Config.Log.Log'"))
                }

                // Resolve the team
                Team *team = Team::Name2Team(currentTeam->GetStringValue());
                if (team)
                {
                  // Set the strategic object
                  info->SetObject(team->GetStrategicObject());

                  // Add the filter
                  log->AddFilter(team->GetConsoleId(0xEDF0E1CF)); // "Team"
                  log->AddFilter(team->GetConsoleId(0x622EF512)); // "Strategic"
                  log->AddFilter(team->GetConsoleId(0x0E39AE9E)); // "Strategic::Script"
                  log->AddFilter(team->GetConsoleId(0x29853B05)); // "Strategic::Orderer"

                  // Set the name of the window
                  char buf[64];
                  Utils::Sprintf(buf, 64, "AI Info for Team '%s'", team->GetName());
                  ctrl->SetTextString(Utils::Ansi2Unicode(buf), TRUE);

                  // Activate it
                  IFace::Activate(ctrl);
                }
                else
                {
                  delete ctrl;
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
        Team *team = Team::Id2Team(i);
        if (team)
        {
          // Build the team string
          char buffer[256];
          Strategic::Object *strategic = team->GetStrategicObject();
          Utils::Sprintf(buffer, 256, "'%s' [%s]", team->GetName(), strategic ? "AI" : "Not AI");
          teamList->AddTextItem(team->GetName(), Utils::Ansi2Unicode(buffer));
        }
      } 
    }
  }
}
