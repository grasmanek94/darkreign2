///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_PRIVATE_H
#define __STUDIO_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio.h"
#include "studio_brush.h"
#include "common_gamewindow.h"
#include "terraindatadec.h"
#include "console.h"


//
// Forward declarations
//
struct Cell;


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  // Information setup each event processing cycle
  struct CycleInfo
  {
    // Current mouse position in screen pixels (always updated)
    Point<S32> mouse;

    // Game window mouse is over (if NULL, all info below is invalid)
    Common::GameWindow *gameWindow;

    // Info for the object currently under the mouse OR 'mapObj' is NULL
    struct 
    { 
      MapObj *mapObj; 
      UnitObj *unitObj;
    } mObject;

    // Terrain position the mouse is over OR 'cell' is NULL
    struct MTerrainStruct
    { 
      Cell *cell;
      Vector pos; 
      TerrainData::Cell *dataCell;
      U32 cellX, cellZ;
    } mTerrain;
  };

  // Private system-wide data structure
  struct SystemData
  {
    // Event processing cycle counter and last update
    U32 cycleCount, lastUpdate;

    // Current brush, or NULL
    Brush::Base *brush;

    // Current capture code (which mouse button has capture)
    S16 captureCode;

    // Currently selected team
    Team *team;

    // Current list of selected objects
    MapObjList sList;

    // Studio vars
    VarInteger mod1;
    VarInteger mod2;
    VarInteger mod3;

    // Processing cycle information
    CycleInfo cInfo;
  };

  // Private system-wide data instance
  extern SystemData data;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Cmd - Command Handler
  //
  namespace Cmd
  {
    // Studio initialization
    void Init();
    void Done();

    // Simulation initialization
    void InitSim();
    void DoneSim();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Event - Studio event handling
  //
  namespace Event
  {
    // Studio initialization
    void Init();
    void Done();

    // Simulation initialization
    void InitSim();
    void DoneSim();

    // Single frame of event processing
    void Process();

    // Set the current brush
    void UseBrush(const char *name, Bool required = TRUE);

    // Generate info for this processing cycle
    void SetupCycleInfo();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Display - Studio rendering
  //
  namespace Display
  {
    // Studio initialization
    void Init();
    void Done();

    // Simulation initialization
    void InitSim();
    void DoneSim();

    // Single frame of display processing
    void Process();
  }
}

#endif