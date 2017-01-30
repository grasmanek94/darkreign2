///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Power
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "power.h"
#include "environment_light.h"
#include "resolver.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Power
//
namespace Power
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Level
  //
  struct Level
  {
    // Name of the level
    GameIdent name;

    // Cutoff at which this level is active
    F32 cutoff;

    // Color of the level
    Color color;

    // The default efficiency
    F32 efficiency;

    // Constructor
    Level(const char *name, F32 cutoff, const Color &color, F32 efficiency)
    : name(name),
      cutoff(cutoff),
      color(color),
      efficiency(efficiency)
    {
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //

  // The power levels
  static Level levels[NumLevels] =
  {
    Level("Red",    0.5f,     Color(0.75f, 0.00f, 0.00f), 1.0F),
    Level("Yellow", 1.0f,     Color(0.75f, 0.75f, 0.00f), 1.0F),
    Level("Green",  F32_MAX,  Color(0.00f, 0.75f, 0.00f), 1.0F)
  };

  // Filter value
  static const U32 filter = 224;
  static const U32 invFilter = 256 - filter;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Team
  //

  //
  // Constructor
  //
  Team::Team()
  : availableDayRaw(0),
    availableNightRaw(0),
    consumedRaw(0),
    availableRaw(0),
    availableDayFiltered(0),
    availableNightFiltered(0),
    consumedFiltered(0),
    availableFiltered(0),
    stolen(0),
    leaks(&Leak::node)
  {
  }


  //
  // Destructor
  //
  Team::~Team()
  {
    leaks.DisposeAll();
  }


  //
  // Add a unit
  //
  void Team::AddUnit(const Unit &unit)
  {
    availableDayRaw += unit.GetProducedDay();
    availableNightRaw += unit.GetProducedNight();
    consumedRaw += unit.GetRequired();
  }


  //
  // Remove a unit
  //
  void Team::RemoveUnit(const Unit &unit)
  {
    ASSERT(availableDayRaw >= unit.GetProducedDay())
    availableDayRaw -= unit.GetProducedDay();

    ASSERT(availableNightRaw >= unit.GetProducedNight())
    availableNightRaw -= unit.GetProducedNight();

    ASSERT(consumedRaw >= unit.GetRequired())
    consumedRaw -= unit.GetRequired();
  }


  //
  // Add consumption
  //
  void Team::AddConsumption(U32 consumption)
  {
    consumedRaw += consumption;
  }


  //
  // Remove consumption
  //
  void Team::RemoveConsumption(U32 consumption)
  {
    ASSERT(consumedRaw >= consumption)
    consumedRaw -= consumption;
  }


  //
  // Add a power leak
  //
  void Team::AddLeak(UnitObj *facility, UnitObj *thief, U32 amount, U32 rate)
  {
    ASSERT(facility)
    ASSERT(thief)

    Leak *newLeak = new Leak;

    newLeak->facility = facility;
    newLeak->amount = amount;
    newLeak->perCycle = rate;
    newLeak->state = Leak::LEAKING;

    leaks.Add(thief->Id(), newLeak);
  }


  //
  // Remove a power leak
  //
  void Team::RemoveLeak(UnitObj *thief)
  {
    ASSERT(thief)

    Leak *leak = leaks.Find(thief->Id());
    if (leak)
    {
      // Change the direction of the leak
      leak->state = Leak::RECOVERING;
    }
  }


  //
  // Get the current power level
  //
  U32 Team::GetLevel(U32 avail) const
  {
    F32 percentage = consumedRaw ? F32(avail) / F32(consumedRaw) : avail ? levels[NumLevels - 1].cutoff : 0.0f;

    // Compare available against consumption
    for (U32 l = 0; l < NumLevels; l++)
    {
      if (percentage < levels[l].cutoff)
      {
        return (l);
      }
    }
    return (NumLevels - 1);
  }


  //
  // Get the color of the current power level
  //
  const Color & Team::GetColor() const
  {
    return (levels[GetLevel(GetAvailable())].color);
  }


  //
  // Calculate available power
  //
  U32 Team::CalcAvailable() const
  {
    F32 day = Environment::Light::GetPercentageDay();
    return Utils::FtoL(F32(availableDayFiltered) * day + F32(availableNightFiltered) * (1.0f - day));
  }


  //
  // Calculate available power (unfiltered)
  //
  U32 Team::CalcAvailableUnfiltered() const
  {
    F32 day = Environment::Light::GetPercentageDay();
    return Utils::FtoL(F32(GetAvailableDayUnfiltered()) * day + F32(GetAvailableNightUnfiltered()) * (1.0f - day));
  }


  //
  // Process
  //
  void Team::Process()
  {
    // Update power leaks
    NBinTree<Leak>::Iterator i(&leaks);
    Leak *leak;
    stolen = 0;

    while ((leak = i++) != NULL)
    {
      if (leak->facility.Alive())
      {
        if (leak->state == Leak::LEAKING)
        {
          // Increase leak until it reaches the desired amount
          if ((leak->current += leak->perCycle) >= leak->amount)
          {
            leak->current = leak->amount;
            leak->state = Leak::NONE;
          }
        }
        else if (leak->state == Leak::RECOVERING)
        {
          // Reduce leak until it reaches zero
          if ((leak->current -= leak->perCycle) <= 0)
          {
            // Done restoring;
            leaks.Dispose(leak);
            leak = NULL;
          }
        }
        if (leak)
        {
          stolen += leak->current;
        }
      }
      else
      {
        // Facility has died, remove leak
        leaks.Dispose(leak);
      }
    }

    // Update filters
    U32 val;

    // Apply stolen power before filtering
    U32 rawDay = GetAvailableDayUnfiltered();
    U32 rawNight = GetAvailableNightUnfiltered();

    val = (filter * availableDayFiltered + invFilter * rawDay) >> 8;
    if (val == availableDayFiltered)
    {
      availableDayFiltered = rawDay;
    }
    else
    {
      availableDayFiltered = val;
    }

    val = (filter * availableNightFiltered + invFilter * rawNight) >> 8;
    if (val == availableNightFiltered)
    {
      availableNightFiltered = rawNight;
    }
    else
    {
      availableNightFiltered = val;
    }

    val = (filter * consumedFiltered + invFilter * consumedRaw) >> 8;
    if (val == consumedFiltered)
    {
      consumedFiltered = consumedRaw;
    }
    else
    {
      consumedFiltered = val;
    }

    // Update available power based on day time
    availableRaw = CalcAvailableUnfiltered();
    availableFiltered = CalcAvailable();
  }


  //
  // ResetFilter
  //
  void Team::ResetFilter()
  {
    availableDayFiltered = GetAvailableDayUnfiltered();
    availableNightFiltered = GetAvailableNightUnfiltered();
    consumedFiltered = consumedRaw;

    // Update available power based on day time
    availableRaw = CalcAvailableUnfiltered();
    availableFiltered = CalcAvailable();
  }


  //
  // PostLoad
  //
  void Team::PostLoad()
  {
    for (NBinTree<Leak>::Iterator i(&leaks); *i; i++)
    {
      Resolver::Object<UnitObj, UnitObjType>((*i)->facility);
    }
  }


  //
  // SaveState
  //
  void Team::SaveState(FScope *fScope)
  {
    // Total amount stolen
    StdSave::TypeU32(fScope, "Stolen", stolen);

    // Save power leaks
    for (NBinTree<Leak>::Iterator i(&leaks); *i; i++)
    {
      FScope *sScope = StdSave::TypeU32(fScope, "AddLeak", i.GetKey());
      Leak *leak = *i;

      StdSave::TypeReaper(sScope, "Facility", leak->facility);
      StdSave::TypeU32(sScope, "Amount", leak->amount);
      StdSave::TypeU32(sScope, "Current", leak->current);
      StdSave::TypeU32(sScope, "Rate", leak->perCycle);
      StdSave::TypeU32(sScope, "State", leak->state);
    }
  }


  //
  // LoadState
  //
  void Team::LoadState(FScope *fScope)
  {
    FScope *sScope;

    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xBC0A8064: // "Stolen"
        {
          stolen = StdLoad::TypeU32(sScope);
          break;
        }

        case 0x5D4F40C7: // "AddLeak"
        {
          FScope *ssScope;
          Leak *leak = new Leak;
          U32 key = StdLoad::TypeU32(sScope);

          while ((ssScope = sScope->NextFunction()) != NULL)
          {
            switch (ssScope->NameCrc())
            {
              case 0x2B4A9F8C: // "Facility"
                StdLoad::TypeReaper(ssScope, leak->facility);
                break;

              case 0x3E822FCC: // "Amount"
                leak->amount = StdLoad::TypeU32(ssScope);
                break;

              case 0x587C9FAF: // "Current"
                leak->current = StdLoad::TypeU32(ssScope);
                break;

              case 0x8B19A1D1: // "Rate"
                leak->perCycle = StdLoad::TypeU32(ssScope);
                break;

              case 0x96880780: // "State"
                leak->state = StdLoad::TypeU32(ssScope);
                break;
            }
          }

          leaks.Add(key, leak);
          break;
        }
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Unit
  //


  //
  // Constructor
  //
  Unit::Unit()
  : producedDay(0),
    producedNight(0),
    required(0)
  {
  }


  //
  // Setup
  //
  void Unit::Setup(FScope *fScope)
  {
    if (fScope)
    {
      producedDay = StdLoad::TypeU32(fScope, "ProducedDay", 0);
      producedNight = StdLoad::TypeU32(fScope, "ProducedNight", producedDay);
      required = StdLoad::TypeU32(fScope, "Required", 0);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Efficiency
  //


  //
  // Constructor
  //
  Efficiency::Efficiency()
  {
    for (U32 e = 0; e < NumLevels; e++)
    {
      efficiency[e] = levels[e].efficiency;
    }
  }


  //
  // Setup
  //
  void Efficiency::Setup(FScope *fScope)
  {
    if (fScope)
    {
      FScope *sScope;
      Bool found = FALSE;

      while ((sScope = fScope->NextFunction()) != NULL)
      {
        // Attempt to find this level amonst the levels
        for (U32 l = 0; l < NumLevels; l++)
        {
          if (sScope->NameCrc() == levels[l].name.crc)
          {
            // Load the value
            efficiency[l] = Clamp<F32>(0.0F, StdLoad::TypeF32(sScope), 1.0F);
            found = TRUE;
            break;
          }
        }
      }
    
      if (!found)
      {
        fScope->ScopeError("Could not find power level '%s'", sScope->NameStr());
      }
    }
  }
 

  //
  // GetLevelName
  //
  // Get the name of a power level
  //
  const char * GetLevelName(U32 level)
  {
    ASSERT(level < NumLevels)
    return (levels[level].name.str);
  }

}
