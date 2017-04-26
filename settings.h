///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Global application settings - included by std.h
//

#ifdef DEVELOPMENT

  // Activate assertions
  #define ASSERTIONS_ACTIVE

#else

  // Disable the mono
  #define MONO_DISABLED

#endif

// XMM build
//#define __DO_XMM_BUILD

// Demo build
//#define DEMO

// Sync checking build
//#define SYNC_BRUTAL_ACTIVE
