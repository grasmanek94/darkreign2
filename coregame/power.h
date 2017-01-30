///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Relations
//
// 12-NOV-1998
//


#ifndef __POWER_H
#define __POWER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Power
//
namespace Power
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Constants 
  //
  const U32 NumLevels = 3;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Unit
  //
  class Unit
  {
  private:

    // Power produced during the day
    U32 producedDay;

    // Power produced during the night
    U32 producedNight;

    // Power required to operate
    U32 required;

  public:

    // Constructor
    Unit();

    // Setup
    void Setup(FScope *fScope);

  public:

    // Get the power produced during the day
    U32 GetProducedDay() const
    {
      return (producedDay);
    }

    // Get the power produced during the night
    U32 GetProducedNight() const
    {
      return (producedNight);
    }

    // Get the power required
    U32 GetRequired() const
    {
      return (required);
    }

  };

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Team
  //
  class Team
  {
  private:

    // Total available power during the day
    U32 availableDayRaw;

    // Total available power during the night
    U32 availableNightRaw;

    // Currently consumed power
    U32 consumedRaw;

    // Actual amount available now
    U32 availableRaw;

    // Filtered values
    U32 availableDayFiltered;
    U32 availableNightFiltered;
    U32 consumedFiltered;
    U32 availableFiltered;

    // Amount that has been stolen
    U32 stolen;

    // Stealing power
    struct Leak
    {
      enum
      {
        NONE = 0,
        LEAKING,
        RECOVERING,
      };

      // Unit being stolen from
      UnitObjPtr facility;

      // Amount to steal
      S32 amount;

      // Amount stolen
      S32 current;

      // Amount to take per cycle
      S32 perCycle;

      // Current state
      U32 state;

      NBinTree<Leak>::Node node;

      // Constructor
      Leak() : state(NONE), amount(0), current(0), perCycle(0) {}
    };

    // Power leaks, keyed by unitId of unit
    NBinTree<Leak> leaks;

  public:

    // Constructor
    Team();
    ~Team();

    // Add a unit
    void AddUnit(const Unit &unit);

    // Remove a unit
    void RemoveUnit(const Unit &unit);

    // Add consumption
    void AddConsumption(U32 consumption);

    // Remove consumption
    void RemoveConsumption(U32 consumption);

    // Add a power leak
    void AddLeak(UnitObj *facility, UnitObj *thief, U32 amount, U32 rate);

    // Remove a power leak
    void RemoveLeak(UnitObj *thief);

    // Get the current power level
    // Pass in either GetAvailable() or GetAvailableUnfiltered()
    U32 GetLevel(U32 avail) const;

    // Get the color of the current power level
    const Color & GetColor() const;

    // Calculate available power
    U32 CalcAvailable() const;

    // Calculate available power (unfiltered)
    U32 CalcAvailableUnfiltered() const;

    // Process
    void Process();

    // ResetFilter
    void ResetFilter();

    // PostLoad
    void PostLoad();

    // Save and load
    void SaveState(FScope *fScope);
    void LoadState(FScope *fScope);

  public:

    // Get the currently available power
    U32 GetAvailable() const
    {
      return (availableFiltered);
    }

    // Get the total power available during the day
    U32 GetAvailableDay() const
    {
      return (availableDayFiltered);
    }

    // Get the total power available during the night
    U32 GetAvailableNight() const
    {
      return (availableNightFiltered);
    }

    // Get the total power consumed
    U32 GetConsumed() const
    {
      return (consumedFiltered);
    }

    // Get the total power available during the day (unfiltered)
    U32 GetAvailableDayUnfiltered() const
    {
      return (stolen > availableDayRaw ? 0 : availableDayRaw - stolen);
    }

    // Get the total power available during the night (unfiltered)
    U32 GetAvailableNightUnfiltered() const
    {
      return (stolen > availableNightRaw ? 0 : availableNightRaw - stolen);
    }

    // Get the total power consumed (unfiltered)
    U32 GetConsumedUnfiltered() const
    {
      return (consumedRaw);
    }

    // Get the currently available power (unfiltered)
    U32 GetAvailableUnfiltered() const
    {
      return (availableRaw);
    }
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Efficiency
  //
  class Efficiency
  {
  private:

    // Efficiency at each level
    F32 efficiency[NumLevels];

  public:

    // Constructor
    Efficiency();

    // Setup
    void Setup(FScope *fScope);

  public:

    // Get the efficiency given a team power
    F32 Get(const Team &team) const
    {
      return (Get(team.GetLevel(team.GetAvailable())));
    }

    // Get the efficiency for a given power level
    F32 Get(U32 level) const
    {
      ASSERT(level < NumLevels)
      return (efficiency[level]);
    }

  };


  // Get the name of a power level
  const char * GetLevelName(U32 level);

}

#endif