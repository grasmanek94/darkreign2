///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Benchmarking shell
//
// 29-APR-1998
//


#ifndef __TIMINGAPP_H
#define __TIMINGAPP_H


#define ACCEPTCR    0x00000001
#define TOLOWER     0x00000002
#define TOUPPER     0x00000004
#define ACCEPTINT   0x00000008


// Read a character or integer from stdin
U32 ReadInput(char *prompt, U32 flags = 0);

// Return string of last input
char *LastInput();

// Write a string to console from inside a test
void CDECL TestWrite(const char *format, ...);

// Format and display a cyclewatch object
void TestReport(const char *name, Clock::CycleWatch &t);

// Test function structure
typedef void (* TESTFUNC)(void);

struct TestFunc
{
  enum
  {
    LOW = 0x0001,
  };

  TESTFUNC func;
  const char *desc;
  U32 flags;
};


#endif
