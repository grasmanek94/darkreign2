///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Font system
//
// 15-APR-1998
//


#ifndef __FONTSYS_H
#define __FONTSYS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "font.h"
#include "filesys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FontSys
//
namespace FontSys
{

  // Flags for Create
  enum
  {
    READONLY = 0x0001
  };

  // Initialise system
  void Init();

  // Shutdown system
  void Done();

  // Create a new font
  Bool Create(const char *name, const char *path, U32 flags = 0);
  void Create(FScope *fScope);

  // Delete font
  Bool Delete(const char *name, Bool &found);
  Bool Delete(U32 crc, Bool &found);

  Font *GetFont(const char *name);
  Font *GetFont(U32 crc);

  void DeleteAll();

  // Function to be called when the video mode changes
  void OnModeChange();

  // List all fonts to the console
  void Report();

  // Write all fonts to bmp files
  void WriteFonts();

  // Allocate a texture
  Bitmap *AllocTexture(U16 &handle);

  // Find a texture
  Bitmap *FindTexture(U16 handle);

};

#endif
