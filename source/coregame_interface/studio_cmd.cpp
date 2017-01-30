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

#include "studio_private.h"
#include "console.h"
#include "game.h"
#include "worldctrl.h"

#include "promote.h"
#include "unitobj.h"
#include "missions.h"
#include "common_mapwindow.h"
#include "iface.h"
#include "coregame.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Cmd - Command handler
  //
  namespace Cmd
  {
    // System initialized flag
    static Bool studioInit = FALSE;

    // Sim initialized flag
    static Bool simInit = FALSE;

    //
    // CmdHandler
    //
    // Handles commands 
    //
    static void CmdHandler(U32 pathCrc)
    {
      switch (pathCrc)
      {
        case 0x7EBB01DD: // "studio.quicksave"
        {
          if (Missions::GetActive())
          { 
            // Always save designer missions as text for now
            CoreGame::SetTextSave();

            if (Game::SaveMission())
            {
              // Save the minimap
              Common::MapWindow::Save(Missions::GetWritePath());

              // In the studio, set the selected mission as active
              Missions::SetActive(Missions::GetSelected());

              data.brush->Notify(0xC9295036);  // "Command::MissionSaved"

              CON_MSG(("Mission saved [%s]", Missions::GetWritePath()));
            }
            else
            {
              CON_ERR(("QuickSave FAILED!!"))
            }
          }
          else
          {
            IFace::Activate("StudioTool::MissionSave");
          }
          break;
        }

        case 0x8B558FA8: // "studio.usebrush"
        {
          const char *brushName;
          if (Console::GetArgString(1, brushName))
          {
            Event::UseBrush(brushName);
          }
          break;
        }

        case 0x41675FCF: // "studio.undo"
        {
          if (!History::Base::UndoItem())
          {
            CON_MSG(("No more items to undo"))
          }
          break;
        }

        case 0x51493015: // "studio.redo"
        {
          if (!History::Base::RedoItem())
          {
            CON_MSG(("No items to redo, or redo unavailable"))
          }
          break;
        }

        case 0xBD7D751C: // "studio.brushevent"
        {
          const char *command;

          // Generate the command
          if (Console::GetArgString(1, command))
          {
            data.brush->Notify(command);
          }
          break;
        }

        case 0xA96D6512: // "studio.settactical"
        {
          const char *modifier;
          const char *setting;

          // Read the modifier and the settings
          if 
          (
            Console::GetArgString(1, modifier) && 
            Console::GetArgString(2, setting)
          )
          { 
            U8 modifierIndex;
            if (Tactical::FindModifier(Crc::CalcStr(modifier), &modifierIndex))
            {
              U8 settingIndex;
              if (Tactical::FindSetting(modifierIndex, Crc::CalcStr(setting), &settingIndex))
              {
                // Apply these tactical settings to the selected units
                for (MapObjList::Iterator i(&data.sList); *i; i++)
                {
                  UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(**i);
                  if (unit)
                  {
                    unit->settings.Set(modifierIndex, settingIndex);
                  }
                }
              }
              else
              {
                CON_ERR(("Could not find setting '%s' in modifier '%s'", setting, modifier))
              }
            }
            else
            {
              CON_ERR(("Could not find modifier '%s'", modifier))
            }
          }
          else
          {
            CON_ERR(("studio.settactical modifier setting"))
          }
          break;
        }

        case 0xA0085AF6: // "studio.importbitmap"
        {
          char *fileName;

          if (Console::GetArgString(1, fileName))
          {
            Bitmap map;
            F32 scale = 0.4F;
            Console::GetArgFloat(2, scale);

            if (map.Read(fileName))
            {
              if (map.Depth() == 8)
              {
                F32 xMap = F32(map.Width()) / F32(Terrain::CellWidth() + 1);
                F32 zMap = F32(map.Height()) / F32(Terrain::CellHeight() + 1);

                TerrainData::SessionStart();

                for (U32 z = 0; z < Terrain::CellHeight() + 1; z++)
                {
                  for (U32 x = 0; x < Terrain::CellWidth() + 1; x++)
                  {
                    F32 h = F32(map.GetPixel(S32(F32(x) * xMap), S32(F32(z) * zMap))) * scale;

                    if (WorldCtrl::CellOnMap(x, z))
                    {
                      TerrainData::SessionModifyHeight(x, z, h);
                    }
                    else
                    {
                      ASSERT(Terrain::CellOnMap(x, z))

                      Terrain::GetCell(x, z)->height = h;               
                    }
                  }
                }

                TerrainData::SessionEnd();
              }
              else
              {
                CON_ERR(("Incorrect depth (requires 8 bit)"));
              }
            }
            else
            {
              CON_ERR(("Unable to read bitmap [%s]", fileName));
            }
          }
          break;
        }

        case 0x1CA323F4: // "studio.development.nerftheground"
        {
          F32 p0 = 10.0F, p1 = 100.0F, p2 = 100.0F;
          S32 add = TRUE;

          // Make the terrain a plane
          Console::GetArgFloat(1, p0);
          Console::GetArgFloat(2, p1);
          Console::GetArgFloat(3, p2);
          Console::GetArgInteger(4, add);

          F32 dx = (p2 - p1) / F32(Terrain::CellPitch());
          F32 dz = (p1 - p0) / F32(Terrain::CellHeight());

          TerrainData::SessionStart();

          for (U32 x = 0; x < Terrain::CellWidth(); x++)
          {
            for (U32 z = 0; z < Terrain::CellHeight(); z++)
            {
              F32 height = p0 + (F32(x) * dx) + (F32(z) * dz);

              if (add)
              {
                Cell *cell = Terrain::GetCell(x, z);
                height += cell->GetHeight();
              }
              TerrainData::SessionModifyHeight(x, z, height);
            }
          }

          TerrainData::SessionEnd();

          break;

        }
      }
    }


    //
    // Init
    //
    // Studio initialization
    //
    void Init()
    {
      ASSERT(!studioInit);
      ASSERT(!simInit);

      // Register command handler
      VarSys::RegisterHandler("studio", CmdHandler);
      VarSys::RegisterHandler("studio.vars", CmdHandler);
      VarSys::RegisterHandler("studio.development", CmdHandler);
 
      // Create general commands
      VarSys::CreateCmd("studio.quicksave");
      VarSys::CreateCmd("studio.usebrush");
      VarSys::CreateCmd("studio.undo");
      VarSys::CreateCmd("studio.redo");
      VarSys::CreateCmd("studio.brushevent");
      VarSys::CreateCmd("studio.settactical");
      VarSys::CreateCmd("studio.importbitmap");
      VarSys::CreateCmd("studio.development.nerftheground");

      // Set init flag
      studioInit = TRUE;
    }


    //
    // Done
    //
    // Studio shutdown
    //
    void Done()
    {
      ASSERT(!simInit);
      ASSERT(studioInit);

      // Delete the command scope
      VarSys::DeleteItem("studio");

      // Clear init flag
      studioInit = FALSE;
    }


    //
    // InitSim
    //
    // Simulation initialization
    //
    void InitSim()
    {
      ASSERT(studioInit);
      ASSERT(!simInit);
     
      // Set init flag
      simInit = TRUE;
    }


    //
    // DoneSim
    //
    // Simulation shutdown
    //
    void DoneSim()
    {
      ASSERT(studioInit);
      ASSERT(simInit);

      // Clear init flag
      simInit = FALSE;
    }
  }
}
