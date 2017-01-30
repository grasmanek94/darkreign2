///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// mesh statistics
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "statistics.h"
#include "varsys.h"

///////////////////////////////////////////////////////////////////////////////
//
// Namespace Common - Code used in both the game and the studio
//
namespace Statistics
{
  // System initialized flags
  static Bool initialized = FALSE;

  namespace Var
  {
    VarInteger terrainTris;
    VarInteger groundSpriteTris;
    VarInteger spriteTris;

    VarInteger objectTris;
    VarInteger overlayTris;

    VarInteger mrmTris;
    VarInteger nonMRMTris;

    VarInteger ifaceTris;
    VarInteger totalTris;

    VarInteger clipTris;
    VarInteger noClipTris;
  };

  U32 terrainTris;
  U32 groundSpriteTris;
  U32 spriteTris;

  U32 objectTris;
  U32 overlayTris;

  U32 mrmTris;
  U32 nonMRMTris;

  U32 ifaceTris;
  U32 totalTris;

  U32 clipTris;
  U32 noClipTris;

  // used to gather tri data from low level routines
  U32 tempTris;

  void Reset()
  {
//    Var::terrainTris = 0;
//    Var::spriteTris = 0;
//    Var::groundSpriteTris = 0;

//    Var::objectTris = 0;
//    Var::overlayTris = 0;

//    Var::mrmTris = 0;
//    Var::nonMRMTris = 0;

//    Var::ifaceTris = 0;

//    Var::clipTris = 0;
//    Var::noClipTris = 0;

    terrainTris = 0;
    spriteTris = 0;
    groundSpriteTris = 0;

    objectTris = 0;
    overlayTris = 0;

    mrmTris = 0;
    nonMRMTris = 0;

//    ifaceTris = 0;

    clipTris = 0;
    noClipTris = 0;

    tempTris = 0;
  }

  void SetTotal()
  {
    Var::terrainTris      = terrainTris;
    Var::spriteTris       = spriteTris;
    Var::groundSpriteTris = groundSpriteTris;
    Var::objectTris       = objectTris;
    Var::overlayTris      = overlayTris;
    Var::mrmTris          = mrmTris;
    Var::nonMRMTris       = nonMRMTris;
    Var::clipTris         = clipTris;
    Var::noClipTris       = noClipTris;

    totalTris = terrainTris + spriteTris + groundSpriteTris + objectTris + overlayTris + ifaceTris;
  }

  //
  // CmdHandler
  //
  // Handles var system events
  //
  static void CmdHandler(U32 pathCrc)
  {
    pathCrc;

    ASSERT(initialized);
  }

  //
  // Init
  //
  // Initialize this sub-system
  //
  void Init()
  {
    ASSERT(!initialized);

    VarSys::RegisterHandler("statistics", CmdHandler);
    VarSys::RegisterHandler("statistics.tris", CmdHandler);

    // Statistics vars
    VarSys::CreateInteger("statistics.tris.terrain", 0, VarSys::DEFAULT, &Statistics::Var::terrainTris);
    VarSys::CreateInteger("statistics.tris.groundsprite", 0, VarSys::DEFAULT, &Statistics::Var::groundSpriteTris);
    VarSys::CreateInteger("statistics.tris.sprite", 0, VarSys::DEFAULT, &Statistics::Var::spriteTris);

    VarSys::CreateInteger("statistics.tris.object",  0, VarSys::DEFAULT, &Statistics::Var::objectTris);
    VarSys::CreateInteger("statistics.tris.overlay", 0, VarSys::DEFAULT, &Statistics::Var::overlayTris);

    VarSys::CreateInteger("statistics.tris.mrm",    0, VarSys::DEFAULT, &Statistics::Var::mrmTris);
    VarSys::CreateInteger("statistics.tris.nonmrm", 0, VarSys::DEFAULT, &Statistics::Var::nonMRMTris);

    VarSys::CreateInteger("statistics.tris.iface",  0, VarSys::DEFAULT, &Statistics::Var::ifaceTris);
    VarSys::CreateInteger("statistics.tris.total",  0, VarSys::DEFAULT, &Statistics::Var::totalTris);

    VarSys::CreateInteger("statistics.tris.clip",   0, VarSys::DEFAULT, &Statistics::Var::clipTris);
    VarSys::CreateInteger("statistics.tris.noclip", 0, VarSys::DEFAULT, &Statistics::Var::noClipTris);

    initialized = TRUE;
  }


  //
  // Done
  //
  // Shutdown this sub-system
  //
  void Done()
  {
    ASSERT(initialized);

    // Delete the scope
    VarSys::DeleteItem("statistics.tris");
    VarSys::DeleteItem("statistics");

    initialized = FALSE;
  }    
}
