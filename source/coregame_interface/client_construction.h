///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Unit Construction Control
//
// 3-JUN-1999
//


#ifndef __CLIENT_CONSTRUCTION_H
#define __CLIENT_CONSTRUCTION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "iconwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class ConstructionIcon - A single construction icon
  //
  class ConstructionIcon : public IControl
  {
    PROMOTE_LINK(Icon, IControl, 0x99120ED6); // "Client::ConstructionIcon"

  public:

    // Possible icon modes
    enum Mode { MODE_FACILITY, MODE_RIG };

  protected:
  
    // The overlay texture
    TextureInfo *textureOverlay;

    // The position of the queue count
    Point<S32> pointQueue;

    // The position of the cost
    Point<S32> pointCost;

    // The progress bar area
    Area<S32> areaProgress;

    // Type this icon is representing
    UnitObjTypePtr construct;

    // The constructor this icon is currently being used for
    UnitObjPtr constructor;

    // Current mode
    Mode mode;

    // State flags
    enum 
    { 
      FLAG_VALID        = 0x00000001,     // Setup for use
      FLAG_PREREQ       = 0x00000002,     // Have the prereqs for type
      FLAG_ACTIVE       = 0x00000004,     // Currently available for use
      FLAG_OFFMAP       = 0x00000008,     // An offmap object is ready for use
      FLAG_PROGRESS     = 0x00000010,     // Progress display required
      FLAG_HIGHLIGHTED  = 0x00000020,     // Highlighted (flashing and stuff)
      FLAG_LIMITED      = 0x00000040,     // Unit limit has been reached
    };

    // Current state flags
    U32 flags;

    // Used to display task progress
    F32 progress;

    // Construction queue
    S32 queue;

    // Display the prereqs for this icon
    void DisplayPrereqs();

    // Cycle based processing
    void Poll();

    // Draw control
    void DrawSelf(PaintInfo &pi);

    // Process the given facility operation
    void ProcessFacility(U32 event);

    // Process the given rig operation
    void ProcessRig(U32 event);

    // Process the given operation
    void ProcessOperation(U32 event);
  
  public:

    // Constructor and destructor
    ConstructionIcon(IControl *parent, Mode m, UnitObjType *type, UnitObj *unit);
    ~ConstructionIcon();

    // Configure control from an FScope
    void Setup(FScope *fScope);

    // Control methods
    U32 HandleEvent(Event &e);

    // Activate this control
    Bool Activate();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Construction - Unit construction window
  //
  class Construction : public IconWindow
  {
    PROMOTE_LINK(Construction, IconWindow, 0xE20500B8); // "Client::Construction"
  
  protected:

    // The current constructor
    UnitObjPtr constructor;

    // Cycle based processing
    void Poll();

  public:

    // Constructor and destructor
    Construction(IControl *parent);
    ~Construction();

    // Build the icon list using the given constructor
    Bool Generate(UnitObj *target);

    // Refresh the list for the current constructor if matches target
    void Refresh(UnitObj *target);

    // Returns the current constructor, or NULL
    UnitObj * GetConstructor();

  };
}

#endif
