///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshman.cpp
//
// 08-SEP-1998
//

//#include "vid_publicmore.h"
#include "vid_public.h"
#include "terrain_priv.h"
#include "light_priv.h"
#include "meshent.h"
#include "meshoptions.h"
#include "main.h"
#include "console.h"
#include "statistics.h"
#include "terrain.h"
#include "iface.h"
#include "hardware.h"
//----------------------------------------------------------------------------

#define MRMAUTOSPEED    0.004f

U8 startVar;

// static Mesh data
//
NBinTree<MeshRoot>   Mesh::Manager::rootTree;
NList<MeshEnt>       Mesh::Manager::entList; 
Bool                 Mesh::Manager::sysInit = FALSE;
MeshRoot *           Mesh::Manager::nullRoot;
Bitmap *             Mesh::Manager::envMap;

U32                  Mesh::Manager::vertCount;
U32                  Mesh::Manager::vertCountMRM;
U32                  Mesh::Manager::objCountMRM;

Bool                 Mesh::Manager::readErrors;     // were there xsi read errors?

// currently selected objects
//
MeshEnt *            Mesh::Manager::curParent;
MeshEnt *            Mesh::Manager::curEnt;
MeshEnt *            Mesh::Manager::selEnt;
MeshObj *            Mesh::Manager::curChild;
MeshObj *            Mesh::Manager::selChild;

Clock::CycleWatch    timer, timer2;

U8 endVar;
//----------------------------------------------------------------------------

static IControl * CreateHandler(U32 crc, IControl *parent, U32)
{
  IControl *ctrl = NULL;

  switch (crc)
  {
  case 0xE70705BB: // "MeshOptions"
    ctrl = new MeshOptions(parent);
    break;
  }

  return (ctrl);
}
//----------------------------------------------------------------------------

Bool Mesh::Manager::Init()
{
  rootTree.SetNodeMember( &MeshRoot::treeNode);
  entList.SetNodeMember( &MeshEnt::listNode);

  envMap = NULL;

  Statistics::Init();

  InitResources();

  return sysInit = TRUE;
}
//----------------------------------------------------------------------------

Bool Mesh::Manager::InitResources()
{
  // setup the NULL root
  //
  nullRoot = new MeshRoot();
  ASSERT( nullRoot);

  FamilyState meshState;
  meshState.ClearData();
  nullRoot->SetupStates( &meshState, 1);

  nullRoot->SetName( "null");  
  SetupRoot( *nullRoot, "null");

  // other stuff
  //
  Vid::renderState.mrmAutoFactor = 1.0f;

  curChild = selChild = NULL;
  curEnt   = selEnt   = NULL;
  curParent = NULL;

  return TRUE;
}
//----------------------------------------------------------------------------

Bool Mesh::Manager::InitIFace()
{
  IFace::RegisterControlClass("MeshOptions", CreateHandler);

  MeshOptions::Init();

  return TRUE;
}
//----------------------------------------------------------------------------

void Mesh::Manager::DisposeAll()
{
  // detach all the ents from one another first
  //
  NList<MeshEnt>::Iterator li( &entList); 
  for (!li; *li; li++)
  {
    MeshEnt &ent = *(*li);

    if (ent.Parent())
    {
      ent.Detach();
    }
  }
  entList.DisposeAll();

  rootTree.DisposeAll();

  // rootTree.DisposeAll destroyed it
  nullRoot = NULL;

  curChild = selChild = NULL;
  curEnt   = selEnt   = NULL;

  envMap = NULL;
}
//----------------------------------------------------------------------------

void Mesh::Manager::Done()
{
  DisposeAll();

  Statistics::Done();

  sysInit = FALSE;

/*
  LOG_DIAG( (""));
  LOG_DIAG( ("render  : %s", timer.Report() ) );
  LOG_DIAG( ("render2 : %s", timer2.Report() ) );
  LOG_DIAG( (""));
*/
}
//----------------------------------------------------------------------------

void Mesh::Manager::DoneIFace()
{
  MeshOptions::Done();
  IFace::UnregisterControlClass("MeshOptions");
}
//----------------------------------------------------------------------------

void Mesh::Manager::ResetCounts()
{
  vertCount = vertCountMRM = objCountMRM = 0;
}
//----------------------------------------------------------------------------

void Mesh::Manager::UpdateMRMFactor()
{
  if (!Vid::renderState.status.mrmAuto)
  {
    return;
  }

#if 1
  S32 value = Main::triCount;

  static F32 error0 = 0.0f;
  F32 target, error;
  S32 dv;

  if (value < Vid::renderState.mrmLow)
  {
    // too few
    dv = value - S32(Vid::renderState.mrmLow);
//    target = F32(dv) * 0.004f;
    target = F32(dv);
    error = error0 + (target - error0) * Vid::renderState.mrmAutoConstant1;
  }
  else if (value > Vid::renderState.mrmHigh)
  {
    // too many
    dv = value - S32(Vid::renderState.mrmHigh);
//    target = F32(dv) * 0.004f;
    target = F32(dv);
    error = error0 + (target - error0) * Vid::renderState.mrmAutoConstant1;
  }
  else
  {
    // in bounds; slowly adjust to Vid::renderState.mrmHigh
    dv = value - S32(Vid::renderState.mrmLow);
//    target = F32(dv) * 0.004f;
    target = F32(dv);
    error = error0 + (target - error0) * Vid::renderState.mrmAutoConstant2 * target * 0.5f;
//    error = error0 + (target - error0) * *Vid::renderState.status.mrmAutoConstant2;
  }

#else
  S32 elap = S32(Main::elapLast);

  static F32 error0 = 0.0f;
  F32 target, error;
  S32 dms;
  if (elap > Vid::renderState.mrmLow)
  {
    // too slow
    dms = elap - S32(Vid::renderState.mrmLow);
    target = F32(dms) * 0.4f;
    error = error0 + (target - error0) * Vid::renderState.mrmAutoConstant1;
  }
  else if (elap < Vid::renderState.mrmHigh)
  {
    // too fast
    dms = elap - S32(Vid::renderState.mrmHigh);
    target = F32(dms) * 0.4f;
    error = error0 + (target - error0) * Vid::renderState.mrmAutoConstant1;
  }
  else
  {
    // in bounds; slowly adjust to mrmElapFrame
    dms = elap - S32(Vid::renderState.mrmLow);
    target = F32(dms) * 0.4f;
    error = error0 + (target - error0) * Vid::renderState.status.mrmAutoConstant2 * target * 0.5f;
//    error = error0 + (target - error0) * *Vid::renderState.status.mrmAutoConstant2;
  }
//  F32 error = error0 + (target - error0) * (abs(dms) > *Vid::renderState.status.mrmAutoCutoff ? Vid::renderState.mrmAutoConstant1 : *Vid::renderState.status.mrmAutoConstant2 * target);
#endif

  error0 = error;

  error *= Vid::renderState.mrmAutoConstant3;

  if (error <= -1.0f)
  {
    error = -0.99999999999f;
  }
  else if (error > 9.0f)
  {
    error = 9.0f;
  }
  Vid::renderState.mrmAutoFactor = error + 1.0f;

//  MSWRITEV(22, (4, 0, "dv: %d, mrm error: %f, target: %f", dv, error, target) );

//  LOG_DIAG( ("elap: %d, dms: %d, error: %f, target: %f", elap, dms, error, target) );
}
//----------------------------------------------------------------------------

void Mesh::Manager::OnModeChange()
{
  if (sysInit)
  {
    SetupRenderProcList();
  }
}
//----------------------------------------------------------------------------

void Mesh::Manager::MakeName( BuffString &buff, const char *meshName, F32 scale) // = 1.0f)
{
  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;

  Dir::PathExpand( meshName, drive, dir, name, ext);

  buff = meshName;
  char *str = buff.str + Utils::Strlen(buff.str);
  sprintf( str, " %4.4f", scale);
}
//----------------------------------------------------------------------------

MeshRoot *Mesh::Manager::Find( const char *meshName)
{
  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;

  Dir::PathExpand( meshName, drive, dir, name, ext);

	U32 key = Crc::CalcStr(name.str);
	NBinTree<MeshRoot>::Iterator li(&rootTree);
	
	MeshRoot *root = li.Find(key);

  return root;
}
//----------------------------------------------------------------------------

// add it to the search tree
// 
Bool Mesh::Manager::SetupRoot( MeshRoot &root, const char *rootName) // = NULL
{
  root.fileName.Set( rootName);

  root.SortFaces();

  if (!root.Check())
  {
    return FALSE;
  }
  root.CalcBoundingSphere();     // FIXME

  rootTree.Add( root.fileName.crc, &root); 

  return TRUE;
}
//----------------------------------------------------------------------------

void Mesh::Manager::RenderList()
{
  NList<MeshEnt>::Iterator li( &entList); 

  for (!li; *li; li++)
  {
    MeshEnt &ent = *(*li);

    if (ent.BoundsTest() == clipOUTSIDE)
    {
      continue;
    }
    ent.Render(); 

    if (Vid::renderState.status.showShadows && Vid::Light::sun && Vid::Light::sun->IsActive())
    {
      Terrain::RenderShadowWithWater( ent);
    }
  }
}
//----------------------------------------------------------------------------

void Mesh::Manager::SimulateList( F32 dt)
{
  NList<MeshEnt>::Iterator li( &entList); 

  for (!li; *li; li++)
  {
    if ((*li)->clipFlagCache == clipOUTSIDE)
    {
      continue;
    }
    (*li)->SimulateSim( dt);
  }
}
//----------------------------------------------------------------------------

// restore all meshes to max vertex count
//
void Mesh::Manager::FullResList()
{
  for (NList<MeshEnt>::Iterator li(&entList); *li; li++)
  {
    MeshEnt &meshent = *(*li);
    meshent.MRMSetFull();
  }
}
//----------------------------------------------------------------------------

void Mesh::Manager::SetupRenderProcList()
{
  for (NList<MeshEnt>::Iterator li(&entList); *li; li++)
  {
    MeshEnt &meshent = *(*li);
    meshent.SetupRenderProc();
  }
}
//----------------------------------------------------------------------------

MeshRoot *Mesh::Manager::FindExists( const char *meshName)
{
  return rootTree.Find( Crc::CalcStr( meshName));
}
//----------------------------------------------------------------------------

MeshEnt *Mesh::Manager::Create( const MeshRoot *root)
{
  MeshEnt *meshEnt = new MeshEnt( root);

  return meshEnt;
}
//----------------------------------------------------------------------------

// Returns NULL if 'meshName' could not be found
//
MeshEnt *Mesh::Manager::Create( const char *meshName)
{
  MeshRoot *root = Mesh::Manager::FindRead( meshName, Vid::renderState.scaleFactor, Vid::renderState.status.mrmGen);

  return (root ? Create(root) : NULL);
}
//----------------------------------------------------------------------------

MeshEnt * Mesh::Manager::PickAtScreenPos(S32 x, S32 y, MeshObj **child) // = NULL
{
  Vector pos;
  const MeshObj *mobj, *closeObj = NULL;
  MeshEnt *closeEnt = NULL;
  F32 t;

  Vid::ScreenToCamera( pos, x, y);
  pos *= Vid::CurCamera().FarPlane();
  const Matrix &cam = Vid::CurCamera().WorldMatrix();
  cam.Transform(pos);

  Vector diff = pos - cam.posit;
  F32 mindist2 = 1000000000.0f;

  for (NList<MeshEnt>::Iterator li(&entList); *li; li++)
  {
    MeshEnt &meshent = *(*li);
    if (!meshent.CollideBounds( cam.posit, pos))
    {
      continue;
    }
    mobj = meshent.CollidePoly(cam.posit, pos, t);

    if (mobj)
    {
      Vector tdiff = diff * t;
      F32 dist2 = tdiff.Magnitude2();

      if (dist2 < mindist2)
      {
        closeObj = mobj;
        closeEnt = *li;
      }
    }
  }  
  if (child)
  {
    *child = (MeshObj *) closeObj;
  }
  return closeEnt;
}
//----------------------------------------------------------------------------

void Mesh::Manager::SetupPerf()
{
  F32 perf = Vid::renderState.perfs[0];

  Vid::renderState.mrmFactor2 = 1.0f / perf;

  Vid::renderState.status.shadowType  = perf <= .8f  ? Vid::renderState.status.shadowType <= .5 ? MeshRoot::shadowOVAL : MeshRoot::shadowGENERIC : MeshRoot::shadowSEMILIVE;
  Vid::renderState.status.showShadows = perf <= .25f ? FALSE : TRUE;
}
//----------------------------------------------------------------------------

MeshRoot * Mesh::Manager::MakeMesh( U32 vCount, U32 nCount, U32 uvCount, U32 fCount, Bitmap * texture)
{
	MeshRoot * root = new MeshRoot();
	ASSERT( root);

  FamilyState meshState;
  root->SetState( "__globe", meshState, 0);
  root->ClearState();
  root->SetObjectMatrix( Matrix::I);
	root->SetupStates( &meshState, 1);
  root->states[0].SetObjectMatrix( Matrix::I.posit);
  root->stateMats[0].ClearData();

  root->vertToState.Alloc( vCount);

  for (U32 i = 0; i < vCount; i++)
  {
    root->vertToState[i].count  = 1;
    root->vertToState[i].index[0]  = 0;
    root->vertToState[i].weight[0] = 1;
  }

  root->vertices.Alloc( vCount);
  root->normals.Alloc( nCount);
  root->uvs.Alloc( uvCount);
  root->faces.Alloc( fCount);

  root->envMap = FALSE;

	BucketLock bucky;
  bucky.ClearData();
	bucky.material = Vid::defMaterial;
  bucky.diff = bucky.material->Diffuse();
	bucky.texture0  = texture;
  bucky.flags0 = RS_TEX_MODULATE;
	root->buckys.Setup( 1, &bucky);

  return root;
}
//----------------------------------------------------------------------------

MeshRoot * Mesh::Manager::MakeGlobe( F32 radius, U32 bands, Bitmap * texture) 
{
	U32 i, j, LastIndex, BandsX2, LastBandStartIndex;
	U32 TopBandStartIndex, BottomBandStartIndex, zvert;
	U32 dowrap = 1;
//	U32 dowrap = 0;
	Vector BaseVec, BandVec, WVec, TempVec;
	F32 zmin, c1, c2, c0, stickwid, r1, r2;
	F32 *rads;
		
	BandsX2 = bands*2;

	Quaternion att;
	Matrix matY, matZ;
  matY.ClearData();
  matZ.ClearData();

	att.Set( PI / bands, Matrix::I.Up());
	matY.Set( att);
	
	att.Set( PI / bands, Matrix::I.Front());
	matZ.Set( att);

	U32 vertC = 2 + (BandsX2 + 1) * (bands - 1);
	U32 faceC = BandsX2 * bands + BandsX2 * (bands - 2);

	rads = new F32[bands + 1];
  ASSERT( rads);

  MeshRoot * root = MakeMesh( vertC, vertC, vertC, faceC, texture);
  Vector  * vects = root->vertices.data;
  Vector  * norms = root->normals.data;
  UVPair  * uvs   = root->uvs.data;
  FaceObj * faces = root->faces.data;

	zmin = 32000.0f;
	// point at the top
	BaseVec.x = 0.0f;
	BaseVec.y = radius;
	BaseVec.z = 0.0f;

  U32 vertCount = 0, faceCount = 0;
	vects[vertCount] = BaseVec;
	norms[vertCount] = norms[vertCount];
	norms[vertCount].Normalize();
	vertCount++;

	BandVec = BaseVec;

	// create the vertices in each band in turn
	for (i = 1; i < bands; i++)
	{
		// rotate around Z to the next band's latitude
		matZ.Transform( TempVec, BandVec);

		WVec = BandVec = TempVec;
		rads[i] = (F32) sqrt( WVec.x * WVec.x + WVec.z * WVec.z);
		// do the vertices in this band
		U32 startVert = vertCount;
		for (j = 0; j < BandsX2; j++)
		{
			WVec = TempVec;
			vects[vertCount] = WVec;
			norms[vertCount] = vects[vertCount];
			norms[vertCount].Normalize();
			vertCount++;

			if (WVec.z < zmin)
			{
				zmin = WVec.z;
				zvert = vertCount;
			}
			// rotate around Y to the next vertex's longitude
			matY.Transform( TempVec, WVec);
		}
		// duplicate band start vert at the end (for texture wrapping)
		vects[vertCount] = vects[startVert];
		norms[vertCount] = vects[vertCount];
		norms[vertCount].Normalize();
		vertCount++;
	}
	// point at the bottom
	BaseVec.y = -radius;
	vects[vertCount] = BaseVec;
	norms[vertCount] = vects[vertCount];
	norms[vertCount].Normalize();
	LastIndex = vertCount;
	vertCount++;

	r1 = 0.0f;
	r2 = 1.0f / (F32) bands;
	// top band
	stickwid = 100 * rads[1] / radius;
	stickwid += (100 - stickwid)/2;
	for (i = 0; i < BandsX2; i++, faceCount++)
	{
		faces[faceCount].verts[0] = 0;
//		faces[faceCount].verts[1] = ((i + 1) % BandsX2) + 1;
		faces[faceCount].verts[1] = (WORD) (((i + 1) % (BandsX2 + 1)) + 1);
		faces[faceCount].verts[2] = (WORD) (i + 1);

		faces[faceCount].norms[0] = faces[faceCount].verts[0];
		faces[faceCount].norms[1] = faces[faceCount].verts[1];
		faces[faceCount].norms[2] = faces[faceCount].verts[2];

		faces[faceCount].uvs[0] = faces[faceCount].verts[0];
		faces[faceCount].uvs[1] = faces[faceCount].verts[1];
		faces[faceCount].uvs[2] = faces[faceCount].verts[2];
		faces[faceCount].buckyIndex = 0;

		if (dowrap == 1)
		{
			c1 = (F32) i / (F32) BandsX2;
			c2 = (F32) (i + 1) / (F32) BandsX2;
			c0 = (c1 + 0.5f) / (F32) BandsX2;

			uvs[faces[faceCount].verts[0]].u = c0;
			uvs[faces[faceCount].verts[0]].v = r1;

			uvs[faces[faceCount].verts[1]].u = c1;
			uvs[faces[faceCount].verts[1]].v = r2;

			uvs[faces[faceCount].verts[2]].u = c2;
			uvs[faces[faceCount].verts[2]].v = r2;
		}
		else
		{
			uvs[faces[faceCount].verts[0]].u = 0.5f;
			uvs[faces[faceCount].verts[0]].v = 0.0f;

			uvs[faces[faceCount].verts[1]].u = 0.0f;
			uvs[faces[faceCount].verts[1]].v = 1.0f;

			uvs[faces[faceCount].verts[2]].u = 1.0f;
			uvs[faces[faceCount].verts[2]].v = 1.0f;
		}
	}

	// middle bands
	for (j = 0; j < bands - 2; j++)
	{
//		TopBandStartIndex = j * BandsX2 + 1;
//		BottomBandStartIndex = (j + 1) * BandsX2 + 1;
		TopBandStartIndex = j * (BandsX2 + 1) + 1;
		BottomBandStartIndex = (j + 1) * (BandsX2 + 1) + 1;
		r1 = (F32) (j + 1) / (F32) bands;
		r2 = (F32) (j + 2) / (F32) bands;
		for (i = 0; i < BandsX2; i++, faceCount += 2)
		{
			faces[faceCount].verts[0] = (WORD) (i + TopBandStartIndex);
//			faces[faceCount].verts[1] = ((i + 1) % BandsX2) + TopBandStartIndex;
//			faces[faceCount].verts[2] = ((i + 1) % BandsX2) + BottomBandStartIndex;
			faces[faceCount].verts[1] = (WORD) (((i + 1) % (BandsX2 + 1)) + TopBandStartIndex);
			faces[faceCount].verts[2] = (WORD) (((i + 1) % (BandsX2 + 1)) + BottomBandStartIndex);

		  faces[faceCount].norms[0] = faces[faceCount].verts[0];
		  faces[faceCount].norms[1] = faces[faceCount].verts[1];
		  faces[faceCount].norms[2] = faces[faceCount].verts[2];

		  faces[faceCount].uvs[0] = faces[faceCount].verts[0];
		  faces[faceCount].uvs[1] = faces[faceCount].verts[1];
		  faces[faceCount].uvs[2] = faces[faceCount].verts[2];
		  faces[faceCount].buckyIndex = 0;

//			faces[faceCount+1].verts[0] = ((i + 1) % BandsX2) + BottomBandStartIndex;
			faces[faceCount+1].verts[0] = (WORD) (((i + 1) % (BandsX2 + 1)) + BottomBandStartIndex);
			faces[faceCount+1].verts[1] = (WORD) (i + BottomBandStartIndex);
			faces[faceCount+1].verts[2] = (WORD) (i + TopBandStartIndex);

		  faces[faceCount+1].norms[0] = faces[faceCount+1].verts[0];
		  faces[faceCount+1].norms[1] = faces[faceCount+1].verts[1];
		  faces[faceCount+1].norms[2] = faces[faceCount+1].verts[2];

		  faces[faceCount+1].uvs[0] = faces[faceCount+1].verts[0];
		  faces[faceCount+1].uvs[1] = faces[faceCount+1].verts[1];
		  faces[faceCount+1].uvs[2] = faces[faceCount+1].verts[2];
      faces[faceCount+1].buckyIndex = 0;

			if (dowrap == 1)
			{
				c1 = (F32) i / (F32) BandsX2;
				c2 = (F32) (i + 1) / (F32) BandsX2;

				uvs[faces[faceCount].verts[0]].u = c1;
				uvs[faces[faceCount].verts[0]].v = r1;

				uvs[faces[faceCount].verts[1]].u = c2;
				uvs[faces[faceCount].verts[1]].v = r1;

				uvs[faces[faceCount].verts[2]].u = c2;
				uvs[faces[faceCount].verts[2]].v = r2;

				uvs[faces[faceCount+1].verts[0]].u = c2;
				uvs[faces[faceCount+1].verts[0]].v = r2;

				uvs[faces[faceCount+1].verts[1]].u = c1;
				uvs[faces[faceCount+1].verts[1]].v = r2;

				uvs[faces[faceCount+1].verts[2]].u = c1;
				uvs[faces[faceCount+1].verts[2]].v = r1;
			}
			else
			{
				uvs[faces[faceCount].verts[0]].u = 1.0f;
				uvs[faces[faceCount].verts[0]].v = 0.0f;
										     
				uvs[faces[faceCount].verts[1]].u = 0.0f;
				uvs[faces[faceCount].verts[1]].v = 0.0f;
										     
				uvs[faces[faceCount].verts[2]].u = 0.0f;
				uvs[faces[faceCount].verts[2]].v = 1.0f;
										     
				uvs[faces[faceCount+1].verts[0]].u = 0.0f;
				uvs[faces[faceCount+1].verts[0]].v = 1.0f;
										     
				uvs[faces[faceCount+1].verts[1]].u = 1.0f;
				uvs[faces[faceCount+1].verts[1]].v = 1.0f;
										     
				uvs[faces[faceCount+1].verts[2]].u = 1.0f;
				uvs[faces[faceCount+1].verts[2]].v = 0.0f;
			}
		}
	}

	// bottom band
//	LastBandStartIndex = BandsX2 * (bands - 2) + 1;
	LastBandStartIndex = (BandsX2 + 1) * (bands - 2) + 1;
	r1 = (F32) (bands - 1) / (F32) bands;
	r2 = 1.0f;
	for (i = 0; i < BandsX2; i++, faceCount++)
	{
		faces[faceCount].verts[0] = (WORD) LastIndex;
		faces[faceCount].verts[1] = (WORD) (LastBandStartIndex + i);
//		faces[faceCount].verts[2] = LastBandStartIndex + ((i + 1) % BandsX2);
		faces[faceCount].verts[2] = (WORD) (LastBandStartIndex + ((i + 1) % (BandsX2 + 1)));

		faces[faceCount].norms[0] = faces[faceCount].verts[0];
		faces[faceCount].norms[1] = faces[faceCount].verts[1];
		faces[faceCount].norms[2] = faces[faceCount].verts[2];

		faces[faceCount].uvs[0] = faces[faceCount].verts[0];
		faces[faceCount].uvs[1] = faces[faceCount].verts[1];
		faces[faceCount].uvs[2] = faces[faceCount].verts[2];
		faces[faceCount].buckyIndex = 0;

		if (dowrap == 1)
		{
			c1 = (F32) i / (F32) BandsX2;
			c2 = (F32) (i + 1) / (F32) BandsX2;
			c0 = c1 - 0.5f / (F32) BandsX2;

			uvs[faces[faceCount].verts[0]].u = c0;
			uvs[faces[faceCount].verts[0]].v = r2;
									     
			uvs[faces[faceCount].verts[1]].u = c1;
			uvs[faces[faceCount].verts[1]].v = r1;
									     
			uvs[faces[faceCount].verts[2]].u = c2;
			uvs[faces[faceCount].verts[2]].v = r1;
		}
		else
		{
			uvs[faces[faceCount].verts[0]].u = 0.5f;
			uvs[faces[faceCount].verts[0]].v = 0.0f;
									     
			uvs[faces[faceCount].verts[1]].u = 0.0f;
			uvs[faces[faceCount].verts[1]].v = 1.0f;
									     
			uvs[faces[faceCount].verts[2]].u = 1.0f;
			uvs[faces[faceCount].verts[2]].v = 1.0f;
		}
	}
	delete [] rads;

  root->Setup();

	// setup up face plane equations
  //
	root->SetupPlanes();

	if (!SetupRoot(*root, root->GetName()))
  {
    delete root;
    root = NULL;
  }
	return root;
}
//----------------------------------------------------------------------------
U32 remem, recount, rgeo, ranim, rmrm;

U32 Mesh::Manager::Report( MeshRoot & root, Bool all) // = FALSE
{
  U32 mem = root.GetMem();

  U32 emem = 0, count = 0;
  NList<MeshEnt>::Iterator li(&entList);
  while (MeshEnt * ent = li++)
  {
    if (&ent->Root() == &root)
    {
      emem += ent->GetMem();
      count++;
    }
  }
  remem   += emem;
  recount += count;

  U32 geo = root.GetMemGeometry();
  rgeo += geo;
  U32 anim = root.GetMemAnim();
  ranim += anim;
  U32 mrm = root.GetMemMRM();
  rmrm += mrm;

  if (all)
  {
    CON_DIAG( ("%-36s:           f%4ld v%4ld n%4ld u%4ld c%4ld m%3ld a%2ld", 
      root.fileName.str,
      root.faces.count,
		  root.vertices.count,
		  root.normals.count,
		  root.uvs.count,
		  root.colors.count,
      root.states.count,
      root.animCycles.GetCount()
    ));
    LOG_DIAG( ("%-36s:           f%4ld v%4ld n%4ld u%4ld c%4ld m%3ld a%2ld", 
      root.fileName.str,
      root.faces.count,
		  root.vertices.count,
		  root.normals.count,
		  root.uvs.count,
		  root.colors.count,
      root.states.count,
      root.animCycles.GetCount()
    ));
    CON_DIAG( ("%-36s: %9ld   g%6ld a%6ld m%6ld c%6ld", "  mem total",
      mem, geo, anim, mrm, root.GetMemChunks()
    ));
    LOG_DIAG( ("%-36s: %9ld   g%6ld a%6ld m%6ld c%6ld", "  mem total",
      mem, geo, anim, mrm, root.GetMemChunks()
    ));

    CON_DIAG( (" %4d %-30s: %9ld", count, "instances", emem));
    LOG_DIAG( (" %4d %-30s: %9ld", count, "instances", emem));
  }
  else
  {
    CON_DIAG( ("%-36s: %9ld   f%4ld v%4ld i%4ld", 
      root.fileName.str, mem + emem,
      root.faces.count,
		  root.vertices.count,
      count
    ));
    LOG_DIAG( ("%-36s: %9ld   f%4ld v%4ld i%4ld", 
      root.fileName.str, mem + emem,
      root.faces.count,
		  root.vertices.count,
      count
    ));
  }
  return mem;
}
//----------------------------------------------------------------------------

U32 Mesh::Manager::ReportList( const char * name) // = NULL 
{
  U32 count = 0;
  U32 len = 0, hit = FALSE;
  if ( name)
  {
    len = strlen( name);
  }
  else
  {
    hit = TRUE;
  }
  U32 mem = 0;
  remem = recount = rgeo = ranim = rmrm = 0;
  NBinTree<MeshRoot>::Iterator li(&rootTree);
  while (MeshRoot * root = li++)
  {
    if (!root->isChunk && (!name || !strnicmp( root->fileName.str, name, len)))
		{
      mem += Report( *root);
      hit = TRUE;

      count++;
    }
  }
  if (hit)
  {
    CON_DIAG( ("%4ld %-31s: %9ld   g%6ld a%6ld m%6ld", count, "mesh types", mem,
      rgeo, ranim, rmrm
    ));
    LOG_DIAG( ("%4ld %-31s: %9ld   g%6ld a%6ld m%6ld", count, "mesh types", mem,
      rgeo, ranim, rmrm
    ));

    CON_DIAG( ("%4ld %-31s: %9ld", recount, "mesh instances", remem ) );
    LOG_DIAG( ("%4ld %-31s: %9ld", recount, "mesh instances", remem ) );
  }
  else
  {
    CON_DIAG( ("Can't find mesh type: %s", name) );
  }

  return mem + remem;
}
//----------------------------------------------------------------------------

U32 Mesh::Manager::Report()
{
  U32 countT = 0, memT = 0;
  NBinTree<MeshRoot>::Iterator ri(&rootTree);
  while (MeshRoot * root = ri++)
  {
    if (!root->isChunk)
		{
      memT += root->GetMem();
      countT++;
    }
  }

  U32 countE = 0, memE = 0;
  NList<MeshEnt>::Iterator ei(&entList);
  while (MeshEnt * ent = ei++)
  {
    memE += ent->GetMem();
    countE++;
  }

  U32 mem = memT + memE + endVar - startVar;   // namespace

  CON_DIAG( ("%4d %-31s: %9d", countT, "mesh types", memT ) );
  LOG_DIAG( ("%4d %-31s: %9d", countT, "mesh types", memT ) );

  CON_DIAG( ("%4d %-31s: %9d", countE, "mesh instances", memE ) );
  LOG_DIAG( ("%4d %-31s: %9d", countE, "mesh instances", memE ) );

  return mem;
}
//----------------------------------------------------------------------------
