///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Condition System
//
// 11-AUG-1998
//

#ifndef __CONDITION_PRIVATE_H
#define __CONDITION_PRIVATE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "tagobjdec.h"
#include "regionobjdec.h"
#include "mapobjdec.h"
#include "relation.h"
#include "relationaloperator.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Conditions
//
namespace Conditions
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class True
  //
  // The "true" condition always tests to be TRUE
  //
  class True : public Condition
  {
  public:

    // True
    True(FScope *fScope) : Condition(fScope) { };

    // Test
    Bool Test(class Team *) { return (TRUE); }

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("TRUE");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class False
  //
  // The "false" condition always tests to be FALSE
  //
  class False : public Condition
  {
  public:

    // False
    False(FScope *fScope) : Condition(fScope) { };

    // Test
    Bool Test(class Team *) { return (FALSE); }

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("FALSE");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Not
  //
  // The "not" condition inverts the test inside it
  //
  class Not : public Condition
  {
  private:

    Condition *condition;

  public:

    // Not
    Not(FScope *fScope);

    // ~Not
    ~Not();

    // Test
    Bool Test(class Team *team);

    // Reset
    void Reset();

    // Save and load
    void Save(FScope *fScope);
    void Load(FScope *fScope);
    void PostLoad();

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("NOT");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Group
  //
  // A group of conditions - abstract base class
  //
  class Group : public Condition
  {
  public:

    // Conditions in the group
    List<Condition> conditions;

    // Group
    Group(FScope *fScope);

    // ~Group
    ~Group();

    // Reset
    void Reset();

    // Save and load
    void Save(FScope *fScope);
    void Load(FScope *fScope);
    void PostLoad();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class And
  //
  // The "and" condition contains a list of conditions which must ALL be true
  // for this condition to be met.
  //
  class And : public Group
  {
  public:

    // And
    And(FScope *fScope) : Group(fScope) { }

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("And");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Or
  //
  // The "or" condition contains a list of conditions of which only one needs
  // to be true for this condition to be met.
  //
  class Or : public Group
  {
  public: 

    // Or
    Or(FScope *fScope) : Group(fScope) { }

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("Or");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Timer
  //
  // This condition is met when the timeout expires
  //
  class Timer : public Condition
  {
  private:

    // Minimum and maximum timeout periods
    F32 minTimeout, maxTimeout;

    // Game timer
    GameTime::Timer timer;

    // Name of var to update
    GameIdent var;

  public:

    // Timer
    Timer(FScope *fScope);

    // Reset
    void Reset();

    // Test
    Bool Test(class Team *team);

    // Save and load
    void Save(FScope *fScope);
    void Load(FScope *fScope);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("Timer");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TimeOfDay
  //
  // This condition is met when the timeout expires
  //
  class TimeOfDay : public Condition
  {
  private:

    // Time of day to trigger on
    F32 time;

    // Time of day last time the test was performed
    F32 sample;

  public:

    // TimeOfDay
    TimeOfDay(FScope *fScope);

    // Reset
    void Reset();

    // Test
    Bool Test(class Team *team);

    // Save and load
    void Save(FScope *fScope);
    void Load(FScope *fScope);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("TimeOfDay");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class NumTeams
  //
  // Testing the number of related teams
  //
  class NumTeams : public Condition
  {
  private:

    // Relation to test
    Relation relation;

    // Number to test against
    U32 amount;

    // Relational operator to test with
    RelationalOperator<U32> oper;

  public:

    // TimeOfDay
    NumTeams(FScope *fScope);

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("NumTeams");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Region
  //
  struct Region
  {
    // Name of the region
    GameIdent regionName;

    // Reaper to region
    RegionObjPtr region;

    // Constructor
    Region(FScope *fScope);

    // PostLoad processor
    void PostLoad();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Tag
  //
  struct Tag
  {
    // Name of the tag
    GameIdent tagName;

    // Reaper to tag
    TagObjPtr tag;

    // Style
    enum { AMOUNT, PERCENTAGE } style;

    union
    {
      // Percentage of objects in the tag of interest
      F32 percentage;

      // Number of objects in the tag of interest
      U32 amount;
    };

    // Relational operator to test with
    RelationalOperator<U32> oper;

    // Constructor
    Tag(FScope *fScope);

    // Reset
    void Reset();

    // PostLoad processor
    void PostLoad();

    // GetAmount
    U32 GetAmount();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Type
  //
  struct Type
  {
    // Name of the type
    GameIdent typeName;

    // Pointer to the type
    MapObjType *type;

    // All types
    Bool all;
    
    // Number of objects of this type of interest
    U32 amount;

    // Relational operator to test with
    RelationalOperator<U32> oper;

    // Constructor
    Type(FScope *fScope);

    // PostLoad processor
    void PostLoad();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Struct Property
  //
  struct Property
  {
    // Property
    GameIdent property;
    
    // Number of objects of this type of interest
    U32 amount;

    // Relational operator to test with
    RelationalOperator<U32> oper;

    // Constructor
    Property(FScope *fScope);

    // PostLoad processor
    void PostLoad();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagCondition
  //
  class TagCondition : public Condition
  {
  protected:

    // Tag of interest
    Tag *tag;

  public:

    // Constructor and Destructor
    TagCondition(FScope *fScope);
    ~TagCondition();

    // Reset
    void Reset();

    // TagTest
    Bool TagTest(class Team *team);

    // Post load processing
    void PostLoad();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TypeCondition
  //
  class TypeCondition : public Condition
  {
  protected:

    // Type of interest
    Type *type;

  public:

    // Constructor and Destructor
    TypeCondition(FScope *fScope);
    ~TypeCondition();

    // TypeTest
    Bool TypeTest(class Team *team);

    // TypeCount
    U32 TypeCount(class Team *team);

    // PostLoad
    void PostLoad();

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class InRegion
  //
  class InRegion : public Condition
  {
  private:

    // Region of interest
    Region *region;

    // Type of InRegion
    enum { TAG, TYPE, PROPERTY } style;

    union
    {
      // Tag of interest
      Tag *tag;

      // Type of interest
      Type *type;

      // Property of interest
      Property *property;
    };

    // Relation refered to
    Relation *relation; 

    // Combine relations
    U8 combine : 1,

    // Only count what we can see
       canSee : 1;

  public:

    // Constructor
    InRegion(FScope *fScope);
    ~InRegion();

    // Reset
    void Reset();

    // RegionTest
    Bool RegionTest(class Team *team, class Team *canBeSeenBy);

    // RegionTest
    Bool RegionTest(const List<Team> &teams, class Team *canBeSeenBy);

    // Test
    Bool Test(class Team *team);

    // PostLoad
    void PostLoad();

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("InRegion");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ThreatInRegion
  //
  class ThreatInRegion : public Condition
  {
  private:

    // Region of interest
    Region *region;

    // Amount of threat to test
    U32 amount;

    // Armour class to test against
    GameIdent armourName;
    U32 armourClass;

    // Relational operator to test with
    RelationalOperator<U32> oper;

    // Relation refered to
    Relation *relation; 

    // Combine relations
    Bool combine;

  public:

    // Constructor
    ThreatInRegion(FScope *fScope);
    ~ThreatInRegion();

    // PostLoad
    void PostLoad();

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("ThreatInRegion");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagCount
  //
  class TagCount : public Condition
  {
  private:

    // Tag of interest
    Tag *tag;

    // Name of var to update with tag count
    GameIdent var;

  public:

    // Constructor
    TagCount(FScope *fScope);
    ~TagCount();

    // Test
    Bool Test(class Team *team);

    // Reset
    void Reset();

    // PostLoad
    void PostLoad();

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("TagCount");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class HaveTag
  //
  class HaveTag : public TagCondition
  {
  private:

    // Relation refered to
    Relation *relation;

  public:

    // Constructor
    HaveTag(FScope *fScope);
    ~HaveTag();

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("HaveTag");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagProximity
  //
  class TagProximity : public Condition
  {
  private:

    // Source tag name
    GameIdent sourceTagName;

    // Source tag
    TagObjPtr sourceTag;

    // Range
    F32 range;

    // Style
    enum { TAG, REGION, TYPE, PROPERTY } style;

    union
    {
      // Tag of interest
      Tag *tag;

      // Region of interest
      Region *region;

      // Type of interest
      Type *type;

      // Property of interest
      Property *property;

    };

  public:

    // Constructor
    TagProximity(FScope *fScope);
    ~TagProximity();

    // Test
    Bool Test(class Team *team);

    // Reset
    void Reset();

    // PostLoad
    void PostLoad();

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("TagProximity");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class TagInTransport
  //
  class TagInTransport : public Condition
  {
  private:

    // Tag of interest
    Tag *tag;

  public:

    // Constructor
    TagInTransport(FScope *fScope);
    ~TagInTransport();

    // Test
    Bool Test(class Team *team);

    // PostLoad
    void PostLoad();

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("TagInTransport");
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class GuardingTag
  //
  class GuardingTag : public Condition
  {
  private:

    // Tag being guarded
    TagObjPtr guardee;

    // Tag doing the guarding
    TagObjPtr guarder;

    // Idents for post load
    GameIdent guardeeId;
    GameIdent guarderId;

  public:

    // Constructor
    GuardingTag(FScope *fScope);
    ~GuardingTag();

    // Test
    Bool Test(class Team *team);

    // PostLoad
    void PostLoad();

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("GuardingTag");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class HaveType
  //
  class HaveType : public TypeCondition
  {
  private:

    // Relation referred to
    Relation *relation;

    // Combine relations
    Bool combine;

  public:

    // Constructor
    HaveType(FScope *fScope);
    ~HaveType();

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("HaveType");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class HaveMember
  //
  class HaveMember : public Condition
  {
  private:

    // Relation referred to
    Relation *relation;

    // Combine relations
    Bool combine;

    // Relational operator to test with
    RelationalOperator<U32> oper;

    // Number of objects of this type of interest
    U32 amount;

  public:

    // Constructor
    HaveMember(FScope *fScope);
    ~HaveMember();

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("HaveMember");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class ConstructingType
  //
  class ConstructingType : public Condition
  {
  private:

    // Unit type
    UnitObjType *unitType;

  public:

    // Constructor
    ConstructingType(FScope *fScope);
    ~ConstructingType();

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("ConstructingType");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class SoundEffect
  //
  // This condition is met when a sound effect stops playing
  //
  class SoundEffect : public Condition
  {
  private:

    // Name of sound
    FileIdent name;

  public:

    // Constructor and Destructor
    SoundEffect(FScope *fScope);

    // Test
    Bool Test(class Team *team);

    // Return the name of the condition type
    const char * GetTypeName()
    {
      return ("SoundEffect");
    }
  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //
  class Resource : public Condition
  {
  private:

    // Amount to test against
    U32 amount;

    // Relation
    Relation *relation;

    // Operator to use when do tests
    RelationalOperator<U32> oper;

    // Combine flag
    Bool combine;

  public:

    // Constructor and Destructor
    Resource(FScope *fScope);
    ~Resource();

    // Test
    Bool Test(Team *team);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class RelativeRelation
  //
  class RelativeRelation : public Condition
  {
  private:

    class Statistic;

    // Relative amount
    F32 amount;

    // Relation
    Relation relation;

    // Operator to use when do tests
    RelationalOperator<F32> oper;

    // Statistic to test
    Statistic *stat;

    // Combine flag
    Bool combine;

  public:

    // Constructor and Destructor
    RelativeRelation(FScope *fScope);
    ~RelativeRelation();

    // Test
    Bool Test(Team *team);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Tally
  //
  class Tally : public Condition
  {
  private:

    class Statistic;

    // Amount to compare against
    U32 amount;

    // Type
    U32 type;

    // Operator to use when do tests
    RelationalOperator<U32> oper;

    // Statistic to test
    Statistic *stat;

    // Relation
    Relation *relation;

    // Combine flag
    Bool combine;

    // Name of var to update
    GameIdent var;

  public:

    // Constructor and Destructor
    Tally(FScope *fScope);
    ~Tally();

    // Test
    Bool Test(Team *team);

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Class If
  //
  class If : public Condition
  {
  private:

    // FScope
    FScope *fScope;

  public:

    // Constructor and Destructor
    If(FScope *fScope);
    ~If();

    // Test
    Bool Test(Team *team);

    // Returh the name of the condition type
    const char * GetTypeName()
    {
      return ("If");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Template VarConst
  //
  template <class TYPE> class VarConst : public Condition
  {
  private:

    // Name of the var
    GameIdent name;

    // Operator to use when do tests
    RelationalOperator<TYPE> oper;

    // Value to compare against
    TYPE value;

  public:

    // Constructor and Destructor
    VarConst<TYPE>(FScope *fScope);

    // Test
    Bool Test(Team *team);

    // Returh the name of the condition type
    const char * GetTypeName()
    {
      return ("VarConst");
    }

  };


  /////////////////////////////////////////////////////////////////////////////
  //
  // Template VarVar
  //
  template <class TYPE> class VarVar : public Condition
  {
  private:

    // Name of the var
    GameIdent name;

    // Operator to use when do tests
    RelationalOperator<TYPE> oper;

    // Name of var to compare against
    GameIdent var;

  public:

    // Constructor and Destructor
    VarVar<TYPE>(FScope *fScope);

    // Test
    Bool Test(Team *team);

    // Returh the name of the condition type
    const char * GetTypeName()
    {
      return ("VarVar");
    }

  };


};

#endif
