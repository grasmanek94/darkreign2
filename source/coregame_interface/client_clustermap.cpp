///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client Cluster Maps
//
// 19-AUG-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_clustermap.h"
#include "varsys.h"
#include "console.h"
#include "regionobj.h"
#include "team.h"
#include "armourclass.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define CLUSTERMAP_MAXSIZE          150
#define CLUSTERMAP_DEFAULT_REFRESH  1000

#define CLUSTERMAP_THREAT_ALL       0xFFFFFFFF
#define CLUSTERMAP_DEFENSE_ALL      0xFFFFFFFF
#define CLUSTERMAP_PAIN_ALL         0xFFFFFFFF


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ClientClusterMap
  //

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  BinTree<ClusterMap::Map> ClusterMap::maps;

  //
  // CallbackResource
  //
  Color ClusterMap::CallbackResource(void *, U32 x, U32 y)
  {
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);

    U32 value = cluster->ai.GetResource();
    if (value)
    {
      F64 fvalue = 11.090354889 * log((F64) value);

      if (fvalue < 256)
      {
        return (Color(0, (U32) fvalue, 0));
      }
      else
      {
        fvalue -= 256;
        return (Color((U32) fvalue, 255, (U32) fvalue));
      }
    }
    else
    {
      return (Color(0.0f, 0.0f, 0.0f));
    }
  }


  //
  // CallbackOccupation
  //
  Color ClusterMap::CallbackOccupation(void *context, U32 x, U32 y)
  {
    Occupation *occupation = (Occupation *) context;
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);
    U32 value;

    ASSERT(occupation)
    ASSERT(cluster)

    value = cluster->ai.GetOccupation(occupation->team);

    if (value)
    {
      F32 fval = F32(value) * AI::Map::GetOccupationInv(occupation->team);

      // Move onto a quadratic curve
      fval = 1.0f - (1.0f - fval) * (1.0f - fval);

      return (Color(fval * 0.5f, fval, fval * 0.5f));
    }
    else
    {
      return (Color(0.0f, 0.0f, 0.0f));
    }
  }


  //
  // CallbackDisruption
  //
  Color ClusterMap::CallbackDisruption(void *, U32 x, U32 y)
  {
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);
    U32 value;

    ASSERT(cluster)
    value = cluster->ai.GetDisruption();

    if (value)
    {
      F32 fval = F32(value) * AI::Map::GetDisruptionInv();
      fval *= 0.7f;
      fval += 0.3f;
      fval = 1.0f - (1.0f - fval) * (1.0f - fval);
      return (Color(0.0f, fval * 0.5f, fval));
    }
    else
    {
      return (Color(0.0f, 0.0f, 0.0f));
    }
  }


  //
  // CallbackThreat
  //
  Color ClusterMap::CallbackThreat(void *context, U32 x, U32 y)
  {
    Threat *threat = (Threat *) context;
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);
    U32 value;

    ASSERT(threat)
    ASSERT(cluster)

    if (threat->armourClass == CLUSTERMAP_THREAT_ALL)
    {
      value = AI::Map::GetTotalThreat(threat->team) ? cluster->ai.GetTotalThreat(threat->team) : 0;
    }
    else
    {
      value = AI::Map::GetThreat(threat->team, threat->armourClass) ? cluster->ai.GetThreat(threat->team, threat->armourClass) : 0;
    }

    if (value)
    {
      F64 fvalue = 16.6355323334 * log((F64) value);

      if (fvalue < 256)
      {
        return (Color((U32) fvalue, 0, 0));
      }
      else if (fvalue < 512)
      {
        return (Color(255, (U32) (fvalue - 256), 0));
      }
      else
      {
        return (Color(255, 255, (U32) (fvalue - 512)));
      }
    }
    else
    {
      return (Color(0.0f, 0.0f, 0.0f));
    }
  }


  //
  // CallBackDefense
  //
  Color ClusterMap::CallbackDefense(void *context, U32 x, U32 y)
  {
    Defense *defense = (Defense *) context;
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);
    U32 value;

    ASSERT(defense)
    ASSERT(cluster)

    if (defense->damageId == CLUSTERMAP_DEFENSE_ALL)
    {
      value = AI::Map::GetTotalDefense(defense->team) ? 
        cluster->ai.GetTotalDefense(defense->team) * 768 / AI::Map::GetTotalDefense(defense->team) : 0;
    }
    else
    {
      value = AI::Map::GetDefense(defense->team, defense->damageId) ? 
        cluster->ai.GetDefense(defense->team, defense->damageId) * 768 / AI::Map::GetDefense(defense->team, defense->damageId) : 0;
    }

    if (value < 256)
    {
      return (Color(0, 0, value));
    }
    else if (value < 512)
    {
      return (Color(0, value, 255));
    }
    else
    {
      return (Color(value, 255, 255));
    }
  }


  //
  // CallbackPain
  //
  Color ClusterMap::CallbackPain(void *context, U32 x, U32 y)
  {
    Pain *pain = (Pain *) context;
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);
    U32 value;

    ASSERT(pain)
    ASSERT(cluster)

    if (pain->armourClass == CLUSTERMAP_PAIN_ALL)
    {
      value = AI::Map::GetTotalPain(pain->team) ? cluster->ai.GetTotalPain(pain->team) : 0;
    }
    else
    {
      value = AI::Map::GetPain(pain->team, pain->armourClass) ? cluster->ai.GetPain(pain->team, pain->armourClass) : 0;
    }

    if (value)
    {
      F64 fvalue = 16.6355323334 * log((F64) value);

      if (fvalue < 256)
      {
        return (Color(0, (U32) fvalue, 0));
      }
      else if (fvalue < 512)
      {
        return (Color(0, 255, (U32) (fvalue - 256)));
      }
      else
      {
        return (Color((U32) (fvalue - 512), 255, 255));
      }
    }
    else
    {
      return (Color(0.0f, 0.0f, 0.0f));
    }
  }


  //
  // CallBackRegion
  //
  Color ClusterMap::CallbackRegion(void *context, U32 x, U32 y)
  {
    Region *region = (Region *) context;
    MapCluster *cluster = WorldCtrl::GetCluster(x, y);

    ASSERT(region)
    ASSERT(cluster)

  /*
    // If this cluster is in the list of clusters for the region then make it yellow
    if (region->region->clusters.InList(cluster))
    {
      return (Color(255, 255, 0));
    }

    // Otherwise make it black
    else */
    {
      return (Color(U32(0), U32(0), U32(0)));
    }
  }


  //
  // CreateCommads
  //
  void ClusterMap::CreateCommands()
  {
    // Register command handlers
    VarSys::RegisterHandler("client.clustermap", CmdHandler);
    VarSys::RegisterHandler("client.clustermap.ai", CmdHandler);

#ifdef DEVELOPMENT

    // ClusterMap Commands
    VarSys::CreateCmd("client.clustermap.create");

    // AI ClusterMap Commands
    VarSys::CreateCmd("client.clustermap.ai.addthreat");
    VarSys::CreateCmd("client.clustermap.ai.removethreat");
    VarSys::CreateCmd("client.clustermap.ai.adddefense");
    VarSys::CreateCmd("client.clustermap.ai.removedefense");

#endif
  }


  //
  // DeleteCommands
  //
  void ClusterMap::DeleteCommands()
  {
    // Delete the clustermap scope
    VarSys::DeleteItem("client.clustermap");
  }


  //
  // CmdHandler
  //
  void ClusterMap::CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0xBCBF9D4F: // "client.clustermap.create"
      {
        char *type;
        if (!Console::GetArgString(1, type))
        {
          CON_ERR(("client.clustermap.create type [params]"))
        }
        else
        {
          switch (Crc::CalcStr(type))
          {
            case 0x4CD1BE27: // "resource"
            {
              // Create the cluster map
              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                "Resources", WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );
            
              // Set grid callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackResource);
              g.SetPollInterval(5000);
              g.SetAxisFlip(FALSE, TRUE);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr("Resources"), new Map(grid, NULL));
              break;
            }

            case 0x6B45E01B: // "occupation"
            {
              char *teamName;

              // Test params
              if (!Console::GetArgString(2, teamName))
              {
                CON_ERR(("client.clustermap.create 'occupation' team"))
                break;
              }

              // Convert team name into a team
              Team *team = Team::Name2Team(teamName);
              if (!team)
              {
                CON_ERR(("Unknown team '%s'", teamName))
                break;
              }

              // Create the cluster map
              char name[256];
              Utils::Sprintf(name, 256, "Occupation [%s]", teamName);

              // Create the occupation
              void *context = new Occupation(team->GetId());

              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                name, WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );
            
              // Set grid callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackOccupation);
              g.SetContext(context);
              g.SetPollInterval(5000);
              g.SetAxisFlip(FALSE, TRUE);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr(name), new Map(grid, context));
              break;
            }

            case 0x6FDCD1BD: // "disruption"
            {
              // Create the cluster map
              char name[256];
              Utils::Sprintf(name, 256, "Disruption");

              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                name, WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );
            
              // Set grid callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackDisruption);
              g.SetContext(NULL);
              g.SetPollInterval(5000);
              g.SetAxisFlip(FALSE, TRUE);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr(name), new Map(grid, NULL));
              break;
            }

            case 0x98D9AF2E: // "threat"
            {
              char *teamName;
              char *armourClassName;

              // Test params
              if (!Console::GetArgString(2, teamName) || !Console::GetArgString(3, armourClassName))
              {
                CON_ERR(("client.clustermap.create 'threat' team *|armourclass"))
                break;
              }

              // Convert team name into a team
              Team *team = Team::Name2Team(teamName);
              if (!team)
              {
                CON_ERR(("Unknown team '%s'", teamName))
                break;
              }

              // Convert armour class name into an armour class
              U32 armourClass;

              if (!Utils::Strcmp(armourClassName, "*"))
              {
                armourClass = CLUSTERMAP_THREAT_ALL;
              }
              else if (ArmourClass::ArmourClassExists(armourClassName))
              {
                armourClass = ArmourClass::Name2ArmourClassId(armourClassName);
              }
              else
              {
                CON_ERR(("Unknown armour class '%s'", armourClassName))
                break;
              }

              // Create the cluster map
              char name[256];
              Utils::Sprintf(name, 256, "Threat [%s] [%s]", teamName, armourClassName);

              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              // Create the threat
              void *context = new Threat(team->GetId(), armourClass);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                name, WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );
            
              // Set grid callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackThreat);
              g.SetContext(context);
              g.SetPollInterval(5000);
              g.SetAxisFlip(FALSE, TRUE);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr(name), new Map(grid, context));
              break;
            }

            case 0x07B0615D: // "defense"
            {
              char *teamName;
              char *damageName;

              // Test params
              if (!Console::GetArgString(2, teamName) || !Console::GetArgString(3, damageName))
              {
                CON_ERR(("client.clustermap.create 'defense' team *|damage"))
                break;
              }

              // Convert team name into a team
              Team *team = Team::Name2Team(teamName);
              if (!team)
              {
                CON_ERR(("Unknown team '%s'", teamName))
                break;
              }

              // Convert damage name into a damage id
              U32 damageId;

              if (!Utils::Strcmp(damageName, "*"))
              {
                damageId = CLUSTERMAP_DEFENSE_ALL;
              }
              else if (ArmourClass::DamageExists(damageName))
              {
                damageId = ArmourClass::Name2DamageId(damageName);
              }
              else
              {
                CON_ERR(("Unknown damage '%s'", damageName))
                break;
              }

              // Create the cluster map
              char name[256];
              Utils::Sprintf(name, 256, "Defense [%s] [%s]", teamName, damageName);

              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              void *context = new Defense(team->GetId(), damageId);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                name, WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );

              // Set the callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackDefense);
              g.SetContext(context);
              g.SetPollInterval(5000);
              g.SetAxisFlip(FALSE, TRUE);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr(name), new Map(grid, context));
              break;
            }

            case 0x116C7A5D: // "pain"
            {
              char *teamName;
              char *armourClassName;

              // Test params
              if (!Console::GetArgString(2, teamName) || !Console::GetArgString(3, armourClassName))
              {
                CON_ERR(("client.clustermap.create 'pain' team *|armourclass"))
                break;
              }

              // Convert team name into a team
              Team *team = Team::Name2Team(teamName);
              if (!team)
              {
                CON_ERR(("Unknown team '%s'", teamName))
                break;
              }

              // Convert armour class name into an armour class
              U32 armourClass;

              if (!Utils::Strcmp(armourClassName, "*"))
              {
                armourClass = CLUSTERMAP_PAIN_ALL;
              }
              else if (ArmourClass::ArmourClassExists(armourClassName))
              {
                armourClass = ArmourClass::Name2ArmourClassId(armourClassName);
              }
              else
              {
                CON_ERR(("Unknown armour class '%s'", armourClassName))
                break;
              }

              // Create the cluster map
              char name[256];
              Utils::Sprintf(name, 256, "Pain [%s] [%s]", teamName, armourClassName);

              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              // Create the pain
              void *context = new Pain(team->GetId(), armourClass);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                name, WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );
            
              // Set grid callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackPain);
              g.SetContext(context);
              g.SetPollInterval(5000);
              g.SetAxisFlip(FALSE, TRUE);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr(name), new Map(grid, context));
              break;
            }

            case 0xB817BF51: // "region"
            {
              char *regionName;

              // Test params
              if (!Console::GetArgString(2, regionName))
              {
                CON_ERR(("client.clustermap.create 'region' region"))
                break;
              }

              // Find the region
              RegionObj *region = RegionObj::FindRegion(regionName);
              if (!region)
              {
                CON_ERR(("Unknown region '%s'", regionName))
                break;
              }

              // Create the cluster map
              char name[256];
              Utils::Sprintf(name, 256, "Region [%s]", regionName);

              U32 width = WorldCtrl::ClusterMapX();
              U32 height = WorldCtrl::ClusterMapZ();

              // Determine the scale
              U32 scale = Min(CLUSTERMAP_MAXSIZE / width, CLUSTERMAP_MAXSIZE / height);

              void *context = new Region(region);

              // Create the grid control
              ICGridWindow *grid = new ICGridWindow
              (
                name, WorldCtrl::ClusterMapX(), WorldCtrl::ClusterMapZ(), scale, scale
              );

              // Set the callback and context
              ICGrid &g = grid->Grid();
              g.SetCellCallBack(CallbackRegion);
              g.SetContext(context);

              IFace::ToggleActive(grid->Name());
              maps.Add(Crc::CalcStr(name), new Map(grid, context));
              break;
            }

            default:
              CON_ERR(("Unknown clustermap type '%s'", type))
              break;
          }
        }
        break;
      }

      case 0xA8B5E6F4: // "client.clustermap.ai.addthreat"
      {
        U32 x, z, amount;
        char *teamName;
        char *armourClassName;

        if (!Console::GetArgInteger(1, (S32 &) x) || 
            !Console::GetArgInteger(2, (S32 &) z) || 
            !Console::GetArgString(3, teamName) ||
            !Console::GetArgString(4, armourClassName) ||
            !Console::GetArgInteger(5, (S32 &) amount))
        {
          CON_ERR(("client.clustermap.ai.addthreat x z team armourclass amount"))
        }
        else
        {
          if (x >= WorldCtrl::ClusterMapX())
          {
            CON_ERR(("x value is out of range [0..%d]", WorldCtrl::ClusterMapX() - 1))
            break;
          }

          if (z >= WorldCtrl::ClusterMapZ())
          {
            CON_ERR(("z value is out of range [0..%d]", WorldCtrl::ClusterMapZ() - 1))
            break;
          }

          // Convert team name into a team
          Team *team = Team::Name2Team(teamName);
          if (!team)
          {
            CON_ERR(("Unknown team '%s'", teamName))
            break;
          }

          // Test Armour Class
          U32 armourClass;
          if (ArmourClass::ArmourClassExists(armourClassName))
          {
            armourClass = ArmourClass::Name2ArmourClassId(armourClassName);
          }
          else
          {
            CON_ERR(("Unknown armour class '%s'", armourClassName))
            break;
          }

          MapCluster *cluster = WorldCtrl::GetCluster(x, z);
          ASSERT(cluster)
          cluster->ai.AddThreat(team->GetId(), armourClass, amount);
        }
        break;
      }

      case 0xC692B49A: // "client.clustermap.ai.removethreat"
      {
        U32 x, z, amount;
        char *teamName;
        char *armourClassName;

        if (!Console::GetArgInteger(1, (S32 &) x) || 
            !Console::GetArgInteger(2, (S32 &) z) || 
            !Console::GetArgString(3, teamName) ||
            !Console::GetArgString(4, armourClassName) ||
            !Console::GetArgInteger(5, (S32 &) amount))
        {
          CON_ERR(("client.clustermap.ai.addremove x z team armourclass amount"))
        }
        else
        {
          if (x >= WorldCtrl::ClusterMapX())
          {
            CON_ERR(("x value is out of range [0..%d]", WorldCtrl::ClusterMapX() - 1))
            break;
          }

          if (z >= WorldCtrl::ClusterMapZ())
          {
            CON_ERR(("z value is out of range [0..%d]", WorldCtrl::ClusterMapZ() - 1))
            break;
          }

          // Convert team name into a team
          Team *team = Team::Name2Team(teamName);
          if (!team)
          {
            CON_ERR(("Unknown team '%s'", teamName))
            break;
          }

          // Test Armour Class
          U32 armourClass;
          if (ArmourClass::ArmourClassExists(armourClassName))
          {
            armourClass = ArmourClass::Name2ArmourClassId(armourClassName);
          }
          else
          {
            CON_ERR(("Unknown armour class '%s'", armourClassName))
            break;
          }

          MapCluster *cluster = WorldCtrl::GetCluster(x, z);
          ASSERT(cluster)
          cluster->ai.RemoveThreat(team->GetId(), armourClass, amount);
        }
        break;
      }

      case 0xBFCBD6CD: // "client.clustermap.ai.adddefense"
      {
        U32 x, z, amount;
        char *teamName;
        char *damageName;

        if (!Console::GetArgInteger(1, (S32 &) x) || 
            !Console::GetArgInteger(2, (S32 &) z) || 
            !Console::GetArgString(3, teamName) ||
            !Console::GetArgString(4, damageName) ||
            !Console::GetArgInteger(5, (S32 &) amount))
        {
          CON_ERR(("client.clustermap.ai.adddefense x z team damage amount"))
        }
        else
        {
          if (x >= WorldCtrl::ClusterMapX())
          {
            CON_ERR(("x value is out of range [0..%d]", WorldCtrl::ClusterMapX() - 1))
            break;
          }

          if (z >= WorldCtrl::ClusterMapZ())
          {
            CON_ERR(("z value is out of range [0..%d]", WorldCtrl::ClusterMapZ() - 1))
            break;
          }

          // Convert team name into a team
          Team *team = Team::Name2Team(teamName);
          if (!team)
          {
            CON_ERR(("Unknown team '%s'", teamName))
            break;
          }

          // Test Armour Class
          U32 damageId;
          if (ArmourClass::DamageExists(damageName))
          {
            damageId = ArmourClass::Name2DamageId(damageName);
          }
          else
          {
            CON_ERR(("Unknown damage '%s'", damageName))
            break;
          }

          MapCluster *cluster = WorldCtrl::GetCluster(x, z);
          ASSERT(cluster)
          cluster->ai.AddDefense(team->GetId(), damageId, amount);
        }
        break;
      }

      case 0x1D5FE45D: // "cliiet.clustermap.ai.removedefense"
      {
        U32 x, z, amount;
        char *teamName;
        char *damageName;

        if (!Console::GetArgInteger(1, (S32 &) x) || 
            !Console::GetArgInteger(2, (S32 &) z) || 
            !Console::GetArgString(3, teamName) ||
            !Console::GetArgString(4, damageName) ||
            !Console::GetArgInteger(5, (S32 &) amount))
        {
          CON_ERR(("client.clustermap.ai.removedefense x z team damage amount"))
        }
        else
        {
          if (x >= WorldCtrl::ClusterMapX())
          {
            CON_ERR(("x value is out of range [0..%d]", WorldCtrl::ClusterMapX() - 1))
            break;
          }

          if (z >= WorldCtrl::ClusterMapZ())
          {
            CON_ERR(("z value is out of range [0..%d]", WorldCtrl::ClusterMapZ() - 1))
            break;
          }

          // Convert team name into a team
          Team *team = Team::Name2Team(teamName);
          if (!team)
          {
            CON_ERR(("Unknown team '%s'", teamName))
            break;
          }

          // Test Armour Class
          U32 damageId;
          if (ArmourClass::DamageExists(damageName))
          {
            damageId = ArmourClass::Name2DamageId(damageName);
          }
          else
          {
            CON_ERR(("Unknown damage '%s'", damageName))
            break;
          }

          MapCluster *cluster = WorldCtrl::GetCluster(x, z);
          ASSERT(cluster)
          cluster->ai.RemoveDefense(team->GetId(), damageId, amount);
        }
        break;
      }
    }
  };


  //
  // DeleteMaps
  //
  void ClusterMap::DeleteMaps()
  {
    maps.DisposeAll();
  }
}

