///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-JUL-1998
//


#ifndef __MAPOBJSTATE_H
#define __MAPOBJSTATE_H




//
// struct MapObjState - Low level map object states
//
struct MapObjState
{
  // All Possible object states
  enum
  {
    NONE,
    MOVE,
    FIRE,
  };


  // State identifier
  U32 id;

  // Data for each object state
  union
  {
    struct
    {
      F32 x;
      F32 z;
    } move;

    struct
    {
      U32 target;
    } fire;
  };
};


#endif
