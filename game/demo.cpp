///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Demo recording/playback
//
// 25-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "demo.h"
#include "varsys.h"
#include "console.h"
#include "main.h"
#include "blockfile.h"
#include "version.h"
#include "gametime.h"
#include "game_rc.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "iface_util.h"
#include "random.h"

#include "orders.h"
#include "viewer.h"
#include "iclistbox.h"
#include "missions.h"
#include "user.h"

#include "vid.h"
#include "vid_public.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Demo
//
namespace Demo
{

  // Demo directory
  const char *DemoDir = "demos";

  // Demo extension
  const char *DemoExtn = "demo";
  const char *AutoDemoExtn = "auto_demo";

  // Version name block
  const char *VersionBlock = "Dark Reign ][ Demo";

  // Build block
  static const char *BuildBlock = "BuildString";

  // Mission name block
  static const char *MissionBlock = "Mission";

  // Current version
  static const U32 Version = 1;

  // Default camera to use when starting a new demo playback
  static const char *DefaultCamera = "Playback0";

  // Demo state
  enum DemoState
  {
    NONE,
    LOADING,
    PLAYING,
    DONE,
  };

  // System initialised?
  static Bool sysInit = FALSE;

  // Demo state
  static DemoState state = NONE;

  // Length of demo in game cycles
  static U32 demoCycles = 0;

  // Notification function
  typedef void (NotifyProc)(U32);
  static NotifyProc *notifyProc = NULL;

  // Demo block file
  static BlockFile blockFile;

  // Last demo name
  static FileName lastDemo;

  // Initial camera
  static GameIdent initialCam;

  // Output file
  static VarString recordName;

  // Loop the demo?
  static VarInteger loopDemo;

  // Time between looping benchmarks
  static VarInteger benchmarkDelay;

  // Current state of demo
  static VarInteger playing;

  // Capturing movies
  static VarInteger movieCapture;

  // Benchmark results window
  static IControlPtr msgWindow;

  static U32 startTime;
  static U32 endTime;
  static U32 startFrame;
  static U32 endFrame;

  // Command handler
  void CmdHandler(U32 pathCrc);

  // Notify functions
  static void StopDemo();
  static void NotifyDemo(U32);
  static void NotifyBenchmark(U32);
  static void NotifyRandom(U32);


  //
  // Initialise
  //
  void Init()
  {
    ASSERT(!sysInit)

    // Register command handler
    VarSys::RegisterHandler("demo", CmdHandler);

    // Create commands
    VarSys::CreateCmd("demo.save");
    VarSys::CreateCmd("demo.play");
    VarSys::CreateCmd("demo.benchmark");
    VarSys::CreateCmd("demo.stop");
    VarSys::CreateCmd("demo.buildlist");
    VarSys::CreateCmd("demo.random");

    // Create vars
    VarSys::CreateString("demo.record", "", VarSys::DEFAULT, &recordName);
    VarSys::CreateInteger("demo.loop", FALSE, VarSys::DEFAULT, &loopDemo);
    VarSys::CreateInteger("demo.benchtime", 8, VarSys::DEFAULT, &benchmarkDelay);
    VarSys::CreateInteger("demo.playing", FALSE, VarSys::NOEDIT, &playing);
    VarSys::CreateInteger("demo.moviecapture", FALSE, VarSys::DEFAULT, &movieCapture);

    *initialCam.str = 0;

    // System is up
    sysInit = TRUE;
  }


  //
  // Done
  //
  void Done()
  {
    ASSERT(sysInit)

    // notify StopDemo that we're shutting down
    sysInit = FALSE;

    // Ensure the demo is stopped
    StopDemo();

    // Delete var scope
    VarSys::DeleteItem("demo");
  }


  //
  // Set the state
  //
  static void SetState(DemoState s)
  {
    state = s;
    playing = (state == PLAYING) ? TRUE : FALSE;
  }

  //
  // Stop demo playback
  //
  void StopDemo()
  {
    //ASSERT(state != LOADING)

    if (state == PLAYING)
    {
      SetState(DONE);

      // Clear notification proc
      notifyProc = NULL;

      // Kill the playback cameras
      Viewer::Record::StopDemo();

      // Close the block file
      blockFile.Close();
      LOG_DIAG(("Demo Blockfile closed"))
    }
  }


  // 
  // Demo processing
  //
  void Process()
  {
    if (movieCapture && GameTime::GetFastForwardMode())
    {
      IFace::ScreenDump("Movie capture frame %08d.bmp");
    }

    switch (state)
    {
      case PLAYING:
      {
        if (GameTime::GameCycle() == 1)
        {
          if (notifyProc)
          {
            notifyProc(0x3B8E7EE1); // "Start"
          }
        }
        else

        if (GameTime::GameCycle() == demoCycles)
        {
          CON_MSG(("Demo complete"))

          // Call termination function
          if (notifyProc)
          {
            notifyProc(0xB2C53B91); // "End"
          }

          StopDemo();
        }
        break;
      }

      case DONE:
      {
        // After a while close the window and restart the benchmark
        if (loopDemo && msgWindow.Alive())
        {
          // Wait n seconds before looping
          if ((Clock::Time::Ms() - endTime) > U32(benchmarkDelay * 1000))
          {
            // Repeat last demo
            Benchmark(lastDemo.str, initialCam.str);
          }
        }
        break;
      }

      case NONE:
      {
        // Save camera position each tick
        //if (!Viewer::movie)
        {
          Viewer::Record::SavePos();
        }
        break;
      }
    }
  }


  //
  // Save mission name
  //
  static Bool SaveMissionInfo(BlockFile &bFile)
  {
    if (bFile.OpenBlock(MissionBlock))
    {
      // Get the active mission
      const Missions::Mission *mission = Missions::GetActive();

      ASSERT(mission)

      // Write the group path
      const char *path = mission->GetGroup().GetPath().str;
      U16 len = U16(Utils::Strlen(path) + 1);
      bFile.WriteToBlock(&len, 2);
      bFile.WriteToBlock(path, len);

      // Write the mission name
      const char *name = mission->GetName().str;
      len = U16(Utils::Strlen(name) + 1);
      bFile.WriteToBlock(&len, 2);
      bFile.WriteToBlock(name, len);

      // Write mission CRC
      U32 crc = mission->GetDataCrc();
      bFile.WriteToBlock(&crc, 4);

      // Write demo length
      U32 demoLength = GameTime::GameCycle();
      bFile.WriteToBlock(&demoLength, 4);

      bFile.CloseBlock();

      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Save Build Info
  //
  static Bool SaveBuildInfo(BlockFile &bFile)
  {
    if (bFile.OpenBlock(BuildBlock))
    {
      const char *name = Version::GetBuildString();
      U16 len = U16(Utils::Strlen(name) + 1);

      // Save build string
      bFile.WriteToBlock(&len, 2);
      bFile.WriteToBlock(name, len);

      bFile.CloseBlock();

      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Save demo
  //
  Bool Save(const char *file, U32 flags)
  {
    ASSERT(file)

    // Make sure we have a mission
    if (!Missions::Initialized() || !Missions::GetActive())
    {
      return (FALSE);
    }

    // Create output directory
    FileDir demoDir(DemoDir);

    if (!File::Exists(demoDir.str, "."))
    {
      Dir::Make(demoDir.str);
    }

    // Write to user directory
    BlockFile bFile;
    FilePath path;
    Dir::PathMake(path, "", DemoDir, file, DemoExtn);

    if (!bFile.Open(path.str, BlockFile::CREATE, FALSE))
    {
      LOG_ERR((bFile.LastError()));
      CON_ERR((bFile.LastError()))
      return (FALSE);
    }

    // Save version block
    if (bFile.OpenBlock(VersionBlock))
    {
      bFile.WriteToBlock(&Version, sizeof(Version));      
      bFile.CloseBlock();
    }
    else
    {
      goto Error;
    }

    // Save mission name
    if (!SaveMissionInfo(bFile))
    {
      goto Error;
    }

    // Save build string
    if (!SaveBuildInfo(bFile))
    {
      goto Error;
    }

    // Save orders
    if (flags & SAVE_ORDERS)
    {
      if (!Orders::SaveDemo(bFile))
      {
        goto Error;
      }
    }

    // Save camera movements
    if (flags & SAVE_CAMERA)
    {
      if (!Viewer::Record::SaveDemo(bFile))
      {
        goto Error;
      }
    }

    // Success
    bFile.CloseBlock();
    bFile.Close();

    // Rebuild filesys indexes
    FileSys::BuildIndexes();

    CON_MSG(("Demo saved"))

    return (TRUE);

  Error:
    // Failed
    bFile.CloseBlock();
    bFile.Close();

    // Delete file
    File::Unlink(path.str);

    CON_ERR(("Error saving demo"))

    return (FALSE);
  }


  //
  // Read mission info
  //
  Bool ReadMissionInfo(BlockFile &bFile)
  {
    if (bFile.OpenBlock(MissionBlock))
    {
      // Read group path 
      PathIdent path;
      U16 len;
      bFile.ReadFromBlock(&len, 2);
      bFile.ReadFromBlock(path.str, len);

      // Read mission name
      FileIdent name;
      bFile.ReadFromBlock(&len, 2);
      bFile.ReadFromBlock(name.str, len);
      name.Update();

      // Read mission crc
      U32 crc;
      bFile.ReadFromBlock(&crc, 4);

      // Read demo length
      bFile.ReadFromBlock(&demoCycles, 4);

      bFile.CloseBlock();

      // Find the mission
      if (const Missions::Mission *mission = Missions::FindMission(name, path.str))
      {
        CON_DIAG(("Loading mission [%s][%s]", path.str, name.str))

        // Select the mission
        Missions::SetSelected(mission);

        // Success
        return (TRUE);
      }

      CON_DIAG(("Mission not found [%s][%s]", path.str, name.str))
    }

    return (FALSE);
  }


  //
  // Play demo
  //
  static Bool CommonPlay(const char *file, const char *camera, NotifyProc *proc = NULL, const char *extn = DemoExtn)
  {
    // Read from user directory
    FilePath path;
    Dir::PathMake(path, "", DemoDir, file, extn);

    if (!blockFile.Open(path.str, BlockFile::READ, FALSE))
    {
      return (FALSE);
    }

    // Read mission name
    if (!ReadMissionInfo(blockFile))
    {
      CON_ERR(("Demo '%s' not found", path.str))
      goto Error;
    }

    notifyProc = proc;
    initialCam = camera;
    lastDemo = file;

    // Success, set the runcode
    SetState(LOADING);
    Main::runCodes.Set(0xA293FAEB); // "Mission"

    return (TRUE);

  Error:
    // Failed
    blockFile.Close();

    initialCam = "";
    lastDemo = "";
    SetState(NONE);
    notifyProc = NULL;

    CON_ERR(("Could not load demo"))

    return (FALSE);
  }


  //
  // Play
  //
  Bool Play(const char *file, const char *camera)
  {
    // Play demo
    return (CommonPlay(file, camera, NotifyDemo));
  }

  //
  // Benchmark
  //
  Bool Benchmark(const char *file, const char *camera)
  {
    // Play demo
    return (CommonPlay(file, camera, NotifyBenchmark));
  }


  //
  // Random
  //
  Bool RandomPlay(const char *file)
  {
    // Play demo
    return (CommonPlay(file, DefaultCamera, NotifyRandom, AutoDemoExtn));
  }


  //
  // Post load the demo
  //
  void PostLoad()
  {
    if (IsLoading())
    {
      // Load orders
      Orders::LoadDemo(blockFile);

      // Load camera movement
      Viewer::Record::LoadDemo(blockFile);

      // Loading is complete
      Loaded();
    }

    // Clear demo name
    recordName = "";
  }


  //
  // Pre simulation processing
  //
  void InitSimulation()
  {
    if (IsPlaying())
    {
      // Setup inital camera
      if (*initialCam.str)
      {
        Viewer::SetCurrent(initialCam.str);
      }
    }
  }


  //
  // Simulation is shutting down
  //
  void DoneSimulation()
  {
    if (!IsLoading())
    {
      // Do we need to autosave the replay
      const char *autoSave = recordName;

      if (*autoSave)
      {
        Save(autoSave);
      }

      // Clear the var
      recordName = "";
    }

    // Call notify function
    if (IsPlaying())
    {
      // Call termination function
      if (notifyProc)
      {
        notifyProc(0xD718E59B); // "Terminate"
      }

      StopDemo();
    }
  }


  //
  // Is the demo loading?
  //
  Bool IsLoading()
  {
    return (state == LOADING ? TRUE : FALSE);
  }


  //
  // Is the demo playing?
  //
  Bool IsPlaying()
  {
    return (state == PLAYING ? TRUE : FALSE);
  }


  //
  // Inform that all blocks have been loaded
  //
  void Loaded()
  {
    ASSERT(IsLoading())

    // Set state to playing
    SetState(PLAYING);

    LOG_DIAG(("Demo is %.1f seconds", F32(demoCycles) * GameTime::INTERVAL))
  }


  //
  // Callback for benchmark display
  //
  static void BenchmarkMBProc(U32 crc, U32)
  {
    switch (crc)
    {
      case 0xA4C4F136: // "ok"
      {
        if (loopDemo)
        {
          // Repeat last demo
          Benchmark(lastDemo.str, initialCam.str);
        }
        else
        {
          Main::runCodes.Set("Shell");
        }
        break;
      }
    }
  }


  //
  // Demo notification function
  //
  static void NotifyDemo(U32 id)
  {
    switch (id)
    {
      case 0x3B8E7EE1: // "Start"
      {
        // Disable window activations
        //IFace::SetFlag(IFace::DISABLE_ACTIVATE, TRUE);
        break;
      }

      case 0xB2C53B91: // "End"
      {
        // Enable window activations
        //IFace::SetFlag(IFace::DISABLE_ACTIVATE, FALSE);

        if (loopDemo)
        {
          // Repeat last demo
          Play(lastDemo.str, initialCam.str);
        }
        break;
      }
    }
  }


  //
  // Display benchmark results on completion
  //
  static void NotifyBenchmark(U32 id)
  {
    switch (id)
    {
      case 0x3B8E7EE1: // "Start"
      {
        Game::RC::ResetFrameRate();
        
        startFrame = Main::frameNumber;

        // Store start time
        startTime = Clock::Time::Ms();

        // Setup gametime
        GameTime::SetFastMode(TRUE);
        GameTime::SetDisplayMode(TRUE);

        // Disable interface
        IFace::RootWindow()->DisposeChildren();
        IFace::SetFlag(IFace::DISABLE_DRAW, TRUE);
        IFace::SetFlag(IFace::DISABLE_ACTIVATE, TRUE);

        break;
      }

      case 0xB2C53B91: // "End"
      {
        endFrame = Main::frameNumber;

        // Store end time
        endTime = Clock::Time::Ms();

        char buf[256];

        // Enable interface
        IFace::SetFlag(IFace::DISABLE_DRAW, FALSE);
        IFace::SetFlag(IFace::DISABLE_ACTIVATE, FALSE);

        // Reset game time
        GameTime::SetFastMode(FALSE);

        U32 dtime = endTime - startTime;
        F32 invdtime = 1000.0f / (F32) (dtime);

        // Build display string
        Utils::Sprintf
        (
          buf, 256, 
          "%d ms\\n%.1f fps\\n%.0f tps", 
          dtime,
          (F32) (endFrame - startFrame) * invdtime,
          (F32) (Game::RC::TriCount()) * invdtime
        );
        Vid::LogPerf
        ( 
          "%9d ms  %4.2f fps  %9.0f tps\n", 
          dtime,
          (F32) (endFrame - startFrame) * invdtime,
          (F32) (Game::RC::TriCount())  * invdtime
        );

        if (sysInit)
        {
          // don't do it if we're shutting down
          //
          msgWindow = IFace::MsgBox
          (
            TRANSLATE(("#system.benchmark")),
            TRANSLATE((buf)), 0,
            new MBEventCallback("Ok", TRANSLATE(("#standard.buttons.ok")), BenchmarkMBProc)
          );
        }
        CON_MSG((buf))

        break;
      }
    }
  }


  //
  // Demo notification function
  //
  static void NotifyRandom(U32 id)
  {
    switch (id)
    {
      case 0x3B8E7EE1: // "Start"
      {
        break;
      }

      case 0xB2C53B91: // "End"
      {
        // Back to shell
        Main::runCodes.Set("Shell");
        break;
      }
    }
  }



  //
  // Command handler
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x779EB802: // "demo.save"
      {
        const char *file;

        if (Console::GetArgString(1, file))
        {
          Save(file);
        }
        else
        {
          CON_ERR(("demo.save filename"))
        }
        break;
      }

      case 0x4115D5B2: // "demo.play"
      {
        const char *file, *camera = DefaultCamera;

        if (Console::GetArgString(1, file))
        {
          Console::GetArgString(2, camera);
          Play(file, camera);
        }
        else
        {
          CON_ERR(("demo.play filename"))
        }
        break;
      }

      case 0xBA40C017: // "demo.benchmark"
      {
        const char *file, *camera = DefaultCamera;

        if (Console::GetArgString(1, file))
        {
          Console::GetArgString(2, camera);
          Benchmark(file, camera);
        }
        else
        {
          CON_ERR(("demo.play filename"))
        }
        break;
      }

      case 0x7F7AC76F: // "demo.stop"
      {
        StopDemo();
        break;
      }

      case 0x7297F6AF: // "demo.buildlist"
      {
        const char *s1;

        // Get the name of the listbox control
        if (Console::GetArgString(1, s1))
        {
          if (ICListBox *listBox = ICListBox::FindListBox(s1))
          {
            Dir::Find find;

            // Clear out that listbox
            listBox->DeleteAllItems();

            // Iterate the demos folder
            if (Dir::FindFirst(find, "demos", "*.demo"))
            {
              do
              {
                if (!(find.finddata.attrib & File::SUBDIR))
                {
                  // Get the name without the extension
                  if (char *ext = Utils::FindExt(find.finddata.name))
                  {
                    *ext = '\0';
                  }

                  // Add to the list
                  listBox->AddTextItem(find.finddata.name, NULL);
                }
              }
              while (Dir::FindNext(find));
            }
          }
          else
          {
            LOG_WARN(("Unable to find control [demo.buildlist %s]", s1));
          }
        }
        else
        {
          CON_ERR((Console::ARGS))       
        }
        break;
      }

      case 0x4DA68C45: // "demo.random"
      {
        Dir::Find find;
        List<GameIdent> list;

        // Iterate the "auto_demo" files
        if (Dir::FindFirst(find, "demos", "*.auto_demo"))
        {
          do
          {
            if (!(find.finddata.attrib & File::SUBDIR))
            {
              // Get the name without the extension
              if (char *ext = Utils::FindExt(find.finddata.name))
              {
                *ext = '\0';
              }

              // Add to the list
              list.Append(new GameIdent(find.finddata.name));
            }
          }
          while (Dir::FindNext(find));
        }

        if (list.GetCount())
        {
          GameIdent *newDemo = list[Random::nonSync.Integer(list.GetCount())];
          RandomPlay(newDemo->str);
          CON_MSG(("Playing demo [%s]", newDemo->str))
          list.DisposeAll();
        }
        else
        {
          CON_MSG(("No demo files found"))
        }
        break;
      }

    }
  }
}
