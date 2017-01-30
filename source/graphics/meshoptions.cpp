///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshConfig
//
// 20-JAN-1999
//

#include "meshent.h"
#include "meshview.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_messagebox.h"
#include "babel.h"          // for TRANSLATE
#include "iclistbox.h"
#include "console.h"
#include "terrain_priv.h"
//----------------------------------------------------------------------------

MeshConfig        MeshOptions::meshConfig;

VarString         MeshOptions::name;

VarInteger        MeshOptions::verts;
VarInteger        MeshOptions::tris;

VarFloat          MeshOptions::height;
VarFloat          MeshOptions::width;
VarFloat          MeshOptions::depth;
VarFloat          MeshOptions::radius;

VarString         MeshOptions::fileName;
VarFloat          MeshOptions::scale;

VarInteger        MeshOptions::shadow;
VarInteger        MeshOptions::shadowGeneric;
VarInteger        MeshOptions::shadowSemiLive;
VarInteger        MeshOptions::shadowLive;
VarFloat          MeshOptions::shadowRadius;

VarInteger        MeshOptions::mrm;
VarFloat          MeshOptions::mrmFactor;
VarInteger        MeshOptions::mrmMax;
VarInteger        MeshOptions::mrmMin;

VarInteger        MeshOptions::animLoop;
VarInteger        MeshOptions::anim2Way;
VarInteger        MeshOptions::anim1Way;
VarInteger        MeshOptions::animControl;

VarInteger        MeshOptions::animBlend;

VarString         MeshOptions::cycleName;
VarFloat          MeshOptions::animSpeed;
VarFloat          MeshOptions::moveSpeed;
VarFloat          MeshOptions::animFactor;
VarFloat          MeshOptions::controlFrame;
VarFloat          MeshOptions::targetFrame;

VarInteger        MeshOptions::texAnimActive;
VarFloat          MeshOptions::texAnim;
VarFloat          MeshOptions::treadAnim;

VarInteger        MeshOptions::envMap;
VarInteger        MeshOptions::envColor;

VarInteger        MeshOptions::quickLight;
VarInteger        MeshOptions::chunkify;

MeshRoot *        MeshOptions::root;
MeshEnt  *        MeshOptions::ent;

Bool              MeshOptions::reset;
Bool              MeshOptions::lock;
Bool              MeshOptions::vertBrush;
//-----------------------------------------------------------------------------

U32 MeshOptions::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        U32 id = e.iface.p1;

        switch (id)
        {
          case IControlNotify::Activated:
          {
            Setup( ent);
            break;
          }
          case 0x04E37631: // "Apply"
            break;
          case 0x8C1F21CA: // "Done"
            break;
          case 0x7B23075F: // "Check"
            Check();
            break;
/*
          case 0x8BA3FB72: // "MrmGen"
            if (root)
            {
              MeshView::SetMessage( "Generating MRM data for ", root->fileName.str, "...");

              root->MRMGen( vertBrush ? &ent->selData->verts : NULL);

              ent->UnSelectVerts();
              ent->Reset( *root);
              Setup( ent);

              reset  = TRUE;
            }
            break;
*/
        }
      }
    }
  }

  return (ICWindow::HandleEvent(e));
}
//----------------------------------------------------------------------------

void MeshOptions::Check()
{
  IControl *options = IFace::FindByName( "MeshOptions");

  if (options)
  {
    options->Find( Crc::CalcStr( "vertsedit"))->SetEnabled( FALSE);
    options->Find( Crc::CalcStr( "trisedit"))->SetEnabled( FALSE);

    options->Find( Crc::CalcStr( "heightedit"))->SetEnabled( FALSE);
    options->Find( Crc::CalcStr( "widthedit"))->SetEnabled( FALSE);
    options->Find( Crc::CalcStr( "depthedit"))->SetEnabled( FALSE);
    options->Find( Crc::CalcStr( "radiusedit"))->SetEnabled( FALSE);

    options->Find( Crc::CalcStr( "live"))->SetEnabled( *shadow);
    options->Find( Crc::CalcStr( "semilive"))->SetEnabled( *shadow);
    options->Find( Crc::CalcStr( "generic"))->SetEnabled( *shadow);
//    options->Find( Crc::CalcStr( "shadowedit"))->SetEnabled( *shadow);
    options->Find( Crc::CalcStr( "shadowslider"))->SetEnabled( *shadow);

    options->Find( Crc::CalcStr( "mrmbutton"))->SetEnabled( root->mrm ? TRUE : FALSE);    
    options->Find( Crc::CalcStr( "mrmstatic"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmedit"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmslider"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmmaxstatic"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmmaxedit"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmmaxslider"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmminstatic"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmminedit"))->SetEnabled( *mrm);
    options->Find( Crc::CalcStr( "mrmminslider"))->SetEnabled( *mrm);

    options->Find( Crc::CalcStr( "texanimbutton"))->SetEnabled( ent->hasTexAnim);
    options->Find( Crc::CalcStr( "texanimslider"))->SetEnabled( ent->hasTexAnim);
    options->Find( Crc::CalcStr( "treadanimslider"))->SetEnabled( ent->Root().hasTread);

    IControl *butt = options->Find( Crc::CalcStr( "envmap"));
    if (butt)
    {
      butt->SetEnabled( ent->envMap);
    }

#if 0
    butt = options->Find( Crc::CalcStr( "save"));
    butt->SetEnabled( !root->godLoad);
#endif
  }
}
//----------------------------------------------------------------------------

void MeshOptions::SetupQuick()
{
  verts = root->vertices.count;
  tris  = root->faces.count;

  height = ent->ObjectBoundsRender().Height();
  width  = ent->ObjectBoundsRender().Width();
  depth  = ent->ObjectBoundsRender().Breadth();
  radius = ent->ObjectBoundsRender().Radius();
}
//----------------------------------------------------------------------------

void MeshOptions::Setup( MeshEnt *_ent)
{
  if (!_ent || lock)
  {
    return;
  }
  ent = _ent;
  root = (MeshRoot *) &ent->Root();

  lock = TRUE;

  meshConfig.Setup( *root);

  SetupQuick();

  BuffString buff = meshConfig.fileName.str;
  char * p = strchr( buff.str, ' ');
  if (p)
  {
    *p = '\0';
  }
  name = buff.str;

  U32 minMin;
  if (root->mrm)
  {
    mrmMax = (U32) root->mrm->maxVertCount;
    mrmMin = (U32) root->mrm->minVertCount;
    minMin = (U32) root->mrm->minMinVertCount;
    mrm = root->useMrm;
  }
  else
  {
    mrmMax = (U32) root->vertices.count;
    minMin = mrmMin = 7;
    mrm = FALSE;
  }
  VarSys::SetIntegerRange("god.mrm.max", minMin, root->vertices.count);
  VarSys::SetIntegerRange("god.mrm.min", minMin, root->vertices.count);

  IControl * options = IFace::FindByName( "MeshOptions");

  mrmFactor      = root->mrmFactor;
 
  shadow         = root->shadowRadius == 0.0f ? FALSE : TRUE;
  shadowLive     = root->shadowType == MeshRoot::shadowLIVE ? TRUE : FALSE;
  shadowSemiLive = root->shadowType == MeshRoot::shadowSEMILIVE ? TRUE : FALSE;
  shadowGeneric  = root->shadowType == MeshRoot::shadowGENERIC  ? TRUE : FALSE;
  shadowRadius   = root->shadowType == MeshRoot::shadowGENERIC ? root->shadowInfo.radxRender : root->shadowRadius;

  quickLight   = root->quickLight;
  envColor     = 0xffffffff;
  chunkify     = root->chunkify;

  scale        = root->scale;
  envMap       = FALSE;

  U32 i;
  for (i = 0; i < root->buckys.count; i++)
  {
    BucketLock & bucky = root->buckys[i];

    if (bucky.envMap)
    {
      envMap = TRUE;
      break;
    }
  }

  if (ent->curCycle)
  {
    animSpeed  = ent->curCycle->animSpeed;
//    animFactor = ent->curCycle->animSpeed;
    controlFrame = ent->curCycle->controlFrame;

    MeshConfig::Animation *anim = meshConfig.FindAnim( ent->curCycle->name.str);
    moveSpeed = anim->moveSpeed;

    VarSys::SetFloatRange("app.anim.frame", 1.0f, ent->curCycle->maxFrame + 1.0f);

    Terrain::moveSpeed = *moveSpeed * *animFactor;
    ent->fps = *animSpeed * *animFactor;
  }
  else
  {
    Terrain::moveSpeed = 0;
  }

  if (options)
  {
    ICListBox *animBox = (ICListBox *) options->Find(Crc::CalcStr("AnimCycle"));
    if (animBox)
    {
      // Rebuild the list
      animBox->DeleteAllItems();

      NBinTree<AnimList>::Iterator cli(&root->animCycles); 
      U32 count, current = 1;
      for (!cli, count = 1; *cli; cli++, count++)
      {
        AnimList *animList = (*cli);

        animBox->AddTextItem(animList->name.str, NULL);
        if (animList == ent->curCycle)
        {
          current = count;
          switch (ent->curCycle->type)
          {
          case animLOOP:
            animLoop = TRUE;
            anim2Way = FALSE;
            anim1Way = FALSE;
            animControl = FALSE;
            break;
          case anim1WAY:
            animLoop = FALSE;
            anim2Way = FALSE;
            anim1Way = TRUE;
            animControl = FALSE;
            break;
          case anim2WAY:
            animLoop = FALSE;
            anim2Way = TRUE;
            anim1Way = FALSE;
            animControl = FALSE;
            break;
          case animCONTROL:
            animLoop = FALSE;
            anim2Way = FALSE;
            anim1Way = FALSE;
            animControl = TRUE;
            break;
          }
        }
      }
      animBox->SetSelectedItem( current);
    }

    texAnim = ent->texTimer;
    texAnimActive = ent->textureAnim;

    Check();
  }

  lock = FALSE;
}
//----------------------------------------------------------------------------

void MeshOptions::CmdHandler(U32 pathCrc)
{
  if (lock || !root)
  {
    return;
  }

  switch (pathCrc)
  {
  case 0xDD47AEB2: // "god.save"
  {
    MeshView::SetMessage( "Saving god file: ", *name, "...");

    U32 stateCount = root->states.count;

    meshConfig.Setup( *root);
    meshConfig.Optimize();
    meshConfig.Save( *name);

    // don't allow resaving
    root->godLoad = TRUE;

    S32 cycleID = -1;
    if (ent->curCycle)
    {
      cycleID = (S32) ent->curCycleID;
    }

    // setup root as full mrm
    //
    ent->MRMSetFull();

    delete ent;
    ent = Mesh::Manager::Create( root);
    reset = TRUE;
    Mesh::Manager::curEnt = ent;

    if (cycleID > 0)
    {
      ent->SetAnimCycle( (U32)cycleID);
      ent->fps = *animSpeed * *animFactor;
    }

    BuffString fullname;
    fullname = *name;
    Utils::Strcat( fullname.str, ".god");
    root->Save( *name);

    if (root->states.count != stateCount)
    {
      root->LogHierarchy();
    }

    // reset file system for new saved god files
    FileSys::BuildIndexes();

    Console::ProcessCmd( "app.saved");
    return;
  }

  case 0x6B7FAF86: // "god.chunkify"
    root->chunkify = *chunkify;
    break;
  case 0x0F27FB15: // "god.quicklight"
    root->quickLight = *quickLight;
    ent->SetupRenderProc();
    break;
  case 0xBEE31C4C: // "god.envmap"
    break;

  case 0x91A5B73F: // "god.scale"
    break;

  case 0xB21147F2: // "god.shadow.active"
    shadowRadius = *shadow ? *shadowGeneric ? root->shadowInfo.radxRender : root->shadowRadius : 0.0f;
    break;
  case 0xE7F5E006: // "god.shadow.live"
    lock = TRUE;
    shadowSemiLive = FALSE;
    shadowGeneric  = FALSE;
    if (*shadowLive)
    {
      ent->shadowType = root->shadowType = MeshRoot::shadowLIVE;
    }
    else
    {
      ent->shadowType = root->shadowType = MeshRoot::shadowOVAL;
    }
    lock = FALSE;
    break;
  case 0xBF27F191: // "god.shadow.semilive"
    lock = TRUE;
    shadowLive = FALSE;
    shadowGeneric  = FALSE;
    if (*shadowSemiLive)
    {
      ent->shadowType = root->shadowType = MeshRoot::shadowSEMILIVE;
    }
    else
    {
      ent->shadowType = root->shadowType = MeshRoot::shadowOVAL;
    }
    lock = FALSE;
    break;
  case 0x5518642C: // "god.shadow.generic"
    lock = TRUE;
    shadowLive = FALSE;
    shadowSemiLive = FALSE;
    lock = FALSE;
    if (*shadowGeneric)
    {
      shadowRadius = root->shadowInfo.radxRender;
      ent->shadowType = root->shadowType = MeshRoot::shadowGENERIC;
      root->RenderShadowTextureGeneric( 0xffffffff, RS_BLEND_DEF, 8, TRUE);
    }
    else
    {
      shadowRadius = root->shadowRadius;
      ent->shadowType = root->shadowType = MeshRoot::shadowOVAL;
    }
    break;
  case 0x685AB1E5: // "god.shadow.radius"
    if (*shadowGeneric)
    {
      root->shadowInfo.radxRender = root->shadowInfo.radyRender = *shadowRadius;
    }
    else
    {
      root->shadowRadius = *shadowRadius;
    }
    break;

  case 0xD6AC9AD6: // "god.mrm.active"
    root->useMrm = *mrm;
    break;

  case 0x06D831DC: // "god.mrm.max"
    if (root->mrm)
    {
      root->mrm->maxVertCount = (U16) *mrmMax;
    }
    break;
  case 0xDA07E44C: // "god.mrm.min"
    if (root->mrm)
    {
      root->mrm->minVertCount = (U16) *mrmMin;
//      if (root->mrm->minVertCount < root->mrm
    }
    break;
  case 0xE15B8457: // "god.mrm.factor"
    root->mrmFactor = *mrmFactor;
    break;

  case 0x2059B393: // "god.anim.loop"
    if (ent->curCycle)
    {
      lock = TRUE;

      if (*animLoop)
      {
        ent->curCycle->type = animLOOP;
        anim2Way = FALSE;
        anim1Way = FALSE;
        animControl = FALSE;
      }
      else
      {
        ent->curCycle->type = anim2WAY;
        anim2Way = TRUE;
        anim1Way = FALSE;
        animControl = FALSE;
      }

      lock = FALSE;
    }
    break;
  case 0x61435200: // "god.anim.2way"
    if (ent->curCycle)
    {
      lock = TRUE;

      if (*anim2Way)
      {
        ent->curCycle->type = anim2WAY;
        animLoop = FALSE;
        anim1Way = FALSE;
        animControl = FALSE;
      }
      else
      {
        ent->curCycle->type = anim1WAY;
        animLoop = FALSE;
        anim1Way = TRUE;
        animControl = FALSE;
      }

      lock = FALSE;
    }
    break;
  case 0x0134E06E: // "god.anim.1way"
    if (ent->curCycle)
    {
      lock = TRUE;

      if (*anim1Way)
      {
        ent->curCycle->type = anim1WAY;
        animLoop = FALSE;
        anim2Way = FALSE;
        animControl = FALSE;
      }
      else
      {
        ent->curCycle->type = animLOOP;
        animLoop = TRUE;
        anim2Way = FALSE;
        animControl = FALSE;
      }

      lock = FALSE;
    }
    break;
  case 0xE1DF8328: // "god.anim.control"
    if (ent->curCycle)
    {
      lock = TRUE;

      if (*animControl)
      {
        ent->curCycle->type = animCONTROL;
        ent->SetAnimOverlay( ent->curCycle);
        ent->SetAnimCycle( Crc::CalcStr( "default"));

        animLoop = FALSE;
        anim2Way = FALSE;
        anim1Way = FALSE;

        VarSys::SetFloatRange("god.anim.controlframe", 0.0f, ent->conCycle->endFrame);
        VarSys::SetFloatRange("god.anim.targetframe",  0.0f, ent->conCycle->endFrame);
      }
      else
      {
        ent->conCycle->type = animLOOP;
        ent->SetAnimCycle( *ent->conCycle);
        ent->SetAnimOverlay( (AnimList *)NULL);

        animLoop = TRUE;
        anim2Way = FALSE;
        anim1Way = FALSE;
      }

      lock = FALSE;
    }
    break;

  case 0x9427D6F4: // "god.anim.controlframe"
    if (ent->curCycle)
    {
      ent->curCycle->controlFrame = *controlFrame;
    }
    break;

  case 0x931FB849: // "god.anim.targetframe"
    if (ent->curCycle)
    {
      ent->SetAnimTargetFrame( *targetFrame);
    }
    break;

  case 0x6E7AFA6C: // "god.anim.speed"
    if (ent->curCycle)
    {
      lock = TRUE;

      MeshConfig::Animation *anim = meshConfig.FindAnim( ent->curCycle->name.str);

//      animFactor = *animSpeed;

      anim->animSpeed = *animSpeed;
      anim->framesPerMeter = *moveSpeed == 0 ? 0 : *animSpeed / *moveSpeed;

      ent->curCycle->animSpeed = anim->animSpeed;
      ent->curCycle->framesPerMeter  = anim->framesPerMeter;

      ent->fps = *animSpeed * *animFactor;

      lock = FALSE;
    }
    break;
  case 0x9D18EBDB: // "god.anim.factor"
    if (ent->curCycle)
    {
      ent->fps = *animSpeed * *animFactor;
      Terrain::moveSpeed = *moveSpeed * *animFactor;
    }
    break;
  case 0x2FEF1A31: // "god.anim.fpsinc"
    animFactor = *animFactor + 0.1f;
    break;
  case 0xDF337C13: // "god.anim.fpsdec"
    animFactor = *animFactor - 0.1f;
    break;

  case 0xEFE080A2: // "god.anim.move"
    if (ent->curCycle)
    {
      MeshConfig::Animation *anim = meshConfig.FindAnim( ent->curCycle->name.str);

      anim->moveSpeed = *moveSpeed;
      anim->framesPerMeter = *moveSpeed == 0 ? 0 : *animSpeed / *moveSpeed;
      ent->curCycle->framesPerMeter = anim->framesPerMeter;

      Terrain::moveSpeed = *moveSpeed * *animFactor;
    }
    break;
  case 0x906C82F6: // "god.anim.cycle"
  {
    lock = TRUE;

    MeshConfig::Animation *anim = meshConfig.FindAnim( *cycleName);
    AnimList * cycle  = NULL;
    if (anim)
    {
      if (anim->type == animCONTROL)
      {
        ent->SetAnimOverlay( *cycleName);
        cycle = ent->conCycle;
      }
      else
      {
        if (*animBlend)
        {
          ent->BlendAnimCycle( *cycleName);
        }
        else
        {
          ent->SetAnimCycle( *cycleName);
        }
        cycle = ent->curCycle;
        ent->fps = cycle->animSpeed * *animFactor;
      }
      animSpeed = cycle->animSpeed;
//      animFactor = *animSpeed;
      moveSpeed = anim->moveSpeed;

      IControl *options = IFace::FindByName( "MeshOptions");
      ICListBox *animBox = (ICListBox *) options->Find(Crc::CalcStr("AnimCycle"));

      NBinTree<AnimList>::Iterator cli(&root->animCycles); 
      U32 count;
      for (!cli, count = 1; *cli; cli++, count++)
      {
        AnimList *animList = (*cli);
        if (animList == cycle)
        {
          animBox->SetSelectedItem( count);
        }
      }
      switch (cycle->type)
      {
      case animLOOP:
        animLoop = TRUE;
        anim2Way = FALSE;
        anim1Way = FALSE;
        animControl = FALSE;
        break;
      case anim1WAY:
        animLoop = FALSE;
        anim2Way = FALSE;
        anim1Way = TRUE;
        animControl = FALSE;
        break;
      case anim2WAY:
        animLoop = FALSE;
        anim2Way = TRUE;
        anim1Way = FALSE;
        animControl = FALSE;
        break;
      case animCONTROL:
        animLoop = FALSE;
        anim2Way = FALSE;
        anim1Way = FALSE;
        animControl = TRUE;
        break;
      }
    }
    else
    {
      CON_ERR(( "No animation cycle: %s", *cycleName))
    }

    VarSys::SetFloatRange("app.anim.frame", 1.0f, cycle->maxFrame + 1.0f);
    VarSys::SetFloatRange("god.anim.controlframe", 0.0f, cycle->endFrame);
    VarSys::SetFloatRange("god.anim.targetframe",  0.0f, cycle->endFrame);

    Terrain::moveSpeed = *moveSpeed * *animFactor;

    lock = FALSE;
    break;
  }

  case 0xC5A8F69F: // "god.anim.texactive"
    ent->ActivateTexAnim( *texAnimActive);
    break;
  case 0x1B02CD2F: // "god.anim.texture"
    ent->texTimer  = *texAnim;
    root->texTimer = *texAnim;
    break;
  case 0x6CA7D808: // "god.anim.tread"
    root->treadPerMeter = *treadAnim;
    break;
  //
  }
}
//----------------------------------------------------------------------------

void MeshOptions::Init()
{
  VarSys::RegisterHandler("god", CmdHandler);
  VarSys::RegisterHandler("god.mrm", CmdHandler);
  VarSys::RegisterHandler("god.shadow", CmdHandler);
  VarSys::RegisterHandler("god.anim", CmdHandler);

  VarSys::CreateString("god.name", "", VarSys::DEFAULT, &name);

  VarSys::CreateInteger("god.verts", 0, VarSys::DEFAULT, &verts);
  VarSys::CreateInteger("god.tris", 0, VarSys::DEFAULT, &tris);

  VarSys::CreateFloat("god.height", 0, VarSys::DEFAULT, &height);
  VarSys::CreateFloat("god.width",  0, VarSys::DEFAULT, &width);
  VarSys::CreateFloat("god.depth",  0, VarSys::DEFAULT, &depth);
  VarSys::CreateFloat("god.radius", 0, VarSys::DEFAULT, &radius);

  VarSys::CreateCmd("god.save");
  VarSys::CreateInteger("god.quicklight", FALSE, VarSys::NOTIFY, &quickLight);
  VarSys::CreateInteger("god.envmap", FALSE, VarSys::NOTIFY, &envMap);
  VarSys::CreateInteger("god.chunkify", FALSE, VarSys::NOTIFY, &chunkify);
  VarSys::CreateFloat("god.scale", 1.0f, VarSys::NOTIFY, &scale);
  VarSys::SetFloatRange("god.scale", 0.0001f, 100.0f);

  VarSys::CreateInteger("god.shadow.active", TRUE, VarSys::NOTIFY, &shadow);
  VarSys::CreateInteger("god.shadow.live", FALSE, VarSys::NOTIFY, &shadowLive);
  VarSys::CreateInteger("god.shadow.semilive", FALSE, VarSys::NOTIFY, &shadowSemiLive);
  VarSys::CreateInteger("god.shadow.generic", FALSE, VarSys::NOTIFY, &shadowGeneric);
  VarSys::CreateFloat("god.shadow.radius", 0.0f, VarSys::NOTIFY, &shadowRadius)->SetFloatRange( 0.0f, 22.0f);

  VarSys::CreateInteger("god.mrm.active", TRUE, VarSys::NOTIFY, &mrm);
  VarSys::CreateInteger("god.mrm.max", 0, VarSys::NOTIFY, &mrmMax);
  VarSys::CreateInteger("god.mrm.min", 0, VarSys::NOTIFY, &mrmMin);
  VarSys::CreateFloat("god.mrm.factor", 1.0f, VarSys::NOTIFY, &mrmFactor);
  VarSys::SetFloatRange("god.mrm.factor", 0.001f, 10.0f);

  VarSys::CreateInteger("god.envcolor", 0, VarSys::NOTIFY, &envColor);

  VarSys::CreateInteger("god.anim.loop", 1, VarSys::NOTIFY, &animLoop);
  VarSys::CreateInteger("god.anim.2way", 0, VarSys::NOTIFY, &anim2Way);
  VarSys::CreateInteger("god.anim.1way", 0, VarSys::NOTIFY, &anim1Way);
  VarSys::CreateInteger("god.anim.control", 0, VarSys::NOTIFY, &animControl);
  VarSys::CreateInteger("god.anim.blend", 1, VarSys::NOTIFY, &animBlend);

  VarSys::CreateFloat("god.anim.controlframe", 0, VarSys::NOTIFY, &controlFrame);
  VarSys::SetFloatRange("god.anim.controlframe", 0.0f, 100.0f);
  VarSys::CreateFloat("god.anim.targetframe", 0, VarSys::NOTIFY, &targetFrame);
  VarSys::SetFloatRange("god.anim.targetframe", 0.0f, 100.0f);

  VarSys::CreateString("god.anim.cycle", DEFCYCLENAME, VarSys::NOTIFY, &cycleName);
  VarSys::CreateFloat("god.anim.speed", 20.0f, VarSys::NOTIFY, &animSpeed);
  VarSys::SetFloatRange("god.anim.speed", 0.0f, 100.0f);
  VarSys::CreateFloat("god.anim.move", 0.0f, VarSys::NOTIFY, &moveSpeed);
  VarSys::SetFloatRange("god.anim.move", 0.0f, 40.0f);
  VarSys::CreateFloat("god.anim.factor", 1.0f, VarSys::NOTIFY, &animFactor);
  VarSys::SetFloatRange("god.anim.factor", 0.0f, 1.0f);
  VarSys::CreateCmd("god.anim.fpsinc");
  VarSys::CreateCmd("god.anim.fpsdec");

  VarSys::CreateInteger("god.anim.texactive", 0, VarSys::NOTIFY, &texAnimActive);
  VarSys::CreateFloat("god.anim.texture", .1f, VarSys::NOTIFY, &texAnim)->SetFloatRange( 0, .4f);
  VarSys::CreateFloat("god.anim.tread", 1.0f, VarSys::NOTIFY, &treadAnim)->SetFloatRange( .001f, 2.0f);

  root = NULL;
  ent = NULL;
}
//----------------------------------------------------------------------------

void MeshOptions::Done()
{
  VarSys::DeleteItem("god.anim");
  VarSys::DeleteItem("god.shadow");
  VarSys::DeleteItem("god.mrm");
  VarSys::DeleteItem("god");

  meshConfig.Release();
}
//----------------------------------------------------------------------------
