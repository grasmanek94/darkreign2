/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Operatorion
//
// 12-NOV-1998
//

#ifndef __OPERATION_H
#define __OPERATION_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "fscope.h"


/////////////////////////////////////////////////////////////////////////////
//
// Namespace Operation
//
namespace Operation
{

  // Console operation
  void Console(const char *var, const char *op, VarSys::VarItem *param);

  // FScope operation
  void Function(FScope *fScope, void *context);

}


#endif
