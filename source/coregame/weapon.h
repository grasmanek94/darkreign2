///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Weapon system
//


#ifndef __WEAPON_H
#define __WEAPON_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "damage.h"
#include "projectileobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Weapon
//
namespace Weapon
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Style
  //
  namespace Style
  {
    const U32 Constant        = 0x3B52BFAA; // "Constant"
    const U32 Instant         = 0x79123359; // "Instant"
    const U32 Projectile      = 0xFEA41B2C; // "Projectile"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Notify
  //
  namespace Notify
  {
    const U32 Completed       = 0xF39385CF; // "Weapon::Notify::Completed"
    const U32 Failed          = 0x28FE6856; // "Weapon::Notify::Failed"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace NodeStyle
  //
  namespace NodeStyle
  {
    const U32 Null            = 0x5B2A0A5F; // "Null"
    const U32 XYZ             = 0xF0311751; // "XYZ"
    const U32 X               = 0x774BB014; // "X"
    const U32 Y               = 0x738AADA3; // "Y"
    const U32 Y_X             = 0x3D1E0938; // "Y_X"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Flags
  //
  namespace Flags
  {
    const U32 Fixed           = 0x00000001;
    const U32 KillUser        = 0x00000002;
    const U32 HighTrajectory  = 0x00000004;
    const U32 OneShot         = 0x00000008;
    const U32 LeadAngle       = 0x00000010;
    const U32 PotShot         = 0x00000020;
    const U32 AimAtBox        = 0x00000040;
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //
  class Type
  {
  private:

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct FirePoint
    //
    struct FirePoint
    {
      // Name of the fire point
      GameIdent ident;

      // Index
      U32 index;

      // Animation frame to fire on
      F32 frame;

      // Costructor
      FirePoint()
      : index(0),
        frame(0.0f)
      {
      }

      // Setup
      void Setup(const GameIdent &identIn, U32 indexIn, F32 frameIn)
      {
        ident = identIn.str;
        index = indexIn;
        frame = frameIn;
      }

    };

    // Name of the weapon
    GameIdent ident;

    // Weapon style
    U32 style;
    
    // Node hieachy
    U32 nodeStyle;

    // Nodes
    GameIdent nodes[MaxWeaponNodes];

    // Amount of damage this weapon does (hp)
    Damage::Type damage;

    // Maximum range of this weapon (m)
    F32 maxRange;

    // Maximum range of this weapon squared (m2)
    F32 maxRange2;               

    // Minimum range of this weapon (m)
    F32 minRange;

    // Minimum range of this weapon squared (m2)
    F32 minRange2;               

    // Angles which make up the firing frustrum
    VectorDir angle;  
  
    // Angle relative to the front
    VectorDir angleFront;           
    VectorDir separation;                     

    // The 4 vectors which make up the firing frustrum
    Vector topLeft;              
    Vector topRight;         
    Vector bottomLeft;                 
    Vector bottomRight;                
                                              
    // Initial polar direction the weapon faces
    VectorDir defaultDir;

    // Speed at which the weapon turns (rad/sec)
    F32 turnRate;             

    // Dot product which must be exceeded for close enough
    F32 turnError;            
                                              
    // Range to avoid hitting
    F32 avoidRange;


    // Names of points of fire
    FirePoint firePoints[MaxFirePointNodes];

    // Number of fire points
    U32 numFirePoints;


    // Delay between firing (s)
    InstanceModifierType delay;

    // Fire rate (1/s) (16/16)
    U32 rate;


    // Amount of ammunition this weapon has
    U32 ammunition;

    // Height offset
    F32 heightOffset;


    // Flags
    U32 flags;


    // Projectile specific

    // Type of projectile which this weapon fires
    ProjectileObjTypePtr projectileType;

    // Initial speed of the projectile (m/s)
    F32 initialSpeed;         

  public:

    // Manager node
    NBinTree<Type>::Node nodeManager;

  public:

    // Constructor and Destructor
    Type(const GameIdent &ident, FScope *fScope);
    ~Type();

    // Called after all types are loaded
    void PostLoad();

    // Initialize resources
    void InitializeResources(UnitObjType &unit);

    // Create a projectile at a given location
    ProjectileObj * CreateProjectile(const Matrix &location, Team *team, const Target &target);

    // GetThreat: Amount of damage this weapon does against the specifed armour class per second
    S32 GetThreat(U32 armourClass);

  public:

    // GetStyle
    U32 GetStyle()
    {
      return (style);
    }

    // GetName
    const char * GetName()
    {
      return (ident.str);
    }

    // GetNameCrc
    U32 GetNameCrc()
    {
      return (ident.crc);
    }

    // GetAmminition
    U32 GetAmmunition()
    {
      return (ammunition);
    }

    // GetMaxRange
    F32 GetMaxRange()
    {
      return (maxRange);
    }

    // GetMaxRange2
    F32 GetMaxRange2()
    {
      return (maxRange2);
    }

    // GetDamage
    Damage::Type & GetDamage()
    {
      return (damage);
    }

    // Get the firing frustrum vectors
    const Vector & GetTopLeft()
    {
      return (topLeft);
    }
    const Vector & GetTopRight()
    {
      return (topRight);
    }
    const Vector & GetBottomLeft()
    {
      return (bottomLeft);
    }
    const Vector & GetBottomRight()
    {
      return (bottomRight);
    }

    // Can we take pot shots ?
    Bool CanTakePotShot()
    {
      return (flags & Flags::PotShot ? TRUE : FALSE);
    }

    // Get Delay IM
    InstanceModifierType & GetDelayIM()
    {
      return (delay);
    }

    ProjectileObjType * GetProjectileType()
    {
      return (projectileType);
    }

  public:

    friend class Object;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Object
  //
  class Object
  {
  private:

    // The weapon type
    Type &type;

    // Unit which owns this weapon
    UnitObj &unit;

    // Weapon state
    U32 state;

    // Damage
    Damage::Object damage;


    // Instance modified values
    InstanceModifier delay;


    // Amount of ammunition remaining in the weapon
    U32 ammunition;


    // The weapons current target
    Target target;

    // Our last position
    Vector unitLastPosition;

    // Targets last position
    Vector targetLastPosition;

    // The position we're mean't to be guarding
    Vector guardPosition;

    // The current firing solution
    Vector solution;

    // Firing solution in object space
    Vector objSolution;

    // Angle (yaw) of direct turn to solution
    F32 turnToAngle;


    // Last time the weapon fired
    F32 lastFireTime;

    // Last time we were told to wait
    F32 lastWaitTime;

    // Time when firing commenced
    F32 firingStartTime;

    // Last time the weapon directed movement
    F32 lastMoveTime;

    // Last time we wanted to move
    F32 lastMoveRequestTime;

    // Last time we saw the target
    F32 lastTargetSeenTime;

    // Last cell that couldn't see target from
    U16 lastBadCellX;
    U16 lastBadCellZ;

    // Damage applied
    S32 damageApplied;

    // Movement request handle
    Movement::Handle moveHandle;

    // Anim frame counter
    F32 animFrame;

    // Is the target moving ?
    Bool targetMoving;

  public:

    // Constructor
    Object(Type &type, UnitObj &unit);

    // Destructor
    ~Object();

    // Does the weapon have ammunition
    Bool HaveAmmunition()
    {
      return (ammunition || !type.ammunition ? TRUE : FALSE);
    }

  private:

    // Expend one projectile worth of ammunition
    void ExpendAmmunition();

    // Set the state
    void SetState(U32 state);

    // IsIdle
    Bool IsIdle();

    // IsAllowedMovement
    Bool IsAllowedMovement();

    // Start Firing
    void StartFiring();

    // Set the direction of a node
    void SetNodeMatrix(const NodeIdent &node, const VectorDir &dir);

    // Calculate a firing solution
    U32 FiringSolution(Vector &solution, Vector &objSolution, F32 &fParam, const Target &target);

    // Calcuates the direction to shoot to hit a target with a parabolic projectile
    Bool CalculateArc(const Vector &deltaPos, F32 speed, Vector &velocity, F32 &time);

    // Launch a projectile
    void LaunchProjectile(Matrix &m, F32 speed);

    // Move to target
    void MoveToTarget();

    // Fire Callback
    static Bool FireCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

    // Are we still on the last bad cell?
    Bool IsBadCell();

    // Clear the last bad cell
    void ClearBadCell();

    // Mark the current cell as bad
    void MarkBadCell();

    // Can we hit the target from here without hitting an obstacle
    Bool ClearLineOfFire();

  public:

    // Per-cycle processing
    void Process();

    // Process firing
    void ProcessFiring();

    // Load and save state configuration
    void LoadState(FScope *fScope);
    void SaveState(FScope *fScope);
    void PostLoad();

    // Save the current location as the guard location
    void SaveLocation();

    // Set the target of the weapon
    void SetTarget(const Target &target);

    // Offer a new target to the weapon
    Bool OfferTarget(const Target &target, MapObj *guard = NULL);

    // InValidate Target
    void InValidateTarget();

    // Turn the weapon to its default
    void TurnToDefault(Bool snap);

    // Turn the weapon towards a solution
    F32 TurnToSolution(const VectorDir &solutionDir);

    // Get the firing location
    Vector GetFiringLocation();

    // Get the weapon front
    Bool GetWeaponFront(Vector &vector);

    // Reload the weapon
    void ReloadAmmunition(U32 ammunition);

    // Fire the weapon
    void Fire(Type::FirePoint *firePoint);

    // Stop firing the weapon
    void HaltFire();

    // Ready to fire ?
    Bool ReadyToFire();

    // Process Event
    Bool ProcessEvent(const Task::Event &event);

    // Do we have a target ?
    Bool HaveTarget();

    // Break off the attack
    void BreakOffAttack();

    // Get Info
    const char * GetInfo();

    #ifndef MONO_DISABLED

    // Mono output
    static void SetupMono(Mono::Buffer *buf);
    void DisplayMono(Mono::Buffer *buf);

    #endif

    Bool GetTargetPos( Vector & tvect, Bool onSurface = TRUE);

    // Get the current delay as a percentage of the max
    F32 GetDelayPercent();

  public:

    // Get the type
    Type & GetType()
    {
      return (type);
    }

    // GetDamage: Amount of damage this weapon does to this armour class
    S32 GetDamage(U32 armourClass)
    {
      return (damage.GetAmount(armourClass));
    }

    // GetDamage
    Damage::Object & GetDamage()
    {
      return (damage);
    }

    // Instance modifiers
    InstanceModifier &GetDamageIM()
    {
      return (damage.GetInstanceModifier());
    }

    // Get ammunitiion
    U32 GetAmmunition()
    {
      return (ammunition);
    }

    // Get the target
    const Target & GetTarget()
    {
      return (target);
    }

    // Get the guard position
    const Vector & GetGuardPosition()
    {
      return (guardPosition);
    }

    // Get the solution
    const Vector & GetSolution()
    {
      return (solution);
    }

    // Get Delay IM
    InstanceModifier & GetDelayIM()
    {
      return (delay);
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Manager
  //
  namespace Manager
  {

    // Initialization and Shutdown
    void Init();
    void Done();

    // PostLoad
    void PostLoad();

    // Create a weapon type
    void ProcessCreateWeaponType(FScope *fScope);

    // Find a weapon type by name
    Type * FindType(const GameIdent &typeName);

  }



}

#endif
