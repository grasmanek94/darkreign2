///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_brush_applycell.h"
#include "worldctrl.h"
#include "common.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class ApplyCell - Base for all terrain cell application brushes
    //

    //
    // Constructor
    //
    ApplyCell::ApplyCell(const char *name) : Apply(name)
    {
      // Setup default brush size
      xSize = 3;
      zSize = 3;

      // Setup default brush min and max
      xSizeMin = 1;
      zSizeMin = 1;
      xSizeMax = MAXBRUSHX;
      zSizeMax = MAXBRUSHZ;

      // Reset data
      bitmap = NULL;
      brushSetup = FALSE;

      // Create interface vars
      varBrushBitmap = CreateString("brushBitmap", "");
      varMirrorAxes = CreateInteger("mirrorAxes", 0, 0, 3);
      varMirrorOffsetX = CreateInteger("mirrorOffsetX", 0);
      varMirrorOffsetZ = CreateInteger("mirrorOffsetZ", 0);
    }


    //
    // Destructor
    //
    ApplyCell::~ApplyCell()
    {
      // Delete any cells left in list
      cellList.DisposeAll();

      // Delete any remaining bitmap
      if (bitmap)
      {
        delete bitmap;
      }
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool ApplyCell::HasProperty(U32 property)
    {
      return (Apply::HasProperty(property));
    }

    
    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void ApplyCell::NotifyVar(IFaceVar *var)
    {
      // Bitmap name has changed
      if (var == varBrushBitmap)
      {
        // Get the bitmap file name
        const char *fileName = varBrushBitmap->GetStringValue();

        // Delete any existing bitmap
        if (bitmap)
        {
          delete bitmap;
          bitmap = NULL;
        }

        // Do we have a file name
        if (*fileName != '\0')
        {
          // Create a new bitmap
          bitmap = new Bitmap(bitmapNORMAL | bitmapNOMANAGE);

          // Read in the file
          if (!bitmap->Read(fileName) || bitmap->Depth() != 8)
          {
            LOG_WARN(("Bitmap not found or not 8 bit [%s]", fileName));

            // Delete the bitmap
            delete bitmap;
            bitmap = NULL;

            // Clear the var
            varBrushBitmap->SetStringValue("");
          }
        }
      }
      else
      {
        // Pass notification down
        Apply::NotifyVar(var);
      }
    }

    void ApplyCell::SetSize( S32 sx, S32 sy)
    {
      xSize = sx;
      zSize = sy;

      // The unclipped points              
      brushPoint0.Set(centre.x - (xSize / 2), centre.z - (zSize / 2));
      brushPoint1.Set(brushPoint0.x + xSize - 1, brushPoint0.z + zSize - 1);

      // Generate the cell list based on these points
      GenerateCellList(brushPoint0, brushPoint1, cellList, bitmap);

      // Setup the unclipped rectangle
      brushRect.Set( 
        brushPoint0.x, brushPoint0.z,
        brushPoint1.x + 1, brushPoint1.z + 1);

      // The clipped points
      clipPoint0.x = Max<S32>(brushPoint0.x, 0);
      clipPoint0.z = Max<S32>(brushPoint0.z, 0);
      clipPoint1.x = Min<S32>(brushPoint1.x, WorldCtrl::CellMapX() - 1);
      clipPoint1.z = Min<S32>(brushPoint1.z, WorldCtrl::CellMapZ() - 1);

      // Setup the clipped rectangle
      clipRect.Set( 
        clipPoint0.x, clipPoint0.z,
        clipPoint1.x + 1, clipPoint1.z + 1);
    }

    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void ApplyCell::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x44D1C3B4: // "System::PostCycleInfoPoll"
        {
          // Do not update if key held or not over terrain
          if (Common::Input::GetModifierKey(1) || !(data.cInfo.gameWindow && data.cInfo.mTerrain.cell))
          {
            break;
          }

          // Is cursor over the terrain
          ASSERT(data.cInfo.gameWindow && data.cInfo.mTerrain.cell)

          // If we don't have capture, or we're applying the brush
          if (!HasCapture() || captureApply == CM_ON || captureResize == CM_ON)
          {
            // Reset data
            brushSetup = FALSE;
            cellList.DisposeAll();

            // Where is the starting position (middle of brush)
            centre.x = (captureResize == CM_ON) ? resizePoint.x : data.cInfo.mTerrain.cellX;
            centre.z = (captureResize == CM_ON) ? resizePoint.z : data.cInfo.mTerrain.cellZ;

            SetSize( xSize, zSize);

            ASSERT(xSize > 0 && zSize > 0)
            ASSERT(clipRect.p0.x < clipRect.p1.x && clipRect.p0.y < clipRect.p1.y)

            // Points are valid
            brushSetup = TRUE;
          }

          // Always pass system events down
          break;
        }
        
        case 0xB112E9BD: // "Brush::ResizeStart"
        {
          // Is the cursor over a terrain cell
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            // Save the starting cell position
            resizePoint.Set
            (
              data.cInfo.mTerrain.cellX, data.cInfo.mTerrain.cellZ
            );
          }
          else
          {
            // Clear the resize capture (set in apply brush)
            captureResize = CM_OFF;
          }

          // Block this event
          return;
        }
      }

      // Not blocked at this level
      Apply::Notification(crc, e);
    }


    //
    // GenerateCellList
    //
    // Generate a cell list using the given bitmap
    //
    void ApplyCell::GenerateCellList(Point<S32> a, Point<S32> b, CellList &list, Bitmap *map)
    {
      F32 xMap = 0.0F, zMap = 0.0F;
      S32 value = 255;

      // Ensure sorted points
      if (a.x > b.x) { Swap(a.x, b.x); }
      if (a.z > b.z) { Swap(a.z, b.z); }  

      // Setup mapping values
      if (map)
      {
        xMap = F32(map->Width()) / F32(b.x - a.x + 1);
        zMap = F32(map->Height()) / F32(b.z - a.z + 1);
      }

      // Iterate over the cell area
      for (S32 z = a.z; z <= b.z; z++)
      {
        for (S32 x = a.x; x <= b.x; x++)
        {
          if (Terrain::CellOnMap(x, z))
          {
            // Get value from bitmap
            if (map)
            {
              // Get pixel source
              S32 px = S32(F32(x - a.x) * xMap);
              S32 pz = S32(F32(z - a.z) * zMap);
            
              // Paranoia
              ASSERT(px >= 0 && px < map->Width());
              ASSERT(pz >= 0 && pz < map->Height());

              // Set the value
              value = map->GetPixel(px, pz);
            }

            S32 wx = WorldCtrl::CellMapX() + varMirrorOffsetX->GetIntegerValue();
            S32 wz = WorldCtrl::CellMapZ() + varMirrorOffsetZ->GetIntegerValue();

            // Add the cell 
            switch (varMirrorAxes->GetIntegerValue())
            {
              case 0:
                list.Append(new Cell(x, z, value));
                break;

              case 1:
                list.Append(new Cell(x, z, value));
                list.Append(new Cell(wx - x, z, value));
                break;

              case 2:
                list.Append(new Cell(x, z, value));
                list.Append(new Cell(wx - x, z, value));
                list.Append(new Cell(x, wz - z, value));
                list.Append(new Cell(wx - x, wz - z, value));
                break;

              case 3:
                list.Append(new Cell(x, z, value));
                list.Append(new Cell(wx - x, z, value));
                list.Append(new Cell(x, wz - z, value));
                list.Append(new Cell(wx - x, wz - z, value));

                list.Append(new Cell(z, x, value));
                list.Append(new Cell(z, wx - x, value));
                list.Append(new Cell(wz - z, x, value));
                list.Append(new Cell(wz - z, wx - x, value));
                break;
            }
          }
        }
      }         
    }


    //
    // ModifySize
    //
    // Resize this brush using the given deltas
    //
    void ApplyCell::ModifySize(S32 dx, S32 dz)
    {
      // Set new values and clamp
      xSize = Clamp<S32>(xSizeMin, xSize + dx, xSizeMax);
      zSize = Clamp<S32>(zSizeMin, zSize + dz, zSizeMax);
    }
  }
}
