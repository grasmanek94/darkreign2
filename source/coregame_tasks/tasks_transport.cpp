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
#include "transportobj.h"


//
// ~GameTask
//
// Destructor
//
template <> GameTask<TransportObjType, TransportObj>::~GameTask()
{
}


//
// GameProcess
//
// Type specific processing
//
template <> void GameTask<TransportObjType, TransportObj>::GameProcess()
{
}

