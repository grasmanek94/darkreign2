///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Light Time
//
// 1-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "environment_time.h"
#include "environment_light.h"
#include "gametime.h"
#include "multilanguage.h"
#include "babel.h"
#include "stdload.h"
#include "message.h"


//////////////////////////////////////////////////////////////////////////////
//
// NameSpace Environment
//
namespace Environment
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Time
  //
  namespace Time
  {

    // Initialization Flag
    static Bool initialized = FALSE;

    // Current day of the month
    static U32 day;

    // Current month of the year
    static U32 month;

    // Current year
    static U32 year;


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)

      // Setup some reasonable defaults

      // Current day of the month
      day = 1;

      // Current month of the year
      month = 1;

      // Current year
      year = 2000;

      // Set initialized flag
      initialized = TRUE;
    }


    //
    // Done
    //
    void Done()
    {
      ASSERT(initialized)

      // Clear initialized flag
      initialized = FALSE;
    }


    //
    // IncDay
    //
    // Increment the day
    //
    void IncDay()
    {
      ASSERT(initialized)

      // Is the current day the last day of the month ?
      if (day == Clock::Date::DaysInMonth(year, month))
      {
        // Reset day
        day = 1;

        // Is the current month the last month of the year ?
        if (month == 12)
        {
          // Reset month and increment year
          month = 1;
          year++;
        }         
      }
      else
      {
        // Increment day
        day++;
      }
    }


    //
    // Get the day of the month
    //
    U32 GetDayOfMonth()
    {
      return (day);
    }


    //
    // Get the day of the week
    //
    const CH * GetDayOfWeekStr()
    {
      return (TRANSLATE((MultiLanguage::BuildKey(2, "standard.days", Clock::Date::GetDayStr(Clock::Date::DayOfWeek(year, month, day))))));
    }


    //
    // Get the month of the year
    //
    const CH * GetMonthStr()
    {
      return (TRANSLATE((MultiLanguage::BuildKey(2, "standard.months", Clock::Date::GetMonthStr(month)))));
    }


    //
    // Get the month of the year
    //
    U32 GetMonth()
    {
      return (month);
    }


    //
    // Get the year
    //
    U32 GetYear()
    {
      return (year);
    }


    //
    // SetDayOfMonth
    //
    // Set the day of the month
    //
    void SetDayOfMonth(U32 dayIn)
    {
      day = dayIn;
    }


    //
    // SetMonth
    //
    // Set the month of the year
    //
    void SetMonth(U32 monthIn)
    {
      month = monthIn;
    }


    //
    // SetYear
    //
    // Set the year
    //
    void SetYear(U32 yearIn)
    {
      year = yearIn;
    }


    //
    // GenerateMessage
    //
    // Generate a game message to display the current time and date
    //
    void GenerateMessage()
    {
      Message::TriggerGameMessage(
        0xEF97B9E3,
        4,
        GetDayOfWeekStr(),
        GetDayOfMonth(),
        GetMonthStr(),
        GetYear()); // "Environment::Time::Date"

      // Time
      // Get the time in seconds from the environment system
      U32 time = Environment::Light::GetTime();

      // Work out the hours minutes and seconds
      U32 hours = time / 3600;
      U32 minutes = time / 60 % 60;
      U32 seconds = time % 60;
  
      // Trigger the game message
      Message::TriggerGameMessage(0x5E46E9E5, 3, hours, minutes, seconds); // "Environment::Time::Time"
    }


    //
    // Load information
    //
    void LoadInfo(FScope *fScope)
    {
      ASSERT(fScope)

      day = StdLoad::TypeU32(fScope, "Day");
      month = StdLoad::TypeU32(fScope, "Month");
      year = StdLoad::TypeU32(fScope, "Year");
    }


    //
    // Save information
    //
    void SaveInfo(FScope *fScope)
    {
      StdSave::TypeU32(fScope, "Day", day);
      StdSave::TypeU32(fScope, "Month", month);
      StdSave::TypeU32(fScope, "Year", year);
    }
    

    //
    // PostLoad
    //
    void PostLoad()
    {

    }

  }
}
