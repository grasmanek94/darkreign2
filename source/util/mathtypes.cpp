///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// gamemath.cpp
//
// 01-APR-1998        John Cooke
//

//#define NO_ASM_MATH 

#include "mathtypes.h"
#include "perfstats.h"
//----------------------------------------------------------------------------

const Matrix Matrix::I
( 
  Vector( 1.0f, 0.0f, 0.0f), 
  Vector( 0.0f, 1.0f, 0.0f), 
  Vector( 0.0f, 0.0f, 1.0f), 
  Vector( 0.0f, 0.0f, 0.0f) 
);

//----------------------------------------------------------------------------

F32 Vector::Normalize()
{
	F32 r = Magnitude();
  // careful of bogus values
  if (r == 0)
  {
    r = F32_EPSILON;
  }

	F32 ri = 0.9999999f / r;
	x *= ri;
	y *= ri;
	z *= ri;

  return r;
}
//----------------------------------------------------------------------------

F32 Vector::NormalizeXZ()
{
  y = 0.0f;

  F32 r = MagnitudeXZ();
  // careful of bogus values
  if (r == 0)
  {
    r = F32_EPSILON;
  }

	F32 ri = 0.9999999f / r;
	x *= ri;
	z *= ri;

  return r;
}

ostream& operator<<(ostream &o, const Vector &v)
{
  return (o << '[' << v.x << ',' << v.y << ',' << v.z << ']');
}

//----------------------------------------------------------------------------

Quaternion Quaternion::operator*(const Quaternion &qq) const
{
	Quaternion result;
	
	result.s = s * qq.s - v.Dot(qq.v);
	result.v = v * qq.s + v.Cross(qq.v) + qq.v * s;
	
	return result;
}
//----------------------------------------------------------------------------

void Quaternion::operator*=(const Quaternion &qq)
{
	F32 st = s * qq.s - v.Dot(qq.v);
	v = qq.v * s + v * qq.s + v.Cross(qq.v);
	s = st;
}
//----------------------------------------------------------------------------

void Quaternion::Set(F32 rads, const Vector &vv)
{
	rads *= 0.5f;
	
	s =  (F32) cos(rads);
	v =  vv;
	v *= (F32) sin(rads);
}
//----------------------------------------------------------------------------

void Quaternion::Rotate(F32 rads, const Vector &vv) 
{
	rads *= 0.5f;
	
	F32 st = (F32) cos(rads);
	Vector vt = vv;
	vt *= (F32) sin(rads);
	
	F32 ns = s * st - v.Dot(vt);
	v = vt * s + v * st + v.Cross(vt);
	s = ns;
}
//----------------------------------------------------------------------------

void Quaternion::Set( const Matrix &matrix)
{
	F32 w = matrix.Trace();
	if (w > 0.0f)
	{
		w = (F32) sqrt(w + 1.0f);
		s = w * 0.5f;
		
		w = 0.5f / w;
		v.x = (matrix.front.y - matrix.up.z) * w;
		v.y = (matrix.right.z - matrix.front.x) * w;
		v.z = (matrix.up.x - matrix.right.y) * w;
	}
	else
	{
		static U32 next[] = { 1, 2, 0 };
		U32 i = 0;
		if (matrix.up.y > matrix.right.x)
		{
			i = 1;
		}
		if (matrix.front.z > matrix.Value(i, i))
		{
			i = 2;
		}
		U32 j = next[i];
		U32 k = next[j];
		w = (F32) sqrt((matrix.Value(i, i) - (matrix.Value(j, j) + matrix.Value(k, k))) + 1.0f);
		F32 *qi = (F32 *) &v + i;
		F32 *qj = (F32 *) &v + j;
		F32 *qk = (F32 *) &v + k;
		*qi = w * 0.5f;
		w   = 0.5f / w;
		s   = (matrix.Value(k, j) - matrix.Value(j, k)) * w;
		*qj = (matrix.Value(j, i) + matrix.Value(i, j)) * w;
		*qk = (matrix.Value(k, i) + matrix.Value(i, k)) * w;
	}
}
//----------------------------------------------------------------------------

Vector Quaternion::GetRight()
{
	F32 d =  (v.x * v.x + v.y * v.y + v.z * v.z + s * s);
  ASSERT( d != 0);
	F32 n =  2.0f / d;
	F32 yn = v.y * n;	F32 zn = v.z * n;
	F32 sy = s   * yn;	F32 sz = s   * zn;
	F32 xy = v.x * yn;	F32 xz = v.x * zn;
	F32 yy = v.y * yn;	
  F32 zz = v.z * zn;

	return Vector( 1.0f - (yy + zz), xy - sz, xz + sy);
}
//----------------------------------------------------------------------------

Vector Quaternion::GetUp()
{
	F32 d =  (v.x * v.x + v.y * v.y + v.z * v.z + s * s);
  ASSERT( d != 0);
	F32 n =  2.0f / d;
	F32 xn = v.x * n;	F32 yn = v.y * n;	F32 zn = v.z * n;
	F32 sx = s   * xn;	F32 sz = s   * zn;
	F32 xx = v.x * xn;	F32 xy = v.x * yn;
	F32 yz = v.y * zn;	F32 zz = v.z * zn;
	
	return Vector( xy + sz, 1.0f - (xx + zz), yz - sx);
}
//----------------------------------------------------------------------------

Vector Quaternion::GetFront()
{
	F32 d =  (v.x * v.x + v.y * v.y + v.z * v.z + s * s);
  ASSERT( d != 0);
	F32 n =  2.0f / d;
	F32 xn = v.x * n;	F32 yn = v.y * n;	F32 zn = v.z * n;
	F32 sx = s   * xn;	F32 sy = s   * yn;
	F32 xx = v.x * xn;	F32 xz = v.x * zn;
	F32 yy = v.y * yn;	F32 yz = v.y * zn;
	
  return Vector( xz - sy, yz + sx, 1.0f - (xx + yy));
}


ostream& operator<<(ostream &o, const Quaternion &q)
{
  return (o << 's' << q.s << 'v' << q.v);
}


//----------------------------------------------------------------------------

//
// SLERP
//
Quaternion Quaternion::Interpolate(const Quaternion &q1, F32 t) const
{
	Quaternion q(*this);

	// calculate cosine
	F64 dot = 
    q.s   * q1.s   +
		q.v.x * q1.v.x +
		q.v.y * q1.v.y +
		q.v.z * q1.v.z;

	// adjust signs (if necessary)
	if (dot < 0.0)
	{
		dot = -dot;
		q.s   = -q1.s;
		q.v.x = -q1.v.x;
		q.v.y = -q1.v.y;
		q.v.z = -q1.v.z;
	}
	else
	{
		q.s   = q1.s;
		q.v.x = q1.v.x;
		q.v.y = q1.v.y;
		q.v.z = q1.v.z;
	}

	// calculate coefficients
	F64 scale0, scale1;

	if (1.0 - dot > 1.0e-4F) 
	{
		// spherical linear interpolation
		F64 omega = acos(dot);
		F64 scale = 1.0 / sin(omega);
		scale0 = sin((1.0 - t) * omega) * scale;
		scale1 = sin(t * omega) * scale;
	}
	else
	{
		// linear interpolation
		scale0 = 1.0 - t;
		scale1 = t;
	}

	// calculate final values
	q.s   = (F32)(scale0 * s   + scale1 * q.s  );
	q.v.x = (F32)(scale0 * v.x + scale1 * q.v.x);
	q.v.y = (F32)(scale0 * v.y + scale1 * q.v.y);
	q.v.z = (F32)(scale0 * v.z + scale1 * q.v.z);

	// return the quaternion
	return (q);
}
//----------------------------------------------------------------------------

Quaternion Quaternion::Slerp( const Quaternion &q1, F32 t) const
{
	// correct quaternion for interp
  //
  F32 dot = this->Dot( q1);

  return *this + ( (dot < 0 ? q1 * -1.0f : q1) - *this) * t;
}
//----------------------------------------------------------------------------

//		dst.u = src.u * right.x + src.v * front.x;
//		dst.v = src.u * right.z + src.v * front.z;

void Matrix::UnRotate( UVPair & dst, UVPair & src) const
{
	dst.u = src.u * right.x + src.v * right.z;
	dst.v = src.u * front.x + src.v * front.z;
}
//----------------------------------------------------------------------------

void Matrix::Set( const Vector &r, const Vector &u, const Vector &f)
{
	right.x = r.x ; right.y = r.y ; right.z = r.z ; rightw  = 0.0f;
	up.x    = u.x ; up.y    = u.y ; up.z    = u.z ; upw     = 0.0f;
	front.x = f.x ; front.y = f.y ; front.z = f.z ; frontw  = 0.0f;
	posit.x = 0.0f; posit.y = 0.0f; posit.z = 0.0f; positw  = 1.0f;
}
//----------------------------------------------------------------------------

void Matrix::Set( const Vector &r, const Vector &u, const Vector &f, const Vector &p)
{
	right.x = r.x ; right.y = r.y ; right.z = r.z ; rightw  = 0.0f;
	up.x    = u.x ; up.y    = u.y ; up.z    = u.z ; upw     = 0.0f;
	front.x = f.x ; front.y = f.y ; front.z = f.z ; frontw  = 0.0f;
	posit.x = p.x ; posit.y = p.y ; posit.z = p.z ; positw  = 1.0f;
}
//----------------------------------------------------------------------------

void Matrix::ClearData()
{
	right.x = 1.0f; right.y = 0.0f; right.z = 0.0f; rightw  = 0.0f;
	up.x    = 0.0f; up.y    = 1.0f; up.z    = 0.0f; upw     = 0.0f;
	front.x = 0.0f; front.y = 0.0f; front.z = 1.0f; frontw  = 0.0f;
	posit.x = 0.0f; posit.y = 0.0f; posit.z = 0.0f; positw  = 1.0f;
}
//----------------------------------------------------------------------------

void Matrix::Zero()
{
  Utils::Memset( this, 0, sizeof( Matrix));
}
//----------------------------------------------------------------------------

void Matrix::operator+=(const Matrix &m)
{
  right  += m.right;
  up     += m.up;
  front  += m.front;
  posit  += m.posit;
}
//----------------------------------------------------------------------------

void Matrix::Mirror( const Plane & plane) 
{
#if 1
  right.y *= -1;
  up.y    *= -1;
  front.y *= -1;
  posit.y = -2 * plane.d - posit.y;
#else
	F32 nxx = 1.0f - 2.0f * plane.x * plane.x;
	F32 nyy = 1.0f - 2.0f * plane.y * plane.y;
	F32 nzz = 1.0f - 2.0f * plane.z * plane.z;

	F32 nxy = - 2.0f * plane.x * plane.y;
	F32 nyz = - 2.0f * plane.y * plane.z;
	F32 nzx = - 2.0f * plane.z * plane.x;

	F32 ndx = - 2.0f * plane.x * plane.d;
	F32 ndy = - 2.0f * plane.y * plane.d;
	F32 ndz = - 2.0f * plane.z * plane.d;

	*this = *this * Matrix(
    Vector(nxx, nxy, nzx),
    Vector(nxy, nyy, nyz),
    Vector(nzx, nyz, nzz),
    Vector(ndx, ndy, ndz));
#endif
}
//----------------------------------------------------------------------------

void Matrix::SetProjection(F32 nearPlane, F32 farPlane, F32 fov, F32 aspectHW) 
{
	// fov is expected radians
	fov *= 0.5f;
	
	Zero();

	F32 c = (F32) cos(fov);
	F32 s = (F32) sin(fov);
	F32 q = farPlane / (farPlane - nearPlane);
	
	right.x = up.y = c / s;
  right.x *= aspectHW;
	front.z = q;
	posit.z = -q * nearPlane;
	frontw = 1.0f;
}
//----------------------------------------------------------------------------

void Matrix::Set( const Vector &position)
{
	posit = position;
	positw = 1.0f;
}
//----------------------------------------------------------------------------

void Matrix::SetScale( const Vector &scale)
{
  right.x *= scale.x;
  up.x    *= scale.x;
  front.x *= scale.x;

  right.y *= scale.y;
  up.y    *= scale.y;
  front.y *= scale.y;

  right.z *= scale.z;
  up.z    *= scale.z;
  front.z *= scale.z;
}
//----------------------------------------------------------------------------

void Matrix::Set( const Quaternion &qq)
{
	F32 denom = qq.v.x * qq.v.x + qq.v.y * qq.v.y + qq.v.z * qq.v.z + qq.s * qq.s;
  ASSERT( denom > 0);

	F32 n =  2.0f / denom;
	F32 xn = qq.v.x * n;	F32 yn = qq.v.y * n;	F32 zn = qq.v.z * n;
	F32 sx = qq.s   * xn;	F32 sy = qq.s   * yn;	F32 sz = qq.s   * zn;
	F32 xx = qq.v.x * xn;	F32 xy = qq.v.x * yn;	F32 xz = qq.v.x * zn;
	F32 yy = qq.v.y * yn;	F32 yz = qq.v.y * zn;	F32 zz = qq.v.z * zn;
	
	right.x = 1.0f - (yy + zz);
	right.y = xy - sz;
	right.z = xz + sy;
	
	up.x = xy + sz;
	up.y = 1.0f - (xx + zz);
	up.z = yz - sx;
	
	front.x = xz - sy;
	front.y = yz + sx;
	front.z = 1.0f - (xx + yy);
}

//----------------------------------------------------------------------------

void Matrix::SetInverse( const Quaternion &qq)
{
	F32 denom = qq.v.x * qq.v.x + qq.v.y * qq.v.y + qq.v.z * qq.v.z + qq.s * qq.s;
  ASSERT( denom > 0);

	F32 n =  2.0f / denom;

	F32 xn = qq.v.x * n;	F32 yn = qq.v.y * n;	F32 zn = qq.v.z * n;
	F32 sx = qq.s   * xn;	F32 sy = qq.s   * yn;	F32 sz = qq.s   * zn;
	F32 xx = qq.v.x * xn;	F32 xy = qq.v.x * yn;	F32 xz = qq.v.x * zn;
	F32 yy = qq.v.y * yn;	F32 yz = qq.v.y * zn;	F32 zz = qq.v.z * zn;
	
	right.x = 1.0f - (yy + zz);
	right.y = xy + sz;
	right.z = xz - sy;
	
	up.x = xy - sz;
	up.y = 1.0f - (xx + zz);
	up.z = yz + sx;
	
	front.x = xz + sy;
	front.y = yz - sx;
	front.z = 1.0f - (xx + yy);
}
//----------------------------------------------------------------------------

void Matrix::SetInverse( const Matrix &m)
{
	right.x = m.right.x; 
	right.y = m.up.x;
	right.z = m.front.x;
	rightw  = m.rightw;
	up.x    = m.right.y;
	up.y    = m.up.y;
	up.z    = m.front.y;
	upw     = m.upw;
	front.x = m.right.z;
	front.y = m.up.z;
	front.z = m.front.z;
	frontw  = m.frontw;
	
	posit.x = -m.posit.Dot( m.right);
	posit.y = -m.posit.Dot(m.up);
	posit.z = -m.posit.Dot(m.front);
	positw  =  m.positw;
}
//----------------------------------------------------------------------------

void Matrix::SetInverse()
{
	Vector r = right, u = up, f = front, p = posit;
	F32 rw = rightw, uw = upw, fw = frontw;
	
	right.y = u.x;
	right.z = f.x;
	rightw = p.x;

	up.x = r.y;
	up.z = f.y;
	upw = p.z;

	front.x = r.z;
	front.y = u.z;
	frontw = p.z;

	posit.x = rw;
	posit.y = uw;
	posit.z = fw;
}
//----------------------------------------------------------------------------

// 'this' must have a valid 'front' vector!!!
//
void Matrix::SetFromUp(const Vector & _up)
{

  up = _up;
  right = Cross(up, front);
  right.Normalize();
  front = Cross(right, up);
}
//----------------------------------------------------------------------------

// 'this' must have a valid 'up' vector!!!
//
void Matrix::SetFromFront(const Vector & _front)
{
  front = _front;
  right = Cross( up, front);
  right.Normalize();
  up = Cross(front, right);
}
//----------------------------------------------------------------------------

F32 Matrix::Trace() const
{
	return right.x + up.y + front.z;
}
//----------------------------------------------------------------------------

Vector Matrix::Dot( const Vector & v)
{
	return Vector( 
    v.x * right.x + v.y * up.x + v.z * front.x,
    v.x * right.y + v.y * up.y + v.z * front.y,
    v.x * right.z + v.y * up.z + v.z * front.z);
}
//----------------------------------------------------------------------------

void Plane::ClearData()
{
	Vector::ClearData();
	d = 0.0f;
}
//----------------------------------------------------------------------------	

void Plane::Set(const Vector &vv)
{
	Vector::Set(vv.x, vv.y, vv.z);
	d = 0.0f;
}
//----------------------------------------------------------------------------

void Plane::Set(const Vector &v0, const Vector &v1, const Vector &v2)
{
  *((Vector *)this) = v1 - v0;
  *((Vector *)this) = Cross( v2 - v0);
	d = -Dot(v0);
}
//----------------------------------------------------------------------------
  
F32 Plane::Normalize()
{
	F32 r = Magnitude();
  // careful of bogus values
  if (r == 0)
  {
    r = F32_EPSILON;
  }

	F32 ri = 0.9999999f / r;
	x *= ri;
	y *= ri;
	z *= ri;
	d *= ri;

  return r;
}
//----------------------------------------------------------------------------
  
F32 Plane::Evalue(const Vector &vv) const
{
	return x * vv.x + y * vv.y + z * vv.z + d;
}
//----------------------------------------------------------------------------

void Plane::operator*=(F32 ss)
{
	x *= ss;
  y *= ss;
  z *= ss;
  d *= ss;
}
//----------------------------------------------------------------------------

void Bounds::SetSphereTest()
{
  testFuncs[0] = Bounds::TestSphereNear;
  testFuncs[1] = Bounds::TestSphereFar;
  testFuncs[2] = Bounds::TestSphereLeft;
  testFuncs[3] = Bounds::TestSphereRight;
  testFuncs[4] = Bounds::TestSphereTop;
  testFuncs[5] = Bounds::TestSphereBottom;

  frustrumFunc = Bounds::FrustrumSphereTest;
}
//----------------------------------------------------------------------------

void Bounds::SetBoxTest()
{
  testFuncs[0] = Bounds::TestBoxNear;
  testFuncs[1] = Bounds::TestBoxFar;
  testFuncs[2] = Bounds::TestBoxLeft;
  testFuncs[3] = Bounds::TestBoxRight;
  testFuncs[4] = Bounds::TestBoxTop;
  testFuncs[5] = Bounds::TestBoxBottom;

  frustrumFunc = Bounds::FrustrumBoxTest;
}
//----------------------------------------------------------------------------

#define SPHERE_EPSILON	1e-5f

static inline void ROT(
	Matrix &a,
	S32 i,
	S32 j,
	S32 k,
	S32 l, 
	F64 s,
	F64 tau) 
{
	F64 g;
	F64 h;
	g = a.Value(i, j); 
	h = a.Value (k, l); 
	a.Value(i, j) = (F32)(g - s * (h + g * tau)); 
	a.Value(k, l) = (F32)(h + s * (g - h * tau));
}
//----------------------------------------------------------------------------

#if 0
void Sphere::SetSphereTest()
{
  testFuncs[0] = Sphere::TestSphereNear;
  testFuncs[1] = Sphere::TestSphereFar;
  testFuncs[2] = Sphere::TestSphereLeft;
  testFuncs[3] = Sphere::TestSphereRight;
  testFuncs[4] = Sphere::TestSphereTop;
  testFuncs[5] = Sphere::TestSphereBottom;

  frustrumFunc = Sphere::FrustrumSphereTest;
}
//----------------------------------------------------------------------------

void Sphere::SetBoxTest()
{
  testFuncs[0] = Sphere::TestBoxNear;
  testFuncs[1] = Sphere::TestBoxFar;
  testFuncs[2] = Sphere::TestBoxLeft;
  testFuncs[3] = Sphere::TestBoxRight;
  testFuncs[4] = Sphere::TestBoxTop;
  testFuncs[5] = Sphere::TestBoxBottom;

  frustrumFunc = Sphere::FrustrumBoxTest;
}
//----------------------------------------------------------------------------
#endif

void Sphere::Calculate_Dimensions (
	Vector *VertexList, 
	U32 VertexCount, 
	U16 *IndexList, 
	U32 IndexCount,
	const Matrix *Basis)  // = NULL
{
	U32 i;
	U32 j;
	Vector Eigen_Value;
	Vector Scale;
	F32 *Ptr;
	F32 *Ptr1;
	F64 Tol;
	F64 Err;

	if (VertexCount < 3)
  {
		right = Vector(1.0, 0.0, 0.0);
		up = Vector(0.0, 1.0, 0.0);
		front = Vector(0.0, 0.0, 1.0);
		posit = Vector(0.0, 0.0, 0.0);

		Width = 0.0f; 
		Height = 0.0f; 
		Breadth = 0.0f; 
		radius2 = 1.0e-10f;
		radius = 1.0e-5f;
		return;
	}

	ASSERT (IndexCount > 0);

	if (!Basis)
  {
		Scale.x = 1.0f;
		Scale.y = 1.0f;
		Scale.z = 1.0f;
	 	Eigen_Value = Calculate_Principal_Axis (Scale, VertexList, VertexCount, IndexList, IndexCount);

		// if the object has symmetries the remove the symmetries
		Tol = Eigen_Value.x;
		if (Tol < Eigen_Value.y)
    {
			Tol = Eigen_Value.y;
		}
		if (Tol < Eigen_Value.z)
    {
			Tol = Eigen_Value.z;
		}

		Tol *= 0.05f;
		Ptr = (F32*)&Eigen_Value;
		Ptr1 = (F32*)&Scale;
		for (i = 0; i < 3; i ++)
    {
			for (j = i + 1; j < 3; j ++)
      {
				Err = fabs (Ptr[i] - Ptr[j]);
				if (Err < Tol)
        {
					Ptr1[i] *= 0.5f;
					Eigen_Value = Calculate_Principal_Axis (Scale, VertexList, VertexCount, IndexList, IndexCount);
		
					Tol = Eigen_Value.x;
					if (Tol < Eigen_Value.y)
          {
            Tol = Eigen_Value.y;
          }
					if (Tol < Eigen_Value.z)
          {
            Tol = Eigen_Value.z;
          }
					Tol *= 0.01f;
				}
			}
		}
	}
  else
  {
		right = Basis->right;
		up = Basis->up;
		front = Basis->front;
	}

	Vector Tmp(0, 0, 0);
  Vector Min( 1.0e10f,  1.0e10f,  1.0e10f); 
  Vector Max(-1.0e10f, -1.0e10f, -1.0e10f); 

	for (i = 0; i < VertexCount; i ++)
  {
		Tmp.x = right.Dot(VertexList[i]);
		Tmp.y = up.Dot(VertexList[i]);
		Tmp.z = front.Dot(VertexList[i]);

		if (Tmp.x < Min.x) Min.x = Tmp.x;
		if (Tmp.y < Min.y) Min.y = Tmp.y;
		if (Tmp.z < Min.z) Min.z = Tmp.z;

		if (Tmp.x > Max.x) Max.x = Tmp.x;
		if (Tmp.y > Max.y) Max.y = Tmp.y;
		if (Tmp.z > Max.z) Max.z = Tmp.z;
	}

	Vector Center_Mass (Max + Min);
	Center_Mass *= 0.5;

	Rotate (posit, Center_Mass);

	Center_Mass	= Max - Min;
	Center_Mass *= 0.5f;

  Width   = Center_Mass.x;  
	Height  = Center_Mass.y;  
	Breadth = Center_Mass.z;  

	radius2 = Center_Mass.Dot(Center_Mass);
	radius = (F32)sqrt (radius2);
}

//Jacobian method for computing the eigen vectors of a symetric matrix
void Sphere::Eigen_Vectors (Vector &Eigen_Value)
{
	S32 i;
	S32 nrot;
	F64 t;
	F64  s;
	F64  h;
	F64  g;
	F64  c;
	F64  sm;
	F64  tau;
	F64  theta;
	F64  tresh;
	F64  b[3];
	F64  z[3];
	F64  d[3];
	Matrix *My;
	Matrix Eigen_Vectors (Matrix::I);
	
	My = (Matrix *)this;
	
	b[0] = Value(0, 0); 
	b[1] = Value(1, 1);
	b[2] = Value(2, 2);
	
	d[0] = Value(0, 0); 
	d[1] = Value(1, 1); 
	d[2] = Value(2, 2); 
	
	z[0] = 0.0;
	z[1] = 0.0;
	z[2] = 0.0;
	
	
	nrot = 0;
	for (i = 0; i < 10; i++)
  {
		sm = fabs(Value(0, 1)) + fabs(Value(0, 2)) + fabs(Value(1, 2));
		
		if (sm < SPHERE_EPSILON * 1.0e-5f)
    {
			ASSERT (fabs(Eigen_Vectors.right.Dot(Eigen_Vectors.right) - 1.0) < SPHERE_EPSILON);
			ASSERT (fabs(Eigen_Vectors.up.Dot(Eigen_Vectors.up) - 1.0) < SPHERE_EPSILON);
			ASSERT (fabs(Eigen_Vectors.front.Dot(Eigen_Vectors.front) - 1.0) < SPHERE_EPSILON);
			
			// order the eigen vectors	
//			Vector Tmp;
//			Eigen_Vectors.right.Cross__ (Tmp, Eigen_Vectors.up);
			Vector Tmp (Cross (Eigen_Vectors.right, Eigen_Vectors.up));
			
			if (Tmp.Dot (Eigen_Vectors.front) < 0)
      {
				Eigen_Vectors.front.x *= -1;
				Eigen_Vectors.front.y *= -1;
				Eigen_Vectors.front.z *= -1;
			}
			
			SetInverse(Eigen_Vectors);
			Eigen_Value.x = (F32)d[0];
			Eigen_Value.y = (F32)d[1];
			Eigen_Value.z = (F32)d[2];
			return;
		}
		
		if (i < 3)
    {
			tresh = (F32)(0.2 / 9.0) * sm; 
		}
    else
    {
			tresh = 0.0;
		}
		
		
		// First row
		g = 100.0 * fabs(Value(0, 1));
		if (i > 3 && fabs(d[0]) + g == fabs(d[0]) && fabs(d[1]) + g == fabs(d[1]))
    {
			Value(0, 1) = 0.0;
		}
    else if (fabs(Value(0, 1)) > tresh)
    {
			h = d[1] - d[0];
			if (fabs(h) + g == fabs(h))
      {
				t = Value(0, 1) / h;
			}
      else
      {
				theta = (0.5 * h / (Value(0, 1)));
				t = (1.0 / (fabs(theta) + sqrt(1.0 + theta * theta)));
				if (theta < 0.0)
        {
					t = -t;
				}
			}
			c = 1.0 / sqrt( 1 + t * t); 
			s = t * c; 
			tau = s / (1.0 + c); 
			h = t * Value(0, 1);
			z[0] -= h; 
			z[1] += h; 
			d[0] -= h; 
			d[1] += h;
			Value(0, 1) = 0.0;
			ROT (*this, 0, 2, 1, 2, s, tau); 
			ROT (Eigen_Vectors, 0, 0, 0, 1, s, tau); 
			ROT (Eigen_Vectors, 1, 0, 1, 1, s, tau); 
			ROT (Eigen_Vectors, 2, 0, 2, 1, s, tau); 
			
			nrot++;
		}
		
		
		// second row
		g = 100.0 * fabs(Value(0, 2));
		if (i>3 && fabs(d[0]) + g == fabs(d[0]) && fabs(d[2]) + g == fabs(d[2]))
    {
			Value(0, 2) = 0.0;
		}
    else if (fabs(Value(0, 2)) > tresh)
    {
			h = d[2] - d[0];
			if (fabs(h) + g == fabs(h))
      {
				t = (Value(0, 2)) / h;
			}
      else 
      {
				theta = (0.5 * h / Value(0, 2));
				t = (1.0 / (fabs(theta) + sqrt(1.0 + theta * theta)));
				if (theta < 0.0)
        {
					t = -t;
				}
			}
			c = (1.0 / sqrt(1 + t * t)); 
			s = t * c; 
			tau = (s / (1.0 + c)); 
			h = t * Value(0, 2);
			z[0] -= h; 
			z[2] += h; 
			d[0] -= h; 
			d[2] += h;
			Value(0, 2)=0.0;
			ROT (*this, 0, 1, 1, 2, s, tau); 
			ROT (Eigen_Vectors, 0, 0, 0, 2, s, tau); 
			ROT (Eigen_Vectors, 1, 0, 1, 2, s, tau); 
			ROT (Eigen_Vectors, 2, 0, 2, 2, s, tau); 
		}
		
		// trird row
		g = 100.0 * fabs(Value(1, 2));
		if (i > 3 && fabs(d[1]) + g == fabs(d[1]) && fabs(d[2]) + g == fabs(d[2]))
    {
			Value(1, 2) = 0.0;
		}
    else if (fabs(Value(1, 2)) > tresh)
    {
			h = d[2] - d[1];
			if (fabs(h) + g == fabs(h))
      {
				t = Value(1, 2) / h;
			}
      else
      {
				theta = (0.5 * h / Value(1, 2));
				t = (1.0 / (fabs(theta) + sqrt(1.0 + theta * theta)));
				if (theta < 0.0)
        {
					t = -t;
				}
			}
			c = 1.0 / sqrt(1 + t*t); 
			s = t * c; 
			tau = s / (1.0 + c); 
			
			h = t * Value(1, 2);
			z[1] -= h; 
			z[2] += h; 
			d[1] -= h; 
			d[2] += h;
			Value(1, 2) = 0.0;
			ROT (*this, 0, 1, 0, 2, s, tau); 
			ROT (Eigen_Vectors, 0, 1, 0, 2, s, tau); 
			ROT (Eigen_Vectors, 1, 1, 1, 2, s, tau); 
			ROT (Eigen_Vectors, 2, 1, 2, 2, s, tau); 
			nrot++;
		}
		
		b[0] += z[0]; d[0] = b[0]; z[0] = 0.0;
		b[1] += z[1]; d[1] = b[1]; z[1] = 0.0;
		b[2] += z[2]; d[2] = b[2]; z[2] = 0.0;
	}
	
	Eigen_Value.x = (F32)d[0];
	Eigen_Value.y = (F32)d[1];
	Eigen_Value.z = (F32)d[2];
	*My = Matrix::I;
} 	


Vector Sphere::Calculate_Principal_Axis (Vector &Scale,
										 Vector *VertexList, U32 VertCount,
										 U16 *IndexList, U32 IndexCount)
{
	VertCount;

	S32 i;
	S32 index;
	F64 K;
	F64 Area;
	F64 Ixx;
	F64 Iyy;
	F64 Izz;
	F64 Ixy;
	F64 Ixz;
	F64 Iyz;
	F64 Total_Area;
	
	Vector Center (0, 0, 0);
	Vector Center_Mass (0, 0, 0);
	Vector Var (0, 0, 0);
	Vector Cov (0, 0, 0);
	
	Total_Area = 0.0;
  for (i = 0; i < (S32)IndexCount; i += 3)
  {
		index = IndexList[i];
		Vector P0 (VertexList[index].x * Scale.x, 
			VertexList[index].y * Scale.y, 
			VertexList[index].z * Scale.z);
		
		index = IndexList[i+1];
		Vector P1 (VertexList[index].x * Scale.x, 
			VertexList[index].y * Scale.y, 
			VertexList[index].z * Scale.z);
		
		index = IndexList[i+2];
		Vector P2 (VertexList[index].x * Scale.x, 
			VertexList[index].y * Scale.y, 
			VertexList[index].z * Scale.z);
		
		Vector Normal ((P1 - P0) * (P2 - P0));
		
		Area = 0.5f * Normal.Magnitude();
		
		Center = P0 + P1 + P2;
		Center *= (1.0f / 3.0f);
		
		// Inercia of each point in the triangle
		Ixx = P0.x * P0.x + P1.x * P1.x + P2.x * P2.x;	
		Iyy = P0.y * P0.y + P1.y * P1.y + P2.y * P2.y;	
		Izz = P0.z * P0.z + P1.z * P1.z + P2.z * P2.z;	
		
		Ixy = P0.x * P0.y + P1.x * P1.y + P2.x * P2.y;	
		Iyz = P0.y * P0.z + P1.y * P1.z + P2.y * P2.z;	
		Ixz = P0.x * P0.z + P1.x * P1.z + P2.x * P2.z;	
		
		if (Area > SPHERE_EPSILON * 10.0)
		{
			K = Area / 12.0f;
			//Choriollis theorem for Inercia of a triangle of arbitrary orientation
			Ixx = K * (Ixx + 9.0f * Center.x * Center.x);
			Iyy = K * (Iyy + 9.0f * Center.y * Center.y);
			Izz = K * (Izz + 9.0f * Center.z * Center.z);
			
			Ixy = K * (Ixy + 9.0f * Center.x * Center.y);
			Ixz = K * (Ixz + 9.0f * Center.x * Center.z);
			Iyz = K * (Iyz + 9.0f * Center.y * Center.z);
			Center *= (F32)Area;
		} 
		
		Total_Area += Area;
		Center_Mass += Center;
		Var += Vector ((F32)Ixx, (F32)Iyy, (F32)Izz);
		Cov += Vector ((F32)Ixy, (F32)Ixz, (F32)Iyz);
	}
	
	
	if (Total_Area > SPHERE_EPSILON * 10.0)
  {
		K = 1.0 / Total_Area; 
		Var *= (F32)K;
		Cov *= (F32)K;
		Center_Mass *= (F32)K;
	}
	
	Ixx = Var.x - Center_Mass.x * Center_Mass.x;
	Iyy = Var.y - Center_Mass.y * Center_Mass.y;
	Izz = Var.z - Center_Mass.z * Center_Mass.z;
	
	Ixy = Cov.x - Center_Mass.x * Center_Mass.y;
	Ixz = Cov.y - Center_Mass.x * Center_Mass.z;
	Iyz = Cov.z - Center_Mass.y * Center_Mass.z;

  Matrix::Set(Vector ((F32)Ixx, (F32)Ixy, (F32)Ixz), 
		Vector ((F32)Ixy, (F32)Iyy, (F32)Iyz),  
		Vector ((F32)Ixz, (F32)Iyz, (F32)Izz));

	Vector Eigen_Value;
	Eigen_Vectors (Eigen_Value);

  return Eigen_Value;
}



void Matrix::Transform_H_Vectors (
	const F32 *Dst, 
	F32 &W, 
	const F32 *Src) const
{
	F32 x;
	F32 y;
	F32 z;
	F32 *TmpDst;

	TmpDst = (F32*)Dst;

	x = Src[0];
	y = Src[1];
	z = Src[2];

	TmpDst [0] = right.x * x + up.x * y + front.x * z + posit.x;
	TmpDst [1] = right.y * x + up.y * y + front.y * z + posit.y;
	TmpDst [2] = right.z * x + up.z * y + front.z * z + posit.z;
	W		   = rightw  * x + upw  * y + frontw  * z + positw;
}

void Matrix::Rotate_Vectors (
	const F32 *Dst, 
	const F32 *Src, 
	S32 Stride, 
	S32 Count) const
{
	S32 i;
	F32 x;
	F32 y;
	F32 z;
	F32 *TmpDst;

	TmpDst = (F32*)Dst;
	
	Stride >>= 2;
	for (i = 0; i <	Count; i ++) {
		x = Src[0];
		y = Src[1];
		z = Src[2];
		Src	+= Stride;

		TmpDst[0] = right.x * x + up.x * y + front.x * z;
		TmpDst[1] = right.y * x + up.y * y + front.y * z;
		TmpDst[2] = right.z * x + up.z * y + front.z * z;
		TmpDst += Stride;
	}
}


void Matrix::Unrotate_Vectors (
	const F32 *Dst, 
	const F32 *Src, 
	S32 Stride, 
	S32 Count) const
{
	S32 i;
	F32 x;
	F32 y;
	F32 z;
	F32 *TmpDst;

	TmpDst = (F32*)Dst;
	
	Stride >>= 2;
	for (i = 0; i <	Count; i ++) {
		x = Src[0];
		y = Src[1];
		z = Src[2];
		Src	+= Stride;

		TmpDst[0] = right.x * x + right.y * y + right.z * z;
		TmpDst[1] = up.x * x + up.y * y + up.z * z;
		TmpDst[2] = front.x * x + front.y * y + front.z * z;
		TmpDst += Stride;
	}
}

Vector Matrix::Vector_Rotate (const Vector &v) const 
{
	return Vector(right.x * v.x + up.x * v.y + front.x * v.z,
				  right.y * v.x + up.y * v.y + front.y * v.z,
				  right.z * v.x + up.z * v.y + front.z * v.z);
}

Vector Matrix::Vector_RotateInv (const Vector &v) const 
{
	return Vector(right.x * v.x + right.y * v.y + right.z * v.z,
				  up.x	  * v.x + up.y	  * v.y + up.z	  * v.z,
				  front.x * v.x + front.y * v.y + front.z * v.z);
}

Vector Matrix::Vector_TransformInv (const Vector &v) const
{
	F32 x;
	F32 y;
	F32 z;

	x = v.x - posit.x;
	y = v.y - posit.y;
	z = v.z - posit.z;
	return Vector(right.x * x + right.y * y + right.z * z,
				  up.x * x + up.y * y + up.z * z,
				  front.x * x + front.y * y + front.z * z);
}


ostream& operator<<(ostream &o, const Matrix &m)
{
  return 
  (
    o << 'p' << m.posit << m.positw << 'f' << m.front << m.frontw << 'r' << m.right << m.rightw << 'u' << m.up << m.upw
  );
}



#ifdef 	NO_ASM_MATH 

Vector Matrix::Vector_Transform (const Vector &v) const 
{
	return Vector(right.x * v.x + up.x * v.y + front.x * v.z + posit.x,
				  right.y * v.x + up.y * v.y + front.y * v.z + posit.y,
				  right.z * v.x + up.z * v.y + front.z * v.z + posit.z);
}

Matrix operator* (const Matrix &A, const Matrix &B)
{
	Matrix M;

	M.right.x = A.right.x * B.right.x + A.right.y * B.up.x + A.right.z * B.front.x + A.rightw * B.posit.x;
	M.right.y = A.right.x * B.right.y + A.right.y * B.up.y + A.right.z * B.front.y + A.rightw * B.posit.y;
	M.right.z = A.right.x * B.right.z + A.right.y * B.up.z + A.right.z * B.front.z + A.rightw * B.posit.z;
	M.rightw  = A.right.x * B.rightw  + A.right.y * B.upw  + A.right.z * B.frontw  + A.rightw * B.positw;

	M.up.x    = A.up.x * B.right.x	  + A.up.y * B.up.x	   + A.up.z * B.front.x	   + A.upw * B.posit.x;
	M.up.y    = A.up.x * B.right.y	  + A.up.y * B.up.y	   + A.up.z * B.front.y	   + A.upw * B.posit.y;
	M.up.z    = A.up.x * B.right.z	  + A.up.y * B.up.z	   + A.up.z * B.front.z	   + A.upw * B.posit.z;
	M.upw     = A.up.x * B.rightw	    + A.up.y * B.upw	   + A.up.z * B.frontw	   + A.upw * B.positw;

	M.front.x = A.front.x * B.right.x + A.front.y * B.up.x + A.front.z * B.front.x + A.frontw * B.posit.x;
	M.front.y = A.front.x * B.right.y + A.front.y * B.up.y + A.front.z * B.front.y + A.frontw * B.posit.y;
	M.front.z = A.front.x * B.right.z + A.front.y * B.up.z + A.front.z * B.front.z + A.frontw * B.posit.z;
	M.frontw  = A.front.x * B.rightw  + A.front.y * B.upw  + A.front.z * B.frontw  + A.frontw * B.positw;

	M.posit.x = A.posit.x * B.right.x + A.posit.y * B.up.x + A.posit.z * B.front.x + A.positw * B.posit.x;
	M.posit.y = A.posit.x * B.right.y + A.posit.y * B.up.y + A.posit.z * B.front.y + A.positw * B.posit.y;
	M.posit.z = A.posit.x * B.right.z + A.posit.y * B.up.z + A.posit.z * B.front.z + A.positw * B.posit.z;
	M.positw  = A.posit.x * B.rightw  + A.posit.y * B.upw  + A.posit.z * B.frontw  + A.positw * B.positw; 
	
	return M;
}

void Matrix::Transform_Vectors (
	const F32 *Dst, 
	const F32 *Src, 
	S32 Stride, 
	S32 Count) const
{
	S32 i;
	F32 x;
	F32 y;
	F32 z;
	F32 *TmpDst;

	TmpDst = (F32*)Dst;
	
	Stride >>= 2;
	for (i = 0; i <	Count; i ++) {
		x = Src[0];
		y = Src[1];
		z = Src[2];
		Src	+= Stride;

		TmpDst[0] = right.x * x + up.x * y + front.x * z + posit.x;
		TmpDst[1] = right.y * x + up.y * y + front.y * z + posit.y;
		TmpDst[2] = right.z * x + up.z * y + front.z * z + posit.z;
		TmpDst += Stride;
	}
}

#else

#pragma warning (push, 4)
#pragma warning (disable: 4100)

#define	STACK_FRAME(pushcount) 	 (4	+ pushcount*4)

static NAKED Transform_Component_Sparse ()
{
	__asm {
		fld   (MATRIX_STRUCT [ecx]).right.x				 // a00
		fmul  dword ptr (VectorData [edx]).x			  	 // x*a00

		fld   (MATRIX_STRUCT [ecx]).up.x				 // a10 x*a00
		fmul  dword ptr (VectorData [edx]).y			  	 // y*a10 x*a00

		fxch  st(1)										// x*a00 y*a10 
		fadd  dword ptr (MATRIX_STRUCT [ecx]).posit.x	// x*a00+a30 y*a10 

		fld   (MATRIX_STRUCT [ecx]).front.x				// a20 x*a00+a30 y*a10 
		fmul  dword ptr (VectorData [edx]).z			  	// a20*z x*a00+a30 y*a10 
														
		fxch  st(2)										// y*a10 x*a00+a30  a20*z 
		faddp st(1), st(0)								// x*a00+y*a10+a30  a20*z 
		faddp st(1), st(0)								// x*a00+y*a10+a20*z+a30  
		ret
	}
}

static NAKED Transform_Dense_Vector ()
{
	__asm {
		fld   (MATRIX_STRUCT [eax]).right.x				 // a00
		fmul  dword ptr (QUADRUPLET [edx]).x			 // x*a00

		fld   (MATRIX_STRUCT [eax]).up.x				 // a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).y			 // y*a10 x*a00

		fld   (MATRIX_STRUCT [eax]).front.x				 // a20 y*a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).z			 // z*a20 y*a10 x*a00

		fxch  st(1)										 // y*a10 z*a20 x*a00
		faddp st(2), st(0) 								 // z*a20 x*a00+y*a10 

		fld   (MATRIX_STRUCT [eax]).posit.x				 // a31 z*a20 x*a00+y*a10 
		fmul  dword ptr (QUADRUPLET [edx]).w			 // w*a31 z*a20 x*a00+y*a10 
														
		fxch  st(1)										// z*a20 w*a31  x*a00+y*a10 
		faddp st(1), st(0)								// w*a31  x*a00+y*a10+z*a20  
		faddp st(1), st(0)								// x*a00+y*a10+z*a20+w*a31    


		fld   (MATRIX_STRUCT [eax]).right.y				 // a00
		fmul  dword ptr (QUADRUPLET [edx]).x			 // x*a00

		fld   (MATRIX_STRUCT [eax]).up.y				 // a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).y			 // y*a10 x*a00

		fld   (MATRIX_STRUCT [eax]).front.y				 // a20 y*a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).z			 // z*a20 y*a10 x*a00

		fxch  st(1)										 // y*a10 z*a20 x*a00
		faddp st(2), st(0) 								 // z*a20 x*a00+y*a10 

		fld   (MATRIX_STRUCT [eax]).posit.y				 // a31 z*a20 x*a00+y*a10 
		fmul  dword ptr (QUADRUPLET [edx]).w			 // w*a31 z*a20 x*a00+y*a10 
														
		fxch  st(1)										// z*a20 w*a31  x*a00+y*a10 
		faddp st(1), st(0)								// w*a31  x*a00+y*a10+z*a20  
		faddp st(1), st(0)								// x*a00+y*a10+z*a20+w*a31    


		fld   (MATRIX_STRUCT [eax]).right.z				 // a00
		fmul  dword ptr (QUADRUPLET [edx]).x			 // x*a00

		fld   (MATRIX_STRUCT [eax]).up.z				 // a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).y			 // y*a10 x*a00

		fld   (MATRIX_STRUCT [eax]).front.z				 // a20 y*a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).z			 // z*a20 y*a10 x*a00

		fxch  st(1)										 // y*a10 z*a20 x*a00
		faddp st(2), st(0) 								 // z*a20 x*a00+y*a10 

		fld   (MATRIX_STRUCT [eax]).posit.z				 // a31 z*a20 x*a00+y*a10 
		fmul  dword ptr (QUADRUPLET [edx]).w			 // w*a31 z*a20 x*a00+y*a10 
														
		fxch  st(1)										// z*a20 w*a31  x*a00+y*a10 
		faddp st(1), st(0)								// w*a31  x*a00+y*a10+z*a20  
		faddp st(1), st(0)								// x*a00+y*a10+z*a20+w*a31    


		fld   (MATRIX_STRUCT [eax]).right.w				 // a00
		fmul  dword ptr (QUADRUPLET [edx]).x			 // x*a00

		fld   (MATRIX_STRUCT [eax]).up.w				 // a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).y			 // y*a10 x*a00

		fld   (MATRIX_STRUCT [eax]).front.w				 // a20 y*a10 x*a00
		fmul  dword ptr (QUADRUPLET [edx]).z			 // z*a20 y*a10 x*a00

		fxch  st(1)										 // y*a10 z*a20 x*a00
		faddp st(2), st(0) 								 // z*a20 x*a00+y*a10 

		fld   (MATRIX_STRUCT [eax]).posit.w				 // a31 z*a20 x*a00+y*a10 
		fmul  dword ptr (QUADRUPLET [edx]).w			 // w*a31 z*a20 x*a00+y*a10 
														
		fxch  st(1)										// z*a20 w*a31  x*a00+y*a10 
		faddp st(1), st(0)								// w*a31  x*a00+y*a10+z*a20  
		faddp st(1), st(0)								// x*a00+y*a10+z*a20+w*a31    

		ret
	}
}


NAKED Vector Matrix::Vector_Transform (const Vector &v) const 
{

//	return Vector(right.x * v.x + up.x * v.y + front.x * v.z + posit.x,
//				  right.y * v.x + up.y * v.y + front.y * v.z + posit.y,
//				  right.z * v.x + up.z * v.y + front.z * v.z + posit.z);


	__asm {
		mov		eax, [esp + STACK_FRAME(0)]
		mov		edx, [esp + STACK_FRAME(0) + 4]
		call	Transform_Component_Sparse							//x0
		lea		ecx, (MATRIX_STRUCT [ecx]).right.y		    
		call	Transform_Component_Sparse							//y0 x0
		lea		ecx, (MATRIX_STRUCT [ecx]).right.y
		call	Transform_Component_Sparse							//z0 y0 x0
		mov		edx, [eax]
		fxch	st(2)										//x0 y0 z0
		fstp	dword ptr (VectorData [eax]).x					//y0 z0
		fstp	dword ptr (VectorData [eax]).y					//z0
		fstp	dword ptr (VectorData [eax]).z					
		ret		8
	}

}




NAKED void Matrix::Transform_Vectors (
	const F32 *Dst, 
	const F32 *Src, 
	S32 Stride, 
	S32 Count) const
{
//	S32 i;
//	F32 x;
//	F32 y;
//	F32 z;
//	F32 *TmpDst;
//
//	TmpDst = (F32*)Dst;
//	
//	Stride >>= 2;
//	for (i = 0; i <	Count; i ++) {
//		x = Src[0];
//		y = Src[1];
//		z = Src[2];
//		Src	+= Stride;
//
//		TmpDst[0] = right.x * x + up.x * y + front.x * z + posit.x;
//		TmpDst[1] = right.y * x + up.y * y + front.y * z + posit.y;
//		TmpDst[2] = right.z * x + up.z * y + front.z * z + posit.z;
//		TmpDst += Stride;
//	}

	__asm {
		push	esi
		push	edi
		push	ebx

		mov		eax, [esp + STACK_FRAME(3)]			//	Get Dst
		mov		edx, [esp + STACK_FRAME(3) + 4]		//	Get Src
		mov		edi, [esp + STACK_FRAME(3) + 8]		//	Get Stride
		mov		ebx, [esp + STACK_FRAME(3) + 12]	 //	Get Count

		mov		esi, [eax]
		mov		esi, [eax + edi]

		test	ebx, ebx
		mov		esi, ecx
		jmp		End_Transform_Loop
	Begin_Transform_Loop:

		call	Transform_Component_Sparse							//x0
		lea		ecx, (MATRIX_STRUCT [ecx]).right.y		    
		call	Transform_Component_Sparse							//y0 x0
		lea		ecx, (MATRIX_STRUCT [ecx]).right.y
		call	Transform_Component_Sparse							//z0 y0 x0
		mov		ecx, [eax + edi * 2]                    // preload cache-line; can crash on end+1 element
		add		edx, edi

		mov		ecx, esi
		fstp	dword ptr (VectorData [eax]).z					//y0 z0
		fstp	dword ptr (VectorData [eax]).y					//z0
		fstp	dword ptr (VectorData [eax]).x					

		add		eax, edi
		dec		ebx
	End_Transform_Loop:	
		jg		Begin_Transform_Loop
		pop		ebx
		pop		edi
		pop		esi

		ret		16 
	}
}



NAKED Matrix operator* (const Matrix &A, const Matrix &B)
{
/*
	Matrix M;

	M.right.x = A.right.x * B.right.x + A.right.y * B.up.x + A.right.z * B.front.x + A.rightw * B.posit.x;
	M.right.y = A.right.x * B.right.y + A.right.y * B.up.y + A.right.z * B.front.y + A.rightw * B.posit.y;
	M.right.z = A.right.x * B.right.z + A.right.y * B.up.z + A.right.z * B.front.z + A.rightw * B.posit.z;
	M.rightw  = A.right.x * B.rightw  + A.right.y * B.upw  + A.right.z * B.frontw  + A.rightw * B.positw;

	M.up.x    = A.up.x * B.right.x	  + A.up.y * B.up.x	   + A.up.z * B.front.x	   + A.upw * B.posit.x;
	M.up.y    = A.up.x * B.right.y	  + A.up.y * B.up.y	   + A.up.z * B.front.y	   + A.upw * B.posit.y;
	M.up.z    = A.up.x * B.right.z	  + A.up.y * B.up.z	   + A.up.z * B.front.z	   + A.upw * B.posit.z;
	M.upw     = A.up.x * B.rightw	  + A.up.y * B.upw	   + A.up.z * B.frontw	   + A.upw * B.positw;

	M.front.x = A.front.x * B.right.x + A.front.y * B.up.x + A.front.z * B.front.x + A.frontw * B.posit.x;
	M.front.y = A.front.x * B.right.y + A.front.y * B.up.y + A.front.z * B.front.y + A.frontw * B.posit.y;
	M.front.z = A.front.x * B.right.z + A.front.y * B.up.z + A.front.z * B.front.z + A.frontw * B.posit.z;
	M.frontw  = A.front.x * B.rightw  + A.front.y * B.upw  + A.front.z * B.frontw  + A.frontw * B.positw;

	M.posit.x = A.posit.x * B.right.x + A.posit.y * B.up.x + A.posit.z * B.front.x + A.positw * B.posit.x;
	M.posit.y = A.posit.x * B.right.y + A.posit.y * B.up.y + A.posit.z * B.front.y + A.positw * B.posit.y;
	M.posit.z = A.posit.x * B.right.z + A.posit.y * B.up.z + A.posit.z * B.front.z + A.positw * B.posit.z;
	M.positw  = A.posit.x * B.rightw  + A.posit.y * B.upw  + A.posit.z * B.frontw  + A.positw * B.positw; 
  
	return M;
*/


	__asm {
		push	esi				
		mov		eax, [ecx]

		mov		esi, 4
	Begin_Matrix_Loop:	

		mov		eax, [ecx + size QUADRUPLET]

		mov		eax, [esp + STACK_FRAME(1)]
		call	Transform_Dense_Vector						  // w z y x
		add		edx, size QUADRUPLET

		fxch	st(3)										  // x z y w
		fstp	dword ptr (QUADRUPLET [ecx]).x				  
		fstp	dword ptr (QUADRUPLET [ecx]).z				   
		fstp	dword ptr (QUADRUPLET [ecx]).y					
		fstp	dword ptr (QUADRUPLET [ecx]).w					
		add		ecx, size QUADRUPLET

		dec		esi
		jnz		Begin_Matrix_Loop
		pop		esi
		lea		eax, [ecx - size MATRIX_STRUCT]
		ret		4
	}
}
#pragma warning (pop)

#endif


#if 0
void Matrix::Transform( Matrix &result, const Matrix &m) const 
{
	result.right.x = right.x * m.right.x + right.y * m.up.x + right.z * m.front.x + rightw * m.posit.x;
	result.right.y = right.x * m.right.y + right.y * m.up.y + right.z * m.front.y + rightw * m.posit.y;
	result.right.z = right.x * m.right.z + right.y * m.up.z + right.z * m.front.z + rightw * m.posit.z;
	result.rightw  = right.x * m.rightw  + right.y * m.upw  + right.z * m.frontw  + rightw * m.positw;
	
	result.up.x = up.x * m.right.x + up.y * m.up.x + up.z * m.front.x + upw * m.posit.x;
	result.up.y = up.x * m.right.y + up.y * m.up.y + up.z * m.front.y + upw * m.posit.y;
	result.up.z = up.x * m.right.z + up.y * m.up.z + up.z * m.front.z + upw * m.posit.z;
	result.upw  = up.x * m.rightw  + up.y * m.upw  + up.z * m.frontw  + upw * m.positw;
	
	result.front.x = front.x * m.right.x + front.y * m.up.x + front.z * m.front.x + frontw * m.posit.x;
	result.front.y = front.x * m.right.y + front.y * m.up.y + front.z * m.front.y + frontw * m.posit.y;
	result.front.z = front.x * m.right.z + front.y * m.up.z + front.z * m.front.z + frontw * m.posit.z;
	result.frontw  = front.x * m.rightw  + front.y * m.upw  + front.z * m.frontw  + frontw * m.positw;
	
	result.posit.x = posit.x * m.right.x + posit.y * m.up.x + posit.z * m.front.x + positw * m.posit.x;
	result.posit.y = posit.x * m.right.y + posit.y * m.up.y + posit.z * m.front.y + positw * m.posit.y;
	result.posit.z = posit.x * m.right.z + posit.y * m.up.z + posit.z * m.front.z + positw * m.posit.z;
	result.positw  = posit.x * m.rightw  + posit.y * m.upw  + posit.z * m.frontw  + positw * m.positw;
}
//----------------------------------------------------------------------------

void Matrix::Transform( Vector *dst, const Vector *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//----------------------------------------------------------------------------

void Matrix::Rotate( Vector *dst, const Vector *src, U32 count) const 
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//----------------------------------------------------------------------------

void Matrix::RotateInv( Vector *dst, const Vector *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//----------------------------------------------------------------------------

void Matrix::Transform( Vector *dst, Vector *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Transform( dst[i], src[i]);
  }
}
//-----------------------------------------------------------------------------

void Matrix::Rotate( Vector *dst, Vector *src, U32 count) const 
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    Rotate( dst[i], src[i]);
  }
}
//----------------------------------------------------------------------------

void Matrix::RotateInv( Vector *dst, Vector *src, U32 count) const
{
  U32 i;
  for (i = 0; i < count; i++)
  {
    RotateInv( dst[i], src[i]);
  }
}
//----------------------------------------------------------------------------

void Vector::Cross(Vector &dst, const Vector &src) const
{
	dst.x = y * src.z - z * src.y;
	dst.y = z * src.x - x * src.z;
	dst.z = x * src.y - y * src.x;
}
#endif
//----------------------------------------------------------------------------

void BoundingBox::Calculate(Vector *verts, U32 count)
{
  min.Set(  F32_MAX,  F32_MAX,  F32_MAX);
	max.Set( -F32_MAX, -F32_MAX, -F32_MAX);

	for (U32 i = 0; i < count; i++)
	{
    // get the vertex position
	  Vector &v = verts[i];

		// update the bounding box
		if (v.x < min.x)
		{
			min.x = v.x;
		}
		if (v.y < min.y)
		{
			min.y = v.y;
		}
		if (v.z < min.z)
		{
			min.z = v.z;
		}
		if (v.x > max.x)
		{
			max.x = v.x;
		}
		if (v.y > max.y)
		{
			max.y = v.y;
		}
		if (v.z > max.z)
		{
			max.z = v.z;
		}
  }
}

#if 0
// A direction of a unit vector in polar coordinates
struct PolarAnim
{
  VectorDir current, target;

  VectorDir() { }

  inline Vector GetVector()
  {
    return Vector(current);
  }

  void ClearData()
  {
    current.ClearData();
    target.ClearData();
  }

  Turn( F32 t)
  {
    // Figure out how much angle there is remaining to turn
    VectorDir dd = target - current;

    if (fabs(dd.u) < t)
    {
      // clamp
      current.u = target.u;
    }
    else
    {
      // animate
      current.u += (dd.u > 0) ? t : -t;
    }
   
    if (fabs(dd.v) < turn)
    {
      // clamp
      current.v = target.v;
    }
    else
    {
      // animate
      current.v += (dd.v > 0) ? t : -t;
    }
  }
};
//----------------------------------------------------------------------------
#endif
