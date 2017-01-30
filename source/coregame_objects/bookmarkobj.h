///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-AUG-1998
//

#ifndef __BOOKMARKOBJ_H
#define __BOOKMARKOBJ_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "bookmarkobjdec.h"
#include "gameobj.h"
#include "promote.h"
#include "mathtypes.h"
#include "spline.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Forward delcarations
class BookmarkObj;

// Map object reapers
typedef Reaper<BookmarkObj> BookmarkObjPtr;


///////////////////////////////////////////////////////////////////////////////
//
// Class BookmarkObjType - A labelled bookmark composed a camera position
//

class BookmarkObjType : public GameObjType
{
  PROMOTE_LINK(BookmarkObjType, GameObjType, 0x77BEDA4A); // "BookmarkObjType"

public:

  // Constructor
  BookmarkObjType(const char *name, FScope *fScope);

  // Called after all types are loaded
  void PostLoad();

  // Create a new instance using this type
  GameObj* NewInstance(U32 id);
};


///////////////////////////////////////////////////////////////////////////////
//
// Class BookmarkObj - Instance class for above type
//

class BookmarkObj : public GameObj
{
public:

  // Bookmark type
  enum Type
  {
    POSITION,
    CURVE
  };


  //
  // Data structures for building a curve list from the studio or when loaded
  //
  struct CurveSrcSegment
  {
    // Orientation at this node
    Matrix m;

    // Strength at this node
    F32 strength;

    // Time scale
    F32 time;

    NList<CurveSrcSegment>::Node node;
  };

  struct CurveSrc
  {
    NList<CurveSrcSegment> segmentList;

    // Constructor
    CurveSrc() : segmentList(&CurveSrcSegment::node) {}

    // Add a node
    void AddNode(const Matrix &m, F32 strength, F32 timeScale);
  };


  //
  // Data structures for maintaining and interating a curve during the game
  //
  struct CurveSegment
  {
    // The spline
    CubicSpline spline;

    NList<CurveSegment>::Node node;
  };

  struct Curve
  {
    // List of segments
    NList<CurveSegment> segmentList;

    // Total curve length
    F32 totalTime;

    // Does the curve loop?
    U32 loop : 1;

    // Constructor
    Curve() : segmentList(&CurveSegment::node) {}

    // Build the curve from a source
    void Build(CurveSrc &list, Bool loop);

    // Step along the curve
    Bool Step(F32 &time, Vector &pos, Vector *tangent = NULL);
  };


  // List of all current bookmarks
  static NBinTree<BookmarkObj> allBookmarks;

  // Find a bookmark by name
  static BookmarkObj* FindBookmark(const char *bookMarkName);
  
  // Create a bookmark using the current camare location
  static BookmarkObj* CreateBookmark(const char *bookMarkName, Type subType);

private:

  // Name of this bookmark
  GameIdent name;

  // Type
  Type subType;

  // Tree node
  NBinTree<BookmarkObj>::Node node;

  // Bookmark data
  union
  {
    struct
    {
      // Position
      Vector  position;
      F32     yaw;
      F32     pitch;
    };
    struct
    {
      // Source data for curve
      CurveSrc *curveSrc;

      // The actual curve
      Curve *curve;

      // Source data for optional focus curve
      CurveSrc *focusSrc;

      // The actual focus curve
      Curve *focus;
    };
  };


public:

  // Constructor and destructor
  BookmarkObj(BookmarkObjType *objType, U32 id);
  ~BookmarkObj();

  // Called to before deleting the object
  void PreDelete();

  // Load and save state configuration
  void LoadState(FScope *fScope);
  virtual void SaveState(FScope *fScope, MeshEnt * theMesh = NULL);

  // Called after all objects are loaded
  void PostLoad();

  // Jump the camera to the position indicated by the bookmark
  void JumpTo();

  // Swoop the camera to the position indicated by the bookmark
  void SwoopTo();

  // Initalise curve memory
  void InitCurveMemory();

  // InitFocusMemory
  void InitFocusMemory();

  // Reset curve memory
  void DisposeCurves();

  // Step to a point on the curve
  Bool Step(F32 &time, Matrix &matrix);

  // Returns the name of this bookmark
  const char * GetName()
  {
    return (name.str);
  }

  // Returns the type of this bookmark
  Type GetType()
  {
    return (subType);
  }

  // Returns the position of the bookmark
  const Vector & GetPosition()
  {
    ASSERT(subType == POSITION)
    return (position);
  }

  // Returns the curve object
  Curve &GetCurve()
  {
    ASSERT(subType == CURVE)
    ASSERT(curve)
    return (*curve);
  }

  CurveSrc &GetCurveSrc()
  {
    ASSERT(subType == CURVE)
    ASSERT(curveSrc)
    return (*curveSrc);
  }

  // Get optional focus
  Bool HasFocus()
  {
    ASSERT(subType == CURVE)
    return (focusSrc ? TRUE : FALSE);
  }

  Curve &GetFocus()
  {
    ASSERT(subType == CURVE)
    ASSERT(focus)
    return (*focus);
  }

  CurveSrc &GetFocusSrc()
  {
    ASSERT(subType == CURVE)
    ASSERT(focusSrc)
    return (*focusSrc);
  }

  // Returns the yaw of the bookmark
  F32 GetYaw()
  {
    return (yaw);
  }

  // Returns the pitch of the bookmark
  F32 GetPitch()
  {
    return (pitch);
  }

};

#endif  