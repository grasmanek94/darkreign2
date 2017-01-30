///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Squad Control
//
// 15-JAN-1999
//


#ifndef __CLIENT_SQUADCONTROL_H
#define __CLIENT_SQUADCONTROL_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "squadobj.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SquadControl
  //
  class SquadControl : public IControl
  {
    PROMOTE_LINK(SquadControl, IControl, 0x8CA88CEB) // "SquadControl"

  protected:

    // Squad this control is associated with
    SquadObjPtr squad;

    // Client rectangles for each state
    ClipRect clientRects[2];

    // Is the control depressed
    Bool down;

    // IFaceVars
    IFaceVar *reset;

    // Location of squad number
    Point<S32> number;

    // Location of the count
    Point<S32> count;

    // Location and size of health
    Area<S32> health;

    // Current task
    IFaceVar *task;

    // Client-side squad identifier
    S32 clientId;

    // SquadControl node
    NList<SquadControl>::Node node;

    // All SquadControl objects
    static NList<SquadControl> allSquadControls;

  public:

    // Constructor and destructor
    SquadControl(IControl *parent);
    ~SquadControl();

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Control methods
    void DrawSelf(PaintInfo &pi);

    // Handle Event
    U32 HandleEvent(Event &e);

    // Adjust geometry of control
    void AdjustGeometry();

    // Activate
    Bool Activate();

    // Deactivate
    Bool Deactivate();

  protected:

    // Set button state
    void SetState(Bool state);

    // Var changed
    void Notify(IFaceVar *var);

  public:

    // Notification
    static Bool Notify(GameObj *from, U32 message, U32 param1, U32 param2);

    // Map a squad id to a client id
    static Bool MapSquadToClient(U32 squadId, U32 &id);

    // Map a squad id to a control
    static SquadControl * MapSquadToControl(U32 squadId);

    // Save state of all controls
    static void Save(FScope *scope);

    // Load state of all controls
    static void Load(FScope *scope);

  };

}

#endif
