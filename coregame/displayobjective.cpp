///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Display Objectives
//
// 11-AUG-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "displayobjective.h"
#include "babel.h"
#include "console.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace DisplayObjective
//
namespace DisplayObjective
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Item
  //


  //
  // Constructor
  //
  Item::Item(FScope *fScope)
  {
    LoadState(fScope);
  }


  //
  // Constructor
  //
  Item::Item(const char *name, FScope *fScope)
  : fScope(fScope->Dup()),
    state(ACTIVE),
    name(name)
  {
  }


  //
  // Destructor
  //
  Item::~Item()
  {
    delete fScope;
  }


  //
  // Console
  //
  const CH * Item::GetText(State &s, Team *team)
  {
    // Assign state
    s = state;

    // Firstly get the text key
    const char *key = StdLoad::TypeString(fScope, "Text");

    // Iterate to get the params

    // Process the parameters first
    U32 params[10];
    CH buffers[10][256];
    U32 numParam = 0;

    fScope->InitIterators();

    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xA494FF21: // "Param"
        {
          const char *p = StdLoad::TypeString(sScope);
          VarSys::VarItem *var = VarSys::FindVarItem(p, team);
          if (var)
          {
            Utils::Strcpy(buffers[numParam], var->GetUnicodeStringValue());
            params[numParam] = reinterpret_cast<U32>(buffers[numParam]);
            numParam++;
          }
          else
          {
            // Use the parameter directly
            Utils::Ansi2Unicode(buffers[numParam], 256, p);
            params[numParam] = reinterpret_cast<U32>(buffers[numParam]);
            numParam++;
          }
          break;
        }
      }
    }

    const CH *text;

    // Push the parameters onto the stack
    S32 p;
    for (p = numParam - 1; p >= 0; p--)
    {
      U32 val = params[p];

      __asm
      {
        push val
      }
    }

    // Translate
    text = TRANSLATE((key, numParam));
    
    // Pop params off the stack
    for (p = 0; p < (S32) numParam; p++)
    {
      __asm
      {
        pop eax
      }
    }

    return (text);
  }


  //
  // Dump
  //
  void Item::Dump(Team *team)
  {
    State state;
    CON_MSG((GetText(state, team)))
  }


  //
  // SaveState
  //
  void Item::SaveState(FScope *scope)
  {
    StdSave::TypeString(scope, "Name", name.str);
    StdSave::TypeU32(scope, "State", state);
    scope->AddFunction("FScope")->AddDup(fScope);
  }


  //
  // LoadState
  //
  void Item::LoadState(FScope *scope)
  {
    name = StdLoad::TypeString(scope, "Name");
    state = State(StdLoad::TypeU32(scope, "State"));
    
    if (FScope *sScope = scope->GetFunction("FScope")->NextFunction())
    {
      fScope = sScope->Dup();
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Set
  //

  //
  // Constructor
  //
  Set::Set()
  : items(&Item::node),
    team(team)
  {
  }


  //
  // Destructor
  //
  Set::~Set()
  {
    items.DisposeAll();
  }


  //
  // Add an item to the set
  //
  void Set::AddItem(const GameIdent &itemName, FScope *fScope)
  {
    // Check to see if this item is already in the tree
    if (items.Exists(itemName.crc))
    {
      fScope->ScopeError("Display Objective '%s' is already active", itemName.str);
    }

    // Add the new objective
    items.Add(itemName.crc, new Item(itemName.str, fScope));
  }


  //
  // Remove an item from the set
  //
  void Set::RemoveItem(const GameIdent &itemName)
  {
    Item *item = items.Find(itemName.crc);

    if (item)
    {
      items.Dispose(item);
    }
  }


  //
  // Complete an item in the set
  //
  void Set::CompleteItem(const GameIdent &itemName)
  {
    Item *item = items.Find(itemName.crc);

    if (item)
    {
      item->Complete();
    }
  }


  //
  // Complete an item in the set
  //
  void Set::IncompleteItem(const GameIdent &itemName)
  {
    Item *item = items.Find(itemName.crc);

    if (item)
    {
      item->Incomplete();
    }
  }


  //
  // Abandon an item in the set
  //
  void Set::AbandonItem(const GameIdent &itemName)
  {
    Item *item = items.Find(itemName.crc);

    if (item)
    {
      item->Abandon();
    }
  }


  //
  // Dump an item to the console
  //
  void Set::DumpItem(const GameIdent &itemName)
  {
    Item *item = items.Find(itemName.crc);

    if (item)
    {
      item->Dump(team);
    }
  }


  //
  // Reset iterator
  //
  void Set::ResetIterator(Iterator &iterator)
  {
    iterator.SetTree(&items);
  }


  //
  // Iterate: iterate through the items
  //
  Bool Set::Iterate(Iterator &iterator, GameIdent &ident, const CH *&text, Item::State &state)
  {
    Item *item = iterator++;

    if (item)
    {
      text = item->GetText(state, team);
      ident = item->name;
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // SaveState
  //
  void Set::SaveState(FScope *scope)
  {
    for (Iterator i(&items); *i; i++)
    {
      (*i)->SaveState(scope->AddFunction("Item"));
    }
  }


  //
  // LoadState
  //
  void Set::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x1AE0C78A: // "Item"
        {
          // Create a new item
          Item *item = new Item(sScope);

          // Add to the tree
          items.Add(item->GetName().crc, item);
          break;
        }
      }
    }
  }
}
