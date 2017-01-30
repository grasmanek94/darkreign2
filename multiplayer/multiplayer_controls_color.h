///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_COLOR_H
#define __MULTIPLAYER_CONTROLS_COLOR_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "ifvar.h"


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
    // NameSpace ColorMsg
    //
    namespace ColorMsg
    {
      const U32 SetColor = 0xEE632C05; // "MultiPlayer::Color::Message::SetColor"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Color
    //
    class Color : public IControl
    {
    private:

      // Current color
      IFaceVar *colorCurrent;

      // Selected color
      IFaceVar *colorSelected;

      // Team id
      U32 teamId;

    public:

      // Constructor
      Color(IControl *parent);

      // Destructor
      ~Color();

      // HandleEvent
      U32 HandleEvent(Event &e);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      // Setup the control
      void Setup(FScope *fScope);

    };

  }

}


#endif
