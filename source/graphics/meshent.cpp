///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// meshent.cpp       MeshObj and MeshEnt
//
// 08-SEP-1998
//

#include "vid_private.h"
#include "terrain_priv.h"
#include "main.h"
#include "random.h"
#include "perfstats.h"
#include "stdload.h"
//----------------------------------------------------------------------------

const F32 TEXTURETIMER = .1f;

// MeshObj

void MeshObj::ClearData()
{
  nodeType = nodeMeshObj;
  mesh = NULL;
  clipFlagCache = clipOUTSIDE;
}
//----------------------------------------------------------------------------

// meshEnt/meshObj hierarchies must be setup via SetupFamily for this release to always work
//
MeshObj::~MeshObj()
{
//  children.DisposeAll();
//  FamilyNode::Release();
//  delete this;
}
//----------------------------------------------------------------------------

// inserts 'node' as 'this's' primary/last child depending on 'insert' flag
//
void MeshEnt::Attach( FamilyNode &node, Bool insert) // = TRUE
{
  if (node.IsMeshEnt())
  {
    MeshEnt * entNode = (MeshEnt *) &node;

    // put it in the list
    if (insert)
    {
      eChildren.Prepend( entNode);
    }
    else
    {
      eChildren.Append( entNode);
    }
    entNode->eParent = this;

    // clear object matrix
    //
    entNode->SetSimTarget( Matrix::I);
    entNode->states0[0].SetSlaveObject( entNode->states1[0]);
    entNode->statesR[0].SetSlaveObject( entNode->states1[0]);

    FamilyNode::Attach( node);

    entNode->SetWorldRecurseRender( WorldMatrix(), &entNode->statesR[0]);
  }
  else
  {
    FamilyNode::Attach( node);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::AttachAt( FamilyNode & at, FamilyNode &node, Bool insert) // = TRUE)
{
  if (node.IsMeshEnt())
  {
    MeshEnt * entNode = (MeshEnt *) &node;

    // put it in the list
    if (insert)
    {
      eChildren.Prepend( entNode);
    }
    else
    {
      eChildren.Append( entNode);
    }
    entNode->eParent = this;

    // clear object matrix
    //
    entNode->SetSimTarget( Matrix::I);
    entNode->states0[0].SetSlaveObject( entNode->states1[0]);
    entNode->statesR[0].SetSlaveObject( entNode->states1[0]);

    at.FamilyNode::Attach( node);

    entNode->SetWorldRecurseRender( at.WorldMatrix(), &entNode->statesR[0]);
  }
  else
  {
    at.FamilyNode::Attach( node);
  }
}
//----------------------------------------------------------------------------

// removes 'this' and all its children from parent
//
void MeshEnt::Detach()
{
  FamilyNode::Detach();

  if (eParent)
	{
    ASSERT(eParent->eChildren.GetCount());

    eParent->eChildren.Unlink(this);
		eParent = NULL;

    // maintain world matrix
    //
    SetSimCurrent( states0[0].WorldMatrix());
  }
}
//----------------------------------------------------------------------------

int MeshObj::GetMeshArray( MeshObj **array, int maxSize, int count)
{
	if (count >= maxSize)
  {
    return count;
  }
	if (IsMeshObj())
	{
		array[count] = this;
		count++;
	}

  NList<FamilyNode>::Iterator kids(&children);
  FamilyNode * node;
  while ((node = kids++) != NULL)
  {
  	count = ((MeshObj *)node)->GetMeshArray(array, maxSize, count);
	}
	return count;
}
//----------------------------------------------------------------------------

Bool MeshObj::SetupFamily( const Mesh *_mesh, Array<FamilyState> & stateArray)
{
  mesh = (Mesh *) _mesh;

#if 1
  name = mesh->name;

  if (!_mesh->Parent())
  {
    // setup FamilyNode::state and FamilyState::node pointers
    SetState( stateArray[name.index]);
  }
#else

  SetState( mesh->GetName(), stateArray[name.index], mesh->GetIndex());
#endif

  NList<FamilyNode>::Iterator kids(&mesh->children);
  U32 insert = TRUE;
  Mesh * node;
  for ( ; (node = (Mesh *) kids++) != NULL; insert = FALSE)
  {
    if (node->nodeType != nodeMesh)
    {
      continue;
    }
    MeshObj *childObj = new MeshObj( &stateArray[node->name.index]);
    Attach( *childObj, insert);

    if (!childObj->SetupFamily( node, stateArray))
    {
      return FALSE;
    }
  }
  return TRUE;
}
//----------------------------------------------------------------------------

// MeshEnt
//
void MeshEnt::ClearData()
{
  nodeType = nodeMeshEnt;
  eParent = NULL;
  eChildren.SetNodeMember(&MeshEnt::eChildNode);

  mesh = NULL;
  curCycle = NULL;
  conCycle = NULL;
  curCycleID = 0;
  fps = ANIMFPS;
  texTimer = TEXTURETIMER;
  texTime = 0;
  name.str[0] = '\0';

  animState0.ClearData();
  animStateR.ClearData();
  interpolated = FALSE;
  interpFrame = 0.0f;

  viewOrigin.ClearData();

  renderProc = MeshEnt::RenderAnimVtl;

  selData = NULL;

  renderProcSave = NULL;
  effect = NULL;
  texture = NULL;
  dirtyShadow = TRUE;    // update shadowTexture

  baseColor = 0xffffffff;
  teamColor = 0xffffffff;

  texAnimPoll = FALSE;

  envMap = FALSE;
  hasTexAnim = FALSE;
  textureAnim = FALSE;
  effecting = FALSE;
  texAnimAuto = FALSE;

  fogCurrent = 255;
  alphaCurrent = 255;

  shadowTexture = NULL;
  shadowTime = 0;
  shadowAlpha0 = 255;
  shadowAlpha1 = 0;
  shadowInfo.texture = NULL;

  dirtyRoot = dirtyAll = dirtyIntRoot = dirtyIntAll = TRUE;
}
//----------------------------------------------------------------------------
static U32 row = 2;

MeshEnt::MeshEnt( const MeshRoot * root) // = NULL
{
  ClearData();

  if (root)
  {
    Setup( *root);
  }
}
//----------------------------------------------------------------------------

MeshEnt::MeshEnt( const char * meshName)
{
  ClearData();

  MeshRoot * root = Mesh::Manager::FindRead( meshName, Vid::renderState.scaleFactor, Vid::renderState.status.mrmGen);

  if (root)
  {
    Setup( *root);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::ReleaseBuckys()
{
  // release data
  //
  for (U32 i = 0; i < buckys.count; i++)
  {
    // array templates don't call their elements destructors
    //
    buckys[i].Release();
  }
  buckys.Release();
}
//----------------------------------------------------------------------------

MeshEnt::~MeshEnt()
{
#if 1
  // remove the effect
  //
  if (effect)
  {
    effect->Detach( this);
  }
#endif

  // remove from hierarchy
  //
  if (Parent())
  {
    Detach();
  }

  // remove from ent hierarchy
  //
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->Detach();
  }
  eChildren.UnlinkAll();

  // check if its already been removed from the manager list 
  //
  if (listNode.InUse())
  {  
    Mesh::Manager::entList.Unlink( this);
  }

  ReleaseBuckys();

  statesR.Release();
  states0.Release();
  states1.Release();

  treads.Release();

  if (selData)
  {
    delete selData;
  }
}
//----------------------------------------------------------------------------

Bool MeshEnt::SetupStates( const MeshRoot &root)
{
  if (states0.Setup( root.states) == NULL
   || statesR.Setup( root.states)  == NULL
   || states1.Alloc( root.states.count) == NULL
   || blends.Alloc( root.states.count)  == NULL)
  {
    return FALSE;
  }

  if (root.vOffsets.count)
  {
    treads.Alloc( root.vOffsets.count);
  }

  U32 i;
  for (i = 0; i < statesR.count; i++)
  {
    states1[i].ClearData();

    if (root.vOffsets.count)
    {
      treads[i].offset = 0;
      treads[i].rate  = 0;
    }
  }

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshEnt::Reset( const MeshRoot & root)
{
  SetupBuckys( root);

  // default team color
  //
  SetTeamColor( Vid::renderState.teamColor);
}
//----------------------------------------------------------------------------

void MeshEnt::SetupBuckys( const MeshRoot & root)
{
  ReleaseBuckys();

  // setup instance buckys
  //
  buckys.Setup( RootPriv().groups);

  // setup face arrays
  //
  for (U32 i = 0; i < buckys.count; i++)
  {
    FaceGroup & bucky = buckys[i];
    FaceGroup & group = RootPriv().groups[i];

    // setup does a copy: clear the copied pointers! hack!!
    bucky.geo.ClearData();
    bucky.faces.count = 0;
    bucky.faces.data = NULL;
    bucky.faces.count = 0;
    bucky.faces.Setup( group.faces);
  }

  // setup instance mrm vert counts
  //
  vertCount = nextVertCount = root.vertices.count;
  faceCount = root.faces.count;

  // setup instance bounds
  //
  animState0.bounds = root.ObjectBounds();
  animStateR.bounds = animState0.bounds;
}
//----------------------------------------------------------------------------

void MeshEnt::Setup( const MeshRoot & root)
{
  // state arrays
  //
  if (!SetupStates( root))
  {
    ERR_FATAL( ("MeshEnt::MeshEnt: SetupStates() failure on %s", root.GetName()) );
  }

  // MeshObj hierarchy
  //
  if (!SetupFamily( &root, states0))
  {
    ERR_FATAL( ("MeshEnt::MeshEnt: SetupFamily() failure on %s", root.GetName()) );
  }

  // setup FamilyState::node pointers
  //
  U32 i;
  for (i = 0; i < statesR.count; i++)
  {
    statesR[i].SetNode( states0[i]);

    // setup initial target states
    //
    states1[i] = /*states0[i] =*/ root.states[i];
  }
  if (statesR.count)
  {
    // interval for interpolation of statesR[0]
    //
    states1[0].frame = 0.1f;
  }

  SetupBuckys( root);

  envMap     = root.envMap;
  hasAnim    = root.hasAnim;
  hasTexAnim = root.hasTexAnim;
  hasTread   = root.hasTread;
  hasControl = root.hasControl;

  shadowType = root.shadowType;

  // default team color
  //
  SetTeamColor( Vid::renderState.teamColor);

  // setup instance animating texture timer
  //
  texTimer = root.texTimer;

  // clear root object matrices
  //
  SetObjectMatrix( Matrix::I);

  // start with standard default anim cycle
  //
  SetAnimCycle( DEFCYCLENAME);

/*
  if (curCycle)
  {
    // random anim start frame
    //
    animState0.curFrame = animStateR.curFrame = (F32) Random::sync.Integer((U32) curCycle->maxFrame);
  }
*/

  // setup render procedure pointer
  //
  SetupRenderProc();

  // add it to the manager list
  //
  Mesh::Manager::entList.Append( this);

  // setup intial world matrices
  //
  SetWorldAll(Matrix::I);
}
//----------------------------------------------------------------------------

void MeshEnt::SetTeamColor( Color _teamColor)
{
  teamColor = _teamColor;

  // setup instance bucket lock descriptions
  // 
  for (U32 i = 0; i < buckys.count; i++)
  {
    BucketLock & bucky = buckys[i];

    if (!bucky.teamColor)
    {
      continue;
    }

    bucky.diff.r = U8toNormF32 * teamColor.r;
    bucky.diff.g = U8toNormF32 * teamColor.g;
    bucky.diff.b = U8toNormF32 * teamColor.b;

    ColorF32 zero(0,0,0), diffuse( 
      bucky.diff.r * Material::Manager::diffuseVal,
      bucky.diff.g * Material::Manager::diffuseVal,
      bucky.diff.b * Material::Manager::diffuseVal,
      bucky.diff.a
    );
    GameIdent gi;
    Material::Manager::GenerateName( gi.str, diffuse, zero, 0, zero, diffuse);
    Material * mat = Material::Manager::FindCreate( name.str);

    mat->SetDiffuse(  diffuse.r, diffuse.g, diffuse.b, diffuse.a);

    bucky.material = mat;
  }

  // setup child ents
  //
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SetTeamColor( teamColor);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetBaseColor( Color _baseColor)
{
  baseColor = _baseColor;

#if 0
  // setup child ents
  // 
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SetBaseColor( baseColor);
  }
#endif
}
//----------------------------------------------------------------------------

void MeshEnt::ModulateBaseColor( Color bColor)
{
  baseColor.Modulate( bColor.r * U8toNormF32, bColor.g * U8toNormF32, bColor.b * U8toNormF32, bColor.a * U8toNormF32); 
}
//----------------------------------------------------------------------------

// setup just the opaque part of the baseColor
//
void MeshEnt::SetOpaque( Color oColor)
{
  baseColor.r = oColor.r;
  baseColor.g = oColor.g;
  baseColor.b = oColor.b;

#if 0
  // setup child ents
  // 
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SetBaseColor( baseColor);
  }
#endif
}
//----------------------------------------------------------------------------

// setup just the opaque part of the baseColor
//
void MeshEnt::SetOpaque( U32 oColor)
{
  baseColor.r = U8(oColor);
  baseColor.g = U8(oColor);
  baseColor.b = U8(oColor);

#if 0
  // setup child ents
  // 
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SetBaseColor( baseColor);
  }
#endif
}
//----------------------------------------------------------------------------

// setup just the alpha part of the baseColor
//
void MeshEnt::SetTranslucent( U32 t)
{
  baseColor.a = U8(t);

#if 0
  // setup child ents
  // 
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SetBaseColor( baseColor);
  }
#endif
}
//----------------------------------------------------------------------------

// get a vertex's world location
//
void MeshEnt::GetVertWorld( Vector & vert, U32 index) const
{
  if (!Root().vertices.count)
  {
    vert.Set( 0, 0, 0);
    return;
  }

  if (index >= Root().vertices.count)
  {
    index = Random::nonSync.Integer( Root().vertices.count - 1);
  }
  VertIndex &vindex = Root().vertToState[index];
  Matrix trany = Root().stateMats[vindex.index[0]] * states0[vindex.index[0]].WorldMatrix();

  trany.Transform(vert, Root().vertices[index]);
}
//----------------------------------------------------------------------------

// setup the sim world matrices
//
void MeshEnt::SetWorldRecurse( const Matrix &world)
{
  FamilyNode::SetWorldRecurse( world);

  if (Parent())
  {
    statesR[0].SetObject( states0[0]);
  }
  else
  {
    statesR[0].SetWorld( states0[0]);
  }

  WorldMatrix().Transform( animState0.origin, animState0.bounds.Offset());
  WorldMatrix().Transform( animState0.rootOrigin, RootBounds().Offset());
}
//----------------------------------------------------------------------------

// setup the interpolated world matrices
//
void MeshEnt::SetWorldRecurseRender( const Matrix &world, FamilyState *stateArray)
{
  stateArray;

  FamilyNode::SetWorldRecurseRender( world, &statesR[0]);

  statesR[0].WorldMatrix().Transform( animStateR.origin, animStateR.bounds.Offset());
  statesR[0].WorldMatrix().Transform( animStateR.rootOrigin, RootBounds().Offset());
}
//----------------------------------------------------------------------------

// returns was dirty
//
Bool MeshEnt::UpdateSim( F32 dt)
{

#ifdef DEVELOPMENT

  if (this == Mesh::Manager::curEnt)
  {
    NodeIdent ident( "CP-WEAPON");
    FindIdent( ident);
    U32 index = 0;
    if (Mesh::Manager::curChild)
    {
      index = Mesh::Manager::curChild->GetIndex();
    }
    else
    {
      index = ident.index == 0xffffffff ? 0 : ident.index;
    }

    Matrix mat( states1[index].quaternion);

    const Vector & simPos = states0[index].ObjectMatrixPriv().posit;
    const Vector & intPos = statesR[index].ObjectMatrixPriv().posit;
    const Vector & tarPos = states1[index].position;
    const Vector & simFnt = states0[index].ObjectMatrixPriv().front;
    const Vector & intFnt = statesR[index].ObjectMatrixPriv().front;
    const Vector & tarFnt = mat.front;


    MSWRITEV(22, (2,  2, "%s:%s: interpFrame: %f", Root().xsiName.str, states0[index].GetName(), interpFrame));

    MSWRITEV(22, (4,  2, "SimPos: %.2f,%.2f,%.2f",  simPos.x, simPos.y, simPos.z ));
    MSWRITEV(22, (5,  2, "IntPos: %.2f,%.2f,%.2f",  intPos.x, intPos.y, intPos.z ));
    MSWRITEV(22, (6,  2, "TarPos: %.2f,%.2f,%.2f",  tarPos.x, tarPos.y, tarPos.z ));

    MSWRITEV(22, (8,  2, "SimFnt: %.2f,%.2f,%.2f",  simFnt.x, simFnt.y, simFnt.z ));
    MSWRITEV(22, (9,  2, "IntFnt: %.2f,%.2f,%.2f",  intFnt.x, intFnt.y, intFnt.z ));
    MSWRITEV(22, (10, 2, "TarFnt: %.2f,%.2f,%.2f",  tarFnt.x, tarFnt.y, tarFnt.z ));

    MSWRITEV(22, (12, 2, "AnimState0: %f",  animState0.curFrame));
    MSWRITEV(22, (13, 2, "AnimStateR: %f",  animStateR.curFrame));
    MSWRITEV(22, (14, 2, "diff      : %f",  animStateR.curFrame - animState0.curFrame));

    MSWRITEV(22, (16, 2, "LodVal: %f  Multiweight %d", lodValue, (controlFlags & controlMULTIWEIGHT) ? TRUE : FALSE));

    if (Root().useMrm)
    {
      MSWRITEV(22, (16, 2, "Verts: Cur: %4d  Min: %4d  Max: %4d    ", vertCount, Root().mrm->minVertCount, Root().mrm->maxVertCount));
    }
    else
    {
      MSWRITEV(22, (16, 2, "Verts: Cur: %4d                           ", vertCount));
    }
  }


#endif

  ASSERT( dt > 0);

  // setup current states
  //
  dirtyIntRoot = dirtyRoot;
  dirtyIntAll  = dirtyAll;

  interpolated = FALSE;

  Bool wasDirty = dirtyRoot;

  if (dirtyRoot)
  {
    states0[0].quaternion = states1[0].quaternion;
    states0[0].position   = states1[0].position;
    states0[0].scale      = states1[0].scale;

    if (Parent())
    {
      states0.data->SetObject( *states1.data);
    }
    else
    {
      states0.data->SetWorld( *states1.data);
    }

    dirtyRoot = FALSE;
  }

  // save interval time for interpolation
  //
  states1[0].frame = dt;

  // do it for children ents too
  //
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->UpdateSim( dt);
  }

  animStateR = animState0;

  SimulateSim( dt);

  if (dirtyAll)
  {
    FamilyState * src0, * es = &states0[statesR.count];
    AnimKey * src1 = &states1[1];

    for (src0 = &states0[1]; src0 < es; src0++, src1++)
    {
      // child states
      //
      if (src1->type & animDIRTY)
      {
        src0->quaternion = src1->quaternion;
        src0->position   = src1->position;
        src0->scale      = src1->scale;

        src0->SetObject( *src1);    // FIXME

        // clear dirty flag
        //
//        src->type &= ~animALLDIRTY;
        src1->type &= ~animDIRTY;
      }
    }

    dirtyAll = FALSE;

    if (!eParent)
    {
      // setup the world matrices
      //
      SetWorldAll();
    }
  }

  return wasDirty;
}
//----------------------------------------------------------------------------

// setup target matrices via animation
//
void MeshEnt::SimulateSim( F32 dt)
{
  // do it for children ents too
  //
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SimulateSim( dt);
  }

  if (animState0.active)
  {
    SetTreeDirty( FALSE);

    animState0.lastFrame = animState0.curFrame;
    animState0.curFrame += dt * fps * animState0.dir;

    ClampFrame( animState0);

    if (animState0.curFrame >= 0.0f)
    {
      // no blending on sim states
      SetFrameSimulate( animState0, animState0.lastFrame, states1);
    }
  }
}
//----------------------------------------------------------------------------

// setup render states via animation
//
void MeshEnt::SimulateInt( F32 dt, Bool isInterpFrame) // = TRUE
{
  isInterpFrame;

  if (dt > states1[0].frame)
  {
    dt = (F32) fmod( dt, states1[0].frame);
  }

  interpFrame = interpolated ? interpFrame + dt : 0.0f;

  // interpolate the root matrix
  //
  F32 dtdi = interpFrame / states1[0].frame;

  if (dirtyIntRoot)
  {
    statesR[0].quaternion = states0[0].GetRotation().Slerp( states1[0].GetRotation(), dtdi);

    statesR[0].position   = states0[0].GetPosition() + (states1[0].GetPosition() - states0[0].GetPosition()) * dtdi;
    statesR[0].scale      = states0[0].GetScale()    + (states1[0].GetScale()    - states0[0].GetScale())    * dtdi;

    if (Parent())
    {
      statesR[0].SetObject();
      statesR[0].SetObjectScale();
    }
    else
    {
      statesR[0].SetWorld();
      statesR[0].SetWorldScale();
    }
    dirtyShadow = TRUE;
  }

  SimulateIntRecurse( dt, dtdi);

  if (dirtyIntAll)
  {
    SetWorldRecurseRender( statesR[0].WorldMatrix(), &statesR[0]);
    dirtyShadow = TRUE;
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SimulateIntRecurse( F32 dt, F32 dtdi)
{
  if (animStateR.active)
  {
    animStateR.lastFrame = animStateR.curFrame;

    animStateR.curFrame += dt * fps * animStateR.dir;
    ClampFrame( animStateR);

    SetFrameSimulate( animStateR, animStateR.lastFrame, statesR);

    dirtyIntAll = TRUE;
    if (eParent)
    {
      eParent->dirtyIntAll = TRUE;
    }
  }
  if (dirtyIntAll)
  {
    // setup render states
    //
    FamilyState * ss, * es = &statesR[statesR.count], * src0 = &states0[1];
    AnimKey * src1 = &states1[1];
    for (ss = &statesR[1]; ss < es; ss++, src0++, src1++)
    {
      if (src1->type & animCONDIRTY)
      {
        ss->Set( src0->GetRotation().Slerp( src1->GetRotation(), dtdi));
        ss->Set( src0->GetPosition()   + (src1->GetPosition() - src0->GetPosition()) * dtdi);
        ss->SetScale( src0->GetScale() + (src1->GetScale()    - src0->GetScale())    * dtdi);
      }

      if (ss->type & animDIRTY)
      {
        ss->SetObject();
        ss->SetObjectScale();

        ss->type &= ~animALLDIRTY;
      }
    }
  }

  // do it for children ents too
  //
  NList<MeshEnt>::Iterator kids(&eChildren);
  MeshEnt * node;
  while ((node = kids++) != NULL)
  {
    node->SimulateIntRecurse( dt, dtdi);
  }
  interpolated = TRUE;
}
//----------------------------------------------------------------------------

// meter per sec
//
void MeshEnt::SetTreadRate( NodeIdent & ident, F32 rate)
{
  ASSERT( ident.index < states0.count);

  const MeshRoot & root = Root();

  rate *= -root.treadPerMeter;

  if (root.states[ident.index].IsTread())
  {
    treads[ident.index].rate = rate;
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetUVAnimRate( F32 rate)
{
  const MeshRoot & root = Root();

  rate *= -root.treadPerMeter;

  FamilyState * s, * e = root.states.data + root.states.count;
  U32 i = 0;
  for (s = root.states.data; s < e; s++, i++)
  {
    if (s->IsTread())
    {
      treads[i].rate = rate;
    }
  }
}
//----------------------------------------------------------------------------

// 2 = auto run
//
void MeshEnt::ActivateTexAnim( U32 active) // = TRUE
{
  if (!hasTexAnim)
  {
    return;
  }
  textureAnim = active ? 1 : 0;
  texAnimAuto = active > 1 ? 1 : 0;

  if (!textureAnim)
  {
    SetTexFrame( 0);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::PollActivateTexAnim()
{
  texAnimPoll = 1;
}
//----------------------------------------------------------------------------

// animate textures
//
void MeshEnt::SimulateTex( F32 dt, Bool simFrame)
{
  if (!textureAnim && !texAnimPoll)
  {
    if (lastTexPoll)
    {
      SetTexFrame( 0);
      lastTexPoll = FALSE;
    }
    return;
  }

  texTime += dt;

  if (texTime >= texTimer)
  {
    texTime = fmodf( texTime, texTimer);

    RootPriv().TexAnim( &buckys);

    if (simFrame)
    {
      lastTexPoll = texAnimPoll;
      texAnimPoll = texAnimAuto;
    }
  }
}
//----------------------------------------------------------------------------

// animate textures
//
void MeshEnt::SetTexFrame( U32 frame) // = 0
{
  if (!hasTexAnim)
  {
    return;
  }

  texTime = 0;

  U32 i; 
  for (i = 0; i < buckys.count; i++)
  {
    BucketLock & bucky = buckys[i];

    if (bucky.texture0)
    {
      U32 f = bucky.texture0->GetAnimFrame();

      while (bucky.texture0->GetAnimFrame() != frame)
      {
        bucky.texture0 = bucky.texture0->GetNext();

        // clamp at the end
        //
        if (f > bucky.texture0->GetAnimFrame())
        {
          break;
        }
        f = bucky.texture0->GetAnimFrame();
      }
    }
    if (bucky.texture1)
    {
      U32 f = bucky.texture1->GetAnimFrame();

      while (bucky.texture1->GetAnimFrame() != frame)
      {
        bucky.texture1 = bucky.texture1->GetNext();

        // clamp at the end
        //
        if (f > bucky.texture1->GetAnimFrame())
        {
          break;
        }
        f = bucky.texture1->GetAnimFrame();
      }
    }
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetFrameSimulate( AnimState &animState, F32 lframe, Array<FamilyState> & stateArray)
{
  if (animState.curFrame < 0.0f)
  {
    curCycle->SetBlend( animState.curFrame, &blends[0], &curCycle->states[0], stateArray);
  }
  else
  {
    animState.blend  = FALSE;

    if (lframe < 0.0f)
    {
      // setup the inital states for this cycle
      U32 i;
      for (i = 1; i < statesR.count; i++)
      {
        stateArray[i].Set( curCycle->states[i]);
      }
      // set this cycle's bounds
      //
      animState.bounds = curCycle->bounds;

      if (curCycle->endFrame <= 0.0f)
      {
        animState.active = FALSE;
      }
      else
      {
        curCycle->SetFrame( animState.curFrame, stateArray);
      }
    }
    else
    {
      curCycle->SetFrame( animState.curFrame, stateArray);
    }
/*
    if (conCycle && *Mesh::Manager::animOverlay)
    {
      conCycle->SetFrameOverlay( animState.conFrame, stateArray);
    }
*/
  }

/*
  if (statesR[0].type & animSCALE)
  {
    F32 scale = statesR[0].scale.x > statesR[0].scale.y ? statesR[0].scale.x : statesR[0].scale.y;
    if (statesR[0].scale.z > scale)
    {
      scale = statesR[0].scale.z;
    }
    SetWorldBoundsRadius( ObjectBounds().Radius() * scale); 
  }
*/
}
//----------------------------------------------------------------------------

void MeshEnt::SetFrameSimulate( AnimState & animState, F32 lframe, Array<AnimKey> & keyArray)
{
  if (animState.curFrame < 0.0f)
  {
    curCycle->SetBlend( animState.curFrame, &blends[0], &curCycle->states[0], keyArray);
  }
  else
  {
    animState.blend  = FALSE;

    if (lframe < 0.0f)
    {
      // setup the inital states for this cycle
      U32 i;
      for (i = 1; i < statesR.count; i++)
      {
        keyArray[i].Set( curCycle->states[i]);
      }
      // set this cycle's bounds
      //
      animState.bounds = curCycle->bounds;

      if (curCycle->endFrame <= 0.0f)
      {
        animState.active = FALSE;
      }
      else
      {
        curCycle->SetFrame( animState.curFrame, keyArray);
      }
    }
    else
    {
      curCycle->SetFrame( animState.curFrame, keyArray);
    }
/*
    if (conCycle && *Mesh::Manager::animOverlay)
    {
      conCycle->SetFrameOverlay( animState.conFrame, keyArray);
    }
*/
  }

/*
  if (statesR[0].type & animSCALE)
  {
    F32 scale = statesR[0].scale.x > statesR[0].scale.y ? statesR[0].scale.x : statesR[0].scale.y;
    if (statesR[0].scale.z > scale)
    {
      scale = statesR[0].scale.z;
    }
    SetWorldBoundsRadius( ObjectBounds().Radius() * scale); 
  }
*/
}
//----------------------------------------------------------------------------

// Set the current and next simulation frame's rotation, position, and scale
//
void MeshEnt::SetSimCurrent( const Quaternion & quaternion)
{
  SetSimTarget( quaternion);

  if (!parent)
  {
    states0[0].SetSlaveWorld( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( WorldMatrix(), &statesR[0]);
  }
  else
  {
    states0[0].SetSlaveObject( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( parent->WorldMatrix(), &statesR[0]);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimCurrent( const Vector & position)
{
  SetSimTarget( position);

  if (!parent)
  {
    states0[0].SetSlaveWorld( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( WorldMatrix(), &statesR[0]);
  }
  else
  {
    states0[0].SetSlaveObject( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( parent->WorldMatrix(), &statesR[0]);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimCurrent( const Quaternion & quaternion, const Vector & position)
{
  SetSimTarget( quaternion, position);

  if (!parent)
  {
    states0[0].SetSlaveWorld( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( WorldMatrix(), &statesR[0]);
  }
  else
  {
    states0[0].SetSlaveObject( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( parent->WorldMatrix(), &statesR[0]);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimCurrent( const Matrix & matrix)
{
  SetSimTarget( matrix);

  if (!parent)
  {
    states0[0].SetSlaveWorld( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( WorldMatrix(), &statesR[0]);
  }
  else
  {
    states0[0].SetSlaveObject( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( parent->WorldMatrix(), &statesR[0]);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimCurrentScale( const Vector & scale)
{
  SetSimTargetScale( scale);

  if (!parent)
  {
    states0[0].SetSlaveWorld( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( WorldMatrix(), &statesR[0]);
  }
  else
  {
    states0[0].SetSlaveObject( states1[0]);

    SetWorldAll();

    SetWorldRecurseRender( parent->WorldMatrix(), &statesR[0]);
  }
}
//----------------------------------------------------------------------------

// Set the next simulation frame's rotation, position, and scale
//
void MeshEnt::SetSimTarget( const Quaternion & quaternion)
{
  states1[0].Set( quaternion);
  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTarget( const Vector & position)
{
  states1[0].Set( position);
  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTarget( const Quaternion & quaternion, const Vector & position)
{
  states1[0].Set( quaternion, position);
  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTarget( const Matrix & matrix)
{
  states1[0].Set( matrix);
  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTargetScale( const Vector & scale)
{
  states1[0].SetScale( scale);
  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetTreeDirty( Bool rootDirty) // = TRUE)
{
  dirtyRoot = rootDirty;

  // dirty the root o' the tree too
  //
  MeshEnt * p = this;
  while (p)
  {
    p->dirtyAll = TRUE;
    p = p->eParent;
  }
}
//----------------------------------------------------------------------------

// Set the next simulation frame's rotation, position, and scale
//
void MeshEnt::SetSimTarget( const NodeIdent & ident, const Quaternion & quaternion)
{
  AnimKey & state = states1[ident.index];
  state.Set( quaternion);
  state.type |= animCONDIRTY;

  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTarget( const NodeIdent & ident, const Vector & position)
{
  AnimKey & state = states1[ident.index];
  state.Set( position);
  state.type |= animCONDIRTY;

  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTarget( const NodeIdent & ident, const Quaternion & quaternion, const Vector & position)
{
  AnimKey & state = states1[ident.index];
  state.Set( quaternion, position);
  state.type |= animCONDIRTY;

  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTarget( const NodeIdent & ident, const Matrix & matrix)
{
  AnimKey & state = states1[ident.index];
  state.Set( matrix);
  state.type |= animCONDIRTY;

  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetSimTargetScale( const NodeIdent & ident, const Vector & scale)
{
  AnimKey & state = states1[ident.index];
  state.Set( scale);
  state.type |= animCONDIRTY;

  SetTreeDirty();
}
//----------------------------------------------------------------------------

void MeshEnt::SetWorldRender()
{
  if (!parent)
  {
    SetWorldRecurseRender( WorldMatrix(), &statesR[0]);
  }
  else
  {
    SetWorldRecurseRender( parent->WorldMatrix(), &statesR[0]);
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SetAnimType( AnimType _type)
{
  if (curCycle)
  {
    curCycle->SetType( _type);
    animState0.dir = animStateR.dir = 1.0f;
    animState0.active = animStateR.active = TRUE;
  }
}
//----------------------------------------------------------------------------

void MeshEnt::ClampFrame( AnimState & animState)
{
  if (curCycle->type == animLOOP)
  {
    if (animState.curFrame > curCycle->maxFrame)
    {
      animState.curFrame = (F32) fmod( animState.curFrame, curCycle->maxFrame);
    }
    else if (animState.dir < 0.0f && animState.curFrame < 0.0f)
    {
      animState.curFrame = (F32) fmod( animState.curFrame, curCycle->maxFrame) + curCycle->maxFrame;
    }
  }
  else // 1 or 2 way
  {
    if (animState.curFrame > curCycle->endFrame)
    {
      animState.curFrame = curCycle->endFrame;

      if (curCycle->type == anim2WAY)
      {
        animState.dir = -1.0f;
      }
      animState.active = FALSE;
    }
    else if (!animState.blend && animState.curFrame < 0.0f)
    {
      animState.curFrame = 0.0f;

      animState.dir = 1.0f;
      animState.active = FALSE;
    }
  }
}
//----------------------------------------------------------------------------

Bool MeshEnt::SetFrame( F32 frame) // = 0.0)
{
  if (!curCycle)
  {
    return FALSE;
  }
  // setup this cycle's bounds if coming out of a blend
  //
  if (animState0.curFrame < 0)
  {
    animState0.bounds = curCycle->bounds;
    animState0.blend  = FALSE;
  }

  animState0.curFrame = frame;
  ClampFrame( animState0);

  animStateR = animState0;

  UpdateSim(0.1f);
  SimulateInt(0.1f);

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshEnt::SetAnimOverlay( U32 cycleID)
{
  if (!mesh)
  {
    return TRUE;
  }
  const MeshRoot & root = Root();
  AnimList * cycle = root.FindAnimCycle( cycleID);
  if (!cycle)
  {
    return FALSE;
  }
  return SetAnimOverlay( cycle);
}
//----------------------------------------------------------------------------

Bool MeshEnt::SetAnimOverlay( AnimList * cycle)
{
  if (!mesh || !cycle)
  {
    conCycle = NULL;
    return TRUE;
  }
  if (cycle->type != animCONTROL || cycle == conCycle)
  {
    return FALSE;
  }

  conCycle = cycle;

  animState0.conDir = 1.0f;
  animState0.conFrame = cycle->controlFrame;
  animState0.targetFrame = cycle->controlFrame; 

  animStateR.conDir = animState0.conDir;
  animStateR.conFrame = animState0.conFrame;
  animStateR.targetFrame = animState0.targetFrame; 

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshEnt::SetAnimTarget( F32 target)  // -1, 0, 1
{
  if (!conCycle)
  {
    return;
  }
  ASSERT( conCycle->type == animCONTROL);

  if (target > 0)
  {
    target = conCycle->controlFrame + (conCycle->endFrame - conCycle->controlFrame) * target;
  }
  else if (target < 0)
  {
    target = conCycle->controlFrame + (conCycle->controlFrame) * target;
  }
  else
  {
    target = conCycle->controlFrame;
  }
  ASSERT( target >= 0 && target <= conCycle->endFrame);

  animState0.targetFrame  = target;
  animState0.conDir       = animState0.targetFrame > animState0.conFrame ? 1.0f : -1.0f;
  
  animStateR.targetFrame  = animState0.targetFrame;
  animStateR.conDir       = animState0.conDir;
}
//----------------------------------------------------------------------------

void MeshEnt::SetAnimTargetFrame( F32 target)
{
  if (!conCycle)
  {
    return;
  }
  ASSERT( conCycle->type == animCONTROL);
  ASSERT( target >= 0 && target <= conCycle->endFrame);

  animState0.targetFrame = target;
  animState0.conDir = animState0.targetFrame > animState0.conFrame ? 1.0f : -1.0f;

  animStateR.targetFrame = animState0.targetFrame;
  animStateR.conDir         = animState0.conDir;
}
//----------------------------------------------------------------------------

Bool MeshEnt::SetAnimCycle( U32 cycleID)
{
  if (!mesh)
  {
    return TRUE;
  }
  const MeshRoot & root = Root();
  AnimList * cycle = root.FindAnimCycle( cycleID);
  if (!cycle)
  {
    return FALSE;
  }
  return SetAnimCycle( *cycle);
}
//----------------------------------------------------------------------------

Bool MeshEnt::SetAnimCycle( AnimList & cycle)
{
  if (!mesh)
  {
    return TRUE;
  }
/*
  if (cycle.type == animCONTROL)
  {
    return SetAnimOverlay( &cycle);
  }
*/

  animState0.blend  = FALSE;

  if (&cycle == curCycle)
  {
    if (cycle.endFrame > 0)
    {
      if (cycle.type == anim1WAY)
      {
        animState0.curFrame = 0.0f;
        animState0.dir = 1.0f;
        SetWorldAll();  // FIXME
      }

      if (curCycle->endFrame > 0.0f)
      {
        animState0.active = TRUE;
      }
      animStateR = animState0;
    }
    return TRUE;
  }
  curCycle = &cycle;
  curCycleID = cycle.name.crc;
  animState0.dir = 1.0f;
  animState0.active = cycle.endFrame > 0.0f ? TRUE : FALSE;

/*
  if (cycle.type == animCONTROL)
  {
    animState0.curFrame = cycle.controlFrame;
    animState0.targetFrame = cycle.controlFrame; 
  }
  else
*/
  {
    animState0.curFrame = 0.0f;
    animState0.targetFrame = cycle.endFrame; 
  }

  // setup the actual play speed
  //
  fps = cycle.animSpeed;

  // setup the intial state
  U32 i;
  for (i = 1; i < statesR.count; i++)
  {
#if 1
    if (states0[i].type & animNOANIM)
    {
      // don't do control nodes
      continue;
    }
#endif

    states0[i].SetSlaveObject( cycle.states[i]);
    statesR[i].SetSlaveObject( cycle.states[i]);
//    statesR[i] = states0[i];
//    states1[i].Set( cycle.states[i]);
  }
  SetWorldAll();
  SetWorldRender();

  // setup this cycle's bounds
  //
  animState0.bounds = cycle.bounds;

  animStateR = animState0;

  return TRUE;
}
//----------------------------------------------------------------------------

Bool MeshEnt::BlendAnimCycle( U32 cycleID, F32 blendTime) // = DEFBLENDTIME
{
  if (!mesh)
  {
    return TRUE;
  }
  const MeshRoot &root = Root();
  AnimList * cycle = root.FindAnimCycle( cycleID);
  if (!cycle)
  {
    return FALSE;
  }
  return BlendAnimCycle( *cycle, blendTime);
}
//----------------------------------------------------------------------------

Bool MeshEnt::BlendAnimCycle( AnimList & cycle, F32 blendTime) // = DEFBLENDTIME
{
//  return SetAnimCycle( cycle);

  if (&cycle == curCycle || blendTime == 0 || cycle.type == animCONTROL)
  {
    return SetAnimCycle( cycle);
  }
  curCycle = &cycle;
  curCycleID = cycle.name.crc;
  animState0.curFrame = -blendTime;
  animState0.dir = 1.0f;
  animState0.active = TRUE;
  animState0.blend  = TRUE;

  animState0.targetFrame = cycle.endFrame;

  // setup the actual play speed
  //
  fps = cycle.animSpeed;
//  fps = Mesh::Manager::animBlendRate;

  // setup other blend key info
  U32 i;
  for (i = 1; i < statesR.count; i++)
  {
    // copy state info
//    blends[i] = states0[i];
    blends[i] = statesR[i];

    // blend keys are negative
    blends[i].frame = -blendTime;
//    blends[i].type = cycle.states[i].type;
    blends[i].type = animQUATERNION | animSCALE;

    Quaternion & qb = blends[i].quaternion;
    Quaternion & qc = cycle.states[i].quaternion;
    // invert backwards quaternions
    F32 dot = qb.Dot( qc);
    if (dot < 0)
    {
      qb *= -1.0f;
    }
    ASSERT( !(qb.s ==  1.0f && qb.v == 0 && qc.s == -1.0f && qc.s == 0));
    ASSERT( !(qb.s == -1.0f && qb.v == 0 && qc.s ==  1.0f && qc.s == 0));
  }

#if 0
  // setup this cycle's bounds
  //
  Bounds &cycleBounds = cycle.bounds;

  if (cycleBounds.width > bounds.width)
  {
    bounds.width = cycleBounds.width;
  }
  if (cycleBounds.height > bounds.height)
  {
    bounds.height = cycleBounds.height;
  }
  if (cycleBounds.breadth > bounds.breadth)
  {
    bounds.breadth = cycleBounds.breadth;
  }
  if (cycleBounds.radius > bounds.radius)
  {
    bounds.radius  = cycleBounds.radius;
  }
#endif

  animStateR = animState0;

  return TRUE;
}
//----------------------------------------------------------------------------

void MeshEnt::ActivateAnim( Bool activate) // = TRUE
{
  if (!activate)
  {
    animState0.active = animStateR.active = FALSE;
  }
  else if (curCycle && curCycle->endFrame > 0)
  {
    animState0.active = TRUE;
    animState0.blend  = FALSE;

    if (curCycle->type == anim1WAY)
    {
      animState0.curFrame = 0.0f;
      animState0.dir = 1.0f;
    }

    animStateR = animState0;
  }
}
//----------------------------------------------------------------------------

Bool MeshEnt::IsAnimCompleted()
{
  return curCycle ? AnimCurFrame() == curCycle->endFrame : FALSE;
}
//----------------------------------------------------------------------------

void MeshEnt::Copy(const MeshEnt *srcEnt, Bool local) // = FALSE
{
  srcEnt;
  local;

	const MeshRoot * root = &Root(); // fixme
	if (root != NULL)
	{
		if (!SetupStates( *root))
		{
		  ERR_FATAL( ("MeshEnt::MeshEnt: SetupStates() failure on %s", root->GetName()) );
		}
		// must be after SetupStates()
		if (!SetupFamily( root, states0))
		{
		  ERR_FATAL( ("MeshEnt::MeshEnt: SetupFamily() failure on %s", root->GetName()) );
		}
	}
//	SetSimCurrent( orig->WorldMatrix());
  SetWorldAll();
}
//----------------------------------------------------------------------------

Bool MeshEnt::CollideBounds(const Vector &vStart, const Vector &vEnd)
{
  Vector v1 = vEnd - vStart;
  Vector v2 = Origin() - vStart;

  v2 = v1.Cross( v2);
  F32 r2 = v2.Dot( v2) / v1.Dot( v1);

  return r2 <= ObjectBounds().Radius2();
}
//----------------------------------------------------------------------------

const MeshObj * MeshEnt::CollidePoly(const Vector &vStart, const Vector &vEnd, F32 &t)
{
  // pass in this's stateArray to root's CollidePoly
  //
  MeshObj *meshobj = (MeshObj *) Root().CollidePoly( states0, vStart, vEnd, t);

  return meshobj;
}
//----------------------------------------------------------------------------

void MeshEnt::SaveState( FScope * fScope)
{
  ASSERT( fScope);

  if (curCycle)
  {
    FScope * sScope = fScope->AddFunction("Animation");
    ASSERT( sScope);

    StdSave::TypeU32( sScope, "CycleId", curCycleID);

    StdSave::TypeF32( sScope, "CurrFrame", animState0.curFrame  < 0 ? 0 : animState0.curFrame );
    StdSave::TypeF32( sScope, "LastFrame", animState0.lastFrame < 0 ? 0 : animState0.lastFrame);
    StdSave::TypeF32( sScope, "Direction", animState0.dir);
    StdSave::TypeU32( sScope, "Active",    (U32) animState0.active);
    StdSave::TypeF32( sScope, "FPS",       fps);
  }
  if (hasTexAnim)
  {
    FScope * sScope = fScope->AddFunction("TextureAnim");
    ASSERT( sScope);

    StdSave::TypeF32( sScope, "Timer",    texTimer);
    StdSave::TypeU32( sScope, "Poll",     texAnimPoll);
    StdSave::TypeU32( sScope, "LastPoll", lastTexPoll);
    StdSave::TypeU32( sScope, "Auto",     texAnimAuto);
  }

  if (hasTread)
  {
    FScope * sScope = fScope->AddFunction("TreadStates");
    ASSERT( sScope);

    for (U32 i = 0; i < states0.count; i++)
    {
      if (states0[i].IsTread())
      {
        FScope * ssScope = sScope->AddFunction( "Tread");
        ASSERT( ssScope);

        ssScope->AddArgString( states0[i].GetName());

        StdSave::TypeF32( ssScope, "Offset", treads[i].offset);
        StdSave::TypeF32( ssScope, "Rate",   treads[i].rate);
      }
    }
  }

  if (hasControl)
  {
    FScope * sScope = fScope->AddFunction("ControlStates");
    ASSERT( sScope);

    for (U32 i = 0; i < states0.count; i++)
    {
      if (states0[i].IsControl())
      {
        FScope * ssScope = sScope->AddFunction( "Control");
        ASSERT( ssScope);

        ssScope->AddArgString( states0[i].GetName());

        states0[i].SaveState( ssScope);
        states1[i].SaveState( ssScope);
      }
    }
  }

//  fogCurrent->SaveState( sScope, "FogFactor");
//  alphaCurrent->SaveState( sScope, "alphaCurrent");
//  StdSave::TypeS32( fScope, "ExtraFog",   extraFog);
}
//----------------------------------------------------------------------------

void MeshEnt::LoadState( FScope * fScope)
{
  ASSERT( fScope);

  while (FScope * sScope = fScope->NextFunction())
  {
    switch (sScope->NameCrc())
    {
    case 0xBAB1E5A5: // "Animation"
      ASSERT( hasAnim);

      SetAnimCycle( StdLoad::TypeU32( sScope, "CycleId", curCycleID));
      SetFrame( StdLoad::TypeF32( sScope, "CurrFrame", animState0.curFrame));

      animState0.lastFrame = StdLoad::TypeF32( sScope, "LastFrame", animState0.lastFrame);
      animState0.dir       = StdLoad::TypeF32( sScope, "Direction", animState0.dir);
      animState0.active    = StdLoad::TypeU32( sScope, "Active",    (U32) animState0.active);
      fps                  = StdLoad::TypeF32( sScope, "FPS",       fps);
      break;

    case 0xA897C4CB: // "TextureAnim"
      ASSERT( hasTexAnim);

      texTimer    = StdLoad::TypeF32( sScope, "Timer",    texTimer);
      texAnimPoll = StdLoad::TypeU32( sScope, "Poll",     texAnimPoll);
      lastTexPoll = StdLoad::TypeU32( sScope, "LastPoll", lastTexPoll);
      texAnimAuto = StdLoad::TypeU32( sScope, "Auto",     texAnimAuto);
      break;

    case 0xE383575D: // "TreadStates"
      ASSERT( hasTread);

      while (FScope * ssScope = sScope->NextFunction())
      {
        ASSERT( ssScope->NameCrc() == 0x294B9C85); // "Tread"

        NodeIdent ident = ssScope->NextArgString();
        MeshObj * meshObj = FindIdent( ident);
        ASSERT( meshObj); meshObj;

        treads[ident.index].offset = StdLoad::TypeF32( ssScope, "Offset", treads[ident.index].offset);
        treads[ident.index].rate   = StdLoad::TypeF32( ssScope, "Rate",   treads[ident.index].rate);
      }
      break;

    case 0x01008A89: // "ControlStates"
      ASSERT( hasControl);

      while (FScope * ssScope = sScope->NextFunction())
      {
        ASSERT( ssScope->NameCrc() == 0x838CB20C); // "Control"

        NodeIdent ident = ssScope->NextArgString();
        MeshObj * meshObj = FindIdent( ident);
        ASSERT( meshObj); meshObj;

        states0[ident.index].LoadState( ssScope);
        states1[ident.index].LoadState( ssScope);
      }
      break;
    }
  }

//  fogFactor->SaveState( sScope, "FogFactor");
//  alphaCurrent->SaveState( sScope, "alphaCurrent");
//  StdSave::TypeS32( fScope, "ExtraFog",   extraFog);
}
//----------------------------------------------------------------------------

void MeshEnt::SetFogTarget( S32 fog, S32 alpha, Bool immediate) // = 255, FALSE
{
  if (immediate)
  {
    fogCurrent = fog;
    alphaCurrent = alpha;

//    SetOpaque( Color( (U32)fogCurrent, (U32)fogCurrent, (U32)fogCurrent, U32(255)));
//    SetTranslucent( (U32)alphaCurrent);
  }
  else 
  {
    if (fogCurrent > fog)
    {
      fogCurrent = Max<S32>( fog, fogCurrent - *Vid::Var::Terrain::shroudRate);
    }
    else
    {
      fogCurrent = Min<S32>( fog, fogCurrent + *Vid::Var::Terrain::shroudRate);
    }
    if (alphaCurrent > alpha)
    {
      alphaCurrent = Max<S32>( alpha, alphaCurrent - *Vid::Var::Terrain::shroudRate);
    }
    else
    {
      alphaCurrent = Min<S32>( alpha, alphaCurrent + *Vid::Var::Terrain::shroudRate);
    }
  }
}
//----------------------------------------------------------------------------

void MeshEnt::SimulateIntBasic( F32 dt, Bool simFrame)
{
  SimulateTex( dt, simFrame);

  if (Root().hasTread)
  {
    TreadState * s, * e = treads.data + treads.count;
    for (s = treads.data; s < e; s++)
    {
      s->offset = (F32) fmod( s->offset + s->rate * dt, 1.0f);
    }
  }
  SetOpaque( Color( (U32)fogCurrent, (U32)fogCurrent, (U32)fogCurrent, U32(255)));
  SetTranslucent( (U32)alphaCurrent);
}
//----------------------------------------------------------------------------

U32 MeshEnt::GetMem() const
{
  U32 mem = sizeof( this);

  mem += buckys.size;
  mem += states0.size;
  mem += states1.size;
  mem += statesR.size;
  mem += blends.size;
  mem += treads.size;

//  NList<MeshEnt>          eChildren;
//  NList<MeshEnt>::Node    eChildNode;

  return mem;
}
//----------------------------------------------------------------------------

void MeshEnt::MRMGen( Bool doSelVerts) // = TRUE
{
  RootPriv().MRMGen( doSelVerts ? &selData->verts : NULL );

//  UnSelectVerts();
  Reset( RootPriv());
}
//----------------------------------------------------------------------------