///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dynamic game variable management system
//
// 10-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"



///////////////////////////////////////////////////////////////////////////////
//
// Class VarSys - Manages the game var system
//

//
// Static data members
//
Bool VarSys::sysInit = FALSE;
VarSys::VarScope* VarSys::gScope = NULL;
NBinTree<VarSys::VarItem> VarSys::allItems(&VarSys::VarItem::allItemsNode);
NBinTree<VarSys::Substitution, char> VarSys::substitutions(&Substitution::node);


// Type string representations
char* VarSys::itemTypeStrings[VI_MAXIMUM] = 
{
  "none", "string", "integer", "float", "binary", "command", "scope"
};


//
// VarSys::SplitDelimited
//
// Splits the delimited path 'source' into two sections
//
// eg. MatchLeft  : "engine.gameplay.config" -> "engine" and "gameplay.config"
// eg. MatchRight : "engine.gameplay.config" -> "engine.gameplay" and "config"
// eg. MatchLeft  : "engine" -> "engine" and ""
// eg. MatchRight : "engine" -> "" and "engine"
//
// Returns FALSE when no delimiter is found
//
Bool VarSys::SplitDelimited(String &one, String &two, String source, Bool matchLeft)
{
  // Ensure not exceeding max string limit
  if (source.GetLength() > VARSYS_MAXVARPATH)
  {
    ERR_FATAL(("Var path '%s' exceeds max length (%d)", *source, VARSYS_MAXVARPATH))
  }

  // Get a pointer to the start of 'one'
  const char *oneStart = source;

  ASSERT(oneStart);

  // Get a pointer to the delimiter
  const char *twoStart = (matchLeft) ? 
    String::MatchLeft(source, VARSYS_SCOPEDELIM) :
      String::MatchRight(source, VARSYS_SCOPEDELIM);

  // Copy both sections
  if (twoStart)
  {    
    // Copy group
    VarPathIdent pBuf;
    Utils::Memcpy(pBuf.str, oneStart, twoStart - oneStart);
    pBuf.str[twoStart - oneStart] = '\0';
    one.Dup(pBuf.str);

    // Copy last portion
    twoStart++;
    two.Dup(twoStart);

    return (TRUE);
  }
  else
  {
    // No delimiter
    if (matchLeft)
    {
      one.Dup(oneStart);
      two.Dup("");
    }
    else
    {
      one.Dup("");   
      two.Dup(oneStart);
    }

    return (FALSE);
  }
}


// 
// VarSys::CreateVarItem
//
// Creates a new var item.  Any missing scopes will be created.  The
// last item in the path must NOT exist. The resulting varItem is NOT 
// setup but IS added to the specified scope.
//
VarSys::VarItem* VarSys::CreateVarItem(const char *path, void *context)
{
  ASSERT(sysInit);
  
  // Build the full var name
  path = BuildVarName(path, context);

  String ripped, scope1, scope2, process = path;
  VarItem *item = NULL;

  // Point current scope at global scope
  VarScope *cScope = gScope;

  // Ensure destination scope exists
  while (SplitDelimited(scope1, scope2, process, TRUE))
  {
    ASSERT(scope1.GetLength() > 0);
    ASSERT(scope2.GetLength() > 0);

    // Add to total ripped string
    ripped += scope1;

    // Does this scope already exist
    item = cScope->FindItem(scope1.Crc());

    // Check item
    if (item)
    {
      // Make sure it's a scope
      if (item->type != VI_SCOPE)
      {
        ERR_FATAL(("Unable to create '%s' since '%s' is not a scope", path, *scope1))
      }
    }
    else
    {
      // Create new item at current scope
      item = cScope->CreateNewItem(scope1, ripped.Crc());
    
      // Setup as a scope
      item->InitScope();
    }

    ASSERT(item);
    ASSERT(item->type == VI_SCOPE);

    // Point at new scope level
    cScope = item->ScopePtr();

    // Process remaining
    process = scope2;

    // Add the dot
    ripped += ".";
  }

  ASSERT(scope1.GetLength() > 0); 

  // Ensure does not exist at this level
  if (cScope->FindItem(scope1.Crc()))
  {
    ERR_FATAL(("Unable to create '%s' since item '%s' already exists", path, *scope1))  
  }

  // Create new item at current scope
  item = cScope->CreateNewItem(scope1, Crc::CalcStr(path));
  
  // And return it
  return (item);
}

  
//
// VarSys::Init
//
// Intialise the var system
//
void VarSys::Init()
{
  ASSERT(!sysInit);

  // Create global scope
  gScope = new VarScope();

  // Set init flag
  sysInit = TRUE;
};


//
// VarSys::Done
//
// Shutdown var system
//
void VarSys::Done()
{
  ASSERT(sysInit);
  ASSERT(gScope);

  // Everything should be deleted before this
  for (NBinTree<VarSys::VarItem>::Iterator i(&allItems); *i; i++)
  {
    // If at the global scope and not a command
    if ((*i)->pScope == gScope && (*i)->type != VI_CMD)
    {
      LOG_DIAG(("VarItem '%s' (%s) not being deleted", (*i)->itemId.str, GetTypeString((*i)->type)));
    }
  }

  // Delete the global scope
  delete gScope;

  // Clear init flag
  sysInit = FALSE;
};


// 
// VarSys::RegisterSubstitution
//
// Register a substitution
//
void VarSys::RegisterSubstitution(Substitution &sub)
{
  substitutions.Add(sub.ch, &sub);
}


//
// VarSys::UnregisterSubstitution
//
// Unregister a substitution
//
void VarSys::UnregisterSubstitution(Substitution &sub)
{
  substitutions.Unlink(&sub);
}


//
// VarSys::BuildVarName
//
// Find a var using substitution, or NULL if not found
//
const char * VarSys::BuildVarName(const char *path, void *context)
{
  ASSERT(path)

  // Is the first character a registered substitution ?
  Substitution *substitution;

  char ch = *path;

  if ((substitution = substitutions.Find(ch)) != NULL)
  {
    path++;

    // Make sure that the context is valid
    if (ch != substitution->ch)
    {
      context = NULL;
    }

    // Grab everything up to the first '.' and get the substitution to expand it
    const char *dot = Utils::Strchr(path, '.');
    static char buf[VARSYS_MAXVARPATH];

    if (dot)
    {
      U32 length = dot - path;
      Utils::Strncpy(buf, path, Min<U32>(length, 256));
      buf[length] = '\0';
      Utils::Strcpy(buf, substitution->Expand(buf, context));
      Utils::Strcat(buf, dot);
      return (buf);
    }
    else
    {
      Utils::Strncpy(buf, path, 256);
      return (substitution->Expand(buf, context));
    }
  }
  else
  {
    return (path);
  }
}


//
// VarSys::FindVarItem
//
// Find a var using substitution, or NULL if not found
//
VarSys::VarItem * VarSys::FindVarItem(const char *path, void *context, Bool required)
{
  const char *name = BuildVarName(path, context);

  // Split path into path and item names
  String s1, s2;
  SplitDelimited(s1, s2, name, FALSE);

  // Find using full path crc and item name
  VarSys::VarItem *item = FindVarItemHelper(Crc::CalcStr(name), *s2);

  if (required && !item)
  {
    ERR_FATAL(("Could not find var '%s'", name))
  }

  return (item);
}


//
// VarSys::FindVarItem
//
// Find a var using a path and an identifier
//
VarSys::VarItem* VarSys::FindVarItem(const char *path, const char *ident, void *context)
{
  ASSERT(sysInit);

  // Build the expanded var name
  path = BuildVarName(path, context);

  // Build full path out of path and ident
  StrCrc<VARSYS_MAXVARPATH> fullPath = path;
  Utils::Strcat(fullPath.str, VARSYS_SCOPEDELIMSTR);
  Utils::Strcat(fullPath.str, ident);
  fullPath.Update();

  return (FindVarItemHelper(fullPath.crc, ident));
}


// 
// VarSys::FindVarItemHelper
//
// Returns a pointer to the var item 'pathCrc', or NULL if not found
//
VarSys::VarItem* VarSys::FindVarItemHelper(U32 pathCrc, const char *itemName)
{
  U32 key = pathCrc;

  // CRC of item name
  U32 itemCrc = Crc::CalcStr(itemName);

  // Find first item with matching key
  NBinTree<VarSys::VarItem>::Iterator i(&allItems);

  if (i.Find(key))
  {
    do
    {
      if ((*i)->itemId.crc == itemCrc)
      {
        return (*i);
      }
    } 
    while (i.FindNext(key));
  }
  return (NULL);
}


//
// VarSys::FindVariable
//
// Returns TRUE if the variable 'path' exists
//
Bool VarSys::FindVariable(const char *path)
{
  ASSERT(sysInit);

  // Get the item
  VarItem *item = FindVarItem(path);

  // Did we find anything
  if (item)
  {
    // Check that it's a variable
    switch (item->type)
    {
      case VI_STRING:
      case VI_INTEGER:
      case VI_FPOINT:
      case VI_BINARY:
        return (TRUE);

      case VI_NONE :
      case VI_SCOPE :
        break;

      default:
        ERR_FATAL(("Unknown var item type"))
    }   
  }

  // Didn't find or wrong type
  return (FALSE);
}


//
// VarSys::FindVarScope
//
// Returns a pointer to the var scope 'path', or NULL if not found
//
VarSys::VarScope* VarSys::FindVarScope(const char *path)
{
  // Split path into path and item names
  String s1, s2;
  SplitDelimited(s1, s2, path, FALSE);

  // Find using full path crc and item name
  VarItem *item = FindVarItemHelper(Crc::CalcStr(path), *s2);

  if (item && (item->type == VI_SCOPE))
  {
    return (item->scope.ptr);
  }
  else
  {
    return (NULL);
  }
}


//
// VarSys::CopyVarItem
//
// Copy a var (src MUST exist)
//
VarSys::VarItem* VarSys::CopyVarItem(const char *dst, const char *src)
{
  ASSERT(sysInit)
  ASSERT(dst)
  ASSERT(src)

  // Find the source item
  VarItem *oldItem = FindVarItem(src);

  if (oldItem)
  {
    return (CopyVarItem(dst, oldItem));
  }
  else
  {
    ERR_FATAL(("Source of copy '%s' does not exist!"))
  }
}


//
// VarSys::CopyVarItem
//
// Copies the var item in src to a new var item called dst
//
VarSys::VarItem* VarSys::CopyVarItem(const char *dst, VarItem *src)
{
  ASSERT(src);
  ASSERT(dst);

  VarItem *newItem;

  switch(src->type)
  {
    case VI_STRING:
      newItem = CreateString(dst, src->Str());
      break;

    case VI_INTEGER:
      newItem = CreateInteger(dst, src->Integer());
      break;

    case VI_FPOINT:
      newItem = CreateFloat(dst, src->Float());
      break;

    case VI_BINARY:
      newItem = CreateBinary(dst, src->BinarySize(), src->Binary());
      break;

    case VI_SCOPE:
    {
      newItem = CreateVarItem(dst);
      newItem->InitScope();

      // For each item in the scope, copy them over to the new scope
      BinTree<VarSys::VarItem>::Iterator i(&src->scope.ptr->items);
      for (!i; *i; i++)
      {
        // Compose the new name
        StrBuf<VARSYS_MAXVARPATH> newName;
        newName = dst;
        Utils::Strcat(newName.str, VARSYS_SCOPEDELIMSTR);
        Utils::Strcat(newName.str, (*i)->itemId.str);

        // Copy it over
        CopyVarItem(newName.str, (*i));
      }
      break;
    }

    default:
      ERR_FATAL(("Unable to copy var item type %d", src->type))
  }

  return (newItem);
}


//
// VarSys::RegisterHandler
//
// Registers a command handler for scope 'path', creating if not found
//
void VarSys::RegisterHandler(const char *path, VarSysCallBack *func, U32 flagsIn)
{
  ASSERT(func);

  VarItem *item = FindVarItem(path);

  if (item)
  {
    // Make sure it's a scope
    if (item->type != VI_SCOPE)
    {
      ERR_FATAL(("Attempt to register handler with an item that wasn't a scope"))
    }
  }
  else
  {
    // Create new item
    item = CreateVarItem(path);

    // Initialize as a scope
    item->InitScope();
  }

  ASSERT(item);
  ASSERT(item->type == VI_SCOPE);

  // Set the handler
  item->scope.ptr->callBack = func;

  // Set the flags for for this scope ... yes they are duplicated
  item->flags = item->scope.ptr->flags = flagsIn;
}


//
// VarSys::RegisterRootHandler
//
// Registers a command handler for the root scope
//
void VarSys::RegisterRootHandler(VarSysCallBack *func)
{
  ASSERT(gScope);

  // Set the handler
  gScope->callBack = func;
}


//
// VarSys::CreateVar
//
// Create an empty Var
//
VarSys::VarItem* VarSys::CreateVar(const char *path, const char *ident, void *context)
{
  ASSERT(sysInit);

  // Build the expanded var name
  path = BuildVarName(path, context);

  // Build full path out of path and ident
  StrCrc<VARSYS_MAXVARPATH> fullPath = path;
  Utils::Strcat(fullPath.str, VARSYS_SCOPEDELIMSTR);
  Utils::Strcat(fullPath.str, ident);
  fullPath.Update();

  // Make sure that a var doesn't already occupy that spot
  if (FindVarItemHelper(fullPath.crc, ident))
  {
    ERR_FATAL(("Var %s already exists", fullPath.str))
  }

  // Find the scope refered to by the parent
  VarScope *parent = FindVarScope(path);

  if (!parent)
  {
    ERR_FATAL(("Parent scope %s not found", path))
  }

  // Create the var and return it
  return (parent->CreateNewItem(ident, fullPath.crc));
}


//
// VarSys::CreateScope
//
// Create a SCOPE using the given path (ignored if already exists)
//
VarSys::VarItem* VarSys::CreateScope(const char *path, U32 flagsIn)
{
  VarItem *var = FindVarItem(path);

  if (var)
  {
    // Make sure it's a scope
    if (var->type != VI_SCOPE)
    {
      ERR_FATAL(("Attempt to create a scope, but a different item type already exists [%s]", path))
    }
  }
  else
  {
    // Create new item
    var = CreateVarItem(path);

    // Initialize as a scope
    var->InitScope();

    // Set the flags
    var->flags = flagsIn | var->pScope->flags;
  }

  ASSERT(var)

  return (var);
}


//
// VarSys::CreateString
//
// Create a STRING variable
//
VarSys::VarItem* VarSys::CreateString(const char *path, const char *value, U32 flagsIn, VarString *varPtr, void *context)
{
  ASSERT(sysInit);

  // Create new item
  VarItem *var = CreateVarItem(path, context);

  // Initialize as a string
  var->InitString(value);

  // Set property flags inheriting flags from parent scope
  var->flags = flagsIn | var->pScope->flags;

  // Point var at item
  if (varPtr)
  {
    varPtr->PointAt(path);
  }

  return (var);
}


//
// VarSys::CreateInteger
//
// Create an S32 variable
//
VarSys::VarItem* VarSys::CreateInteger(const char *path, S32 value, U32 flagsIn, VarInteger *varPtr, void *context)
{
  ASSERT(sysInit);

  // Create new item
  VarItem *var = CreateVarItem(path, context);

  // Initialize as an integer
  var->InitInteger(value);

  // Set property flags inheriting flags from parent scope
  var->flags = flagsIn | var->pScope->flags;

  // Point var at item
  if (varPtr)
  {
    varPtr->PointAt(path);
  }

  return (var);
}


//
// VarSys::CreateFloat
//
// Create a FPOINT variable
//
VarSys::VarItem* VarSys::CreateFloat(const char *path, F32 value, U32 flagsIn, VarFloat *varPtr, void *context)
{
  ASSERT(sysInit);

  // Create new item
  VarItem *var = CreateVarItem(path, context);

  // Initialize as a fpoint
  var->InitFPoint(value);

  // Set property flags inheriting flags from parent scope
  var->flags = flagsIn | var->pScope->flags;

  // Point var at item
  if (varPtr)
  {
    varPtr->PointAt(path);
  }

  return (var);
}


//
// VarSys::CreateBinary
//
// Create a BINARY variable
//
VarSys::VarItem* VarSys::CreateBinary(const char *path, U32 size, const U8 *value, U32 flagsIn, VarBinary *varPtr, void *context)
{
  ASSERT(sysInit);

  // Create new item
  VarItem *var = CreateVarItem(path, context);

  // Initialize as a binary
  var->InitBinary(size, value);

  // Set property flags inheriting flags from parent scope
  var->flags = flagsIn | var->pScope->flags;

  // Point var at item
  if (varPtr)
  {
    varPtr->PointAt(path);
  }

  return (var);
}


//
// VarSys::CreateCmd
//
// Create a CMD item
//
void VarSys::CreateCmd(const char *path, U32 flagsIn, void *context)
{ 
  ASSERT(sysInit);

  // Create new item
  VarItem *var = CreateVarItem(path, context);

  // Initialize as a cmd
  var->InitCmd();

  // Inherit flags from parent scope, always setting notify for cmd's
  var->flags = flagsIn | var->pScope->flags | NOTIFY;
}


//
// VarSys::DeleteItem
//
// Delete an item
//
void VarSys::DeleteItem(VarItem *item)
{
  ASSERT(sysInit);

  // Tell it's parent to delete it
  item->pScope->DeleteItem(item);
}


//
// VarSys::DeleteItem
//
// Delete an item, returning FALSE if item was not found
//
Bool VarSys::DeleteItem(const char *path, void *context)
{
  ASSERT(sysInit);

  VarItem *item = FindVarItem(path, context);

  // Did we find it
  if (item)
  {
    DeleteItem(item);
    return (TRUE);
  }

  return (FALSE);
}


//
// VarSys::SetIntegerRange
//
// Set the range of an integer
//
Bool VarSys::SetIntegerRange(const char *path, S32 low, S32 high)
{
  // Locate the item
  VarItem *item = FindVarItem(path);

  // Did we find it
  if (item)
  {
    item->SetIntegerRange(low, high);
    return TRUE;
  }

  return FALSE;
}


//
// VarSys::SetFloatRange
//
// Set the range of a float
//
Bool VarSys::SetFloatRange(const char *path, F32 low, F32 high)
{
  // Locate the item
  VarItem *item = FindVarItem(path);

  // Did we find it
  if (item)
  {
    item->SetFloatRange(low, high);
    return TRUE;
  }
  return FALSE;
}


//
// VarSys::TriggerCmd
//
// Trigger the command item pointed to by 'path'.  Returns FALSE
// if the command was not found.
//
Bool VarSys::TriggerCmd(const char *path)
{
  // Locate the item
  VarItem *item = FindVarItem(path);

  if (item)
  {
    // Make sure it's a command
    if (item->type == VI_CMD)
    {
      // Trigger the callback
      item->TriggerCmd();
      return (TRUE);
    }
    else
    {
      LOG_ERR(("Attempt to trigger an item that wasn't a command! (%s - %d)", item->itemId.str, item->type))
    }
  }

  // Didn't find it
  return (FALSE); 
}


// 
// VarSys::GetTypeString
//
// Returns a string representation of 'type'
//
char* VarSys::GetTypeString(VarItemType type)
{
  ASSERT(type < VI_MAXIMUM);

  return(itemTypeStrings[type]);
}
