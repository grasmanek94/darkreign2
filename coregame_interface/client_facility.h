///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Facility Shortcut Control
//
// 17-SEP-1999
//


#ifndef __CLIENT_FACILITY_H
#define __CLIENT_FACILITY_H


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
  // Class FacilityIcon - A single facility icon
  //
  class FacilityIcon : public IControl
  {
    PROMOTE_LINK(Icon, IControl, 0x0A89CE92); // "Client::FacilityIcon"

  protected:

    // Current progress value
    F32 progress;

    // Current progress type
    U32 progressType;

    // Generic count
    U32 count;

    // The facility this icon is representing
    UnitObjPtr facility;

    // The overlay texture
    TextureInfo *textureOverlay;

    // The position of the count
    Point<S32> pointCount;

    // The progress bar area
    Area<S32> areaProgress;

    // The health bar area
    Area<S32> areaHealth;

    // The alpha values to use for a bar
    Point<F32> alphas;

    // Cycle based processing
    void Poll();

    // Draw control
    void DrawSelf(PaintInfo &pi);

    // Process the given operation
    void ProcessOperation(U32 event);
  
  public:

    // Constructor and destructor
    FacilityIcon(IControl *parent, UnitObj *facility);
    ~FacilityIcon();

    // Configure control from an FScope
    void Setup(FScope *fScope);

    // Control methods
    U32 HandleEvent(Event &e);

    // Activate this control
    Bool Activate();

    // Returns the facilty for this icon, or NULL if none
    UnitObj * GetFacility();
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Facility - Facility shortcut control
  //
  class Facility : public IconWindow
  {
    PROMOTE_LINK(Construction, IconWindow, 0xE23D708F); // "Client::Facility"

  protected:

    // Cycle based processing
    void Poll();
 
  public:

    // Constructor and destructor
    Facility(IControl *parent);
    ~Facility();

    // Get the icon for a particular object
    FacilityIcon * GetIcon(UnitObj *facility);

    // Add a facility to the bar
    void Add(UnitObj *facility);
  };
}

#endif
