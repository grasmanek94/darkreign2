/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Relational Operators
//
// 12-NOV-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "relationaloperator.h"


/////////////////////////////////////////////////////////////////////////////
//
// Specializations for const char *
//
namespace RelationalOperatorTest
{
  template<> Bool Greater(const char *value1, const char *value2)
  {
    return (Utils::Stricmp(value1, value2) > 0);
  }

  template<> Bool GreaterEqual(const char *value1, const char *value2)
  {
    return (Utils::Stricmp(value1, value2) >= 0);
  }

  template<> Bool Less(const char *value1, const char *value2)
  {
    return (Utils::Stricmp(value1, value2) < 0);
  }

  template<> Bool LessEqual(const char *value1, const char *value2)
  {
    return (Utils::Stricmp(value1, value2) <= 0);
  }

  template<> Bool Equal(const char *value1, const char *value2)
  {
    return (!Utils::Stricmp(value1, value2));
  }

  template<> Bool NotEqual(const char *value1, const char *value2)
  {
    return (Utils::Stricmp(value1, value2));
  }
}
