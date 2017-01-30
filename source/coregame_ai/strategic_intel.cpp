/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Intel
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_intel.h"
#include "strategic_intel_manager.h"
#include "strategic_object.h"

#include "unitobjctrl.h"
#include "spyobj.h"
#include "orders_game.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace IntelClasses
  //
  namespace IntelClasses
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SpyBuildingSweep
    //
    class SpyBuildingSweep : public Intel
    {
    private:

      // Probability of discovering a spy
      F32 probability;

      // Probability of discovering a spy in a building with the given property
      BinTree<F32> properties;

    public:

      // Constructor
      SpyBuildingSweep(Manager &manager, const GameIdent &configName, FScope *fScope);

      // Destructor
      ~SpyBuildingSweep();

      // Process the intel
      Bool Process();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SpyFieldSweep
    //
    class SpyFieldSweep : public Intel
    {
    private:

      // Probability of discovering a spy
      F32 probability;

    public:

      // Constructor
      SpyFieldSweep(Manager &manager, const GameIdent &configName, FScope *fScope);

      // Process the intel
      Bool Process();

    };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Intel
  //


  //
  // Constructor
  //
  Intel::Intel(Manager &manager, const GameIdent &configName, FScope *fScope)
  : manager(manager),
    configName(configName),
    idle(TRUE)
  {
    intervalMin = StdLoad::TypeU32(fScope, "IntervalMin");
    intervalMax = StdLoad::TypeU32(fScope, "IntervalMax", intervalMin);
   
    SetTime();
  }


  //
  // Destructor
  //
  Intel::~Intel()
  {
  }


  //
  // Set the next time to process
  //
  void Intel::SetTime()
  {
    nextTime = GameTime::SimCycle() + intervalMin + manager.GetObject().GetRandomGenerator()->Integer(intervalMax - intervalMin);
  }


  //
  // Is it ready to go
  //
  Bool Intel::IsReady()
  {
    return ((GameTime::SimCycle() > nextTime) ? TRUE : FALSE);
  }


  //
  // Process the intel
  //
  Bool Intel::Process()
  {
    return (TRUE);
  }


  //
  // Create an intel from the given fscope
  //
  Intel * Intel::Create(Manager &manager, const GameIdent &configName, FScope *fScope)
  {
    const char *type = StdLoad::TypeString(fScope, "Type");

    switch (Crc::CalcStr(type))
    {
      case 0x922FE20A: // "SpyBuildingSweep"
        return (new IntelClasses::SpyBuildingSweep(manager, configName, fScope));
        break;

      case 0x40EA45DC: // "SpyFieldSweep"
        return (new IntelClasses::SpyFieldSweep(manager, configName, fScope));
        break;

      default:
        fScope->ScopeError("Unknown Intel Type '%s'", type);
    }

  }



  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace IntelClasses
  //
  namespace IntelClasses
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SpyBuildingSweep
    //


    //
    // Constructor
    //
    SpyBuildingSweep::SpyBuildingSweep(Manager &manager, const GameIdent &configName, FScope *fScope)
    : Intel(manager, configName, fScope)
    {
      probability = StdLoad::TypeF32(fScope, "Probability", 1.0f, Range<F32>::percentage);

      FScope *iScope = fScope->GetFunction("Properties");
      if (iScope)
      {
        while (FScope *sScope = iScope->NextFunction())
        {
          if (sScope->NameCrc() == 0x9F1D54D0) // "Add"
          {
            U32 property = StdLoad::TypeStringCrc(sScope);
            F32 *prob = new F32(StdLoad::TypeF32(sScope));
            properties.Add(property, prob);
          }
        }
      }
    }


    //
    // Destructor
    //
    SpyBuildingSweep::~SpyBuildingSweep()
    {
      properties.DisposeAll();
    }


    //
    // Process the intel
    //
    Bool SpyBuildingSweep::Process()
    {
      LOG_AI(("Performing building sweep"))

      // Sweep all of the buildings which can contain spies and eject them
      for (List<UnitObjType>::Iterator t(&UnitObjCtrl::GetInfiltratableTypes()); *t; ++t)
      {
        // Get the list of units on our team of this type
        const NList<UnitObj> *units = manager.GetObject().GetTeam()->GetUnitObjects((*t)->GetNameCrc());

        if (units)
        {
          F32 p = probability;

          // Is there a configured probability for this type
          for (BinTree<F32>::Iterator prop(&properties); *prop; ++prop)
          {
            if ((*t)->HasProperty(prop.GetKey()))
            {
              p = **prop;
              break;
            }
          }

          // Iterate the units and see if they have a spy in them
          for (NList<UnitObj>::Iterator u(units); *u; ++u)
          {
            if ((*u)->GetSpyingTeams())
            {
              F32 r = manager.GetObject().GetRandomGenerator()->Float();

              // Should we eject
              if (r < p)
              {
                LOG_AI(("Ejecting Spy [%d] %f < %f", (*u)->Id(), r, p))

                Orders::Game::ClearSelected::Generate(manager.GetObject());
                Orders::Game::AddSelected::Generate(manager.GetObject(), *u);
                Orders::Game::EjectSpy::Generate(manager.GetObject());
              }
            }
          }
        }
      }

      return (FALSE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class SpyFieldSweep
    //


    //
    // Constructor
    //
    SpyFieldSweep::SpyFieldSweep(Manager &manager, const GameIdent &configName, FScope *fScope)
    : Intel(manager, configName, fScope)
    {
      probability = StdLoad::TypeF32(fScope, "Probability", 1.0f, Range<F32>::percentage);
    }


    //
    // Process the intel
    //
    Bool SpyFieldSweep::Process()
    {
      LOG_AI(("Performing field sweep"))

      // Sweep all of the enemy spies on the map which we can see and reveal them
      Team *team = manager.GetObject().GetTeam();

      // Check for all spies that are in this building
      for (NList<SpyObj>::Iterator s(&SpyObj::allSpies); *s; ++s)
      {
        SpyObj *spy = *s;

        // Is this spy on the map on a team ?
        if (spy->OnMap() && spy->GetTeam())
        {
          // Is this an enemy spy
          if (team->TestRelation(spy->GetTeam()->GetId(), Relation::ENEMY))
          {
            // Can we see it ?
            if (spy->TestCanSee(team->GetId()))
            {
              // Roll the dice
              F32 r = manager.GetObject().GetRandomGenerator()->Float();

              if (r < probability)
              {
                LOG_AI(("Revealing Spy [%d] %f < %f", (*s)->Id(), r, probability))

                // Reveal it
                Orders::Game::RevealSpy::Generate(manager.GetObject(), (*s)->Id());
              }
            }
          }
        }
      }

      return (FALSE);
    }

  }

}

