///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshConfig
//
// 20-JAN-1999
//

#include "vid_private.h"
#include "meshconfig.h"
#include "stdload.h"
//----------------------------------------------------------------------------

void MeshConfig::ClearData()
{
  mrmMin = 7;
  meshRoot = NULL;
}
//----------------------------------------------------------------------------

void MeshConfig::Release()
{
  animations.DisposeAll();
}
//----------------------------------------------------------------------------

void MeshConfig::Configure(FScope *fScope)
{
  FScope *sScope;

  // Get name of god file, or make it a null object
  if ((sScope = fScope->GetFunction("GodFile", FALSE)) != NULL)
  {
    name = StdLoad::TypeString(sScope);
    isNullMesh = FALSE;
  }
  else
  { 
    name = "<Null>";
    isNullMesh = TRUE;
  }
  fileName = name.str;

  // Scaling factor
  scale = StdLoad::TypeF32(fScope, "Scale", 1.0F, Range<F32>(0.0001f, 1000.0f));

  shadowGeneric= StdLoad::TypeU32(fScope, "ShadowGeneric", 0);
  shadowSemiLive= StdLoad::TypeU32(fScope, "ShadowSemiLive", 0);
  shadowLive   = StdLoad::TypeU32(fScope, "ShadowLive", 0);
  shadowRadius = StdLoad::TypeF32(fScope, "ShadowRadius", -1.0F);
  mrmMaxFactor = StdLoad::TypeF32(fScope, "MaxVertFactor", 1.0f, Range<F32>(0.001f, 1.0f));
  mrmMin = StdLoad::TypeU32(fScope, "MinVertCount",  mrmMin);

  treadPerMeter = StdLoad::TypeF32(fScope, "TreadPerMeter", 1.0F);

  // MRM generation
  mrmFactor = StdLoad::TypeF32(fScope, "MrmFactor", 0.0f, Range<F32>(0.0f, 10.0f));
  mrm = StdLoad::TypeU32(fScope, "Mrm", mrmFactor > 0.0f ? TRUE : FALSE);

  chunkify   = StdLoad::TypeU32(fScope, "Chunkify", FALSE);
  quickLight = StdLoad::TypeU32(fScope, "QuickLight", FALSE);
  envMap     = StdLoad::TypeU32(fScope, "EnvMap", FALSE);

  texTimer   = StdLoad::TypeF32(fScope, "TexTimerF", .1f);

  // Clear mesh root
  meshRoot = NULL;
}
//----------------------------------------------------------------------------

void MeshConfig::ConfigureAnim(FScope *pScope)
{
  FScope *sScope, *fScope = pScope->GetFunction("GodFile", FALSE);

  if (fScope)
  {
    name = StdLoad::TypeString(fScope);
    isNullMesh = FALSE;

    return;
  }
  fScope = pScope->GetFunction("Mesh");

  Configure( fScope);

  // Get the animation list
  // check parent scope first; legacy support
  sScope = pScope->GetFunction("Animations", FALSE);

  if (!sScope)
  {
    // otherwise try the mesh scope
    sScope = fScope->GetFunction("Animations", FALSE);
  }

  if (sScope)
  {
    FScope *ssScope;
    while ((ssScope = sScope->NextFunction()) != NULL)
    {
      switch (ssScope->NameCrc())
      {
        case 0x9F1D54D0: // "Add"
        {
          Animation *anim = new Animation;
          anim->name = ssScope->NextArgString();
          switch (Crc::CalcStr(ssScope->NextArgString()))
          {
            case 0xFC38C807: // "Loop"
              anim->type = animLOOP;
              break;

            case 0xBD222994: // "2Way"
              anim->type = anim2WAY;
              break;

            case 0xDD559BFA: // "1Way"
              anim->type = anim1WAY;
              break;

            case 0x838CB20C: // "Control"
              anim->type = animCONTROL;
              break;

            default:
              ERR_CONFIG(("Unknown animation type"));
              break;
          }
          // Animation speed
          anim->animSpeed = StdLoad::TypeF32(ssScope, "AnimSpeed", 22.0f);
          anim->moveSpeed = StdLoad::TypeF32(ssScope, "MoveSpeed", 0.0f);

          if (anim->animSpeed == 0.0f || anim->moveSpeed == 0.0f)
          {
            anim->framesPerMeter = 0.0f;
          }
          else
          {
            anim->framesPerMeter = anim->animSpeed / anim->moveSpeed;
          }

          anim->controlFrame = StdLoad::TypeF32(ssScope, "ControlFrame", 0.0f);

          animations.Append(anim);
          break;
        }
      }
    }
  }
}
//----------------------------------------------------------------------------

// checks for a god file
//
void MeshConfig::PostLoad()
{
  meshRoot = Mesh::Manager::FindRead(isNullMesh ? NULL : name.str, scale, mrm, fileName.str);

  PostConfig();

#if 0
  if (!isNullMesh && !meshRoot->doLoadGod)
  {
    meshRoot->SaveScale( name.str);

    meshRoot->doLoadGod = TRUE;   // its optimized now
  }
#endif
}
//----------------------------------------------------------------------------

// doesn't check for a god file
//
void MeshConfig::PostLoadXSI()
{
  Bool doLoadGod = Vid::Var::doLoadGod;
  Vid::Var::doLoadGod = FALSE;

  meshRoot = Mesh::Manager::FindRead(isNullMesh ? NULL : name.str, scale, Vid::Var::doMrmGen, fileName.str);

  Vid::Var::doLoadGod = doLoadGod;

  PostConfig( FALSE);
}
//----------------------------------------------------------------------------

void MeshConfig::PostConfig( Bool optimize) // = TRUE
{
  // We must find the root mesh
  if (!meshRoot)
  {
    ERR_CONFIG(("Unable to read mesh file '%s'", name.str));
  }
  if (meshRoot->godLoad)
  {
    Setup( *meshRoot);
    return;
  }

  meshRoot->chunkify = chunkify;

  if (!meshRoot->godLoad)
  {
    if (!isNullMesh)
    {
      FileString strbuf;
      FileDrive drive;
      FileDir dir;
      FileName fname;
      FileExt ext;
      Dir::PathExpand(name.str, drive, dir, fname, ext);

      // Load the animations only if not loaded from a god file
      for (List<Animation>::Iterator n(&animations); *n; n++)
      {
        Animation &anim = *(*n);

        Utils::Sprintf(strbuf.str, MAX_FILEIDENT, "%s-%s%s", fname.str, anim.name.str, ext.str);

        AnimList *animList = Mesh::Manager::ReadAnimCycle(strbuf.str, *meshRoot, anim.name.str);
        if (!animList)
        {
          ERR_CONFIG(("Can't find animation file %s for %s", anim.name.str, meshRoot->fileName.str) );
        }
        animList->SetType( anim.type);
        animList->animSpeed = anim.animSpeed;
        animList->framesPerMeter = anim.framesPerMeter;
        animList->controlFrame = anim.controlFrame;
      }

      AnimList *animList = meshRoot->FindAnimCycle( DEFCYCLENAME);
      if (animList)
      {
        animList->SetType( animLOOP);
        animList->animSpeed = 0.0f;
        animList->framesPerMeter = 0.0f;
        animList->controlFrame = 0.0f;
      }

      if (optimize)
      {
        meshRoot->CompressStates();
      }
      meshRoot->CalcBoundingSphere();
      meshRoot->SetAnimCycle();

      if (optimize && chunkify)
      {
        meshRoot->Chunkify();
      }
    }
  }
  meshRoot->texTimer = texTimer;

  if (shadowRadius >= 0.0f)
  {
    meshRoot->shadowRadius = shadowRadius;
    if (shadowLive)
    {
      meshRoot->shadowType = MeshRoot::shadowLIVE;
    }
    else if (shadowSemiLive)
    {
      meshRoot->shadowType = MeshRoot::shadowSEMILIVE;
    }
    else if (shadowGeneric)
    {
      meshRoot->shadowType = MeshRoot::shadowGENERIC;
    }
    else
    {
      meshRoot->shadowType = MeshRoot::shadowOVAL;
    }
  }
  meshRoot->treadPerMeter = treadPerMeter;

  if (meshRoot->mrm)
  {
    if (mrmMaxFactor > 0.0f)
    {
      meshRoot->mrm->maxVertCount = (U16)((F32)meshRoot->vertices.count * mrmMaxFactor);
    }
    meshRoot->mrm->minVertCount = (U16) mrmMin;
  }
  meshRoot->useMrm = mrm;
  meshRoot->envMap = envMap;

  // Set the mrm factor
  meshRoot->mrmFactor = mrmFactor;

  // Set lighting type
  meshRoot->quickLight = quickLight;

#if 0
  else
  {
    // FIXME
    AnimList *animList = meshRoot->FindAnimCycle( DEFCYCLENAME);
    if (animList)
    {
      idleSpeed = animList->animSpeed;
    }
    animList = meshRoot->FindAnimCycle( "Move");
    if (animList)
    {
      animSpeed = animList->animSpeed;
    }
  }
#endif

  meshRoot->fileName = fileName.str;
}
//----------------------------------------------------------------------------

void MeshConfig::Optimize()
{
  meshRoot->CompressStates();
  meshRoot->CalcBoundingSphere();
  meshRoot->SetAnimCycle();

  if (chunkify)
  {
    meshRoot->Chunkify();
  }
}
//----------------------------------------------------------------------------

void MeshConfig::Setup( MeshRoot &root)
{
  Release();

  meshRoot = &root;

  name = root.xsiName.str;

  Utils::Strcat( name.str, ".xsi");

  fileName = root.fileName.str;

  isNullMesh = root.vertices.count == 0 ? TRUE : FALSE;
  scale = root.scale;

  chunkify = root.chunkify;

  shadowLive = root.shadowType == MeshRoot::shadowLIVE;
  shadowSemiLive= root.shadowType == MeshRoot::shadowSEMILIVE;
  shadowGeneric = root.shadowType == MeshRoot::shadowGENERIC;
  shadowRadius  = root.shadowRadius;
  treadPerMeter = root.treadPerMeter;

  if (root.mrm)
  {
    mrmMaxFactor = (F32) root.mrm->maxVertCount / (F32) root.vertices.count;
    mrmMin  = root.mrm->minVertCount;
  }
  else
  {
    mrmMaxFactor = 1.0f;
    mrmMin  = root.vertices.count;
  }
  mrm = root.useMrm;

  mrmFactor  = root.mrmFactor;
  quickLight = root.quickLight;
  envMap     = root.envMap;

  texTimer   = root.texTimer;

  NBinTree<AnimList>::Iterator li(&root.animCycles); 
  for (!li; *li; li++)
  {
   
    AnimList *animCycle = (*li);

    Animation *anim = new Animation;

    anim->type = animCycle->type;
    anim->name = animCycle->name.str;
    anim->animSpeed = animCycle->animSpeed;
    anim->framesPerMeter = animCycle->framesPerMeter;
    anim->moveSpeed = animCycle->framesPerMeter == 0 ? 0 : animCycle->animSpeed / animCycle->framesPerMeter;
    anim->controlFrame = animCycle->controlFrame;

    animations.Append(anim);
  }
}
//----------------------------------------------------------------------------

MeshRoot *MeshConfig::Load( const char *_fileName)
{
  PTree tree;

  // Read the file
  if (!tree.AddFile( _fileName))
  {
    return NULL;
  }

  FileDrive drive;
  FileDir dir;
  FileName fname;
  FileExt ext;
  Dir::PathExpand( _fileName, drive, dir, fname, ext);

  FScope *fScope = tree.GetGlobalScope();

  if (!Utils::Stricmp( ext.str, ".cfg"))
  {
    fScope = fScope->GetFunction("CreateObjectType");
    fScope = fScope->GetFunction("MapObj");
  }

  ConfigureAnim( tree.GetGlobalScope());
  fileName = fname.str;

  PostLoad();

  return meshRoot;
}
//----------------------------------------------------------------------------

MeshRoot *MeshConfig::LoadXSI( const char *_fileName)
{
  PTree tree;

  // Read the file
  if (!tree.AddFile( _fileName))
  {
    return NULL;
  }

  FileDrive drive;
  FileDir dir;
  FileName fname;
  FileExt ext;
  Dir::PathExpand( _fileName, drive, dir, fname, ext);

  FScope *fScope = tree.GetGlobalScope();

  if (!Utils::Stricmp( ext.str, ".cfg"))
  {
    fScope = fScope->GetFunction("CreateObjectType");
    fScope = fScope->GetFunction("MapObj");
  }

  ConfigureAnim( fScope);
  fileName = fname.str;

  PostLoadXSI();

  return meshRoot;
}
//----------------------------------------------------------------------------

Bool MeshConfig::Save( const char *_fileName)
{
  PTree tree;

  // Add the top level scope
  FScope *sScope, *fScope = tree.GetGlobalScope()->AddFunction("Mesh");

  // Get name of god file, or make it a null object
  if (isNullMesh)
  {
    StdSave::TypeString(fScope, "GodFile", "<Null>");
  }
  else
  {
    StdSave::TypeString(fScope, "GodFile", name.str);
  }

  StdSave::TypeF32(fScope, "Scale", scale);
  StdSave::TypeU32(fScope, "ShadowLive",    (U32) shadowLive);
  StdSave::TypeU32(fScope, "ShadowSemiLive",    (U32) shadowSemiLive);
  StdSave::TypeU32(fScope, "ShadowGeneric",    (U32) shadowGeneric);
  StdSave::TypeF32(fScope, "ShadowRadius",  shadowRadius);
  StdSave::TypeF32(fScope, "TreadPerMeter", treadPerMeter);

  StdSave::TypeF32(fScope, "MaxVertFactor", mrmMaxFactor);
  StdSave::TypeU32(fScope, "MinVertCount",  mrmMin);

  StdSave::TypeF32(fScope, "MrmFactor", mrmFactor);
  StdSave::TypeU32(fScope, "Mrm", mrm);

  StdSave::TypeU32(fScope, "Chunkify",   chunkify);
  StdSave::TypeU32(fScope, "QuickLight", quickLight);
  StdSave::TypeU32(fScope, "EnvMap",     envMap);

//  StdSave::TypeColor(fScope, "EnvMapColor", envColor);
//  StdSave::TypeF32(fScope, "AnimationSpeed", animSpeed);
//  StdSave::TypeF32(fScope, "IdleSpeed", idleSpeed);

  StdSave::TypeF32(fScope, "TexTimerF", texTimer);

  if (animations.GetCount())
  {
    // Get the animation list
    // check parent scope first; legacy support
    sScope = fScope->AddFunction("Animations");

    ASSERT( sScope);

    for (List<Animation>::Iterator n(&animations); *n; n++)
    {
      Animation *anim = (*n);

      if (!Utils::Stricmp( anim->name.str, DEFCYCLENAME))
      {
        continue;
      }

      FScope *ssScope = sScope->AddFunction("Add");

      ssScope->AddArgString( anim->name.str);
      switch (anim->type)
      {
      case animLOOP:
        ssScope->AddArgString( "Loop");
        break;
      case anim2WAY:
        ssScope->AddArgString( "2Way");
        break;
      case anim1WAY:
        ssScope->AddArgString( "1Way");
        break;
      case animCONTROL:
        ssScope->AddArgString( "Control");
        break;
      }
      StdSave::TypeF32(ssScope, "AnimSpeed", anim->animSpeed);
      StdSave::TypeF32(ssScope, "MoveSpeed", anim->moveSpeed);
      StdSave::TypeF32(ssScope, "ControlFrame", anim->controlFrame);
    }
  }

  Dir::MakeFull( Vid::Var::gfgFilePath);

  FileDrive drive;
  FileDir dir;
  FileName fname;
  FileExt ext;
  Dir::PathExpand( _fileName, drive, dir, fname, ext);

  fileName = fname.str;

  FilePath path;
  Dir::PathMake( path, NULL, Vid::Var::gfgFilePath, fname.str, "gfg");

  return (tree.WriteTreeText( path.str));
}
//----------------------------------------------------------------------------

MeshConfig::Animation *MeshConfig::FindAnim( const char *name)
{
  for (List<Animation>::Iterator n(&animations); *n; n++)
  {
    Animation *anim = *n;

    if (!Utils::Stricmp( anim->name.str, name))
    {
      return anim;
    }
  }
  return NULL;
}
//----------------------------------------------------------------------------
