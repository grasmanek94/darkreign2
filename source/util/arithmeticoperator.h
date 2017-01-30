/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Arithmetic Operators
//
// 12-NOV-1998
//

#ifndef __ARITHMETICOPERATOR_H
#define __ARITHMETICOPERATOR_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "stdload.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace ArithmeticOperator
//
namespace ArithmeticOperator
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Float
  //
  class Float
  {
  private:

    // Assign Operator
    static F32 Assign(F32, F32 value2)
    {
      return (value2);
    }

    // Add Operator
    static F32 Add(F32 value1, F32 value2) 
    { 
      return (value1 + value2);
    }

    // Subtract Operator
    static F32 Subtract(F32 value1, F32 value2) 
    { 
      return (value1 - value2);
    }
  
    // Multiply Operator
    static F32 Multiply(F32 value1, F32 value2) 
    { 
      return (value1 * value2);
    }

    // Divide Operator
    static F32 Divide(F32 value1, F32 value2) 
    { 
      return (value1 / value2);
    }

    // Modulus Operator
    static F32 Modulus(F32 value1, F32 value2)
    {
      return (F32(fmod(value1, value2)));
    }

    // Function pointer to operator to use
    F32 (*oper)(F32, F32);

  public:

    // Constructor
    Float()
    : oper(NULL)
    {
    };

    // Constructor
    Float(const char *operName)
    {
      Load(operName);
    }

    // Constructor
    Float(FScope *fScope)
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
        case 0x5408AB08: // "="
          oper = Assign;
          break;

        case 0x029F3DCA: // "+"
          oper = Add;
          break;

        case 0x18197078: // "-"
          oper = Subtract;
          break;

        case 0x065E207D: // "*"
          oper = Multiply;
          break;

        case 0x119B4B16: // "/"
          oper = Divide;
          break;

        case 0x3E119DC0: // "%"
          oper = Modulus;
          break;

        default:
          ERR_CONFIG(("Unknown floating point arithmetic operation '%s'", operName.str))
          break;
      }
    }

    // Function operator
    F32 operator()(F32 value1, F32 value2)
    {
      ASSERT(oper)
      return (oper(value1, value2));
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Integer
  //
  class Integer
  {
  private:

    // Assign Operator
    static U32 Assign(U32, U32 value2)
    {
      return (value2);
    }

    // Add Operator
    static U32 Add(U32 value1, U32 value2) 
    { 
      return (value1 + value2);
    }

    // Subtract Operator
    static U32 Subtract(U32 value1, U32 value2) 
    { 
      return (value1 - value2);
    }
  
    // Multiply Operator
    static U32 Multiply(U32 value1, U32 value2) 
    { 
      return (value1 * value2);
    }

    // Divide Operator
    static U32 Divide(U32 value1, U32 value2) 
    { 
      return (value1 / value2);
    }

    // Modulus Operator
    static U32 Modulus(U32 value1, U32 value2) 
    { 
      return (value1 % value2);
    }

    // And Operator
    static U32 And(U32 value1, U32 value2) 
    { 
      return (value1 & value2);
    }

    // Or Operator
    static U32 Or(U32 value1, U32 value2) 
    { 
      return (value1 | value2);
    }

    // Xor Operator
    static U32 Xor(U32 value1, U32 value2) 
    { 
      return (value1 ^ value2);
    }


    // Function pointer to operator to use
    U32 (*oper)(U32, U32);

  public:

    // Constructor
    Integer()
    : oper(NULL)
    {
    };

    // Constructor
    Integer(const char *operName)
    {
      Load(operName);
    }

    // Constructor
    Integer(FScope *fScope)
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
        case 0x5408AB08: // "="
          oper = Assign;
          break;

        case 0x029F3DCA: // "+"
          oper = Add;
          break;

        case 0x18197078: // "-"
          oper = Subtract;
          break;

        case 0x065E207D: // "*"
          oper = Multiply;
          break;

        case 0x119B4B16: // "/"
          oper = Divide;
          break;

        case 0x3E119DC0: // "%"
          oper = Modulus;
          break;

        case 0x3352BB19: // "&"
          oper = And;
          break;

        case 0x644FC6C8: // "|"
          oper = Or;
          break;

        case 0xF5EE4B46: // "^"
          oper = Xor;
          break;

        default:
          ERR_CONFIG(("Unknown integer arithmetic operation '%s'", operName.str))
          break;
      }
    }

    // Function operator
    U32 operator()(U32 value1, U32 value2)
    {
      ASSERT(oper)
      return (oper(value1, value2));
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class String
  //
  class String
  {
  private:

    // Assign Operator
    static ::String Assign(::String, ::String value2)
    {
      return (value2);
    }

    // Function pointer to operator to use
    ::String (*oper)(::String, ::String);

  public:

    // Constructor
    String()
    : oper(NULL)
    {
    };

    // Constructor
    String(const char *operName)
    {
      Load(operName);
    }

    // Constructor
    String(FScope *fScope)
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
        case 0x5408AB08: // "="
          oper = Assign;
          break;

        default:
          ERR_CONFIG(("Unknown string arithmetic operation '%s'", operName.str))
          break;
      }
    }

    // Function operator
    ::String operator()(::String value1, ::String value2)
    {
      ASSERT(oper)
      return (oper(value1, value2));
    }

  };

}

#endif
