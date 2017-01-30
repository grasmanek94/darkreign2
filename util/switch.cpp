/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Switch
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "comparison.h"
#include "relationaloperator.h"


/////////////////////////////////////////////////////////////////////////////
//
// Namespace Switch
//
namespace Switch
{

  struct Value
  {
    enum Type
    {
      STRING,
      INTEGER,
      FLOAT
    } type;

    union
    {
      String *string;
      S32 integer;
      F32 fpoint;
    };

    Value(const char *string)
    : type(STRING),
      string(new String(string))
    {
    }

    Value(S32 integer)
    : type(INTEGER),
      integer(integer)
    {
    }

    Value(F32 fpoint)
    : type(FLOAT),
      fpoint(fpoint)
    {
    }

    ~Value()
    {
      switch (type)
      {
        case STRING:
          delete string;
          break;

        case INTEGER:
          break;

        case FLOAT:
          break;
      }
    }
  };


  //
  // Get the value of a switch statement
  //
  Value & GetValue(FScope *fScope, void *context)
  {
    // Get the var which we're going to perform relation on
    const char *var = StdLoad::TypeString(fScope);
    VarSys::VarItem *item = VarSys::FindVarItem(var, context);

    if (item)
    {
      switch (item->type)
      {
        case VarSys::VI_INTEGER:
          return (*new Value(item->Integer()));

        case VarSys::VI_FPOINT:
          return (*new Value(item->Float()));

        case VarSys::VI_STRING:
          return (*new Value(item->Str()));

        default:
          fScope->ScopeError("Switch can only be performed on Integer, Float and String vars [%s]", item->itemId.str);
      }
    }
    else
    {
      fScope->ScopeError("Could not resolve var1", var);
    }
  }


  //
  // Compare the value of a switch statement
  //
  Bool CompareValue(Value &value, FScope *fScope, void *context)
  {
    switch (value.type)
    {
      case Value::STRING:
      {
        RelationalOperator<const char *> oper("==");

        const char *param;

        // Get the next argument
        VNode *vNode = fScope->NextArgument();

        if (vNode)
        {
          switch (vNode->aType)
          {
            case VNode::AT_STRING:
            {
              param = vNode->GetString();

              // Dereference if first character is '*'
              if (param && (*param == '*'))
              {
                VarSys::VarItem *var = VarSys::FindVarItem(param + 1, context);
                if (var && var->type == VarSys::VI_STRING)
                {
                  param = var->Str();
                }
                else
                {
                  fScope->ScopeError("Could not find resolve var2", var);
                }
              }
              break;
            }

            default:
              fScope->ScopeError("Expected string argument or var name");
          }
        }
        else
        {
          fScope->ScopeError("Expected an argument");
        }
        return (oper(*value.string, param));
      }

      case Value::INTEGER:
      {
        // Get the next argument
        VNode *vNode = fScope->NextArgument();

        RelationalOperator<S32> oper("==");
        S32 param;

        if (vNode)
        {
          switch (vNode->aType)
          {
            case VNode::AT_STRING:
            {
              VarSys::VarItem *var = VarSys::FindVarItem(vNode->GetString(), context);
              if (var)
              {
                param = var->Integer();
              }
              else
              {
                fScope->ScopeError("Could not find resolve var2", var);
              }
              break;
            }

            case VNode::AT_INTEGER:
              param = vNode->GetInteger();
              break;

            default:
              fScope->ScopeError("Expected an integer or a var name");
              break;
          }
        }
        else
        {
          fScope->ScopeError("Expected an argument");
        }

        // Perform the operation
        return (oper(value.integer, param));
      }

      case Value::FLOAT:
      {
        // Get the operator
        RelationalOperator<F32> oper("");

        F32 param;

        // Get the next argument
        VNode *vNode = fScope->NextArgument();

        if (vNode)
        {
          switch (vNode->aType)
          {
            case VNode::AT_STRING:
            {
              VarSys::VarItem *var = VarSys::FindVarItem(vNode->GetString(), context);
              if (var)
              {
                param = var->Float();
              }
              else
              {
                fScope->ScopeError("Could not find resolve var2", var);
              }
              break;
            }

            case VNode::AT_INTEGER:
              param = F32(vNode->GetInteger());
              break;

            case VNode::AT_FPOINT:
              param = vNode->GetFPoint();
              break;

            default:
              fScope->ScopeError("Expected a float, integer or a var name");
              break;
          }
        }
        else
        {
          fScope->ScopeError("Expected an argument");
        }

        // Perform the operation
        return (oper(value.fpoint, param));
      }

      default:
        ERR_FATAL(("Unknown value type %d", value.type))
    }
  }


  //
  // Dispose of a value
  //
  void DisposeValue(Value &value)
  {
    delete &value;
  }

}
