//////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"

#include "mapobjctrl.h"
#include "worldctrl.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "resolver.h"
#include "physicsctrl.h"
#include "perfstats.h"
#include "unitobj.h"
#include "unitobjiter.h"
#include "random.h"
#include "footprint.h"
#include "terrain.h"
#include "main.h"
#include "team.h"
#include "gametime.h"
#include "terraindata.h"
#include "environment_light.h"
#include "tasks_mapdeath.h"
#include "common.h"
#include "explosionobj.h"
#include "collisionctrl.h"
#include "ray.h"
#include "fx.h"
#include "sync.h"
#include "console.h"
#include "savegame.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "MapObj"


///////////////////////////////////////////////////////////////////////////////
//
// Struct MapObjType::Attachment
//
struct MapObjType::Attachment
{
  // Name of the object type to attach
  GameIdent typeName;

  // Object type to attach
  MapObjType *type;

  // Name of point to attach to on the mesh
  NodeIdent pointIdent;
};


///////////////////////////////////////////////////////////////////////////////
//
// Struct MapObjType::AnimationFX
//
struct MapObjType::AnimationFX
{
  // FX Key
  U32 typeCrc;

  // Animation Frame
  F32 animFrame;

  // Constructor
  AnimationFX(F32 animFrame, FScope *fScope) 
  : animFrame(animFrame),
    typeCrc(StdLoad::TypeStringCrc(fScope))
  {
  }

};


///////////////////////////////////////////////////////////////////////////////
//
// Struct MapObjType::AnimationFXList
//
struct MapObjType::AnimationFXList : public List<AnimationFX>
{
  ~AnimationFXList()
  {
    DisposeAll();
  }
};


///////////////////////////////////////////////////////////////////////////////
//
// Class MapObjType - Base type class for all map object types
//


// Basic rigid body physics
//
void MapObjType::RigidBodyPhysics(MapObj &obj, Matrix &m)
{
  Vector veloc = obj.GetVelocity();

  // Apply gravitational acceleration
  veloc.y -= PhysicsCtrl::GetGravity() * GameTime::INTERVAL;

  // Apply drag
  veloc -= (veloc * movementModel->Drag());

  // Update position
  m.posit += (veloc * GameTime::INTERVAL);

  // Update speed and velocity
  F32 vLen = veloc.Magnitude();
  obj.SetSpeed(vLen);
  obj.SetVelocity(veloc);

  // Align object with trajectory
  if (movementModel->AlignTrajectory())
  {
    if (vLen > 1e-4F)
    {
      // Normalize velocity
      veloc *= (1.0F / vLen);
      m.SetFromFront(veloc);
    }
  }
}


//
// RigidBodyCollision
//
// Rigid body collision function
//
void MapObjType::RigidBodyCollision(MapObj &obj, MapObj *, const Vector *)
{
  // FIXME: not done
  obj.SelfDestruct();
}


//
// FatalCollision
//
// Fatal collision function - object dies on collision
//
void MapObjType::FatalCollision(MapObj &obj, MapObj *, const Vector *)
{
  obj.SelfDestruct();
}


//
// MapObjType
//
// Constructor
//
MapObjType::MapObjType(const char *name, FScope *fScope) 
: GameObjType(name, fScope),
  mapPhysicsProc(NULL),
  mapCollideProc(NULL),
  isProjectile(FALSE),
  isExplosion(FALSE),
  resourcesInitialized(FALSE),
  idleAnimationCount(0)
{
  ASSERT(fScope);

  FScope *sScope, *ssScope;

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Get category
  categoryId = StdLoad::TypeString(fScope, "Category", "Unassigned");

  // Mesh
  meshConfig.ConfigureAnim(fScope);

  // No footprint by default
  footPrintType = NULL;

  // Get the attachment list
  sScope = fScope->GetFunction("Attachments", FALSE);

  if (sScope)
  {
    while ((ssScope = sScope->NextFunction()) != NULL)
    {
      switch (ssScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          Attachment  *attachment = new Attachment;
          attachment->typeName = ssScope->NextArgString();
          attachment->pointIdent = ssScope->NextArgString();
          attachments.Append(attachment);
          break;
        }

        default:
          break;
      }
    }
  }

  // Are there any lights being added
  if ((sScope = fScope->GetFunction("Lights", FALSE)) != NULL)
  {
    while ((ssScope = sScope->NextFunction()) != NULL)
    {
      lights.Append( new Vid::Light::Desc( ssScope));
    }
  }

  // Armour Class
  armourClassName = StdLoad::TypeString(fScope, "ArmourClass", "armor_invulnerable");
  armourClass = ArmourClass::RegisterArmourClass(armourClassName);

  // Maximum armour of an object
  armour = StdLoad::TypeU32(fScope, "Armour", 0, Range<U32>(0, 100000));
  armourInv = (armour > F32_EPSILON) ? 1.0f / (F32) armour : 0.0F;

  // Rate at which armour regens
  armourRegenInterval = U16(StdLoad::TypeU32(fScope, "ArmourRegenInterval", 0, Range<U32>(0, U16_MAX)));
  
  // Has armour initially
  armourInitial = StdLoad::TypeU32(fScope, "InitialArmour", 1);

  // Maximum hitpoints of an object
  hitpoints = StdLoad::TypeU32(fScope, "HitPoints", 0, Range<U32>(0, 100000));
  hitpointsInv = (hitpoints > F32_EPSILON) ? 1.0F / (F32) hitpoints : 0.0F;
  autoRepairHitpoints = StdLoad::TypeU32(fScope, "AutoRepairHitPoints", hitpoints / 2, Range<U32>(0, 100000));

  // High and Low health marks
  healthHighMark = (S32) (StdLoad::TypeF32(fScope, "HealthHighMark", 0.6666f, Range<F32>::percentage) * (F32) hitpoints);
  healthLowMark = (S32) (StdLoad::TypeF32(fScope, "HealthLowMark", 0.3333f, Range<F32>::percentage) * (F32) hitpoints);

  // Movement model 
  movementIdent = StdLoad::TypeString(fScope, "PhysicsModel", "Default");

  // Mass of the object in g (convert to kg)
  mass = StdLoad::TypeF32(fScope, "Mass", 1E6f) * 1E-3f;
  massInv = (mass > F32_EPSILON) ? (1.0F / mass) : 0.0F;

  // Explosions
  StdLoad::TypeReaperObjType(fScope, "DeathExplosion", deathExplosion);
  StdLoad::TypeReaperObjType(fScope, "SelfDestructExplosion", selfDestructExplosion);

  // Traction type for each level
  if ((sScope = fScope->GetFunction("TractionType", FALSE)) != NULL)
  {
    tractionType[Claim::LAYER_LOWER] = StdLoad::TypeStringD(sScope, "Default");
    tractionType[Claim::LAYER_UPPER] = StdLoad::TypeStringD(sScope, tractionType[Claim::LAYER_LOWER].str);
  }
  else
  {
    tractionType[Claim::LAYER_LOWER] = "Default";
    tractionType[Claim::LAYER_UPPER] = "Default";
  }

  // Resolve the traction type
  tractionIndex[Claim::LAYER_LOWER] = MoveTable::TractionIndex(tractionType[Claim::LAYER_LOWER].str);
  tractionIndex[Claim::LAYER_UPPER] = MoveTable::TractionIndex(tractionType[Claim::LAYER_UPPER].str);

  // GrainSize
  if ((sScope = fScope->GetFunction("GrainSize", FALSE)) != NULL)
  {
    grainSize = StdLoad::TypeU32(sScope, Range<U32>(1, 2));
  }
  else
  {
    // Auto calculate in LoadResource
    grainSize = U32_MAX;
  }
  
  // Always use a box test
  rayTestFlags = Ray::BOX;
  
  // Death explosion
  if ((sScope = fScope->GetFunction("RayTestFlags", FALSE)) != NULL)
  {
    VNode *vNode;

    while ((vNode = sScope->NextArgument(VNode::AT_STRING, FALSE)) != NULL)
    {
      switch (Crc::CalcStr(vNode->GetString()))
      {
        case 0x83B41B3B: // "Sphere"
          rayTestFlags |= Ray::SPHERE;
          break;

        case 0x1CCAFDCC: // "Box"
          rayTestFlags |= Ray::BOX;
          break;

        case 0xDE119E77: // "Poly"
          rayTestFlags |= Ray::POLY;
          break;
      }
    }
  }

  // Detachable flag
  if ((sScope = fScope->GetFunction("Detachable", FALSE)) != NULL)
  {
    isDetachable = StdLoad::TypeU32(sScope, Range<U32>::flag);
    detachableConfig = TRUE;
  }
  else
  {
    // Default is not to allow detachment
    isDetachable = FALSE;

    // But allow over-riding by a code-class
    detachableConfig = FALSE;
  }

  hasShadow = meshConfig.shadowRadius == 0.0f ? FALSE : TRUE;
  hasShadowConfig = hasShadow;

  // Read the showSeen flag configuration
  if ((sScope = fScope->GetFunction("ShowSeen", FALSE)) != NULL)
  {
    showSeen = StdLoad::TypeU32(sScope, Range<U32>::flag);
    showSeenConfig = TRUE;
  }
  else
  {
    // Default is hide objects under fog
    showSeen = FALSE;

    // But allow over-riding by a code-class
    showSeenConfig = FALSE;
  }

  showAlways = StdLoad::TypeU32(fScope, "ShowAlways", FALSE);
  nearFadeFactor = StdLoad::TypeF32(fScope, "NearFadeFactor", 1.0F);
  
  // Apply terrain damage to this type
  applyTerrainHealth = StdLoad::TypeU32(fScope, "ApplyTerrainHealth", FALSE, Range<U32>::flag);

  // Get the animation list
  if ((sScope = fScope->GetFunction("TypeDisplay", FALSE)) != NULL)
  {
    displayConfig = sScope->Dup();
  }
  else
  {
    displayConfig = NULL;
  }

  // AnimationFX
  if ((sScope = fScope->GetFunction("AnimationFX", FALSE)) != NULL)
  {
    while ((ssScope = sScope->NextFunction()) != NULL)
    {
      switch (ssScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          // Name of the animation
          GameIdent animation = StdLoad::TypeString(ssScope);

          // Find the animation FX list in the tree
          AnimationFXList *animFXList = animationFX.Find(animation.crc);

          // If not found, create one
          if (!animFXList)
          {
            animationFX.Add(animation.crc, animFXList = new AnimationFXList);
          }

          F32 frame = StdLoad::TypeF32(ssScope);
          animFXList->Append(new AnimationFX(frame, ssScope));
        }
      }
    }
  }

  // GenericFX
  if ((sScope = fScope->GetFunction("GenericFX", FALSE)) != NULL)
  {
    while ((ssScope = sScope->NextFunction()) != NULL)
    {
      switch (ssScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          // Read the key
          U32 key = Crc::CalcStr(ssScope->NextArgString());

          // Create the tree entry
          genericFX.Add(key, new GameIdent(ssScope->NextArgString()));
        }
      }
    }
  }

  // Does the type require constant texture animation
  hasTextureAnimation = HasProperty(0xEF9B0347); // "Flag::HasTextureAnimation"

  uvAnimRate = 0;
  if ((sScope = fScope->GetFunction("UVAnimRate", FALSE)) != NULL)
  {
    uvAnimRate = StdLoad::TypeF32(sScope);
  }
}


//
// ~MapObjType
//
// Destructor
//
MapObjType::~MapObjType()
{
  // Free lists
  meshConfig.Release();
  attachments.DisposeAll();
  lights.DisposeAll();

  if (footPrintType)
  {
    delete footPrintType;
    footPrintType = NULL;
  }

  if (displayConfig)
  {
    delete displayConfig;
    displayConfig = NULL;
  }

  animationFX.DisposeAll();
  genericFX.DisposeAll();
}


//
// PostLoad
//
// Called after all types are loaded
//
void MapObjType::PostLoad()
{
  // Call parent scope first
  GameObjType::PostLoad();

  // Resolve explosions
  Resolver::Type<ExplosionObjType>(deathExplosion);
  Resolver::Type<ExplosionObjType>(selfDestructExplosion);

  // Resolve the physics model
  movementModel = Movement::FindModel(movementIdent);

  if (movementModel->hasPhysics)
  {
    // Setup the physics simulation function pointer
    switch (movementModel->SimulationModel())
    {
      case 0x7E1FE4E4: // "RigidBody"
        mapPhysicsProc = RigidBodyPhysics;
        break;

      default:
        // This isnt life threatening, just means objects will sit around doing nothing
        LOG_ERR(("Invalid physics simulation type for [%s]", typeId.str))
        break;
    }

    // Setup collision function pointer
    switch (movementModel->CollisionModel())
    {
      case 0x7E1FE4E4: // "RigidBody"
        mapCollideProc = RigidBodyCollision;
        break;

      case 0x95BA4B11: // "Fatal"
      default:
        mapCollideProc = FatalCollision;
        break;
    }
  }
}


//
// InitializeResources
//
// Initialized type specific resources
//
Bool MapObjType::InitializeResources()
{
  // Do we need to initialize resources ?
  if (resourcesInitialized)
  {
    return (FALSE);
  }

  // Set the initialization flag
  resourcesInitialized = TRUE;

  // Read the mesh for this type
  meshConfig.PostLoad();

  // Get the mesh root
  MeshRoot & root = *GetMeshRoot();

  hasShadow = root.shadowRadius == 0.0f ? FALSE : TRUE;

  // Set the near fade factor in the mesh root
  root.SetNearFadeFactor(GetNearFadeFactor());

  // Is this object footprinted
  if (root.FamilyNode::FindMesh("SP-0"))
  {
    // Load the footprint
    footPrintType = new FootPrint::Type(root, GetName(), this);
  }
  // discard mesh local geometry (only footprints in dr2)
//  root.ReleaseLocals();

  // Resolve any light attatchment points
  List<Vid::Light::Desc>::Iterator il(&lights);
  while (Vid::Light::Desc * l = il++)
  {
    // resolve pointIdent's index
    //
    if (!l->pointIdent.Null() && !root.FindIdent( l->pointIdent))
    {
      LOG_ERR(("Can't find light attach point '%s' on %s", l->pointIdent.str, typeId.str));
      lights.Dispose( l);
      continue;
    }
  }

  // Resolve any attatchment types
  List<Attachment>::Iterator i(&attachments);
  Attachment *a;
  while ((a = i++) != NULL)
  {
    // Resolve the type
    a->type = GameObjCtrl::FindType<MapObjType>(a->typeName.crc);

    if (!a->type)
    {
      LOG_ERR(("Attachment '%s' is not a map object type!", a->typeName.str));
      attachments.Dispose(a);
      continue;
    }
    // Resolve the attatchment point
    if (!a->pointIdent.Null() && !root.FindIdent( a->pointIdent))
    {
      LOG_ERR(("Can't find attach point '%s' on %s", a->pointIdent.str, typeId.str))
      attachments.Dispose(a);
      continue;
    }
  }

  // Calculate the surface area (use the surface area of the bounding sphere)
  const Bounds &bounds = GetMeshRoot()->ObjectBounds();

  surface = 8.0F * ((bounds.Width() * bounds.Height()) + (bounds.Width() * bounds.Breadth()) + (bounds.Height() * bounds.Breadth()));
  surfaceInv = surface ? 1.0f / surface : 0.0f;

  // Auto-calculate grain size of object if not already specified
  if (grainSize == U32_MAX)
  {
    const F32 GRAIN_METRES = WC_CELLSIZEF32 * 0.5F;

    F32 radius = Max<F32>(bounds.Width(), bounds.Breadth());
    grainSize = Clamp<U32>(1, Utils::FtoL((GRAIN_METRES + radius * 2.0F) / GRAIN_METRES), 2);
  }

  // Death animation
  if (AnimList *animList = GetMeshRoot()->FindAnimCycle(0xF40D135F)) // "Death"
  {
    if (animList->type != anim1WAY)
    {
      animList->type = anim1WAY;
      LOG_WARN(("Object type '%s' has Death animation but its not 1way", GetName()))
    }
  }

  // Find all idle animations
  for (;;)
  {
    GameIdent idleName;

    // Generate the animation name (damn artist naming conventions!)
    if (idleAnimationCount)
    {
      Utils::Sprintf(idleName.str, idleName.GetSize(), "Idle%d", idleAnimationCount + 1);
      idleName.Update();
    }
    else
    {
      idleName = "Idle";
    }
    
    // Does this type have this animation
    if (GetMeshRoot()->FindAnimCycle(idleName.crc))
    {
      // Increase the count
      idleAnimationCount++;
    }
    else
    {
      break;
    }
  }

  // Success
  return (TRUE);
}


//
// GetMeshRoot
//
// Returns the mesh root for this type
//
MeshRoot * MapObjType::GetMeshRoot()
{
  if (!resourcesInitialized)
  {
    InitializeResources();
  }

  return (meshConfig.meshRoot);
}


//
// GetFootPrintType
//
// Get the footprint type, or NULL if none
//
FootPrint::Type * MapObjType::GetFootPrintType()
{
  // Ensure resources are initialized
  InitializeResources();

  // Return the foot type
  return (footPrintType);
}


//
// FindGenericFX
//
// Returns the FX type for the given key, or NULL
//
FX::Type * MapObjType::FindGenericFX(U32 key)
{
  // Find ident for the given key
  GameIdent *ident = genericFX.Find(key);

  // Find the referenced FX type
  return (ident ? FX::Find(ident->crc) : NULL);
}


//
// StartGenericFX
//
// Generate a generic effect on the given object
//
FX::Object * MapObjType::StartGenericFX(MapObj *obj, U32 key, FX::FXCallBack callBack, Bool process, const Vector *velocity, void *context, F32 lifeTime)
{
  ASSERT(obj)

  // Find the type
  if (FX::Type *type = FindGenericFX(key))
  {
    // Create the FX
    return (type->Generate(obj, callBack, process, velocity, context, lifeTime));
  }

  return (NULL);
}



///////////////////////////////////////////////////////////////////////////////
//
// Class MapObj - Instance class for above type
//

//
// CreateLight
//
// Create the specified light for the given mesh entity
//
void MapObj::CreateLight( Vid::Light::Desc * desc)
{
  ASSERT(meshEnt);
  ASSERT(desc);

  // InitResources resolves pointIdent
  //
  MeshObj * meshobj = Mesh().Get(desc->pointIdent);
  ASSERT( meshobj);

  Vid::Light::Obj * light = Vid::Light::Create( *desc, Environment::Light::IsNight());

  // Vid::Light::Create filters light new via performance and priority
  //
  if (light)
  {
    meshobj->Attach(*light);

    lights.Append( light);
  }
}

//
// Constructor
//
MapObj::MapObj(MapObjType *objType, U32 id) : GameObj(objType, id),
  attachments(&MapObj::attachedNode),
  animList(NULL),
  animationFX(NULL),
  negativeModifyHitPoints(FALSE),
  dying(FALSE),
  balanceData(NULL),
  iterTicker(0),
  currentCluster(NULL),
  clustOverlapX(0),
  clustOverlapZ(0),
  cellX(-1),
  cellZ(-1),
  currentLayer(objType->GetDefaultLayer()),
  footInstance(NULL),
  hitpoints(0),
  armour(0),
  meshEnt(NULL)
{
  // NOTE: Cell position initaialised to an INVALID location so that 
  // the first call to UpdateMapPos will update the cell pos.

  SYNC_BRUTAL("MapObjConstruct: " << objType->GetName() << " Id: " << id)

  // Save the object's time of birth
  birthTime = GameTime::SimTotalTime();

  // Ensure that the resources have been loaded for this type
  if (!MapType()->resourcesInitialized)
  {
    MapType()->InitializeResources();
  }
                                   
  // Cluster position
  for (U32 i = 0; i < 4; i++)
  {
    clustList[i] = NULL;
  }

  // Create the associated mesh object
  SetMesh(new MeshEnt(objType->GetMeshRoot()));

  // Does this object have constant texture animation
  if (MapType()->HasTextureAnimation())
  {
    Mesh().ActivateTexAnim(2);
  }

  // Does this object have constant uv animation
  if (MapType()->uvAnimRate)
  {
    Mesh().SetUVAnimRate( MapType()->uvAnimRate);
  }

  // keep track of lights
  lights.SetNodeMember( &Vid::Light::Obj::mapNode);

  // Create attached lights
  for (List<Vid::Light::Desc>::Iterator li(&objType->lights); *li; li++)
  {
    CreateLight(*li);
  }

  // Register the construction of this object
  MapObjCtrl::RegisterConstruction(this);

  // Reset speed
  speed = 0.0F;
  velocity.ClearData();

  // Set our initial movement processing state
  SetPrimitiveProcessing(MapType()->DefaultPrimitiveProcessing());

  // Reset the armour regen cycles
  armourRegenCycles = MapType()->armourRegenInterval;
}


//
// Destructor
//
MapObj::~MapObj()
{
  lights.UnlinkAll();

  // Delete the associated mesh object
  delete meshEnt;
}


//
// MapObj::MarkedForDeletion
//
// Called when the object is marked for deletion
//
void MapObj::MarkedForDeletion()
{
  // Mark attached objects for deletion
  NList<MapObj>::Iterator i(&attachments);

  while (*i)
  {
    // Mark attachment for deletion
    GameObjCtrl::MarkForDeletion(i++);
  }

  // Call parent scope
  GameObj::MarkedForDeletion();
}


//
// MapObj::PreDelete
//
// Pre deletion cleanup
//
void MapObj::PreDelete()
{
  // If this object is attached, detach it
  if (parent.Alive())
  {
    Detach();
  }

  // Detach any attached objects
  NList<MapObj>::Iterator i(&attachments);

  while (*i)
  {
    // Detach from parent
    (i++)->Detach();
  }

  // Register the destruction of this object - takes it off the map
  MapObjCtrl::RegisterDestruction(this);

  // Call parent scope last
  GameObj::PreDelete();
}

  
//
// MapObj::SetPrimitiveProcessing
//
// Move object to/from movement processing list
//
void MapObj::SetPrimitiveProcessing(Bool request)
{
  // Set local value
  requestPrimitive = request;

  // Move us to/from the movement list
  MapObjCtrl::UpdatePrimitiveProcessingState(this);
}


//
// MapObj::ProcessCycle
//
// Per-cycle processing
//
void MapObj::ProcessCycle()
{
  SYNC_BRUTAL("ProcessCycle " << TypeName() << ' ' << Id())

  // Perform type specific physics
  PERF_S(("MapObj Physics"))
  if (MapType()->mapPhysicsProc)
  {
    Matrix m = WorldMatrix();
    Vector s;

    MapType()->ProcessMapPhysics(*this, m);
    SetSimTarget(m);

    // Register movement with the collision system
    CollisionCtrl::AddObject(this);
  }
  PERF_E(("MapObj Physics"))

  // Regen armor
  PERF_S(("Regen Armor"))
  if (armour > 0 && armour < MapType()->armour)
  {
    if (armourRegenCycles)
    {
      if (!--armourRegenCycles)
      {
        ModifyArmour(1);
        armourRegenCycles = MapType()->armourRegenInterval;
      }
    }
  }
  PERF_E(("Regen Armor"))

  // Process animation
  PERF_S(("Animation"))
  ProcessAnimation();
  PERF_E(("Animation"))
}


//
// MapObj::SaveState
//
// Save a state configuration scope
//
void MapObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  GameObj::SaveState(fScope);

  // Create our specific config scope
  fScope = fScope->AddFunction(SCOPE_CONFIG);

  // Save the armour and hitpoints
  StdSave::TypePercentage(fScope, "Armour", MapType()->GetArmour(), armour);
  StdSave::TypePercentage(fScope, "HitPoints", MapType()->GetHitPoints(), hitpoints);

  // Save parent pointer (if we are attached)
  if (parent.Alive())
  {
    // Save reaper to parent
    StdSave::TypeReaper(fScope, "Parent", parent);

    // Save attach point
    StdSave::TypeString(fScope, "AttachPoint", attachPointIdent.str);

    // Save orientation matrix
    StdSave::TypeMatrix(fScope, "Orientation", ObjectMatrix());
  }
  else
  {
    // Are we on the map
    if (OnMap())
    {
      // Save position matrix
      StdSave::TypeMatrix(fScope, "Position", WorldMatrix());
    }
  }

  // If not on the map
  if (!OnMap())
  {
    fScope->AddFunction("OffMap");
  }

  // Are we zipped
  if (GetFootInstance())
  {
    fScope->AddFunction("Zip");
  }
  else

  // Generate a save error if saving non-zipped objects
  if (MapType()->GetFootPrintType() && !SaveGame::SaveActive())
  {
    // "Mission::LoadError"
    CONSOLE(0xA1D5DDD2, ("Object %s (id:%d) was not zipped", TypeName(), Id()))
  }

  // Save game data
  if (SaveGame::SaveActive())
  {
    StdSave::TypeVector(fScope, "Velocity", velocity);
    StdSave::TypeF32(fScope, "BirthTime", birthTime);

    if (Bool(requestPrimitive) != MapType()->DefaultPrimitiveProcessing())
    {
      StdSave::TypeU32(fScope, "RequestPrimitive", requestPrimitive);
    }

    if (negativeModifyHitPoints)
    {
      StdSave::TypeU32(fScope, "NegativeModifyHitPoints", negativeModifyHitPoints);
    }

    if (dying)
    {
      StdSave::TypeU32(fScope, "Dying", dying);
    }
    
    if (animList)
    {
      StdSave::TypeU32(fScope, "AnimList", animList->name.crc);
    }

    if (MapType()->armour)
    {
      StdSave::TypeU32(fScope, "ArmourRegenCycles", armourRegenCycles);
    }

    if (!theMesh)
    {
      theMesh = meshEnt;
    }
    theMesh->SaveState(fScope->AddFunction("MeshEnt"));
  }
}


//
// MapObj::LoadState
//
// Load a state configuration scope
//
void MapObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  GameObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);
  FScope *sScope;

  // Add objects to the map by default
  Bool addToMap = TRUE;
  Bool zip = FALSE;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0x59CA1A6D: // "Armour"
        armour = StdLoad::TypePercentage(sScope, MapType()->GetArmour());
        break;

      case 0x761A32B0: // "HitPoints"
        hitpoints = StdLoad::TypePercentage(sScope, MapType()->GetHitPoints());
        break;

      case 0x411AC76D: // "Parent"
        // Load the parent pointer
        StdLoad::TypeReaper(sScope, parent);
        break;

      case 0xF7C04E02: // "AttachPoint"
        // Load the attachment point name
        attachPointIdent = StdLoad::TypeString(fScope, "AttachPoint");
        break;

      case 0xEE2D2689: // "Orientation"
      {
        Matrix m;
        m.ClearData();
        StdLoad::TypeMatrix(sScope, m);
        SetSimCurrent( m);
        break;
      }

      case 0x8D878A02: // "Position"
      {
        Matrix m;
        m.ClearData();
        StdLoad::TypeMatrix(sScope, m);
        SetSimCurrent( m);
        break;
      }

      case 0xBF87218C: // "OffMap"
        addToMap = FALSE;
        break;

      case 0xCC5B039A: // "Zip"
        zip = TRUE;
        break;

      case 0x7010E6E4: // "Velocity"
        StdLoad::TypeVector(sScope, velocity);
        speed = velocity.Magnitude();
        break;

      case 0x9D13A00A: // "BirthTime"
        birthTime = StdLoad::TypeF32(sScope);
        break;

      case 0xA358E0F0: // "ArmourRegenCycles"
        armourRegenCycles = U16(StdLoad::TypeU32(sScope));
        break;

      case 0xFF54E071: // "AnimList"
      {
        U32 crc = StdLoad::TypeU32(sScope);
        animList = GetMeshRoot()->FindAnimCycle(crc);
        animationFX = MapType()->animationFX.Find(crc);
        break;
      }

      case 0x6459A3A0: // "RequestPrimitive"
        requestPrimitive = StdLoad::TypeU32(sScope);
        break;

      case 0x2C7BA4FA: // "NegativeModifyHitPoints"
        negativeModifyHitPoints = StdLoad::TypeU32(sScope);
        break;

      case 0x8A677538: // "Dying"
        dying = StdLoad::TypeU32(sScope);
        break;

      case 0xC1DFB952: // "MeshEnt"
        meshEnt->LoadState(sScope);
        break;
    }
  }

  // Should the object be added to the map
  if (addToMap)
  {
    // All zipping objects should be prepended so that surface types 
    // are updated before any moving units are placed down.
    MapObjCtrl::AddToListOnMap(this, zip, zip);
  }
}


//
// MapObj::Equip
//
// Equip the object
//
void MapObj::Equip()
{
  // Call parent scope first
  GameObj::Equip();

  MapObjType *type = MapType();

  // Add the attachments descriped in the type
  for (List<MapObjType::Attachment>::Iterator i(&type->attachments); *i; i++)
  {
    // Create the object
    MapObj *obj = MapObjCtrl::ObjectNew((*i)->type);

    // Attach it to ourself
    obj->Attach(this, &(*i)->pointIdent);
  }
}


//
// MapObj::PostLoad
//
// Called after all objects are loaded
//
void MapObj::PostLoad()
{
  // Call parent scope first
  GameObj::PostLoad();

  // Are we meant to have a parent
  if (parent.HasResolveId())
  {
    // Resolve parent pointer
    Resolver::Object<MapObj, MapObjType>(parent);

    // If we were attached then attach ourselves to the parent
    if (parent.Alive())
    {
      // resolve the attach point ident index
      if (!(*parent).Mesh().FindIdent( attachPointIdent))
      {
        LOG_WARN(("Can't find attach point %s on %s", attachPointIdent.str, (*parent).type->typeId.str));
        attachPointIdent.index = 0;
      }
      Attach();
    }
    else
    {
      // For some reason parent is not alive, so delete
      MarkForDeletion();

      LOG_DIAG(("No Parent, Marking for deletion [%s]", TypeName()))
    }
  }
}


//
// MapObj::ProcessEvent
//
// Process an event
//
Bool MapObj::SendEvent(const Task::Event &event, Bool idle)
{
  // Send event to the task first
  return (GameObj::SendEvent(event, idle));
}


//
// MapObj::UpdateMapPos
//
// check if its on the map and update cluster lists
//
void MapObj::UpdateMapPos()
{
  PERF_S("MapObj::UpdateMapPos");

  // Update position of attachments
  for (NList<MapObj>::Iterator a(&attachments); *a; a++)
  {
    (*a)->UpdateMapPos();
  }

  Bool onMap = OnMap();

  // Verify that it is entirely on the map
  Vector pos = WorldMatrix().posit;

  if (!CheckWorldPosition(pos))
  {
    // LOG_DIAG(("\"%s\" (id:%d) MOVED OFF MAP (%.1f,%.1f)", TypeName(), Id(), inMat.posit.x, inMat.posit.z));
    MarkForDeletion();
  }

  SYNC_BRUTAL("UpdateMapPos: " << Id() << ' ' << Crc::Calc(&WorldMatrix(), sizeof Matrix))
  
  // Update current cluster pointer
  MapCluster *newCluster = WorldCtrl::MetresToCluster(pos.x, pos.z);

  // Update current cell indexes
  S32 newCellX = WorldCtrl::MetresToCellX(pos.x);
  S32 newCellZ = WorldCtrl::MetresToCellZ(pos.z);

  if (newCellX != cellX || newCellZ != cellZ)
  {
    if (onMap)
    {
      // Release cell hooks
      CaptureCellHooks(FALSE);
    }

    // Update cell indexes
    cellX = newCellX;
    cellZ = newCellZ;

    // Get the new cell
    TerrainData::Cell *dataCell = &TerrainData::GetCell(cellX, cellZ);

    // Update terrain balance pointer
    balanceData = &(MoveTable::GetBalance(dataCell->surface, MapType()->GetTractionIndex(GetCurrentLayer())));

    // Capture cell hooks
    if (onMap)
    {
      CaptureCellHooks(TRUE);

      // Update the cell position of attachments
      for (NList<MapObj>::Iterator a(&attachments); *a; a++)
      {
        (*a)->cellX = cellX;
        (*a)->cellZ = cellZ;
      }
    }
  }

  ASSERT(WorldCtrl::MetreOnMap(pos.x, pos.z))

  // Are we on the map
  if (onMap)
  {
    // Has the centre of this object has moved into a new cluster?
    if (currentCluster != newCluster)
    {
      SYNC_BRUTAL("ChangeCluster: " << Id() << ' ' << Crc::Calc(&WorldMatrix(), sizeof Matrix))

      // Release all cluster usage hooks
      CaptureClusterHooks(FALSE);

      // Release other map hooks
      CaptureMapHooks(FALSE);

      // Assign new cluster
      currentCluster = newCluster;

      // Recapture map hooks
      CaptureMapHooks(TRUE);

      // Recapture cluster usage hooks
      CaptureClusterHooks(TRUE, TRUE);

      SYNC("ChangeCluster: " << Id() << ' ' << Crc::Calc(&WorldMatrix(), sizeof Matrix))
    }
    else
    {
      // The centre point is in the same cluster but the bounding sphere of the 
      // object may have moved over into another cluster
      S8 x, z;

      TestClusterOverlap(x, z);

      if (x != clustOverlapX || z != clustOverlapZ)
      {
        // The x- or z-overlap has changed, remove current hooks
        CaptureClusterHooks(FALSE);

        // Update overlap values with the ones we just calculated
        clustOverlapX = x;
        clustOverlapZ = z;

        // Recapture cluster hooks, requesting that it does NOT 
        // recalculate the overlap values
        CaptureClusterHooks(TRUE, FALSE);
      }
    }
  }
  else
  {
    // Not on map but keep the cluster pointer current anyway
    currentCluster = newCluster;
  }

  PERF_E("MapObj::UpdateMapPos");
}


//
// MapObj::CheckWorldPosition
//
// Test the position of the object and ensure it fits within the world
// Logs a nasty message if it goes off the map.
//
Bool MapObj::CheckWorldPosition(Vector &v)
{
  if (_isnan(v.x) || _isnan(v.z) || _isnan(v.y))
  {
    #ifdef DEVELOPMENT
      Debug::CallStack::Dump();
      LOG_WARN(("POSITION WAS NOT A NUMBER ! %f %f %f", v.x, v.y, v.z))
    #endif

    // Place the object in the middle
    v.x = WorldCtrl::MetreMapX() * 0.5f;
    v.y = -100.0f;
    v.z = WorldCtrl::MetreMapX() * 0.5f;
    return (FALSE);
  }

  // Work out the difference between the centre of the world sphere and the
  // world matrix position.  We need to ensure BOTH points are on the map.
  const Vector &sPos = Origin();
  const Vector &wPos = Position();

  F32 errorX = (F32) (fabs(sPos.x - wPos.x) + WorldCtrl::CellSize());
  F32 errorZ = (F32) (fabs(sPos.z - wPos.z) + WorldCtrl::CellSize());
  
  Bool offMap = FALSE;

  // Clamp position to within map bounds
  if ((v.x - errorX) < WorldCtrl::MetreMapXMin())
  {
    v.x = WorldCtrl::MetreMapXMin() + errorX;
    offMap = TRUE;
  }
  if ((v.x + errorX) > WorldCtrl::MetreMapXMax())
  {
    v.x = WorldCtrl::MetreMapXMax() - errorX;
    offMap = TRUE;
  }

  if ((v.z - errorZ) < WorldCtrl::MetreMapZMin())
  {
    v.z = WorldCtrl::MetreMapZMin() + errorZ;
    offMap = TRUE;
  }
  if ((v.z + errorZ) > WorldCtrl::MetreMapZMax())
  {
    v.z = WorldCtrl::MetreMapZMax() - errorZ;
    offMap = TRUE;
  }

  return (!offMap);
}


//
// SetMeshEnt
//
// Swap meshes for morphing units
//
void MapObj::SetMesh(MeshEnt *ent)
{
  ASSERT(ent)

  if (meshEnt && meshEnt->GetEffect())
  {
    // existing mesh; take care of effects
    //
    meshEnt->GetEffect()->SwapMesh( meshEnt, ent);
  }

  meshEnt = ent;
  meshState0 = &meshEnt->states0[0];
  meshState1 = &meshEnt->states1[0];
}


//
// MapObj::Attach
//
// Attach an object to a node on a parent object
//
void MapObj::Attach(MapObj *par, FamilyNode &node)
{
  ASSERT( par);

  // Set the parrent
  parent = par;

  // deal with the possiblity of multiple ents in the chain
  //
  const FamilyNode *n = &node;
  while (n && !n->IsMeshEnt())
  {
    n = n->Parent();
  }
  ASSERT( n);

  // Attach out mesh to the node
  ((MeshEnt *) n)->AttachAt( node, Mesh());

  // Set the point
  attachPointIdent = node.GetName();

  // Append this object to the parents object list
  parent->attachments.Append(this);

  // Check to make sure that there is no circular stuff hapenning
  MapObjPtr walk = parent;
  while (walk.Alive())
  {
    if (this == walk.GetData())
    {
      ERR_CONFIG(("Circular attachment path detected!"))
    }
    walk = walk->parent;
  }

  Mesh().SetTeamColor( parent->Mesh().teamColor);
}


//
// MapObj::Attach
//
// Attach this object to a parent object
//
void MapObj::Attach(MapObj *par, NodeIdent *ident)
{
  // If the pointer was supplied then setup the reaper (equip)
  // If the pointer was not supplied then assume the reaper is already set (postload)
  if (par)
  {
    ASSERT(ident)

    parent = par;
    attachPointIdent = *ident;
  }

  // Check to make sure that there is no circular stuff hapenning
  MapObjPtr walk = parent;
  while (walk.Alive())
  {
    if (this == walk.GetData())
    {
      ERR_CONFIG(("Circular attachment path detected!"))
    }
    walk = walk->parent;
  }

  // Append this object to the parents object list
  parent->attachments.Append(this);

  // InitializeResources resolves attachPointIdent
  //
  MeshObj *meshobj = parent->Mesh().Get(attachPointIdent);

  // Attach this object to the parents mesh
  parent->Mesh().AttachAt( *meshobj, Mesh());

  // If we are being equiped then add ourselves
  // to the map if our parent is on the map
  if (par)
  {
    if (parent->OnMap())
    {
      // Add to the map
      MapObjCtrl::AddToMap(this);
    }
  }

  Mesh().SetTeamColor( parent->Mesh().teamColor);
}


//
// MapObj::Detach
//
// Detach this object from its parent
//
void MapObj::Detach()
{
  ASSERT(parent.Alive())

  // Unlink from the parent
  parent->attachments.Unlink(this);

  // Detach ourselves from the parents mesh
  Mesh().Detach();

  // Set the parent pointer to nought
  parent.Clear();
}


//
// MapObj::ModifyHitPoints
//
// Modify the number of hitpoints an object has
//
void MapObj::ModifyHitPoints(S32 mod, UnitObj *sourceUnit, Team *sourceTeam, const Vector *velocity)
{
  SYNC("ModifyHitPoints: " << Id() << " HP: " << hitpoints << " Armour: " << armour << " Mod: " << mod)

  // If its dead we can't hurt it anymore
  if (hitpoints < 0)
  {
    return;
  }

  // Is there any armour ?
  if (armour)
  {
    // Will this modification destroy the armour ?
    if (mod < -armour)
    {
      // Armour is now gone
      StartGenericFX(0x354295F9, NULL, TRUE, velocity); // "MapObj::Armour::Lost"

      // Reduce the modifier by the amount armour will absorb
      mod += armour;

      // Clear the armour value to prevent it from regenerating
      armour = 0;
    }
    else
    // Is this an increase or a decrease ?
    // Armour can only be increased by ModifyArmour!
    if (mod < 0)
    {
      // Is our armour currently at max ?
      if (armour == MapType()->armour)
      {
        // Armour is now gone
        StartGenericFX(0x4D3B0CCE, RegenArmourCallBack); // "MapObj::Armour::Regen"
      }

      // Modify armour accordingly
      armour += mod;

      // If armour was modified, generate armour fx and add to ouch list
      if ((mod < 0) && OnMap())
      {
        StartGenericFX(0x944E1B17, NULL, TRUE, velocity); // "MapObj::Armour::Damaged"
        MapObjCtrl::GetOuchList().AppendNoDup(this);
      }

      return;
    }
    else
    {
      // Ignore positive bumps if we have armour
      return;
    }
  }

  // Save hitpoints before modification
  S32 before = hitpoints;
  
  if ((hitpoints += mod) < 0)
  {
    // Does this object block death
    if (!Blocked(Tasks::MapDeath::GetConfigBlockingPriority()))
    {
      // Notify the task that the unit is dying
      SendEvent(Task::Event(MapObjNotify::Dying)); 

      // Flush current tasks
      FlushTasks(Tasks::MapDeath::GetConfigBlockingPriority());

      // Set dying flag
      dying = TRUE;

      // Start Death FX
      StartGenericFX(0x700372AA, NULL, TRUE, velocity); // "MapObj::Death"
      
      // Give credit to the team that caused the kill
      if (sourceTeam)
      {
        sourceTeam->ReportKill(this);
      }

      // Generate an explosion
      if (MapType()->deathExplosion.Alive())
      {
        MapType()->deathExplosion->Detonate(Origin(), sourceUnit, sourceTeam);
      }

      // Kill all units on the footprint
      if (footInstance)
      {
        UnitObjIter::ClaimRect i
        (
          footInstance->GetOrigin() + Point<S32>(1, 1), 
          footInstance->GetOrigin() + footInstance->Size() - Point<S32>(2, 2)
        );
        UnitObj *obj;

        while ((obj = i.Next()) != NULL)
        {
          obj->SelfDestruct(FALSE, sourceTeam);
        }   
      }

      // Now give the object a death task
      if (Tasks::MapDeath::RequiresDeathTask(this))
      {
        PrependTask(new Tasks::MapDeath(this));
      }
      else
      {
        MarkForDeletion();
      }
    }
    else
    {
      LOG_DIAG(("Object %s (%u) blocked death!", TypeName(), Id()))
    }
  }
  else
  {
    // Clamp to max 
    if (hitpoints > MapType()->GetHitPoints())
    {
      hitpoints = MapType()->GetHitPoints();
    }

    // Did the hits points cross a high or low health mark ?
    if (mod > 0)
    {
      if (before <= MapType()->healthHighMark && 
        hitpoints > MapType()->healthHighMark)
      {
        // Start High Health FX
        StartGenericFX(0x40CAF006, HighHealthCallBack); // "MapObj::Health::High"
      }

      // Has there been a negative mod yet (for positive health increase during construction)
      if (negativeModifyHitPoints)
      {
        if (before < MapType()->healthLowMark &&
            hitpoints >= MapType()->healthLowMark &&
            hitpoints <= MapType()->healthHighMark)
        {
          // Start Medium Health FX
          StartGenericFX(0x30D3723D, MediumHealthCallBack); // "MapObj::Health::Medium"
        }
      }
    }
    else
    {
      // First time hit, or we were above the low range
      if (!negativeModifyHitPoints || before >= MapType()->healthLowMark)
      {
        // Are we in the low range now
        if (hitpoints < MapType()->healthLowMark)
        {
          // Start Low Health FX
          StartGenericFX(0x5B403BC7, LowHealthCallBack); // "MapObj::Health::Low"
        }
      }

      // First time hit, or we were above the high range
      if (!negativeModifyHitPoints || before > MapType()->healthHighMark)
      {
        // Are we in the medium range
        if (hitpoints >= MapType()->healthLowMark && hitpoints <= MapType()->healthHighMark)
        {
          // Start Medium Health FX
          StartGenericFX(0x30D3723D, MediumHealthCallBack); // "MapObj::Health::Medium"
        }
      }

      // We have now been hit
      negativeModifyHitPoints = TRUE;
    }
  }

  // If hitpoints were modified, generate damage fx and add to ouch list
  if ((hitpoints != before) && OnMap())
  {
    if (mod < 0)
    {
      StartGenericFX(0xE9931FAF, NULL, TRUE, velocity); // "MapObj::Damaged"
      MapObjCtrl::GetOuchList().AppendNoDup(this);
    }
  }
}


//
// MapObj::ModifyArmour
//
// Modify the number amount of armour an object has
//
void MapObj::ModifyArmour(S32 mod)
{
  SYNC("ModifyArmour: " << Id() << " Armour: " << armour << " Mod: " << mod)

  // Modify the armour
  armour += mod;
  
  // Clamp the new armour values
  armour = Clamp<S32>(0, armour, MapType()->armour);
}


//
// StartGenericFX
//
// Starts the given generic FX (returns NULL if not created)
//
FX::Object * MapObj::StartGenericFX(U32 key, FX::FXCallBack callBack, Bool process, const Vector *velocity, void *context, F32 lifeTime)
{
  // Find the type
  if (FX::Type *type = MapType()->FindGenericFX(key))
  {
    // Create the FX
    return (type->Generate(this, callBack, process, velocity, context, lifeTime));
  }

  return (NULL);
}


//
// StartFX
//
// Starts the given FX type (returns NULL if not created)
//
FX::Object * MapObj::StartFX(U32 typeCrc, FX::FXCallBack callBack, Bool process, const Vector *velocity, void *context, F32 lifeTime)
{
  // Find the type
  if (FX::Type *type = FX::Find(typeCrc))
  {
    // Create the FX
    return (type->Generate(this, callBack, process, velocity, context, lifeTime));
  }

  return (NULL);
}


//
// Get the color and percentage for the health of the unit
//
void MapObj::GetHealthInfo(Color &color, F32 &pct)
{
  if (GetArmour() && MapType()->GetArmour())
  {
    pct = F32(Clamp<S32>(0, GetArmour(), MapType()->GetArmour())) / F32(MapType()->GetArmour());
    U32 p = U32(255.0f * pct);
    color = Color(U32(0), p, 128 + (p >> 1), 255);
  }
  else
  if (MapType()->GetHitPoints())
  {
    pct = F32(Clamp<S32>(0, GetHitPoints(), MapType()->GetHitPoints())) / F32(MapType()->GetHitPoints());
    U32 p = U32(255.0f * pct);
    color = Color(U32(255 - p), p, 0, 255);
  }
  else
  {
    color = (Color(U32(0), 0, 0, 0));
  }
}


//
// GetSeenVisible
//
// Is this object seen/visible to the given team
//
void MapObj::GetSeenVisible(Team *team, Bool &seen, Bool &visible)
{
  // Are we attached to another object
  if (parent.Alive())
  {
    // Adopt visibility of parent
    parent->GetSeenVisible(team, seen, visible);
  }
  else
  {
    // Is this object footprinted
    if (footInstance)
    {
      // Get fringe bounds
      Point<S32> p0(footInstance->GetMin() - 1);
      Point<S32> p1(footInstance->GetMax() + 1);

      // Clamp onto the map
      WorldCtrl::ClampCellPoint(p0);
      WorldCtrl::ClampCellPoint(p1);

      // Get seen/visible for area
      Sight::SeenVisible(Area<U32>(U32(p0.x), U32(p0.y), U32(p1.x), U32(p1.y)), team, seen, visible);  
    }
    else
    {
      // Can we see the cell the object is on
      Sight::SeenVisible(cellX, cellZ, team, seen, visible);
    }
  }
}


//
// GetSeen
//
// Has this object ever been seen by the given team
//
Bool MapObj::GetSeen(Team *team)
{
  if (footInstance)
  {
    Bool seen, visible;
    GetSeenVisible(team, seen, visible);
    return(seen);
  }
  else
  {
    return (Sight::Seen(cellX, cellZ, team));
  }
}


//
// GetVisible
//
// Is this object visible to the given team
//
Bool MapObj::GetVisible(Team *team)
{
  if (footInstance)
  {
    Bool seen, visible;
    GetSeenVisible(team, seen, visible);
    return(visible);
  }
  else
  {
    return (Sight::Visible(cellX, cellZ, team));
  }
}


//
// MapObj::AddToMapHook
//
// AddToMapHook
//
void MapObj::AddToMapHook()
{
  // Update current cell indexes
//  cellX = WorldCtrl::MetresToCellX(WorldMatrix().posit.x);
//  cellZ = WorldCtrl::MetresToCellZ(WorldMatrix().posit.z);

  // Capture all map hooks
  CaptureAllHooks(TRUE);

  // All attachments must be added to the map as well
  NList<MapObj>::Iterator i(&attachments);
  while (*i)
  {
    // Add the attachment to the map
    MapObjCtrl::AddToMap(i++);
  }

  // Add any effects this object generates now that its on the map

  // SmokeFX
  StartGenericFX(0x67D5607A, SmokeCallBack); // "MapObj::Smoke"

  // Ignore HealthFX if no hitpoints (unit construction)
  if (hitpoints > 0)
  {
    if (hitpoints > MapType()->healthHighMark)
    {
      // Start High Health FX
      StartGenericFX(0x40CAF006, HighHealthCallBack); // "MapObj::Health::High"
    }
    else

    if (negativeModifyHitPoints)
    {
      if (hitpoints >= MapType()->healthLowMark)
      {
        // Start Medium Health FX
        StartGenericFX(0x30D3723D, MediumHealthCallBack); // "MapObj::Health::Medium"
      }
      else
      {
        // Start Low Health FX
        StartGenericFX(0x5B403BC7, LowHealthCallBack); // "MapObj::Health::Low"
      }
    }
  }
}


//
// MapObj::RemoveFromMapHook
//
// Remove from map hook
//
void MapObj::RemoveFromMapHook()
{
  // Toggle any footprint
  ToggleFootPrint(FALSE);

  // Release all map hooks
  CaptureAllHooks(FALSE);

  // All attachments must be removed from the map as well
  NList<MapObj>::Iterator i(&attachments);
  while (*i)
  {
    // Mark attachment for deletion
    MapObjCtrl::RemoveFromMap(i++);
  }
}


//
// MapObj::CaptureAllHooks
// 
// Capture All hooks
//
void MapObj::CaptureAllHooks(Bool capture)
{
  CaptureMapHooks(capture);
  CaptureClusterHooks(capture);
  CaptureCellHooks(capture);
}


//
// MapObj::CaptureMapHooks
//
// Capture map hooks when the object's centre point moves to a different cluster
//
void MapObj::CaptureMapHooks(Bool capture)
{
  // CaptureMapHooks
  if (capture)
  {
    // Recalculate new cluster if it has not been set
    if (currentCluster == NULL)
    {
      currentCluster = WorldCtrl::MetresToCluster(Position().x, Position().z);
    }
  }

  // ReleaseMapHooks
  else
  {
  }
}


//
// MapObj::CaptureCellHooks
//
// Capture/Release cell hooks when part of the bounding sphere of an object moves cells
//
void MapObj::CaptureCellHooks(Bool capture)
{
  if (capture)
  {
    ASSERT(!terrainHealthNode.InUse());

    // Add to the terrain health effects list
    if (MapType()->ApplyTerrainHealth() && GetBalanceData().health)
    {
      MapObjCtrl::GetTerrainHealthList().Append(this);
    }
  }
  else
  {
    // Remove from terrain health list
    if (terrainHealthNode.InUse())
    {
      MapObjCtrl::GetTerrainHealthList().Unlink(this);
    }
  }
}


//
// MapObj::CaptureClusterHooks
//
// Capture/Release cluster hooks when part of the bounding sphere of an object moves clusters
//
void MapObj::CaptureClusterHooks(Bool capture, Bool calcOverlap)
{
  ASSERT(currentCluster);

  if 
  (
    MapType()->IsProjectile() ||
    MapType()->IsExplosion()
  )
  {
    return;
  }

  // CaptureClusterHooks
  if (capture)
  {
    // If the cluster overlap needs to be recalculated, do it now
    if (calcOverlap)
    {
      TestClusterOverlap(clustOverlapX, clustOverlapZ);
    }

    ASSERT(clustOverlapX >= -1 && clustOverlapX <= 1);
    ASSERT(clustOverlapZ >= -1 && clustOverlapZ <= 1);

    // Hook into the overlapped clusters
    U32 nx = abs(clustOverlapX) + 1;
    U32 nz = abs(clustOverlapZ) + 1;
    S32 cx = currentCluster->xIndex;
    S32 cz = currentCluster->zIndex;

    // The object may be in 1, 2, or 4 clusters, using a bounding box
    // method of calculating its clusters.  This could be improved
    // to use the bounding sphere to test whether the object is actually
    // overlapping the 4th cluster or just 3 clusters.
    for (U32 x = 0; x < nx; ++x, cx += clustOverlapX)
    {
      for (U32 z = 0; z < nz; ++z, cz += clustOverlapZ)
      {
        MapCluster *c = WorldCtrl::GetCluster(cx, cz);
        c->listObjs.Append(this);
        clustList[c->nodeIndex] = c;
      }
      cz = currentCluster->zIndex;
    }
  }

  // ReleaseClusterHooks
  else
  {
    for (U32 i = 0; i < 4; i++)
    {
      if (clustList[i])
      {
        clustList[i]->listObjs.Unlink(this);
        clustList[i] = NULL;
      }
    }
  }
}


//
// ToggleFootPrint
//
// Apply/Remove an optional footprint
//
void MapObj::ToggleFootPrint(Bool toggle)
{
  // See if type has a setup footprint
  FootPrint::Type *footType = MapType()->GetFootPrintType();

  if (footType)
  {
    footType->Toggle(this, toggle);
  }
}


//
// MapObj::TestClusterOverlap
//
// Determine if this object overlaps any clusters, return results in x and z
//
void MapObj::TestClusterOverlap(S8 &x, S8 &z)
{
  ASSERT(currentCluster);

  const Matrix &m = WorldMatrix();
  F32 radius = ObjectBounds().Radius();

  x = z = 0;

  // Test for overlaps along x-axis
  if (m.posit.x + radius > currentCluster->x1)
  {
    x = S8((currentCluster->xIndex < (WorldCtrl::ClusterMapX() - 1)) ? 1 : 0);
  }
  else if (m.posit.x - radius < currentCluster->x0)
  {
    x = S8((currentCluster->xIndex > 0) ? -1 : 0);
  }

  // Test for overlaps along z-axis
  if (m.posit.z + radius > currentCluster->z1)
  {
    z = S8((currentCluster->zIndex < (WorldCtrl::ClusterMapZ() - 1)) ? 1 : 0);
  }
  else if (m.posit.z - radius < currentCluster->z0)
  {
    z = S8((currentCluster->zIndex > 0) ? -1 : 0);
  }
}


//
// SelfDestruct
//
// Make this object self destruct
//
void MapObj::SelfDestruct(Bool explosion, Team *modifier)
{
  // Modify the hitpoints if on the map
  if (OnMap())
  {
    // Trigger the explosion
    if (explosion)
    {
      if (MapType()->selfDestructExplosion.Alive())
      {
        MapType()->selfDestructExplosion->Detonate
        (
          Origin(), 
          Promote::Object<UnitObjType, UnitObj>(this), 
          modifier
        );
      }
    }

    // Kill the object, passing in the optional team modifier
    ModifyHitPoints(S32_MIN, NULL, modifier);
  }
  else
  {
    MarkForDeletion();
  }
}


//
// MapObj::Info
//
// Dump information about the object
//
ostream & MapObj::Info(ostream &o)
{
  return (
    o 
    << GameObj::Info(o)
    << "W:" << WorldMatrix() << endl
    << "WCRC:" << Crc::Calc(&WorldMatrix(), sizeof Matrix) << endl
    << "TP:" << Crc::Calc(&TargetPosition(), sizeof Vector) << endl
    << "TR:" << Crc::Calc(&TargetRotation(), sizeof Quaternion) << endl
    << "O:" << Crc::Calc(&ObjectMatrix(), sizeof Matrix) << endl
    << "HP:" << hitpoints << endl
  );
}


//
// MapObj::HasAnimation
//
Bool MapObj::HasAnimation(U32 crc)
{
  return (MapType()->GetMeshRoot()->FindAnimCycle(crc) ? TRUE : FALSE);
}


//
// MapObj::SetAnimation
//
// Set the current animation
//
void MapObj::SetAnimation(const char *name, Bool blend, Bool activate) // = TRUE, = TRUE
{
  SetAnimation(Crc::CalcStr(name), blend, activate);
}

//
// MapObj::SetFogTarget
//
// Set the current fog of war target value
//
void MapObj::SetFogTarget( U32 fog, U32 alpha, Bool immediate) // = TRUE
{
  Mesh().SetFogTarget( Terrain::fogFactorsS32[ U32( *Vid::Var::Terrain::shroud ? fog : 7)], Terrain::fogFactorsS32[ U32( *Vid::Var::Terrain::shroud ? alpha : 7)], immediate);
}

//
// MapObj::SetAnimation
//
// Set the current animation
//
void MapObj::SetAnimation(U32 crc, Bool blend, Bool activate) // = TRUE, TRUE
{   
  SYNC_BRUTAL("Animation " << crc << ' ' << blend << ' ' << activate << ' ' << TypeName() << ' ' << Id())

  // Is there an animation currently active
  if (animList && Mesh().AnimIsActive())
  {
    // Notify the task that the animation is completed
    PostEvent(Task::Event(MapObjNotify::AnimationDone, this, animList->name.crc));

    // Notify the parent if there is one (for animating upgrades)
    if (parent.Alive())
    {
      parent->PostEvent(Task::Event(MapObjNotify::AnimationDoneChild, this, animList->name.crc));
    }
  }

  // Will this object have animations processed
  if (OnPrimitiveList())
  {
    // Attempt to find the requested animation
    if ((animList = GetMeshRoot()->FindAnimCycle(crc)) != NULL)
    { 
      if (blend)
      {
        Mesh().BlendAnimCycle(*animList, Vid::renderState.animBlendTime);
      }
      else
      {
        Mesh().SetAnimCycle(*animList);
      }

      // Start any animation effects
      animationFX = MapType()->animationFX.Find(crc);

      // If the animation was only one frame, we're done
      if (!animList->endFrame)
      {
        animList = NULL;
      }

      if (!activate)
      {
        Mesh().ActivateAnim( FALSE);
      }
    }
  }
  else
  {
    animList = NULL;
  }

  // Was the animation not found, or only one frame
  if (!animList)
  {
    // Notify the task that the animation is completed
    PostEvent(Task::Event(MapObjNotify::AnimationDone, this, crc));

    // Notify the parent if there is one (for animating upgrades)
    if (parent.Alive())
    {
      parent->PostEvent(Task::Event(MapObjNotify::AnimationDoneChild, this, crc));
    }
  }
}


//
// MapObj::SetAnimationSpeed
//
// Set the speed of the current animation
//
void MapObj::SetAnimationSpeed(F32 speed)
{
  Mesh().fps = speed;
}


//
// MapObj::SetTreadSpeed
//
// Set the speed of the treads in meters per second
//
void MapObj::SetTreadSpeed( F32 speedL, F32 speedR)
{
  speedR;

  const MeshRoot & root = Mesh().Root();

  if (root.hasTread)            // FIXME: hacked to match the art assets
  {
    NodeIdent ident( "tread");
    if (root.FindIdent( ident))
    {
      Mesh().SetTreadRate( ident, speedL);
    }
    ident = "treads";
    if (root.FindIdent( ident))
    {
      Mesh().SetTreadRate( ident, speedL);
    }
    ident = "tread2";
    if (root.FindIdent( ident))
    {
      Mesh().SetTreadRate( ident, speedL);
//      Mesh().SetTreadRate( ident, speedR);
    }
    ident = "treads2";
    if (root.FindIdent( ident))
    {
      Mesh().SetTreadRate( ident, speedL);
    }
  }
}

//
// MapObj::ProcessAnimation
//
// Process the animation
//
void MapObj::ProcessAnimation()
{
  if (animList)
  {
    SYNC_BRUTAL("Animation")

    if (animationFX)
    {
      F32 from;
      F32 to;

      GetAnimationFrame(from, to);

      // Go through the Animation FX list and 
      // check to see if any FX are to be generated
      for (MapObjType::AnimationFXList::Iterator a(animationFX); *a; a++)
      {
        MapObjType::AnimationFX *animFX = *a;

        if (animFX->animFrame > from && animFX->animFrame <= to)
        {
          StartFX(animFX->typeCrc, NULL, TRUE);
        }
      }
    }

    // Has the animation reached its end point ?
    if (Mesh().AnimCurFrame() == animList->endFrame)
    {
      SYNC_BRUTAL("Finished")

      // Don't mess with this, the event handlers may change animList
      U32 crc = animList->name.crc;
      animList = NULL;

      // Notify the task that the animation is completed
      SendEvent(Task::Event(MapObjNotify::AnimationDone, this, crc));

      // Notify the parent if there is one (for animating upgrades)
      if (parent.Alive())
      {
        parent->SendEvent(Task::Event(MapObjNotify::AnimationDoneChild, this, crc));
      }
    }
  }
}


//
// MapObj::GetAnimationFrame
//
Bool MapObj::GetAnimationFrame(F32 &from, F32 &to)
{
  if (animList)
  {
    to   = Mesh().AnimCurFrame();
    from = Mesh().AnimLastFrame();

    return (TRUE);
  }
  else
  {
    return (FALSE);
  }
}


//
// MapObj::Collided
//
// This map object has collided with something
//
void MapObj::Collided(MapObj *with, const Vector *veloc)
{
  if (MapType()->mapCollideProc)
  {
    MapType()->ProcessCollision(*this, with, veloc);
  }
}


//
// GetMeshPosition
//
// Get the world position of a mesh object (ignores animation)
//
Bool MapObj::GetMeshPosition(const char *name, Vector &pos)
{
  Matrix offset, temp;
  offset.ClearData();
  
  MeshRoot * root = (MeshRoot *) &Mesh().Root();
  if (root->FindOffsetLocal(name, offset))
  {
    WorldMatrix().Transform(pos, offset.posit);

#if 0
    FamilyNode *n = Mesh().Find(name);
    pos = n->WorldMatrix().posit;
    LOG_DIAG(("Instance : %f,%f,%f", pos.x, pos.y, pos.z));
    LOG_DIAG(("Offset   : %f,%f,%f", pos.x, pos.y, pos.z));

    n = Mesh().Root().Find(name);
    LOG_DIAG(("root : %f,%f,%f", n->ObjectMatrix().right.x, n->ObjectMatrix().right.y, n->ObjectMatrix().right.z));
    LOG_DIAG(("root : %f,%f,%f", n->ObjectMatrix().up.x, n->ObjectMatrix().up.y, n->ObjectMatrix().up.z));
    LOG_DIAG(("root : %f,%f,%f", n->ObjectMatrix().front.x, n->ObjectMatrix().front.y, n->ObjectMatrix().front.z));
    LOG_DIAG(("root : %f,%f,%f", n->ObjectMatrix().posit.x, n->ObjectMatrix().posit.y, n->ObjectMatrix().posit.z));

    n = Mesh().Find(name);
    LOG_DIAG(("inst : %f,%f,%f", n->ObjectMatrix().right.x, n->ObjectMatrix().right.y, n->ObjectMatrix().right.z));
    LOG_DIAG(("inst : %f,%f,%f", n->ObjectMatrix().up.x, n->ObjectMatrix().up.y, n->ObjectMatrix().up.z));
    LOG_DIAG(("inst : %f,%f,%f", n->ObjectMatrix().front.x, n->ObjectMatrix().front.y, n->ObjectMatrix().front.z));
    LOG_DIAG(("inst : %f,%f,%f", n->ObjectMatrix().posit.x, n->ObjectMatrix().posit.y, n->ObjectMatrix().posit.z));
#endif
    return (TRUE);
  } 

  return (FALSE);
}



//
// RecurseFootInstance
//
// Recurse until a footprinted parent is found, or return NULL
//
FootPrint::Instance * MapObj::RecurseFootInstance()
{
  // Do we have a footprint
  if (GetFootInstance())
  {
    return (GetFootInstance());
  }

  // Ask parent for footprint
  if (parent.Alive())
  {
    return (parent->RecurseFootInstance());
  }

  return (NULL);
}


//
// MapObj::SmokeCallBack
//
// Smoke Callback
//
Bool MapObj::SmokeCallBack(MapObj *, FX::CallBackData &, void *)
{
  // Just keep blowin the smoke
  return (FALSE);
}


//
// MapObj::HighHealthCallBack
//
// High Health Callback
//
Bool MapObj::HighHealthCallBack(MapObj *mapObj, FX::CallBackData &, void *)
{
  // If out health has fallen below the high 
  // mark then stop producing high health mark FX
  return (mapObj->hitpoints < mapObj->MapType()->healthHighMark ? TRUE : FALSE);
}


//
// MapObj::MediumHealthCallBack
//
// Medium Health Callback
//
Bool MapObj::MediumHealthCallBack(MapObj *mapObj, FX::CallBackData &, void *)
{
  // If out health has fallen below the low mark or above
  // the high mark then stop producing medium health mark FX
  return (mapObj->hitpoints >= mapObj->MapType()->healthHighMark ||
          mapObj->hitpoints <= mapObj->MapType()->healthLowMark ? TRUE : FALSE);
}


//
// MapObj::LowHealthCallBack
//
// Low Health Callback
//
Bool MapObj::LowHealthCallBack(MapObj *mapObj, FX::CallBackData &, void *)
{
  // If out health has risen above the low 
  // mark then stop producing low health FX
  return (mapObj->hitpoints > mapObj->MapType()->healthLowMark ? TRUE : FALSE);
}


//
// MapObj::RegenArmourCallBack
//
// Regen Armour Callback
//
Bool MapObj::RegenArmourCallBack(MapObj *mapObj, FX::CallBackData &, void *)
{
  // If the armour has risen to max or is now zero
  return (((mapObj->armour == mapObj->MapType()->armour) || !mapObj->armour) ? TRUE : FALSE);
}