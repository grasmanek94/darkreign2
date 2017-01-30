///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Text File Parse Tree Management
//
// 24-NOV-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "tbuf.h"
#include "vnode.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class FScope
//

// 
// FScope::IsLegalIdent
//
// true if 'ident' is a legal identifier name
//
Bool FScope::IsLegalIdent(const char *ident)
{
  // check legal first character
  if (!isalpha(*ident))
  {
    return (FALSE);
  }

  // rockin
  return (TRUE);
}


//
// FScope::FScope
//
// constructor
//
FScope::FScope(FScope *parent, const char *name, U32 crc) : 
  parentScope(parent),
  argList(&VNode::node),
  bodyList(&VNode::node),
  argItr(&argList),
  bodyItr(&bodyList),
  scopeItr(&bodyList)
#ifdef DEVELOPMENT
  ,dirty(FALSE)
#endif
{
  // Set function name
  nameStr = name ? Utils::Strdup(name) : NULL;

  // Set crc of name
  nameCrc = nameStr ? Crc::CalcStr(nameStr) : crc;
 
  // Add to list of previous scope
  if (parentScope)
  {
    // create a scope node
    VNode *node = new VNode;

    node->SetupScope(this);
    parentScope->bodyList.Append(node);
  }
}


//
// FScope::~FScope
//
// destructor
//
FScope::~FScope()
{
  // Delete nodes
  argList.DisposeAll();
  bodyList.DisposeAll();

  // Delete the allocated name
  if (nameStr)
  {
    delete [] nameStr;
  }
}


// 
// FScope::ScopeError
//
// for fatal errors in this scope, displays with context information
//
void CDECL FScope::ScopeError(const char *fmt, ...)
{
  LOG_WARN(("Parse Callstack:"))

  // display the callstack for the scope
  StackRecurse();

  // process the variable args
  va_list args;
  char fmtBuf[1024];
  va_start(args, fmt);
  vsprintf(fmtBuf, fmt, args);
  va_end(args);

  // trigger the error
  ERR_CONFIG(("(%s) Error! %s", NameStr(), fmtBuf));
}



//
// FScope::NameStr
//
// returns the string name of this function
//
const char * FScope::NameStr()
{
  // This is returned if scope loaded from a binary file
  static const char *noName = "[Unavailable]";

  return (nameStr ? nameStr : noName);
}


//
// FScope::InitIterators
//
// Initializes all list iterators for this scope
//
void FScope::InitIterators()
{
  argItr.GoToHead();
  bodyItr.GoToHead();
  scopeItr.GoToHead();
}


//
// FScope::Dup
//
// Duplicate the entire scope and all data
//
FScope *FScope::Dup(FScope *parent)
{
  FSCOPE_DIRTY(this)

  // Create the new scope
  FScope *newScope = new FScope(parent, nameStr, nameCrc);

  // Duplicate argument list
  for (NList<VNode>::Iterator args(&argList); *args; args++)
  {
    VNode *node = (*args)->NewAtomicNode();
    ASSERT(node);

    newScope->argList.Append(node);
  }

  // Duplicate body list
  for (NList<VNode>::Iterator body(&bodyList); *body; body++)
  {
    VNode *node = *body;

    switch (node->aType)
    {
      case VNode::AT_SCOPE:
      {
        node->GetScope()->Dup(newScope);
        break;
      }

      default:
      {
        VNode *newNode = (*args)->NewAtomicNode();
        ASSERT(newNode);
        newScope->bodyList.Append(newNode);
        break;
      }       
    }
  }

  return newScope;
}


//
// FScope::NextArgument
//
// returns the VNode for the next argument, returning NULL if no more and not required
//
VNode* FScope::NextArgument(Bool required)
{
  VNode *n = argItr++;

  FSCOPE_DIRTY(this)

  if (!n && required)
  {
    ScopeError("Argument expected (pos %d)", argItr.GetPos());
  }

  return (n);
}


//
// FScope::NextArgument
//
// returns the VNode for the next argument (error if atomic type
// of argument does not match 'aType')
//
VNode* FScope::NextArgument(VNode::VNodeAtomicType aType, Bool required)
{
  // save the position of this 'expected' argument
  U32 argPos = argItr.GetPos() + 1;
 
  // get the next argument
  VNode *vNode = NextArgument();

  // did we find one
  if (vNode)
  {
    // is it the right type
    if (vNode->aType != aType)
    {
      ScopeError
      (
        "'%s' argument expected (pos %d) but found type '%s'",
        VNode::GetAtomicString(aType), 
        argPos,
        VNode::GetAtomicString(vNode->aType)
      );
    }
  }
  else
  {
    // was it required
    if (required)
    {
      ScopeError("'%s' argument expected (pos %d)", VNode::GetAtomicString(aType), argPos);
    }
  }
  
  // success
  return (vNode);
}


//
// FScope::NextArgString
//
// gets the next argument as a string, or trigger error
//
const char* FScope::NextArgString()
{
  VNode *vNode = NextArgument(VNode::AT_STRING);
  ASSERT(vNode); 
  return (vNode->GetString());
}


//
// FScope::NextArgInteger
//
// gets the next argument as an integer, or trigger error
//
S32 FScope::NextArgInteger()
{
  VNode *vNode = NextArgument(VNode::AT_INTEGER);
  ASSERT(vNode); 
  return (vNode->GetInteger());
}


//
// FScope::NextArgFPoint
//
// gets the next argument as a floating point, or trigger error
//
F32 FScope::NextArgFPoint()
{
  // save the position of this 'expected' argument
  U32 argPos = argItr.GetPos() + 1;

  // get the next argument
  VNode *vNode = NextArgument();

  if (vNode)
  {
    switch (vNode->aType)
    {
      case VNode::AT_FPOINT:
        return (vNode->GetFPoint());
        break;

      case VNode::AT_INTEGER:
        // integers can be turned into floats
        return ((F32) vNode->GetInteger());
        break;

      default:
        ScopeError("Floating Point argument expected (pos %d) but found type '%s'", argPos, VNode::GetAtomicString(vNode->aType));
    }
  }
  else
  {
    ScopeError("Floating Point argument expected (pos %d) but none found", argPos);
  }
}


//
// FScope::PeekArgument
//
// peek at the VNode for the next argument
//
VNode* FScope::PeekArgument()
{
  return (*argItr);
}


//
// FScope::IsNextArgString
//
Bool FScope::IsNextArgString()
{
  VNode *vNode = PeekArgument();
  return ((vNode && vNode->aType == VNode::AT_STRING) ? TRUE : FALSE);
}


//
// FScope::IsNextArgInteger
//
Bool FScope::IsNextArgInteger()
{
  VNode *vNode = PeekArgument();
  return ((vNode && vNode->aType == VNode::AT_INTEGER) ? TRUE : FALSE);
}


//
// FScope::IsNextArgFPoint
//
Bool FScope::IsNextArgFPoint()
{
  VNode *vNode = PeekArgument();
  return ((vNode && (vNode->aType == VNode::AT_FPOINT || vNode->aType == VNode::AT_INTEGER)) ? TRUE : FALSE);
}


//
// FScope::NextBodyVNode
//
// returns the next body VNode
//
VNode* FScope::NextBodyVNode()
{
  return (bodyItr++);
}


//
// FScope::GetVarInteger
//
// get REQUIRED integer variable (error if not found)
//
S32 FScope::GetVarInteger(const char *ident)
{
  ASSERT(ident);
  
  S32 iVal = 0;
  
  // can we see this variable
  if (!GetVarIntegerRef(ident, iVal))
  {
    ScopeError
    (
      "expected to find variable '%s' of type '%s'", 
      ident, VNode::GetAtomicString(VNode::AT_INTEGER)
    );
  }

  return (iVal);
}


//
// FScope::GetVarFPoint
//
// get REQUIRED FPoint variable (error if not found)
//
F32 FScope::GetVarFPoint(const char *ident)
{
  ASSERT(ident);
  
  F32 fVal = 0.0;
  
  // can we see this variable
  if (!GetVarFPointRef(ident, fVal))
  {
    ScopeError
    (
      "expected to find variable '%s' of type '%s'", 
      ident, VNode::GetAtomicString(VNode::AT_FPOINT)
    );
  }

  return (fVal);
}


//
// FScope::GetVarString
//
// get REQUIRED String variable (error if not found)
//
const char* FScope::GetVarString(const char *ident)
{
  ASSERT(ident);
  
  const char *sVal = NULL;
  
  // can we see this variable
  if (!GetVarStringRef(ident, sVal))
  {
    ScopeError
    (
      "expected to find variable '%s' of type '%s'", 
      ident, VNode::GetAtomicString(VNode::AT_STRING)
    );
  }

  return (sVal);
}


//
// FScope::GetVarInteger
//
// get OPTIONAL integer variable (return 'dVal' if not found)
//
S32 FScope::GetVarInteger(const char *ident, S32 dVal)
{
  ASSERT(ident);

  S32 iVal = 0;
  
  // can we see this variable
  if (GetVarIntegerRef(ident, iVal))
  {
    return (iVal);
  }

  return (dVal);
}


//
// FScope::GetVarFPoint
//
// get OPTIONAL FPoint variable (return 'dVal' if not found)
//
F32 FScope::GetVarFPoint(const char *ident, F32 dVal)
{
  ASSERT(ident);

  F32 fVal = 0.0;
  
  // can we see this variable
  if (GetVarFPointRef(ident, fVal))
  {
    return (fVal);
  }

  return (dVal);
}


//
// FScope::GetVarString
//
// get OPTIONAL String variable (return 'dVal' if not found)
//
const char* FScope::GetVarString(const char *ident, const char *dVal)
{
  ASSERT(ident);

  const char *sVal = NULL;
  
  // can we see this variable
  if (GetVarStringRef(ident, sVal))
  {
    return (sVal);
  }

  return (dVal);
}


//
// FScope::GetVarIntegerRef
//
// get OPTIONAL integer variable (returns FALSE and 'dest' unchanged if not found)
//
Bool FScope::GetVarIntegerRef(const char *ident, S32 &dest)
{
  ASSERT(ident);

  VNode *vNode;
  
  // try and find the variable visible from this scope
  vNode = FindVariableVisible(Crc::CalcStr(ident));

  if (vNode)
  {
    // check the atomic type
    if (vNode->aType != VNode::AT_INTEGER)
    {
      ScopeError
      (
        "expecting '%s' to be of type '%s' but found '%s'", ident,
        VNode::GetAtomicString(VNode::AT_INTEGER),
        VNode::GetAtomicString(vNode->aType)
      );
    }

    // store the value
    dest = vNode->GetInteger();
  }
  
  return (vNode ? TRUE : FALSE);
}


//
// FScope::GetVarFPointRef
//
// get OPTIONAL FPoint variable (returns FALSE and 'dest' unchanged if not found)
//
Bool FScope::GetVarFPointRef(const char *ident, F32 &dest)
{
  ASSERT(ident);

  VNode *vNode;
  
  // try and find the variable visible from this scope
  vNode = FindVariableVisible(Crc::CalcStr(ident));

  if (vNode)
  {
    // check the atomic type
    if (vNode->aType != VNode::AT_FPOINT)
    {
      ScopeError
      (
        "expecting '%s' to be of type '%s' but found '%s'", ident,
        VNode::GetAtomicString(VNode::AT_FPOINT),
        VNode::GetAtomicString(vNode->aType)
      );
    }

    // store the value
    dest = vNode->GetFPoint();
  }
  
  return (vNode ? TRUE : FALSE);
}


//
// FScope::GetVarStringRef
//
// get OPTIONAL String variable (returns FALSE and 'dest' unchanged if not found)
//
Bool FScope::GetVarStringRef(const char *ident, const char * &dest)
{
  ASSERT(ident);

  VNode *vNode;
  
  // try and find the variable visible from this scope
  vNode = FindVariableVisible(Crc::CalcStr(ident));

  if (vNode)
  {
    // check the atomic type
    if (vNode->aType != VNode::AT_STRING)
    {
      ScopeError
      (
        "expecting '%s' to be of type '%s' but found '%s'", ident,
        VNode::GetAtomicString(VNode::AT_STRING),
        VNode::GetAtomicString(vNode->aType)
      );
    }

    // store the value
    dest = vNode->GetString();
  }
  
  return (vNode ? TRUE : FALSE);
}


//
// FScope::NextFunction
//
// returns the next sub-scope in the list, or null if none. resets
// all iterators in the resulting scope.
//
FScope* FScope::NextFunction()
{
  VNode *node;

  // get the next node
  while ((node = scopeItr++) != 0)
  {
    // is it a function scope
    if (node->aType == VNode::AT_SCOPE)
    {
      FScope *fScope = node->GetScope();

      return (fScope);
    }
  }
  
  // nout more scopes
  return (NULL);
}


//
// FScope::PeekFunction
//
// returns the next sub-scope in the list, or null if none. resets
// all iterators in the resulting scope.
//
FScope* FScope::PeekFunction()
{
  VNode *node;

  // Copy the iterator
  NList<VNode>::Iterator itr(scopeItr);

  // get the next node
  while ((node = itr++) != 0)
  {
    // is it a function scope
    if (node->aType == VNode::AT_SCOPE)
    {
      FScope *fScope = node->GetScope();
      return (fScope);
    }
  }
  
  // nout more scopes
  return (NULL);
}


//
// returns the parent function 
//
FScope* FScope::ParentFunction()
{
  return (parentScope);
}


//
// FScope::GetFunction
//
// returns first occurence of a specific function, or null if not
// found.  resets 'this' sub-scope iterator, and resets all iterators
// in the resulting scope.
//
FScope* FScope::GetFunction(const char *name, Bool required)
{
  FSCOPE_DIRTY(this)

  // calc the crc we're looking for
  U32 crc = Crc::CalcStr(name);

  // step through nodes
  for (NList<VNode>::Iterator i(&bodyList); *i; i++)
  {
    // is it a function scope
    if ((*i)->aType == VNode::AT_SCOPE)
    {
      // get the scope
      FScope *fScope = (*i)->GetScope();
      
      // is this the one we're after
      if (fScope->NameCrc() == crc)
      {
        FSCOPE_DIRTY(fScope)

        // return it
        return (fScope);
      }
    }
  }

  // do we need to trigger an error
  if (required)
  {
    ScopeError("expected function '%s'", name);
  }

  // didn't find it
  return (NULL);
}


//
// FScope::FindVariableInScope
//
// find a variable type VNode (within this scope only), with
// identifier crc matching 'crcVal'.  returns a pointer to the
// node or null if not found.
//
VNode* FScope::FindVariableInScope(U32 crcVal)
{
  // search list for crcVal
  for (NList<VNode>::Iterator i(&bodyList); *i; i++)
  {
    if ((*i)->nType == VNode::NT_VARIABLE)
    {
      // is this the one we're looking for
      if ((*i)->GetVariableCrc() == crcVal)
      {
        return (*i);
      }
    }
  }

  // not visible from current scope
  return (NULL);
}


//
// FScope::FindVisibleVariable
//
// find a variable type VNode (visible from this scope), with
// identifier crc matching 'crcVal'.  returns a pointer to the
// node or null if not found.
//
VNode* FScope::FindVariableVisible(U32 crcVal)
{
  VNode *node;

  // search 'fScope' and all above it, in turn
  for (FScope *fScope = this; fScope; fScope = fScope->parentScope)
  {
    // is the variable in 'pScope'
    if ((node = fScope->FindVariableInScope(crcVal)) != 0)
    {
      return (node);
    }
  }

  // not visible from 'fScope'
  return (NULL);
}


// 
// FScope::GetArgCount
//
// Returns the number of arguments
//
U32 FScope::GetArgCount()
{
  return (argList.GetCount());
}


// 
// FScope::GetBodyCount
//
// Returns the number of items in the body
//
U32 FScope::GetBodyCount()
{
  return (bodyList.GetCount());
}


// 
// FScope::ExpectArgCount
//
// generates error if not 'count' arguments
//
void FScope::ExpectArgCount(U32 count)
{
  // check argument list node count
  if (argList.GetCount() != count)
  {
    ERR_CONFIG
    (( 
      "Expecting %d argument%s in function '%s' but found %d", 
      count,
      (count > 1) ? "s" : "",
      NameStr(),
      argList.GetCount()
    ))
  }
}


//
// FScope::AddFunction
//
// add a function to this scope
//
FScope* FScope::AddFunction(const char *name, U32 crc)
{
  // allocate a new function scope
  FScope *newScope = new FScope(this, name, crc);

  FSCOPE_DIRTY(newScope)

  // and return it
  return (newScope);
}


//
// FScope::AddDup
//
// add a dup of 'src' to this scope
//
FScope* FScope::AddDup(FScope *src)
{
  return (src->Dup(this));
}


//
// FScope::AddVar
//
// add or modify a variable
//
VNode* FScope::AddVar(const char *vName, VNode *valNode)
{
  VNode *varNode;

  // make sure legal variable name
  if (!IsLegalIdent(vName))
  {
    ERR_FATAL(("Attempt to add illegal identifer name '%s' to '%s'", vName, NameStr()));
  }

  // are we just changing the value of an existing variable
  varNode = FindVariableInScope(Crc::CalcStr(vName));

  if (varNode)
  {
    // make sure atomic type hasn't changed
    if (valNode->aType != varNode->aType)
    {
      ERR_FATAL(("Attempt to change atomic type of '%s' in '%s'", vName, NameStr()));
    }

    // clear the variable VNode
    varNode->Clear();

    // and setup again
    varNode->SetupVariable(vName, valNode);
  }
  else
  {
    // create a variable node and attach value node
    varNode = new VNode;
    varNode->SetupVariable(vName, valNode);

    // add to global vars within scope
    bodyList.Append(varNode);
  }

  return (varNode);
}


//
// FScope::AddVarInteger
//
// add an integer variable to this scope
//
VNode* FScope::AddVarInteger(const char *vName, S32 data)
{
  VNode *valNode = new VNode;
  valNode->SetupInteger(data);
  return (AddVar(vName, valNode));
}


//
// FScope::AddVarFPoint
//
// add a floating point variable to this scope
//
VNode* FScope::AddVarFPoint(const char *vName, F32 data)
{
  VNode *valNode = new VNode;
  valNode->SetupFPoint(data);
  return (AddVar(vName, valNode));
}


//
// FScope::AddVarString
//
// add a string variable to this scope
//
VNode* FScope::AddVarString(const char *vName, const char *data)
{
  VNode *valNode = new VNode;
  valNode->SetupString(data);
  return (AddVar(vName, valNode));
}


//
// FScope::AddArgInteger
//
// add an integer argument to this scope
//
VNode* FScope::AddArgInteger(S32 data)
{
  VNode *valNode = new VNode;
  valNode->SetupInteger(data);
  argList.Append(valNode);
  return (NULL);
}


//
// FScope::AddArgFPoint
//
// add a floating point argument to this scope
//
VNode* FScope::AddArgFPoint(F32 data)
{
  VNode *valNode = new VNode;
  valNode->SetupFPoint(data);
  argList.Append(valNode);
  return (NULL);
}


//
// FScope::AddArgString
//
// add a string argument to this scope
//
VNode* FScope::AddArgString(const char *data)
{
  VNode *valNode = new VNode;
  valNode->SetupString(data);
  argList.Append(valNode);
  return (NULL);
}


//
// FScope::DumpScope
//
// Dump the contents of this scope
//
void FScope::DumpScope()
{
  // Compose Scope Name and arguments into a buffer
  static char buffer[256];
  Utils::Sprintf(buffer, 256, "%s(", NameStr());

  // Reset scope iterators
  InitIterators();

  // Write arguments
  Bool firstArg = TRUE;
  VNode *aNode;

  while ((aNode = NextArgument()) != 0)
  {
    if (!firstArg)
    {
      Utils::Strcat(buffer, ", ");
    }

    Utils::Strcat(buffer, aNode->StringForm());
    firstArg = FALSE;
  }
  Utils::Strcat(buffer, ")");

  // Write the log
  LOG_WARN(("%s", buffer))
}


//
// FScope::StackRecurse
//
// Print out information about this scope and if a parent scope exists,
// recurse to it
//
void FScope::StackRecurse()
{
  // Log the name of this scope and any arguments it has
  DumpScope();

  // Does this scope have a parent scope ?
  if (parentScope)
  {
    // Recurse to the parent scope
    parentScope->StackRecurse();
  }
}


#ifdef DEVELOPMENT

//
// FScope::Dirty
//
// Sets the scope to dirty and recurses up to the parent
//
void FScope::Dirty()
{
  dirty = TRUE;

  // Dirty the parent if its not already dirty
  if (parentScope && !parentScope->dirty)
  {
    FSCOPE_DIRTY(parentScope)
  }
}


//
// FScope::DirtyAll
//
// Dirty's the scope and all subscopes
//
void FScope::DirtyAll()
{
  dirty = TRUE;

  for (NList<VNode>::Iterator body(&bodyList); *body; body++)
  {
    VNode *node = *body;

    switch (node->aType)
    {
      case VNode::AT_SCOPE:
      {
        node->GetScope(FALSE)->DirtyAll();
        break;
      }
    }
  }
}


//
// FScope::CleanAll
//
// Dirty's the scope and all subscopes
//
void FScope::CleanAll()
{
  dirty = FALSE;

  for (NList<VNode>::Iterator body(&bodyList); *body; body++)
  {
    VNode *node = *body;

    switch (node->aType)
    {
      case VNode::AT_SCOPE:
      {
        node->GetScope(FALSE)->CleanAll();
        break;
      }
    }
  }
}




//
// FScope::CheckDirty
//
// Checks to make sure that the entire scope is dirty
//
void FScope::CheckDirty()
{
  if (!dirty)
  {
    LOG_ERR(("Unreferenced FScope:"))
    StackRecurse();
    return;
  }

  // Check all of the subscopes
  for (NList<VNode>::Iterator body(&bodyList); *body; body++)
  {
    VNode *node = *body;

    switch (node->aType)
    {
      case VNode::AT_SCOPE:
      {
        node->GetScope(FALSE)->CheckDirty();
        break;
      }
    }
  }
}


#endif