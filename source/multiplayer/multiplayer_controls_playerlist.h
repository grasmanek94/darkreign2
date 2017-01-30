///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_PLAYERLIST_H
#define __MULTIPLAYER_CONTROLS_PLAYERLIST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iclistbox.h"
#include "icstatic.h"
#include "icbutton.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace PlayerListMsg
    //
    namespace PlayerListMsg
    {
      namespace Team
      {
        const U32 Ally        = 0x2CBECF5C; // "MultiPlayer::PlayerList::Team::Message::Ally"
        const U32 CoOp        = 0xC2EFBDD0; // "MultiPlayer::PlayerList::Team::Message::CoOp"
        const U32 UnCoOp      = 0xD677ED40; // "MultiPlayer::PlayerList::Team::Message::UnCoOp"
        const U32 UnAlly      = 0x38269FCC; // "MultiPlayer::PlayerList::Team::Message::UnAlly"
        const U32 AllyWith    = 0x88762AD3; // "MultiPlayer::PlayerList::Team::Message::AllyWith"
        const U32 UnAllyWith  = 0x8884687C; // "MultiPlayer::PlayerList::Team::Message::UnAllyWith"
      }

      namespace Player
      {
        const U32 SendMap     = 0x4BA98FC4; // "MultiPlayer::PlayerList::Player::Message::SendMap"
        const U32 Kick        = 0x38A7D976; // "MultiPlayer::PlayerList::Player::Message::Kick"
      }

      namespace AI
      {
        const U32 Delete      = 0x2FA3C466; // "MultiPlayer::PlayerList::AI::Message::Delete"
      }
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace PlayerListNotify
    //
    namespace PlayerListNotify
    {
      const U32 Captain       = 0x9A8579C5; // "MultiPlayer::PlayerList::Notify::Captain"
      const U32 Ally          = 0x0324139A; // "MultiPlayer::PlayerList::Notify::Ally"
      const U32 CoOp          = 0xED756116; // "MultiPlayer::PlayerList::Notify::CoOp"
      const U32 UnAlly        = 0x7353798A; // "MultiPlayer::PlayerList::Notify::UnAlly"
      const U32 UnCoOp        = 0x9D020B06; // "MultiPlayer::PlayerList::Notify::UnCoOp"
      const U32 UnAllyWith    = 0xBEBBB517; // "MultiPlayer::PlayerList::Notify::UnAllyWith"
      const U32 SendMap       = 0x64E63DD5; // "MultiPlayer::PlayerList::Notify::SendMap"
      const U32 Kickable      = 0x2C739338; // "Multiplayer::PlayerList::Notify::Kickable"
    }                       


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class PlayerList
    //
    class PlayerList : public ICListBox
    {
    public:

      class PlayerItem;
      class AIPlayerItem;
      class TeamItem;
      class GroupItem;
      class AllyWithButton;

      friend PlayerItem;
      friend AIPlayerItem;
      friend TeamItem;
      friend GroupItem;

    private:

      // Flags
      U32 dirty : 1;

      // Icons
      TextureInfo iconLaunchReady[2];
      TextureInfo iconHaveMission[2];

      // Offsets
      S32 offsetName;
      S32 offsetSide;
      S32 offsetPing;
      S32 widthPing;
      S32 widthTeam;
      S32 widthName;
      S32 offsetDifficulty;
      S32 offsetLaunchReady;
      S32 offsetHaveMission;

      Bool basic;

      FScope *teamItemCfg;
      FScope *playerItemCfg;
      FScope *aiPlayerItemCfg;
      FScope *groupItemCfg;

      // List of all PlayerList controls
      static NList<PlayerList> allControls;

      // Node
      NList<PlayerList>::Node node;


    public:

      // Constructor
      PlayerList(IControl *parent);

      // Destructor
      ~PlayerList();

      // HandleEvent
      U32 HandleEvent(Event &e);

      // Redraw self
      void DrawSelf(PaintInfo &pi);

      // Setup
      void Setup(FScope *fScope);

      // Marks all PlayerList controls as dirty
      static void Dirty();


      ///////////////////////////////////////////////////////////////////////////////
      //
      // Class PlayerItem
      //
      class PlayerItem : public ICStatic
      {
        PROMOTE_LINK(PlayerItem, ICStatic, 0x0095F557); // "PlayerList::PlayerItem"

      protected:

        // PlayerList this player item is a member of
        PlayerList &playerList;

        // Player Network id
        U32 networkId;

        // Team id
        U32 teamId;

        // Tips
        CH *tipReady;
        CH *tipNotReady;
        CH *tipHasMap;
        CH *tipDoesntHaveMap;

      public:

        // Constructor and Destructor
        PlayerItem(PlayerList &playerList, IControl *parent, U32 networkId, U32 teamId);
        ~PlayerItem();

        // Redraw self
        void DrawSelf(PaintInfo &pi);

        // HandleEvent
        U32 HandleEvent(Event &e);

        // Setup
        void Setup(FScope *fScope);

      public:

        // Get network Id
        U32 GetNetworkId()
        {
          return (networkId);
        }

      };


      ///////////////////////////////////////////////////////////////////////////////
      //
      // Class AIPlayerItem
      //
      class AIPlayerItem : public ICStatic
      {
        PROMOTE_LINK(AIPlayerItem, ICStatic, 0xE12442A3) // "PlayerList::AIPlayerItem"

      private:

        // PlayerList this player item is a member of
        PlayerList &playerList;

        // Team id
        U32 teamId;

      public:

        // Constructor
        AIPlayerItem(PlayerList &playerList, IControl *parent, U32 teamId) 
        : ICStatic(parent),
          playerList(playerList),
          teamId(teamId)
        { 
        }

        // Redraw self
        void DrawSelf(PaintInfo &pi);

        // HandleEvent
        U32 HandleEvent(Event &e);

        // Get the team id
        U32 GetTeamId()
        {
          return (teamId);
        }
      };


      ///////////////////////////////////////////////////////////////////////////////
      //
      // Class TeamItem
      //
      class TeamItem : public ICStatic
      {
        PROMOTE_LINK(TeamItem, ICStatic, 0x976987F9) // "PlayerList::TeamItem"

      private:

        // PlayerList this player item is a member of
        PlayerList &playerList;

        // Team id
        U32 teamId;

        // Group id
        U32 groupId;

      public:

        // Constructor
        TeamItem(PlayerList &playerList, IControl *parent, U32 teamId, U32 groupId) 
        : ICStatic(parent),
          playerList(playerList),
          teamId(teamId),
          groupId(groupId)
        { 
        }

        // Redraw self
        void DrawSelf(PaintInfo &pi);

        // HandleEvent
        U32 HandleEvent(Event &e);

      public:

        // Get the team id
        U32 GetTeamId()
        {
          return (teamId);
        }

        // Get the group id
        U32 GetGroupId()
        {
          return (groupId);
        }

      };


      ///////////////////////////////////////////////////////////////////////////////
      //
      // Class GroupItem
      //
      class GroupItem : public ICStatic
      {
        PROMOTE_LINK(GroupItem, ICStatic, 0x57FB5BA7) // "PlayerList::GroupItem"

      public:

        // Constructor
        GroupItem(IControl *parent) 
        : ICStatic(parent) 
        { 
        }

        // Redraw self
        void DrawSelf(PaintInfo &pi);

        // Automatically resize geometry
        void AutoSize();
      };


      ///////////////////////////////////////////////////////////////////////////////
      //
      // Class AllyWithButton
      //
      class AllyWithButton : public ICButton
      {
        PROMOTE_LINK(AllyWithButton, ICButton, 0xA37B7494) // "PlayerList::AllyWithButton"

      private:

        // Color to display
        ::Color color;

        // Team this will ally with
        U32 group;

      public:

        // Constructor
        AllyWithButton(IControl *parent);

        // Redraw self
        void DrawSelf(PaintInfo &pi);

        friend PlayerList;
        friend TeamItem;

      };

    };

  }

}


#endif
