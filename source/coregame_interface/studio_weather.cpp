///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// environment_weather.cpp
//
// 28-OCT-1999      John Cooke
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "environment_rain.h"
#include "environment_rain_priv.h"
#include "studio_weather.h"
#include "utiltypes.h"
#include "stdload.h"
#include "effects_utils.h"
#include "console.h"
#include "gametime.h"
#include "terrain.h"
#include "random.h"
//-----------------------------------------------------------------------------

namespace Studio
{
  //
  // Constructor
  //
  Weather::Weather(IControl *parent) : ICWindow(parent)
  {
  }
  //-----------------------------------------------------------------------------

  //
  // Destructor
  //
  Weather::~Weather()
  {
  }
  //-----------------------------------------------------------------------------

  void Weather::Check()
  {
    Find( Crc::CalcStr( "RainButton"))->SetEnabled( Environment::Rain::typeList.GetCount() ? TRUE : FALSE);    
  }
  //-----------------------------------------------------------------------------

  U32 Weather::FillRainTypes()
  {
    U32 ret = 0;

    ICListBox * box = RainTypesBox();
    if (box)
    {
      // Rebuild the list
      box->DeleteAllItems();

      BinTree<Environment::Rain::Type>::Iterator i(&Environment::Rain::typeList);
      U32 c = 0;
      while (Environment::Rain::Type * t = i++)
      {
        box->AddTextItem( t->name.str, NULL);

        if (t == Environment::Rain::type)
        {
          // current type
          ret = c;
        }
        c++;
      }
    }

    Check();

    return ret;
  }
  //-----------------------------------------------------------------------------

  U32 Weather::FillSplatTypes()
  {
    U32 ret = 0;

    ICListBox * box = SplatTypesBox();
    if (box)
    {
      // Rebuild the list
      box->DeleteAllItems();
    }
    ICListBox *dropG = SplatTypesDropGround();
    if (dropG)
    {
      dropG->DeleteAllItems();
    }
    ICListBox *dropW = SplatTypesDropWater();
    if (dropW)
    {
      dropW->DeleteAllItems();
    }

    BinTree<Environment::Rain::Splat::Type>::Iterator i(&Environment::Rain::Splat::typeList);
    U32 c = 0;
    while (Environment::Rain::Splat::Type * t = i++)
    {
      if (box)
      {
        box->AddTextItem( t->name.str, NULL);
      }
      if (dropG && t->type == Environment::Rain::Splat::Type::GROUND)
      {
        dropG->AddTextItem( t->name.str, NULL);
      }
      if (dropW && t->type == Environment::Rain::Splat::Type::WATER)
      {
        dropW->AddTextItem( t->name.str, NULL);
      }

      if (t == Environment::Rain::Splat::type)
      {
        // current type
        ret = c;
      }

      c++;
    }

    if (dropG)
    {
      dropG->AddTextItem( "none", NULL);
    }
    if (dropW)
    {
      dropW->AddTextItem( "none", NULL);
    }

    SetSelRainType( ret);

    return ret;
  }
  //-----------------------------------------------------------------------------

  void Weather::SetSelRainType( U32 sel)
  {
    ICListBox * box = RainTypesBox();
    if (box)
    {
      box->SetSelectedItem( sel); 
    }
  }
  //-----------------------------------------------------------------------------

  U32 Weather::HandleEvent(Event &e)
  {
    static S32 mode, lastMode, dd, lastDD;
    static U32 timer;

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
            case 0xF70C251F: // "FillRainTypes"
            {
              FillRainTypes();

//              U32 sel = FillRainTypes();
//              SetSelRainType( sel);
 
              // fall through              
            }
            case 0xE494073D: // "FillSplatTypes"
            {
              FillSplatTypes();
              break;
            }
          }
        }
      }
    }

    return (ICWindow::HandleEvent(e));
  }
  //-----------------------------------------------------------------------------
}
//-----------------------------------------------------------------------------