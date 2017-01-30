///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Camera system
//
// 16-FEB-1999
//


#ifndef __VIEWER_PRIVATE_H
#define __VIEWER_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "viewer.h"
#include "varsys.h"
#include "mathtypes.h"
#include "mapobj.h"
#include "tagobj.h"
#include "bookmarkobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{
  // Camera movement bindings
  extern VarInteger bindLeft;
  extern VarInteger bindRight;
  extern VarInteger bindForward;
  extern VarInteger bindBack;
  extern VarInteger bindFreeLook;
  extern VarInteger bindZoom;

  // Sensitivity settings
  extern VarFloat edgeRate;
  extern VarFloat spinRate;
  extern VarFloat swoopRate;
  extern VarFloat scrollRate;
  extern VarFloat wheelRate;
  extern VarFloat keyRate;

  // Swoop parameters
  extern VarFloat swoopDolly;
  extern VarFloat maxHeight;
  extern VarFloat toggleHeight;

  // Game settings
  extern VarInteger trackTerrain;

  // Minimum camera height
  const F32 MIN_HEIGHT = 2.0F;

  // Maximum camera height
  const F32 MAX_HEIGHT = 1000.0F;

  // Recorded Camera position item
  struct RecItem
  {
    Quaternion q;
    Vector p;
  };


  // Register a new camera
  void RegisterViewer(Base *viewer);

  // Delete a camera
  void DeleteViewer(Base *viewer);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Standard - standard camera
  //
  class Standard : public Base
  {
  private:

    enum State
    {
      STATE_FREE,
      STATE_ROTATE,
      STATE_ZOOM,
      STATE_PAN,
    };

    // Interpolate camera position
    U32 smoothPos : 1,
        smoothHeight : 1,

    // Interpolate orientation
        smoothAttX : 1,
        smoothAttY : 1,

    // Post processing for swoop
        postSwoop : 1,

    // Post processing for circle
        postCircle : 1,

    // Toggle the camera up or down
        toggleDown : 1,

    // Track terrain when panning
        panTrack : 1,

    // Current state
        state : 8;

    // Height above terrain
    F32 height;

    // PostCircle info
    Vector pivotPoint;
    F32 pivotDist;

    // Swoop altitude
    F32 targetHeight;

    // Panning
    Vector panPos0, panPos1;
    Quaternion panAtt0, panAtt1;
    F32 panTime;
    F32 panElapsed;

    // Zooming
    F32 zoomPos;
    F32 zoomMax;
    Matrix zoomOrigin;

    // Current orientation
    Quaternion currAttX, currAttY;

    // Target orientation
    Quaternion targetAttX, targetAttY;

    // Target position
    Vector targetPos;

  protected:

    // Rotate on the spot
    void Spin(Quaternion &q, F32 rads);

    // Circle around a point infront of camera
    void Circle(Quaternion &q, F32 rads, Bool absolute = FALSE);
    void PostCircle();

    // Swoop by desired number of metres
    void Swoop(F32 dist);
    void PostSwoop();

  public:

    // Constructor
    Standard(const char *name);

    // Destructor
    ~Standard();

    // Save and load camera state
    Save(FScope *);
    Load(FScope *);

    // Move to position
    void LookAt(F32 x, F32 z);

    // Exec a scope
    void Exec(FScope *fScope);

    // Simulation
    Bool Simulate(FamilyNode *node, F32 time);

    // Set camera transform
    void Set(const Vector &pos, const Quaternion &quat, U32 flags);

    // Generate quaternion and vector for looking at a position
    Bool GenerateLookAt(F32 x, F32 z, Vector &pos, Quaternion &quat, F32 yaw, F32 height);

    // Notification
    Bool Notify(ViewNotify notify, S32 p1, S32 p2);

    // Command handler
    void CmdHandler(U32 pathCrc);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TrackCam - object tracking camera
  //
  class TrackCam : public Base
  {
  private:

    // Current and desired orientation
    Quaternion q1;

    // Orientation as a quaternion
    Quaternion qView;

    // Pitch in radians - needs to be clamped
    F32 pitch;

    // Adjusted pitch for fitting to terrain
    F32 adjPitch;

    // Time to wait before switching back to main cam
    // after tracking unit has died
    F32 switchDelay;

    // Tracking distance
    F32 radius;

    // Target radius
    F32 targetRadius;

    // Map object to follow
    MapObjPtr trackObj;

  protected:

    // Set pitch
    void SetPitch(F32 r);

    // Set the radius
    void SetRadius(F32 r);

  public:

    // Constructor
    TrackCam(const char *name);

    // Destructor
    ~TrackCam();

    // Move to position
    void LookAt(F32 x, F32 z);

    // Simulation
    Bool Simulate(FamilyNode *node, F32 time);

    // Notification
    Bool Notify(ViewNotify notify, S32 p1, S32 p2);

    // Command handler
    void CmdHandler(U32 pathCrc);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Playback - replay camera
  //
  class PlaybackCam : public Base
  {
  protected:

    // Recorded positions
    RecItem *buf;

    // Number of items
    U32 count;

    // Free the buffer?
    U32 freeBuf : 1;

    // Last game cycle
    U32 lastCycle;

    // Destination orientation and position
    Quaternion qDest;
    Vector pDest;
   
  public:

    // Constructor
    PlaybackCam(const char *name, RecItem *items, U32 count, Bool freeBuf);

    // Destructor
    ~PlaybackCam();

    // Move to position
    void LookAt(F32 x, F32 z);

    // Simulation
    Bool Simulate(FamilyNode *node, F32 time);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CinemaCam - Cinematic camera
  //
  class CinemaCam : public Base
  {
  protected:

    // Tag to follow
    TagObjPtr tag;

    // Orienation
    Quaternion target;

    // Filter value
    F32 filter;

  public:

    Matrix curveMatrix;

    // Constructor
    CinemaCam(const char *name);

    // Simulation
    Bool Simulate(FamilyNode *node, F32 time);

    // Exec a scope
    void Exec(FScope *fScope);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CustomCam - Custom simulate camera
  //
  class CustomCam : public Base
  {
  protected:

    SimulateProc *simProc;
    void *context;

  public:

    // Constructor
    CustomCam(const char *name, SimulateProc *simProc, void *context);

    // Simulation
    Bool Simulate(FamilyNode *node, F32 time);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class PadlockCam
  //
  class PadlockCam : public Base
  {
  protected:

    // Is height data valid?
    U32 validHeight : 1;

    // Position to sit at
    Vector pos;

    // Height
    F32 height;

    // Orientation
    Quaternion target;

    // Tag to follow
    TagObjPtr tag;

    // Filter value
    F32 filter;

  public:

    // Constructor
    PadlockCam(const char *name);

    // Simulation
    Bool Simulate(FamilyNode *node, F32 time);

    // Exec a scope
    void Exec(FScope *fScope);
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Action
  //
  namespace Action
  {

    class Cineractive;

    ///////////////////////////////////////////////////////////////////////////
    //
    // Base cineractive primitive
    //
    class Prim
    {
    public:

      // Start time in seconds from beginning of cineractive
      F32 startTime;
    
      // Set to true when finished
      Bool done;

      // Owner cineractive
      Cineractive *cineractive;

      // Priority - lower is drawn earlier
      S32 priority;

      // NList node
      NList<Prim>::Node node;

    public:

      // Constructor
      Prim(Cineractive *, FScope *, S32 defaultPriority = 0);

      // Destructor
      virtual ~Prim() {}

      // Simulation 
      virtual void Notify(U32 crc) = 0;

      // Priority
      S32 Priority()
      {
        return (priority);
      }
    };


    ///////////////////////////////////////////////////////////////////////////
    //
    // Debrief primitive
    //
    class DebriefPrim : public Prim
    {
    public:

      // The one ane only allowed debriefing
      static DebriefPrim *current;

      // Stuff to execute at end of debriefing
      BinTree<FScope> endScripts;

    protected:

      // List of bookmarks to randomly cycle through
      BookmarkObjList bookmarkList;

      // Move on a random bookmark
      void PickRandomBookmark();

    public:

      // Constructor
      DebriefPrim(Cineractive *, FScope *fScope);
      ~DebriefPrim();

      // Simulation
      void Notify(U32 crc);
    };
  }
}

#endif
