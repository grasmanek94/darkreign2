///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Display Objectives
//

#ifndef __DISPLAY_OBJECTIVE_H
#define __DISPLAY_OBJECTIVE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


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
  class Item
  {
  public:

    // State of the item
    enum State
    {
      ACTIVE,
      ABANDONED,
      COMPLETED
    };

  private:

    // Name
    GameIdent name;

    // FScope
    FScope *fScope;

    // State
    State state;

    // Tree node
    NBinTree<Item>::Node node;

  public:

    // Constructor
    Item(FScope *fScope);
    Item(const char *name, FScope *fScope);

    // Destructor
    ~Item();

    // GetText
    const CH * GetText(State &state, Team *team);

    // Dump
    void Dump(Team *team);

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

  public:

    // Get the item name ident
    const GameIdent & GetName()
    {
      return (name);
    }

    // Abandon
    void Abandon()
    {
      state = ABANDONED;
    }

    // Complete
    void Complete()
    {
      state = COMPLETED;
    }

    // Incomplete
    void Incomplete()
    {
      state = ACTIVE;
    }

  public:

    friend class Set;

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Set
  //
  class Set
  {
  private:

    // List of active items
    NBinTree<Item> items;

    // Team
    Team *team;

  public:

    typedef NBinTree<Item>::Iterator Iterator;

  public:

    // Constructor
    Set();

    // Destructor
    ~Set();

    // Add an item to the set
    void AddItem(const GameIdent &item, FScope *fScope);

    // Remove an item from the set
    void RemoveItem(const GameIdent &item);

    // Complete an item in the set
    void CompleteItem(const GameIdent &item);

    // Complete an item in the set
    void IncompleteItem(const GameIdent &item);

    // Abandon an item in the set
    void AbandonItem(const GameIdent &item);

    // Dump the item to the console
    void DumpItem(const GameIdent &item);


    // Reset Iterator
    void ResetIterator(Iterator &iterator);

    // Iterate: iterate through the items
    Bool Iterate(Iterator &iterator, GameIdent &ident, const CH *&text, Item::State &state);


    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

  public:

    // SetTeam
    void SetTeam(Team *t)
    {
      team = t;
    }

    // Get the list of active items
    const NBinTree<Item> & GetItems()
    {
      return (items);
    }

  };

}

#endif
