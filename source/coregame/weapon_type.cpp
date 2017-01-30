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
#include "mapobjctrl.h"
#include "physicsctrl.h"
#include "resolver.h"
#include "explosionobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Weapon
//
namespace Weapon
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type
  //


  //
  // Type::Type
  //
  Type::Type(const GameIdent &ident, FScope *fScope)
  : delay(InstanceModifierType::FPOINT),
    ident(ident),
    numFirePoints(0),
    flags(0)
  {
    ASSERT(fScope)

    FScope *sScope;

    // Style
    style = StdLoad::TypeStringCrc(fScope, "Style");

    // Setup damage
    damage.Setup(ident, fScope->GetFunction("Damage"));

    // Node Style
    nodeStyle = StdLoad::TypeStringCrc(fScope, "NodeStyle", NodeStyle::Null);

    // Reset the number of nodes
    U32 numNodes = 0;

    // Nodes
    if ((sScope = fScope->GetFunction("Nodes", FALSE)) != NULL)
    {
      while (sScope->IsNextArgString())
      {
        if (numNodes < MaxWeaponNodes)
        {
          nodes[numNodes++] = StdLoad::TypeString(sScope);
        }
        else
        {
          sScope->ScopeError("Too many nodes (maximum %d)", MaxWeaponNodes);
        }
      }
    }

    // Check to see if the number of nodes specified matches the node style
    switch (nodeStyle)
    {
      case NodeStyle::Null:
        if (numNodes != 0)
        {
          fScope->ScopeError("NodeStyle 'NULL' does not allow any nodes, you specified %d", numNodes);
        }
        break;

      case NodeStyle::XYZ:
        if (numNodes != 1)
        {
          fScope->ScopeError("NodeStyle 'XYZ' must have one (1) node, you specified %d", numNodes);
        }
        break;

      case NodeStyle::X:
        if (numNodes != 1)
        {
          fScope->ScopeError("NodeStyle 'X' must have one (1) node, you specified %d", numNodes);
        }
        break;

      case NodeStyle::Y:
        if (numNodes != 1)
        {
          fScope->ScopeError("NodeStyle 'Y' must have one (1) node, you specified %d", numNodes);
        }
        break;

      case NodeStyle::Y_X:
        if (numNodes != 2)
        {
          fScope->ScopeError("NodeStyle 'Y_X' must have two (2) nodes, you specified %d", numNodes);
        }
        break;

      default:
        fScope->ScopeError("Unknown NodeStyle, should be (NULL, XYZ, X, Y, Y_X)");
        break;
    }

    // Range
    maxRange = StdLoad::TypeF32(fScope, "MaxRange", 50.0f, Range<F32>(0.0f, 500.0f));
    maxRange2 = maxRange * maxRange;
    minRange = StdLoad::TypeF32(fScope, "MinRange", 0.0f, Range<F32>(0.0f, maxRange));
    minRange2 = minRange * minRange;

    // Avoid Range
    avoidRange = StdLoad::TypeF32(fScope, "AvoidRange", 20.0, Range<F32>(0.0f, 100.0f));

    // Allowed angles of movement
    angle.v      = StdLoad::TypeF32(fScope, "VertAngle",       90.0f, Range<F32>(0.0f, 180.0f)) * DEG2RAD;
    angle.u      = StdLoad::TypeF32(fScope, "HorizAngle",      90.0f, Range<F32>(-180.0f, 180.0f)) * DEG2RAD;
    separation.v = StdLoad::TypeF32(fScope, "VertSeparation",  10.0f, Range<F32>(5.0f, 180.0f)) * DEG2RAD;
    separation.u = StdLoad::TypeF32(fScope, "HorizSeparation", 10.0f, Range<F32>(5.0f, 180.0f)) * DEG2RAD;

    // Calculate the vectors of the movement frustrum
    topLeft.Set(angle.u + separation.u, angle.v - separation.v);
    topRight.Set(angle.u - separation.u, angle.v - separation.v);
    bottomLeft.Set(angle.u + separation.u, angle.v + separation.v);
    bottomRight.Set(angle.u - separation.u, angle.v + separation.v);

    // Determine the initial direction of the vector
    defaultDir.v = StdLoad::TypeF32(fScope, "VertDefault", angle.v * RAD2DEG, Range<F32>(0.0f, 180.0f)) * DEG2RAD;
    defaultDir.u = StdLoad::TypeF32(fScope, "HorizDefault", angle.u * RAD2DEG, Range<F32>(-180.0f, 180.0f)) * DEG2RAD;

    // Rotate angles to be in terms of the front vector of the object
    VectorDir frontDir;
    Matrix::I.front.Convert(frontDir);
    angleFront = angle;
    angleFront -= frontDir;

    // Turn Rate
    turnRate = StdLoad::TypeF32(fScope, "TurnRate", 3600.0) * DEG2RAD;

    // Turn Error
    turnError = StdLoad::TypeF32(fScope, "TurnError", 2.0f, Range<F32>(0.0, 90.0)) * DEG2RAD;
    turnError *= turnError;

    // Ammunition
    ammunition = StdLoad::TypeU32(fScope, "Ammunition", 0);

    // Height offset
    heightOffset = StdLoad::TypeF32(fScope, "HeightOffset", 0.0f, Range<F32>(-500.0, 500.0f));

    // Delay
    if (style != Style::Constant)
    {
      sScope = fScope->GetFunction("Delay");
      delay.LoadFPoint(sScope, 1.0F, Range<F32>(0.1F, F32_MAX), 1.0F, 1.0F, 1e5F);
      rate = (U32) Clamp(0.0f, 65536.0f / delay.float32.value, F32(U32_MAX));
    }
    else
    {
      // Set rate to 1.0
      rate = 65536; 
    }

    // Flags
    flags |= StdLoad::TypeU32(fScope, "Fixed", 0, Range<U32>::flag) ? Flags::Fixed : 0;
    flags |= StdLoad::TypeU32(fScope, "KillUser", 0, Range<U32>::flag) ? Flags::KillUser : 0;
    flags |= StdLoad::TypeU32(fScope, "HighTrajectory", 0, Range<U32>::flag) ? Flags::HighTrajectory : 0;
    flags |= StdLoad::TypeU32(fScope, "OneShot", 0, Range<U32>::flag) ? Flags::OneShot : 0;
    flags |= StdLoad::TypeU32(fScope, "LeadAngle", 0, Range<U32>::flag) ? Flags::LeadAngle : 0;
    flags |= StdLoad::TypeU32(fScope, "PotShot", 0, Range<U32>::flag) ? Flags::PotShot : 0;
    flags |= StdLoad::TypeU32(fScope, "AimAtBox", 0, Range<U32>::flag) ? Flags::AimAtBox : 0;

    // Style specific
    switch (style)
    {
      case Style::Constant:
        break;

      case Style::Instant:
        break;

      case Style::Projectile:
      {
        // Speed
        initialSpeed = StdLoad::TypeF32(fScope, "Speed", -1.0f) * PhysicsConst::KMH2MPS;

        // Get required projectile name
        StdLoad::TypeReaperObjType(fScope, "Projectile", projectileType);

        if (!projectileType.HasResolveId())
        {
          ERR_CONFIG(("Weapon [%s] requires a projectile type", ident.str));
        }
        break;
      }
    }

    // Load fire points
    if ((sScope = fScope->GetFunction("FirePoints", FALSE)) != NULL)
    {
      FScope *ssScope;
      while ((ssScope = sScope->NextFunction()) != NULL)
      {
        switch (ssScope->NameCrc())
        {
          case 0x9F1D54D0: // "Add"
          {
            if (numFirePoints < MaxFirePointNodes)
            {
              NodeIdent ident = StdLoad::TypeString(ssScope);
              F32 frame = StdLoad::TypeF32(ssScope, 0.0f);
              firePoints[numFirePoints].Setup(ident, numFirePoints, frame);
              numFirePoints++;
            }
            else
            {
              ssScope->ScopeError("Too many fire points (maximum %d)", MaxFirePointNodes);
            }
            break;
          }
        }
      }
    }
  }


  //
  // Type::~Type
  //
  Type::~Type()
  {
  }


  //
  // Called after all types are loaded
  //
  void Type::PostLoad()
  {
    switch (style)
    {
      case Style::Instant:
        break;

      case Style::Constant:
        break;

      case Style::Projectile:
      {
        // Resolve the projectile type if a projectile was supplied
        Resolver::Type<ProjectileObjType>(projectileType, TRUE);

        // LOG_DIAG(("Projectile weapon %s has projectile %s", GetName(), projectileType->GetName()))

        // Does the projectile have mass ?
        if (
          !projectileType->GetMass() || 
          projectileType->GetModel() == ProjectileModel::StraightTrajectory)
        {
          // The initial speed should have been specified
          if (initialSpeed < 0.0f)
          {
            ERR_CONFIG(("Speed was not specified for weapon type %s", ident.str))
          }
        }
        else
        {
          if (initialSpeed < 0.0f)
          {
            // Initial speed of the projectile
            initialSpeed = F32(sqrt(PhysicsCtrl::GetGravity() * maxRange));
          }
        }
        break;
      }
    }
  }


  //
  // Initialize Resources
  //
  void Type::InitializeResources(UnitObjType &type)
  {
    const MeshRoot &mesh = *type.GetMeshRoot();
    AnimList *animList;

    // Transfer nodes to the unit's nodes
    for (U32 n = 0; n < MaxWeaponNodes; n++)
    {
      type.weaponNodes[n] = nodes[n];
    }

    // Transfer fire points to units fire points
    for (U32 f = 0; f < numFirePoints; f++)
    {
      type.firePointNodes[f] = firePoints[f].ident;
    }

    // Check the animations and make sure their right
    if ((animList = mesh.FindAnimCycle(0x43B55F8D)) != NULL) // "PreFire"
    {
      if (animList->type != anim1WAY)
      {
        animList->type = anim1WAY;
        LOG_WARN(("Weapon '%s' on unit '%s' has PreFire animation but its not 1way", ident.str, type.GetName()))
      }
    }

    // Check the animations and make sure their right
    if ((animList = mesh.FindAnimCycle(0xEBEB1BE9)) != NULL) // "Fire"
    {
      switch (style)
      {
        case Style::Instant:
        case Style::Projectile:
          if (animList->type != anim1WAY)
          {
            animList->type = anim1WAY;
            LOG_WARN(("Instant/Projectile Weapon '%s' on unit '%s' has Fire animation but its not 1way", ident.str, type.GetName()))
          }
          break;

        case Style::Constant:
          if (animList->type != animLOOP)
          {
            animList->type = animLOOP;
            LOG_WARN(("Constant Weapon '%s' on unit '%s' has Fire animation but its not Loop", ident.str, type.GetName()))
          }
          break;
      }
    }

    for (f = 0; f < numFirePoints; f++)
    {
      if (!mesh.FindIdent(type.firePointNodes[f]))
      {
        ERR_CONFIG(("Can't find fire point '%s' on %s", type.firePointNodes[f].str, ident.str));
      }
    }

    const Mesh *m = NULL;

    switch (nodeStyle)
    {
      case NodeStyle::Null:
        break;

      case NodeStyle::XYZ:
      case NodeStyle::X:
      case NodeStyle::Y:
        m = mesh.FindIdent(type.weaponNodes[0]);
        if (!m)
        {
          ERR_FATAL(("Can't find weapon node '%s' in weapon '%s'", type.weaponNodes[0].str, ident.str))
        }
        // If there's no fire node then use this node
        if (!type.fireNode)
        {
          type.fireNode = &type.weaponNodes[0];
        }
        break;

      case NodeStyle::Y_X:
      {
        m = mesh.FindIdent(type.weaponNodes[0]);
        if (!m)
        {
          ERR_FATAL(("Can't find weapon node(0) '%s' in weapon '%s'", type.weaponNodes[0].str, ident.str))
        }
        m = mesh.FindIdent(type.weaponNodes[1]);
        if (!m)
        {
          ERR_FATAL(("Can't find weapon node(1) '%s' in weapon '%s'", type.weaponNodes[1].str, ident.str))
        }
        // If there's no fire node then use this node
        if (!type.fireNode)
        {
          type.fireNode = &type.weaponNodes[1];
        }
        break;
      }
    }

    // If there's only one firing point then set the firing node
    if (!type.fireNode && numFirePoints > 0)
    {
      type.fireNode = &type.firePointNodes[0];
    }

    // Determine the length of the barrel
    if (m && numFirePoints > 0)
    {
      Matrix offset;

      // Find the average offset (assumes that there is 1 firepoint or firepoints are symetricle)
      for (f = 0; f < numFirePoints; f++)
      {
        offset.ClearData();
        const_cast<Mesh *>(m)->FindOffset(type.firePointNodes[f].str, offset);
        type.barrel += offset.posit;
      }

      type.barrel /= F32(numFirePoints);
    }
  }


  //
  // CreateProjectile
  //
  // Create a projectile at a given location
  //
  ProjectileObj * Type::CreateProjectile(const Matrix &location, Team *team, const Target &target)
  {
    if (style != Style::Projectile)
    {
      ERR_FATAL(("Attempt to create a projectile with a non projectile style weapon!"))
    }

    // Create the projectile
    ProjectileObj *p = (ProjectileObj *) projectileType->NewInstance(0);

    // And set it up
    p->SetSimCurrent(location);
    p->Setup(NULL, team, this, target, damage.GetInstanceModifierType().GetInteger(), initialSpeed);

    // Add to the map
    MapObjCtrl::AddToMap(p);

    return (p);
  }


  //
  // GetThreat
  //
  // Amount of damage this weapon does against the specifed armour class per second
  //
  S32 Type::GetThreat(U32 armourClass)
  {
    // Style specific
    switch (style)
    {
      case Style::Constant:
        // For a constant weapon the damage is the amount we do per second
        return (damage.GetAmount(armourClass));

      case Style::Instant:
        // For instant weapons the damage is the amount we do per firing rate
        return (damage.GetAmount(armourClass) * rate >> 16);

      case Style::Projectile:
        // For projectile weapons the damage is the amount we do per projectile plus the damage any explosions do created by this projectile
        if ((projectileType.Alive()))
        {
          return 
          (
            (
              (damage.GetAmount(armourClass) * rate >> 16) 
              +
              (
                projectileType->GetExploisionType() ?
                (projectileType->GetExploisionType()->GetThreat(armourClass) * rate >> 16) : 0
              )
            ) 
            *
            (
              numFirePoints ? numFirePoints : 1
            )
          );
        }
        else
        {
          return
          (
            (
              (damage.GetAmount(armourClass) * rate >> 16)
            )
            *
            (
              numFirePoints ? numFirePoints : 1
            )
          );
        }
        break;

      default:
        ERR_FATAL(("Unknown weapon style %d", style))
    }
  }
}
