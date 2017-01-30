/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic AI Configuration
//
// 22-SEP-1998
//


#ifndef __STRATEGIC_CONFIG_H
#define __STRATEGIC_CONFIG_H


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic.h"
#include "fscope.h"
#include "utiltypes.h"
#include "tactical.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Config
  //
  namespace Config
  {

    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RecruitForce
    //
    class RecruitForce
    {
    private:

      class TweakListNode : public UnitObjTypePtr
      {
      public:
        NList<TweakListNode>::Node node;
        F32 tweak;
      };
      typedef ReaperList<UnitObjType, TweakListNode> TweakList;

      struct Evaluation
      {
        F32 constitution;
        F32 effectiveness;
        F32 time;
        F32 cost;
        F32 tweak;
        TweakList tweakList;
        F32 tweakDefault;

      } evaluation;

      struct
      {
        F32 defense;
        F32 threat;

      } allocation;
 
      // The config name
      GameIdent name;

    public:

      // Constructor
      RecruitForce(FScope *fScope);

      // Destructor
      ~RecruitForce();

      // PostLoad
      void PostLoad();

      //
      // Evaluation
      //
      F32 GetEvaluationConstitution() const
      {
        return (evaluation.constitution);
      }
      F32 GetEvaluationEffectiveness() const
      {
        return (evaluation.effectiveness);
      }
      F32 GetEvaluationTime() const
      {
        return (evaluation.time);
      }
      F32 GetEvaluationCost() const
      {
        return (evaluation.cost);
      }
      F32 GetEvaluationTweak() const
      {
        return (evaluation.tweak);
      }
      F32 GetEvaluationTweak(UnitObjType *type) const
      {
        // Is this type in the tweak list ?
        for (TweakList::Iterator t(&evaluation.tweakList); *t; t++)
        {
          if ((*t)->Alive() && **t == type)
          {
            return ((*t)->tweak);
          }
        }

        // Not found, return the default
        return (evaluation.tweakDefault);
      }

      //
      // Allocation
      //
      F32 GetAllocationDefense() const
      {
        return (allocation.defense);
      }
      F32 GetAllocationThreat() const
      {
        return (allocation.threat);
      }

      // Get the name
      const GameIdent & GetName() const
      {
        return (name);
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class RecruitType
    //
    class RecruitType
    {
    public:

      class TypeListNode : public UnitObjTypePtr
      {
      public:
        NList<TypeListNode>::Node node;
        U32 amount;
      };
      typedef ReaperList<UnitObjType, TypeListNode> TypeList;

    private:

      // Types and the amounts
      TypeList types;

      // Amount tree by type id
      BinTree<U32> amounts;

      // The config name
      GameIdent name;

    public:

      // Constructor
      RecruitType(FScope *fScope);

      // Destructor
      ~RecruitType();

      // PostLoad
      void PostLoad();

      // How many of the given type do we need
      U32 GetAmount(U32 type) const;

    public:

      // Get the amounts tree
      const BinTree<U32> & GetAmounts() const
      {
        return (amounts);
      }

      // Get the types list
      const TypeList & GetTypes() const
      {
        return (types);
      }
     
      // Get the name
      const GameIdent & GetName() const
      {
        return (name);
      }
    };


    /////////////////////////////////////////////////////////////////////////////
    //
    // Class Generic
    //
    class Generic
    {
    private:

      // Name of the config
      GameIdent name;

      // The configuration (in fScope form)
      FScope *fScope;

    public:

      // Constructor
      Generic(const char *name, FScope *fScope);

      // Destructor
      ~Generic();

    public:

      // Get the name of the config
      const char * GetName()
      {
        return (name.str);
      }

      // Get the fscope of the config
      FScope * GetFScope()
      {
        fScope->InitIterators();
        return (fScope);
      }

    };



    // Initialization and Shutdown
    void Init();
    void Done();

    // Process Config
    void ProcessConfig(FScope *fScope);

    // Find a force config
    RecruitForce * FindRecruitForce(U32 crc);

    // Find a type config
    RecruitType * FindRecruitType(U32 crc);

    // Find a generic config
    Generic * FindConfig(U32 type, const GameIdent &config, Bool required = TRUE);

  }

}

#endif