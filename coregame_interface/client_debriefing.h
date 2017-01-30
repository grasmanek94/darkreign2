///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Debriefing controls
//
// 11-AUG-1999
//


#ifndef __CLIENT_DEBRIEFING_H
#define __CLIENT_DEBRIEFING_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace DebriefingResultNotify
  //
  namespace DebriefingResultNotify
  {
    const U32 Init    = 0xA37362D6; // "Client::DebriefingResult::Notify::Init"
    const U32 Start   = 0xC49F0D59; // "Client::DebriefingResult::Notify::Start"
    const U32 Proceed = 0x6FF18DD7; // "Client::DebriefingResult::Notify::Proceed"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class DebriefingResult - Win/Lose display
  //
  class DebriefingResult : public ICWindow 
  {
    PROMOTE_LINK(Construction, ICWindow, 0x2755EC71); // "Client::DebriefingResult"

    // Have we recieved the initialize event
    Bool initialized;

    // The timer
    U32 triggerTime;

  public:

    // Constructor and destructor
    DebriefingResult(IControl *parent);
    ~DebriefingResult();

    // Handle events
    U32 HandleEvent(Event &e);

    // Poll the control
    void Poll();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace DebriefingMsg
  //
  namespace DebriefingMsg
  {
    const U32 Back        = 0xF8A8A0B8; // "Client::Debriefing::Message::Back"
    const U32 SaveStats   = 0xA9C34D39; // "Client::Debriefing::Message::SaveStats"
    const U32 Replay      = 0x84FB9D35; // "Client::Debriefing::Message::Replay"
    const U32 Next        = 0xE67927F4; // "Client::Debriefing::Message::Next"
    const U32 Setup       = 0x8737389B; // "Client::Debriefing::Message::Setup"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace DebriefingNotify
  //
  namespace DebriefingNotify
  {
    const U32 SingleCampaign      = 0xDF2EF14D; // "Client::Debriefing::Notify::SingleCampaign"
    const U32 SingleNonCampaign   = 0xB5180E98; // "Client::Debriefing::Notify::SingleNonCampaign"
    const U32 MultiCampaign       = 0x00FBA9B3; // "Client::Debriefing::Notify::MultiCampaign"
    const U32 MultiNonCampaign    = 0xDD180E08; // "Client::Debriefing::Notify::MultiNonCampaign"

    const U32 NextUnavailable     = 0xB4B76585; // "Client::Debriefing::Notify::NextUnavailable"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Debriefing - Post game management control
  //
  class Debriefing : public ICWindow 
  {
    PROMOTE_LINK(Construction, ICWindow, 0x0A9917CC); // "Client::Debriefing"

  protected:

    // Are we in a campaign mission
    Bool campaign;

    // Is there a next mission
    Bool next;

    // Did we win the mission
    Bool win;

    // Were we online
    Bool online;

  public:

    // Constructor and destructor
    Debriefing(IControl *parent);
    ~Debriefing();

    // Handle events
    U32 HandleEvent(Event &e);
  };
}


#endif

