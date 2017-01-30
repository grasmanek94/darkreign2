///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//



///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer_pingdisplay.h"
#include "iface_util.h"


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
  namespace PingDisplay
  {

    const U32 pingClamp = 3000;
    const U32 minLineValue = 32;
    const U32 maxLineValue = 800;
    const F32 maxLineAlpha = 0.9f;
    const F32 scaleLineAlpha = 0.75f;

    //
    // Draw
    //
    void Draw(U32 numPings, const U16 *pings, const ClipRect &rect, F32 alphaScale)
    {
      // Work out how many pings from the width
      U32 num = (rect.Width() - 4) / 4; 
      if (num < numPings)
      {
        numPings = num;
      }

      // Work out what the maximum is of the ping data
      U16 pingMax = 0;
      for (U32 p = 0; p < numPings; ++p)
      {
        if (pings[p] != U16_MAX)
        {
          pingMax = Max(pingMax, pings[p]);
        }
      }

      // Clamp to 3 seconds
      pingMax = U16(Clamp<U16>(0, pingMax, pingClamp));

      // From that maximum, calculate the number of pixels per second
      U32 lineValue = maxLineValue;
      U32 lines = pingMax / lineValue + 1;
      U32 pixels = rect.Height() / lines;

      while (pixels > 7 && lineValue > minLineValue)
      {
        lineValue /= 2;
        lines = pingMax / lineValue + 1;
        pixels = rect.Height() / lines;
      }

      // Now, lets draw the recent pings onto the display
      for (p = 0; p < numPings; ++p)
      {
        Color color1;
        Color color2;
        U16 ping;

        if (pings[p] == U16_MAX)
        {
          ping = U16(lines * lineValue);

          // A loss, make it blue
          color1 = Color(0.0f, 0.5f, 1.0f, alphaScale);
          color2 = color1;
          color2.LightenInline(128);
        }
        else
        {
          ping = U16(Clamp<U16>(0, pings[p], pingClamp));

          if (ping <= 1000)
          {
            color1 = Color(F32(ping) * 0.001f, 1.0f - F32(ping) * 0.0005f, 0.0f, alphaScale);
          }
          else if (ping <= 2000)
          {
            color1 = Color(1.0f, 1.0f - F32(ping) * 0.0005f, 0.0f, alphaScale);
          }
          else
          {
            color1 = Color(1.0f, 0.0f, 0.0f, alphaScale);
          }
          color2 = color1;
          color2.DarkenInline(200);
        }

        IFace::RenderGradient
        (
          ClipRect
          (
            rect.p1.x - (p * 4) - 2 - 4, rect.p1.y - (ping * pixels / lineValue),
            rect.p1.x - (p * 4) - 2, rect.p1.y + 1
          ), 
          color1, 
          color2
        );
      }

      U32 divisions = maxLineValue / lineValue;

      // Draw some lines to give an indication of the scale
      for (U32 l = 0; l <= lines; ++l)
      {
        U32 d = divisions - 1;
        F32 alpha = alphaScale * maxLineAlpha;

        while (d & l)
        {
          alpha *= scaleLineAlpha;
          d >>= 1;
        }

        IFace::RenderRectangle
        (
          ClipRect
          (
            rect.p0.x, rect.p1.y - (l * pixels), 
            rect.p1.x, rect.p1.y - (l * pixels) + 1
          ), 
          Color(1.0f, 1.0f, 1.0f, alpha),
          NULL, 
          alphaScale
        );
      }

    }

  }

}


