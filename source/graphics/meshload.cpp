///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// meshload.cpp   god file load/save
//
// 08-FEB-1999
//

#include "vid_private.h"
#include "stdload.h"
#include "godfile.h"


// MeshRoot block file key
const char *MeshRootBlock1  = "MeshRoot 4.0";
const char *MeshRootBlock2  = "MeshRoot 2.0";
const char *MeshRootBlock3  = "MeshRoot 3.0";
const char *MeshRootBlock5  = "MeshRoot 5.0";
const char *MeshRootBlock6  = "MeshRoot 6.0";
const char *MeshRootBlock7  = "MeshRoot 7.0";
const char *MeshRootBlock8  = "MeshRoot 8.0";
const char *MeshRootBlock9  = "MeshRoot 9.0";
const char *MeshRootBlock10 = "MeshRoot 10.0";
const char *MeshRootBlock11 = "MeshRoot 11.0";
const char *MeshRootBlock12 = "MeshRoot 12.0";

// Shadow plane block key
const char *ShadowPlaneBlock = "ShadowPlane 1.0";
//----------------------------------------------------------------------------

void Mesh::LoadAnimation( GodFile *god, Animation &animation, AnimType type)
{
  type;

  God::Load(*god, animation.index);
  God::Load(*god, animation.maxFrame);
  God::LoadArray(*god, animation.keys);

  if (animation.keys.count)
  {
    // always at least 2 keys
    AnimKey & key0 = animation.keys[animation.keys.count-2];
    AnimKey & key1 = animation.keys[animation.keys.count-1];

    if ((key0.type == animQUATERNION) && (key1.type == animQUATERNION) && key0.quaternion == key1.quaternion)
    {
      Array<AnimKey> keys(animation.keys.count-1);
      for (U32 i = 0; i < animation.keys.count-1; i++)
      {
        keys[i] = animation.keys[i];
      }
      keys.Swap( animation.keys);

//      LOG_DIAG(("removing redundant end key"));

      F32 dot = keys[keys.count-2].quaternion.Dot( keys[keys.count-1].quaternion);
      if (dot < 0)
      {
        keys[keys.count-1].quaternion *= -1.0f;
      }

      return;
    }

    F32 dot = key0.quaternion.Dot( key1.quaternion);
    if (dot < 0)
    {
      key1.quaternion *= -1.0f;
    }
  }
}
//----------------------------------------------------------------------------

void Mesh::LoadAnimCycle( GodFile *god, AnimList &animCycle)
{
  god->LoadStr(animCycle.name.str, MAX_GAMEIDENT);

  animCycle.type = (AnimType)god->LoadU32();
  God::Load(*god, animCycle.maxFrame);
  God::Load(*god, animCycle.endFrame);
  God::Load(*god, animCycle.animSpeed);

  if (god->version > 1)
  {
    // new versions
    //
    if (god->version >= 5)
    {
      God::Load(*god, animCycle.controlFrame);
    }

    God::Load(*god, animCycle.framesPerMeter);
    God::Load(*god, animCycle.bounds);
  }
  else
  {
    F32 metersPerFrame;
    God::Load(*god, metersPerFrame);
    animCycle.framesPerMeter = metersPerFrame == 0.0f ? 0.0f : 1.0f / metersPerFrame;

    Sphere sphere;
    God::Load(*god, sphere);
    animCycle.bounds.Set( sphere);
  }

  God::LoadArray(*god, animCycle.states);

  U32 animCount = god->LoadU32();

  for (U32 i = 0; i < animCount; i++)
  {
    Animation *anim = new Animation;
    animCycle.Append( anim);
    LoadAnimation( god, *anim, animCycle.type);
  }
}
//----------------------------------------------------------------------------

void Mesh::SaveAnimation( GodFile *god, const Animation &animation)
{
  God::Save(*god, animation.index);
  God::Save(*god, animation.maxFrame);
  God::SaveArray(*god, animation.keys);
}
//----------------------------------------------------------------------------

void Mesh::SaveAnimCycle( GodFile *god, const AnimList &animCycle)
{
  god->SaveStr(animCycle.name.str);
  God::Save(*god, U32(animCycle.type));
  God::Save(*god, animCycle.maxFrame);
  God::Save(*god, animCycle.endFrame);
  God::Save(*god, animCycle.animSpeed);

  God::Save(*god, animCycle.controlFrame);

  God::Save(*god, animCycle.framesPerMeter);
  God::Save(*god, animCycle.bounds);

  God::SaveArray(*god, animCycle.states);

  God::Save(*god, U32(animCycle.GetCount()));
  for (List<Animation>::Iterator li(&animCycle); *li; li++)
  {
    SaveAnimation( god, *(*li));
  }
}
//----------------------------------------------------------------------------

void Mesh::LoadMRMFace( GodFile *god, MRM::Face &face)
{
  face.face  = (U16) god->LoadU32();
  face.token = (MRM::Face::Token) god->LoadU32();
  face.index[0] = (U16) god->LoadU32();
  face.index[1] = (U16) god->LoadU32();
}
//----------------------------------------------------------------------------

void Mesh::LoadMRMVertex( GodFile *god, MRM::Vertex &vertex)
{
	vertex.newFaceCount = (U16) god->LoadU32();
	vertex.newNormCount = (U16) god->LoadU32();
	vertex.newTextCount = (U16) god->LoadU32();
  vertex.faceCount = (U16) god->LoadU32();

  for (U32 i = 0; i < vertex.faceCount; i++)
  {
	  LoadMRMFace( god, vertex.face[i]);
  }
}
//----------------------------------------------------------------------------

MRM * Mesh::LoadMRM( GodFile *god, const MeshRoot &root)
{
  MRM *mrm = new MRM;

  mrm->vertCount = (U16) god->LoadU32();
	U32 faceCount = god->LoadU32();
	U32 normCount = god->LoadU32();
  U32 textCount = god->LoadU32();

  faceCount, normCount, textCount;
  ASSERT( mrm->vertCount == root.vertices.count
       && faceCount == root.faces.count
       && normCount == root.normals.count
       && textCount == root.uvs.count);

  mrm->faceCount = (U16)god->LoadU32();

  mrm->maxVertCount = (U16) god->LoadU32();
  mrm->minVertCount = (U16) god->LoadU32();

  if (god->version >= 7)
  {
    mrm->minMinVertCount = (U16) god->LoadU32();
  }
  else
  {
    mrm->minMinVertCount = mrm->minVertCount;
  }
  if (mrm->minMinVertCount > mrm->vertCount)
  {
    mrm->minMinVertCount = mrm->vertCount;
  }
  if (mrm->minVertCount > mrm->vertCount)
  {
    mrm->minVertCount = mrm->vertCount;
  }

  mrm->Alloc( mrm->faceCount, mrm->vertCount);
  MRM::Face *face = mrm->vertex[0].face;

  for (U32 i = 0; i < root.vertices.count; i++)
  {
    // setup pointers to alloced faceupdate mem
    mrm->vertex[i].face = face;

    LoadMRMVertex( god, mrm->vertex[i]);

    face += mrm->vertex[i].faceCount;
  }

  if (mrm->vertCount != root.vertices.count)
  {
    ERR_FATAL( ("Bad mrm load mrm %s", root.xsiName.str));
  }

  S32 fCount = root.faces.count;
  for (S32 ii = mrm->vertCount - 1; ii >= 0; ii--)
  {
    MRM::Vertex & vertex = mrm->vertex[ii];

    fCount -= vertex.newFaceCount;

    if (fCount < 0)
    {
      ERR_FATAL( ("Bad mrm load vertex %s", root.xsiName.str));
    }
  }

  return mrm;
}
//----------------------------------------------------------------------------

void Mesh::SaveMRMFace( GodFile *god, const MRM::Face &face)
{
  God::Save(*god, U32(face.face));
  God::Save(*god, U32(face.token));
  God::Save(*god, U32(face.index[0]));
  God::Save(*god, U32(face.index[1]));
}
//----------------------------------------------------------------------------

void Mesh::SaveMRMVertex( GodFile *god, const MRM::Vertex &vertex)
{
  God::Save(*god, U32(vertex.newFaceCount));
	God::Save(*god, U32(vertex.newNormCount));
	God::Save(*god, U32(vertex.newTextCount));
  God::Save(*god, U32(vertex.faceCount));

  for (U32 i = 0; i < vertex.faceCount; i++)
  {
	  SaveMRMFace( god, vertex.face[i]);
  }
}
//----------------------------------------------------------------------------

void Mesh::SaveMRM( GodFile *god, const MeshRoot &root)
{
  if (!root.mrm)
  {
    return;
  }

  God::Save(*god, U32(0xF18F2BDE)); // "MrmUpdates"

	God::Save(*god, U32(root.mrm->vertCount));
	God::Save(*god, U32(root.faces.count));
	God::Save(*god, U32(root.normals.count));
	God::Save(*god, U32(root.uvs.count));
  God::Save(*god, U32(root.mrm->faceCount));
  
  God::Save(*god, U32(root.mrm->maxVertCount));
  God::Save(*god, U32(root.mrm->minVertCount));
  God::Save(*god, U32(root.mrm->minMinVertCount));

  for (U32 i = 0; i < root.mrm->vertCount; i++)
  {
    SaveMRMVertex( god, root.mrm->vertex[i]);
  }
}
//----------------------------------------------------------------------------

Bool MeshRoot::SaveScale( const char *_fileName)
{
  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;
  Dir::PathExpand( _fileName, drive, dir, name, ext);  

  BuffString buff;
  Mesh::Manager::MakeName( buff, name.str, scale);
  Utils::Strcat( buff.str, ".god");

  return Save( buff.str);
}
//----------------------------------------------------------------------------

Bool MeshRoot::Save( const char * _fileName)
{
  Dir::MakeFull( Vid::Var::godFilePath);

  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;
  Dir::PathExpand( _fileName, drive, dir, name, ext);  

  FilePath path;
  Dir::PathMake( path, NULL, Vid::Var::godFilePath, name.str, "god");

  // Create a block file
  BlockFile bFile;
  if (!bFile.Open(path.str, BlockFile::CREATE, FALSE))
  {
    LOG_ERR((bFile.LastError()))
    return (FALSE);
  }

  // Save meshroot block
  if (bFile.OpenBlock(MeshRootBlock12))
  {
    GodFile god(&bFile);

    fileName.Set( name.str);
    Save(&god);

    bFile.CloseBlock();

    const Mesh *mesh = FindMesh("SP-0");

    // Save shadow plane
    if (mesh)
    {
      shadowPlane = TRUE;

      if (bFile.OpenBlock(ShadowPlaneBlock))
      {
        God::SaveArray4(god, mesh->local->vertices);
        God::SaveArray4(god, mesh->local->colors);

        bFile.CloseBlock();
      }
    }
    bFile.Close();

    LOG_DIAG( ("Saved god file for %s", name.str) );

    return (TRUE);
  }
  else
  {
    bFile.Close();

    return (FALSE);
  }
}
//----------------------------------------------------------------------------

Bool MeshRoot::Save( GodFile *god)
{
  god->SaveStr(xsiName.str);

  God::Save(*god, bigBounds);

  God::Save(*god, scale);
  God::Save(*god, (U32) quickLight);
  God::Save(*god, (U32) shadowPlane);
  God::Save(*god, (U32) envMap);
  U32 texT = U32(texTimer * 1000);
  God::Save(*god, texT);

  God::Save(*god, shadowInfo.radxRender);
  God::Save(*god, shadowInfo.radyRender);
  God::Save(*god, shadowInfo.p2);

  God::Save(*god, (U32) hasTread);
  God::Save(*god, (U32) hasControl);

  God::Save(*god, shadowType);
  God::Save(*god, shadowRadius);
  God::Save(*god, treadPerMeter);

  God::SaveArray4(*god, vertices);
  God::SaveArray4(*god, normals);
  God::SaveArray4(*god, uvs);
  God::SaveArray4(*god, colors);

  God::SaveArray(*god, faces);
  God::SaveArray(*god, buckys);
  God::SaveArray(*god, vertToState);

//  God::SaveArray(*god, groups);
//  God::SaveArray4(*god, indices);
  God::SaveArray(*god, planes);
  
  God::SaveArray(*god, stateMats);

  Array<AnimKey> keys;
  keys.Alloc( states.count);
  U32 i;
  for (i = 0; i < states.count; i++)
  {
    keys[i] = states[i];
  }
  God::SaveArray(*god, keys);

#if 0
  // environment mapping data     // FIXME
  God::Save(*god, U32(0));    // empty array
  God::Save(*god, U32(0));    // empty array
  God::Save(*god, 0);
#endif

  God::Save(*god, U32(animCycles.GetCount()));
  for (NBinTree<AnimList>::Iterator li(&animCycles); *li; li++)
  {
    Mesh::SaveAnimCycle( god, *(*li));
  }

  God::Save( *god, mrmFactor);
  if (useMrm)
  {
    Mesh::SaveMRM( god, *this);
  }

  // save mesh hierarchy 
  Mesh::Save( god);

  // chunks
  God::Save( *god, chunkIndex);
  God::Save( *god, chunks.count);
  for (i = 0; i < chunks.count; i++)
  {
    chunks[i]->Save( god);
  }

  // End of mesh marker
  God::Save(*god, U32(0xE3BB47F1)); // "EndOfMeshRoot"

  return TRUE;
}
//----------------------------------------------------------------------------

MeshRoot *Mesh::Manager::FindLoadScale( const char *godFileName)
{
  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;
  Dir::PathExpand( godFileName, drive, dir, name, ext);

  MeshRoot *root = NULL;

  // check for a file without the scale name first
  root = Find( name.str);
  if (root)
  {
    return root;
  }

  FilePath path;
  Dir::PathMake( path, NULL, NULL, name.str, "god");

  BlockFile bFile;
  Bool exists = bFile.Open( path.str, BlockFile::READ, FALSE);
  bFile.Close();

  if (exists)
  {
    // no scale name
    //
    root = FindLoad( path.str);
  }
  else
  {
    // look for a scale name
    //
    BuffString buff;
    Mesh::Manager::MakeName( buff, name.str, Vid::renderState.scaleFactor);
    Utils::Strcat( buff.str, ".god");

    root = FindLoad( buff.str);
  }

  return root;
}
//----------------------------------------------------------------------------

MeshRoot *Mesh::Manager::FindLoad( const char *godFileName)
{
  FileDrive drive;
  FileDir dir;
  FileName name;
  FileExt ext;
  Dir::PathExpand( godFileName, drive, dir, name, ext);

  MeshRoot *root = Find( name.str);
  if (root)
  {
    return root;
  }

//#ifdef DEVELOPMENT
//  U32 t1 = Clock::Time::Ms();
//#endif

  // Open the block file
  BlockFile bFile;

  FilePath path;
  Dir::PathMake( path, NULL, NULL, name.str, "god");

  if (!bFile.Open( path.str, BlockFile::READ, FALSE))
  {
    //LOG_ERR((bFile.LastError()))
    return (FALSE);
  }

  // Open meshroot block
  U32 size;
  U32 ver = 0;

  if (bFile.OpenBlock(MeshRootBlock1, FALSE, &size))
  {
    ver = 1;
  }
  else if (bFile.OpenBlock(MeshRootBlock2, FALSE, &size))
  {
    ver = 2;
  }
  else if (bFile.OpenBlock(MeshRootBlock3, FALSE, &size))
  {
    ver = 3;
  }
  else if (bFile.OpenBlock(MeshRootBlock5, FALSE, &size))
  {
    ver = 5;
  }
  else if (bFile.OpenBlock(MeshRootBlock6, FALSE, &size))
  {
    ver = 6;
  }
  else if (bFile.OpenBlock(MeshRootBlock7, FALSE, &size))
  {
    ver = 7;
  }
  else if (bFile.OpenBlock(MeshRootBlock8, FALSE, &size))
  {
    ver = 8;
  }
  else if (bFile.OpenBlock(MeshRootBlock9, FALSE, &size))
  {
    ver = 9;
  }
  else if (bFile.OpenBlock(MeshRootBlock10, FALSE, &size))
  {
    ver = 10;
  }
  else if (bFile.OpenBlock(MeshRootBlock11, FALSE, &size))
  {
    ver = 11;
  }
  else if (bFile.OpenBlock(MeshRootBlock12, FALSE, &size))
  {
    ver = 12;
  }

  if (ver)
  {
    GodFile god(bFile.GetBlockPtr(), size, ver);

    root = new MeshRoot();
    root->Load( &god);

    if (!SetupRoot( *root, name.str))
    {
      ERR_FATAL( ("Error loading %s", name.str));
    }
    root->Check();  // FIXME

    if (root->shadowType == MeshRoot::shadowGENERIC)
    {
      root->RenderShadowTextureGeneric();
    }

    bFile.CloseBlock();
    bFile.Close();

//#ifdef DEVELOPMENT
    //U32 elapsed = Clock::Time::Ms() - t1;
    //LOG_DIAG(("%6dms Loading [%s]", elapsed, path.str))
//#endif
  }

  return root;
}
//----------------------------------------------------------------------------

Bool MeshRoot::Load( GodFile *god)
{
  god->LoadStr(xsiName.str, MAX_GAMEIDENT);

  if (god->version > 1)
  {
    // new versions
    //
    God::Load(*god, bigBounds);
  }
  else
  {
    Sphere bigSphere;
    God::Load(*god, bigSphere);
    bigBounds.Set( bigSphere);
  }
  fixedBounds = bigBounds;    // backup

  God::Load(*god, scale);
  quickLight  = god->LoadU32();
  shadowPlane = god->LoadU32();

  if (god->version > 1)
  {
    envMap      = god->LoadU32();

    U32 texT;
    God::Load(*god, texT);
    texTimer = F32(texT) * .001f;
  }

  if (god->version > 11)
  {
    God::Load(*god, shadowInfo.radxRender);
    God::Load(*god, shadowInfo.radyRender);
    God::Load(*god, shadowInfo.p2);
    shadowInfo.p1 = shadowInfo.p2;
  }

  if (god->version > 5)
  {
    hasTread  = god->LoadU32();
    hasControl = god->LoadU32();
  }

  if (god->version > 10)
  {
    shadowType = god->LoadU32();
  }
  else if (god->version > 8)
  {
    shadowType = god->LoadU32() ? shadowSEMILIVE : shadowOVAL;
  }
  God::Load(*god, shadowRadius);

  if (god->version > 7)
  {
    God::Load(*god, treadPerMeter);
  }

  God::LoadArray4(*god, vertices, Vid::renderState.maxVerts);
  God::LoadArray4(*god, normals, Vid::renderState.maxVerts);
  God::LoadArray4(*god, uvs, Vid::renderState.maxVerts);
  God::LoadArray4(*god, colors, Vid::renderState.maxVerts);

  God::LoadArray(*god, faces, Vid::renderState.maxTris);
  God::LoadArray(*god, buckys, MAXBUCKYS);
  God::LoadArray(*god, vertToState, Vid::renderState.maxVerts);
  God::LoadArray(*god, planes, Vid::renderState.maxTris);
  
  God::LoadArray(*god, stateMats, MAXMESHPERGROUP);

  Array<AnimKey> keys;
  God::LoadArray(*god, keys, MAXMESHPERGROUP);

  states.Alloc( keys.count);
  U32 i;
  for (i = 0; i < keys.count; i++)
  {
    states[i] = keys[i];
  }

  for (i = 0; i < faces.count; i++)
  {
    faces[i].index = U16(i);
  }

  if (god->version < 2)
  {
    // environment mapping data
    Array4<UVPair,4> uvs2;
    God::LoadArray4(*god, uvs2);
    uvs2.Release();
    Array<BucketLock> buckys2;
    God::LoadArray(*god, buckys2, MAXBUCKYS);
    buckys2.Release();
    Color envColor = god->LoadU32();
  }

  U32 animCount;
  God::Load(*god, animCount);

  for (i = 0; i < animCount; i++)
  {
    AnimList *animCycle = new AnimList;
    Mesh::LoadAnimCycle( god, *animCycle);
    SetupAnimCycle( *animCycle, animCycle->name.str);
  }

  God::Load( *god, mrmFactor);

  if (god->Peek() == 0xF18F2BDE) // "MrmUpdates"
  {
    god->Skip(4);
    mrm = Mesh::LoadMRM( god, *this);
  }
  else
  {
    mrm = NULL;
  }

  // load mesh hierarchy 
  Mesh::Load( god, *this, NULL, TRUE);

  states[0].ClearData();

  // chunks
  chunkIndex = god->LoadU32();
  U32 chunkCount  = god->LoadU32();
  if (chunkCount)
  {
    chunkify = TRUE;

    chunks.Alloc( chunkCount);
    for (i = 0; i < chunkCount; i++)
    {
      chunks[i] = new MeshRoot;
      chunks[i]->Load( god);
      chunks[i]->isChunk = TRUE;

      static U32 chunknum = 0;
      NameString namebuf;
      sprintf( namebuf.str, "chunk%d", chunknum);
      chunknum++;

      if (!Mesh::Manager::SetupRoot( *chunks[i], namebuf.str))
      {
        ERR_FATAL( ("Error loading %s", namebuf.str));
      }
    }
  }

  // check end of mesh marker
  if (god->LoadU32() !=  0xE3BB47F1) // "EndOfMeshRoot"
  {
    ERR_FATAL(("Corrupt GOD file"));
  }

  // general setup
  //
  Setup();

  godLoad = TRUE;

  SetWorldAll( Matrix::I);

  if (shadowRadius < .5f)
  {
    shadowRadius = 0;
  }

  return TRUE;
}
//----------------------------------------------------------------------------

void Mesh::Save( GodFile *god)
{
  god->SaveStr(name.str);
  God::Save(*god, name.index);

  God::Save(*god, renderFlags);

  // legacy nested bounds support
  //
  Sphere s;
  s.ClearData();
  God::Save(*god, s);

  God::Save(*god, ObjectMatrix());

  God::Save(*god, local ? 1 : 0);
  if (local)
  {
    God::SaveArray4(*god, local->vertices);
    God::SaveArray4(*god, local->normals);
    God::SaveArray4(*god, local->uvs);
    God::SaveArray4(*god, local->colors);
    God::SaveArray( *god, local->groups);
    God::SaveArray4(*god, local->indices);
  }

  NList<FamilyNode>::Iterator kids(&children);
  Mesh * node;
  while ((node = (Mesh *) kids++) != NULL)
  {
    if (node->IsMesh())
    {
      God::Save(*god, U32(0xF74C51EE)); // "Child"
      node->Save( god);
    }
  }
  God::Save(*god, U32(0xA93EB864)); // "EndOfMesh"
}
//----------------------------------------------------------------------------

void Mesh::Load( GodFile *god, MeshRoot &root, Mesh *_parent, Bool isPrimary)
{
  god->LoadStr(name.str, MAX_GAMEIDENT);
  name.Update();

  if (!Utils::Strnicmp( name.str, "cp-", 3))
  {
    isControl = TRUE;
  }

  God::Load(*god, name.index);
  // setup FamilyNode::state and FamilyState::node pointers
  SetState( root.states[name.index]);
  if (_parent)
  {
    _parent->Attach( *this, isPrimary);
  }

  God::Load(*god, renderFlags);

  // legacy nested bounds support
  //
  Sphere s;
  God::Load(*god, s);

  ClearState();

  Matrix matrix;
  God::Load(*god, matrix);
  SetObjectMatrix( matrix);

  Bool hasLocal = 1;
  if (god->version >= 10)
  {
    hasLocal = god->LoadU32();
  }
  if (hasLocal)
  {
    MeshData mdata;

    God::LoadArray4(*god, mdata.vertices);
    God::LoadArray4(*god, mdata.normals);
    God::LoadArray4(*god, mdata.uvs);
    God::LoadArray4(*god, mdata.colors);

    God::LoadArray( *god, mdata.groups);
    God::LoadArray4(*god, mdata.indices);

    if (mdata.vertices.count)
    {
      SwapLocal( mdata);
    }
  }

  // Load child
  while (god->Peek() == 0xF74C51EE) // "Child"
  {
    god->Skip(4);

    Mesh *newMesh = new Mesh;
    newMesh->Load( god, root, this, TRUE);
  }

  if (god->LoadU32() != 0xA93EB864) // "EndOfMesh"
  {
    ERR_FATAL(("Corrupt GOD file"));
  }

  // Load siblings
  while (god->Peek() == 0xB8990880) // "Sibling"
  {
    god->Skip(4);

    Mesh *newMesh = new Mesh;
    newMesh->Load( god, root, (Mesh *) Parent(), FALSE);
  }
}
//----------------------------------------------------------------------------

