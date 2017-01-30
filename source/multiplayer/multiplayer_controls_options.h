///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
//


#ifndef __MULTIPLAYER_CONTROLS_OPTIONS_H
#define __MULTIPLAYER_CONTROLS_OPTIONS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "ifvar.h"
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MultiPlayer
//
namespace MultiPlayer
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Controls
  //
  namespace Controls
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Options
    //
    class Options : public ICWindow
    {
    public:

      // Constructor
      Options(IControl *parent);

      // Destructor
      ~Options();

      // Activate
      Bool Activate();

      // Deactivate
      Bool Deactivate();

    protected:

      // Upload values
      void Upload();

      // Download values
      void Download();

    };

  }

}


#endif
