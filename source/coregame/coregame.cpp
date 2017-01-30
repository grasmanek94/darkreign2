///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "coregame.h"
#include "particlesystem.h"
#include "worldctrl.h"
#include "worldload.h"
#include "missions.h"
#include "sight.h"
#include "pathsearch.h"
#include "gameobjctrl.h"
#include "mapobjctrl.h"
#include "unitobjctrl.h"
#include "terrain.h"
#include "unitobj.h"
#include "team.h"
#include "physicsctrl.h"
#include "resolver.h"
#include "regionobj.h"
#include "bookmarkobj.h"
#include "tagobj.h"
#include "terraingroup.h"
#include "terraindata.h"
#include "common.h"
#include "fx.h"
#include "iface.h"
#include "gamegod.h"
#include "movement_pathfollow.h"
#include "weapon.h"
#include "connectedregion.h"
#include "savegame.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace CoreGame - Controls all core game systems
//
namespace CoreGame
{

  // 
  // Static data
  //
  static Bool initialized = FALSE;
  static Bool missionLoaded;

  // Are we simulating the game
  static Bool inSimulation;

  // Saving config
  static VarInteger binarySave;


  //
  // Prototypes
  //

  // Handles commands from the 'cg' var scope
  void CmdHandler(U32 pathCrc);


  //
  // Init
  //
  // Initialise core game systems
  //
  void Init()
  {
    ASSERT(!initialized);

    // Register command handler
    VarSys::RegisterHandler("coregame", CmdHandler);
    VarSys::RegisterHandler("coregame.tag", CmdHandler);
    VarSys::RegisterHandler("coregame.region", CmdHandler);
    VarSys::RegisterHandler("coregame.bookmark", CmdHandler);

    // Create commands
    VarSys::CreateCmd("coregame.sizeof");
    VarSys::CreateCmd("coregame.worldinfo");
    VarSys::CreateCmd("coregame.listtypes");
    VarSys::CreateCmd("coregame.terraingroups");
    VarSys::CreateCmd("coregame.listarmourclasses");
    VarSys::CreateCmd("coregame.listdamages");
    VarSys::CreateCmd("coregame.damagetable");
    VarSys::CreateCmd("coregame.movetable");
    VarSys::CreateCmd("coregame.tractiontypes");
    VarSys::CreateCmd("coregame.loadresources");

#ifdef DEVELOPMENT
    VarSys::CreateCmd("coregame.createobj");
    VarSys::CreateCmd("coregame.listobjs");
    VarSys::CreateCmd("coregame.listoffmap");
    VarSys::CreateCmd("coregame.listbyclassid");
    VarSys::CreateCmd("coregame.listclusters");
    VarSys::CreateCmd("coregame.listunits");
    VarSys::CreateCmd("coregame.removeobj");
    VarSys::CreateCmd("coregame.addobj");
#endif

    VarSys::CreateCmd("coregame.tag.create");
    VarSys::CreateCmd("coregame.tag.list");

    VarSys::CreateCmd("coregame.region.create");
    VarSys::CreateCmd("coregame.region.list");

    VarSys::CreateCmd("coregame.bookmark.create");
    VarSys::CreateCmd("coregame.bookmark.destroy");
    VarSys::CreateCmd("coregame.bookmark.list");
    VarSys::CreateCmd("coregame.bookmark.jumpto");
    VarSys::CreateCmd("coregame.bookmark.swoopto");

    VarSys::CreateInteger("coregame.savebinary", TRUE, VarSys::DEFAULT, &binarySave);

    // No mission loaded
    missionLoaded = FALSE;

    // Not in simulation by default
    inSimulation = FALSE;

    // System now initialized
    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown core game systems
  //
  void Done()
  {
    ASSERT(initialized);
    ASSERT(!missionLoaded);

    // Delete the command scope
    VarSys::DeleteItem("coregame");

    // System now shutdown
    initialized = FALSE;
  }


  //
  // OpenMission
  //
  // Loads the active mission
  //
  Bool OpenMission()
  {
    ASSERT(initialized);
    ASSERT(!missionLoaded);

    // Is there an active mission
    const Missions::Mission *mission = Missions::GetActive();

    // Particle system Post Load processing
    ParticleSystem::PostLoad();

    // Effects system processing
    FX::PostLoad();

    // Create a world
    WorldCtrl::CreateWorld();

    // Update smoothed terrain used by movement
    Movement::UpdateSmoothedTerrain();

    // Initialise line of sight
    Sight::Init(Common::InStudio());

    // Initialize CRE
    ConnectedRegion::Init();

    // Initialize resources for all units
    //UnitObjCtrl::InitializeResources();

    // Load world object file, if one exists
    if (mission)
    {
      WorldLoad::LoadObjectFile(Game::FILENAME_MISSION_OBJECTS);
      WorldCtrl::InitPlayField();
    }

    // Notify the path searching that the map is up
    PathSearch::NotifyPostMissionLoad();

    // Do post-mission load notification
    MapObjCtrl::PostLoad();
    UnitObjCtrl::PostLoad();

    // Objects may be marked for deletion, destroy them now
    GameObjCtrl::DeleteDyingObjects();

    // Notify save game of load position
    SaveGame::Notify(0xC2EFB3C7); // "Load::CoreGame::OpenMission"

    // Post load other systems
    Sight::NotifyPostMissionLoad();

    // Now have a mission loaded
    missionLoaded = TRUE;

    return (TRUE);
  }


  //
  // CloseMission
  //
  // Release an open mission if one exists
  //
  void CloseMission()
  {
    ASSERT(missionLoaded);

    // Delete all remaining game objects
    GameObjCtrl::DeleteAll();

    // Shutdown CRE
    ConnectedRegion::Done();

    // Shutdown line of sight
    Sight::Done();
      
    // Mission no longer loaded
    missionLoaded = FALSE;
  }


  //
  // SaveMission
  //
  // Save currently loaded mission
  //
  Bool SaveMission()
  {
    ASSERT(initialized);
    ASSERT(missionLoaded);

    // Save the world data
    if (!WorldCtrl::SaveWorld())
    {
      CON_ERR(("Unable to save world data"))
    }

    // Save the object data
    if (!WorldLoad::SaveObjectFile(Game::FILENAME_MISSION_OBJECTS))
    {
      CON_ERR(("Unable to save object data"))
    }

    return TRUE;
  }


  //
  // CmdHandler
  //
  // Handles commands 
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x6340FE7C: // "coregame.sizeof"
      {
        CON_DIAG(("TerrainData::Cell %5u", sizeof(TerrainData::Cell)))
        CON_DIAG(("Terrain::Cell     %5u", sizeof(Cell)))
        CON_DIAG(("MapCluster        %5u", sizeof(MapCluster)))
        CON_DIAG((""))
        CON_DIAG(("GameObjType       %5u", sizeof(GameObjType)))
        CON_DIAG(("MapObjType        %5u", sizeof(MapObjType)))
        CON_DIAG(("UnitObjType       %5u", sizeof(UnitObjType)))
        CON_DIAG(("Weapon::Type      %5u", sizeof(Weapon::Type)))
        CON_DIAG((""))
        CON_DIAG(("GameObj           %5u", sizeof(GameObj)))
        CON_DIAG(("MapObj            %5u", sizeof(MapObj)))
        CON_DIAG(("UnitObj           %5u", sizeof(UnitObj)))
        CON_DIAG(("Weapon::Object    %5u", sizeof(Weapon::Object)))
        CON_DIAG((""))
        CON_DIAG(("PathFollower      %5u", sizeof(Movement::Driver)))
        break;
      }

      case 0x0ECEDB11: // "coregame.worldinfo"
      {
        if (WorldCtrl::WorldInitialized())
        {
          CON_DIAG(("World Information"))
          CON_DIAG(("Mission name       : %s", Missions::GetActive() ? Missions::GetActive()->GetName().str : "No current mission"))
          CON_DIAG(("Cell size          : %.0f x %.0f metres", WorldCtrl::CellSize(), WorldCtrl::CellSize()))
          CON_DIAG(("Game cluster size  : %.0f x %.0f metres", WorldCtrl::ClusterSize(), WorldCtrl::ClusterSize()))
          CON_DIAG((""))
          CON_DIAG(("Map sizes :"))
          CON_DIAG(("Cells              : %d x %d", WorldCtrl::CellMapX(), WorldCtrl::CellMapZ()))
          CON_DIAG(("Game clusters      : %d x %d", WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ()))
          CON_DIAG(("Metres             : %.0f x %.0f", WorldCtrl::MetreMapX(), WorldCtrl::MetreMapZ()))
          CON_DIAG((""))
          CON_DIAG(("Objects : "))
          CON_DIAG(("GameObj            : %d", GameObjCtrl::listAll.GetCount()))
          CON_DIAG(("MapObj On-Map      : %d", MapObjCtrl::GetOnMapList().GetCount()))
          CON_DIAG(("MapObj Off-Map     : %d", MapObjCtrl::GetOffMapList().GetCount()))
        }
        else
        {
          CON_DIAG(("No world is currently loaded"))
        }
        break;
      }

      case 0x6E521A3F: // "coregame.createobj"
      {
        const char *ident;
        F32 x, y, z;

        if 
        (
          Console::GetArgString(1, ident) && 
          Console::GetArgFloat(2, x) && 
          Console::GetArgFloat(3, y) && 
          Console::GetArgFloat(4, z) 
        )
        {
          if (UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(ident))
          {
            Vector v;
            v.Set(x, y, z);

            // Create a unit at the closest available location, or return NULL
            type->SpawnClosest(v, Team::GetDisplayTeam());
          }
          else
          {
            CON_ERR(("Unable to find type [%s]", ident));
          }
        }
        else
        {
          CON_ERR((Console::ARGS));
        }
        break;
      }

      case 0x1FF6453B: // "coregame.listtypes"
      {
        for (List<GameObjType>::Iterator i(&GameObjCtrl::objTypesList); *i; i++)
        {
          CON_DIAG(("%-20s", (*i)->GetName()))
        }
        CON_DIAG(("Total: %d", GameObjCtrl::objTypesList.GetCount()))
        break;
      }

      case 0x4F87A33A: // "coregame.loadresources"
      {
        for (List<GameObjType>::Iterator i(&GameObjCtrl::objTypesList); *i; i++)
        {
          MapObjType *mapType = Promote::Type<MapObjType>(*i);

          if (mapType)
          {
            mapType->InitializeResources();
          }
        }
        break;
      }

      case 0x5ADD6E88: // "coregame.terraingroups"
      {
        for (BinTree<TerrainGroup::Group>::Iterator i(&TerrainGroup::GetGroupTree()); *i; i++)
        {
          CON_DIAG(("%-20s", (*i)->Name()))
          for (List<TerrainGroup::Type>::Iterator j(&(*i)->GetTypeList()); *j; j++)
          {
            CON_DIAG(("- %-20s Index=%d Surface=%d", (*j)->Name(), (*j)->Index(), (*j)->Surface()))
          }
        }
        break;
      }

      case 0x70319E54: // "coregame.listobjs"
      {
        CON_DIAG(("Listing all ON-MAP objects"))

        for (NList<MapObj>::Iterator i(&MapObjCtrl::GetOnMapList()); *i; i++)
        {
          UnitObj *u = Promote::Object<UnitObjType, UnitObj>(*i);

          CON_DIAG
          ((
            "%-20s : %7.3f, %7.3f, %7.3f [%u] (%s)", 
            (*i)->TypeName(), 
            (*i)->Position().x, 
            (*i)->Position().y, 
            (*i)->Position().z,
            (*i)->Id(),
            u ? (u->GetTeam() ? u->GetTeam()->GetName() : "No Team") : "Not a unit"
          ))
        }

        CON_DIAG(("Total: %d", MapObjCtrl::GetOnMapList().GetCount()))
        break;
      }

      case 0x2164BC5B: // "coregame.listoffmap"
      {
        CON_DIAG(("Listing all OFF-MAP objects"))

        for (NList<MapObj>::Iterator i(&MapObjCtrl::GetOffMapList()); *i; i++)
        {
          UnitObj *u = Promote::Object<UnitObjType, UnitObj>(*i);

          CON_DIAG
          ((
            "%-20s [%u] (%s)", 
            (*i)->TypeName(), 
            (*i)->Id(), 
            u ? (u->GetTeam() ? u->GetTeam()->GetName() : "No Team") : "Not a unit"
          ))
        }
        CON_DIAG(("Total: %d", MapObjCtrl::GetOffMapList().GetCount()))
        break;
      }

      case 0x1A0A0AE5: // "coregame.listbyclassid"
      {
        const char *filter = "GameObjType";
        Console::GetArgString(1, filter);
        U32 filterCrc = Crc::CalcStr(filter);
        U32 count = 0;
      
        for (NList<GameObj>::Iterator i(&GameObjCtrl::listAll); *i; i++)
        {
          // Does this one pass the filter
          if ((*i)->GameType()->DerivedFrom(filterCrc))
          {
            count++;
            CON_DIAG(("%-20s (%u)", (*i)->TypeName(), (*i)->Id()))
          }
        }
        CON_DIAG(("Total: %d", count))
        break;
      }

      case 0xC6B415C8: // "coregame.listclusters"
      {
        for (U32 x = 0; x < WorldCtrl::ClusterMapX(); x++)
        {
          for (U32 z = 0; z < WorldCtrl::ClusterMapZ(); z++)
          {
            MapCluster *clust = WorldCtrl::GetCluster(x, z);

            if (clust->listObjs.GetCount())
            {
              CON_DIAG(("Cluster [%3d,%3d] - %d objects", x, z, clust->listObjs.GetCount()))
            }
          }
        }
        break;
      }

      case 0x6DB2AB7E: // "coregame.listunits"
      {
        for (U32 x = 0; x < WorldCtrl::ClusterMapX(); x++)
        {
          for (U32 z = 0; z < WorldCtrl::ClusterMapZ(); z++)
          {
            MapCluster *clust = WorldCtrl::GetCluster(x, z);

            if (clust->unitList.GetCount())
            {
              CON_DIAG(("Cluster [%3d,%3d] - %d objects", x, z, clust->unitList.GetCount()))
            }
          }
        }
        break;
      }

      case 0xA4CB56F3: // "coregame.listarmourclasses"
      {
        CON_DIAG(("Armour Classes"))
        for (U32 id = 0; id < ArmourClass::NumClasses(); id++)
        {
          CON_DIAG((" - [%3d] %s", id, ArmourClass::Id2ArmourClassName(id)))
        }
        break;
      }

      case 0xDB188175: // "coregame.listdamages"
      {
        CON_DIAG(("Damages"))
        for (U32 id = 0; id < ArmourClass::NumDamages(); id++)
        {
          CON_DIAG((" - [%3d] %s", id, ArmourClass::Id2DamageName(id)))
        }
        break;
      }

      case 0x0E10B92B: // "coregame.damagetable"
      {
        CON_DIAG(("Damage Table"))
        for (U32 ac = 0; ac < ArmourClass::NumClasses(); ac++)
        {
          CON_DIAG(("[%3d] %s", ac, ArmourClass::Id2ArmourClassName(ac)))
          for (U32 id = 0; id < ArmourClass::NumDamages(); id++)
          {
            CON_DIAG((" - %5.2f [%3d] %s", F32(ArmourClass::Lookup(id, ac)) / 65536.0f, id, ArmourClass::Id2DamageName(id)))
          }
        }

        break;
      }

      case 0xF5521A60: // "coregame.movetable"
      {
        MoveTable::MoveBalanceTable &table = MoveTable::GetTable();

        for (NBinTree<MoveTable::MoveBalanceTable::KeyInfo>::Iterator t(&table.GetYTree()); *t; t++)
        {
          CON_DIAG(("[%s]", (*t)->ident.str))
        
          for (NBinTree<MoveTable::MoveBalanceTable::KeyInfo>::Iterator s(&table.GetXTree()); *s; s++)
          {
            MoveTable::BalanceData &data = MoveTable::GetBalance((*s)->index, (*t)->index);
            CON_DIAG
            ((
              "%-20s %u - %3.2f, %2u, %3.2f", 
              (*s)->ident.str, (*s)->index, data.speed, data.slope, data.health
            ))
          }       
        }
        break;
      }

      case 0x97AABAC5: // "coregame.tractiontypes"
      {
        MoveTable::MoveBalanceTable &table = MoveTable::GetTable();

        for (NBinTree<MoveTable::MoveBalanceTable::KeyInfo>::Iterator t(&table.GetYTree()); *t; t++)
        {
          CON_DIAG(("[%d] %s", MoveTable::TractionIndex((*t)->ident.str), (*t)->ident.str))
        }
        break;
      }

      case 0xCDF8F7A2: // "coregame.removeobj"
      {
        S32 id;

        if (!Console::GetArgInteger(1, id))
        {
          CON_ERR(("console.removeobj id"))
        }
        else
        {
          if (GameObj *gameObj = GameObjCtrl::FindObject(id))
          {
            MapObj *obj = Promote::Object<MapObjType, MapObj>(gameObj);

            if (obj)
            {
              if (obj->OnMap())
              {
                MapObjCtrl::RemoveFromMap(obj);
              }
            }
          }
          else
          {
            CON_ERR(("Object [%d] not found", id))
          }
        }
        break;
      }

      case 0x205D19B3: // "coregame.addobj"
      {
        S32 id;

        if (!Console::GetArgInteger(1, id))
        {
          CON_ERR(("console.addobj id"))
        }
        else
        {
          if (GameObj *gameObj = GameObjCtrl::FindObject(id))
          {
            MapObj *obj = Promote::Object<MapObjType, MapObj>(gameObj);

            if (obj)
            {
              if (!obj->OnMap())
              {
                MapObjCtrl::AddToMap(obj);
              }
            }
          }
          else
          {
            CON_ERR(("Object [%d] not found", id))
          }
        }
        break;
      }

      case 0x6CF8316F: // "coregame.tag.create"
      {
        const char *s1;

        if (Console::GetArgString(1, s1))
        {
          MapObjList mapObjList;
          int i = 2;
          S32 id;

          while (Console::GetArgInteger(i, id))
          { 
            MapObj *mapObj;
            if ((mapObj = Resolver::Object<MapObj, MapObjType>(id)) != NULL)
            {
              mapObjList.AppendNoDup(mapObj);
            }
            else
            {
              CON_ERR(("Cannot find map object with id %d", id))
            }
            i++;
          }
          
          if (!TagObj::CreateTag(s1, mapObjList))
          {
            CON_ERR(("Cound not create tag"))
          }

          mapObjList.Clear();
        }
        else
        {
          CON_ERR((Console::ARGS))       
        }
        break;
      }

      case 0xCCCACF99: // "coregame.tag.list"
      {
        const char *s1;

        if (Console::GetArgString(1, s1))
        {
          TagObj *obj = TagObj::FindTag(s1);
          if (obj)
          {
            CON_DIAG(("Listing objects in tagged group '%s' :", s1))

            for (MapObjList::Iterator i(&obj->list); *i; i++)
            {
              if ((*i)->Alive())
              {
                CON_DIAG(("%s (%u)", (**i)->TypeName(), (**i)->Id()))
              }
              else
              {
                CON_DIAG(("Dead object"))
              }
            }
          }
          else
          {
            CON_ERR(("Unable to find tag '%s'", s1))
          }
        }
        else
        {
          CON_DIAG(("Listing all current tags"))
          for (NBinTree<TagObj>::Iterator i(&TagObj::allTags); *i; i++)
          {
            (*i)->list.PurgeDead();
            CON_DIAG(("%s : %s [%u obj]", (*i)->TypeName(), (*i)->TagName(), (*i)->list.GetCount()))
          }
        }
        break;
      }

      case 0xB44BC00B: // "coregame.region.create"
      {
        const char *s1;
        Area<F32> a;

        if (!Console::GetArgString(1, s1) ||
            !Console::GetArgFloat(2, a.p0.x) ||
            !Console::GetArgFloat(3, a.p0.y) ||
            !Console::GetArgFloat(4, a.p1.x) ||
            !Console::GetArgFloat(5, a.p1.y))
        {
          CON_ERR((Console::ARGS))       
        }
        else
        {
          a.Sort();

          if (!RegionObj::CreateRegion(s1, a))
          {
            CON_ERR(("Could not create region"))
          }
        }
        break;
      }

      case 0x600201EB: // "coregame.region.list"
      {
        CON_DIAG(("Regions:"))
        for (NList<RegionObj>::Iterator i(&RegionObj::allRegions); *i; i++)
        {
          CON_DIAG(("%s : (%g,%g)-(%g,%g) (%g,%g)",
            (*i)->RegionName(), 
            (*i)->GetArea().p0.x, (*i)->GetArea().p0.y, 
            (*i)->GetArea().p1.x, (*i)->GetArea().p1.y,
            (*i)->GetMidPoint().x, (*i)->GetMidPoint().y))
        }
        break;
      }

      case 0xCA4A1FB9: // "coregame.bookmark.create"
      {
        const char *name;
        if (!Console::GetArgString(1, name))
        {
          CON_ERR(("coregame.bookmark.create bookmarkname"))
          break;
        }

        // Create the book mark
        BookmarkObj::CreateBookmark(name, BookmarkObj::POSITION);
        break;
      }

      case 0x1CB030B8: // "coregame.bookmark.destroy"
      {
        const char *name;
        if (!Console::GetArgString(1, name))
        {
          CON_ERR(("coregame.bookmark.jumpto bookmarkname"))
          break;
        }

        BookmarkObj *bookMark;

        // Find the bookmark
        if ((bookMark = BookmarkObj::FindBookmark(name)) != NULL)
        {
          // Delete it
          GameObjCtrl::MarkForDeletion(bookMark);
        }
        else
        {
          CON_ERR(("Cound not find book mark '%s'. Try 'coregame.bookmark.list'", name))
        }
        break;
      }

      case 0x07B687FA: // "coregame.bookmark.list"
      {
        // List all bookmarks
        CON_DIAG(("Bookmarks:"))
        for (NBinTree<BookmarkObj>::Iterator i(&BookmarkObj::allBookmarks); *i; i++)
        {
          CON_DIAG
          ((
            " - '%s' [x%g y%g z%g yaw %g pitch %g]",
            (*i)->GetName(),
            (*i)->GetPosition().x,
            (*i)->GetPosition().y,
            (*i)->GetPosition().z,
            (*i)->GetYaw(),
            (*i)->GetPitch()
          ))
        }
        break;
      }

      case 0x39F3783E: // "coregame.bookmark.jumpto"
      {
        const char *name;
        if (!Console::GetArgString(1, name))
        {
          CON_ERR(("coregame.bookmark.jumpto bookmarkname"))
          break;
        }

        BookmarkObj *bookMark;

        // Find the bookmark
        if ((bookMark = BookmarkObj::FindBookmark(name)) != NULL)
        {
          bookMark->JumpTo();
        }
        else
        {
          CON_ERR(("Cound not find book mark '%s'. Try 'coregame.bookmark.list'", name))
        }
        break;
      }

      case 0xA324D763: // "coregame.bookmark.swoopto"
      {
        const char *name;
        if (!Console::GetArgString(1, name))
        {
          CON_ERR(("coregame.bookmark.swoopto bookmarkname"))
          break;
        }

        BookmarkObj *bookMark;

        // Find the bookmark
        if ((bookMark = BookmarkObj::FindBookmark(name)) != NULL)
        {
          bookMark->SwoopTo();
        }
        else
        {
          CON_ERR(("Cound not find book mark '%s'. Try 'coregame.bookmark.list'", name))
        }
        break;
      }
    }
  }


  //
  // Save mission in binary format
  //
  void SetBinarySave()
  {
    binarySave = TRUE;
  }


  //
  // Save mission in text format
  //
  void SetTextSave()
  {
    binarySave = FALSE;
  }


  //
  // Is mission saving in binary
  //
  Bool IsBinarySave()
  {
    return (*binarySave);
  }


  //
  // WriteTree
  //
  // Save the given tree
  //
  Bool WriteTree(PTree &pTree, const char *name)
  {
    return 
    (
      IsBinarySave() ? pTree.WriteTreeBinary(name) : pTree.WriteTreeText(name)
    );
  }


  //
  // SetInSimulation
  //
  // Is the game being simulated
  //
  void SetInSimulation(Bool flag)
  {
    ASSERT(initialized)
    inSimulation = flag;
  }


  //
  // GetInSimulation
  //
  // Is the game being simulated
  //
  Bool GetInSimulation()
  {
    return (initialized ? inSimulation : FALSE);
  }
}
