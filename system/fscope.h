///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Text File Parse Tree Management
//
// 24-NOV-1997
//

#ifndef __FSCOPE_H
#define __FSCOPE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vnode.h"


///////////////////////////////////////////////////////////////////////////////
//
// Defines
//
#ifdef DEVELOPMENT
  #define FSCOPE_CHECK(x) x->CheckDirty();
  #define FSCOPE_DIRTY(x) x->Dirty();
  #define FSCOPE_DIRTYALL(x) x->DirtyAll();
  #define FSCOPE_CLEANALL(x) x->CleanAll();
#else
  #define FSCOPE_CHECK(x)
  #define FSCOPE_DIRTY(x)
  #define FSCOPE_DIRTYALL(x)
  #define FSCOPE_CLEANALL(x)
#endif



///////////////////////////////////////////////////////////////////////////////
//
// Class FScope
//
// Each function scope has a function name, an argument list, a local
// variable list, and a local function list.
//

class FScope
{
protected:

  // Name of this function, or NULL if binary
  char *nameStr;

  // Crc of the name
  U32 nameCrc;

  // Parent scope
  FScope *parentScope;

  // Function arguments
  NList<VNode> argList;
  NList<VNode>::Iterator argItr;

  // Function body
  NList<VNode> bodyList;
  NList<VNode>::Iterator bodyItr;
  NList<VNode>::Iterator scopeItr;
  
protected:

  // Find a variable type VNode within this scope only
  VNode* FindVariableInScope(U32 crcVal);

  // Find a variable type VNode visible from this scope
  VNode* FindVariableVisible(U32 crcVal);

  // Works close with
  friend class PTree;

public:

  // Constructor and destructor
  FScope(FScope *parent, const char *name, U32 crc = 0);
  ~FScope();

  // true if 'ident' is a legal identifier name
  static Bool IsLegalIdent(const char *ident);

  // for fatal errors in this scope, displays with context information
  void NORETURN CDECL ScopeError(const char *fmt, ...);  

  // returns the string name of this function
  const char * NameStr();

  // resets all iterators in the scope
  void InitIterators();

  // duplicate the current scope
  FScope *Dup(FScope *parent = NULL);

  // Returns the number of arguments
  U32 GetArgCount();

  // Returns the number of items in the body
  U32 GetBodyCount();

  // generates error if not 'count' function arguments
  void ExpectArgCount(U32 count);

  // returns the VNode for the next argument, returning NULL if no more and not required
  VNode* NextArgument(Bool required = FALSE);

  // returns the VNode for the next argument, error if type is not 'aType'
  VNode* NextArgument(VNode::VNodeAtomicType aType, Bool required = TRUE);

  // returns the atomic types for the next argument
  const char* NextArgString();
  S32 NextArgInteger();
  F32 NextArgFPoint();

  // peek at the VNode for the next argument
  VNode* PeekArgument();

  // tests to see if there is a next argument
  Bool IsNextArgString();
  Bool IsNextArgInteger();
  Bool IsNextArgFPoint();

  // returns the next body VNode
  VNode* NextBodyVNode();

  // get REQUIRED variable (error if not found)
  S32 GetVarInteger(const char *ident);
  F32 GetVarFPoint(const char *ident);
  const char* GetVarString(const char *ident);
  
  // get OPTIONAL variable (return 'dVal' if not found)
  S32 GetVarInteger(const char *ident, S32 dVal);
  F32 GetVarFPoint(const char *ident, F32 dVal);
  const char* GetVarString(const char *ident, const char *dVal);

  // get OPTIONAL variable (returns FALSE and 'dest' unchanged if not found)
  Bool GetVarIntegerRef(const char *ident, S32 &dest);
  Bool GetVarFPointRef(const char *ident, F32 &dest);
  Bool GetVarStringRef(const char *ident, const char * &dest);

  // returns the next function in the list
  FScope* NextFunction();

  // returns the next function in the list but doesn't move forward
  FScope* PeekFunction();

  // returns the parent function 
  FScope* ParentFunction();

  // returns first occurence of a specific function
  FScope* GetFunction(const char *name, Bool required = TRUE);

  // add a function to this scope
  FScope* AddFunction(const char *name, U32 crc = 0);

  // add a dup of 'src' to this scope
  FScope* AddDup(FScope *src);

  // add a variable to this scope
  VNode* AddVar(const char *vName, VNode *valNode);
  VNode* AddVarInteger(const char *vName, S32 data);
  VNode* AddVarFPoint(const char *vName, F32 data);
  VNode* AddVarString(const char *vName, const char *data);

  // add a parameter to this scope
  VNode* AddArgInteger(S32 data);
  VNode* AddArgFPoint(F32 data);
  VNode* AddArgString(const char *data);

  // Print out a scope and its arguments
  void DumpScope();

  // Recursive function for dumping fscope callstacks
  void StackRecurse();

#ifdef DEVELOPMENT

  // Is the scope dirty
  Bool dirty;

  // Dirty this scope and all parent scopes
  void Dirty();

  // Dirty this scope and all child scopes
  void DirtyAll();

  // Clear this scope and all child scopes
  void CleanAll();

  // Check to see if this scope is entirely dirtied
  void CheckDirty();

#endif

  // Returns the crc of the string name 
  U32 NameCrc()
  {
    return (nameCrc);
  }

  // Does function have a body (ie. vars or sub-functions)
  Bool HasBody()
  {
    return (bodyList.GetCount() > 0);
  }
};


#endif