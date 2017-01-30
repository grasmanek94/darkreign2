///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Math Types
//
// 01-APR-1998        John Cooke
//

#ifndef __MATHTYPES_H
#define __MATHTYPES_H

//----------------------------------------------------------------------------

#pragma warning(push, 3)
#include <ostream>
#pragma warning(pop)
using std::ostream;

#include "utils.h"
#include "utiltypes.h"
//----------------------------------------------------------------------------

const F32 PI				      = 3.14159265358979323846f;
const F32 PI2 				    = PI * 2.0f;
const F32 PIBY2				    = PI * 0.5f;
const F32 VALDEGTORAD		  = PI / 180.0f;
const F32 DEG2RAD     	  = PI / 180.0f;
const F32 RAD2DEG     	  = 180.0f / PI;
const F32 PIINV           = 1.0f / PI;

#define COMPASS2ANGLE(x) ((90.0f - (x)) * DEG2RAD) 
#define ANGLE2COMPASS(x) (90.0f - ((x) * RAD2DEG))


//----------------------------------------------------------------------------

class Bitmap;
struct Matrix;			// forward reference for Vector and Quaternion
struct Vertex;
struct VertexL;
struct VertexTL;
struct Plane;

struct UVPairS
{
	F32 u, v;
	
	void ClearData()
	{
		u = v = 0.0f;
	}
	void Set( F32 _u, F32 _v)
	{
		u = _u; v = _v;
	}
};

struct UVPair : public UVPairS
{
	UVPair() {}
	UVPair( F32 _u, F32 _v)
	{
		Set( _u, _v);
	}
	UVPair ( const UVPairS &uv)
	{
		Set( uv.u, uv.v);
	}
	inline void operator=( const UVPairS &uv)
	{
		Set( uv.u, uv.v);
	}
};
//----------------------------------------------------------------------------

// A direction of a unit vector in polar coordinates
struct VectorDir
{
  F32 u, v;

  VectorDir() { }
  VectorDir(F32 u, F32 v) : u(u), v(v) { }

  void Zero()
  {
    u = v = 0.0;
  }
  void ClearData()
  {
    Zero();
  }
	void Set(F32 uIn, F32 vIn)
	{
		u = uIn;
    v = vIn;
	}
	void operator-=(const VectorDir &d)
	{
    u -= d.u;
    v -= d.v;
    Fix();
  }
  void operator+=(const VectorDir &d)
	{
    u += d.u;
    v += d.v;
    Fix();
	}
  static void FixU(F32 &u)
  {
    if (u > PI)
    {
      u -= PI2;
    }
    if (u < -PI)
    {
      u += PI2;
    }
  }
  static void FixV(F32 &v)
  {
    if (v > PI) 
    { 
      v -= PI2;
    }
    if (v < -PI)
    {
      v += PI2;
    }
/*
    if (v > PI)
    {
      v = PI - v;
      u += PI;
      if (u > PI)
      {
        u -= PI2;
      }
    }
    if (v < 0) 
    {
      v = -v;
      u += PI;
      if (u > PI)
      {
        u -= PI2; 
      }
    }
*/
  }
  void Fix()
  {
    FixU();
    FixV();
  }
  void FixU()
  {
    FixU(u);
  }
  void FixV()
  {
    FixV(v);
  }

  inline friend ostream& operator<<(ostream &o, const VectorDir &v)
  {
    return (o << '[' << v.u << ',' << v.v << ']');
  }

};
//----------------------------------------------------------------------------

typedef struct 
{
  F32 x;
  F32 y; 
  F32 z;
} VectorData;

struct Vector
{
	union 
  {
		struct 
    {
			F32	x;
			F32	y; 
			F32	z;
		};

		VectorData data;
	};

  // Default constructor
	Vector() {}

  // Initializing constructor
	Vector(F32 _x, F32 _y, F32 _z) 
	{
    Set( _x, _y, _z);
	}

  Vector( const Vector &v)
  {
    Set( v.x, v.y, v.z);
  }

  // VectorData copy constructor
  Vector(const VectorData &data)
  {
    x = data.x;
    y = data.y;
    z = data.z;
  }

  // VectorDir constructor
  Vector(const VectorDir &v)
  {
    Set(v);
  }

	// setup
	void ClearData()
	{
		Zero();
	}
	inline void Zero()
	{ 
		x = y = z = 0.0f;
	}
	inline void Set(F32 xx, F32 yy, F32 zz)
	{
		x = xx; y = yy; z = zz;
	}
  inline void Set(const VectorDir &v)
  {
    Set(v.u, v.v);
  }
	inline void Set(F32 u, F32 v)
  {
    F32 sv = (F32) sin(v);
    x = (F32) (cos(u)) * sv;
    y = (F32) (cos(v));
    z = (F32) (sin(u)) * sv;
  }

	// vector functions
	inline Bool operator==(const Vector &vv) const
	{
		return (Bool)(x == vv.x && y == vv.y && z == vv.z);
	}
	inline Bool operator!=(const Vector &vv) const
	{
		return (Bool)(x != vv.x || y != vv.y || z != vv.z);
	}
	inline void operator-=(const Vector &vv)
	{
		x -= vv.x; y -= vv.y; z -= vv.z;
	}
	inline void operator+=(const Vector &vv)
	{
		x += vv.x; y += vv.y; z += vv.z; 
	}
	inline void operator/=(const Vector &vv)
	{
		x /= vv.x; y /= vv.y; z /= vv.z;
	}
	inline void operator*=(const Vector &vv)
	{
		x *= vv.x; y *= vv.y; z *= vv.z;
	}

#if 0
  inline Vector operator-(const Vector &vv) const
	{
		Vector result = *this;
		result -= vv;
		return result;
	}
	inline Vector operator+(const Vector &vv) const
	{
		Vector result = *this;
		result += vv;
		return result;
	}
#endif

  inline Vector operator*(const Vector &vv) const
	{
		Vector result = *this;
		result *= vv;
		return result;
	}
	inline Vector operator/(const Vector &vv) const
	{
		Vector result = *this;
		result /= vv;
		return result;
	}

	friend inline Vector operator*(const Vector &A, F32 ss)  
	{
		return Vector (A.x * ss , A.y * ss, A.z * ss);
	}
	friend inline Vector operator/(const Vector &A, F32 ss)  
	{
		return Vector (A.x / ss, A.y / ss, A.z / ss);
	}
	friend inline Vector operator-(const Vector &A, const Vector &B) 
	{
		return Vector (A.x - B.x, A.y - B.y, A.z - B.z);
	}
	friend inline Vector operator+(const Vector &A, const Vector &B) 
	{
		return Vector (A.x + B.x, A.y + B.y, A.z + B.z);
	}
	friend inline Vector Cross (const Vector &A,  const Vector &B)
	{
		return Vector (A.y * B.z - A.z * B.y, A.z * B.x - A.x * B.z, A.x * B.y - A.y * B.x);
	}
	inline Vector Cross(const Vector &vv) const
	{
		return ::Cross (*this, vv);
	}

	F32 Dot(const Vector &vv) const
	{
		return x * vv.x + y * vv.y + z * vv.z;
	}
//  void Cross(Vector &dst, const Vector &vv) const;
	
	// F32 functions
	inline void operator=(F32 ss)
	{
		x = ss; y = ss; z = ss;
	}
	inline Bool operator==(F32 ss) const
	{
		return (Bool)(x == ss && y == ss && z == ss);
	}
	inline void operator+=(const F32 ss)
	{
		x += ss; y += ss; z += ss;
	}
	inline void operator*=(const F32 ss)
	{
		x *= ss; y *= ss; z *= ss; 
	}
	inline void operator/=(const F32 ss)
	{
		F32 iss = 1.0f / ss;
		*this *= iss;
	}
	
  inline Bool operator>(Vector v)
  {
    return (F32)fabs(x) > v.x || (F32)fabs(y) > v.y || (F32)fabs(z) > v.z;
  }
  inline Bool operator<(Vector v)
  {
    return (F32)fabs(x) < v.x && (F32)fabs(y) < v.y && (F32)fabs(z) < v.z;
  }

	inline F32 Magnitude2() const	// magnitude squared
	{
		return Dot(*this);
	}
	inline F32 MagnitudeXZ2() const
	{
		return x * x + z * z;
	}
	inline F32 Magnitude() const
	{ 
		return (F32) sqrt(Magnitude2()); 
	}
	inline F32 MagnitudeXZ() const
	{ 
		return (F32) sqrt(MagnitudeXZ2()); 
	}
	inline F32 Distance( const Vector &vv) const
	{
		F32 dx = x-vv.x, dy = y-vv.y, dz = z-vv.z;
		return (F32) sqrt(dx*dx+dy*dy+dz*dz);
	}
  inline void Convert(VectorDir &d) const
  {
    d.u = (F32) atan2(z, x);
    d.v = (F32) acos(y);
  }
  inline void Convert(VectorData &d) const
  {
    d.x = x;
    d.y = y;
    d.z = z;
  }
  friend ostream& operator<<(ostream &o, const Vector &v);

	F32  Normalize();
	F32  NormalizeXZ();
};

struct Vector4 : public Vector
{
  F32       w;

  Vector4 () {}
	Vector4 ( F32 _x, F32 _y, F32 _z) 
	{
    Set( _x, _y, _z);
	}
  Vector4 ( const Vector &v)
  {
    Set( v.x, v.y, v.z);
  }

  Vector4 ( F32 _x, F32 _y, F32 _z, F32 _w) 
	{
    Set( _x, _y, _z, _w);
	}
  Vector4 ( const Vector &v, F32 _w)
  {
    Set( v.x, v.y, v.z, _w);
  }

  void Set ( F32 _x, F32 _y, F32 _z) 
  {
    x = _x; y = _y;  z = _z;
  }
  void Set ( F32 _x, F32 _y, F32 _z, F32 _w) 
  {
    x = _x; y = _y; z = _z; w = _w;
  }

  inline void operator=( const Vector &v)
	{
    Set( v.x, v.y, v.z);
	}
};
//----------------------------------------------------------------------------

struct VectorNorm : public Vector
{
  // Initializing constructor
	VectorNorm(F32 _x, F32 _y, F32 _z) 
	{
    Set( _x, _y, _z);
	}
  VectorNorm( const Vector &v)
  {
    Set( v.x, v.y, v.z);
  }
  VectorNorm(const VectorData & data)
  {
    Set( data.x, data.y, data.z);
  }
	inline void Set(F32 xx, F32 yy, F32 zz)
	{
		x = xx; y = yy; z = zz;

    Normalize();
	}
};
//----------------------------------------------------------------------------

// used to represent and manipulate angular displacements
struct Quaternion
{
	F32			  s;			// scalar part
	Vector		v;			// vector part

  Quaternion() {}
  Quaternion( const Matrix &m)
  {
    Set( m);
  }
  Quaternion( F32 rads, const Vector &v) 
  {
    Set( rads, v);
  }

	// setup
	void ClearData()
	{ 
		s = 1.0f; v.x = v.y = v.z = 0.0f;
	}
	inline void Zero()
	{
		ClearData(); 
	}
	void Set(F32 rads, const Vector &v);
	inline void Set(F32 ss, F32 xx, F32 yy, F32 zz)
	{
		s = ss;  v.x = xx;  v.y = yy;  v.z = zz;
	}
	void Set( const Matrix &matrix);
	
	// Quaternion + operator
	void operator+=(const Quaternion &qq)
	{
		s += qq.s; 
    v += qq.v; 
	}
	Quaternion operator+(const Quaternion &qq) const 
	{
		Quaternion result = *this;
		result += qq;
		return result;
	}

  // Quaternion - operator
	void operator-=(const Quaternion &qq)
	{
		s -= qq.s; v -= qq.v;
	}
	Quaternion operator-(const Quaternion &qq) const
	{
		Quaternion result = *this;
		result -= qq;
		return result;
	}

  // Scalar * operator
	void operator*=(F32 ss)
	{
		s *= ss; 
    v *= ss;
	}
  Quaternion operator*(const F32 ss) const
  {
    Quaternion result = *this;
    result *= ss;
    return (result);
  }

	inline Bool operator==(const Quaternion &q) const
	{
		return (Bool)(s == q.s && v == q.v);
	}
	
	inline F32 Dot(const Quaternion &qq) const
	{
		return s * qq.s + v.x * qq.v.x + v.y * qq.v.y + v.z * qq.v.z;
	}
	
	Quaternion operator*(const Quaternion &qq) const;
	void operator*=(const Quaternion &qq);
	void Rotate(F32 rads, const Vector &vv);

  // SLERP
  Quaternion Interpolate(const Quaternion &q1, F32 time) const;
  Quaternion Slerp(const Quaternion &q1, F32 time) const;

  Vector GetRight();
  Vector GetUp();
  Vector GetFront();

  friend ostream& operator<<(ostream &o, const Quaternion &q);
};
//----------------------------------------------------------------------------

typedef struct 
{
   F32	x;
   F32	y; 
   F32	z;
   F32	w;
} QUADRUPLET;

typedef struct 
{
   QUADRUPLET right;
   QUADRUPLET up;
   QUADRUPLET front;
   QUADRUPLET posit;
} MATRIX_STRUCT;


struct Matrix
{
private:
	void Transform_Vectors (const F32 *Dst, const F32 *Src, S32 Stride, S32 Count) const;
	void Rotate_Vectors (const F32 *Dst, const F32 *Src, S32 Stride, S32 Count) const;
	void Unrotate_Vectors (const F32 *Dst, const F32 *Src, S32 Stride, S32 Count) const;
	void Transform_H_Vectors (const F32 *Dst, F32 &W, const F32 *Src) const;

public:
	union {
		struct {
			Vector right;
			F32    rightw;

			Vector up;
			F32    upw;

			Vector front;
			F32    frontw;

			Vector posit;
			F32    positw;
		};
		MATRIX_STRUCT Mat;
	};

  Matrix() {} 

	Matrix( const Matrix &Mat) 
  {
    memcpy( this, &Mat, sizeof (Matrix));
	}
	Matrix( const Vector &r, const Vector &u, const Vector &f, const Vector &p)
  {
    Set( r, u, f, p);
  }
	Matrix( const Quaternion &q) 
  {
    ClearData();
    Set( q);
	}
	Matrix( const Vector &pos) 
  {
    ClearData();
    Set( pos);
	}
	Matrix( const Quaternion &q, const Vector &pos) 
  {
    ClearData();
    Set( q);
    Set( pos);
	}

	// setup
	void ClearData();
  void Zero();
	void Set( const Vector &r, const Vector &u, const Vector &f);
	void Set( const Vector &r, const Vector &u, const Vector &f, const Vector &p);
	void Set( const Vector &position);
	void Set( const Quaternion &attitude);
  void SetScale( const Vector &scale);

  inline void Set( const Vector &position, const Quaternion &attitude)
	{
		Set(position);
    Set(attitude);
	}

  void Mirror( const Plane & plane);

  // 'this' must have a valid 'front' vector!!!
  //
	void SetFromUp( const Vector & _up);

  // 'this' must have a valid 'up' vector!!!
  //
  void SetFromFront( const Vector & _front);

  Vector Dot( const Vector & v);

	void SetInverse( const Quaternion &attitude);
	void SetInverse( const Matrix &matrix);
	void SetInverse();
  void SetProjection( F32 nearPlane, F32 farPlane, F32 fov, F32 aspectHW); 
  
	// query
	inline F32 &Value(U32 r, U32 c) const
	{
		return ((F32 *)&right.x)[r*4+c];
	}
  Vector &Right()     { return (right);	}
  Vector &Up()        { return (up); }
  Vector &Front()     { return (front); }
  Vector &Position()  { return (posit); }
  const Vector &Right()     const { return (right); }
  const Vector &Up()        const { return (up);    }
  const Vector &Front()     const { return (front); }
  const Vector &Position()  const { return (posit); }
  F32 Trace() const;

  // operate
	// Obsolete	method keep for complatibility
	inline void Transform(Matrix &result, const Matrix &m) const
	{
		result = *this * m;
	}
//	Matrix operator*( const Matrix &m) const
//	{
//		Matrix result;
//		Transform(result, m);
//		return result;
//	}

	friend Matrix operator*	(const Matrix &A, const Matrix &B); 

	void operator=(const Matrix &m)
	{
    Utils::Memcpy(this, &m, sizeof(Matrix));
	}
	Bool operator==(const Matrix &m) const
	{
    return 
         right.x == m.right.x && right.y == m.right.y && right.z == m.right.z
      && up.x == m.up.x && up.y == m.up.y && up.z == m.up.z
      && front.x == m.front.x && front.y == m.front.y && front.z == m.front.z
      && posit.x == m.posit.x && posit.y == m.posit.y && posit.z == m.posit.z;
	}
	inline Bool operator!=(const Matrix &m) const
	{
    return !(*this == m);
  }

	void operator+=(const Matrix &m);


	// Vectors Transforms

	inline void Transform(Vector *dst, const Vector *src, U32 count) const
	{
	   Transform_Vectors ((F32*)dst, (F32*)src, sizeof (Vector), count);
	}

	inline void Transform(Vector *dst, Vector *src, U32 count) const
	{
	   Transform_Vectors ((F32*)dst, (F32*)src, sizeof (Vector), count);
	}

	inline void Transform(Vector &dst, const Vector &src) const
	{
	   Transform_Vectors ((F32*)&dst, (F32*)&src, sizeof (Vector), 1);
	}

	inline void Transform(Vector &dst, Vector &src) const
	{
	   Transform_Vectors ((F32*)&dst, (F32*)&src, sizeof (Vector), 1);
	}

	inline void Transform(Vector &dst) const
	{
	   Transform_Vectors ((F32*)&dst, (F32*)&dst, sizeof (Vector), 1);
	}

	inline void Transform(Vector &dst, F32 &w, const Vector &src) const
	{
		Transform_H_Vectors ((F32*)&dst, w, (F32*)&src);
	}

	inline void Transform(Vector &dst, F32 &w, Vector &src) const
	{
		Transform_H_Vectors ((F32*)&dst, w, (F32*)&src);
	}

	inline void Rotate (Vector *dst, const Vector *src, U32 count) const
	{
		Rotate_Vectors ((F32*)dst, (F32*)src, sizeof (Vector), count);
	}

	inline void Rotate(Vector *dst, Vector *src, U32 count) const
	{
		Rotate_Vectors ((F32*)dst, (F32*)src, sizeof (Vector), count);
	}

	inline void Rotate(Vector &dst, const Vector &src) const
	{
		Rotate_Vectors ((F32*)&dst, (F32*)&src, sizeof (Vector), 1);
	}

	inline void Rotate(Vector &dst, Vector &src) const
	{
		Rotate_Vectors ((F32*)&dst, (F32*)&src, sizeof (Vector), 1);
	}

	inline void Rotate( Vector &dst) const
	{
		Rotate_Vectors ((F32*)&dst, (F32*)&dst, sizeof (Vector), 1);
	}


	inline void RotateInv( Vector *dst, const Vector *src, U32 count) const
	{
		Unrotate_Vectors ((F32*)dst, (F32*)src, sizeof (Vector), count);
	}

	inline void RotateInv( Vector *dst, Vector *src, U32 count) const
	{
		Unrotate_Vectors ((F32*)dst, (F32*)src, sizeof (Vector), count);
	}

	inline void RotateInv(Vector &dst, const Vector &src) const
	{
		Unrotate_Vectors ((F32*)&dst, (F32*)&src, sizeof (Vector), 1);
	}

	inline void RotateInv(Vector &dst, Vector &src) const
	{
		Unrotate_Vectors ((F32*)&dst, (F32*)&src, sizeof (Vector), 1);
	}

	inline void RotateInv( Vector &dst) const
	{
		Unrotate_Vectors ((F32*)&dst, (F32*)&dst, sizeof (Vector), 1);
	}

	inline void Rotate( UVPair &dst, UVPair &src) const
	{
		dst.u = src.u * right.x + src.v * front.x;
		dst.v = src.u * right.z + src.v * front.z;
	}

	inline void Rotate( UVPair &dst) const
	{
		UVPair tmp = dst;
		Rotate( dst, tmp);
	}

	void UnRotate( UVPair &dst, UVPair &src) const;
  inline void UnRotate( UVPair &dst) const
	{
		UVPair tmp = dst;
		UnRotate( dst, tmp);
	}

	// Vertex Transforms
	inline void Transform(Vertex *dst, const Vertex *src, U32 count) const;
	inline void Transform(Vertex *dst, Vertex *src, U32 count) const;
	inline void Transform(VertexL *dst, const VertexL *src, U32 count) const;
	inline void Transform(VertexTL *dst, const VertexTL *src, U32 count) const;
	inline void Transform(VertexTL *dst, VertexTL *src, U32 count) const;
	inline void Transform(Vertex &dst, const Vertex &src) const;
	inline void Transform(VertexL &dst, const VertexL &src) const;
	inline void Transform(VertexL &dst, VertexL &src) const;
	inline void Transform(Vertex &dst, Vertex &src) const;
	inline void Transform(VertexTL &dst, VertexTL &src) const;
	inline void Transform(VertexTL &dst, const VertexTL &src) const;
	inline void Transform(VertexL *dst, VertexL *src, U32 count) const;

	// vertex Rotate in vertex.h
	inline void Rotate(Vertex &dst, const Vertex &src) const;
	inline void Rotate(Vertex &dst, Vertex &src) const;
	inline void Rotate(Vertex *dst, const Vertex *src, U32 count) const;
	inline void Rotate(Vertex *dst, Vertex *src, U32 count) const;
	inline void Rotate(VertexL &dst, const VertexL &src) const;
	inline void Rotate(VertexL &dst, VertexL &src) const;
	inline void Rotate(VertexL *dst, const VertexL *src, U32 count) const;
	inline void Rotate(VertexL *dst, VertexL *src, U32 count) const;
	inline void Rotate(VertexTL &dst, const VertexTL &src) const;
	inline void Rotate(VertexTL &dst, VertexTL &src) const;
	inline void Rotate(VertexTL *dst, const VertexTL *src, U32 count) const;
	inline void Rotate(VertexTL *dst, VertexTL *src, U32 count) const;

	// vertex RotateInv in vertex.h
	inline void RotateInv(Vertex &dst, const Vertex &src) const;
	inline void RotateInv(Vertex &dst, Vertex &src) const;
	inline void RotateInv(Vertex *dst, const Vertex *src, U32 count) const;
	inline void RotateInv(Vertex *dst, Vertex *src, U32 count) const;
	inline void RotateInv(VertexL &dst, const VertexL &src) const;
	inline void RotateInv(VertexL &dst, VertexL &src) const;
	inline void RotateInv(VertexL *dst, const VertexL *src, U32 count) const;
	inline void RotateInv(VertexL *dst, VertexL *src, U32 count) const;
	inline void RotateInv(VertexTL &dst, const VertexTL &src) const;
	inline void RotateInv(VertexTL &dst, VertexTL &src) const;
	inline void RotateInv(VertexTL *dst, const VertexTL *src, U32 count) const;
	inline void RotateInv(VertexTL *dst, VertexTL *src, U32 count) const;


	Vector Vector_Rotate (const Vector &v) const; 
	Vector Vector_RotateInv (const Vector &v) const; 
	Vector Vector_Transform (const Vector &v) const; 
	Vector Vector_TransformInv(const Vector &v) const;

  friend ostream& operator<<(ostream &o, const Matrix &m);

  // Identity matrix
  static const Matrix I;
};
//----------------------------------------------------------------------------

struct Plane : public Vector
{
	F32     d;

  Plane() {}
  Plane( const Vector & v0,  const Vector & v1, const Vector & v2)
  {
    Set( v0, v1, v2);
  }

	void ClearData();
	void Set(const Vector & vv);
	void Set(const Vector & v0, const Vector & v1, const Vector & v2);
	
	F32 Evalue(const Vector &vv) const;

	F32  Normalize();
  void operator*=(F32 ss);
};
//----------------------------------------------------------------------------
class Camera;
struct Bounds;

struct Sphere: public Matrix
{
	F32	Width;
	F32	Height;
	F32	Breadth;

	F32 radius;
	F32 radius2;    // radius squared

  typedef U32 (Sphere::*SpherePlaneTestProc)( const Matrix & camOrigin, const Plane * planes) const;
  SpherePlaneTestProc testFuncs[6];

  typedef U32 (Sphere::*SphereFrustTestProc)( const Matrix & camOrigin, const Plane * planes);
  SphereFrustTestProc frustrumFunc;

private:
  void Eigen_Vectors (Vector &Eigen_Value);

  Vector Calculate_Principal_Axis( Vector &Scale, 
    Vector * VertexList, U32 VertexCount, U16 * IndexList, U32 IndexCount);

  // in indexing order for static lists
  //
  U32 TestSphereNear(   const Matrix & camOrigin, const Plane * planes) const;  // 0
  U32 TestSphereFar(    const Matrix & camOrigin, const Plane * planes) const;  // 1
  U32 TestSphereLeft(   const Matrix & camOrigin, const Plane * planes) const;  // 2
  U32 TestSphereRight(  const Matrix & camOrigin, const Plane * planes) const;  // 3
  U32 TestSphereTop(    const Matrix & camOrigin, const Plane * planes) const;  // 4
  U32 TestSphereBottom( const Matrix & camOrigin, const Plane * planes) const;  // 5

  U32 TestBoxNear(   const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxFar(    const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxRight(  const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxLeft(   const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxTop(    const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxBottom( const Matrix & camOrigin, const Plane * planes) const;

public:
	Sphere ()
	{
		ClearData();
	}
  Sphere( const Vector &v, F32 r)
  {
    ClearData();

    SetOrigin( v);
    SetRadius( r);
  }

	void ClearData()
	{
		Matrix::ClearData();
		Width = 0.0;
		Height = 0.0;
		Breadth = 0.0;
		radius  = 0.0f;
		radius2 = 0.0f;
//    SetSphereTest();
	}

  void SetRadius( F32 r)
  {
    radius  = r;
    radius2 = r * r;
  }
  void SetOrigin( const Vector &v)
  {
    posit = v;
  }
  const Vector &Origin() const
  {
    return posit;
  }
  F32 Radius() const
  {
    return radius;
  }
  F32 Radius2() const
  {
    return radius2;
  }
  inline void Set( const Bounds &bounds);

  void Calculate_Dimensions (Vector *VertexList, U32 VertexCount, 
							  U16 *IndexList, U32 IndexCount,
							  const Matrix *Basis = NULL);

  void Combine( const Sphere & s1);

#if 0
  void SetSphereTest();
  void SetBoxTest();
  U32 FrustrumSphereTest( const Matrix & camOrigin, const Plane * planes);
  U32 FrustrumBoxTest(    const Matrix & camOrigin, const Plane * planes);

  U32 FrustrumTest(       const Matrix & camOrigin, const Plane * planes) const
  {
    return (this->*frustrumFunc)( camOrigin, planes);
  }
  void RenderBox(    Color color, Bitmap * texture = NULL) const;
  void RenderSphere( Color color, Bitmap * texture = NULL) const;
  void Render(       Color color = 0xff008800, Bitmap * texture = NULL) const
  {
    RenderBox( color, texture);
  }
#endif

};

struct Bounds
{
protected:
  friend class MeshEnt;

	F32	width;
	F32	height;
	F32	breadth;

	F32 radius;

  Vector offset;

  typedef U32 (Bounds::*BoundsPlaneTestProc)( const Matrix & camOrigin, const Plane * planes) const;
  BoundsPlaneTestProc testFuncs[6];

  typedef U32 (Bounds::*BoundsFrustTestProc)( const Matrix & camOrigin, const Plane * planes);
  BoundsFrustTestProc frustrumFunc;

private:

  F32 zdepth;   // cached depth

  // in indexing order for static lists
  //
  U32 TestSphereNear(   const Matrix & camOrigin, const Plane * planes) const;  // 0
  U32 TestSphereFar(    const Matrix & camOrigin, const Plane * planes) const;  // 1
  U32 TestSphereLeft(   const Matrix & camOrigin, const Plane * planes) const;  // 2
  U32 TestSphereRight(  const Matrix & camOrigin, const Plane * planes) const;  // 3
  U32 TestSphereTop(    const Matrix & camOrigin, const Plane * planes) const;  // 4
  U32 TestSphereBottom( const Matrix & camOrigin, const Plane * planes) const;  // 5

  U32 TestBoxNear(   const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxFar(    const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxRight(  const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxLeft(   const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxTop(    const Matrix & camOrigin, const Plane * planes) const;
  U32 TestBoxBottom( const Matrix & camOrigin, const Plane * planes) const;

public:
	Bounds ()
	{
		ClearData();
	}
  Bounds( const Vector &o, F32 r)
  {
    ClearData();

    SetOffset( o);
    SetRadius( r);
  }
  Bounds( const Bounds & bounds)
  {
    *this = bounds;
  }

	void ClearData()
	{
    offset = 0;

		width = 0.0;
		height = 0.0;
		breadth = 0.0;

		radius  = 0.0f;

    SetSphereTest();
	}

  void SetSphereTest();
  void SetBoxTest();

  void SetRadius( F32 r)
  {
    radius  = r;
  }
  void SetOffset( const Vector & o)
  {
    offset = o;
  }
  void Set( F32 w, F32 h, F32 b)
  {
    width = w;
    height = h;
    breadth = b;
  }

  inline void Set( const Sphere & sphere)
  {
    width     = sphere.Width;
    height    = sphere.Height;
    breadth   = sphere.Breadth;
    SetRadius( sphere.radius);
    SetOffset( sphere.posit);
  }

  void Combine( const Bounds & b1);

  inline const Vector & Offset() const
  {
    return offset;
  }
  inline F32 Radius() const
  {
    return radius;
  }
  inline F32 Radius2() const
  {
    return radius * radius;
  }
  inline F32 Width() const
  {
    return width;
  }
  inline F32 Height() const
  {
    return height;
  }
  inline F32 Breadth() const
  {
    return breadth;
  }
  inline F32 * WidthHeightBreadth() const
  {
    return (F32 *) &width;
  }
  U32 FrustrumSphereTest( const Matrix & camOrigin, const Plane * planes);
  U32 FrustrumBoxTest(    const Matrix & camOrigin, const Plane * planes);
/*
  U32 FrustrumTest( const Matrix & camOrigin, const Plane * planes) const
  {
    return (this->*frustrumFunc)( camOrigin, planes);
  }
*/
  void RenderBox(     const Matrix & world, Color color, Bitmap * texture = NULL) const;
  void RenderSphere(  const Matrix & world, Color color, Bitmap * texture = NULL) const;
  void Render(        const Matrix & world, Color color = 0xff008800, Bitmap * texture = NULL) const
  {
    RenderBox( world, color, texture);
  }
};

//
// Bounding Box
//
struct BoundingBox
{
  Vector min;
  Vector max;

  // Calculate box from a list of vectors
  void Calculate(Vector *v, U32 count);
};

inline void Sphere::Set( const Bounds &bounds)
{
  Width   = bounds.Width();
  Height  = bounds.Height();
  Breadth = bounds.Breadth();
  SetRadius( bounds.Radius());
  *(Matrix *)this = Matrix::I;  
  SetOrigin( bounds.Offset());
}

//----------------------------------------------------------------------------

#endif		// GAMEMATHH_DEF
