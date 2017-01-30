///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_DOWNLOAD_H
#define __MULTIPLAYER_CONTROLS_DOWNLOAD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "icwindow.h"


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
    // NameSpace DownloadMsg
    //
    namespace DownloadMsg
    {
      const U32 Halt = 0xD70B1311; // "MultiPlayer::Download::Message::Halt"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Download
    //
    class Download : public ICWindow
    {
      PROMOTE_LINK(Download, ICWindow, 0x9763293B) // "Download"

    private:

      // Offsets
      Point<S32> offsetFile;
      Point<S32> offsetTransferred;
      Point<S32> offsetRate;
      Point<S32> offsetETA;
      Point<S32> offsetProgress;
      S32 heightProgress;

    public:

      // Constructor
      Download(IControl *parent);

      // Setup
      void Setup(FScope *fScope);

      // HandleEvent
      U32 HandleEvent(Event &e);

      // DrawSelf
      void DrawSelf(PaintInfo &pi);

    };

  }

}


#endif
