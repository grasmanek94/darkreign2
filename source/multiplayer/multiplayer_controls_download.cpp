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
#include "multiplayer_controls_download.h"
#include "multiplayer_download.h"
#include "stdload.h"
#include "iface.h"
#include "iface_util.h"
#include "font.h"


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
    // Class Download
    //


    //
    // Download::Download
    //
    // Constructor
    //
    Download::Download(IControl *parent)
    : ICWindow(parent),
      heightProgress(10)
    {
    }


    //
    // Download::Setup
    //
    void Download::Setup(FScope *fScope)
    {
      switch (fScope->NameCrc())
      {
        case 0xF39D93DD: // "OffsetFile"
          StdLoad::TypePoint<S32>(fScope, offsetFile);
          break;

        case 0xEFC39332: // "OffsetTransferred"
          StdLoad::TypePoint<S32>(fScope, offsetTransferred);
          break;

        case 0xECA04E02: // "OffsetRate"
          StdLoad::TypePoint<S32>(fScope, offsetRate);
          break;

        case 0x92BD62DD: // "OffsetETA"
          StdLoad::TypePoint<S32>(fScope, offsetETA);
          break;

        case 0x5592337B: // "OffsetProgress"
          StdLoad::TypePoint<S32>(fScope, offsetProgress);
          break;

        case 0x4669855C: // "HeightProgress"
          heightProgress = StdLoad::TypeU32(fScope);
          break;

        default:
          ICWindow::Setup(fScope);
          break;
      }
    }


    //
    // HandleEvent
    //
    U32 Download::HandleEvent(Event &e)
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
              case DownloadMsg::Halt:

                // Abort the download
                MultiPlayer::Download::AbortDownload();

                // Handled
                return (TRUE);
            }
          }
        }
      }
      return (ICWindow::HandleEvent(e));
    }


    //
    // DrawSelf
    //
    void Download::DrawSelf(PaintInfo &pi)
    {
      DrawCtrlBackground(pi);
      DrawCtrlFrame(pi);

      const MultiPlayer::Download::Context &context = MultiPlayer::Download::GetDownloadContext();

      CH buff[128];
      const CH *ch;
      
      // Draw the name of the file
      ch = Utils::Ansi2Unicode(context.file.str);
      pi.font->Draw
      (
        pi.client.p0.x + offsetFile.x,
        pi.client.p0.y + offsetFile.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Draw the amount transferred
      if (context.size < 1000)
      {
        Utils::Sprintf(buff, 128, L"%.1f/%.1fB", F32(context.transferred), F32(context.size));
      }
      else if (context.size < 1000000)
      {
        Utils::Sprintf(buff, 128, L"%.1f/%.1fkB", F32(context.transferred) / 1000.0f, F32(context.size) / 1000.0f);
      }
      else
      {
        Utils::Sprintf(buff, 128, L"%.1f/%.1fMB", F32(context.transferred) / 1000000.0f, F32(context.size) / 1000000.0f);
      }
      ch = buff;
      pi.font->Draw
      (
        pi.client.p0.x + offsetTransferred.x,
        pi.client.p0.y + offsetTransferred.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      /*
      // Draw the rate
      if (rate < 1000)
      {
        Utils::Sprintf(buff, 128, L"%.1fB/s", F32(rate));
      }
      else if (rate < 1000000)
      {
        Utils::Sprintf(buff, 128, L"%.1fkB/s", F32(rate) / 1000.0f);
      }
      else
      {
        Utils::Sprintf(buff, 128, L"%.1fMB/s", F32(rate) / 1000000.0f);
      }
      pi.font->Draw
      (
        pi.client.p0.x + offsetRate.x,
        pi.client.p0.y + offsetRate.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Draw the ETA
      if (rate)
      {
        S32 eta = remaining / rate;
        Utils::Sprintf(buff, 128, L"%d:%02d", eta / 60, eta % 60);
      }
      else
      {
        ch = L"?:??";
      }
      pi.font->Draw
      (
        pi.client.p0.x + offsetETA.x,
        pi.client.p0.y + offsetETA.y, 
        ch, 
        Utils::Strlen(ch),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );
      */

      // Draw a progress bar
      ClipRect c
      (
        pi.client.p0.x + offsetProgress.x, pi.client.p0.y + offsetProgress.y,
        pi.client.p1.x - offsetProgress.x, pi.client.p0.y + offsetProgress.y + heightProgress
      );

      IFace::RenderRectangle
      (
        c,
        Color(0.0f, 0.0f, 0.0f, pi.alphaScale), 
        NULL, 
        pi.alphaScale
      );

      if (context.size)
      {
        IFace::RenderGradient
        (
          ClipRect
          (
            c.p0.x, c.p0.y,
            c.p0.x + (c.Width() * context.transferred / context.size), c.p1.y
          ), 
          Color(0.2f, 1.0f, 1.0f, pi.alphaScale), 
          Color(0.1f, 0.5f, 0.5f, pi.alphaScale)
        );
      }

    }

  }

}
