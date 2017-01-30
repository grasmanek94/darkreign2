///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Weapon system
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "weapon.h"
#include "physicsctrl.h"
#include "sight.h"
#include "mapobjiter.h"
#include "mapobjctrl.h"
#include "ray.h"
#include "taskctrl.h"
#include "tasks_unitattack.h"
#include "tasks_unitguard.h"
#include "tasks_restoremobile.h"
#include "movement_pathfollow.h"
#include "collisionctrl.h"
#include "random.h"
#include "sync.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Weapon
//
namespace Weapon
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Constants
  //

  // How long we should wait between telling the unit to move
  const F32 MinimumMoveInterval = 5.0f;

  // How long we can track the target without seeing it
  const F32 MaximumTargetUnseenInterval = 2.0f;

  // How long do we need to have not requested movement before we should shoot
  const F32 MinimumMoveRequestDelay = 1.0f;

  // How long we should wait before returning
  const F32 MinimumWaitDelay = 3.0f;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Solution
  //
  namespace Solution
  {
    const U32 Ok              = 0xA4C4F136; // "Ok"
    const U32 Left            = 0xBA190163; // "Left"
    const U32 Right           = 0xE2DDD72B; // "Right"
    const U32 Up              = 0xF975A769; // "Up"
    const U32 Down            = 0xEF54F336; // "Down"
    const U32 TooFar          = 0xD5B35F1A; // "TooFar"
    const U32 TooClose        = 0xC9C8D4A4; // "TooClose"
    const U32 NoAmmunition    = 0xE0524EE3; // "NoAmmunition"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace States
  //
  namespace State
  {
    const U32 Idle            = 0x793360CC; // "Idle"
    const U32 Moving          = 0x9E947215; // "Moving"
    const U32 Waiting         = 0xCC45C48B; // "Waiting"
    const U32 Returning       = 0x1E2A5702; // "Returning"
    const U32 Ready           = 0x9DB00C1E; // "Ready"
    const U32 PreFire         = 0x43B55F8D; // "PreFire"
    const U32 Fire            = 0xEBEB1BE9; // "Fire"
    const U32 Firing          = 0xA4B70EC1; // "Firing"
    const U32 PostFire        = 0x4B249129; // "PostFire"
  }

  const char * GetStateName(U32 state)
  {
    switch (state)
    {
      case State::Idle:
        return ("Idle");

      case State::Moving:
        return ("Moving");

      case State::Waiting:
        return ("Waiting");

      case State::Returning:
        return ("Returning");

      case State::Ready:
        return ("Ready");

      case State::PreFire:
        return ("PreFire");

      case State::Fire:
        return ("Fire");

      case State::Firing:
        return ("Firing");

      case State::PostFire:
        return ("PostFire");

      default:
        ERR_FATAL(("Unkown state!"))
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //


  //
  // Object::Object
  //
  // Constructor
  //
  Object::Object(Type &type, UnitObj &unit)
  : type(type),
    unit(unit),
    state(State::Idle),
    damage(type.damage),
    delay(&type.delay),
    lastFireTime(0.0F),
    lastWaitTime(0.0f),
    firingStartTime(0.0f),
    lastMoveTime(0),
    lastMoveRequestTime(-MinimumMoveRequestDelay),
    lastTargetSeenTime(0.0f),
    unitLastPosition(0.0f, 0.0f, 0.0f),
    targetLastPosition(0.0f, 0.0f, 0.0f),
    guardPosition(0.0f, 0.0f, 0.0f),
    solution(0.0f, 0.0f, 0.0f),
    damageApplied(0),
    animFrame(0.0f),
    targetMoving(FALSE),
    turnToAngle(0.0F)
  {
    // Set amunition to full
    ammunition = type.ammunition;

    // Calculate the effective range of this weapon

    // If the weapon can fire indirectly then the weapons range is the configured range
    // Otherwise, the range of the weapon is clamped to the parents seeing range.
    F32 range = type.maxRange;

    if (!unit.UnitType()->CanFireIndirect())
    {
      range = Min(range, WorldCtrl::CellSize() * unit.UnitType()->GetMaxSeeingRange());
    }

    ClearBadCell();
  }


  //
  // Object::~Object
  //
  // Destructor
  //
  Object::~Object()
  {
  }


  //
  // Expend one projectile worth of ammunition
  //
  void Object::ExpendAmmunition()
  {
    if (type.ammunition)
    {
      // We shouldn't be expending ammunition if we have none
      ASSERT(ammunition)

      ammunition--;

      if (!ammunition)
      {
        unit.PostRestoreNowEvent();
      }
    }
  }


  //
  // Set the state
  //
  void Object::SetState(U32 newState)
  {
    //LOG_DIAG(("Setting state to '%s' [%08X] from '%s' [%08X]", 
    //  GetStateName(newState), newState, GetStateName(state), state))

    switch (newState)
    {
      case State::Idle:
        state = State::Idle;
        break;

      case State::Moving:
        state = State::Moving;
        break;

      case State::Waiting:
        lastWaitTime = GameTime::SimTotalTime();
        state = State::Waiting;
        break;

      case State::Returning:
        state = State::Returning;
        break;

      case State::Ready:
        // Drop out of direct control mode
        if (IsAllowedMovement())
        {
          unit.GetDriver()->SetupDirectControl(FALSE);
        }
        switch (state)
        {
          case State::PreFire:
          case State::Fire:
          case State::Firing:
            unit.SetAnimation(0x4B249129); // "PostFire"
            state = State::PostFire;
            break;

          default:
            state = newState;
            break;
        }
        break;

      case State::PreFire:
        unit.SetAnimation(0x43B55F8D); // "PreFire"
        state = State::PreFire;
        break;

      case State::Fire:
        state = State::Fire;
        StartFiring();
        break;

      case State::Firing:
        state = State::Firing;
        break;

      case State::PostFire:
        unit.SetAnimation(0x4B249129); // "PostFire"
        state = State::PostFire;
        break;
    }
  }


  //
  // Is the object idle
  //
  Bool Object::IsIdle()
  {
    return (state == State::Idle);
  }


  //
  // IsAllowedMovement
  //
  Bool Object::IsAllowedMovement()
  {
    return 
    (
      (
        TaskCtrl::Promote<Tasks::UnitAttack>(&unit) ||
        TaskCtrl::Promote<Tasks::UnitGuard>(&unit)
      ) &&
      unit.CanEverMove() ? TRUE : FALSE
    );
  }


  //
  // Start Firing
  //
  void Object::StartFiring()
  {
    firingStartTime = GameTime::SimTotalTime();
    damageApplied = 0;
  }


  //
  // Set the direction of a node
  //
  void Object::SetNodeMatrix(const NodeIdent &node, const VectorDir &dir)
  {
#ifdef DEVELOPMENT
    //LOG_DIAG(("SetNodeMatrix %8.2f,%8.2f", dir.u, dir.v))
#endif

    // Convert the direction into cartesian coordinates
    Vector vect;
    vect.Set(dir);

    // Using this vector as the front vector build a matrix
    Matrix matrix = unit.ObjectMatrix(node);
    matrix.Up() = Matrix::I.up;
    matrix.SetFromFront(vect);

    // Set the matrix on the unit
    unit.SetSimTarget(node, matrix);
  }


  //
  // Calculate a firing solution
  //
  U32 Object::FiringSolution(Vector &solution, Vector &objSolution, F32 &fParam, const Target &target)
  {
    //SYNC_BRUTAL("FiringSolution: " << target.GetInfo())
    //SYNC_BRUTAL("Mode: " << Utils::FP::GetState())

    // Do we have ammunition ?
    if (!HaveAmmunition())
    {
      return (Solution::NoAmmunition);
    }

    // Position to perform calculations from
    Vector position = GetFiringLocation();

    // Get the targets location
    Vector targetVector = target.GetLocation();

    // Add in any offset
    targetVector.y += type.heightOffset;

    // Is this a projectile weapon with arc trajectory projectiles ?
    if 
    (
      type.style == Style::Projectile && 
      type.projectileType->GetModel() == ProjectileModel::ArcTrajectory
    )
    {
      // Check to see if the target is too close
      if (Vector(targetVector - position).Magnitude2() < type.minRange2)
      {
        return (Solution::TooClose);
      }

      F32 time;

      // Ask the projectile for an angular solution and a range check
      if (CalculateArc(targetVector - position, type.initialSpeed, solution, time))
      {
        // If we're allowed to recompute using the time of flight to determine a lead angle
        if (
          (type.flags & Flags::LeadAngle) &&
          (target.GetType() == Target::OBJECT))
        {
          ASSERT(target.Alive())

          // Add to the target vector the velocity of the target (if any) times this tie
          targetVector += target.GetObj()->GetVelocity() * time;

          if (!CalculateArc(targetVector - position, type.initialSpeed, solution, time))
          {
            return (Solution::TooFar);
          }
        }

        // What's the offset of the barrel now
        solution.Normalize();
  
        Matrix m;
        m.ClearData();
        m.up = Matrix::I.up;
        m.SetFromFront(solution);
        Vector offset = unit.UnitType()->barrel;
        m.Rotate(offset);

        // Remove the barrel offset and recompute
        targetVector -= offset;

        if (!CalculateArc(targetVector - position, type.initialSpeed, solution, time))
        {
          return (Solution::TooFar);
        }
      }
      else
      {
        return (Solution::TooFar);
      }
    }
    else
    {
      // Get the straight line to the target
      solution = targetVector - position;

      // Recalculate solution to nearest point on bounding box
      if ((type.flags & Flags::AimAtBox) && (target.GetType() == Target::OBJECT))
      {
        F32 dist = 0.0F;

        if (Ray::Test(target.GetObj(), position, targetVector, dist, Ray::BOX))
        {
          // If inside the box, use a small distance
          if (dist < 0.01F)
          {
            dist = 0.01F;
          }

          // Reduce length of solution vector, but maintain direction
          F32 scale = F32(sqrt(dist)) / solution.Magnitude();
          ASSERT(scale <= 1.0F)
          solution *= scale;
          targetVector = position + solution;
        }
      }

      // If the projectile has a straight trajectory 
      // and we're allowed to, recompute from the 
      // position the target will be in when the 
      // projectile reaches it
      if 
      (
        (type.flags & Flags::LeadAngle) &&
        (target.GetType() == Target::OBJECT) &&
        (type.style == Style::Projectile) && 
        (type.projectileType->GetModel() == ProjectileModel::StraightTrajectory)
      )
      {
        // Given the distance to the target, 
        // and the speed of the projectile, 
        // work out the time 
        F32 time = solution.Magnitude() / type.initialSpeed;

        ASSERT(target.Alive())

        // Add to the target vector the velocity of the target (if any) times this tie
        targetVector += target.GetObj()->GetVelocity() * time;

        // Get the new solution
        solution = targetVector - position;
      }

      // What's the offset of the barrel now
      solution.Normalize();
      Matrix m;
      m.ClearData();
      m.up = Matrix::I.up;
      m.SetFromFront(solution);
      Vector offset = unit.UnitType()->barrel;
      m.Rotate(offset);

      // Remove the barrel offset and recompute
      targetVector -= offset;

      // Recompute
      solution = targetVector - position;

      // Is this target out of range ?
      if (solution.Magnitude2() > type.maxRange2)
      {
        return (Solution::TooFar);
      }

      // Is the target too close ?
      if (solution.Magnitude2() < type.minRange2)
      {
        return (Solution::TooClose);
      }
    }


    // Normalize the solution
    solution.Normalize();

    // We have a solution, can the weapon acheive the solution without requiring movement

    // Calculate solution in object space of unit
    Matrix m;
    m.ClearData();
    objSolution = solution;
    m.SetInverse(unit.WorldMatrix());
    m.Rotate(objSolution);

    // Convert the solution into polar coordinates
    VectorDir solutionDir;
    objSolution.Convert(solutionDir);
    solutionDir -= VectorDir(PIBY2, PIBY2);

    // LOG_DIAG(("Solution is %8.2f,%8.2f", solutionDir.u, solutionDir.v))
    // LOG_DIAG(("[%s] Firing solution for %g %g", type->GetName(), solutionDir.u * RAD2DEG, solutionDir.v * RAD2DEG))
    // LOG_DIAG(("Object %g %g Solution %g %g", objectDir.u * RAD2DEG, objectDir.v * RAD2DEG, solutionDir.u * RAD2DEG, solutionDir.v * RAD2DEG))

  /*
    // Does the solution indicate that it is behind us ?
    if (solutionDir.v < 0)
    {
      LOG_DIAG(("Solution is above and behind us, inverting horizontal axis"))
      solutionDir.u = -solutionDir.u;
      solutionDir.v = -solutionDir.v;
    }
    if (solutionDir.v > PI)
    {
      LOG_DIAG(("Solution is belose and behind us, inverting horizontal axis"))
      solutionDir.u = -solutionDir.u;
      solutionDir.v = PI - solutionDir.v;
    }
  */

    // Convert the polar objet coordinates into polar weapon coordinates
    solutionDir -= type.angleFront;

    // LOG_DIAG(("Weapon %g %g Solution %g %g", type->Angle().u * RAD2DEG, type->Angle().v * RAD2DEG, solutionDir.u * RAD2DEG, solutionDir.v * RAD2DEG))

    // Is the solution in the allowed movement frustrum ?

    // Horizontal solution must be between -horizontal separation and +horizontal separation
    // Really small values should be avoided as some precision is lost in the movement system
    // The minimum should be less than half the minimum frustrum
    const F32 MinTurn = 0.5F * DEG2RAD;

    if (solutionDir.u < -type.separation.u)
    {
      // Note: turn angle is negated when turning right
      fParam = solutionDir.u - type.separation.u;
      ASSERT(fParam <= 0.0F)
      fParam = Min<F32>(fParam, -MinTurn);
      return (Solution::Right);
    }

    if (solutionDir.u > type.separation.u)
    {
      fParam = solutionDir.u - type.separation.u;
      ASSERT(fParam >= 0.0F)
      fParam = Max<F32>(fParam, MinTurn);
      return (Solution::Left);
    }

    // Vertical solution must be between -vertical separation and +vertical separation
    if (solutionDir.v < -type.separation.v)
    {
      fParam = type.separation.v - solutionDir.v;
      return (Solution::Up);
    }
    if (solutionDir.v > type.separation.v)
    {
      fParam = solutionDir.v - type.separation.v;
      return (Solution::Down);
    }

    // Set fParam to be the offset from the target
    fParam = solutionDir.u;

    return (Solution::Ok);
  }


  //
  // Object::CalculateArc
  //
  // Calcuates the direction to shoot to hit a target with a parabolic projectile
  //
  Bool Object::CalculateArc(const Vector &deltaPos, F32 speed, Vector &velocity, F32 &time)
  {
    // Common subexpressions
    F32 speed2 = speed * speed;             // initial speed
    F32 positX2 = deltaPos.x * deltaPos.x;  // x difference squared
    F32 positY2 = deltaPos.y * deltaPos.y;  // y difference squared
    F32 positZ2 = deltaPos.z * deltaPos.z;  // z difference squared
    F32 positXZ2 = positX2 + positZ2;       // distance in XZ plane

    // Quadratix term coefficient
    F32 a = positY2 / positXZ2 + 1.0f;

    // Linear term coefficient
    F32 b = speed2 - deltaPos.y * PhysicsCtrl::GetGravity();

    // Constant term coefficient
    F32 c = 0.25f * PhysicsCtrl::GetGravity() * PhysicsCtrl::GetGravity() * positXZ2;

    // Discriminant
    F32 d = b * b - 4 * a * c;

    // Out of range ?
    if (d < 0)
    {
      return (FALSE);
    }

    // Get the square root of the discriminant
    d = (F32) sqrt(d);

    // Velocity in the XZ plane
    F32 velocityXZ2;
    F32 invTime;

    // Pick preferred trajectory
    velocityXZ2 = ((type.flags & Flags::HighTrajectory) ? b - d : b + d) / (a + a);

    // Inverse of time to impact
    invTime = (F32) sqrt(velocityXZ2 / positXZ2);

    // Time
    time = 1.0f / invTime;

    // Velocity in world coordinates
    velocity.x = deltaPos.x * invTime;
    velocity.y = deltaPos.y * invTime + 0.5f * PhysicsCtrl::GetGravity() * time;
    velocity.z = deltaPos.z * invTime;

    return (TRUE);
  }


  //
  // Object::LaunchProjectile
  //
  void Object::LaunchProjectile(Matrix &m, F32 speed)
  {
    if (HaveAmmunition())
    {
      // Expend the ammunition
      ExpendAmmunition();

      ProjectileObj *projectile = (ProjectileObj *) type.projectileType->NewInstance(0);

      // Set the location of the projectile
      projectile->SetSimCurrent(m);

      // Setup the projectile
      projectile->Setup
      (
        &unit, unit.GetTeam(), &type, target, damage.GetInstanceModifier().GetInteger(),
        speed ? speed : type.initialSpeed
      );

      // Add the object to the map
      MapObjCtrl::AddToMap(projectile);
    }
  }


  //
  // Move to target
  //
  void Object::MoveToTarget()
  {
    // Save the time we were asked to move
    lastMoveRequestTime = GameTime::SimTotalTime();

    // Are we allowed to control movement ?
    if (IsAllowedMovement())
    {
      Task *task = unit.GetCurrentTask();

      // Is there a task invovled
      if (task)
      {
        // Did the task come from an order ?
        if (task->GetFlags() & Task::TF_FROM_ORDER)
        {
          if (task->GetFlags() & Task::TF_FLAG2)
          {
            // If the move without attacking flag is 
            // set then we should break off the attack
            BreakOffAttack();
            return;
          }
        }
        else 
        {
          // Are we mean't to be holding our position ?
          if (Tactical::QueryProperty(task->GetTaskTable(), &unit, Tactical::TP_HOLDPOSITION))
          {
            BreakOffAttack();
            return;
          }

          // Are we not mean't to follow the subject beyond our line of sight ?
          if (!Tactical::QueryProperty(task->GetTaskTable(), &unit, Tactical::TP_FOLLOWSUBJECT))
          {
            F32 seeingRange = F32(unit.GetSeeingRange()) * WorldCtrl::CellSize();
            seeingRange *= seeingRange;

            // Are we more than our line of sight from the guard position ?
            if (Vector(unit.RootOrigin() - guardPosition).Magnitude2() > seeingRange)
            {
              BreakOffAttack();
              return;
            }
          }
        }
      }

      // If we're trying to move, check to see how long ago it was that we saw the target
      if (GameTime::SimTotalTime() - lastTargetSeenTime > MaximumTargetUnseenInterval)
      {
        BreakOffAttack();
        return;
      }

      // If we're not moving, or
      // The target is moving and we haven't updated in a while
      if 
      (
        (state != State::Moving) ||
        (GameTime::SimTotalTime() - lastMoveTime > MinimumMoveInterval && targetMoving)
      )
      {
        unit.GetDriver()->SetupDirectControl(FALSE);
        unit.Move(moveHandle, &target.GetLocation(), FALSE, NULL);
        SetState(State::Moving);

        // Save the last time we moved
        lastMoveTime = GameTime::SimTotalTime();
      }
    }
    else
    {
      // We should give up since we can't move
      BreakOffAttack();
    }
  }


  //
  // Object::IsBadCell
  //
  // Are we still on the last bad cell?
  //
  Bool Object::IsBadCell()
  {
    return (unit.GetCellX() == lastBadCellX) && (unit.GetCellZ() == lastBadCellZ);
  }


  //
  // Object::ClearBadCell
  //
  // Clear the last bad cell
  //
  void Object::ClearBadCell()
  {
    lastBadCellX = lastBadCellZ = U16_MAX;
  }


  //
  // Object::MarkBadCell
  //
  // Mark the current cell as bad
  //
  void Object::MarkBadCell()
  {
    lastBadCellX = U16(unit.GetCellX());
    lastBadCellZ = U16(unit.GetCellZ());
  }


  //
  // Object::ClearLineOfFire
  //
  // Can we hit the target from here without hitting an obstacle
  //
  Bool Object::ClearLineOfFire()
  {
    if (
      !(type.style == Style::Projectile && 
        type.projectileType->GetModel() == ProjectileModel::ArcTrajectory))
    {
      // Allow a larger tolerance along the ray if shooting at terrain
      F32 rayTolerance = target.GetType() == Target::LOCATION ? 0.5F : 0.1F;
      Bool testWater = target.GetType() == Target::LOCATION;

      // Small cushion above terrain
      F32 terrainError = 0.005F;

      return (!Ray::TerrainTest(GetFiringLocation(), target.GetLocation(), rayTolerance, terrainError, testWater, NULL));
    }
    else
    {
      // Arcing projectiles pass this test always
      return (TRUE);
    }
  }


  //
  // GetTargetPos
  //
  Bool Object::GetTargetPos( Vector & tvect, Bool onSurface) // = TRUE
  {
    if (!target.Alive())
    {
      return FALSE;
    }

    // If the target is a unit, select a random location on the bounding bounds of the target
    if (target.GetType() == Target::OBJECT)
    {
      MapObj * targetObj = target.GetObj();

      tvect = targetObj->Origin();

      if (onSurface)
      {
        // Select a random distance up to 0.5 of the radius
        VectorDir dir;
        dir.u = Random::nonSync.Float() * PI2 - PI;
        dir.v = Random::nonSync.Float() * PI;

        Vector offset = dir;
        const Bounds &bounds = targetObj->ObjectBounds();
        offset *= Min(Min(bounds.Height(), bounds.Width()), bounds.Breadth()) * Random::nonSync.Float();

        tvect += offset;
      }
    }
    else
    {
      // Use the ground location
      tvect = target.GetLocation();
    }
    return TRUE;
  }


  //
  // GetDelayPercent
  //
  // Get the current delay as a percentage of the max
  //
  F32 Object::GetDelayPercent()
  {
    // Do we have a last fire time
    if (lastFireTime)
    {
      // Get the current delay
      F32 d = delay.GetFPoint();

      if (d > 0.0F)
      {
        // Get the number of seconds since last fire time
        F32 elapsed = (GameTime::SimTotalTime() - lastFireTime) * unit.GetEfficiency();
    
        // Is there still time to wait
        if (elapsed < d)
        {
          return (elapsed / d);
        }
      }
    }

    // Ready to fire
    return (1.0F);
  }


  //
  // Object::FireCallback
  //
  // Fire Callback
  //
  Bool Object::FireCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context)
  {
    ASSERT(context)
    ASSERT( (Promote::Object<UnitObjType, UnitObj>(mapObj)) )

    UnitObj *unitObj = static_cast<UnitObj *>(mapObj);
    NodeIdent *node = static_cast<NodeIdent *>(context);

    Weapon::Object *weapon = unitObj->GetWeapon();
    ASSERT(weapon)

    if (
      weapon->state == State::Fire ||
      weapon->state == State::Firing)
    {
      // We're going to update the cbd
      cbd.particle.valid = TRUE;

      // Set the position of the weapon
      cbd.particle.matrix = node ? unitObj->WorldMatrix(*node) : unitObj->WorldMatrix();

      if (weapon->GetTargetPos( cbd.particle.length, FALSE))
      {
        cbd.particle.length -= cbd.particle.matrix.posit;

        // Keep goin, it aint dead yet
        return (FALSE);
      }
      else
      {
        // Target is dead, bail out
        return (TRUE);
      }
    }
    else
    {
      // We're done
      return (TRUE);
    }
  }



  //
  // Object::Process
  //
  // Per-cycle processing
  //
  void Object::Process()
  {
    damage.GetInstanceModifier().Simulate();
    delay.Simulate();

    // Clear target moving flag
    targetMoving = FALSE;

    // What state is the weapon in ?
    switch (state)
    {
      case State::Idle:
        // We have no target, bail
        return;

      case State::Ready:
        // If our target is invalid then we now become idle
        if (!target.Valid())
        {
          SetState(State::Idle);
          return;
        }
        break;

      case State::Returning:
        return;

      case State::Waiting:

        // have we waited long enough, if so, go to returning state
        if (GameTime::SimTotalTime() - lastWaitTime > MinimumWaitDelay)
        {
          // Notify any waiting task that we're now done
          unit.PostEvent(Task::Event(Notify::Completed));

          // Return to the guard position
          unit.GetDriver()->SetupDirectControl(FALSE);

          unit.Move(moveHandle, &guardPosition, FALSE, NULL);
          SetState(State::Returning);

          // Save the time we were asked to move (we do this to prevent fringe effects)
          lastMoveRequestTime = GameTime::SimTotalTime();

          // Save the last time we moved
          lastMoveTime = GameTime::SimTotalTime();
        }
        return;

      case State::Moving:
        break;

      case State::Fire:
      case State::Firing:
        // The main event.
        break;

      case State::PreFire:
        return;

      case State::PostFire:
        return;

      default:
        ERR_FATAL(("Unknown Weapon State 0x%08X", state));
    }


    //
    // If the target is dead then leave the firing state and return to the idle state
    //
    if (!target.Alive() || !target.Valid())
    {
      // Are we a guard, if so, return to our original position
      Task *task = unit.GetCurrentTask();
      if (
        IsAllowedMovement() && 
        task && 
        !(task->GetFlags() & Task::TF_FROM_ORDER) &&
        !Tactical::QueryProperty(task->GetTaskTable(), &unit, Tactical::TP_FOLLOWSUBJECT))
      {
        // Return to the guard position
        SetState(State::Waiting);
      }
      else

      // If we told the unit to move tell it to stop
      if (state == State::Moving)
      {
        unit.PostEvent(Task::Event(Notify::Completed));
        unit.Stop();
      }
      else
      {
        unit.PostEvent(Task::Event(Notify::Completed));
        HaltFire();
      }

      return;
    }

    // Has the target moved ?
    if (targetLastPosition != target.GetLocation())
    {
      targetLastPosition = target.GetLocation();
      targetMoving = TRUE;
    }

    // Can fire indirectly ? 
    if (unit.GetTeam() && unit.UnitType()->CanFireIndirect())
    {
      // Can the team see the target
      if (target.IsVisible(unit.GetTeam()))
      {
        // Record the time we saw the target
        lastTargetSeenTime = GameTime::SimTotalTime();
      }
      else
      {
        // Move to the target
        MoveToTarget();

        return;
      }
    }
    else
    {
      // The unit has to be able to see the target else we'll need to move
      if (target.IsVisible(&unit))
      {
        // Record the time we saw the target
        lastTargetSeenTime = GameTime::SimTotalTime();
      }
      else
      {
        // If our team can see the unit then we get to update the last seen position
        if (target.IsVisible(unit.GetTeam()))
        {
          // Record the time we saw the target
          lastTargetSeenTime = GameTime::SimTotalTime();
        }

        // Move to the target
        MoveToTarget();

        return;
      }
    }

    // Have we moved ?
    if 
    (
      targetMoving || 
      unitLastPosition != unit.RootOrigin()
    )
    {
      unitLastPosition = unit.RootOrigin();
    }
    else
    {
      switch (state)
      {
        case State::Fire:
        case State::Firing:
          // Since neither has moved perform regular process firing
          ProcessFiring();
          return;
      }
    }

    // Calculate current orientation, used as basis for turnToAngle
    const Vector &front = unit.WorldMatrix().front;
    F32 currOrientation = F32(atan2(front.z, front.x));

    // Compute a new firing solution
    F32 turn;

    switch (FiringSolution(solution, objSolution, turn, target))
    {
      case Solution::Ok:
      {
        // Adjust orientation by exact angle to target
        turnToAngle = currOrientation + turn;

        // Check to see if we haven't requested a move recently
        if (!targetMoving && (GameTime::SimTotalTime() - lastMoveRequestTime < MinimumMoveRequestDelay))
        {
          // We should not engage since we are on the fringe of our range and we
          // will probably have to move again before we get a single shot off
        }
        else
        {
          // Test for clear line of fire to target if its not an arcing weapon
          if (!ClearLineOfFire())
          {
            MarkBadCell();
            MoveToTarget();
            break;
          }

          // If we are moving, stop
          if (
            IsAllowedMovement() && 
            !unit.GetDriver()->IsStopped() && 
            !unit.GetDriver()->IsDirect())
          {
            unit.GetDriver()->Stop();
          }

          // Current cell is OK
          ClearBadCell();

          // Good solution found, go with it
          switch (state)
          {
            case State::Ready:
              SetState(State::PreFire);
              return;

            case State::Fire:
            case State::Firing:
              ProcessFiring();
              break;
          }
        }
        break;
      }

      case Solution::Left:
      case Solution::Right:
      {
        // Turn directly by the distance returned from FiringSolution
        turnToAngle = currOrientation + turn;

        if (IsAllowedMovement())
        {
          // Can we see the unit from here
          // Ideally, this should test the line of fire with the unit rotated to "turnToAngle"
          if (!ClearLineOfFire())
          {
            // No, don't stop moving
            MarkBadCell();

            // Or start moving if not moving
            if (unit.GetDriver()->IsStopped())
            {
              MoveToTarget();
            }
            break;
          }

          // If we are moving, stop
          if (
            !unit.GetDriver()->IsStopped() && 
            !unit.GetDriver()->IsDirect())
          {
            unit.GetDriver()->Stop();
          }

          // The target is to the left, 
          // we'll need to rotate the unit to the left by at least 'turn'

          // If we're allowed to turn the unit then turn it
          if (unit.GetDriver()->AllowDirectControl())
          {
            // Make sure we're in direct control mode
            unit.GetDriver()->SetupDirectControl(TRUE);

            // We'll need to incorporate the centre of the firing frustrum 
            // into this in the case where units have a sideways firing weapon
            unit.GetDriver()->DirectTurn(turnToAngle);
          }
        }

        break;
      }

      case Solution::TooFar:

        // Move closer
        MoveToTarget();
        break;

      case Solution::TooClose:
        // Move further away ???

      case Solution::Up:
      case Solution::Down:
      case Solution::NoAmmunition:

        // Break off the attack
        BreakOffAttack();
        break;

      default:
        ERR_FATAL(("Unknown result from the weapon"))
        break;
    }
  }


  //
  // Object::ProcessFiring
  //
  void Object::ProcessFiring()
  {
    // Convert the solution to polar coordinates
    VectorDir solutionDir;
    objSolution.Convert(solutionDir);
    solutionDir -= VectorDir(PIBY2, PIBY2);

    // If we need to, turn the weapon to face the target
    if (TurnToSolution(solutionDir) < type.turnError)
    {
      // The weapon is close enough to the solution, we are authorized for weapons release

      // Are we in state Fire or Firing ?
      switch (state)
      {
        case State::Fire:

          // Has there been sufficient delay between firing ?
          if (ReadyToFire())
          {
            // Save the time which the weapon was fired
            lastFireTime = GameTime::SimTotalTime();

            // Does the unit has a firing animation ?
            if (unit.HasAnimation(0xEBEB1BE9)) // "Fire"
            {
              // Play the firing animation
              unit.SetAnimation(0xEBEB1BE9, FALSE); // "Fire"
              animFrame = -1.0f;

              // Change to state firing
              SetState(State::Firing);
            }
            else
            {
              // No animation, fire the weapon
              Fire(NULL);
            }
          }
          break;

        case State::Firing:
        {
          F32 from; 
          F32 to;

          if (type.style == Style::Constant)
          {
            // Fire from the origin
            Fire(NULL);
          }
          else
          {
            if (unit.GetAnimationFrame(from, to))
            {
              // Has the animation moved
              if (to > animFrame)
              {
                animFrame = to;
                //LOG_DIAG(("Anim frame: From %f To %f", from, to))

                if (type.numFirePoints)
                {
                  // Check the fire points of the weapon to see if any of the should be fired
                  for (U32 f = 0; f < type.numFirePoints; f++)
                  {
                    if (
                      type.firePoints[f].frame >= from && 
                      type.firePoints[f].frame < to)
                    {
                      // Fire from this point
                      Fire(&type.firePoints[f]);
                    }
                  }
                }
                else
                {
                  if (0.0f >= from && 0.0f < to)
                  {
                    // Fire from the origin
                    Fire(NULL);
                  }
                }
              }
            }

            // Has there been sufficient delay between firing ?
            if (ReadyToFire())
            {
              //LOG_DIAG(("Ready to fire"))

              // Save fire time
              lastFireTime = GameTime::SimTotalTime();

              // Play the units firing animation (NO BLEND!)
              unit.SetAnimation(0xEBEB1BE9, FALSE); // "Fire"
              animFrame = -1.0f;
            }
          }

          break;
        }
      }
    }

    // If we're allowed to turn the unit then turn it
    if (IsAllowedMovement() && unit.GetDriver()->AllowDirectControl())
    {
      // Make sure we're in direct control mode
      unit.GetDriver()->SetupDirectControl(TRUE);

      // We'll need to incorporate the centre of the firing frustrum 
      // into this in the case where units have a sideways firing weapon
      unit.GetDriver()->DirectTurn(turnToAngle);
    }

  }


  //
  // Object::SaveState
  //
  // Save a state configuration scope
  //
  void Object::SaveState(FScope *fScope)
  {
    StdSave::TypeU32(fScope, "State", state);
    damage.SaveState(fScope->AddFunction("Damage"));
    delay.SaveState(fScope->AddFunction("Delay"));
    StdSave::TypeU32(fScope, "Ammunition", ammunition);

    if (target.Valid())
    {
      target.SaveState(fScope->AddFunction("Target"));
    }

    StdSave::TypeVector(fScope, "UnitLastPosition", unitLastPosition);
    StdSave::TypeVector(fScope, "TargetLastPosition", targetLastPosition);
    StdSave::TypeVector(fScope, "GuardPosition", guardPosition);
    StdSave::TypeVector(fScope, "Solution", solution);
    StdSave::TypeF32(fScope, "LastFireTime", lastFireTime);
    StdSave::TypeF32(fScope, "LastWaitTime", lastWaitTime);
    StdSave::TypeF32(fScope, "FiringStartTime", firingStartTime);
    StdSave::TypeF32(fScope, "LastMoveTime", lastMoveTime);
    StdSave::TypeF32(fScope, "LastMoveRequestTime", lastMoveRequestTime);
    StdSave::TypeF32(fScope, "LastTargetSeenTime", lastTargetSeenTime);
    StdSave::TypeU32(fScope, "DamageApplied", U32(damageApplied));
    moveHandle.SaveState(fScope->AddFunction("MoveHandle"));
    StdSave::TypeF32(fScope, "AnimFrame", animFrame);
    StdSave::TypeU32(fScope, "TargetMoving", U32(targetMoving));
  }


  //
  // Object::LoadState
  //
  // Load a state configuration scope
  //
  void Object::LoadState(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x96880780: // "State"
          state = StdLoad::TypeU32(sScope);
          break;

        case 0x9E64852D: // "Damage"
          damage.LoadState(sScope);
          break;

        case 0x33CED45D: // "Delay"
          delay.LoadState(sScope);
          break;

        case 0xA4B634AF: // "Ammunition"
          ammunition = StdLoad::TypeU32(sScope);
          break;

        case 0xD6A8B702: // "Target"
          target.LoadState(sScope);
          break;

        case 0xE9F30D59: // "UnitLastPosition"
          StdLoad::TypeVector(sScope, unitLastPosition);
          break;

        case 0xA76B3231: // "TargetLastPosition"
          StdLoad::TypeVector(sScope, targetLastPosition);
          break;

        case 0x74300EDA: // "GuardPosition"
          StdLoad::TypeVector(sScope, guardPosition);
          break;

        case 0xAD73E64A: // "Solution"
          StdLoad::TypeVector(sScope, solution);
          break;

        case 0x13AC04EA: // "LastFireTime"
          lastFireTime = StdLoad::TypeF32(sScope);
          break;

        case 0x1E4ADBF3: // "LastWaitTime"
          lastWaitTime = StdLoad::TypeF32(sScope);
          break;

        case 0x30D3CC52: // "FiringStartTime"
          firingStartTime = StdLoad::TypeF32(sScope);
          break;

        case 0xE050E4BB: // "LastMoveTime"
          lastMoveTime = StdLoad::TypeF32(sScope);
          break;

        case 0x0764A9A6: // "LastMoveRequestTime"
          lastMoveRequestTime = StdLoad::TypeF32(sScope);
          break;

        case 0x0879C110: // "LastTargetSeenTime"
          lastTargetSeenTime = StdLoad::TypeF32(sScope);
          break;

        case 0x4C7C3241: // "DamageApplied"
          damageApplied = S32(StdLoad::TypeU32(sScope));
          break;

        case 0xD3D101D2: // "MoveHandle"
          moveHandle.LoadState(sScope);
          break;

        case 0x04C19B3B: // "AnimFrame"
          animFrame = StdLoad::TypeF32(sScope);
          break;

        case 0xEC5B4F6C: // "TargetMoving"
          targetMoving = S32(StdLoad::TypeU32(sScope));
          break;
      }
    }
  }


  //
  // PostLoad
  //
  // Post load weapon object data
  //
  void Object::PostLoad()
  {
    target.PostLoad();
  }


  //
  // Save the current location as the guard location
  //
  void Object::SaveLocation()
  {
    // If we're not returning or waiting then save the position
    switch (state)
    {
      case State::Waiting:
      case State::Returning:
        break;

      default:
        // Save this position for reference
        guardPosition = unit.RootOrigin();
    }
  }


  //
  // Set the target of the weapon
  //
  void Object::SetTarget(const Target &newTarget)
  {
    //LOG_DIAG(("%s: %d setting a new target %s", unit.TypeName(), unit.Id(), newTarget.GetInfo()))
    //LOG_DIAG(("Old target %s", target.GetInfo()))

    // Copy in the new target
    target = newTarget;

    // Switch to the ready state
    SetState(State::Ready);
  }


  //
  // Offer a new target to the weapon
  //
  Bool Object::OfferTarget(const Target &newTarget, MapObj *guard)
  {
    ASSERT(newTarget.Alive())

    //LOG_DIAG(("%s: %d offered a new target %s", unit.TypeName(), unit.Id(), newTarget.GetInfo()))
    //LOG_DIAG(("Current target %s", target.GetInfo()))

    // Do we have ammunition ?
    if (!HaveAmmunition())
    {
      unit.PostRestoreNowEvent();
      return (FALSE);
    }

    // If we're not guarding anything then guard ourselves
    if (!guard)
    {
      guard = &unit;
    }

    // Do we care if we have a firing solution ?
    Task *task = unit.GetCurrentTask();

    // Do we have a firing solution on the new target ?
    Vector solution;
    F32 param;
    Bool haveSolution;

    switch (FiringSolution(solution, objSolution, param, newTarget))
    {
      case Solution::Ok:
      case Solution::Left:
      case Solution::Right:
        haveSolution = TRUE;
        break;

      default:
        haveSolution = FALSE;
        break;
    }

    // We care if we're already performing an order or we can't move
    if 
    (
      !(task->GetFlags() & Task::TF_FLAG1) &&
      (
        task->GetFlags() & Task::TF_FROM_ORDER ||
        !unit.CanEverMove()
      )
    )
    {
      if (!haveSolution)
      {
        //LOG_DIAG(("Rejected since there's no firing solution to the new target"))
        return (FALSE);
      }
    }

    // If we're not meant to follow and we're more than half 
    // our guard distance away from our guard position, don't
    // accept targets we don't have solutions on
    if 
    (
      TaskCtrl::Promote<Tasks::UnitAttack>(task) && 
      !Tactical::QueryProperty(task->GetTaskTable(), &unit, Tactical::TP_FOLLOWSUBJECT)
    )
    {
      F32 seeingRange = F32(unit.GetSeeingRange()) * WorldCtrl::CellSize();
      if (!haveSolution && (unit.RootOrigin() - guardPosition).Magnitude() > (seeingRange * 0.5f))
      {
        // LOG_DIAG(("Rejected since we're guarding and more than half way from our guard position and we'd have to move to engage"))
        return (FALSE);
      }
    }

    // If we don't have a current target then this target wins
    if (!target.Alive())
    {
      //LOG_DIAG(("Accepted since we don't have a target"))
      return (TRUE);
    }

    switch (target.GetType())
    {
      case Target::OBJECT:
        // If our current target is an object, test to see which is better
        if 
        (
          newTarget.GetType() == Target::OBJECT &&
          newTarget.GetObj().Alive()
        )
        {
          MapObj *currentMap = target.GetObj();
          MapObj *newMap = newTarget.GetObj();

          // Is this the same target ?
          if (currentMap == newMap)
          {
            //LOG_DIAG(("Rejected since its the same target"))
            return (FALSE);
          }

          // Is the old target a restorer ... we must kill healers 
          // with our last ounce of strength no matter the cost
          if (Promote::Object<RestoreObjType, RestoreObj>(currentMap))
          {
            //LOG_DIAG(("Rejected since our current target is a restorer"))
            return (FALSE);
          }

          // Is the new target healing the old target ?
          Tasks::RestoreMobile *restoreMobileTask = TaskCtrl::Promote<Tasks::RestoreMobile>(newMap->GetCurrentTask());
          if (restoreMobileTask && currentMap == restoreMobileTask->GetTarget())
          {
            return (TRUE);
          }

          // Is our current target targetting us (and is actually a threat) ?
          UnitObj *currentUnit = Promote::Object<UnitObjType, UnitObj>(currentMap);
          U32 currentThreat = 0;

          if (currentUnit)
          {
            currentThreat = currentUnit->UnitType()->GetThreat(guard->MapType()->GetArmourClass());

            if 
            (
              currentUnit->GetWeapon() && 
              currentUnit->GetWeapon()->GetTarget().CheckTarget(*guard)
            )
            {
              // Boost the threat if its targetting us
              currentThreat *= TargetThreatBoost;
            }
          }

          UnitObj *newUnit = Promote::Object<UnitObjType, UnitObj>(newMap);

          U32 newThreat = 0;
          if (newUnit)
          {
            newThreat = newUnit->UnitType()->GetThreat(guard->MapType()->GetArmourClass());

            if 
            (
              newUnit->GetWeapon() &&
              newUnit->GetWeapon()->GetTarget().CheckTarget(*guard)
            )
            {
              // Boost the threat if its targetting us
              newThreat *= TargetThreatBoost;
            }
          }
          else
          {
            // If the new target is a map object, ignore it since its harmless
            //LOG_DIAG(("Rejected since the new target is a map object"))
            return (FALSE);
          }

          // Is the new target more of a threat ?
          if (newThreat > currentThreat)
          {
            // New tartet is more of a threat, switch to it !
            //LOG_DIAG(("Accepted since the new target is more of a threat"))
            return (TRUE);
          }
          else 
          {
            // New target is not more of a threat, stick with current target
            //LOG_DIAG(("Rejected since current target is more (or equal) of a threat"))
            return (FALSE);
          }
        }

      case Target::LOCATION:
        // If our current target is a location accept the new target
        //LOG_DIAG(("Accepted since our current target is only a location"))
        return (TRUE);

      case Target::INVALID:
      default:
        ERR_FATAL(("Can't do that"))
    }
  }


  //
  // InValidate Target
  //
  void Object::InValidateTarget()
  {
    //LOG_DIAG(("Invalidating Target"))
    //Debug::CallStack::Caller();

    target.InValidate();
    targetLastPosition = Vector(0.0f, 0.0f, 0.0f);
    lastBadCellX = lastBadCellZ = U16_MAX;
  }


  //
  // Turn the weapon to its default
  //
  void Object::TurnToDefault(Bool snap)
  {
    if (type.nodeStyle == NodeStyle::Null)
    {
      // Null nodestyle indicates that there is no geometry to manipulate
      return;
    }

    VectorDir objectDir;

    if (snap)
    {
      objectDir = type.defaultDir;
    }
    else
    {
      Vector front;
    
      // Depending upon the node style we operate with differing nodes
      switch (type.nodeStyle)
      {
        case NodeStyle::XYZ:
        case NodeStyle::X:
        case NodeStyle::Y:
          // A single node with all of the information in a single matrix
          front = unit.ObjectMatrix(unit.UnitType()->weaponNodes[0]).front;
          break;

        case NodeStyle::Y_X:
          // Two nodes with the first containing the Y rotation and the second containing the XZ rotation
          front = unit.ObjectMatrix(unit.UnitType()->weaponNodes[1]).front;
          unit.ObjectMatrix(unit.UnitType()->weaponNodes[0]).Rotate(front);
          break;

        default:
          ERR_FATAL(("Unknown node style [%08X]", type.nodeStyle))
          break;
      }

      // Convert the weapon to polar coordinates
      front.Convert(objectDir);

      // Figure out how much angle there is remaining to turn
      VectorDir remaining = type.defaultDir;
      remaining -= objectDir;

      F32 turn = type.turnRate * GameTime::SimTime();

      if (fabs(remaining.u) < turn)
      {
        objectDir.u = type.defaultDir.u;
      }
      else
      {
        objectDir.u += (remaining.u > 0) ? turn : -turn;
      }
      if (fabs(remaining.v) < turn)
      {
        objectDir.v = type.defaultDir.v;
      }
      else
      {
        objectDir.v += (remaining.v > 0) ? turn : -turn;
      }
    }

    // Depending upon the node style setup the weapon nodes
    switch (type.nodeStyle)
    {
      case NodeStyle::XYZ:
      {
        // Only a single node, give it the new direction
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], objectDir);
        break;
      }

      case NodeStyle::X:
      {
        // Set the new front, but clamp X to zero
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], VectorDir(PIBY2, objectDir.v));
        break;
      }

      case NodeStyle::Y:
      {
        // Set the new front, but clamp Y to zero
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], VectorDir(objectDir.u, PIBY2));
        break;
      }

      case NodeStyle::Y_X:
      {
        // Rotate about Y
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], VectorDir(objectDir.u, PIBY2));

        // Rotate about X
        SetNodeMatrix(unit.UnitType()->weaponNodes[1], VectorDir(PIBY2, objectDir.v));
        break;
      }
    }
  }


  //
  // Object::TurnToSolution
  //
  F32 Object::TurnToSolution(const VectorDir &solutionDir)
  {
    Vector front;
   
    if (!GetWeaponFront(front))
    {
      // No available front, tell the caller we're aligned
      return (0.0f);
    }

    // Convert the weapon to polar coordinates
    VectorDir weaponDir;
    front.Convert(weaponDir);
    weaponDir -= VectorDir(PIBY2, PIBY2);

    // Figure out how much angle there is remaining to turn
    VectorDir remaining = solutionDir;
    remaining -= weaponDir;

    F32 turn = type.turnRate * GameTime::SimTime();

    if (fabs(remaining.u) < turn)
    {
      weaponDir.u = solutionDir.u;
    }
    else
    {
      weaponDir.u += (remaining.u > 0) ? turn : -turn;
    }
    if (fabs(remaining.v) < turn)
    {
      weaponDir.v = solutionDir.v;
    }
    else
    {
      weaponDir.v += (remaining.v > 0) ? turn : -turn;
    }

    // The object dir is centred about 0, 0 when we need it to be centred about 90, 90
    VectorDir objectDir = weaponDir;
    objectDir += VectorDir(PIBY2, PIBY2);

    // Depending upon the node style setup the weapon nodes
    switch (type.nodeStyle)
    {
      case NodeStyle::XYZ:
      {
        // Only a single node, give it the new direction
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], objectDir);
        break;
      }

      case NodeStyle::X:
      {
        // Set the new front, but clamp X to zero
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], VectorDir(PIBY2, objectDir.v));
        break;
      }

      case NodeStyle::Y:
      {
        // Set the new front, but clamp Y to zero
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], VectorDir(objectDir.u, PIBY2));
        break;
      }

      case NodeStyle::Y_X:
      {
        // Rotate about Y
        SetNodeMatrix(unit.UnitType()->weaponNodes[0], VectorDir(objectDir.u, PIBY2));

        // Rotate about X
        SetNodeMatrix(unit.UnitType()->weaponNodes[1], VectorDir(PIBY2, objectDir.v));
        break;
      }

      default:
        ERR_FATAL(("Unknown node style [%08X]", type.nodeStyle))
        break;
    }

    // How much is remaining now
    remaining = solutionDir;
    remaining -= weaponDir;

    // Depending upon the node style setup the weapon nodes
    switch (type.nodeStyle)
    {
      case NodeStyle::X:
      case NodeStyle::XYZ:
      case NodeStyle::Y_X:
        return (remaining.u * remaining.u + remaining.v * remaining.v);

      case NodeStyle::Y:
        return (remaining.u * remaining.u);

      default:
        ERR_FATAL(("Unknown node style [%08X]", type.nodeStyle))
        break;
    }

  }


  //
  // Object::GetFiringLocation
  //
  // Returns the location a projectile would be launched from this weapon
  //
  Vector Object::GetFiringLocation()
  {
    // Depending upon the node style we operate with differing nodes
    switch (type.nodeStyle)
    {
      case NodeStyle::Null:
        if (unit.UnitType()->fireNode)
        {
          return (unit.WorldMatrix(*unit.UnitType()->fireNode).posit);
        }
        else
        {
          return (unit.RootOrigin());
        }
        break;

      case NodeStyle::XYZ:
      case NodeStyle::X:
      case NodeStyle::Y:
        // Use the location of the node
        return (unit.WorldMatrix(unit.UnitType()->weaponNodes[0]).posit);
        break;

      case NodeStyle::Y_X:
        // Use the location of the second node
        return (unit.WorldMatrix(unit.UnitType()->weaponNodes[1]).posit);
        break;

      default:
        ERR_FATAL(("Unknown NodeStyle [%08X]", type.nodeStyle))
    }
  }


  //
  // Object::GetWeaponFront
  //
  // Get the weapon front
  //
  Bool Object::GetWeaponFront(Vector &front)
  {
    // Depending upon the node style we operate with differing nodes
    switch (type.nodeStyle)
    {
      case NodeStyle::Null:
        return (FALSE);

      case NodeStyle::X:
      case NodeStyle::XYZ:
      case NodeStyle::Y:
        // A single node with all of the information in a single matrix
        front = unit.ObjectMatrix(unit.UnitType()->weaponNodes[0]).front;
        return (TRUE);
        break;

      case NodeStyle::Y_X:
        // Two nodes with the first containing the Y rotation and the second containing the XZ rotation
        front = unit.ObjectMatrix(unit.UnitType()->weaponNodes[1]).front;
        unit.ObjectMatrix(unit.UnitType()->weaponNodes[0]).Rotate(front);
        return (TRUE);
        break;

      default:
        ERR_FATAL(("Unknown node style [%08X]", type.nodeStyle))
        break;
    }
  }


  //
  // Object::ReloadAmmunition
  //
  // Reload the weapon
  //
  void Object::ReloadAmmunition(U32 ammo)
  {
    ammunition = Min(type.ammunition, ammunition + ammo);
  }


  //
  // Fire the weapon
  //
  void Object::Fire(Type::FirePoint *firePoint)
  {
    // Should the user be killed
    if (type.flags & Flags::KillUser)
    {
      unit.SelfDestruct(TRUE, unit.GetTeam());
      return;
    }

    //LOG_DIAG(("Firing"))

    switch (type.style)
    {
      case Style::Constant:

        // Use the damage applied as an indication to start the firing FX
        if (!damageApplied)
        {
          if (type.numFirePoints)
          {
            // Generate FX for each point
            for (U32 f = 0; f < type.numFirePoints; f++)
            {
              NodeIdent *node = &unit.UnitType()->firePointNodes[type.firePoints[f].index];
              unit.StartGenericFX(0xE87BF766, FireCallBack, TRUE, NULL, node); // "Weapon::Fire"
            }
          }
          else
          {
            // Generate FX for the origin
            unit.StartGenericFX(0xE87BF766, FireCallBack); // "Weapon::Fire"
          }
          damageApplied = 1;
        }

        // Apply damage to the target
        if (target.GetType() == Target::OBJECT)
        {
          ASSERT(target.Alive())
          MapObj *mapObj = target.GetObj();

          // Calculate how much damage we should have applied by now
          F32 damage = (F32) GetDamage(mapObj->MapType()->GetArmourClass());
          F32 time = GameTime::SimTotalTime() - firingStartTime;
          S32 total = (S32) (time * damage);

          // Have we done that much damage ?
          if (total > damageApplied)
          {
            //Vector dir = weapon->WorldMatrix().front;
//            Vector dir = (target.GetLocation() - weaponPtr->Position());
//            dir.Normalize();

            mapObj->ModifyHitPoints(-(total - damageApplied), &unit, unit.GetTeam());
            damageApplied = total;

            // Apply hit modifiers
            type.damage.GetModifiers().Apply(mapObj);
          }
        }

        break;

      case Style::Instant:
      {
        ASSERT(target.Alive())

        // Trigger Fire FX
        unit.StartGenericFX(0xE87BF766); // "Weapon::Fire"

        if (target.GetType() == Target::OBJECT)
        {
          ASSERT(target.Alive())

          // The target is an object, apply damage directly to it
          MapObj *mapObj = target.GetObj();

          // How much damage do we do ?
          S32 damage = GetDamage(mapObj->MapType()->GetArmourClass());

          //Vector dir = target.GetLocation() - unit.Position();
          //dir.Normalize();

          mapObj->ModifyHitPoints(-damage, &unit, unit.GetTeam()/*, &dir*/);

          // Apply hit modifiers
          type.damage.GetModifiers().Apply(mapObj);

          // Start Hit FX on the target
          mapObj->StartGenericFX(0x4A2533FF, NULL, TRUE/*, &dir*/); // "Weapon::Hit"
        }
        else
        {
          // Start FX on the ground directly (how do we do this ?)

          //destination = target.GetLocation();
        }
        break;
      }

      case Style::Projectile:
      {
        // Is the projectile type alive ?
        if (type.projectileType.Alive())
        {
          // Trigger Fire FX
          unit.StartGenericFX(0xE87BF766); // "Weapon::Fire"

          Matrix m;
          MeshEnt &mesh = unit.Mesh();

          // Was a firing pointIdent specified ?
          if (firePoint)
          {
            FamilyNode *node = mesh.Get(unit.UnitType()->firePointNodes[firePoint->index]);
            ASSERT(node)
            m = node->WorldMatrix();

            if (!(type.flags & Flags::Fixed))
            {
              m.up = Matrix::I.up;
              m.SetFromFront(solution);
            }

            LaunchProjectile(m, 0);
          }
          else
          {
            // Are there firing points ?
            if (type.numFirePoints)
            {
              // If there's a mesh, then there's firing points
              for (U32 f = 0; f < type.numFirePoints; f++)
              {
                m = unit.WorldMatrix(unit.UnitType()->firePointNodes[type.firePoints[f].index]);

                if (!(type.flags & Flags::Fixed))
                {
                  m.up = Matrix::I.up;
                  m.SetFromFront(solution);
                }
                LaunchProjectile(m, 0);
              }
            }
            else
            {
              // Fire from the origin of the weapon
              m.ClearData();
              m.posit = unit.RootOrigin();
              if (!(type.flags & Flags::Fixed))
              {
                m.up = Matrix::I.up;
                m.SetFromFront(solution);
              }
              LaunchProjectile(m, 0);
            }
          }
        }
        break;
      }

      default:
        ERR_FATAL(("Unknown weapon style %d", type.style))
    }

    if (type.flags & Flags::OneShot)
    {
      HaltFire();
    }
  }


  //
  // Ready to fire ?
  //
  Bool Object::ReadyToFire()
  {
    // Do we have a last fire time
    if (lastFireTime)
    {
      // Get the number of seconds since last fire time
      if (((GameTime::SimTotalTime() - lastFireTime) * unit.GetEfficiency()) < delay.GetFPoint())
      {
        return (FALSE);
      }
    }

    // Ready to fire
    return (unit.GetEfficiency() > 0.0F);
  }


  //
  // Halt Fire
  //
  void Object::HaltFire()
  {
    // Give up direct control mode
    if (unit.CanEverMove())
    {
      unit.GetDriver()->SetupDirectControl(FALSE);
    }

    InValidateTarget();
    SetState(State::Ready);
  }


  //
  // Process Event
  //
  Bool Object::ProcessEvent(const Task::Event &event)
  {
    switch (event.message)
    {
      case 0x1C6E3199: // "Movement::Started"
        // Did we enact this movement ?
        if (moveHandle == event.param1)
        {
          return (TRUE);
        }
        else
        {
          // We didn't do it, return to ready
          SetState(State::Ready);
          return (FALSE);
        }
        
      case 0x71BB2A61: // "Movement::Incapable"
        // Did we enact this movement ?
        if (moveHandle == event.param1)
        {
          switch (state)
          {
            case State::Moving:
              unit.PostEvent(Task::Event(Notify::Failed));
              break;

            case State::Returning:
              unit.PostEvent(Task::Event(Notify::Failed));
              SetState(State::Ready);
              break;
          }
          return (TRUE);
        }
        return (FALSE);

      case 0xE609174A: // "Movement::Completed"
        // Did we enact this movement ?
        if (moveHandle == event.param1)
        {
          switch (state)
          {
            case State::Moving:
              SetState(State::Ready);
              break;

            case State::Returning:
              unit.PostEvent(Task::Event(Notify::Failed));
              SetState(State::Ready);
              break;
          }
          return (TRUE);
        }
        return (FALSE);
        
      case MapObjNotify::AnimationDone:
        // Was this an animation we started ?
        switch (event.param1)
        {
          case 0x43B55F8D: // "PreFire"
            // Change to the fire state
            SetState(State::Fire);
            return (TRUE);

          case 0x4B249129: // "PostFire"
            // Change to the ready state
            SetState(State::Ready);
            return (TRUE);
        }
        return (FALSE);

      default:
        return (FALSE);
    }
  }


  //
  // Do we have a target ?
  //
  Bool Object::HaveTarget()
  {
    return 
    (
      (
        state != State::Idle &&
        target.Alive() 
      )
      ? TRUE : FALSE
    );
  }


  //
  // Break off the attack
  //
  void Object::BreakOffAttack()
  {
    // Invalidate the target
    InValidateTarget();

    // Get the current task
    Task *task = unit.GetCurrentTask();

    // Are we a guard ?
    if 
    (
      IsAllowedMovement() &&
      task && 
      !Tactical::QueryProperty(task->GetTaskTable(), &unit, Tactical::TP_HOLDPOSITION) &&
      !Tactical::QueryProperty(task->GetTaskTable(), &unit, Tactical::TP_FOLLOWSUBJECT) &&
      !(task->GetFlags() & Task::TF_FLAG2)
    )
    {
      SetState(State::Waiting);
    }
    else
    {
      // Tell the task that we failed
      unit.PostEvent(Task::Event(Notify::Failed));
      SetState(State::Ready);
    }
  }

  
  //
  // Get Info
  //
  const char * Object::GetInfo()
  {
    return (GetStateName(state));
  }


  #ifndef MONO_DISABLED

  #define ROW 12

  //
  // Mono output
  //

  void Object::SetupMono(Mono::Buffer *buf)
  {
    S32 row = ROW;

    MonoBufWrite(buf, row++, 0, "Weapon        ", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "State         ", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Ammunition    ", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Target        ", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Guard Position", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Barrel        ", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Solution      ", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Barrel Offset ", Mono::BRIGHT);

    MonoBufWrite(buf, row++, 0, "Fired@                    \x7F", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Moved@                    \x7F", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "Move Requested@           \x7F", Mono::BRIGHT);
    MonoBufWrite(buf, row++, 0, "TargetSeen@               \x7F", Mono::BRIGHT);

    //                           012345678901234567890123456789
  }

  void Object::DisplayMono(Mono::Buffer *buf)
  {
    S32 row = ROW;
    S32 col = 16;

    MonoBufWriteV(buf, (row++, col, "%-40s", type.GetName()));
    MonoBufWriteV(buf, (row++, col, "%-20s", GetInfo()));
    MonoBufWriteV(buf, (row++, col, "%10d", ammunition));

    switch (target.GetType())
    {
      case Target::INVALID:
        MonoBufWriteV(buf, (row++, col, "Invalid                                           "));
        //                               01234567890123456789012345678901234567890123456789
        break;

      case Target::OBJECT:
        if (target.Alive())
        {
          MapObj *obj = target.GetObj();
          MonoBufWriteV(buf, (row++, col, "Object [%d] @ %8.2f %8.2f %8.2f     ", obj->Id(), obj->RootOrigin().x, obj->RootOrigin().y, obj->RootOrigin().z));
          //                               01234567890123456789012345678901234567890123456789
        }
        else
        {
          MonoBufWriteV(buf, (row++, col, "Object - DEAD                                     "));
          //                               01234567890123456789012345678901234567890123456789
        }
        break;

      case Target::LOCATION:
      {
        const Vector &loc = target.GetLocation();

        MonoBufWriteV(buf, (row++, col, "Location %8.2f %8.2f %8.2f              ", loc.x, loc.y, loc.z));
        //                               01234567890123456789012345678901234567890123456789
        break;
      }
    }

    MonoBufWriteV(buf, (row++, col, "%8.2f %8.2f %8.2f", guardPosition.x, guardPosition.y, guardPosition.z));
    MonoBufWriteV(buf, (row++, col, "%8.2f %8.2f %8.2f", unit.UnitType()->barrel.x, unit.UnitType()->barrel.y, unit.UnitType()->barrel.z));
    MonoBufWriteV(buf, (row++, col, "%8.2f %8.2f %8.2f", solution.x, solution.y, solution.z));

    Matrix m;
    m.ClearData();
    m.up = Matrix::I.up;
    m.SetFromFront(solution);
    Vector offset = unit.UnitType()->barrel;
    m.Rotate(offset);

    MonoBufWriteV(buf, (row++, col, "%8.2f %8.2f %8.2f", offset.x, offset.y, offset.z));

    MonoBufWriteV(buf, (row, col, "%8.2f", lastFireTime));
    MonoBufWriteV(buf, (row++, col + 10, "%8.2f", GameTime::SimTotalTime() - lastFireTime));

    MonoBufWriteV(buf, (row, col, "%8.2f", lastMoveTime));
    MonoBufWriteV(buf, (row++, col + 10, "%8.2f", GameTime::SimTotalTime() - lastMoveTime));

    MonoBufWriteV(buf, (row, col, "%8.2f", lastMoveRequestTime));
    MonoBufWriteV(buf, (row++, col + 10, "%8.2f", GameTime::SimTotalTime() - lastMoveRequestTime));

    MonoBufWriteV(buf, (row, col, "%8.2f", lastTargetSeenTime));
    MonoBufWriteV(buf, (row++, col + 10, "%8.2f", GameTime::SimTotalTime() - lastTargetSeenTime));

  }

  #endif

}
