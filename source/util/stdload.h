///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FScope handling of common types
//
// 03-JUN-1998
//


#ifndef __STDLOAD_H
#define __STDLOAD_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "mathtypes.h"
#include "dtrack.h"
#include "reaperlist.h"
#include "utiltypes.h"
#include "array.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace StdSave - Standard FScope saving methods
//
namespace StdSave
{
  void TypeU16Array(FScope *parent, const char *name, Array4<U16, 4> &array);

  FScope* TypeU32(FScope *scope, const U32 &val);
  FScope* TypeU32(FScope *parent, const char *name, const U32 &val);

  FScope* TypeF32(FScope *scope, const F32 &val);
  FScope* TypeF32(FScope *parent, const char *name, const F32 &val);

  FScope* TypeString(FScope *scope, const char *val);
  FScope* TypeString(FScope *parent, const char *name, const char *val);

  FScope* TypeBinary(FScope *scope, U32 size, const U8 *data);
  FScope* TypeBinary(FScope *parent, const char *name, U32 size, const U8 *data);

  void TypeVector(FScope *fScope, const Vector &v);
  void TypeVector(FScope *parent, const char *name, const Vector &v);
  void TypeVectorArray(FScope *parent, const char *name, const Array<Vector, 4> &array);

  void TypeMatrix(FScope *fScope, const Matrix &m);
  void TypeMatrix(FScope *parent, const char *name, const Matrix &m);
  void TypeMatrixArray(FScope *parent, const char *name, const Array<Matrix> &array);

  void TypeUVPair(FScope *fScope, const UVPair &uv);
  void TypeUVPair(FScope *parent, const char *name, const UVPair &uv);
  void TypeUVPairArray(FScope *parent, const char *name, const Array<UVPair, 4> &array);

  void TypePlane(FScope *fScope, const Plane &plane);
  void TypePlane(FScope *parent, const char *name, const Plane &plane);
  void TypePlaneArray(FScope *parent, const char *name, const Array<Plane> &array);

  void TypeColor(FScope *fScope, const Color &c);
  void TypeColor(FScope *parent, const char *name, const Color &c);
  void TypeColorArray(FScope *parent, const char *name, const Array<Color, 4> &array);

  void TypeQuaternion(FScope *fScope, const Quaternion &q);
  void TypeQuaternion(FScope *parent, const char *name, const Quaternion &q);

  void TypeSphere(FScope *fScope, const Sphere &sphere);
  void TypeSphere(FScope *parent, const char *name, const Sphere &sphere);

  template <class TYPE> void TypePercentage(FScope *parent, const char *name, TYPE type, TYPE current);

  template <class TYPE> void TypePoint(FScope *parent, const char *name, const Point<TYPE> &val);

  template <class TYPE> void TypeArray(FScope *parent, const char *name, const TYPE *array, U32 count);

  template <class TYPE> void TypeArea(FScope *parent, const char *name, const Area<TYPE> &val)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    // Save point 1 and point 2
    TypePoint<TYPE>(sScope, "Point1", val.p0);
    TypePoint<TYPE>(sScope, "Point2", val.p1);
  }


  //
  // Methods for reapers using 32 bit id's
  //
  template <class DATA> FScope * TypeReaper(FScope *parent, const char *name, Reaper<DATA> &r)
  {
    // Only add function if reaper is alive
    if (r.Alive())
    {
      // Create the function using the supplied name
      FScope *sScope = parent->AddFunction(name);

      // Add the argument
      sScope->AddArgInteger(r.Id());

      return (sScope);
    }

    return (NULL);
  }

  template <class DATA, class NODE> FScope * TypeReaperList(FScope *parent, const char *name, ReaperList<DATA, NODE> &l)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    for (ReaperList<DATA, NODE>::Iterator i(&l); *i; i++)
    {
      TypeReaper(sScope, "ReaperId", **i);
    }

    return (sScope);
  }
  

  //
  // Methods for reapers to game object types (resolved using strings)
  //
  template <class DATA> FScope * TypeReaperObjType(FScope *parent, const char *name, Reaper<DATA> &r)
  {
    // Only add function if reaper is alive
    if (r.Alive())
    {
      // Create the function using the supplied name
      FScope *sScope = parent->AddFunction(name);

      // Add the argument
      sScope->AddArgString(r->GetName());

      return (sScope);
    }
    
    return (NULL);
  }


  template <class DATA, class NODE> FScope * TypeReaperListObjType
  (
    FScope *parent, const char *name, ReaperList<DATA, NODE> &l
  )
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    // Step through each type reaper
    for (ReaperList<DATA, NODE>::Iterator i(&l); *i; i++)
    {
      TypeReaperObjType(sScope, "Add", **i);
    }

    return (sScope);
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace StdLoad - Standard FScope loading methods
//
namespace StdLoad
{
  //
  // Two methods for each type, depending on whether you already have the 
  // target scope.  Some types have a third method that takes a default 
  // value incase the scope is not found.  This should always be used 
  // for the configuration files edited by the designers so the programmers 
  // can add features and not have to wait for the files to become available.
  //
  // A range can be optionally supplied which will fail if the value loaded
  // is not within the range specified.  The range is tested inclusively.
  //

  void TypeU16Array(FScope *fScope, Array4<U16, 4> &array);
  void TypeU16Array(FScope *parent, const char *name, Array4<U16, 4> &array, Bool required = FALSE);

  U32 TypeU32(FScope *fScope, const Range<U32> &range = Range<U32>::full);
  U32 TypeU32(FScope *fScope, U32 dVal, const Range<U32> &range = Range<U32>::full);
  U32 TypeU32(FScope *parent, const char *name, const Range<U32> &range = Range<U32>::full);
  U32 TypeU32(FScope *parent, const char *name, U32 dVal, const Range<U32> &range = Range<U32>::full);

  F32 TypeF32(FScope *fScope, const Range<F32> &range = Range<F32>::full);
  F32 TypeF32(FScope *fScope, F32 dVal, const Range<F32> &range = Range<F32>::full);
  F32 TypeF32(FScope *parent, const char *name, const Range<F32> &range = Range<F32>::full);
  F32 TypeF32(FScope *parent, const char *name, F32 dVal, const Range<F32> &range = Range<F32>::full);

  F32 TypeCompassAngle(FScope *fScope, const Range<F32> &range = Range<F32>(0.0f, 360.0f));
  F32 TypeCompassAngle(FScope *fScope, F32 dVal, const Range<F32> &range = Range<F32>(0.0f, 360.0f));
  F32 TypeCompassAngle(FScope *parent, const char *name, const Range<F32> &range = Range<F32>(0.0f, 360.0f));
  F32 TypeCompassAngle(FScope *parent, const char *name, F32 dVal, const Range<F32> &range = Range<F32>(0.0f, 360.0f));

  const char* TypeString(FScope *fScope);
  const char* TypeStringD(FScope *fScope, const char *dVal);
  const char* TypeString(FScope *parent, const char *name); 
  const char* TypeString(FScope *parent, const char *name, const char *dVal); 

  U32 TypeStringCrc(FScope *fScope);
  U32 TypeStringCrc(FScope *fScope, U32 dVal);
  U32 TypeStringCrc(FScope *parent, const char *name);
  U32 TypeStringCrc(FScope *parent, const char *name, U32 dVal);

  void TypeBinary(FScope *scope, U32 size, U8 *buffer);

  Vector & TypeVector(FScope *fScope, Vector &v);
  Vector & TypeVector(FScope *parent, const char *name, Vector &v);
  Vector & TypeVector(FScope *parent, const char *name, Vector &v, const Vector dVal);
  void TypeVectorArray(FScope *fScope, Array<Vector, 4> &array);
  void TypeVectorArray(FScope *parent, const char *name, Array<Vector, 4> &array, Bool required = FALSE);

  Matrix & TypeMatrix(FScope *fScope, Matrix &m);
  Matrix & TypeMatrix(FScope *parent, const char *name, Matrix &m);
  Matrix & TypeMatrix(FScope *parent, const char *name, Matrix &m, const Matrix dVal);
  void TypeMatrixArray(FScope *fScope, Array<Matrix> &array);
  void TypeMatrixArray(FScope *parent, const char *name, Array<Matrix> &array, Bool required = FALSE);

  UVPair & TypeUVPair(FScope *fScope, const UVPair &uv);
  UVPair & TypeUVPair(FScope *parent, const char *name, UVPair &uv);
  void TypeUVPairArray(FScope *fScope, Array<UVPair, 4> &array);
  void TypeUVPairArray(FScope *parent, const char *name, Array<UVPair, 4> &array, Bool required = FALSE);

  Plane & TypePlane(FScope *fScope, Plane &plane);
  Plane & TypePlane(FScope *parent, const char *name, Plane &plane);
  void TypePlaneArray(FScope *fScope, Array<Plane> &array);
  void TypePlaneArray(FScope *parent, const char *name, Array<Plane> &array);

  Color & TypeColor(FScope *fScope, Color &c);
  Color & TypeColor(FScope *parent, const char *name, Color &c);
  Color & TypeColor(FScope *parent, const char *name, Color &c, const Color dVal);
  void TypeColorArray(FScope *fScope, Array<Color, 4> &array);
  void TypeColorArray(FScope *parent, const char *name, Array<Color, 4> &array, Bool required = FALSE);

  Quaternion & TypeQuaternion(FScope *fScope, Quaternion &q);
  Quaternion & TypeQuaternion(FScope *parent, const char *name, Quaternion &q);

  Sphere & TypeSphere(FScope *fScope, Sphere &sphere);
  Sphere & TypeSphere(FScope *parent, const char *name, Sphere &sphere);

  template <class TYPE> TYPE TypePercentage(FScope *scope, TYPE type);
  template <class TYPE> TYPE TypePercentage(FScope *parent, const char *name, TYPE type, TYPE dVal);

  //
  // Methods for Point template
  //
  template <class TYPE> void TypePoint(FScope *fScope, Point<TYPE> &val);

  template <class TYPE> void TypePoint(FScope *parent, const char *name, Point<TYPE> &val)
  {
    FScope *fScope = parent->GetFunction(name);
    TypePoint<TYPE>(fScope, val);
  }

  template <class TYPE> void TypePoint(FScope *parent, const char *name, Point<TYPE> &val, const Point<TYPE> &dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    fScope ? TypePoint<TYPE>(fScope, val) : val = dVal;
  }


  //
  // Methods for raw arrays
  //
  template <class TYPE> void TypeArray(FScope *fScope, F32 *array, U32 count);


  //
  // Methods for Area template
  //
  template <class TYPE> void TypeArea(FScope *fScope, Area<TYPE> &val)
  {
    // Load point 1 and point 2
    TypePoint<TYPE>(fScope, "Point1", val.p0);
    TypePoint<TYPE>(fScope, "Point2", val.p1);
  }

  template <class TYPE> void TypeArea(FScope *parent, const char *name, Area<TYPE> &val)
  {
    FScope *fScope = parent->GetFunction(name);
    TypeArea<TYPE>(fScope, val);
  }

  template <class TYPE> void TypeArea(FScope *parent, const char *name, Area<TYPE> &val, const Area<TYPE> &dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    fScope ? TypeArea<TYPE>(fScope, val) : val = dVal;
  }


  //
  // Methods for Reaper template
  //
  template <class DATA> void TypeReaper(FScope *fScope, Reaper<DATA> &r)
  {
    // Support old style where argument was not saved for dead reapers
    if (VNode *vNode = fScope->NextArgument(VNode::AT_INTEGER, FALSE))
    {
      r.SetResolveId(U32(vNode->GetInteger()));
    }
  }

  template <class DATA> void TypeReaper(FScope *parent, const char *name, Reaper<DATA> &r)
  {
    if (FScope *fScope = parent->GetFunction(name, FALSE))
    {
      TypeReaper(fScope, r);
    }
  }


  //
  // Methods for Reaper List template
  //
  template <class DATA, class NODE> void TypeReaperList(FScope *fScope, ReaperList<DATA, NODE> &l)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x2C77A1B7: // "ReaperId"
        {
          // Append a new node
          NODE *node = l.Append();

          // Set the resolve id
          node->SetResolveId(TypeU32(sScope));
          break;
        }
      }
    }  
  }

  template <class DATA, class NODE> void TypeReaperList(FScope *parent, const char *name, ReaperList<DATA, NODE> &l)
  {
    if (FScope *fScope = parent->GetFunction(name, FALSE))
    {
      TypeReaperList(fScope, l);
    }
  }

  
  //
  // Methods for reapers to game object types (resolved using strings)
  //
  inline U32 GetIdentAsU32(const char *ident)
  {
    return (reinterpret_cast<U32>(new GameIdent(ident)));
  }

  template <class DATA> void TypeReaperObjType(FScope *fScope, Reaper<DATA> &r)
  {
    // Support old style where argument was not saved for dead reapers
    if (VNode *vNode = fScope->NextArgument(VNode::AT_STRING, FALSE))
    {
      r.SetResolveId(GetIdentAsU32(vNode->GetString()));
    }
  }

  template <class DATA> void TypeReaperObjType(FScope *parent, const char *name, Reaper<DATA> &r)
  {
    // Did we find the method
    if (FScope *fScope = parent->GetFunction(name, FALSE))
    {
      TypeReaperObjType(fScope, r);
    }
  }

  template <class DATA, class NODE> void TypeReaperListObjType(FScope *fScope, ReaperList<DATA, NODE> &l)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          // Append a new node
          NODE *node = l.Append();

          // Load the type name
          TypeReaperObjType(sScope, *node);
          break;
        }
      }
    }  
  }

  template <class DATA, class NODE> void TypeReaperListObjType(FScope *parent, const char *name, ReaperList<DATA, NODE> &l)
  {
    if (FScope *fScope = parent->GetFunction(name, FALSE))
    {
      TypeReaperListObjType(fScope, l);
    }
  }

  template <U32 SIZE> void TypeStrBufList(FScope *fScope, List<StrBuf<SIZE> > &strings)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          const char *str = sScope->NextArgString();

          strings.Append(new StrBuf<SIZE>(str));
          break;
        }
      }
    }  
  }

  template <U32 SIZE> void TypeStrBufList(FScope *parent, const char *name, List<StrBuf<SIZE> > &strings)
  {
    TypeStrBufList(fScope->GetFunction(name), idents);
  }

  template <U32 SIZE> void TypeStrBufBinTree(FScope *fScope, BinTree<StrBuf<SIZE> > &strings)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          const char *str = sScope->NextArgString();
          strings.Add(Crc::CalcStr(str), new StrBuf<SIZE>(str));
          break;
        }
      }
    }  
  }

  template <U32 SIZE> void TypeStrBufBinTree(FScope *parent, const char *name, BinTree<StrBuf<SIZE> > &strings)
  {
    TypeStrBufBinTree(fScope->Getfunction(name), idents);
  }

  template <U32 SIZE> void TypeStrCrcList(FScope *fScope, List<StrCrc<SIZE> > &strings)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          const char *str = sScope->NextArgString();

          strings.Append(new StrCrc<SIZE>(str));
          break;
        }
      }
    }  
  }

  template <U32 SIZE> void TypeStrCrcList(FScope *fScope, const char *name, List<StrCrc<SIZE> > &strings)
  {
    TypeStrCrcList(fScope->GetFunction(name), idents);
  }

  template <U32 SIZE> void TypeStrCrcBinTree(FScope *fScope, BinTree<StrCrc<SIZE> > &idents)
  {
    FScope *sScope;

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          const char *str = sScope->NextArgString();
          idents.Add(Crc::CalcStr(str), new StrCrc<SIZE>(str));
          break;
        }
      }
    }  
  }

  template <U32 SIZE> void TypeStrCrcBinTree(FScope *parent, const char *name, BinTree<StrCrc<SIZE> > &idents)
  {
    TypeStrCrcBinTree(parent->GetFunction(name), idents);
  }

};

#endif