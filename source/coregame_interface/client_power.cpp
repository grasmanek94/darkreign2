///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Power Control
//
// 13-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_power.h"
#include "iface_util.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//

namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Power
  //


  //
  // Constructor
  //
  Power::Power(IControl *parent) 
  : IControl(parent),
    notchValue(100),
    notchPixelMax(16),
    notchPixelMin(2),
    notchWidth(3)
  {
  }


  //
  // Destructor
  //
  Power::~Power()
  {
  }


  //
  // Configure this control with an FScope
  //
  void Power::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x9D4C1655: // "NotchValue"
        notchValue = StdLoad::TypeU32(fScope, Range<U32>(1, U32_MAX));
        break;

      case 0xDE0C1438: // "NotchPixelMax"
        notchPixelMax = StdLoad::TypeU32(fScope, Range<U32>(1, U32_MAX));
        break;

      case 0x02D3C1A8: // "NotchPixelMin"
        notchPixelMin = StdLoad::TypeU32(fScope, Range<U32>(1, U32_MAX));
        break;

      case 0xC0BACAE5: // "NotchWidth"
        notchWidth = StdLoad::TypeU32(fScope, Range<U32>(1, U32_MAX));
        break;

      default:
        IControl::Setup(fScope);
    }
  }


  //
  // Draw this control
  //
  void Power::DrawSelf(PaintInfo &pi)
  {
    DrawCtrlBackground(pi, GetTexture());
    DrawCtrlFrame(pi);

    // Get the display team
    Team *team = Team::GetDisplayTeam();

    if (team)
    {
      // Get this teams power
      const ::Power::Team &power = team->GetPower();

      U32 available = power.GetAvailable();
      U32 availableDay = power.GetAvailableDay();
      U32 availableNight = power.GetAvailableNight();
      U32 consumed = power.GetConsumed();

      // Get the maximum
      U32 max = 0;
      max = Max(max, available);
      max = Max(max, availableDay);
      max = Max(max, availableNight);
      max = Max(max, consumed);

      // What the available height
      U32 height = pi.client.Height();
      ASSERT(height > 0)

      // How many notches are required to display this max (round up)
      div_t d = div(max, notchValue);
      U32 numNotches = d.quot + (d.rem ? 1 : 0);

      // How many pixels can we assign to each notch
      U32 notchPixels = numNotches ? Min(notchPixelMax, height / numNotches) : notchPixelMax;

      // Reduce notch pixels to the nearest power of two
      U32 mask = 0x10000000;
      do
      {
        if (mask & notchPixels)
        {
          notchPixels = mask;
          break;
        }
      }
      while (mask >>= 1);

      // We now have the number of pixels per notch, which also gives the ratio of pixels/units

      // Draw background
      IFace::RenderRectangle(
        pi.client,
        Color(0.0f, 0.0f, 0.0f), 
        NULL, 
        pi.alphaScale);

      // Display the available power
      IFace::RenderRectangle(
        ClipRect(
          pi.client.p0.x, pi.client.p1.y - notchPixels * available / notchValue,
          pi.client.p1.x, pi.client.p1.y),
        power.GetColor(), 
        NULL, 
        pi.alphaScale);

      // Display the day night difference
      IFace::RenderRectangle(
        ClipRect(
          pi.client.p0.x, pi.client.p1.y - notchPixels * availableDay / notchValue,
          pi.client.p1.x, pi.client.p1.y - notchPixels * availableNight / notchValue),
        Color(1.0f, 1.0f, 1.0f, 0.4f), 
        NULL, 
        pi.alphaScale);

      // Display the current power
      IFace::RenderRectangle(
        ClipRect(
          pi.client.p0.x, pi.client.p1.y - notchPixels * consumed / notchValue,
          pi.client.p1.x, pi.client.p1.y - notchPixels * consumed / notchValue + 1),
        Color(1.0f, 1.0f, 1.0f), 
        NULL, 
        pi.alphaScale);


      if (notchPixels > notchPixelMin)
      {
        // Draw the notches
        S32 y = pi.client.p1.y;
        while (y >= pi.client.p0.y)
        {
          IFace::RenderRectangle(
            ClipRect(
              pi.client.p0.x, y,
              pi.client.p0.x + notchWidth, y + 1),
            Color(1.0f, 1.0f, 1.0f), 
            NULL, 
            pi.alphaScale);

          IFace::RenderRectangle(
            ClipRect(
              pi.client.p1.x - notchWidth, y,
              pi.client.p1.x, y + 1),
            Color(1.0f, 1.0f, 1.0f), 
            NULL, 
            pi.alphaScale);

          y -= notchPixels;
        }

      }

    }

  }

}
