///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Demo recording/playback
//
// 25-MAR-1999
//

#ifndef __DEMO_H
#define __DEMO_H


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Demo
//
namespace Demo
{
  // Save options
  enum
  {
    SAVE_ORDERS = 0x0001,
    SAVE_CAMERA = 0x0002,
    SAVE_ALL    = 0xFFFF
  };

  // Initialise
  void Init();

  // Shutdown
  void Done();

  // Is the demo loading?
  Bool IsLoading();

  // Is the demo playing?
  Bool IsPlaying();

  // PostLoad the demo
  void PostLoad();

  // Pre simulation processing
  void InitSimulation();

  // Post simulation processing
  void DoneSimulation();

  // Inform that all blocks have been loaded
  void Loaded();

  // Demo processing
  void Process();

  // Save demo
  Bool Save(const char *file, U32 flags = SAVE_ALL);

  // Play demo
  Bool Play(const char *file, const char *camera = NULL);

  // Benchmark
  Bool Benchmark(const char *file, const char *camera = NULL);
}

#endif
