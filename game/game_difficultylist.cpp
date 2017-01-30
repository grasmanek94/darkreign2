///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Difficulty Setting List
//
// 11-FEB-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_difficultylist.h"
#include "difficulty.h"
#include "babel.h"
#include "iface.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DifficultyList
  //

  //
  // Constructor
  //
  DifficultyList::DifficultyList(IControl *parent) 
  : ICListBox(parent),
    selectDefault(TRUE)
  {
  }


  //
  // Destructor
  //
  DifficultyList::~DifficultyList()
  {
  }


  //
  // Setup
  //
  // Configure control from an FScope
  //
  void DifficultyList::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x2A82B85D: // "DontSelectDefault"
        selectDefault = FALSE;
        break;

      // Pass it to the previous level in the hierarchy
      default:
        ICListBox::Setup(fScope);
    }
  }


  //
  // Event handling
  //
  U32 DifficultyList::HandleEvent(Event &e)
  {
    // Rebuild
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:

              // Clear any current items
              DeleteAllItems();

              // Add each existing difficulty setting
              for (NList<Difficulty::Setting>::Iterator i(&Difficulty::GetSettings()); *i; i++)
              {
                AddTextItem
                (
                  (*i)->GetName().str, TRANSLATE(((*i)->GetDescription().str))
                );
              }
              break;
          }
        }
      }
    }
    return (ICListBox::HandleEvent(e));
  }


  //
  // PostConfigure
  //
  // Called after Configure() is completed
  //
  void DifficultyList::PostConfigure()
  {
    // Post configure list box
    ICListBox::PostConfigure();

    // Select the default campaign
    selectedVar->SetStringValue(Difficulty::GetCurrentSetting().GetName().str);

  }
}
