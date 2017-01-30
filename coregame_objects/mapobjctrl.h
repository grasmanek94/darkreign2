///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUL-1998
//

#ifndef __MAPOBJCTRL_H
#define __MAPOBJCTRL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "mapobjdec.h"
#include "mathtypes.h"
#include "mapobjouchlist.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MapObjCtrl - Manages all map object instances
//
namespace MapObjCtrl
{

  // Initialize system
  void Init();

  // Shutdown system
  void Done();

  // Register the construction and destruction of a map object
  void RegisterConstruction(MapObj *obj);
  void RegisterDestruction(MapObj *obj);

  // Add/remove object from primitive processing list
  void UpdatePrimitiveProcessingState(MapObj *obj);

  // Add and remove objects from the map
  void AddToMap(MapObj *obj);
  void RemoveFromMap(MapObj *obj);
  void RemoveAllFromMap();

  // Delete existing map objects
  void DeleteAll();

  // Remember information at load time
  void AddToListOnMap(MapObj *obj, Bool zip, Bool prepend);

  // Post Load
  void PostLoad();

  // Methods for creating new map objects (always return valid pointers)
  MapObj* ObjectNew(MapObjType *type, U32 id = 0, Bool complete = TRUE);
  MapObj* ObjectNewOnMap(MapObjType *type, const Matrix &m, U32 id = 0, Bool zip = TRUE, Bool complete = TRUE);

  // Same as above methods, but attempt to find type and return NULL on failure
  MapObj* ObjectNew(U32 typeCrc, U32 id = 0, Bool complete = TRUE);
  MapObj* ObjectNewOnMap(U32 typeCrc, const Matrix &m, Bool zip = TRUE, Bool complete = TRUE);

  // sets up for the next frame
  void UpdateMapPos();

  // Do all per-cycle object processing
  void ProcessObjects();

  // Update all mesh transforms
  void ProcessObjectMesh();
  
  // Immediately set the fogging values for all objects 
  void SetObjectFogging(Team *team);

  // Select objects to display for the next frame
  void BuildDisplayList(Team *team, Bool simulateBasic = TRUE);

  // Render all visible objects
  void Render();

  // build mirrored object display list
  void BuildMirrorList();

  // render mirrored objects
  void RenderMirror();

  // Interpolate
  void SimulateInt(F32 elap);

  // Get the list of objects on the map
  const NList<MapObj> & GetOnMapList();

  // Get the list of objects off the map
  const NList<MapObj> & GetOffMapList();

  // Get the list of objects being displayed
  const NList<MapObj> & GetDisplayList();

  // Get the list of objects on health modifier cells
  NList<MapObj> & GetTerrainHealthList();

  // Get the list of objects on the ouch list
  MapObjOuchList & GetOuchList();

}

#endif