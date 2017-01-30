///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Private Parts
// 1-OCT-1998
//

#ifndef __MULTIPLAYER_PRIVATE_H
#define __MULTIPLAYER_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "multiplayer.h"
#include "multiplayer_data.h"
#include "utiltypes.h"
#include "game_preview.h"
#include "icontrol.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //


  #pragma pack(push, 1)

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Player
  //
  struct Player
  {
    static const CRC key;

    // CPU
    StrBuf<40> cpu;

    // Memory
    U32 memory;

    // Graphics
    StrBuf<40> video;

    // OS
    StrBuf<40> os;

    // Readiness
    U8 ready : 1;

    // Data Crc
    U32 dataCrc;

    // Latitude and Longitude of the player
    F32 latitude;
    F32 longitude;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct PlayerInfo
  //
  struct PlayerInfo
  {
    static const CRC key;

    // Team ID
    U32 teamId;

    // Is ready to launch
    U8 launchReady : 1,

    // Has the current mission
       haveMission : 1,

    // Has the same version as us
       correctVersion : 1;
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Team
  //
  struct Team
  {
    static const CRC key;

    // Group ID
    U32 groupId;

    // Player who owns the team
    U32 ownerId;

    // Team color
    U32 color;

    // Start location
    U32 startLocation;

    // Flags
    U32 ai : 1;

    // AI Personality
    U32 personality;

    // Difficulty
    U32 difficulty;

    // Side
    GameIdent side;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct TeamMap
  //
  struct TeamMap
  {
    static const CRC key;

    // The name of the team mapped to
    GameIdent teamName;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Commands
  //
  namespace Commands
  {
    const U32 IntegrityChallenge  = 0xD87853B4; // "Command::IntegrityChallenge"
    const U32 IntegrityResponse   = 0x074F762D; // "Command::IntegrityResponse"

    const U32 Launch              = 0x3154C1EE; // "Command::Launch"
    const U32 LaunchReady         = 0x19AA2502; // "Command::LaunchReady"
    const U32 HaveMission         = 0xCD10816F; // "Command::HaveMission"

    const U32 JoinTeam            = 0x700C0D02; // "Command::JoinTeam"
    const U32 LeaveTeam           = 0xB3F8691B; // "Command::LeaveTeam"
    const U32 SetTeamColor        = 0x6998B1CB; // "Command::SetTeamColor"
    const U32 SetTeamSide         = 0xA6BDB21F; // "Command::SetTeamSide"
    const U32 SetStartLocation    = 0x32E6F81A; // "Command::SetStartLocation"
    const U32 SetDifficulty       = 0x9E136F17; // "Command::SetDifficulty"
    const U32 SetPersonality      = 0xCF2637ED; // "Command::SetPersonality"

    const U32 JoinGroup           = 0x9EA66073; // "Command::JoinGroup"
    const U32 LeaveGroup          = 0x3A0BCF33; // "Command::LeaveGroup"
    const U32 JoinGroupAI         = 0xB441926F; // "Command::JoinGroupAI"
    const U32 LeaveGroupAI        = 0x3CE88DA5; // "Command::LeaveGroupAI"

    const U32 Message             = 0x0FEC65C5; // "Command::Message"
    const U32 MessageQuote        = 0x6F9335B9; // "Command::MessageQuote"
    const U32 MessagePrivate      = 0xB624308E; // "Command::MessagePrivate"
    const U32 MessageTeam         = 0x1CC0E2C7; // "Command::MessageTeam"
    const U32 MessageGroup        = 0xCBE0F880; // "Command::MessageGroup"

    const U32 TransferOffer       = 0x31B7D250; // "Command::TransferOffer"
    const U32 TransferOfferAccept = 0xCE6E32D4; // "Command::TransferOfferAccept"
    const U32 TransferOfferDeny   = 0xFDDAEB47; // "Command::TransferOfferDeny"
    const U32 TransferInitiate    = 0xF390D630; // "Command::TransferInitiate"


    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Data
    //
    namespace Data
    {
      struct IntegrityChallenge
      {
        U32 crc;
      };

      struct IntegrityResponse
      {
        U32 crc;
      };

      struct SetTeamColor
      {
        U32 teamId;
        U32 color;
      };

      struct JoinTeam
      {
        U32 teamId;
      };

      struct JoinGroup
      {
        U32 groupId;
      };

      struct JoinGroupAI
      {
        U32 teamId;
        U32 groupId;
      };

      struct LeaveGroupAI
      {
        U32 teamId;
      };

      struct SetTeamSide
      {
        U32 teamId;
        GameIdent side;
      };

      struct SetStartLocation
      {
        U32 teamId;
        U32 startLocation;
      };

      struct SetDifficulty
      {
        U32 teamId;
        U32 difficulty;
      };

      struct SetPersonality
      {
        U32 teamId;
        U32 personality;
      };

      struct TransferOffer
      {
        U32 size;
        U32 type;
        U32 id;
        FilePath path;
      };

      struct TransferOfferAccept
      {
        U32 id;
      };

      struct TransferOfferDeny
      {
        U32 id;
      };

      struct TransferInitiate
      {
        U32 ip;
        U16 port;
        U32 id;
      };

    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Settings
  //
  namespace Settings
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Mission
    //
    namespace Mission
    {
      const U32 NoMission       = 0x2012C013; // "Mission::NoMission"
      const U32 NoStartLocation = 0xAB0855B6; // "Mission::NoStartLocation"
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // NameSpace Options
    //
    namespace Options
    {
      const U32 Random              = 0x57B883C0; // "Placement::Random"
      const U32 Fixed               = 0xF2AE9544; // "Placement::Fixed"

      const U32 DenyDifficulty      = 0x59D85B29; // "Difficulty::DenyDifficulty"
      const U32 AllowDifficulty     = 0x6E9EEE94; // "Difficulty::AllowDifficulty"

      const U32 UseMissionRules     = 0xB4EF7786; // "RuleSet::UseMissionRules"

      const U32 ResourceTypeNormal  = 0x484C2F25; // "ResourceType::Normal"
      const U32 ResourceTypeRegen   = 0x60F82B0A; // "ResourceType::Regen"
    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Mission
  //
  struct Mission
  {
    static const CRC key;

    // Current mission folder
    U32 missionFolder;

    // Current mission
    U32 mission;

    // Crc of key data files
    U32 crc;

  };


  const U32 maxFilters = 32;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct PropertyFilter
  //
  struct PropertyFilter
  {
    static const CRC key;

    // Number of filters
    U32 numFilters;

    // Filters
    CRC filters[maxFilters];

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Options
  //
  struct Options
  {
    static const CRC key;

    // Placement
    U32 placement;

    // Difficulty
    U32 difficulty;

    // Random seed
    U32 randomSeed;

    // Rule Set
    U32 ruleSet;

    // Start credits
    U32 startCredits;

    // Launch
    U32 launch : 1;

    // Unit limit
    U32 unitLimit;

    // Resource type
    U32 resourceType;

    // Reserved
    U32 reserved[1];

  };

  #pragma pack(pop)


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct TeamTree
  //
  struct TeamTree
  {
    // Pointer to team
    const Team *team;

    // List of players on this team
    BinTree<const Player> players;

    ~TeamTree()
    {
      players.UnlinkAll();
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct GroupTree
  //
  struct GroupTree
  {
    // List of teams in this group
    BinTree<TeamTree> teams;

    ~GroupTree()
    {
      teams.DisposeAll();
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Private Functions
  //


  // We have hosted a new session
  void HostedSession();

  // We have joined a new session
  void JoinedSession();

  // Get the actual color for a color index
  const Color & GetTeamColor(U32 index);

  // Get background team color
  const Color & GetTeamBgColor(U32 index);

  // Get background player color
  const Color & GetPlayerBgColor(U32 index);

  // Get the start location usage
  void GetStartLocations(BinTree<const Team> &teams);

  // Construct the player hierachy
  void BuildPlayerHierachy(BinTree<GroupTree> &groups);

  // Update local mission information
  void UpdateMission();

  // Update local player information
  void UpdatePlayerInfo();


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace PrivData
  //
  namespace PrivData
  {
    // Local player has the mission
    extern Bool haveMission;

    // Preview for the mission, if haveMission is TRUE
    extern Game::Preview *preview;
  
    // Local Player info
    extern Player player;

    // Recent pings to the server
    const U32 maxLocalPings = 500;
    extern U16 localPings[maxLocalPings];

    // Game setup control
    extern IControlPtr gameSetupCtrl;

    // Client control
    extern IControlPtr clientCtrl;

    // Server control
    extern IControlPtr serverCtrl;

    // Transfer control
    extern IControlPtr transferCtrl;

    // Transfer list control
    extern IControlPtr transferListCtrl;

    // Player list window
    extern IControlPtr playersCtrl;

    // Game chat window
    extern IControlPtr chatCtrl;

    // Update control
    extern IControlPtr updateCtrl;

    // Download control
    extern IControlPtr downloadCtrl;

    // Info control
    extern IControlPtr infoCtrl;

    // Clear data when a mission changes
    void ClearMission();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CmdLine
  //
  namespace CmdLine
  {
    extern Bool host;
    extern Bool join;
    extern FileIdent user;
    extern FileIdent ip;
    extern FileIdent port;
    extern FileIdent session;
    extern FileIdent password;
    extern U32 maxPlayers;

    // Reset the command line
    void Reset();
  }

}


#endif
