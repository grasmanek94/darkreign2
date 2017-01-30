///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MeshEffect system
//
// 
//

#include "meshent.h"
#include "mesheffect_system.h"

#include "regionobj.h"
#include "console.h"
///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MeshEffectSystem
//
namespace MeshEffectSystem
{
  static Bool sysInit = FALSE;

  // DerivedType structure
  struct DerivedType
  {
    // Identifier for meshEffect type
    GameIdent ident;

    // Identifier of parent
    GameIdent parent;

    // Configuration scope
    FScope *fScope;

    // Default constructor
    DerivedType() : fScope(NULL) {}

    // Destructor
    ~DerivedType()
    {
      if (fScope)
      {
        delete fScope;
      }
    }

  };

  // List of all derived meshEffect classes
  BinTree<DerivedType> derivedMeshEffects;

   // List of all meshEffect types
  BinTree<MeshEffectType> meshEffectTypes;

  // Enable/disable creation of meshEffects
  VarInteger enableMeshEffects;

  NList<MeshEffect> effects;

  //
  // Console Command handler
  //
  void CmdHandler(U32 pathCrc)
  {
    pathCrc;
    ASSERT(sysInit);
  }


  //
  // Initialise meshEffect system
  //
  void Init()
  {
    ASSERT(!sysInit);

    // Register command scope
    VarSys::RegisterHandler("mesheffect", CmdHandler);

    // Create vars
    VarSys::CreateInteger("mesheffect.enabled", TRUE, VarSys::DEFAULT, &enableMeshEffects);

    effects.SetNodeMember(&MeshEffect::listNode);

    sysInit = TRUE;
  }


  //
  // Shutdown meshEffect system
  //
  void Done()
  {
    ASSERT(sysInit);

    // Dispose of derived classes
    derivedMeshEffects.DisposeAll();

    // Dispose of all meshEffect types
    meshEffectTypes.DisposeAll();

    // Remove var scope
    VarSys::DeleteItem("mesheffect");

    sysInit = FALSE;
  }


  //
  // Mission is closing
  //
  void CloseMission()
  {
    ASSERT(sysInit)
  }


  //
  // MeshEffect Simulator Babel
  //
  static MeshEffectType *Create(GameIdent &typeId)
  {
    MeshEffectType *p = NULL;

    switch (typeId.crc)
    {
      case 0x08D8B107: // "TextureCrossFade"
        p = new MeshTextCrossFadeType();
        break;
      case 0xC908780F: // "Color"
        p = new MeshColorType();
        break;
      case 0xE7070C09: // "Scale"
        p = new MeshScaleType();
        break;
      case 0x798B47EF: // "Glow"
        p = new MeshGlowType();
        break;
      case 0x4CD1BE27: // "Resource"
        p = new MeshResourceType();
        break;
      case 0x798A5CF0: // "Plane"
        p = new MeshPlaneType();
        break;
      case 0xEC9692B5: // "PlaneBuild"
        p = new MeshPlaneBuildType();
        break;
      case 0x019CE632: // "LiquidMetal"
        p = new MeshLiquidMetalType();
        break;
    }
    return p;
  }


  //
  // Process a CreateMeshEffectType scope
  //
  MeshEffectType * ProcessCreate(FScope *fScope)
  {
    ASSERT(sysInit);
    ASSERT(fScope);

    // The first argument is the type identifier
    GameIdent typeId = fScope->NextArgString();

    // check if it exists
    MeshEffectType * type = Find( Crc::CalcStr( typeId.str));

    if (!fScope->IsNextArgString())
    {
      if (!type)
      {
        ERR_CONFIG( ("Can't resolve mesh effect type %s", typeId.str) );
      }
      return type;
    }
    // The second argument is the code type name
    GameIdent baseTypeId = fScope->NextArgString();

    // Create the MeshEffect Simulator type
    MeshEffectType * newMeshEffect = NULL;

    if (type)
    {
      ERR_CONFIG( ("Definition scope for exsiting mesh effect type %s", typeId.str) );

      LOG_WARN(("MeshEffect type '%s' already defined, ignoring definition", typeId.str))

      return type;
    }
    else
    {
      if ((newMeshEffect = Create(baseTypeId)) != NULL)
      {
        // its a real base type
        newMeshEffect->Setup(fScope);

        // Add it to list of derived types
        DerivedType *newDerived = new DerivedType;

        newDerived->ident  = typeId;
        newDerived->parent = baseTypeId;
        newDerived->fScope = fScope->Dup();

        derivedMeshEffects.Add(typeId.crc, newDerived);
      }
      else
      {
        // its a derived type
        DerivedType * derived = derivedMeshEffects.Find(baseTypeId.crc);

        if (derived)
        {
          // FIXME: doesnt handle multiple levels of inheritance
          newMeshEffect = Create(derived->parent);
          newMeshEffect->Setup(derived->fScope);
          newMeshEffect->Setup(fScope);
        }
        else
        {
          ERR_CONFIG(("MeshEffect base class [%s] does not exist", baseTypeId.str));
        }
      }
    }

    newMeshEffect->typeId = typeId;

    // Add to list
    meshEffectTypes.Add(typeId.crc, newMeshEffect);

//    LOG_DIAG(("Registered MeshEffect [%s]", typeId.str));

    // Success
    return newMeshEffect;
  }


  //
  // Construct a new meshEffect instance
  //
  MeshEffect * New( MeshEffectType *p, MeshEnt &_ent, F32 _lifeTime, U32 _flags) //  = 0.0f, = Effects::flagDESTROY | Effects::flagLOOP
  {
    ASSERT(p);

    // MeshEffect return old effect if it exists
    //

//    if (enableMeshEffects && !_ent.GetEffect())
    if (_ent.GetEffect())
    {
      // don't let the old effect be deleted
      //
      return _ent.GetEffect();
    }
    else
    {
      return p->Build( _ent, _lifeTime, _flags);
    }
//    else
//    {
//      return NULL;
//    }
  }


  //
  // MeshEffect system post load
  //
  void PostLoad()
  {
    // Post load simulator classes
    for (BinTree<MeshEffectType>::Iterator i(&meshEffectTypes); *i; i++)
    {
      (*i)->PostLoad();
    }
  }


  //
  // Return the type specified
  //
  MeshEffectType *Find(U32 id)
  {
    return (meshEffectTypes.Find(id));
  }

  //
  // ProcessTypeFile
  //
  // Process a type definition file, FALSE if unable to load file
  //
  Bool ProcessTypeFile( const char *name)
  {
    ASSERT(sysInit);
    ASSERT(name);

    PTree tFile;
    FScope *sScope, *fScope;

    // Parse the file
    if (!tFile.AddFile(name))
    {
      return (FALSE);
    }
  
    // Get the global scope
    fScope = tFile.GetGlobalScope();

    // Step through each function in this scope
    while ((sScope = fScope->NextFunction()) != 0)
    {
      ProcessTypeFile( sScope);
    }

    PostLoad();

    return TRUE;
  }


  //
  // ProcessTypeFile
  //
  // Process a type definition file, FALSE if unable to load file
  //
  Bool ProcessTypeFile( FScope *fScope)
  {
    ASSERT(sysInit);
    ASSERT(fScope);

    switch (fScope->NameCrc())
    {
    default:
      return FALSE;

    case 0x17610FAD: // "CreateMeshEffectType"
      MeshEffectSystem::ProcessCreate( fScope);
      break;
    }
    return TRUE;
  } 

  void Process( F32 dt)
  {
    NList<MeshEffect>::Iterator i( &effects);
    while (MeshEffect * effect = *i)
    {
      i++;

      effect->Simulate( dt);
    }
  }
  //----------------------------------------------------------------------------

  U32 MeshEffectSystem::Report( MeshEffectType & effect)
  {
    U32 mem = effect.GetMem();

    CON_DIAG(( "%-36s: %9lu", effect.typeId.str, mem ));
    LOG_DIAG(( "%-36s: %9lu", effect.typeId.str, mem ));

    return mem;
  }
  //----------------------------------------------------------------------------

  U32 MeshEffectSystem::ReportList( const char * name) // = NULL
  {
    U32 mem = 0, count = 0;

    U32 len = name ? strlen( name) : 0;

    BinTree<MeshEffectType>::Iterator i(&meshEffectTypes);
    while (MeshEffectType * type = i++)
    {
      if (!name || !Utils::Strnicmp( name, type->typeId.str, len))
      {
        mem += Report( *type);
        count++;
      }
    }

    CON_DIAG(( "%4ld %-31s: %9lu", count, "mesheffect types", mem ));
    LOG_DIAG(( "%4ld %-31s: %9lu", count, "mesheffect types", mem ));

    return mem;
  }
  //----------------------------------------------------------------------------

  U32 MeshEffectSystem::Report()
  {
    U32 mem = 0, count = 0;

    BinTree<MeshEffectType>::Iterator i(&meshEffectTypes);
    while (MeshEffectType * type = i++)
    {
      mem += type->GetMem();
      count++;
    }

    CON_DIAG(( "%4ld %-31s: %9lu", count, "mesheffect types", mem ));
    LOG_DIAG(( "%4ld %-31s: %9lu", count, "mesheffect types", mem ));

    return mem;
  }
  //----------------------------------------------------------------------------

}
