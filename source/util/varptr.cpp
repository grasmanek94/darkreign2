///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dynamic game variable management system
//
// 10-APR-1998
//

#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class VarString - Points to a String game variable
//

//
// VarString::VarString
//
// Constructor
//
VarString::VarString(const char *path, void *context)
{
  // Point at target
  PointAt(path, context);
}


//
// VarString::VarString
//
// Constructor
//
VarString::VarString(VarSys::VarItem *var)
{
  // Point at target
  PointAt(var);
}


//
// VarString::PointAt
//
// Setup/change to point to the var 'path', which MUST exist
//
void VarString::PointAt(const char *path, void *context)
{
  // Get the requested item
  VarSys::VarItem *i = VarSys::FindVarItem(path, context, TRUE);

  // Item must exist
  ASSERT(i)
  PointAt(i);
}


//
// VarString::PointAt
//
// Setup/change to point to the var
//
void VarString::PointAt(VarSys::VarItem *var)
{
  if (var->type != VarSys::VI_STRING)
  {
    ERR_FATAL(("Attempt to point a VarString at another type (%d)", var->type))
  }

  // Point at this item
  item = var;
}


//
// VarString::PointAt
//
// Setup/change to point to the VarString 'var', which MUST be setup
//
void VarString::PointAt(VarString &var)
{
  // Make sure target is valid
  if (!var.Alive())
  {
    ERR_FATAL(("Attempt to point a VarString at an invalid VarPtr"))
  }

  // Get the target item
  item = var.item;
}


//
// VarString::operator= const char *
//
// Set the value of our target variable
//
VarString& VarString::operator=(const char *val)
{
  Validate();

  if (item->Str() != val)
  {
    item->SetStr(val);
  }
  
  return (*this);
}


//
// VarString::operator= VarString
//
// Set the value of our target variable
//
VarString& VarString::operator=(VarString &val)
{
  Validate();

  item->SetStr(val);

  return (*this);
}


//
// VarString::operator const char *
//
// Return string value
//
VarString::operator const char *() 
{
  Validate();

  return (item->Str());
}


//
// VarString::operator *
//
// Explicit cast to const char *  (for printf style use)
//
const char* VarString::operator*()
{
  Validate();

  return (item->Str());
}



///////////////////////////////////////////////////////////////////////////////
//
// Class VarInteger - Points to an S32 game variable
//

//
// VarInteger::VarInteger
//
// Constructor
//
VarInteger::VarInteger(const char *path, void *context)
{
  // Point at target
  PointAt(path, context);
}


//
// VarInteger::VarInteger
//
// Constructor
//
VarInteger::VarInteger(VarSys::VarItem *var)
{
  // Point at target
  PointAt(var);
}


//
// VarInteger::PointAt
//
// Setup/change to point to the var 'path', which MUST exist
//
void VarInteger::PointAt(const char *path, void *context)
{
  // Get the requested item
  VarSys::VarItem *i = VarSys::FindVarItem(path, context, TRUE);

  // Item must exist
  ASSERT(i)
  PointAt(i);
}


//
// VarInteger::PointAt
//
// Setup/change to point to the var
//
void VarInteger::PointAt(VarSys::VarItem *var)
{
  // Item must exist
  if (var->type != VarSys::VI_INTEGER)
  {
    ERR_FATAL(("Attempt to point a VarInteger at another type (%d)", var->type))
  }

  // Point at this item
  item = var;
}


//
// VarInteger::PointAt
//
// Setup/change to point to the VarInteger 'var', which MUST be setup
//
void VarInteger::PointAt(VarInteger &var)
{
  // Make sure target is valid
  if (!var.Alive())
  {
    ERR_FATAL(("Attempt to point a VarInteger at an invalid var"))
  }

  // Get the target item
  item = var.item;
}


//
// VarInteger::operator= S32
//
// Set the value of our target variable
//
VarInteger& VarInteger::operator=(S32 val)
{
  Validate();

  item->SetInteger(val);
  
  return (*this);
}


//
// VarInteger::operator= VarInteger
//
// Set the value of our target variable
//
VarInteger& VarInteger::operator=(VarInteger &val)
{
  Validate();

  item->SetInteger(val);

  return (*this);
}


//
// VarInteger::operator S32
//
// Return S32 value
//
VarInteger::operator S32 () 
{
  Validate();

  return (item->Integer());
}


//
// VarInteger::operator *
//
// Explicit cast to S32 (for printf style use)
//
S32 VarInteger::operator*()
{
  Validate();

  return (item->Integer());
}



///////////////////////////////////////////////////////////////////////////////
//
// Class VarFloat - Points to an F32 game variable
//

//
// VarFloat::VarFloat
//
// Constructor
//
VarFloat::VarFloat(const char *path, void *context)
{
  // Point at target
  PointAt(path, context);
}


//
// VarFloat::VarFloat
//
// Constructor
//
VarFloat::VarFloat(VarSys::VarItem *var)
{
  // Point at target
  PointAt(var);
}


//
// VarFloat::PointAt
//
// Setup/change to point to the var 'path', which MUST exist
//
void VarFloat::PointAt(const char *path, void *context)
{
  // Get the requested item
  VarSys::VarItem *i = VarSys::FindVarItem(path, context, TRUE);

  // Item must exist
  ASSERT(i)
  PointAt(i);
}


//
// VarFloat::PointAt
//
// Setup/change to point to the var 'path', which MUST exist
//
void VarFloat::PointAt(VarSys::VarItem *var)
{
  if (var->type != VarSys::VI_FPOINT)
  {
    ERR_FATAL(("Attempt to point a VarFloat at another type (%d)", var->type))
  }

  // Point at this item
  item = var;
}


//
// VarFloat::PointAt
//
// Setup/change to point to the VarFloat 'var', which MUST be setup
//
void VarFloat::PointAt(VarFloat &var)
{
  // Make sure target is valid
  if (!var.Alive())
  {
    ERR_FATAL(("Attempt to point a VarFloat at an invalid var"))
  }

  // Get the target item
  item = var.item;
}


//
// VarFloat::operator= F32
//
// Set the value of our target variable
//
VarFloat& VarFloat::operator=(F32 val)
{
  Validate();

  item->SetFloat(val);
  
  return (*this);
}


//
// VarFloat::operator= VarFloat
//
// Set the value of our target variable
//
VarFloat& VarFloat::operator=(VarFloat &val)
{
  Validate();

  item->SetFloat(val);

  return (*this);
}


//
// VarFloat::operator F32
//
// Return F32 value
//
VarFloat::operator F32 () 
{
  Validate();

  return (item->Float());
}


//
// VarFloat::operator *
//
// Explicit cast to F32 (for printf style use)
//
F32 VarFloat::operator*()
{
  Validate();

  return (item->Float());
}


///////////////////////////////////////////////////////////////////////////////
//
// Class VarBinary - Points to an BINARY game variable
//


//
// VarBinary::VarBinary
//
// Constructor
//
VarBinary::VarBinary(const char *path, void *context)
{
  // Point at target
  PointAt(path, context);
}


//
// VarBinary::VarBinary
//
// Constructor
//
VarBinary::VarBinary(VarSys::VarItem *var)
{
  // Point at target
  PointAt(var);
}


//
// VarBinary::PointAt
//
// Setup/change to point to the var 'path', which MUST exist
//
void VarBinary::PointAt(const char *path, void *context)
{
  // Get the requested item
  VarSys::VarItem *i = VarSys::FindVarItem(path, context, TRUE);

  // Item must exist
  ASSERT(i)
  PointAt(i);
}


//
// VarBinary::PointAt
//
// Setup/change to point to the var
//
void VarBinary::PointAt(VarSys::VarItem *var)
{
  if (var->type != VarSys::VI_BINARY)
  {
    ERR_FATAL(("Attempt to point a VarBinary at another type (%d)", var->type))
  }

  // Point at this item
  item = var;
}


//
// VarBinary::PointAt
//
// Setup/change to point to the VarBinary 'var', which MUST be setup
//
void VarBinary::PointAt(VarBinary &var)
{
  // Make sure target is valid
  if (!var.Alive())
  {
    ERR_FATAL(("Attempt to point a VarFloat at an invalid var"))
  }

  // Get the target item
  item = var.item;
}


//
// VarBinary::Set
//
// Set the value of out target variable
//
void VarBinary::Set(U32 size, const U8 *val)
{
  Validate();

  item->SetBinary(size, val);
}


//
// VarBinary::operator= const U8 *
//
// Set the value of our target variable
//
VarBinary& VarBinary::operator=(const U8 *val)
{
  Validate();

  item->SetBinary(GetSize(), val);
  
  return (*this);
}


//
// VarBinary::operator= VarBinary
//
// Set the value of our target variable
//
VarBinary& VarBinary::operator=(VarBinary &val)
{
  Validate();

  item->SetBinary(val.GetSize(), val);

  return (*this);
}


//
// VarBinary::operator const U8 *
//
// Return const U8 * value
//
VarBinary::operator const U8 * () 
{
  Validate();

  return (item->Binary());
}


//
// VarBinary::operator *
//
// Explicit cast to const U8 * (for printf style use)
//
const U8 * VarBinary::operator*()
{
  Validate();

  return (item->Binary());
}


//
// VarBinary::GetSize
//
// Return the size of the BINARY var
//
U32 VarBinary::GetSize()
{
  Validate();

  return (item->BinarySize());
}


///////////////////////////////////////////////////////////////////////////////
//
// Class VarCmd - Points to a command item
//


//
// VarCmd::VarCmd
//
// Constructor
//
VarCmd::VarCmd(const char *path, void *context)
{
  // Point at target
  PointAt(path, context);
}


//
// VarCmd::VarCmd
//
// Constructor
//
VarCmd::VarCmd(VarSys::VarItem *var)
{
  // Point at target
  PointAt(var);
}


//
// VarCmd::PointAt
//
// Setup/change to point to the cmd 'path', which MUST exist
//
void VarCmd::PointAt(const char *path, void *context)
{
  // Get the requested item
  VarSys::VarItem *i = VarSys::FindVarItem(path, context, TRUE);

  // Item must exist
  ASSERT(i)
  if (i->type != VarSys::VI_CMD)
  {
    ERR_FATAL(("Attempt to point a VarCmd at another type (%d)", i->type))
  }

  // Point at this item
  item = i;
}


//
// VarCmd::PointAt
//
// Setup/change to point to the cmd 'path', which MUST exist
//
void VarCmd::PointAt(VarSys::VarItem *var)
{
  if (var->type != VarSys::VI_CMD)
  {
    ERR_FATAL(("Attempt to point a VarCmd at another type (%d)", var->type))
  }

  // Point at this item
  item = var;
}


//
// VarCmd::PointAt
//
// Setup/change to point to the VarCmd 'var', which MUST be setup
//
void VarCmd::PointAt(VarCmd &var)
{
  // Make sure target is valid
  if (!var.Alive())
  {
    ERR_FATAL(("Attempt to point a VarCmd at an invalid var"))
  }

  // Get the target item
  item = var.item;
}


//
// VarCmd::Trigger
//
// Trigger the command associated with this VarCmd
//
void VarCmd::Trigger()
{
  Validate();

  item->TriggerCmd();
}
