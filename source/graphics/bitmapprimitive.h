///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Bitmap graphics library
//
// 06-NOV-1997
//


#ifndef __BITMAPPRIMTIVE_H
#define __BITMAPPRIMTIVE_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace BitmapPrim
//
namespace BitmapPrim
{
  void Init();

  void STDCALL HLine8 (void *data, U32 length, U32 color);
  void STDCALL HLine16(void *data, U32 length, U32 color);
  void STDCALL HLine24(void *data, U32 length, U32 color);
  void STDCALL HLine32(void *data, U32 length, U32 color);

  void STDCALL VLine8 (void *data, U32 pitch, U32 len, U32 color);
  void STDCALL VLine16(void *data, U32 pitch, U32 len, U32 color);
  void STDCALL VLine24(void *data, U32 pitch, U32 len, U32 color);
  void STDCALL VLine32(void *data, U32 pitch, U32 len, U32 color);

}

#endif
