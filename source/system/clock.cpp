///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Time Related Functions
// 26-NOV-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Libraries
//
#pragma comment (lib, "winmm")

#include <mmsystem.h>


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Clock
//
namespace Clock
{
  //
  // GetDateAndTime
  //
  // Returns a string with the local date and time
  //
  const char * GetDateAndTime()
  {
    static char str1[256];
    char str2[128];

    // Get the local time
    SYSTEMTIME time;
    GetLocalTime(&time);

    // Get the date and time
    GetDateFormat(LOCALE_SYSTEM_DEFAULT, DATE_LONGDATE, &time, NULL, str1, sizeof(str1));
    GetTimeFormat(LOCALE_SYSTEM_DEFAULT, LOCALE_NOUSEROVERRIDE, &time, NULL, str2, sizeof(str2));

    // Combine them
    Utils::Strcat(str1, " ");
    Utils::Strcat(str1, str2);

    // Return the static string
    return (str1);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Time
  //
  namespace Time
  {

    // QueryPerformanceFrequency
    static U64 perfFreq;

    // QueryPerformanceFrequencyF64
    static F64 perfStep;

    // Initialised Flag
    static Bool initialized = FALSE;

    #pragma warning (disable: 4725) // Turn off FDIV warning

    //
    // Initialize time
    //
    void Init()
    {
      ASSERT(!initialized)

      // Store frequency
      QueryPerformanceFrequency((LARGE_INTEGER *)&perfFreq);

      perfStep = 1e6f;

      __asm
      {
        fld qword ptr perfStep;     // ST[1]
        fild qword ptr perfFreq;    // ST[0]
        fdivp ST(1), ST(0);         // We know, and we don't give a rats arse
        fstp qword ptr perfStep;
      }

      initialized = TRUE;
    }


    //
    // Returns the current millisecond counter
    //
    U32 Ms()
    {
      // This could be inlined, but in the whole scheme of things, a 2000cycle 
      // call to timeGetTime dwarfs the 5 or so cycle overhead for the extra 
      // function call to Ms
      return (timeGetTime());
    }


    //
    // Returns the current microseconds counter
    //
    U64 Us()
    {
      ASSERT(initialized)

      U64 timer;
      QueryPerformanceCounter((LARGE_INTEGER *) &timer);

      return (timer * 1000000 / perfFreq);
    }


    //
    // Return the current microseconds counter lower half
    //
    U32 UsLwr()
    {
      ASSERT(initialized)

      U64 timer;

      QueryPerformanceCounter((LARGE_INTEGER *) &timer);

      __asm
      {
        fild qword ptr timer
        fmul perfStep
        fistp qword ptr timer;
      }

      return (U32(timer));
    }


    //
    // Format current time
    //
    void GetStr(char *str)
    {
      _strtime(str);
    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Date
  //
  namespace Date
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Internal Data
    //

    static U32 days[] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };
    static U32 mdays[] = { 0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365 };
    static const char *months[] = 
    {
      "January", "February", "March", "April", "May", "June", 
      "July", "August" , "September", "October", "November", "December" 
    };
    static const char *daysOfWeek[] =
    {
      "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
    };


    //
    // Format current date
    //
    void GetStr(char *str)
    {
      _strdate(str);
    }


    //
    // Format current date
    //
    char *GetVerbose()
    {
      S32 ltime;
      time(&ltime);
      return (ctime(&ltime));
    }


    //
    // Get time in seconds since 1970
    //
    U32 GetUTC()
    {
      return (time(NULL));
    }


    //
    // Given a day month and year, return the day of the week
    //
    U32 DayOfWeek(U32 year, U32 month, U32 day)
    {
      // Only handle years above 1752
      ASSERT(year > 1752)

      // Make sure the month is in range
      ASSERT(month >= 1 && month <= 12)

      // Make sure the day is in range
      ASSERT(day >= 1 && day <= DaysInMonth(year, month))

      year -= 1;

      U32 dow = mdays[month - 1];
      dow += year;
      dow += year / 4;
      dow -= year / 100;
      dow += year / 400;
      dow += day;
      dow += (LeapYear(year + 1) && (month >= 2)) ? 1 : 0;
      dow %= 7;

      dow += 1;

      return (dow);
    }


    //
    // Is the given year a leap year ?
    //
    Bool LeapYear(U32 year)
    {
      return (!(year % 4) && ((year % 100) || !(year % 400)));
    }


    //
    // Given a month and year, return number of days in the month
    //
    U32 DaysInMonth(U32 year, U32 month)
    {
      ASSERT(year > 1752)
      ASSERT(month >= 1 && month <= 12)
      return (days[month - 1] + ((LeapYear(year) && (month == 2)) ? 1 : 0));
    }


    //
    // Given a month index return a string
    //
    const char * GetMonthStr(U32 month)
    {
      ASSERT(month >= 1 && month <= 12)
      return (months[month - 1]);
    }


    //
    // Given a day of the week return a string
    //
    const char * GetDayStr(U32 day)
    {
      ASSERT(day >= 1 && day <= 7)
      return (daysOfWeek[day - 1]);
    }

  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace CycleTimer
  //
  namespace CycleTimer
  {
    // RDTSC data
    U32 oh;
    U32 lo;
    U32 hi;


    //
    // Start the cycle timer
    //
    void Start()
    {
      __asm
      {
        _emit   0x0F                    // rdtsc
        _emit   0x31
        mov     ebx, eax
        _emit   0x0F                    // rdtsc
        _emit   0x31
        sub     eax, ebx
        mov     [oh], eax               // store overhead

        _emit   0x0F                    // rdtsc
        _emit   0x31
        mov     lo, eax
        mov     hi, edx
      }
    }


    //
    // Stop the cycle timer
    //
    void Stop()
    {
      __asm
      {
        _emit   0x0F                    // rdtsc
        _emit   0x31
        sub     eax, [lo]
        sbb     edx, [hi]
        sub     eax, [oh]
        sbb     edx, 0
        mov     [lo], eax
        mov     [hi], edx
      }
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Watch
  //

  //
  // Static data
  //
  static char reportBuf[128];


  //
  // Reset results
  //
  void Watch::Reset()
  {
    minimum = 0xFFFFFFFF;
    samples = 0;
    maximum = 0;
    smooth  = 0;
    sum     = 0;
  }

  
  //
  // Add a sample
  //
  void Watch::Sample(U32 sample)
  {
    samples++;
    minimum = Min(minimum, sample);
    maximum = Max(maximum, sample);
    smooth = smooth ? (smooth * 240 + sample * 16) >> 8 : sample;
    sum += sample;
  }


  //
  // Return average
  //
  U32 Watch::GetAvg() 
  { 
    return (samples ? U32(sum / U64(samples)) : 0); 
  }


  //
  // Return a formatted report string
  //
  const char *Watch::Report()
  {
    S32 n;

    n = sprintf
    (
      reportBuf, 
      "%6d\xE4 %8d\x19 %8d\x18 %8d\xE6 %8d\xF7", 
      samples,
      minimum,
      maximum,
      GetAvg(),
      smooth
    );

    // Check for buffer overflow
    ASSERT(n < sizeof(reportBuf));

    return (reportBuf);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class CycleWatch
  //

  //
  // Start the timer
  //
  void CycleWatch::Start()
  {
    if (!on)
    {
      on = TRUE;

      // Record start time
      __asm
      {
        push    eax
        push    edx
        push    ecx

        mov     ecx, this
        _emit   0x0F
        _emit   0x31
        mov     [ecx]this.tscLo, eax
        mov     [ecx]this.tscHi, edx

        pop     ecx
        pop     edx
        pop     eax
      }
    }
  }


  //
  // Stop the timer and accumulate statistics
  //
  void CycleWatch::Stop()
  {
    __asm
    {
      push    eax
      push    edx
      push    ecx

      _emit   0x0F
      _emit   0x31
      mov     ecx, this
      sub     eax, [ecx]this.tscLo
      sbb     edx, [ecx]this.tscHi
      mov     [ecx]this.tscLo, eax
      mov     [ecx]this.tscHi, edx

      pop     ecx
      pop     edx
      pop     eax
    }

    if (on)
    {
      // If timer overflowed pass in U32_MAX
      Sample(tscHi ? U32_MAX : tscLo);
      on = FALSE;
    }
  }
}
