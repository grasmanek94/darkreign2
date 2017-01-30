///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Command points display
//
// 05-APR-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_commandpoints.h"
#include "iface.h"
#include "iface_util.h"
#include "team.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  //
  // Constructor
  //
  CommandPoints::CommandPoints(IControl *parent) 
  : IControl(parent)
  {
    clr1.Set(0L, 255L, 0L);
    clr2.Set(255L, 0L, 0L);
  }


  //
  // Setup
  //
  void CommandPoints::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x163F5FFA: // "Color1"
        IFace::FScopeToColor(fScope, clr1);
        break;

      case 0x1B7C7923: // "Color2"
        IFace::FScopeToColor(fScope, clr2);
        break;

      default:
        IControl::Setup(fScope);
        break;
    }
  }


  //
  // Event handler
  //
  U32 CommandPoints::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::DISPLAYTIP:
        {
          Team *team;
          if ((team = Team::GetDisplayTeam()) != NULL)
          {
            // Adjust tip text string
            SetTipText(
              TRANSLATE(("#game.client.infogroup.unitlimit", 2, S32(team->GetUnitLimit()) - team->GetUnitLimitBalance(), team->GetUnitLimit())),
              TRUE);
          }

          // Display the tip in base class
          break;
        }
      }
    }

    return (IControl::HandleEvent(e));
  }


  //
  // DrawSelf
  //
  void CommandPoints::DrawSelf(PaintInfo &pi)
  {
    Team *team;

    if (((team = Team::GetDisplayTeam()) != NULL) && team->GetUnitLimit())
    {
      F32 percent = Clamp<F32>(0.0F, F32(S32(team->GetUnitLimit()) - team->GetUnitLimitBalance()) / F32(team->GetUnitLimit()), 1.0F);

      // Interpolate color
      Color topColor;
      topColor.Interpolate(clr1, clr2, percent);

      // Adjust top of rectangle
      pi.client.p0.y = pi.client.p1.y - Utils::FtoL(F32(pi.client.Height()) * percent); 

      // Render a gradient
      IFace::RenderGradient(pi.client, topColor, clr1, TRUE, texture, pi.alphaScale);
    }
  }
}
