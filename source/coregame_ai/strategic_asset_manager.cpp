/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset Manager
// 25-MAR-1999
//


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_asset_manager.h"
#include "strategic_object.h"
#include "perfstats.h"


/////////////////////////////////////////////////////////////////////////////
//
// Definitions
//
#define LOG_STATE(x) 


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Manager
  //

  //
  // Internal Data
  //
  StateMachine<Asset::Manager> Asset::Manager::stateMachine;
  const F32 MinScanInterval = 20.0f;


  //
  // Asset::Manager::Manager
  //
  // Constructor
  //
  Asset::Manager::Manager() 
  : assets(&Asset::nodeManager),
    newAssets(&Asset::nodeManager),
    requests(&Request::nodeManager),
    assetIter(&assets),
    requestIter(&requests.GetActive()),
    inst(&stateMachine, "Init"),
    flag(FALSE),
    lastTime(0.0f)
  {
  }


  //
  // Manager::~Manager
  //
  // Destructor
  //
  Asset::Manager::~Manager()
  {
    // Clean up the requests
    requests.CleanUp();

    // Clean up the assets
    assets.DisposeAll();
    newAssets.DisposeAll();
  }


  //
  // Loader
  //
  // Used to load the various request types
  //
  Asset::Request * Asset::Manager::Loader(FScope *scope, NBinTree<Request> &tree, void *context)
  {
    Request *r = NULL;

    // Load the tree key
    U32 key = StdLoad::TypeU32(scope);

    switch (Crc::CalcStr(StdLoad::TypeString(scope)))
    {
      case 0xE8C3BE10: // "Request::Force"
        r = new Request::Force();
        break;

      case 0xD1AE9FC6: // "Request::ForceMap"
        r = new Request::ForceMap();
        break;

      case 0x6DC1414A: // "Request::Squad"
        r = new Request::Squad();
        break;

      case 0xE6BA1813: // "Request::Tag"
        r = new Request::Tag();
        break;

      case 0x5B71DC5D: // "Request::Type"
        r = new Request::Type();
        break;

      case 0xF099D0A5: // "Request::TypeBase"
        r = new Request::TypeBase();
        break;
    }

    ASSERT(r)

    // Must add to the tree before loading state (token base resolution)
    tree.Add(key, r);

    // Now load the state data
    r->LoadState(scope, context);

    return (r);
  }


  //
  // SaveState
  //
  // Save state information
  //
  void Asset::Manager::SaveState(FScope *scope)
  {
    requests.SaveState(scope->AddFunction("Requests"));
    StdSave::TypeF32(scope, "LastTime", lastTime);
  }


  //
  // LoadState
  //
  // Load state information
  //
  void Asset::Manager::LoadState(FScope *scope)
  {
    FScope *sScope;

    while ((sScope = scope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xAD659048: // "Requests"
          requests.LoadState(sScope, Loader, reinterpret_cast<void *>(this));
          break;

        case 0x6AE53035: // "LastTime"
          lastTime = StdLoad::TypeF32(sScope);
          break;
      }
    }

    // Set flag to TRUE so processing is done
    flag = TRUE;
  }


  //
  // SubmitAsset
  //
  // Submit an asset
  //
  void Asset::Manager::SubmitAsset(Asset &asset)
  {
    ASSERT(asset.Alive())
    ASSERT(!asset.nodeManager.InUse())
    ASSERT(!asset.nodeAssigned.InUse())

    if (assets.Find(asset.GetId()) || newAssets.Find(asset.GetId()))
    {
      // Already here, throw this one away
      delete &asset;
      return;
    }

    newAssets.Add(asset.GetId(), &asset);

    // Set process flag
    flag = TRUE;
  }


  //
  // SubmitAssets
  //
  // Submit a list of assets
  //
  void Asset::Manager::SubmitAssets(NList<Asset> &assets)
  {
    // Add each asset in the list
    for (NList<Asset>::Iterator a(&assets); *a; a++)
    {
      SubmitAsset(**a);
    }
  }


  //
  // RemoveUnit
  //
  // Remove a unit
  //
  void Asset::Manager::RemoveUnit(UnitObj &unit)
  {
    // Remove any asset which has this id
    Asset *asset = assets.Find(unit.Id());

    if (asset)
    {
      assets.Dispose(asset);
    }
  }


  //
  // Evaluation
  //
  // Give a unit a direct evaluation
  //
  void Asset::Manager::Evaluation(Request &request, UnitObj &unit, F32 evaluation)
  {
    // Find the asset with this is
    Asset *asset = assets.Find(unit.Id());

    if (asset && evaluation > 0.0f)
    {
      //LOG_AI(("%s: %s evaluated asset %d at %f", request.script.Alive() ? request.script->GetName() : "DEAD", request.GetName(), asset->GetId(), evaluation))

      request.evaluation.Add(evaluation, asset);
      asset->requests.Append(&request);
    }
  }


  //
  // SubmitRequests
  //
  // Submit a request
  //
  void Asset::Manager::SubmitRequest(Request &request, U32 weighting, U32 priority)
  {
//    LOG_AI(("%s: %s submitted", request.script.Alive() ? request.script->GetName() : "DEAD", request.GetName()))

    // Add the request
    requests.AddItem(request, weighting, priority);

    // Set process flag
    flag = TRUE;
  }


  //
  // Process
  //
  // Perform processing
  //
  void Asset::Manager::Process()
  {
    inst.Process(this);
  }


  //
  // StateInit
  //
  void Asset::Manager::StateInit()
  {
    //PERF_S("Init");

    // Do we want to process ?
    if (flag || (GameTime::SimTotalTime() - lastTime) > MinScanInterval)
    {
      // Clear the flag
      flag = FALSE;

      // Save the time we processed
      lastTime = GameTime::SimTotalTime();

      // Are there any new assets ?
      if (newAssets.GetCount())
      {
        // Transfer the new assets to the asset list
        NBinTree<Asset>::Iterator a(&newAssets);

        Asset *asset;
        while ((asset = a++) != NULL)
        {
          if (asset->Alive())
          {
            newAssets.Unlink(asset);
            assets.Add(asset->GetId(), asset);
          }
          else
          {
            LOG_AI(("Found dead asset whilst moving new assets to assets"))
            newAssets.Dispose(asset);
          }
        }
      }

      // Activate all of the idle requests
      requests.ActivateIdle();

      // Are there any requests ?
      if (requests.GetActive().GetCount())
      {
        // Notify the scripts and allow them to add their static requests
        strategic->GetScriptManager().NotifyAsset();

        // For each request, generate the list of siblings
        NBinTree<Request>::Iterator r(&requests.GetActive());
        Request *request;
        while ((request = r++) != NULL)
        {
          // Reset the request
          request->Reset();

          for (NBinTree<Request>::Iterator q(&requests.GetActive()); *q; q++)
          {
            if (
              (request != *q) && 
              (request->script.DirectId() == 
              (*q)->script.DirectId()))
            {
//              LOG_AI(("%s: %s found sibling %s %08Xh", request->script.Alive() ? request->script->GetName() : "DEAD", request->GetName(), (*q)->GetName(), *q))
              request->siblings.Append(*q);
            }
          }
        }

        // Reset the request iterator
        !requestIter;

        // Reset usage of requests
        requests.Clear(1);

        // Reset assets and remove dead ones
        NBinTree<Asset>::Iterator a(&assets);
        Asset *asset;
        while ((asset = a++) != NULL)
        {
          if (asset->Alive())
          {
            asset->requests.UnlinkAll();
          }
          else
          {
            assets.Dispose(asset);
          }
        }

        // Proceed to the evaluation phase
        inst.Set(0x4CF7934B); // "Evaluation"

        // Clear the flag again since it may have gotten set again
        flag = FALSE;
      }
    }

    //PERF_E("Init");
  }


  //
  // StateEvaluation
  //
  void Asset::Manager::StateEvaluation()
  {
    //PERF_S("Evaluation");

    LOG_STATE(("StateEvaluation"))

    // Are there anymore requests ?
    if (*requestIter)
    {
      Request *request = *requestIter;

      if (request->DirectEvaluation(*this))
      {
        // The request has completed its evaluation, set its iterator to the end
        request->evaluationIter.GoToEnd();

        // Proceed to the next request
        requestIter++;

        //PERF_E("Evaluation");
        return;
      }
      else
      {
        // Reset the asset iterator
        assetIter.SetTree(&assets);

        // Proceed to the evaluation request phase
        inst.Set(0xA4CEC980); // "EvaluationRequest"
      }
    }
    else
    {
      // Evaluation phase is complete, proceed to matching phase
      inst.Set(0x01CF769E); // "Matching"
    }

    //PERF_E("Evaluation");
  }


  //
  // StateEvaluationRequest
  //
  void Asset::Manager::StateEvaluationRequest()
  {
    //PERF_S("EvaluationRequest");

    LOG_STATE(("StateEvaluationRequest"))

    Request *request = *requestIter;
    ASSERT(request)

    // Are there anymore assets ?
    if (*assetIter)
    {
      Asset *asset = assetIter++;

      // Is the asset alive ?
      if (asset->Alive())
      {
        // Get the request to evaluate this asset
        F32 evaluation = request->Evaluate(*asset);

        if (evaluation > 0.0f)
        {
//          LOG_AI(("%s: %s evaluated asset %d at %f", request->script.Alive() ? request->script->GetName() : "DEAD", request->GetName(), asset->GetId(), evaluation))
        }

        // If the evaluation resulted in a score above zero (0)
        // then add this object to the evaluation tree and add
        // the request to the the assets list for reverse mapping
        if (evaluation > 0.0f)
        {
          request->evaluation.Add(evaluation, asset);
          asset->requests.Append(request);
        }
      }
    }
    else
    {
      // The request has completed its evaluation, set its iterator to the end
      request->evaluationIter.GoToEnd();

      // Proceed to the next request
      requestIter++;

      // Go back to the evaluation state
      inst.Set(0x4CF7934B); // "Evaluation"
    }

    //PERF_E("EvaluationRequest");
  }


  //
  // StateMatching
  //
  void Asset::Manager::StateMatching()
  {
    //PERF_S("Matching");

    LOG_STATE(("StateMatching"))


    // Are we done ?
    if (!requests.GetActive().GetCount())
    {
      // If we are done then proceed to the CleanUp phase
      inst.Set(0x345DE90A); // "CleanUp"

      //PERF_E("Matching");
      return;
    }

    Weighting::Cuts<Request> cuts(&Request::nodeCut);

    // Sort the requests into those which have more 
    // than they need and those which have less
    requests.Process(cuts);

    // Offer the requests which are below their cut the assets they want
    Request *request;
    Weighting::Cuts<Request>::Item *item = cuts.items.GetFirst();

    request = item->belowCut.GetFirst();
    if (!request)
    {
      request = item->aboveCut.GetFirst();
    }
    ASSERT(request)

    cuts.Clear();

    // Is this request interested in any more units ?
    while (*request->evaluationIter)
    {
      Asset *asset = *request->evaluationIter;

      // Ignore any assets which are dead
      if (!asset->Alive())
      {
        request->evaluationIter--;
        continue;
      }

      // Ignore any assets which have been assigned
      if (asset->IsAssigned())
      {
        request->evaluationIter--;
        continue;
      }

/*
      LOG_AI(("%s: %s [%d:%d]-[%d:%d] is offered asset %d", 
        request->script.Alive() ? request->script->GetName() : "DEAD",
        request->GetName(), 
        requests.GetItemWeighting(*request), 
        requests.GetWeighting(),
        requests.GetItemUsage(*request), 
        requests.GetUsage(),
        asset->GetId()))
*/

      // Offer this asset to the request
      if (request->Offer(*asset))
      {
//        LOG_AI(("%s: %s accepted %d", request->script.Alive() ? request->script->GetName() : "DEAD", request->GetName(), asset->GetId()))

        // Update the usage of this request
        requests.AddUsage(*request, 1);

        // The request has accepted the asset
        request->Accept(*asset, TRUE);

        // The siblings need to be notified of the asset too
        for (List<Request>::Iterator r(&request->siblings); *r; r++)
        {
          (*r)->Accept(*asset, FALSE);
        }

        // Asset was accepted ...
        //PERF_E("Matching");
        return;
      }
      else
      {
//        LOG_AI(("%s: %s rejected %d", request->script.Alive() ? request->script->GetName() : "DEAD", request->GetName(), asset->GetId()))
      }

      // Move request's iterator to next pick
      request->evaluationIter--;
    }

    // If the request has exhausted all of the assets it's interested in then set it to idle
    requests.DeactivateItem(*request);

    //PERF_E("Matching");
  }


  //
  // StateCleanUp
  //
  void Asset::Manager::StateCleanUp()
  {
    //PERF_S("CleanUp");

    LOG_STATE(("StateCleanUp"))

    // At this point ALL of the requests should be idle
    ASSERT(!requests.GetActive().GetCount())
    Request *request;
    NBinTree<Request>::Iterator r(&requests.GetIdle()); 

    while ((request = r++) != NULL)
    {
      if (request->OutOfAssets(*this))
      {
//        LOG_AI(("%s: %s proceeding", request->script.Alive() ? request->script->GetName() : "DEAD", request->GetName()))
        requests.RemoveItem(*request);
        delete request;
      }
      else
      {
//        LOG_AI(("%s: %s hasn't got enough to proceed", request->script.Alive() ? request->script->GetName() : "DEAD", request->GetName()))
      }
    }

    // All done, return to the Init state
    inst.Set(0xABAA7B48); // "Init"

    //PERF_E("CleanUp");
  }


  //
  // Init
  //
  // Initialization
  //
  void Asset::Manager::Init()
  {
    stateMachine.AddState("Init", &StateInit);
    stateMachine.AddState("Evaluation", &StateEvaluation);
    stateMachine.AddState("EvaluationRequest", &StateEvaluationRequest);
    stateMachine.AddState("Matching", &StateMatching);
    stateMachine.AddState("CleanUp", &StateCleanUp);
  }


  //
  // Done
  //
  // Shutdown
  //
  void Asset::Manager::Done()
  {
    // Cleanup the state machine
    stateMachine.CleanUp();
  }

}
