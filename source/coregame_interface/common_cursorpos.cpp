///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 11-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_cursorpos.h"
#include "movement_pathfollow.h"
#include "worldctrl.h"
#include "terrain.h"
#include "input.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TerrainCursorPos
  //


  //
  // TerrainCursorPos::TerrainCursorPos
  //
  TerrainCursorPos::TerrainCursorPos(IControl *parent) : ICStatic(parent)
  {
    // Create a buffer for the display
    SetTextString(buf, FALSE);

    // Setup justification
    SetTextJustify(JUSTIFY_RIGHT);
  }


  //
  // TerrainCursorPos::DrawSelf
  //
  // Update cursor position and redraw self
  //
  void TerrainCursorPos::DrawSelf(PaintInfo &pi)
  {
    // Update the cursor position
    Vector pos;

    *textStr = CH(0);

    // Fixme: screentoterrain shouldnt return a position off the map
    if (Terrain::mapInit && TerrainData::ScreenToTerrain(::Input::MousePos().x, ::Input::MousePos().y, pos) && WorldCtrl::MetreOnMap(pos.x, pos.z))
    {
      switch (units)
      {
        case 0x6EC59045: // "Grains"
        {
          S32 gx, gz;
          U8 q = Movement::MetreToGrain(pos.x, pos.z, gx, gz);
          Utils::Sprintf(buf, 128, L"G%4d,%4d Q%d", gx, gz, q);
          break;
        }

        case 0xFAD28AB8: // "Cells"
          Utils::Sprintf(buf, 128, L"%4d,%4d", WorldCtrl::MetresToCellX(pos.x), WorldCtrl::MetresToCellZ(pos.z));
          break;

        case 0x6ECAAA17: // "Clusters"
          Utils::Sprintf(buf, 128, L"%4d,%4d", WorldCtrl::MetresToClusterX(pos.x), WorldCtrl::MetresToClusterZ(pos.z));
          break;

        case 0x47ED7A3C: // "Metres"
        default:
          Utils::Sprintf(buf, 128, L"%6.1fm,%6.1fm", pos.x, pos.z);
          break;
      }
    }

    // Redraw the control
    ICStatic::DrawSelf(pi);
  }


  //
  // Configuration
  //
  void TerrainCursorPos::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xCED02493: // "Units"
      {
        units = StdLoad::TypeStringCrc(fScope);
        break;
      }

      default:
        ICStatic::Setup(fScope);
        break;
    }
  }
}
