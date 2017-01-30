/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Operatorion
//
// 12-NOV-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "operation.h"
#include "arithmeticoperator.h"


/////////////////////////////////////////////////////////////////////////////
//
// Namespace Operation
//
namespace Operation
{

  //
  // Console operation
  //
  void Console(const char *var, const char *op, VarSys::VarItem *p)
  {
    // Get the var which we're going to perform arithmetic on
    VarSys::VarItem *item = VarSys::FindVarItem(var);

    if (item)
    {
      switch (item->type)
      {
        case VarSys::VI_INTEGER:
        {
          // Get the operator
          ArithmeticOperator::Integer oper(op);

          U32 param;

          switch (p->type)
          {
            case VarSys::VI_STRING:
            {
              VarSys::VarItem *var = VarSys::FindVarItem(p->Str());
              if (var)
              {
                param = var->Integer();
              }
              else
              {
                ERR_FATAL(("Could not find resolve var2"))
              }
              break;
            }

            case VarSys::VI_INTEGER:
              param = p->Integer();
              break;

            default:
              ERR_FATAL(("Expected an integer or a var name"))
              break;
          }

          // Perform the operation
          item->SetInteger(oper(item->Integer(), param));
          break;
        }

        case VarSys::VI_FPOINT:
        {
          // Get the operator
          ArithmeticOperator::Float oper(op);

          F32 param;

          switch (p->type)
          {
            case VarSys::VI_STRING:
            {
              VarSys::VarItem *var = VarSys::FindVarItem(p->Str());
              if (var)
              {
                param = var->Float();
              }
              else
              {
                ERR_FATAL(("Could not find resolve var2"))
              }
              break;
            }

            case VarSys::VI_INTEGER:
              param = F32(p->Integer());
              break;

            case VarSys::VI_FPOINT:
              param = p->Float();
              break;

            default:
              ERR_FATAL(("Expected a float, integer or a var name"))
              break;
          }

          // Perform the operation
          item->SetFloat(oper(item->Float(), param));
          break;
        }

        case VarSys::VI_STRING:
        {
          // Get the operator
          ArithmeticOperator::String oper(op);

          String param;

          switch (p->type)
          {
            case VarSys::VI_STRING:
            {
              VarSys::VarItem *var = VarSys::FindVarItem(p->Str());
              if (var)
              {
                param = var->Str();
              }
              else
              {
                // Var lookup failed use the string
                param = p->Str();
              }
              break;
            }

            default:
              ERR_FATAL(("Expected a string or a var name"))
              break;
          }

          // Perform the operation
          item->SetStr(oper(item->Str(), param));
          break;
        }

        default:
          ERR_CONFIG(("Op can only be performed on Integer, Float and String vars"))
      }
    }
    else
    {
      ERR_FATAL(("Could not resolve var1"))
    }
  }


  //
  // Function operation
  //
  void Function(FScope *fScope, void *context)
  {
    // Get the var which we're going to perform arithmetic on
    VarSys::VarItem *item = VarSys::FindVarItem(StdLoad::TypeString(fScope), context);

    if (item)
    {
      switch (item->type)
      {
        case VarSys::VI_INTEGER:
        {
          // Get the operator
          ArithmeticOperator::Integer oper(StdLoad::TypeString(fScope));

          U32 param;

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
                  param = var->Integer();
                }
                else
                {
                  fScope->ScopeError("Could not find resolve var2");
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
            fScope->ScopeError("Expected a third argument");
          }

          // Perform the operation
          item->SetInteger(oper(item->Integer(), param));
          break;
        }

        case VarSys::VI_FPOINT:
        {
          // Get the operator
          ArithmeticOperator::Float oper(StdLoad::TypeString(fScope));

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
                  fScope->ScopeError("Could not find resolve var2");
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
            fScope->ScopeError("Expected a third argument");
          }

          // Perform the operation
          item->SetFloat(oper(item->Float(), param));
          break;
        }

        case VarSys::VI_STRING:
        {
          // Get the operator
          ArithmeticOperator::String oper(StdLoad::TypeString(fScope));

          String param;

          // Get the next argument
          VNode *vNode = fScope->NextArgument();

          if (vNode)
          {
            switch (vNode->aType)
            {
              case VNode::AT_STRING:
              {
                const char *node = vNode->GetString();
                VarSys::VarItem *var = VarSys::FindVarItem(node, context);
                if (var && var->type == VarSys::VI_STRING)
                {
                  param = var->Str();
                }
                else
                {
                  param = node;
                }
                break;
              }

              default:
                fScope->ScopeError("Expected a string or a var name");
                break;
            }
          }
          else
          {
            fScope->ScopeError("Expected a third argument");
          }

          // Perform the operation
          item->SetStr(oper(item->Str(), param));
          break;
        }

        default:
          ERR_CONFIG(("Op can only be performed on Integer, Float and String vars [%s]", item->itemId.str))
      }
    }
    else
    {
      fScope->ScopeError("Could not resolve var1");
    }

  }

}
