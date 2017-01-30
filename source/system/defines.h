///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Standard Definitions
//


#ifndef __DEFINES_H
#define __DEFINES_H


//
// Windows Specific
//
#ifdef _WIN32

  //
  // Warnings we don't want
  //
  #pragma warning(disable: 4201 4505 4121 4514 4512 4786 4702 4710 4711)

  //
  // Warnings we want to be errors
  //
  #pragma warning(error: 4715)

#endif


//
// Definitions
//
#ifdef TRUE
  #undef TRUE
#endif

#ifdef FALSE
  #undef FALSE
#endif

#define STDCALL                 __stdcall
#define FASTCALL                __fastcall
#define NAKED                   __declspec(naked)
#define THREAD                  __declspec(thread)
#define NORETURN                __declspec(noreturn)
#define CDECL                   __cdecl
#define DLL_EXPORT              __declspec(dllexport)
#define DLL_IMPORT              __declspec(dllimport)


//
// Type Definitions
//

#ifndef NULL
  #define NULL                  0
#endif

#ifndef CH
  typedef unsigned short        CH;
#endif

#ifndef U8
  typedef unsigned char         U8;
#endif
                                
#ifndef S8                      
  typedef signed char           S8;
#endif                          
                                
#ifndef U16                     
  typedef unsigned short        U16;
#endif                          
                                
#ifndef S16                     
  typedef signed short          S16;
#endif                          
                                
#ifndef U32                     
  typedef unsigned long         U32;
#endif                          
                                
#ifndef S32                     
  typedef signed long           S32;
#endif                          
                                
#ifdef _WIN32                   
  #ifndef U64                   
    typedef unsigned __int64    U64;
  #endif

  #ifndef S64
    typedef signed __int64      S64;
  #endif
#endif

#ifdef _LINUX
  #ifndef U64
    typedef unsigned long long  U64;
  #endif

  #ifndef S64
    typedef signed long long    S64;
  #endif
#endif


typedef unsigned int            UI;
typedef signed int              SI;
typedef float                   F32;
typedef double                  F64;
typedef long double             F80;
typedef int                     Bool;
                                
#define FALSE                   0
#define TRUE                    1
                                

//
// Limits
//
// Minimum and maximum allowed values for each type
//
#define U8_MAX        255
#define U8_MIN        0
#define S8_MAX        127
#define S8_MIN        128

#define U16_MAX       65535
#define U16_MIN       0
#define S16_MAX       32767
#define S16_MIN       -32768

#define U32_MAX       4294967295
#define U32_MIN       0

#define S32_MAX       2147483647

// Moronic compiler bug workaround
#define S32_MIN       ((S32) 0x80000000)
//#define S32_MIN     -2147483647

#define UI_MAX       4294967295
#define UI_MIN       0
#define SI_MAX       2147483647

// Moronic compiler bug workaround
#define SI_MIN        ((SI) 0x80000000)
//#define SI_MIN      -2147483647

#define F32_MAX       3.402823466e+38F
#define F32_MIN       -3.402823466e+38F
#define F32_EPSILON   1.192092896e-07F
#define F32_MIN_POS   1.175494351e-38F
#define F32_MIN_MOD   1.0e-3F
#define F32_MAX_MOD   1.0e+20F

#define F64_MAX       1.7976931348623158e+308
#define F64_MIN       -1.7976931348623158e+308
#define F64_EPSILON   2.2204460492503131e-016
#define F64_MIN_POS   2.2250738585072014e-308


//
// Min/Max/Clamp Templates
//
template <class T>
T Min(T x, T y) { return ((x > y) ? y : x); }

template <class T>
T Max(T x, T y) { return ((x < y) ? y : x); }

template <class T>
T Clamp(T x, T y, T z) { return (Min(Max(x, y), z)); }


//
// Swap template
//
template <class T>
void Swap(T &x0, T &x1) { T tmp = x0; x0 = x1; x1 = tmp; }


#endif

