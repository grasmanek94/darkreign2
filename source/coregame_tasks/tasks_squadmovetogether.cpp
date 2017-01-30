///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tasks_squadmovetogether.h"
#include "taskctrl.h"
#include "tasks_unitmove.h"
#include "resolver.h"

#include "common.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Tasks
//
namespace Tasks
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  // 
  const U32 distanceMin2 = 4;
  const U32 distanceMax2 = 64;
  const U32 meetingRange = 4;
  const S32 pointDiffMax = 2;
  const U32 timeDiffMax = 50;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // State machine
  StateMachine<SquadMoveTogether> SquadMoveTogether::stateMachine;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadMoveTogether
  //


  //
  // Constructor
  //
  SquadMoveTogether::SquadMoveTogether(GameObj *subject) 
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine),
    task(NULL),
    points(&NPoint::node)
  {
  }


  //
  // Constructor
  //
  SquadMoveTogether::SquadMoveTogether(GameObj *subject, const Vector &destination) 
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    task(NULL),
    destination(destination),
    points(&NPoint::node)
  {
  }


  //
  // Constructor
  //
  SquadMoveTogether::SquadMoveTogether(GameObj *subject, TrailObj *trail, U32 index)
  : GameTask<SquadObjType, SquadObj>(staticConfig, subject),
    inst(&stateMachine, "Init"),
    task(NULL),
    trail(trail),
    index(index),
    points(&NPoint::node)
  {
  }


  //
  // Destructor
  //
  SquadMoveTogether::~SquadMoveTogether()
  {
    points.DisposeAll();
  }


  //
  // Save
  //
  void SquadMoveTogether::Save(FScope *fScope)
  {
    SaveTaskData(fScope);
    inst.SaveState(fScope->AddFunction("StateMachine"));
    StdSave::TypeVector(fScope, "Destination", destination);
    StdSave::TypeF32(fScope, "Direction", direction);

    if (trail.Alive())
    {
      StdSave::TypeReaper(fScope, "Trail", trail);
      StdSave::TypeU32(fScope, "TrailIndex", index);
    }
  }


  //
  // Load
  //
  void SquadMoveTogether::Load(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x22C4A13F: // "StateMachine"
          inst.LoadState(sScope);
          break;

        case 0xA302E408: // "Destination"
          StdLoad::TypeVector(sScope, destination);
          break;

        case 0x04BC5B80: // "Direction"
          direction = StdLoad::TypeF32(sScope);
          break;

        case 0x82698073: // "Trail"
          StdLoad::TypeReaper(sScope, trail);
          break;

        case 0xA3998582: // "TrailIndex"
          index = StdLoad::TypeU32(sScope);
          break;

        default:
          LoadTaskData(sScope);
          break;
      }
    }  
  }


  //
  // Called after all objects are loaded
  //
  void SquadMoveTogether::PostLoad()
  {
    Resolver::Object<TrailObj, TrailObjType>(trail);
  }


  //
  // Perform task processing
  //
  Bool SquadMoveTogether::Process()
  {
    inst.Process(this);  

    return (quit);
  }


  //
  // Notify this task of an event
  //
  Bool SquadMoveTogether::ProcessEvent(const Event &event)
  {
    switch (event.message)
    {
      case GameObjNotify::Interrupted:
        LOG_DIAG(("SquadMoveTogether: Interrupted"))
        inst.Set(0xABAA7B48); // "Init"
        return (TRUE);

      case 0x6B0DB5AA: // "Move::Incapable"
      {
        // One of the units in the squad is telling us
        // it can't make it to the destination
//        LOG_DIAG(("Unit %d in the squad could not make it to its destination", event.param2))

        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if (
            (*i)->Alive() && 
            (*i)->task == event.param1 &&
            (*i)->Id() == event.param2)
          {
            (*i)->completed = TRUE;
            GoToNextPoint(*i);
          }
        }
        return (TRUE);
      }

      case 0xFCBF8881: // "Move::Completed"
      {
        // One of the units in the squad is telling us 
        // it made it to the destination
//        LOG_DIAG(("Unit %d in the squad made it to its destination", event.param2))

        // Mark the completed flag for this squad member
        for (SquadObj::UnitList::Iterator i(&subject->GetList()); *i; i++)
        {
          if (
            (*i)->Alive() && 
            (*i)->task == event.param1 &&
            (*i)->Id() == event.param2)
          {
            (*i)->completed = TRUE;
            if ((*i)->data < point || TestState(0x9E947215)) // "Moving"
            {
//              LOG_DIAG(("We're in the move state so proceeding immediately"))
              GoToNextPoint(*i);
            }
            break;
          }

        }
        return (TRUE);
      }

      default:
        return (GameTask<SquadObjType, SquadObj>::ProcessEvent(event));
    }
  }


  //
  // Initial state
  //
  void SquadMoveTogether::StateInit()
  {
    LOG_DIAG(("SquadMoveTogether: Init"))

    // Find the slowest unit in the squad and use their traction type
    U8 traction = 0;
    F32 bottomSpeed = F32_MAX;

    // Work out the centre position of the squad
    Vector location(0.0f, 0.0f, 0.0f);
    U32 count = 0;
    SquadObj::UnitList::Iterator i(&subject->GetList());
    for (!i; *i; i++)
    {
      if ((*i)->Alive())
      {
        // Take this oportunity to reset the units node
        (*i)->completed = TRUE;
        (*i)->data = 0;

        if ((**i)->CanEverMove())
        {
          // Grab the unit so we don't need to continually pound the iterator
          UnitObj *unit = **i;

          // Flush its tasks
          unit->FlushTasks(Tasks::UnitMove::GetConfigBlockingPriority());

          // Add its position to the total
          location.x += unit->WorldMatrix().posit.x;
          location.z += unit->WorldMatrix().posit.z;
          count++;

          // Is this the slowest unit in the squad
          F32 speed = unit->GetMaxSpeed();
          if (speed < bottomSpeed)
          {
            bottomSpeed = speed;
            traction = unit->MapType()->GetTractionIndex(unit->MapType()->GetDefaultLayer());
          }
        }
      }
    }

    if (!count)
    {
      Quit();
      return;
    }

    // Work out the averate location
    location.x /= count;
    location.z /= count;
    location.y = TerrainData::FindFloor(location.x, location.z);

    // What is the direction from the source (location) to the dest (destination)
    VectorDir dir;
    Vector(destination - location).Convert(dir);
    direction = dir.u;

    // Build a formation from the units using the direction to the destination
    Formation::Create(location, direction, subject, 16.0f);

    // Get the source and destination in terms of cells
    Point<F32> avg(location.x, location.z);

    Point<U32> srcCell;
    WorldCtrl::MetresToCellPoint(avg, srcCell);

    Point<U32> destCell;
    WorldCtrl::MetresToCellPoint(Point<F32>(destination.x, destination.z), destCell);

    switch (subject->GetPathFinder().RequestPath(srcCell.x, srcCell.z, destCell.x, destCell.z, traction))
    {
      case PathSearch::Finder::RR_SUBMITTED:
        NextState(0xFDE9D5E3); // "Pathing"
        break;

      case PathSearch::Finder::RR_SAMECELL:
      case PathSearch::Finder::RR_OFFMAP:
        Quit();
        return;

      default:
        ERR_FATAL(("Unknown path request result"))
    }
  }


  //
  // Pathing State
  //
  void SquadMoveTogether::StatePathing()
  {
    switch (subject->GetPathFinder().State())
    {
      case PathSearch::FS_QUEUED:
      case PathSearch::FS_ACTIVE:
        break;

      case PathSearch::FS_IDLE:
        ERR_FATAL(("Path is idle when it shouldn't be"))
        break;

      case PathSearch::FS_NOPATH:
        Quit();
        break;

      case PathSearch::FS_DIRECT:
        // We should handle direct especially perhaps ?
        
      case PathSearch::FS_FOUND:
      case PathSearch::FS_CLOSEST:
        NextState(0x367BCF47); // "PathAnalysis"
        break;

      default:
        ERR_FATAL(("Unknown path finder state [%d]", subject->GetPathFinder().State()));
    }
  }


  //
  // Path Analysis State
  //
  void SquadMoveTogether::StatePathAnalysis()
  {
    Common::Display::ClearMarkers();

    // Copy the points from the path
    for (PathSearch::PointList::Iterator p(&subject->GetPathFinder().GetPointList()); *p; p++)
    {
      points.Append(new NPoint((*p)->x, (*p)->z));
    }

    // Proceed backward through the point list and remove/add points as neccesary
    NList<NPoint>::Node *node = points.GetTailNode();
    while (node)
    {
      // Is there another point ?
      NList<NPoint>::Node *next = node->GetPrev();

      if (next)
      {
        // What is the distance to this next point ?
        S32 distX = next->GetData()->x - node->GetData()->x;
        S32 distZ = next->GetData()->z - node->GetData()->z;
        U32 distance = distX * distX + distZ * distZ;

        if (distance < distanceMin2)
        {
          // Remove the next point and go again
          points.Dispose(next->GetData());
          continue;
        }

        if (distance > distanceMax2)
        {
          // Add another point halfway between the two given points and go again
          points.InsertBefore(node, new NPoint((*next->GetData() + *node->GetData()) / 2));
          continue;
        }
      }

      // Proceed to the next point
      node = next;
    }

    // Go through the point list and set the direction from each
    // point to the next one and duplicate this for the last point
    node = points.GetHeadNode();
    NList<NPoint>::Node *prev = NULL;
    point = 0;
    while (node)
    {
      Common::Display::AddMarker(node->GetData()->x, node->GetData()->z, 1.0f);

      NList<NPoint>::Node *next = node->GetNext();

      if (next)
      {
        // What is the direction to the next point from this point
        F32 distX = F32(next->GetData()->x) - F32(node->GetData()->x);
        F32 distZ = F32(next->GetData()->z) - F32(node->GetData()->z);

        node->GetData()->direction = (F32) atan2(distZ, distX);
      }
      else
      {
        node->GetData()->direction = direction;
      }
      prev = node;
      node = next;
      point++;
    }

    // Reset the point of interest to 0
    point = 0;

    NextState(0xCC45C48B); // "Waiting"
  }

  
  //
  // Moving State
  //
  void SquadMoveTogether::StateMoving()
  {
    SquadObj::UnitList::Iterator i(&subject->GetList());
    for (!i; *i; i++)
    {
      if ((*i)->Alive() && (*i)->data <= S32(points.GetCount()))
      {
        // Someone hasn't made it
        return;
      }
    }

//    LOG_DIAG(("Entire squad has made it to the destination!"))

    // Were we spawned by a task ?
    if (task)
    {
      task->ProcessEvent(Event(0xF14439C5)); // "SquadMove::Completed"
    }
    else
    {
      subject->NotifyPlayer(0x763C5781); // "Squad::TaskCompleted"
    }
    Quit();
  }


  //
  // Waiting State
  //
  void SquadMoveTogether::StateWaiting()
  {
    // We're in this state because one of the units detected they 
    // were behind, we  need to assertain if all of the units are 
    // at the same point.  This may not be possible since some units 
    // are skipping points which they cannot travel to.

    // If all of the units are beyond the current point, then it is
    // safe for all of the other units to proceed.

    SquadObj::UnitList::Iterator i(&subject->GetList());
    for (!i; *i; i++)
    {
      if ((*i)->Alive() && (*i)->data < point)
      {
        // Someone hasn't made it
        return;
      }
    }

    // All units which are waiting can now proceed to the next point
//    LOG_DIAG(("Proceeding to point %d [%d]", point, points.GetCount()))

    NextState(0x9E947215); // "Moving"

    for (!i; *i; i++)
    {
      if 
      (
        (*i)->Alive() && 
        (**i)->CanEverMove() &&
        (*i)->completed
      )
      {
        GoToNextPoint(*i);
      }
    }
  }


  //
  // GoToNextPoint
  //
  void SquadMoveTogether::GoToNextPoint(SquadObj::ListNode *node)
  {
    // We are attempting to go to the next point
    NPoint *p = points[node->data];
    node->data++;
    node->completed = TRUE;

    point = Max(point, node->data);

    ASSERT(node->Alive())
    UnitObj *unit = node->GetData();

    if (p)
    {
      // Mark the time a unit attempts to move to this point
      if (p->timestamp)
      {
        // The squad will need to wait for me to catch up
        if (GameTime::SimCycle() - p->timestamp > timeDiffMax)
        {
//          LOG_DIAG(("Detected that we're [%d] way behind at this point, changing to waiting state", node->Id()))
          NextState(0xCC45C48B); // "Waiting"
        }
      }
      else
      {
        p->timestamp = GameTime::SimCycle();

        // Since we're the first unit to this point we 
        // should check to make sure that no units are 
        // more than two points behind.  If there are,
        // then we'll have to wait here for them

        SquadObj::UnitList::Iterator i(&subject->GetList());
        for (!i; *i; i++)
        {
          if 
          (
            (*i)->Alive() && 
            ((*i)->data + pointDiffMax < point)
          )
          {
//            LOG_DIAG(("We're [%d] first to this point [%d] and detected that [%d] is way behind", node->Id(), node->data, (*i)->Id()))
            NextState(0xCC45C48B); // "Waiting"
            return;
          }
        }
      }

      // Find the closest point to that point that we can go to
      U32 x, z;

      // Use the formation slot offset for this unit
      Vector location(WorldCtrl::CellToMetresX(p->x), 0.0f, WorldCtrl::CellToMetresZ(p->z));

      F32 dir = p->direction + node->slot.direction;
      VectorDir::FixU(dir);

      F32 orient = p->direction + node->slot.orientation;
      VectorDir::FixU(orient);

      Vector offset;
      offset.x = (F32) cos(dir);
      offset.y = 0.0f;
      offset.z = (F32) sin(dir);
      offset *= node->slot.distance;
      offset += location;

      // Make sure the point is on the map
      WorldCtrl::ClampMetreMap(offset.x, offset.z);

      x = WorldCtrl::MetresToCellX(offset.x);
      z = WorldCtrl::MetresToCellZ(offset.z);

      if (PathSearch::FindClosestCell(unit->MapType()->GetTractionIndex(unit->MapType()->GetDefaultLayer()), x, z, x, z, meetingRange))
      {
        node->completed = FALSE;
        unit->PrependTask(new Tasks::UnitMove(unit, offset));
      }
      else
      {
        // Cound not find a cell, go to the next point
        GoToNextPoint(node);
      }
    }
  }


  //
  // Initialization
  //
  void SquadMoveTogether::Init()
  {
    // Setup config
    staticConfig.Setup();

    // Build state machine
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Pathing", &StatePathing);
    stateMachine.AddState("PathAnalysis", &StatePathAnalysis);
    stateMachine.AddState("Moving", &StateMoving);
    stateMachine.AddState("Waiting", &StateWaiting);
  }


  //
  // Shutdown
  //
  void SquadMoveTogether::Done()
  {
    stateMachine.CleanUp();
  }

}
