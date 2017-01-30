///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Cyclic Redundancy Checking
// 1-DEC-1997
//

#ifndef __CRC_H
#define __CRC_H


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
typedef U32 CRC;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Crc
//
namespace Crc
{

  //
  // Calculate the CRC of given buffer (32bit)
  //
  U32 Calc(const void *buff, U32 len, U32 crc = 0);

  //
  // As for Calc but using lowercase ascii characters
  //
  U32 CalcLower(const void *buff, U32 len, U32 crc = 0);

  //
  // As for CalcLower, but stops at null terminator
  //
  U32 CalcStr(const char *str, U32 crc = 0);

  //
  // CalcStr for Unicode strings
  //
  U32 CalcStr(const CH *str, U32 crc = 0);

}

#endif
