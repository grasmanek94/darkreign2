///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// General Utility Functions
//
// 27-APR-1998
//

#ifndef __UTILS_H
#define __UTILS_H


// Fixed point defines
#define FIXED_TRUNC(shift, value) ((value) >> (shift))
#define FIXED_ROUND(shift, value) (((value) + (1 << ((shift) - 1))) >> (shift))


//#define DOQUICKFTOL

struct Float2Int
{
  // generate a U32 from a F32 via a neat exponent trick
  //
  union
  {
	  S32     i;
	  double  f;
  };

  // constructor
  //
  Float2Int() { }

  Float2Int( double _f)
  {
    f = _f;
  }

  // the magic number
  //
  static double magic;

  // just sign bit
  //
  struct Sign
  {
    union
    {
      F32 f;
      S32 i;
    };

    // constructor
    //
    Sign() {} 

    Sign( F32 _f)
    {
      f = _f;
    }
  };
};


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Utils - General Utility Functions
//
namespace Utils
{

  // Division
  U32 Div(U32 upr, U32 lwr, U32 by);
  void Div(U32 upr, U32 lwr, U32 by, U32 &quot, U32 &rem);
  S32 Div(S32 upr, U32 lwr, S32 by);
  void Div(S32 upr, U32 lwr, S32 by, S32 &quot, S32 &rem);


  // Strlen
  U32 Strlen(const char *str);
  U32 Strlen(const CH *str);

  // Strcmp
  S32 Strcmp(const char *str1, const char *str2);
  S32 Strcmp(const CH *str1, const CH *str2);
  
  // Stricmp
  S32 Stricmp(const char *str1, const char *str2);
  S32 Stricmp(const CH *str1, const CH *str2);

  // Strncmp
  S32 Strncmp(const char *str1, const char *str2, U32 count);
  S32 Strncmp(const CH *str1, const CH *str2, U32 count);

  // Strnicmp
  S32 Strnicmp(const char *str1, const char *str2, U32 count);
  S32 Strnicmp(const CH *str1, const CH *str2, U32 count);
  
  // Strcpy
  char * Strcpy(char *dest, const char *src);
  CH * Strcpy(CH *dest, const CH *src);

  // Strncpy
  char * Strncpy(char *dest, const char *src, U32 count);
  CH * Strncpy(CH *dest, const CH *src, U32 count);

  // Strmcpy
  char * Strmcpy(char *dest, const char *src, U32 count);
  CH * Strmcpy(CH *dest, const CH *src, U32 count);

  // Strcat
  char * Strcat(char *dest, const char *src);
  CH * Strcat(CH *dest, const CH *src);

  // Strchr
  char * Strchr(const char *str, char c);
  CH * Strchr(const CH *str, CH c);

  // Strrchr
  char * Strrchr(const char *str, char c);
  CH * Strrchr(const CH *str, CH c);

  // Strstr
  char * Strstr(const char *str, const char *substr);
  CH * Strstr(const CH *str, const CH *substr);

  // Strcspn
  U32 Strcspn(const char *str, const char *charset);
  U32 Strcspn(const CH *str, const CH *charset);

  // Strlwr
  char * Strlwr(char *str);
  CH * Strlwr(CH *str);

  // Sstrupr
  char * Strupr(char *str);
  CH * Strupr(CH *str);

  // Strdup
  char * Strdup(const char *str);
  CH * Strdup(const CH *str);

  // Strndup
  char * Strndup(const char *str, U32 count);
  CH * Strndup(const CH *str, U32 count);

  // IsAlphaNum
  Bool IsAlphaNum(char c);
  Bool IsAlphaNum(CH c);

  // IsAlpha
  Bool IsAlpha(char c);
  Bool IsAlpha(CH c);

  // IsDigit
  Bool IsDigit(char c);
  Bool IsDigit(CH c);

  // IsGraph
  Bool IsGraph(char c);
  Bool IsGraph(CH c);

  // IsHexDigit
  Bool IsHexDigit(char c);
  Bool IsHexDigit(CH c);

  // IsLower
  Bool IsLower(char c);
  Bool IsLower(CH c);

  // IsPrintable
  Bool IsPrintable(char c);
  Bool IsPrintable(CH c);

  // IsPunctuation
  Bool IsPunctuation(char c);
  Bool IsPunctuation(CH c);

  // IsWhiteSpace
  Bool IsWhiteSpace(char c);
  Bool IsWhiteSpace(CH c);

  // IsUpper
  Bool IsUpper(char c);
  Bool IsUpper(CH c);

  //
  // Memset
  //
  inline void* Memset(void *dest, int c, U32 count)
  {
    ASSERT(dest);
    return (memset(dest, c, count));
  }

  //
  // Memcpy
  //
  inline void* Memcpy(void *dest, const void *src, U32 count)
  {
    ASSERT(dest)
    ASSERT(src)
    return (memcpy(dest, src, count));
  }

  //
  // Memmove
  //
  inline void* Memmove(void *dest, const void *src, U32 count)
  {
    ASSERT(dest)
    ASSERT(src)
    return (memmove(dest, src, count));
  }

  // Sprintf
  S32 CDECL Sprintf(char *buffer, S32 bufSize, const char *format, ...);
  S32 CDECL Sprintf(CH *buffer, S32 bufSize, const CH *format, ...);

  // ItoA
  char * ItoA(int value, char *buffer, int radix);
  CH * ItoA(int value, CH *buffer, int radix);

  // AtoI
  int AtoI(const char *str);
  int AtoI(const CH *str);

  // AtoF
  F32 AtoF(const char *str);

  // Ansi2Unicode
  int Ansi2Unicode(CH *str, U32 size, const char *ansi);

  // Ansi2Unicode
  int Ansi2Unicode(CH *str, U32 size, const char *ansi, U32 ansiLen);

  // Ansi2Unicode
  const CH *Ansi2Unicode(const char *ansi);

  // Unicode2Ansi
  int Unicode2Ansi(char *str, U32 size, const CH *unicode);

  // Unicode2Ansi
  const char *Unicode2Ansi(const CH *unicode);

  // Unicode2AnsiEncode
  int Unicode2AnsiEncode(char *str, U32 size, const CH *unicode);

  // Unicode2AnsiEncode
  const char *Unicode2AnsiEncode(const CH *unicode);
  
  // CharEncode: Converts src to text
  void CharEncode(char *dst, const U8 *src, U32 srcSize, U32 dstSize);
  void CharEncode64(char *dst, const U8 *src, U32 srcSize, U32 dstSize);

  // CharDecode: Converts src to bin
  void CharDecode(U8 *dst, const char *src, U32 srcSize, U32 dstSize);
  S32 CharDecode64(U8 *dst, const char *src, U32 srcSize, U32 dstSize);

  // MemoryDump: Dumps the given data to the log file
  void MemoryDump(const U8 *src, U32 size, Bool error = FALSE);

  // Flags for the StrFmt functions
  enum
  {
    STRFMT_COMMA      = 0x0001,
    STRFMT_HEXPREFIX  = 0x0002,
    STRFMT_HEXSUFFIX  = 0x0004,
  };

  // Convert an integer to a string
  char *StrFmtDec(char *s, U32 width, S32 value, U32 flags = 0);

  // Converts a signed int to a hexadecimal string
  char *StrFmtHex(char *s, U32 width, U32 value, U32 flags = 0);

  // Converts a floating point value to a string
  char *StrFmtFloat(char *s, U32 width, U32 decpt, F32 value, U32 flags = 0);

  // MakeFile : File Maker
  void MakeFile(char *dst, U32 dstSize, const char *file, const char *ext);

  // MakePath: Path Builder
  void MakePath(char *dst, U32 dstSize, const char *dir, const char *file, const char *ext);

  // ChopPath: return a pointer to the file name in a file path
  char *ChopPath(const char *path);

  // FindExt: return a pointer to the path's extension
  char *FindExt(const char *path);


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace FP - Floating point functions
  //
  namespace FP
  {
    // Floating point exception masks
    const U16 EX_INVALID_OP  = 0x0001;
    const U16 EX_DENORM_OP   = 0x0002;
    const U16 EX_ZERODIVIDE  = 0x0004;
    const U16 EX_OVERFLOW    = 0x0008;
    const U16 EX_UNDERFLOW   = 0x0010;
    const U16 EX_PRECISION   = 0x0020;
    const U16 EX_ALL         = 0x003F;

    // Mask (block) a floating point exception
    U16 MaskException(U16 mask);

    // Unmask (unblock) a floating point exception
    U16 UnmaskException(U16 mask);

    // Set truncation mode, return old control word
    U16 FASTCALL SetTruncMode();

    // Set rounding up mode, return old control word
    U16 FASTCALL SetRoundUpModeProc();

    // Set rounding down mode, return old control word
    U16 FASTCALL SetRoundDownModeProc();

    // Set nearest rounding mode, return old control word
    U16 FASTCALL SetNearestModeProc();

    // Restore previous mode returned from SetRoundMode
    void FASTCALL RestoreModeProc(U16);

    // Reset the FPU
    void Reset();

    // Get current state
    const char *GetState();

#ifdef DOQUICKFTOL

    extern U16 FtoLRoundMode;
    extern double FtoLRoundMagic;
    extern double FtoLRoundMagicLast;

    enum FTOLROUNDMODE
    {
      ftolUP,
      ftolDOWN,
      ftolNEAREST
    };

    // Set truncation mode, return old control word
    inline U16 FASTCALL SetRoundUpMode()
    {
      U16 last = FtoLRoundMode;

      FtoLRoundMode = ftolUP;

      FtoLRoundMagic = Float2Int::magic + 0.9999999999; 

      return last;
    }
    inline U16 FASTCALL SetRoundDownMode()
    {
      U16 last = FtoLRoundMode;

      FtoLRoundMode = ftolDOWN;

      FtoLRoundMagic = Float2Int::magic; 

      return last;
    }
    inline U16 FASTCALL SetRoundNearestMode()
    {
      U16 last = FtoLRoundMode;

      FtoLRoundMode = ftolNEAREST;

      FtoLRoundMagic = Float2Int::magic + 0.5; 

      return last;
    }
    inline void FASTCALL RestoreMode(U16 last)
    {
      FtoLRoundMode = last;
    }

#else

    // Set truncation mode, return old control word
    inline U16 FASTCALL SetRoundUpMode()
    {
      return SetRoundUpModeProc();
    }
    inline U16 FASTCALL SetRoundDownMode()
    {
      return SetRoundDownModeProc();
    }
    inline U16 FASTCALL SetNearestMode()
    {
      return SetNearestModeProc();
    }
    inline void FASTCALL RestoreMode(U16 last)
    {
      RestoreModeProc(last);
    }

#endif

  }

  S32 STDCALL FastFtoLProc(F32);

  // Convert an F32 to a S32, compatible with a C type cast
  S32 STDCALL FtoL(F32);          // truncation
  S32 STDCALL FtoLUpProc(F32);
  S32 STDCALL FtoLDownProc(F32);
  S32 STDCALL FtoLNearestProc(F32);

#ifdef DOQUICKFTOL

  inline S32 STDCALL FtoLUp(F32 f)
  {
    Float2Int f2i( f + 0.9999999999f + Float2Int::magic);

    return f2i.i;
  }
  inline S32 STDCALL FtoLDown(F32 f)
  {
    Float2Int f2i( f + Float2Int::magic);

    return f2i.i;
  }
  inline S32 STDCALL FtoLNearest(F32 f)
  {
    Float2Int f2i( f + 0.5f + Float2Int::magic);

    return f2i.i;
  }
  inline S32 STDCALL FastFtoL(F32 f)
  {
    Float2Int f2i( f + FP::FtoLRoundMagic);

    return f2i.i;
  }

#else

  inline S32 STDCALL FtoLUp(F32 f)
  {
    return FtoLUpProc( f);
  }
  inline S32 STDCALL FtoLDown(F32 f)
  {
    return FtoLDownProc( f);
  }
  inline S32 STDCALL FtoLNearest(F32 f)
  {
    return FtoLNearestProc( f);
  }
  inline S32 STDCALL FastFtoL( F32 f)
  {
    return FastFtoLProc( f);
  }

#endif

  inline F32 FSign( F32 val)
  {
    return val < 0.0f ? -1.0f : 1.0f;
  }

  // Exports
  extern const char translateEncode[];
  extern const U8   translateDecode[];

};

#endif
