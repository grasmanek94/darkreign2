///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Events
//
// 19-AUG-1998
//


#ifdef DEVELOPMENT


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "client_private.h"
#include "physicsctrl.h"
#include "worldctrl.h"
#include "team.h"
#include "promote.h"
#include "unitobj.h"
#include "common.h"
#include "weapon.h"
#include "movement_pathfollow.h"


namespace Client
{ 
  namespace Debug
  {
    // Used to display info on the currently selected objects
    MonoBufDef(monoSelectedList);
    MonoBufDef(monoClientInfo);
    MonoBufDef(monoClientInfo2);


    //
    // Render
    //
    // General client side debugging
    //
    void Render()
    {
      #ifndef MONO_DISABLED

      S32 rem = monoSelectedList->Height() - 1;
      char buff[128];
      char *s = buff;
      U32 n;
      U32 y = 0;

      #endif

      Common::Display::RenderDebug();

      UnitObj *mouseOver = MouseOver() ? Promote::Object<UnitObjType, UnitObj>(MouseOver()) : NULL;

      data.sList.PurgeDead();

      for (UnitObjList::Iterator o(&data.sList); *o; o++)
      {
        UnitObj *u = **o;

        // If moused over unit is in list, then don't need to render it
        if (u == mouseOver)
        {
          mouseOver = NULL;
        }

        // Unit specific debug rendering
        u->RenderDebug();

        #ifndef MONO_DISABLED

        if (rem > 0)
        {
          rem--;
          y++;

          if (WorldCtrl::MetreOnMap(u->WorldMatrix().posit.x, u->WorldMatrix().posit.z))
          {
            // Move pointer back to start of buffer
            s = buff;

            // Add the ID      
            Utils::StrFmtDec(s, 6, u->Id());
            *(s += 6)++ = ' ';

            // Add the Type Name
            const char *p = u->MapType()->GetName();
            n = 0;
            while (n++ < 12)
            {
              *s++ = *p ? *p++ : ' ';
            }
            *s++ = ' ';

            // Add the Team Name
            p = u->GetTeam() ? u->GetTeam()->GetName() : "None";
            n = 0;
            while (n++ < 7)
            {
              *s++ = *p ? *p++ : ' ';
            }
            *s++ = ' ';

            // Add the Hit Points
            Utils::StrFmtDec(s, 5, u->GetHitPoints());
            *(s += 5)++ = ' ';

            // Get the position matrix
            const Vector &pos = u->Position();

            // Add the locaton in metres
            Utils::StrFmtFloat(s, 7, 2, pos.x);
            *(s += 7)++ = ',';
            Utils::StrFmtFloat(s, 7, 2, pos.z);
            *(s += 7)++ = ' ';

            // Add the location in Cells
            Utils::StrFmtDec(s, 3, WorldCtrl::MetresToCellX(pos.x));
            *(s += 3)++ = ',';
            Utils::StrFmtDec(s, 3, WorldCtrl::MetresToCellZ(pos.z));
            *(s += 3)++ = ' ';

            // Add the location in Clusters
            Utils::StrFmtDec(s, 3, WorldCtrl::MetresToClusterX(pos.x));
            *(s += 3)++ = ',';
            Utils::StrFmtDec(s, 3, WorldCtrl::MetresToClusterZ(pos.z));
            *(s += 3)++ = '\0';

            // Write it to the mono
            MonoBufWrite(monoSelectedList, y, 0, buff, Mono::NORMAL);
          }
        }

        #endif

      }

      // Render mouse over if it wasnt selected
      if (mouseOver)
      {
        mouseOver->RenderDebug();
      }

      #ifndef MONO_DISABLED

      MapObj *first = FirstSelected();

      if (first)
      {
        UnitObj *firstUnit = Promote::Object<UnitObjType, UnitObj>(first);

        if (firstUnit)
        {
          if (firstUnit->CanEverMove())
          {
            firstUnit->GetDriver()->DisplayMono(monoClientInfo2);
          }

          if (firstUnit->UnitType()->CanBoard())
          {
            //firstUnit->GetBoardManager()->DisplayMono(monoClientInfo2);
          }

          if (firstUnit->GetWeapon())
          {
            firstUnit->GetWeapon()->DisplayMono(monoClientInfo2);
          }
        }
      }

      while (rem-- >= 0)
      {
        MonoBufFillRow(monoSelectedList, y++, ' ', Mono::NORMAL);
      }

      CycleInfo &i = data.cInfo;
      S32 col = 22, row = 0;

      // Non-cycle info
      MonoBufWriteV(monoClientInfo, (row++, col, "%-5d ", data.clientMode));
      MonoBufWriteV(monoClientInfo, (row++, col, "%-5s", data.captureHandler ? "Yes" : "No"));
      MonoBufWriteV(monoClientInfo, (row++, col, "%d / %d     ", i.displayListCount, MapObjCtrl::GetOnMapList().GetCount()));

      // Are we over a game window (ie. is info valid)
      if (i.gameWnd.Alive())
      {
        MonoBufWriteV(monoClientInfo, (row++, col, "%-30s", i.gameWnd->Name()));
        MonoBufWriteV(monoClientInfo, (row++, col, "%-3d", i.pEvent));
        MonoBufWriteV(monoClientInfo, (row++, col, "%d, %d       ", i.mouse.x, i.mouse.y));

        // Is the mouse over the terrain
        if (i.t.cell)
        {
          MoveTable::KeyInfo *info = MoveTable::FindSurfaceInfo(i.t.dataCell->surface);
          MonoBufWriteV(monoClientInfo, (row++, col, "%.2f, %.2f, %.2f       ", i.t.pos.x, i.t.pos.y, i.t.pos.z));
          MonoBufWriteV(monoClientInfo, (row++, col, "%03u, %03u : %-20s [%03u, %03u]", 
            i.t.cellX, i.t.cellZ, info ? info->ident.str : "Error",
            WorldCtrl::CellsToClusterX(i.t.cellX), WorldCtrl::CellsToClusterZ(i.t.cellZ)));

          if (first)
          {
            MoveTable::BalanceData &d = MoveTable::GetBalance(i.t.dataCell->surface, first->MapType()->GetTractionIndex(first->MapType()->GetDefaultLayer()));
            MonoBufWriteV(monoClientInfo, (row++, col, "%02u/%02u, %.2f (%d,%d)", i.t.dataCell->slope, d.slope, d.speed, i.t.seen, i.t.visible));
          }
          else
          {
            row++;
          }
        }
        else
        {
          row += 3;
        }
     
        // Skip separator
        //row++;
        // Camera position
        const Vector &camPos = Vid::CurCamera().WorldMatrix().Position();
        MonoBufWriteV(monoClientInfo, (row++, col, "%.2f, %.2f, %.2f       ", camPos.x, camPos.y, camPos.z));

        // Display for first selected unit, or mouse over if none
        MapObj *mapObj = FirstSelected();
        
        if (!mapObj && i.o.map.Alive())
        {
          mapObj = i.o.map;
        }

        if (mapObj && WorldCtrl::MetreOnMap(mapObj->WorldMatrix().posit.x, mapObj->WorldMatrix().posit.z))
        {
          // Promote to a unit
          UnitObj *unitObj = mapObj ? Promote::Object<UnitObjType, UnitObj>(mapObj) : NULL;

          // Id
          MonoBufWriteV(monoClientInfo, (row++, col, "%-9d", mapObj->Id()));

          // Type
          MonoBufWriteV(monoClientInfo, (row++, col, "%-30s", mapObj->TypeName()));

          // Team
          MonoBufWriteV(monoClientInfo, (row++, col, "%-20s", unitObj ? unitObj->GetTeam() ? unitObj->GetTeam()->GetName() : "None" : "Not a Unit"));
      
          // Hitpoints
          MonoBufWriteV(monoClientInfo, (row++, col, "%9d [%9d] of %9d [%9d]", mapObj->GetHitPoints(), mapObj->GetArmour(), mapObj->MapType()->GetHitPoints(), mapObj->MapType()->GetArmour()));

          // Armour Class
          MonoBufWriteV(monoClientInfo, (row++, col, "%-20s", ArmourClass::Id2ArmourClassName(mapObj->MapType()->GetArmourClass())));

          // Weapons
          if (unitObj && unitObj->GetWeapon())
          {
            MonoBufWriteV(monoClientInfo, (row++, col, "%-25s [%10s]", unitObj->GetWeapon()->GetType().GetName(), unitObj->GetWeapon()->GetInfo()));
          }
          else
          {
            MonoBufWriteV(monoClientInfo, (row++, col, "%-40s", "None"));
          }

          // Location
          const Vector &pos = mapObj->Position();

          // Add the locaton in metres
          MonoBufWriteV(monoClientInfo, (row++, col, "%7.2f, %7.2f", pos.x, pos.z));

          // Add the location in Cells
          MonoBufWriteV(monoClientInfo, (row++, col, "%7d, %7d", mapObj->GetCellX(), mapObj->GetCellZ()));

          // Add the location in Clusters
          MonoBufWriteV(monoClientInfo, (row++, col, "%7d, %7d", WorldCtrl::MetresToClusterX(pos.x), WorldCtrl::MetresToClusterZ(pos.z)));

          // Speed
          if (unitObj)
          {
            const MoveTable::BalanceData &balance = unitObj->GetBalanceData();
            MonoBufWriteV(monoClientInfo, (row, col, "%6.1fkm/h [%-20s %5.1f%%]", mapObj->GetSpeed() * PhysicsConst::MPS2KMH, unitObj->UnitType()->GetTractionType(unitObj->GetCurrentLayer()), balance.speed * 100.0F));
          }
          row++;

          // Resources
          if (unitObj)
          {
            MonoBufWriteV(monoClientInfo, (row, col, "Transport %d [%d]", unitObj->GetResourceTransport(), unitObj->UnitType()->GetResourceTransport()));
          }
          row++;

          // Tasks
          s = buff;
          Utils::StrFmtDec(s, 3, mapObj->GetTaskList().GetCount());
          *(s += 3)++ = ' ';
          *s = '\0';

          for (NList<Task>::Iterator t(&mapObj->GetTaskList()); *t; t++)
          {
            Utils::Strcat(s, (*t)->GetName());
            Utils::Strcat(s, " [");
            Utils::Strcat(s, (*t)->Info());
            Utils::Strcat(s, "] ");

            char buff[5];
            Utils::Sprintf(buff, 5, "%04X", (*t)->GetFlags());
            Utils::Strcat(s, buff);

            if (!t.IsTail())
            {
              Utils::Strcat(s, ", ");
            }

            if (Utils::Strlen(s) > 60)
            {
              break;
            }
          }
          MonoBufWriteV(monoClientInfo, (row++, col, "%-50s", buff));

          // Idle Task
          if (mapObj->GetIdleTask())
          {
            MonoBufWriteV(monoClientInfo, (row++, col, "%s [%s] %04X", mapObj->GetIdleTask()->GetName(), mapObj->GetIdleTask()->Info(), mapObj->GetIdleTask()->GetFlags()));
          }
          else
          {
            MonoBufWriteV(monoClientInfo, (row++, col, "None"));
          }

          // Tactical
          if (unitObj)
          {
            s = buff;
            *s = '\0';

            for (U8 mod = 0; mod < Tactical::GetNumModifiers(); mod++)
            {
              Utils::Strcat(s, Tactical::GetModifierName(mod));
              Utils::Strcat(s, " [");
              Utils::Strcat(s, Tactical::GetSettingName(mod, unitObj->settings.Get(mod)));
              Utils::Strcat(s, "]");
              if (mod < Tactical::GetNumModifiers() - 1)
              {
                Utils::Strcat(s, ", ");
              }
              if (Utils::Strlen(s) > 60)
              {
                break;
              }
            }
            MonoBufWriteV(monoClientInfo, (row, col, "%-50s", buff));
          }
          row++;
        }
        else
        {
          MonoBufWriteV(monoClientInfo, (row++, col, "Nought"));
        } 
      }
      else
      {
        MonoBufWriteV(monoClientInfo, (row++, col, "* None *          "));
      }

      #endif
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      // Create Selected Objects Mono Buffer
      MonoBufCreate("Selected Objects", &monoSelectedList);

      // Write the titles onto the mono
      MonoBufWrite(monoSelectedList, 0, 0, "    Id Type         Team    State      HP           Metre    Cell Cluster", Mono::BRIGHT);
      //                             000000 012345678912 0123456 0123456 00000 0000.00,0000.00 000,000 000,000

      // Create Mouse Over Mono Buffer
      MonoBufCreate("Cycle Info", &monoClientInfo);
      MonoBufCreate("Cycle Info 2", &monoClientInfo2);

      // Write the titles onto the mono
      S32 row = 0;

      MonoBufWrite(monoClientInfo, row++, 0, "Client Mode         ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Capture Handler     ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Display List        ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "GameWindow          ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "PrimaryEvent        ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Mouse (Screen)      ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Terrain Position    ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Cell Position       ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Balance             ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Camera Position     ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Id                  ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Type                ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Team                ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "HitPoints           ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Armour Class        ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Weapon              ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Location (metres)   ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Location (cells)    ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Location (clusters) ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Speed               ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Resources           ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Tasks               ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Idle Task           ", Mono::BRIGHT);
      MonoBufWrite(monoClientInfo, row++, 0, "Tactical Modifiers  ", Mono::BRIGHT);
      //                                      01234567890123456789

      // Allow weapons to setup their portion of the mono display
      Weapon::Object::SetupMono(monoClientInfo2);
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      // Destroy Selected Objects Mono Buffer
      MonoBufDestroy(&monoSelectedList);

      // Destroy Mouse Over Mono Buffer
      MonoBufDestroy(&monoClientInfo);
      MonoBufDestroy(&monoClientInfo2);
    }
  }
}

#endif
