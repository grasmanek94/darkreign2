///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Camera system
//
// 16-FEB-1999
//


#ifndef __VIEWER_H
#define __VIEWER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mathtypes.h"
#include "utiltypes.h"
#include "fscope.h"
#include "mapobj.h"
#include "environment_quake.h"

///////////////////////////////////////////////////////////////////////////////
//
// Forward declarations
//
class FamilyNode;
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{
  // playing a full screen movie
  extern Bool movie;

  // Camera notification flags
  enum ViewNotify
  {
    // Activating camera
    VN_ACTIVATE,
    VN_REACTIVATE,

    // Deactivating camera
    VN_DEACTIVATE,

    // Camera lost capture
    VN_LOSTCAPTURE,

    // Mouse button notifcations
    VN_LBUTTON,
    VN_MBUTTON,
    VN_RBUTTON,

    // Mouse move notifications
    VN_MOUSEMOVE,
    VN_MOUSEAXIS,
  };

  ///////////////////////////////////////////////////////////////////////////////
  //
  // class Base - base camera class
  //
  class Base
  {
  public:

    // Set flags
    enum
    {
      SET_SMOOTH = 0x0001
    };

    NBinTree<Base>::Node node;

  protected:

    // Camera name
    GameIdent ident;

    // Right scroll mode
    U32 rScroll : 1,

    // Middle scroll mode
        mScroll : 1,

    // First simulation after activation
        firstSim : 1;

    // Current mode
    U32 mode;

    // Orientation as a matrix
    Matrix m;

    // Orientation as a quaternion
    Quaternion orientation;

    // Current velocity
    Vector velocity;

    // Mouse deltas
    S32 dx, dy, dz;

    // Last object locked on to
    MapObjPtr lastLockObj;

  protected:

    // FindObject flags
    enum FindObjectMode
    {
      FO_LASTUNIT,
      FO_FIRST,
      FO_NEXT,
      FO_PREV,
    };

    // Find the next map object from the object passed in
    MapObj *FindObject(MapObjPtr &object, FindObjectMode mode);

    // Are we allowed to look at/track this object
    Bool CanTrack(MapObj *obj);

    // Lock on previous target
    void LockPrevTarget();

    // Lock on next target
    void LockNextTarget();

    // Build a quaternion from p1->p2
    void MakeQuaternion(const Vector &p1, const Vector &p2, Quaternion &q);

  public:

    // Constructor
    Base(const char *name);

    // Destructor
    virtual ~Base() {}

    // Save and load camera state
    virtual Save(FScope *) {};
    virtual Load(FScope *) {};

    // Look at a position
    virtual void LookAt(F32, F32) {}

    // Notification
    virtual Bool Notify(ViewNotify, S32 p1 = 0, S32 p2 = 0);

    // Simulation
    virtual Bool Simulate(FamilyNode *viewNode, F32 time) = 0;

    // Set camera transform
    virtual void Set(const Vector &pos, const Quaternion &quat, U32 flags = 0);

    // Generate quaternion and vector for looking at a position
    virtual Bool GenerateLookAt(F32 x, F32 z, Vector &pos, Quaternion &quat, F32 yaw, F32 height);

    // Execute a single fscope statement
    virtual void Exec(FScope *) {}

    // Command handler
    virtual void CmdHandler(U32) {}

    // SetFirstSim
    void SetFirstSim(Bool f)
    {
      firstSim = f;
    }

    // Get name of camera
    const char *GetName() const
    {
      return (ident.str);
    }

    const Quaternion &GetOrienation() const
    {
      return (orientation);
    }

    // Get current camera position
    const Vector &GetPosition() const
    {
      return (m.posit);
    }

    // Get current camera velocity
    const Vector &GetVelocity() const
    {
      return (velocity);
    }

    // Get current matrix
    const Matrix &GetMatrix() const
    {
      return (m);
    }

    // set family node matrix, add in quake
    //
    void SetWorldMatrix( FamilyNode & node, Matrix & matrix)
    {
      Environment::Quake::SetWorldMatrix( node, matrix);
    }

    // Get current camera mode
    U32 GetMode() const
    {
      return (mode);
    }
  };


  // Current camera
  extern Base *current;

  // Simulation callback
  typedef Bool (SimulateProc)(FamilyNode *, F32, void *);


  // Initialise cameras
  void Init();

  // Shutdown camears
  void Done();

  // Save and load cameras
  void Save(FScope *scope);
  void Load(FScope *scope);

  // Load configuration from game.cfg
  void SaveConfig(FScope *scope);
  void LoadConfig(FScope *scope);

  // Simulation
  void Simulate();

  // Gametime simulation
  void GameTimeSim();

  // Rendering
  void Render(U32 mode);

  void SetWorldMatrix( FamilyNode & node, const Matrix & matrix);

  // Set current camera
  Bool SetCurrent(const char *name, Bool inherit = TRUE, FScope *fScope = NULL);
  Bool SetCurrent(Base *cam, Bool inherit = TRUE, FScope *fScope = NULL);

  // Get current cameras
  inline Base *GetCurrent()
  {
    ASSERT(current);
    return (current);
  }

  // Delete a camera
  Bool Delete(const char *name);
  Bool Delete(Base *cam);

  // Snap current camera to position with yaw/pitch
  void Snap(const Vector &pos, F32 yaw, F32 pitch);

  // Move current camera to position with yaw/pitch
  void Move(const Vector &pos, F32 yaw, F32 pitch);

  // Get orientation of current camera
  void GetOrientation(Vector &pos, F32 &yaw, F32 &pitch);

  // Find smoothed height at metre position
  F32 SmoothedFloor(F32 &x, F32 &z);

  // Get floor at location, handles off map locations
  F32 Floor(F32 &x, F32 &z);

  // Create a custom camera
  void CreateCustomCamera(const char *name, SimulateProc *proc, void *context);

  // Delete a custom camera
  void DeleteCustomCamera(const char *name);

  // Get default height (user configurable)
  F32 GetDefaultHeight();

  // Get default pitch:height ratio
  F32 GetPitchRatio();

  // Get pitch for a certain height
  F32 GetPitchForHeight(F32 height);


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Record - Record camera movement
  //
  namespace Record
  {
    // Save demo info
    Bool SaveDemo(BlockFile &bFile);

    // Load demo info
    Bool LoadDemo(BlockFile &bFile);

    // Stop demo
    void StopDemo();

    // Record camera position - should be done each sim cycle
    void SavePos();
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Action - Lights, camera, ...  *grin*
  //
  namespace Action
  {
    // Init/done
    void Init();
    void Done();

    // Begin cineractive
    void Execute(Team *team, FScope *fScope);

    // Run a debriefing cineractive
    void RunDebriefing(const char *name);

    // Simulation each game cycle
    void GameTimeSim();

    // Abort the cineractive and return to gameplay
    void Abort();

    // Is a cineractive playing back?
    Bool IsPlaying();

    // Pre-simulate processing
    void PreSimulate(F32 time);

    // Rendering
    void Render(U32 mode);
  }
}

#endif
