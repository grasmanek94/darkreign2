///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// String-Crc Templates
//
// 28-APR-1998
//


#ifndef __STRBUF_H
#define __STRBUF_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#pragma warning (push, 3)
#include <ostream>
using std::ostream;
#pragma warning (pop)


///////////////////////////////////////////////////////////////////////////////
//
// Template StrBuf
//
template <U32 STR_SIZE, class BASE = char> class StrBuf
{
public:

  // String value
  BASE str[STR_SIZE + 1];

  // Get the size of the strbuf
  U32 GetSize() const
  {
    return (STR_SIZE);
  }

  // Set a new string value
  void Set(const BASE *newStr)
  {
    // Copy string
    Utils::Strmcpy(str, newStr, STR_SIZE);
  }

  // Returns TRUE if the string is empty
  Bool Null()
  {
    return (*str == '\0');
  }

  // Default constructor
  StrBuf<STR_SIZE, BASE>()
  {
    *str = 0;
  }
  
  // Constructor with string
  StrBuf<STR_SIZE, BASE>(const BASE *newStr)
  {
    Set(newStr);
  }

  // operator = (char *)
  StrBuf<STR_SIZE, BASE>& operator=(const BASE *newStr)
  {
    Set(newStr);  
    return (*this);
  }

  // operator = (StrBuf &)
  StrBuf<STR_SIZE, BASE>& operator=(StrBuf<STR_SIZE, BASE> &ident)
  {
    Set(ident.str); 
    return (*this);
  }

  // operator<<
  friend ostream & operator<<(ostream &o, const StrBuf<STR_SIZE> &s)
  {
    return (o << s.str);
  }

};

#endif