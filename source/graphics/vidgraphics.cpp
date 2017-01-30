///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// vidgraphics.cpp
//
// 15-NOV-1999      John Cooke
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "iface.h"
#include "iface_priv.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  //
  // Constructor
  //
  Graphics::Graphics(IControl *parent) : ICWindow(parent)
  {
  }
  //-----------------------------------------------------------------------------

  //
  // Destructor
  //
  Graphics::~Graphics()
  {
  }
  //-----------------------------------------------------------------------------

  void Graphics::OnModeChange()
  {
    if (IFace::sysInit)
    {
      Graphics * con = (Graphics *) IFace::FindByName( "Vid::Graphics");
      if (con)
      {
        con->_OnModeChange();
      }

      con = (Graphics *) IFace::FindByName( "View::Graphics");
      if (con)
      {
        con->_OnModeChange();
      }
    }
  }
  //-----------------------------------------------------------------------------

  void Graphics::_OnModeChange()
  {
    SendNotify( this, Vid::caps.clipGuard ? 0x92DDA669 : 0x7CA5E1CC); // "Vid::Graphics:GuardOff"
  }
  //-----------------------------------------------------------------------------

  U32 Graphics::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          U32 id = e.iface.p1;

          switch (id)
          {
            case IControlNotify::Activated:
              Command::OnModeChange();
              _OnModeChange();
              break;
          }
        }
      }
    }
    return (ICWindow::HandleEvent(e));
  }
  //-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------