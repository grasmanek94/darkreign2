///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// String-Crc Templates
//
// 28-APR-1998
//


#ifndef __STRCRC_H
#define __STRCRC_H


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
// Template Class StrCrc
//
// Used to generate identifier types which contain a char string and its crc.
// When the string is directly modified, Update() can be called to calculate
// the new crc.  Otherwise just call Set(), or use the assignment operator, 
// and it will do it all for you.
//
// The crc is based on the lower case version of the string, but the original
// case is preserved in the string copy.
//
template <U32 STR_SIZE, class BASE = char> class StrCrc
{
public:

  // string value
  BASE str[STR_SIZE + 1];

  // crc value
  U32 crc;

  // Get the size of the strcrc
  U32 GetSize() const
  {
    return (STR_SIZE);
  }

  // calculates the crc from current string value
  void Update()
  {
    // calculate new crc
    crc = Crc::CalcStr(str);
  }

  // set a new string value and call Update
  void Set(const BASE *newStr)
  {
    // copy string
    Utils::Strmcpy(str, newStr, STR_SIZE);

    // update crc
    Update();
  }

  // Returns TRUE if the string is empty
  Bool Null()
  {
    return (*str == '\0');
  }

  // default constructor
  StrCrc<STR_SIZE, BASE>()
  {
    *str = '\0';
    crc = 0;
  }
  
  // constructor with string
  StrCrc<STR_SIZE, BASE>(const BASE *newStr)
  {
    Set(newStr);
  }

  // operator = (char *)
  StrCrc<STR_SIZE, BASE>& operator=(const BASE *newStr)
  {
    Set(newStr);  
    return (*this);
  }

  // operator = (StrCrc &)
  StrCrc<STR_SIZE, BASE>& operator=(StrCrc<STR_SIZE, BASE> &ident)
  {
    Set(ident.str); 
    return (*this);
  }

  // operator == (StrCrc &)
  int operator==(const StrCrc<STR_SIZE, BASE> &ident)
  {
    return (crc == ident.crc);    
  }

  // operator != (StrCrc &)
  int operator!=(const StrCrc<STR_SIZE, BASE> &ident)
  {
    return (crc != ident.crc);    
  }

  // operator<<
  friend ostream & operator<<(ostream &o, const StrCrc<STR_SIZE> &s)
  {
    return (o << s.str);
  }

};


#endif