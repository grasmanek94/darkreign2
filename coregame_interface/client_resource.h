///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Resource Control
//


#ifndef __CLIENT_RESOURCE_H
#define __CLIENT_RESOURCE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"


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
  class Resource : public IControl
  {
    PROMOTE_LINK(Resource, IControl, 0x85A65124); // "Client::Resource"

  private:

    IFaceVar *resource;

  public:

    // Constructor and destructor
    Resource(IControl *parent);
    ~Resource();

    // Activate
    Bool Activate();

    // Deactivate
    Bool Deactivate();

    // Draw this control
    void DrawSelf(PaintInfo &pi);

  };

}

#endif
