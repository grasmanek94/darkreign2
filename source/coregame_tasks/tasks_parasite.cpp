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
#include "parasiteobj.h"


//
// ~GameTask
//
// Destructor
//
template <> GameTask<ParasiteObjType, ParasiteObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<ParasiteObjType, ParasiteObj>::GameProcess()
{
}
