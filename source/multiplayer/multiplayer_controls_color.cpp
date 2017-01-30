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
#include "multiplayer_controls_color.h"
#include "multiplayer_controls_context.h"
#include "multiplayer_private.h"
#include "multiplayer_data.h"
#include "multiplayer_host.h"
#include "multiplayer_network.h"
#include "iface.h"
#include "iface_types.h"
#include "gameconstants.h"
#include "iface_util.h"
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
    // Class Color
    //


    //
    // Constructor
    //
    Color::Color(IControl *parent)
    : IControl(parent),
      colorCurrent(NULL),
      colorSelected(NULL),
      teamId(U32_MAX)
    {
    }


    //
    // Destructor
    //
    Color::~Color()
    {
      if (colorCurrent)
      {
        delete colorCurrent;
      }
      if (colorSelected)
      {
        delete colorSelected;
      }
    }


    /*

    //
    // CanActivate
    //
    Bool Color::CanActivate()
    {
      if (IControl::CanActivate())
      {
        // Get the color for the current color
        const PlayerInfo *playerInfo;
        const Team *team;

        if 
        (
          Data::Get(PlayerInfo::GetIdent(Network::GetCurrentPlayer().GetId()), playerInfo) &&
          Data::Get(Team::GetIdent(playerInfo->teamId), team)
        )
        {
          return (TRUE);
        }
      }
      return (FALSE);
    }


    //
    // Activate
    //
    Bool Color::Activate()
    {
      if (IControl::Activate())
      {
        // Activate vars
        ActivateVar(colorCurrent);
        ActivateVar(colorSelected);

        // Get the color for the current color
        const PlayerInfo *playerInfo;
        Data::Get(PlayerInfo::GetIdent(Network::GetCurrentPlayer().GetId()), playerInfo);
        ASSERT(playerInfo);

        const Team *team;
        Data::Get(Team::GetIdent(playerInfo->teamId), team);
        ASSERT(team);

        colorCurrent->SetIntegerValue(team->color);
        colorSelected->SetIntegerValue(team->color);

        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Deactivate
    //
    Bool Color::Deactivate()
    {
      if (IControl::Deactivate())
      {
        // Deactivate vars
        colorCurrent->Deactivate();
        colorSelected->Deactivate();

        return (TRUE);
      }
      return (FALSE);
    }
    */


    //
    // HandleEvent
    //
    // Pass any events to the registered handler
    //
    U32 Color::HandleEvent(Event &e)
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

                colorSelected->SetIntegerValue(mouse.x / GetSize().y);
              }
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
              case IControlNotify::Activating:
              {
                // Activate vars
                ActivateVar(colorCurrent);
                ActivateVar(colorSelected);
                break;
              }

              case IControlNotify::Deactivated:
              {
                // Deactivate vars
                colorCurrent->Deactivate();
                colorSelected->Deactivate();
                break;
              }

              case ContextMsg::SetTeam:
              {
                teamId = e.iface.p2;
                break;
              }

              case ContextMsg::ClearTeam:
              {
                teamId = U32_MAX;
                break;
              }
            }
          }
        }
      }

      return (IControl::HandleEvent(e));  
    }


    //
    // DrawSelf
    //
    // Redraw self
    //
    void Color::DrawSelf(PaintInfo &pi)
    {
      IControl::DrawSelf(pi);

      // Get the color for the current color
      const PlayerInfo *playerInfo;
      Data::Get(&playerInfo, Network::GetCurrentPlayer().GetId());
      ASSERT(playerInfo);
      ASSERT(teamId < Game::MAX_TEAMS);

      const Team *team;
      Data::Get(&team, teamId);
      ASSERT(team);

      // Update the current color
      colorCurrent->SetIntegerValue(team->color);

      // Figure out which colors are available
      BinTree<GroupTree> groups;
      BuildPlayerHierachy(groups);
      Bool colors[Game::MAX_TEAMS + 1];

      for (U32 t = 0; t <= Game::MAX_TEAMS; t++)
      {
        colors[t] = FALSE;
      }
      for (BinTree<GroupTree>::Iterator gti(&groups); *gti; gti++)
      {
        for (BinTree<TeamTree>::Iterator tti(&(*gti)->teams); *tti; tti++)
        {
          colors[(*tti)->team->color] = TRUE;
        }
      }
      groups.DisposeAll();

      /*
      // Is the currently selected color still available ?
      if (colors[colorSelected->GetIntegerValue()])
      {
        colorSelected->SetIntegerValue(team->color);
      }
      */

      // Display all of the available colors
      for (t = 0; t <= Game::MAX_TEAMS; t++)
      {
        // Is the color available ?
        if (!colors[t] || t == team->color)
        {
          Point<S32> p(t * pi.client.Height(), 0);

          // Draw the team color
          ClipRect c(
            pi.client.p0.x + p.x + 3, 
            pi.client.p0.y + p.y + 3, 
            pi.client.p0.x + p.x + pi.client.Height() - 3, 
            pi.client.p0.y + p.y + pi.client.Height() - 3);

          IFace::RenderShadow(
            c, 
            c + IFace::GetMetric(IFace::DROPSHADOW_UP), 
            ::Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 
            0);

          IFace::RenderGradient(c, GetTeamColor(t), 150);

          // Is this the selected color ?
          if (t == U32(colorSelected->GetIntegerValue()))
          {
            IFace::RenderGradient(
              ClipRect(
                pi.client.p0.x + p.x + 1, 
                pi.client.p0.y + p.y + 1, 
                pi.client.p0.x + p.x + pi.client.Height() - 1, 
                pi.client.p0.y + p.y + pi.client.Height() - 1),
              ::Color(1.0f, 1.0f, 1.0f, 0.4f), 
              ::Color(0.5f, 0.5f, 0.5f, 0.4f));
          }
          // Is this the current color ?
          else if (t == team->color)
          {
            IFace::RenderGradient(
              ClipRect(
                pi.client.p0.x + p.x + 1, 
                pi.client.p0.y + p.y + 1, 
                pi.client.p0.x + p.x + pi.client.Height() - 1, 
                pi.client.p0.y + p.y + pi.client.Height() - 1),
              ::Color(1.0f, 1.0f, 1.0f, 0.15f),
              ::Color(0.5f, 0.5f, 0.5f, 0.15f));
          }
        }
      }
    }


    //
    // Color::Setup
    //
    // Setup the control
    void Color::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0x12F4AA17: // "CurColorVar"
          ConfigureVar(colorCurrent, fScope);
          break;

        case 0x2CF75C50: // "SelColorVar"
          ConfigureVar(colorSelected, fScope);
          break;

        default:
          IControl::Setup(fScope);
          break;
      }
    }
  }
}
