///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team List Editor
//
// 24-NOV-1998
//


#ifndef __AI_DEBUG_INFO_H
#define __AI_DEBUG_INFO_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai_debug.h"
#include "icwindow.h"
#include "iclistbox.h"
#include "strategic_base.h"
#include "strategic_asset.h"


///////////////////////////////////////////////////////////////////////////////
//
// Forward Declarations
//
class Team;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace AI
//
namespace AI
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Debug
  //
  namespace Debug
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Info
    //
    class Info : public ICWindow
    {
      PROMOTE_LINK(TeamList, ICWindow, 0x9A7BBCB9) // "AI::Debug::Info"

    protected:

      Strategic::Object *object;

      IFaceVar *powerCurrentConsumed;
      IFaceVar *powerCurrentProduced;
      IFaceVar *powerCurrentProducedDay;
      IFaceVar *powerCurrentProducedNight;

      IFaceVar *powerOnOrderConsumed;
      IFaceVar *powerOnOrderProducedDay;
      IFaceVar *powerOnOrderProducedNight;

      IFaceVar *powerInConstructionConsumed;
      IFaceVar *powerInConstructionProducedDay;
      IFaceVar *powerInConstructionProducedNight;

      IFaceVar *resourceCurrent;
      IFaceVar *resourceOnOrder;
      IFaceVar *resourceInConstruction;
      IFaceVar *resourceAvailable;

      ICListBoxPtr baseList;
      ICListBoxPtr bombardierList;
      ICListBoxPtr resourceList;
      ICListBoxPtr scriptList;
      ICListBoxPtr assetList;
      ICListBoxPtr objectiveList;

    public:

      // Constructor
      Info(IControl *parent);
      ~Info();

      // Activate and deactivate this control
      Bool Activate();
      Bool Deactivate();

      // Poll
      void Poll();

    private:

      // Add base
      void AddBase(Strategic::Base &base);

      // Add orderer
      void AddOrderer(Strategic::Base::Orderer &orderer, U32 weight, U32 totalWeight, U32 usage, U32 totalUsage);

      // Add token
      void AddToken(Strategic::Base::Token &token, U32 indent);

      // Add asset request
      void AddAssetRequest(Strategic::Asset::Request &request);

    public:

      // SetObject
      void SetObject(Strategic::Object *o)
      {
        object = o;
      }

    };
  }
}

#endif
