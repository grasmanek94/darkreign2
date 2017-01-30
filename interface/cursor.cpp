/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Cursor system
//
// 01-JUN-1998
//


#include "cursor.h"
#include "iface.h"
#include "iface_util.h"
#include "clock.h"
#include "vid.h"


///////////////////////////////////////////////////////////////////////////////
//
// namespace CursorSys private implementation
//
namespace CursorSys
{
  class Base
  {
  public:

    // Notifications
    enum
    {
      CN_ACTIVATE,
      CN_DEACTIVATE
    };

  public:

    // Destructor
    virtual ~Base() {}

    // Notifications
    virtual void Notify(U32) {}

    // Configure the cursor
    virtual void Configure(FScope *) {};

    // Simulate the cursor
    virtual void Simulate(U32) {}

    // Draw the cursor
    virtual void Draw(const Point<S32> &) {};
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Cursor derived from another cursor
  //
  class Derived : public Base
  {
  protected:

    Base *base;

  public:

    // Constructor
    Derived(Base *base) : base(base) {}

    // Notify
    void Notify(U32 id)
    {
      ASSERT(base)
      base->Notify(id);
    }

    // Simulate
    void Simulate(U32 ms)
    {
      ASSERT(base)
      base->Simulate(ms);
    }

    // Display
    void Draw(const Point<S32> &p)
    {
      ASSERT(base)
      base->Draw(p);
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Animated bitmap cursor
  //
  class Bmp : public Base
  {
  protected:

    // Frame data
    struct FrameInfo
    {
      TextureInfo tex;
      Point<S32> hotspot;
    };

    // List of frames
    List<FrameInfo> frames;

    // Current frame
    List<FrameInfo>::Iterator current;

    // Frame time in ms
    S32 speed;
    S32 elapsed;

  protected:

    // Add a frame
    void AddFrame(FScope *fScope)
    {
      FrameInfo *frame = new FrameInfo;
      FScope *sScope;

      // Read image info
      sScope = fScope->GetFunction("Texture");
      IFace::FScopeToTextureInfo(sScope, frame->tex);

      // Optional hotspot
      if ((sScope = fScope->GetFunction("Hotspot", FALSE)) != NULL)
      {
        frame->hotspot.x = sScope->NextArgInteger();
        frame->hotspot.y = sScope->NextArgInteger();
      }
      else
      {
        frame->hotspot.Set(0, 0);
      }

      // Add frame to list
      frames.Append(frame);
    }

  public:

    // Default constructor
    Bmp() : speed(100), elapsed(0)
    {
      current.SetList(&frames);
    }

    // Destructor
    ~Bmp()
    {
      frames.DisposeAll();
    }

    //
    // Notifications
    //
    void Notify(U32 id)
    {
      switch (id)
      {
        case CN_ACTIVATE:
          current.GoToHead();
          return;
      }

      Base::Notify(id);
    }

    //
    // Configure the cursor
    //
    void Configure(FScope *fScope)
    {
      FScope *sScope;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0x207C29E1: // "FrameRate"
            speed = 1000 / Clamp<U32>(1, sScope->NextArgInteger(), 1000);
            break;

          case 0xD13EA311: // "AddFrame"
            AddFrame(sScope);
            break;
        }
      }
    }

    //
    // Simulate
    //
    void Simulate(U32 ms)
    {
      if (frames.GetCount() > 1)
      {
        if ((elapsed += S32(ms)) > speed)
        {
          // Move to next frame
          elapsed = 0;
          current++;

          // Move back to head if necessary
          if (!*current)
          {
            current.GoToHead();
          }
        }
      }
    }

    //
    // Draw the cursor
    //
    void Draw(const Point<S32> &p)
    {
      FrameInfo *frame;

      if ((frame = *current) != NULL)
      {
        frame->tex.texRect = frame->tex.pixels - frame->tex.pixels.p0 - frame->hotspot + p;
        IFace::RenderRectangle(frame->tex.texRect, 0xFFFFFFFF, &frame->tex);
      }
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Simple Geometric
  //
  class Geometric : public Base
  {
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Cursor system implementation
  //

  // Private data
  static Bool sysInit = FALSE;

  // Registered cursors
  static BinTree<Base> cursors;
  static Base *current = NULL;

  // Standard cursors 
  static U32 standardCrs[MAX_CURSORS];


  //
  // Initialise the cursor system
  //
  void Init()
  {
    ASSERT(!sysInit);

    current = NULL;

    // Initialise standard cursors
    for (U32 i = 0; i < MAX_CURSORS; i++)
    {
      standardCrs[i] = 0;
    }

    // System is initialised
    sysInit = TRUE;
  }


  //
  // Shutdown the cursor system
  //
  void Done()
  {
    ASSERT(sysInit);

    DeleteAll();

    sysInit = FALSE;
  }


  //
  // Find a cursor by name
  //
  U32 FindByName(const char *name)
  {
    U32 id = Crc::CalcStr(name);

    // Ensure the cursor exists
    if (cursors.Find(id))
    {
      return (id);
    }
    else
    {
      LOG_ERR(("Cursor [%s] not found", name));
      return (0);
    }
  }


  //
  // Process a CreateCursor scope
  //
  void ProcessCreateCursor(FScope *fScope)
  {
    // Cursor name is first argument
    const char *name = fScope->NextArgString();

    // Cursor class is second argument
    const char *cls  = fScope->NextArgString();

    // Create the cursor
    Base *newCrs = NULL;
    U32 key = Crc::CalcStr(cls);

    switch (key)
    {
      case 0x5B2A0A5F: // "Null"
        newCrs = new Base;
        break;

      case 0xE04B5BBC: // "Bitmap"
        newCrs = new Bmp;
        break;

      case 0xE5A51519: // "Geometric"
        newCrs = new Geometric;
        break;

      default:
      {
        Base *derived;

        if ((derived = cursors.Find(key)) != NULL)
        {
          newCrs = new Derived(derived);
        }
        else
        {
          LOG_ERR(("Unknown Cursor Class [%s]", cls));
          return;
        }
        break;
      }
    }

    // Configure the cursor
    newCrs->Configure(fScope);

    // Add it to the list
    cursors.Add(Crc::CalcStr(name), newCrs);
  }


  //
  // Process a StandardCursors scope
  //
  void ProcessStandardCursors(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0x8F651465: // "Default"
          standardCrs[DEFAULT] = FindByName(sScope->NextArgString());
          break;

        case 0x23C19271: // "IBeam"
          standardCrs[IBEAM] = FindByName(sScope->NextArgString());
          break;

        case 0x6E758990: // "Wait"
          standardCrs[WAIT] = FindByName(sScope->NextArgString());
          break;

        case 0x65D94636: // "No"
          standardCrs[NO] = FindByName(sScope->NextArgString());
          break;

        default:
        {
          LOG_ERR(("Unknown standard cursor type [%s]", sScope->NameStr()));
          break;
        }
      }
    }
  }


  //
  // Delete all cursors
  //
  void DeleteAll()
  {
    ASSERT(sysInit);

    // Unset the current cursor
    current = NULL;

    // Clear standard cursors
    for (int i = 0; i < MAX_CURSORS; i++)
    {
      standardCrs[i] = 0;
    }

    // Delete all cursors
    cursors.DisposeAll();
  }


  //
  // Set the cursor to be the active cursor
  //
  Bool Set(const char *name)
  {
    ASSERT(sysInit);
    ASSERT(name);

    if (!Set(Crc::CalcStr(name)))
    {
      LOG_ERR(("Cursor [%s] not found", name));
      return (FALSE);
    }
    return (TRUE);
  }


  //
  // Set the cursor to be the active cursor
  // Specifying 0 will use the system default cursor
  //
  Bool Set(U32 id)
  {
    ASSERT(sysInit);

    // If id is 0 use the default cursor
    if (id == 0)
    {
      id = standardCrs[0];
    }

    Base *crs = cursors.Find(id);

    if (crs != NULL)
    {
      if (crs != current)
      {
        // Shutdown current
        if (current)
        {
          current->Notify(Base::CN_DEACTIVATE);
        }

        // Start the cursor
        current = crs;
        current->Notify(Base::CN_ACTIVATE);
      }
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }



  //
  // Get default cursor
  //
  U32 DefaultCursor()
  {
    return (standardCrs[DEFAULT]);
  }


  //
  // Find a standard cursor
  //
  U32 GetStandardCursor(StdCursor crs)
  {
    ASSERT(crs < MAX_CURSORS);

    if (standardCrs[crs])
    {
      return (standardCrs[crs]);
    }
    else
    {
      return (standardCrs[DEFAULT]);
    }
  }


  //
  // Draw the cursor
  //
  void Display(S32 x, S32 y)
  {
    if (current)
    {
      current->Simulate(IFace::TimeStepMs());
      current->Draw(Point<S32>(x, y));
    }
  }
}
