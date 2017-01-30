///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Movie Player
//
// 29-APRIL-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "movieplayer.h"
#include "vid_public.h"
//#include "vid_decl.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MoviePlayer - Used to play a Bink movie
//


//
// Constructor
//
MoviePlayer::MoviePlayer() : bitmap(NULL)
{
}


//
// Destructor
//
MoviePlayer::~MoviePlayer()
{
  Stop();
}



//
// Start
//
// Start the given movie
//
Bool MoviePlayer::Start(const char *file, Bool stretch)
{
  // Stop any current movie
  Stop();

  // Allocate a bitmap
  bitmap = new Bitmap(bitmapSURFACE);

  // Match the pixel format
  bitmap->SetPixFormat(Vid::backBmp.PixelFormat());

  // Attempt to load the bink
  if (bitmap->LoadBink(file, TRUE, stretch))
  {
    // Activate it
    bitmap->BinkSetActive(1);

    // Clear the screen
    Vid::ClearBack();
    Vid::RenderFlush();
    Vid::ClearBack();
    Vid::RenderFlush();
    Vid::ClearBack();

    // Success
    return (TRUE);
  }
  else
  {
    Stop();

    // Failed
    return (FALSE);
  }
}


//
// Stop
//
// Stop any current movie
//
void MoviePlayer::Stop()
{
  if (bitmap)
  {
    delete bitmap;
    bitmap = NULL;
  }
}


//
// Active
//
// Is there a movie playing
//
Bool MoviePlayer::Active()
{
  return (bitmap && !bitmap->BinkDone());
}
