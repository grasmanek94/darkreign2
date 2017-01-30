///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// General Utility Functions
//
// 27-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include <float.h>


///////////////////////////////////////////////////////////////////////////////
//
// Extern data
//
double Float2Int::magic = 65536.0 * 65536.0 * 65536.0 * 16.0 * 1.5;    // max out the mantissa;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Utils - General Utility Functions
//
namespace Utils
{
  LOGDEFLOCAL("Utils")

  //
  // Internal Data
  //
  const char translateEncode[] = "0123456789ABCDEF";
  const U8   translateDecode[] = 
  {
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x1,0x2,0x3,0x4,0x5,0x6,0x7, 0x8,0x9,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0xA,0xB,0xC,0xD,0xE,0xF,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,

    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0,
    0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0, 0x0,0x0,0x0,0x0,0x0,0x0,0x0,0x0
  };

  // Base64 encoding table
  static const char base64Encode[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

  // Base64 decoding table
  static const U8 base64Decode[256] =
  {
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,  62,   0,   0,   0,  63, 
   52,  53,  54,  55,  56,  57,  58,  59,    60,  61,   0,   0,   0,   0,   0,   0, 
    0,   0,   1,   2,   3,   4,   5,   6,     7,   8,   9,  10,  11,  12,  13,  14, 
   15,  16,  17,  18,  19,  20,  21,  22,    23,  24,  25,   0,   0,   0,   0,   0, 
    0,  26,  27,  28,  29,  30,  31,  32,    33,  34,  35,  36,  37,  38,  39,  40, 
   41,  42,  43,  44,  45,  46,  47,  48,    49,  50,  51,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
    0,   0,   0,   0,   0,   0,   0,   0,     0,   0,   0,   0,   0,   0,   0,   0, 
  };

  // Base64 decode structure
  static union
  {
    U32 i;
    U8 ch[4];
  } base64data;

  //
  // Division
  //

  U32 Div(U32 upr, U32 lwr, U32 by)
  {
    __asm
    {
      mov edx, [upr]
      mov eax, [lwr]
      div [by]
      mov [by], eax
    }
    return (by);
  }

  void Div(U32 upr, U32 lwr, U32 by, U32 &quot, U32 &rem)
  {
    __asm
    {
      mov edx, [upr]
      mov eax, [lwr]
      div [by]
      mov [quot], eax
      mov [rem], edx
    }
  }

  S32 Div(S32 upr, U32 lwr, S32 by)
  {
    __asm
    {
      mov edx, [upr]
      mov eax, [lwr]
      idiv [by]
      mov [by], eax
    }
    return (by);
  }

  void Div(S32 upr, U32 lwr, S32 by, S32 &quot, S32 &rem)
  {
    __asm
    {
      mov edx, [upr]
      mov eax, [lwr]
      idiv [by]
      mov [quot], eax
      mov [rem], edx
    }
  }


  //
  // Strlen
  //
  U32 Strlen(const char *str)
  {
    ASSERT(str);
    return (strlen(str));
  }
  U32 Strlen(const CH *str)
  {
    ASSERT(str);
    return (wcslen(str));
  }


  //
  // Strcmp
  //
  S32 Strcmp(const char *str1, const char *str2)
  {
    ASSERT(str1 && str2);
    return (strcmp(str1, str2));
  }
  S32 Strcmp(const CH *str1, const CH *str2)
  {
    ASSERT(str1 && str2);
    return (wcscmp(str1, str2));
  }
  

  //
  // Stricmp
  //
  S32 Stricmp(const char *str1, const char *str2)
  {
    ASSERT(str1 && str2);
    return (stricmp(str1, str2));
  }
  S32 Stricmp(const CH *str1, const CH *str2)
  {
    ASSERT(str1 && str2);
    return (wcsicmp(str1, str2));
  }

  
  //
  // Strncmp
  //
  S32 Strncmp(const char *str1, const char *str2, U32 count)
  {
    ASSERT(str1 && str2);
    return (strncmp(str1, str2, count));
  }
  S32 Strncmp(const CH *str1, const CH *str2, U32 count)
  {
    ASSERT(str1 && str2);
    return (wcsncmp(str1, str2, count));
  }

  
  //
  // Strnicmp
  //
  S32 Strnicmp(const char *str1, const char *str2, U32 count)
  {
    ASSERT(str1 && str2);
    return (strnicmp(str1, str2, count));
  }
  S32 Strnicmp(const CH *str1, const CH *str2, U32 count)
  {
    ASSERT(str1 && str2);
    return (wcsnicmp(str1, str2, count));
  }

  
  //
  // Strcpy
  //
  char * Strcpy(char *dest, const char *src)
  {
    ASSERT(dest && src);
    return (strcpy(dest, src));
  }
  CH * Strcpy(CH *dest, const CH *src)
  {
    ASSERT(dest && src);
    return (wcscpy(dest, src));
  }

  
  //
  // Strncpy
  //
  char * Strncpy(char *dest, const char *src, U32 count)
  {
    ASSERT(dest && src);
    ASSERT(count >= 0);
    return (strncpy(dest, src, count));  
  }
  CH * Strncpy(CH *dest, const CH *src, U32 count)
  {
    ASSERT(dest && src);
    ASSERT(count >= 0);
    return (wcsncpy(dest, src, count));  
  }


  //
  // Strmcpy
  //
  char * Strmcpy(char *dest, const char *src, U32 count)
  {
    ASSERT(dest && src);
    ASSERT(count > 0);
    strncpy(dest, src, count);
    dest[count - 1] = '\0';
    return(dest);
  }
  CH * Strmcpy(CH *dest, const CH *src, U32 count)
  {
    ASSERT(dest && src);
    ASSERT(count > 0);
    wcsncpy(dest, src, count);
    dest[count - 1] = L'\0';
    return(dest);
  }


  //
  // Strcat
  //
  char * Strcat(char *dest, const char *src)
  {
    ASSERT(dest && src);
    return (strcat(dest, src));
  }
  CH * Strcat(CH *dest, const CH *src)
  {
    ASSERT(dest && src);
    return (wcscat(dest, src));
  }

  
  //
  // Strchr
  //
  char * Strchr(const char *str, char c)
  {
    ASSERT(str);
    return (strchr(str, c));
  }
  CH * Strchr(const CH *str, CH c)
  {
    ASSERT(str);
    return (wcschr(str, c));
  }

  
  //
  // Strrchr
  //
  char * Strrchr(const char *str, char c)
  {
    ASSERT(str);
    return (strrchr(str, c));
  }
  CH * Strrchr(const CH *str, CH c)
  {
    ASSERT(str);
    return (wcsrchr(str, c));
  }


  //
  // Strstr
  //
  char * Strstr(const char *str, const char *substr)
  {
    ASSERT(str && substr);
    return (strstr(str, substr));
  }
  CH * Strstr(const CH *str, const CH *substr)
  {
    ASSERT(str && substr);
    return (wcsstr(str, substr));
  }


  //
  // Strcspn
  //
  U32 Strcspn(const char *str, const char *charset)
  {
    ASSERT(str)
    ASSERT(charset)
    return (strcspn(str, charset));
  }
  U32 Strcspn(const CH *str, const CH *charset)
  {
    ASSERT(str)
    ASSERT(charset)
    return (wcscspn(str, charset));
  }


  //
  // Strlwr
  //
  char * Strlwr(char *str)
  {
    ASSERT(str);
    return (strlwr(str));
  }
  CH * Strlwr(CH *str)
  {
    ASSERT(str);
    return (wcslwr(str));
  }


  //
  // Strupr
  //
  char * Strupr(char *str)
  {
    ASSERT(str);
    return (strupr(str));
  }
  CH * Strupr(CH *str)
  {
    ASSERT(str);
    return (wcsupr(str));
  }


  //
  // Strdup
  //
  char * Strdup(const char *str)
  {
    ASSERT(str);

    U32 len = Strlen(str) + 1;
    char *s = new char[len];
    Memcpy(s, str, len);
    return s;
  }
  CH * Strdup(const CH *str)
  {
    ASSERT(str);

    U32 len = Strlen(str) + 1;
    CH *s = new CH[len];
    Memcpy(s, str, len * sizeof (CH));
    return s;
  }


  //
  // Strndup
  //
  char * Strndup(const char *str, U32 count)
  {
    ASSERT(str)
    ASSERT(count > 0)

    U32 len = Strlen(str) + 1;
    char *s;

    if (len >= count)
    {
      s = new char[count + 1];
      Memcpy(s, str, count);
      s[count] = '\0';
    }
    else
    {
      s = new char[len];
      Memcpy(s, str, len);
    }
    return (s);
  }
  CH * Strndup(const CH *str, U32 count)
  {
    ASSERT(str);

    U32 len = Strlen(str) + 1;
    CH *s;
    
    if (len >= count)
    {
      s = new CH[count + 1];
      Memcpy(s, str, (count) * sizeof (CH));
      s[count] = L'\0';
    }
    else
    {
      s = new CH[len];
      Memcpy(s, str, len * sizeof (CH));
    }
    return (s);
  }


  //
  // IsAlphaNum
  //
  Bool IsAlphaNum(char c)
  {
    return (isalnum(c));
  }
  Bool IsAlphaNum(CH c)
  {
    return (iswalnum(c));
  }

  
  //
  // IsAlpha
  //
  Bool IsAlpha(char c)
  {
    return (isalpha(c));
  }
  Bool IsAlpha(CH c)
  {
    return (iswalpha(c));
  }


  //
  // IsDigit
  //
  Bool IsDigit(char c)
  {
    return (isdigit(c));
  }
  Bool IsDigit(CH c)
  {
    return (iswdigit(c));
  }

  
  //
  // IsGraph
  //
  Bool IsGraph(char c)
  {
    return (isgraph(c));
  }
  Bool IsGraph(CH c)
  {
    return (iswgraph(c));
  }

  
  //
  // IsHexDigit
  //
  Bool IsHexDigit(char c)
  {
    return (isxdigit(c));
  }
  Bool IsHexDigit(CH c)
  {
    return (iswxdigit(c));
  }


  //
  // IsLower
  //
  Bool IsLower(char c)
  {
    return (islower(c));
  }
  Bool IsLower(CH c)
  {
    return (iswlower(c));
  }


  //
  // IsPrintable
  //
  Bool IsPrintable(char c)
  {
    return (isprint(c));
  }
  Bool IsPrintable(CH c)
  {
    return (iswprint(c));
  }


  //
  // IsPunctuation
  //
  Bool IsPunctuation(char c)
  {
    return (ispunct(c));
  }
  Bool IsPunctuation(CH c)
  {
    return (iswpunct(c));
  }


  //
  // IsWhiteSpace
  //
  Bool IsWhiteSpace(char c)
  {
    return (isspace(c));
  }
  Bool IsWhiteSpace(CH c)
  {
    return (iswspace(c));
  }


  //
  // IsUpper
  //
  Bool IsUpper(char c)
  {
    return (isupper(c));
  }
  Bool IsUpper(CH c)
  {
    return (iswupper(c));
  }

  //
  // Sprintf: safe sprintf replacement
  //
  S32 CDECL Sprintf(char *buffer, S32 bufSize, const char *format, ...)
  {
    va_list args;
    S32 count;

    // Store the number of characters written
    va_start(args, format);
    count = vsprintf(buffer, format, args);
    va_end(args);

    // Was the buffer exceeded?
    if (count >= bufSize || count < 0)
    {
      ERR_FATAL(("Sprintf exceeded buffer size [actual=%d buffer=%d]", count, bufSize));
    }

    return (count);
  }
  S32 CDECL Sprintf(CH *buffer, S32 bufSize, const CH *format, ...)
  {
    va_list args;
    S32 count;

    // Store the number of characters written
    va_start(args, format);
    count = vswprintf(buffer, format, args);
    va_end(args);

    // Was the buffer exceeded?
    if (count >= bufSize || count < 0)
    {
      ERR_FATAL(("Sprintf exceeded buffer size [actual=%d buffer=%d]", count, bufSize));
    }

    return (count);
  }


  //
  // ItoA
  //
  char * ItoA(int value, char *buffer, int radix)
  {
    ASSERT(buffer)
    return (itoa(value, buffer, radix));
  }
  CH * ItoA(int value, CH *buffer, int radix)
  {
    ASSERT(buffer)
    return (_itow(value, buffer, radix));
  }


  //
  // AtoI
  //
  int AtoI(const char *str)
  {
    ASSERT(str)
    return (atoi(str));
  }
  int AtoI(const CH *str)
  {
    ASSERT(str)
    return (_wtoi(str));
  }


  //
  // AtoF
  //
  F32 AtoF(const char *str)
  {
    ASSERT(str)
    return ((F32) atof(str));
  }


  //
  // Ansi2Unicode
  //
  int Ansi2Unicode(CH *str, U32 size, const char *ansi)
  {
    int ansilen = Min(Strlen(ansi), size - 1);
    int i = MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, ansi, ansilen, str, size);
    str[i++] = CH('\0');
    return (i);
  }


  //
  // Ansi2Unicode
  //
  int Ansi2Unicode(CH *str, U32 size, const char *ansi, U32 ansiLen)
  {
    int i = MultiByteToWideChar(GetACP(), MB_PRECOMPOSED, ansi, ansiLen, str, size);
    str[i++] = CH('\0');
    return (i);
  }



  //
  // Ansi2Unicode
  //
  const CH *Ansi2Unicode(const char *ansi)
  {
    static U16 buffer[256];
    Ansi2Unicode(buffer, 256, ansi);
    return (buffer);
  }


  //
  // Unicode2Ansi
  //
  int Unicode2Ansi(char *str, U32 size, const CH *unicode)
  {
    int unicodelen = Min(Strlen(unicode), size - 1);
    int i = WideCharToMultiByte(GetACP(), WC_COMPOSITECHECK, unicode, unicodelen, str, size, NULL, NULL);
    str[i++] = CH('\0');
    return (i);
  }


  //
  // Unicode2Ansi
  //
  const char *Unicode2Ansi(const CH *unicode)
  {
    static char buffer[256];
    Unicode2Ansi(buffer, 256, unicode);
    return (buffer);
  }


  //
  // Unicode2AnsiEncode
  //
  int Unicode2AnsiEncode(char *str, U32 size, const CH *unicode)
  {
    S32 unicodelen = Strlen(unicode);
    S32 count = Min<int>(unicodelen, (size >> 1) - 1);
    S32 bytes = count * 2;
    const char *src = (const char *)unicode;
    char *dst = str;

    while (bytes--)
    {
      if (isprint(*src))
      {
        *dst = *src;
      }
      else
      {
        *dst = '.';
      }
      src++;
      dst++;
    }
    *dst = 0;

    return (count);
  }


  //
  // Unicode2AnsiEncode
  //
  const char *Unicode2AnsiEncode(const CH *unicode)
  {
    static char buffer[256];
    Unicode2AnsiEncode(buffer, 256, unicode);
    return (buffer);
  }


  //
  // CharEncode: Converts src to text
  //
  void CharEncode(char *dst, const U8 *src, U32 srcSize, U32 dstSize)
  {
    ASSERT(dstSize > srcSize * 2)
    dstSize;

    while (srcSize--)
    {
      *(dst++) = translateEncode[*src & 0x0F];
      *(dst++) = translateEncode[(*(src++) >> 4) & 0x0F];
    }

    // Null terminate the string
    *dst = '\0';
  }



  //
  // CharEncode64: Converts src to text
  //
  void CharEncode64(char *dst, const U8 *src, U32 srcSize, U32 dstSize)
  {
    dstSize;

    ASSERT(dstSize * 3 > srcSize * 4)

    // High byte is never used
    base64data.ch[3] = 0;

    // Calculate number of whole runs
    S32 whole = srcSize / 3;
    S32 remainder = srcSize - (whole * 3);

    while (whole--)
    {
      // Move 3 bytes in
      base64data.ch[2] = *src++;
      base64data.ch[1] = *src++;
      base64data.ch[0] = *src++;

      // Encode 6 bits at a time
      for (int shift = 18, mask = 0xFC0000; mask; shift -= 6, mask >>= 6)
      {
        *dst++ = base64Encode[(base64data.i & mask) >> shift];
      }
    }

    // Clean up remainder
    switch (remainder)
    {
      case 1:
      {
        base64data.ch[2] = *src++;
        base64data.ch[1] = 0;
        base64data.ch[0] = 0;

        *dst++ = base64Encode[(base64data.i & 0xFC0000) >> 18];
        *dst++ = base64Encode[(base64data.i & 0x03F000) >> 12];
        *dst++ = '=';
        *dst++ = '=';
        break;
      }

      case 2:
      {
        base64data.ch[2] = *src++;
        base64data.ch[1] = *src++;
        base64data.ch[0] = 0;

        *dst++ = base64Encode[(base64data.i & 0xFC0000) >> 18];
        *dst++ = base64Encode[(base64data.i & 0x03F000) >> 12];
        *dst++ = base64Encode[(base64data.i & 0x000FC0) >>  6];
        *dst++ = '=';
        break;
      }
    }

    // Null terminate
    *dst = '\0';
  }


  //
  // CharDecode: Converts src to bin
  //
  void CharDecode(U8 *dst, const char *src, U32 srcSize, U32 dstSize)
  {
    ASSERT(dstSize >= srcSize / 2)
    dstSize;

    while (srcSize)
    {
      *(dst) = (U8) (translateDecode[*(src++)] & 0x0F);
      *(dst++) |= (translateDecode[*(src++)] << 4) & 0xF0;
      srcSize -= 2;
    }
  }


  //
  // CharDecode64: Converts src to bin
  //
  S32 CharDecode64(U8 *dst, const char *src, U32 srcSize, U32 dstSize)
  {
    dstSize;

    ASSERT((srcSize & 3) == 0)
    const U8 *orig = dst;

    // Calculate remainder
    const char *p = src + srcSize - 1;
    while (*p-- == '=')
    {
      srcSize--;
    }

    S32 whole = srcSize / 4;
    S32 remainder = srcSize & 3;

    // Process whole chunks of 4 characters -> 3 bytes
    while (whole--)
    {
      base64data.i  = (base64Decode[*src++] << 18);
      base64data.i += (base64Decode[*src++] << 12);
      base64data.i += (base64Decode[*src++] <<  6);
      base64data.i += (base64Decode[*src++]      );
    
      *dst++ = base64data.ch[2];
      *dst++ = base64data.ch[1];
      *dst++ = base64data.ch[0];
    }

    // Fill in remainder
    if (remainder)
    {
      base64data.i = 0;

      for (int i = remainder, shift = 18; i; i--, srcSize--, shift -= 6)
      {
        base64data.i += (base64Decode[*src++] << shift);
      }
      
      // Move in up to 3 bytes
      for (i = 0; i < remainder - 1; i++)
      {
        *dst++ = base64data.ch[2 - i];
      }
    }

    // Return number of bytes decoded
    S32 len = S32(dst - orig);
    ASSERT(len > 0 && len <= S32(dstSize))
    return (len);
  }


  //
  // MemoryDump: Dumps the given data to the log file
  //
  void MemoryDump(const U8 *data, U32 size, Bool error)
  {
    char buf[80];
    U32 offs = 0;

    // Clear buffer with spaces
    memset(buf, ' ', sizeof(buf));
    buf[35] = '-';
    buf[sizeof(buf)-1] = '\0';

    while (size)
    {
      // Print the offset
      Utils::StrFmtHex(buf + 2, 6, offs);
      buf[8] = ' ';

      // Print each byte
      for (U32 i = 0; i < 16; ++i, ++offs)
      {
        char *hexPtr = (buf + 12) + (i * 3);
        char *ascPtr = (buf + 62) + i;

        if (size)
        {
          U8 ch = data[offs];

          // Store hex value
          *(hexPtr    ) = Utils::translateEncode[(ch & 0xF0) >> 4];
          *(hexPtr + 1) = Utils::translateEncode[(ch & 0x0F)];

          // Store ASCII value
          *(ascPtr) = ch > ' ' ? ch : '.';

          // Next memory location
          --size;
        }
        else
        {
          // Clear location
          *(hexPtr    ) = ' ';
          *(hexPtr + 1) = ' ';
          *(ascPtr    ) = ' ';
        }
      }

      // Print out one line
      LOG(error ? Log::ERR : Log::DIAG, (buf))
    }
  }


  //
  // Local data
  //
  static char fmtBuf[35];


  //
  // StrFmtDec
  //
  // Convert an integer to a base 10 string
  //
  char *StrFmtDec(char *s, U32 width, S32 value, U32 flags)
  {
    Bool end    = FALSE;
    Bool neg    = FALSE;
    S32  comma  = 3;
    char *start = s, *p;
  
    if (value < 0)
    {
      neg = TRUE;
      value = -value;
    }

    // Use default buffer if none specified
    if (s == NULL)
    {
      ASSERT(width < sizeof(fmtBuf));
      start = fmtBuf;
    }

    p = start + width - 1;
    *(p + 1) = '\0';

    // Format the number
    while (p >= start)
    {
      if (!end)
      {
        *p-- = char((value % 10) + '0');
        value /= 10;
        end = value == 0;

        if (end)
        {
          // Prepend the negative sign
          if (neg && (p > start))
          {
            *p-- = '-';
          }
        }
        else
        {
          // Insert a comma
          if ((flags & STRFMT_COMMA) && (p > start) && (--comma == 0))
          {
            *p-- = ',';
            comma = 3;
          }
        }
      }
      else
      {
        *p-- = ' ';
      }
    }

    return start;
  }


  //
  // StrFmtHex
  //
  // Converts a signed int to a hexadecimal string
  //
  char *StrFmtHex(char *s, U32 width, U32 value, U32 flags)
  {
    Bool end    = FALSE;
    char *start = s, *p;

    // Use default buffer if not specified
    if (s == NULL)
    {
      ASSERT(width < sizeof(fmtBuf));
      start = fmtBuf;
    }

    p = start + width - 1;
    *(p + 1) = '\0';

    // Append 'h'
    if (flags & STRFMT_HEXSUFFIX)
    {
      *p-- = 'h';
    }

    // Format the number
    while (p >= start)
    {
      if (!end)
      {
        *p-- = translateEncode[value & 0xF];
        value >>= 4;
        end = (flags & STRFMT_HEXPREFIX) ? (value == 0) : FALSE;

        // Prepend '0x' if room
        if (end)
        {
          if (flags & STRFMT_HEXPREFIX)
          {
            if (p - start >= 2)
            {
              *p-- = 'x';
              *p-- = '0';
            }
          }
        }
      }
      else
      {
        *p-- = ' ';
      }
    }

    return start;
  }


  //
  // StrFmtFloat
  //
  // Converts a floating point value to a string
  //
  char *StrFmtFloat(char *s, U32 width, U32 decpt, F32 value, U32)
  {
    ASSERT(width > decpt);

    Bool end    = FALSE;
    Bool neg = FALSE;
    char *start = s, *p;

    // Use default buffer if not specified
    if (s == NULL)
    {
      ASSERT(width < sizeof(fmtBuf));
      start = fmtBuf;
    }

    p = start + width - 1;
    *(p + 1) = '\0';

    F64 fFrac, fInt;
    S32 iFrac, iInt;

    fFrac = modf(value, &fInt);

    // Format fractional part
    if (decpt)
    {
      U32 n = decpt;

      while (n--)
      {
        fFrac *= 10.0;
      }

      iFrac = Utils::FtoL(F32(fFrac));

      if (iFrac < 0)
      {
        iFrac = -iFrac;
      }

      while (decpt--)
      {
        *p-- = char((iFrac % 10) + '0');
        iFrac /= 10;
      }
      *p-- = '.';
    }

    // Format integer part
    iInt = Utils::FtoL(F32(fInt));

    if (iInt < 0)
    { 
      neg = TRUE;
      iInt = -iInt;
    }

    while (p >= start)
    {
      if (!end)
      {
        *p-- = char((iInt % 10) + '0');
        iInt /= 10;
        end = iInt == 0;

        if (end && neg && p > start)
        {
          *p-- = '-';
        }
      }
      else
      {
        *p-- = ' ';
      }
    }

    return start;
  }


  //
  // MakeFile : File Maker
  //
  void MakeFile(char *dst, U32 dstSize, const char *file, const char *ext)
  {
    ASSERT(dst)
    ASSERT(file)
    ASSERT(ext)

    S32 size; 

    // File
    size = Strlen(file);
    Strmcpy(dst, file, dstSize);
    dstSize -= size;
    dst += size;

    // Ext
    size = Strlen(ext);
    Strmcpy(dst, ext, dstSize);
  }


  //
  // MakePath: Path Builder
  //
  // Add the dir, file and ext in that order to dst checking the size as we go
  //
  void MakePath(char *dst, U32 dstSize, const char *dir, const char *file, const char *ext)
  {
    S32 size; 

    // Dir
    if (dir && *dir)
    {
      size = Strlen(dir);
      Strmcpy(dst, dir, dstSize);
      dstSize -= size;
      dst += size;

      // If there is no trailing '\\' then append on
      if ((Strlen(dir) > 0) || (*(dst - 1) != '\\'))
      {
        *(dst++) = '\\';
        dstSize--;
      }
    }

    // File
    if (file && *file)
    {
      size = Strlen(file);
      Strmcpy(dst, file, dstSize);
      dstSize -= size;
      dst += size;
    }

    // Ext
    if (ext && *ext)
    {
      size = Strlen(ext);
      Strmcpy(dst, ext, dstSize);
    }
  }


  //
  // ChopPath: return a pointer to the file name in a file path
  //
  char *ChopPath( const char *path)
  {
    char *fileName = (char *)path;

    // find the first slash
    char *slash;
    for (slash = fileName; slash[0] != '\\' && slash[0] != '/'; slash++)
    {
      if (slash[0] == '\0')
      {
        return fileName;
      }
    }

    // find the last slash
    char *p;
    for (p = slash; p[0] != '\0'; p++)
    {
      if (p[0] == '\\' || p[0] == '/')
      {
        slash = p;
      }
    }
    fileName = slash + 1;

    return fileName;
  }


  //
  // FindExt: return a pointer to the path's extension
  //
  char *FindExt( const char *path)
  {
    char *p;

    // find the last '.'
    for (p = (char *)path + strlen(path); p != path; p--)
    {
      if (p[0] == '.')
      {
        return p;
      }
    }
    return NULL;
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace FP - Floating point functions
  //
  namespace FP
  {
    // Internal data
    U32 tmpCW;


    //
    // Set truncation mode, return old control word
    //
    NAKED U16 FASTCALL SetTruncMode()
    {
      __asm
      {
        // 1. Store control word into ECX
        fstcw   tmpCW

        // 2. Store control word into ECX
        mov     ecx, tmpCW

        // 3. Backup control word into EAX as the return value
        mov     eax, ecx

        // 4. Set truncation mode, bit 10=1 11=1
        or      ecx, 0x00000C00

        // 5. Load control word
        mov     tmpCW, ecx

        // 6. Load control word
        fldcw   word ptr tmpCW

        // 7. Return
        ret
      }
    }


    //
    // Set rounding up mode, return old control word
    //
    NAKED U16 FASTCALL SetRoundUpModeProc()
    {
      __asm
      {
        // 1. Store control word into ECX
        fstcw   tmpCW

        // 2. Store control word into ECX
        mov     ecx, tmpCW

        // 3. Backup control word into EAX as the return value
        mov     eax, ecx

        // 4. Set truncation mode, bit 10=0 11=1
        and     ecx, 0x0000FBFF
        or      ecx, 0x00000800

        // 5. Load control word
        mov     tmpCW, ecx

        // 6. Load control word
        fldcw   word ptr tmpCW

        // 7. Return
        ret
      }
    }


    //
    // Set rounding down mode, return old control word
    //
    NAKED U16 FASTCALL SetRoundDownModeProc()
    {
      __asm
      {
        // 1. Store control word into ECX
        fstcw   tmpCW

        // 2. Store control word into ECX
        mov     ecx, tmpCW

        // 3. Backup control word into EAX as the return value
        mov     eax, ecx

        // 4. Set truncation mode, bit 10=1 11=0
        and     ecx, 0x0000F7FF
        or      ecx, 0x00000400

        // 5. Load control word
        mov     tmpCW, ecx

        // 6. Load control word
        fldcw   word ptr tmpCW

        // 7. Return
        ret
      }
    }


    //
    // Set nearest rounding mode, return old control word
    //
    NAKED U16 FASTCALL SetNearestModeProc()
    {
      __asm
      {
        // 1. Store control word into ECX
        fstcw   tmpCW

        // 2. Store control word into ECX
        mov     ecx, tmpCW

        // 3. Backup control word into EAX as the return value
        mov     eax, ecx

        // 4. Set truncation mode, bit 10=0 11=0
        and     ecx, 0xFFFFF3FF

        // 5. Load control word
        mov     tmpCW, ecx

        // 6. Load control word
        fldcw   word ptr tmpCW

        // 7. Return
        ret
      }
    }


    //
    // Restore previous mode returned from SetRoundMode
    //
    NAKED void FASTCALL RestoreModeProc(U16)
    {
      __asm
      {
        // 1. Move arg0 into temp var
        mov     tmpCW, ecx

        // 2. Load control word
        fldcw   word ptr tmpCW

        // 3. Return
        ret
      }
    }


    //
    // Set precision to 53 bits, return old control word
    //
    NAKED U16 FASTCALL SetPrecision53()
    {
      __asm
      {
        // 1. Store control word into ECX
        fstcw   tmpCW

        // 2. Store control word into ECX
        mov     ecx, tmpCW

        // 3. Backup control word into EAX as the return value
        mov     eax, ecx

        // 4. Set precision to 53-bit, bit 8=0 9=1
        and     ecx, 0x0000FEFF
        or      ecx, 0x00000200

        // 5. Load control word
        mov     tmpCW, ecx

        // 6. Load control word
        fldcw   word ptr tmpCW

        // 7. Return
        ret
      }
    }



    //
    // Set precision to 64 bits, return old control word
    //
    NAKED U16 FASTCALL SetPrecision64()
    {
      __asm
      {
        // 1. Store control word into ECX
        fstcw   tmpCW

        // 2. Store control word into ECX
        mov     ecx, tmpCW

        // 3. Backup control word into EAX as the return value
        mov     eax, ecx

        // 4. Set precision to 64-bit, bit 8=1 9=1
        or      ecx, 0x00000300

        // 5. Load control word
        mov     tmpCW, ecx

        // 6. Load control word
        fldcw   word ptr tmpCW

        // 7. Return
        ret
      }
    }



    //
    // Mask (block) a floating point exception
    //
    U16 MaskException(U16 mask)
    {
      U16 cw, original;

      // Store control word
      __asm fstcw word ptr original;

      // Set desired bit
      cw = U16(original | (mask & 0x3F));

      // Load control word
      __asm fldcw word ptr cw;

      return (original);
    }


    //
    // Unmask (unblock) a floating point exception
    //
    U16 UnmaskException(U16 mask)
    {
      U16 cw, original;

      // Store control word
      __asm fstcw word ptr original;

      // Clear the desired bit
      cw = U16(original & ~(mask & 0x3F));

      // Load control word
      __asm fldcw word ptr cw;

      return (original);
    }


    //
    // Reset the FPU
    //
    void Reset()
    {
      // Reset status word
      _clear87();

      // Reset precision
      SetPrecision53();

      // Reset rounding
      SetTruncMode();
    }


    //
    // Get current state
    //
    const char *GetState()
    {
      static char buf[64];
      U16 ctl, stat;

      __asm
      {
        fstcw word ptr ctl
        fstsw word ptr stat
      }

      sprintf(buf, "PC: %d RC: %d INF: %d", (ctl >> 8) & 0x3, (ctl >> 10) & 0x3, (ctl >> 12) & 1);
      return (buf);
    }
  }

#ifdef DOQUICKFTOL
  U16 FP::FtoLRoundMode  = FP::ftolDOWN;
  double FP::FtoLRoundMagic = Float2Int::magic;
  double FP::FtoLRoundMagicLast = Float2Int::magic;
#endif

  //
  // Convert an F32 to a S32, using current rounding mode
  //
  // Call FP::SetTruncMode to set trunaction mode that is compatible
  // with a C type cast.
  //
  NAKED S32 STDCALL FastFtoLProc(F32)
  {
    __asm
    {
      // 1. Load F32 value from arg0
      fld     dword ptr [esp+4]

      // 2. Store S32 back into arg0
      fistp   dword ptr [esp+4]

      // 3. Copy S32 into EAX as the return value
      mov     eax, [esp+4]

      // 4. Return
      ret     4
    }
  }

  //
  // Convert an F32 to a S32, compatible with a C type cast
  //
  NAKED S32 STDCALL FtoL(F32)
  {
    static U16 tmpCW;

    __asm
    {
      // 1. Store control word into EAX
      fstcw   tmpCW

      // 7. Move float argument into FP register
      fld     dword ptr [esp+4]

      // 2. Store control word into EAX
      mov     ax, tmpCW

      // 3. Save original control word into ECX
      mov     ecx, eax

      // 4. Set truncation mode, bit 10=1 11=1
      or      eax, 0x00000C00

      // 5. Move adjusted control word back to temp var
      mov     tmpCW, ax

      // 6. Set control word
      fldcw   tmpCW

      // 8. Store float as integer
      fistp   dword ptr [esp+4]

      // 9. Restore original control word
      mov     tmpCW, cx

      // 11. Store result into EAX
      mov     eax, dword ptr [esp+4]

      // 10. Restore original control word
      fldcw   tmpCW

      // 12. Return
      ret     4
    }
  }

  //
  // Convert an F32 to a S32, compatible with a C type cast
  //
  NAKED S32 STDCALL FtoLUpProc(F32)
  {
    static U16 tmpCW;

    __asm
    {
      // 1. Store control word into EAX
      fstcw   tmpCW

      // 7. Move float argument into FP register
      fld     dword ptr [esp+4]

      // 2. Store control word into EAX
      mov     ax, tmpCW

      // 3. Save original control word into ECX
      mov     ecx, eax

      // 4. Set truncation mode, bit 10=0 11=1
      and     eax, 0x0000FBFF
      or      eax, 0x00000800

      // 5. Move adjusted control word back to temp var
      mov     tmpCW, ax

      // 6. Set control word
      fldcw   tmpCW

      // 8. Store float as integer
      fistp   dword ptr [esp+4]

      // 9. Restore original control word
      mov     tmpCW, cx

      // 11. Store result into EAX
      mov     eax, dword ptr [esp+4]

      // 10. Restore original control word
      fldcw   tmpCW

      // 12. Return
      ret     4
    }
  }

  //
  // Convert an F32 to a S32, compatible with a C type cast
  //
  NAKED S32 STDCALL FtoLDownProc(F32)
  {
    static U16 tmpCW;

    __asm
    {
      // 1. Store control word into EAX
      fstcw   tmpCW

      // 7. Move float argument into FP register
      fld     dword ptr [esp+4]

      // 2. Store control word into EAX
      mov     ax, tmpCW

      // 3. Save original control word into ECX
      mov     ecx, eax

      // 4. Set truncation mode, bit 10=1 11=0
      and     eax, 0x0000F7FF
      or      eax, 0x00000400

      // 5. Move adjusted control word back to temp var
      mov     tmpCW, ax

      // 6. Set control word
      fldcw   tmpCW

      // 8. Store float as integer
      fistp   dword ptr [esp+4]

      // 9. Restore original control word
      mov     tmpCW, cx

      // 11. Store result into EAX
      mov     eax, dword ptr [esp+4]

      // 10. Restore original control word
      fldcw   tmpCW

      // 12. Return
      ret     4
    }
  }


  //
  // Convert an F32 to a S32, compatible with a C type cast
  //
  NAKED S32 STDCALL FtoLNearestProc(F32)
  {
    static U16 tmpCW;

    __asm
    {
      // 1. Store control word into EAX
      fstcw   tmpCW

      // 7. Move float argument into FP register
      fld     dword ptr [esp+4]

      // 2. Store control word into EAX
      mov     ax, tmpCW

      // 3. Save original control word into ECX
      mov     ecx, eax

      // 4. Set nearest mode, bit 10=0 11=0
      and     eax, 0x0000F3FF

      // 5. Move adjusted control word back to temp var
      mov     tmpCW, ax

      // 6. Set control word
      fldcw   tmpCW

      // 8. Store float as integer
      fistp   dword ptr [esp+4]

      // 9. Restore original control word
      mov     tmpCW, cx

      // 11. Store result into EAX
      mov     eax, dword ptr [esp+4]

      // 10. Restore original control word
      fldcw   tmpCW

      // 12. Return
      ret     4
    }
  }

}