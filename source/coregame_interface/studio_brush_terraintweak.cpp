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

#include "studio_brush_terraintweak.h"
#include "studio_history_terrain.h"
#include "terrain.h"
#include "common.h"
#include "viewer.h"


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
    // Class TerrainTweak - Brush for advanced terrain editing
    //

    //
    // Constructor
    //
    TerrainTweak::TerrainTweak(const char *name) : ApplyCell(name)
    {
      // Create interface vars
      varVertexColor1 = CreateInteger("vertexColor1", Color((S32)64, (S32)128, (S32)64));
      varVertexColor2 = CreateInteger("vertexColor2", Color((S32)255, (S32)255, (S32)255));
      varColorScale = CreateFloat("colorScale", 0.3f, 0.001f, 1.0f);
      varSmoothFloor = CreateInteger("smoothFloor", FALSE);
    }


    //
    // Destructor
    //
    TerrainTweak::~TerrainTweak()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool TerrainTweak::HasProperty(U32 property)
    {
      return (ApplyCell::HasProperty(property));
    }

  
    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void TerrainTweak::NotifyVar(IFaceVar *var)
    {
      // Pass notification down
      ApplyCell::NotifyVar(var);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void TerrainTweak::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x0FE20111: // "Brush::Apply"
        { 
          // Add history
          History::Terrain *history = History::Terrain::GetItem();

          Color dest
          (
            Common::Input::GetModifierKey(3) ? varVertexColor2->GetIntegerValue() : varVertexColor1->GetIntegerValue()
          );

          F32 scale = varColorScale->GetFloatValue();

          // Apply color to each cell in list
          for (CellList::Iterator i(&cellList); *i; i++)
          {
            Cell &c = **i;

            if (Terrain::CellOnMap(c.pos.x, c.pos.z))
            {
              // Add this cell to the history
              history->Add(c.pos.x, c.pos.z);

              // Get the raw value (0..255)
              F32 value = Clamp<F32>(0.0f, (F32)c.value, 255.0f);

              // Is there a value at this cell
              if (value)
              {
                // Get the increment value (0..1)
                F32 inc = Min<F32>((value / 255.0f) * scale, 1.0f);

                // Get the terrain cell
                ::Cell *cell = Terrain::GetCell(c.pos.x, c.pos.z);

                // Get the source color
                Color src(cell->GetColor());

                // If not already there
                if (src != dest)
                {
                  src.Interpolate(src, dest, inc);
                  cell->SetColor(src);
                }
              }
            }
          }

          // Block this event
          return;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Is mouse over the terrain
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            ::Cell *cell = Terrain::GetCell
            (
              data.cInfo.mTerrain.cellX, data.cInfo.mTerrain.cellZ
            );
            
            if (Common::Input::GetModifierKey(3))
            {
              varVertexColor2->SetIntegerValue(cell->GetColor());
            }
            else
            {
              varVertexColor1->SetIntegerValue(cell->GetColor());
            }
          }

          // Block this event
          return;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          // Are the points valid
          if (brushSetup)
          {
            Color dest
            (
              Common::Input::GetModifierKey(3) ? varVertexColor2->GetIntegerValue() : varVertexColor1->GetIntegerValue()
            );

            // Draw the brush
            ::Terrain::RenderCellRect(clipRect, dest, TRUE, TRUE);

            if (varSmoothFloor->GetIntegerValue())
            {
              Matrix  m = Matrix::I;

              for (S32 x = clipRect.p0.x; x <= clipRect.p1.x; x++)
              {
                for (S32 z = clipRect.p0.y; z <= clipRect.p1.y; z++)
                {
                  F32 mx = F32(x) * WC_CELLSIZEF32;
                  F32 mz = F32(z) * WC_CELLSIZEF32;

                  if (WorldCtrl::MetreOnMap(mx, mz))
                  {
                    m.posit.x = mx;
                    m.posit.z = mz;
                    m.posit.y = Movement::SmoothedFindFloor(mx, mz, NULL);

                    Common::Display::Mesh(0x92098DF3, m, 0xFFFFFFFF); // "TerrainMarker"
                  }
                }
              }
            }
          }

          // Always pass system events down
          break;
        }
      }

      // Not blocked at this level
      ApplyCell::Notification(crc, e);
    }
  }
}
