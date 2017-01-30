/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Comparison
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "comparison.h"
#include "relationaloperator.h"


/////////////////////////////////////////////////////////////////////////////
//
// Namespace Comparison
//
namespace Comparison
{

  //
  // Function operation
  //
  Bool Function(FScope *fScope, void *context)
  {
    // Get the var which we're going to perform relation on
    const char *var = StdLoad::TypeString(fScope);
    VarSys::VarItem *item = VarSys::FindVarItem(var, context);

    if (item)
    {
      switch (item->type)
      {
        case VarSys::VI_INTEGER:
        {
          if (fScope->IsNextArgString())
          {
            // Get the operator
            RelationalOperator<U32> oper(StdLoad::TypeString(fScope));

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
              fScope->ScopeError("Expected a third argument");
            }

            // Perform the operation
            return (oper(item->Integer(), param));
          }
          else
          {
            RelationalOperator<U32> oper("!=");
            return (oper(item->Integer(), 0));
          }
          break;
        }

        case VarSys::VI_FPOINT:
        {
          // Get the operator
          RelationalOperator<F32> oper(StdLoad::TypeString(fScope));

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
            fScope->ScopeError("Expected a third argument");
          }

          // Perform the operation
          return (oper(item->Float(), param));
        }

        case VarSys::VI_STRING:
        {
          RelationalOperator<const char *> oper(StdLoad::TypeString(fScope));

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
            fScope->ScopeError("Expected a third argument");
          }
          return (oper(item->Str(), param));
        }

        default:
          ERR_CONFIG(("If can only be performed on Integer, Float and String vars [%s]", item->itemId.str))
      }
    }
    else
    {
      fScope->ScopeError("Could not resolve var1", var);
    }

  }

}
