///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 29-JAN-1998
//


#ifndef __ICCLASS_H
#define __ICCLASS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class IControl;


///////////////////////////////////////////////////////////////////////////////
//
// Class IControlClass
//
// Registered Interface control classes
//
class ICClass
{
public:
  // Handler function that is able to instantiate a control of a specified type
  typedef IControl* (CREATEPROC)(U32, IControl *, U32);

  GameIdent       type;
  GameIdent       base;
  Bool            derived;
  FScope         *scope;
  CREATEPROC     *createProc;

public:
  ICClass() : derived(FALSE), scope(NULL), createProc(NULL) {}

  ~ICClass()
  {
    if (scope)
    {
      delete scope;
    }
  }
};

#endif
