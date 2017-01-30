///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Multilanguage Babel
//
// 18-MAR-1999
//


#ifndef __BABEL_H
#define __BABEL_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define TRANSLATE(x) Babel::Translate x


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Babel
//
namespace Babel
{
  // Translate
  const CH * CDECL Translate(const char *key, U32 params = 0, ...);

  // Translate
  const CH * Translate(va_list args, U32 params, const char *key);

  // Sprintf
  void CDECL Sprintf(CH *buffer, U32 size, const CH *format, U32 params = 0, ...);

  // Sprintf
  void Sprintf(CH *buffer, U32 size, va_list args, U32 params, const CH *format);

}


#endif
