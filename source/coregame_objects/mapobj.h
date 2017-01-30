///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//

#ifndef __MAPOBJ_H
#define __MAPOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_decl.h"

#include "gameobj.h"
#include "armourclass.h"
#include "movetable.h"
#include "mapobjdec.h"
#include "unitobjdec.h"
#include "fx_type.h"
#include "gameconstants.h"
#include "meshconfig.h"
#include "instance_modifier.h"
#include "explosionobjdec.h"
#include "meshent.h"
#include "movement.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
struct MapCluster;
class MeshEnt;
class FamilyNode;
class AnimList;
class Team;
class Bitmap;
namespace FootPrint
{
  class Type;
  class Instance;
}


/////////////////////////////////////////////////////////////////////////////
//
// Namespace MapObjNotify - Notification CRC's
//
namespace MapObjNotify
{
  // Object is dying
  const U32 Dying = 0x0E6914CD; // "MapObj::Dying"

  // Animation has completed
  const U32 AnimationDone = 0xADE97BE4; // "MapObj::AnimationDone"

  // Animation on a child has completed
  const U32 AnimationDoneChild = 0x9363174B; // "MapObj::AnimationDoneChild"
}


///////////////////////////////////////////////////////////////////////////////
//
// Class MapObjType - Parent class for all on-map objects
//

class MapObjType : public GameObjType
{
  PROMOTE_LINK(MapObjType, GameObjType, 0x4542B0F8); // "MapObjType"

protected:

  // A single object attachment
  struct Attachment;

  // Animation FX which contains an FX type and information about when to trigger it
  struct AnimationFX;

  // A list of animation FX
  struct AnimationFXList;

  // Mesh configuration
  MeshConfig meshConfig;

  // Footprint or NULL
  FootPrint::Type *footPrintType;

  // Category of this type
  GameIdent categoryId;

  // List of objects to attach to this object
  List<Attachment> attachments;

  // List of lights to attach to this object
  List<Vid::Light::Desc> lights;

  // Armour Class & the Corresponding ID
  GameIdent armourClassName;
  S32 armourClass;

  // Maximum armour of an object
  S32 armour;
  F32 armourInv;

  // Has armour initially
  Bool armourInitial;

  // Number of cycles between regen
  U16 armourRegenInterval;
  
  // Maximum hitpoints of an object
  S32 hitpoints;
  F32 hitpointsInv;

  // Auto repair mark
  S32 autoRepairHitpoints;

  // Size of object in grains
  U32 grainSize;

  // Mass of the object in kg
  F32 mass;
  F32 massInv;

  // Surface area of the object in m2
  F32 surface;
  F32 surfaceInv;

  // Traction type of object
  GameIdent tractionType[Claim::LAYER_COUNT];
  U8 tractionIndex[Claim::LAYER_COUNT];

  // Explosion which occurs when this object dies
  ExplosionObjTypePtr deathExplosion;

  // Explosion which occurs when this object self destructs
  ExplosionObjTypePtr selfDestructExplosion;

  // Bit flags
  U32 isProjectile         : 1, // Object is a projectile
      isExplosion          : 1, // Object is an explosion
      detachableConfig     : 1, // Detachable setting has been set in configuration
      isDetachable         : 1, // Detached when its parent dies
      hasShadow            : 1, // Object has a shadow
      hasShadowConfig      : 1, // Shadow setting has been set in configuration
      showSeen             : 1, // Always show object once terrain is seen
      showSeenConfig       : 1, // ShowSeen flag set in configuration
      showAlways           : 1, // Always show this object under fog/shroud
      applyTerrainHealth   : 1, // Should terrain modifiers be applied to this type
      hasTextureAnimation  : 1, // Does this object have constant texture animation
      resourcesInitialized : 1; // Have resources been initialized

  // Does it have constant uv animation, whats the rate?
  F32 uvAnimRate;               

  // At what distance should near plane translucency start
  F32 nearFadeFactor;

  // Ray test flags
  U32 rayTestFlags;

  // FX which are generated when this object is at a particular animation frame
  BinTree<AnimationFXList> animationFX;

  // High health mark
  S32 healthHighMark;

  // Low heatlh mark
  S32 healthLowMark;

  // Configuration for display of object
  FScope *displayConfig;

  // Tree of FX type identifiers keyed on code triggers
  BinTree<GameIdent> genericFX;

  // Movement model name
  GameIdent movementIdent;

  // Movement model definition
  Movement::Model *movementModel;

  // The number of idle animations in this type
  S32 idleAnimationCount;

  // Pointer to a physics simulation function
  void (MapObjType::*mapPhysicsProc)(MapObj &, Matrix &);

  // Pointer to a collision function
  void (MapObjType::*mapCollideProc)(MapObj &, MapObj *, const Vector *);

protected:

  // Basic rigid body physics
  void RigidBodyPhysics(MapObj &obj, Matrix &m);

  // Basic rigid body collision
  void RigidBodyCollision(MapObj &, MapObj *, const Vector *);

  // Fatal collision function
  void FatalCollision(MapObj &, MapObj *, const Vector *);

  // Simulation function
  void ProcessMapPhysics(MapObj &obj, Matrix &m)
  {
    (this->*mapPhysicsProc)(obj, m);
  }

  // Collision function
  void ProcessCollision(MapObj &obj, MapObj *with, const Vector *veloc)
  {
    (this->*mapCollideProc)(obj, with, veloc);
  }

public:

  // Constructor
  MapObjType(const char *name, FScope *fScope);

  // ~Destructor
  ~MapObjType();

  // Called after all types are loaded
  void PostLoad();

  // Returns the mesh root for this type
  MeshRoot * GetMeshRoot();

  // Initialized type specific resources
  virtual Bool InitializeResources();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id) = 0;

  // Get the footprint type, or NULL if none
  FootPrint::Type * GetFootPrintType();

  // Returns the FX type for the given key, or NULL
  FX::Type * FindGenericFX(U32 key);

  // Generate a generic effect on the given object
  FX::Object * StartGenericFX(MapObj *obj, U32 key, FX::FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 lifeTime = 0.0f);

  // Are instances of this type on the movement list by default
  virtual Bool DefaultPrimitiveProcessing()
  {
    return (FALSE);
  }

  // Returns the category of this type
  const char* Category()
  {
    return (categoryId.str);
  }

  // Returns the CRC of the category of this type
  U32 CategoryCrc()
  {
    return (categoryId.crc);
  }

  // Returns the traction type
  const char *GetTractionType(Claim::LayerId layer)
  {
    return (tractionType[layer].str);
  }

  // Returns the traction index for a layer
  U8 GetTractionIndex(Claim::LayerId layer)
  {
    return (tractionIndex[layer]);
  }

  // Return the default claim layer of this type
  Claim::LayerId GetDefaultLayer()
  {
    ASSERT(movementModel)
    return (movementModel->defaultLayer);
  }

  // Returns the mass of this type
  F32 GetMass()
  {
    return (mass);
  }

  // Set mass
  void SetMass(F32 f)
  {
    mass = f;
    massInv = (mass > F32_EPSILON) ? (1.0F / mass) : 0.0F;
  }    

  // Returns the inverse of the mass
  F32 GetMassInv()
  {
    return (massInv);
  }

  // Returns the surface area of this type
  F32 GetSurface()
  {
    ASSERT(resourcesInitialized)
    return (surface);
  }

  // Returns the inverse of the surface area
  F32 GetSurfaceInv()
  {
    ASSERT(resourcesInitialized)
    return (surfaceInv);
  }

  // Has armour initially
  Bool GetArmourInitial()
  {
    return (armourInitial);
  }

  // Returns armour class
  S32 GetArmourClass()
  {
    return (armourClass);
  }

  // Returns the maximum hitpoints of the type
  S32 GetHitPoints()
  {
    return (hitpoints);
  }

  // Returns the inverse of the maximum hitpoints
  F32 GetHitPointsInv()
  {
    return (hitpointsInv);
  }

  // Object size in grains
  U32 GetGrainSize()
  {
    ASSERT(resourcesInitialized)
    return (grainSize);
  }

  // Returns the maximum armour of the type
  S32 GetArmour()
  {
    return (armour);
  }

  // Returns the inverse of the maximum armour
  F32 GetArmourInv()
  {
    return (armourInv);
  }

  // Returns the ray test flags for this type
  U32 GetRayTestFlags()
  {
    return (rayTestFlags);
  }

  // Is this object detachable ?
  Bool IsDetachable()
  {
    return (isDetachable);
  }

  // Does the object have a shadow ?
  Bool HasShadow()
  {
    return (hasShadow);
  }

  // Always show object once terrain has been seen
  Bool ShowSeen()
  {
    return (showSeen);
  }

  // Always show this object under fog/shroud
  Bool ShowAlways()
  {
    return (showAlways);
  }

  // At what distance should near plane translucency start
  F32 GetNearFadeFactor()
  {
    return (nearFadeFactor);
  }

  // Should terrain health modifiers be applied to this type
  Bool ApplyTerrainHealth()
  {
    return (applyTerrainHealth);
  }

  // Does this object have constant texture animation
  Bool HasTextureAnimation()
  {
    return (hasTextureAnimation);
  }

  // Is this a null object
  Bool IsNullObj()
  {
    ASSERT(resourcesInitialized)
    return (meshConfig.isNullMesh);
  }

  // Is this object a projectile
  Bool IsProjectile()
  {
    return (isProjectile);
  }

  // Is the object an explosion
  Bool IsExplosion()
  {
    return (isExplosion);
  }

  // Configuration for display of object
  FScope *GetDisplayConfig()
  {
    return (displayConfig);
  }

  // FindFloor
  F32 FindFloor(F32 x, F32 z, Vector *normal = NULL)
  {
    ASSERT(movementModel)
    return (movementModel->findFloor(x, z, normal));
  }

  // Return the movement model
  Movement::Model &GetMovementModel()
  {
    ASSERT(movementModel)
    return (*movementModel);
  }

  // Returns the number of idle animations on this type
  S32 GetIdleAnimationCount()
  {
    return (idleAnimationCount);
  }

  // Get the self destruct explosion
  ExplosionObjType * GetSelfDestructExplosion()
  {
    return (selfDestructExplosion.GetPointer());
  }

  //
  // Friends of MapObjType
  //
  friend MapObj;
};


///////////////////////////////////////////////////////////////////////////////
//
// Class MapObj - Instance class for above type
//

class MapObj : public GameObj
{
protected:

  // Time when the object was created
  F32 birthTime;

  // Current animation list
  AnimList *animList;

  // Current animation FX set to use
  MapObjType::AnimationFXList *animationFX;

  // Does this object currently require movement processing
  U32 requestPrimitive : 1,

      // Has there been a negative modify hitpoints yet
      negativeModifyHitPoints : 1,

      // Is this object in the process of dying?
      dying : 1;

  // The physical representation of this map object
  MeshEnt *meshEnt;
  FamilyState * meshState0;
  AnimKey * meshState1;

  // Pointer to parent object (attatched to)
  MapObjPtr parent;

  // Name of position to attach to
  NodeIdent attachPointIdent;
  
  // Velocity
  Vector velocity;

  // Speed (m/s)
  F32 speed;

  // Move table balance data for this object on the current cell
  MoveTable::BalanceData *balanceData;

  // Current layer
  Claim::LayerId currentLayer;

  // Footprint instance, or NULL
  FootPrint::Instance *footInstance;

  // Current Hitpoints
  S32 hitpoints;

  // Current Armour
  S32 armour;

  // Number of cycles to next regen
  U16 armourRegenCycles;
  
  // The direction of cluster overlap in the x- and z- axis
  //   0 is no overlap 
  //  -1 is overlap along negative axis
  //   1 is overlap along positive axis
  S8 clustOverlapX;
  S8 clustOverlapZ;

  // List of Attached Objects
  NList<MapObj> attachments;

  // attached lights
  NList<Vid::Light::Obj> lights;

public:

  // List nodes
  NList<MapObj>::Node offMapNode;
  NList<MapObj>::Node onMapNode;
  NList<MapObj>::Node primitiveNode;
  NList<MapObj>::Node attachedNode;
  NList<MapObj>::Node displayNode;
  NList<MapObj>::Node terrainHealthNode;

  // The 4 map clusters that this object may occupy
  NList<MapObj>::Node clustNode0;
  NList<MapObj>::Node clustNode1;
  NList<MapObj>::Node clustNode2;
  NList<MapObj>::Node clustNode3;

  // The cluster that the object is currently centred on
  MapCluster *currentCluster;

  // The current cluster that each cluserNode resides in
  MapCluster *clustList[4];

  // The x- and z- indexes of the cell this object is centred on
  S32 cellX;
  S32 cellZ;

  // Counter used by map iterators to prevent dupes slipping in
  U32 iterTicker;

private:

  void CreateLight( Vid::Light::Desc * light);

public:
  
  // Constructor and destructor
  MapObj(MapObjType *objType, U32 id);
  ~MapObj();

  // attached lights
  const NList<Vid::Light::Obj> & GetLights() const
  {
    return lights;
  }

  // Called when the object is marked for deletion
  void MarkedForDeletion();

  // Called to before deleting the object
  void PreDelete();

  // Move object to/from primitive processing list
  void SetPrimitiveProcessing(Bool request);

  // Per-cycle processing
  virtual void ProcessCycle();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Equip the object
  void Equip();

  // Called after all objects are loaded
  void PostLoad();

  // Send an event
  Bool SendEvent(const Task::Event &event, Bool idle = FALSE);

  // AddToMapHook
  virtual void AddToMapHook();

  // RemoveFromMapHook
  virtual void RemoveFromMapHook();

  // Capture All hooks
  void CaptureAllHooks(Bool capture);

  // Capture/Release map hooks when the object's centre point moves clusters
  virtual void CaptureMapHooks(Bool capture);

  // Capture/Release cell hooks when the object's centre point moves cells
  virtual void CaptureCellHooks(Bool capture);

  // Capture/Release cluster hooks when part of the bounding sphere of an object moves clusters
  void CaptureClusterHooks(Bool capture, Bool calcOverlap = TRUE);

  // Apply/Remove an optional footprint
  void ToggleFootPrint(Bool toggle);

  // Determine if this object overlaps any clusters, return results in x and z
  void TestClusterOverlap(S8 &x, S8 &z);

  // Make this object self destruct, optionally triggering special explosion
  void SelfDestruct(Bool explosion = FALSE, Team *modifier = NULL);

  // Dump information about the object
  ostream & Info(ostream &o);

  // Has Animation
  Bool HasAnimation(U32 nameCrc);

  // Set Animation
  void SetAnimation(const char *name, Bool blend = TRUE, Bool activate = TRUE);

  // Set Animation
  void SetAnimation(U32 nameCrc, Bool blend = TRUE, Bool activate = TRUE);

  Bool IsAnimCompleted()
  {
    return Mesh().IsAnimCompleted();
  }

  // Change Speed
  void SetAnimationSpeed(F32 speed);
  void SetTreadSpeed( F32 speedL, F32 speedR);

  // Process Animation
  void ProcessAnimation();

  // Get the animation frames
  Bool GetAnimationFrame(F32 &from, F32 &to);

  // This object has collided with another object
  virtual void Collided(MapObj *with, const Vector *veloc);

  // Get the world position of a mesh object (ignores animation)
  Bool GetMeshPosition(const char *name, Vector &pos);

  // Recurse until a footprinted parent is found, or return NULL
  FootPrint::Instance * RecurseFootInstance();

  // Modify the number of hitpoints an object has
  virtual void ModifyHitPoints(S32 mod, UnitObj *sourceUnit = NULL, Team *sourceTeam = NULL, const Vector *direction = NULL);

  // Modify the armour an object has
  virtual void ModifyArmour(S32 mod);

  // Starts the given generic FX (returns NULL if not created)
  FX::Object * StartGenericFX(U32 key, FX::FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 lifeTime = 0.0f);

  // Starts the given FX type (returns NULL if not created)
  FX::Object * StartFX(U32 typeCrc, FX::FXCallBack callBack = NULL, Bool process = FALSE, const Vector *velocity = NULL, void *context = NULL, F32 lifeTime = 0.0f);

  // Get the color and percentage for the health of the unit
  void GetHealthInfo(Color &color, F32 &pct);

  // Is this object seen/visible to the given team
  void GetSeenVisible(Team *team, Bool &seen, Bool &visible);

  // Has this object ever been seen by the given team
  Bool GetSeen(Team *team);

  // Is this object visible to the given team
  Bool GetVisible(Team *team);

public:

  // FX Callbacks

  // Smoke Callback
  static Bool SmokeCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  // High Health Callback
  static Bool HighHealthCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  // Medium Health Callback
  static Bool MediumHealthCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  // Low Health Callback
  static Bool LowHealthCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

  // Regen Armour Callback
  static Bool RegenArmourCallBack(MapObj *mapObj, FX::CallBackData &cbd, void *context);

public:

  // Has there been a negative mod yet
  Bool GetNegativeModifyHitPoints()
  {
    return (negativeModifyHitPoints);
  }

  // Get birth time of this object
  F32 GetBirthTime()
  {
    return (birthTime);
  }

  // GetCellX
  U32 GetCellX()
  {
    return cellX;
  }

  // GetCellZ
  U32 GetCellZ()
  {
    return cellZ;
  }

  // Set the velocity of this object
  void SetVelocity(const Vector &v) 
  {
    velocity = v;
  }

  // Set the speed of this object
  void SetSpeed(F32 f)
  {
    speed = f;
  }

  // Velocity
  const Vector &GetVelocity()
  {
    return (velocity);
  }

  // Speed
  F32 GetSpeed()
  {
    return (speed);
  }

  // Return a pointer to the balance data
  const MoveTable::BalanceData &GetBalanceData()
  {
    ASSERT(balanceData)
    return (*balanceData);
  }

  // Set the current layer
  void SetCurrentLayer(Claim::LayerId layer)
  {
    currentLayer = layer;
  }

  // Return current layer
  Claim::LayerId GetCurrentLayer()
  {
    return (currentLayer);
  }

  // Returns the current number of HitPoints an object has
  S32 GetHitPoints()
  {
    return (hitpoints);
  }

  // Returns the HitPoints as a percentage of the maximum
  F32 GetHitPointPercentage()
  {
    return (F32(GetHitPoints()) * MapType()->GetHitPointsInv());
  }

  // Returns the current armour an object has
  S32 GetArmour()
  {
    return (armour);
  }

  // Compute the defense this unit has against the specified weapon
  U32 Defense(U32 weaponId)
  {
    MapObjType *type = MapType();
    return ((type->hitpoints + type->armour) * ArmourClass::Lookup(weaponId, type->armourClass) >> 16);
  }

  // Returns TRUE if object is on the map
  Bool OnMap() const
  {
    return (onMapNode.InUse());
  }

  // Returns TRUE if object is on the primitive list
  Bool OnPrimitiveList()
  {
    return (primitiveNode.InUse());
  }

  // Does this object currently require primitive processing
  Bool RequestingPrimitive()
  {
    return (requestPrimitive);
  }

  // Get pointer to type
  MapObjType* MapType()
  {
    // This is a safe cast
    return ((MapObjType*)type);
  };

  // Get the footprint instance, or NULL
  FootPrint::Instance * GetFootInstance()
  {
    return (footInstance);
  }

  // Set the footprint instance
  void SetFootInstance(FootPrint::Instance *instance)
  {
    footInstance = instance;
  }

  // Get pointer to parent (if it's alive)
  MapObj * GetParent()
  {
    return (parent.GetPointer());
  }

  // Get the list of attached objects
  const NList<MapObj> & GetAttachments()
  {
    return (attachments);
  }

  // Is the object dying (e.g in death task)
  Bool Dying()
  {
    return (dying);
  }

  // Test the position of the object and ensure it fits within the world
  void UpdateMapPos();
  Bool CheckWorldPosition(Vector &v);

  // Swap mesh for morphing units
  void SetMesh(MeshEnt *mesh);

  // Get object instance's mesh root
  const MeshRoot *GetMeshRoot() const
  {
    ASSERT(meshEnt)
    return &(meshEnt->Root());
  }

  // MeshEnt overloads
  //
  MeshEnt & Mesh() const
  {
    ASSERT(meshEnt)
    return *meshEnt;
  };

  //  fog of war
  //
  void SetFogTarget( U32 fog, U32 alpha, Bool immedidate = FALSE);

  // Attach this object to a parent
  virtual void Attach(MapObj *parent, FamilyNode &node);
  void Attach(MapObj *parent = NULL, NodeIdent *ident = NULL);

  inline MeshObj * FindIdent( NodeIdent &ident) const
  {
    return meshEnt->FindIdent( ident);
  }
  inline MeshObj * FindMesh( const char *pointName) const
  {
    return meshEnt->FindMeshObj( pointName);
  }
  inline MeshObj * GetIdent(NodeIdent &nodeIdent) const
  {
    ASSERT( nodeIdent.Valid());

    return meshEnt->Get( nodeIdent);
  }

  // Detach this object from its parent
  virtual void Detach();

  // world spatial data
  //
  inline const Matrix & WorldMatrix() const
  {
    return meshState0->WorldMatrix();
  }
  inline const Vector & Position() const
  {
    return meshState0->WorldMatrix().Position();
  }
  inline const Vector & Origin() const
  {
    return meshEnt->Origin();
  }
  inline const Vector & RootOrigin() const
  {
    return meshEnt->RootOrigin();
  }

  // child world spatial data
  //
  inline const Matrix & WorldMatrix( const NodeIdent & nodeIdent) const
  {
    ASSERT( nodeIdent.Valid());

    return meshEnt->WorldMatrixChild( nodeIdent.index);
  }
  inline const Vector & Position( const NodeIdent & nodeIdent) const
  {
    ASSERT( nodeIdent.Valid());

    return meshEnt->WorldMatrixChild( nodeIdent.index).Position();
  }


  // display (interpolated) world spatial data
  //
  inline const Matrix & WorldMatrixRender() const
  {
    return meshEnt->WorldMatrixRender();
  }
  inline const Vector & PositionRender() const
  {
    return WorldMatrixRender().Position();
  }
  inline const Vector & OriginRender() const
  {
    return meshEnt->OriginRender();
  }
  inline const Vector & RootOriginRender() const
  {
    return meshEnt->RootOriginRender();
  }

  // object spatial data
  //
  inline const Matrix & ObjectMatrix() const
  {
    return meshState0->ObjectMatrix();
  }
  inline const Quaternion & Rotation() const
  {
    return meshState0->GetRotation();
  }
  inline const Vector & Scale() const
  {
    return meshState0->GetScale();
  }
  inline const Bounds & ObjectBounds() const
  {
    return meshEnt->ObjectBounds();
  }
  inline const Bounds & RootBounds() const
  {
    return meshEnt->RootBounds();
  }

  // child object spatial data 
  //
  inline const Matrix & ObjectMatrix( const NodeIdent & nodeIdent) const
  {
    ASSERT( nodeIdent.Valid());

    return meshEnt->GetSimObjectMatrix( nodeIdent.index);
  }

  // display (interpolated) object spatial data
  //
  inline const Matrix & ObjectMatrixRender() const
  {
    return meshEnt->GetIntObjectMatrix();
  }
  inline const Quaternion & RotationRender() const
  {
    return meshEnt->GetIntRotation();
  }

  // child object spatial data 
  //
  inline const Matrix & ObjectMatrixRender( const NodeIdent & node) const
  {
    return meshEnt->GetIntObjectMatrix( node.index);
  }
  inline const Quaternion & RotationRender( const NodeIdent & node) const
  {
    return meshEnt->GetIntRotation( node.index);
  }

  // get target
  //
  inline const Vector & TargetPosition() const
  {
    return meshState1->GetPosition();
  }
  inline const Quaternion & TargetRotation() const
  {
    return meshState1->GetRotation();
  }

  // get target child
  //
  inline const Vector & TargetPosition( const NodeIdent & node) const
  {
    return meshEnt->GetTargetPosition( node);
  }
  inline const Quaternion & TargetRotation( const NodeIdent & node) const
  {
    return meshEnt->GetTargetRotation( node);
  }

  // set target
  //
  inline void SetSimTarget( const Quaternion & quaternion)
  {
    meshEnt->SetSimTarget( quaternion);
  }
  inline void SetSimTarget( const Vector & position)
  {
    meshEnt->SetSimTarget( position);
  }
  inline void SetSimTarget( const Quaternion & quaternion, const Vector & position)
  {
    meshEnt->SetSimTarget( quaternion, position);
  }
  inline void SetSimTarget( const Matrix & matrix)
  {
    meshEnt->SetSimTarget( matrix);
  }

  // set target child
  //
  inline void SetSimTarget( const NodeIdent & ident, const Quaternion & quaternion)
  {
    meshEnt->SetSimTarget( ident, quaternion);
  }
  inline void SetSimTarget( const NodeIdent & ident, const Vector & position)
  {
    meshEnt->SetSimTarget( ident, position);
  }
  inline void SetSimTarget( const NodeIdent & ident, const Quaternion & quaternion, const Vector & position)
  {
    meshEnt->SetSimTarget( ident, quaternion, position);
  }
  inline void SetSimTarget( const NodeIdent & ident, const Matrix & matrix)
  {
    meshEnt->SetSimTarget( ident, matrix);
  }

  // Set the current and next simulation frame's rotation, position, and scale
  //
  inline void SetSimCurrent( const Quaternion & quaternion)
  {
    meshEnt->SetSimCurrent( quaternion);
  }
  inline void SetSimCurrent( const Vector & position)
  {
    meshEnt->SetSimCurrent( position);
  }
  inline void SetSimCurrent( const Quaternion & quaternion, const Vector & position)
  {
    meshEnt->SetSimCurrent( quaternion, position);
  }
  inline void SetSimCurrent( const Matrix & matrix)
  {
    meshEnt->SetSimCurrent( matrix);
  }

  void UpdateInt(F32 dt)
  {
    meshEnt->SimulateInt( dt);
  }

  void UpdateIntBasic(F32 dt, Bool simFrame = TRUE)
  {
    meshEnt->SimulateIntBasic( dt, simFrame);
  }

  // return was dirty
  //
  Bool UpdateSim(F32 dt)
  {
    return meshEnt->UpdateSim( dt);
  }

  void PollActivateTexAnim()
  {
    meshEnt->PollActivateTexAnim();
  }


  //
  // Debugging
  //
  #ifdef DEVELOPMENT

  virtual void RenderDebug() {}
  
  #endif
};

#endif