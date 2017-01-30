///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 17-JUN-1998
//

#ifndef __TERRAINGROUP_H
#define __TERRAINGROUP_H


#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace TerrainGroup - Manages terrain types in groups
//
namespace TerrainGroup
{
  // The blend types
  enum 
  { 
    BLEND_FULL      = 0,
    BLEND_STRAIGHT  = 1,
    BLEND_CORNER    = 2,
    BLEND_BEND      = 3,
    BLEND_COUNT     = 4 
  };

  // Forward declarations
  class Group;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type - A single terrain type record
  //
  class Type
  {
  private:

    // Information for each blend in this type
    class BlendInfo
    {
      // Configured weighting
      struct WeightConfig
      {
        U32 v;
        F32 w;

        WeightConfig(U32 v, F32 w) : v(v), w(w) 
        {
        }
      };

      // List of weight configs
      List<WeightConfig> weightConfigs;

      // Number of actual variations
      U32 variations;

      // Weights for each variation
      F32 *weights;

      // The total weighting value
      F32 totalWeight;

    public:

      // Constructor and destructor
      BlendInfo();
      ~BlendInfo();

      // Add a weight configuration
      void AddWeight(U32 variation, F32 weight);

      // Set the actual number of variations
      void SetVariations(U32 v);

      // Get the number of variations
      U32 GetVariations();

      // Returns a random variation index
      U32 GetRandomVariation();
    };

    // The name of this type
    GameIdent name;

    // The asset name
    GameIdent asset;

    // Group this type belongs to
    Group *group;

    // Group list index (zero is base type)
    U8 index;

    // Index to surface type
    U8 surface;

    // Info on each blend
    BlendInfo blendInfo[BLEND_COUNT];

  public:

    // Constructor
    Type(const char *name, U8 index, const char *surfaceName, const char *asset, Group *group, FScope *fScope);

    // Enumerates available variations
    void EnumerateResources();

    // Returns the number of variations for the given blend
    U32 GetVariations(U32 blend);

    // Returns a random variation for the given blend
    U32 GetRandomVariation(U32 blend);

    // Returns static buffer containing the file name of the given blend
    const char * Type::BlendName(U32 blend, U32 variation);

    // The name of this type
    const char * Name()
    {
      return (name.str);
    }

    // The name of the assets
    const char * Asset()
    {
      return (asset.str);
    }

    // The crc of the name 
    U32 NameCrc()
    {
      return (name.crc);
    }

    // The index
    U8 Index()
    {
      return (index);
    }

    // The surface type
    U8 Surface()
    {
      return (surface);
    }
  };


 
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Group - A single terrain group containing types
  //
  class Group
  {
  private:

    // Name of this terrain type group
    GameIdent name;

    // List of types in this group
    List<Type> list;

  public:

    // Constructor and destructor
    Group(const char *name, FScope *fScope);
    ~Group();

    // Enumerates resources (call each time current group changes)
    void EnumerateResources();

    // Find a particular type
    Type * FindType(U32 crc);
    Type * FindType(const char *name);

    // Add a new terrain type
    void AddType(FScope *fScope);

    // Returns the base type for this group
    Type * BaseType();

    // The name of this group
    const char * Name()
    {
      return (name.str);
    }

    // The crc of the name
    U32 NameCrc()
    {
      return (name.crc);
    }

    // Get the type list
    const List<Type> & GetTypeList()
    {
      return (list);
    }
  };

 
  ///////////////////////////////////////////////////////////////////////////////
  //
  // System Functions
  //

  // Initialize and shutdown system
  void Init();
  void Done();

  // Is the system initialized
  Bool Initialized();

  // Find a group by crc (NULL if not found)
  Group * FindGroup(U32 crc);

  // Find a group by name (NULL if not found)
  Group * FindGroup(const char *name);

  // Set the default group (TRUE if found)
  Bool SetDefaultGroup(const char *name);

  // Get the default or first group (never returns NULL)
  Group & GetDefaultGroup();

  // Set the currently active group
  void SetActiveGroup(Group &group);

  // Get the currently active group (FATAL if none setup)
  Group & ActiveGroup();

  // Number of types in the current active group index
  U32 ActiveTypeCount();

  // Get a type from the active group via index
  Type & GetActiveType(U8 i);

  // Load/Save the terrain group scope in mission config
  void LoadConfig(FScope *fScope);
  void SaveConfig(FScope *fScope);

  // Get the group tree
  const BinTree<Group> & GetGroupTree();
}

#endif