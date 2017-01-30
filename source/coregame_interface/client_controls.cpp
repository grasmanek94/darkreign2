///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Custom Controls
//
// 28-OCT-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_private.h"
#include "client_squadcontrol.h"
#include "client_scorchcontrol.h"
#include "client_debriefing.h"
#include "client_construction.h"
#include "client_facility.h"
#include "client_power.h"
#include "client_resource.h"
#include "client_displayobjectives.h"
#include "client_tactical.h"
#include "client_unitcontext.h"
#include "client_unitselection.h"
#include "client_orderlag.h"
#include "client_timeofday.h"
#include "client_commandpoints.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  namespace Controls
  {
    //
    // CreateHandler
    //
    static IControl * CreateHandler(U32 crc, IControl *parent, U32)
    {
      IControl *ctrl = NULL;

      switch (crc)
      {
        case 0x8CA88CEB: // "SquadControl"
          ctrl = new SquadControl(parent);
          break;

        case 0x2755EC71: // "Client::DebriefingResult"
          ctrl = new DebriefingResult(parent);
          break;

        case 0x0A9917CC: // "Client::Debriefing"
          ctrl = new Debriefing(parent);
          break;

        case 0xE20500B8: // "Client::Construction"
          ctrl = new Construction(parent);
          break;

        case 0xE23D708F: // "Client::Facility"
          ctrl = new Facility(parent);
          break;

        case 0x2CF71032: // "Scorch"
          ctrl = new Scorch(parent);
          break;

        case 0x5DC0415F: // "Client::Power"
          ctrl = new Power(parent);
          break;

        case 0x85A65124: // "Client::Resource"
          ctrl = new Resource(parent);
          break;

        case 0x98C419B9: // "Client::DisplayObjectives"
          ctrl = new DisplayObjectives(parent);
          break;

        case 0xCEA5E93F: // "Client::TacticalButton"
          ctrl = new TacticalButton(parent);
          break;
        
        case 0x5EE82923: // "Client::UnitContextButton"
          ctrl = new UnitContextButton(parent);
          break;

        case 0x30F543AF: // "Client::UnitContext"
          ctrl = new UnitContext(parent);
          break;
        
        case 0x3468E355: // "Client::UnitSelection"
          ctrl = new UnitSelection(parent);
          break;

        case 0x4506579B: // "Client::OrderLag"
          ctrl = new OrderLag(parent);
          break;

        case 0x76DF26FC: // "Client::TimeOfDay"
          ctrl = new TimeOfDay(parent);
          break;

        case 0x81A911E6: // "Client::CommandPoints"
          ctrl = new CommandPoints(parent);
          break;
      }

      return (ctrl);
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      IFace::RegisterControlClass("SquadControl", CreateHandler);
      IFace::RegisterControlClass("Client::DebriefingResult", CreateHandler);
      IFace::RegisterControlClass("Client::Debriefing", CreateHandler);
      IFace::RegisterControlClass("Client::Construction", CreateHandler);
      IFace::RegisterControlClass("Client::Facility", CreateHandler);
      IFace::RegisterControlClass("Scorch", CreateHandler);
      IFace::RegisterControlClass("Client::Power", CreateHandler);
      IFace::RegisterControlClass("Client::Resource", CreateHandler);
      IFace::RegisterControlClass("Client::DisplayObjectives", CreateHandler);
      IFace::RegisterControlClass("Client::TacticalButton", CreateHandler);
      IFace::RegisterControlClass("Client::UnitContextButton", CreateHandler);
      IFace::RegisterControlClass("Client::UnitContext", CreateHandler);
      IFace::RegisterControlClass("Client::UnitSelection", CreateHandler);
      IFace::RegisterControlClass("Client::OrderLag", CreateHandler);
      IFace::RegisterControlClass("Client::TimeOfDay", CreateHandler);
      IFace::RegisterControlClass("Client::CommandPoints", CreateHandler);
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    { 
      IFace::UnregisterControlClass("SquadControl");
      IFace::UnregisterControlClass("Client::DebriefingResult");
      IFace::UnregisterControlClass("Client::Debriefing");
      IFace::UnregisterControlClass("Client::Construction");
      IFace::UnregisterControlClass("Client::Facility");
      IFace::UnregisterControlClass("Scorch");
      IFace::UnregisterControlClass("Client::Power");
      IFace::UnregisterControlClass("Client::Resource");
      IFace::UnregisterControlClass("Client::DisplayObjectives");
      IFace::UnregisterControlClass("Client::TacticalButton");
      IFace::UnregisterControlClass("Client::UnitContextButton");
      IFace::UnregisterControlClass("Client::UnitContext");
      IFace::UnregisterControlClass("Client::UnitSelection");
      IFace::UnregisterControlClass("Client::OrderLag");
      IFace::UnregisterControlClass("Client::TimeOfDay");
      IFace::UnregisterControlClass("Client::CommandPoints");
    }
  }
}
