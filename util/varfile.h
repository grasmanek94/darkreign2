///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// VarSys Loading and Saving
//
// 06-JAN-1999
//


#ifndef __VARFILE_H
#define __VARFILE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace VarFile
//
namespace VarFile
{
  // Save from the Var Scope into the FScope
  void Save(FScope *fScope, VarSys::VarScope *vScope);

  // Load from the FScope into the Var Scope
  void Load(FScope *fScope, const char *path);
}


#endif
