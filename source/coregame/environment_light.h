///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Environment Light
//
// 1-MAR-1999
//


#ifndef __ENVIRONMENT_LIGHT_H
#define __ENVIRONMENT_LIGHT_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "fscope.h"
#include "utiltypes.h"


///////////////////////////////////////////////////////////////////////////////
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
    // Struct LightColor
    //
    struct LightColor
    {
      // Fraction
      F32 fraction;

      // Ambient lighting
      Color ambient;

      // Sun lighting
      Color sun;

      // Fog
      Color fog;

      // Next in the list
      LightColor *next;

      // Prev in the list
      LightColor *prev;

      // Default Constructor
      LightColor() { }

      // Initializing Constructor
      LightColor(F32 fraction, const Color &ambient, const Color &sun, const Color &fog);

      // Loading Constructor
      LightColor(FScope *fScope);

      // Save
      void Save(FScope *fScope);

    };
    

    // Initialization and Shutdown
    void Init();
    void Done();

    // Process LightTime
    void Process();

    // Is it currently day time
    Bool IsDay();

    // Is it currently night time
    Bool IsNight();

    // What percentage day is it
    F32 GetPercentageDay();

    // Calculate the percentage day given the percentage day
    F32 CalcPercentageDay(F32 fraction);

    // Get the minimum and maximum percentage day for the supplied period
    void GetPercentageDay(F32 start, F32 end, F32 &min, F32 &max);

    // Is the sun up; Should we be drawing shadows ?
    Bool IsSunUp();

    // Get the azimuth of the sun
    F32 Azimuth();

    // Get the time of the day (in seconds since midnight)
    U32 GetTime();

    // Get the time of the day (as a fraction of the day)
    F32 GetCycleFraction();

    // Load information
    void LoadInfo(FScope *fScope);

    // Save information
    void SaveInfo(FScope *fScope);
    
    // PostLoad
    void PostLoad();

    // Get the colors
    LightColor * GetColors();

    // Given a day fraction, fill in the color
    void FillInColor(LightColor &lightColor);

    // Add a new color to the colors
    void AddColor(LightColor *color);

    // Remove a color from the colors
    void RemoveColor(LightColor *color);

    // Clear all of the colors
    void ClearColors();

    // Get a default light color
    LightColor DefaultColor();

    // Get the var pointer to 
    VarFloat & GetStartTimeVar();

  }

}


#endif