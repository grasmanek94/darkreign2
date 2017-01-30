///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Physics System
//
// 09-SEP-98
//


#ifndef __PHYSICS_CONST_H
#define __PHYSICS_CONST_H


///////////////////////////////////////////////////////////////////////////////
//
// Namespace PhysicsConst - Physics constants
//

namespace PhysicsConst
{
  //
  // Metric conversions
  //

  // Metres per second to kilometres per hour
  const F32 MPS2KMH = 3.6F;

  // Kilometres per hour to metres per second
  const F32 KMH2MPS = 1.0F / MPS2KMH;


  //
  // Physical constants
  //

  // Gravity on earth in ms^2
  const F32 EARTHGRAVITY = 9.8F;

  // Gravity on the moon in ms^2
  const F32 MOONGRAVITY = 1.6F;


  //
  // Other constants
  //

  // Epsilon for physics system
  const F32 EPSILON = 1E-03F;

  // Epsilon squared
  const F32 EPSILON2 = 1E-06F;

}

#endif
