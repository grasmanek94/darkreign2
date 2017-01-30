///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound Configuration Control
//
// 27-MAY-1999
//


#ifndef __ICSOUNDCONFIG_H
#define __ICSOUNDCONFIG_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icwindow.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSoundConfig - Sound configuration control
//
class ICSoundConfig : public ICWindow
{
  PROMOTE_LINK(ICSoundConfig, ICWindow, 0x4AD4A636); // "ICSoundConfig"

  // Provider index
  IFaceVar *providerIndex;
  IFaceVar *message2D;
  IFaceVar *message3D;
  IFaceVar *lastError;
  IFaceVar *digitalVolume;
  IFaceVar *reservedVolume;
  IFaceVar *redbookVolume;
  IFaceVar *redbookEnabled;

  ICListBoxPtr listBox;

  // Update state information
  void UpdateState();

  // Notification that a local var has changed value
  void Notify(IFaceVar *var);

public:

  // Constructor and destructor
  ICSoundConfig(IControl *parent);
  ~ICSoundConfig();

  // Control methods
  U32 HandleEvent(Event &e);

  // Control is being activated
  Bool Activate();
};

#endif
