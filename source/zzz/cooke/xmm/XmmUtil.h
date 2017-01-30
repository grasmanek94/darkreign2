// XmmUtil.h -- Katmai New Instruction utilities (intrinsics version)
// Harry Travis, (602) 552-2286, harry.s.travis@intel.com
// Intel Corporation, 1998-99

#ifndef __XMMUTIL_H
#define __XMMUTIL_H

#ifdef __DO_XMM_BUILD

#ifdef _USE_INTEL_COMPILER

#include "debug.h"        // ASSERT() etc.

#ifndef ASSERT(b)
#include <assert.h>
#define ASSERT(b) assert(b)
#endif

//include <xmmintrin.h>

////////////////////////////////////////////////////////////////////////////
// helpers:

// number of single-precision floating point elements in xmm registers
#define SIMD_WIDTH 4

// the memory address passed in is 16 byte aligned
#define IS_ALIGNED16(mem)  ((0x0000000f & (unsigned __int32)(mem)) == 0)

// the first element of a xmm register as a float
#define GET_FLOAT(a) *((float *)&(a))


////////////////////////////////////////////////////////////////////////////
// xmm utility functions:
struct XmmUtil
{
  ////////////////////////////////////////////////////////////////////////////
  // NAME:				LoadPlanar_4x4
  // PARAMETERS:	in0 -> in3  - input data
  //							out0 -> out3 - receive the transposed data elements from 4 
  //                             sequential structures starting at "in"
  // RETURNS:			none
  // DESCRIPTION:	Simultaneously loads and transposes data.  Leaves planar 
  //              data in xmm0, xmm1, xmm2, xmm3.
  //
  // input data:  a0, a1, a2, a3		output data: a0, b0, c0, d0
  //              b0, b1, b2, b3                 a1, b1, c1, d1
  //              c0, c1, c2, c3                 a2, b2, c2, d2
  //              d0, d1, d2, d3                 a3, b3, c3, d3
  ////////////////////////////////////////////////////////////////////////////
  static inline void LoadPlanar_4x4(
    float *in0,
    float *in1,
    float *in2,
    float *in3,
	  __m128 &out0, 
	  __m128 &out1, 
	  __m128 &out2, 
	  __m128 &out3)
  {	
	  ASSERT( in0 );
	  ASSERT( in1 );
	  ASSERT( in2 );
	  ASSERT( in3 );

    __m128 zero = _mm_setzero_ps();

    // could get away with two temporary vars, but let's go easy on the compiler
    __m128 t0, t1, t2, t3;

    t0 = _mm_loadl_pi(zero, (__m64*) in0); 
    t0 = _mm_loadh_pi(t0,   (__m64*) in1);        // t0 = b1 a1 b0 a0
    t1 = _mm_loadl_pi(zero, (__m64*) in2); 
    t1 = _mm_loadh_pi(t1,   (__m64*) in3);        // t1 = b3 a3 b2 a2
    out0 = _mm_shuffle_ps(t0, t1, 0x88);          // 0x88 = 10 00 10 00
    out1 = _mm_shuffle_ps(t0, t1, 0xdd);          // 0xdd = 11 01 11 01

    t2 = _mm_loadl_pi(zero, (__m64*) (in0+2));
    t2 = _mm_loadh_pi(t2,   (__m64*) (in1+2));    // t2 = d1 c1 d0 c0
    t3 = _mm_loadl_pi(zero, (__m64*) (in2+2));
    t3 = _mm_loadh_pi(t3,   (__m64*) (in3+2));    // t3 = d3 c3 d2 c2
    out2 = _mm_shuffle_ps(t2, t3, 0x88);          // 0x88 = 10 00 10 00
    out3 = _mm_shuffle_ps(t2, t3, 0xdd);          // 0xdd = 11 01 11 01
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				LoadPlanar_4x4
  // PARAMETERS:	in - pointer to input arrays with 4 sequential floating 
  //                   point data elements
  //							stride - sizeof(structure) for the interleaved data
  //							out0 -> out3 - receive the transposed data elements from 4 
  //                             sequential structures starting at "in"
  // RETURNS:			none
  // DESCRIPTION:	Simultaneously loads and transposes data.  Leaves planar 
  //              data in xmm0, xmm1, xmm2, xmm3.
  //
  // input data:  a0, a1, a2, a3		output data: a0, b0, c0, d0
  //              b0, b1, b2, b3                 a1, b1, c1, d1
  //              c0, c1, c2, c3                 a2, b2, c2, d2
  //              d0, d1, d2, d3                 a3, b3, c3, d3
  ////////////////////////////////////////////////////////////////////////////
  static inline void LoadPlanar_4x4(
	  float *in, 
	  unsigned __int32 stride, 
	  __m128 &out0, 
	  __m128 &out1, 
	  __m128 &out2, 
	  __m128 &out3)
  {	
    unsigned __int8 *_in = (unsigned __int8*) in;

    LoadPlanar_4x4(
      (float*) (_in+0*stride),
      (float*) (_in+1*stride),
      (float*) (_in+2*stride),
      (float*) (_in+3*stride),
      out0,
      out1,
      out2,
      out3);
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				LoadPlanar
  // PARAMETERS:	in - pointer to input arrays with 3 sequential floating 
  //                   point data elements
  //							stride - sizeof(structure) for the interleaved data
  //							out0 -> out2 - receive the transposed data elements from 3 
  //                             sequential structures starting at "in"
  //              fourth - value to stick in the fourth data element of each 
  //                       output __m128
  // RETURNS:			none
  // DESCRIPTION:	Simultaneously loads and transposes data.
  //
  // input data:  a0, a1, a2		output data: out0 = a0, b0, c0, fourth
  //              b0, b1, b2                 out1 = a1, b1, c1, fourth
  //              c0, c1, c2                 out2 = a2, b2, c2, fourth 
  ////////////////////////////////////////////////////////////////////////////
  static inline void LoadPlanar_3x3(
	  float *in0,
    float *in1, 
    float *in2,
	  __m128 &out0, 
	  __m128 &out1, 
	  __m128 &out2,
    float fourth = 1.0f)
  {	
	  ASSERT( in0 );
	  ASSERT( in1 );
	  ASSERT( in2 );
  
    __m128 f = _mm_set_ps1(fourth);

    // could get away with two temporary vars, but let's go easy on the compiler
    __m128 t0, t1, t2, t3;	  

    t0 = _mm_loadl_pi(f,  (__m64*) in0);
    t0 = _mm_loadh_pi(t0, (__m64*) in1);                // t0 = a0 b0 a1 b1
    t1 = _mm_loadl_pi(f,  (__m64*) in2);                // t1 = a2 b2 f  f
    out0 = _mm_shuffle_ps(t0, t1, 0x88);                // 0x88 = 10 00 10 00
    out1 = _mm_shuffle_ps(t0, t1, 0xdd);                // 0xdd = 11 01 11 01

    t2 = _mm_loadh_pi(f, (__m64*) (in0+2));
    t2 = _mm_load_ss(in1+2);                            // t2 = c0 f f c1
    t3 = f;
    t3 = _mm_load_ss(in2+2);                            // t3 = f  f f c2
    out2 = _mm_shuffle_ps(t2, t3, 0x3c);                // 0x3c = 00 11 11 00
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				LoadPlanar_3x3
  // PARAMETERS:	in - pointer to input arrays with 3 sequential floating 
  //                   point data elements
  //							stride - sizeof(structure) for the interleaved data
  //							out0 -> out2 - receive the transposed data elements from 3 
  //                             sequential structures starting at "in"
  //              fourth - value to stick in the fourth data element of each 
  //                       output __m128
  // RETURNS:			none
  // DESCRIPTION:	Simultaneously loads and transposes data.
  //
  // input data:  a0, a1, a2		output data: out0 = a0, b0, c0, fourth
  //              b0, b1, b2                 out1 = a1, b1, c1, fourth
  //              c0, c1, c2                 out2 = a2, b2, c2, fourth 
  ////////////////////////////////////////////////////////////////////////////
  static inline void LoadPlanar_3x3(
	  float *in, 
	  unsigned __int32 stride, 
	  __m128 &out0, 
	  __m128 &out1, 
	  __m128 &out2,
    float fourth = 1.0f)
  {	
    unsigned __int8 *_in = (unsigned __int8*) in;

    LoadPlanar_3x3(
      (float*) (_in+0*stride),
      (float*) (_in+1*stride),
      (float*) (_in+2*stride),
      out0,
      out1,
      out2,
      fourth);
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				NormalizeVector3D
  // PARAMETERS:	x, y, z -- well, duh
  // RETURNS:			none
  // DESCRIPTION:	normalizes the four 3D vectors contained in x, y, z
  ////////////////////////////////////////////////////////////////////////////
  static inline void NormalizeVector_3D(__m128 x, __m128 y, __m128 z)
  {
    __m128 inverse_magnitude = RcpSqrt(_mm_add_ps(Sqr(x),_mm_add_ps(Sqr(y), Sqr(z))));

    x = _mm_mul_ps(x, inverse_magnitude);
    y = _mm_mul_ps(y, inverse_magnitude);
    z = _mm_mul_ps(z, inverse_magnitude);
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				PowApprox
  // PARAMETERS:	n - number to be raised to a power
  //              exponent - the power to raise n to
  // RETURNS:			
  // DESCRIPTION:	n^exponent: Quick but less accurate power function.
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 PowApprox(const __m128 &n, F32 exponent)
  {
    unsigned __int32 e = (unsigned __int32) (e + 0.5f);

    __m128 result = n;

	  for ( unsigned __int32 i = 1; i < e; i++ )
    {
	    result = _mm_mul_ps(result, n);
    }

    return result;
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				PowApprox2
  // PARAMETERS:	n - number to be raised to a power
  //              exponent - the power to raise n to -- must be a power of 2
  // RETURNS:			
  // DESCRIPTION:	n^exponent: Quick but less accurate power function.
  //              Use only when exponents are powers of 2.
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 PowApprox_2(const __m128 &n, F32 exponent)
  {
    unsigned __int32 e = (unsigned __int32) (exponent + 0.5f);

    __m128 result = n;

    for ( unsigned __int32 i = 2; i < e; i *= 2 )
    {
	    result = _mm_mul_ps(result, n);
    }

    return result;
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				Rcp
  // PARAMETERS:	n - four numbers to invert
  // RETURNS:			1/n
  // DESCRIPTION:	Calculates reciprocol using the Newton-Raphson method:
  //    
  //                    rcp_a = 2 * rcp(a) - a * rcp(a)^2
  //
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 Rcp(const __m128 &n)
  {
    __m128 t;

    t = _mm_rcp_ps(n);
  	t = _mm_sub_ps(_mm_add_ps(t,t),_mm_mul_ps(t, _mm_mul_ps(n,t)));	

    return t;
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				RcpSqrt
  // PARAMETERS:	n - four numbers to calculate reciprocol square root of
  // RETURNS:			1/(n^0.5)
  // DESCRIPTION:	Calculates reciprocol square root using Newton-Raphson method:
  //
  //                    rsqrt_a = 0.5 * rsqrt(a) * ( 3 - (rsqrt(a)^2))
  //
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 RcpSqrt(const __m128 &n)
  {
    __m128 t;
    
    t = _mm_rsqrt_ps(n);
	  t = _mm_mul_ps(_mm_set_ps1(0.5f), _mm_mul_ps(t, _mm_sub_ps(_mm_set_ps1(3.0f), _mm_mul_ps(n, _mm_mul_ps(t, t)))));

    return t;
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				Sqr
  // PARAMETERS:	n - four numbers to be squared
  // RETURNS:			n*n
  // DESCRIPTION:	Calculates square of n
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 Sqr(const __m128 &n) 
  {
    return _mm_mul_ps(n, n);
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				Abs
  // PARAMETERS:	n
  // RETURNS:			|n|
  // DESCRIPTION:	Calculates absolute values n.
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 Abs(const __m128 &n) 
  {
    const static unsigned __int32 m = 0x7fffffff;
    const static __m128 abs_mask = _mm_set_ps1(*((float*) &m));

    return _mm_and_ps(n, abs_mask);
//    return _mm_max_ps(n, _mm_mul_ps(_mm_set_ps1(-1.0f), n));
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				Dot
  // PARAMETERS:	x0, y0, z0, x1, y1, z1
  // RETURNS:			x0*x1 + y0*y1 + z0*z1
  // DESCRIPTION:	Calculates absolute values n.
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 Dot(const __m128 &x0, const __m128 &y0, const __m128 &z0, const __m128 &x1, const __m128 &y1, const __m128 &z1)
  {
    return _mm_add_ps(
             _mm_add_ps(_mm_mul_ps(x0, x1), _mm_mul_ps(y0, y1)),
             _mm_mul_ps(z0,z1));
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				TransformVector_3D
  // PARAMETERS:	vec - vector 
  //              m0, m1, m2, m3 - matrix
  // RETURNS:			transformed vector in __m128
  // DESCRIPTION:	Transforms 3D vector (x,y,z) using matrix formed by the 
  //              elements of m0, m1, m2, m3.
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 TransformVector_3D(float *v, const __m128 &m0, const __m128 &m1, const __m128 &m2, const __m128 &m3)
  {
    ASSERT( v );

    __m128 vec = _mm_loadu_ps(v); // _mm_loadu_ps instead of _mm_load_ps
    __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
    __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
    __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

    return _mm_add_ps(
             _mm_add_ps(_mm_mul_ps(x, m0), _mm_mul_ps(y, m1)),
             _mm_add_ps(_mm_mul_ps(z, m2), m3));
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				TransformVector_3D_Aligned
  // PARAMETERS:	vec - vector 
  //              m0, m1, m2, m3 - matrix
  // RETURNS:			transformed vector in __m128
  // DESCRIPTION:	Transforms 3D vector (x,y,z) using matrix formed by the 
  //              elements of m0, m1, m2, m3.
  ////////////////////////////////////////////////////////////////////////////
  static inline __m128 TransformVector_3D_Aligned(float *v, const __m128 &m0, const __m128 &m1, const __m128 &m2, const __m128 &m3)
  {
    ASSERT( v );

    __m128 vec = _mm_load_ps(v);  // _mm_load_ps instead of _mm_loadu_ps
    __m128 x = _mm_shuffle_ps(vec, vec, 0x00);
    __m128 y = _mm_shuffle_ps(vec, vec, 0x55);
    __m128 z = _mm_shuffle_ps(vec, vec, 0xaa);

    return _mm_add_ps(
             _mm_add_ps(_mm_mul_ps(x, m0), _mm_mul_ps(y, m1)),
             _mm_add_ps(_mm_mul_ps(z, m2), m3));
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				Transpose_4x4
  // PARAMETERS:	r0 -> r3 -- four rows of data to be transposed
  // RETURNS:			none
  // DESCRIPTION:	
  // 
  //   input data: r0 = x0 y0 z0 w0   output data: r0 = x0 x1 x2 x3
  //               r1 = x1 y1 z1 w1                r1 = y0 y1 y2 y3
  //               r2 = x2 y2 z2 w2                r2 = z0 z1 z2 z3
  //               r3 = x3 y3 z3 w3                r3 = w0 w1 w2 w3
  //
  ////////////////////////////////////////////////////////////////////////////
  static inline void Transpose_4x4(__m128 &r0, __m128 &r1, __m128 &r2, __m128 &r3)
  {
    __m128 t0, t1, t2, t3;

    t0 = _mm_unpacklo_ps(r0, r1);               // t0 = y1 y0 x1 x0
    t1 = _mm_unpacklo_ps(r2, r3);               // t1 = y3 y2 x3 x2
    t2 = _mm_unpackhi_ps(r0, r1);               // t2 = w1 w0 z1 z0
    t3 = _mm_unpackhi_ps(r2, r3);               // t3 = w3 w2 z3 z2

#if 1
    r0 = _mm_shuffle_ps(t0, t1, 0x44);
    r1 = _mm_shuffle_ps(t0, t1, 0xee);
    r2 = _mm_shuffle_ps(t2, t3, 0x44);
    r3 = _mm_shuffle_ps(t2, t3, 0xee);
#else
    r0 = _mm_movelh_ps(t0, t1);                 // r0 = x3 x2 x1 x0
    r1 = _mm_movehl_ps(t1, t0);                 // r1 = y3 y2 y1 y0
    r2 = _mm_movelh_ps(t2, t3);                 // r2 = z3 z2 z1 z0
    r3 = _mm_movehl_ps(t3, t2);                 // r3 = w3 w2 w1 w0
#endif
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				Transpose_4x4
  // PARAMETERS:	r0 -> r3 -- four rows of data to be transposed
  // RETURNS:			none
  // DESCRIPTION:	
  // 
  //   input data: r0 = x0 y0 z0 w0   output data: r0 = x0 x1 x2 x3
  //               r1 = x1 y1 z1 w1                r1 = y0 y1 y2 y3
  //               r2 = x2 y2 z2 w2                r2 = z0 z1 z2 z3
  //               r3 = x3 y3 z3 w3                r3 = w0 w1 w2 w3
  //
  ////////////////////////////////////////////////////////////////////////////
  static inline void TransposeAndStore_4x4(__m128 &r0, __m128 &r1, __m128 &r2, __m128 &r3, float *out0, float *out1, float *out2, float *out3)
  {
    __m128 t0, t1, t2, t3;

    t0 = _mm_unpacklo_ps(r0, r1);               // t0 = y1 y0 x1 x0
    t1 = _mm_unpacklo_ps(r2, r3);               // t1 = y3 y2 x3 x2
    t2 = _mm_unpackhi_ps(r0, r1);               // t2 = w1 w0 z1 z0
    t3 = _mm_unpackhi_ps(r2, r3);               // t3 = w3 w2 z3 z2

    _mm_storel_pi((__m64*) out0 + 0, t0);
    _mm_storel_pi((__m64*) out0 + 1, t1);

    _mm_storeh_pi((__m64*) out1 + 0, t0);
    _mm_storeh_pi((__m64*) out1 + 1, t1);
    
    _mm_storel_pi((__m64*) out2 + 0, t2);
    _mm_storel_pi((__m64*) out2 + 1, t3);
    
    _mm_storeh_pi((__m64*) out3 + 0, t2);
    _mm_storeh_pi((__m64*) out3 + 1, t3);
  }

  ////////////////////////////////////////////////////////////////////////////
  // NAME:				XmmToDword
  // PARAMETERS:	xmm - __m128 to be converted to a dword
  //              max - value to clip xmm to
  // RETURNS:			dword with 
  // DESCRIPTION:	
  // 
  // NOTE: After using this you must issue and emms instruction.
  //
  ////////////////////////////////////////////////////////////////////////////
  static inline unsigned __int32 Xm2mToDword(__m128 xmm, __m128 max)
  {
    __m128 _xmm = _mm_min_ps(xmm, max);
    __m64 rg = _mm_cvt_ps2pi(_xmm);
    __m64 ba = _mm_cvt_ps2pi(_mm_movehl_ps(_xmm, _xmm));
	  return _m_to_int(_m_por(_m_punpcklwd(rg, ba), _m_psllwi(_m_punpckhwd(rg, ba), 8)));
  }
};

// xmm macros from for functions in XmmUtil
#define LOAD_PLANAR_4X4(i0,i1,i2,i3,o0,o1,o2,o3)      XmmUtil::LoadPlanar_4x4(i0,i1,i2,i3,o0,o1,o2,o3)
#define LOAD_PLANAR_3X3(i0,i1,i3,o0,o1,o2,f)          XmmUtil::LoadPlanar_3x3(i0,i1,i2,o0,o1,o2,f)
#define LOAD_PLANAR_4X4_STRIDED(i,s,o0,o1,o2,o3)      XmmUtil::LoadPlanar_4x4(i,s,o0,o1,o2,o3)
#define LOAD_PLANAR_3X3_STRIDED(i,s,o0,o1,o2,f)       XmmUtil::LoadPlanar_3x3(i,s,o0,o1,o2,f)
#define NORM_VEC_3D(x,y,z)                            XmmUtil::NormalizeVector_3D(x,y,z)
#define POW_APPROX(n,e)                               XmmUtil::PowApprox(n,e)
#define POW_APPROX_2(n,e)                             XmmUtil::PowApprox_2(n,e)
#define RCP(n)                                        XmmUtil::Rcp(n)
#define RCP_SQRT(n)                                   XmmUtil::RcpSqrt(n)
#define SQR(n)                                        XmmUtil::Sqr(n)
#define ABS(n)                                        XmmUtil::Abs(n)
#define DOT(x0,y0,z0,x1,y1,z1)                        XmmUtil::Dot(x0,y0,z0,x1,y1,z1)
#define TRANSFORM_VEC_3D(v,m0,m1,m2,m3)               XmmUtil::TransformVector_3D(v,m0,m1,m2,m3)
#define TRANSFORM_VEC_3D_ALIGNED(v,m0,m1,m2,m3)       XmmUtil::TransformVector_3D(v,m0,m1,m2,m3)
#define TRANSPOSE_4X4(r0,r1,r2,r3)                    XmmUtil::Transpose_4x4(r0,r1,r2,r3)
#define TRANSPOSE_STORE_4X4(r0,r1,r2,r3,o0,o1,o2,o3)  XmmUtil::TransposeAndStore_4x4(r0,r1,r2,r3,o0,o1,o2,o3)
#define XMM_TO_DWORD(xmm,max)                         XmmUtil::XmmToDword(xmm,max)

#else

// this is basically here as a forward declaration
typedef float __m128[4];

#endif _USE_INTEL_COMPILER

#endif __DO_XMM_BUILD

#endif __XMMUTIL_H