///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Light Time
//
// 1-MAR-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "vid_public.h"
#include "light_priv.h"
#include "environment.h"
#include "environment_light.h"
#include "utiltypes.h"
#include "stdload.h"
#include "gametime.h"
#include "unitobjctrl.h"
#include "random.h"
#include "message.h"


//////////////////////////////////////////////////////////////////////////////
//
// NameSpace Environment
//
namespace Environment
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Light
  //
  namespace Light
  {

    ///////////////////////////////////////////////////////////////////////////////
    //
    // Prototypes
    //
    static void CmdHandler(U32 pathCrc);


    // Initialization Flag
    static Bool initialized = FALSE;

    // The colors
    static LightColor *lightColors;

    // Cycle period
    static VarFloat cycleTime;

    // Current time (within a cycle)
    static F32 currentTime;

    // Last time (within a cycle)
    static F32 lastTime;

    // Percentage of day
    static F32 pctDay;

    // Start time
    static VarFloat startTime;

    // Day start/end time
    static VarFloat dayStartTime;
    static VarFloat dayEndTime;

    // Sunrise start/end time
    static VarFloat sunriseStartTime;
    static VarFloat sunriseEndTime;

    // Sunset start/end time
    static VarFloat sunsetStartTime;
    static VarFloat sunsetEndTime;

    // angular shift from pure east/west (in degrees)
    static VarFloat season;

    // Normalized time of day 
    static F32 azimuth;

    // Vars for development
    VarString  tempText;
    VarFloat   tempVal;


    //
    // Init
    //
    void Init()
    {
      ASSERT(!initialized)

      // Last time (within a cycle)
      lastTime = 0.3f;
      currentTime = 0.3f;
      pctDay = 0.0f;

      // Add a single color to the color tree
      AddColor(new LightColor(DefaultColor()));

      // Create the environment light vars
      VarSys::RegisterHandler("environment.light", CmdHandler);

      // Cycle period
      VarSys::CreateFloat("environment.light.cycletime", 600.0f, VarSys::DEFAULT, &cycleTime);

      // Current time (within a cycle)
      VarSys::CreateFloat("environment.light.startTime", 0.3f, VarSys::DEFAULT, &startTime)->SetFloatRange(0.0f, 1.0f);

      // Day start/end time
      VarSys::CreateFloat("environment.light.dayStartTime", 0.25f, VarSys::NOTIFY, &dayStartTime)->SetFloatRange(0.0f, 1.0f);
      VarSys::CreateFloat("environment.light.dayEndTime", 0.75f, VarSys::NOTIFY, &dayEndTime)->SetFloatRange(0.0f, 1.0f);

      // Sunrise start/end time
      VarSys::CreateFloat("environment.light.sunriseStartTime", 0.2f, VarSys::NOTIFY, &sunriseStartTime)->SetFloatRange(0.0f, 1.0f);
      VarSys::CreateFloat("environment.light.sunriseEndTime", 0.3f, VarSys::NOTIFY, &sunriseEndTime)->SetFloatRange(0.0f, 1.0f);

      // Sunset start/end time
      VarSys::CreateFloat("environment.light.sunsetStartTime", 0.7f, VarSys::NOTIFY, &sunsetStartTime)->SetFloatRange(0.0f, 1.0f);
      VarSys::CreateFloat("environment.light.sunsetEndTime", 0.8f, VarSys::NOTIFY, &sunsetEndTime)->SetFloatRange(0.0f, 1.0f);

      VarSys::CreateString("environment.light.temp.text", "", VarSys::DEFAULT, &tempText);
      VarSys::CreateFloat("environment.light.temp.val", 0, VarSys::DEFAULT, &tempVal);

      // angular shift from pure east/west (in degrees)
      VarSys::CreateFloat("environment.light.season", 0.0f, VarSys::NOTIFY, &season)->SetFloatRange(0.0f, 45.0f);

      Vid::Light::SetSunAngle( season);

      initialized = TRUE;
    }


    //
    // Done
    //
    void Done()
    {
      ASSERT(initialized)

      // Cleaup development vars
      VarSys::DeleteItem("environment.light");

      // Clear the colors
      ClearColors();

      initialized = FALSE;
    }

    //
    // Process
    //
    // Process LightTime
    //
    void Process()
    {
      ASSERT(initialized)
      ASSERT(*cycleTime)

      // Determine the current light time
      currentTime = (F32) fmod(GameTime::SimTotalTime(), cycleTime) / cycleTime;
      currentTime = (F32) fmod(currentTime + startTime, 1.0f);

      // Are we after sunrise and before sunset ?
      if (currentTime > sunriseStartTime && currentTime < sunsetStartTime)
      {
        tempText = "Day";
        tempVal = (sunsetStartTime - currentTime) * 24.0f * 60.0f;
      }
      else
      {
        tempText = "Night";
        tempVal = sunriseStartTime - currentTime;
        if (tempVal < 0.0f)
        {
          tempVal = tempVal + 1.0f;
        }
        tempVal = tempVal * 24.0f * 60.0f;
      }

      // Did we cross over from night to day ?
      if (lastTime < dayStartTime && currentTime >= dayStartTime ||
          lastTime >= dayEndTime && currentTime < dayEndTime)
      {
        Vid::Light::SwitchLights(FALSE);
      }

      // Did we cross over from day to night ?
      if (lastTime < dayEndTime && currentTime >= dayEndTime ||
          lastTime >= dayStartTime && currentTime < dayStartTime)
      {
        Vid::Light::SwitchLights(TRUE);
      }

      // Did sunrise begin ?
      if (lastTime < sunriseStartTime && currentTime >= sunriseStartTime)
      {
        Message::TriggerGameMessage(0x238FE3CE); // "Environment::Light::Sunrise::Start"
      }
      else
      // Did sunrise end ?
      if (lastTime < sunriseEndTime && currentTime >= sunriseEndTime)
      {
        Message::TriggerGameMessage(0x1A7A8E86); // "Environment::Light::Sunrise::End"
      }
      else
      // Did sunset begin ?
      if (lastTime < sunsetStartTime && currentTime >= sunsetStartTime)
      {
        Message::TriggerGameMessage(0x24DD24DE); // "Environment::Light::Sunset::Start"
      }
      else
      // Did sunset end ?
      if (lastTime < sunsetEndTime && currentTime >= sunsetEndTime)
      {
        Message::TriggerGameMessage(0xA357CA15); // "Environment::Light::Sunset::End"
      }

      MSWRITEV(11, (0, 0, "Current Time      : %8.2f", currentTime));
      MSWRITEV(11, (1, 0, "Cycle Time        : %8.2f", cycleTime));

      pctDay = CalcPercentageDay(currentTime);

      MSWRITEV(11, (2, 0, "Percentage Day    : %8.2f", pctDay));
      MSWRITEV(11, (3, 0, "Sunrise Start     : %8.2f", *sunriseStartTime));
      MSWRITEV(11, (4, 0, "Sunrise End       : %8.2f", *sunriseEndTime));
      MSWRITEV(11, (5, 0, "Sunset Start      : %8.2f", *sunsetStartTime));
      MSWRITEV(11, (6, 0, "Sunset End        : %8.2f", *sunsetEndTime));

      // Update types which are sensitive to time changes
      UnitObjCtrl::UpdateTimeSensitive(pctDay);

      // From the current time, determine the color we should be using
      LightColor color;
      color.fraction = currentTime;
      FillInColor(color);

      // Set the sunlight color
      F32 r, b, g;
      r = color.sun.r / 255.0f;
      g = color.sun.g / 255.0f;
      b = color.sun.b / 255.0f;

      MSWRITEV(11, (10, 0, "Sun     : %4.2f %4.2f %4.2f", r, g, b));

      F32 lightLength = sunsetEndTime - sunriseStartTime;

      // Work out the angle of the sun
      if (currentTime >= sunriseStartTime && currentTime < sunsetEndTime)
      {
        if (lightLength >= 0.0f)
        {
          // Calculate the azimuth
          azimuth = (currentTime - sunriseStartTime) / lightLength;
        }
        else
        {
          // It appears that day is inconsequential ... set it to 0
          azimuth = 0.0f;
        }
      }
      else
      {
        if (lightLength < 1.0f)
        {
          if (currentTime >= sunsetEndTime)
          {
            azimuth = (currentTime - sunsetEndTime) / (1.0f - lightLength);
          }
          else
          {
            azimuth = (currentTime + 1.0f - sunsetEndTime) / (1.0f - lightLength);
          }
        }
        else
        {
          // It appears that night is inconsequential ... set it to 0
          azimuth = 0.0f;
        }
      }

      MSWRITEV(11, (7, 0, "Azimuth           : %8.2f", azimuth));

      Vid::Light::SetSun(azimuth, r, g, b, IsSunUp());

      // Set the fog color
      r = color.fog.r / 255.0f;
      g = color.fog.g / 255.0f;
      b = color.fog.b / 255.0f;

      Vid::SetFogColor(r, g, b);

      MSWRITEV(11, (11, 0, "Fog     : %4.2f %4.2f %4.2f", r, g, b));

      // Set the ambient light in Vid
      r = color.ambient.r / 255.0f;
      g = color.ambient.g / 255.0f;
      b = color.ambient.b / 255.0f;

      Vid::SetAmbientColor(r, g, b);

      MSWRITEV(11, (9, 0, "Ambient : %4.2f %4.2f %4.2f", r, g, b));

      // Save the last time
      lastTime = currentTime;
    }


    //
    // IsDay
    //
    // Is it currently day time
    //
    Bool IsDay()
    {
      return ((currentTime >= dayStartTime && currentTime < dayEndTime) ? TRUE : FALSE);
    }


    //
    // IsNight
    //
    // Is it currently night time
    //
    Bool IsNight()
    {
      return (!IsDay());
    }


    //
    // GetPercentageDay
    //
    // What percentage day is it
    //
    F32 GetPercentageDay()
    {
      return (pctDay);
    }


    //
    // Calculate the percentage day given the percentage day
    //
    F32 CalcPercentageDay(F32 fraction)
    {
      F32 pctDay;

      if (fraction < sunriseStartTime)
      {
        pctDay = 0.0f;
      }
      else if (fraction < sunriseEndTime)
      {
        F32 diff = sunriseEndTime - sunriseStartTime;
        if (diff <= 0.0f)
        {
          pctDay = 0.0f;
        }
        else
        {
          pctDay = (fraction - sunriseStartTime) / diff;
        }
      }
      else if (fraction < sunsetStartTime)
      {
        pctDay = 1.0f;
      }
      else if (fraction < sunsetEndTime)
      {
        F32 diff = sunsetEndTime - sunsetStartTime;
        if (diff <= 0.0f)
        {
          pctDay = 0.0f;
        }
        else
        {
          pctDay = 1.0f - ((fraction - sunsetStartTime) / diff);
        }
      }
      else
      {
        pctDay = 0.0f;
      }

      return (pctDay);
    }


    //
    // Get the minimum and maximum percentage day for the supplied period
    //
    void GetPercentageDay(F32 start, F32 end, F32 &min, F32 &max)
    {
      F32 pct;

      min = 1.0f;
      max = 0.0f;

      // Get the pct at the start 
      pct = CalcPercentageDay(start);
      min = Min(min, pct);
      max = Max(max, pct);

      // Get the pct at the end
      pct = CalcPercentageDay(end);
      min = Min(min, pct);
      max = Max(max, pct);

      // Are there any critical points between start and end
      if (start < end)
      {
        if (start < sunriseStartTime && end > sunriseStartTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunriseStartTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
        if (start < sunriseEndTime && end > sunriseEndTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunriseEndTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
        if (start < sunsetStartTime && end > sunsetStartTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunsetStartTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
        if (start < sunsetEndTime && end > sunsetEndTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunsetEndTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
      }
      else
      {
        if (start > sunriseStartTime && end > sunriseStartTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunriseStartTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
        if (start > sunriseEndTime && end > sunriseEndTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunriseEndTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
        if (start > sunsetStartTime && end > sunsetStartTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunsetStartTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
        if (start > sunsetEndTime && end > sunsetEndTime)
        {
          // Get the pct at the sunrise start
          pct = CalcPercentageDay(sunsetEndTime);
          min = Min(min, pct);
          max = Max(max, pct);
        }
      }
    }


    //
    // IsSunUp
    //
    Bool IsSunUp()
    {
      return ((currentTime >= sunriseStartTime && currentTime < sunsetEndTime) ? TRUE : FALSE);
    }


    //
    // Azimuth
    //
    F32 Azimuth()
    {
      return (azimuth);
    }


    //
    // GetTime
    //
    U32 GetTime()
    {
      return (U32(currentTime * 24.0f * 60.0f * 60.0f));
    }


    //
    // GetCycleFraction
    //
    F32 GetCycleFraction()
    {
      return (currentTime);
    }


    //
    // LoadInfo
    //
    // Load information
    //
    void LoadInfo(FScope *fScope)
    {
      ASSERT(initialized)

      // Get all of the settings
      cycleTime = StdLoad::TypeF32(fScope, "CycleTime", 600.0f);
      currentTime = StdLoad::TypeF32(fScope, "CurrentTime", 0.3f);
      lastTime = StdLoad::TypeF32(fScope, "LastTime", 0.3f);
      startTime = StdLoad::TypeF32(fScope, "StartTime", 0.3f);
      dayStartTime = StdLoad::TypeF32(fScope, "DayStartTime", 0.25f);
      dayEndTime = StdLoad::TypeF32(fScope, "DayEndTime", 0.75f);
      sunriseStartTime = StdLoad::TypeF32(fScope, "SunriseStartTime", 0.2f);
      sunriseEndTime = StdLoad::TypeF32(fScope, "SunriseEndTime", 0.3f);
      sunsetStartTime = StdLoad::TypeF32(fScope, "SunsetStartTime", 0.7f);
      sunsetEndTime = StdLoad::TypeF32(fScope, "SunsetEndTime", 0.8f);

      // Clear the colors
      ClearColors();

      // Load the colors
      fScope = fScope->GetFunction("Colors", FALSE);
      if (fScope)
      {
        FScope *sScope;

        while ((sScope = fScope->NextFunction()) != NULL)
        {
          switch (sScope->NameCrc())
          {
            case 0x9F1D54D0: // "Add"
              AddColor(new LightColor(sScope));
              break;
          }
        }
      }
    }


    //
    // SaveInfo
    //
    // Save information
    //
    void SaveInfo(FScope *fScope)
    {
      ASSERT(initialized)

      // Save all of the settings
      StdSave::TypeF32(fScope, "CycleTime", cycleTime);
      StdSave::TypeF32(fScope, "CurrentTime", currentTime);
      StdSave::TypeF32(fScope, "LastTime", lastTime);
      StdSave::TypeF32(fScope, "StartTime", startTime);
      StdSave::TypeF32(fScope, "DayStartTime", dayStartTime);
      StdSave::TypeF32(fScope, "DayEndTime", dayEndTime);
      StdSave::TypeF32(fScope, "SunriseStartTime", sunriseStartTime);
      StdSave::TypeF32(fScope, "SunriseEndTime", sunriseEndTime);
      StdSave::TypeF32(fScope, "SunsetStartTime", sunsetStartTime);
      StdSave::TypeF32(fScope, "SunsetEndTime", sunsetEndTime);

      // Save the colors
      fScope = fScope->AddFunction("Colors");

      // Iterate through all of the colors
      LightColor *color = lightColors;
     
      if (color)
      {
        do
        {
          FScope *sScope = fScope->AddFunction("Add");
          color->Save(sScope);
          color = color->next;
        }
        while (color != lightColors);
      }
    }


    //
    // PostLoad
    //
    void PostLoad()
    {
      Vid::Light::FindCreateSun("sun");

      if (IsDay())
      {
        if (Vid::Light::AreOn())
        {
          Vid::Light::SwitchLights( FALSE);
        }
      }
      else if (!Vid::Light::AreOn())
      {
        Vid::Light::SwitchLights( TRUE);
      }
    }


    //
    // Get the colors
    //
    LightColor * GetColors()
    {
      return (lightColors);
    }


    //
    // Given a day fraction, fill in the color
    //
    void FillInColor(LightColor &lightColor)
    {
      // Find the light color that matches
      LightColor *color = lightColors;

      if (color)
      {
        // Is this fraction left or right of the root color ?
        if (lightColor.fraction < color->fraction)
        {
          // Go to the left until we find a color that is 
          // less than the fraction we asked for or we loop
          while (lightColor.fraction < color->fraction)
          {
            color = color->prev;

            // Did we go backwards ?
            if (color->fraction > color->next->fraction)
            {
              break;
            }

            // Did we go all the way around ?
            if (color == lightColors)
            {
              break;
            }
          }
        }
        else
        {
          // Go to the right until we find a color that is
          // greater than the fraction we asked for or we loop
          while (lightColor.fraction >= color->fraction)
          {
            color = color->next;

            // Did we go forwards ?
            if (color->fraction < color->prev->fraction)
            {
              break;
            }

            if (color == lightColors)
            {
              break;
            }
          }

          // Go back once
          color = color->prev;
        }

        // We want to average between this color and the next one
        LightColor *next = color->next;

        F32 fraction = next->fraction - color->fraction;

        if (fraction == 0.0f)
        {
          lightColor.ambient = color->ambient;
          lightColor.sun = color->sun;
          lightColor.fog = color->fog;
        }
        else 
        {
          if (fraction < 0.0f)
          {
            fraction += 1.0f;
          }

          MSWRITEV(11, (13, 0, "Fraction: %f", fraction));

          F32 portion = lightColor.fraction - color->fraction;

          if (portion < 0.0f)
          {
            portion += 1.0f;
          }

          fraction = portion / fraction;

          MSWRITEV(11, (14, 0, "Portion : %f", portion));
          MSWRITEV(11, (15, 0, "Fraction: %f", fraction));

          lightColor.ambient.r = U8((F32(color->ambient.r) * (1.0f - fraction)) + F32(next->ambient.r) * fraction);
          lightColor.ambient.g = U8((F32(color->ambient.g) * (1.0f - fraction)) + F32(next->ambient.g) * fraction);
          lightColor.ambient.b = U8((F32(color->ambient.b) * (1.0f - fraction)) + F32(next->ambient.b) * fraction);
          lightColor.ambient.a = 255;

          lightColor.sun.r = U8((F32(color->sun.r) * (1.0f - fraction)) + F32(next->sun.r) * fraction);
          lightColor.sun.g = U8((F32(color->sun.g) * (1.0f - fraction)) + F32(next->sun.g) * fraction);
          lightColor.sun.b = U8((F32(color->sun.b) * (1.0f - fraction)) + F32(next->sun.b) * fraction);
          lightColor.sun.a = 255;

          lightColor.fog.r = U8((F32(color->fog.r) * (1.0f - fraction)) + F32(next->fog.r) * fraction);
          lightColor.fog.g = U8((F32(color->fog.g) * (1.0f - fraction)) + F32(next->fog.g) * fraction);
          lightColor.fog.b = U8((F32(color->fog.b) * (1.0f - fraction)) + F32(next->fog.b) * fraction);
          lightColor.fog.a = 255;
        }
      }
      else
      {
        // No colors ... put in a default
        lightColor = DefaultColor();
      }
    }


    //
    // AddColor
    //
    // Add a new color to the colors
    //
    void AddColor(LightColor *lightColor)
    {
      // If there's no colors in there at all this this is the only color
      if (!lightColors)
      {
        lightColors = lightColor;
        lightColor->next = lightColor;
        lightColor->prev = lightColor;
      }
      else
      {
        // Find the light color that matches
        LightColor *color = lightColors;

        // Is this fraction left or right of the root color ?
        if (lightColor->fraction < color->fraction)
        {
          // Go to the left until we find a color that is 
          // less than the fraction we asked for or we loop
          while (lightColor->fraction < color->fraction)
          {
            color = color->prev;

            // Did we go backwards ?
            if (color->fraction > color->next->fraction)
            {
              break;
            }

            // Did we go all the way around ?
            if (color == lightColors)
            {
              break;
            }
          }
        }
        else
        {
          // Go to the right until we find a color that is
          // greater than the fraction we asked for or we loop
          while (lightColor->fraction >= color->fraction)
          {
            color = color->next;

            // Did we go forwards ?
            if (color->fraction < color->prev->fraction)
            {
              break;
            }

            if (color == lightColors)
            {
              break;
            }
          }

          // Go back once
          color = color->prev;
        }

        // Color is now pointing at the color which we want to go after

        lightColor->prev = color;
        lightColor->next = color->next;

        color->next = lightColor;
        lightColor->next->prev = lightColor;
      }
    }


    //
    // RemoveColor
    //
    // Remove a color from the colors
    //
    void RemoveColor(LightColor *color)
    {
      // Is this color the only color ?
      if (color->next == color)
      {
        lightColors = NULL;
      }
      else
      {
        // Correct the links on either side
        color->prev->next = color->next;
        color->next->prev = color->prev;
        
        // If this was the root color set it to the next one
        if (color == lightColors)
        {
          lightColors = color->next;
        }

        // Invalidate the links in this color
        color->next = NULL;
        color->prev = NULL;
      }
    }


    //
    // ClearColors
    //
    void ClearColors()
    {
      if (lightColors)
      {
        // Delete all of the light colors
        LightColor *ptr = lightColors;

        // Break the chain
        ptr->prev->next = NULL;

        // Delete it all
        while (ptr)
        {
          LightColor *c = ptr;
          ptr = ptr->next;
          delete c;
        }

        lightColors = NULL;
      }

    }


    //
    // DefaultColor
    //
    // Get a default light color
    //
    LightColor DefaultColor()
    {
      return (LightColor(0.5f, Color(0.1f, 0.1f, 0.1f), Color(1.0f, 1.0f, 1.0f), Color(0.6f, 0.6f, 0.6f)));
    }


    //
    // Get the var pointer to 
    //
    VarFloat & GetStartTimeVar()
    {
      return (startTime);
    }


    //
    // CmdHandler
    //
    // Handles var system events
    //
    static void CmdHandler(U32 pathCrc)
    {
      ASSERT(initialized);

      switch (pathCrc)
      {
        case 0xBC36B889: // "environment.light.cycletime"
          break;

        case 0x2098FB7B: // "environment.light.startTime"
          break;

        case 0x574394A8: // "environment.light.dayStartTime"
          if (dayStartTime > dayEndTime)
          {
            dayEndTime = dayStartTime;
          }
          break;

        case 0x0A751D2E: // "environment.light.dayEndTime"
          if (dayEndTime < dayStartTime)
          {
            dayStartTime = dayEndTime;
          }
          break;

        case 0x5CA74FA3: // "environment.light.sunriseStartTime"
          if (sunriseStartTime > sunriseEndTime)
          {
            sunriseEndTime = sunriseStartTime;
          }
          break;

        case 0xA7121B72: // "environment.light.sunriseEndTime"
          if (sunriseEndTime < sunriseStartTime)
          {
            sunriseStartTime = sunriseEndTime;
          }
          if (sunsetStartTime < sunriseEndTime)
          {
            sunsetStartTime = sunriseEndTime;
          }
          break;

        case 0xD96CC4F0: // "environment.light.sunsetStartTime"
          if (sunsetStartTime > sunsetEndTime)
          {
            sunsetEndTime = sunsetStartTime;
          }
          if (sunriseEndTime > sunsetStartTime)
          {
            sunriseEndTime = sunsetStartTime;
          }
          break;

        case 0xADB2452C: // "environment.light.sunsetEndTime" 
          if (sunsetEndTime < sunsetStartTime)
          {
            sunsetStartTime = sunsetEndTime;
          }
          break;

        case 0xD4645A58: // "environment.light.season"
          Vid::Light::SetSunAngle( *season);
          break;
      }
    }


    //
    // LightColor::LightColor
    //
    // Initializing Constructor
    //
    LightColor::LightColor(F32 fraction, const Color &ambient, const Color &sun, const Color &fog) 
    : fraction(fraction), 
      ambient(ambient), 
      sun(sun), 
      fog(fog)
    {

    }


    //
    // LightColor::LightColor
    //
    LightColor::LightColor(FScope *fScope)
    {
      fraction = StdLoad::TypeF32(fScope, "Fraction");
      StdLoad::TypeColor(fScope, "Ambient", ambient);
      StdLoad::TypeColor(fScope, "Sun", sun);
      StdLoad::TypeColor(fScope, "Fog", fog);
    }


    //
    // LightColor::Save
    //
    void LightColor::Save(FScope *fScope)
    {
      StdSave::TypeF32(fScope, "Fraction", fraction);
      StdSave::TypeColor(fScope, "Ambient", ambient);
      StdSave::TypeColor(fScope, "Sun", sun);
      StdSave::TypeColor(fScope, "Fog", fog);
    }
  }
}
