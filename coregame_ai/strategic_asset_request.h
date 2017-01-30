/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Asset Request
// 25-MAR-1999
//


#ifndef __STRATEGIC_ASSET_REQUEST_H
#define __STRATEGIC_ASSET_REQUEST_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_asset.h"
#include "strategic_config.h"
#include "strategic_weighting.h"
#include "strategic_script.h"
#include "squadobj.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request
  //
  class Asset::Request : public Weighting::Item
  {
  public:

    /////////////////////////////////////////////////////////////////////////////
    //
    // Forward Declarations
    //
    class Tag;
    class Squad;
    class Force;
    class ForceMap;
    class Type;
    class TypeBase;

  private:

    // Manager node
    NBinTree<Request>::Node nodeManager;

    // Cut node
    NBinTree<Request, F32>::Node nodeCut;

    // Tree of assets being evaluated
    BinTree<Asset, F32> evaluation;

    // Iterator used during matching phase
    BinTree<Asset, F32>::Iterator evaluationIter;

    // List of sibling requests
    List<Request> siblings;

  protected:

    // Script the request is for
    Reaper<Script> script;

    // Handle
    U32 handle;

    // List of assets which have been assigned to us
    NList<Asset> assigned;

  public:

    // Constructors
    Request(Script *script, U32 handle);
    Request();

    // Destructor
    virtual ~Request();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    virtual void Reset();

    // Requests can if they like submit assets they want 
    // directly which is more efficient for some requests
    virtual Bool DirectEvaluation(Manager &manager) = 0;

    // Evaluate the given asset
    virtual F32 Evaluate(Asset &asset) = 0;

    // The given asset has been accepted
    virtual void Accept(Asset &asset, Bool primary) = 0;

    // The given asset is being offered, do we want it ?
    virtual Bool Offer(Asset &asset) = 0;

    // The asset manager is informing us there's no more assets
    virtual Bool OutOfAssets(Manager &manager) = 0;

    // Give the assigned units to the squad and notify the 
    // script manager that the request was completed
    void Completed(Manager &manager);

    // Abandon the assigned units and give them back to the
    // strategic asset manager
    void Abandoned(Manager &manager);

    // Get the name of the request
    virtual const char * GetName() = 0;

  public:

    // Get the script this request is for
    Script * GetScript()
    {
      return (script.GetPointer());
    }

  public:

    // Friends of requests
    friend class Manager;

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Tag
  //
  class Asset::Request::Tag : public Asset::Request
  {
  private:

    // Tag to request
    TagObjPtr tag;

  public:

    // Constructors
    Tag(Script *script, U32 handle, TagObj *tag);
    Tag();

    // Destructor
    ~Tag();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    void Reset();

    // Submit asset requests directly to the manager
    Bool DirectEvaluation(Manager &manager);

    // Evaluate the given asset
    F32 Evaluate(Asset &asset);

    // The given asset has been accepted
    void Accept(Asset &asset, Bool primary);

    // The given asset is being offered, do we want it ?
    Bool Offer(Asset &asset);

    // The asset manager is informing us there's no more assets
    Bool OutOfAssets(Manager &manager);

  public:

    // Get the name of this request
    const char * GetName()
    {
      return ("Request::Tag");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Squad
  //
  class Asset::Request::Squad : public Asset::Request
  {
  private:

    // Squad to get them from
    SquadObjPtr from;

  public:

    // Constructors
    Squad(Script *script, U32 handle, SquadObj *from);
    Squad();

    // Destructor
    ~Squad();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    void Reset();

    // Submit asset requests directly to the manager
    Bool DirectEvaluation(Manager &manager);

    // Evaluate the given asset
    F32 Evaluate(Asset &asset);

    // The given asset has been accepted
    void Accept(Asset &asset, Bool primary);

    // The given asset is being offered, do we want it ?
    Bool Offer(Asset &asset);

    // The asset manager is informing us there's no more assets
    Bool OutOfAssets(Manager &manager);

  public:

    // Get the name of this request
    const char * GetName()
    {
      return ("Request::Squad");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Force
  //
  class Asset::Request::Force : public Asset::Request
  {
  private:

    // Force matching config to use
    const Config::RecruitForce *config;

    // Location to perform the analysis
    Point<F32> location;

    // Range to perform evaluation
    F32 range;

    // Threat to the units
    F32 *threat;
    F32 threatTotal;
    F32 threatTotalInv;

    // Defenses opposing the units
    F32 *defense;
    F32 defenseTotal;
    F32 defenseTotalInv;

    // Allocation stats
    F32 *allocThreat;
    F32 *allocDefense;

    // Accept insuffient units
    Bool acceptInsufficient;

    // Allocate and reset data buffers
    void AllocateData();

  public:

    // Constructors
    Force(Script *script, U32 handle, const Config::RecruitForce *config, const Point<F32> &location, F32 range, Bool acceptInsufficient, Object *object);
    Force();

    // Destructor
    ~Force();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    void Reset();

    // Submit asset requests directly to the manager
    Bool DirectEvaluation(Manager &)
    {
      return (FALSE);
    }

    // Evaluate the given asset
    F32 Evaluate(Asset &asset);

    // The given asset has been accepted
    void Accept(Asset &asset, Bool primary);

    // The given asset is being offered, do we want it ?
    Bool Offer(Asset &asset);

    // The asset manager is informing us there's no more assets
    Bool OutOfAssets(Manager &manager);

  public:

    // Get the name of this request
    const char * GetName()
    {
      return ("Request::Force");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::ForceMap
  //
  class Asset::Request::ForceMap : public Asset::Request
  {
  private:

    // Force Map matching config to use
    const Config::RecruitForce *config;

    // Location to perform the analysis
    Point<F32> location;

    // Threat to the units
    F32 *threat;
    F32 threatTotal;
    F32 threatTotalInv;

    // Defenses opposing the units
    F32 *defense;
    F32 defenseTotal;
    F32 defenseTotalInv;

    // Allocation stats
    F32 *allocThreat;
    F32 *allocDefense;

    // Accept insufficient units
    Bool acceptInsufficient;

  public:

    // Constructors
    ForceMap(Script *script, U32 handle, const Config::RecruitForce *config, const Point<F32> &location, Bool acceptInsufficient, Object *object);
    ForceMap();

    // Destructor
    ~ForceMap();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    void Reset();

    // Submit asset requests directly to the manager
    Bool DirectEvaluation(Manager &)
    {
      return (FALSE);
    }

    // Evaluate the given asset
    F32 Evaluate(Asset &asset);

    // The given asset has been accepted
    void Accept(Asset &asset, Bool primary);

    // The given asset is being offered, do we want it ?
    Bool Offer(Asset &asset);

    // The asset manager is informing us there's no more assets
    Bool OutOfAssets(Manager &manager);

  public:

    // Get the name of this request
    const char * GetName()
    {
      return ("Request::ForceMap");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::Type
  //
  class Asset::Request::Type : public Asset::Request
  {
  private:

    // Type config to use
    const Config::RecruitType *config;

    // Location to perform the analysis
    Point<F32> location;

    // Range to consider (stored as the square!)
    F32 range2;
    F32 range2inv;

    // Accept insufficient units
    Bool acceptInsufficient;

    // Types we've got
    BinTree<U32> amounts;

  public:

    // Constructors
    Type(Script *script, U32 handle, const Config::RecruitType *config, const Point<F32> &location, F32 range, Bool acceptInsufficient);
    Type();

    // Destructor
    ~Type();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    void Reset();

    // Submit asset requests directly to the manager
    Bool DirectEvaluation(Manager &)
    {
      return (FALSE);
    }

    // Evaluate the given asset
    F32 Evaluate(Asset &asset);

    // The given asset has been accepted
    void Accept(Asset &asset, Bool primary);

    // The given asset is being offered, do we want it ?
    Bool Offer(Asset &asset);

    // The asset manager is informing us there's no more assets
    Bool OutOfAssets(Manager &manager);

  public:

    // Get the name of this request
    const char * GetName()
    {
      return ("Request::Type");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Asset::Request::TypeBase
  //
  class Asset::Request::TypeBase : public Asset::Request
  {
  private:

    // Type config to use
    const Config::RecruitType *config;

    // The base we're recruiting from
    Reaper<Base> base;

    // Accept insufficient units
    Bool acceptInsufficient;

    // Types we've got
    BinTree<U32> amounts;

  public:

    // Constructors
    TypeBase(Script *script, U32 handle, const Config::RecruitType *config, Base *base, Bool acceptInsufficient);
    TypeBase();

    // Destructor
    ~TypeBase();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope, void *context);

    // Reset the request
    void Reset();

    // Submit asset requests directly to the manager
    Bool DirectEvaluation(Manager &manager);

    // Evaluate the given asset
    F32 Evaluate(Asset &asset);

    // The given asset has been accepted
    void Accept(Asset &asset, Bool primary);

    // The given asset is being offered, do we want it ?
    Bool Offer(Asset &asset);

    // The asset manager is informing us there's no more assets
    Bool OutOfAssets(Manager &manager);

  public:

    // Get the name of this request
    const char * GetName()
    {
      return ("Request::TypeBase");
    }

  };

}
#endif