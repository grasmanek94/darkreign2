///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Parameter System
//
// 20-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "param.h"
#include "stdload.h"


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


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class String
  //


  //
  // Constructors
  //
  String::String(FScope *fScope, void *context)
  {
    // Get the next parameter
    Set(StdLoad::TypeString(fScope), context);
  }

  String::String(FScope *fScope, const char *dVal, void *context)
  {
    // Get the next parameter using the default value
    Set(StdLoad::TypeStringD(fScope, dVal), context);
  }

  String::String(const char *name, FScope *fScope, void *context)
  {
    // Get the next parameter
    Set(StdLoad::TypeString(fScope, name), context);
  }

  String::String(const char *name, FScope *fScope, const char *dVal, void *context)
  {
    // Get the next parameter using the default value
    Set(StdLoad::TypeString(fScope, name, dVal), context);
  }

  void String::Set(const char *param, void *context)
  {
    VarSys::VarItem *item = VarSys::FindVarItem(param, context);

    // Is this a var ?
    if (item)
    {
      type = TYPE_VAR;
      var.item = item;

      if (item->type != VarSys::VI_STRING)
      {
        ERR_FATAL(("Var '%s' is not a string", param))
      }
    }
    else
    {
      type = TYPE_CONST;
      value = ::String(param);
    }
  }

  // Operator
  String::operator const char *()
  {
    switch (type)
    {
      case TYPE_CONST:
        return (value);
        break;

      case TYPE_VAR:
        return (var);
        break;

      default:
        ERR_FATAL(("Unknown Type"))
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Ident
  //


  //
  // Constructors
  //
  Ident::Ident(FScope *fScope, void *context)
  {
    // Get the next parameter
    Set(StdLoad::TypeString(fScope), context);
  }

  Ident::Ident(FScope *fScope, const char *dVal, void *context)
  {
    // Get the next parameter using the default value
    Set(StdLoad::TypeStringD(fScope, dVal), context);
  }

  Ident::Ident(const char *name, FScope *fScope, void *context)
  {
    // Get the next parameter
    Set(StdLoad::TypeString(fScope, name), context);
  }

  Ident::Ident(const char *name, FScope *fScope, const char *dVal, void *context)
  {
    // Get the next parameter using the default value
    Set(StdLoad::TypeString(fScope, name, dVal), context);
  }

  void Ident::Set(const char *param, void *context)
  {
    VarSys::VarItem *item = VarSys::FindVarItem(param, context);

    // Is this a var ?
    if (item)
    {
      type = TYPE_VAR;
      var.item = item;

      if (item->type != VarSys::VI_STRING)
      {
        ERR_FATAL(("Var '%s' is not a string", param))
      }
    }
    else
    {
      type = TYPE_CONST;
      value = param;
    }
  }

  // Operator
  Ident::operator const char *()
  {
    return (GetStr());
  }

  Ident::operator const GameIdent &()
  {
    return (GetIdent());
  }

  Ident::operator U32()
  {
    return (GetCrc());
  }

  // Explicit
  const char * Ident::GetStr()
  {
    switch (type)
    {
      case TYPE_CONST:
        return (value.str);
        break;

      case TYPE_VAR:
        return (var);
        break;

      default:
        ERR_FATAL(("Unknown Type"))
    }
  }

  const GameIdent & Ident::GetIdent()
  {
    switch (type)
    {
      case TYPE_CONST:
        return (value);
        break;

      case TYPE_VAR:
        value = var;
        return (value);
        break;

      default:
        ERR_FATAL(("Unknown Type"))
    }
  }

  U32 Ident::GetCrc()
  {
    switch (type)
    {
      case TYPE_CONST:
        return (value.crc);
        break;

      case TYPE_VAR:
        return (Crc::CalcStr(var));
        break;

      default:
        ERR_FATAL(("Unknown Type"))
    }
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Integer
  //

  // Constructors
  Integer::Integer(FScope *fScope, void *context)
  {
    // Is the next argument a string ?
    if (fScope->IsNextArgString())
    {
      // Get the next parameter
      Set(StdLoad::TypeString(fScope), context);
    }
    else
    {
      // Get the next parameter
      Set(StdLoad::TypeU32(fScope));
    }
  }

  Integer::Integer(FScope *fScope, S32 dVal, void *context)
  {
    // Is this next argument a string ?
    if (fScope->IsNextArgString())
    {
      // Get the next parameter
      Set(StdLoad::TypeString(fScope), context);
    }
    else
    {
      // Get the next parameter
      Set(StdLoad::TypeU32(fScope, dVal));
    }
  }

  Integer::Integer(const char *name, FScope *fScope, void *context)
  {
    FScope *sScope = fScope->GetFunction(name, NULL);

    if (sScope)
    {
      // Is the next argument a string ?
      if (sScope->IsNextArgString())
      {
        // Get the next parameter
        Set(StdLoad::TypeString(sScope), context);
      }
      else
      {
        // Get the next parameter
        Set(StdLoad::TypeU32(sScope));
      }
    }
    else
    {
      fScope->ScopeError("Could not find scope '%s'", name);
    }
  }

  Integer::Integer(const char *name, FScope *fScope, S32 dVal, void *context)
  {
    FScope *sScope = fScope->GetFunction(name, NULL);

    if (sScope)
    {
      // Is this next argument a string ?
      if (sScope->IsNextArgString())
      {
        // Get the next parameter
        Set(StdLoad::TypeString(sScope), context);
      }
      else
      {
        // Get the next parameter
        Set(StdLoad::TypeU32(sScope, dVal));
      }
    }
    else
    {
      Set(dVal);
    }
  }

  // Setup the var
  void Integer::Set(const char *param, void *context)
  {
    VarSys::VarItem *item = VarSys::FindVarItem(param, context);

    // Is this a var ?
    if (item)
    {
      type = TYPE_VAR;
      var.item = item;

      if (item->type != VarSys::VI_INTEGER)
      {
        ERR_FATAL(("Var '%s' is not an integer", param))
      }
    }
    else
    {
      ERR_FATAL(("Could not find var '%s'", param))
    }
  }

  // Setup the const
  void Integer::Set(S32 param)
  {
    type = TYPE_CONST;
    value = param;
  }

  // Operator
  Integer::operator S32()
  {
    return (GetS32());
  }

  // Explicit
  S32 Integer::GetS32()
  {
    switch (type)
    {
      case TYPE_CONST:
        return (value);
        break;

      case TYPE_VAR:
        return (var);
        break;

      default:
        ERR_FATAL(("Unknown Type"))
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Float
  //

  // Constructors
  Float::Float(FScope *fScope, void *context)
  {
    // Is the next argument a string ?
    if (fScope->IsNextArgString())
    {
      // Get the next parameter
      Set(StdLoad::TypeString(fScope), context);
    }
    else
    {
      // Get the next parameter
      Set(StdLoad::TypeF32(fScope));
    }
  }

  Float::Float(FScope *fScope, F32 dVal, void *context)
  {
    // Is this next argument a string ?
    if (fScope->IsNextArgString())
    {
      // Get the next parameter
      Set(StdLoad::TypeString(fScope), context);
    }
    else
    {
      // Get the next parameter
      Set(StdLoad::TypeF32(fScope, dVal));
    }
  }

  Float::Float(const char *name, FScope *fScope, void *context)
  {
    FScope *sScope = fScope->GetFunction(name, NULL);

    if (sScope)
    {
      // Is the next argument a string ?
      if (sScope->IsNextArgString())
      {
        // Get the next parameter
        Set(StdLoad::TypeString(sScope), context);
      }
      else
      {
        // Get the next parameter
        Set(StdLoad::TypeF32(sScope));
      }
    }
    else
    {
      fScope->ScopeError("Could not find scope '%s'", name);
    }
  }

  Float::Float(const char *name, FScope *fScope, F32 dVal, void *context)
  {
    FScope *sScope = fScope->GetFunction(name, NULL);

    if (sScope)
    {
      // Is this next argument a string ?
      if (sScope->IsNextArgString())
      {
        // Get the next parameter
        Set(StdLoad::TypeString(sScope), context);
      }
      else
      {
        // Get the next parameter
        Set(StdLoad::TypeF32(sScope, dVal));
      }
    }
    else
    {
      Set(dVal);
    }
  }

  // Setup the var
  void Float::Set(const char *param, void *context)
  {
    VarSys::VarItem *item = VarSys::FindVarItem(param, context);

    // Is this a var ?
    if (item)
    {
      type = TYPE_VAR;
      var.item = item;

      if (item->type != VarSys::VI_FPOINT)
      {
        ERR_FATAL(("Var '%s' is not a float", param))
      }
    }
    else
    {
      ERR_FATAL(("Could not find var '%s'", param))
    }
  }

  // Setup the const
  void Float::Set(F32 param)
  {
    type = TYPE_CONST;
    value = param;
  }

  // Operator
  Float::operator F32()
  {
    return (GetF32());
  }

  // Explicit
  F32 Float::GetF32()
  {
    switch (type)
    {
      case TYPE_CONST:
        return (value);
        break;

      case TYPE_VAR:
        return (var);
        break;

      default:
        ERR_FATAL(("Unknown Type"))
    }
  }

}
