///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


#ifndef __MULTIPLAYER_CONTROLS_EARTH_H
#define __MULTIPLAYER_CONTROLS_EARTH_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icstatic.h"
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
    // NameSpace EarthMsg
    //
    namespace EarthMsg
    {
      const U32 Upload   = 0x93976F49; // "MultiPlayer::Earth::Message::Upload"
      const U32 Download = 0x92CD3BAC; // "MultiPlayer::Earth::Message::Download"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Earth
    //
    class Earth : public IControl
    {
      PROMOTE_LINK(Earth, IControl, 0xDE16052E) // "Earth"

    public:

      struct Precomp
      {
        F32 sine;
        F32 cosine;
      };

    private:

      // Precomputed row data
      Precomp rows[256];

      // Precomputed column data
      Precomp columns[2][256];

      // Earth Bitmaps
      Bitmap *earth[2];
      
      // Lit Bitmaps
      Bitmap *lit[2];

      // Last time drawn
      U32 lastRedraw;

      // Latitude var
      IFaceVar *latitude;
      IFaceVar *longitude;

    public:

      // Constructor and Destructor
      Earth(IControl *parent);
      ~Earth();

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      // Setup
      void Setup(FScope *fScope);

      // Control activation
      Bool Activate();

      // Control deactivation
      Bool Deactivate();

      // HandleEvent
      U32 HandleEvent(Event &e);

    private:

      void Update(Bool force);

    };

  }

}


#endif
