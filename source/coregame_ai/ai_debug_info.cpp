///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ai_debug_info.h"
#include "ifvar.h"
#include "iface.h"
#include "team.h"
#include "strategic_object.h"
#include "strategic_base_state.h"
#include "strategic_base_token.h"
#include "strategic_script.h"
#include "strategic_script_state.h"


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

    //
    // Constructor
    //
    Info::Info(IControl *parent) 
    : ICWindow(parent),
      object(NULL)
    {
      powerCurrentConsumed = new IFaceVar(this, CreateInteger("powerCurrentConsumed", 0));
      powerCurrentProduced = new IFaceVar(this, CreateInteger("powerCurrentProduced", 0));
      powerCurrentProducedDay = new IFaceVar(this, CreateInteger("powerCurrentProducedDay", 0));
      powerCurrentProducedNight = new IFaceVar(this, CreateInteger("powerCurrentProducedNight", 0));

      powerOnOrderConsumed = new IFaceVar(this, CreateInteger("powerOnOrderConsumed", 0));
      powerOnOrderProducedDay = new IFaceVar(this, CreateInteger("powerOnOrderProducedDay", 0));
      powerOnOrderProducedNight = new IFaceVar(this, CreateInteger("powerOnOrderProducedNight", 0));

      powerInConstructionConsumed = new IFaceVar(this, CreateInteger("powerInConstructionConsumed", 0));
      powerInConstructionProducedDay = new IFaceVar(this, CreateInteger("powerInConstructionProducedDay", 0));
      powerInConstructionProducedNight = new IFaceVar(this, CreateInteger("powerInConstructionProducedNight", 0));
      
      resourceCurrent = new IFaceVar(this, CreateInteger("resourceCurrent", 0));
      resourceOnOrder = new IFaceVar(this, CreateInteger("resourceOnOrder", 0));
      resourceInConstruction = new IFaceVar(this, CreateInteger("resourceInConstruction", 0));
      resourceAvailable = new IFaceVar(this, CreateInteger("resourceAvailable", 0));

      pollInterval = 2000;
      nextPollTime = pollInterval;
    }


    //
    // Destructor
    //
    Info::~Info()
    {
      delete powerCurrentConsumed;
      delete powerCurrentProduced;
      delete powerCurrentProducedDay;
      delete powerCurrentProducedNight;

      delete powerOnOrderConsumed;
      delete powerOnOrderProducedDay;
      delete powerOnOrderProducedNight;

      delete powerInConstructionConsumed;
      delete powerInConstructionProducedDay;
      delete powerInConstructionProducedNight;

      delete resourceCurrent;
      delete resourceOnOrder;
      delete resourceInConstruction;
      delete resourceAvailable;
    }


    //
    // Activate
    //
    // Activate this control
    //
    Bool Info::Activate()
    {
      if (ICWindow::Activate())
      {
        ActivateVar(powerCurrentConsumed);
        ActivateVar(powerCurrentProduced);
        ActivateVar(powerCurrentProducedDay);
        ActivateVar(powerCurrentProducedNight);

        ActivateVar(powerOnOrderConsumed);
        ActivateVar(powerOnOrderProducedDay);
        ActivateVar(powerOnOrderProducedNight);

        ActivateVar(powerInConstructionConsumed);
        ActivateVar(powerInConstructionProducedDay);
        ActivateVar(powerInConstructionProducedNight);

        ActivateVar(resourceCurrent);
        ActivateVar(resourceOnOrder);
        ActivateVar(resourceInConstruction);
        ActivateVar(resourceAvailable);
 
        baseList = IFace::Find<ICListBox>("Config.Bases.Bases", this, TRUE);
        bombardierList = IFace::Find<ICListBox>("Config.Bombardiers.Bombardiers", this, TRUE);
        resourceList = IFace::Find<ICListBox>("Config.Resources.Resources", this, TRUE);
        scriptList = IFace::Find<ICListBox>("Config.Scripts.Scripts", this, TRUE);
        assetList = IFace::Find<ICListBox>("Config.Asset.Asset", this, TRUE);
        objectiveList = IFace::Find<ICListBox>("Config.Objectives.Objectives", this, TRUE);

        return (TRUE);
      }

      return (FALSE);
    }


    //
    // Deactivate
    //
    // Deactivate this control
    //
    Bool Info::Deactivate()
    {
      if (ICWindow::Deactivate())
      {
        powerCurrentConsumed->Deactivate();
        powerCurrentProduced->Deactivate();
        powerCurrentProducedDay->Deactivate();
        powerCurrentProducedNight->Deactivate();

        powerOnOrderConsumed->Deactivate();
        powerOnOrderProducedDay->Deactivate();
        powerOnOrderProducedNight->Deactivate();

        powerInConstructionConsumed->Deactivate();
        powerInConstructionProducedDay->Deactivate();
        powerInConstructionProducedNight->Deactivate();

        resourceCurrent->Deactivate();
        resourceOnOrder->Deactivate();
        resourceInConstruction->Deactivate();
        resourceAvailable->Deactivate();

        // Destroy this control
        MarkForDeletion();

        return (TRUE);
      }

      return (FALSE);
    }


    //
    // Poll
    //
    void Info::Poll()
    {
      if (object)
      {
        Team *team = object->GetTeam();
        ASSERT(team)

        powerCurrentConsumed->SetIntegerValue(team->GetPower().GetConsumed());
        powerCurrentProduced->SetIntegerValue(team->GetPower().GetAvailable());
        powerCurrentProducedDay->SetIntegerValue(team->GetPower().GetAvailableDay());
        powerCurrentProducedNight->SetIntegerValue(team->GetPower().GetAvailableNight());

        powerOnOrderConsumed->SetIntegerValue(object->GetBaseManager().GetOnOrderStats().powerConsumption);
        powerOnOrderProducedDay->SetIntegerValue(object->GetBaseManager().GetOnOrderStats().powerDaySupply);
        powerOnOrderProducedNight->SetIntegerValue(object->GetBaseManager().GetOnOrderStats().powerNightSupply);

        powerInConstructionConsumed->SetIntegerValue(object->GetBaseManager().GetInConstructionStats().powerConsumption);
        powerInConstructionProducedDay->SetIntegerValue(object->GetBaseManager().GetInConstructionStats().powerDaySupply);
        powerInConstructionProducedNight->SetIntegerValue(object->GetBaseManager().GetInConstructionStats().powerNightSupply);

        resourceCurrent->SetIntegerValue(team->GetResourceStore());
        resourceOnOrder->SetIntegerValue(object->GetBaseManager().GetOnOrderStats().resource);
        resourceInConstruction->SetIntegerValue(object->GetBaseManager().GetInConstructionStats().resource);
        resourceAvailable->SetIntegerValue(team->GetResourceStore() - object->GetBaseManager().GetInConstructionStats().resource);

        const U32 buffSize = 128;
        char buff[buffSize];

        // Base List
        if (baseList->IsActive())
        {
          ICListBox::Rebuild *rebuild = baseList->PreRebuild();

          baseList->DeleteAllItems();

          const Strategic::Weighting::Group<Strategic::Base> &bases = object->GetBaseManager().GetBases();

          Utils::Sprintf(buff, buffSize, "Active Bases: %d", bases.GetActive().GetCount());
          baseList->AddTextItem(buff, NULL);
          Utils::Sprintf(buff, buffSize, "Idle Bases: %d", bases.GetIdle().GetCount());
          baseList->AddTextItem(buff, NULL);

          for (NBinTree<Strategic::Base>::Iterator a(&bases.GetActive()); *a; a++)
          {
            AddBase(**a);
          }
          for (NBinTree<Strategic::Base>::Iterator i(&bases.GetIdle()); *i; i++)
          {
            AddBase(**i);
          }

          baseList->PostRebuild(&rebuild);
        }

        // Script List
        if (scriptList->IsActive())
        {
          ICListBox::Rebuild *rebuild = scriptList->PreRebuild();

          scriptList->DeleteAllItems();

          const NBinTree<Strategic::Script> &scripts = object->GetScriptManager().GetScripts();

          Utils::Sprintf(buff, buffSize, "Scripts: %d", scripts.GetCount());
          scriptList->AddTextItem(buff, NULL);

          for (NBinTree<Strategic::Script>::Iterator s(&scripts); *s; s++)
          {
            Strategic::Script &script = **s;
            
            Color color(1.0f, 1.0f, 1.0f);
            scriptList->AddTextItem(script.GetName(), NULL, &color);

            color = Color(0.5f, 1.0f, 1.0f);

            Utils::Sprintf(buff, buffSize, "Config: %s", script.GetConfigName());
            scriptList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

            Strategic::Script::State *state = script.GetCurrentState();

            if (state)
            {
              Utils::Sprintf(buff, buffSize, "State: %s", state->GetName());
              scriptList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));
            }

            SquadObj *squad = script.GetSquad(FALSE);
            if (squad)
            {
              Utils::Sprintf(buff, buffSize, "Squad: %d", squad->Id());
              scriptList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

              Utils::Sprintf(buff, buffSize, "Members: %d", squad->GetList().GetCount());
              scriptList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

              SquadObj::UnitList::Iterator i(&squad->GetList());
              for (!i; *i; i++)
              {
                if ((*i)->Alive())
                {
                  color = Color(0.5f, 0.5f, 1.0f);
                  Utils::Sprintf(buff, buffSize, "[%08d] - %s", (*i)->DirectId(), (**i)->TypeName());
                  scriptList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
                }
                else
                {
                  color = Color(1.0f, 0.5f, 0.5f);
                  Utils::Sprintf(buff, buffSize, "[%08d] - DEAD", (*i)->DirectId());
                  scriptList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
                }
              }
            }
          }

          scriptList->PostRebuild(&rebuild);
        }

        // Bombardier List
        if (bombardierList->IsActive())
        {
          ICListBox::Rebuild *rebuild = bombardierList->PreRebuild();
          bombardierList->DeleteAllItems();

          // Add all of the bombardiers
          const NList<Strategic::Bombardier> &bombardiers = object->GetBombardierManager().GetBombardiers();

          Utils::Sprintf(buff, buffSize, "Bombardiers: %d", bombardiers.GetCount());
          Color color(1.0f, 1.0f, 1.0f);
          bombardierList->AddTextItem(buff, NULL, &color);

          for (NList<Strategic::Bombardier>::Iterator b(&bombardiers); *b; b++)
          {
            if ((*b)->GetUnit())
            {
              Color color(0.5f, 0.8f, 1.0f);
              Utils::Sprintf(buff, buffSize, "[%08d] '%s'", 
                (*b)->GetUnit()->Id(), 
                (*b)->GetUnit()->TypeName());
              bombardierList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

              color = Color(0.5f, 1.0f, 0.8f);
              Utils::Sprintf(buff, buffSize, "Type: %s", (*b)->GetName());
              bombardierList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));

              color = Color(1.0f, 0.5f, 0.8f);
              Utils::Sprintf(buff, buffSize, "Info: %s", (*b)->GetInfo());
              bombardierList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
            }
            else
            {
              Color color(1.0f, 0.5f, 0.5f);
              bombardierList->AddTextItem("DEAD", NULL, &color, Point<S32>(10, 0));
            }
          }

          bombardierList->PostRebuild(&rebuild);
        }

        // Resource List
        if (resourceList->IsActive())
        {
          ICListBox::Rebuild *rebuild = resourceList->PreRebuild();

          resourceList->DeleteAllItems();

          // Add all of the resources
          for (NBinTree<Strategic::Resource, F32>::Iterator r(&object->GetResourceManager().GetResources()); *r; ++r)
          {
            Color color(1.0f, 0.2f, 0.5f);
            resourceList->AddTextItem("Resource", NULL, &color);

            color = Color(1.0f, 1.0f, 1.0f);

            // Put in the resource information
            Utils::Sprintf(buff, buffSize, "Total Resource: %d", (*r)->GetResource());
            resourceList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

            Utils::Sprintf(buff, buffSize, "Total Regeneration: %d", (*r)->GetRegen());
            resourceList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

            Utils::Sprintf(buff, buffSize, "Base Proximity: %f", (*r)->GetBaseProximity());
            resourceList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

            Utils::Sprintf(buff, buffSize, "Update Cycle: %d", (*r)->GetUpdateCycle());
            resourceList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

            resourceList->AddTextItem("Clusters:", NULL, &color, Point<S32>(10, 0));

            for (List<MapCluster>::Iterator c(&(*r)->GetClusters()); *c; c++)
            {
              Color color(0.2f, 1.0f, 0.5f);
              Utils::Sprintf(buff, buffSize, "%03d, %03d", (*c)->xIndex, (*c)->zIndex);
              resourceList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
            }

            resourceList->AddTextItem("Resource Storage:", NULL, &color, Point<S32>(10, 0));

            for (UnitObjList::Iterator s(&(*r)->GetResourceStorers()); *s; ++s)
            {
              if ((*s)->Alive())
              {
                Utils::Sprintf(buff, buffSize, "[%08d] %s", (**s)->Id(), (**s)->TypeName());
              }
              else
              {
                Utils::Sprintf(buff, buffSize, "[%08d] DEAD", (*s)->DirectId());
              }
              Color color(0.2f, 0.5f, 1.0f);
              resourceList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
            }

            resourceList->AddTextItem("Resource Transports:", NULL, &color, Point<S32>(10, 0));

            for (UnitObjList::Iterator t(&(*r)->GetResourceTransports()); *t; ++t)
            {
              if ((*t)->Alive())
              {
                Utils::Sprintf(buff, buffSize, "[%08d] %s", (**t)->Id(), (**t)->TypeName());
              }
              else
              {
                Utils::Sprintf(buff, buffSize, "[%08d] DEAD", (*t)->DirectId());
              }
              Color color(0.2f, 0.5f, 1.0f);
              resourceList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
            }

            resourceList->AddTextItem("Units:", NULL, &color, Point<S32>(10, 0));

            for (UnitObjList::Iterator u(&(*r)->GetUnits()); *u; ++u)
            {
              if ((*u)->Alive())
              {
                Utils::Sprintf(buff, buffSize, "[%08d] %s", (**u)->Id(), (**u)->TypeName());
              }
              else
              {
                Utils::Sprintf(buff, buffSize, "[%08d] DEAD", (*u)->DirectId());
              }
              Color color(0.2f, 0.5f, 1.0f);
              resourceList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
            }

            resourceList->AddTextItem("OnOrder:", NULL, &color, Point<S32>(10, 0));

            for (UnitObjTypeList::Iterator o(&(*r)->GetOnOrder()); *o; ++o)
            {
              Utils::Sprintf(buff, buffSize, "%s", (**o)->GetName());
              Color color(1.0f, 1.0f, 0.2f);
              resourceList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
            }
          }

          resourceList->PostRebuild(&rebuild);
        }

        // Asset list
        if (assetList->IsActive())
        {
          Color color(1.0f, 1.0f, 1.0f);

          ICListBox::Rebuild *rebuild = objectiveList->PreRebuild();

          assetList->DeleteAllItems();

          // Get the asset manager
          Strategic::Asset::Manager &manager = object->GetAssetManager();

          // General Info
          Utils::Sprintf(buff, buffSize, "Last Update: %f", manager.GetLastTime()); 
          assetList->AddTextItem(buff, NULL, &color);
          
          // Requests
          assetList->AddTextItem("Requests:", NULL, &color);
          const Strategic::Weighting::Group<Strategic::Asset::Request> &requests = manager.GetRequests();
          for (NBinTree<Strategic::Asset::Request>::Iterator r(&requests.GetActive()); *r; ++r)
          {
            AddAssetRequest(**r);
          }
          for (NBinTree<Strategic::Asset::Request>::Iterator i(&requests.GetIdle()); *i; ++i)
          {
            AddAssetRequest(**i);
          }

          // Assets
          assetList->AddTextItem("Assets:", NULL, &color);
          for (NBinTree<Strategic::Asset>::Iterator a(&manager.GetAssets()); *a; ++a)
          {
            // Add the information about the asset
            
          }

          assetList->PostRebuild(&rebuild);
        }

        // Objective List
        if (objectiveList->IsActive())
        {
          ICListBox::Rebuild *rebuild = objectiveList->PreRebuild();

          objectiveList->DeleteAllItems();

          Color color(1.0f, 1.0f, 1.0f);

          Objective::ObjList::Iterator i(&team->GetCurrentObjectives());
          objectiveList->AddTextItem("Current Objectives:", NULL, &color);

          color = Color(1.0f, 1.0f, 0.3f);

          for (!i; *i; i++)
          {
            objectiveList->AddTextItem((**i)->GetType()->GetName(), NULL, &color);
          }
                    
          objectiveList->PostRebuild(&rebuild);
        }
      }
    }


    //
    // Add base
    //
    void Info::AddBase(Strategic::Base &base)
    {
      Color color = base.idle ? Color(0.5f, 0.7f, 0.7f) : Color(1.0f, 1.0f, 1.0f);

      char buff[64];

      baseList->AddTextItem(base.GetName(), NULL, &color, Point<S32>(10, 0));

      if (base.GetManager().GetPrimaryBase() == &base)
      {
        baseList->AddTextItem("Primary Base", NULL, &color, Point<S32>(10, 0));
      }

      Utils::Sprintf(buff, 64, "Units: %d", base.GetNumUnits());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(15, 0));

      {
        for (UnitObjList::Iterator u(&base.GetUnits()); *u; u++)
        {
          Color color(0.5f, 0.7f, 0.5f);

          if ((*u)->Alive())
          {
            Utils::Sprintf(buff, 64, "[%08d] : %s", (**u)->Id(), (**u)->TypeName());
            baseList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
          }
        }
      }

      Utils::Sprintf(buff, 64, "Constructors Idle: %d", base.GetNumConstructorsIdle());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(15, 0));

      {
        for (UnitObjList::Iterator u(&base.GetConstructorsIdle()); *u; u++)
        {
          Color color(0.7f, 0.5f, 0.5f);

          if ((*u)->Alive())
          {
            Utils::Sprintf(buff, 64, "[%08d] : %s", (**u)->Id(), (**u)->TypeName());
            baseList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
          }
        }
      }

      Utils::Sprintf(buff, 64, "Constructors Working: %d", base.GetNumConstructorsWorking());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(15, 0));

      {
        for (UnitObjList::Iterator u(&base.GetConstructorsWorking()); *u; u++)
        {
          Color color(0.5f, 0.5f, 0.7f);

          if ((*u)->Alive())
          {
            Utils::Sprintf(buff, 64, "[%08d] : %s", (**u)->Id(), (**u)->TypeName());
            baseList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));
          }
        }
      }

      if (base.GetCurrentState())
      {
        Utils::Sprintf(buff, 64, "State: %s", base.GetCurrentState()->GetName());
        baseList->AddTextItem(buff, NULL, &color, Point<S32>(15, 0));

        Strategic::Weighting::Cuts<Strategic::Base::Orderer> cuts(&Strategic::Base::Orderer::nodeCut);
        const Strategic::Weighting::Group<Strategic::Base::Orderer> &orderers = base.GetCurrentState()->GetOrderers();

        Utils::Sprintf(buff, 64, "Orderers: Idle[%d] Active[%d]", orderers.GetIdle().GetCount(), orderers.GetActive().GetCount());
        baseList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));

        // Idle
        for (NBinTree<Strategic::Base::Orderer>::Iterator i(&orderers.GetIdle()); *i; i++)
        {
          AddOrderer(**i, orderers.GetItemWeighting(**i), orderers.GetWeighting(), orderers.GetItemUsage(**i), orderers.GetUsage());
        }

        // Active 
        orderers.Process(cuts);
        for (NBinTree<Strategic::Weighting::Cuts<Strategic::Base::Orderer>::Item>::Iterator p(&cuts.items); *p; ++p)
        {
          Utils::Sprintf(buff, 64, "Priority: %d", p.GetKey());
          baseList->AddTextItem(buff, NULL, &color, Point<S32>(20, 0));

          for (NBinTree<Strategic::Base::Orderer, F32>::Iterator b(&(*p)->belowCut); *b; ++b)
          {
            AddOrderer(**b, orderers.GetItemWeighting(**b), orderers.GetWeighting(), orderers.GetItemUsage(**b), orderers.GetUsage());
          }
          for (NBinTree<Strategic::Base::Orderer, F32>::Iterator a(&(*p)->aboveCut); *a; ++a)
          {
            AddOrderer(**a, orderers.GetItemWeighting(**a), orderers.GetWeighting(), orderers.GetItemUsage(**a), orderers.GetUsage());
          }
        }

        cuts.Clear();
      }

      baseList->AddTextItem("Tokens", NULL, &color, Point<S32>(15, 0));

      for (NBinTree<Strategic::Base::Token>::Iterator s(&base.GetTokensSearching()); *s; s++)
      {
        AddToken(**s, 25);
      }
      for (NBinTree<Strategic::Base::Token>::Iterator w(&base.GetTokensWaiting()); *w; w++)
      {
        AddToken(**w, 25);
      }
      for (NBinTree<Strategic::Base::Token>::Iterator c(&base.GetTokensConstructing()); *c; c++)
      {
        AddToken(**c, 25);
      }

    }


    //
    // Add orderer
    //
    void Info::AddOrderer(Strategic::Base::Orderer &orderer, U32 weight, U32 totalWeight, U32 usage, U32 totalUsage)
    {
      char buff[128];
      Color color = orderer.idle ? Color(0.5f, 0.7f, 0.7f) : Color(1.0f, 1.0f, 1.0f);

      Utils::Sprintf(buff, 128, "%s", orderer.GetName());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(25, 0));

      F32 current = totalUsage ? F32(usage) * 100.0f / F32(totalUsage) : 0.0f;
      F32 avail = totalWeight ? F32(weight) * 100.0f / F32(totalWeight) : 0.0f;

      Utils::Sprintf(buff, 128, "Ratio: %f U: %d of %d [%f%%] W %d of %d [%f%%]",
        avail ? current / avail : 1E6f,
        usage, totalUsage, current, 
        weight, totalWeight, avail);
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(35, 0));

      const UnitObjList &units = orderer.GetUnitsOnField();
      Utils::Sprintf(buff, 128, "On Field: %d", units.GetCount());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(35, 0));
      for (UnitObjList::Iterator u(&units); *u; ++u)
      {
        Utils::Sprintf(buff, 64, "%s [%d]", (**u)->UnitType()->GetName(), (*u)->Id());
        baseList->AddTextItem(buff, NULL, &color, Point<S32>(45, 0));
      }

      Utils::Sprintf(buff, 128, "Tokens On Order: %d", orderer.GetTokensOnOrder().GetCount());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(35, 0));

      for (NList<Strategic::Base::Token>::Iterator o(&orderer.GetTokensOnOrder()); *o; ++o)
      {
        AddToken(**o, 45);
      }

      Utils::Sprintf(buff, 128, "Tokens In Construction: %d", orderer.GetTokensInConstruction().GetCount());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(35, 0));

      for (NList<Strategic::Base::Token>::Iterator c(&orderer.GetTokensInConstruction()); *c; ++c)
      {
        AddToken(**c, 45);
      }
    }


    //
    // Add token
    //
    void Info::AddToken(Strategic::Base::Token &token, U32 indent)
    {
      char buff[64];
      Color color(0.7f, 0.7f, 0.5f);

      Utils::Sprintf(buff, 64, "%s [%d]", token.GetType().GetName(), token.GetPriority());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(indent, 0));

      Utils::Sprintf(buff, 64, "Orderer: %s", token.GetOrderer() ? token.GetOrderer()->GetName() : "Orphaned");
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(indent + 10, 0));

      Utils::Sprintf(buff, 64, "State: %s", token.GetState());
      baseList->AddTextItem(buff, NULL, &color, Point<S32>(indent + 10, 0));

      UnitObj *constructor = token.GetConstructor();
      if (constructor)
      {
        Color color(0.5f, 0.7f, 0.7f);
        Utils::Sprintf(buff, 128, " - [%08d] : %s", constructor->Id(), constructor->TypeName());
        baseList->AddTextItem(buff, NULL, &color, Point<S32>(indent + 10, 0));
      }
    }


    //
    // Add asset request
    //
    void Info::AddAssetRequest(Strategic::Asset::Request &request)
    {
      char buff[64];
      Color color(0.3f, 0.8f, 1.0f);

      if (request.GetScript())
      {
        Utils::Sprintf(buff, 64, "%s", request.GetName());
        assetList->AddTextItem(buff, NULL, &color);

        Color color(1.0f, 0.8f, 0.3f);
        
        Utils::Sprintf(buff, 64, "Script: %s", request.GetScript()->GetName());
        assetList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

        Utils::Sprintf(buff, 64, "Script Config: %s", request.GetScript()->GetConfigName());
        assetList->AddTextItem(buff, NULL, &color, Point<S32>(10, 0));

      }

    }

  }

}
