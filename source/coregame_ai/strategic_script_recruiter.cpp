/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Recruitment
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script_recruiter.h"
#include "strategic_location.h"
#include "strategic_object.h"
#include "param.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{


	/////////////////////////////////////////////////////////////////////////////
	//
  // NameSpace Recruiters
  //
  namespace Recruiters
  {

	  /////////////////////////////////////////////////////////////////////////////
	  //
	  // Class Tag
	  //
    class Tag : public Script::Recruiter
	  {
	  private:

	    // Name of the tag to recruit from
	    Param::Ident tagName;

	  public:

	    // Constructor and Destructor
	    Tag(Script &script, FScope *fScope);
	    ~Tag();

      // Inact the recruiter
      void Execute(Script &script, U32 handle);

	  };


	  /////////////////////////////////////////////////////////////////////////////
	  //
	  // Class Squad
	  //
    class Squad : public Script::Recruiter
	  {
	  private:

	    // Name of the squad script to recruit from
	    GameIdent scriptName;

	  public:

	    // Constructor and Destructor
	    Squad(Script &script, FScope *fScope);
	    ~Squad();

      // Inact the recruiter
      void Execute(Script &script, U32 handle);

	  };


	  /////////////////////////////////////////////////////////////////////////////
	  //
	  // Class Force
	  //
    class Force : public Script::Recruiter
	  {
	  private:

	    // Name of the config to use
      Param::Ident configName;

	    // Location of interest
	    Location *location;

	    // Range to consider
	    F32 range;

	    // Will we accept insufficient
      Param::Integer acceptInsufficient;

	  public:

	    // Constructor and Destructor
	    Force(Script &script, FScope *fScope);
	    ~Force();

      // Inact the recruiter
      void Execute(Script &script, U32 handle);

	  };


	  /////////////////////////////////////////////////////////////////////////////
	  //
	  // Class ForceMap
	  //
    class ForceMap : public Script::Recruiter
	  {
	  private:

	    // Name of the config to use
	    Param::Ident configName;

	    // Location 
	    Location *location;

	    // Will we accept insufficient
	    Param::Integer acceptInsufficient;

	  public:

	    // Constructor and Destructor
	    ForceMap(Script &script, FScope *fScope);
	    ~ForceMap();

      // Inact the recruiter
      void Execute(Script &script, U32 handle);

	  };


	  /////////////////////////////////////////////////////////////////////////////
	  //
	  // Class Type
	  //
    class Type : public Script::Recruiter
	  {
	  private:

	    // Name of the config to use
      Param::Ident configName;

	    // Location
	    Location *location;

      // Range to consider
      Param::Float range;

	    // Will we accept insufficient
	    Param::Integer acceptInsufficient;

	  public:

	    // Constructor and Destructor
	    Type(Script &script, FScope *fScope);
	    ~Type();

      // Inact the recruiter
      void Execute(Script &script, U32 handle);

	  };


	  /////////////////////////////////////////////////////////////////////////////
	  //
	  // Class TypeBase
	  //
    class TypeBase : public Script::Recruiter
	  {
	  private:

	    // Name of the config to use
      Param::Ident configName;

      // Base
      Param::Ident baseName;

	    // Will we accept insufficient
	    Param::Integer acceptInsufficient;

	  public:

	    // Constructor and Destructor
	    TypeBase(Script &script, FScope *fScope);
	    ~TypeBase();

      // Inact the recruiter
      void Execute(Script &script, U32 handle);

	  };

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Script::Recruiter
  //


  //
  // Recruiter::Create
  //
  // Create an action from the given fscope
  //
  Script::Recruiter * Script::Recruiter::Create(Script &script, FScope *fScope)
  {
    ASSERT(fScope)

    const char *type = fScope->NextArgString();

    switch (Crc::CalcStr(type))
    {
      case 0x1E534497: // "Tag"
        return (new Recruiters::Tag(script, fScope));

      case 0xAF55CD8F: // "Squad"
        return (new Recruiters::Squad(script, fScope));

      case 0x2A5732D5: // "Force"
        return (new Recruiters::Force(script, fScope));

      case 0x069BBD20: // "ForceMap"
        return (new Recruiters::ForceMap(script, fScope));

      case 0x1D9D48EC: // "Type"
        return (new Recruiters::Type(script, fScope));

      case 0x27ACF243: // "TypeBase"
        return (new Recruiters::TypeBase(script, fScope));

      default:
        fScope->ScopeError("Unknown recruiter type '%s'", type);
    }
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Recruiters
  //
  namespace Recruiters
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Tag
    //


    //
    // Constructor
    //
    Tag::Tag(Script &script, FScope *fScope)
    : tagName("Tag", fScope, &script)
    {
    }


    //
    // Destructor
    //
    Tag::~Tag()
    {
    }


    //
    // Execute
    //
    void Tag::Execute(Script &script, U32 handle)
    {
      Object &object = script.GetObject();

      // Resolve the tag
      TagObj *tag = TagObj::FindTag(tagName.GetCrc());
      if (!tag)
      {
        ERR_CONFIG(("Could not resolve Tag '%s'", tagName.GetStr()))
      }

      // Submit a request for forces from the tag
      object.GetAssetManager().SubmitRequest(*new Asset::Request::Tag(&script, handle, tag), script.GetWeighting(), script.GetPriority());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Squad
    //


    //
    // Constructor
    //
    Squad::Squad(Script &, FScope *fScope)
    {
      scriptName = StdLoad::TypeString(fScope, "Script");
    }


    //
    // Destructor
    //
    Squad::~Squad()
    {
    }


    //
    // Execute
    //
    void Squad::Execute(Script &script, U32 handle)
    {
      Object &object = script.GetObject();

      // Resolve the script
      Script *s = script.GetManager().FindScript(scriptName);

      if (s)
      {
        // Get the squad from the script
        SquadObj *squad = s->GetSquad(FALSE);

        // The squad may not be alive yet
        if (squad)
        {
          // Submit a request for forces from the force matching
          object.GetAssetManager().SubmitRequest(*new Asset::Request::Squad(&script, handle, squad), script.GetWeighting(), script.GetPriority());
        }
      }
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Force
    //


    //
    // Constructor
    //
    Force::Force(Script &script, FScope *fScope)
    : configName("Config", fScope, &script),
      location(Location::Create(fScope->GetFunction("Location"), &script)),
      range(StdLoad::TypeF32(fScope, "Range", Range<F32>(5.0f, 100.0f))),
      acceptInsufficient("AcceptInsufficient", fScope, FALSE, &script)
    {
    }


    //
    // Destructor
    //
    Force::~Force()
    {
      delete location;
    }


    //
    // Execute
    //
    void Force::Execute(Script &script, U32 handle)
    {
      Object &object = script.GetObject();

      // Resolve the config
      Config::RecruitForce *config = Config::FindRecruitForce(configName.GetCrc());
      if (!config)
      {
        ERR_CONFIG(("Could not find RecruitForce '%s'", configName.GetStr()))
      }

      // Submit a request for forces from the force matching
      object.GetAssetManager().SubmitRequest(*new Asset::Request::Force(&script, handle, config, location->GetPoint(), range, acceptInsufficient, &object), script.GetWeighting(), script.GetPriority());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class ForceMap
    //


    //
    // Constructor
    //
    ForceMap::ForceMap(Script &script, FScope *fScope)
    : configName("Config", fScope, &script),
      location(Location::Create(fScope->GetFunction("Location"), &script)),
      acceptInsufficient("AcceptInsufficient", fScope, FALSE, &script)
    {
    }


    //
    // Destructor
    //
    ForceMap::~ForceMap()
    {
      delete location;
    }


    //
    // Execute
    //
    void ForceMap::Execute(Script &script, U32 handle)
    {
      Object &object = script.GetObject();

      // Resolve the config
      Config::RecruitForce *config = Config::FindRecruitForce(configName.GetCrc());
      if (!config)
      {
        ERR_CONFIG(("Could not find RecruitForce '%s'", configName.GetStr()))
      }

      // Submit a request for forces from the force matching
      object.GetAssetManager().SubmitRequest(*new Asset::Request::ForceMap(&script, handle, config, location->GetPoint(), acceptInsufficient, &object), script.GetWeighting(), script.GetPriority());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Type
    //


    //
    // Constructor
    //
    Type::Type(Script &script, FScope *fScope)
    : configName("Config", fScope, &script),
      location(Location::Create(fScope->GetFunction("Location"), &script)),
      range("Range", fScope, &script),
      acceptInsufficient("AcceptInsufficient", fScope, FALSE, &script)
    {
    }


    //
    // Destructor
    //
    Type::~Type()
    {
      delete location;
    }


    //
    // Execute
    //
    void Type::Execute(Script &script, U32 handle)
    {
      Object &object = script.GetObject();

      // Resolve the config
      Config::RecruitType *config = Config::FindRecruitType(configName.GetCrc());
      if (!config)
      {
        ERR_CONFIG(("Could not find RecruitType '%s'", configName.GetStr()))
      }

      // Submit a request for forces from the force matching
      object.GetAssetManager().SubmitRequest(*new Asset::Request::Type(&script, handle, config, location->GetPoint(), range, acceptInsufficient), script.GetWeighting(), script.GetPriority());
    }


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class TypeBase
    //


    //
    // Constructor
    //
    TypeBase::TypeBase(Script &script, FScope *fScope)
    : configName("Config", fScope, &script),
      baseName("Base", fScope, &script),
      acceptInsufficient("AcceptInsufficient", fScope, FALSE, &script)
    {
    }


    //
    // Destructor
    //
    TypeBase::~TypeBase()
    {
    }


    //
    // Execute
    //
    void TypeBase::Execute(Script &script, U32 handle)
    {
      Object &object = script.GetObject();

      // Resolve the config
      Config::RecruitType *config = Config::FindRecruitType(configName.GetCrc());
      if (!config)
      {
        ERR_CONFIG(("Could not find RecruitType '%s'", configName.GetStr()))
      }

      // Resolve the base
      Base *base = script.GetObject().GetBaseManager().FindBase(baseName);
      if (!base)
      {
        ERR_CONFIG(("Could not find Base '%s'", baseName.GetStr()))
      }

      // Submit a request for forces from the force matching
      object.GetAssetManager().SubmitRequest(*new Asset::Request::TypeBase(&script, handle, config, base, acceptInsufficient), script.GetWeighting(), script.GetPriority());
    }

  }

}
