///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Command Handler
//
// 19-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_private.h"
#include "varsys.h"
#include "input.h"
#include "gameobjctrl.h"
#include "mapobjctrl.h"
#include "console.h"
#include "game.h"
#include "tagobj.h"
#include "regionobj.h"
#include "unitobj.h"
#include "promote.h"
#include "iface.h"
#include "viewer.h"
#include "terrain.h"
#include "sight.h"
#include "sync.h"
#include "resolver.h"
#include "movement_pathfollow.h"
#include "client_clustermap.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Cmd
  //
  namespace Cmd
  {

    //
    // LogTasks
    //
    // Logs the tasks for the given unit
    // 
    static void LogTasks(MapObj *obj)
    {
      LOG_DIAG(("Tasks for [%s][%d] (%d)", obj->TypeName(), obj->Id(), obj->GetTaskList().GetCount()));

      for (NList<Task>::Iterator t(&obj->GetTaskList()); *t; t++)
      {
        LOG_DIAG((" - %s (%s)", (*t)->GetName(), (*t)->Info()));
      }     
    }


    //
    // CmdHandler
    //
    // Handles commands 
    //
    static void CmdHandler(U32 pathCrc)
    {
      S32 i1, i2, i3;
      const char *s1, *s2, *s3;

      switch (pathCrc)
      {
        case 0xF4909AFB: // "client.groupop"
        {
          if (Console::GetArgString(1, s1) && Console::GetArgInteger(2, i1))
          {
            Group::Operation op = Group::O_MAX;

            // Convert the operation name
            switch (Crc::CalcStr(s1))
            {
              case 0x07984B08: // "new"
                op = Group::O_NEWGROUP; break;
              case 0x5243C5BB: // "newremove"
                op = Group::O_NEWGROUP_REMOVE; break; 
              case 0x9FF22134: // "select"
                op = Group::O_SELECTGROUP; break;
              case 0xBC7FC0D3: // "selectscroll"
                op = Group::O_SELECTGROUP_SCROLL; break;
              case 0xC9FCFE2A: // "clear"
                op = Group::O_CLEAR; break;
            }

            // Did we recognize the operation
            if (op != Group::O_MAX)
            {
              if (!Group::DoOperation(i1 - 1, op))
              {
                CON_ERR(("Group operation failed, invalid index? (%d)", i1))
              }
            }
            else
            {
              CON_ERR(("Unknown group operation (%s)", s1))
            }
          }
          else
          {
            CON_ERR((Console::ARGS))       
          }
          break;
        }

        case 0x462D7266: // "client.event"
        {
          if (Console::GetArgString(1, s1))
          {
            U32 param1 = Console::GetArgString(2, s2) ? Crc::CalcStr(s2) : (Console::GetArgInteger(2, i2) ? i2 : 0);
            U32 param2 = Console::GetArgString(3, s3) ? Crc::CalcStr(s3) : (Console::GetArgInteger(3, i3) ? i3 : 0);
            Events::HandleDiscreteEvent(NULL, Crc::CalcStr(s1), param1, param2);
          }
          else
          {
            CON_ERR((Console::ARGS))       
          }
          break;
        }

        case 0x713D0C6B: // "client.giveunits"
        {
          if (Console::GetArgString(1, s1))
          {
            Player *player = Player::Id2Player(Utils::AtoI(s1));
            if (player)
            {
              Events::UpdateSelectedLists();
              Orders::Game::GiveUnits::Generate(GetPlayer(), player);
            }
          }
          break;
        }

        case 0x35E1FC17: // "client.giveresource"
        {
          if (Console::GetArgString(1, s1) && Console::GetArgInteger(2, i1))
          {
            Player *player = Player::Id2Player(Utils::AtoI(s1));
            if (player)
            {
              Events::UpdateSelectedLists();
              Orders::Game::GiveResource::Generate(GetPlayer(), player, i1);
            }
          }
          break;
        }

        case 0x9E2395E0: // "client.triggermode"
        {
          if (Console::GetArgString(1, s1))
          {
            Events::TriggerClientMode(s1);
          }
          break;
        }       

        case 0x627A1F63: // "client.selected"
        {
          Events::UnitsSelected();
          break;
        }
       
        case 0x1FEE7D2F: // "client.pause"
        {
          Orders::Game::Pause::Generate(GetPlayer());
          break;
        }

        case 0x97EA8F5C: // "client.steponce"
        {
          Orders::Game::StepOnce::Generate(GetPlayer());
          break;
        }

        case 0x17ECFD36: // "client.listselected"
        {
          for (UnitObjList::Iterator i(&data.sList); *i; i++)
          {
            if ((*i)->Alive())
            {
              CON_DIAG(("%s (%d)", (**i)->TypeName(), (**i)->Id()))
            }
            else
            {
              CON_DIAG(("Dead object"))
            }
          }
          break;
        }

        case 0xB5A2EE79: // "client.settactical"
        {
          if (Console::GetArgString(1, s1) && Console::GetArgString(2, s2))
          { 
            U8 m, s;

            // Get the modifier index
            if (Tactical::FindModifier(Crc::CalcStr(s1), &m))
            {
              // Get the setting index
              if (Tactical::FindSetting(m, Crc::CalcStr(s2), &s))
              {
                Events::ModifyTacticalSetting(m, s);
              }
            }
          }

          break;
        }

        case 0xEBFBDD9C: // "client.savegame"
        {
          // Set default slot
          char *slot = "Slot0";

          // Get user slot
          Console::GetArgString(1, slot);

          // Set default description
          const char *desc = Clock::GetDateAndTime();

          // Get user description
          Console::GetArgString(2, desc);

          // Attempt to save the game
          if (SaveGame::Save(slot, desc))
          {
            CON_MSG(("Game successfully saved [%s]", slot))
          }
          else
          {
            CON_ERR(("Failed saving game [%s]", slot));
          }           
          break;
        }

        case 0x7E94354D: // "client.loadgame"
        {
          // Set default slot
          char *slot = "Slot0";

          // Get user slot
          Console::GetArgString(1, slot);

          // Attempt to load the game
          if (!SaveGame::Load(slot))
          {
            CON_ERR(("Failed loading save game [%s]", slot));
          }
          break;
        }

        case 0x463AD946: // "client.viewregion"
        {
          if (!Console::GetArgString(1, s1))
          {
            CON_ERR(("client.viewregion region"))
          }
          else
          {
            RegionObj *region = RegionObj::FindRegion(s1);
            if (region)
            {
              // Set the region
              data.paintRegion = region;

              // Move to that location on the map
              Viewer::GetCurrent()->LookAt(region->GetMidPoint().x, region->GetMidPoint().y);
            }
            else
            {
              CON_ERR(("Could not find region '%s'", s1))
            }
          }
          break;
        }

        case 0xFAE1F9CE: // "client.clearview"
        {
          data.paintRegion = NULL;
          break;
        }

        case 0xB364AC40: // "client.autopilot"
        {
          char *pilot;
          if (Console::GetArgString(1, pilot))
          {
            Player::SetAutoPilot(pilot);
          }
          else
          {
            CON_ERR(("client.autopilot pilot(ai type)"))
          }
          break;
        }

        #ifdef DEVELOPMENT

        case 0x2185FED7: // "client.development.blastthosebastards"
        {
          SYNC("Development::BlastThoseBastards!")
          LOG_DIAG(("Development::BlastThoseBastards!"));

          if (data.sList.GetCount())
          {
            for (UnitObjList::Iterator i(&data.sList); *i; i++)
            {
              if ((*i)->Alive())
              {
                (**i)->SelfDestruct(TRUE);
              }
            }
          }
          else

          if (data.cInfo.gameWnd.Alive() && data.cInfo.o.map.Alive())
          {
            data.cInfo.o.map->SelfDestruct(TRUE);
          }

          break;
        }

        case 0xA4DD0F24: // "client.development.gapinganus"
          CON_ERR(("You should now use 'client.development.money' :]"));
          break;

        case 0x70FACF72: // "client.development.money"
        {
          SYNC("Development::Money!")
          LOG_DIAG(("Development::Money!"))

          Team::GetDisplayTeam()->AddResourceStore(1000000);
          Team::GetDisplayTeam()->ReportResource(1000000, "resource.development");
          break;
        }

        case 0x7B34554D: // "client.development.setteam"
        {
          const char *name;

          SYNC("Development::SetTeam!")
          LOG_DIAG(("Development::SetTeam!"))

          if (Console::GetArgString(1, name))
          {
            Team *team = Team::Name2Team(name);
            if (team)
            {
              data.sList.Clear();
              Team::SetDisplayTeam(team);
              Sight::ResetTerrainDisplay();
            }
          }
          break;
        }

        case 0x9728C5D2: // "client.development.tasks"
        {
          if (data.sList.GetCount())
          {
            for (UnitObjList::Iterator i(&data.sList); *i; i++)
            {
              if ((*i)->Alive())
              {
                LogTasks(**i);
              }
            }
          }
          else

          if (data.cInfo.gameWnd.Alive() && data.cInfo.o.map.Alive())
          {
            LogTasks(data.cInfo.o.map);
          }

          break;
        }


        case 0xF28E8E4D: // "client.development.directmodeon"
        {
          SYNC("DirectMode")

          for (UnitObjList::Iterator i(&data.sList); *i; i++)
          {
            UnitObj *u = **i;

            if (u->CanEverMove())
            {
              u->GetDriver()->SetupDirectControl(TRUE);
            }
          }

          break;
        }

        case 0x8328035B: // "client.development.directmodeoff"
        {
          SYNC("DirectMode")

          for (UnitObjList::Iterator i(&data.sList); *i; i++)
          {
            UnitObj *u = **i;

            if (u->CanEverMove())
            {
              u->GetDriver()->SetupDirectControl(FALSE);
            }
          }
          break;
        }

        case 0xA8BDFA44: // "client.development.directturn"
        {
          SYNC("DirectMode")

          if (data.cInfo.t.cell)
          {
            for (UnitObjList::Iterator i(&data.sList); *i; i++)
            {
              UnitObj *u = **i;

              if (u->CanEverMove())
              {
                Vector dir = data.cInfo.t.pos - u->Position();

                if (dir.MagnitudeXZ() > 1e-4F)
                {
                  F32 angle = F32(atan2(dir.z, dir.x));
                  LOG_DIAG(("DirectAngle = %f", angle * RAD2DEG))
                  u->GetDriver()->DirectTurn(angle);
                }             
              }
            }
          }
          break;
        }

        case 0x54872967: // "client.development.debug"
        {
          const char *type;
          F32 f = 0.0F;

          if (Console::GetArgString(1, type))
          {
            Console::GetArgFloat(2, f);

            Events::UpdateSelectedLists();
            Orders::Game::Debug::Generate
            (
              GetPlayer(), Crc::CalcStr(type), 
              data.cInfo.o.map.Alive() ? data.cInfo.o.map.Id() : 0, S32(f), f
            );
          }
        }

        #endif
      }
    }


    //
    // Init
    //
    // Initialize the client command system
    //
    void Init()
    {
      // Register command handlers
      VarSys::RegisterHandler("client", CmdHandler);
 
      // Create commands
      VarSys::CreateCmd("client.groupop");    
      VarSys::CreateCmd("client.event");
      VarSys::CreateCmd("client.triggermode");
      VarSys::CreateCmd("client.selected");

      VarSys::CreateCmd("client.giveunits");
      VarSys::CreateCmd("client.giveresource");

      VarSys::CreateCmd("client.pause");
      VarSys::CreateCmd("client.steponce");

      VarSys::CreateCmd("client.listselected");
      VarSys::CreateCmd("client.showsettings");
      VarSys::CreateCmd("client.settactical");
      
      VarSys::CreateCmd("client.savegame");
      VarSys::CreateCmd("client.loadgame");

      VarSys::CreateCmd("client.viewregion");
      VarSys::CreateCmd("client.clearview");

      VarSys::CreateCmd("client.autopilot");

      VarSys::CreateInteger("client.trackobject", TRUE, VarSys::DEFAULT, &data.trackObject);
      VarSys::CreateInteger("client.trackdistance", 40, VarSys::DEFAULT, &data.trackDistance);
      VarSys::CreateInteger("client.giveamount", 1000, VarSys::DEFAULT, &data.giveAmount);
      VarSys::CreateInteger("client.fullsquadhud", TRUE, VarSys::DEFAULT, &data.fullSquadHud);

      VarSys::CreateInteger("client.sundialdir", 1, VarSys::DEFAULT, &data.sundialDir);

      VarSys::CreateInteger("client.squad.reset", FALSE, VarSys::DEFAULT, &data.squadReset);

      // Commands for development only (ie. will get the game oos!)
      #ifdef DEVELOPMENT
        VarSys::RegisterHandler("client.development", CmdHandler);
        VarSys::CreateCmd("client.development.setteam");
        VarSys::CreateCmd("client.development.blastthosebastards");
        VarSys::CreateCmd("client.development.money");
        VarSys::CreateCmd("client.development.tasks");

        VarSys::CreateCmd("client.development.directmodeon");
        VarSys::CreateCmd("client.development.directmodeoff");
        VarSys::CreateCmd("client.development.directturn");
        VarSys::CreateCmd("client.development.debug");
      #endif

      // ClusterMap
      ClusterMap::CreateCommands();
    }


    //
    // Done
    //
    // Shutdown the client command system
    //
    void Done()
    {
      // ClusterMap
      ClusterMap::DeleteCommands();

      // Delete the command scope
      VarSys::DeleteItem("client");
    }
  }
}
