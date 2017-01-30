///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// meshent.h
//
// 17-SEP-1998
//

#ifndef __MESHENT_H
#define __MESHENT_H

#include "mesh.h"
#include "mesheffect.h"
//----------------------------------------------------------------------------

#define ANIMFPS               20.0f
#define DEFBLENDTIME          10.0f
//----------------------------------------------------------------------------

class MeshObj : public FamilyNode
{
protected:
  friend Mesh::Manager;
  friend class MeshEnt;

public:

  Mesh                   *mesh;              // the geometry
  U32                     clipFlagCache;     // cashed clip flags set by Camera::BoundsTest( MeshEnt &)

protected:
  int GetMeshArray( MeshObj **array, int MaxSize, int Count);

  Bool SetupFamily( const Mesh * _mesh, Array<FamilyState> & stateArray);

public:
  void ClearData();

  MeshObj( FamilyState *state = NULL)
  {
    ClearData();
    if (state)
    {
      SetState( *state);
    }
  }
  ~MeshObj();

  int GetMeshArray (MeshObj **array, int MaxSize)
  {
	  return GetMeshArray (array, MaxSize, 0);
  }
};
//----------------------------------------------------------------------------

// animation play data (need 2 sets; 1 for Sim, 1 for Interp)
//
struct AnimState
{
  U32                     active    : 1;        // is it currently animating
  U32                     blend     : 1;        // is it currently blending between animations

  F32                     curFrame, lastFrame;  // < 0: blending, > 0 & < maxFrame: normal anim
  F32                     dir;                  // 1 or -1 factor for fps

  F32                     conFrame, targetFrame;// control overlay
  F32                     conDir;

  Bounds                  bounds;               // object space
  Vector                  origin;               // world space
  Vector                  rootOrigin;           // non-changing origin; world space

  void ClearData()
  {
    active = blend = FALSE;

    curFrame = 0.0f;
    dir = 1.0f;
    conFrame = targetFrame = 0.0f;
    conDir = 1.0f;

    bounds.ClearData();
    origin.ClearData();
    rootOrigin.ClearData();
  }
};
//----------------------------------------------------------------------------

class MeshEnt : public MeshObj
{
protected:
  friend Mesh::Manager;
  friend Mesh;
  friend MeshObj;

  NList<MeshEnt>::Node    listNode;               // node for Mesh manager list

  MeshEnt                 *eParent;               // parent/child MeshEnt's
  NList<MeshEnt>          eChildren;
  NList<MeshEnt>::Node    eChildNode;

//  Array<BucketLock>       buckys;                 // instance buckys
  Array<FaceGroup>        buckys;

  typedef void (MeshEnt::*ENTRENDERPROCPTR)();    // render function
  ENTRENDERPROCPTR        renderProc;

  friend class MeshEffect;                        //  mesh effects

  MeshEffect *            effect;
  ENTRENDERPROCPTR        renderProcSave;         // save real render func during effect

  MeshRoot & RootPriv() const
  {
    return *((MeshRoot *) mesh);
  }

public:                               

  ShadowInfo              shadowInfo;
  Bitmap *                shadowTexture;
  U32                     shadowTime;
  S32                     shadowAlpha0;
  S32                     shadowAlpha1;

  S32                     fogCurrent;
  S32                     alphaCurrent;
  S32                     extraFog;

  Array<FamilyState>      states0, statesR;   // sim current and render current state arrays
  Array<AnimKey>          states1, blends;    // sim target and anim temp blend key arrays (no matrices)

  struct TreadState
  {
    F32 offset;
    F32 rate;
  };
  Array<TreadState>       treads;             // tread offsets per state

  AnimState               animState0, animStateR;
  F32                     interpFrame;        // for root interpolation

  ENTRENDERPROCPTR        renderProcEffect;
  MeshRoot::ROOTRENDERPROCPTR  rootRenderProc;       // handy pointers


  Color                   color;       // general effects data
  Color                   baseColor;
  Color                   teamColor;
  Bitmap *                texture;

  U32                     controlFlags;
  F32                     lodValue;

  U32                     shadowType   : 2;
  U32                     noLight      : 1;   // emissive lighting?
  U32                     envMap       : 1;   // does it have specularity?
  U32                     hasAnim      : 1;   // does it have animations?
  U32                     hasTexAnim   : 1;   // does it have animating textures?
  U32                     hasTread     : 1;   // does it have treads
  U32                     hasControl   : 1;   // does it have control nodes
  U32                     textureAnim  : 1;   // is it texture animating?
  U32                     dirtyRoot    : 1;   // are the 1st and 2nd generation world matrices dirty
  U32                     dirtyAll     : 1;   // are the other world matrices dirty
  U32                     dirtyIntRoot : 1;   
  U32                     dirtyIntAll  : 1;   
  U32                     dirtyShadow  : 1;   
  U32                     interpolated : 1;   
  U32                     effecting    : 1;   
  U32                     visible      : 1;   

  AnimList *              curCycle;           // current animation cycle
  AnimList *              conCycle;           // control cycle overlay
  U32                     curCycleID;         // current cycle id number
  Vector                  viewOrigin;         // last camera transformed sphere origin

  F32                     fps;                // current anim frame rate
  F32                     texTimer, texTime;  // texture animation

  U32                     faceCount;          // current mrm facecount
  U32                     vertCount;          // current mrm vertcount
  U32                     nextVertCount;      // requested mrm vertcount

  U32                     texAnimPoll, lastTexPoll, texAnimAuto;

  U32                     clipFlagSave;

  void ClipSave()
  {
    clipFlagSave = clipFlagCache;
  }
  void ClipRestore()
  {
    clipFlagCache = clipFlagSave;
  }

protected:
  void ClearData();

  Bool SetupStates( const MeshRoot &root);

  void ClampFrame( AnimState & animState);

  // inserts 'node' as 'this's' primary/last child depending on 'insert' flag
  virtual void Attach( FamilyNode &node, Bool insert = TRUE);

  void SetFeatureLOD();
  U32  MrmUpdate();

  void SetupBuckys( const MeshRoot & root);
  void ReleaseBuckys();

public:

  void SetBucketDataZ( F32 z = 1);

  void Reset( const MeshRoot & root);

  void SetFogTarget( S32 fog, S32 alpha = 255, Bool immediate = FALSE);

  void SaveState( FScope * fScope);
  void LoadState( FScope * fScope);

  U32 GetMem() const;

  //  meshviewer editing
  //
  struct SelectData
  {
    List<U16> verts;
    List<U16> faces;

    ~SelectData()
    {
      verts.DisposeAll();
      faces.DisposeAll();
    }
  };

  SelectData *            selData;

  // NULL = all
  //
  void SelectVerts( Area<S32> * rect = NULL, Bool append = FALSE, Bool toggle = FALSE);
  void SelectFaces( Area<S32> * rect = NULL, Bool append = FALSE, Bool toggle = FALSE);
  void UnSelectVerts( Area<S32> * rect = NULL);
  void UnSelectFaces( Area<S32> * rect = NULL);

  Bool SelectVert( U16 index, Bool append = FALSE);
  Bool SelectFace( U16 index, Bool append = FALSE);
  Bool UnSelectVert( U16 index);
  Bool UnSelectFace( U16 index);

  void RenderSelVerts( Color color0, Bool show = FALSE, Color color1 = 0x00000000)
  {
    if ((show || (selData && selData->verts.GetCount())) && clipFlagCache != clipOUTSIDE)
    {
      RootPriv().RenderSelVerts( show, selData ? &selData->verts : NULL, 
        statesR, color0, color1, clipFlagCache);
    }
  }
  void RenderSelFaces( Color color0, Bool show = FALSE, Color color1 = 0x00000000)
  {
    if ((show || (selData && selData->verts.GetCount())) && clipFlagCache != clipOUTSIDE)
    {
      RootPriv().RenderSelFaces( show, selData ? &selData->faces : NULL,
        statesR, color0, color1, clipFlagCache);
    }
  }

  // constructors
  //
  MeshEnt( const MeshRoot *root = NULL);
  MeshEnt( const char *name);

  virtual ~MeshEnt();

  void SetTeamColor( Color tColor);
  void SetBaseColor( Color bColor);
  void ModulateBaseColor( Color bColor);

  void SetOpaque( Color oColor);
  void SetOpaque( U32 o);
  void SetTranslucent( U32 t);

  const MeshEnt * EntParent() const
  {
    return eParent;
  }

  NList<MeshEnt> * EntChildren()
  {
    return &eChildren;
  }

  const MeshEnt * EntChild() const
  {
    return eChildren.GetHead();
  }

  void SetTreadRate( NodeIdent & ident, F32 rate);
  void SetUVAnimRate( F32 rate);

  // removes 'this' and all its children from parent
  virtual void Detach();

  void AttachAt( FamilyNode & at, FamilyNode & node, Bool insert = TRUE);

  inline MeshObj * Get( NodeIdent &ident) const
  {
    ASSERT( ident.index < statesR.count);

    return states0[ident.index].GetMeshObj();
  }

  inline MeshObj * FindIdent( NodeIdent &ident) const
  {
    return Root().FindIdent( ident) ? Get( ident) : NULL;
  }

  MeshEffect * GetEffect() const
  {
    return effect;
  }
  void GetVertWorld( Vector &vert, U32 index) const;
  void GetNormWorld( Vector &norm, U32 index) const;

  void Copy( const MeshEnt *orig, Bool local = FALSE);
  void Setup( const MeshRoot &root);

  const MeshRoot & Root() const
  {
    ASSERT(mesh)
    return *((MeshRoot *) mesh);
  }
  void MRMGen( Bool doSelVerts = TRUE);

  void Render( const Matrix &world);
  void RenderColor( const Matrix &world, Color _color);
  void RenderColor( Color _color);
  void RenderChildren();

  virtual void RenderSingle( Color teamColor = 0xffffffff, U32 _controlFlags = controlDEF);
  virtual void Render();

  void SetupRenderProc();
  void SetRenderProc( ENTRENDERPROCPTR proc)
  {
    renderProc = proc;
  }
  U32 BoundsTest();

  virtual void SetWorldRecurse( const Matrix & world);
  virtual void SetWorldRecurseRender( const Matrix & world, FamilyState * stateArray);

  // Set the current and next simulation frame's rotation, position, and scale
  //
  void SetSimCurrent( const Matrix & matrix);
  void SetSimCurrent( const Quaternion & quaternion);
  void SetSimCurrent( const Vector & position);
  void SetSimCurrent( const Quaternion & quaternion, const Vector & position);
  void SetSimCurrentScale( const Vector & scale);

  // Set the next simulation frame's rotation, position, and scale
  //
  void SetSimTarget( const Matrix & matrix);
  void SetSimTarget( const Quaternion & quaternion);
  void SetSimTarget( const Vector & position);
  void SetSimTarget( const Quaternion & quaternion, const Vector & position);
  void SetSimTargetScale( const Vector & scale);
  void SetTreeDirty( Bool rootDirty = TRUE);

  // Set the next simulation frame's rotation, position, and scale for a child node
  //
  void SetSimTarget( const NodeIdent & ident, const Matrix & matrix);
  void SetSimTarget( const NodeIdent & ident, const Quaternion & quaternion);
  void SetSimTarget( const NodeIdent & ident, const Vector & position);
  void SetSimTarget( const NodeIdent & ident, const Quaternion & quaternion, const Vector & position);
  void SetSimTargetScale( const NodeIdent & ident, const Vector & scale);

  void SetWorldRender();

  // get sim
  //
  const Matrix & GetSimObjectMatrix() const
  {
    return states0.data->ObjectMatrixPriv();
  }
  const Quaternion & GetSimRotation() const
  {
    return states0.data->GetRotation();
  }
  const Vector & GetSimPosition() const
  {
    return states0.data->GetPosition();
  }
  const Vector & GetSimScale() const
  {
    return states0.data->GetScale();
  }

  // get sim child
  //
  const Matrix & GetSimObjectMatrix( U32 index) const
  {
    return states0[index].ObjectMatrixPriv();
  }
  const Quaternion & GetSimRotation( U32 index) const
  {
    return states0[index].GetRotation();
  }
  const Vector & GetSimPosition( U32 index) const
  {
    return states0[index].GetPosition();
  }
  const Vector & GetSimScale( U32 index) const
  {
    return states0[index].GetScale();
  }

  // get interp
  //
  const Matrix & GetIntObjectMatrix() const
  {
    return statesR.data->ObjectMatrixPriv();
  }
  const Quaternion & GetIntRotation() const
  {
    return statesR.data->GetRotation();
  }
  const Vector & GetIntPosition() const
  {
    return statesR.data->GetPosition();
  }
  const Vector & GetIntScale() const
  {
    return statesR.data->GetScale();
  }

  // get interp child
  //
  const Matrix & GetIntObjectMatrix( U32 index) const
  {
    return statesR[index].ObjectMatrixPriv();
  }
  const Quaternion & GetIntRotation( U32 index) const
  {
    return statesR[index].GetRotation();
  }
  const Vector & GetIntPosition( U32 index) const
  {
    return statesR[index].GetPosition();
  }
  const Vector & GetIntScale( U32 index) const
  {
    return statesR[index].GetScale();
  }

  // get target
  //
  const Quaternion & GetTargetRotation() const
  {
    return states1.data->GetRotation();
  }
  const Vector & GetTargetPosition() const
  {
    return states1.data->GetPosition();
  }
  const Vector & GetTargetScale() const
  {
    return states1.data->GetScale();
  }

  // get target child
  //
  const Quaternion & GetTargetRotation( const NodeIdent & node) const
  {
    return states1[node.index].GetRotation();
  }
  const Vector & GetTargetPosition( const NodeIdent & node) const
  {
    return states1[node.index].GetPosition();
  }
  const Vector & GetTargetScale( const NodeIdent & node) const
  {
    return states1[node.index].GetScale();
  }

  // returns was dirty
  //
  Bool UpdateSim( F32 dt);

  void SimulateSim( F32 dt);
  void SimulateInt( F32 dt, Bool isInterpFrame = TRUE);
  void SimulateIntRecurse( F32 dt, F32 dtdi);
  void SimulateIntBasic( F32 dt, Bool simFrame = TRUE);
  void SimulateTex( F32 dt, Bool simFrame = TRUE);
  void SetTexFrame( U32 frame = 0);

  const Matrix & WorldMatrixChild( U32 index)
  {
    return states0[index].WorldMatrix();
  }
  const Vector & Origin()
  {
    return animState0.origin;
  }
  const Vector & Offset()
  {
    return animState0.bounds.Offset();
  }
  const Bounds & ObjectBounds()
  {
    return animState0.bounds;
  }

  const Matrix & WorldMatrixRender()
  {
    return statesR.data->WorldMatrix();
  }
  const Matrix & WorldMatrixChildRender( U32 index)
  {
    return statesR[index].WorldMatrix();
  }
  const Vector & OriginRender()
  {
    return animStateR.origin;
  }
  const Vector & OffsetRender()
  {
    return animStateR.bounds.Offset();
  }
  const Bounds & ObjectBoundsRender()
  {
    return animStateR.bounds;
  }

  const Vector & RootOrigin()
  {
    return animState0.rootOrigin;
  }
  const Vector & RootOriginRender()
  {
    return animStateR.rootOrigin;
  }
  const Bounds & RootBounds() const
  {
    return Root().fixedBounds;
  }

  virtual void Render( Array<FamilyState> & stateArray, Color tColor, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  virtual void RenderColor( Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);

  void RenderBounds( Color color, Bitmap *texture = NULL)
  {
    RenderBoundingBox( color, texture);
    RenderBoundingSphere( color, texture);
  }
  void RenderBoundingSphere( Color color, Bitmap *texture = NULL);
  void RenderBoundingBox( Color color, Bitmap *texture = NULL);

  void RenderHardPoints( Color color);
  void RenderOrigin( Color color, MeshObj * childMesh = NULL, Color color1 = 0xffffffff);

  void RenderNormals( Color color)
  {
    RootPriv().RenderNormals( statesR, color, clipFlagCache); 
  }

  void RenderShadowPlane( U32 number, Color color)
  {
    RootPriv().RenderShadowPlane( number, statesR, color, clipFlagCache); 
  }

  void RenderNone();
  void RenderAnimVtl();
  void RenderNoAnimVtl();
  void RenderNoLightAnimVtl();
  void RenderNoLightNoAnimVtl();
  void RenderQuickLightAnimVtl();
  void RenderQuickLightNoAnimVtl();
  void RenderOverlayVtl();
  void RenderEnvMapVtl( Color color = 0xffffffff, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  void RenderAnimV();
  void RenderNoAnimV();
  void RenderNoLightAnimV();
  void RenderNoLightNoAnimV();
  void RenderQuickLightAnimV();
  void RenderQuickLightNoAnimV();
  void RenderOverlayV();
  void RenderEnvMapV( Color color = 0xffffffff, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  // mesh effects generic renders
  //
  void RenderShadowTexture( const Matrix ** lightA = NULL, U32 lCount = 1, Color color = 0xffffffff, U32 blend = RS_BLEND_DEF);

  // mesh effects specialized renders
  //
  void RenderTextCrossFadeEffect();
  void RenderColorEffect();
  void RenderGlowEffect();
  void RenderPlaneEffect();
  void RenderPlaneBuildEffect();
  void RenderLiquidMetalEffect();

  // 2 = auto run
  void ActivateTexAnim( U32 active = TRUE);
  void PollActivateTexAnim();

  void SetAnimType( AnimType _type);
  void ActivateAnim( Bool activate = TRUE);
  Bool IsAnimCompleted();

  Bool SetAnimCycle( const char * cycleName)
  {
    return SetAnimCycle( Crc::CalcStr( cycleName)); 
  }
  Bool SetAnimCycle( U32 cycleID);
  Bool SetAnimCycle( AnimList & cycle);

  Bool BlendAnimCycle( const char * cycleName, F32 blendTime = DEFBLENDTIME)
  {
    return BlendAnimCycle( Crc::CalcStr( cycleName), blendTime); 
  }
  Bool BlendAnimCycle( U32 cycleID, F32 blendTime = DEFBLENDTIME);
  Bool BlendAnimCycle( AnimList & cycle, F32 blendTime = DEFBLENDTIME);

  Bool SetAnimOverlay( const char * cycleName)
  {
    return SetAnimOverlay( Crc::CalcStr( cycleName)); 
  }
  Bool SetAnimOverlay( U32 cycleID);
  Bool SetAnimOverlay( AnimList * cycle);
  void SetAnimTargetFrame( F32 target);
  void SetAnimTarget( F32 targetNormal);  // -1, 0, 1

  Bool AnimIsActive()
  {
    return animState0.active;
  }
  Bool AnimCheckFrame( F32 frame)
  {
    return frame >= animState0.lastFrame && frame < animState0.curFrame;
  }
  F32 AnimDirection()
  {
    return animState0.dir;
  }
  F32 AnimCurFrame()
  {
    return animState0.curFrame;
  }
  F32 AnimLastFrame()
  {
    return animState0.lastFrame;
  }

  Bool SetFrame( F32 frame = 0.0);
  void SetFrameSimulate( AnimState &animState, F32 lastFrame, Array<FamilyState> & stateArray);
  void SetFrameSimulate( AnimState &animState, F32 lastFrame, Array<AnimKey> & keyArray);

  Bool CollideBounds(const Vector &vStart, const Vector &vEnd);

  U32  MRMSetVertCount();
  void MRMSetVertCount( U32 count);
  U32  MRMSetResolution();
  U32  MRMSetFull();

  void Chunkify();

  const MeshObj * CollidePoly(const Vector &vStart, const Vector &vEnd, F32 &t);
};
//----------------------------------------------------------------------------

#endif    // __MESHENT_H
