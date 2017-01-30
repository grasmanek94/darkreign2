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
#include "multiplayer_controls_mission.h"
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_host.h"
#include "multiplayer_network.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_util.h"
#include "game_missionselection.h"
#include "unitobj.h"
#include "worldctrl.h"
#include "team.h"
#include "input.h"


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
    // Class Mission
    //


    //
    // Constructor
    //
    Mission::Mission(IControl *parent)
    : ICStatic(parent),
      selectionEnabled(FALSE),
      startLocationCurrent(NULL),
      startLocationSelected(NULL)
    {
      // Default style
      controlStyle &= ~IControl::STYLE_INERT;
    }


    //
    // Destructor
    //
    Mission::~Mission()
    {
      if (startLocationCurrent)
      {
        delete startLocationCurrent;
      }
      if (startLocationSelected)
      {
        delete startLocationSelected;
      }
    }


    //
    // HandleEvent
    //
    // Pass any events to the registered handler
    //
    U32 Mission::HandleEvent(Event &e)
    {
      if (e.type == ::Input::EventID())
      {
        // Input events
        switch (e.subType)
        {
          case ::Input::MOUSEBUTTONDOWN:
          case ::Input::MOUSEBUTTONDBLCLK:
          {
            if (e.input.code == ::Input::LeftButtonCode())
            {
              Point<S32> mouse(e.input.mouseX, e.input.mouseY);
              if (InClient(mouse))
              {
                // Figure out which day (if any) was clicked on
                mouse = ScreenToClient(mouse);

                const Options *options;

                // Is selection enabled and we have a map and we're in fixed placement mode?
                if (selectionEnabled && 
                  PrivData::haveMission &&
                  Data::Get(&options) &&
                  (options->placement == Settings::Options::Fixed))
                {
                  BinTree<const Team> teams;
                  GetStartLocations(teams);

                  // Which starting location did they select (if any) ?
                  for (U32 t = 0; t < Game::MAX_TEAMS; t++)
                  {
                    Game::Preview::TeamInfo *team = PrivData::preview->FindTeamId(t);

                    if (team)
                    {
                      Point<S32> point(S32(team->GetStartPoint().x * 128.0f), S32(team->GetStartPoint().y * 128.0f));
                      if (Area<S32>(point - Point<S32>(4, 4), point + Point<S32>(4, 4)).In(mouse))
                      {
                        // They clicked on this team, is this team available ?
                        if (!teams.Find(team->GetName().crc) || (team->GetName().crc == U32(startLocationCurrent->GetIntegerValue())))
                        {
                          // This team is available, set it to our team
                          startLocationSelected->SetIntegerValue(team->GetName().crc);
                        }
                      }
                    }
                  }
                  teams.UnlinkAll();
                }
              }
            }
            else if (e.input.code == ::Input::RightButtonCode())
            {
              SendNotify(this, MissionMsg::ClearStartLocation);
            }
            break;
          }
        }
      }
      else if (e.type == IFace::EventID())
      {
        switch (e.subType)
        {
          case IFace::NOTIFY:
          {
            // Do specific handling
            switch (e.iface.p1)
            {
              case MissionMsg::EnableSelection:
                selectionEnabled = TRUE;
                break;

              case MissionMsg::ClearStartLocation:
                if (selectionEnabled)
                {
                  startLocationSelected->SetIntegerValue(Settings::Mission::NoMission);
                }
                break;
            }
            break;
          }

          case IFace::DISPLAYMODECHANGED:
          {
            if (PrivData::preview)
            {
              PrivData::preview->ReloadTextures();
            }
            break;
          }
        }
      }
      return (FALSE);  
    }


    //
    // Redraw self
    //
    void Mission::DrawSelf(PaintInfo &pi)
    {
      // Attempt to get all the current info
      const MultiPlayer::Mission *mission;
      const PlayerInfo *playerInfo;
      const Team *team;

      // Check that we have a mission and all of the data
      if 
      (
        !PrivData::haveMission ||
        !Data::Get(&mission) ||
        mission->mission == Settings::Mission::NoMission ||
        !Data::Get(&playerInfo, Network::GetCurrentPlayer().GetId()) ||
        !Data::Get(&team, playerInfo->teamId)
      )
      {
        controlStyle |= STYLE_TRANSPARENT;
        SetImage(NULL);
        ICStatic::DrawSelf(pi);
        return;
      }

      // Get the teams by start location
      BinTree<const Team> teams;
      GetStartLocations(teams);

      if (selectionEnabled)
      {
        // Update the current start location
        startLocationCurrent->SetIntegerValue(team->startLocation);

        /*
        // Check to see if the selected start location is available
        if (teams.Find(startLocationSelected->GetIntegerValue()) || startLocationSelected->GetIntegerValue() == Settings::Mission::NoStartLocation)
        {
          // Set the selected location back to the current
          startLocationSelected->SetIntegerValue(startLocationCurrent->GetIntegerValue());
        }
        */
      }

      // This doesnt need to be done every frame
      ASSERT(PrivData::preview)

      if (PrivData::preview->GetTerrainTexture())
      {
        controlStyle &= ~STYLE_TRANSPARENT;
        TextureInfo texture(PrivData::preview->GetTerrainTexture(), TextureInfo::TM_CENTRED);
        SetImage(&texture);
      }

      ICStatic::DrawSelf(pi);

      // Are we showing fixed start locations or random start locations

      // Calculate the fade color

      // Get a fraction from 0 to 512
      U32 fraction = Clock::Time::Ms() % 512;

      Color fade(1.0f, 1.0f, 1.0f);
      if (fraction >= 256)
      {
        fade.a = U8(512 - fraction);
      }
      else
      {
        fade.a = U8(fraction);
      }

      const Options *options;

      if (Data::Get(&options))
      {
        switch (options->placement)
        {
          case Settings::Options::Random:
          {
            for (U32 t = 0; t < Game::MAX_TEAMS; t++)
            {
              Game::Preview::TeamInfo *team = PrivData::preview->FindTeamId(t);

              if (team)
              {
                Point<S32> point(S32(team->GetStartPoint().x * 128.0f), S32(team->GetStartPoint().y * 128.0f));

                ClipRect c(
                  pi.client.p0.x + point.x - 4, 
                  pi.client.p0.y + point.y - 4, 
                  pi.client.p0.x + point.x + 4, 
                  pi.client.p0.y + point.y + 4);

                IFace::RenderGradient(c, fade, 150);
              }
            }
            break;
          }

          case Settings::Options::Fixed:
          {
            // Figure out which teams own which start locations
            for (U32 t = 0; t < Game::MAX_TEAMS; t++)
            {
              Game::Preview::TeamInfo *team = PrivData::preview->FindTeamId(t);

              if (team)
              {
                // Is there a team claiming to have this start location ?
                const Team *teamInfo = teams.Find(team->GetName().crc);

                Color color;

                if (teamInfo)
                {
                  color = GetTeamColor(teamInfo->color);
                }
                else
                {
                  color = fade;
                }

                Point<S32> point(S32(team->GetStartPoint().x * 128.0f), S32(team->GetStartPoint().y * 128.0f));

                ClipRect c(
                  pi.client.p0.x + point.x - 4, 
                  pi.client.p0.y + point.y - 4, 
                  pi.client.p0.x + point.x + 4, 
                  pi.client.p0.y + point.y + 4);

                IFace::RenderGradient(c, color, 150);

                if (selectionEnabled)
                {
                  // Is this the selected start location ?
                  if (U32(startLocationSelected->GetIntegerValue()) == team->GetName().crc)
                  {
                    IFace::RenderGradient(
                      ClipRect(c.p0.x - 2, c.p0.y - 2, c.p1.x + 2, c.p1.y + 2),
                      ::Color(1.0f, 1.0f, 1.0f, 0.4f), 
                      ::Color(0.5f, 0.5f, 0.5f, 0.4f));
                  }
                  // Is this the current start location ?
                  else if (U32(startLocationCurrent->GetIntegerValue()) == team->GetName().crc)
                  {
                    IFace::RenderGradient(
                      ClipRect(c.p0.x - 2, c.p0.y - 2, c.p1.x + 2, c.p1.y + 2),
                      ::Color(1.0f, 1.0f, 1.0f, 0.15f),
                      ::Color(0.5f, 0.5f, 0.5f, 0.15f)); 
                  }
                }
              }
            }
            break;
          }
        }
      }
      teams.UnlinkAll();
    }


    //
    // Setup control
    //
    void Mission::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xF90F9AB3: // "StartLocCurVar"
          ConfigureVar(startLocationCurrent, fScope);
          break;

        case 0x028D8FFD: // "StartLocSelVar"
          ConfigureVar(startLocationSelected, fScope);
          break;

        default:
          IControl::Setup(fScope);
          break;
      }
    }
  }
}
