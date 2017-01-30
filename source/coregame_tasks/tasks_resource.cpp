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
#include "resourceobj.h"


//
// ~GameTask
//
// Destructor
//
template <> GameTask<ResourceObjType, ResourceObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<ResourceObjType, ResourceObj>::GameProcess()
{
}
