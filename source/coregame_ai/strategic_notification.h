/////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Strategic Object
//
// 31-AUG-1998
//


#ifndef __STRATEGIC_NOTIFICATION_H
#define __STRATEGIC_NOTIFICATION_H


/////////////////////////////////////////////////////////////////////////////
//
// NameSpace Strategic
//
namespace Strategic
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // Class Notification
  //
  struct Notification
  {
    // From
    GameObjPtr from;

    // The message
    U32 message;

    // The parameters
    U32 param1;
    U32 param2;

    // List node
    NList<Notification>::Node node;

    // Constructor
    Notification(GameObj *from, U32 message, U32 param1, U32 param2)
    : from(from),
      message(message),
      param1(param1),
      param2(param2)
    {
    }

  };

}

#endif