///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FScope handling of common types
//
// 20-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "stdload.h"
///////////////////////////////////////////////////////////////////////////////
//
// NameSpace StdSave - Standard FScope saving methods
//
namespace StdSave
{
  //
  // Methods for U16 type
  // 
  void TypeU16Array(FScope *parent, const char *name, Array4<U16, 4> &array)
  {
    if (!array.count)
    {
      return;
    }

    FScope *fScope = parent->AddFunction(name);

    fScope->AddArgInteger( (S32) array.count);

    for (U32 i = 0; i < array.count; i++)
    {
      StdSave::TypeU32( fScope, "Index", (U32) array[i]);
    }
  }
  

  //
  // Methods for U32 type
  // 
  FScope* TypeU32(FScope *scope, const U32 &val)
  {
    scope->AddArgInteger(S32(val));
    return (scope);
  }

  FScope* TypeU32(FScope *parent, const char *name, const U32 &val)
  {
    return (TypeU32(parent->AddFunction(name), val));
  }

  //
  // Methods for F32 type
  // 
  FScope* TypeF32(FScope *scope, const F32 &val)
  {
    scope->AddArgFPoint(val);
    return (scope);
  }

  FScope* TypeF32(FScope *parent, const char *name, const F32 &val)
  {
    return (TypeF32(parent->AddFunction(name), val));
  }


  //
  // Methods for string type
  //
  FScope* TypeString(FScope *scope, const char *val)
  {
    scope->AddArgString(val);
    return (scope);
  }

  FScope* TypeString(FScope *parent, const char *name, const char *val)
  {
    return (TypeString(parent->AddFunction(name), val));
  }


  //
  // Methods for binary type
  //
  FScope* TypeBinary(FScope *scope, U32 size, const U8 *data)
  {
    char buffer[65];

    while (size > 32)
    {
      Utils::CharEncode(buffer, data, 32, 65);
      TypeString(scope, "Value", buffer);

      size -= 32;
      data += 32;
    }

    if (size)
    {
      Utils::CharEncode(buffer, data, size, 65);
      StdSave::TypeString(scope, "Value", buffer);
    }

    return (scope);
  }

  FScope* TypeBinary(FScope *parent, const char *name, U32 size, const U8 *data)
  {
    return (TypeBinary(parent->AddFunction(name), size, data));
  }


  //
  // Methods for vector type
  //
  void TypeVector(FScope *fScope, const Vector &v)
  {
    // Create 3 arguments from the vector
    fScope->AddArgFPoint(v.x);
    fScope->AddArgFPoint(v.y);
    fScope->AddArgFPoint(v.z);
  }
  void TypeVector(FScope *parent, const char *name, const Vector &v)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    TypeVector(sScope, v);
  }
  void TypeVectorArray(FScope *parent, const char *name, const Array<Vector, 4> &array)
  {
    if (!array.count)
    {
      return;
    }

    FScope *fScope = parent->AddFunction(name);

    fScope->AddArgInteger( array.count);

    for (U32 i = 0; i < array.count; i++)
    {
      TypeVector( fScope, "Vector", array[i]);
    }
  }

  void TypeMatrix(FScope *fScope, const Matrix &m)
  {
    TypeVector(fScope, "Pos",   m.posit);
    TypeVector(fScope, "Right", m.right);
    TypeVector(fScope, "Up",    m.up);
    TypeVector(fScope, "Front", m.front);
  }
  void TypeMatrix(FScope *parent, const char *name, const Matrix &m)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    TypeMatrix( sScope, m);
  }
  void TypeMatrixArray(FScope *parent, const char *name, const Array<Matrix> &array)
  {
    if (!array.count)
    {
      return;
    }

    FScope *fScope = parent->AddFunction(name);

    fScope->AddArgInteger( array.count);

    for (U32 i = 0; i < array.count; i++)
    {
      TypeMatrix( fScope, "Matrix", array[i]);
    }
  }

  void TypeUVPair(FScope *fScope, const UVPair &uv)
  {
    // Create 2 arguments
    fScope->AddArgFPoint(uv.u);
    fScope->AddArgFPoint(uv.v);
  }
  void TypeUVPair(FScope *parent, const char *name, const UVPair &uv)
  {
    FScope *fScope = parent->AddFunction(name);
    TypeUVPair( fScope, uv);
  }
  void TypeUVPairArray(FScope *parent, const char *name, const Array<UVPair, 4> &array)
  {
    if (!array.count)
    {
      return;
    }

    FScope *fScope = parent->AddFunction(name);

    fScope->AddArgInteger( array.count);

    for (U32 i = 0; i < array.count; i++)
    {
      TypeUVPair( fScope, "UVPair", array[i]);
    }
  }

  void TypePlane(FScope *fScope, const Plane &plane)
  {
    // vector plus distance
    TypeVector(fScope, plane);
    fScope->AddArgFPoint(plane.d);
  }
  void TypePlane(FScope *parent, const char *name, const Plane &plane)
  {
    FScope *fScope = parent->AddFunction(name);
    TypePlane( fScope, plane);
  }
  void TypePlaneArray(FScope *parent, const char *name, const Array<Plane> &array)
  {
    if (!array.count)
    {
      return;
    }

    FScope *fScope = parent->AddFunction(name);

    fScope->AddArgInteger( array.count);

    for (U32 i = 0; i < array.count; i++)
    {
      TypePlane( fScope, "Plane", array[i]);
    }
  }

  void TypeQuaternion(FScope *fScope, const Quaternion &q)
  {
    // 4 floats
    fScope->AddArgFPoint(q.s);
    fScope->AddArgFPoint(q.v.x);
    fScope->AddArgFPoint(q.v.y);
    fScope->AddArgFPoint(q.v.z);
  }
  void TypeQuaternion(FScope *parent, const char *name, const Quaternion &q)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    TypeQuaternion( sScope, q);
  }

  void TypeSphere(FScope *fScope, const Sphere &sphere)
  {
    // matrix plus box size and radius
    fScope->AddArgFPoint( sphere.radius);

    TypeMatrix(fScope, "Matrix", sphere);
    TypeF32( fScope, "Width", sphere.Width);
    TypeF32( fScope, "Height", sphere.Height);
    TypeF32( fScope, "Breadth", sphere.Breadth);
  }
  void TypeSphere(FScope *parent, const char *name, const Sphere &sphere)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    TypeSphere( sScope, sphere);
  }

  void TypeColor(FScope *fScope, const Color &c)
  {
    // Save the RGBA values
    fScope->AddArgInteger(c.R());
    fScope->AddArgInteger(c.G());
    fScope->AddArgInteger(c.B()); 
    fScope->AddArgInteger(c.A());
  }
  void TypeColor(FScope *parent, const char *name, const Color &c)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    TypeColor( sScope, c);
  }
  void TypeColorArray(FScope *parent, const char *name, const Array<Color, 4> &array)
  {
    if (!array.count)
    {
      return;
    }

    FScope *fScope = parent->AddFunction(name);

    fScope->AddArgInteger( array.count);

    for (U32 i = 0; i < array.count; i++)
    {
      TypeColor( fScope, "Color", array[i]);
    }
  }

  template <> void TypePercentage(FScope *parent, const char *name, F32 type, F32 current)
  {
    TypeF32(parent, name, F32(type ? current / type : 0.0F));
  }

  template <> void TypePercentage(FScope *parent, const char *name, U32 type, U32 current)
  {
    TypeF32(parent, name, F32(type ? F32(current) / F32(type) : 0.0F));
  }

  template <> void TypePercentage(FScope *parent, const char *name, S32 type, S32 current)
  {
    TypeF32(parent, name, F32(type ? F32(current) / F32(type) : 0.0F));
  }

  template <> void TypePoint(FScope *parent, const char *name, const Point<F32> &val)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    // Save the X and Y values
    sScope->AddArgFPoint(val.x);
    sScope->AddArgFPoint(val.y);
  }

  template <> void TypePoint(FScope *parent, const char *name, const Point<U32> &val)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    // Save the X and Y values
    sScope->AddArgInteger(val.x);
    sScope->AddArgInteger(val.y);
  }

  template <> void TypePoint(FScope *parent, const char *name, const Point<S32> &val)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    // Save the X and Y values
    sScope->AddArgInteger(val.x);
    sScope->AddArgInteger(val.y);
  }


  template <> void TypeArray(FScope *parent, const char *name, const F32 *array, U32 count)
  {
    // Create the function using the supplied name
    FScope *sScope = parent->AddFunction(name);

    for (U32 i = 0; i < count; i++)
    {
      StdSave::TypeF32(sScope, "Item", array[i]);
    }
  }
}



///////////////////////////////////////////////////////////////////////////////
//
// NameSpace StdLoad - Standard FScope loading methods
//
namespace StdLoad
{

  //
  // Methods for U16 type
  // 
  void TypeU16Array(FScope *fScope, Array4<U16, 4> &array)
  {
    S32 count = fScope->NextArgInteger();
    array.Alloc( count);

    for (S32 i = 0; i < count; i++)
    {
      FScope *sScope = fScope->NextFunction();
      array[i] = (U16) sScope->NextArgInteger();
    }
  }
  void TypeU16Array(FScope *parent, const char *name, Array4<U16, 4> &array, Bool required) // = FALSE)
  {
    FScope *fScope = parent->GetFunction(name, required);
    if (fScope)
    {
      TypeU16Array( fScope, array);
    }
  }

  //
  // Methods for U32 type
  // 
  U32 TypeU32(FScope *fScope, const Range<U32> &range)
  {
    U32 val = (U32) fScope->NextArgInteger();

    if (!range.Inc(val))
    {
      fScope->ScopeError("Value %d for '%s' is out of range %d to %d", val, fScope->NameStr(), range.Min(), range.Max());
    }

    return (val);
  }

  U32 TypeU32(FScope *fScope, U32 dVal, const Range<U32> &range)
  {
    // get the next argument
    VNode *vNode = fScope->NextArgument();

    U32 val;

    if (vNode)
    {
      if (vNode->aType == VNode::AT_INTEGER)
      {
        val = vNode->GetInteger();
      }
      else
      {
        fScope->ScopeError("Integer argument expected but found type '%s'", VNode::GetAtomicString(vNode->aType));
      }
    }
    else
    {
      val = dVal;
    }

    if (!range.Inc(val))
    {
      fScope->ScopeError("Value %d for '%s' is out of range %d to %d", val, fScope->NameStr(), range.Min(), range.Max());
    }

    return (val);
  }

  U32 TypeU32(FScope *parent, const char *name, const Range<U32> &range)
  {
    FScope *fScope = parent->GetFunction(name);
    return (TypeU32(fScope, range));
  }

  U32 TypeU32(FScope *parent, const char *name, U32 dVal, const Range<U32> &range)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    return (fScope ? TypeU32(fScope, range) : dVal);
  }


  //
  // Methods for F32 type
  // 
  F32 TypeF32(FScope *fScope, const Range<F32> &range)
  {
    F32 val = fScope->NextArgFPoint();

    if (!range.Inc(val))
    {
      fScope->ScopeError("Value %g for '%s' is out of range %g to %g", val, fScope->NameStr(), range.Min(), range.Max());
    }

    return (val);
  }

  F32 TypeF32(FScope *fScope, F32 dVal, const Range<F32> &range)
  {
    // get the next argument
    VNode *vNode = fScope->NextArgument();

    F32 val;

    if (vNode)
    {
      switch (vNode->aType)
      {
        case VNode::AT_FPOINT:
          val = vNode->GetFPoint();
          break;

        case VNode::AT_INTEGER:
          // integers can be turned into floats
          val = (F32) vNode->GetInteger();
          break;

        default:
          fScope->ScopeError("Floating Point argument expected but found type '%s'", VNode::GetAtomicString(vNode->aType));
      }
    }
    else
    {
      val = dVal;
    }

    if (!range.Inc(val))
    {
      fScope->ScopeError("Value %g for '%s' is out of range %g to %g", val, fScope->NameStr(), range.Min(), range.Max());
    }

    return (val);
  }

  F32 TypeF32(FScope *parent, const char *name, const Range<F32> &range)
  {
    FScope *fScope = parent->GetFunction(name);
    return (TypeF32(fScope, range));
  }

  F32 TypeF32(FScope *parent, const char *name, F32 dVal, const Range<F32> &range)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    return (fScope ? TypeF32(fScope, range) : dVal);
  }


  F32 TypeCompassAngle(FScope *fScope, const Range<F32> &range)
  {
    return (COMPASS2ANGLE(TypeF32(fScope, range)));
  }

  F32 TypeCompassAngle(FScope *fScope, F32 dVal, const Range<F32> &range)
  {
    return (COMPASS2ANGLE(TypeF32(fScope, dVal, range)));
  }

  F32 TypeCompassAngle(FScope *parent, const char *name, const Range<F32> &range)
  {
    return (COMPASS2ANGLE(TypeF32(parent, name, range)));
  }

  F32 TypeCompassAngle(FScope *parent, const char *name, F32 dVal, const Range<F32> &range)
  {
    return (COMPASS2ANGLE(TypeF32(parent, name, dVal, range)));
  }


  //
  // Methods for String type
  // 
  const char* TypeString(FScope *fScope)
  {
    return (fScope->NextArgString());
  }

  const char* TypeStringD(FScope *fScope, const char *dVal)
  {
    // get the next argument
    VNode *vNode = fScope->NextArgument();

    const char *val;

    if (vNode)
    {
      if (vNode->aType == VNode::AT_STRING)
      {
        val = vNode->GetString();
      }
      else
      {
        fScope->ScopeError("String argument expected but found type '%s'", VNode::GetAtomicString(vNode->aType));
      }
    }
    else
    {
      val = dVal;
    }
    return (val);
  }

  const char* TypeString(FScope *parent, const char *name)
  {
    FScope *fScope = parent->GetFunction(name);
    return (TypeString(fScope));
  }

  const char* TypeString(FScope *parent, const char *name, const char *dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    return (fScope ? TypeString(fScope) : dVal);
  }


  //
  // Methods for a Crc from a string
  //
  U32 TypeStringCrc(FScope *fScope)
  {
    return (Crc::CalcStr(fScope->NextArgString()));
  }

  U32 TypeStringCrc(FScope *fScope, U32 dVal)
  {
    // Get the Next Argument
    VNode *vNode = fScope->NextArgument();

    U32 val;

    if (vNode)
    {
      if (vNode->aType == VNode::AT_STRING)
      {
        val = Crc::CalcStr(vNode->GetString());
      }
      else
      {
        fScope->ScopeError("String argument expected but found type '%s'", VNode::GetAtomicString(vNode->aType));
      }
    }
    else
    {
      val = dVal;
    }

    return (val);
  }

  U32 TypeStringCrc(FScope *parent, const char *name)
  {
    return (TypeStringCrc(parent->GetFunction(name)));
  }

  U32 TypeStringCrc(FScope *parent, const char *name, U32 dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    return (fScope ? TypeStringCrc(fScope) : dVal);
  }


  //
  // Methods for binary data
  //
  void TypeBinary(FScope *scope, U32 size, U8 *buffer)
  {
    // Ensure we get all scopes
    scope->InitIterators();

    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x729BA3E0: // "Value"
        {
          const char *string = TypeString(sScope);
          U32 length = Utils::Strlen(string);
          Utils::CharDecode(buffer, string, length, size);
          buffer += length / 2;
          size -= length / 2;
          break;
        }
      }
    }
  }


  //
  // Methods for a List of Idents
  //
  void TypeIdentList(FScope *fScope, List<GameIdent> &idents)
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

          idents.Append(new GameIdent(str));
          break;
        }
      }
    }  
  }

  void TypeIdentList(FScope *fScope, const char *name, List<GameIdent> &idents)
  {
    TypeIdentList(fScope->GetFunction(name), idents);
  }


  //
  // Methods for Vector type
  // 
  Vector & TypeVector(FScope *fScope, Vector &v)
  {
    v.x = fScope->NextArgFPoint();      
    v.y = fScope->NextArgFPoint();      
    v.z = fScope->NextArgFPoint();
    return (v);
  }
  Vector & TypeVector(FScope *parent, const char *name, Vector &v)
  {
    FScope *fScope = parent->GetFunction(name);
    TypeVector(fScope, v);
    return (v);
  }
  Vector & TypeVector(FScope *parent, const char *name, Vector &v, const Vector dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    fScope ? TypeVector(fScope, v) : v = dVal;
    return (v);
  }
  void TypeVectorArray(FScope *fScope, Array<Vector, 4> &array)
  {
    S32 count = fScope->NextArgInteger();
    array.Alloc( count);

    for (S32 i = 0; i < count; i++)
    {
      FScope *sScope = fScope->NextFunction();
      TypeVector( sScope, array[i]);
    }
  }
  void TypeVectorArray(FScope *parent, const char *name, Array<Vector, 4> &array, Bool required) // = FALSE)
  {
    FScope *fScope = parent->GetFunction(name, required);
    if (fScope)
    {
      TypeVectorArray( fScope, array);
    }
  }

  //
  // Methods for Matrix type
  // 
  Matrix & TypeMatrix(FScope *fScope, Matrix &m)
  {
    FScope *sScope;

    // Ensure the matrix is cleared
    m.ClearData();

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x29BAA7D3: // "Pos"
          TypeVector(sScope, m.posit);
          break;

        case 0xE2DDD72B: // "Right"
          TypeVector(sScope, m.right);
          break;

        case 0xF975A769: // "Up"
          TypeVector(sScope, m.up);
          break;

        case 0xC46186FD: // "Front"
          TypeVector(sScope, m.front);
          break;
      }
    }

    return (m);
  }
  Matrix & TypeMatrix(FScope *parent, const char *name, Matrix &m)
  {
    FScope *fScope = parent->GetFunction(name);
    TypeMatrix(fScope, m);
    return (m);
  }
  Matrix & TypeMatrix(FScope *parent, const char *name, Matrix &m, const Matrix dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);

    if (fScope)
    {
      TypeMatrix(fScope, m);
    }
    else
    {
      m = dVal;
    }
    return (m);
  }
  void TypeMatrixArray(FScope *fScope, Array<Matrix> &array)
  {
    S32 count = fScope->NextArgInteger();
    array.Alloc( count);

    for (S32 i = 0; i < count; i++)
    {
      FScope *sScope = fScope->NextFunction();
      TypeMatrix( sScope, array[i]);
    }
  }
  void TypeMatrixArray(FScope *parent, const char *name, Array<Matrix> &array, Bool required) // = FALSE)
  {
    FScope *fScope = parent->GetFunction(name, required);
    if (fScope)
    {
      TypeMatrixArray( fScope, array);
    }
  }

  UVPair & TypeUVPair(FScope *fScope, UVPair &uv)
  {
    uv.u = fScope->NextArgFPoint();      
    uv.v = fScope->NextArgFPoint();      
    return (uv);
  }
  UVPair & TypeUVPair(FScope *parent, const char *name, UVPair &uv)
  {
    FScope *fScope = parent->GetFunction(name);

    return TypeUVPair( fScope, uv);
  }
  void TypeUVPairArray(FScope *fScope, Array<UVPair, 4> &array)
  {
    S32 count = fScope->NextArgInteger();
    array.Alloc( count);

    for (S32 i = 0; i < count; i++)
    {
      FScope *sScope = fScope->NextFunction();
      TypeUVPair( sScope, array[i]);
    }
  }
  void TypeUVPairArray(FScope *parent, const char *name, Array<UVPair, 4> &array, Bool required) // = FALSE)
  {
    FScope *fScope = parent->GetFunction(name, required);
    if (fScope)
    {
      TypeUVPairArray( fScope, array);
    }
  }

  Plane & TypePlane(FScope *fScope, Plane &plane)
  {
    // vector plus distance
    TypeVector(fScope, plane);
    plane.d = fScope->NextArgFPoint();
    return plane;
  }
  Plane & TypePlane(FScope *parent, const char *name, Plane &plane)
  {
    FScope *fScope = parent->GetFunction(name);

    return TypePlane( fScope, plane);
  }
  void TypePlaneArray(FScope *fScope, Array<Plane> &array)
  {
    S32 count = fScope->NextArgInteger();
    array.Alloc( count);

    for (S32 i = 0; i < count; i++)
    {
      FScope *sScope = fScope->NextFunction();
      TypePlane( sScope, array[i]);
    }
  }
  void TypePlaneArray(FScope *parent, const char *name, Array<Plane> &array)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    if (fScope)
    {
      TypePlaneArray( fScope, array);
    }
  }

  Quaternion & TypeQuaternion(FScope *fScope, Quaternion &q)
  {
    // 4 floats
    q.s   = fScope->NextArgFPoint();
    q.v.x = fScope->NextArgFPoint();
    q.v.y = fScope->NextArgFPoint();
    q.v.z = fScope->NextArgFPoint();
    return q;
  }
  Quaternion & TypeQuaternion(FScope *parent, const char *name, Quaternion &q)
  {
    FScope *fScope = parent->GetFunction(name);
    TypeQuaternion(fScope, q);
    return (q);
  }

  Sphere & TypeSphere(FScope *fScope, Sphere &sphere)
  {
    // matrix plus box size and radius
    sphere.radius = fScope->NextArgFPoint();
    sphere.radius2 = sphere.radius * sphere.radius;

    TypeMatrix(fScope, "Matrix", sphere);
    sphere.Width   = TypeF32( fScope, "Width");
    sphere.Height  = TypeF32( fScope, "Height");
    sphere.Breadth = TypeF32( fScope, "Breadth");

    return sphere;
  }
  Sphere & TypeSphere(FScope *parent, const char *name, Sphere &sphere)
  {
    FScope *fScope = parent->GetFunction(name);
    TypeSphere(fScope, sphere);
    return (sphere);
  }

  //
  // Methods for Color type
  // 
  Color & TypeColor(FScope *fScope, Color &c)
  {
    // Read the values
    U32 r = TypeU32(fScope);
    U32 g = TypeU32(fScope);
    U32 b = TypeU32(fScope);
    U32 a = TypeU32(fScope, 255);

    // And set the color
    c.Set(r, g, b, a);
    return (c);
  }
  Color & TypeColor(FScope *parent, const char *name, Color &c)
  {
    FScope *fScope = parent->GetFunction(name);
    TypeColor(fScope, c);
    return (c);
  }
  Color & TypeColor(FScope *parent, const char *name, Color &c, const Color dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    fScope ? TypeColor(fScope, c) : c = dVal;
    return (c);
  }
  void TypeColorArray(FScope *fScope, Array<Color, 4> &array)
  {
    S32 count = fScope->NextArgInteger();
    array.Alloc( count);

    for (S32 i = 0; i < count; i++)
    {
      FScope *sScope = fScope->NextFunction();
      TypeColor( sScope, array[i]);
    }
  }
  void TypeColorArray(FScope *parent, const char *name, Array<Color, 4> &array, Bool required) // = FALSE)
  {
    FScope *fScope = parent->GetFunction(name, required);
    if (fScope)
    {
      TypeColorArray( fScope, array);
    }
  }

  template <> F32 TypePercentage(FScope *scope, F32 type)
  {
    return (type * scope->NextArgFPoint());
  }

  template <> U32 TypePercentage(FScope *scope, U32 type)
  {
    return (Utils::FtoLNearest(F32(type) * scope->NextArgFPoint()));
  }

  template <> S32 TypePercentage(FScope *scope, S32 type)
  {
    return (Utils::FtoLNearest(F32(type) * scope->NextArgFPoint()));
  }

  template <> F32 TypePercentage(FScope *parent, const char *name, F32 type, F32 dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    
    return (fScope ? type * fScope->NextArgFPoint() : dVal);
  }

  template <> U32 TypePercentage(FScope *parent, const char *name, U32 type, U32 dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    
    return (fScope ? Utils::FtoLNearest(F32(type) * fScope->NextArgInteger()) : dVal);
  }

  template <> S32 TypePercentage(FScope *parent, const char *name, S32 type, S32 dVal)
  {
    FScope *fScope = parent->GetFunction(name, FALSE);
    
    return (fScope ? Utils::FtoLNearest(F32(type) * fScope->NextArgInteger()) : dVal);
  }

  template <> void TypePoint(FScope *fScope, Point<F32> &val)
  {
    val.x = fScope->NextArgFPoint();      
    val.y = fScope->NextArgFPoint();      
  }

  template <> void TypePoint(FScope *fScope, Point<U32> &val)
  {
    val.x = fScope->NextArgInteger();      
    val.y = fScope->NextArgInteger();      
  }

  template <> void TypePoint(FScope *fScope, Point<S32> &val)
  {
    val.x = fScope->NextArgInteger();      
    val.y = fScope->NextArgInteger();      
  }

  template <> void TypeArray(FScope *fScope, F32 *array, U32 count)
  {
    FScope *sScope;
    U32 i = 0;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1AE0C78A: // "Item"
        {
          if (i < count)
          {
            array[i++] = StdLoad::TypeF32(sScope);
          }
          else
          {
            return;
          }
          break;
        }
      }
    }
  }
}
