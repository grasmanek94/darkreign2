///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Performance monitoring
//
// 28-JUL-1998
//


#ifndef __PERFSTATS_H
#define __PERFSTATS_H




//#if 0
#ifdef DEVELOPMENT

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace PerfStats - Real time profiling
//
namespace PerfStats
{

  // Initialise PerfStats
  void Init();

  // Shutdown PerfStats
  void Done();

  // Start a timer, create it if it doesnt exist
  void Start(const char *s, Bool atRoot = FALSE);

  // Stop a timer
  void Stop(const char *s);

  // Display the performance stats on the mono
  void Display();

  // Reset all statistics
  void Reset();

  // Turn performance stats on or off
  void EnableDisplay(Bool b = TRUE);

  // Set display refresh rate
  void SetUpdateRate(U32 n);
};


//
// PerfStat macros
//
#define PERF_INIT     PerfStats::Init();
#define PERF_DONE     PerfStats::Done();
#define PERF_S(s)     PerfStats::Start(s);
#define PERF_SROOT(s) PerfStats::Start(s, TRUE);
#define PERF_E(s)     PerfStats::Stop(s);
#define PERF_REDRAW   PerfStats::Display();

#if 0

#define PERF_X_S(s)   PERF_S(s)
#define PERF_X_E(s)   PERF_E(s)

#else

#define PERF_X_S(s)
#define PERF_X_E(s)

#endif

#else

// Performance stats are disabled in a non DEVELOPMENT build
#define PERF_INIT
#define PERF_DONE
#define PERF_S(s)
#define PERF_SROOT(s)
#define PERF_E(s)
#define PERF_REDRAW

#define PERF_X_S(s)
#define PERF_X_E(s)

#endif

#endif
