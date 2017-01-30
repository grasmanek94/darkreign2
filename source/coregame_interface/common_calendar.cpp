///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Calendar
//
// 20-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_calendar.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_types.h"
#include "ifvar.h"
#include "font.h"
#include "clock.h"
#include "babel.h"
#include "stdload.h"
#include "input.h"
#include "environment_time.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Calendar
  //


  //
  // Calendar::Calendar
  //
  // Constructor
  //
  Calendar::Calendar(IControl *parent) : 
    IControl(parent)
  {
    year = new IFaceVar(this, CreateInteger("Year", 2000, 1753, 999999));
    month = new IFaceVar(this, CreateInteger("Month", 1, 1, 12));
    day = new IFaceVar(this, CreateInteger("Day", 1, 1, 31));

    // Set the default width and height
    itemWidth = 25;
    itemHeight = 20;
  }


  //
  // Calendar::Calendar
  //
  Calendar::~Calendar()
  {
    delete year;
    delete month;
    delete day;
  }


  //
  // Calendar::Setup
  //
  // Setup this control from one scope function
  //
  void Calendar::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xD51072E9: // "ItemWidth"
        itemWidth = StdLoad::TypeU32(fScope);
        break;

      case 0x5D3E96A9: // "ItemHeight"
        itemHeight = StdLoad::TypeU32(fScope);
        break;

      default:
        IControl::Setup(fScope);
        break;
    }
  }


  //
  // Calendar::Notify
  //
  // Handle notifications
  //
  void Calendar::Notify(IFaceVar *var)
  {
    if (var == month)
    {
      day->SetIntegerValue(Min<U32>(day->GetIntegerValue(), Clock::Date::DaysInMonth(year->GetIntegerValue(), month->GetIntegerValue())));
    }
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 Calendar::HandleEvent(Event &e)
  {
    if (e.type == ::Input::EventID())
    {
      // Input events
      switch (e.subType)
      {
        case ::Input::MOUSEBUTTONDOWN:
        case ::Input::MOUSEBUTTONDBLCLK:
        {
          if (e.input.code == ::Input::LeftButtonCode())
          {
            Point<S32> mouse(e.input.mouseX, e.input.mouseY);
            if (InClient(mouse))
            {
              // Figure out which day (if any) was clicked on
              mouse = ScreenToClient(mouse);
              S32 index = mouse.x / itemWidth + (mouse.y / itemHeight * 7) - Clock::Date::DayOfWeek(year->GetIntegerValue(), month->GetIntegerValue()) + 2;

              if (index >= 1 && index <= (S32) Clock::Date::DaysInMonth(year->GetIntegerValue(), month->GetIntegerValue()))
              {
                day->SetIntegerValue(index);
              }
            }
          }
          break;
        }
      }
    }
    else if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case 0x5A5D4C5A: // "Grab"
              Environment::Time::SetYear(year->GetIntegerValue());
              Environment::Time::SetMonth(month->GetIntegerValue());
              Environment::Time::SetDayOfMonth(day->GetIntegerValue());
              break;
          }
        }
      }
    }

    return (IControl::HandleEvent(e));  
  }


  //
  // Calendar::DrawSelf
  //
  // Control draws itself
  //
  void Calendar::DrawSelf(PaintInfo &pi)
  {
    if (!pi.font)
    {
      ERR_CONFIG(("Calendar must have a font"))
    }

    // Figure out the first day of the month

    U32 dow = Clock::Date::DayOfWeek(year->GetIntegerValue(), month->GetIntegerValue()) - 1;
    U32 first = 1;
    U32 last = 7 - dow;
    U32 line;
    U32 row = dow;

    for (line = 0; line < 6; ++line)
    {
      while (first <= last)
      {
        U32 x = pi.client.p0.x + row * itemWidth;
        U32 y = pi.client.p0.y + line * itemHeight;

        U32 space = IFace::GetMetric(IFace::DROPSHADOW_DOWN) + 1;

        ClipRect rect(
          x, y, 
          x + itemWidth - space, 
          y + itemHeight - space);

        const CH * str = TRANSLATE(("{1:d}", 1, first));
        U32 len = Utils::Strlen(str);

        // Highlight the day
        if (first == (U32) day->GetIntegerValue())
        {
          IFace::RenderGradient(rect, pi.colors->bg[ColorGroup::SELECTED], IFace::GetMetric(IFace::VGRADIENT), TRUE);
          pi.font->Draw(
            x + (itemWidth - space - pi.font->Width(str, len)) / 2, 
            y + (itemHeight - space - pi.font->Height()) / 2, 
            str, len, pi.colors->fg[ColorGroup::SELECTED], &pi.client);
        }
        else
        {
          IFace::RenderGradient(rect, pi.colors->bg[ColorGroup::NORMAL], IFace::GetMetric(IFace::VGRADIENT), TRUE);
          pi.font->Draw(
            x + (itemWidth - space - pi.font->Width(str, len)) / 2, 
            y + (itemHeight - space - pi.font->Height()) / 2, 
            str, len, pi.colors->fg[ColorGroup::NORMAL], &pi.client);
        }

        // Draw the drop shadow
        IFace::RenderShadow(rect, rect + IFace::GetMetric(IFace::DROPSHADOW_DOWN), Color(0, 0, 0, IFace::GetMetric(IFace::SHADOW_ALPHA)), 0);
  
        first++;
        row++;
      }

      U32 days = Clock::Date::DaysInMonth(year->GetIntegerValue(), month->GetIntegerValue());
      row = 0;
      last = (last + 7) > days ? days: last + 7;
    }
  }


  //
  // Calendar::Activate
  //
  Bool Calendar::Activate()
  {
    if (IControl::Activate())
    {
      ActivateVar(year);
      ActivateVar(month);
      ActivateVar(day);

      year->SetIntegerValue(Environment::Time::GetYear());
      month->SetIntegerValue(Environment::Time::GetMonth());
      day->SetIntegerValue(Environment::Time::GetDayOfMonth());

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Calendar::Deactivate
  //
  Bool Calendar::Deactivate()
  {
    if (IControl::Deactivate())
    {
      year->Deactivate();
      month->Deactivate();
      day->Deactivate();

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


}
