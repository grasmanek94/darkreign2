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

#include "studio_brush_terrain.h"
#include "studio_history_terrain.h"
#include "mapobjctrl.h"
#include "movement_pathfollow.h"
#include "input.h"
#include "terraindata.h"
#include "terraingroup.h"
#include "iface.h"
#include "iclistbox.h"
#include "common.h"
#include "fontsys.h"
#include "iface_util.h"


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
    // Class Terrain - Brush for standard terrain editing
    //

    //
    // Constructor
    //
    Terrain::Terrain(const char *name) : ApplyCell(name)
    {
      // Allocate the heightfield brush
      bInfo.heightField = new HeightField;

      // Setup the segment rectangle to the full field
      bInfo.segment.Set(SAMPLE_X, SAMPLE_Z);

      // Create the custom brush bitmap
      bInfo.map.Create(SAMPLE_X, SAMPLE_Z, FALSE, 0, 8);

      // Create interface vars
      varBrushType = CreateString("brushtype", "Bell");
      varDrawTexture = CreateInteger("drawtexture", 0, 0, 1);
      varDrawHeight = CreateInteger("drawheight", 1, 0, 1);
      varRaise = CreateInteger("raise", 1, 0, 1);
      varRandom = CreateInteger("random", 0, 0, 1);
      varTerrainType = CreateString("terraintype", "Base");
      varScale = CreateFloat("scale", 1.0f, 0.01f, 1.0f);
      varHeight = CreateFloat("height", 10.0f, -2000.0f, 2000.0f);
      
      // Setup with default settings
      if (!bInfo.heightField->Setup(SAMPLE_X, SAMPLE_Z, 1, HeightField::BELL, 1.0F, &bInfo.map))
      {
        ERR_FATAL(("Failed setting up heightfield for terrain brush"));
      }

      // Create the default brush
      SetBrushType("Bell");

      // Set brush to paint base terrain
      bInfo.terrainType = 0;

      // Custom cell adjustment
      direction = TerrainData::BD_NORTH;
      variation = 0;
      adjustCells = FALSE;
    }


    //
    // Destructor
    //
    Terrain::~Terrain()
    {
      // Delete the heightfield
      delete bInfo.heightField;

      // Delete the custom bitmap
      bInfo.map.Release();
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Terrain::HasProperty(U32 property)
    {
      return (ApplyCell::HasProperty(property));
    }

  
    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Terrain::NotifyVar(IFaceVar *var)
    {
      // Set new brush type
      if (var == varBrushType)
      {
        SetBrushType(var->GetStringValue());
      }
      else

      // Change terrain type
      if (var == varTerrainType)
      {
        // Get the specified type
        TerrainGroup::Type *type = TerrainGroup::ActiveGroup().FindType
        (
          varTerrainType->GetStringValue()
        );

        if (type)
        {
          bInfo.terrainType = type->Index();
        }
      }
      else

      // Brush flags
      if (var == varDrawHeight || var == varRaise)
      {
        UpdateBrushFlags();
      }

      // Pass notification down
      ApplyCell::NotifyVar(var);
    }


    //
    // SetBrushType
    //
    // Sets the current brush type
    //
    void Terrain::SetBrushType(const char *brushName)
    {
      switch (Crc::CalcStr(brushName))
      {
        case 0xC3BF1620: // "Bell"
          brushType = BELL;
          bInfo.type = HeightField::BELL;
          break;

        case 0xC73B0775: // "Flat"
          brushType = FLAT;
          bInfo.type = HeightField::FLAT; 
          break;

        case 0xED9B0C17: // "Smooth"
          brushType = SMOOTH;
          bInfo.type = HeightField::CUSTOM; 
          break;

        case 0xCE0F7702: // "Plasma"
          brushType = PLASMA;
          bInfo.type = HeightField::CUSTOM; 
          break;

        // Ignore unknown or empty brush names
        default:
          return;
      }

      // Make a new brush with the new type
      bInfo.heightField->Make(bInfo.type, 1.0F, &bInfo.map);

      // Update the brush flags based on the new brush type
      UpdateBrushFlags();
    }


    //
    // UpdateBrushFlags
    //
    // Update the brush flags based on current var values
    //
    void Terrain::UpdateBrushFlags()
    {
      // Clear the flags
      bInfo.flags = 0;

      // Set flags based on the brush
      switch (brushType)
      {
        case PLASMA: bInfo.flags |= HeightField::EDITPLASMA; break;
        case SMOOTH: bInfo.flags |= HeightField::EDITSMOOTH; break;
      }

      // Should we draw height
      if (varDrawHeight->GetIntegerValue())
      {
        bInfo.flags |= HeightField::EDITHEIGHTS;
      }

      // Should we add height
      if (varRaise->GetIntegerValue())
      {
        bInfo.flags |= HeightField::EDITADD;
      }
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Terrain::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x0ACEA1BD: // "Brush::RightMouseClick"
        {
          // Deform and paint
          if (varDrawHeight->GetIntegerValue() && varDrawTexture->GetIntegerValue())
          {
            // Switch to deform
            varDrawTexture->SetIntegerValue(FALSE);
          }
          else

          // Deform only
          if (!varDrawTexture->GetIntegerValue())
          {
            // Switch to paint
            varDrawHeight->SetIntegerValue(FALSE);
            varDrawTexture->SetIntegerValue(TRUE);
          }
          else
          {
            // Switch on deform
            varDrawHeight->SetIntegerValue(TRUE);
          }

          // Block this event
          return;
        }

        case 0x0FE20111: // "Brush::Apply"
        { 
          // Were brush points setup this cycle
          if (brushSetup)
          {
            // Add history
            History::Terrain *history = History::Terrain::GetItem();
            history->Add(brushPoint0, brushPoint1);

            // Are we drawing heights
            if (varDrawHeight->GetIntegerValue())
            {
              // Get the brush scale
              F32 scale = varScale->GetFloatValue();
            
              // Reverse if shift is held
              if (Common::Input::GetModifierKey(3))
              {
                scale = -scale;
              }
             
              // Start a modification session
              TerrainData::SessionStart();

              // Paste the terrain
              ::Terrain::Paste
              (
                brushRect, *bInfo.heightField, bInfo.segment, scale, bInfo.flags, 
                varHeight->GetFloatValue()
              );

              // Notify that area has changed
              TerrainData::SessionAddArea(brushPoint0, brushPoint1);
              TerrainData::SessionEnd();
            }

            // Should we draw textures
            if (varDrawTexture->GetIntegerValue())
            {
              TerrainData::PaintList paintList;

              // Build the paint list
              for (CellList::Iterator i(&cellList); *i; i++)
              {
                if ((*i)->value)
                {
                  paintList.Append(new TerrainData::PaintCell((*i)->pos, bInfo.terrainType));
                }
              }

              // Paint the terrain
              TerrainData::Paint(paintList);

              // Dispose of paint list
              paintList.DisposeAll();
            }
          }

          // Block this event
          return;
        }

        case 0xE6863C47: // "System::InitSimulation"
        {
          // Find the terrain type list box
          ICListBox *listBox = IFace::Find<ICListBox>("TerrainTypeList", config);

          if (listBox)
          {
            // Clear the list
            listBox->DeleteAllItems();

            // Add each current type
            for (U32 i = 0; i < TerrainGroup::ActiveTypeCount(); i++)
            {
              listBox->AddTextItem(TerrainGroup::GetActiveType((U8)i).Name(), NULL);
            }
          }

          // Always pass system events down
          break;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Is mouse over the terrain
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            // Should we select the terrain type
            if (varDrawTexture->GetIntegerValue())
            {
              // Get the cell at this location
              TerrainData::Cell &cell = TerrainData::GetCell
              (
                data.cInfo.mTerrain.cellX, data.cInfo.mTerrain.cellZ
              );

              // Get the terrain type
              TerrainGroup::Type &type = TerrainGroup::GetActiveType(cell.terrain);

              // Set the string name which in turn will update the index
              varTerrainType->SetStringValue(type.Name());
            }

            // Should we select the height
            if (varDrawHeight->GetIntegerValue())
            {
              varHeight->SetFloatValue
              (
                ::TerrainData::FindFloor(data.cInfo.mTerrain.pos.x, data.cInfo.mTerrain.pos.z)
              );
            }
          }

          // Block this event
          return;
        }

        case 0x14BFCAFA: // "Command::Up"
        {
          variation--;
          adjustCells = TRUE;

          // Block this event
          return;
        }

        case 0x0EDB94AC: // "Command::Down"
        {
          variation++;
          adjustCells = TRUE;

          // Block this event
          return;
        }

        case 0x5B9666F9: // "Command::Left"
        {
          if (++direction == TerrainData::BD_MAXIMUM)
          {
            direction = TerrainData::BD_NORTH;
          }
          adjustCells = TRUE;

          // Block this event
          return;
        }

        case 0xACD276E5: // "Command::Right"
        {
          if (direction == TerrainData::BD_NORTH)
          {
            direction = TerrainData::BD_WEST;
          }
          else
          {
            direction--;
          }
          adjustCells = TRUE;

          // Block this event
          return;
        }

        case 0xDA2656D6: // "Command::Execute"
        {
          // Activate cell adjustment
          adjustCells = TRUE;

          // Block this event
          return;
        }

        case 0x44942217: // "Command::BlendMap"
        {
          // Blend this map
          TerrainData::BlendEntireMap();
          
          // Block this event
          return;
        }

        case 0x78992ED8: // "Command::UpdateSmoothedTerrain"
        {
          U32 t = Clock::Time::Ms();

          // Update smoothed heights
          Movement::UpdateSmoothedTerrain();

          CON_MSG(("%d ms", Clock::Time::Ms() - t))

          // Realign all objects on the map
          for (NList<MapObj>::Iterator i(&MapObjCtrl::GetOnMapList()); *i; i++)
          {
            UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(*i);

            if (unitObj && unitObj->CanEverMove())
            {
              Matrix m = unitObj->WorldMatrix();
              unitObj->GetDriver()->AlignObject(m.front, m);
              unitObj->SetSimCurrent(m);
            }
          }   

          // Block this event
          return;
        }

        case 0x8C6EE5DF: // "System::PostEventPoll"
        {
          // Adjust each cell
          if (adjustCells)
          {
            // Add history item
            History::Terrain *history = new History::Terrain();
            History::Base::AddItem(history);
            history->Add(brushPoint0, brushPoint1);

            for (CellList::Iterator i(&cellList); *i; i++)
            {
              if (WorldCtrl::CellOnMap((*i)->pos.x, (*i)->pos.z))
              {
                TerrainData::AdjustCell
                (
                  (*i)->pos.x, (*i)->pos.z, (TerrainData::BlendDir)direction, variation
                );
              }
            }

            adjustCells = FALSE;
          }

          // Always pass system events down
          break;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          // Should we display the terrain brush
          if (varDrawHeight->GetIntegerValue() || varDrawTexture->GetIntegerValue())
          {
            // Are the points valid
            if (brushSetup)
            {
              F32 r, g, b;

              // Are we resizing the brush
              if (Common::Input::GetModifierKey(2) && captureApply == CM_OFF)
              {
                r = g = b = 1.0F;
              }
              else
              {
                r = varDrawHeight->GetIntegerValue() ? 1.0F : 0.0F;
                g = varDrawTexture->GetIntegerValue() ? 1.0F : 0.0F;
                b = 0.0F;
              }

              // Draw the brush
              ::Terrain::RenderCellRect
              (
                clipRect, Color(r, g, b, Common::Input::GetModifierKey(3) ? 0.5F : 1.0F), TRUE, TRUE
              );
            }
          }

          // Always pass system events down
          break;
        }

        /*
        case 0x8B9FFA39: // "System::PostDraw"
        {
          // Is the cursor over the terrain
          if (brushSetup && data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            Font *font = FontSys::GetFont(0xF81D1051); // "System"
          
            if (font)
            {
              IFace::RenderF32
              (
                data.cInfo.mTerrain.pos.y, 2, font, Color(1.0F, 1.0F, 1.0F),
                data.cInfo.mouse.x + 20, data.cInfo.mouse.y - 20
              );
            }
          }

          // Always pass system events down
          break;
        }
        */
      }

      // Not blocked at this level
      ApplyCell::Notification(crc, e);
    }
  }
}
