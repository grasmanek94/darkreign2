///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Time Related Functions
// 26-NOV-1997
//


#ifndef __CLOCK_H
#define __CLOCK_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Clock
//
namespace Clock
{
  // Returns a string with the local date and time
  const char * GetDateAndTime();


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Time
  //
  namespace Time
  {
    // Returns the current millisecond counter
    U32 Ms();

    // Returns the current microseconds counter
    U64 Us();

    // Return the current microseconds counter lower half
    U32 UsLwr();

    // Format current time
    void GetStr(char *str);

    // Initialize Time
    void Init();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Date
  //
  namespace Date
  {
    // Format current date
    void GetStr(char *str);

    // Format current date 
    char *GetVerbose();

    // Get time in seconds since 1970
    U32 GetUTC();

    // Given a day month and year, return the day of the week
    U32 DayOfWeek(U32 year, U32 month, U32 day = 1);

    // Is the given year a leap year ?
    Bool LeapYear(U32 year);

    // Given a month and year, return number of days in the month
    U32 DaysInMonth(U32 year, U32 month);

    // Given a month index return a string
    const char * GetMonthStr(U32 month);

    // Given a day of the week return a string
    const char * GetDayStr(U32 day);

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CycleTimer
  //
  namespace CycleTimer
  {
    // RDTSC data
    extern U32 oh;
    extern U32 lo;
    extern U32 hi;

    // Start the cycle timer
    void Start();

    // Stop the cycle timer
    void Stop();

    // Return result of last count
    inline U32 GetCount()
    {
      return (lo);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Watch
  //
  class Watch
  {
  private:

    U32 samples;
    U32 minimum;
    U32 maximum;
    U32 smooth;
    U64 sum;

  public:

    // Constructor
    Watch() 
    { 
      Reset(); 
    }

    // Reset results
    void Reset();

    // Add a sample
    void Sample(U32 sample);

    // Return number of samples
    U32 GetSamples() 
    { 
      return (samples); 
    }

    // Return lowest sample value
    U32 GetMin() 
    { 
      return (minimum); 
    }

    // Return highest sample value
    U32 GetMax() 
    { 
      return (maximum); 
    }

    // Return smoothed average
    U32 GetSmooth() 
    { 
      return (smooth); 
    }

    // Return average
    U32 GetAvg();

    // Return sum
    U64 GetSum() 
    { 
      return (sum); 
    }

    // Return a formatted report string
    const char *Report();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CycleWatch
  //
  class CycleWatch : public Watch
  {
  private:

    U32 tscHi;
    U32 tscLo;
    Bool on;

  public:

    // Constructor
    CycleWatch() : Watch(), on(FALSE) {}

    // Start the timer
    void Start();

    // Stop the timer and accumulate statistics
    void Stop();

    // Reset all samples and averages
    void Reset()
    {
      on = FALSE;
      Watch::Reset();
    }
  };

}

#endif
