///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client side systems
//
// 11-AUG-1998
//

#ifndef __CLIENT_H
#define __CLIENT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "player.h"
#include "event.h"
#include "mathtypes.h"
#include "mapobjdec.h"
#include "offmapobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Client - Controls client side interface
//
namespace Client
{
  // Initialize and shutdown the system
  void Init();
  void Done();

  // Is the system initialized
  Bool Initialized();

  // Called after configuration files are processed
  void Prepare();

  // Save and load client state
  void Save(FScope *scope);
  void Load(FScope *scope);

  // Returns a reference to the current player
  Player & GetPlayer();

  // Get the object the mouse is over, or NULL (DEBUGGING ONLY!)
  MapObj * MouseOver();
  MapObj * DataObj();

  // Notify the client of a game event (TRUE if accepted)
  Bool Notify(GameObj *from, U32 message, U32 param1 = 0, U32 param2 = 0);

  // Set highlighted construction type
  void SetHiliteConstructType(UnitObjType *type);


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Events
  //
  namespace Events
  {
    // Process each event cycle
    void Process();

    // Set event processing state
    void SetProcessing(Bool state);
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Display - Rendering functions
  //
  namespace Display
  {
    void SetPlayerMarker(const Player *player, S32 cx, S32 cz);
    void ClearPlayerMarker(const Player *player);

    void PreRender();
    void PreTerrain();
    void Render();
    void PostRender();
  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace HUD - Heads up display
  //
  namespace HUD
  {
    // Initialise and shutdown the system
    void Init();
    void Done();

    // Display the HUD
    void Render();

    // Enable or disable the HUG
    void Enable(Bool enable);

    // Return the Color entry associated with a key
    Color GetColorEntry(U32 crc);

    // RenderStatusIcon
    void RenderStatusIcon(U32 crc, const ClipRect &relative);
  }
}

#endif