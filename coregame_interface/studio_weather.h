///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// studio weather editor
//
// 28-OCT-1999
//


#ifndef __STUDIO_WEATHER_H
#define __STUDIO_WEATHER_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_messagebox.h"
#include "icwindow.h"
#include "iclistbox.h"
#include "icdroplist.h"
#include "fontsys.h"
#include "input.h"

///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Environment
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Options : video options dialog
  //
  class Weather : public ICWindow
  {
    PROMOTE_LINK(Weather, ICWindow, 0x43A3F670); // "Weather"

  protected:
    U32  FillRainTypes();
    U32  FillSplatTypes();

    void SetSelRainType( U32 sel);

    ICListBox * RainTypesBox()
    {
      return (ICListBox *) Find(Crc::CalcStr("RainTypes"));
    }
    ICListBox * SplatTypesBox()
    {
      return (ICListBox *) Find(Crc::CalcStr("SplatTypes"));
    }
    ICListBox * SplatTypesDropGround()
    {
      ICDropList * box = (ICDropList *) Find(Crc::CalcStr("SplatGroundList"));

      return box ? box->GetListBox() : NULL;
    }
    ICListBox * SplatTypesDropWater()
    {
      ICDropList * box = (ICDropList *) Find(Crc::CalcStr("SplatWaterList"));

      return box ? box->GetListBox() : NULL;
    }

    // enable/disable controls based on context
    //
    void Check();

  public:

    // Constructor 
    Weather(IControl *parent);
    ~Weather();

    // Event handler
    U32 HandleEvent(Event &e);
  };

}


#endif