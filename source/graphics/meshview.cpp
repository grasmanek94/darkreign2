///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshView system
//
// 10-JUN-1999
//

#include "vid_private.h"
#include "light_priv.h"
#include "terrain_priv.h"
#include "mesheffect_system.h"
#include "meshview.h"
#include "meshconfig.h"
#include "main.h"
#include "input.h"
#include "iclistbox.h"
#include "direct.h"
#include <time.h>
#include "console.h"
#include "random.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_messagebox.h"
#include "icwindow.h"
#include "iclistbox.h"
#include "icdroplist.h"
#include "fontsys.h"
#include "input.h"
//----------------------------------------------------------------------------

const F32 OBJZ            = 10.0f;
const F32 INTENSITYMIN		= 0.0f;
const F32 INTENSITYMAX		= 1.0f;
const F32 INTENSITYINC		= 0.1f;

const F32 MOVERATEMAX			= 100.0f;
const F32 MOVERATE			  = 10.0f;
const F32 ROTATERATEMAX		= (PI2 * 2.0f);
const F32 CAMROTATERATE		= (ROTATERATEMAX * 0.22f);
const F32 OBJROTATERATE		= (ROTATERATEMAX * 0.11f);
const F32 DRAG				    = 5.5f;
const F32 NUDGEFACTOR			= 0.20f;
const F32 FASTFACTOR      = 2.0f;

const U32 SINGLESTEPTIME  = CLOCKS_PER_SEC / 6;
const F32 FPSINC          = 0.2f;

const F32 RADMULT         = 3.0f;

const char *DEFBACKNAME   = "engine_sky.bmp";
const char *DEFGROUNDNAME = "ground";

const char *DEFMESHNAME   = "engine_walker.xsi";

const F32 MINCAMROTX      = 0.26f;
//----------------------------------------------------------------------------

extern F32 moveSpeed;
//----------------------------------------------------------------------------

namespace MeshView
{
  class Brush
  {
  public:
    S16 captureCode;

    Point<S32> startMouse, curMouse;

    Vector startPosObj;
    Vector startPosCam;
    Vector startRotCam;

    virtual void Init();
    virtual void Done();

    virtual char * GetIFaceName() 
    {
      return "";
    }

    virtual void Clear() {}
    virtual void Render() {}

    virtual Bool FASTCALL DefaultHandler(Event &e);
    virtual void CmdHandler(U32 pathCrc) { pathCrc; }

    virtual void Setup( Bool on)
    {
      IControl * control = IFace::FindByName( GetIFaceName());
      if (control)
      {
        if (on)
        {
          control->Activate();
        }
        else
        {
          control->Deactivate();
        }
      }
    }
    void Toggle()
    {
      IControl * control = IFace::FindByName( GetIFaceName());
      if (control)
      {
        Setup( !control->IsActive());
      }
    }

  };

  class ViewBrush : public Brush
  {
  public:

    virtual void Init();
    virtual void Done();

    virtual char * GetIFaceName() 
    {
      return "ViewBrushControl";
    }

    virtual Bool FASTCALL DefaultHandler(Event &e);
  };

  class AnimBrush : public ViewBrush
  {
  public:

    virtual char * GetIFaceName() 
    {
      return "AnimBrushControl";
    }
  };

  class TreadBrush : public ViewBrush
  {
  public:

    virtual char * GetIFaceName() 
    {
      return "TreadBrushControl";
    }
  };

  class AimBrush : public ViewBrush
  {
  public:

    virtual char * GetIFaceName() 
    {
      return "AimBrush";
    }
  };

  class VertBrush : public Brush
  {
  public:
    VarInteger showverts;
    VarInteger append;
    Bool appendKey, appendLast;

    U32        groupSelect    : 1;

    virtual void Init();
    virtual void Done();

    virtual char * GetIFaceName() 
    {
      return "VertBrushControl";
    }

    virtual void Setup( Bool on)
    {
      IControl * control = IFace::FindByName( GetIFaceName());
      if (control)
      {
        if (on)
        {
          control->Activate();
          MeshOptions::vertBrush = TRUE;
        }
        else
        {
          control->Deactivate();
          MeshOptions::vertBrush = FALSE;
        }
      }
    }

    virtual void Clear();
    virtual void Render();

    virtual Bool FASTCALL DefaultHandler(Event &e);
    virtual void CmdHandler(U32 pathCrc);
  };

  class FaceBrush : public Brush
  {
  public:

    virtual char * GetIFaceName() 
    {
      return "FaceBrushControl";
    }
  };

  static ViewBrush        viewBrush;
  static AnimBrush        animBrush;
  static TreadBrush       treadBrush;
  static AimBrush         aimBrush;
  static VertBrush        vertBrush;
  static Brush *          curBrush;

  static Vector		        rotateObj;
  static Vector		        moveCam;
  static Vector		        rotateCam;
  static Vector		        camVector;
  static Vector		        camRotate;
  static Vector		        camOffset;

  static Matrix		        worldCam;
  static Matrix		        viewCam;
  static Quaternion        attCam;
  static Quaternion        quat;

  static VarInteger        moveX1;
  static VarInteger        moveX2;
  static VarInteger        moveY1;
  static VarInteger        moveY2;
  static VarInteger        moveZ1;
  static VarInteger        moveZ2;

  static VarInteger        rotX1;
  static VarInteger        rotX2;
  static VarInteger        rotY1;
  static VarInteger        rotY2;

  static VarInteger        doCamLock;
  static VarInteger        doMouseSelect;
  static VarInteger        doRotate;

  static VarInteger        brushMoveX;
  static VarInteger        brushMoveY;
  static VarInteger        brushMoveZ;

  static VarInteger        brushMoveXKey;
  static VarInteger        brushMoveYKey;
  static VarInteger        brushMoveZKey;

  static VarInteger        treadActive;
  static VarFloat          treadSpeed;

  // settings
  //
  static VarInteger        camLight;
  static VarInteger        lightColor;
  static VarInteger        lightIntensity;
  static VarInteger        lightPoint;
  static VarInteger        lightSpot;

  static VarInteger        sunLight;
  static VarInteger        sunIntensity;

  static VarInteger        ambLight;
  static VarInteger        ambColor;
  static VarInteger        ambIntensity;

  static VarInteger        showGround;
  static VarInteger        showWater;
  static VarInteger        showOrigin;
  static VarInteger        showOffset;
  static VarInteger        showInterface;
  static VarInteger        showShadowTex;

  static VarInteger        showSelected;
  static VarInteger        selFaces;
  static VarInteger        selVerts;

  static VarInteger        mrmAbsolute;
  static VarInteger        mrmDistance;

  static F32               mrmFactorAbs;
  static F32               mrmFactorDist;

  static Vid::Light::Obj * light = NULL;
  static Vid::Light::Obj * sun = NULL;
  static Vector			       lightVector;
  static Bool              lightLock;

  // animation
  // 
  static VarInteger        animActive;
  static VarFloat          animFrame;
  static Bool              animFrameLock;
  static VarInteger        animLoop;
  static U32               stepTime;
  static VarInteger        stepHold;

  // aim
  //
  static VarInteger        aimX;
  static VarInteger        aimY;
  static VarInteger        aimZ;
  static VarInteger        aimMix;

  static VarFloat          aimFactor;

  // load data
  //
  static VarString         appMessage;

  static NameString        defMeshName;
  static VarString         meshName;
  static VarString         searchPath;
  static VarString         backName;
  static VarString         groundName;
  static VarInteger        doAnimCycles;
  static VarInteger        loadMulti;
  static VarInteger        loadAttach;
  static VarInteger        loadCycle;
  static VarString         curName;
  static VarString         attachName;
  static Bool              loadLock;

  static VarFloat          scale;
  static Bitmap          * background = NULL;
  static Bitmap          * ground = NULL;

  static Material         *meshmat = NULL;
  static ColorF32        diffuse;
  static ColorF32        specular;
  static F32               specpower;
  static Bool              doAttach;

  static VarFloat          moveRate;

  static U32               groundColor;
  static U32               stickColor;
  static U32               boundsColor;

  static U32               originColor0;
  static U32               originColor1;
  static U32               originColor2;

  static F32               quatAngle;
  static Vector            quatAxis;

  static Bool              isInit = FALSE;
  static MeshEnt *         selMesh = NULL;
  static MeshObj *         childMesh = NULL;

  static F32               worldWidth;

  static char lastOpenPath[MAX_PATH] = "";  // Last directory from GetOpenFilename
  static U32 statCycleID = 0;
  static U32 lastShadowType;
  //----------------------------------------------------------------------------

  void ClearData();
  void CmdHandler(U32 pathCrc);
  Bool FASTCALL DefaultHandler(Event &e);
  void OnModeChange();
  Bool Setup();
  Bool Load();
  void LoadListBox();
  void Update();
  void AddAnimToRoot( MeshRoot &root, const char *meshName, const char *path);
  //----------------------------------------------------------------------------

  static MeshRoot *lastRoot;
  static Bool envMap;
  static Bool wasLoadGod;
  static U32 lastShadowSize, lastFog;
  static U32 lastHeapVerts, lastHeapIdx;
  static U32 lastAlphaNear;

  void Init()
  {
   // Create the game window
    IFace::SetGameHandler(DefaultHandler);

    ClearData();

    Vid::InitResources();
    MeshEffectSystem::Init();
    Mesh::Manager::InitResources();

    MeshEffectSystem::ProcessTypeFile( "types_mesheffects.cfg");

    wasLoadGod = Vid::Var::doLoadGod;

    // setup app var scope
    VarSys::RegisterHandler("app", CmdHandler);
    VarSys::RegisterHandler("app.list", CmdHandler);
    VarSys::RegisterHandler("app.view", CmdHandler);
    VarSys::RegisterHandler("app.anim", CmdHandler);
    VarSys::RegisterHandler("app.move", CmdHandler);
    VarSys::RegisterHandler("app.rot", CmdHandler);
    VarSys::RegisterHandler("app.control", CmdHandler);
    VarSys::RegisterHandler("app.light", CmdHandler);
    VarSys::RegisterHandler("app.toggle", CmdHandler);
    VarSys::RegisterHandler("app.show", CmdHandler);
    VarSys::RegisterHandler("app.mrm", CmdHandler);
    VarSys::RegisterHandler("app.select", CmdHandler);
    VarSys::RegisterHandler("app.tread", CmdHandler);
    VarSys::RegisterHandler("app.attach", CmdHandler);
    VarSys::RegisterHandler("aim", CmdHandler);
    VarSys::RegisterHandler("brush", CmdHandler);

    VarSys::CreateCmd("app.control.toggle");
    VarSys::CreateCmd("app.control.reset");

    VarSys::CreateCmd("app.dialog");
    VarSys::CreateCmd("app.list.fill");
    VarSys::CreateCmd("app.list.god");
    VarSys::CreateCmd("app.list.xsi");
    VarSys::CreateCmd("app.list.gfg");
    VarSys::CreateCmd("app.list.next");
    VarSys::CreateCmd("app.list.rebuild");
    VarSys::CreateCmd("app.list.delete");
    VarSys::CreateString("app.list.path", ".", VarSys::NOTIFY, &searchPath);
    VarSys::CreateFloat("app.list.scale", 1.0f, VarSys::NOTIFY, &scale);

    VarSys::CreateCmd("app.delete");
    VarSys::CreateCmd("app.quit");

    VarSys::CreateInteger("app.list.multi",  FALSE, VarSys::NOTIFY, &loadMulti);
    VarSys::CreateInteger("app.list.attach", FALSE, VarSys::NOTIFY, &loadAttach);
    VarSys::CreateInteger("app.list.cycle",  FALSE, VarSys::NOTIFY, &loadCycle);
    VarSys::CreateString("app.list.attachname", "", VarSys::NOTIFY, &attachName);
    VarSys::CreateString("app.curname", "", VarSys::DEFAULT, &curName);

    VarSys::CreateCmd("app.view.reset");
    VarSys::CreateCmd("app.view.x");
    VarSys::CreateCmd("app.view.y");
    VarSys::CreateCmd("app.view.z");
    VarSys::CreateCmd("app.view.cam");
    VarSys::CreateInteger("app.view.camlock", TRUE, VarSys::DEFAULT, &doCamLock);
    VarSys::CreateInteger("app.view.mousesel",  FALSE, VarSys::DEFAULT, &doMouseSelect);

    VarSys::CreateInteger("app.move.x1", FALSE, VarSys::NOTIFY, &moveX1);
    VarSys::CreateInteger("app.move.x2", FALSE, VarSys::NOTIFY, &moveX2);
    VarSys::CreateInteger("app.move.y1", FALSE, VarSys::NOTIFY, &moveY1);
    VarSys::CreateInteger("app.move.y2", FALSE, VarSys::NOTIFY, &moveY2);
    VarSys::CreateInteger("app.move.z1", FALSE, VarSys::NOTIFY, &moveZ1);
    VarSys::CreateInteger("app.move.z2", FALSE, VarSys::NOTIFY, &moveZ2);

    VarSys::CreateInteger("app.rot.x1", FALSE, VarSys::NOTIFY, &rotX1);
    VarSys::CreateInteger("app.rot.x2", FALSE, VarSys::NOTIFY, &rotX2);
    VarSys::CreateInteger("app.rot.y1", FALSE, VarSys::NOTIFY, &rotY1);
    VarSys::CreateInteger("app.rot.y2", FALSE, VarSys::NOTIFY, &rotY2);

    VarSys::CreateCmd("app.anim.play");
    VarSys::CreateCmd("app.anim.toggle");
    VarSys::CreateCmd("app.anim.steptoggle");
    VarSys::CreateCmd("app.anim.nextcycle");
    VarSys::CreateFloat("app.anim.frame", 1.0f, VarSys::NOTIFY, &animFrame);
    VarSys::SetFloatRange("app.anim.frame", 0.0f, 100.0f);
    
    VarSys::CreateInteger("app.anim.active", TRUE, VarSys::NOTIFY, &animActive);
    VarSys::CreateInteger("app.anim.loadcycles", FALSE, VarSys::DEFAULT, &doAnimCycles);
    VarSys::CreateInteger("app.anim.loop", TRUE, VarSys::DEFAULT, &animLoop);
    VarSys::CreateInteger("app.anim.step", FALSE, VarSys::DEFAULT, &stepHold);

    VarSys::CreateInteger("app.light.cam",  FALSE, VarSys::NOTIFY, &camLight);
    VarSys::CreateInteger("app.light.point", TRUE, VarSys::NOTIFY, &lightPoint);
    VarSys::CreateInteger("app.light.spot", FALSE, VarSys::NOTIFY, &lightSpot);
    VarSys::CreateInteger("app.light.color", 0xffffffff, VarSys::NOTIFY, &lightColor);

    Color color( 100l, 100l, 100l);
    VarSys::CreateInteger("app.light.ambient",  TRUE, VarSys::NOTIFY, &ambLight);
    VarSys::CreateInteger("app.light.ambcolor", color, VarSys::NOTIFY, &ambColor);

    VarSys::CreateInteger("app.light.sun",  TRUE, VarSys::NOTIFY, &sunLight);

    VarSys::CreateInteger("app.show.ground", TRUE,  VarSys::DEFAULT, &showGround);
    VarSys::CreateInteger("app.show.water",  FALSE, VarSys::NOTIFY,  &showWater);
    VarSys::CreateInteger("app.show.origin", TRUE,  VarSys::DEFAULT, &showOrigin);
    VarSys::CreateInteger("app.show.offset", FALSE, VarSys::DEFAULT, &showOffset);
    VarSys::CreateInteger("app.show.rotate", FALSE, VarSys::DEFAULT, &doRotate);
    VarSys::CreateInteger("app.show.selected", TRUE, VarSys::DEFAULT, &showSelected);
    VarSys::CreateInteger("app.show.interface", TRUE, VarSys::NOTIFY, &showInterface);
    VarSys::CreateInteger("app.show.shadowtex", FALSE, VarSys::DEFAULT, &showShadowTex);

    VarSys::CreateInteger("app.mrm.distance", FALSE, VarSys::NOTIFY, &mrmDistance);
    VarSys::CreateInteger("app.mrm.absolute", TRUE, VarSys::NOTIFY, &mrmAbsolute);
    VarSys::CreateCmd("app.mrm.reset");
    VarSys::CreateCmd("app.mrm.gen");

    VarSys::CreateInteger("app.tread.active", FALSE, VarSys::NOTIFY, &treadActive);
    VarSys::CreateFloat("app.tread.speed", 0.17f, VarSys::NOTIFY, &treadSpeed);
    VarSys::SetFloatRange("app.tread.speed", -2.0f, 2.0f);

    mrmFactorAbs  = Vid::renderState.mrmFactor1;
    mrmFactorDist = 1.0f;

    VarSys::CreateCmd("app.diffuse");
    VarSys::CreateCmd("app.specular");
    VarSys::CreateCmd("app.specinc");

    VarSys::CreateCmd("app.toggle.sunlight");
    VarSys::CreateCmd("app.toggle.camlight");
    VarSys::CreateCmd("app.toggle.lighttype");
    VarSys::CreateCmd("app.toggle.ground");
    VarSys::CreateCmd("app.toggle.res");
    VarSys::CreateCmd("app.toggle.interface");

    VarSys::CreateCmd("app.groundcolor");
    VarSys::CreateCmd("app.stickcolor");
    VarSys::CreateCmd("app.boundscolor");

    VarSys::CreateCmd("app.saved");

    VarSys::CreateCmd("app.attach.light");


    VarSys::CreateString("app.message", "", VarSys::DEFAULT, &appMessage);
    VarSys::CreateString("app.meshname", DEFMESHNAME, VarSys::NOTIFY, &meshName);
    VarSys::CreateString("app.backname", DEFBACKNAME, VarSys::NOTIFY, &backName);
    VarSys::CreateString("app.groundname", DEFGROUNDNAME, VarSys::NOTIFY, &groundName);

    VarSys::CreateFloat("app.moverate", MOVERATE, VarSys::DEFAULT, &moveRate);

    VarSys::CreateCmd("app.select.all");

    VarSys::CreateInteger("brush.moveX", FALSE, VarSys::DEFAULT, &brushMoveX);
    VarSys::CreateInteger("brush.moveY", FALSE, VarSys::DEFAULT, &brushMoveY);
    VarSys::CreateInteger("brush.moveZ", TRUE,  VarSys::DEFAULT, &brushMoveZ);

    VarSys::CreateInteger("brush.moveXkey", FALSE, VarSys::NOTIFY, &brushMoveXKey);
    VarSys::CreateInteger("brush.moveYkey", FALSE, VarSys::NOTIFY, &brushMoveYKey);
    VarSys::CreateInteger("brush.moveZkey", FALSE, VarSys::NOTIFY, &brushMoveZKey);

    VarSys::CreateCmd("brush.next");
    VarSys::CreateCmd("brush.toggle");

    VarSys::CreateInteger("aim.X", TRUE,  VarSys::NOTIFY, &aimX);
    VarSys::CreateInteger("aim.Y", FALSE, VarSys::NOTIFY, &aimY);
    VarSys::CreateInteger("aim.Z", FALSE, VarSys::NOTIFY, &aimZ);
    VarSys::CreateInteger("aim.mix", FALSE,  VarSys::NOTIFY, &aimMix);

    VarSys::CreateFloat("aim.factor", PI, VarSys::NOTIFY, &aimFactor);
    VarSys::SetFloatRange("aim.factor", 0.0F, 2.0f * PI);

    viewBrush.Init();
//    animBrush.Init(); same as viewBrush
    vertBrush.Init();

    Vid::SetModeChangeCallback(OnModeChange);

    Setup();

    lightLock = FALSE;
    isInit = TRUE;

    lastHeapVerts = Vid::renderState.maxVerts;
    lastHeapIdx   = Vid::renderState.maxIndices;

    char * string = FileSys::GetSub( "@vertmax");
    U32 maxverts = atol( string);
    string = FileSys::GetSub( "@trimax");
    U32 maxtris  = atol( string);

    Vid::Heap::Init( maxverts, maxtris * 3);

    string = FileSys::GetSub( "@vertmaxcheck");
    Vid::Var::checkMaxVerts = atol( string);
    string = FileSys::GetSub( "@trimaxcheck");
    Vid::Var::checkMaxTris = atol( string);

    Main::Exec( "meshview.cfg");

    searchPath = FileSys::GetSub( "@meshviewpath");
    meshName   = FileSys::GetSub( "@meshviewmodel");

    BuffString buff;
    buff = *searchPath;
    char * b = buff.str + strlen(buff.str);

    Utils::Strcpy( b, "\\god");
    Vid::Var::godFilePath = buff.str;

    Utils::Strcpy( b, "\\gfg");
    Vid::Var::gfgFilePath = buff.str;

    stepTime = 0;

    // Initial working directory
    getcwd(lastOpenPath, MAX_PATH);

    // force high detail
    Vid::Var::detail.Set( 0);

    lastFog = Vid::SetFogState( FALSE);
//    lastShadowSize = Vid::renderState.texShadowSize;
//    Vid::Var::varShadowSize = 128;

    lastAlphaNear = Vid::renderState.status.alphaNear;
    Vid::Var::alphaNearActive = FALSE;

    // start up movies
    Bitmap::Manager::MovieFirstStart();

    Vid::Var::varNightLight = FALSE;

    lastShadowType = Vid::renderState.status.shadowType;
    Vid::renderState.status.shadowType = MeshRoot::shadowLIVE;
  }
  //----------------------------------------------------------------------------

  void Done()
  {
    Vid::renderState.status.shadowType = lastShadowType;

    Vid::SetFogState( lastFog);
//    Vid::Var::varShadowSize = lastShadowSize;

    ClearData();

    viewBrush.Done();
//    animBrush.Done(); same as view brush
    vertBrush.Done();

    VarSys::DeleteItem("app.attach");
    VarSys::DeleteItem("app.tread");
    VarSys::DeleteItem("app.select");
    VarSys::DeleteItem("app.show");
    VarSys::DeleteItem("app.toggle");
    VarSys::DeleteItem("app.light");
    VarSys::DeleteItem("app.control");
    VarSys::DeleteItem("app.rot");
    VarSys::DeleteItem("app.move");
    VarSys::DeleteItem("app.anim");
    VarSys::DeleteItem("app.view");
    VarSys::DeleteItem("app.list");
    VarSys::DeleteItem("app.mrm");

    VarSys::DeleteItem("aim");
    VarSys::DeleteItem("brush");
    VarSys::DeleteItem("app");

//    Terrain::Done();

    Mesh::Manager::DisposeAll();
    MeshEffectSystem::Done();
  
    // Remove all interface items
    IFace::DisposeAll();

    // Clear all Vid data
    Vid::DisposeAll();

    Vid::Var::doLoadGod = wasLoadGod;

    Vid::Heap::Init( lastHeapVerts, lastHeapIdx);
    Vid::Var::alphaNearActive = lastAlphaNear;
  }
  //----------------------------------------------------------------------------
  static Bool messageLock = FALSE;

  Bool SetMessageInternal( const char * mess0, const char * mess1 = NULL, const char * mess2 = NULL)
  {
//    if (messageLock || !selMesh)
    if (messageLock)
    {
      return FALSE;
    }

    BuffString buff;

    buff = mess0;
    if (mess1)
    {
      Utils::Strcat( buff.str, mess1);
    }
    if (mess2)
    {
      Utils::Strcat( buff.str, mess2);
    }
    appMessage = buff.str;

    return TRUE;
  }
  //----------------------------------------------------------------------------

  void SetMessage( const char * mess0, const char * mess1, const char * mess2) // = NULL, = NULL)
  {
    if (SetMessageInternal( mess0, mess1, mess2) && Vid::isStatus.initialized)
    {
      messageLock = TRUE;

      Process();

      messageLock = FALSE;
    }
  }
  //----------------------------------------------------------------------------

  void SetCurrName()
  {
    curName = selMesh->Root().fileName.str;

    if (childMesh)
    {
      SetMessageInternal( selMesh->Root().fileName.str, 
        ":  ", 
        childMesh->GetName());
    }
    else
    {
      SetMessageInternal( selMesh->Root().fileName.str);
    }
  }
  //----------------------------------------------------------------------------

  Bool SetCurrent( MeshEnt * ent, MeshObj * child = NULL, Bool force = FALSE)
  {
    Bool isNew = selMesh != ent ? TRUE : FALSE;
    selMesh = ent;

    Mesh::Manager::curEnt = ent;
    Mesh::Manager::curParent = Mesh::Manager::curEnt->FindMeshEnt();

    childMesh = child ? child : ent;
    Mesh::Manager::curChild  = childMesh;

    attachName = childMesh->GetName();
    MeshOptions::Setup( ent);
    
    if (isNew || force)
    {
      selMesh = ent;

      ent->SetupRenderProc();

      if (!Mesh::Manager::curParent)
      {
        Mesh::Manager::curParent = ent;
      }

      SetCurrName();
      scale = ent->Root().scale;

      IControl * filebox  = IFace::FindByName( "ListBoxFile");
      ICDropList * droplist = NULL;
      ICListBox * flistbox = NULL;
      if (filebox)
      {
        droplist = (ICDropList *) filebox->Find( Crc::CalcStr("AttachName"));
      }
      if (droplist)
      {
        flistbox = droplist->GetListBox();
      }
      if (flistbox)
      {
        flistbox->DeleteAllItems();
      }

#if 0
      IControl * aimbrush = IFace::FindByName( "AimBrush");
      droplist = NULL;
      ICListBox * alistbox = NULL;
      if (aimbrush)
      {
        droplist = (ICDropList *) aimbrush->Find( Crc::CalcStr("AttachName"));
      }
      if (droplist)
      {
        alistbox = droplist->GetListBox();
      }
      if (alistbox)
      {
        alistbox->DeleteAllItems();
      }
#endif

      BuffString names[MAXMESHPERGROUP];
      U32 count = ent->GetHierarchy( names);

      U32 i, hit = count;
      for (i = 0; i < count; i++)
      {
        if (flistbox)
        {
          flistbox->AddTextItem(names[i].str, NULL);
        }
#if 0
        if (alistbox)
        {
          alistbox->AddTextItem(names[i].str, NULL);
        }
#endif

/*
        char *p = names[i].str;
        while (*p == ' ')
        {
          p++;
        }
        if (hit == count && (!strnicmp( p, "hp-", 3) || !strnicmp( p, "op-", 3) || !strnicmp( p, "cp-", 3)))
        {
          hit = i;
        }
*/
      }
      if (hit == count)
      {
        hit = 0;
      }
      char * p = names[hit].str;
      while (*p == ' ')
      {
        p++;
      }
      attachName = p;
      childMesh = (MeshObj *) selMesh->FindMeshObj( Crc::CalcStr(*attachName));

      IControl * graphicsbox = IFace::FindByName( "View::Graphics");
      if (graphicsbox)
      {
        IControl * button = (IControl *) graphicsbox->Find( Crc::CalcStr("ShowSP0Button"));
        if (button)
        {
          button->SetEnabled( selMesh->FindLocal( "SP-0") ? TRUE : FALSE);
        }
        button = (IControl *) graphicsbox->Find( Crc::CalcStr("ShowSP1Button"));
        if (button)
        {
          button->SetEnabled( selMesh->FindLocal( "SP-1") ? TRUE : FALSE);
        }
        button = (IControl *) graphicsbox->Find( Crc::CalcStr("ShowMutiTextButton"));
        if (button)
        {
          button->SetEnabled( Vid::caps.texMulti);
        }
      }
      return TRUE;
    }
    return FALSE;
  }
  //----------------------------------------------------------------------------

  void SetBrush( Brush * brush = NULL)
  {
    if (!brush)
    {
      brush = &viewBrush;
    }
    if (curBrush != brush)
    {
      curBrush->Setup( FALSE);
      curBrush = brush;
      curBrush->Setup( TRUE);
    }
  }
  //----------------------------------------------------------------------------

  static Bool doGodFiles = FALSE;

  void CmdHandler(U32 pathCrc)
  {
    static Bool lockMrm = FALSE;
    static Bool lastMX = 0;
    static Bool lastMY = 0;
    static Bool lastMZ = 0;

    static Bool lockout = FALSE;
    if (lockout)
    {
      return;
    }

    static char * s1;

    switch (pathCrc)
    {
    default:
      curBrush->CmdHandler( pathCrc);
      break;

    case 0x7DA4790D: // "app.show.water"
    {
      if (Terrain::mapInit)
      {
        U32 i, j;
        for (i = 0; i < Terrain::ClusterHeight(); i++)
        {
          for (j = 0; j < Terrain::ClusterWidth(); j++)
          {
            Cluster & clus = Terrain::GetCluster(i, j);
            clus.status.water = *showWater;
            clus.waterHeight = 0.0f;
          }
        }

        for (i = 0; i < Terrain::CellMax(); i++)
        {
          Terrain::SetCellHeight( i, *showWater ? -1.0f : 0.0f);
        }
        Terrain::Recalc();
      }
      break;
    }

    case 0x26F00408: // "aim.x"
      if (*aimMix)
      {
        break;
      }

      lockout = TRUE;
      if (*aimX)
      {
        aimY = FALSE;
        aimZ = FALSE;
      }
      else
      {
        aimY = TRUE;
        aimZ = FALSE;
      }
      lockout = FALSE;
      break;
    case 0x223119BF: // "aim.y"
      if (*aimMix)
      {
        break;
      }

      lockout = TRUE;
      if (*aimY)
      {
        aimX = FALSE;
        aimZ = FALSE;
      }
      else
      {
        aimX = TRUE;
        aimZ = FALSE;
      }
      lockout = FALSE;
      break;
    case 0x2F723F66: // "aim.z"
      if (*aimMix)
      {
        break;
      }

      lockout = TRUE;
      if (*aimZ)
      {
        aimY = FALSE;
        aimX = FALSE;
      }
      else
      {
        aimY = TRUE;
        aimX = FALSE;
      }
      lockout = FALSE;
      break;

    case 0xE4793DCD: // "app.quit"
      Console::ProcessCmd( "sys.runcode quit");
      break;

    case 0x006C9DC3: // "app.meshName"
      Load();
      break;
    case 0x01966885: // "app.dialog"
      LoadListBox();
      break;

    case 0xD794D6FB: // "app.delete"
      if (selMesh->Parent())
      {
        MeshEnt * ent = Mesh::Manager::curParent;
        delete selMesh;
        SetCurrent( ent);
      }
      break;
    case 0x6C31720B: // "app.saved"
      SetCurrent( Mesh::Manager::curEnt, NULL, TRUE);
      break;

    case 0x48FF65CA: // "app.attach.light"
    {
      const char * p = *attachName;
      while (*p == ' ')
      {
        p++;
      }

      FamilyNode * node = (FamilyNode *) selMesh->Find( p);
      if (!node)
      {
        break;
      }
      Vid::Light::Obj * light = new Vid::Light::Obj( "spot", Vid::Light::lightSPOT);
      selMesh->AttachAt( *node, *light);
      break;
    }

    case 0x84277DEC: // "app.list.delete"
    {
      ICListBox *listBox = ICListBox::FindListBox("|ListBoxFile.FileList");
      if (listBox)
      {
        S32 sel = listBox->GetSelectedIndex();
        if (sel > 0)
        {
          char na[256];
          Utils::Unicode2Ansi( na, 256, listBox->GetSelectedItem()->GetTextString());

          // Open a file using its file name
          FileSys::DataFile * file  = FileSys::Open( na);

          if (file)
          {
            char buff1[MAX_BUFFLEN + 1];
            Utils::MakePath( buff1, MAX_BUFFLEN, file->Path(), na, NULL);

            _unlink( buff1);
          }
        }
      }
      break;
    }
    case 0x76A5D2B8: // "app.list.scale"
      Vid::Var::scaleFactor = *scale;
      break;
    case 0xFEF01D15: // "app.list.path"
    {
      FilePath absolute;
      Dir::PathAbsolute(absolute, *searchPath);

      FileSys::DeleteStream( "search");
      FileSys::AddSrcDirRecurse( "search", absolute.str);
      // fall through
    }
    case 0x4ED39ECF: // "app.list.fill"

      SetMessage( "Filling file list from ", *searchPath, "...");

      if (doGodFiles == 2)
      {
        ICListBox *listBox = ICListBox::FindListBox("|ListBoxFile.FileList");
        if (listBox)
        {
          listBox->FillFromPath( *searchPath, "*.gfg", TRUE, TRUE);
          listBox->Sort();
        }
      }
      else if (doGodFiles == 1)
      {
        ICListBox *listBox = ICListBox::FindListBox("|ListBoxFile.FileList");
        if (listBox)
        {
          listBox->FillFromPath( *searchPath, "*.god", TRUE, TRUE);
          listBox->Sort();
        }
      }
      else
      {
        ICListBox *listBox = ICListBox::FindListBox("|ListBoxFile.FileList");
        if (listBox)
        {
          listBox->FillFromPath( *searchPath, "*.xsi", TRUE, TRUE);
          listBox->FillFromPath( *searchPath, "*.x", FALSE, TRUE);
          listBox->Sort();
        }
      }
      break;
    case 0x7AE0618C: // "app.list.xsi"
      doGodFiles = 0;
      Console::ProcessCmd( "app.list.fill");
      break;
    case 0x8254F56A: // "app.list.god"
      doGodFiles = 1;
      Console::ProcessCmd( "app.list.fill");
      break;
    case 0xD746513D: // "app.list.gfg"
      doGodFiles = 2;
      Console::ProcessCmd( "app.list.fill");
      {
        IControl * button = IFace::FindByName( "ListBoxFile.Rebuild");
        if (button)
        {
          button->SetEnabled( doGodFiles == 2 ? TRUE : FALSE);
        }
      }
      break;
    case 0x81F25C93: // "app.list.next"
      doGodFiles++;
      if (doGodFiles > 2)
      {
        doGodFiles = 0;
      }
      Console::ProcessCmd( "app.list.fill");

      {
        IControl * button = IFace::FindByName( "ListBoxFile.Rebuild");
        if (button)
        {
          button->SetEnabled( doGodFiles == 2 ? TRUE : FALSE);
        }
      }
      break;

    case 0x5BD4464E: // "app.list.rebuild"
      {
        ICListBox *listBox = ICListBox::FindListBox("|ListBoxFile.FileList");
        if (listBox)
        {
          // Get the list of children
          const NList<IControl> & children = listBox->GetChildren();
          NList<IControl>::Iterator i(&children);
          for (!i; *i; i++)
          {
            char na[256];
            Utils::Unicode2Ansi( na, 256, (*i)->GetTextString());
            meshName = na;

            Console::ProcessCmd("god.save");
          }
        }
      }
      break;

    case 0x96E1E7D2: // "app.list.multi"
      if (!loadLock)
      {
        loadLock = TRUE;

        loadAttach = FALSE;
        loadCycle  = FALSE;

        loadLock = FALSE;
      }
      break;
    case 0x26BF48FA: // "app.list.attach"
      if (!loadLock)
      {
        loadLock = TRUE;

        loadMulti = FALSE;
        loadCycle = FALSE;

        loadLock = FALSE;
      }
      break;
    case 0x442218CA: // "app.list.cycle"
      if (!loadLock)
      {
        loadLock = TRUE;

        loadAttach = FALSE;
        loadMulti  = FALSE;

        loadLock = FALSE;
      }
      break;
    case 0x372F5FB1: // "app.list.attachname"
    {
      const char *p = *attachName;
      while (*p == ' ')
      {
        p++;
      }
      const MeshObj * node = selMesh->FindMeshObj( Crc::CalcStr( p));
      if (node)
      {
        childMesh = (MeshObj *) node;
      }
      break;
    }
    case 0x12A098E5: // "app.move.x1"
      moveCam.x = *moveRate;
      if (Input::KeyState(Input::CTRLDOWN))
      {
        moveCam *= FASTFACTOR;
      }
      else if (Input::KeyState(Input::SHIFTDOWN))
      {
        moveCam *= NUDGEFACTOR;
      }
      break;
    case 0x1FE3BE3C: // "app.move.x2"
      moveCam.x = -*moveRate;
      if (Input::KeyState(Input::CTRLDOWN))
      {
        moveCam *= FASTFACTOR;
      }
      else if (Input::KeyState(Input::SHIFTDOWN))
      {
        moveCam *= NUDGEFACTOR;
      }
      break;
    case 0xC0B95939: // "app.move.y1"
      moveCam.y = *moveRate;
      if (Input::KeyState(Input::CTRLDOWN))
      {
        moveCam *= FASTFACTOR;
      }
      else if (Input::KeyState(Input::SHIFTDOWN))
      {
        moveCam *= NUDGEFACTOR;
      }
      break;
    case 0xCDFA7FE0: // "app.move.y2"
      moveCam.y = -*moveRate;
      if (Input::KeyState(Input::CTRLDOWN))
      {
        moveCam *= FASTFACTOR;
      }
      else if (Input::KeyState(Input::SHIFTDOWN))
      {
        moveCam *= NUDGEFACTOR;
      }
      break;
    case 0xB25206EA: // "app.move.z1"
      moveCam.z = *moveRate;
      if (Input::KeyState(Input::CTRLDOWN))
      {
        moveCam *= FASTFACTOR;
      }
      else if (Input::KeyState(Input::SHIFTDOWN))
      {
        moveCam *= NUDGEFACTOR;
      }
      break;
    case 0xBF112033: // "app.move.z2"
      moveCam.z = -*moveRate;
      if (Input::KeyState(Input::CTRLDOWN))
      {
        moveCam *= FASTFACTOR;
      }
      else if (Input::KeyState(Input::SHIFTDOWN))
      {
        moveCam *= NUDGEFACTOR;
      }
      break;
    case 0x38504138: // "app.rot.x1"
      rotateCam.x = CAMROTATERATE;
      if (Input::KeyState(Input::SHIFTDOWN))
      {
        rotateCam *= NUDGEFACTOR;
      }
      break;
    case 0x351367E1: // "app.rot.x2"
      rotateCam.x = -CAMROTATERATE;
      if (Input::KeyState(Input::SHIFTDOWN))
      {
        rotateCam *= NUDGEFACTOR;
      }
      break;
    case 0xEA4980E4: // "app.rot.y1"
      rotateCam.y = CAMROTATERATE;
      if (Input::KeyState(Input::SHIFTDOWN))
      {
        rotateCam *= NUDGEFACTOR;
      }
      break;
    case 0xE70AA63D: // "app.rot.y2"
      rotateCam.y = -CAMROTATERATE;
      if (Input::KeyState(Input::SHIFTDOWN))
      {
        rotateCam *= NUDGEFACTOR;
      }
      break;
    case 0x6DBA0F78: // "app.view.reset"
      {
        doRotate = FALSE;

        quatAngle = PI * 0.25f;
        quatAxis  = Matrix::I.Front();
        quat.Set( quatAngle, quatAxis);

        moveCam.Zero();
        rotateCam.Zero();
        rotateObj.Zero();
        rotateObj.y = PI * 0.5f;

        camOffset.Zero();
        camRotate.Zero();
        camRotate.x = MINCAMROTX;
        camRotate.y = PI;

        F32 width = 0.0f, maxrad = 0.0f;
        Bool hit = FALSE;
        NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
        while (MeshEnt * ent = li++)
        {
          if (ent->Parent())
          {
            continue;
          }
          ent->SetFrame();

          if (ent->ObjectBounds().Radius() > maxrad)
          {
            maxrad = ent->ObjectBounds().Radius();
          }
          width += ent->ObjectBounds().Radius() * 2.0f;
          if (hit)
          {
            width += 0.5f;
          }
          else
          {
            hit = TRUE;
          }
        }
        width *= -0.5f;

        hit = FALSE;
        !li;  while (MeshEnt * ent = li++)
        {
          if (ent->Parent())
          {
            continue;
          }

          width += ent->ObjectBounds().Radius();

          Vector v(Vector(width,0,0));
          Quaternion q( 0, Matrix::I.up);
          ent->SetSimCurrent( q, v);

          width += ent->ObjectBounds().Radius();

          if (hit)
          {
            width += 0.5f;
          }
          else
          {
            hit = TRUE;
          }
        }
        worldWidth = width;

        camVector.x = 0.0f;
        camVector.y = 0.0f;
        camVector.z = -(maxrad * RADMULT + Vid::Math::nearPlane);

        camOffset.y = maxrad * 0.6666f;

        attCam.ClearData();
        Quaternion q( -camRotate.x, Matrix::I.Right());
        attCam = q;
    	  q.Set( camRotate.y, Matrix::I.Up());
    	  attCam *= q;
	      worldCam.Set( attCam);
        Vid::CurCamera().SetWorld( attCam, camVector);

        F32 rad = selMesh->ObjectBounds().Radius();
    	  lightVector.Set( rad * 2, 0, -rad/4.0f);
        Vector lv;
        Vid::CurCamera().WorldMatrix().Transform( lv, lightVector);
        light->SetWorld( lv);
        light->SetRange( maxrad * 10.0f);

        MeshRoot * root = Mesh::Manager::FindLoad( "engine_pandemic.god");
        if (Mesh::Manager::entList.GetCount() == 1 && root == &selMesh->Root())
        {
          doRotate = TRUE;
          camLight = TRUE;
//          showWater = TRUE;
        }
        else
        {
          doRotate = FALSE;
          camLight = FALSE;
          showWater = FALSE;
        }
/*
        F32 farp = Max<F32>( 100.0f, worldWidth * 4);
        Vid::SetFarPlane( farp);
        Vid::SetFogDepth( farp / 4);
*/
      }
      break;
    case 0x0B5566DB: // "app.view.x"
      if (doCamLock)
      {
        rotateObj.Zero();
        Quaternion q( 0.5f * PI, Matrix::I.up);
        selMesh->SetSimCurrent( q);
      }
      else
      {
        attCam.Set( PIBY2, Matrix::I.Up());
        worldCam.Set( attCam);
        Vector pos = Matrix::I.Front();
        pos *= (-Mesh::Manager::curParent->ObjectBounds().Radius() * RADMULT);
        worldCam.Rotate( worldCam.Position(), pos);
        worldCam.Position() += Mesh::Manager::curParent->Origin();
      }
      break;

    case 0x0F947B6C: // "app.view.y"
      if (doCamLock)
      {
        rotateObj.Zero();
      }
      else
      {
        attCam.Set( -PIBY2, Matrix::I.Right());
        worldCam.Set( attCam);
        Vector pos = Matrix::I.Front();
        pos *= (-Mesh::Manager::curParent->ObjectBounds().Radius() * RADMULT);
        worldCam.Rotate( worldCam.Position(), pos);
        worldCam.Position() += Mesh::Manager::curParent->Origin();
      }
      break;
    case 0x02D75DB5: // "app.view.z" 
      if (doCamLock)
      {
        rotateObj.Zero();
      }
      else
      {
        attCam.Zero();
        worldCam.Set( attCam);
        Vector pos = Matrix::I.Front();
        pos *= (-Mesh::Manager::curParent->ObjectBounds().Radius() * RADMULT);
        worldCam.Rotate( worldCam.Position(), pos);
        worldCam.Position() += Mesh::Manager::curParent->Origin();
      }
      break;
    case 0xB1AC1E26: // "app.view.cam"
      doCamLock = !*doCamLock;
      break;

    case 0xE14AFC75: // "app.rotate"
      doRotate = !doRotate;
      break;

    case 0xE6C39FFF: // "app.control.reset"
      break;
    case 0x8D6D7E99: // "app.control.toggle"
    {
      IControl *ctrl = IFace::FindByName("AnimControl");
      Bool active = FALSE;
      if (ctrl)
      {
        if (ctrl->IsActive())
        {
          IFace::Deactivate(ctrl);
        }
        else
        {
          IFace::Activate(ctrl);
          active = TRUE;
        }
      }
      ctrl = IFace::FindByName("MrmControl");

      if (ctrl)
      {
        if (active)
        {
          IFace::Deactivate(ctrl);
        }
        else
        {
          IFace::Activate(ctrl);
        }
      }
      break;
    }

    case 0x79580743: // "app.anim.play"
    {
//      animFrame = 1.0f;

      NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
      while (MeshEnt * ent = li++)
      {
        if (ent->curCycle && !ent->AnimIsActive())
        {
          ent->ActivateAnim();
        }
      }
      break;
    }
    case 0xFD1695D5: // "app.anim.active"
      selMesh->ActivateAnim( *animActive);
      break;
    case 0xFA5CB556: // "app.anim.toggle"
      animActive = !*animActive;
      break;
    case 0x3AB56A05: // "app.anim.frame"
      if (!animFrameLock)
      {
        selMesh->SetFrame( *animFrame - 1.0f);
      }
      break;
    case 0xFA2D93BE: // "app.anim.nextcycle"
      if (selMesh->Root().animCycles.GetCount())
      {
        NBinTree<AnimList>::Iterator li(&selMesh->Root().animCycles); 
        for (!li; *li; li++)
        {
          AnimList *animCycle = (*li);

          if (selMesh->curCycle == animCycle)
          {
            li++;
            break;
          }
        }
        if (!*li) 
        {
          !li;
        }
        if (selMesh->curCycle != (*li))
        {
          if (*MeshOptions::animBlend)
          {
            selMesh->BlendAnimCycle( (*li)->name.str, Vid::renderState.animBlendTime);
          }
          else
          {
            selMesh->SetAnimCycle( (*li)->name.str);
          }
          selMesh->fps = *MeshOptions::animFactor * selMesh->curCycle->animSpeed;
          SetCurrent( selMesh);

          LOG_DIAG( ("Animation Cycle = %s", (*li)->name.str) );
        }
      }
      break;

    case 0xCE1C4F13: // "app.r"
    case 0x95C8FF08: // "app.g"
    case 0x820D9463: // "app.b"
      Setup();
      break;
    case 0x7F194214: // "app.toggle.ground"
      showGround = !*showGround;
      break;

    case 0xF9EA605A: // "app.toggle.camlight"
      camLight = !*camLight;
      break;
    case 0xA6A49A26: // "app.toggle.sunlight"
      sunLight = !*sunLight;
      break;
    case 0x2229A155: // "app.toggle.lighttype"
      lightPoint = !*lightPoint;
      break;
    case 0xD51FB08A: // "app.toggle.res"
      {
        F32 r = 1.0f;
        static F32 res = 0.6f;
        if (!Console::GetArgFloat( 1, r))
        {
          const MeshRoot & root = selMesh->Root();
          if (root.vertices.count == selMesh->vertCount)
          {
            r = res;
          }
          else
          {
            res = (F32) selMesh->vertCount / (F32) selMesh->Root().vertices.count;
            r = 1.0f;
          }
          Vid::renderState.mrmFactor1 = (1.0f - r) * 10.0f;
          break;
        }
        if (r <= 1.0)
        {
          res = r;
        }
        else
        {
          res = r / selMesh->Root().vertices.count;
          if (res < 0.0f)
          {
            res = 0.0f;
          }
          else if (res > 1.0f)
          {
            res = 1.0f;
          }
        }
        Vid::renderState.mrmFactor1 = (1.0f - res) * 10.0f;
        break;
      }

    case 0xF84E0E50: // "app.light.cam"
      if (light)
      {
        light->SetActive( *camLight);
      }
      break;
    case 0xBDF25A43: // "app.light.sun"
      if (sun)
      {
        sun->SetActive( *sunLight);
      }
      break;
    case 0x1B0934B8: // "app.light.point"
      if (!lightLock)
      {
        lightLock = TRUE;
        lightSpot = !*lightPoint;
        lightLock = FALSE;

        if (light)
        {
          light->SetType( *lightPoint ? Vid::Light::lightPOINT : Vid::Light::lightSPOT);
        }
      }
      break;
    case 0xE84DBC9A: // "app.light.spot"
      if (!lightLock)
      {
        lightLock = TRUE;
        lightPoint = !*lightSpot;
        lightLock = FALSE;

        if (light)
        {
          light->SetType( *lightPoint ? Vid::Light::lightPOINT : Vid::Light::lightSPOT);
        }
      }
      break;
    case 0x8DE900D6: // "app.light.color"
      if (light)
      {
        Color color = *lightColor;
        light->SetColor( color);
      }
      break;

    case 0xF95B919A: // "app.light.ambient"
    case 0xD5615797: // "app.light.ambcolor"
      if (*ambLight)
      {
        Color color = *ambColor;
        F32 r = (F32) color.r / 255.0f;
        F32 g = (F32) color.g / 255.0f;
        F32 b = (F32) color.b / 255.0f;
        Vid::SetAmbientColor( r, g, b);
      }
      else
      {
        Vid::SetAmbientColor( 0.0f, 0.0f, 0.0f);
      }
      break;
 
    case 0x63A6A13B: // "app.diffuse"
    {
      F32 r = 1.0f, g, b, a = 1.0f;
      if (!Console::GetArgFloat( 1, r))
      {
        CON_DIAG( ("diffuse = %f %f %f %f", 
          diffuse.r, diffuse.g, diffuse.b, diffuse.a) );
        break;
      }
      if (!Console::GetArgFloat( 2, g))
      {
        g = r;
      }
      if (!Console::GetArgFloat( 3, b))
      {
        b = r;
      }
      Console::GetArgFloat( 4, a);

      diffuse.r = r;
      diffuse.g = g;
      diffuse.b = b;
      diffuse.a = a;

      if (meshmat)
      {
        meshmat->SetDiffuse( r, g, b, a);
      }
      break;
    }
    case 0xB907CC4F: // "app.specular"
    {
      F32 r = 1.0f, g, b, a = 1.0f, p = 50.0f;
      if (!Console::GetArgFloat( 1, r))
      {
        CON_DIAG(( "specular = %f %f %f %f %f", 
          specular.r, specular.g, specular.b, specular.a, specpower))
        break;
      }
      if (!Console::GetArgFloat( 2, g))
      {
        g = r;
      }
      if (!Console::GetArgFloat( 3, b))
      {
        b = r;
      }
      Console::GetArgFloat( 4, a);
      Console::GetArgFloat( 5, p);

      specular.r = r;
      specular.g = g;
      specular.b = b;
      specular.a = a;
      specpower = p;

      if (meshmat)
      {
        meshmat->SetSpecular( r, g, b, a, specpower);
      }
      break;
    }
    case 0x6198EA4F: // "app.specinc"
      {
        F32 f1 = 1.0f;
        Console::GetArgFloat(1, f1);
        specpower += f1;
        if (specpower < 0.0f)
        {
          specpower = 0.0f;
        }
        if (meshmat)
        {
          meshmat->SetSpecular( specular.r, specular.g, specular.b, specular.a, specpower);
        }
      }
      break;

    case 0x1F8C6C0B: // "app.groundcolor"
      {
        S32 r = 0, g = 255, b = 0, a = 122;
        Console::GetArgInteger( 1, r);
        Console::GetArgInteger( 2, g);
        Console::GetArgInteger( 3, b);
        Console::GetArgInteger( 4, a);
        groundColor = RGBA_MAKE( r, g, b, a);
      }
      break;
    case 0x28D258F6: // "app.stickcolor"
      {
        S32 r = 255, g = 0, b = 0, a = 122;
        Console::GetArgInteger( 1, r);
        Console::GetArgInteger( 2, g);
        Console::GetArgInteger( 3, b);
        Console::GetArgInteger( 4, a);
        stickColor = RGBA_MAKE( r, g, b, a);
      }
      break;
    case 0x477D5B9E: // "app.boundscolor"
      {
        S32 r = 255, g = 0, b = 0, a = 122;
        Console::GetArgInteger( 1, r);
        Console::GetArgInteger( 2, g);
        Console::GetArgInteger( 3, b);
        Console::GetArgInteger( 4, a);
        boundsColor = RGBA_MAKE( r, g, b, a);
      }
      break;

/*
    case 0x068E946F: // "app.backname"
      {
        Material *mat = Material::Manager::FindCreate( *backName);
        if (mat)
        {
          mat->GetTexture();

          background = mat;

          Mesh::Manager::envMap = background->GetTexture();
        }
      }
      break;
*/
    case 0x1F096402: // "app.groundname"
      {
      }
      break;

    case 0x3FE9644A: // "app.moveanim"
    case 0x2CD81382: // "app.indexed"
      Load();
      break;

    case 0x94BFAC47: // "app.mrm.absolute"
      if (!lockMrm)
      {
        lockMrm = TRUE;
        mrmDistance = !*mrmAbsolute;
        lockMrm = FALSE;

        Vid::Var::doMRM = !*mrmAbsolute;
        Vid::renderState.mrmFactor1 = *mrmAbsolute ? mrmFactorAbs : mrmFactorDist;
      }
      break;
    case 0x717E8E4A: // "app.mrm.distance"
      if (!lockMrm)
      {
        lockMrm = TRUE;
        mrmAbsolute = !*mrmDistance;
        lockMrm = FALSE;

        Vid::Var::doMRM = !*mrmAbsolute;
        Vid::renderState.mrmFactor1 = *mrmAbsolute ? mrmFactorAbs : mrmFactorDist;
      }
      break;
    case 0x340C18F5: // "app.mrm.reset"
      if (*mrmAbsolute)
      {
        mrmFactorAbs = 0.0f;
      }
      else
      {
        mrmFactorDist = 1.0f;
      }
      Vid::renderState.mrmFactor1 = *mrmAbsolute ? mrmFactorAbs : mrmFactorDist;
      break;
    case 0x6F41DDD7: // "app.mrm.gen"
      if (selMesh)
      {
        MeshView::SetMessage( "Generating MRM data for ", selMesh->Root().fileName.str, "...");

        selMesh->MRMGen( curBrush == &vertBrush);

        MeshOptions::reset = TRUE;
      }
      break;
    case 0xF2462CF9: // "app.select.all"
      selMesh->SelectVerts();
      break;

    case 0xE6B90EE0: // "brush.next"
      if (curBrush == &viewBrush)
      {
        SetBrush( &animBrush);
      }
      else if (curBrush == &animBrush)
      {
        SetBrush( &aimBrush);
      }
      else if (curBrush == &aimBrush)
      {
        SetBrush( &treadBrush);
      }
      else if (curBrush == &treadBrush)
      {
        SetBrush( &vertBrush);
      }
      else
      {
        SetBrush( &viewBrush);
      }
      break;
    case 0xFA344BEA: // "brush.toggle"
      curBrush->Toggle();
      break;

    case 0x02C25363: // "brush.moveXKey"
      if (*brushMoveXKey)
      {
        lastMX = brushMoveX;
        brushMoveX = *brushMoveXKey;
      }
      else
      {
        brushMoveX = lastMX;
      }
      break;
    case 0xDEAFC9D4: // "brush.moveYKey"
      if (*brushMoveYKey)
      {
        lastMY = brushMoveY;
        brushMoveY = *brushMoveYKey;
      }
      else
      {
        brushMoveY = lastMY;
      }
      break;
    case 0xBED87BBA: // "brush.moveZKey"
      if (*brushMoveZKey)
      {
        lastMZ = brushMoveZ;
        brushMoveZ = *brushMoveZKey;
      }
      else
      {
        brushMoveZ = lastMZ;
      }
      break;

    case 0x829AB27F: // "app.toggle.interface"
      showInterface = !*showInterface;
      break;
    case 0x9B5D3CD4: // "app.show.interface"
      {
        static Bool actives[11] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

        if (*showInterface)
        {
          if (actives[0])
          {
            IFace::Activate( "ListBoxFile");
            actives[0] = 0;
          }
          if (actives[1])
          {
            IFace::Activate( "MeshOptions");
            actives[1] = 0;
          }
          if (actives[2])
          {
            IFace::Activate( "View::Graphics");
            actives[2] = 0;
          }
          if (actives[3])
          {
            IFace::Activate( "Vid::Graphics");
            actives[3] = 0;
          }
          if (actives[4])
          {
            IFace::Activate( "Vid::Mesh");
            actives[4] = 0;
          }
          if (actives[5])
          {
            IFace::Activate( "AnimControl");
            actives[5] = 0;
          }
          if (actives[6])
          {
            IFace::Activate( "ViewBrushControl");
            actives[6] = 0;
          }
          if (actives[7])
          {
            IFace::Activate( "VertBrushControl");
            actives[7] = 0;
          }
          if (actives[8])
          {
            IFace::Activate( "ViewerErrors");
            actives[8] = 0;
          }
          if (actives[9])
          {
            IFace::Activate( "TreadBrushControl");
            actives[9] = 0;
          }
          if (actives[10])
          {
            IFace::Activate( "AimBrush");
            actives[10] = 0;
          }
        }
        else
        {
          actives[0]  = IFace::Deactivate( "ListBoxFile");
          actives[1]  = IFace::Deactivate( "MeshOptions");
          actives[2]  = IFace::Deactivate( "View::Graphics");
          actives[3]  = IFace::Deactivate( "Vid::Graphics");
          actives[4]  = IFace::Deactivate( "Vid::Mesh");
          actives[5]  = IFace::Deactivate( "AnimControl");
          actives[6]  = IFace::Deactivate( "ViewBrushControl");
          actives[7]  = IFace::Deactivate( "VertBrushControl");
          actives[8]  = IFace::Deactivate( "ViewerErrors");
          actives[9]  = IFace::Deactivate( "TreadBrushControl");
          actives[10] = IFace::Deactivate( "AimBrush");
        }
      }
      break;
    }
  }
  //----------------------------------------------------------------------------

  void ClearData()
  {
    light = NULL;
    sun = NULL;
    selMesh = NULL;
    Mesh::Manager::curParent = NULL;
    childMesh = NULL;
    background = NULL;
    ground = NULL;
    meshmat = NULL;

    quatAngle = PI * 0.25f;
    quatAxis  = Matrix::I.Front();
    quat.Set( quatAngle, quatAxis);

    moveCam.Zero();
    rotateCam.Zero();
    rotateObj.Zero();
    camRotate.Zero();

    attCam.ClearData();
    worldCam.ClearData();
//    worldCam.Position().z = -OBJZ;

	  lightVector.Set( 2.0f, 0, -4.0f);

    curBrush = &viewBrush;

    doAttach = FALSE;

    groundColor = RGBA_MAKE( 0, 255, 0, 122);
    stickColor  = RGBA_MAKE( 255, 0, 0, 122);
    boundsColor = RGBA_MAKE( 255, 0, 0, 122);

    originColor0 = RGBA_MAKE( 255, 0, 0, 122);
    originColor1 = RGBA_MAKE( 0, 255, 0, 122);
    originColor2 = RGBA_MAKE( 255, 255, 0, 122);

    isInit = FALSE;
    loadLock = FALSE;
  }
  //----------------------------------------------------------------------------

  static attach = FALSE;

  Bool Load()
  {
    if (!isInit || *meshName == '\0')
    {
      return FALSE;
    }

    FileDrive drive;
    FileDir dir;
    FileName name;
    FileExt ext;
    Dir::PathExpand( *meshName, drive, dir, name, ext);

    if (*loadAttach || Input::KeyState( Input::SHIFTDOWN))
    {
      const char *p = *attachName;
      while (*p == ' ')
      {
        p++;
      }

      FamilyNode *node = (FamilyNode *) selMesh->Find( p);
      if (!node)
      {
        return FALSE;
      }
      SetMessage( "Attaching ", name.str, "...");

      MeshEnt *newMesh = NULL;

      Bool isGod = !Utils::Stricmp( ext.str, ".god");
      Vid::Var::doLoadGod = isGod;

      Utils::Strcat( name.str, ext.str);

      newMesh = Mesh::Manager::Create( name.str);

      Vid::Var::doLoadGod = FALSE;

      if (Mesh::Manager::readErrors)
      {
        IFace::Activate( "ViewerErrors");
      }

      if (!newMesh)
      {
        return FALSE;
      }
      if (!isGod)
      {
        // no shadow to start
        ((MeshRoot *) &newMesh->Root())->shadowRadius = 0;
      }

      selMesh->AttachAt( *node, *newMesh);

      attach = TRUE;

      SetCurrent( newMesh);

      return TRUE;
    }

    if (*ext.str == '\0')
    {
      return TRUE;
    }

    if (selMesh && !Utils::Stricmp( ext.str, ".xsi") && (*loadCycle || Input::KeyState( Input::ALTDOWN)))
    {
      // load xsi file as a new animation cycle

      if (!stricmp( name.str, selMesh->Root().fileName.str))
      {
        return TRUE;
      }

/*
      char *dashp = name.str + strlen(name.str) - 1;
      while (*dashp != '-' && dashp > name.str) 
      {
        dashp--;
      }
*/
      char *dashp = strchr( name.str, '-');
      if (!dashp || *dashp != '-')
      {
        return TRUE;
      }

      U32 len = U32(dashp - name.str);

      if (strnicmp( name.str, selMesh->Root().fileName.str, len))
      {
        return TRUE;
      }
      // don't load frog pose as an animation
      if (!strnicmp( dashp, "-frog", 5))
      {
        return TRUE;
      }
      SetMessage( "Loading animation cycle ", dashp + 1, "...");

      NameString cycleName;
      strcpy( cycleName.str, dashp + 1);

      Mesh::Manager::ReadAnimCycle( meshName, *(MeshRoot *) &selMesh->Root(), cycleName.str);
  
      selMesh->SetupRenderProc();
      selMesh->SetAnimCycle( cycleName.str);
      if (selMesh->curCycle)
      {
        selMesh->fps = *MeshOptions::animFactor * selMesh->curCycle->animSpeed;
      }

      SetCurrent( selMesh, childMesh);

      if (Mesh::Manager::readErrors)
      {
        IFace::Activate( "ViewerErrors");
      }

      return TRUE;
    }

    SetMessage( "Loading ", meshName, "...");

    if ((!Input::KeyState( Input::CTRLDOWN) && !*loadMulti) && selMesh)
    {
      Mesh::Manager::DisposeAll();
      Mesh::Manager::InitResources();
      attach = FALSE;
      selMesh = NULL;
    }

    // do absolute mrm mode
    Vid::Var::doMRM = FALSE;
    Vid::renderState.mrmFactor1 = 0.0f;
    Mesh::Manager::envMap = background;
    Vid::Var::doLoadGod = FALSE;
    Vid::renderState.zBias = 1.0;

    MeshEnt *meshEnt = NULL;

    MeshRoot *root = NULL;

    if (!Utils::Stricmp( ext.str, ".xsi"))
    {
      root = Mesh::Manager::FindRead( meshName, Vid::Var::scaleFactor, Vid::Var::doMrmGen);
      if (root)
      {
        if (doAnimCycles)
        {
          AddAnimToRoot( *root, meshName, *searchPath);
        }

        // no shadow to start
        root->shadowRadius = 0;
      }
    }
    else if (!Utils::Stricmp( ext.str, ".god"))
    {
      root = Mesh::Manager::FindLoad( meshName);
    }
    else if (!Utils::Stricmp( ext.str, ".gfg") || !Utils::Stricmp( ext.str, ".cfg"))
    {
      MeshConfig config;

      root = config.LoadXSI( meshName);
    }

    if (!root || root->vertices.count <= 0)
    {
      LOG_ERR( ("No geometry in file %s", name.str) );

      root = Mesh::Manager::FindLoad( "engine_pandemic.god");
    }

    meshEnt = Mesh::Manager::Create( root);

    if (!meshEnt || meshEnt->Root().vertices.count <= 0)
    {
      if (meshEnt)
      {
        delete meshEnt;
      }
      LOG_ERR( ("No geometry in file %s", name.str) );

      root = Mesh::Manager::FindLoad( "engine_pandemic.god");
      meshEnt = Mesh::Manager::Create( root);
    }

    SetCurrent( meshEnt);

    Console::ProcessCmd( "app.view.reset");
    Console::ProcessCmd( "app.control.reset");

    U32 meterPerCell = 8;
    U32 cellPerClus  = 4;
    U32 wid = Max<U32>( 200, 8 * (U32) worldWidth) / (meterPerCell * cellPerClus);
    wid *= cellPerClus;
    F32 fwid = F32(wid * meterPerCell);
    F32 center = fwid * 0.5f;
    Terrain::Setup( wid, wid, meterPerCell, center, center, 0.0f);
    Vid::Var::Terrain::shroud = FALSE;
    Terrain::SetTexClamp( FALSE);
    showWater = *showWater;

    statCycleID = selMesh->curCycleID;

//    root->LogHierarchy();

    U32 hitcount = 0;
    NBinTree<AnimList>::Iterator li(&selMesh->Root().animCycles); 
    for (!li; *li; li++, hitcount++)
    {
      AnimList *animCycle = (*li);

      LOG_DIAG( ("cycle: %s", animCycle->name.str) );
    }
    if (!hitcount)
    {
      LOG_DIAG( ("no animations.") );
    }

    U32 i;
    for (i = 0; i < meshEnt->Root().buckys.count; i++)
    {
      BucketLock & bucky = meshEnt->Root().buckys[i];

      LOG_DIAG( ("mat%d: texture0: %s  stage:%d", 
        i, 
        bucky.texture0 ? bucky.texture0->GetName() : "null", 
        bucky.texture0 ? bucky.texture0->GetDesc().dwTextureStage : 0
        ) );
      LOG_DIAG( ("mat%d: texture1: %s  stage:%d", 
        i, 
        bucky.texture1 ? bucky.texture1->GetName() : "null",
        bucky.texture1 ? bucky.texture1->GetDesc().dwTextureStage : 0 
        ) );
    }

    if (Mesh::Manager::readErrors)
    {
      IFace::Activate( "ViewerErrors");
    }

    Vid::Var::doLoadGod = TRUE;
    Terrain::Sky::Load( "engine_sky.xsi", "engine_cloud.tga");
    Vid::Var::doLoadGod = FALSE;
    Terrain::Sky::SetLevel( 0); 

    defMeshName = MeshOptions::meshConfig.name.str;

    Update();

    // reset file system for new saved god files
    FileSys::BuildIndexes();

    IControl * button = IFace::FindByName( "ListBoxFile.Rebuild");
    if (button)
    {
      button->SetEnabled( doGodFiles == 2 ? TRUE : FALSE);
    }

    Quaternion q1( PI * 0.29f, Matrix::I.right);
    Quaternion q2( PI * 0.33f, Matrix::I.right);
    Quaternion q3( PI * 0.44f, Matrix::I.right);

//    Mesh::Manager::render2 = TRUE;

    return TRUE;
  }
  //----------------------------------------------------------------------------

  Bool Setup()
  {
    if (!light) 
    {
      // first time
      background = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, *backName);
      ground     = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, *groundName);

      light = Vid::Light::FindCreate( "camlight");
      if (!light)
      {
        return FALSE;
      }
      if (!sun)
      {
        sun = Vid::Light::FindCreateSun( "sun");
        if (sun)
        {
          Vid::Light::SetSun( 0.30f, 1.0f, 1.0f, 1.0f);
          sun->SetActive( *sunLight);
        }
      }

      rotateObj.y = OBJROTATERATE;

      Vid::Var::varMirror = TRUE;
    }
    Vector lv;
    Vid::CurCamera().WorldMatrix().Transform( lv, lightVector);
    light->SetWorld( lv);

    Color color = *lightColor;
    light->SetColor( color);
    light->SetActive( *camLight);

    if (*ambLight)
    {
      color = *ambColor;
      F32 r = (F32) color.r / 255.0f;
      F32 g = (F32) color.g / 255.0f;
      F32 b = (F32) color.b / 255.0f;
      Vid::SetAmbientColor( r, g, b);
    }
    else
    {
      Vid::SetAmbientColor( 0.0f, 0.0f, 0.0f);
    }

    if (!selMesh && !Load())
    {
      return FALSE;
    }

    Vid::SetFogColor( 0.8f, 0.8f, 0.6f);
    Vid::SetFogRange( 0.0f, 100.0f, 1.0f);
    Vid::SetFarPlane( 1000.0f);

    return TRUE;
  }
  //----------------------------------------------------------------------------

  void AddAnimToRoot( MeshRoot &root, const char *meshName, const char *path)
  {
    FileDrive drive;
    FileDir dir;
    FileName name;
    FileExt ext;
    Dir::PathExpand( meshName, drive, dir, name, ext);

    Dir::Find find;

    NameString extstr;
    strcpy( extstr.str, "*");
    strcat( extstr.str, ext.str);

    U32 len = strlen(name.str);

    // Add each file
    if (Dir::FindFirst(find, path, extstr.str))
    {
      do
      {
          // Don't add directories
        if (*find.finddata.name != '.')
        {
          char *dashp = strchr( find.finddata.name, '-');
          if (dashp)
          {
            char *dotp  = strchr( dashp, '.');
  //          if (dotp && !strnicmp( name.str, find.finddata.name, dashp - find.finddata.name))
            if (dotp && !strnicmp( name.str, find.finddata.name, len))
            {
              // don't load frog pose as an animation
              if (strnicmp( dashp, "-frog", 5))
              {
                NameString cycleName;
                strcpy( cycleName.str, dashp + 1);
                cycleName.str[ dotp - dashp - 1] = '\0';

                if (stricmp( find.finddata.name, meshName))
                {
                  SetMessage( "Loading animation cycle ", cycleName.str, "...");

                  Mesh::Manager::ReadAnimCycle( find.finddata.name, root, cycleName.str);
                }
              }
            }
          }
        }
      } 
      while (Dir::FindNext(find));
    }  

    // Finish find operation
    Dir::FindClose(find);

    if (Dir::FindFirst(find, path, "*."))
    {
      do
      {
        U32 len = strlen( find.finddata.name);
        if (!((len == 1 || len == 2) && *find.finddata.name == '.'))
        {
          char buffer[MAX_BUFFLEN + 1];
          Utils::MakePath( buffer, MAX_BUFFLEN, path, "\\", find.finddata.name);

          AddAnimToRoot( root, meshName, buffer);
        }
      } 
      while (Dir::FindNext(find));

      // Finish find operation
      Dir::FindClose(find);
    }
  }
  //----------------------------------------------------------------------------

  void LoadListBox()
  {
    IControl *ctrl = IFace::FindByName("ListBoxFile");
    Bool active = FALSE;
    if (ctrl)
    {
      if (ctrl->IsActive())
      {
        IFace::Deactivate(ctrl);
      }
      else
      {
        IFace::Activate(ctrl);
        active = TRUE;
      }
    }
    ctrl = IFace::FindByName("MeshOptions");

    if (ctrl)
    {
      if (active)
      {
        IFace::Deactivate(ctrl);
      }
      else
      {
        IFace::Activate(ctrl);
      }
    }
  }
  //----------------------------------------------------------------------------

  Bool LoadDialog( Bool _doAttach) // = FALSE)
  {
    _doAttach;

    if (Vid::isStatus.fullScreen)
    {
      return TRUE;
    }
    return TRUE;
#if 0
	  OPENFILENAME ofn;
    char path[MAX_BUFFLEN + 1], name[MAX_BUFFLEN + 1];

    // char dir[MAX_BUFFLEN + 1];
    //_getcwd( dir, MAX_BUFFLEN);

    // Save current working directory
    char origPath[MAX_PATH];
    getcwd(origPath, MAX_PATH);

    strcpy( path, *meshName);
    strcpy( name, *meshName);

    Utils::Memset( &ofn, 0, sizeof( ofn));
    ofn.lStructSize       = sizeof( OPENFILENAME);
    ofn.lpstrFile			    = path;
    ofn.nMaxFile			    = MAX_BUFFLEN;
    ofn.lpstrFileTitle		= name;
	  ofn.nMaxFileTitle		  = MAX_BUFFLEN;
    ofn.lpstrInitialDir   = lastOpenPath;
  //  ofn.lpstrFilter       = "Mesh Files\0*.xsi;*.x\0\0"; 
    ofn.lpstrFilter       = "Mesh Files\0*.xsi;*.god\0\0"; 

	  ofn.nFilterIndex      = 1;

    ofn.hwndOwner         = Vid::hWnd;
    ofn.hInstance         = Vid::hInst;
  //	ofn.Flags             = OFN_FILEMUSTEXIST;
	  ofn.lpstrDefExt       = "xsi";
	  ofn.lpstrTitle  	    = "Load a mesh file";

    Bool rc = GetOpenFileName( &ofn);

    if (rc)
    {
      // Save working directory
      getcwd(lastOpenPath, MAX_PATH);

      doAttach = _doAttach;
      meshName = ofn.lpstrFile + ofn.nFileOffset;
      //meshName = ofn.lpstrFile;
	  }	

    // Restore original working dir
    chdir(origPath);

    return rc;
#endif
  }
  //----------------------------------------------------------------------------

  Bool SaveDialog()
  {
    if (Vid::isStatus.fullScreen)
    {
      return TRUE;
    }
    return TRUE;

#if 0
	  OPENFILENAME ofn;
    char path[MAX_BUFFLEN + 1], name[MAX_BUFFLEN + 1];

    //char dir[MAX_BUFFLEN + 1];
    //_getcwd( dir, MAX_BUFFLEN);

    // Save current working directory
    char origPath[MAX_PATH];
    getcwd(origPath, MAX_PATH);

    strcpy( path, *meshName);
    strcpy( name, *meshName);

    Utils::Memset( &ofn, 0, sizeof( ofn));
    ofn.lStructSize       = sizeof( OPENFILENAME);
    ofn.lpstrFile			    = path;
    ofn.nMaxFile			    = MAX_BUFFLEN;
    ofn.lpstrFileTitle		= name;
	  ofn.nMaxFileTitle		  = MAX_BUFFLEN;
    ofn.lpstrInitialDir   = lastOpenPath;
  //  ofn.lpstrFilter       = "Mesh Files\0*.xsi;*.x\0\0"; 
    ofn.lpstrFilter       = "Mesh Files\0*.xsi;*.god\0\0"; 

	  ofn.nFilterIndex      = 1;

    ofn.hwndOwner         = Vid::hWnd;
    ofn.hInstance         = Vid::hInst;
  //	ofn.Flags             = OFN_FILEMUSTEXIST;
	  ofn.lpstrDefExt       = "god";
	  ofn.lpstrTitle  	    = "Save a mesh file";

    Bool rc = GetSaveFileName( &ofn);

    if (rc)
    {
      // Save working directory
      getcwd(lastOpenPath, MAX_PATH);

      //selMesh->mesh->Save( ofn.lpstrFile);
      selMesh->Root().Save(ofn.lpstrFile);

      // reset file system for new saved god files
      FileSys::BuildIndexes();
	  }	

    // Restore original working dir
    chdir(origPath);

    return rc;
#endif
  }
  //----------------------------------------------------------------------------

  Bool AddCycleDialog()
  {
    if (Vid::isStatus.fullScreen)
    {
      return TRUE;
    }
    return TRUE;

#if 0
	  OPENFILENAME ofn;
    char path[MAX_BUFFLEN + 1], name[MAX_BUFFLEN + 1];

    //char dir[MAX_BUFFLEN + 1];
    //_getcwd( dir, MAX_BUFFLEN);

    // Save current working directory
    char origPath[MAX_PATH];
    getcwd(origPath, MAX_PATH);

    strcpy( path, *meshName);
    strcpy( name, *meshName);

    Utils::Memset( &ofn, 0, sizeof( ofn) );
    ofn.lStructSize       = sizeof( OPENFILENAME);
    ofn.lpstrFile			    = path;
    ofn.nMaxFile			    = MAX_BUFFLEN;
    ofn.lpstrFileTitle		= name;
	  ofn.nMaxFileTitle		  = MAX_BUFFLEN;
    ofn.lpstrInitialDir   = lastOpenPath;
    ofn.lpstrFilter       = "Mesh Files\0*.xsi\0\0"; 

	  ofn.nFilterIndex      = 1;

    ofn.hwndOwner         = Vid::hWnd;
    ofn.hInstance         = Vid::hInst;
  //	ofn.Flags             = OFN_FILEMUSTEXIST;
	  ofn.lpstrDefExt       = "x";

    char buffer[MAX_BUFFLEN];
    Utils::Sprintf(buffer, MAX_BUFFLEN, "Add a mesh file as the %s cycle", *cycleName);
	  ofn.lpstrTitle  	    = buffer;

    Bool rc = GetOpenFileName( &ofn);

    if (rc)
    {
      // Save working dir
      getcwd(lastOpenPath, MAX_PATH);
    }

    // Restore original working dir
    chdir(origPath);

    if (rc)
    {
      if (!Mesh::Manager::ReadAnimCycle( ofn.lpstrFile, selMesh->Root(), *cycleName))
      {
        return FALSE;
      }
      selMesh->SetAnimCycle( *cycleName);
      selMesh->fps = *MeshOptions::animFactor * selMesh->curCycle->animSpeed;
	  }	

    return rc;
#endif
  }
  //----------------------------------------------------------------------------

  void OnModeChange()
  {
    // Notify input system of mode change
    //Input::OnModeChange();

    // Notify interface of mode change
    IFace::OnModeChange();

    Setup();
    Vid::InitResources( TRUE);
  }
  //----------------------------------------------------------------------------

  void DrawBack()
  {
    VertexTL vertmem[4];
    vertmem[0].vv.x = 0.0f;
    vertmem[0].vv.y = 0.0f;
    vertmem[0].vv.z = 1.0f;
    vertmem[0].rhw  = Vid::RHW( Vid::Math::farPlane);
    vertmem[0].diffuse  = 0xffffffff;
    vertmem[0].specular = 0xff000000;
    vertmem[0].u = 0.0f;
    vertmem[0].v = 0.0f;

    vertmem[1].vv.x = (F32) Vid::viewRect.Width();
    vertmem[1].vv.y = 0.0f;
    vertmem[1].vv.z = vertmem[0].vv.z;
    vertmem[1].rhw  = vertmem[0].rhw;
    vertmem[1].diffuse  = vertmem[0].diffuse;
    vertmem[1].specular = vertmem[0].specular;
    vertmem[1].u = 1.0f;
    vertmem[1].v = 0.0f;

    vertmem[2].vv.x = (F32) Vid::viewRect.Width();
    vertmem[2].vv.y = (F32) Vid::viewRect.Height();
    vertmem[2].vv.z = vertmem[0].vv.z;
    vertmem[2].rhw  = vertmem[0].rhw;
    vertmem[2].diffuse  = vertmem[0].diffuse;
    vertmem[2].specular = vertmem[0].specular;
    vertmem[2].u = 1.0f;
    vertmem[2].v = 1.0f;

    vertmem[3].vv.x = 0.0f;
    vertmem[3].vv.y = (F32) Vid::viewRect.Height();
    vertmem[3].vv.z = vertmem[0].vv.z;
    vertmem[3].rhw  = vertmem[0].rhw;
    vertmem[3].diffuse  = vertmem[0].diffuse;
    vertmem[3].specular = vertmem[0].specular;
    vertmem[3].u = 0.0f;
    vertmem[3].v = 1.0f;

    Vid::SetTexture( background);

    Vid::DrawFanStripPrimitive( 
      PT_TRIANGLEFAN,
      FVF_TLVERTEX,
      vertmem,
      4,
      DP_DONOTUPDATEEXTENTS | DP_DONOTCLIP | RS_BLEND_DEF);

  }
  //----------------------------------------------------------------------------

  void DrawGround()
  {
    // set bucket primitive description
    Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | RS_2SIDED | RS_BLEND_DEF);

    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( ground, (groundColor & 0xff000000) < 0xff000000);
    Vid::SetWorldTransform( Matrix::I);

    Vector origin = Mesh::Manager::curParent->Origin();
    origin.y = 0.0f;
    F32 radius = Mesh::Manager::curParent->ObjectBounds().Radius() + 10.0f;

    VertexTL vertmem[4];
    U16 indexmem[] = { 0, 1, 2, 0, 2, 3 };

    vertmem[0].vv.x = origin.x - radius;
    vertmem[0].vv.y = origin.y;
    vertmem[0].vv.z = origin.z + radius;
    vertmem[0].diffuse  = groundColor;
    vertmem[0].specular = 0xff000000;
    vertmem[0].u = 0.0f;
    vertmem[0].v = 0.0f;

    vertmem[1].vv.x = origin.x + radius;
    vertmem[1].vv.y = origin.y;
    vertmem[1].vv.z = origin.z + radius;
    vertmem[1].diffuse  = vertmem[0].diffuse;
    vertmem[1].specular = vertmem[0].specular;
    vertmem[1].u = 1.0f;
    vertmem[1].v = 0.0f;

    vertmem[2].vv.x = origin.x + radius;
    vertmem[2].vv.y = origin.y;
    vertmem[2].vv.z = origin.z - radius;
    vertmem[2].diffuse  = vertmem[0].diffuse;
    vertmem[2].specular = vertmem[0].specular;
    vertmem[2].u = 1.0f;
    vertmem[2].v = 1.0f;

    vertmem[3].vv.x = origin.x - radius;
    vertmem[3].vv.y = origin.y;
    vertmem[3].vv.z = origin.z - radius;
    vertmem[3].diffuse  = vertmem[0].diffuse;
    vertmem[3].specular = vertmem[0].specular;
    vertmem[3].u = 0.0f;
    vertmem[3].v = 1.0f;

    Vid::ProjectClip( vertmem, 4, indexmem, 6);

    origin.x -= Mesh::Manager::curParent->ObjectBoundsRender().Radius();
    vertmem[0].vv.x = origin.x;
    vertmem[0].vv.y = origin.y + 1.0f;
    vertmem[0].vv.z = origin.z;
    vertmem[0].diffuse  = stickColor;

    vertmem[1].vv.x = origin.x + 0.2f;
    vertmem[1].vv.y = origin.y + 1.0f;
    vertmem[1].vv.z = origin.z;
    vertmem[1].diffuse  = vertmem[0].diffuse;

    vertmem[2].vv.x = origin.x + 0.2f;
    vertmem[2].vv.y = origin.y;
    vertmem[2].vv.z = origin.z;
    vertmem[2].diffuse  = vertmem[0].diffuse;

    vertmem[3].vv.x = origin.x;
    vertmem[3].vv.y = origin.y;
    vertmem[3].vv.z = origin.z;
    vertmem[3].diffuse  = vertmem[0].diffuse;

    Vid::ProjectClip( vertmem, 4, indexmem, 6);
  }
  //----------------------------------------------------------------------------

  void DrawOrigin()
  {
    if (!*showOrigin)
    {
      return;
    }

    MeshRoot *root = Mesh::Manager::FindLoad( "engine_arrow.god");

    if (!root)
    {
      return;
    }

    selMesh->RenderOrigin( originColor0, childMesh, originColor1);

    if (*showOffset && childMesh && selMesh != childMesh)
    {
      F32 s = 0.1f / root->ObjectBounds().Radius() * selMesh->ObjectBoundsRender().Radius();

      Matrix matrix, scale;
      scale.ClearData();
      scale.right.x = s;
      scale.up.y    = s;
      scale.front.z = s;

      matrix.ClearData();
      selMesh->FindOffset( childMesh, matrix);

      matrix = matrix * selMesh->WorldMatrixRender();
      scale.right.x *= 2.0f;
      scale.up.y    *= 2.0f;
      scale.front.z *= 2.0f;
      matrix = scale * matrix;

      root->RenderColor( matrix, originColor2);
    }
  }
  //----------------------------------------------------------------------------

  static U32 pvcount, ptcount, cvcount, ctcount;

  void Process()
  {
    if (!Vid::isStatus.initialized)
    {
      return;
    }

    // Process input and interface events
    // must be first
    IFace::Process();

    if (!Vid::isStatus.initialized)
    {
      return;
    }

    // update statistics
    // 
    if (MeshOptions::reset)
    {
      selMesh = MeshOptions::ent;
      selMesh = selMesh;
      Mesh::Manager::curParent = selMesh;
      childMesh = selMesh;
      MeshOptions::reset = FALSE;
    }

    NList<MeshEnt>::Iterator li( &Mesh::Manager::entList);     
    if (Vid::renderState.status.shadowType >= MeshRoot::shadowSEMILIVE)
    {
      // render shadow textures
      // 
      const Matrix * lightA[2];
      U32 lCount = 0;
      if (*sunLight)
      {
        lightA[lCount++] = &Vid::Light::shadowMatrix;
      }
      if (*camLight)
      {
//        lightA[lCount++] = &light->WorldMatrix();
      }
      if (lCount)
      {
        while ( MeshEnt * ent = li++)
        {
          if (ent->Root().shadowType == MeshRoot::shadowSEMILIVE || ent->Root().shadowType == MeshRoot::shadowLIVE)
          {
            ent->RenderShadowTexture( lightA, lCount);
          }
        }
      }
    }

    if (selMesh)
    {
      SetCurrName();

      // setup local vars
      //
      if (mrmAbsolute)
      {
        mrmFactorAbs  = Vid::renderState.mrmFactor1;
      }
      else
      {
        mrmFactorDist = Vid::renderState.mrmFactor1;
      }

      // setup mrm
      //
      MeshRoot & root = *((MeshRoot *) &selMesh->Root());
      if (root.useMrm)
      {
        if (!*Vid::Var::doMRM)
        {
          U32 count = root.vertices.count;
          count -= (U32)(Vid::renderState.mrmFactor1 * (count - root.mrm->minVertCount) / 10.0f);
          selMesh->nextVertCount = count;
//          selMesh->MRMSetVertCount( count);
        }
        else
        {
          // update auto mrm error factor
          //
  //        Mesh::Manager::UpdateMRMFactor();
        }

  //      MSWRITEV(22, (0, 5, "mrmFactor %f, vertCount %d", 
  //        *Vid::renderState.mrmFactor1, root.vertCount));
      }
      else if (root.mrm && root.vertices.count < root.vertices.count)
      {
        selMesh->MRMSetFull();
      }

      // update mrm vert/face display
      //
      MeshOptions::SetupQuick();

      // start render
      //
      Vid::RenderBegin();
      Vid::RenderClear();

      Bool text = Vid::SetTextureStateI(TRUE);
      U32 shade = Vid::SetShadeStateI( Vid::shadeGOURAUD);

      if (Vid::renderState.status.mirror && Vid::Mirror::Start())
      {
        while (Vid::Mirror::LoopStart())
        {
          if (*showGround)
          {
            Terrain::Sky::Render();
            Terrain::Render();
          }

          // restore render
          //
          Vid::SetShadeStateI( shade);
          if (!text)
          {
            Vid::SetTextureStateI(text);
            Vid::SetTextureDX( NULL);
          }

          // render mirrored meshes
          // 
          !li;  while ( MeshEnt * ent = li++)
          {
            ent->Render();
          }
          // flush mirrored models
          //
          Vid::FlushBuckets();
          Vid::FlushTranBuckets();

          // back to no wire
          Vid::SetTextureStateI(TRUE);
          Vid::SetShadeStateI( Vid::shadeGOURAUD);

          Vid::Mirror::LoopStop();
        }
        Vid::Mirror::Stop();
      }

      // render background, ground
      //
//      DrawBack();
      if (*showGround)
      {
        Terrain::Sky::Render();
        Terrain::Render();
      }
      // flush ground
      //
      Vid::FlushBuckets();
      Vid::FlushTranBuckets();

      // render shadows
      //
      !li;  while ( MeshEnt * ent = li++)
      {
        if (*showGround && Vid::renderState.status.showShadows && (*sunLight || *camLight))
        {
          if (Terrain::BoundsTestShadowWithWater( *ent))
          {
//            ent->shadowInfo.color = 0xffffffff;
            Terrain::RenderShadowWithWater( *ent);
          }
        }
      }

      // render selection ring
      //
      if (*showGround && selMesh)
      {
        // render selection ring
        //
        Bitmap * texture = Bitmap::Manager::FindCreate( Bitmap::reduceNONE, "engine_healthbar.tga");

        F32 radius = selMesh->ObjectBoundsRender().Radius();

        Terrain::RenderGroundSpriteWithWater
        ( 
          selMesh->WorldMatrixRender().Position(), radius, radius, 
          selMesh->WorldMatrixRender().Front(),
          texture, stickColor, RS_BLEND_DEF, 
          UVPair(0,0), UVPair(1,0), UVPair(1,1)
        );
      }
      
      // flush
      //
      Vid::FlushBuckets();
      Vid::FlushTranBuckets();

      // restore render
      //
      Vid::SetShadeStateI( shade);
      if (!text)
      {
        Vid::SetTextureStateI(text);
        Vid::SetTextureDX( NULL);
      }

      // render meshes
      // 
      !li;  while ( MeshEnt * ent = li++)
      {
        ent->Render();
      }

      // flush
      //
      Vid::FlushBuckets();
      Vid::FlushTranBuckets();

      Vid::SetTextureStateI(TRUE);
      Vid::SetShadeStateI( Vid::shadeGOURAUD);

      if (Vid::Var::showSP0)
      {
        selMesh->RenderShadowPlane( 0, Color( U32(255), U32(0), U32(0), U32(222)) ); 
      }
      if (Vid::Var::showSP1)
      {
        selMesh->RenderShadowPlane( 1, Color( U32(255), U32(255), U32(0), U32(222)) ); 
      }

      DrawOrigin();

      curBrush->Render();

      // flush
      //
      Vid::FlushBuckets();
      Vid::FlushTranBuckets();

      // restore render
      //
      Vid::SetShadeStateI( shade);
      if (!text)
      {
        Vid::SetTextureStateI(text);
        Vid::SetTextureDX( NULL);
      }

      IFace::Render();

      // end render
      //
      Vid::RenderEnd();

      if (*showShadowTex && Vid::renderState.status.shadowType >= MeshRoot::shadowLIVE && selMesh->shadowTexture)
      {
        selMesh->shadowTexture->CopyBits
        ( 
          Vid::backBmp, 0, 0, 0, 0, 
          selMesh->shadowTexture->Width(), 
          selMesh->shadowTexture->Height()
        );
      }

      Vid::RenderFlush();

      // simulate anim, etc.
      //
      Update();
    }
    else
    {
      Vid::RenderBegin();

      static U32 firstTime = 1;
      if (firstTime)
      {
        DrawBack();
        firstTime = 0;
      }

      IFace::Render();

      // end render
      //
      Vid::RenderEnd();
	    Vid::RenderFlush();
    }

  }
  //----------------------------------------------------------------------------

  void Update()
  {
    Camera &cam = Vid::CurCamera();

    const S32 elapTicks = 100;
    const F32 elapSecs  = (F32) elapTicks / 1000.0f;

    if (*animActive)
    {
      Terrain::SimulateViewer( Main::elapSecs);
    }
    if (childMesh)
    {
      attachName = childMesh->GetName();
    }

    static S32 lastTime = -elapTicks;
    S32 elap = Main::thisTime - lastTime;
    if (elap >= elapTicks)
    {
      F32 felap = F32(Main::thisTime - lastTime) / 1000.0f;
      lastTime = Main::thisTime;

      NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
      while (MeshEnt * ent = li++)
      {
        if (!ent->EntParent())
        {
          // copy target states to current states
          //
          ent->UpdateSim( elapSecs);
        }
      }

      if (*animActive)
      {
        // setup this sim frame from last target
        //
        NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
        while (MeshEnt * ent = li++)
        {
          if (!ent->EntParent())
          {
            // setup display states
            //
            ent->SimulateInt( Vid::Var::doInterpolate ? Main::elapSecs : felap);
          }
          ent->SimulateIntBasic( Main::elapSecs);
        }
        if (doRotate)
        {
          Quaternion q( Mesh::Manager::curParent->GetSimRotation());
    //      q *= Quaternion( rotateObj.x * elapSecs, Mesh::Manager::curParent->WorldMatrix().Right());
          q *= Quaternion( PI * 0.25f * elapSecs, Mesh::Manager::curParent->WorldMatrix().Up());

          Mesh::Manager::curParent->SetSimTarget( q);      
        }
      }
      else
      {
        // setup this sim frame from last target
        //
        NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
        while (MeshEnt * ent = li++)
        {
          ent->ActivateAnim( FALSE);

          if (!ent->EntParent())
          {
            // setup display states
            //
            ent->SimulateInt( Vid::Var::doInterpolate ? Main::elapSecs : felap);
          }
          ent->SimulateIntBasic( Main::elapSecs);
        }
      }
    }

    // render interpolated frame
    //
    else if (Vid::Var::doInterpolate)
    {
      if (*animActive)
      {
        NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
        while (MeshEnt * ent = li++)
        {
          if (!ent->EntParent())
          {
            ent->SimulateInt( Main::elapSecs);
          }
          ent->SimulateIntBasic( Main::elapSecs);
        }
      }
      else
      {
        if (stepHold && stepTime <= Main::thisTime)
        {
          stepTime = Main::thisTime + SINGLESTEPTIME;

          NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
          while (MeshEnt * ent = li++)
          {
            if (!ent->EntParent())
            {
              ent->SetFrame( ent->animState0.curFrame + 1.0f);
            }
          }
          if (selMesh->curCycle)
          {
            animFrameLock = TRUE;
            animFrame = selMesh->animStateR.curFrame + 1.0f;
            animFrameLock = FALSE;
          }
        }
      }
    }
    if (selMesh->curCycle && *animActive)
    {
      animFrameLock = TRUE;
      animFrame = selMesh->animStateR.curFrame + 1.0f;
      animFrameLock = FALSE;
    }

    if (*animLoop)
    {
      // force loop behavior even on non-looping animation cycles
      //
      NList<MeshEnt>::Iterator li(&Mesh::Manager::entList);
      while (MeshEnt * ent = li++)
      {
        if (ent->curCycle && !ent->AnimIsActive())
        {
          ent->ActivateAnim();
        }
      }
    }

    // treads
    //
    if (*treadActive && selMesh && selMesh->Root().hasTread)
    {
      Terrain::moveSpeed = *treadSpeed;
      for (U32 k = 0; k < selMesh->states0.count; k++)
      {
        NodeIdent ident;
        ident.index = k;
        selMesh->SetTreadRate( ident, *treadSpeed);
      }
    }

    // aim
    //
    if (curBrush == &aimBrush && childMesh && childMesh != (MeshObj *) selMesh)
    {
#if 0
      Bool hit = FALSE;
      if (selMesh->curCycle)
      {
        List<Animation>::Iterator li(selMesh->curCycle); 

        for (!li; *li; li++)
        {
          Animation & anim = *(*li);
          if (anim.index == childMesh->GetIndex())
          {
            hit = TRUE;
            break;
          }
        }
      }
      if (!hit)
      {
//        selMesh->states0[childMesh->GetIndex()].SetSlaveObject( selMesh->Root().states[childMesh->GetIndex()]);
//        selMesh->statesR[childMesh->GetIndex()].SetSlaveObject( selMesh->Root().states[childMesh->GetIndex()]);
      }
#endif

      Quaternion q;
      q.ClearData();
      Bool hit = FALSE;

      if (*aimX)
      {
        q *= Quaternion( (*aimFactor - PI), Matrix::I.right);
        hit = TRUE;
      }
      if (*aimY && (!hit || *aimMix))
      {
        q *= Quaternion( (*aimFactor - PI), Matrix::I.up);        
        hit = TRUE;
      }
      if (*aimZ && (!hit || *aimMix))
      {
        q *= Quaternion( (*aimFactor - PI), Matrix::I.front);
      }
#if 1
      NodeIdent ident;
      ident.index = childMesh->GetIndex();
//      Matrix mat = selMesh->states0[childMesh->GetIndex()].ObjectMatrixPriv();
//      mat.Set(q);
//      selMesh->SetSimTarget( ident, mat);
      selMesh->SetSimTarget( ident, q);
#else
      AnimKey * key = &selMesh->states1[childMesh->GetIndex()];
      key->Set( key->quaternion * q);
      key = &selMesh->statesR[childMesh->GetIndex()];
      key->Set( key->quaternion * q);

      // setup render states
      //
      FamilyState * ss, * es = &selMesh->statesR[selMesh->statesR.count];
      for (ss = &selMesh->statesR[1]; ss < es; ss++)
      {
        if (ss->type & animDIRTY)
        {
          ss->SetObject();
          ss->SetObjectScale();

          ss->type &= ~animALLDIRTY;
        }
      }
      selMesh->SetWorldRecurseRender( selMesh->statesR[0].WorldMatrix(), &selMesh->statesR[0]);
#endif
    }

    // update camera position
    camVector += moveCam * Main::elapSecs;

    // update camera rotation
    camRotate += rotateCam * Main::elapSecs;
    if (camRotate.x > PIBY2)
    {
      camRotate.x = PIBY2;
    }
    else if (camRotate.x < -PIBY2)
    {
      camRotate.x = -PIBY2;
    }

//	  worldCam.Rotate( right, );
    Quaternion q;
	  q.Set( -camRotate.x, Matrix::I.Right());
	  attCam = q;
//    worldCam.Rotate( up, Matrix::I.Up());
	  q.Set( camRotate.y, Matrix::I.Up());
	  attCam *= q;
	  worldCam.Set( attCam);

    Matrix camMatrix = worldCam;
    worldCam.Rotate( camMatrix.posit, camVector);
    camMatrix.posit += camOffset;
    cam.SetWorld( camMatrix);

    // apply camera motion drag
	  float drag = 1.0f - DRAG * Main::elapSecs;
	  if (drag < 0.0f)
	  {
		  drag = 0.0f;
	  }
	  if (!moveX1 && !moveX2) 
	  {	
		  moveCam.x *= drag;
	  }
	  if (!moveY1 && !moveY2) 
	  {
		  moveCam.y *= drag;
	  }
	  if (!moveZ1 && !moveZ2) 
	  {
		  moveCam.z *= drag;
	  }
 
    if (!rotX1 && !rotX2)
    {
 		  rotateCam.x *= drag;
    }
    if (!rotY1 && !rotY2) 
	  {
      rotateCam.y *= drag;
    }

    if (!doRotate)
    {
      rotateObj.x = 0.0f;
    }
    if (!doRotate)
    {
      rotateObj.y = 0.0f;
    }

    // update light position
    if (light && light->IsActive())
	  {
      Vector lv;
      Vid::CurCamera().WorldMatrix().Transform( lv, lightVector);
      light->SetWorld( lv);
	  }
  }
  //----------------------------------------------------------------------------

  Bool FASTCALL DefaultHandler(Event &e)
  {
    return curBrush->DefaultHandler( e);
  }
  //----------------------------------------------------------------------------

  Bool DoCommand( DWORD id)
  {
    id;

#if 0
	  switch (id) {
	  default:
		  if (LOWORD( id) >= ID_DISPLAY_3DDRIVER && LOWORD( id) < ID_DISPLAY_3DDRIVER + MAXD3DDRIVERS)
		  {
			  Vid::InitD3D( LOWORD( id) - ID_DISPLAY_3DDRIVER);
			  break;
		  }
		  if (LOWORD( id) >= ID_DISPLAY_VIDMODE && LOWORD( id) <= ID_DISPLAY_VIDMODE + MAXVIDMODES)
		  {
			  Vid::SetMode( LOWORD( id) - ID_DISPLAY_VIDMODE);
			  break;
		  }
		  return FALSE;

	  case ID_FILE_EXIT:
		  PostMessage( Vid::hWnd, WM_CLOSE, NULL, NULL);
		  break;

	  case ID_FILE_OPEN:
      LoadDialog( FALSE);
		  break;

	  case ID_FILE_SAVE:
      SaveDialog();
		  break;

	  case ID_EDIT_ADDCYCLE:
      AddCycleDialog();
		  break;

    case ID_EDIT_ATTACHMESH:
      LoadDialog( childMesh ? TRUE : FALSE);
		  break;

    case ID_EDIT_DETACHMESH:
      if (childMesh)
      {
        childMesh->Detach();
  //      Mesh::Delete( childMesh);
  //      childMesh = NULL;
  //      EnableMenuItem( hm, ID_EDIT_ATTACHMESH, MF_BYCOMMAND | MF_GRAYED);
  //      EnableMenuItem( hm, ID_EDIT_DETACHMESH, MF_BYCOMMAND | MF_GRAYED);
      }
		  break;

    case ID_DISPLAY_FULLSCREEN:
      Vid::ToggleWindowedMode();
      break;
    case ID_DISPLAY_640X480:
		  Vid::SetSize( 640, 480);
      break;
    case ID_DISPLAY_640X400:
		  Vid::SetSize( 640, 400);
      break;
    case ID_DISPLAY_400X300:
		  Vid::SetSize( 400, 300);
      break;
    case ID_DISPLAY_320X240:
	    Vid::SetSize( 320, 240);
      break;
    case ID_DISPLAY_320X200:
		  Vid::SetSize( 700, 500);
      break;
    }
#endif
	  return TRUE;
  }
  //----------------------------------------------------------------------------

  void Brush::Init()
  {
    VarSys::RegisterHandler("brush.def", MeshView::CmdHandler);
  }
  //----------------------------------------------------------------------------

  void Brush::Done()
  {
    VarSys::DeleteItem("brush.def");
  }
  //----------------------------------------------------------------------------

  Bool FASTCALL Brush::DefaultHandler(Event &e)
  {
    if (e.type == Input::EventID())
    {
      switch (e.subType)
      {
        case Input::MOUSEBUTTONDOWN:
        {
          startPosObj = camOffset;
          startPosCam = camVector;
          startRotCam = camRotate;

          IFace::SetMouseCapture((IControl *)IFace::GameWindow());

          // Save mouse code
          captureCode = e.input.code;

          startMouse = Input::MousePos();
          curMouse   = Input::MousePos();

          if (e.input.state & Input::LBUTTONDOWN)
          {
            MeshObj *child = NULL;
            MeshEnt *mesh = Mesh::Manager::PickAtScreenPos( startMouse.x, startMouse.y, &child);
            if (mesh)
            {
              return SetCurrent( mesh, child);
            }
          }
          break;
        }

        case Input::MOUSEMOVE:
        {
          if (e.input.state & Input::RBUTTONDOWN)
          {
            Camera &cam = Vid::CurCamera();
            Vector pos;
            Vid::TransformFromWorld( pos, selMesh->WorldMatrix().Position());
            F32 factor = pos.z * cam.ProjInvConstY();

            Bool hit = FALSE;
            if (*brushMoveZ)
            {
              // zoom the object in the camera's view - Z
              //
              S32 dy = -startMouse.y + Input::MousePos().y;

              camVector.z = startPosCam.z + F32(dy) * 0.02f * *moveRate;

              hit = TRUE;
            }
            else if (*brushMoveY)
            {
              // move the object in the camera's view - only in Y
              //

              S32 dy = startMouse.y - Input::MousePos().y;

//              worldObj.Position().y = startPosObj.y - (F32) (dy * factor);
              camOffset.y = startPosObj.y + F32(dy) * factor;

              hit = TRUE;
            }

            if (*brushMoveX)
            {
              // move the object in the camera's view - in X and Y
              //                
              S32 dx = startMouse.x - Input::MousePos().x;
              const Vector &v = cam.WorldMatrix().Right();

              camOffset.x = startPosObj.x - F32(dx) * factor * v.x;
              camOffset.z = startPosObj.z - F32(dx) * factor * v.z;
            }
            else
            {
              S32 dx = startMouse.x - Input::MousePos().x;
              S32 dy = startMouse.y - Input::MousePos().y;

              // rotate the object in the camera's view
              //
              if (!hit)
              {
                camRotate.x = startRotCam.x - F32(dy) * PI * 0.003f;
              }
         	    camRotate.y = startRotCam.y + F32(dx) * PI * 0.003f;
            }
          }
          break;
        }
        case Input::MOUSEBUTTONUP:
        {
          IFace::ReleaseMouseCapture((IControl *)IFace::GameWindow());

          break;
        }
      }
    }
    return FALSE;
  }
  //----------------------------------------------------------------------------

  void ViewBrush::Init()
  {
    Brush::Init();
    VarSys::RegisterHandler("brush.nav", MeshView::CmdHandler);
  }
  //----------------------------------------------------------------------------

  void ViewBrush::Done()
  {
    VarSys::DeleteItem("brush.nav");
    Brush::Done();
  }
  //----------------------------------------------------------------------------

  Bool FASTCALL ViewBrush::DefaultHandler(Event &e)
  {
    if (e.type == Input::EventID())
    {
      Bool retValue = Brush::DefaultHandler( e);

      switch (e.subType)
      {
        case Input::MOUSEBUTTONDOWN:
        {
          if (retValue)
          {
            const MeshRoot & root = selMesh->Root();

            if (root.mrm && !*Vid::Var::doMRM)
            {
              const MeshRoot * r = &selMesh->Root();
              F32 count = (F32) r->vertices.count - r->mrm->minVertCount;
              Vid::renderState.mrmFactor1 = (F32) (r->vertices.count - selMesh->vertCount) * 10.0f / count;
            }
          }
          break;
        }
      }
    }

    return FALSE;
  }
  //----------------------------------------------------------------------------

  void VertBrush::Init()
  {
    VarSys::RegisterHandler("brush.vert", MeshView::CmdHandler);
    VarSys::CreateInteger("brush.vert.show", TRUE, VarSys::DEFAULT, &showverts);
    VarSys::CreateInteger("brush.vert.append", FALSE, VarSys::DEFAULT, &append);
    VarSys::CreateCmd("brush.vert.selectnone");
    VarSys::CreateCmd("brush.vert.selectall");

    groupSelect = FALSE;
    Clear();
  }
  //----------------------------------------------------------------------------

  void VertBrush::Done()
  {
    VarSys::DeleteItem("brush.vert");
  }
  //----------------------------------------------------------------------------

  void VertBrush::Clear()
  {
    appendKey = FALSE;
    appendLast = *append;
  }
  //----------------------------------------------------------------------------

  void VertBrush::Render()
  {
    selMesh->RenderSelVerts( Color( U32(255), U32(0), U32(0), U32(222)),
      *showverts, Color( U32(255), U32(255), U32(0), U32(222)));

    if (groupSelect)
    {
      Area<F32> rect( (F32) startMouse.x, (F32) startMouse.y, (F32) curMouse.x, (F32) curMouse.y);
      Vid::RenderRectangleOutline( rect, Color( U32(255), U32(0), U32(0), U32(222)));
    }
  }
  //----------------------------------------------------------------------------

  Bool FASTCALL VertBrush::DefaultHandler(Event &e)
  {
    if (e.type == Input::EventID())
    {
      Brush::DefaultHandler( e);

#if 0
      if (Input::KeyState( Input::SHIFTDOWN))
      {
        appendLast = *append;
        append = appendKey = TRUE;
      }
      else
      {
        append = appendLast;
        appendKey = FALSE;
      }
#endif

      switch (e.subType)
      {
        case Input::MOUSEMOVE:
        {
          if (e.input.state & Input::LBUTTONDOWN)
          {
            curMouse = Input::MousePos();
            if (abs(curMouse.x - startMouse.x) >= 2 || abs(curMouse.y - startMouse.y) >= 2)
            {
              groupSelect = TRUE;
            }
          }
          break;
        }
        case Input::MOUSEBUTTONUP:
        {
          Bool toggle = FALSE;

          if (abs(curMouse.x - startMouse.x) < 8 && abs(curMouse.y - startMouse.y) < 8)
          {
            curMouse.x = startMouse.x + 4;
            curMouse.y = startMouse.y + 4;
            startMouse.x -= 4;
            startMouse.y -= 4;

            toggle = TRUE;
          }
          Area<S32> rect( startMouse.x, startMouse.y, curMouse.x, curMouse.y);
          selMesh->SelectVerts( &rect, *append || appendKey ? TRUE : FALSE, toggle);

          groupSelect = FALSE;

          break;
        }
      }
    }

    return FALSE;
  }
  //----------------------------------------------------------------------------

  void VertBrush::CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
    case 0xC9C1BA61: // "brush.vert.selectnone"
      selMesh->UnSelectVerts();
      break;

    case 0x7A4B9B91: // "brush.vert.selectall"
      selMesh->SelectVerts();
      break;
    }
  
  }
  //----------------------------------------------------------------------------

}
