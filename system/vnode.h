///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Text File Parse Tree Management
//
// 24-NOV-1997
//

#ifndef __VNODE_H
#define __VNODE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tbuf.h"
#include "nlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward reference for recursive data
//

class FScope;


///////////////////////////////////////////////////////////////////////////////
//
// Class VNode
//
// Holds the info for one 'value' type, be it an integer, string etc, or 
// perhaps more complicated types later such as 'add' nodes for expressions.
//

class VNode
{
public:

  // type of this node
  enum VNodeType
  { 
    // not initialized
    NT_NONE,

    // signed 32 bit integer value
    NT_INTEGER,

    // floating point value
    NT_FPOINT,

    // string value (dynamic length)
    NT_STRING,

    // has string identifier and pointer to another VNode type
    NT_VARIABLE,

    // pointer to an existing NT_VARIABLE VNode
    NT_POINTER,

    // sub-scope
    NT_SCOPE,

    // max node type value
    NT_MAX

  } nType;


  // VNode atomic data types
  enum VNodeAtomicType
  {
    // some node types may not have an atomic type
    AT_NONE,

    // signed 32bit integer
    AT_INTEGER,

    // floating point value
    AT_FPOINT,

    // char * string
    AT_STRING,

    // function scope
    AT_SCOPE,

    // max atomic type value
    AT_MAX
  } aType;


private:

  // string representations of the ATOMIC types
  static char* aTypeStrings[AT_MAX];

  // type data
  union
  {
    // integer value
    struct
    {
      S32 i;
    } integer;

    // floating point value
    struct
    {
      F32 f;
    } fpoint;

    // string value
    struct
    {
      char *s;
      U32 crc;
    } string;

    // variable pointing to any other VNode type
    struct
    {
      char *ident;
      U32 crc;
      VNode *vNode;
    } variable;

    // pointer to an existing VNode (eg. legs = 2; legsPtr = &legs;)
    struct
    {
      VNode *vNode;
    } pointer;

    // pointer to a sub-scope
    struct
    {
      FScope *fScope;
    } scope;
  };

public:

  // NList node
  NList<VNode>::Node node;

public:
  
  // constructor and destructor
  VNode();
  ~VNode();

  // clear type specific data, and reset type
  void Clear();

  // create a new atomic node of this type
  VNode *NewAtomicNode();

  // setup VNode types
  void SetupInteger(S32 intVal);
  void SetupFPoint(F32 dVal);
  void SetupString(const char *strVal);
  void SetupVariable(const char *identVal, VNode *node);
  void SetupPointer(VNode *node);
  void SetupScope(FScope *fScope);

  // get VNode atomic data types
  S32 GetInteger();
  F32 GetFPoint();
  const char* GetString();
  FScope *GetScope(Bool reset = TRUE);

  // returns the pre-calculated crc for a string vNode
  U32 GetStringCrc();

  // string representation of the atomic value
  const char* StringForm();

  // returns a variable identifier
  const char* GetVariableStr();
  U32 GetVariableCrc();

  // returns a string representation of the atomic type 'aType'
  static const char* GetAtomicString(VNodeAtomicType aType);
};

#endif