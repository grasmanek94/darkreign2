/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Recruitment
// 25-MAR-1999
//


#ifndef __STRATEGIC_SCRIPT_RECRUIT
#define __STRATEGIC_SCRIPT_RECRUIT


/////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "strategic_script.h"


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

	/////////////////////////////////////////////////////////////////////////////
	//
  // Class Script::Recruiter
  //
  class Script::Recruiter
  {
  public:

    // Destructor
    virtual ~Recruiter() { };

    // Inact the recruiter
    virtual void Execute(Script &script, U32 handle) = 0;

  public:

    // Create a recruiter
    static Recruiter * Create(Script &script, FScope *fScope);

  };

}

#endif