///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 11-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_teamlistbox.h"
#include "iface.h"
#include "ifvar.h"
#include "team.h"
#include "player.h"
#include "sides.h"
#include "strategic.h"
#include "strategic_object.h"
#include "babel.h"


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

  //
  // TeamListBox::TeamListBox
  //
  TeamListBox::TeamListBox(IControl *parent) : ICListBox(parent)
  {
    updateVar = new IFaceVar(this, "team.num");
  }

  
  //
  // TeamListBox::~TeamListBox
  //
  TeamListBox::~TeamListBox()
  {
    delete updateVar;
  }


  //
  // TeamListBox::Notify
  //
  void TeamListBox::Notify(IFaceVar *var)
  {
    if (var == updateVar)
    {
      BuildTeamList();
    }
  }


  //
  // TeamListBox::BuildTeamList
  //
  void TeamListBox::BuildTeamList()
  {
    DeleteAllItems();

    for (U32 i = 0; i < Game::MAX_TEAMS; i++)
    {
      Team *team = Team::Id2Team(i);
      if (team)
      {
        AddTextItem(team->GetName(), NULL);
      }
    } 
  }


  //
  // TeamListBox::Activate
  //
  Bool TeamListBox::Activate()
  {
    if (ICListBox::Activate())
    {
      // Bind to refresh var
      updateVar->Activate();
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Listbox is being deactivated
  //
  Bool TeamListBox::Deactivate()
  {
    if (ICListBox::Deactivate())
    {
      // Unbind from update var
      updateVar->Deactivate();
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // TeamListBox::HandleEvent
  //
  U32 TeamListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:
            {
              BuildTeamList();

              // Handled
              return (TRUE);
            }
          }
          break;
        }
      }
    }
    return ICListBox::HandleEvent(e);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PlayerListBox
  //

  //
  // PlayerListBox::PlayerListBox
  //
  PlayerListBox::PlayerListBox(IControl *parent) 
  : ICListBox(parent),
    playerListStyle(0)
  {
  }


  //
  // PlayerListBox::BuildPlayerList
  //
  void PlayerListBox::BuildPlayerList()
  {
    DeleteAllItems();

    // Step through all players
    for (U32 i = 0; i < Game::MAX_PLAYERS; i++)
    {
      if (Player *player = Player::Id2Player(i))
      {
        // Always filter AI players from this list
        if (player->GetType() != Player::AI)
        {
          // Should we show the current player
          if ((player != Player::GetCurrentPlayer()) || (playerListStyle & STYLE_SHOWCURRENT))
          {
            // Get the players team
            if (Team *team = player->GetTeam())
            {
              // Is the relation acceptable
              if ((playerListStyle & STYLE_ALLRELATIONS) || Team::TestRelation(team, Team::GetDisplayTeam(), Relation::ALLY))
              {
                char buf[32];

                // Key by name as default
                const char *key = player->GetName();

                if (playerListStyle & STYLE_KEYID)
                {
                  Utils::Sprintf(buf, sizeof(buf), "%3d", i);
                  key = buf;
                }

                Color c = player->GetTeam()->GetColor();
                AddTextItem(key, player->GetDisplayName(), &c);
              }
            }
          }
        }
      }
    } 
  }


  //
  // PlayerListBox::SetStyleItem
  //
  Bool PlayerListBox::SetStyleItem(const char *s, Bool toggle)
  {
    U32 style;

    switch (Crc::CalcStr(s))
    {
      case 0x65603055: // "KeyId"
        style = STYLE_KEYID;
        break;

      case 0x23FCF85E: // "AllRelations"
        style = STYLE_ALLRELATIONS;
        break;

      case 0x48D4D395: // "ShowCurrent"
        style = STYLE_SHOWCURRENT;
        break;

      default:
        return (ICListBox::SetStyleItem(s, toggle));
    }

    // Toggle the style
    playerListStyle = (toggle) ? (playerListStyle | style) : (playerListStyle & ~style);

    return (TRUE);
  }


  //
  // PlayerListBox::HandleEvent
  //
  U32 PlayerListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:
            {
              BuildPlayerList();

              // Handled
              return (TRUE);
            }
          }
          break;
        }
      }
    }
    return (ICListBox::HandleEvent(e));
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SideListBox
  //



  //
  // BuildSideList
  //
  void SideListBox::BuildSideList()
  {
    DeleteAllItems();

    if (random)
    {
      AddTextItem("Random", TRANSLATE(("#game.sides.random")));
    }

    for (NBinTree<Sides::Side>::Iterator sides(&Sides::GetSides()); *sides; sides++)
    {
      AddTextItem((*sides)->GetName().str, TRANSLATE(((*sides)->GetDescription().str)));
    }
  }


  //
  // SideListBox::HandleEvent
  //
  U32 SideListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:
            {
              BuildSideList();

              // Handled
              return (TRUE);
            }
          }
          break;
        }
      }
    }
    return ICListBox::HandleEvent(e);
  }


  //
  // Setup
  //
  void SideListBox::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xB8586215: // "Random"
        random = StdLoad::TypeU32(fScope);
        break;

      default:
        ICListBox::Setup(fScope);
        break;
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PersonalityListBox
  //


  //
  // BuildPersonalityList
  //
  void PersonalityListBox::BuildPersonalityList()
  {
    DeleteAllItems();

    if (random)
    {
      AddTextItem("Random", TRANSLATE(("#mods.personality.random")));
    }

    const NBinTree<Mods::Mod> *mods = Mods::GetMods(Mods::Types::Personality);
    if (mods)
    {
      for (NBinTree<Mods::Mod>::Iterator mod(mods); *mod; ++mod)
      {
        AddTextItem((*mod)->GetName().str, TRANSLATE(((*mod)->GetDescription().str)));
      }
    }
  }


  //
  // PersonalityListBox::HandleEvent
  //
  U32 PersonalityListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:
            {
              BuildPersonalityList();

              // Handled
              return (TRUE);
            }
          }
          break;
        }
      }
    }
    return ICListBox::HandleEvent(e);
  }


  //
  // Setup
  //
  void PersonalityListBox::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xB8586215: // "Random"
        random = StdLoad::TypeU32(fScope);
        break;

      default:
        ICListBox::Setup(fScope);
        break;
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RuleSetListBox
  //



  //
  // BuildRuleSetList
  //
  void RuleSetListBox::BuildRuleSetList()
  {
    ICListBox::Rebuild *rebuild = PreRebuild();

    DeleteAllItems();

    if (!pub)
    {
      // Add the 'no rule' option
      AddTextItem("None", NULL);
    }

    const NBinTree<Mods::Mod> *mods = Mods::GetMods(Mods::Types::RuleSet);
    if (mods)
    {
      for (NBinTree<Mods::Mod>::Iterator mod(mods); *mod; ++mod)
      {
        if (!(pub && (*mod)->IsPrivate()))
        {
          AddTextItem((*mod)->GetName().str, TRANSLATE(((*mod)->GetDescription().str)));
        }
      }
    }

    PostRebuild(&rebuild);
  }


  //
  // Setup this control
  //
  void RuleSetListBox::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x7FAC5AF1: // "Public"
        pub = StdLoad::TypeU32(fScope, Range<U32>::flag);
        break;

      default:
        ICListBox::Setup(fScope);
        break;
    }
  }


  //
  // RuleSetListBox::HandleEvent
  //
  U32 RuleSetListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:
            {
              BuildRuleSetList();

              // Handled
              return (TRUE);
            }
          }
          break;
        }
      }
    }
    return ICListBox::HandleEvent(e);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class WorldListBox
  //

  //
  // BuildList
  //
  void WorldListBox::BuildWorldList()
  {
    DeleteAllItems();

    if (TerrainGroup::Initialized())
    {
      // Get the group tree
      for (BinTree<TerrainGroup::Group>::Iterator i(&TerrainGroup::GetGroupTree()); *i; ++i)
      {
        AddTextItem((*i)->Name(), NULL);
      }
    }
  }


  //
  // WorldListBox::HandleEvent
  //
  U32 WorldListBox::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case ICListBoxMsg::Rebuild:
            {
              BuildWorldList();

              // Handled
              return (TRUE);
            }
          }
          break;
        }
      }
    }
    return ICListBox::HandleEvent(e);
  }
}
