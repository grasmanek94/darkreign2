///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//

#ifndef __COMMON_H
#define __COMMON_H

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "utiltypes.h"
#include "mapobjdec.h"
#include "varsys.h"
#include "footprint.h"
#include "trailobj.h"


//
// Forward declarations
//
class Bitmap;

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Common - Code used in both the game and the studio
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Display - Various common display functions
  //
  namespace Display
  {
    // clear screen to black?
    extern VarInteger clear;   
    
    // brighten units the cursor is over
    extern VarInteger bright;       

    // Initialize and shutdown sub-system
    void Init();
    void Done();

    void InitSim();
    void DoneSim();

    // Get a texture asset
    Bitmap *GetTexture(U32 id);

    // Draws a rectangle flat on the screen
    void Rectangle(Color color, const Area<S32> &rect);

    // Displays the given texture on the ground below the object
    void ObjectGroundSprite(MapObj *obj, Color color, Bitmap *texture = NULL);

    // Displays an object as selected
    void ObjectSelection(MapObj *obj, Bitmap *texture = NULL, U32 alpha = 128);

    // Draws the health of a map object
    void ObjectHealthBar(MapObj *obj, Bitmap *tex, U32 alpha = 128);

    // Render a mesh with a color
    void Mesh(U32 id, const Matrix &m, const Color &c);

    // Render a mesh without a color
    void Mesh(U32 id, const Matrix &m);

    // Get a mesh from an id
    MeshRoot * GetMesh(U32 id);

    // Mark a terrain position with an arrow
    void MarkPosition(F32 x, F32 z, Color color);

    // Mark a world position with an arrow
    void MarkPosition(const Vector &pos, Color color);

    // Mark a world position using the given matrix
    void MarkPosition(const Matrix &matrix, Color color);

    // Mark a cell with an arrow
    void MarkCell(U32 cx, U32 cz, Color color);

    // Marks cells on the perimeter of an area
    void MarkAreaCell(const Area<U32> &area, Color *colors = NULL);

    // Marks cells on the perimeter of an area
    void MarkAreaMetre(const Area<F32> &area, Color *colors = NULL);

    // Marks cells on the perimeter of a circle
    void MarkCircleMetre(const Point<F32> &point, F32 radius, const Color &color);

    // Draw a rectangle around a grain
    void MarkGrain(S32 x, S32 z, Color c, F32 height);


    // Add a marker
    void AddMarker(S32 x, S32 z, F32 value);

    // Remove a marker
    void RemoveMarker(S32 x, S32 z);

    // Clear markers
    void ClearMarkers();

    // Render markers
    void RenderMarkers();


    // PlacementDeform structure
    struct PlacementDeform
    {
      enum { MAX_PTS = 64 };

      // Terrain cells that are hidden
      Point<S32> points[MAX_PTS];

      // Number of hidden cells
      U32 count;

      // Constructor
      PlacementDeform() 
      {
        Init();
      }

      // Initialise
      void Init()
      {
        count = 0;
      }
    };


    // Display a footprint placement grid
    void FootPrintPlacement(FootPrint::Placement &placement, Bool mesh = FALSE, Matrix *matrix = NULL, F32 alpha = 1.0F);
    
    // Display a footprint placement grid with terrain deformation
    void FootPrintPlacementDeform(FootPrint::Placement &placement, PlacementDeform &deform, Bool mesh = FALSE, Matrix *matrix = NULL, F32 alpha = 1.0F);

    // PostFootPrintPlacementDeform
    void PostFootPrintPlacementDeform(PlacementDeform &deform);

    // Display the mesh of a footprint placement grid
    void FootPrintPlacementMesh(FootPrint::Placement &placement, Matrix *matrix, const Color &color);


    // Return the group selection color
    Color GroupSelectColor();

    // Display a list of waypoints
    void WayPoints
    (
      const TrailObj::WayPointList &list, const Color &c, TrailObj::Mode mode = TrailObj::MODE_TWOWAY
    );

    // Render debugging info
    void RenderDebug();
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Cycle - Cycle based information for client and studio
  //
  namespace Cycle
  {
    // Initialize and shutdown sub-system
    void Init();
    void Done();
    
    void InitSim();
    void DoneSim();

    // Call at the start and end of a cycle
    void Start();
    void End();

    // Clear the current selected object list
    void ClearSelected();

    // Add an object to the selected list
    void AddSelected(MapObj *obj);

    // Set the object the mouse is over
    void SetMouseOverObject(MapObj *obj);

    // Get the list of selected objects
    MapObjList & SelectedList();

    // Get the object in focus
    MapObj * FocusObject();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Input - Common input functions
  //
  namespace Input
  {
    // Initialise common input
    void Init();

    // Is a modifier key down?
    Bool GetModifierKey(U32 i);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Debug
  //
  namespace Debug
  {
    // Debugging vars
    struct Data
    {
      VarInteger pathSearch;
      VarInteger movement;
      VarInteger weapons;
      VarInteger claiming;
      VarInteger target;
      VarInteger sight;
      VarInteger hud;
      VarInteger footDeform;
      VarInteger thumpMutex;
    };

    extern Data data;
  }

  // Display full preview
  extern VarInteger fullPreview;

  // Initialize and shutdown common systems
  void Init();
  void Done();

  // Initialize and shutdown simulation systems
  void InitSim();
  void DoneSim();

  // Are we in the studio
  Bool InStudio();
}

#endif