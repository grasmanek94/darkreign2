///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 20-JAN-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_brush_overlay.h"
#include "studio_history_terrain.h"
#include "input.h"
#include "terraindata.h"
#include "terraingroup.h"
#include "iface.h"
#include "iclistbox.h"
#include "icbutton.h"
#include "icstatic.h"
#include "common.h"
#include "fontsys.h"
#include "iface_util.h"
#include "terrain.h"
#include "terrain_priv.h"

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
    // Class Overlay - Brush for standard terrain editing
    //

    //
    // Constructor
    //
    Overlay::Overlay(const char *name) : ApplyCell(name)
    {
      captureModifier = CM_OFF;

      bInfo.overlayIndex = 0;
      bInfo.textureIndex = 0;
      bInfo.style = 0;
      bInfo.rate = 1;

      varOverlay = CreateString("overlay", "4x4");
      varTexture = CreateString("texture", "texture");
      varBlend   = CreateString("blend",   "modulate");
      varRate    = CreateFloat("rate", 1);

      VarSys::SetFloatRange( "rate", 0, 8);

      changed = FALSE;
    }


    //
    // Destructor
    //
    Overlay::~Overlay()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Overlay::HasProperty(U32 property)
    {
      return (ApplyCell::HasProperty(property));
    }

  
    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Overlay::NotifyVar(IFaceVar *var)
    {
      // Set new brush type
      if (var == varOverlay)
      {
        GameIdent buff1 = var->GetStringValue();

        // find the new overlay index
        //
        for (U32 i = 0; i < Terrain::overlays.count; i++)
        {
          Terrain::Overlay & overlay = Terrain::overlays[i];
          GameIdent buff;
          overlay.GetName( buff);
          if (!Utils::Stricmp( buff.str, buff1.str))
          {
            break;
          }
        }
        if (i < Terrain::overlays.count)
        {
          bInfo.overlayIndex = i;

          Terrain::Overlay & overlay = Terrain::overlays[i];

          bInfo.style = overlay.style;

          SetSize( overlay.size.x, overlay.size.z);
        }
      }
      else if (var == varTexture)
      {
        // find the new texture index
        S32 index = S32_MAX;
        GameIdent buff = var->GetStringValue();
        sscanf( buff.str, "%d", &index);

        if (index >= 0 && index < S32(Terrain::overlayCount))
        {
          bInfo.textureIndex = index;
        }
      }
      else if (var == varBlend)
      {
        bInfo.blend = Effects::Blend::GetValue( var->GetStringValue(), bInfo.blend);
      }
      else if (var == varRate)
      {
        bInfo.rate = varRate->GetFloatValue();
      }

      // Pass notification down
      ApplyCell::NotifyVar(var);
    }

    void Overlay::FillLists( Bool doTextures) //= TRUE
    {
      // Find the terrain type list box
      ICListBox * listBox = IFace::Find<ICListBox>("BlendList", config);

      if (listBox)
      {
        // Clear the list
        listBox->DeleteAllItems();

        static char * blends[] = {
          "add",
          "decal",
          "glow",
          "modulate",
          NULL
        };

        for (char ** b = blends; *b; b++)
        {
          listBox->AddTextItem( *b, NULL);
        }
      }

      // Find the terrain type list box
      listBox = IFace::Find<ICListBox>("OverlayList", config);

      if (listBox)
      {
        // Clear the list
        listBox->DeleteAllItems();

        for (U32 i = 0; i < Terrain::overlays.count; i++)
        {
          Terrain::Overlay & overlay = Terrain::overlays[i];
          GameIdent buff;
          overlay.GetName( buff);
          listBox->AddTextItem( buff.str, NULL);
        }
//        listBox->SetSelectedItem( bInfo.overlayIndex, TRUE, FALSE);
      }

      if (doTextures)
      {
        listBox = IFace::Find<ICListBox>("OverlayTextureList", config);

        if (listBox)
        {
          // Clear the list
          listBox->DeleteAllItems();

          // Add each current type
          for (U32 i = 0; i < Terrain::overlayCount; i++)
          {
            GameIdent buff;
            sprintf( buff.str, "%3d", i);
            listBox->AddTextItem( buff.str, NULL);
          }
//          listBox->SetSelectedItem( bInfo.textureIndex, TRUE, FALSE);
        }
      }
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Overlay::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x27546BF9: // "Brush::LeftMouseClick"
        {
          // Block this event
          return;
        }
        case 0xFEA6C563: // "Brush::LeftMouseCaptureStart"
        {
          if (Common::Input::GetModifierKey(1))
          {
            // modifier active
            captureModifier = CM_ON;

            bInfo.saveStyle = bInfo.style;
//            SaveMousePosition(e);
          }
          // Always pass system events down
          break;
        }

        case 0x252BA28C: // "Brush::LeftMouseCaptureEnd"
        {
          // Were brush points setup this cycle
          if (changed && brushSetup && !Common::Input::GetModifierKey(3))
          {
            // Add history item
            History::Terrain * history = new History::Terrain();
            History::Base::AddItem(history);
            history->Add(brushPoint0, brushPoint1);

            Point<S32> a = brushPoint0;
            Point<S32> b = brushPoint1;

            // Ensure sorted points
            if (a.x > b.x) { Swap(a.x, b.x); }
            if (a.z > b.z) { Swap(a.z, b.z); }  

            Point<S32> size( b.x - a.x + 1, b.z - a.z + 1);
            bInfo.overlayIndex = Terrain::AddOverlay( size, bInfo.style, bInfo.blend);
            Terrain::ApplyOverlay( a.x, a.z, bInfo.overlayIndex, bInfo.textureIndex);

            changed = FALSE;
          }

          if (captureModifier)
          {
            RestoreMousePosition();
          }

          // clear modifier capture
          captureModifier = CM_OFF;

          FillLists( FALSE);

          // Always pass system events down
          break;
        }

        case 0xFA75C2F5: // "Input::MouseMove"
        {
          ASSERT(e);

          if (captureModifier)
          {
            // Get mouse delta
            S32 dx = DeltaX();
            if (dx < 0)
            {
              // its circular, keep it positive
              dx %= 360;
              dx += 360;
            }
            bInfo.style = (bInfo.saveStyle + dx);

            // make it a multiple of 4 degrees
            bInfo.style >>= 2;
            bInfo.style <<= 2;
            
            bInfo.style %= 360;

            // Block this event
            return;
          }

          // Always pass system events down
          break;
        }

        case 0x0FE20111: // "Brush::Apply"
        { 
          // Do not update if key held or not over terrain
          if (captureModifier)
          {
            captureApply = CM_OFF;
            return;
          }
          if (!(data.cInfo.gameWindow && data.cInfo.mTerrain.cell))
          {
            return;
          }

         // Were brush points setup this cycle
          if (brushSetup && Common::Input::GetModifierKey(3))
          {
            // Add history
            History::Terrain *history = History::Terrain::GetItem();
            history->Add(brushPoint0, brushPoint1);
            Point<S32> a = brushPoint0;
            Point<S32> b = brushPoint1;

            // Ensure sorted points
            if (a.x > b.x) { Swap(a.x, b.x); }
            if (a.z > b.z) { Swap(a.z, b.z); }  

            Point<S32> size( b.x - a.x + 1, b.z - a.z + 1);
            Terrain::RemoveOverlay( a.x, a.z, size);
          }
          else
          { 
            changed = TRUE;
          }
          // Block this event
          return;
        }

        case 0xC9295036: // "Command::MissionSaved"
        case 0xE6863C47: // "System::InitSimulation"
        {
          FillLists();
          // Always pass system events down
          break;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Is mouse over the terrain
          if (data.cInfo.gameWindow && data.cInfo.mTerrain.cell)
          {
            // Get the cell at this location
            ::Cell & cell = *Terrain::GetCell
            (
              data.cInfo.mTerrain.cellX, data.cInfo.mTerrain.cellZ
            );
            if (cell.flags & ::Cell::cellOVERLAY)
            {
              // set the brush from the cell
              //
              Terrain::Overlay & overlay = Terrain::overlays[cell.overlay];

              GameIdent buff;
              overlay.GetName( buff);
              varOverlay->SetStringValue( buff.str);

              sprintf( buff.str, "%3d", cell.texture1);
              varTexture->SetStringValue( buff.str);

              Effects::Blend::GetString( overlay.blend, buff);
              varBlend->SetStringValue( buff.str);
            }
          }
          // Block this event
          return;
        }

        case 0x14BFCAFA: // "Command::Up"
        {
          bInfo.style = 0;

          // Block this event
          return;
        }

        case 0x0EDB94AC: // "Command::Down"
        {
          // rotate to 180 degrees
          //
          bInfo.style = 180;

          // Block this event
          return;
        }

        case 0x5B9666F9: // "Command::Left"
        {
          // rotate to 270 degrees
          //
          bInfo.style = 270;

          // Block this event
          return;
        }

        case 0xACD276E5: // "Command::Right"
        {
          // rotate to 90 degrees
          //
          bInfo.style = 90;

          // Block this event
          return;
        }

        case 0xDA2656D6: // "Command::Execute"
        {
          // Block this event
          return;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          // Are the points valid
          if (brushSetup)
          {
            F32 r, g, b, a = 1;

            Bool mod = Common::Input::GetModifierKey(3) && !(Common::Input::GetModifierKey(1) || Common::Input::GetModifierKey(2));

            // Are we resizing the brush
            if (Common::Input::GetModifierKey(2) || captureResize == CM_ON)
            {
              r = 1.0F;
              g = b = 0;
              a = Common::Input::GetModifierKey(3) ? .5f : 1; 
            }
            else if (mod)
            {
              r = g = 0;
              b = 1.0f;
            }
            else
            {
              r = b = Common::Input::GetModifierKey(1) || captureModifier == CM_ON ? 0 : 1.0f;
              g = 1.0f;
            }

            // Draw the brush
            Terrain::RenderCellRect
            (
              clipRect, Color(r, g, b, a), 
              TRUE, FALSE, mod, mod ? NULL : Terrain::overlayList[bInfo.textureIndex], 
              mod ? 0 : F32(bInfo.style) / 180.0f * PI
            );
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
