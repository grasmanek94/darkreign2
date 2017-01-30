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
// Class VarScope - A single scope, containing a list of var items
//


//
// VarSys::VarScope::CreateNewItem
//
// creates a new item within this scope, setting the parent
//
VarSys::VarItem* VarSys::VarScope::CreateNewItem(const char *identIn, U32 pathCrcIn)
{
  ASSERT(!items.Find(Crc::CalcStr(identIn)));

  // Create the item
  VarItem *item = new VarItem(identIn, pathCrcIn, this);

  // Add it to the item list
  items.Add(item->itemId.crc, item);

  // And return it
  return (item);
}


//
// VarSys::VarScope::DeleteItem
//
// Delete an item from this scope
//
void VarSys::VarScope::DeleteItem(VarItem *item)
{
  ASSERT(item);

  // Delete the item
  items.Dispose(item->itemId.crc);
}


//
// VarSys::VarScope::VarScope
//
// Constructor
//
VarSys::VarScope::VarScope()
{
  // Clear call back
  callBack = NULL;

  flags = 0;
}


//
// VarSys::VarScope::~VarScope
//
// Destructor
//
VarSys::VarScope::~VarScope()
{
  // Delete all scope items
  items.DisposeAll();
}


//
// VarSys::VarScope::FindItem
//
// Returns item with ident matching 'crc', or NULL if not found
//
VarSys::VarItem* VarSys::VarScope::FindItem(U32 crc)
{
  return (items.Find(crc));
}
