///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 20-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "bookmarkobj.h"
#include "stdload.h"
#include "gameobjctrl.h"
#include "viewer.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//

// Config scope name for type and world files
#define SCOPE_CONFIG   "BookmarkObj"


///////////////////////////////////////////////////////////////////////////////
//
// Class BookmarkObjType - A labelled bookmark composed a camera position
//


//
// BookmarkObjType::BookmarkObjType
//
// Constructor
//
BookmarkObjType::BookmarkObjType(const char *name, FScope *fScope) : GameObjType(name, fScope)
{
}


//
// BookmarkObjType::PostLoad
//
// Called after all types are loaded
//
void BookmarkObjType::PostLoad()
{
  // Call parent scope first
  GameObjType::PostLoad();
}


//
// BookmarkObjType::NewInstance
//
// Create a new map object instance using this type
//
GameObj* BookmarkObjType::NewInstance(U32 id)
{
  // Allocate new object instance
  return (new BookmarkObj(this, id));
}


///////////////////////////////////////////////////////////////////////////////
//
// Class BookmarkObj - Instance class for above type
//


//
// Static data
//
NBinTree<BookmarkObj> BookmarkObj::allBookmarks(&BookmarkObj::node);


//
// BookmarkObj::FindBookmark
//
// Find a book mark by name
//
BookmarkObj* BookmarkObj::FindBookmark(const char *bookMarkName)
{
  return (allBookmarks.Find(Crc::CalcStr(bookMarkName)));
}


//
// BookmarkObj::CreateBookmark
//
// Create a new bookmark using the current camera location
//
BookmarkObj* BookmarkObj::CreateBookmark(const char *bookMarkName, BookmarkObj::Type subTypeIn)
{
  // If the book mark doesn't exist then make a new one
  if (!FindBookmark(bookMarkName))
  {
    // Find the type 
    if (BookmarkObjType *type = GameObjCtrl::FindType<BookmarkObjType>("BookMark"))
    {
      // Create a new bookmark object (safe cast)
      BookmarkObj *obj = (BookmarkObj*) type->NewInstance(0);
  
      // Set the name of the book mark
      obj->name = bookMarkName;

      // Add to the book mark list
      allBookmarks.Add(obj->name.crc, obj);

      // Set the sub-type
      obj->subType = subTypeIn;

      switch (obj->subType)
      {
        case POSITION:
          // Set the camera matrix of the book mark
          Viewer::GetOrientation(obj->position, obj->yaw, obj->pitch);
          break;

        case CURVE:
          // Instantiate curves
          obj->InitCurveMemory();
          break;
      }

      return (obj);
    }
  }

  return (NULL);
}


//
// BookmarkObj::BookmarkObj
//
// Constructor
//
BookmarkObj::BookmarkObj(BookmarkObjType *objType, U32 id) 
: GameObj(objType, id),
  curveSrc(NULL),
  curve(NULL),
  focusSrc(NULL),
  focus(NULL)
{
  // Set default book mark name
  name = "No Name";
}


//
// BookmarkObj::BookmarkObj
//
// Destructor
//
BookmarkObj::~BookmarkObj()
{
  // Delete curves
  if (subType == CURVE)
  {
    if (curveSrc)
    {
      curveSrc->segmentList.DisposeAll();
      delete curveSrc;
    }
    if (curve)
    {
      curve->segmentList.DisposeAll();
      delete curve;
    }
    if (focusSrc)
    {
      focusSrc->segmentList.DisposeAll();
      delete focusSrc;
    }
    if (focus)
    {
      focus->segmentList.DisposeAll();
      delete focus;
    }
  }

  // Remove from the book mark tree
  allBookmarks.Unlink(this);
}


//
// BookmarkObj::PreDelete
//
// Pre deletion cleanup
//
void BookmarkObj::PreDelete()
{
  // Call parent scope last
  GameObj::PreDelete();
}


//
// Load a curve
//
static void LoadCurve(FScope *fScope, BookmarkObj::CurveSrc *curveSrc, BookmarkObj::Curve *curve)
{
  ASSERT(curveSrc)
  ASSERT(curve)

  FScope *sScope;

  while ((sScope = fScope->NextFunction()) != NULL)
  {
    switch (sScope->NameCrc())
    {
      case 0xF0180394: // "Segment"
      {
        Matrix m;
        F32 strength;
        F32 time;

        StdLoad::TypeMatrix(sScope, "Matrix", m);
        strength = StdLoad::TypeF32(sScope, "Strength");
        time = StdLoad::TypeF32(sScope, "Time");
        curveSrc->AddNode(m, strength, time);
        break;
      }
    }
  } 
  
  // Build the curve
  curve->Build(*curveSrc, StdLoad::TypeU32(fScope, "Loop"));
}


//
// Save a curve
//
static void SaveCurve(FScope *fScope, BookmarkObj::CurveSrc *curveSrc, BookmarkObj::Curve *curve)
{
  // Save the curve parameters
  StdSave::TypeU32(fScope, "Loop", curve->loop);

  // Save each curve segment
  for (NList<BookmarkObj::CurveSrcSegment>::Iterator i(&curveSrc->segmentList); *i; i++)
  {
    FScope *sScope = fScope->AddFunction("Segment");
    StdSave::TypeMatrix(sScope, "Matrix", (*i)->m);
    StdSave::TypeF32(sScope, "Strength", (*i)->strength);
    StdSave::TypeF32(sScope, "Time", (*i)->time);
  }
}


//
// BookmarkObj::LoadState
//
// Load a state configuration scope
//
void BookmarkObj::LoadState(FScope *fScope)
{
  // Call parent scope first
  GameObj::LoadState(fScope);

  // Get specific config scope
  fScope = fScope->GetFunction(SCOPE_CONFIG);

  // Get bookmark name
  name = StdLoad::TypeString(fScope);

  // Load the camera position and orientation
  FScope *sScope;
  
  if ((sScope = fScope->GetFunction("Camera", FALSE)) != NULL)
  {
    subType = POSITION;

    StdLoad::TypeVector(sScope, "Position", position);
    yaw = StdLoad::TypeF32(sScope, "Yaw");
    pitch = StdLoad::TypeF32(sScope, "Pitch");
  }
  else

  if ((sScope = fScope->GetFunction("Curve", FALSE)) != NULL)
  {
    ASSERT(!curveSrc)
    ASSERT(!curve)

    subType = CURVE;
    curveSrc = new CurveSrc;
    curve = new Curve;

    // Load movement curve
    FScope *ssScope;

    ssScope = sScope->GetFunction("Movement");
    LoadCurve(ssScope, curveSrc, curve);

    // Load optional focus curve
    if ((ssScope = sScope->GetFunction("Focus", FALSE)) != NULL)
    {
      focusSrc = new CurveSrc;
      focus = new Curve;
      LoadCurve(ssScope, focusSrc, focus);
    }
  }
  else
  {
    ERR_FATAL(("Bookmark requires either Curve or Camera"))
  }

  // Add to the book mark list
  allBookmarks.Add(name.crc, this);
}


//
// BookmarkObj::SaveState
//
// Save a state configuration scope
//
void BookmarkObj::SaveState(FScope *fScope, MeshEnt * theMesh) // = NULL)
{
  // Call parent scope first
  GameObj::SaveState(fScope);

  // Save config scope with bookmark name
  fScope = StdSave::TypeString(fScope, SCOPE_CONFIG, name.str);

  if (subType == POSITION)
  {
    // Save the camera position and orientation
    FScope *sScope = fScope->AddFunction("Camera");
    StdSave::TypeVector(sScope, "Position", position);
    StdSave::TypeF32(sScope, "Yaw", yaw);
    StdSave::TypeF32(sScope, "Pitch", pitch);
  }
  else

  if (subType == CURVE)
  {
    ASSERT(curve)

    FScope *sScope = fScope->AddFunction("Curve");
    FScope *ssScope;

    // Save the movement curve
    ssScope = sScope->AddFunction("Movement");
    SaveCurve(ssScope, curveSrc, curve);

    // Save optional focus curve
    if (focusSrc && focusSrc->segmentList.GetCount())
    {
      ssScope = sScope->AddFunction("Focus");
      SaveCurve(ssScope, focusSrc, focus);
    }
  }
}


//
// BookmarkObj::PostLoad
//
// Called after all objects are loaded
//
void BookmarkObj::PostLoad()
{
  // Call parent scope first
  GameObj::PostLoad();
}


//
// BookmarkObj::JumpTo
//
// Jump the camera to the position indicated by the bookmark
//
void BookmarkObj::JumpTo()
{
  Viewer::Snap(position, yaw, pitch);
}


//
// BookmarkObj::SwoopTo
//
// Move the camera to the position indicated by the bookmark
//
void BookmarkObj::SwoopTo()
{
  Viewer::Move(position, yaw, pitch);
}


//
// InitCurveMemory
//
// Initalise curve memory
//
void BookmarkObj::InitCurveMemory()
{
  ASSERT(subType == CURVE)
  ASSERT(!curveSrc)
  ASSERT(!curve)

  curveSrc = new CurveSrc;
  curve = new Curve;
}


//
// InitFocusMemory
//
// Initialise focus memory
//
void BookmarkObj::InitFocusMemory()
{
  ASSERT(subType == CURVE)
  ASSERT(!focusSrc)
  ASSERT(!focus)

  focusSrc = new CurveSrc;
  focus = new Curve;
}


//
// DisposeCurves
//
// Reset curve memory
//
void BookmarkObj::DisposeCurves()
{
  ASSERT(subType == CURVE)

  if (curveSrc)
  {
    curveSrc->segmentList.DisposeAll();
    delete curveSrc;
    curveSrc = NULL;
  }
  if (curve)
  {
    curve->segmentList.DisposeAll();
    delete curve;
    curve = NULL;
  }
  if (focusSrc)
  {
    focusSrc->segmentList.DisposeAll();
    delete focusSrc;
    focusSrc = NULL;
  }
  if (focus)
  {
    focus->segmentList.DisposeAll();
    delete focus;
    focus = NULL;
  }
}


//
// Step
//
// Step to a point on the curve
//
Bool BookmarkObj::Step(F32 &time, Matrix &matrix)
{
  ASSERT(subType == CURVE)

  //F32 t = time;
  Vector pos, tangent, focus;

  if (!GetCurve().Step(time, pos, &tangent))
  {
    return (FALSE);
  }

  if (HasFocus())
  {
    if (!GetFocus().Step(time, focus))
    {
      return (FALSE);
    }

    // Point at the focus
    tangent = focus - pos;
  }

  F32 mag;
  matrix = Matrix::I;

  if ((mag = tangent.Magnitude()) > 1e-4F)
  {
    tangent *= (1.0F / mag);
    matrix.SetFromFront( tangent);
  }
  matrix.posit = pos;

  return (TRUE);
}


//
// BookmarkObj::CurveSrc::AddNode
//
// Add a node
//
void BookmarkObj::CurveSrc::AddNode(const Matrix &m, F32 strength, F32 timeScale)
{
  CurveSrcSegment *node = new CurveSrcSegment;

  node->m = m;
  node->strength = strength;
  node->time = timeScale;

  segmentList.Append(node);
}


//
// BookmarkObj::Curve::Build
//
// Build the curve from a source
//
void BookmarkObj::Curve::Build(BookmarkObj::CurveSrc &list, Bool loopIn)
{
  // Save flags
  loop = loopIn;
  totalTime = 0.0F;

  // Rebuild list
  segmentList.DisposeAll();

  for (NList<CurveSrcSegment>::Iterator i(&list.segmentList); *i; i++)
  {
    CurveSrcSegment *curr = *i;
    CurveSrcSegment *next = NULL;

    if (i.IsTail())
    {
      if (loop)
      {
        // Join up to the first
        next = list.segmentList.GetHead();
      }
    }
    else
    {
      // Grab the next
      next = curr->node.GetNext()->GetData();
    }

    // Add a new curve segment
    if (next)
    {
      CurveSegment *seg = new CurveSegment;

      // Setup the curve
      Vector e0 = curr->m.posit;
      Vector t0 = curr->m.front * curr->strength;
      Vector e1 = next->m.posit;
      Vector t1 = next->m.front * next->strength;

      seg->spline.SetupHermite(e0, t0, e1, t1, curr->time);

      // Add this node
      segmentList.Append(seg);

      // Update total time
      totalTime += curr->time;
    }
    else
    {
      // Done
      break;
    }
  }
}


//
// BookmarkObj::Curve::Step
//
// Step along the curve
//
Bool BookmarkObj::Curve::Step(F32 &time, Vector &pos, Vector *tangent)
{
  // Is there at least 2 nodes?
  if (segmentList.GetCount() == 0)
  {
    return (FALSE);
  }

  // Passed the end?
  if (time >= totalTime && !loop)
  {
    time = totalTime;
    return (FALSE);
  }

  // Normalize the time
  time = F32(fmod(time, totalTime));

  // Iterate to the approprate segment
  F32 startTime = 0.0F;

  for (NList<CurveSegment>::Iterator i(&segmentList); *i; i++)
  {
    CurveSegment *curr = *i;

    if ((startTime + curr->spline.length) < time)
    {
      // Move to next segment
      startTime += curr->spline.length;
      continue;
    }

    pos = curr->spline.Step(time - startTime, tangent);

    return (TRUE);
  }
  return (FALSE);
}
