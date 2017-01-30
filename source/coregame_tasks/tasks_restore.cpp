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
#include "restoreobj.h"


//
// ~GameTask
//
// Destructor
//
template <> GameTask<RestoreObjType, RestoreObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<RestoreObjType, RestoreObj>::GameProcess()
{
}
