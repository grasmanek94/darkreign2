///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Environment control
//
// 1-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "light_priv.h"
#include "environment.h"
#include "environment_light.h"
#include "environment_time.h"
#include "environment_rain.h"
#include "environment_quake.h"
#include "varsys.h"
#include "console.h"
#include "perfstats.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Environment
//
namespace Environment
{
  // Initialization Flag
  static Bool initialized = FALSE;

  //
  // CmdHandler
  //
  // Handles var system events
  //
  static void CmdHandler(U32 pathCrc)
  {
    ASSERT(initialized);

    switch (pathCrc)
    {
      case 0x964EB57B: // "environment.report"
      {
        // report fogcolor
        U32 r = (U32) (Vid::renderState.fogColorF32.r * 255.0f);
        U32 g = (U32) (Vid::renderState.fogColorF32.g * 255.0f);        
        U32 b = (U32) (Vid::renderState.fogColorF32.b * 255.0f);
        CON_DIAG(("fog       = %d %d %d", r, g, b));

        // ambient
        r = (U32) (Vid::renderState.ambientColor.r * 255.0f);
        g = (U32) (Vid::renderState.ambientColor.g * 255.0f);
        b = (U32) (Vid::renderState.ambientColor.b * 255.0f);
        CON_DIAG(("ambient  = %d %d %d", r, g, b));

        // light
        r = (U32) Vid::Light::sunColor.r;
        g = (U32) Vid::Light::sunColor.g;
        b = (U32) Vid::Light::sunColor.b;
        CON_DIAG(("light     = %d %d %d", r, g, b));

        // light angle
        F32 angle = Vid::Light::sun->WorldMatrix().Front().Dot( Matrix::I.Right());
        angle = (angle + 1.0f) * 90.0f;
        CON_DIAG(("angle     = %3.0f %s", angle, Environment::Light::IsDay() ? "day" : "night"));

        // farplane
        CON_DIAG(("farplane = %4.0f", Vid::CurCamera().FarPlane()));

        // fogdepth
        CON_DIAG(("fogdist   = %4.0f", Vid::renderState.fogMin));
        break;
      }
    }
  }


  //
  // Init
  //
  void Init()
  {
    ASSERT(!initialized)

    VarSys::RegisterHandler("environment", CmdHandler);

    VarSys::CreateCmd("environment.report");

    // Initialize the rain
    Rain::Init();

    // Initialize the light
    Light::Init();

    // Initialize the time
    Time::Init();

    // guess what?
    Quake::Init();

    initialized = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(initialized)

    Quake::Done();

    Rain::Done();

    Light::Done();

    Time::Done();

    // Destroy environment scope
    VarSys::DeleteItem("environment");

    initialized = FALSE;
  }


  //
  // Process
  //
  // Process Environment
  //
  void Process()
  {
    ASSERT(initialized)

    PERF_S(("Light::Process"))
    Light::Process();
    PERF_E(("Light::Process"))

    // must be after Light::Process for lightning to work
    //
    PERF_S(("Rain::Process"))
    Rain::Process();
    PERF_E(("Rain::Process"))

  }

  //
  // Render
  //
  // Render Environment
  //
  void Render()
  {
    ASSERT(initialized)

    Rain::Render();
  }


  //
  // LoadInfo
  //
  // Load Environment information
  //
  void LoadInfo(FScope *fScope)
  {
    ASSERT(initialized)

    // Load rain
    // tolerant of NULL fScope
    //
    Rain::LoadInfo( fScope->GetFunction("Rain", FALSE));

    // Load light
    Light::LoadInfo(fScope->GetFunction("Light"));

    // Load time
    Time::LoadInfo(fScope->GetFunction("Time"));
  }


  //
  // SaveInfo
  //
  // Save Environment information
  //
  void SaveInfo(FScope *fScope)
  {
    ASSERT(initialized)

    // Save rain
    Rain::SaveInfo(fScope->AddFunction("Rain"));

    // Save light
    Light::SaveInfo(fScope->AddFunction("Light"));

    // Save time
    Time::SaveInfo(fScope->AddFunction("Time"));
  }


  //
  // PostLoad
  //
  void PostLoad()
  {
    ASSERT(initialized)

    // PostLoad Rain before Light
    //
    Rain::PostLoad();

    // PostLoad Light
    Light::PostLoad();
  }
}
