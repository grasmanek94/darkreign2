///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Movie Player
//
// 29-APRIL-2000
//


#ifndef __MOVIEPLAYER_H
#define __MOVIEPLAYER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "bitmap.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class MoviePlayer - Used to play a Bink movie
//
class MoviePlayer
{
protected:

  Bitmap *bitmap;

public:

  // Constructor and destructor
  MoviePlayer();
  ~MoviePlayer();

  // Start the given movie
  Bool Start(const char *file, Bool stretch = TRUE);

  // Stop any current movie
  void Stop();

  // Is there a movie playing
  Bool Active();
};


#endif