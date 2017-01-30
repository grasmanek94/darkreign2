///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 10-JUL-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_private.h"
#include "light_priv.h"
#include "terrain_priv.h"
#include "mapobjctrl.h"
#include "gameobjctrl.h"
#include "worldctrl.h"
#include "gametime.h"
#include "input.h"
#include "collisionctrl.h"
#include "team.h"
#include "perfstats.h"
#include "promote.h"
#include "sight.h"
#include "unitobj.h"
#include "mapobj.h"
#include "sync.h"
#include "common.h"
#include "main.h"
#include "client.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MapObjCtrl - Manages all map object instances
//
namespace MapObjCtrl
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Defines
  //
  #define MAX_MESHENTS 1024
  #define MAX_MESHES 256
  #define MAX_SHADOWS 256


  enum FogOfWar
  {
    fowNONE       = 7,
    fowFOG        = 2,
    fowSHROUD     = 0,
  };

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct
  //
  struct RenderMesh
  {
    // Mesh Root
    MeshRoot *mesh;

    // Matrix
    Matrix matrix;

    // Color
    Color color;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // Has the system been initialized
  static Bool sysInit;

  // All objects to be added to the map
  struct AddToMapItem
  {
    // Object pointer
    MapObj *obj;

    // Does it need to be zipped?
    U32 zip : 1;

    NList<AddToMapItem>::Node node;

    AddToMapItem(MapObj *obj, Bool zip = FALSE) 
    : obj(obj), 
      zip(zip) 
    {}
  };

  static NList<AddToMapItem> listAddToMap(&AddToMapItem::node);

  // All objects currently off the map
  static NList<MapObj> listOffMap(&MapObj::offMapNode);
  
  // All objects currently on the map
  static NList<MapObj> listOnMap(&MapObj::onMapNode);

  // Objects that need primitive processing
  static NList<MapObj> listPrimitive(&MapObj::primitiveNode);

  // List of objects to display
  static NList<MapObj> listDisplay(&MapObj::displayNode);

  // All objects on health modifier cells
  static NList<MapObj> listTerrainHealth(&MapObj::terrainHealthNode);

  // List of shadows to display
  static List<MeshEnt> listShadows;

  // Array of non-MRM MeshEnt nodes to display
  static MeshEnt * arrayMeshEnt[MAX_MESHENTS];

  // non-MRM MeshEnt node index
  static MeshEnt ** indexMeshEnt;

  // Last non-MRM MeshEnt node index
  static MeshEnt ** lastMeshEnt = &arrayMeshEnt[MAX_MESHENTS];

  // Array of MRM MeshEnt nodes to display
  static MeshEnt * arrayMeshEntMirror[MAX_MESHENTS];

  // MRM MeshEnt node index
  static MeshEnt ** indexMeshEntMirror;

  // Last MRM MeshEnt node index
  static MeshEnt ** lastMeshEntMirror = &arrayMeshEntMirror[MAX_MESHENTS];

  // Array of meshes to display
  static RenderMesh arrayMesh[MAX_MESHES];
  
  // Mesh index
  static RenderMesh * indexMesh;

  // Last Mesh Index
  static const RenderMesh *lastMesh = &arrayMesh[MAX_MESHES];

  // Array of lights to display
  static Vid::Light::Obj * arrayLight[MAX_MESHES];

  // Light index
  static Vid::Light::Obj ** indexLight;

  // Last Light Index
  static Vid::Light::Obj ** lastLight = &arrayLight[MAX_MESHES];

  // Array of shadows to display
  static MeshEnt * arrayShadow[MAX_SHADOWS];

  // Shadow index
  static MeshEnt **indexShadow;

  // Last Shadow Index
  static MeshEnt **lastShadow = &arrayShadow[MAX_SHADOWS];

  // Objects that were recently hurt
  static MapObjOuchList ouchList;

  // Iteration node for processing the primitive list
  NList<MapObj>::Node *processingNode = NULL;


  // 
  // RegisterConstruction
  //
  // Register the construction of an object
  //
  void RegisterConstruction(MapObj *obj)
  {
    ASSERT(obj);

    // Add to the off-map list
    listOffMap.Append(obj);
  }


  //
  // RegisterDestruction
  //
  // Register the destruction of an object
  //
  void RegisterDestruction(MapObj *obj)
  {
    ASSERT(obj);

    // Is it still on the map
    if (obj->OnMap())
    {
      RemoveFromMap(obj);
    }

    ASSERT(!obj->OnMap());

    // Remove from the off-map list
    listOffMap.Unlink(obj);

    // Remove from the display list
    if (obj->displayNode.InUse())
    {
      listDisplay.Unlink(obj);
    }
  }


  //
  // UpdatePrimitiveProcessingState
  //
  // Add/remove object from movement processing list
  //
  void UpdatePrimitiveProcessingState(MapObj *obj)
  {
    ASSERT(obj);

    // Should we be on the list in our current state
    if (obj->OnMap() && obj->RequestingPrimitive())
    {
      // If not on list already, slap it on
      if (!obj->OnPrimitiveList())
      {
        listPrimitive.Prepend(obj);
      }
    }
    else
    {
      // If on the list, remove it
      if (obj->OnPrimitiveList())
      {
        // Is this the next object to be processed ?
        if (processingNode && &obj->MapObj::primitiveNode == processingNode)
        {
          // Set the processingNode to the next node
          processingNode = processingNode->GetNext();
        }
       
        listPrimitive.Unlink(obj);
      }
    }
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!sysInit);
    ASSERT(!listOffMap.GetCount());
    ASSERT(!listOnMap.GetCount());
    ASSERT(!listPrimitive.GetCount());
    ASSERT(!listTerrainHealth.GetCount());

    // System now initialized
    sysInit = TRUE;
  }

 
  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(sysInit);
    ASSERT(!listOffMap.GetCount());
    ASSERT(!listOnMap.GetCount());
    ASSERT(!listPrimitive.GetCount());
    ASSERT(!listTerrainHealth.GetCount());

    // The ouch list should not contain any live objects
    ouchList.PurgeDead();

    // Cleanup the display list
    listDisplay.UnlinkAll();

    // System now shutdown
    sysInit = FALSE;
  }


  //
  // AddToListOnMap
  //
  // Remember an object that needs to be added to the map
  //
  void AddToListOnMap(MapObj *obj, Bool zip, Bool prepend)
  {
    ASSERT(sysInit);
    ASSERT(obj);
    ASSERT(!obj->OnMap());

    AddToMapItem *item = new AddToMapItem(obj, zip);

    if (prepend)
    {
      listAddToMap.Prepend(item);
    }
    else
    {
      listAddToMap.Append(item);
    }
  }

  //
  // SetFogTarget
  //
  // Setup fog of war
  //
  static Bool SetFogTarget( MapObj * obj, Team * team, Bool immediate = FALSE)
  {
    // Do we need to check line of sight for a team
    Bool fogFilter = TRUE;

#ifdef DEVELOPMENT
      // Override sight in debug mode
      if (Sight::DebugMode())
      {
        fogFilter = FALSE;
      }
#endif

    // This should only be set by cineractives
    if (Sight::ShowAllUnits())
    {
      fogFilter = FALSE;
    }

    MapObjType * type = obj->MapType();

    // Default to completely visible
    U32 fog = fowNONE;
    U32 alpha = fowNONE;

    // Only fog if a team was supplied
    if (team)
    {
      // Is objects location seen/visible by given team
      Bool seen, visible;

      // Either check real values, or see everything
      if (fogFilter)
      {
        obj->GetSeenVisible(team, seen, visible);
      }
      else
      {
        seen = visible = TRUE;
      }

      // Has the location ever been seen
      if (seen)
      {
        // Is the location currently visible
        if (visible)
        {
          // Is this object a unit
          if (UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(obj))
          {
            // Is this unit invisible to the given team
            if (unit->GetFlag(UnitObj::FLAG_INVISIBLE) && Team::TestRelation(team, unit->GetTeam(), Relation::ENEMY))
            {
              fog = alpha = fowSHROUD;
            }
          }
        }
        else

        // Can this object been seen under fog
        if (type->ShowSeen() || type->ShowAlways())
        {
          fog = *Vid::Var::Terrain::shroudFog;
        }
        else
        {
          fog = alpha = fowSHROUD;
        }
      }
      else
      {
        // Show this object as shrouded
        fog = fowSHROUD;

        // Alpha this object out, unless it's always seen
        if (!type->ShowAlways())
        {
          alpha = fowSHROUD;
        }
      }
    }

    // Set the fogging target in the object
    obj->SetFogTarget( fog, alpha, immediate);

    // Object should be drawn if it has not yet faded out
    return (obj->Mesh().alphaCurrent != 0);
  }

  
  //
  // AddToMap
  //
  // Move an object from offMap -> onMap
  //
  void AddToMap(MapObj *obj)
  {
    ASSERT(sysInit);
    ASSERT(obj);
    ASSERT(!obj->OnMap());
    ASSERT(!obj->OnPrimitiveList());

    // Remove from the off-map list
    listOffMap.Unlink(obj);

    // update before its on the map
    obj->UpdateMapPos();

    // Add to the on-map list
    listOnMap.Append(obj);

    // Process Hook function
    obj->AddToMapHook();

    // Add to primitive list
    UpdatePrimitiveProcessingState(obj);

    ASSERT(obj->OnMap());

    // Immediately set the appropriate fogging value
    SetFogTarget( obj, Team::GetDisplayTeam(), TRUE);
  }


  //
  // RemoveFromMap
  //
  // Move an object from onMap -> offMap
  //
  void RemoveFromMap(MapObj *obj)
  {
    ASSERT(sysInit);
    ASSERT(obj);
    ASSERT(obj->OnMap());

    // Process Hook function
    obj->RemoveFromMapHook();

    // Remove from the on-map list
    listOnMap.Unlink(obj);

    // Add to the off-map list
    listOffMap.Append(obj);

    // Pull off movement list
    UpdatePrimitiveProcessingState(obj);

    ASSERT(!obj->OnMap());
    ASSERT(!obj->OnPrimitiveList());
  }


  //
  // RemoveAllFromMap
  //
  // Removes all objects from the map
  //
  void RemoveAllFromMap()
  {
    NList<MapObj>::Iterator i(&listOnMap);

    while (*i)
    {
      RemoveFromMap(i++);
    }

    ASSERT(!listOnMap.GetCount());
  }


  //
  // AddToOuchList
  //
  // Add and remove objects from the ouch list
  //
  void AddToOuchList(MapObj *obj)
  {
    ouchList.AppendNoDup(obj);
  }


  //
  // RemoveFromOuchList
  //
  void RemoveFromOuchList(MapObj *obj)
  {
    ouchList.Remove(obj);
  }


  //
  // RemoveFromOuchList
  //
  void RemoveFromOuchList(MapObjOuchListNode *node)
  {
    ouchList.Dispose(node);
  }


  //
  // DeleteAll
  //
  // Deletes all existing map objects
  //
  void DeleteAll()
  {
    // Remove all objects from the map
    RemoveAllFromMap();

    // Delete each object
    NList<MapObj>::Iterator i(&listOffMap);
    while (*i)
    {   
      GameObjCtrl::MarkForDeletion(i++);
    }

    ASSERT(!listOffMap.GetCount());
  }


  //
  // Post Load
  //
  // Do post mission load functions
  //
  void PostLoad()
  {
    // Add required objects to the map
    for (NList<AddToMapItem>::Iterator i(&listAddToMap); *i; i++)
    {
      MapObj *obj = (*i)->obj;

      if (!obj->GetParent())
      {
        AddToMap(obj);

        if ((*i)->zip)
        {
          obj->ToggleFootPrint(TRUE);
        }
      }
    }

    listAddToMap.DisposeAll();
  }


  //
  // ObjectNew
  //
  // Create a new object of 'type'
  //
  MapObj* ObjectNew(MapObjType *type, U32 id, Bool complete)
  {
    ASSERT(type);

    // Create a new instance of the supplied type
    MapObj *mapObj = (MapObj *) type->NewInstance(id);

    // Equip the object
    mapObj->Equip();

    // Should the object be completed
    if (complete)
    {
      // Set the hitpoints of this object via its type information
      mapObj->ModifyHitPoints(type->GetHitPoints());

      // Set the armour of this object via its type information
      if (type->GetArmourInitial())
      {
        mapObj->ModifyArmour(type->GetArmour());
      }
    }

    return (mapObj);
  }


  //
  // ObjectNewOnMap
  //
  // Creates a new object of 'type' and places on the map using 'matrix'
  //
  MapObj* ObjectNewOnMap(MapObjType *type, const Matrix &matrix, U32 id, Bool zip, Bool complete)
  {
    ASSERT(type);

    // Create the new object
    MapObj *newObj = ObjectNew(type, id, complete);

    // Set the object transform
    newObj->SetSimCurrent(matrix);

    // Add to the map
    AddToMap(newObj);

    // Zip if required
    if (zip)
    {
      newObj->ToggleFootPrint(TRUE);
    }

    return (newObj);
  }


  //
  // ObjectNew
  //
  // Overload for using a string type identifier
  //
  MapObj* ObjectNew(U32 typeCrc, U32 id, Bool complete)
  {
    MapObjType *type = GameObjCtrl::FindType<MapObjType>(typeCrc);
    return (type ? ObjectNew(type, id, complete) : NULL);
  }


  //
  // ObjectNewOnMap
  //
  // Overload for using a string type identifier
  //
  MapObj* ObjectNewOnMap(U32 typeCrc, const Matrix &m, Bool zip, Bool complete)
  {
    MapObjType *type = GameObjCtrl::FindType<MapObjType>(typeCrc);
    return (type ? ObjectNewOnMap(type, m, zip, complete) : NULL);
  }


  //
  // ProcessTerrainHealth
  //
  // Process all objects on the terrain health modifier list
  //
  static void ProcessTerrainHealth()
  {
    // Iterate optimally over list
    NList<MapObj>::Node *node = listTerrainHealth.GetHeadNode();

    while (node)
    {
      // Get object
      MapObj *m = node->GetData();

      // Modify the current hitpoints
      m->ModifyHitPoints
      (
        S32(m->GetBalanceData().health * m->MapType()->GetHitPoints())
      );

      // Proceed to next object
      node = node->GetNext();
    }    
  }


  //
  // ProcessObjects
  //
  // Do all per-cycle object processing
  //
  void ProcessObjects()
  {
    // Iterate optimally over movement list
    processingNode = listPrimitive.GetHeadNode();

    while (processingNode)
    {
      MapObj *m = processingNode->GetData();

      // Proceed to next object
      processingNode = processingNode->GetNext();

      // Update instance modifiers
      PERF_S("Process Cycle")
      m->ProcessCycle();
      PERF_E("Process Cycle")
    }
  }


  // set up for the next sim frame
  //
  void UpdateMapPos()
  {
    F32 dt = GameTime::SimTime();

    NList<MapObj>::Iterator li(&listPrimitive); 
    while (MapObj * obj = li++)
    {
      if (obj->GetParent())
      {
        // child ents UpdateSim by parents; don't need to verify map pos
        continue;
      }

      // if this is a unit without a parent
      //

      //SYNC_BRUTAL("Before UpdateSim: " << obj)

      // setup for this frame
      //
      Bool wasDirty = obj->UpdateSim( dt);

      //SYNC_BRUTAL("After UpdateSim: " << obj)

      if (wasDirty)
      {
        // verify it's on the map; update cluster hooks
        //
        obj->UpdateMapPos();
      }
    }
  }


  //
  // Simulate
  //
  // Animate the MapObjs' current cycle
  //
  void SimulateInt(F32 dt)
  { 
    NList<MapObj>::Iterator li(&listPrimitive); 
    while (MapObj * obj = li++)
    {
      if (!obj->GetParent())
      {
        // ent function clips dt
        //
        obj->Mesh().SimulateInt(dt); 
      }
    }
  }


  //
  // SetFoggingValues
  //
  // Immediately set the fogging values for all objects 
  //
  void SetObjectFogging(Team *team)
  {
    NList<MapObj>::Iterator li(&listOnMap); 
    while (MapObj * obj = li++)
    {
      SetFogTarget( obj, team, TRUE);
    }  
  }


  //
  // BuildDisplayList
  //
  // Select objects to display for the next frame
  //
  void BuildDisplayList( Team * team, Bool simFrame)
  {
    // Unlink previous frame
    listDisplay.UnlinkAll();

    indexMeshEnt    = arrayMeshEnt;
    indexMesh       = arrayMesh;
    indexShadow     = arrayShadow;
    indexLight      = arrayLight;

    // Build list using clusters
    //FIXME(916355881, "apayne"); // Thu Jan 14 15:18:01 1999

    Mesh::Manager::ResetCounts();

    // Do we need to check line of sight for a team
    Bool fogFilter = TRUE;

    #ifdef DEVELOPMENT
      // Override sight in debug mode
      if (Sight::DebugMode())
      {
        fogFilter = FALSE;
      }
    #endif

    // THis should only be set by cineractives
    if (Sight::ShowAllUnits())
    {
      fogFilter = FALSE;
    }

    // Step over all objects on the map
    U32 oldTime = U32_MAX;    // find the oldest shadow
    MeshEnt * old = NULL;
    NList<MapObj>::Iterator li(&listOnMap); 
    while (MapObj * obj = li++)
    {
      MeshEnt & ent = obj->Mesh();
      MapObjType * type = obj->MapType();

      // Don't bother will null objects
      if (type->IsNullObj())
      {
        ent.visible = FALSE;
        continue;
      }

      // cast shadows 
      //
      if (type->HasShadow() && ent.shadowType >= MeshRoot::shadowSEMILIVE)
      {
        if (ent.shadowTime == 0 || ent.dirtyShadow || ent.shadowType == MeshRoot::shadowLIVE)
        {
          // first time or moved or animated
          ent.RenderShadowTexture();
        }
        else if (ent.shadowTime < oldTime)
        {
          // find the oldest shadow 
          // one object is recalced
          old = &obj->Mesh();
          oldTime = ent.shadowTime;
        }
      }

      // Basic simulation for all objects at frame rate
      obj->UpdateIntBasic(Main::elapSecs, simFrame);

      ent.visible = SetFogTarget( obj, team);
      if (!ent.visible)
      {
        continue;
      }

      // add visible shadows
      //
      if (type->HasShadow() && indexShadow < lastShadow)
      {
        if (TerrainData::BoundsTestShadowWithWater( ent))
        {
          *(indexShadow++) = &ent;
        }
      }

      // add to display list if object is on the screen
      //
      if (ent.BoundsTest() != clipOUTSIDE)
      {
        if (indexMeshEnt < lastMeshEnt)
        {
          *indexMeshEnt = &ent;
          indexMeshEnt++;
        }
        else
        {
          //LOG_DIAG( ("MapObjCtrl::BuildDisplayList: max ents %d", indexMeshEnt - arrayMeshEnt) );
        }

        listDisplay.Append(obj);
      }

      NList<Vid::Light::Obj>::Iterator l(&obj->GetLights());
      while (Vid::Light::Obj * light = l++)
      {
        ASSERT( light);

        if (light->IsActive() && light->IsRenderable())
        {
          if (indexLight < lastLight)
          {
            *indexLight = light;
            indexLight++;
          }
          else
          {
            //LOG_DIAG( ("MapObjCtrl::BuildDisplayList: max lights %d", indexLight - arrayLight) );
          }
        }
      }
    }

    if (old)
    {
      old->RenderShadowTexture();
    }
  }


  //
  // Render
  //
  // Render all visible objects
  //
  void Render()
  {
/*
    if (Vid::renderState.status.mirror)
    {
      return;
    }
*/
//    Vid::FlushTranBuckets();

    // Render all of the shadows
    if (Vid::renderState.status.showShadows)
    {
      for (MeshEnt ** index = arrayShadow; index < indexShadow; index++)
      {
        ASSERT( *index);

        TerrainData::RenderShadowWithWater( (*index)->shadowInfo);

        Vid::extraFog = 0;
      }
//      Vid::FlushTranBuckets();
    }

    // Render all of the lights
    for (Vid::Light::Obj ** light = arrayLight; light < indexLight; light++)
    {
      ASSERT( (light - arrayLight) <= MAX_MESHENTS && *light);

#if 1
      if (*Vid::Var::Terrain::lightMap)
      {
        TerrainData::RenderLightWithWater( **light);
      }
#endif
      (*light)->RenderSingle();
    }

    // Render all of MeshEnt nodes
    for (MeshEnt ** index = arrayMeshEnt; index < indexMeshEnt; index++)
    {
      ASSERT( (index - arrayMeshEnt) <= MAX_MESHENTS && *index);

      (*index)->RenderSingle();

    }
  }


  //
  // BuildMirrorList
  //
  // Test displayed objects for mirror plane clip 
  //
  void BuildMirrorList()
  {
    indexMeshEntMirror = arrayMeshEntMirror;

    NList<MapObj>::Iterator li(&listOnMap); 
    while (MapObj * obj = li++)
    {
      MeshEnt & ent = obj->Mesh();
      
      if (ent.visible && ent.BoundsTest() != clipOUTSIDE)
      {
        *indexMeshEntMirror++ = &ent;

        if (indexMeshEntMirror >= lastMeshEntMirror)
        {
          break;
        }
      }
    }
  }

  //
  // RenderMirror
  //
  // Render all visible mirrored objects
  //
  void RenderMirror()
  {
    // Render all mirror MeshEnts

    for (MeshEnt ** index = arrayMeshEntMirror; index < indexMeshEntMirror; index++)
    {
      ASSERT( (index - arrayMeshEntMirror) <= MAX_MESHENTS && *index);

      (*index)->RenderSingle();
    }

  }


  //
  // GetOnMapList
  //
  // Get the list of objects on the map
  //
  const NList<MapObj> & GetOnMapList()
  {
    return (listOnMap);
  }


  //
  // GetOffMapList
  //
  // Get the list of objects off the map
  //
  const NList<MapObj> & GetOffMapList()
  {
    return (listOffMap);
  }


  //
  // GetDisplayList
  //
  // Get the list of objects being displayed
  //
  const NList<MapObj> & GetDisplayList()
  {
    return (listDisplay);
  }


  //
  // GetTerrainHealthList
  //
  // Get the list of objects on health modifier cells
  //
  NList<MapObj> & GetTerrainHealthList()
  {
    return (listTerrainHealth);
  }
 

  //
  // GetOuchList
  //
  // Get the list of objects off the map
  //
  MapObjOuchList & GetOuchList()
  {
    return (ouchList);
  }

}