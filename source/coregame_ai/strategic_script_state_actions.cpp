/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Script
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script_state.h"
#include "strategic_script_recruiter.h"
#include "strategic_object.h"
#include "strategic_location.h"
#include "orders_squad.h"
#include "orders_game.h"
#include "tagobj.h"
#include "resolver.h"
#include "param.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptActions
  //
  namespace ScriptActions
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Init
    //
    class Init : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      Init(Script &script)
      : Action(script)
      { 
      }

      ~Init() 
      { 
      }

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Delete
    //
    class Delete : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      Delete(Script &script, FScope *) 
      : Action(script) 
      {
      }

      ~Delete() 
      { 
      }

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Recruit
    //
    class Recruit : public Script::State::Action
    {
    private:

      // The recruiter
      Script::Recruiter *recruiter;

      // Save id
      U32 savedId;

    public:

      // Constructor and Destructor
      Recruit(Script &script, FScope *fScope);
      ~Recruit();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Transfer
    //
    class Transfer : public Script::State::Action
    {
    private:

      // The squad to transfer to
      Param::Ident scriptName;

    public:

      // Constructor and Destructor
      Transfer(Script &script, FScope *fScope);
      ~Transfer();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Spawn
    //
    class Spawn : public Script::State::Action
    {
    private:

	    // Name of the config to use
      Param::Ident configName;

      // Name of the formation to use
      Param::Ident formationName;

      // Direction to face the spawn
      Param::Float direction;

      // Location
      Location *location;

    public:

      // Constructor and Destructor
      Spawn(Script &script, FScope *fScope);
      ~Spawn();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Discharge
    //
    class Discharge : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      Discharge(Script &script, FScope *fScope);
      ~Discharge();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);
    
    };

    
    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Move
    //
    class Move : public Script::State::Action
    {
    private:

      // Location
      Location *location;

      // Attack whilst moving
      Param::Integer attack;

      // Just turn to the location
      Param::Integer turn;

    public:

      // Constructor and Destructor
      Move(Script &script, FScope *fScope);
      ~Move();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Explore
    //
    class Explore : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      Explore(Script &script, FScope *fScope);
      ~Explore();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Stop
    //
    class Stop : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      Stop(Script &script, FScope *fScope);
      ~Stop();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Trail
    //
    class Trail : public Script::State::Action
    {
    private:

      // Name of the trail to use
      Param::Ident trailName;

      // Index to start out on the trail
      Param::Integer index;

      // Attack whilst moving
      Param::Integer attack;

      // Use the nearest index (overides the index)
      Param::Integer resume;

    public:

      // Constructor and Destructor
      Trail(Script &script, FScope *fScope);
      ~Trail();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Formation
    //
    class Formation : public Script::State::Action
    {
    private:

      // Name of the formation to use
      GameIdent formationName;

      // Direction to face the formation
      F32 direction;

    public:

      // Constructor and Destructor
      Formation(Script &script, FScope *fScope);
      ~Formation();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Restore
    //
    class Restore : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      Restore(Script &script, FScope *fScope);
      ~Restore();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ApplyRule
    //
    class ApplyRule : public Script::State::Action
    {
    private:

      // The rule
      Rule *rule;

    public:

      // Constructor and Destructor
      ApplyRule(Script &script, FScope *fScope);
      ~ApplyRule();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class AttackTag
    //
    class AttackTag : public Script::State::Action
    {
    private:

      // Name of the tag to attack
      Param::Ident tagName;

    public:

      // Constructor and Destructor
      AttackTag(Script &script, FScope *fScope);
      ~AttackTag();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    protected:

      // Find a target to attack
      Bool FindTarget();

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FollowTag
    //
    class FollowTag : public Script::State::Action
    {
    private:

      // Name of the tag to follow
      Param::Ident tagName;

      // Attack whilst moving
      Param::Integer attack;

    public:

      // Constructor and Destructor
      FollowTag(Script &script, FScope *fScope);
      ~FollowTag();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Defect
    //
    class Defect : public Script::State::Action
    {
    private:

      GameIdent teamName;

    public:

      // Constructor and Destructor
      Defect(Script &script, FScope *fScope);
      ~Defect();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportAcquire
    //
    class TransportAcquire : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      TransportAcquire(Script &script, FScope *fScope);
      ~TransportAcquire();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportSpawn
    //
    class TransportSpawn : public Script::State::Action
    {
    private:

	    // Name of the config to use
      Param::Ident configName;

      // Name of the transport type to use
      Param::Ident transportName;

      // Location
      Location *location;

    public:

      // Constructor and Destructor
      TransportSpawn(Script &script, FScope *fScope);
      ~TransportSpawn();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportRelease
    //
    class TransportRelease : public Script::State::Action
    {
    private:

      // Location to return to
      Location *location;

    public:

      // Constructor and Destructor
      TransportRelease(Script &script, FScope *fScope);
      ~TransportRelease();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportLoad
    //
    class TransportLoad : public Script::State::Action
    {
    private:

      // Prune off units which won't fit into the transport
      Param::Integer prune;

      // Attack whilst loading
      Param::Integer attack;

    public:

      // Constructor and Destructor
      TransportLoad(Script &script, FScope *fScope);
      ~TransportLoad();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportUnload
    //
    class TransportUnload : public Script::State::Action
    {
    private:

      // Location to unload at
      Location *location;

    public:

      // Constructor and Destructor
      TransportUnload(Script &script, FScope *fScope);
      ~TransportUnload();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportMove
    //
    class TransportMove : public Script::State::Action
    {
    private:

      // Location to return to
      Location *location;

    public:

      // Constructor and Destructor
      TransportMove(Script &script, FScope *fScope);
      ~TransportMove();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportDelete
    //
    class TransportDelete : public Script::State::Action
    {
    public:

      // Constructor and Destructor
      TransportDelete(Script &script, FScope *fScope);
      ~TransportDelete();

      // Execute
      void Execute();

      // Translate the notification
      U32 Notify(U32 message, U32 data);

    };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::State::Action
  //


  //
  // Create an action from the given fscope
  //
  Script::State::Action * Script::State::Action::Create(Script &script, FScope *fScope)
  {
    if (fScope)
    {
      const char *type = fScope->NextArgString();

      switch (Crc::CalcStr(type))
      {
        case 0x0725EAC5: // "Delete"
          return (new ScriptActions::Delete(script, fScope));

        case 0xA78215CE: // "Recruit"
          return (new ScriptActions::Recruit(script, fScope));

        case 0x3B580609: // "Transfer"
          return (new ScriptActions::Transfer(script, fScope));

        case 0xB3DDDF37: // "Spawn"
          return (new ScriptActions::Spawn(script, fScope));

        case 0x6C4C99F3: // "Discharge"
          return (new ScriptActions::Discharge(script, fScope));

        case 0x3381FB36: // "Move"
          return (new ScriptActions::Move(script, fScope));

        case 0x9714BD50: // "Explore"
          return (new ScriptActions::Explore(script, fScope));

        case 0xFF62DA04: // "Stop"
          return (new ScriptActions::Stop(script, fScope));

        case 0x82698073: // "Trail"
          return (new ScriptActions::Trail(script, fScope));

        case 0x7223612A: // "Formation"
          return (new ScriptActions::Formation(script, fScope));

        case 0x5463CB0D: // "Restore"
          return (new ScriptActions::Restore(script, fScope));

        case 0x908D4628: // "ApplyRule"
          return (new ScriptActions::ApplyRule(script, fScope));

        case 0xFA021BDC: // "AttackTag"
          return (new ScriptActions::AttackTag(script, fScope));

        case 0x90A0CA70: // "FollowTag"
          return (new ScriptActions::FollowTag(script, fScope));

        case 0x2D9B79B8: // "Defect"
          return (new ScriptActions::Defect(script, fScope));

        case 0x882CEF6D: // "TransportAcquire"
          return (new ScriptActions::TransportAcquire(script, fScope));

        case 0xD3A29DB9: // "TransportSpawn"
          return (new ScriptActions::TransportSpawn(script, fScope));

        case 0x655DEAC1: // "TransportRelease"
          return (new ScriptActions::TransportRelease(script, fScope));

        case 0xEE424790: // "TransportLoad"
          return (new ScriptActions::TransportLoad(script, fScope));

        case 0x6D8C916B: // "TransportUnload"
          return (new ScriptActions::TransportUnload(script, fScope));

        case 0x117025F9: // "TransportMove"
          return (new ScriptActions::TransportMove(script, fScope));

        case 0xD4C2A252: // "TransportDelete"
          return (new ScriptActions::TransportDelete(script, fScope));

        default:
          fScope->ScopeError("Unknown action type '%s'", type);
      }
    }
    else
    {
      // Get ourselves a squad
      return (new ScriptActions::Init(script));
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace ScriptActions
  //
  namespace ScriptActions
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Init
    //

    //
    // Execute
    //
    void Init::Execute()
    {
      Orders::Squad::Create::Generate(script.GetObject(), U32(&script));
    }


    //
    // Notify
    //
    U32 Init::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x8AA808B7: // "Squad::Created"
          return (Status::Completed);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Delete
    //

    //
    // Execute
    //
    void Delete::Execute()
    {
      Orders::Squad::Delete::Generate(script.GetObject(), script.GetSquad()->Id());
    }


    //
    // Notify
    //
    U32 Delete::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0xE145C9A6: // "Squad::Deleted"
          return (Status::Completed);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Recruit
    //


    //
    // Constructor
    //
    Recruit::Recruit(Script &script, FScope *fScope)
    : Action(script),
      recruiter(Script::Recruiter::Create(script, fScope)),
      savedId(0)
    {
    }


    //
    // Destructor
    //
    Recruit::~Recruit()
    {
      delete recruiter;
    }


    //
    // Execute
    //
    void Recruit::Execute()
    {
      ASSERT(recruiter)

      // Execute the recruiter
      recruiter->Execute(script, savedId = script.GetManager().GetRecruitId());
    }


    //
    // Notify
    //
    U32 Recruit::Notify(U32 message, U32 param)
    {
      switch (message)
      {
        case 0xE6DC8EF5: // "Squad::RecruitComplete"
        {
          if (param == savedId)
          {
          /*
            LOG_AI(("%s: Recruit completed", script.GetName()))

            for (SquadObj::UnitList::Iterator u(&script.GetSquad()->GetList()); *u; u++)
            {
              if ((*u)->Alive())
              {
                LOG_AI(("[%d] %s", (**u)->Id(), (**u)->UnitType()->GetName()))
              }
            }
            */
            return (Status::Completed);
          }
          else
          {
            return (Status::Ignored);
          }
        }

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Transfer
    //


    //
    // Constructor
    //
    Transfer::Transfer(Script &script, FScope *fScope)
    : Action(script),
      scriptName("Script", fScope, &script)
    {
    }


    //
    // Destructor
    //
    Transfer::~Transfer()
    {
    }


    //
    // Execute
    //
    void Transfer::Execute()
    {
      // Resolve the script
      Script *s = script.GetManager().FindScript(scriptName.GetIdent());

      if (s)
      {
        // Select our squad
        Orders::Game::AddSelected::Generate(script.GetObject(), reinterpret_cast<const UnitObjList &>(script.GetSquad()->GetList()));

        // Add to the other squad
        Orders::Squad::AddSelected::Generate(script.GetObject(), s->GetSquad()->Id());
      }

      // Notify that the transfer is completed
      Orders::Squad::Notify::Generate(script.GetObject(), script.GetSquad()->Id(), 0x47C1486C); // "Transfer::TransferComplete"
    }


    //
    // Notify
    //
    U32 Transfer::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x47C1486C: // "Transfer::TransferComplete"
          return (Status::Completed);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Spawn
    //


    //
    // Constructor
    //
    Spawn::Spawn(Script &script, FScope *fScope)
    : Action(script),
      configName("Config", fScope, &script),
      location(Location::Create(fScope->GetFunction("Location"), &script)),
      formationName("Formation", fScope, &script),
      direction("Direction", fScope, &script)
    {
    }


    //
    // Destructor
    //
    Spawn::~Spawn()
    {
      delete location;
    }


    //
    // Execute
    //
    void Spawn::Execute()
    {
      Object &object = script.GetObject();

      // Find a named formation
      ::Formation::Slots *formation = ::Formation::Find(formationName.GetCrc());
      if (!formation)
      {
        ERR_CONFIG(("Could not find formation '%s'", formationName.GetStr()))
      }

      Config::RecruitType *config = Config::FindRecruitType(configName.GetCrc());
      if (!config)
      {
        ERR_CONFIG(("Could not find RecruitType '%s'", configName.GetStr()))
      }

      List<::Formation::Slot>::Iterator slot(&formation->slots);

      F32 angle = (90.0f - direction) * DEG2RAD;

      for (Config::RecruitType::TypeList::Iterator t(&config->GetTypes()); *t; t++)
      {
        if ((*t)->Alive())
        {
          UnitObjType *type = **t;

          for (U32 i = 0; i < (*t)->amount; i++)
          {
            // Is there a slot for this unit ?
            if (*slot)
            {
              // Calculate the absolute direction to the slot
              F32 dir = angle + (*slot)->direction;
              VectorDir::FixU(dir);

              // Calculate the orientation
              F32 orient = angle + (*slot)->orientation;
              VectorDir::FixU(orient);

              // Determine the slot offset
              Vector offset;
              offset.x = (F32) cos(dir);
              offset.y = 0.0f;
              offset.z = (F32) sin(dir);
              offset *= (*slot)->distance;
              offset += location->GetVector();
              offset.y = TerrainData::FindFloor(offset.x, offset.z);

              // Order the creation of the unit
              Orders::Squad::Spawn::Generate(object, script.GetSquad()->Id(), type->GetNameCrc(), offset, orient);

              // Increment the slot
              slot++;
            }
            else
            {
              // Ran out of slots
              LOG_WARN(("Ran out of slots in formation '%s' whilst spawning in units", formationName.GetStr()))
            }
          }
        }
      }

      // Notify that the spawn is completed
      Orders::Squad::Notify::Generate(object, script.GetSquad()->Id(), 0x667A9F3F); // "Squad::SpawnComplete"
    }


    //
    // Notify
    //
    U32 Spawn::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x667A9F3F: // "Squad::SpawnComplete"
        {
          /*
          LOG_AI(("Spawn completed for '%s'", script.GetName()))

          for (SquadObj::UnitList::Iterator u(&script.GetSquad()->GetList()); *u; u++)
          {
            if ((*u)->Alive())
            {
              LOG_AI(("[%d] %s", (**u)->Id(), (**u)->UnitType()->GetName()))
            }
          }
          */
          return (Status::Completed);
        }

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Discharge
    //


    //
    // Constructor
    //
    Discharge::Discharge(Script &script, FScope *)
    : Action(script)
    {
    }


    //
    // Destructor
    //
    Discharge::~Discharge()
    {
    }


    //
    // Execute
    //
    void Discharge::Execute()
    {
      Object &object = script.GetObject();

      Orders::Squad::Empty::Generate(object, script.GetSquad()->Id());
    }


    //
    // Notify
    //
    U32 Discharge::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x2B8A9E2C: // "Squad::Empty"
          return (Status::Completed);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Move
    //


    //
    // Constructor
    //
    Move::Move(Script &script, FScope *fScope)
    : Action(script),
      location(Location::Create(fScope->GetFunction("Location"), &script)),
      attack("Attack", fScope, 1, &script),
      turn("Turn", fScope, 0, &script)
    {
    }

    
    //
    // Destructor
    //
    Move::~Move()
    {
      delete location;
    }


    //
    // Execute
    //
    void Move::Execute()
    {
      Object &object = script.GetObject();

      Orders::Squad::Move::Generate
      (
        object, 
        script.GetSquad()->Id(), 
        location->GetVector(&object, &script), 
        attack, turn,
        Orders::FLUSH
      );
    }


    //
    // Notify
    //
    U32 Move::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Explore
    //


    //
    // Constructor
    //
    Explore::Explore(Script &script, FScope *)
    : Action(script)
    {
    }

    
    //
    // Destructor
    //
    Explore::~Explore()
    {
    }


    //
    // Execute
    //
    void Explore::Execute()
    {
      Object &object = script.GetObject();
      Orders::Squad::Explore::Generate(object, script.GetSquad()->Id(), Orders::FLUSH);
    }


    //
    // Notify
    //
    U32 Explore::Notify(U32, U32)
    {
      return (Status::Ignored);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Stop
    //


    //
    // Constructor
    //
    Stop::Stop(Script &script, FScope *)
    : Action(script)
    {
    }

    
    //
    // Destructor
    //
    Stop::~Stop()
    {
    }


    //
    // Execute
    //
    void Stop::Execute()
    {
      Object &object = script.GetObject();

      // Stop the squad
      Orders::Squad::Stop::Generate(object, script.GetSquad()->Id());

      // Fire off a notification
      Orders::Squad::Notify::Generate(object, script.GetSquad()->Id(), 0xA42667F1); // "Squad::StopCompleted"
    }


    //
    // Notify
    //
    U32 Stop::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0xA42667F1: // "Squad::StopCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Trail
    //


    //
    // Constructor
    //
    Trail::Trail(Script &script, FScope *fScope)
    : Action(script),
      trailName("Trail", fScope, &script),
      index("Index", fScope, 0, &script),
      attack("Attack", fScope, 1, &script),
      resume("Resume", fScope, 0, &script)
    {
    }

    
    //
    // Destructor
    //
    Trail::~Trail()
    {
    }


    //
    // Execute
    //
    void Trail::Execute()
    {
      Object &object = script.GetObject();

      // Attempt to resolve the trail by name
      TrailObj *trail = TrailObj::Find(trailName.GetCrc(), object.GetTeam());
      if (!trail)
      {
        ERR_CONFIG(("Could not find trail '%s'", trailName.GetStr()))
      }

      U32 ind = index;

      // Get the central location of the squad
      Vector location;
      if (script.GetSquad()->GetLocation(location))
      {
        Point<F32> pos;
        pos.x = location.x;
        pos.z = location.z;

        Point<U32> squad;
        WorldCtrl::MetresToCellPoint(pos, squad);

        U32 dist = U32_MAX;

        if (resume)
        {
          // Given the current location of the squad, find the closest point on the trail
          TrailObj::WayPointList &wpl = trail->GetList();
          TrailObj::WayPointList::Iterator i(&wpl);
          TrailObj::WayPoint *winner = NULL;
          U32 d = 0;
          ind = 0;
          for (!i; *i; ++i)
          {
            U32 offset = 
            (
              (squad.x - (**i).x) * (squad.x - (**i).x) + 
              (squad.z - (**i).z) * (squad.z - (**i).z)
            );

            if (offset < dist)
            {
              winner = *i;
              dist = offset;
              ind = d++;
            }
            else
            {
              d++;
            }
          }

          if (!winner)
          {
            return;
          }
        }

        Orders::Squad::Trail::Generate(object, script.GetSquad()->Id(), trail->Id(), ind, attack, Orders::FLUSH);
      }
    }


    //
    // Notify
    //
    U32 Trail::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }



    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Formation
    //


    //
    // Constructor
    //
    Formation::Formation(Script &script, FScope *fScope) 
    : Action(script),
      formationName(StdLoad::TypeString(fScope, "Formation")),
      direction((90.0f - StdLoad::TypeF32(fScope, "Direction", Range<F32>(-180.0f, 180.0f))) * DEG2RAD)
    {
    }

    
    //
    // Destructor
    //
    Formation::~Formation()
    {
    }


    //
    // Execute
    //
    void Formation::Execute()
    {
      // Get the location of the squad
      Vector location;
      if (script.GetSquad()->GetLocation(location))
      {
        Object &object = script.GetObject();
        Orders::Squad::Formation::Generate(object, script.GetSquad()->Id(), formationName.crc, location, direction, Orders::FLUSH);
      }
    }


    //
    // Notify
    //
    U32 Formation::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Restore
    //


    //
    // Constructor
    //
    Restore::Restore(Script &script, FScope *) 
    : Action(script)
    {
    }

    
    //
    // Destructor
    //
    Restore::~Restore()
    {
    }


    //
    // Execute
    //
    void Restore::Execute()
    {
      // Get the location of the squad
      Orders::Squad::Restore::Generate(script.GetObject(), script.GetSquad()->Id(), Orders::FLUSH);
    }


    //
    // Notify
    //
    U32 Restore::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ApplyRule
    //

    //
    // Constructor
    //
    ApplyRule::ApplyRule(Script &script, FScope *fScope)
    : Action(script),
      rule(Rule::Create(fScope, script.GetObject().GetRuleManager(), script))
    {
    }


    //
    // Destructor
    //
    ApplyRule::~ApplyRule()
    {
      delete rule;
    }


    //
    // Execute
    //
    void ApplyRule::Execute()
    {
      rule->Apply();
    }


    //
    // Translate the notification
    //
    U32 ApplyRule::Notify(U32 message, U32)
    {
      switch (message)
      {
        case RuleNotify::Completed:
          return (Status::Completed);

        case RuleNotify::Failed:
          return (Status::Failed);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class AttackTag
    //


    //
    // Constructor
    //
    AttackTag::AttackTag(Script &script, FScope *fScope)
    : Action(script),
      tagName("Tag", fScope, &script)
    {
    }


    //
    // Destructor
    //
    AttackTag::~AttackTag()
    {
    }


    //
    // Execute
    //
    void AttackTag::Execute()
    {
      FindTarget();
    }


    //
    // Notify
    //
    U32 AttackTag::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    //
    // Find a target to attack
    //
    Bool AttackTag::FindTarget()
    {
      Object &object = script.GetObject();

      TagObj *tag = TagObj::FindTag(tagName.GetCrc());

      if (tag)
      {
        // Get the central location of the squad
        Vector location;

        if (script.GetSquad()->GetLocation(location))
        {
          MapObj *mapObj = NULL;
          F32 dist = F32_MAX;

          // Find the closest object to the squad in the tag
          for (MapObjList::Iterator i(&tag->list); *i; i++)
          {
            if ((*i)->Alive())
            {
              Vector offset = (**i)->Origin() - location;
              F32 offsetDist = offset.Magnitude2();
              if (offsetDist < dist)
              {
                mapObj = **i;
                dist = offsetDist;
              }
            }
          }

          if (mapObj)
          {
            // Attack their ass
            Orders::Squad::Attack::Generate(
              object, 
              script.GetSquad()->Id(), 
              mapObj->Id(), 
              TRUE,
              Orders::FLUSH);

            // We found something to attack
            return (TRUE);
          }
        }
      }
      else
      {
        ERR_CONFIG(("Could not resolve tag '%s'", tagName.GetStr()))
      }

      // Didn't find a target (for whatever reason)
      return (FALSE);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class FollowTag
    //


    //
    // Constructor
    //
    FollowTag::FollowTag(Script &script, FScope *fScope)
    : Action(script),
      tagName("Tag", fScope, &script),
      attack("Attack", fScope, &script)
    {
    }


    //
    // Destructor
    //
    FollowTag::~FollowTag()
    {
    }


    //
    // Execute
    //
    void FollowTag::Execute()
    {
      TagObj *tag = TagObj::FindTag(tagName.GetStr());

      if (tag)
      {
        Object &object = script.GetObject();

        // Follow 'em there
        Orders::Squad::FollowTag::Generate(object, script.GetSquad()->Id(), tag->Id(), attack, Orders::FLUSH);
      }
      else
      {
        ERR_CONFIG(("Could not resolve tag '%s'", tagName.GetStr()))
      }
    }


    //
    // Notify
    //
    U32 FollowTag::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Defect
    //


    //
    // Constructor
    //
    Defect::Defect(Script &script, FScope *fScope)
    : Action(script)
    {
      teamName = StdLoad::TypeString(fScope, "Team");
    }


    //
    // Destructor
    //
    Defect::~Defect()
    {
    }


    //
    // Execute
    //
    void Defect::Execute()
    {
      // Resolve the team
      Team *team = Team::Name2Team(teamName.str);

      if (team)
      {
        Object &object = script.GetObject();

        // Change the team of all units in the squad to the defection team
        Orders::Squad::Defect::Generate(object, script.GetSquad()->Id(), team->GetId());
      }
      else
      {
        LOG_WARN(("Defection team '%s' does not exist", teamName.str))
      }
    }


    //
    // Notify
    //
    U32 Defect::Notify(U32, U32)
    {
      return (Status::Ignored);
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportAcquire
    //


    //
    // Constructor
    //
    TransportAcquire::TransportAcquire(Script &script, FScope *fScope)
    : Action(script)
    {
      fScope;
    }


    //
    // Destructor
    //
    TransportAcquire::~TransportAcquire()
    {
    }


    //
    // Execute
    //
    void TransportAcquire::Execute()
    {
      // Release any transports this script has back to the transport manager
      script.ReleaseTransports();

      // Acquire enough transports to move this squad
      script.GetObject().GetTransportManager().FindTransports(script);
    }


    //
    // Notify
    //
    U32 TransportAcquire::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x9BA84E05: // "Transport::Enough"
          return (Status::Completed);

        case 0x3BBBD1F7: // "Transport::NotEnough"
          return (Status::Partial);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportSpawn
    //


    //
    // Constructor
    //
    TransportSpawn::TransportSpawn(Script &script, FScope *fScope)
    : Action(script),
      configName("Config", fScope, &script),
      transportName("Transport", fScope, &script),
      location(Location::Create(fScope->GetFunction("Location"), &script))
    {
    }


    //
    // Destructor
    //
    TransportSpawn::~TransportSpawn()
    {
      delete location;
    }


    //
    // Execute
    //
    void TransportSpawn::Execute()
    {
      // Release any transports this script has back to the transport manager
      script.ReleaseTransports();

      // Resolve the transport type
      TransportObjTypePtr transport;
      Resolver::Type<TransportObjType>(transport, transportName.GetIdent(), TRUE);

      // Spawn in the transport at the location
      Orders::Squad::Spawn::Generate(script.GetObject(), script.GetSquad()->Id(), transport->GetNameCrc(), location->GetVector(), 0);
    }


    //
    // Notify
    //
    U32 TransportSpawn::Notify(U32 message, U32 id)
    {
      switch (message)
      {
        case 0x883F0FF3: // "Squad::Spawn"
        {
          // Resolve this ID into a living breathing transport ...
          TransportObj * t = Resolver::Object<TransportObj, TransportObjType>(id);

          if (t)
          {
            Object &object = script.GetObject();
            Transport * transport = object.GetTransportManager().FindIdleTransport(*t);

            if (transport)
            {
              // Assign this transport to the script
              transport->AssignToSquad(&script);

              Config::RecruitType *config = Config::FindRecruitType(configName.GetCrc());
              if (!config)
              {
                ERR_CONFIG(("Could not find RecruitType '%s'", configName.GetStr()))
              }

              // Spawn in all the types
              for (Config::RecruitType::TypeList::Iterator t(&config->GetTypes()); *t; t++)
              {
                if ((*t)->Alive())
                {
                  UnitObjType *type = **t;

                  for (U32 i = 0; i < (*t)->amount; i++)
                  {
                    // Spawn in the units requested into the transport
                    Orders::Squad::TransportSpawn::Generate(object, script.GetSquad()->Id(), type->GetNameCrc(), id);
                  }
                }
              }

              // Notify that the spawn is completed
              Orders::Squad::Notify::Generate(object, script.GetSquad()->Id(), 0x667A9F3F); // "Squad::SpawnComplete"
            }
          }
          return (Status::Ignored);
        }

        case 0x667A9F3F: // "Squad::SpawnComplete"
        {
          /*
          LOG_AI(("TransportSpawn completed for '%s'", script.GetName()))

          for (SquadObj::UnitList::Iterator u(&script.GetSquad()->GetList()); *u; u++)
          {
            if ((*u)->Alive())
            {
              LOG_AI(("[%d] %s", (**u)->Id(), (**u)->UnitType()->GetName()))
            }
          }
          */
          return (Status::Completed);
        }

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportRelease
    //


    //
    // Constructor
    //
    TransportRelease::TransportRelease(Script &script, FScope *fScope)
    : Action(script),
      location(NULL)
    {
      FScope *sScope = fScope->GetFunction("Location", FALSE);
      if (sScope)
      {
        location = Location::Create(sScope, &script);
      }
    }


    //
    // Destructor
    //
    TransportRelease::~TransportRelease()
    {
      if (location)
      {
        delete location;
      }
    }


    //
    // Execute
    //
    void TransportRelease::Execute()
    {
      if (location)
      {
        script.SendTransports(location->GetVector(&script.GetObject(), &script));
      }
      else
      {
        script.SendTransports();
      }

      // Release the said transport
      script.ReleaseTransports();

      // Fire off a notification
      Orders::Squad::Notify::Generate(script.GetObject(), script.GetSquad()->Id(), 0xB0B846EE); // "Squad::TransportReleaseCompleted"
    }


    //
    // Notify
    //
    U32 TransportRelease::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0xB0B846EE: // "Squad::TransportReleaseCompleted"
          return (Status::Completed);

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportLoad
    //


    //
    // Constructor
    //
    TransportLoad::TransportLoad(Script &script, FScope *fScope)
    : Action(script),
      prune("Prune", fScope, 0, &script),
      attack("Attack", fScope, 0, &script)
    {
    }


    //
    // Destructor
    //
    TransportLoad::~TransportLoad()
    {
    }


    //
    // Execute
    //
    void TransportLoad::Execute()
    {
      // Select the transports
      Orders::Game::ClearSelected::Generate(script.GetObject());

      for (NList<Transport>::Iterator t(&script.GetTransports()); *t; t++)
      {
        if ((*t)->Alive())
        {
          Orders::Game::AddSelected::Generate(script.GetObject(), **t);
        }
      }

      // Order the squad to board the transports
      Orders::Squad::Board::Generate(script.GetObject(), script.GetSquad()->Id(), attack, Orders::FLUSH);
    }


    //
    // Notify
    //
    U32 TransportLoad::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x763C5781: // "Squad::TaskCompleted"
          return (Status::Completed);
          break;

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportUnload
    //


    //
    // Constructor
    //
    TransportUnload::TransportUnload(Script &script, FScope *fScope)
    : Action(script),
      location(Location::Create(fScope->GetFunction("Location"), &script))
    {
    }


    //
    // Destructor
    //
    TransportUnload::~TransportUnload()
    {
      delete location;
    }


    //
    // Execute
    //
    void TransportUnload::Execute()
    {
      // Select the transports
      Orders::Game::ClearSelected::Generate(script.GetObject());

      for (NList<Transport>::Iterator t(&script.GetTransports()); *t; t++)
      {
        if ((*t)->Alive())
        {
          (*t)->ClearFlag();
          Orders::Game::AddSelected::Generate(script.GetObject(), **t);
        }
      }

      // Order them to unload
      Orders::Game::Unload::Generate(script.GetObject(), location->GetVector(&script.GetObject(), &script), Orders::FLUSH);
    }


    //
    // Notify
    //
    U32 TransportUnload::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x38601711: // "Transport::Died"
        case 0x5CA4B1C1: // "Transport::Unloaded"
        {
          // Check to see if they've all had their flags set
          for (NList<Transport>::Iterator t(&script.GetTransports()); *t; t++)
          {
            if (!(*t)->TestFlag())
            {
              return (Status::Ignored);
            }
          }
          return (Status::Completed);
        }

        default:
          return (Status::Ignored);
      }

    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportMove
    //


    //
    // Constructor
    //
    TransportMove::TransportMove(Script &script, FScope *fScope)
    : Action(script),
      location(Location::Create(fScope->GetFunction("Location"), &script))
    {
    }


    //
    // Destructor
    //
    TransportMove::~TransportMove()
    {
      delete location;
    }


    //
    // Execute
    //
    void TransportMove::Execute()
    {
      script.SendTransports(location->GetVector(&script.GetObject(), &script));
    }


    //
    // Notify
    //
    U32 TransportMove::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0x998A995A: // "Transport::Moved"
        {
          // Check to see if they've all had their flags set
          for (NList<Transport>::Iterator t(&script.GetTransports()); *t; t++)
          {
            if (!(*t)->TestFlag())
            {
              return (Status::Ignored);
            }
          }
          return (Status::Completed);
        }

        default:
          return (Status::Ignored);
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TransportDelete
    //


    //
    // Constructor
    //
    TransportDelete::TransportDelete(Script &script, FScope *)
    : Action(script)
    {
    }


    //
    // Destructor
    //
    TransportDelete::~TransportDelete()
    {
    }


    //
    // Execute
    //
    void TransportDelete::Execute()
    {
      Object &object = script.GetObject();

      // Select the transports
      Orders::Game::ClearSelected::Generate(object);

      for (NList<Transport>::Iterator t(&script.GetTransports()); *t; t++)
      {
        if ((*t)->Alive())
        {
          Orders::Game::AddSelected::Generate(object, **t);
        }
      }

      // Order the the transport destruction
      Orders::Game::Delete::Generate(object);

      // Fire off a notification
      Orders::Squad::Notify::Generate(object, script.GetSquad()->Id(), 0xCC6049F4); // "Squad::TransporDeleteCompleted"
    }


    //
    // Notify
    //
    U32 TransportDelete::Notify(U32 message, U32)
    {
      switch (message)
      {
        case 0xCC6049F4: // "Squad::TransporDeleteCompleted"
          return (Status::Completed);

        default:
          return (Status::Ignored);
      }
    }


  }

}
