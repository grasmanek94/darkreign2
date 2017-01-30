/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset Management
// 25-MAR-1999
//


#ifndef __STRATEGIC_ASSET_MANAGER_H
#define __STRATEGIC_ASSET_MANAGER_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_asset.h"
#include "strategic_asset_request.h"
#include "statemachine.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Forward Declarations
  //
  class Object;


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Manager
  //
  class Asset::Manager
  {
  private:

    // Strategic Object the manager belongs to
    Object *strategic;


    // All the available assets by id
    NBinTree<Asset> assets;

    // Incoming assets
    NBinTree<Asset> newAssets;

    // Asset iterator
    NBinTree<Asset>::Iterator assetIter;


    // Weighting group which contains the orderers
    Weighting::Group<Request> requests;

    // Request iterator
    NBinTree<Request>::Iterator requestIter;


    // Manager State machine
    static StateMachine<Manager> stateMachine; 

    // Manager State machine instance
    StateMachine<Manager>::Instance inst;


    // Flag indicating that we should do a round of matching etc
    Bool flag;

    // Time when we last did matching
    F32 lastTime;


    // Used to load the various request types
    static Request * Loader(FScope *scope, NBinTree<Request> &tree, void *context);

  public:

    // Constructor and Destructor
    Manager();
    ~Manager();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);
    
    // Submit an asset
    void SubmitAsset(Asset &asset);

    // Submit a list of assets
    void SubmitAssets(NList<Asset> &assets);

    // Remove a unit
    void RemoveUnit(UnitObj &unit);

    // Give a unit a direct evaluation
    void Evaluation(Request &request, UnitObj &unit, F32 evaluation);

    // Submit a request
    void SubmitRequest(Request &request, U32 weighting, U32 priority);

    // Perform processing
    void Process();

  public:

    // Setup
    void Setup(Object *object)
    {
      strategic = object;
    }

    // Get the strategic object from the manager
    Object & GetObject()
    {
      ASSERT(strategic)
      return (*strategic);
    }

    // Get the last time we processed
    F32 GetLastTime()
    {
      return (lastTime);
    }

    // Get the assets
    const NBinTree<Asset> & GetAssets()
    {
      return (assets);
    }

    // Get the requests
    const Weighting::Group<Request> & GetRequests()
    {
      return (requests);
    }

  private:

    // State machine functions
    void StateInit();
    void StateEvaluation();
    void StateEvaluationRequest();
    void StateMatching();
    void StateCleanUp();

  public:

    // Initialization and shutdown
    static void Init();
    static void Done();

  };
}

#endif