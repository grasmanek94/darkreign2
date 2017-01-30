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
#include "vid_private.h"

#include "viewer_private.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "input.h"
#include "main.h"
#include "sight.h"
#include "team.h"
#include "random.h"
#include "fontsys.h"
#include "babel.h"
#include "console.h"
#include "common.h"
#include "common_mapwindow.h"
#include "client.h"
#include "sound.h"
#include "mapobjctrl.h"
#include "demo.h"
#include "particlesystem.h"
#include "multiplayer_data.h"

//#define LOG_VIEWER(x) LOG_DIAG(x)
#define LOG_VIEWER(x)


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{
  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Action
  //
  namespace Action
  {
    // Cineractive camera name
    static const char *CAMERA_NAME = "Viewer::Action::Cinematic";

    // Forwards
    static void CustomRedraw();


    //
    // Add new primitive to the list
    //
    static void AddPrim(NList<Prim> &list, Prim *prim)
    {
      NList<Prim>::Iterator i(&list);
      NList<Prim>::Node *node = NULL;

      while ((*i) && (*i)->Priority() <= prim->Priority())
      {
        node = i.CurrentNode();
        i++;
      }

      if (node)
      {
        list.InsertAfter(node, prim);
      }
      else
      {
        list.Prepend(prim);
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Class Cineractive
    //
    class Cineractive
    {
    public:

      NList<Cineractive>::Node node;

      // Instruction list
      FScope *instructions;

      // Current instruction
      FScope *nextScope;
      U32 nextCycle;

      // Initial game cycle
      U32 startCycle;

      // Elapsed sim cycles
      U32 elapsedCycles;

      // Elapsed real time (at frame rate)
      F32 elapsedReal;

      // Team that owns this cineractive
      Team *team;

      // Primitive list
      NList<Prim> primitiveList;

      // highest priority prim
      Prim * moviePrim;

      // Current curve
      BookmarkObjPtr bookmarkPtr;
      F32 curveOfs;
      F32 curveStart;

      // TRUE when cineractive is to be deleted
      U32 done      : 1;
      U32 alphaNear : 1;        // alpha at the nearplane
      U32 alphaNearStart : 1;   // for restoring

    public:

      // Constructor
      Cineractive(Team *team, FScope *fScope);

      // Execute a block of commands
      void ExecBlock(FScope *fScope);

      // End the cineractive 
      void Terminate();

      // Step to the next instruction "At" instruction
      void NextInstruction();

      // Process the debriefing block
      void ProcessDebrief(FScope *fScope);

      // Setup a bookmark to follow
      void SetBookmark(FScope *sScope);

      // Simulation
      void GameTimeSim();
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Global data for cineractive
    //

    // Flags
    enum
    {
      DISABLE_IFACE   = 0x0001,
      DISABLE_INPUT   = 0x0002,
      DISABLE_CLIENT  = 0x0004,
      DISABLE_SHROUD  = 0x0008,
      DISABLE_HUD     = 0x0010,
    };

    static U32 flags = 0;

    // Previous frame rate cap
    static S32 prevElapCap;

    // Cineractive camera
    static CinemaCam *camera = NULL;

    // List of running cineractives
    static NList<Cineractive> cineractives(&Cineractive::node);

    // Debriefing list
    static BinTree<FScope> debriefings;

    // Previous game handler
    static EventSys::HANDLERPROC oldProc = NULL;

    // Previous iface repaint
    static IFace::RepaintProc *oldRepaint = NULL;

    // Current cineractive viewport, e.g. inside of letterbox
    static Area<S32> cineViewPort;

    // Game window event handler
    static Bool FASTCALL HandlerProc(Event &);


    ///////////////////////////////////////////////////////////////////////////
    //
    // Movie primitive
    //
    class Movie : public Prim
    {
    protected:

      Bitmap * binkbmp;

      U32 movieCycle;

      // Previous paused state
      Bool wasPaused;

    public:

      // Constructor
      Movie( Cineractive * cineractive, FScope * fScope);
      ~Movie();

      // Simulation
      void Notify(U32 crc);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // ChangeCameraPrim
    //
    class ChangeCameraPrim : public Prim
    {
    protected:

      // Camera to set to
      const char *name;

      // Scope to exec
      FScope *scope;

      // Inherit flag
      Bool inherit;

    public:

      // Constructor
      ChangeCameraPrim(Cineractive *cineractive, FScope *fScope, const char *name, Bool inherit = TRUE, FScope *scope = NULL);

      // Notify
      void Notify(U32 crc);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // StepPrim primitive
    //
    class StepHoldPrim : public Prim
    {
    protected:

      // In, hold, and out time extents
      F32 timeIn;
      F32 timeHold;
      F32 timeOut;

      // Values at start/end and middle
      F32 holdAt, dir;

    public:

      // Constructor
      StepHoldPrim(Cineractive *cineractive, FScope *fScope, S32 priority = 0);

      // Simulate
      F32 Simulate();
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Letterbox primitive
    //
    class Letterbox : public StepHoldPrim
    {
    protected:

      // Current border size
      S32 currPixels;

      // Border Color
      Color clr;

      // Current viewport
      Area<S32> viewport;
      Area<S32> fullvp;

    public:

      // Constructor
      Letterbox(Cineractive *cineractive, FScope *fScope);
      ~Letterbox();

      // Simulation
      void Notify(U32 crc);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Fade primitive
    //
    class Fade : public StepHoldPrim
    {
    protected:

      // Fade to color
      Color clr;

    public:

      // Constructor
      Fade(Cineractive *cineractive, FScope *fScope);

      // Simulation
      void Notify(U32 crc);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Text primitive
    //
    class Text : public StepHoldPrim
    {
    protected:

      // Text color
      Color clr;
      
      // Font
      Font *font;

      // Text
      CH *text;
      S32 len;

      // Position
      F32 x, y;

    public:

      // Constructor
      Text(Cineractive *cineractive, FScope *fScope, S32 priority = 0);
      ~Text();

      // Set text
      void SetText(const CH *str)
      {
        if (text)
        {
          delete[] text;
        }
        text = Utils::Strdup(str);
        len = Utils::Strlen(str);
      }

      // Simulation
      void Notify(U32 crc);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Mesh primitive
    //
    class Mesh : public StepHoldPrim
    {
    protected:

      // Mesh
      MeshRoot * root;

      Vector offset;
      
    public:

      // Constructor
      Mesh(Cineractive *cineractive, FScope *fScope, S32 priority = 0);
      ~Mesh();

      // Simulation
      void Notify(U32 crc);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Subtitle primitive
    //
    class Subtitle : public Prim
    {
    protected:

      // Last text primitive
      Text *textPrim;

      // Configuration to apply to new primitives
      FScope *config;

      // Console message filters
      BinTree<U32> filters;

    public:

      // Constructor
      Subtitle(Cineractive *cineractive, FScope *fScope);
      ~Subtitle();

      // Simulation
      void Notify(U32);

      // Console message callback
      static Bool ConsoleHook(const CH *text, U32 &type, void *context);
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Image primitive
    //
    class Image : public StepHoldPrim
    {
    protected:

      // Color
      Color clr;

      // Texture
      TextureInfo texture;

      // Normalized Screen coordinates
      Point<F32> pos;
      Point<F32> size;

      // Absolute Screen coordinates
      Point<S32> absPos;
      Point<S32> absSize;

      // Is in absolute coordinates?
      Bool absolute;

    public:

      // Constructor
      Image(Cineractive *cineractive, FScope *fScope);

      // Simulation
      void Notify(U32 crc);
    };



    ///////////////////////////////////////////////////////////////////////////
    //
    // Wallpaper primitive
    //
    class Wallpaper : public StepHoldPrim
    {
    protected:

      // Color
      Color clr;

      // Sheet information
      BinTree<TextureInfo> images;
      Point<U8> sheets;

      // Normalized Screen coordinates
      Point<F32> pos;
      Point<F32> size;

    public:

      // Constructor
      Wallpaper(Cineractive *cineractive, FScope *fScope);
      ~Wallpaper();

      // Simulation
      void Notify(U32 crc);
    };


    //
    // Restore cineractive display
    //
    static void RestoreDisplay()
    {
      // Restore speeds
      GameTime::SetFastMode(FALSE);
      GameTime::SetDisplayMode(TRUE);

      // Restore sound
      Sound::Digital::SetDisabled(FALSE);
    }


    //
    // Toggle IFace
    //
    static void DisableIFace(Bool mode)
    {
      if (mode && !(flags & DISABLE_IFACE))
      {
        // Install our own iface repaint function
        oldRepaint = IFace::SetRepaintProc(CustomRedraw);

        // Set Client event processing state
        Client::Events::SetProcessing(FALSE);

        // Disable HUD display
        Client::HUD::Enable(FALSE);

        flags |= DISABLE_IFACE;
      }
      else

      if (!mode && (flags & DISABLE_IFACE))
      {
        // Restore full screen viewport
        Area<S32> vp(0, 0, Vid::backBmp.Width(), Vid::backBmp.Height());
        Vid::CurCamera().Setup(vp);

        // Restore prevous repaint proc
        IFace::SetRepaintProc(oldRepaint);

        // Set Client event processing state
        Client::Events::SetProcessing(TRUE);

        // Enable HUD display
        Client::HUD::Enable(TRUE);

        // Fade the interface back in
        IFace::SetFade(0.0F, 0.75F, TRUE);

        flags &= ~DISABLE_IFACE;
      }
    }


    //
    // Toggle HUD
    //
    static void DisableHUD(Bool mode)
    {
      if (mode && !(flags & DISABLE_HUD))
      {
        // Disable HUD display
        Client::HUD::Enable(FALSE);

        flags |= DISABLE_HUD;
      }
      else

      if (!mode && (flags & DISABLE_HUD))
      {
        // Enable HUD display
        Client::HUD::Enable(TRUE);

        flags &= ~DISABLE_HUD;
      }
    }


    //
    // Toggle input
    //
    static void DisableInput(Bool mode)
    {
      if (mode && !(flags & DISABLE_INPUT))
      {
        // Install our own handler to grab all input events
        oldProc = IFace::SetGameHandler(HandlerProc);

        // Disable console activation
        IFace::SetFlag(IFace::DISABLE_CONSOLE, TRUE);

        // Deactivate all modal controls
        IFace::DeactivateModals();

        // And grab keyboard focus
        IFace::GameWindow()->GetKeyFocus();

        flags |= DISABLE_INPUT;
      }
      else

      if (!mode && (flags & DISABLE_INPUT))
      {
        // Release keyboard focus
        IFace::GameWindow()->ReleaseKeyFocus();

        // Enable console activation
        IFace::SetFlag(IFace::DISABLE_CONSOLE, FALSE);

        // Restore previous handler
        IFace::SetGameHandler(oldProc);

        flags &= ~DISABLE_INPUT;
      }
    }


    //
    // Toggle shroud
    //
    static void DisableShroud(Bool mode)
    {
      if (mode && !(flags & DISABLE_SHROUD))
      {
        // Turn off terrain shroud rendering and line of sight filtering
        Vid::Var::Terrain::shroud = FALSE;
        Sight::showAllUnits = TRUE;
        ParticleSystem::SetCineractiveMode(TRUE);

        // Remove minimum framerate
        prevElapCap = Main::elapCap;
        Main::elapCap = 0;

        flags |= DISABLE_SHROUD;

        // Immediately set object fogging values
        MapObjCtrl::SetObjectFogging(Team::GetDisplayTeam());
      }
      else

      if (!mode && (flags & DISABLE_SHROUD))
      {
        // Restore them
        Vid::Var::Terrain::shroud = TRUE;
        Sight::showAllUnits = FALSE;
        ParticleSystem::SetCineractiveMode(FALSE);

        // Restore minimum framerate
        Main::elapCap = prevElapCap;

        // Forcibly redraw minimap when line of sight changes
        Common::MapWindow::LOSDisplayChanged();

        flags &= ~DISABLE_SHROUD;

        // Immediately set object fogging values
        MapObjCtrl::SetObjectFogging(Team::GetDisplayTeam());
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Class Cineractive
    //

    //
    // Cineractive::Cineractive
    //
    Cineractive::Cineractive(Team *team, FScope *fScope) 
    : team(team),
      done(FALSE),
      alphaNear(FALSE),
      primitiveList(&Prim::node)
    {
      moviePrim = NULL;

      // Initialise instructions
      instructions = fScope;
      instructions->InitIterators();
      NextInstruction();

      // Initialise time counter
      startCycle = GameTime::GameCycle();
      elapsedCycles = 0;
      elapsedReal = 0.0F;

      //flags = 0;
      bookmarkPtr.Clear();

      alphaNearStart = Vid::renderState.status.alphaNear;
      Vid::renderState.status.alphaNear = alphaNear;
    }


    //
    // End the cineractive 
    //
    void Cineractive::Terminate()
    {
      LOG_VIEWER(("Terminate: %d left", cineractives.GetCount() - 1))

      // Delete primitives
      primitiveList.DisposeAll();
      done = TRUE;

      Vid::renderState.status.alphaNear = alphaNearStart;

      // if this is the last one
      if (cineractives.GetCount() == 1)
      {
        // Restore everything
        DisableIFace(FALSE);
        DisableInput(FALSE);
        DisableShroud(FALSE);
        RestoreDisplay();

        ASSERT(!flags)
        flags = 0;
      }
    }


    //
    // Step to the next instruction "At" instruction
    //
    void Cineractive::NextInstruction()
    {
      ASSERT(instructions)

      FScope *fScope;
      nextScope = NULL;

      // Step to the next "At"
      while ((fScope = instructions->NextFunction()) != NULL)
      {
        LOG_VIEWER(("NextInstruction: [%s]", fScope->NameStr()))

        switch (fScope->NameCrc())
        {
          case 0x3F159CC9: // "DefineDebriefing"
          {
            debriefings.Add(Crc::CalcStr(fScope->NextArgString()), fScope);
            break;
          }

          case 0xBF046B0F: // "At"
          {
            nextScope = fScope;
            nextCycle = Utils::FtoLNearest(nextScope->NextArgFPoint() * GameTime::CYCLESPERSECOND);
            return;
          }

          default:
          {
            LOG_WARN(("Unexpected function [%s] in Cineractive", fScope->NameStr()))
            break;
          }
        }
      }
    }


    //
    // Process the debriefing block
    //
    void Cineractive::ProcessDebrief(FScope *fScope)
    {
      // Disable some default stuff, this can be overriden after the Debrief scope
      DisableInput(FALSE);
      DisableIFace(FALSE);
      DisableShroud(TRUE);

      // Restore speeds
      GameTime::SetFastMode(FALSE);
      GameTime::SetDisplayMode(TRUE);

      // Kill all game windows
      IFace::PurgeNonSystem();

      // Create the debrief primitive
      AddPrim(primitiveList, new DebriefPrim(this, fScope));
    }


    //
    // Execute a block of commands
    //
    void Cineractive::ExecBlock(FScope *fScope)
    {
      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        LOG_VIEWER(("Exec: [%s]", sScope->NameStr()))

        switch (sScope->NameCrc())
        {
          case 0x9D71F205: // "Movie"
          {
            // Disable movies in multiplayer campaigns
            if (!MultiPlayer::Data::Online())
            {
              if (moviePrim)
              {
                delete moviePrim;
              }
              moviePrim = new Movie(this, sScope);
            }
            break;
          }

          case 0x0DA67726: // "AlphaNear"
            Vid::renderState.status.alphaNear = alphaNear = sScope->NextArgInteger();
            break;

          case 0x70600744: // "DisableIFace"
          {
            DisableIFace(sScope->NextArgInteger());
            break;
          }

          case 0x72C1779F: // "DisableHUD"
          {
            DisableHUD(sScope->NextArgInteger());
            break;
          }

          case 0x288F19CB: // "DisableInput"
          {
            DisableInput(sScope->NextArgInteger());
            break;
          }

          case 0xAA268B85: // "DisableShroud"
          {
            DisableShroud(sScope->NextArgInteger());
            break;
          }

          case 0x47518EE4: // "EndCineractive"
          {
            Terminate();
            break;
          }

          case 0x7E8E3E05: // "SkipPoint"
          {
            RestoreDisplay();
            break;
          }

          case 0xEA4227E1: // "SetBookmark"
          {
            SetBookmark(sScope);
            break;
          }

          case 0xDDD6437A: // "DefaultCamera"
          {
            LOG_VIEWER(("DefaultCamera"))

            if (Demo::IsPlaying())
            {
              SetCurrent("Playback0", StdLoad::TypeU32(sScope, U32(FALSE), Range<U32>::flag), sScope);
            }
            else
            {
              SetCurrent("default", StdLoad::TypeU32(sScope, U32(FALSE), Range<U32>::flag), sScope);
            }
            break;
          }

          case 0xF4356EC8: // "SetCamera"
          {
            SetCurrent(sScope->NextArgString(), FALSE, sScope);
            break;
          }

          case 0x9805A0A6: // "Mesh"
          {
            AddPrim(primitiveList, new Mesh(this, sScope));
            break;
          }

          case 0x16556EBC: // "Letterbox"
          {
            AddPrim(primitiveList, new Letterbox(this, sScope));
            break;
          }

          case 0x10A95B64: // "Fade"
          {
            AddPrim(primitiveList, new Fade(this, sScope));
            break;
          }

          case 0x76802A4E: // "Image"
          {
            AddPrim(primitiveList, new Image(this, sScope));
            break;
          }

          case 0x64DD3931: // "Wallpaper"
          {
            AddPrim(primitiveList, new Wallpaper(this, sScope));
            break;
          }

          case 0xCB28D32D: // "Text"
          {
            AddPrim(primitiveList, new Text(this, sScope));
            break;
          }

          case 0x8E18DC65: // "Subtitle"
          {
            AddPrim(primitiveList, new Subtitle(this, sScope));
            break;
          }

          case 0x37345010: // "Pause"
          {
            if (!GameTime::Paused())
            {
              GameTime::Pause(FALSE);
            }
            break;
          }

          case 0x0642D599: // "Unpause"
          {
            if (GameTime::Paused())
            {
              GameTime::Pause(FALSE);
            }
            break;
          }

          case 0x3F159CC9: // "DefineDebriefing"
          {
            debriefings.Add(Crc::CalcStr(sScope->NextArgString()), sScope);
            break;
          }

          case 0x311D74EF: // "Debrief"
          {
            ProcessDebrief(sScope);
            break;
          }

          case 0x06A3B1BA: // "Action"
          {
            // Execute an objective action block
            ::Action::Execute(team, sScope);
            break;
          }

          default:
          {
            LOG_WARN(("Unknown function [%s] in Cineractive", sScope->NameStr()))
            break;
          }
        }
      }
    }


    //
    // Cineractive::GameTimeSim
    //
    void Cineractive::GameTimeSim()
    {
      if (moviePrim)
      {
        if (moviePrim->done)
        {
          delete moviePrim;
          moviePrim = NULL;
        }
        else
        {
          moviePrim->Notify(0x7FEF2C7B); // "CycleTick"
          return;
        }
      }

      // Currently elapsed seconds
      elapsedCycles = GameTime::GameCycle() - startCycle;

      // Any new instructions to instantiate?
      while (nextScope && (nextCycle <= elapsedCycles && !moviePrim))
      {
        ExecBlock(nextScope);
        NextInstruction();
      }

      // Send all primitives a cycle tick message
      NList<Prim>::Iterator i(&primitiveList);
      Prim *prim;

      while ((prim = i++) != NULL)
      {
        if (prim->done)
        {
          primitiveList.Dispose(prim);
        }
        else
        {
          prim->Notify(0x7FEF2C7B); // "CycleTick"
        }
      }
    }

    //
    // Initialise
    //
    void Init()
    {      
      // Create a camera
      RegisterViewer(camera = new CinemaCam(CAMERA_NAME));

      flags = 0;
    }

    //
    // Stop all movies
    //
    void StopMovies()
    {
      for (NList<Cineractive>::Iterator curr(&cineractives); *curr; curr++)
      {
        Cineractive *cin = *curr;

        if (cin->moviePrim)
        {
          delete cin->moviePrim;
          cin->moviePrim = NULL;
        }
      }
    }


    //
    // Shutdown
    // 
    void Done()
    {
      NList<Cineractive>::Iterator i(&cineractives);
      i.GoToTail(); 
      while (Cineractive * cin = i--)
      {
        if (cin->moviePrim)
        {
          delete cin->moviePrim;
          cin->moviePrim = NULL;
        }
        cin->Terminate();
        cineractives.Dispose(cin);
      }

      debriefings.UnlinkAll();

      // Delete the cineractive cam
      DeleteViewer(camera);
      camera = NULL;
    }


    //
    // Initialise execution of a cineractive
    //
    void Execute(Team *team, FScope *fScope)
    {
      // Create the cineractive object
      cineractives.Append(new Cineractive(team, fScope));
      LOG_VIEWER(("Execute cineracive #%d [%d]", cineractives.GetCount(), GameTime::GameCycle()))
    }


    //
    // Run a debriefing cineractive
    //
    void RunDebriefing(const char *name)
    {
      FScope *fScope;

      if ((fScope = debriefings.Find(Crc::CalcStr(name))) != NULL)
      {
        Execute(NULL, fScope);
      }
      else
      {
        LOG_ERR(("Debriefing not found [%s]", name))
      }
    }


    //
    // Setup a bookmark to follow
    //
    void Cineractive::SetBookmark(FScope *sScope)
    {
      const char *name = StdLoad::TypeString(sScope, "Name");

      // Setup bookmark
      BookmarkObj *obj = BookmarkObj::FindBookmark(name);

      if (obj)
      {
        // Optional start time
        curveOfs = StdLoad::TypeF32(sScope, "Start", 0.0F); 
        curveStart = F32(elapsedCycles) * GameTime::INTERVAL;

        // Setup reaper
        bookmarkPtr = obj;

        // Force cineractive camera
        SetCurrent(camera, FALSE, sScope);
        //AddPrim(primitiveList, new ChangeCameraPrim(this, NULL, CAMERA_NAME, FALSE, sScope));

        LOG_VIEWER(("SetBookmark(%s)", name))
      }
      else
      {
        LOG_WARN(("Could not find bookmark [%s]", name))
      }
    }


    //
    // Simulation once per game cycle
    //
    void GameTimeSim()
    {
      NList<Cineractive>::Iterator i(&cineractives);
      Cineractive *curr;

      while ((curr = i++) != NULL)
      {
        curr->GameTimeSim();
        if (curr->done)
        {
          curr->Terminate();
          cineractives.Dispose(curr);
        }
      }
    }


    //
    // Abort the cineractive and return to gameplay
    //
    void Abort()
    {
      // Speed up simulation
      GameTime::SetFastMode(TRUE);
      GameTime::SetDisplayMode(FALSE);

      // Release the sound drivers
      Sound::Digital::SetDisabled(TRUE);

      // Skip to end of cineractive
      LOG_VIEWER(("Cancelling cineractive"))
    }


    //
    // Processing before simulation
    //
    void PreSimulate(F32)
    {
      if (movie)
      {
        return;
      }

      ASSERT(camera)

      // Update real time
      F32 realOffset = GameTime::GameCycleOffset();
      for (NList<Cineractive>::Iterator i(&cineractives); *i; i++)
      {
        Cineractive *curr = *i;
        curr->elapsedReal = F32(curr->elapsedCycles) * GameTime::INTERVAL + realOffset;
      }

      // Update the camera along its curve
      i.GoToTail();
      for (; *i; i--)
      {
        Cineractive *curr = *i;

        if (curr->bookmarkPtr.Alive())
        {
          F32 t = (curr->elapsedReal - curr->curveStart) + curr->curveOfs;
          Matrix mat;

          if (curr->bookmarkPtr->Step(t, mat))
          {
            camera->curveMatrix = mat;
          }
          break;
        }
      }
    }


    //
    // Processing during rendering
    //
    void Render(U32 mode)
    {
      if (movie)
      {
        return;
      }

      // Initialise cineractive viewport to full viewport incase nothing else modifies it
      if (mode == 0xF530E366) // "Render::PreRender"
      {
        cineViewPort = Vid::viewRect;
      }

      for (NList<Cineractive>::Iterator curr(&cineractives); *curr; curr++)
      {
        Cineractive *cin = *curr;

        for (NList<Prim>::Iterator i(&cin->primitiveList); *i; i++)
        {
          Prim *prim = *i;
          
          if (!prim->done)
          {
            // Ensure that its ready to simulate (may have been started early)
            if ((cin->elapsedReal - prim->startTime) >= 0.0F)
            {
              prim->Notify(mode);
            }
          }
        }
      }
    }


    //
    // Is a cineractive playing?
    //
    Bool IsPlaying()
    {
      return (cineractives.GetCount() > 0 ? TRUE : FALSE);
    }


    //
    // Game window event handler
    //
    static Bool FASTCALL HandlerProc(Event &e)
    {
      // Input events
      if (e.type == Input::EventID())
      {
        switch (e.subType)
        {
          case Input::KEYDOWN:
          //case Input::KEYREPEAT:
          {
            switch (e.input.code)
            {
              case DIK_ESCAPE:
              {
                // Never allow skipping in multiplayer
                if (!MultiPlayer::Data::Online())
                {
                  if (movie)
                  {
                    StopMovies();
                  }
                  else
                  {
                    Abort();
                  }

                  //Input::FlushEvents();
                }

                break;
              }
            }
            break;
          }
        }
      }
      return (TRUE);
    }


    //
    // Custom IFace drawing function
    //
    static void CustomRedraw()
    {
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Base primitive
    //
    Prim::Prim(Cineractive *cineractive, FScope *fScope, S32 defaultPriority) 
    : cineractive(cineractive),
      done(FALSE),
      priority(defaultPriority)
    {
      startTime = F32(cineractive->elapsedCycles) * GameTime::INTERVAL;

      // Load configuration
      if (fScope)
      {
        priority = S32(StdLoad::TypeU32(fScope, "Priority", priority));
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Movie primitive constructor
    //
    Movie::Movie( Cineractive * cineractive, FScope * fScope) 
    : Prim( cineractive, fScope),
      wasPaused(FALSE)
    {
      if (movie)
      {
        StopMovies();
      }
      const char * name = StdLoad::TypeString(fScope, "Name", "");

      U32 stretch = StdLoad::TypeU32(fScope, "Stretch", FALSE);

      binkbmp = new Bitmap( bitmapSURFACE);
      ASSERT( binkbmp);
      binkbmp->SetPixFormat( Vid::backBmp.PixelFormat());

      if (!binkbmp->LoadBink( name, TRUE, stretch))
      {
        LOG_ERR( ("Can't open bink file %s", name) );

        // End the movie primitive next processing cycle
        done = TRUE;
      }
      else
      {
        movie = TRUE;

        binkbmp->BinkSetActive( 1);

        // clear screeen
        Vid::ClearBack();
        Vid::RenderFlush();
        Vid::ClearBack();
        Vid::RenderFlush();
        Vid::ClearBack();
      }
      movieCycle = GameTime::GameCycle();

      // Pause the game
      if (!GameTime::Paused())
      {
        wasPaused = FALSE;
        GameTime::Pause(FALSE);
      }
      else
      {
        wasPaused = TRUE;
      }
    }

    //
    // Movie primitive destructor
    //
    Movie::~Movie()
    {
      if (Vid::isStatus.initialized)
      {
        delete binkbmp;
      }
      binkbmp = NULL;

      // Unpause the game
      if (GameTime::Paused() && !wasPaused)
      {
        GameTime::Pause(FALSE);
      }

      // magically, no elapsed time
      //
      movieCycle = GameTime::GameCycle() - movieCycle;

      for (NList<Cineractive>::Iterator i(&cineractives); *i; i++)
      {
        (*i)->startCycle += movieCycle;
      }
      GameTime::Readjust();

      movie = FALSE;
    }


    //
    // Movie::Notify
    //
    void Movie::Notify(U32 crc)
    {
      switch (crc)
      {
      case 0x7FEF2C7B: // "CycleTick"
        if (binkbmp->BinkDone())
        {
          done = TRUE;
        }

        break;
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // StepHold primitive
    //


    //
    // Constructor
    //
    ChangeCameraPrim::ChangeCameraPrim(Cineractive *cineractive, FScope *fScope, const char *name, Bool inherit, FScope *scope)
    : Prim(cineractive, fScope),
      name(name),
      scope(scope),
      inherit(inherit)
    {
    }


    //
    // Notify
    //
    void ChangeCameraPrim::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xF530E366: // "Render::PreRender"
        {
          SetCurrent(name, inherit, scope);
          done = TRUE;
          return;
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // StepHold primitive
    //
    StepHoldPrim::StepHoldPrim(Cineractive *cineractive, FScope *fScope, S32 priority) 
    : Prim(cineractive, fScope, priority)
    {
      // Read time in, hold, and out from the time function
      FScope *sScope = fScope->GetFunction("Time");

      timeIn = StdLoad::TypeF32(sScope, Range<F32>::positive);
      timeHold = StdLoad::TypeF32(sScope, 1e20F, Range<F32>::positive);
      timeOut = StdLoad::TypeF32(sScope, timeHold, Range<F32>::positive);

      // Rate
      switch (StdLoad::TypeStringCrc(fScope, "Direction", 0xF975A769)) // "Up"
      {
        case 0xF975A769: // "Up"
        {
          dir = 1.0F;
          holdAt = 1.0F;
          break;
        }

        case 0xEF54F336: // "Down"
        default:
        {
          dir = -1.0F;
          holdAt = 0.0F;
          break;
        }
      }
    }


    //
    // StepHoldPrim::Simulate
    //
    F32 StepHoldPrim::Simulate()
    {
      // Animate in
      F32 elapsed = cineractive->elapsedReal - startTime;
      F32 newVal = 0.0F;

      ASSERT(elapsed >= 0.0F)

      // Ramping up
      if (elapsed < timeIn)
      {
        F32 gap = timeIn;
        F32 pct = (gap > 1e-4f) ? elapsed / gap : 1.0F;
        newVal = (1.0F - holdAt) + (dir * pct);
      }
      else

      // Holding steady
      if (elapsed < timeHold)
      {
        newVal = holdAt;
      }
      else

      // Ramping down
      if (elapsed < timeOut)
      {
        F32 gap = timeOut - timeHold;
        F32 pct = (gap > 1e-4f) ? (elapsed - timeHold) / gap : 1.0F;
        newVal = holdAt - (dir * pct);
      }

      else
      {
        LOG_VIEWER(("Primitive ended [%.1f]", elapsed))
        done = TRUE;
      }

      return (newVal);
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Letterbox primitive
    //

    //
    // Letterbox::Letterbox
    //
    Letterbox::Letterbox(Cineractive *cineractive, FScope *fScope) 
    : StepHoldPrim(cineractive, fScope)
    {
      // Color
      StdLoad::TypeColor(fScope, "Color", clr, Color(0L, 0L, 0L, 255L));
    }

    //
    // LetterBox::~LetterBox
    //
    Letterbox::~Letterbox()
    {
      // Restore full viewport
      cineViewPort.Set(0, 0, Vid::backBmp.Width(), Vid::backBmp.Height());
      Vid::CurCamera().Setup(cineViewPort);
    }


    //
    // Letterbox::Notify
    //
    void Letterbox::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xF530E366: // "Render::PreRender"
        {
          F32 curr = Simulate();

          // Modify the viewport
          S32 x = Vid::backBmp.Width();
          S32 y = Vid::backBmp.Height();
          S32 viewY = (x * 9) >> 4;
          S32 bordY = (y - viewY) >> 1;

          currPixels = bordY - Utils::FtoLNearest(F32(bordY) * curr);

          if (bordY > 0)
          {
            // Enter letterbox mode
            viewport.SetSize(0, currPixels, x, y - (currPixels << 1));
            Vid::CurCamera().Setup( viewport);
          }
          else
          {
            viewport.SetSize(0, 0, Vid::backBmp.Width(), Vid::backBmp.Height());
          }

          //LOG_DIAG(("viewport=%d,%d,%d,%d", viewport.p0.x, viewport.p0.z, viewport.p1.x, viewport.p1.z))

          // cineViewPort should reflect current view inside borders
          cineViewPort = viewport;

          // Setup full viewport parameters
          fullvp.Set(0, 0, x, y);

          ClipRect rc;

          return;
        }
        case 0xCEEF613F: // "Render::PostIFace1"
        {
          Vid::ClipScreen();    // full screen clipping

          // Draw border
          ClipRect rc;

          rc.Set(0, 0, fullvp.p1.x, currPixels);
          Vid::RenderRectangle( rc, 0xff000000, NULL, RS_BLEND_DEF, Vid::sortNORMAL0, 0, 1, TRUE);

          rc.Set(0, fullvp.p1.y - currPixels, fullvp.p1.x, fullvp.p1.y);
          Vid::RenderRectangle( rc, 0xff000000, NULL, RS_BLEND_DEF, Vid::sortNORMAL0, 0, 1, TRUE);

          Vid::ClipRestore();   // restore letterbox clipping
          return;
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Fade primitive
    //

    //
    // Fade::Fade
    //
    Fade::Fade(Cineractive *cineractive, FScope *fScope) 
    : StepHoldPrim(cineractive, fScope)
    {
      // Color
      StdLoad::TypeColor(fScope, "Color", clr, Color(0L, 0L, 0L, 255L));
    }


    //
    // Fade::Notify
    //
    void Fade::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xC3AC47E6: // "Render::PostIFace2"
        {
          F32 curr = Simulate();

          // Draw a big ass poly
          IFace::RenderRectangle(cineViewPort, clr, NULL, Clamp<F32>(0.0F, curr, 1.0F));
          return;
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Image primitive
    //

    //
    // Image::Image
    //
    Image::Image(Cineractive *cineractive, FScope *fScope) 
    : StepHoldPrim(cineractive, fScope),
      pos(0.0F, 0.0F),
      size(1.0F, 1.0F),
      absPos(0, 0),
      absSize(32, 32),
      absolute(0)
    {
      IFace::FScopeToTextureInfo(fScope->GetFunction("Image"), texture);

      // Color
      StdLoad::TypeColor(fScope, "Color", clr, Color(255L, 255L, 255L, 255L));

      // Area
      StdLoad::TypePoint<F32>(fScope, "Pos", pos, pos);
      StdLoad::TypePoint<F32>(fScope, "Size", size, size);

      // Absolute area
      StdLoad::TypePoint<S32>(fScope, "AbsPos", absPos, absPos);
      StdLoad::TypePoint<S32>(fScope, "AbsSize", absSize, absSize);

      // Use absolute area?
      absolute = StdLoad::TypeU32(fScope, "Absolute", FALSE);
    }


    //
    // Image::Notify
    //
    void Image::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xC3AC47E6: // "Render::PostIFace2"
        {
          F32 curr = Simulate();

          // Calculate screen rectangle
          const ClipRect &rc = cineViewPort;
          ClipRect newRc;

          if (absolute)
          {
            if (absPos.x < 0)
            {
              newRc.p0.x = rc.p1.x - absSize.x + absPos.x;
            }
            else
            {
              newRc.p0.x = rc.p0.x + absPos.x;
            }

            if (absPos.y < 0)
            {
              newRc.p0.y = rc.p1.y - absSize.y + absPos.y;
            }
            else
            {
              newRc.p0.y = rc.p0.y + absPos.y;
            }

            newRc.p1 = newRc.p0 + absSize;
          }
          else
          {
            newRc.p0.x = rc.p0.x + Utils::FtoL(pos.x * F32(rc.Width()));
            newRc.p0.y = rc.p0.y + Utils::FtoL(pos.y * F32(rc.Height()));
            newRc.p1.x = rc.p0.x + Utils::FtoL((pos.x + size.x) * F32(rc.Width()));
            newRc.p1.y = rc.p0.y + Utils::FtoL((pos.y + size.y) * F32(rc.Height()));
          }
          IFace::RenderRectangle(newRc, clr, &texture, curr * IFace::data.alphaScale);

          return;
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Wallpaper primitive
    //

    //
    // Wallpaper::Wallpaper
    //
    Wallpaper::Wallpaper(Cineractive *cineractive, FScope *fScope) 
    : StepHoldPrim(cineractive, fScope),
      pos(0.0F, 0.0F),
      size(1.0F, 1.0F)
    {
      // Sheets
      IFace::FScopeToSheetInfo(fScope->GetFunction("Sheet"), images, sheets);

      // Color
      StdLoad::TypeColor(fScope, "Color", clr, Color(255L, 255L, 255L, 255L));

      // Area
      StdLoad::TypePoint<F32>(fScope, "Pos", pos, pos);
      StdLoad::TypePoint<F32>(fScope, "Size", size, size);
    }


    //
    // Wallpaper::~Wallpaper
    //
    Wallpaper::~Wallpaper()
    {
      images.DisposeAll();
    }


    //
    // Wallpaper::Notify
    //
    void Wallpaper::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xC3AC47E6: // "Render::PostIFace2"
        {
          F32 curr = Simulate();

          // Rebuild rectangles
          const ClipRect &rc = cineViewPort;
          ClipRect newRc;

          newRc.p0.x = rc.p0.x + Utils::FtoL(pos.x * F32(rc.Width()));
          newRc.p0.y = rc.p0.y + Utils::FtoL(pos.y * F32(rc.Height()));
          newRc.p1.x = rc.p0.x + Utils::FtoL((pos.x + size.x) * F32(rc.Width()));
          newRc.p1.y = rc.p0.y + Utils::FtoL((pos.y + size.y) * F32(rc.Height()));


          IFace::UpdateSheets(newRc, images, sheets);

          // Display
          IFace::RenderSheets(Point<S32>(0, 0), images, clr, curr);
          return;
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Text primitive
    //

    //
    // Constructor
    //
    Text::Text(Cineractive *cineractive, FScope *fScope, S32 priority)
    : StepHoldPrim(cineractive, fScope, priority),
      text(NULL)
    {
      // Font
      const char *name = StdLoad::TypeString(fScope, "Font", "System");

      if ((font = FontSys::GetFont(Crc::CalcStr(name))) == NULL)
      {
        ERR_FATAL(("Font not found [%s] for Cineractive Text", name))
      }

      // Text
      FScope *sScope;

      if ((sScope = fScope->GetFunction("Text", FALSE)) != NULL)
      {
        SetText(TRANSLATE((StdLoad::TypeString(sScope))));
      }

      // Position
      if ((sScope = fScope->GetFunction("Pos", FALSE)) != NULL)
      {
        x = StdLoad::TypeF32(sScope);
        y = StdLoad::TypeF32(sScope);
      }
      else
      {
        x = 0.5F;
        y = 0.5F;
      }

      // Color
      StdLoad::TypeColor(fScope, "Color", clr, Color(255L, 255L, 255L, 255L));
    }


    //
    // Destructor
    //
    Text::~Text()
    {
      if (text)
      {
        delete[] text;
        text = NULL;
      }
    }


    //
    // Simulation
    //
    void Text::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xC3AC47E6: // "Render::PostIFace2"
        {
          F32 curr = Simulate();

          // Draw the text
          if (text)
          {
            S32 width = font->Width(text, len);
            S32 height = font->Height();
            S32 xpos = Max<S32>(Utils::FtoL(F32(Vid::backBmp.Width() - width) * x), 0);
            S32 ypos = Max<S32>(Utils::FtoL(F32(Vid::backBmp.Height() - height) * y), 0);
            font->Draw(xpos, ypos, text, len, clr, NULL, curr);
          }
          return;
        }
      }
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Mesh primitive
    //

    //
    // Constructor
    //
    Mesh::Mesh(Cineractive *cineractive, FScope *fScope, S32 priority)
    : StepHoldPrim(cineractive, fScope, priority)
    {
      const char *name = StdLoad::TypeString(fScope, "Godfile", "");

      offset.ClearData();

      FScope * sScope = fScope->GetFunction( "Offset", FALSE);
      if (sScope)
      {
        offset.x = StdLoad::TypeF32(sScope);
        offset.y = StdLoad::TypeF32(sScope);
        offset.z = StdLoad::TypeF32(sScope);
      }

      root = ::Mesh::Manager::FindRead( name);
    }


    //
    // Destructor
    //
    Mesh::~Mesh()
    {
    }


    //
    // Simulation
    //
    void Mesh::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0xEF30A860: // "Render::PostObject"
        {
          Simulate();

          if (root && !done)
          {
            Matrix mat = Vid::CurCamera().WorldMatrix();
            Vector vect;
            mat.Transform( vect, offset);
            mat.posit = vect;
            root->Render( mat);
          }
          return;
        }
      }
    }

    ///////////////////////////////////////////////////////////////////////////
    //
    // Subtitle primitive
    //

    //
    // Constructor
    //
    Subtitle::Subtitle(Cineractive *cineractive, FScope *fScope)
    : Prim(cineractive, fScope, 100),
      textPrim(NULL)
    {
      // Item configuration
      config = fScope->GetFunction("ItemConfig");

      // Default filters
      filters.Add(0x2FFE8D2A); // "CineractiveMessage"

      // Register console notification hook
      Console::RegisterMsgHook(&filters, ConsoleHook, this);
    }


    //
    // Destructor
    //
    Subtitle::~Subtitle()
    {
      // Unregister message hook
      Console::UnregisterMsgHook(ConsoleHook, this);

      // Delete filters
      filters.DisposeAll();
    }


    //
    // Simulation
    //
    void Subtitle::Notify(U32)
    {
    }


    //
    // Console message callback
    //
    Bool Subtitle::ConsoleHook(const CH *text, U32 &, void *context)
    {
      Subtitle *ptr = (Subtitle *) context;

      // Is the text primitive still alive
      if (ptr->textPrim)
      {
        Bool found = FALSE;

        for (NList<Prim>::Iterator i(&ptr->cineractive->primitiveList); *i; i++)
        {
          if (*i == ptr->textPrim)
          {
            found = TRUE;
            break;
          }
        }

        if (found)
        {
          // Delete the bastard
          ptr->cineractive->primitiveList.Dispose(ptr->textPrim);
          LOG_VIEWER(("Deleting old text subtitle"))
        }
      }

      // Create a new text primitive
      AddPrim(ptr->cineractive->primitiveList, ptr->textPrim = new Text(ptr->cineractive, ptr->config, ptr->priority));
      ptr->textPrim->SetText(text);

      return (TRUE);
    }


    ///////////////////////////////////////////////////////////////////////////
    //
    // Debrief primitive
    //


    // Static data
    DebriefPrim *DebriefPrim::current = NULL;


    //
    // Constructor
    //
    DebriefPrim::DebriefPrim(Cineractive *cineractive, FScope *fScope) 
    : Prim(cineractive, fScope)
    {
      // Only one allowable debriefing at any time
      if (current)
      {
        ERR_FATAL(("Only one debreifing allowed!"))
      }
      else
      {
        current = this;
      }

      // Step through each function
      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xAAD665AB: // "Exec"
          {
            Main::Exec(sScope->NextArgString(), Main::ScopeHandler, FALSE);
            break;
          }

          case 0x546486D8: // "OnEvent"
          {
            U32 id = StdLoad::TypeStringCrc(sScope);
            endScripts.Add(id, sScope);
            break;
          }

          case 0x27884D6D: // "AddBookmark"
          {
            // Add the optional bookmark            
            if (BookmarkObj *obj = BookmarkObj::FindBookmark(StdLoad::TypeString(sScope, "Name")))
            {
              bookmarkList.Append(obj);
            }
            break;
          }

          default:
          {
            LOG_WARN(("Unknown function in Debrief [%s]", sScope->NameStr()))
            break;
          }
        }
      }

      // Start one of the bookmarks going
      PickRandomBookmark();

      LOG_VIEWER(("Entering debriefing"))
    }


    //
    // Destructor
    //
    DebriefPrim::~DebriefPrim()
    {
      bookmarkList.Clear();
      endScripts.UnlinkAll();
      current = NULL;
    }   


    //
    // Simulation
    //
    void DebriefPrim::Notify(U32 crc)
    {
      switch (crc)
      {
        case 0x7FEF2C7B: // "CycleTick"
        {
          // Is the bookmark finished?
          if (!cineractive->bookmarkPtr.Alive())
          {
            PickRandomBookmark();
          }
          return;
        }
      }
    }


    //
    // Pick a random bookmark
    //
    void DebriefPrim::PickRandomBookmark()
    {
      bookmarkList.PurgeDead();
      U32 count = bookmarkList.GetCount();
      if (count)
      {
        BookmarkObj *obj = *(bookmarkList[Random::nonSync.Integer(count)]);
        cineractive->bookmarkPtr = obj;
        cineractive->curveOfs = 0;
        cineractive->curveStart = F32(cineractive->elapsedCycles) * GameTime::INTERVAL;

        ASSERT(camera)
        SetCurrent(camera);

        LOG_VIEWER(("PickBookmark(%s)", obj->GetName()))

        return;
      }
      LOG_VIEWER(("PickBookmark: none available"))
    }       
  }
}
