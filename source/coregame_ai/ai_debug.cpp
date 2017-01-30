/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// AI Debuging
//
// 17-MAY-1999
//


//////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai_debug.h"
#include "ai_debug_teamlist.h"
#include "ai_debug_info.h"
#include "iface.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Debug
  //
  namespace Debug
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //
    static Bool initialized = FALSE;
    static VarInteger showPlanEvaluation;


    /////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static IControl * CreateHandler(U32 crc, IControl *parent, U32);


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)

      VarSys::CreateInteger("ai.debug.show.planevaluation", 0, VarSys::DEFAULT, &showPlanEvaluation);

      initialized = TRUE;
    }


    //
    // Done
    //
    void Done()
    {
      ASSERT(initialized)

      VarSys::DeleteItem("ai.debug");

      initialized = FALSE;
    }


    //
    // InitSimulation
    //
    void InitSimulation()
    {
      ASSERT(initialized)

      // Register controls
      IFace::RegisterControlClass("AI::Debug::TeamList", CreateHandler);
      IFace::RegisterControlClass("AI::Debug::Info", CreateHandler);
    }


    //
    // DoneSimulation
    //
    void DoneSimulation()
    {
      ASSERT(initialized)

      // Unregister controls
      IFace::UnregisterControlClass("AI::Debug::TeamList");
      IFace::UnregisterControlClass("AI::Debug::Info");
    }


    //
    // Show PlanEvaluation
    //
    Bool ShowPlanEvaluation()
    {
      ASSERT(initialized)
      return (showPlanEvaluation);
    }


    //
    // CreateHandler
    //
    IControl * CreateHandler(U32 crc, IControl *parent, U32)
    {
      ASSERT(initialized)

      IControl *ctrl = NULL;

      switch (crc)
      {
        case 0xA79C688D: // "AI::Debug::TeamList"
          ctrl = new TeamList(parent);
          break;

        case 0x9A7BBCB9: // "AI::Debug::Info"
          ctrl = new Info(parent);
          break;
      }
      return (ctrl);
    }

  }

}
