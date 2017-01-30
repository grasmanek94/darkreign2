///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Low pass filter
//
// 16-FEB-99
//


#ifndef __LOPASSFILTER_H
#define __LOPASSFILTER_H

#include "stdload.h"

///////////////////////////////////////////////////////////////////////////////
//
// Class LoPassFilter - Low pass filter
//
template <class TYPE> class LoPassFilter
{
protected:
  Bool firstDone;
  F32 a1, b0;
  TYPE current, target;

public:

  //
  // Constructor
  //
  LoPassFilter()
  {
    Utils::Memset( this, 0, sizeof(this));
    SetSpeed( .5f);
  }
  LoPassFilter( F32 f, const TYPE & mem1)
  {
    SetSpeed( f);
    Set( mem1);
  }


  //
  // SetSpeed
  //
  void SetSpeed(F32 b0in, F32 a1in)
  {
    a1 = a1in;
    b0 = b0in;
  }
  void SetSpeed( F32 _b0) 
  {
    SetSpeed( _b0, _b0 - 1.0f);
  }

  //
  // Set
  //
  void Set( const TYPE & t)
  {
    target = t;
  }
  void SetDone( const TYPE & t)
  {
    target = t;

    if (!firstDone)
    {
      current = target;
      firstDone = TRUE;
    }
  }

  //
  // SetMemory
  //
  void SetMemory( const TYPE & y1)
  {
    current = y1;
  }
  void SetMemory()
  {
    SetMemory( target);
  }

  //
  // Update
  //
  const TYPE & Update()
  {
    return current = (target * b0) - (current * a1);
  }
  const TYPE & Update( const TYPE & t)
  {
    target = t;
    return Update();
  }

  //
  // Thresh
  //
  TYPE Thresh()
  {
    return target - current;
  }

  //
  // Current value
  //
  const TYPE & Current() const
  {
    return (current);
  }

  //
  // Target value
  //
  const TYPE & Target() const
  {
    return (target);
  }

};

class LoPassFilterF32 : public LoPassFilter<F32>
{
public:

  LoPassFilterF32()
  {
    current = target = 0;
    SetSpeed( .5f);
  }
  LoPassFilterF32( F32 f, F32 mem1)
  {
    SetSpeed( f);
    Set( mem1);
  }

  inline Bool CheckThresh()
  {
    return (F32) fabs( target - current) < .08f ;
  }

  void SaveState( FScope * fScope)
  {
    ASSERT( fScope);

    StdSave::TypeU32( fScope, "First",   firstDone);

    StdSave::TypeF32( fScope, "Current", current);
    StdSave::TypeF32( fScope, "Target",  target);
    StdSave::TypeF32( fScope, "A1",      a1);
    StdSave::TypeF32( fScope, "B0",      b0);
  }
  void LoadState( FScope * fScope)
  {
    ASSERT( fScope);

    firstDone = StdLoad::TypeU32( fScope, "First", firstDone);

    current   = StdLoad::TypeF32( fScope, "Current", current);
    target    = StdLoad::TypeF32( fScope, "Target",  target);
    a1        = StdLoad::TypeF32( fScope, "A1",      a1);
    b0        = StdLoad::TypeF32( fScope, "B0",      b0);
  }

  void SaveState( FScope * fScope, const char * name)
  {
    SaveState( fScope->AddFunction( name));
  }
  void LoadState( FScope * fScope, const char * name, Bool required = TRUE)
  {
    LoadState( fScope->GetFunction( name, required));
  }

};

#endif
