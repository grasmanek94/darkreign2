///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 11-AUG-1998
//


#ifndef __COMMON_TEAMLISTBOX_H
#define __COMMON_TEAMLISTBOX_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TeamListBox
  //
  class TeamListBox : public ICListBox
  {
  private:

    // Var for update notification
    IFaceVar *updateVar;

    // Callback function when var changes
    void Notify(IFaceVar *var);

    // BuildTeamList
    void BuildTeamList();

  public:

    TeamListBox(IControl *parent);
    ~TeamListBox();

    // Listbox is being activated
    Bool Activate();

    // Listbox is being deactivated
    Bool Deactivate();
  
    // Event handling function
    U32 HandleEvent(Event &e);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PlayerListBox
  //
  class PlayerListBox : public ICListBox
  {
  private:

    // PlayerListBox attributes
    enum
    {
      STYLE_KEYID         = (1 << 1),   // Key by the id
      STYLE_ALLRELATIONS  = (1 << 2),   // Show all relations (defaults to allies only)
      STYLE_SHOWCURRENT   = (1 << 3),   // Include the current player (not shown by default)
    };

    // Style
    U32 playerListStyle;

    // BuildPlayerList
    void BuildPlayerList();

  public:

    // Constructor
    PlayerListBox(IControl *parent);

    // SetStyleItem
    Bool SetStyleItem(const char *s, Bool toggle);

    // Event handling function
    U32 HandleEvent(Event &e);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SideListBox
  //
  class SideListBox : public ICListBox
  {
  private:

    // Random side ?
    Bool random;

    // BuildSideList
    void BuildSideList();

  public:

    SideListBox(IControl *parent) 
    : ICListBox(parent),
      random(0)
    {
    }

    // Event handling function
    U32 HandleEvent(Event &e);

    // Setup
    void Setup(FScope *fScope);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PersonalityListBox
  //
  class PersonalityListBox : public ICListBox
  {
  private:

    // Random personality ?
    Bool random;

    // BuildPersonalityList
    void BuildPersonalityList();

  public:

    PersonalityListBox(IControl *parent) 
    : ICListBox(parent),
      random(0)
    {
    }

    // Event handling function
    U32 HandleEvent(Event &e);

    // Setup
    void Setup(FScope *fScope);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RuleSetListBox
  //
  class RuleSetListBox : public ICListBox
  {
  private:

    // Only show public rule sets
    Bool pub;

    // BuildRuleSetList
    void BuildRuleSetList();

  public:

    RuleSetListBox(IControl *parent)
    : ICListBox(parent),
      pub(FALSE)
    {
    }

    // Setup this control
    void Setup(FScope *fScope);

    // Event handling function
    U32 HandleEvent(Event &e);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class WorldListBox
  //
  class WorldListBox : public ICListBox
  {
  private:

    // BuildList
    void BuildWorldList();

  public:

    WorldListBox(IControl *parent) : ICListBox(parent)
    {
    }

    // Event handling function
    U32 HandleEvent(Event &e);

  };
}

#endif
