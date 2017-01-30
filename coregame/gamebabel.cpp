///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 27-APR-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "gamebabel.h"
#include "propobj.h"
#include "unitobj.h"
#include "projectileobj.h"
#include "deliveryprojectileobj.h"
#include "resourceobj.h"
#include "objective.h"
#include "explosionobj.h"
#include "offmapteamobj.h"
#include "offmapbombobj.h"
#include "offmapspawnobj.h"
#include "offmapstrikeobj.h"
#include "restoreobj.h"
#include "spyobj.h"
#include "transportobj.h"
#include "trapobj.h"
#include "wallobj.h"
#include "parasiteobj.h"
#include "markerobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace GameBabel - Translates between identifiers and types
//
namespace GameBabel
{

  //
  // CoreGameBabel
  //
  // Babel for all core game code-classes
  //
  static GameObjType* CoreGameBabel(GameIdent &classId, const char *name, FScope *fScope)
  {
    GameObjType *newType = NULL;

    // Allocate type specified in identifier
    switch (classId.crc)
    {
      case 0x9BC68378: // "Prop"
        newType = new PropObjType(name, fScope); 
        break;

      case 0xD9A182E4: // "Unit"
        newType = new UnitObjType(name, fScope);
        break;
        
      case 0x4CD1BE27: // "Resource"
        newType = new ResourceObjType(name, fScope);
        break;

      case 0xFEA41B2C: // "Projectile"
        newType = new ProjectileObjType(name, fScope);
        break;

      case 0x219C4693: // "Explosion"
        newType = new ExplosionObjType(name, fScope);
        break;
        
      case 0x03515E3F: // "DeliveryProjectile"
        newType = new DeliveryProjectileObjType(name, fScope);
        break;

      case 0x56505D1E: // "Objective"
        newType = new Objective::Type(name, fScope);
        break;

      case 0xF4645984: // "OffMapTeam"
        newType = new OffMapTeamObjType(name, fScope);
        break;

      case 0x39C8F28B: // "OffMapBomb"
        newType = new OffMapBombObjType(name, fScope);
        break;

      case 0x49BDBF48: // "OffMapSpawn"
        newType = new OffMapSpawnObjType(name, fScope);
        break;

      case 0x96519EAE: // "OffMapStrike"
        newType = new OffMapStrikeObjType(name, fScope);
        break;

      case 0x5463CB0D: // "Restore"
        newType = new RestoreObjType(name, fScope);
        break;

      case 0xA98F22B7: // "Spy"
        newType = new SpyObjType(name, fScope);
        break;

      case 0xDFB7F0C8: // "Transport"
        newType = new TransportObjType(name, fScope);
        break;

      case 0x88AD3389: // "Trap"
        newType = new TrapObjType(name, fScope);
        break;

      case 0x93515F2D: // "Wall"
        newType = new WallObjType(name, fScope);
        break;

      case 0xDC7624D3: // "Parasite"
        newType = new ParasiteObjType(name, fScope);
        break;

      case 0xB1A0801B: // "Marker"
        newType = new MarkerObjType(name, fScope);
        break;
    }

    return (newType);
  }


  // Is system initialized
  static Bool sysInit = FALSE;

  // Used for callback registration
  struct BabelData
  {
    // The babel function
    BabelCallBack *callBack;

    // NList node
    NList<BabelData>::Node node;
  };

  // List of all registered babels
  static NList<BabelData> babels(&BabelData::node);
  
  //
  // RegisterBabel
  //
  // Register a babel callback
  //
  void RegisterBabel(BabelCallBack *callBack)
  {
    ASSERT(sysInit);

    // Create new babel data
    BabelData *babel = new BabelData;

    // Set the callback
    babel->callBack = callBack;

    // Add to the list
    babels.Prepend(babel);
  }

  
  //
  // NewGameObjectType
  //
  // Returns a new object type instance, or NULL if the class id is not recognized
  //
  GameObjType* NewGameObjType(GameIdent &classId, const char *name, FScope *fScope)
  {
    ASSERT(sysInit);

    GameObjType *newType = NULL;

    // Step through each registered babel
    for (NList<BabelData>::Iterator i(&babels); *i && !newType; i++)
    {
      // See if this babel knows about this control class
      newType = (*i)->callBack(classId, name, fScope);
    }

    return (newType);
  }

  
  //
  // Init
  //
  // Initialize the game babel system
  //
  void Init()
  {
    ASSERT(!sysInit);

    // System is now initialized
    sysInit = TRUE;

    // Register the core game babel
    RegisterBabel(CoreGameBabel);
  }


  //
  // Done
  //
  // Shutdown the game babel system
  //
  void Done()
  {
    ASSERT(sysInit);

    // Delete all registered babel data
    babels.DisposeAll();

    // System is now shutdown
    sysInit = FALSE;
  }
}
