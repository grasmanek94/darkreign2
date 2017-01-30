///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Power Control
//
// 13-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_resource.h"
#include "team.h"
#include "ifvar.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//

namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Resource
  //


  //
  // Constructor
  //
  Resource::Resource(IControl *parent) 
  : IControl(parent)
  {
    resource = new IFaceVar(this, CreateInteger("resource", 0));
  }


  //
  // Destructor
  //
  Resource::~Resource()
  {
    delete resource;
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool Resource::Activate()
  {
    if (IControl::Activate())
    {
      ActivateVar(resource);
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool Resource::Deactivate()
  {
    if (IControl::Deactivate())
    {
      resource->Deactivate();
      return (TRUE);
    }
    return (FALSE);
  }


  //
  // Draw this control
  //
  void Resource::DrawSelf(PaintInfo &pi)
  {
    // Get the display team
    Team *team = Team::GetDisplayTeam();
    if (team)
    {
      resource->SetIntegerValue(team->GetResourceStore());
    }
    else
    {
      resource->SetIntegerValue(0);
    }

    DrawCtrlBackground(pi, GetTexture());
    DrawCtrlFrame(pi);
  }

}
