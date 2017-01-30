///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// MultiPlayer Stuff
// 1-JUL-1999
//


#ifndef __MULTIPLAYER_CONTROLS_CHATEDIT_H
#define __MULTIPLAYER_CONTROLS_CHATEDIT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "console_edit.h"
#include "ifvar.h"


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
    // Class ChatEdit
    //
    class ChatEdit : public ConsoleEdit
    {
    protected:
      
      // Chat message type (all, ally, team)
      IFaceVar *typeVar;

      // Prefix of cmd's to send
      StrBuf<64> cmdPrefix;
      U32 prefixLen;

    public:

      // Constructor
      ChatEdit(IControl *parent);

      // Destructor
      ~ChatEdit();

      // Setup
      void Setup(FScope *fScope);

      // HandleEvent
      U32 HandleEvent(Event &e);

    };

  }

}


#endif
