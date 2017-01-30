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
#include "vnode.h"
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class VNode
//


//
// VNode::aTypeStrings
//
// string representations of atomic types
//
char* VNode::aTypeStrings[AT_MAX] = 
{
  "none", "integer", "floating point", "string", "scope"
};


// 
// VNode::VNode
//
// constructor
//
VNode::VNode()
{
  // set initial types
  nType = NT_NONE;
  aType = AT_NONE;
}


// 
// VNode::~VNode
//
// destructor - contains specific deletion code for each type
//
VNode::~VNode()
{
  // free type specific data
  Clear();
}


//
// VNode::Clear
//
// free type specific data and reset type to NT_NONE
// usually just called by destructor, but can be used
// for instance reuse.
//
void VNode::Clear()
{
  // free type specific data
  switch (nType)
  {
    case NT_NONE:
    case NT_INTEGER:
    case NT_FPOINT:
    case NT_POINTER:
      break;

    case NT_STRING:
      delete [] string.s;
      break;

    case NT_VARIABLE:
      delete [] variable.ident;
      delete variable.vNode;
      break;

    case NT_SCOPE:
      delete scope.fScope;
      break;

    default:
      ERR_FATAL(("Missing case"));
  }

  // reset types
  nType = NT_NONE;
  aType = AT_NONE;
}


// 
// VNode::NewAtomicNode
//
// creates a new atomic node of this type, returning
// a pointer to it or null if not able to create atomic types
// using this VNode type.
//
VNode* VNode::NewAtomicNode()
{
  // allocate a new node
  VNode *newNode = 0;
  
  // check the atomic type
  switch (aType)
  {
    case AT_INTEGER:
      newNode = new VNode;
      newNode->SetupInteger(GetInteger());              
      break;

    case AT_FPOINT:
      newNode = new VNode;
      newNode->SetupFPoint(GetFPoint());              
      break;

    case AT_STRING:
      newNode = new VNode;
      newNode->SetupString(GetString());              
      break;

    case AT_SCOPE:
      break;

    default:
      ERR_FATAL(("Missing case"));

  }

  return (newNode);
}


//
// VNode::SetupInteger
//
// set up NT_INTEGER type
//
void VNode::SetupInteger(S32 intVal)
{
  Clear();
  
  nType = NT_INTEGER;
  aType = AT_INTEGER;
  integer.i = intVal;
}


//
// VNode::SetupFPoint
//
// set up NT_FPOINT type
//
void VNode::SetupFPoint(F32 dVal)
{
  Clear();
  
  nType = NT_FPOINT;
  aType = AT_FPOINT;
  fpoint.f = dVal;
}


//
// VNode::SetupString
//
// set up NT_STRING type
//
void VNode::SetupString(const char *strVal)
{
  ASSERT(strVal);

  Clear();
  
  nType = NT_STRING;
  aType = AT_STRING;
  string.s = Utils::Strdup(strVal);
  string.crc = Crc::CalcStr(string.s);
}


//
// VNode::SetupVariable
//
// set up NT_VARIABLE type
//
void VNode::SetupVariable(const char *identVal, VNode *node)
{
  ASSERT(*identVal != '\0');
  ASSERT(node);

  Clear();

  nType = NT_VARIABLE;
  aType = node->aType;
  variable.ident = Utils::Strdup(identVal);
  variable.crc = Crc::CalcStr(variable.ident);
  variable.vNode = node;
}


//
// VNode::SetupPointer
//
// set up NT_POINTER type
//
void VNode::SetupPointer(VNode *node)
{
  ASSERT(node);
  ASSERT(node->nType == NT_VARIABLE);

  Clear();
  
  nType = NT_POINTER;
  aType = node->aType;
  pointer.vNode = node;
}


//
// VNode::SetupScope
//
// set up NT_SCOPE type
//
void VNode::SetupScope(FScope *fScope)
{
  ASSERT(fScope);

  Clear();
  
  nType = NT_SCOPE;
  aType = AT_SCOPE;
  scope.fScope = fScope;
}


//
// VNode::GetInteger
//
// get data for atomic type AT_INTEGER
//
S32 VNode::GetInteger()
{ 
  ASSERT(aType == AT_INTEGER);

  S32 rVal = 0;

  switch (nType)
  {
    case NT_INTEGER:
      rVal = integer.i;
      break;

    case NT_VARIABLE:
      rVal = variable.vNode->GetInteger();
      break;

    case NT_POINTER:
      rVal = pointer.vNode->GetInteger();
      break;

    default:
      ERR_FATAL(("Unknown integer node type (%d)", nType));
  }

  return (rVal);
}


//
// VNode::GetFPoint
//
// get data for atomic type AT_FPOINT
//
F32 VNode::GetFPoint()
{
  ASSERT(aType == AT_FPOINT);

  F32 rVal = 0;

  switch (nType)
  {
    case NT_FPOINT:
      rVal = fpoint.f;
      break;

    case NT_VARIABLE:
      rVal = variable.vNode->GetFPoint();
      break;

    case NT_POINTER:
      rVal = pointer.vNode->GetFPoint();
      break;

    default:
      ERR_FATAL(("Unknown fpoint node type (%d)", nType));
  }

  return (rVal);
}



//
// VNode::GetString
//
// get data for atomic type AT_STRING
//
const char* VNode::GetString()
{
  ASSERT(aType == AT_STRING);

  const char *rVal = 0;

  switch (nType)
  {
    case NT_STRING:
      rVal = string.s;
      break;

    case NT_VARIABLE:
      rVal = variable.vNode->GetString();
      break;

    case NT_POINTER:
      rVal = pointer.vNode->GetString();
      break;

    default:
      ERR_FATAL(("Unknown string node type (%d)", nType));
  }

  return (rVal);
}


//
// VNode::GetScope
//
// get data for atomic type AT_SCOPE
//
FScope* VNode::GetScope(Bool reset)
{
  ASSERT(aType == AT_SCOPE);

  FScope *rVal = NULL;

  switch (nType)
  {
    case NT_SCOPE:

      ASSERT(scope.fScope);
           
      // get scope and initialize iterators
      rVal = scope.fScope;
      if (reset)
      {
        rVal->InitIterators();
      }
      break;

    default:
      ERR_FATAL(("Unknown scope node type (%d)", nType));
  }

  return (rVal);
}


//
// VNode::StringForm
//
// returns a pointer to a string representation of the atomic
// value of this node.
//
const char* VNode::StringForm()
{
  static char strBuf[256];

  switch (aType)
  {
    case AT_NONE:     
      Utils::Sprintf(strBuf, 256, "no atomic type");
      break;

    case AT_INTEGER:     
      Utils::Sprintf(strBuf, 256, "%d", GetInteger());
      break;

    case AT_FPOINT:
      Utils::Sprintf(strBuf, 256, "%f", GetFPoint());
      break;

    case AT_STRING:
      Utils::Sprintf(strBuf, 256, "\"%s\"", GetString());
      break;

    case AT_SCOPE:
      Utils::Sprintf(strBuf, 256, "%s", GetScope()->NameStr());
      break;

    default:
      Utils::Sprintf(strBuf, 256, "unknown atomic type (%d)", aType);
  }

  return (strBuf);
}


//
// VNode::GetStringCrc
//
// returns the crc of a string vNode
//
U32 VNode::GetStringCrc()
{
  ASSERT(nType == NT_STRING);
  return (string.crc);
};


//
// VNode::GetVariableStr
//
// returns the string of a variable identifier
//
const char* VNode::GetVariableStr()
{
  ASSERT(nType == NT_VARIABLE);
  return (variable.ident);
};


//
// VNode::GetVariableCrc
//
// returns the crc of a variable identifier
//
U32 VNode::GetVariableCrc()
{
  ASSERT(nType == NT_VARIABLE);
  return (variable.crc);
};


// 
// VNode::GetAtomicString
//
// returns a string representation of the atomic type 'aType'
//
const char* VNode::GetAtomicString(VNodeAtomicType aType)
{
  ASSERT(aType < AT_MAX);

  return(aTypeStrings[aType]);
}
