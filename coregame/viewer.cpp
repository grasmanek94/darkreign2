///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Camera system
//
// 16-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "viewer.h"
#include "viewer_private.h"
#include "vid_public.h"
#include "main.h"
#include "iface.h"
#include "iface_util.h"
#include "input.h"
#include "console.h"
#include "terrain.h"
#include "sound.h"
#include "gametime.h"
#include "common.h"
#include "common_gamewindow.h"
#include "demo.h"
#include "user.h"
#include "environment_quake.h"


#ifdef DEVELOPMENT
  #define LOG_VIEWER LOG_DIAG
#else
  #define LOG_VIEWER(x)
#endif


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{

  // Configuration names
  static const char *CFG_HEIGHT = "Height";
  static const char *CFG_PITCH  = "PitchRatio";

  // User data key
  static GameIdent userDataKey("Viewer::Settings");

  // Camera height
  const F32 DEFAULT_CAMERA_HEIGHT = 40.0F;
  const F32 DEFAULT_PITCH_RATIO = 1.0F;

  const F32 RATIO_MIN = 0.2F;
  const F32 RATIO_MAX = 5.0F;

  // Scroll rates
  const F32 RATE_EDGE_DEFAULT   = 80.0F;
  const F32 RATE_EDGE_MIN       = 0.0F;
  const F32 RATE_EDGE_MAX       = 150.0F;

  const F32 RATE_SPIN_DEFAULT   = 8.0F;
  const F32 RATE_SPIN_MIN       = 0.0F;
  const F32 RATE_SPIN_MAX       = 30.0F;

  const F32 RATE_SWOOP_DEFAULT  = 4.0F;
  const F32 RATE_SWOOP_MIN      = 0.0F;
  const F32 RATE_SWOOP_MAX      = 15.0F;

  const F32 RATE_SCROLL_DEFAULT = 16.0F;
  const F32 RATE_SCROLL_MIN     = 0.0F;
  const F32 RATE_SCROLL_MAX     = 50.0F;

  const F32 RATE_WHEEL_DEFAULT  = 40.0F;
  const F32 RATE_WHEEL_MIN      = 0.0F;
  const F32 RATE_WHEEL_MAX      = 100.0F;

  const F32 RATE_KEY_DEFAULT    = 14.0F;
  const F32 RATE_KEY_MIN        = 0.0F;
  const F32 RATE_KEY_MAX        = 40.0F;


  // System initialized
  static Bool sysInit = FALSE;

  // game.cfg configuration scope
  static FScope *configScope = NULL;

  // Current camera
  Base *current = NULL;

  Bool movie = FALSE;

  // Last camera mode
  U32 lastMode;

  // Terrain filter
  const U32 FLT_SIZE = 3;
  const U32 FLT_ELEM = FLT_SIZE + FLT_SIZE + 1;
  static F32 smoothFilter[FLT_ELEM][FLT_ELEM];

  // Bindings
  VarInteger bindLeft;
  VarInteger bindRight;
  VarInteger bindForward;
  VarInteger bindBack;
  VarInteger bindFreeLook;
  VarInteger bindZoom;

  // Defaults
  static VarFloat defaultHeight;
  static VarFloat pitchRatio;

  // Set from studio for configuring mission
  static VarFloat configHeight;
  static VarFloat configPitchRatio;

  // Sensitivity settings
  VarFloat edgeRate;
  VarFloat spinRate;
  VarFloat swoopRate;
  VarFloat scrollRate;
  VarFloat wheelRate;
  VarFloat keyRate;

  // Swoop parameters
  VarFloat swoopDolly;
  VarFloat maxHeight;
  VarFloat toggleHeight;

  // Game settings
  VarInteger trackTerrain;

  // Camera names
  const char *STANDARD_CAM = "Standard";

  // Current default camera
  const char *defaultCam = STANDARD_CAM;

  // Previously processed game cycle (for GameTimeSim)
  static U32 prevCycle = 0;

  // List of registered cameras
  static NBinTree<Base> viewerList(&Base::node);

  // Console Command handler
  static void CmdHandler(U32 pathCrc);


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Record - Record camera movement
  //
  namespace Record
  {

    // Camera movement block
    const char *CameraBlock = "Camera%d Version 1";

    // Maximum number of entries
    U32 max;

    // Current entries
    U32 count;

    // Buffer
    RecItem *buffer = NULL;

    // List of playback cameras
    List<PlaybackCam> cameras;


    //
    // Initialise recording
    //
    void Init(U32 bufSize = 8000)
    {
      //FIXME(933115219, "aiarossi"); // Tue Jul 27 15:40:19 1999

      // change to use small blocks and a temporary file
      max = bufSize;
      buffer = new RecItem[max];
      count = 0;
    }


    //
    // Shutdown recording
    //
    void Done()
    {
      // Delete cameras
      delete[] buffer;
      buffer = NULL;
    }


    //
    // Record camera position
    //
    void SavePos()
    {
      ASSERT(current)

      if (count < max)
      {
        buffer[count].q = current->GetOrienation();
        buffer[count].p = current->GetPosition();
        count++;
      }  
    }


    //
    // Save demo info
    //
    Bool SaveDemo(BlockFile &bFile)
    {
      ASSERT(buffer)

      char buf[128];
      U32 cameraIdx = 0;

      // Save each camera - in a network game this may retrieve
      // blocks from other players.
      Utils::Sprintf(buf, sizeof(buf), CameraBlock, cameraIdx);

      if (bFile.OpenBlock(buf))
      {
        LOG_VIEWER(("Writing %d camera blocks", count))

        // Write player name

        //!!!

        // Write index count
        bFile.WriteToBlock(&count, sizeof(count));
        
        // Write camera movement block
        bFile.WriteToBlock(buffer, sizeof(RecItem) * count);

        bFile.CloseBlock();

        CON_MSG(("Wrote %.1f seconds of camera action", F32(count) * 0.1F))

        return (TRUE);
      }
      return (FALSE);
    }


    //
    // Load demo info
    //
    Bool LoadDemo(BlockFile &bFile)
    {
      U32 cameraIdx = 0;
      char buf[128];

      for (;; cameraIdx++)
      {
        U32 blockSize;

        // Construct camera names until no more found
        Utils::Sprintf(buf, sizeof(buf), CameraBlock, cameraIdx);

        if (!bFile.OpenBlock(buf, FALSE, &blockSize))
        {
          break;
        }

        // Grab pointer to start of camera data
        U8 *dataPtr = bFile.GetBlockPtr();

        if (dataPtr)
        {
          U32 count = *(U32 *)dataPtr;
          RecItem *itemBuf = (RecItem *)(dataPtr + 4);

          ASSERT(blockSize == count * sizeof(RecItem) + 4)

          // Create a playback cam
          Utils::Sprintf(buf, sizeof(buf), "PlayBack%d", cameraIdx);
          PlaybackCam *newCam = new PlaybackCam(buf, itemBuf, count, FALSE);
          cameras.Append(newCam);
          RegisterViewer(newCam);
        }

        bFile.CloseBlock();
      }

      return (TRUE);
    }


    //
    // Stop the demo
    //
    void StopDemo()
    {
      // Delete all cameras created by demo
      PlaybackCam *cam;
      List<PlaybackCam>::Iterator i(&cameras);

      while ((cam = i++) != NULL)
      {
        cameras.Unlink(cam);
        DeleteViewer(cam);
      }
    }
  }


  //
  // Save user settings
  //
  static void SaveToUser()
  {
    // Save user settings
    FScope fScope(NULL, userDataKey.str);

    if (edgeRate != RATE_EDGE_DEFAULT)   
    {
      StdSave::TypeF32(&fScope, "EdgeRate", edgeRate);
    }
    if (spinRate != RATE_SPIN_DEFAULT)   
    {
      StdSave::TypeF32(&fScope, "SpinRate", spinRate);
    }
    if (swoopRate != RATE_SWOOP_DEFAULT)  
    {
      StdSave::TypeF32(&fScope, "SwoopRate", swoopRate);
    }
    if (scrollRate != RATE_SCROLL_DEFAULT) 
    {
      StdSave::TypeF32(&fScope, "ScrollRate", scrollRate);
    }
    if (wheelRate != RATE_WHEEL_DEFAULT)  
    {
      StdSave::TypeF32(&fScope, "WheelRate", wheelRate);
    }
    if (keyRate != RATE_KEY_DEFAULT)    
    {
      StdSave::TypeF32(&fScope, "KeyRate", keyRate);
    }

    User::SetConfigScope(userDataKey.crc, &fScope);
  }


  //
  // Load user settings
  //
  static void LoadFromUser()
  {
    // Read saved settings from user
    if (FScope *fScope = User::GetConfigScope(userDataKey.crc))
    {
      while (FScope *sScope = fScope->NextFunction())
      {
        switch (sScope->NameCrc())
        {
          case 0x05176B0C: // "EdgeRate"
            edgeRate = StdLoad::TypeF32(sScope);
            break;
          case 0xB36ABAB2: // "SpinRate"
            spinRate = StdLoad::TypeF32(sScope);
            break;
          case 0x36EF438B: // "SwoopRate"
            swoopRate = StdLoad::TypeF32(sScope);
            break;
          case 0x83B8D8C4: // "ScrollRate"
            scrollRate = StdLoad::TypeF32(sScope);
            break;
          case 0xEC16ED83: // "WheelRate"
            wheelRate = StdLoad::TypeF32(sScope);
            break;
          case 0x72A3B37A: // "KeyRate"
            keyRate = StdLoad::TypeF32(sScope);
            break;
        }
      }
    }
  }


  //
  // Register a new camera
  //
  void RegisterViewer(Base *viewer)
  {
    ASSERT(viewer);
    viewerList.Add(Crc::CalcStr(viewer->GetName()), viewer);
  }


  //
  // Delete a camera
  //
  void DeleteViewer(Base *viewer)
  {
    for (NBinTree<Base>::Iterator i(&viewerList); *i; i++)
    {
      if (*i == viewer)
      {
        if (viewer == current)
        {
          // Switch back to default cam
          SetCurrent(defaultCam);
        }

        // Double check not deleting default cam
        if (viewer == current)
        {
          ERR_FATAL(("Don't delete default cam"))
        }

        viewerList.Dispose(viewer);
        return;
      }
    }
  }


  //
  // Init
  //
  void Init()
  {
    ASSERT(!sysInit)

    // Command scope
    VarSys::RegisterHandler("camera", CmdHandler);

    // Commands
    VarSys::CreateCmd("camera.set");
    VarSys::CreateCmd("camera.list");

    // Camera commands
    VarSys::CreateCmd("camera.toggleheight");
    VarSys::CreateCmd("camera.nexttarget");
    VarSys::CreateCmd("camera.prevtarget");
    VarSys::CreateCmd("camera.setyaw");
    VarSys::CreateCmd("camera.setheight");
    VarSys::CreateCmd("camera.rotateto");
    VarSys::CreateCmd("camera.rotateby");

    // Camera movement bindings
    VarSys::CreateInteger("camera.bind.left"    , FALSE, VarSys::DEFAULT, &bindLeft);
    VarSys::CreateInteger("camera.bind.right"   , FALSE, VarSys::DEFAULT, &bindRight);
    VarSys::CreateInteger("camera.bind.forward" , FALSE, VarSys::DEFAULT, &bindForward);
    VarSys::CreateInteger("camera.bind.back"    , FALSE, VarSys::DEFAULT, &bindBack);
    VarSys::CreateInteger("camera.bind.freelook", FALSE, VarSys::DEFAULT, &bindFreeLook);
    VarSys::CreateInteger("camera.bind.zoom"    , FALSE, VarSys::DEFAULT, &bindZoom);

    // Camera settings (either from game.cfg or default otherwise)
    F32 gameCfgHeight = DEFAULT_CAMERA_HEIGHT;
    F32 gameCfgPitchRatio = DEFAULT_PITCH_RATIO;

    // Read settings from game.cfg
    if (configScope)
    {
      gameCfgHeight = StdLoad::TypeF32(configScope, CFG_HEIGHT, gameCfgHeight);
      gameCfgPitchRatio = StdLoad::TypeF32(configScope, CFG_PITCH, gameCfgPitchRatio);
    }

    VarSys::CreateFloat("camera.defaultheight",  gameCfgHeight, VarSys::DEFAULT, &defaultHeight)->SetFloatRange(MIN_HEIGHT, MAX_HEIGHT);
    VarSys::CreateFloat("camera.pitchheightratio",  gameCfgPitchRatio, VarSys::DEFAULT, &pitchRatio)->SetFloatRange(RATIO_MIN, RATIO_MAX);

    // Sensitivity settings
    VarSys::RegisterHandler("camera.rate", CmdHandler);
    VarSys::CreateFloat("camera.rate.edge"   , RATE_EDGE_DEFAULT, VarSys::DEFAULT, &edgeRate)->SetFloatRange(RATE_EDGE_MIN, RATE_EDGE_MAX);
    VarSys::CreateFloat("camera.rate.spin"   , RATE_SPIN_DEFAULT, VarSys::DEFAULT, &spinRate)->SetFloatRange(RATE_SPIN_MIN, RATE_SPIN_MAX);
    VarSys::CreateFloat("camera.rate.swoop"  , RATE_SWOOP_DEFAULT, VarSys::DEFAULT, &swoopRate)->SetFloatRange(RATE_SWOOP_MIN, RATE_SWOOP_MAX);
    VarSys::CreateFloat("camera.rate.scroll" , RATE_SCROLL_DEFAULT, VarSys::DEFAULT, &scrollRate)->SetFloatRange(RATE_SCROLL_MIN, RATE_SCROLL_MAX);
    VarSys::CreateFloat("camera.rate.wheel"  , RATE_WHEEL_DEFAULT, VarSys::DEFAULT, &wheelRate)->SetFloatRange(RATE_WHEEL_MIN, RATE_WHEEL_MAX);
    VarSys::CreateFloat("camera.rate.key"    , RATE_KEY_DEFAULT, VarSys::DEFAULT, &keyRate)->SetFloatRange(RATE_KEY_MIN, RATE_KEY_MAX);

    // Default sensitivity settings
    VarSys::RegisterHandler("camera.default", CmdHandler);
    VarSys::CreateFloat("camera.rate.default.edge"   , RATE_EDGE_DEFAULT, VarSys::NOEDIT);
    VarSys::CreateFloat("camera.rate.default.spin"   , RATE_SPIN_DEFAULT, VarSys::NOEDIT);
    VarSys::CreateFloat("camera.rate.default.swoop"  , RATE_SWOOP_DEFAULT, VarSys::NOEDIT);
    VarSys::CreateFloat("camera.rate.default.scroll" , RATE_SCROLL_DEFAULT, VarSys::NOEDIT);
    VarSys::CreateFloat("camera.rate.default.wheel"  , RATE_WHEEL_DEFAULT, VarSys::NOEDIT);
    VarSys::CreateFloat("camera.rate.default.key"    , RATE_KEY_DEFAULT, VarSys::NOEDIT);

    // Swoop parameters
    VarSys::CreateFloat("camera.swoopdolly" ,  0.0F, VarSys::DEFAULT, &swoopDolly);
    VarSys::CreateFloat("camera.maxheight"  , 90.0F, VarSys::DEFAULT, &maxHeight);
    VarSys::CreateFloat("camera.toggletoheight", 5.0F, VarSys::DEFAULT, &toggleHeight);

    // Game settings
    VarSys::CreateInteger("camera.trackterrain", TRUE, VarSys::NOTIFY, &trackTerrain);

    // Studio settings
    if (Common::InStudio())
    {
      VarSys::RegisterHandler("camera.gameconfig", CmdHandler);
      VarSys::CreateFloat("camera.gameconfig.height", gameCfgHeight, VarSys::DEFAULT, &configHeight)->SetFloatRange(MIN_HEIGHT, MAX_HEIGHT);;
      VarSys::CreateFloat("camera.gameconfig.pitchratio", gameCfgPitchRatio, VarSys::DEFAULT, &configPitchRatio)->SetFloatRange(RATIO_MIN, RATIO_MAX);;
    }

    // Load user settings
    LoadFromUser();

    // Create cameras
    RegisterViewer(new Standard(STANDARD_CAM));
    //RegisterViewer(new TrackCam("TrackCam"));
    RegisterViewer(new PadlockCam("PadlockCam"));

    // Set current
    if (!SetCurrent(defaultCam))
    {
      ERR_FATAL(("Camera has not been setup"));
    }

    // Initialise subsystems
    Record::Init();
    Action::Init();

    // Initialise smoother filter matrix
    F32 slope = 1.5F;
    U32 x, y;

    for (x = 0; x <= FLT_SIZE; x++)
    {
      for (y = 0; y <= FLT_SIZE; y++)
      {
        F32 val = F32(Min(x, y) + 1) * slope;

        smoothFilter[x][y]                        = val;
        smoothFilter[x][FLT_ELEM-y-1]             = val;
        smoothFilter[FLT_ELEM-x-1][FLT_ELEM-y-1]  = val;
        smoothFilter[FLT_ELEM-x-1][y]             = val;
      }
    }

    // Normalize
    F32 total = 0.0F;
    F32 *p;
    U32 c;

    for (p = &smoothFilter[0][0], c = 0; c < FLT_ELEM*FLT_ELEM; total += *p, p++, c++);
    total = 1.0F / total;
    for (p = &smoothFilter[0][0], c = 0; c < FLT_ELEM*FLT_ELEM; *p *= total, p++, c++);

    // Reset vars
    lastMode = 0;
    prevCycle = GameTime::GameCycle() - 1;

    movie = FALSE;

    sysInit = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(sysInit)

    movie = FALSE;

    // Save user settings
    SaveToUser();

    // Delete configuration scope
    if (configScope)
    {
      delete configScope;
      configScope = NULL;
    }

    // Shutdown recording
    Action::Done();
    Record::Done();

    // Clear current
    current = NULL;

    // Delete all cameras
    viewerList.DisposeAll();

    // Delete var scope
    VarSys::DeleteItem("camera");

    sysInit = FALSE;
  }


  //
  // Save
  //
  void Save(FScope *scope)
  {
    for (NBinTree<Base>::Iterator i(&viewerList); *i; i++)
    {
      (*i)->Save(scope->AddFunction((*i)->GetName()));
    }
  }


  //
  // Load
  //
  // Load the cameras
  //
  void Load(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      // Find the camera with the given name
      if (Base *view = viewerList.Find(sScope->NameCrc()))
      {
        view->Load(sScope);
      }
    }
  }


  //
  // Save configuration to game.cfg
  //
  void SaveConfig(FScope *fScope)
  {
    // Only save in the studio?
    if (Common::InStudio())
    {
      // Only save if different from default
      if (configHeight != DEFAULT_CAMERA_HEIGHT)
      {
        // Camera height
        StdSave::TypeF32(fScope, CFG_HEIGHT, configHeight);
      }

      if (configPitchRatio != DEFAULT_PITCH_RATIO)
      {
        // Camera height
        StdSave::TypeF32(fScope, CFG_PITCH, configPitchRatio);
      }

    }
  }


  //
  // Load configuration from game.cfg
  //
  void LoadConfig(FScope *fScope)
  {
    if (Common::InStudio())
    {
      ASSERT(sysInit)
      ASSERT(configScope == NULL)

      // Viewer is already initialised at this point when loading the studio
      configHeight = StdLoad::TypeF32(fScope, CFG_HEIGHT, configHeight);
      configPitchRatio = StdLoad::TypeF32(fScope, CFG_PITCH, configPitchRatio);
    }
    else
    {
      ASSERT(!sysInit)

      // View it not initialised in the game though
      if (configScope)
      {
        delete configScope;
      }
      configScope = fScope->Dup();
    }
  }


  //
  // SetCurrent
  //
  Bool SetCurrent(const char *name, Bool inherit, FScope *fScope)
  {
    U32 crc = Crc::CalcStr(name);
    Base *newView;

    ASSERT(defaultCam)

    if (crc == 0x8F651465) // "Default"
    {
      newView = viewerList.Find(Crc::CalcStr(defaultCam));
    }
    else
    {
      newView = viewerList.Find(crc);
    }

    return SetCurrent(newView, inherit, fScope);
  }


  //
  // SetCurrent
  //
  Bool SetCurrent(Base *newView, Bool inherit, FScope *fScope)
  {
    if (newView)
    {
      // Reactivate current camera if it the same
      if (newView == current)
      {
        newView->Notify(VN_REACTIVATE);
      }
      else
      {
        // Attempt to activate new camera
        if (newView->Notify(VN_ACTIVATE))
        {
          newView->SetFirstSim(TRUE);

          // Success, so deactivate current
          if (current)
          {
            // Apply current camera position
            if (inherit)
            {
              Vector pos = current->GetPosition();
              pos.y = pos.y - Floor(pos.x, pos.z);
              newView->Set(pos, current->GetOrienation());
            }
            current->Notify(VN_DEACTIVATE);
          }
          current = newView;
        }
        else
        {
          LOG_VIEWER(("Camera [%s] could not activate", newView->GetName()))
        }
      }

      // Execute optional scope
      if (fScope)
      {
        FScope *sScope;

        while ((sScope = fScope->NextFunction()) != NULL)
        {
          current->Exec(sScope);
        }
      }
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Delete a camera
  //
  Bool Delete(const char *name)
  {
    ASSERT(sysInit)

    Base *cam = viewerList.Find(Crc::CalcStr(name));

    // Anti PEBKAC
    if (Crc::CalcStr(name) == 0x8F651465) // "Default"
    {
      ERR_FATAL(("DON'T delete default cam"))
    }

    return (Delete(cam));
  }


  //
  // Delete a camera
  //
  Bool Delete(Base *cam)
  {
    if (cam)
    {
      if (cam == current)
      {
        // Deactivate current camera before deleting
        cam->Notify(VN_DEACTIVATE);

        // Activate default
        SetCurrent(defaultCam);
      }

      // Delete its ass
      viewerList.Dispose(cam);

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Simulation
  //
  void Simulate()
  {
    ASSERT(sysInit)
    ASSERT(current);

    Environment::Quake::Simulate( Main::ElapTimeFrame());

    if (Action::IsPlaying())
    {
      Action::PreSimulate(Main::ElapTimeFrame());
    }

    if (current->Simulate(&Vid::CurCamera(), Main::ElapTimeFrame()))
    {
      current->SetFirstSim(FALSE);

      // Check for mode change
      if (lastMode != current->GetMode())
      {
        lastMode = current->GetMode();
      }
    }
    else
    {
      // Simulation aborted, switch back to standard cam
      SetCurrent(defaultCam);
    }
  }


  //
  // Simulation once per game cycle
  //
  void GameTimeSim()
  {
    // Only simulate if a game cycle ticked over
    if (prevCycle != GameTime::GameCycle())
    {
      // Cineractive simulation
      if (Action::IsPlaying())
      {
        Action::GameTimeSim();
      }

      prevCycle = GameTime::GameCycle();
    }
  }


  //
  // Render
  //
  void Render(U32 mode)
  {
    if (Action::IsPlaying())
    {
      Action::Render(mode);
    }
  }


  //
  // Set current camera to position with yaw/pitch
  //
  void Snap(const Vector &pos, F32 yaw, F32 pitch)
  {
    ASSERT(current)

    // Construct transformation matrix
    Quaternion q;

    q.ClearData();
    q.Rotate(-pitch * DEG2RAD, Matrix::I.right);
    q.Rotate(yaw * DEG2RAD, Matrix::I.up);

    // Update camera
    current->Set(pos, q);
  }


  //
  // Smoothly move current camera to new position
  //
  void Move(const Vector &pos, F32 yaw, F32 pitch)
  {
    ASSERT(current)

    // Construct transformation matrix
    Quaternion q;

    q.ClearData();
    q.Rotate(-pitch * DEG2RAD, Matrix::I.right);
    q.Rotate(yaw * DEG2RAD, Matrix::I.up);

    // Update camera
    current->Set(pos, q, Viewer::Base::SET_SMOOTH);
  }


  //
  // Get orientation of current camera
  //
  void GetOrientation(Vector &pos, F32 &yaw, F32 &pitch)
  {
    ASSERT(current);

    const Matrix &m = current->GetMatrix();

    pos   = m.posit;
    pos.y = Max(0.0F, m.posit.y - Floor(pos.x, pos.z));
    yaw   = (F32(atan2(m.front.x, -m.front.z)) * RAD2DEG) - 180.0F;
    pitch = (F32(acos(m.front.Dot(Matrix::I.up))) * RAD2DEG) - 90.0F;
  }


  //
  // Create a custom camera
  //
  void CreateCustomCamera(const char *name, SimulateProc *proc, void *context)
  {
    LOG_VIEWER(("Creating custom camera [%s]", name))
    RegisterViewer(new CustomCam(name, proc, context));
  }


  //
  // Delete a custom camera
  //
  void DeleteCustomCamera(const char *name)
  {
    LOG_VIEWER(("Deleting custom camera [%s]", name))
    Delete(name);
  }


  //
  // Get default height (user configurable)
  //
  F32 GetDefaultHeight()
  {
    ASSERT(sysInit)
    return (*defaultHeight);
  }


  //
  // Get default pitch:height ratio
  //
  F32 GetPitchRatio()
  {
    ASSERT(sysInit)
    return (*pitchRatio);
  }


  //
  // Get pitch for a certain height
  //
  F32 GetPitchForHeight(F32 height)
  {
    return Clamp<F32>(0.0F, (height * GetPitchRatio() * DEG2RAD), PIBY2);
  }


  //
  // Get floor at location, handles off map locations
  //
  F32 Floor(F32 &x, F32 &z)
  {
    ASSERT(sysInit)

#ifdef MIRROR_TERRAIN
    // Max distance allowed to move off map
    F32 maxBuf = Terrain::farPlane * 0.8F;
    F32 xmax = F32(Terrain::MeterWidth());
    F32 zmax = F32(Terrain::MeterHeight());
    F32 tx = x;
    F32 tz = z;

    // Ensure max buffer is not larger than the terrain itself
    maxBuf = Min<F32>(maxBuf, xmax - 0.5F);
    maxBuf = Min<F32>(maxBuf, zmax - 0.5F);

    // Clamp x coordinate
    if (x < 0)
    {
      if (x < -maxBuf)
      {
        x = -maxBuf;
      }
      tx = -x;
    }
    else

    if (x >= xmax)
    {
      if (x > xmax + maxBuf)
      {
        x = xmax + maxBuf;
      }
      tx = xmax - (x - xmax) - 0.05F;
    }

    // Clamp z coordinate
    if (z < 0)
    {
      if (z < -maxBuf)
      {
        z = -maxBuf;
      }
      tz = -z;
    }
    else

    if (z >= zmax)
    {
      if (z > zmax + maxBuf)
      {
        z = zmax + maxBuf;
      }
      tz = zmax - (z - zmax) - 0.05F;
    }

    ASSERT(tx >= 0 && tx < xmax && tz >= 0 && tz < zmax)

    // Get terrain height
    return (TerrainData::FindFloor(tx, tz));
#else
    WorldCtrl::ClampPlayField(x, z);

    // Get terrain height
    return (TerrainData::FindFloor(x, z));
#endif
  }


  //
  // Apply the filter to current cell and surrounding cells
  //
  static F32 ApplyFilter(U32 cx, U32 cz)
  {
    ASSERT(cx >= 0 && cx < Terrain::CellPitch())
    ASSERT(cz >= 0 && cz < Terrain::CellHeight())

    if 
    (
      WorldCtrl::CellOnMap(cx - FLT_SIZE, cz - FLT_SIZE) 
      &&
      WorldCtrl::CellOnMap(cx + FLT_SIZE, cz + FLT_SIZE) 
    )
    {
      F32 height = 0.0F;
      F32 *filter = &smoothFilter[0][0];

      for (U32 x = 0; x < FLT_ELEM; x++)
      {
        for (U32 z = 0; z < FLT_ELEM; z++)
        {
          height += *filter++ * TerrainData::GetHeightWithWater(cx - FLT_SIZE + x, cz - FLT_SIZE + z);
        }
      }
      return (height);
    }
    else
    {
      return (TerrainData::GetHeightWithWater(cx, cz));
    }
  }


  //
  // Find smoothed height at metre position
  //
  F32 SmoothedFloor(F32 &x, F32 &z)
  {
    ASSERT(sysInit)

    F32 heights[4];

    WorldCtrl::ClampPlayField(x, z);

    // trunc mode is the default
    //
    U32 cx = (U32) Utils::FtoL(x * WC_CELLSIZEF32INV); 
    U32 cz = (U32) Utils::FtoL(z * WC_CELLSIZEF32INV);

    heights[0] = ApplyFilter(cx    , cz    );
    heights[1] = ApplyFilter(cx    , cz + 1);
    heights[2] = ApplyFilter(cx + 1, cz + 1);
    heights[3] = ApplyFilter(cx + 1, cz    );

    return Movement::GetFloorHelper(heights, x - F32(cx) * WC_CELLSIZEF32, z - F32(cz) * WC_CELLSIZEF32);
  }


  //
  // Console Command handler
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x7447A9E7: // "camera.set"
      {
        const char *name;

        if (Console::GetArgString(1, name))
        {
          SetCurrent(name);
        }
        break;
      }

      case 0xDA4A44D7: // "camera.list"
      {
        for (NBinTree<Base>::Iterator i(&viewerList); *i; i++)
        {
          CON_DIAG(((*i)->GetName()))
        }
        break;
      }

      default:
      {
        ASSERT(current)
        current->CmdHandler(pathCrc);
        break;
      }
    }
  }
}
