///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Object Promoters
//
// 11-NOV-1998
//

#ifndef __PROMOTE_H
#define __PROMOTE_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Promote
//
namespace Promote
{

  // Type Promoter
  template <class TYPE> TYPE * Type(GameObjType *type)
  {
    ASSERT(type);

    // Is this a valid cast
    if (type->DerivedFrom(TYPE::ClassId()))
    {
      return ((TYPE *) type);
    }

    // Not able to cast
    return (NULL);
  }

  // Object Promoter
  template <class TYPE, class OBJECT> OBJECT * Object(GameObj *obj)
  {
    // If we can promote its type, we can do a safe cast
    if (Promote::Type<TYPE>(obj->GameType()))
    {
      return ((OBJECT *) obj);
    }

    // Not able to cast
    return (NULL);
  }

  // Object List Promoter
  template <class BASEOBJECT, class BASENODE, class TYPE, class OBJECT, class NODE> 
  void ObjList(const ReaperList<BASEOBJECT, BASENODE> &listIn, ReaperList<OBJECT, NODE> &listOut)
  {
    ReaperList<BASEOBJECT, BASENODE>::Iterator i(&listIn);
    BASENODE *node;

    listOut.Clear();

    while ((node = i++) != NULL)
    {
      OBJECT *obj = Promote::Object<TYPE, OBJECT>(**i);
      if (obj)
      {
        listOut.Append(obj);
      }
    }
  }

}


#endif