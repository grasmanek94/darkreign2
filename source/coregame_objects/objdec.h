///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 15-JAN-1999
//

#ifndef __OBJDEC_H
#define __OBJDEC_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "reaperlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

#define OBJDEC(obj)                                                           \
                                                                              \
class obj;                                                                    \
class obj##Type;                                                              \
                                                                              \
typedef Reaper<obj> obj##Ptr;                                                 \
                                                                              \
class obj##ListNode : public obj##Ptr                                         \
{                                                                             \
public:                                                                       \
  NList<obj##ListNode>::Node node;                                            \
};                                                                            \
                                                                              \
typedef ReaperList<obj, obj##ListNode> obj##List;                             \
                                                                              \
typedef Reaper<obj##Type> obj##TypePtr;                                       \
                                                                              \
class obj##TypeListNode : public obj##TypePtr                                 \
{                                                                             \
public:                                                                       \
  NList<obj##TypeListNode>::Node node;                                        \
};                                                                            \
                                                                              \
typedef ReaperList<obj##Type, obj##TypeListNode> obj##TypeList;               \


#define OBJDEC_NAMESPACE                                                      \
                                                                              \
class Object;                                                                 \
class Type;                                                                   \
                                                                              \
typedef Reaper<Object> Ptr;                                                   \
                                                                              \
class ListNode : public Ptr                                                   \
{                                                                             \
public:                                                                       \
  NList<ListNode>::Node node;                                                 \
};                                                                            \
                                                                              \
typedef ReaperList<Object, ListNode> ObjList;                                 \
                                                                              \
typedef Reaper<Type> TypePtr;                                                 \
                                                                              \
class TypeListNode : public TypePtr                                           \
{                                                                             \
public:                                                                       \
  NList<TypeListNode>::Node node;                                             \
};                                                                            \
                                                                              \
typedef ReaperList<Type, TypeListNode> TypeList;                              \


#endif