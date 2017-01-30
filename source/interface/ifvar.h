/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 24-JUL-98
//


#ifndef __IFVAR_H
#define __IFVAR_H


#include "varsys.h"


//
// Forward declarations
//
class IControl;


///////////////////////////////////////////////////////////////////////////////
//
// Class IFaceVar - Manages a varitem
//
class IFaceVar : public VarNotify
{
protected:

  // Supported var types
  union
  {
    struct
    {
      VarInteger  *varInteger;
    };
    struct
    {
      VarFloat    *varFloat;
    };
    struct
    {
      VarString   *varString;
    };
  };

  // Var type of this item
  VarSys::VarItemType varType;

  // Pointer to the actual var
  VarPtr             *varPtr;

  // Control that is associated with this var
  IControl           *ctrl;

  // The name of this var, if it needs to be resolved
  char               *varName;

  // Is it active?
  Bool               active;

protected:

  // Setup the var
  void Init(VarSys::VarItem *item);

  // Release the var
  void Done();

  // Implementation of VarNotify::Notify
  void Notify(VarNotify::Mode);

public:

  // Initialise from a var name
  IFaceVar(IControl *ctrl, const char *var);

  // Initialise from a var item
  IFaceVar(IControl *ctrl, VarSys::VarItem *item);

  ~IFaceVar();

  // Activate the var
  void Activate();

  // Deactivate the var
  void Deactivate();

  // Set the value of the variable
  void SetValue(const char *s);

  // Retrieve a text representation of the value, with optional format string
  void GetValue(CH *buf, U32 size, const char *format = NULL, Bool translate = TRUE);
  const CH * GetValue(const char *format, Bool translate = TRUE);

  // Get/Set string value
  const char *GetStringValue();
  void SetStringValue(const char *val);

  // Get/Set the integer value
  S32  GetIntegerValue();
  void SetIntegerValue(S32 val);

  // Get/Set the floating point value
  F32  GetFloatValue();
  void SetFloatValue(F32 val);

public:

  // Return the type of the var
  VarSys::VarItemType Type() 
  { 
    ASSERT(varPtr);
    return varType; 
  }

  // Is Active
  Bool IsActive()
  {
    return (active);
  }

  // Return the var item
  VarSys::VarItem &GetItem();

  // Test if the var is valid
  Bool IsValid();

};

#endif
