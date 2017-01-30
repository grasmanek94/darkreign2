///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Parameter System
//
// 20-SEP-1999
//


#ifndef __PARAM_H
#define __PARAM_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varsys.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Param
//
namespace Param
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Base
  //
  class Base
  {
  protected:

    enum 
    {
      TYPE_VAR,
      TYPE_CONST
    } type;

  public:


  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class String
  //
  class String : public Base
  {
  private:

    ::String value;
    VarString var;

  public:

    // Constructors
    String(FScope *fScope, void *context = NULL);
    String(FScope *fScope, const char *dVal, void *context = NULL);
    String(const char *name, FScope *fScope, void *context = NULL);
    String(const char *name, FScope *fScope, const char *dVal, void *context = NULL);

  protected:

    // Setup the string
    void Set(const char *param, void *context);

  public:

    // Operator
    operator const char *();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Ident
  //
  class Ident : public Base
  {
  private:

    GameIdent value;
    VarString var;

  public:

    // Constructors
    Ident(FScope *fScope, void *context = NULL);
    Ident(FScope *fScope, const char *dVal, void *context = NULL);
    Ident(const char *name, FScope *fScope, void *context = NULL);
    Ident(const char *name, FScope *fScope, const char *dVal, void *context = NULL);

  protected:

    // Setup the string
    void Set(const char *param, void *context);

  public:

    // Operators
    operator const char *();
    operator const GameIdent &();
    operator U32 ();

    // Explicit
    const char * GetStr();
    const GameIdent & GetIdent();
    U32 GetCrc();

  };



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Integer
  //
  class Integer : public Base
  {
  private:

    S32 value;
    VarInteger var;

  public:

    // Constructors
    Integer(FScope *fScope, void *context = NULL);
    Integer(FScope *fScope, S32 dVal, void *context = NULL);
    Integer(const char *name, FScope *fScope, void *context = NULL);
    Integer(const char *name, FScope *fScope, S32 dVal, void *context = NULL);

  protected:

    // Setup the var
    void Set(const char *param, void *context);

    // Setup the const
    void Set(S32 param);

  public:

    // Operator
    operator S32();

    // Explicit
    S32 GetS32();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Float
  //
  class Float : public Base
  {
  private:

    F32 value;
    VarFloat var;

  public:

    // Constructors
    Float(FScope *fScope, void *context = NULL);
    Float(FScope *fScope, F32 dVal, void *context = NULL);
    Float(const char *name, FScope *fScope, void *context = NULL);
    Float(const char *name, FScope *fScope, F32 dVal, void *context = NULL);

  protected:

    // Setup the var
    void Set(const char *param, void *context);

    // Setup the const
    void Set(F32 param);

  public:

    // Operator
    operator F32();

    // Explicit
    F32 GetF32();

  };
  
}

#endif