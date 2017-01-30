///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Path Searching
//
// 8-SEP-1998
//

//
// Includes
//
#include "pathsearch_pqueue.h"
#include "varsys.h"
#include "console.h"
#include "input.h"
#include "icgridwindow.h"
#include "worldctrl.h"
#include "movetable.h"
#include "terraindata.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace PathSearch - Point-based path generation
//

namespace PathSearch
{
  // GridWindow control
  static ICGridWindowPtr pathGrid;

  // Current traction type
  static U8 tractionType;


  
  //
  // SurfaceColor
  //
  // Returns the surface color for use in the path grid
  //
  static F32 SurfaceColor(U8 traction, TerrainData::Cell &cell)
  {
    if (MoveTable::SurfaceCount() && MoveTable::TractionCount())
    {
      MoveTable::BalanceData &d = MoveTable::GetBalance(cell.surface, traction);

      if (CanMoveToCell(d, cell))
      { 
        ASSERT(d.slope)

        return 
        (
          (F32(cell.slope) / F32(d.slope) * 0.5F) + (1.0F - d.speed) * 0.5F
        );
      }
    }

    return (1.0F);
  }


  //
  // PathCellCallBack
  //
  // Draws the surface types on the grid 
  //
  static Color PathCellCallBack(void *, U32 x, U32 z)
  {
    // Is this a blocked cell
    if (data.blockArray->Get2(x, z))
    {
      return (Color(1.0F, 1.0F, 0.0F));
    }

    TerrainData::Cell &tCell = TerrainData::GetCell(x, z);
    Cell *pCell = GetCell(x, z);

    // Default to black
    F32 r = 0.0F, g = 0.0F, b = 0.0F, a = 0.6F;

    switch (data.searchType)
    {
      case ST_ASTAR:
      {
        // From current search
        if (pCell->zMark == data.zMarks[z])
        {
          // No alpha
          a = 1.0F;

          // Closed set
          if (pCell->closed)
          {
            r = 0.4F;
          }
          else
          {
            // In open set
            g = 0.4F;
          }
        }

        b = SurfaceColor(data.request.tractionType, tCell);
        break;
      }

      case ST_TRACE:
      {
        F32 sense[2] = {0.0F, 0.0F};

        if (pCell->zMark == data.zMarks[z])
        {
          // No alpha
          a = 1.0F;

          for (U32 l = 0; l < 2; l++)
          {
            if (pCell->sense[l].visited)
            {
              sense[l] = 0.4F;
              if (x == data.trace.sense[l].curPos.x && z == data.trace.sense[l].curPos.z)
              {
                sense[l] += 0.4F;
              }
            }
          }
        }
    
        F32 box = 
        (
          x >= data.trace.minBound.x && x <= data.trace.maxBound.x &&
          z >= data.trace.minBound.z && z <= data.trace.maxBound.z
        ) 
        ? 0.2F : 0.0F;
        
        r = sense[0] + box;
        g = sense[1] + box;
        b = SurfaceColor(data.request.tractionType, tCell);
      }
    }

    return (Color(r, g, b, a));
  }


  //
  // PathPostCallBack
  //
  // Draws the current path on the grid
  //
  static void PathPostCallBack(ICGrid *control)
  {
    static U32 loop = 0;
    static U32 delay = 0;

    if (data.lastPath)
    {
      U32 l = 0;
      Bool show = FALSE;
      Point showP;

      for (NList<Point>::Iterator i(&data.lastPath->points); *i; i++)
      {
        if (l == loop)
        {
          show = TRUE;
          showP = **i;
        }

        control->DrawCell((*i)->x, (*i)->z, Color(1.0F, 1.0F, 1.0F));
        l++;
      }

      if (show)
      {
        control->DrawCell(showP.x, showP.z, Color(1.0F, 0.0F, loop ? 1.0F : 0.0F));
      }

      if (!(delay%5))
      {
        loop = (loop >= data.lastPath->points.GetCount() - 1 ? 0 : loop + 1);
      }
      delay++;
    }
    else
    {
      loop = 0;
    }
  }


  //
  // CmdHandler
  //
  // Handles commands for the path searching var scope
  //
  static void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x87CEC309: // "coregame.psearch.cellspercycle"
      {
        S32 count;
        if (Console::GetArgInteger(1, count))
        {
          data.cellsPerCycle = (U32)count;
        }
        break;
      }

      case 0xCAC71605: // "coregame.psearch.pathwatch"
      {
        // Create the control
        if (pathGrid.Alive())
        {
          pathGrid->Deactivate();
        }
        else
        {
          U32 xc = 200 / WorldCtrl::CellMapX();
          U32 zc = 200 / WorldCtrl::CellMapZ();
          U32 c  = Max<U32>(1, Min<U32>(xc, zc));

          // Allocate the control
          pathGrid = new ICGridWindow("Path Map", WorldCtrl::CellMapX(), WorldCtrl::CellMapZ(), c, c);

          // Setup the grid
          ICGrid &grid = pathGrid->Grid();
          grid.SetCellCallBack(PathCellCallBack);
          grid.SetPostCallBack(PathPostCallBack);
          grid.SetAxisFlip(FALSE, TRUE);

          // Setup the window
          pathGrid->SetGeometry("top", "right", NULL);
          pathGrid->Activate();
        }
        break;
      }     
    } 
  }

  
  //
  // InitCmd
  //
  // Intialize path searching commands
  //
  void InitCmd()
  {
    // Create the command scope
    VarSys::RegisterHandler("coregame.psearch", CmdHandler);

#ifdef DEVELOPMENT

    // Create commands
    VarSys::CreateCmd("coregame.psearch.cellspercycle");
    VarSys::CreateCmd("coregame.psearch.pathwatch");

#endif

    // Setup private data
    pathGrid = NULL;
    tractionType = 0;

    // Setup debugging
    data.lastPath = NULL;
  }


  //
  // DoneCmd
  //
  // Shutdown command system
  //
  void DoneCmd()
  {
    if (pathGrid.Alive())
    {
      pathGrid->Deactivate();
    }

    // Delete the var scope
    VarSys::DeleteItem("coregame.psearch");
  }
}