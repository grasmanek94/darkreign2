///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Login Interface Control
//
// 15-FEB-2000
//


#ifndef __GAME_LOGIN_H
#define __GAME_LOGIN_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace LoginMsg
  //
  namespace LoginMsg
  {
    const U32 Select = 0x82CDD7C7; // "Game::Login::Message::Select"
    const U32 Login = 0x8FE1ACB5; // "Game::Login::Message::Login"
    const U32 Create = 0x0116ABE7; // "Game::Login::Message::Create"
    const U32 Delete = 0x1A1A1C36; // "Game::Login::Message::Delete"
  }

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace LoginNotify
  //
  namespace LoginNotify
  {
    const U32 LoggedIn = 0xED05F118; // "Game::Login::Notify::LoggedIn"
  };



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Login
  //
  class Login : public ICWindow
  {
    PROMOTE_LINK(Login, ICWindow, 0xE3653A1E); // "Login"

  protected:

    // Name of the currently selected user
    IFaceVar *user;

    // Name of a new user to create
    IFaceVar *newUser;

    // List of users
    ICListBoxPtr userList;

  protected:

    // Notification that a local var has changed value
    void Notify(IFaceVar *var);

    // Build the list of users
    void BuildUserList();

  public:

    // Constructor
    Login(IControl *parent);
    ~Login();

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();
  };
}

#endif
