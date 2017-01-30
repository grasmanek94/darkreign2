/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Switch
//

#ifndef __SWITCH_H
#define __SWITCH_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "fscope.h"


/////////////////////////////////////////////////////////////////////////////
//
// Namespace Switch
//
namespace Switch
{
  struct Value;

  // Get the value of a switch statement
  Value & GetValue(FScope *fScope, void *context);

  // Compare the value of a switch statement
  Bool CompareValue(Value &value, FScope *fScope, void *context);

  // Dispose of a value
  void DisposeValue(Value &value);

}


#endif
