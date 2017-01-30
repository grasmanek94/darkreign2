///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_host.h"
#include "multiplayer_controls_context.h"
#include "multiplayer_cmd_private.h"
#include "tagobjdec.h"
#include "strategic.h"
#include "strategic_object.h"
#include "iface.h"
#include "difficulty.h"
#include "campaigns.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {
    
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class TeamPlayerOptions
    //


    //
    // Constructor
    //
    TeamPlayerOptions::TeamPlayerOptions(IControl *parent)
    : ICWindow(parent),
      type(0),
      online(FALSE)
    {
      // Create vars
      startLocationCurrent = new IFaceVar(this, CreateInteger("startLocationCurrent", Settings::Mission::NoStartLocation));
      startLocationSelected = new IFaceVar(this, CreateInteger("startLocationSelected", Settings::Mission::NoStartLocation));
      colorCurrent = new IFaceVar(this, CreateInteger("colorCurrent", 0));
      colorSelected = new IFaceVar(this, CreateInteger("colorSelected", 0));
      side = new IFaceVar(this, CreateString("side", ""));
      difficulty = new IFaceVar(this, CreateString("difficulty", ""));
      aiPersonality = new IFaceVar(this, CreateString("aipersonality", ""));
    }


    //
    // Destructor
    //
    TeamPlayerOptions::~TeamPlayerOptions()
    {
      delete startLocationCurrent;
      delete startLocationSelected;
      delete colorCurrent;
      delete colorSelected;
      delete side;
      delete difficulty;
      delete aiPersonality;
    }


    //
    // Activate
    //
    Bool TeamPlayerOptions::Activate()
    {
      const Mission *mission = NULL;
      
      // Get the mission data
      if (Data::Get(&mission))
      {
        // Get the mission
        if (const Missions::Mission *m = Missions::FindMission(mission->mission, mission->missionFolder))
        {
          // Is this mission in a campaign
          if (Campaigns::Find(m))
          {
            // Do not allow this control to activate for campaign missions
            return (FALSE);
          }
        }
      }

      if (ICWindow::Activate())
      {
        // Activate vars
        ActivateVar(startLocationCurrent);
        ActivateVar(startLocationSelected);
        ActivateVar(colorCurrent);
        ActivateVar(colorSelected);
        ActivateVar(side);
        ActivateVar(difficulty);
        ActivateVar(aiPersonality);

        // Clear the selected start location
        startLocationSelected->SetIntegerValue(Settings::Mission::NoStartLocation);

        // Check the type of context we are dealing with
        type = 0;

        if (context.Alive())
        {
          PlayerList::TeamItem *item = NULL;

          if ((item = IFace::Promote<PlayerList::TeamItem>(context)) != NULL)
          {
            teamId = item->GetTeamId();

            const Team *team = NULL;
            if (Data::Get(&team, item->GetTeamId()))
            {
              // Is this control refer to an AI owned team?
              if (team->ai)
              {
                type |= CFG_AITEAM;

                // Notify control of type of object we are dealing with
                SendNotify(this, 0xCD5C9374); // "Multiplayer::Context::Message::AI"
              }
              else
              {
                type |= CFG_TEAM;

                // Notify control of type of object we are dealing with
                SendNotify(this, 0x04C5BD70); // "Multiplayer::Context::Message::Human"
              }

              // Pass team id to all child controls 
              NotifyAllChildren(ContextMsg::SetTeam, FALSE, item->GetTeamId());

              // Initialise team color
              colorCurrent->SetIntegerValue(team->color);
              colorSelected->SetIntegerValue(team->color);

              // Initialise side group
              side->SetStringValue(team->side.str);

              // Initialise difficulty
              Difficulty::Setting *setting = Difficulty::FindSetting(team->difficulty);
              if (setting)
              {
                difficulty->SetStringValue(setting->GetName().str);
              }
              else
              {
                difficulty->SetStringValue("");
              }

              // Initialize personality
              if (team->ai)
              {
                Mods::Mod *mod = Mods::GetMod(Mods::Types::Personality, team->personality);

                if (mod)
                {
                  aiPersonality->SetStringValue(mod->GetName().str);
                }
                else
                {
                  aiPersonality->SetStringValue("");
                }
              }

              // If a start location is selected, and that start location 
              // has a fixed side, force the side to take the fixed side 
              // and disable the control

              SendNotify(this, 0xA04C9900); // "Multiplayer::Context::Message::EnableSide"

              if (team->startLocation != Settings::Mission::NoStartLocation)
              {
                if (PrivData::haveMission)
                {
                  Game::Preview::TeamInfo *teamInfo = PrivData::preview->FindTeamCrc(team->startLocation);

                  if (teamInfo && teamInfo->GetSideFixed())
                  {
                    // Force the side to be this value
                    side->SetStringValue(teamInfo->GetSide().str);

                    // Disable side selection
                    SendNotify(this, 0x9B875856); // "Multiplayer::Context::Message::DisableSide"
                  }
                }
              }
            }
          }
        }
        else
        {
          NotifyAllChildren(ContextMsg::ClearTeam, FALSE);
          LOG_ERR(("Expecting context control"))
        }

        online = TRUE;

        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Deactivate
    //
    Bool TeamPlayerOptions::Deactivate()
    {
      if (ICWindow::Deactivate())
      {
        online = FALSE;

        startLocationCurrent->Deactivate();
        startLocationSelected->Deactivate();
        colorCurrent->Deactivate();
        colorSelected->Deactivate();
        side->Deactivate();
        difficulty->Deactivate();
        aiPersonality->Deactivate();

        return (TRUE);
      }
      return (FALSE);
    }


    //
    // TeamPlayerOptions::Notify
    //
    // Handle notifications
    //
    void TeamPlayerOptions::Notify(IFaceVar *var)
    {
      ICWindow::Notify(var);

      if (!online)
      {
        return;
      }

      if (var == startLocationSelected)
      {
        // Send a command set the start location of the current team
        const Options *options;
        if (Data::Get(&options) && (options->placement == Settings::Options::Fixed))
        {
          Commands::Data::SetStartLocation data;
          data.teamId = teamId;
          data.startLocation = startLocationSelected->GetIntegerValue();
          Data::SendHost(Commands::SetStartLocation, data);
        }
      }
      else if (var == colorSelected)
      {
        // Send a command set the color of the current team
        Commands::Data::SetTeamColor data;
        data.teamId = teamId;
        data.color = colorSelected->GetIntegerValue();
        Data::SendHost(Commands::SetTeamColor, data);
      }
      else if (var == side)
      {
        // Set the side
        Commands::Data::SetTeamSide data;
        data.teamId = teamId;
        data.side = side->GetStringValue();
        Data::SendHost(Commands::SetTeamSide, data);
      }
      else if (var == difficulty)
      {
        // Set the difficulty
        Commands::Data::SetDifficulty data;
        data.teamId = teamId;
        data.difficulty = Crc::CalcStr(difficulty->GetStringValue());
        Data::SendHost(Commands::SetDifficulty, data);
      }
      else if (var == aiPersonality)
      {
        // Set the personality
        Commands::Data::SetPersonality data;
        data.teamId = teamId;
        data.personality = Crc::CalcStr(aiPersonality->GetStringValue());
        Data::SendHost(Commands::SetPersonality, data);
      }
    }
  }
}
