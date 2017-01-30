///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Tasks
// 24-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "task.h"
#include "unitobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Template GameTask<UnitObjType, UnitObj>
//


//
// ~GameTask
//
// Destructor
//
template <> GameTask<UnitObjType, UnitObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<UnitObjType, UnitObj>::GameProcess()
{
  // Perform tactical processing
  Tactical::Process(this, GetTaskTable(), subject);
}
