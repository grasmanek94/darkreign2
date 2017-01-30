///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Environment Time
//
// 1-MAR-1999
//


#ifndef __ENVIRONMENT_TIME_H
#define __ENVIRONMENT_TIME_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
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

    // Initialization and Shutdown
    void Init();
    void Done();

    // Increment the day
    void IncDay();

    // Get the day of the month
    U32 GetDayOfMonth();

    // Get the day of the week
    const CH * GetDayOfWeekStr();

    // Get the month of the year
    const CH * GetMonthStr();

    // Get the month of the year
    U32 GetMonth();

    // Get the year
    U32 GetYear();

    // Set the day of the month
    void SetDayOfMonth(U32 day);

    // Set the month of the year
    void SetMonth(U32 month);

    // Set the year
    void SetYear(U32 year);

    // Generate a game message to display the current time and date
    void GenerateMessage();

    // Load information
    void LoadInfo(FScope *fScope);

    // Save information
    void SaveInfo(FScope *fScope);
    
    // PostLoad
    void PostLoad();

  }

}


#endif