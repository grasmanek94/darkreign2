///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Login Interface Control
//
// 15-FEB-2000
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_login.h"
#include "user.h"
#include "ifvar.h"
#include "iface.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Login
  //

  //
  // Constructor
  //
  Login::Login(IControl *parent) : ICWindow(parent)
  {
    user = new IFaceVar(this, CreateString("user", ""));
    newUser = new IFaceVar(this, CreateString("newUser", ""));
  }


  //
  // Destructor
  //
  Login::~Login()
  {
    delete user;
    delete newUser;
  }


  //
  // Notify
  //
  // Notification that a local var has changed value
  //
  void Login::Notify(IFaceVar *)
  {
  }


  //
  // BuildUserList
  //
  // Build the list of users
  //
  void Login::BuildUserList()
  {
    // Do we have a pointer to the list
    if (userList.Alive())
    {
      // Save the name of the current user
      const char *lastUser = User::LoggedIn() ? User::GetName() : NULL;

      // Clear the current list of users
      userList->DeleteAllItems();

      // Add each known user to the list
      for (NBinTree<User::ExistingUser>::Iterator i(&User::GetExistingUsers()); *i; ++i)
      {
        userList->AddTextItem((*i)->GetName().str, NULL);
      }

      // Select the current user
      if (lastUser)
      {
        userList->SetSelectedItem(lastUser);
      }
    }
  }

  
  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 Login::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case LoginMsg::Select:
              return (TRUE);

            case LoginMsg::Login:
            {
              // Get the selected user
              if (const User::ExistingUser *e = User::Find(user->GetStringValue()))
              {
                // Login the selected user
                if (User::Login(e))
                {
                  User::SampleStatCount("Logins");
                  SendNotify(this, LoginNotify::LoggedIn);
                }
              }

              return (TRUE);
            }

            case LoginMsg::Create:
            {
              // Get the name of the new user
              const char *userName = newUser->GetStringValue();

              // Filter empty strings and existing users
              if ((*userName != '\0') && !User::Find(userName))
              {
                // Create the new user
                if (User::Create(userName))
                {
                  // Build the user list
                  BuildUserList();
                }
                else
                {
                  LOG_WARN(("Creation of user [%s] failed", userName));
                }
              }

              return (TRUE);
            }

            case LoginMsg::Delete:
            {
              // Attempt to delete the user
              if (User::Delete(user->GetStringValue()))
              {
                // Rebuild the list
                BuildUserList();
              }

              return (TRUE);
            }
          }

          break;
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool Login::Activate()
  {
    if (ICWindow::Activate())
    {
      // Activate the vars
      ActivateVar(user);
      ActivateVar(newUser);

      // Find the list of users
      userList = IFace::Find<ICListBox>("UserList", this, TRUE);

      // Build the list of users
      BuildUserList();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool Login::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      user->Deactivate();
      newUser->Deactivate();

      if (userList.Alive())
      {
        userList->DeleteAllItems();
      }

      return (TRUE);
    }

    return (FALSE);
  }
}
