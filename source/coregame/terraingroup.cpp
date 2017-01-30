///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 17-JUN-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "terraingroup.h"
#include "movetable.h"
#include "stdload.h"
#include "filesys.h"
#include "random.h"



///////////////////////////////////////////////////////////////////////////////
//
// Namespace TerrainGroup - Manages terrain types in groups
//
namespace TerrainGroup
{
  LOGDEFLOCAL("TerrainGroup");

  // Maximum number of terrain types per group (8 bit)
  enum { MAX_TYPES = U8_MAX };

  // System initialization flag
  static Bool initialized = FALSE;

  // Index to terrain types in current group
  static Type *index[MAX_TYPES];

  // Number of groups in the current index
  static U32 indexCount;

  // Tree of all existing groups
  static BinTree<Group> groupTree;

  // The currently active group
  static Group *activeGroup;

  // The current default group
  static Group *defaultGroup;

  // The group to use when no default
  static GameString defaultGroupName("Default");

  // The name of the configuration file
  static const char *configName = "terrain.cfg";

  // The name of the terrain group file stream
  static const char *streamName = "world";

  // Temporary - the base terrain group directory
  static const char *groupDir = "worlds";



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type::BlendInfo - Information for each blend in a type
  //

  //
  // Constructor
  //
  Type::BlendInfo::BlendInfo() : variations(0), weights(NULL), totalWeight(0.0F)
  {
  }


  //
  // Destructor
  //
  Type::BlendInfo::~BlendInfo()
  {
    weightConfigs.DisposeAll();

    if (weights)
    {
      delete [] weights;
    }
  }


  //
  // AddWeight
  //
  // Add a weight configuration
  //
  void Type::BlendInfo::AddWeight(U32 variation, F32 weight)
  {
    weightConfigs.Append(new WeightConfig(variation, weight));
  }


  //
  // SetVariations
  //
  // Set the actual number of variations
  //
  void Type::BlendInfo::SetVariations(U32 v)
  {
    ASSERT(v)

    // Delete any existing weight array
    if (weights)
    {
      delete [] weights;
    }

    // Set the new count
    variations = v;

    // Allocate new weight array
    weights = new F32[v];

    // Set default weights
    for (U32 i = 0; i < v; weights[i++] = 1.0F);

    // Read configured values
    for (List<WeightConfig>::Iterator w(&weightConfigs); *w; w++)
    {
      // Is the variation count in range
      if ((*w)->v < v)
      {
        weights[(*w)->v] = (*w)->w;
      }
      else
      {
        LOG_WARN(("Ignoring weight (%d,%f)", (*w)->v, (*w)->w));
      }
    }

    // Calculate total weight
    for (i = 0, totalWeight = 0.0F; i < v; totalWeight += weights[i++]);
  }


  //
  // GetVariations
  //
  // Get the number of variations
  //
  U32 Type::BlendInfo::GetVariations()
  {
    return (variations);
  }


  //
  // GetRandomVariation
  //
  // Returns a random variation index
  //
  U32 Type::BlendInfo::GetRandomVariation()
  {
    ASSERT(variations)
    ASSERT(weights)

    // Default to first variation
    U32 v = 0;

    // Select a random point within the weight range
    F32 p = Random::nonSync.Float() * totalWeight;

    // Slide a value along
    F32 s = 0.0f;

    // Iterate through all the variations
    for (U32 i = 0; i < variations; i++)
    {
      // Is the point within this variation
      if (s <= p && p <= (s += weights[i]))
      {
        v = i;
        break;
      }
    }
  
    return (v);
  }
  


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type - A single terrain type record
  //

  //
  // Constructor
  //
  Type::Type(const char *name, U8 index, const char *surfaceName, const char *asset, Group *group, FScope *fScope) 
   : name(name), index(index), asset(asset), group(group)
  {
    // Set the surface index
    surface = MoveTable::SurfaceIndex(surfaceName);
   
    FScope *sScope;
 
    // Process the sub-functions
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xA8D887E9: // "Weight"
        {
          // Read the args
          U32 b = sScope->NextArgInteger();
          U32 v = sScope->NextArgInteger();
          F32 w = sScope->NextArgFPoint();

          // Check blend value
          if (b >= BLEND_COUNT)
          {
            ERR_FATAL(("Blend index is out of range! (%s/%s)", group->Name(), name));
          }

          // Add the weight configuration
          blendInfo[b].AddWeight(v, w);
          break;
        }
      }
    }
  }


  //
  // EnumerateResources
  //
  // Enumerates all available blend resources, and generates
  // fatal errors if missing required files.
  //
  void Type::EnumerateResources()
  {
    // For each blend
    for (U32 b = 0; b < BLEND_COUNT; b++)
    {
      // Find all variations
      for (U32 v = 0; FileSys::Exists(BlendName(b, v)); v++);

      // Were any found
      if (v)
      {
        // Setup blend info
        blendInfo[b].SetVariations(v);
      }
      else

      // Missing blend asset (not the base type OR the first blend)
      if (index || !b)
      {
        ERR_FATAL(("Type '%s' missing '%s'", Name(), BlendName(b, 0)));
      }
    }

    /*
    LOG_DIAG
    ((
      "- Variations for %s [%d,%d,%d,%d]", Name(),
      GetVariations(0), GetVariations(1), GetVariations(2), GetVariations(3)
    ));
    */
  }


  //
  // GetVariations
  //
  // Returns the number of variations for the given blend
  //
  U32 Type::GetVariations(U32 blend)
  {
    ASSERT(blend < BLEND_COUNT)

    return (blendInfo[blend].GetVariations());
  }

  
  //
  // GetRandomVariation
  //
  // Returns a random variation for the given blend
  //
  U32 Type::GetRandomVariation(U32 blend)
  {
    ASSERT(blend < BLEND_COUNT)

    return (blendInfo[blend].GetRandomVariation());
  }


  //
  // BlendName
  //
  // Returns static buffer containing the file name of the given blend
  //
  const char * Type::BlendName(U32 blend, U32 variation)
  {
    ASSERT(blend < BLEND_COUNT)

    static char name[512];

    // Generate name in accordance with the "Blend-O-Rama '99" convention
    Utils::Sprintf(name, sizeof(name), "%s-%s-%d-%d.pic", group->Name(), Asset(), blend, variation);

    return (name);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Group - A single terrain group containing types
  //

  //
  // Constructor
  //
  Group::Group(const char *name, FScope *fScope)
    : name(name)
  { 
    // Add the base type
    AddType(fScope);
  }


  //
  // Destructor
  //
  Group::~Group()
  {
    // Delete all types
    list.DisposeAll();
  }


  //
  // EnumerateResources
  //
  // Enumerates resources (call each time current group changes)
  //
  void Group::EnumerateResources()
  {
    // Notify each type in this group
    for (List<Type>::Iterator i(&list); *i; i++)
    {
      (*i)->EnumerateResources();
    } 
  }

  
  //
  // FindType
  //
  // Find a particular type
  //
  Type * Group::FindType(U32 crc)
  {
    // Check each type
    for (List<Type>::Iterator i(&list); *i; i++)
    {
      // Does the crc match
      if ((*i)->NameCrc() == crc)
      {
        // Bingo
        return (*i);
      }
    }

    // Not found
    return (NULL);
  }


  //
  // FindType
  // 
  // Find a particular type
  //
  Type * Group::FindType(const char *name)
  {
    return (FindType(Crc::CalcStr(name)));
  }


  //
  // AddType
  //
  // Add a new terrain type
  //
  void Group::AddType(FScope *fScope)
  {
    // Get args first
    const char *ident = StdLoad::TypeString(fScope);
    const char *surface = StdLoad::TypeString(fScope);
    const char *asset = StdLoad::TypeStringD(fScope, ident);

    // Fatal if type already exists
    if (FindType(ident))
    {
      ERR_FATAL(("Duplicate type (%s) in group (%s)", ident, Name()));
    }

    // Do we have room for another type
    if (list.GetCount() >= MAX_TYPES)
    {
      ERR_FATAL(("Max terrain types reached (%s/%d)", ident, MAX_TYPES));
    }

    // Create the new type
    list.Append(new Type(ident, (U8)list.GetCount(), surface, asset, this, fScope));
  }


  //
  // BaseType
  //
  // Returns the base type for this group
  //
  Type * Group::BaseType()
  {
    // The base is always first on the list
    return (list.GetHead());
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  //
  // ProcessCreateTerrainGroup
  //
  // Creates a new terrain group
  //
  static void ProcessCreateTerrainGroup(FScope *fScope)
  {
    ASSERT(fScope);

    // Get the name of the group
    GameIdent groupIdent = StdLoad::TypeString(fScope);

    // Does this group already exist
    if (groupTree.Exists(groupIdent.crc))
    {
      ERR_CONFIG(("Terrain group '%s' already defined!", groupIdent.str));
    }

    // Get the base type function (required)
    FScope *sScope = fScope->GetFunction("BaseType");

    // Create the group
    Group *group = new Group(groupIdent.str, sScope);
  
    // Process all sub-functions
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        // Add a type
        case 0xB95A11FF: // "AddType"
          group->AddType(sScope);
          break;
      }
    }
  
    // Add new group to the tree of known groups
    groupTree.Add(groupIdent.crc, group);
  }


  //
  // Init
  //
  // Initialize system
  //
  void Init()
  {
    ASSERT(!initialized);

    PTree pTree;

    // No active group by default
    activeGroup = NULL;

    // No default group by default
    defaultGroup = NULL;

    // Type index is empty by default
    indexCount = 0;

    // Load the configuration file
    if (pTree.AddFile(configName))
    {
      // Get the global scope
      FScope *sScope, *fScope = pTree.GetGlobalScope();

      // Step through each function in this scope
      while ((sScope = fScope->NextFunction()) != NULL)
      {
        switch (sScope->NameCrc())
        {
          case 0xE9E4AC05: // "CreateTerrainGroup"
            ProcessCreateTerrainGroup(sScope);
            break;
        }
      }  
    }
    else
    {
      ERR_FATAL(("Could not open '%s'", configName));
    }

    // Must create at least one group
    if (!groupTree.GetCount())
    {
      ERR_FATAL(("No terrain groups were created in %s", configName));
    }

    // System now initialized
    initialized = TRUE;
  }

 
  //
  // Done
  //
  // Shutdown system
  //
  void Done()
  {
    ASSERT(initialized);

    // Dispose of all groups
    groupTree.DisposeAll();

    // System now shutdown
    initialized = FALSE;
  }


  //
  // Initialized
  //
  // Is the system initialized
  //
  Bool Initialized()
  {
    return (initialized);
  }


  //
  // FindGroup
  //
  // Find a group by crc (NULL if not found)
  //
  Group * FindGroup(U32 crc)
  {
    ASSERT(initialized);
    return (groupTree.Find(crc));
  }


  //
  // FindGroup
  //
  // Find a group by name (NULL if not found)
  //
  Group * FindGroup(const char *name)
  {
    ASSERT(initialized);
    return (FindGroup(Crc::CalcStr(name)));
  }

  
  //
  // SetDefaultGroup
  //
  // Set the default group (can be called before initialization)
  //
  Bool SetDefaultGroup(const char *name)
  {
    // Save this name
    defaultGroupName = name;

    if (initialized)
    {
      // Find the requested group
      defaultGroup = FindGroup(name);

      // Return true if found
      return (defaultGroup ? TRUE : FALSE);
    }

    return (TRUE);
  }


  //
  // GetDefaultGroup
  //
  // Get the default or first group (never returns NULL)
  //
  Group & GetDefaultGroup()
  {
    ASSERT(initialized);
    
    if (!groupTree.GetCount())
    {
      ERR_CONFIG(("No terrain groups have been created"));
    }

    // Find the default default group ;)
    if (!defaultGroup)
    {
      defaultGroup = FindGroup(defaultGroupName.str);
    }

    return (defaultGroup ? *defaultGroup : *groupTree.GetFirst());
  }


  //
  // SetActiveGroup
  //
  // Set the currently active group
  //
  void SetActiveGroup(Group &group)
  {
    ASSERT(initialized);

    // Set the active group
    activeGroup = &group;

    // Clear type index
    indexCount = 0;

    // Setup with types from active group
    for (List<Type>::Iterator i(&activeGroup->GetTypeList()); *i; i++)
    {
      index[indexCount++] = *i;
    }

    // Delete any current stream
    FileSys::DeleteStream(streamName);

    // Add directory as a source
    FileSys::AddResource(streamName, groupDir, activeGroup->Name(), FALSE);

    // Now update resources for active group
    activeGroup->EnumerateResources();
  }


  //
  // ActiveGroup
  //
  // Get the currently active group (FATAL if none setup)
  //
  Group & ActiveGroup()
  {
    ASSERT(activeGroup);
    return (*activeGroup);
  }

  
  //
  // ActiveTypeCount
  //
  // Number of types in the current active group index
  //
  U32 ActiveTypeCount()
  {
    ASSERT(initialized);

    if (!activeGroup)
    {
      ERR_FATAL(("Expected active type, but found none"));
    }

    return (indexCount);
  }


  //
  // GetActiveType
  //
  // Get a type from the active group via index
  //
  Type & GetActiveType(U8 i)
  {
    ASSERT(initialized);

    if (!activeGroup)
    {
      ERR_FATAL(("Expected active type, but found none"));
    }

    if (i >= indexCount)
    {
      ERR_FATAL(("Active type index out of range (%d/%d)", i, indexCount));
    }

    return (*index[i]);
  }


  //
  // GetGroupTree
  //
  // Get the terrain type group tree
  //
  const BinTree<Group> & GetGroupTree()
  {
    ASSERT(initialized);
    return (groupTree);
  }


  //
  // LoadConfig
  //
  // Load the terrain group scope in mission config
  //
  void LoadConfig(FScope *fScope)
  {
    // Get the name of the terrain group to use
    const char *name = fScope->NextArgString();

    // Try and set the default group
    if (!SetDefaultGroup(name))
    {
      LOG_WARN(("Config uses invalid terrain group '%s'", name));
    }
  }

  
  //
  // SaveConfig
  //
  // Save the terrain group scope in mission config
  //
  void SaveConfig(FScope *fScope)
  {
    // Just save active group name
    fScope->AddArgString(ActiveGroup().Name());
  }
}

