///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Scorch Control
//
// 15-JAN-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_scorchcontrol.h"
#include "client.h"
#include "client_private.h"
#include "ifvar.h"
#include "iface.h"
#include "iface_types.h"
#include "orders_game.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//

namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Scorch
  //


  //
  // Constructor
  //
  Scorch::Scorch(IControl *parent) 
  : ICWindow(parent)
  {
    // Create iface vars
    horizontal = new IFaceVar(this, CreateFloat("horizontal", 0.0f, -180.0f, 180.0f));
    vertical = new IFaceVar(this, CreateFloat("vertical", 0.0f, 0.0f, 90.0f));
    speed = new IFaceVar(this, CreateFloat("speed", 50.0f, 0.0f, 500.0f));
  }


  //
  // Destructor
  //
  Scorch::~Scorch()
  {
    // Delete the IFace vars
    delete horizontal;
    delete vertical;
    delete speed;
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 Scorch::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Handle other notifications
          switch (e.iface.p1)
          {
            case 0xEBEB1BE9: // "Fire"
              Events::UpdateSelectedLists();
              Orders::Game::Fire::Generate(
                GetPlayer(),
                PIBY2 - (horizontal->GetFloatValue() * DEG2RAD),
                PIBY2 - (vertical->GetFloatValue() * DEG2RAD),
                speed->GetFloatValue());
              break;

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
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
  Bool Scorch::Activate()
  {
    if (ICWindow::Activate())
    {
      ActivateVar(horizontal);
      ActivateVar(vertical);
      ActivateVar(speed);
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool Scorch::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      horizontal->Deactivate();
      vertical->Deactivate();
      speed->Deactivate();
      return (TRUE);
    }
    return (FALSE);
  }

}
