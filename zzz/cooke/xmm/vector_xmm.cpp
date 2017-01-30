///////////////////////////////////////////////////////////////////////////////
//
// vector_xmm.cpp
//
// 19-Jan-99  Harry Travis
//

#ifdef __DO_XMM_BUILD

#include "vector_xmm.h"
#include "debug_memory.h"

VectorXmm::VectorXmm()
{
  SetZero();
}

VectorXmm::VectorXmm(__m128 _X, __m128 _Y, __m128 _Z)
{
  Set(_X, _Y, _Z);
}

VectorXmm::VectorXmm(Vector &v)
{
  Set(v);
}

void *VectorXmm::operator new(size_t c)
{
  return (void*) Debug::Memory::Aligning::AligningAlloc( c * sizeof(VectorXmm), 4);
}

void VectorXmm::operator delete(void *data)
{
  Debug::Memory::Aligning::AligningFree( data);
}

void VectorXmm::SetZero()
{
  X = Y = Z = _mm_setzero_ps();
}

void VectorXmm::Set(F32 x, F32 y, F32 z)
{
  X = _mm_set_ps1(x);
  Y = _mm_set_ps1(y);
  Z = _mm_set_ps1(z);
}

void VectorXmm::Set(Vector &v)
{
  Set(v.x, v.y, v.z);
}

void VectorXmm::Set(__m128 _X, __m128 _Y, __m128 _Z)
{
  X = _X;
  Y = _Y;
  Z = _Z;
}

void VectorXmm::Set(Vector &v0, Vector &v1, Vector &v2, Vector &v3)
{
  __m128 t;
  LOAD_PLANAR_4X4((F32*) &v0, (F32*) &v1, (F32*) &v2, (F32*) &v3, X, Y, Z, t);
}

void VectorXmm::SetAligned(Vector &v0, Vector &v1, Vector &v2, Vector &v3)
{
  __m128 t;
  LOAD_PLANAR_4X4((F32*) &v0, (F32*) &v1, (F32*) &v2, (F32*) &v3, X, Y, Z, t);
}

void VectorXmm::operator=(F32 ss)
{
  X = Y = Z = _mm_set_ps1(ss);
}

void VectorXmm::operator=(Vector ss)
{
  X = _mm_set_ps1(ss.x);
  Y = _mm_set_ps1(ss.y);
  Z = _mm_set_ps1(ss.z);
}

Bool VectorXmm::operator==(const VectorXmm &vv)
{
  __m128 resultX, resultY, resultZ;
  U32 result = 0;

  resultX = _mm_cmpeq_ps(vv.X, X);
  result |= _mm_movemask_ps(resultX);

  resultY = _mm_cmpeq_ps(vv.Y, Y);
  result |= _mm_movemask_ps(resultY);

  resultZ = _mm_cmpeq_ps(vv.Z, Z);
  result |= _mm_movemask_ps(resultZ);

	return (Bool)(result);
}

Bool VectorXmm::operator!=(const VectorXmm &vv)
{
  __m128 resultX, resultY, resultZ;
  U32 result = 0;

  resultX = _mm_cmpneq_ps(vv.X, X);
  result |= _mm_movemask_ps(resultX);

  resultY = _mm_cmpneq_ps(vv.Y, Y);
  result |= _mm_movemask_ps(resultY);

  resultZ = _mm_cmpneq_ps(vv.Z, Z);
  result |= _mm_movemask_ps(resultZ);

	return (Bool)(result);
}

void VectorXmm::operator-=(const VectorXmm &vv)
{
  X = _mm_sub_ps(X, vv.X);
  Y = _mm_sub_ps(Y, vv.Y);
  Z = _mm_sub_ps(Z, vv.Z);
}

void VectorXmm::operator+=(const VectorXmm &vv)
{
  X = _mm_add_ps(X, vv.X);
  Y = _mm_add_ps(Y, vv.Y);
  Z = _mm_add_ps(Z, vv.Z);
}

void VectorXmm::operator/=(const VectorXmm &vv)
{
  X = _mm_mul_ps(X, RCP(vv.X));
  X = _mm_mul_ps(Y, RCP(vv.Y));
  X = _mm_mul_ps(Z, RCP(vv.Z));
}

void VectorXmm::operator*=(const VectorXmm &vv)
{
  X = _mm_mul_ps(X, vv.X);
  Y = _mm_mul_ps(Y, vv.Y);
  Z = _mm_mul_ps(Z, vv.Z);
}

VectorXmm VectorXmm::operator-(const VectorXmm &vv) const
{
	VectorXmm result = *this;
	result -= vv;
	return result;
}

VectorXmm VectorXmm::operator+(const VectorXmm &vv) const
{
	VectorXmm result = *this;
	result += vv;
	return result;
}

VectorXmm VectorXmm::operator*(const VectorXmm &vv) const
{
	VectorXmm result = *this;
	result *= vv;
	return result;
}

VectorXmm VectorXmm::operator/(const VectorXmm &vv) const
{
	VectorXmm result = *this;
	result /= vv;
	return result;
}

void VectorXmm::operator*=(const __m128 &ss)
{
  X = _mm_mul_ps(X, ss);
  Y = _mm_mul_ps(Y, ss);
  Z = _mm_mul_ps(Z, ss);
}

void VectorXmm::operator+=(const __m128 &ss)
{
  X = _mm_add_ps(X, ss);
  Y = _mm_add_ps(Y, ss);
  Z = _mm_add_ps(Z, ss);
}

void VectorXmm::operator-=(const __m128 &ss)
{
  X = _mm_sub_ps(X, ss);
  Y = _mm_sub_ps(Y, ss);
  Z = _mm_sub_ps(Z, ss);
}

void VectorXmm::operator/=(const __m128 &ss)
{
  __m128 tmp = RCP(ss);
  X = _mm_mul_ps(X, tmp);
  Y = _mm_mul_ps(Y, tmp);
  Z = _mm_mul_ps(Z, tmp);
}

void VectorXmm::operator*=(const F32 &ss)
{
  __m128 tmp = _mm_set_ps1(ss);
  X = _mm_mul_ps(X, tmp);
  Y = _mm_mul_ps(Y, tmp);
  Z = _mm_mul_ps(Z, tmp);
}

void VectorXmm::operator+=(const F32 &ss)
{
  __m128 tmp = _mm_set_ps1(ss);
  X = _mm_add_ps(X, tmp);
  Y = _mm_add_ps(Y, tmp);
  Z = _mm_add_ps(Z, tmp);
}

void VectorXmm::operator-=(const F32 &ss)
{
  __m128 tmp = _mm_set_ps1(ss);
  X = _mm_sub_ps(X, tmp);
  Y = _mm_sub_ps(Y, tmp);
  Z = _mm_sub_ps(Z, tmp);
}

void VectorXmm::operator/=(const F32 &ss)
{
  __m128 tmp = RCP(_mm_set_ps1(ss));
  X = _mm_mul_ps(X, tmp);
  Y = _mm_mul_ps(Y, tmp);
  Z = _mm_mul_ps(Z, tmp);
}

void VectorXmm::Normalize()
{
  NORM_VEC_3D(X, Y, Z);
}

__m128 VectorXmm::Dot(const VectorXmm &v) const
{
  return _mm_add_ps(_mm_add_ps(_mm_mul_ps(X, v.X), _mm_mul_ps(Y, v.Y)),_mm_mul_ps(Z, v.Z));
}

VectorXmm VectorXmm::Cross(const VectorXmm &v) const
{
  VectorXmm t;

  t.X = _mm_sub_ps(_mm_mul_ps(Y, v.Z), _mm_mul_ps(Z, v.Y));
  t.Y = _mm_sub_ps(_mm_mul_ps(X, v.Z), _mm_mul_ps(Z, v.X));
  t.Z = _mm_sub_ps(_mm_mul_ps(X, v.Y), _mm_mul_ps(Y, v.X));

  return t;
}

__m128 VectorXmm::Magnitude2() const
{
  return _mm_add_ps(_mm_mul_ps(X, X), _mm_add_ps(_mm_mul_ps(Y, Y), _mm_mul_ps(Z, Z)));
}

__m128 VectorXmm::InverseMagnitude() const
{ 
	return RCP_SQRT(Magnitude2()); 
}

__m128 VectorXmm::Magnitude() const
{ 
	return RCP(InverseMagnitude()); 
}

__m128 VectorXmm::Distance(const VectorXmm &vv) const
{
  __m128 d = _mm_add_ps(
               _mm_sub_ps(X, vv.X),
               _mm_add_ps(
                 _mm_sub_ps(Y, vv.Y),
                 _mm_sub_ps(Z, vv.Z)));

	return RCP(RCP_SQRT(d));
}

#endif __DO_XMM_BUILD
