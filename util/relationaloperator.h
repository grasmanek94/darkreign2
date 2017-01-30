/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Relational Operators
//
// 12-NOV-1998
//

#ifndef __RELATIONALOPERATOR_H
#define __RELATIONALOPERATOR_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "stdload.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace RelationalOperatorTest
//
namespace RelationalOperatorTest
{
  // Greater Operator
  template<class TYPE> Bool Greater(TYPE value1, TYPE value2) 
  { 
    return (value1 > value2); 
  }

  // Greater or Equal Operator
  template<class TYPE> Bool GreaterEqual(TYPE value1, TYPE value2) 
  { 
    return (value1 >= value2); 
  }

  // Less Operator
  template<class TYPE> Bool Less(TYPE value1, TYPE value2) 
  { 
    return (value1 < value2); 
  }

  // Less or Equal Operator
  template<class TYPE> Bool LessEqual(TYPE value1, TYPE value2) 
  { 
    return (value1 <= value2); 
  }

  // Equal Operator
  template<class TYPE> Bool Equal(TYPE value1, TYPE value2) 
  { 
    return (value1 == value2); 
  }

  // Not Equal Operator
  template<class TYPE> Bool NotEqual(TYPE value1, TYPE value2) 
  { 
    return (value1 != value2); 
  }

  // Specializations for const char *
  template<> Bool Greater(const char *value1, const char *value2);
  template<> Bool GreaterEqual(const char *value1, const char *value2);
  template<> Bool Less(const char *value1, const char *value2);
  template<> Bool LessEqual(const char *value1, const char *value2);
  template<> Bool Equal(const char *value1, const char *value2);
  template<> Bool NotEqual(const char *value1, const char *value2);
};


/////////////////////////////////////////////////////////////////////////////
//
// Template RelationalOperator
//
template <class TYPE> class RelationalOperator
{
private:

  // Function pointer to operator to use
  Bool (*oper)(TYPE, TYPE);

public:

  // Constructor
  RelationalOperator()
  {
  };

  // Constructor
  RelationalOperator(const char *operName)
  {
    Load(operName);
  }

  // Constructor
  RelationalOperator(FScope *fScope)
  {
    Load(fScope->GetFunction("Operator"));
  }

  // Load
  void Load(FScope *fScope)
  {
    Load(StdLoad::TypeString(fScope));
  }

  // Load
  void Load(const GameIdent &operName)
  {
    switch (operName.crc)
    {
      case 0x594B8DD1: // ">"
        oper = RelationalOperatorTest::Greater;
        break;

      case 0x45DB2100: // ">="
        oper = RelationalOperatorTest::GreaterEqual;
        break;

      case 0x50C9B6BF: // "<"
        oper = RelationalOperatorTest::Less;
        break;

      case 0xE529BF0F: // "<="
        oper = RelationalOperatorTest::LessEqual;
        break;

      case 0x37307ED3: // "=="
        oper = RelationalOperatorTest::Equal;
        break;

      case 0xE80D873B: // "!="
        oper = RelationalOperatorTest::NotEqual;
        break;

      default:
        ERR_CONFIG(("Unknown Operator '%s'", operName.str))
        break;
    }
  }

  // Function operator
  Bool operator()(TYPE value1, TYPE value2)
  {
    return (oper(value1, value2));
  }

};

#endif
