///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// VarSys Loading and Saving
//
// 06-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "varfile.h"
#include "stdload.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace VarFile
//
namespace VarFile
{

  //
  // Save from the Var Scope into the FScope
  //
  void Save(FScope *fScope, VarSys::VarScope *vScope)
  {
    // Add all of the items in this vScope to the fScope
    BinTree<VarSys::VarItem>::Iterator i(&vScope->items);
    for (!i; *i; i++)
    {
      switch ((*i)->type)
      {
        case VarSys::VI_SCOPE:
        {
          // Add this scope to the fScope
          FScope *sScope = fScope->AddFunction("VarScope");
          sScope->AddArgString((*i)->itemId.str);
          Save(sScope, (*i)->scope.ptr);
          break;
        }

        case VarSys::VI_INTEGER:
        {
          FScope *sScope = fScope->AddFunction("VarInteger");
          sScope->AddArgString((*i)->itemId.str);
          sScope->AddArgInteger((*i)->Integer());
          break;
        }

        case VarSys::VI_FPOINT:   
        {
          FScope *sScope = fScope->AddFunction("VarFloat");
          sScope->AddArgString((*i)->itemId.str);
          sScope->AddArgFPoint((*i)->Float());
          break;
        }

        case VarSys::VI_STRING:
        {
          FScope *sScope = fScope->AddFunction("VarString");
          sScope->AddArgString((*i)->itemId.str);
          sScope->AddArgString((*i)->Str());
          break;
        }

        case VarSys::VI_BINARY:
        {
          FScope *sScope = fScope->AddFunction("VarBinary");
          sScope->AddArgString((*i)->itemId.str);
          StdSave::TypeU32(sScope, "Size", (*i)->BinarySize());
          StdSave::TypeBinary(sScope, (*i)->BinarySize(), (*i)->Binary());
          break;
        }

        default:
          // We don't save anything else
          break;
      }
    }
  }

  
  //
  // Load from the FScope into the Var Scope
  //
  void Load(FScope *fScope, const char *path)
  {
    FScope *sScope;

    // Create the scope
    VarSys::CreateScope(path);

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xA658D5D6: // "VarScope"
        {
          // Get the scope ident
          const char *ident = StdLoad::TypeString(sScope);

          // Compose the new path
          StrBuf<VARSYS_MAXVARPATH> newPath = path;
          Utils::Strcat(newPath.str, VARSYS_SCOPEDELIMSTR);
          Utils::Strcat(newPath.str, ident);

          // Create if not already found
          if (!VarSys::FindVarItem(newPath.str))
          {
            // Create the scope
            VarSys::CreateVar(path, ident)->InitScope();
          }

          // Load the scope contents
          Load(sScope, newPath.str);
          break;
        }

        case 0xC42A1C61: // "VarInteger"
        {
          // Load the data
          const char *ident = StdLoad::TypeString(sScope);
          U32 value = StdLoad::TypeU32(sScope);

          // Does the var already exist
          if (VarSys::VarItem *item = VarSys::FindVarItem(path, ident))
          {
            // Set the value
            item->SetInteger(value);
          }
          else
          {
            // Create a new var
            VarSys::CreateVar(path, ident)->InitInteger(value);
          }
          break;
        }

        case 0x2383C5BD: // "VarFloat"
        {
          // Load the data
          const char *ident = StdLoad::TypeString(sScope);
          F32 value = StdLoad::TypeF32(sScope);

          // Does the var already exist
          if (VarSys::VarItem *item = VarSys::FindVarItem(path, ident))
          {
            // Set the value
            item->SetFloat(value);
          }
          else
          {
            // Create a new var
            VarSys::CreateVar(path, ident)->InitFPoint(value);
          }
          break;
        }

        case 0x8C0A4128: // "VarString"
        {
          // Load the data
          const char *ident = StdLoad::TypeString(sScope);
          const char *value = StdLoad::TypeString(sScope);

          // Does the var already exist
          if (VarSys::VarItem *item = VarSys::FindVarItem(path, ident))
          {
            // Set the value
            item->SetStr(value);
          }
          else
          {
            // Create a new var
            VarSys::CreateVar(path, ident)->InitString(value);
          }
          break;
        }

        case 0x59CD7465: // "VarBinary"
        {
          // Load the data
          const char *ident = StdLoad::TypeString(sScope);
          U32 size = StdLoad::TypeU32(sScope, "Size");

          // Does the var already exist
          VarSys::VarItem *item = VarSys::FindVarItem(path, ident);

          if (item)
          {
            // Set the value
            item->SetBinary(size);
          }
          else
          {
            // Create a new var
            item = VarSys::CreateVar(path, ident);
            item->InitBinary(size);
          }

          // Load the data
          StdLoad::TypeBinary(sScope, size, const_cast<U8 *>(item->Binary()));

          break;
        }
      }
    }
  }

}
