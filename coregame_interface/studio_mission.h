///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Mission Based Configuration
//
// 20-APRIL-2000
//


#ifndef __STUDIO_MISSION_H
#define __STUDIO_MISSION_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"
#include "iclistbox.h"
#include "icstatic.h"
#include "icdroplist.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace MissionMsg
  //
  namespace MissionMsg
  {
    const U32 Done = 0x6692F0E0; // "Mission::Message::Done"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Mission
  //
  class Mission : public ICWindow
  {
    PROMOTE_LINK(Mission, ICWindow, 0xA293FAEB); // "Mission"

  protected:

    // Interface vars
    IFaceVar *defaultRule;
    IFaceVar *fixedRule;

    // List of rules
    ICListBoxPtr ruleList;

    // List of addons 
    ICListBoxPtr addonList;

  protected:

    // Callback for getting the selected addons
    static void GetAddonCallBack(const char *key, const CH *display, void *context);

  public:

    // Constructor
    Mission(IControl *parent);
    ~Mission();

    // Event handling
    U32 HandleEvent(Event &e);

    // Activate and deactivate this control
    Bool Activate();
    Bool Deactivate();

    // Download settings from the mission
    void Download();

    // Upload settings to the mission
    void Upload();
  };

}

#endif
