///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Host Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_HOST_H
#define __MULTIPLAYER_HOST_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "missions.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Host
  //
  namespace Host
  {

    // Reset
    void Reset();

    // Hosted
    void Hosted();


    // EnterPlayer
    void EnterPlayer(U32 id);

    // ExitPlayer
    void ExitPlayer(U32 id);

    // Interpret a command
    void Handler(CRC from, CRC key, U32 size, const U8 *data);


    // FindEmptyTeam
    U32 FindEmptyTeam(U32 playerId);

    // FindEmptyGroup
    U32 FindEmptyGroup();

    // FindUnusedColor
    U32 FindUnusedColor();

    // Reset a team
    void ResetTeam(U32 teamId);

    // Create an AI Team
    Bool CreateAITeam();

    // Create AI teams to fill all remaining spots
    void FillAITeams();

    // Delete an AI team
    Bool DeleteAITeam(U32 teamId);

    // Change the mission
    void ChangeMission(const Missions::Mission *mission);

    // Get the currently selected mission
    const Missions::Mission * GetMission();

    // Clear the mission
    void ClearMission();

    // Check to see if we can launch
    Bool CheckLaunch();

    // Invalidate Launch Readyness
    void InvalidateLaunchReadyness();

    // Send updated settings to all players
    void SendUpdatedSettings(U32 placement, U32 difficulty, U32 ruleSet, U32 startCredits, U32 unitLimit, U32 resourceType);

    // Update the ruleset
    void SendUpdatedRuleSet(U32 ruleSet);

    // Get team counts
    const U32 * GetTeamCounts();

    // Get group counts
    const U32 * GetGroupCounts();

    // Get color usage
    const Bool * GetColorUsage();

  }

}

#endif
