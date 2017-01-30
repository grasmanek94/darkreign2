/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI
//
// 31-AUG-1998
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_location.h"
#include "strategic_object.h"
#include "strategic_script.h"
#include "strategic_script_manager.h"
#include "tagobj.h"
#include "param.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Location::Base
  //
  class Location::Base
  {
  public:

    // Get the location of the base
    virtual void GetPosition(Vector &position, Object *object, Script *script) = 0;

    // Create a new base location
    static Base * Create(FScope *fScope, Script *script);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Location::Offset
  //
  class Location::Offset
  {
  public:

    // Modify the position by the offset
    virtual void ModifyPosition(Vector &position, Object *object, Script *script) = 0;

    // Create a new offset location
    static Offset * Create(FScope *fScope, Script *script);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Bases
  //
  namespace Bases
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Self
    //
    class Self : public Location::Base
    {
    public:

      // Constructor
      Self(FScope *, Script *)
      {
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *object, Script *script)
      {
        ASSERT(object)
        ASSERT(script)

        // Get the squad associated with this script
        SquadObj * squad = script->GetSquad();

        if (!squad->GetLocation(position))
        {
          #ifdef DEVELOPMENT
            LOG_WARN(("Location::Squad - Squad '%s' was empty and hence its position can't be determined", script->GetName()))
          #endif

          position.x = WorldCtrl::MetreMapX() * 0.5f;
          position.y = -100.0f;
          position.z = WorldCtrl::MetreMapX() * 0.5f;
        }
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Region
    //
    class Region : public Location::Base
    {
    private:

      // Region
      Param::Ident regionName;

    public:

      // Constructor
      Region(FScope *fScope, Script *script)
      : regionName(fScope, script)
      {
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *, Script *)
      {
        RegionObj *region = RegionObj::FindRegion(regionName.GetIdent().str);

        if (region)
        {
          position.x = region->GetMidPoint().x;
          position.z = region->GetMidPoint().z;
          position.y = TerrainData::FindFloor(position.x, position.z);
        }
        else
        {
          ERR_FATAL(("Location::Region - Could not resolve region '%s'", regionName.GetIdent().str))
        }
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tag
    //
    class Tag : public Location::Base
    {
    private:

      // Tag
      Param::Ident tagName;

    public:

      // Constructor
      Tag(FScope *fScope, Script *script)
      : tagName(fScope, script)
      {
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *, Script *)
      {
        TagObj *tag = TagObj::FindTag(tagName.GetIdent().str);

        if (tag)
        {
          if (!tag->GetLocation(position))
          {
            #ifdef DEVELOPMENT
              LOG_WARN(("Location::Tag - Tag '%s' was empty and hence its position can't be determined", tagName.GetIdent().str))
            #endif

            position.x = WorldCtrl::MetreMapX() * 0.5f;
            position.y = -100.0f;
            position.z = WorldCtrl::MetreMapX() * 0.5f;
          }
        }
        else
        {
          ERR_FATAL(("Location::Tag - Could not resolve tag '%s'", tagName.GetIdent().str))
        }
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Squad
    //
    class Squad : public Location::Base
    {
    private:

      // Squad
      Param::Ident squadName;

    public:

      // Constructor
      Squad(FScope *fScope, Script *script)
      : squadName(fScope, script)
      {
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *object, Script *)
      {
        ASSERT(object)

        // Get the script manager from the strategic object and find the script named
        Script * script = object->GetScriptManager().FindScript(squadName.GetIdent());

        if (script)
        {
          // Get the squad associated with this script
          SquadObj * squad = script->GetSquad();

          if (!squad->GetLocation(position))
          {
            #ifdef DEVELOPMENT
              LOG_WARN(("Location::Squad - Squad '%s' was empty and hence its position can't be determined", squadName.GetIdent().str))
            #endif

            position.x = WorldCtrl::MetreMapX() * 0.5f;
            position.y = -100.0f;
            position.z = WorldCtrl::MetreMapX() * 0.5f;
          }
        }
        else
        {
          ERR_FATAL(("Location::Script - Could not find script '%s'", squadName.GetIdent().str))
        }
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Base
    //
    class Base : public Location::Base
    {
    private:

      // Var which contains the cluster index
      Param::Ident base;

    public:

      // Constructor
      Base(FScope *fScope, Script *script)
      : base(fScope, script)
      {
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *object, Script *)
      {
        Strategic::Base *b = object->GetBaseManager().FindBase(base.GetIdent());

        if (b)
        {
          position.x = b->GetLocation().x;
          position.z = b->GetLocation().z;
          position.y = TerrainData::FindFloor(position.x, position.z);
        }
        else
        {
          ERR_FATAL(("Base '%s' does not exist in location", base.GetStr()))
        }
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class VarCluster
    //
    class VarCluster : public Location::Base
    {
    private:

      // Var which contains the cluster index
      Param::Integer clusterIndex;

    public:

      // Constructor
      VarCluster(FScope *fScope, Script *script)
      : clusterIndex(fScope, script)
      {
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *, Script *)
      {
        if (U32(clusterIndex) >= WorldCtrl::ClusterCount())
        {
          ERR_FATAL(("Cluster index %d is out of range %d", clusterIndex.GetS32(), WorldCtrl::ClusterCount()))
        }

        MapCluster *cluster = WorldCtrl::GetCluster(clusterIndex);

        position.x = (cluster->x0 + cluster->x1) * 0.5f;
        position.z = (cluster->z0 + cluster->z1) * 0.5f;
        position.y = TerrainData::FindFloor(position.x, position.z);
      }

    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class LastRadioEvent
    //
    class LastRadioEvent : public Location::Base
    {
    public:

      // Constructor
      LastRadioEvent(FScope *fScope, Script *script)
      {
        fScope;
        script;
      }

      // Get the location of the base
      void GetPosition(Vector &position, Object *, Script *script)
      {
        ASSERT(script)

        // The script stores the location of the last event

        if (!script->GetLocation(position))
        {
          #ifdef DEVELOPMENT
            LOG_WARN(("Location::LastEvent - The last event didn't have a valid location"))
          #endif

          position.x = WorldCtrl::MetreMapX() * 0.5f;
          position.y = -100.0f;
          position.z = WorldCtrl::MetreMapX() * 0.5f;
        }

      }

    };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Offsets
  //
  namespace Offsets
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Polar
    //
    class Polar : public Location::Offset
    {
    private:

      // Distance
      Param::Float distance;

      // Direction 
      Param::Float direction;

    public:

      // Constructor
      Polar(FScope *fScope, Script *script)
      : distance(fScope, script),
        direction(fScope, script)
      {
      }

      // Modify the position by the offset
      void ModifyPosition(Vector &position, Object *, Script *)
      {
        F32 angle = (90.0f - direction) * DEG2RAD;

        position.x += F32(cos(angle)) * distance;
        position.z += F32(sin(angle)) * distance;
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Orthogonal
    //
    class Orthogonal : public Location::Offset
    {
    private:

      // X offset
      Param::Float offsetX;

      // Z offset
      Param::Float offsetZ;

    public:

      // Constructor
      Orthogonal(FScope *fScope, Script *script)
      : offsetX(fScope, script),
        offsetZ(fScope, script)
      {
      }

      // Modify the position by the offset
      void ModifyPosition(Vector &position, Object *, Script *)
      {
        position.x += offsetX;
        position.z += offsetZ;
      }
    };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Location
  //


  //
  // Constructor
  //
  Location::Location(Base *base, Offset *offset)
  : base(base),
    offset(offset)
  {
  }


  //
  // Destructor
  //
  Location::~Location()
  {
    if (base)
    {
      delete base;
    }
    if (offset)
    {
      delete offset;
    }
  }


  //
  // Get the vector position of the location
  //
  Vector Location::GetVector(Object *object, Script *script)
  {
    Vector position;

    ASSERT(base)
    base->GetPosition(position, object, script);

    if (offset)
    {
      offset->ModifyPosition(position, object, script);
    }

    return (position);
  }


  //
  // Get the point position of the location
  //
  Point<F32> Location::GetPoint(Object *object, Script *script)
  {
    Vector position = GetVector(object, script);
    return (Point<F32>(position.x, position.z));
  }


  //
  // Create a location from the given fscope
  //
  Location * Location::Create(FScope *fScope, Script *script)
  {
    Base *base = Base::Create(fScope->GetFunction("Base"), script);
    Offset *offset = Offset::Create(fScope->GetFunction("Offset", FALSE), script);
    return (new Location(base, offset));
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Location::Base
  //

  //
  // Create a new base location
  //
  Location::Base * Location::Base::Create(FScope *fScope, Script *script)
  {
    if (fScope)
    {
      GameIdent type = StdLoad::TypeString(fScope);

      switch (type.crc)
      {
        case 0xC74C77E2: // "Self"
          return (new Bases::Self(fScope, script));

        case 0xB817BF51: // "Region"
          return (new Bases::Region(fScope, script));

        case 0x1E534497: // "Tag"
          return (new Bases::Tag(fScope, script));

        case 0xAF55CD8F: // "Squad"
          return (new Bases::Squad(fScope, script));

        case 0x4BC2F208: // "Base"
          return (new Bases::Base(fScope, script));

        case 0x7EF8BE5E: // "VarCluster"
          return (new Bases::VarCluster(fScope, script));

        case 0x7DE58E6A: // "LastRadioEvent"
          return (new Bases::LastRadioEvent(fScope, script));

        default:
          ERR_FATAL(("Unknown base type '%s'", type.str))
      }
    }
    else
    {
      return (NULL);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Location::Offset
  //

  //
  // Create a new offset
  //
  Location::Offset * Location::Offset::Create(FScope *fScope, Script *script)
  {
    if (fScope)
    {
      GameIdent type = StdLoad::TypeString(fScope);

      switch (type.crc)
      {
        case 0xFE532260: // "Polar"
          return (new Offsets::Polar(fScope, script));

        case 0xD855ECAC: // "Orthogonal"
          return (new Offsets::Orthogonal(fScope, script));

        default:
          ERR_FATAL(("Unknown offset type '%s'", type.str))
      }
    }
    else
    {
      return (NULL);
    }
  }
}

