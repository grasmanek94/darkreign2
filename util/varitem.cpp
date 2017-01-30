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
// Class VarItem - A single var item, such as a variable or a sub-scope
//


//
// VarSys::VarItem::TriggerCallBack
//
// Trigger parent's call back function, if any
//
void VarSys::VarItem::TriggerCallBack()
{
  ASSERT(pScope);

  // Do we have a registered call back function
  if ((flags & NOTIFY) && pScope->callBack)
  {
    // Pass it the crc of our full path
    pScope->callBack(pathCrc);
  }

  // Call each object on the notify list
  VarNotify *p = pNotify;

  while (p)
  {
    ASSERT(p != p->next);

    p->Notify(VarNotify::CHANGED);
    p = p->next;
  }  
}


//
// VarSys::VarItem::InitString
//
// Initialize as STRING type
//
void VarSys::VarItem::InitString(const char *val)
{
  ASSERT(type == VI_NONE);

  // Set the type
  type = VI_STRING;

  // Set the value
  string.val = new String(val);
}


//
// VarSys::VarItem::InitInteger
//
// Initialize as INTEGER type
//
void VarSys::VarItem::InitInteger(S32 val)
{
  ASSERT(type == VI_NONE);

  // Set the type
  type = VI_INTEGER;

  // Set the value
  integer.val = val;
}


//
// VarSys::VarItem::InitFPoint
//
// Initialize as FPOINT type
//
void VarSys::VarItem::InitFPoint(F32 val)
{
  ASSERT(type == VI_NONE);

  // Set the type
  type = VI_FPOINT;

  // Set the value
  fpoint.val = val;
}


//
// VarSys::VarItem::InitBinary
//
// Initialized as BINARY type
//
void VarSys::VarItem::InitBinary(U32 size, const U8 *val)
{
  ASSERT(type == VI_NONE);

  // Set the type
  type = VI_BINARY;

  // Set the value
  binary.maxSize = size;
  binary.size = size;
  binary.data = new U8[size];

  if (val)
  {
    // Copy the passed in value
    Utils::Memcpy(binary.data, val, size);
  }
  else
  {
    // Clear to zero
    Utils::Memset(binary.data, 0x00, size);
  }
}


//
// VarSys::VarItem::InitCmd
//
// Initialize as CMD type
//
void VarSys::VarItem::InitCmd()
{
  ASSERT(type == VI_NONE);

  // Just set the type
  type = VI_CMD;
}


//
// VarSys::VarItem::InitScope
//
// Initialize as SCOPE type (creates a new sub-scope)
//
void VarSys::VarItem::InitScope()
{
  ASSERT(type == VI_NONE);

  // Set the type
  type = VI_SCOPE;

  // Allocate a new scope
  scope.ptr = new VarScope();
}


//
// VarSys::VarItem::ScopePtr
//
// SCOPE type - returns pointer to sub-scope
//
VarSys::VarScope* VarSys::VarItem::ScopePtr()
{
  if (type != VI_SCOPE)
  {
    ERR_FATAL(("Expected '%s' to be a scope item", itemId.str))
  }
  
  ASSERT(scope.ptr);

  return (scope.ptr);
}

    
//
// VarSys::VarItem::Clear
//
// Clear this item (ie. so can be re-initialized)
//
void VarSys::VarItem::Clear()
{
  // Type specific destruction
  switch (type)
  {
    // Do nout
    case VI_NONE:
    case VI_INTEGER:   
    case VI_FPOINT:   
    case VI_CMD:   
      break;

    // Delete the string
    case VI_STRING:
      delete string.val;
      break;

    // Delete the data associated with the var
    case VI_BINARY:
      delete binary.data;
      break;

    // Delete the sub-scope
    case VI_SCOPE:
      delete scope.ptr;
      break;

    // Someone's stuffed up
    default:
      ERR_FATAL(("Unknown scope item type (%d)", type));
      break;
  }

  // Clear type
  type = VI_NONE;
}



//
// VarSys::VarItem::VarItem
//
// Constructor 
//
VarSys::VarItem::VarItem(const char *identIn, U32 pathCrcIn, VarScope *parent)
{
  ASSERT(parent);

  // Setup default values
  type = VI_NONE;
  itemId = identIn;
  pathCrc = pathCrcIn;
  pScope = parent;
  flags = DEFAULT;
  pNotify = NULL;

  // Add to search tree
  if (allItems.Add(pathCrc, this))
  {
    LOG_DIAG(("Duplicate PathCrc %s [0x%.8X]", identIn, pathCrcIn));
  }

  // Register object creation
  TrackSys::RegisterConstruction(dTrack);
}


//
// VarSys::VarItem::~VarItem
//
// Destructor
//
VarSys::VarItem::~VarItem()
{
  // Notify objects that we are dying
  VarNotify *p = pNotify;

  while (p)
  {
    VarNotify *temp = p;

    p = p->next;
    temp->Notify(VarNotify::DELETED);
  }

  ASSERT(!pNotify);

  // Register object destruction
  TrackSys::RegisterDestruction(dTrack);

  // Free up data
  Clear();

  // Remove from search tree
  allItems.Unlink(this);
}


//
// VarSys::VarItem::GetStringValue
//
// Get the string value of a var
//
const char * VarSys::VarItem::GetStringValue()
{
  static char buff[32];

  switch (type)
  {
    default:
    case VarSys::VI_SCOPE:
    case VarSys::VI_BINARY:
      ERR_FATAL(("Unsupported type for getting string value"))
      break;

    case VarSys::VI_INTEGER:
    {
      Utils::Sprintf(buff, 32, "%d", Integer());
      return (buff);
      break;
    }

    case VarSys::VI_FPOINT:   
    {
      Utils::Sprintf(buff, 32, "%f", Float());
      return (buff);
      break;
    }

    case VarSys::VI_STRING:
    {
      return (Str());
      break;
    }
  }
}


//
// VarSys::VarItem::GetUnicodeStringValue
//
const CH * VarSys::VarItem::GetUnicodeStringValue()
{
  return (Utils::Ansi2Unicode(GetStringValue()));
}


//
// VarSys::VarItem::Str
//
// STRING type - returns string value
//
String VarSys::VarItem::Str()
{
  if (type != VI_STRING)
  {
    ERR_FATAL(("Expected '%s' to be a string var (%d)", itemId.str, type))
  }

  return (*(string.val));
}


//
// VarSys::VarItem::SetStr
//
// STRING type - sets string value
//
void VarSys::VarItem::SetStr(const char *newVal)
{
  if (type != VI_STRING)
  {
    ERR_FATAL(("Expected '%s' to be a string var (%d)", itemId.str, type))
  }

  // Set the new value
  string.val->Dup(newVal);

  // Value may have changed, trigger call back
  TriggerCallBack();
}


//
// VarSys::VarItem::Integer
//
// INTEGER type - returns integer value
//
S32 VarSys::VarItem::Integer()
{
  if (type != VI_INTEGER)
  {
    ERR_FATAL(("Expected '%s' to be an integer var (%d)", itemId.str, type))
  }

  return (integer.val);
}


//
// VarSys::VarItem::SetInteger
//
// INTEGER type - sets integer value
//
void VarSys::VarItem::SetInteger(S32 newVal)
{
  if (type != VI_INTEGER)
  {
    ERR_FATAL(("Expected '%s' to be an integer var (%d)", itemId.str, type))
  }

  // Set the new value
  integer.val = newVal;

  if (flags & CLAMP)
  {
    if (integer.val < integer.lo)
    {
      integer.val = integer.lo;
    }
    else if (integer.val > integer.hi)
    {
      integer.val = integer.hi;
    }
  }

  // Value may have changed, trigger call back
  TriggerCallBack();
}


//
// VarSys::VarItem::SetIntegerRange
//
// INTEGER type - sets value range
//
void VarSys::VarItem::SetIntegerRange(S32 low, S32 high)
{
  if (type != VI_INTEGER)
  {
    ERR_FATAL(("Expected '%s' to be an integer var (%d)", itemId.str, type))
  }

  // Set up clamping paramters
  flags |= CLAMP;
  integer.lo = low;
  integer.hi = high;
}


//
// VarSys::VarItem::Float
//
// FPOINT type - returns floating point value
//
F32 VarSys::VarItem::Float()
{
  if (type != VI_FPOINT)
  {
    ERR_FATAL(("Expected '%s' to be a floating point var (%d)", itemId.str, type))
  }

  return (fpoint.val);
}


//
// VarSys::VarItem::SetFloat
//
// FPOINT type - sets floating point value
//
void VarSys::VarItem::SetFloat(F32 newVal)
{
  if (type != VI_FPOINT)
  {
    ERR_FATAL(("Expected '%s' to be a floating point var (%d)", itemId.str, type))
  }

  // Set the new value
  fpoint.val = newVal;

  if (flags & CLAMP)
  {
    if (fpoint.val < fpoint.lo)
    {
      fpoint.val = fpoint.lo;
    }
    else if (fpoint.val > fpoint.hi)
    {
      fpoint.val = fpoint.hi;
    }
  }

  // Value may have changed, trigger call back
  TriggerCallBack();
}

    
//
// VarSys::VarItem::SetFloatRange
//
// FPOINT type - sets value range
//
void VarSys::VarItem::SetFloatRange(F32 low, F32 high)
{
  if (type != VI_FPOINT)
  {
    ERR_FATAL(("Expected '%s' to be a floating point var (%d)", itemId.str, type))
  }

  // Set up clamping paramters
  flags |= CLAMP;
  fpoint.lo = low;
  fpoint.hi = high;
}


//
// VarSys::VarItem::Binary
//
// BINARY type - returns pointer to the binary data
//
const U8 * VarSys::VarItem::Binary()
{
  if (type != VI_BINARY)
  {
    ERR_FATAL(("Expected '%s' to be an binary var (%d)", itemId.str, type))
  }

  return (binary.data);
}


//
// VarSys::VarItem::BinarySize
//
// BINARY type - returns the size of the binary item
//
U32 VarSys::VarItem::BinarySize()
{
  if (type != VI_BINARY)
  {
    ERR_FATAL(("Expected '%s' to be an binary var (%d)", itemId.str, type))
  }

  return (binary.size);
}


//
// VarSys::VarItem::SetBinary
//
// BINARY type - sets the binary value
//
void VarSys::VarItem::SetBinary(U32 size, const U8 *newVal)
{
  if (type != VI_BINARY)
  {
    ERR_FATAL(("Expected '%s' to be an binary var (%d)", itemId.str, type))
  }

  // Do we need to enlarge the binary ?
  if (size > binary.maxSize)
  {
    delete [] binary.data;
    binary.data = new U8[size];
    binary.maxSize = size;
  }

  // Save the data size
  binary.size = size;

  if (newVal)
  {
    // Copy the passed in value
    Utils::Memcpy(binary.data, newVal, size);
  }
  else
  {
    // Clear to zero
    Utils::Memset(binary.data, 0x00, size);
  }

  // Value may have changed, trigger call back
  TriggerCallBack();
}


//
// VarSys::VarItem::TriggerCmd
//
// CMD type - trigger call back
//
void VarSys::VarItem::TriggerCmd()
{
  if (type != VI_CMD)
  {
    ERR_FATAL(("Expected '%s' to be a command item (%d)", itemId.str, type))
  }

  // Value may have changed, trigger call back
  TriggerCallBack();
}


//
// VarSys::VarItem::NotifyMe
//
// Add an object to the list of objects to be notified upon change of value
//
void VarSys::VarItem::NotifyMe(VarNotify *obj)
{
  ASSERT(obj);
  ASSERT(obj->next == NULL);
  ASSERT(obj != pNotify); // Check for cyclical chain

  VarNotify *tmp = pNotify;

  pNotify = obj;
  pNotify->next = tmp;
}


//
// VarSys::VarItem::ForgetMe
//
// Deletes an object from the list of notified objects
//
void VarSys::VarItem::ForgetMe(VarNotify *obj)
{
  ASSERT(obj);

  VarNotify *p = pNotify, *prev = NULL;

  while (p)
  {
    if (p == obj)
    {
      if (prev == NULL)
      {
        // Unlink first element in list
        pNotify = p->next;
      }
      else
      {
        // Unlink from middle or end of list
        prev->next = p->next;
      }
      p->next = NULL;

      return;
    }

    prev = p;
    p = p->next;
  }

  // Not found in list
  ERR_FATAL(("ForgetMe: object not found in list"));
}


//
// VarSys::VarItem::SetFlags
//
// Set some of the flags
//
void VarSys::VarItem::SetFlags(U32 flagMask)
{
  flags |= flagMask;
}


//
// VarSys::VarItem::ClearFlags
//
// Clear some of the flags
//
void VarSys::VarItem::ClearFlags(U32 flagMask)
{
  flags &= ~flagMask;
}


