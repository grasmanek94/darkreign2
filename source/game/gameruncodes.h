///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Game RunCodes
//
// 16-FEB-2000
//


#ifndef __GAMERUNCODES_H
#define __GAMERUNCODES_H


///////////////////////////////////////////////////////////////////////////////
//
// Namespace GameRunCodes - Manages all top level game runcodes
//
namespace GameRunCodes
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace KeyCheck - Check the CD key
  //
  namespace KeyCheck
  {
    void Init();
    void Process();
    void Done();
    U32 Notify(U32);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Intro - Displays intro movies etc
  //
  namespace Intro
  {
    void Init();
    void Process();
    void Done();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Login - Provides user profile selection and login
  //
  namespace Login
  {
    void Init();
    void Process();
    void Done();
    U32 Notify(U32);
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Shell - Shell mission selection
  //
  namespace Shell
  {
    void Init();
    void Process();
    void Done();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Mission - Mission processing
  //
  namespace Mission
  {
    void Init();
    void Process();
    void Done();
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Outro - Displays outro movies etc
  //
  namespace Outro
  {
    void Init();
    void Process();
    void Done();
  }

}

#endif
