///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Display Objectives
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_displayobjectives.h"
#include "team.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DisplayObjectives
  //


  //
  // Constructor
  //
  DisplayObjectives::DisplayObjectives(IControl *parent) 
  : ICListBox(parent),
    colorActive(0.9f, 0.9f, 0.9f, 1.0f),
    colorCompleted(0.9f, 0.5f, 0.5f, 0.7f),
    colorAbandoned(0.5f, 0.5f, 0.9f, 0.7f)
  {
    listBoxStyle |= STYLE_WRAP;

    // Setup the poll interval
    SetPollInterval(50);
  }


  //
  // Destructor
  //
  DisplayObjectives::~DisplayObjectives()
  {
  }

  
  //
  // Setup this control from one scope function
  //
  void DisplayObjectives::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xADED6B39: // "ColorActive"
        StdLoad::TypeColor(fScope, colorActive);
        break;

      case 0x69E4B279: // "ColorCompleted"
        StdLoad::TypeColor(fScope, colorCompleted);
        break;

      case 0x07FC9E08: // "ColorAbandoned"
        StdLoad::TypeColor(fScope, colorAbandoned);
        break;

      default:
        ICListBox::Setup(fScope);
        break;
    }
  }


  //
  // Redraw self
  //
  void DisplayObjectives::Poll()
  {
    ICListBox::Rebuild *rebuild = PreRebuild();

    // Rebuild the list before redrawing
    DeleteAllItems();

    // Get the display team's DisplayObjective::Set
    ASSERT(Team::GetDisplayTeam())

    Team *team = Team::GetDisplayTeam();
    DisplayObjective::Set::Iterator iter;

    const CH *text;
    DisplayObjective::Item::State state;
    GameIdent ident;

    team->GetDisplayObjectives().ResetIterator(iter);
    while (team->GetDisplayObjectives().Iterate(iter, ident, text, state))
    {
      if (state == DisplayObjective::Item::ACTIVE)
      {
        if (ItemCount())
        {
          AddItem("-", new ICListBoxSpacer(this, 5));
        }
        AddTextItem("-", text, &colorActive);
      }
    }

    team->GetDisplayObjectives().ResetIterator(iter);
    while (team->GetDisplayObjectives().Iterate(iter, ident, text, state))
    {
      if (state == DisplayObjective::Item::COMPLETED)
      {
        if (ItemCount())
        {
          AddItem("-", new ICListBoxSpacer(this, 5));
        }
        AddTextItem("-", text, &colorCompleted);
      }
    }

    team->GetDisplayObjectives().ResetIterator(iter);
    while (team->GetDisplayObjectives().Iterate(iter, ident, text, state))
    {
      if (state == DisplayObjective::Item::ABANDONED)
      {
        if (ItemCount())
        {
          AddItem("-", new ICListBoxSpacer(this, 5));
        }
        AddTextItem("-", text, &colorAbandoned);
      }
    }

    PostRebuild(&rebuild);
  }

}
