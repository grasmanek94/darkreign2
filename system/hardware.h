///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Hardware detection routines
//
// 06-DEC-1997
//


#ifndef __HARDWARE_H
#define __HARDWARE_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Hardware
//
namespace Hardware
{

  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Hardware
  //
  namespace CPU
  {

    // CPU Type
    enum Type
    {
      // Unknown or un-identifiable
      UNKNOWN = 0,
      i486,

      // Intel family
      Pentium,
      PentiumMMX,
      PentiumII,
      PentiumIII,
      PentiumPro,
      Celeron,
      Celeron_2,

      // AMD family
      AMD5x86,
      AMDK5,
      AMDK6,
      AMDK6_2,
      AMDK6_3,
      AMDK7,

      // Cyrix family
      Cyrix686,
      Cyrix686MX,
      CyrixMediaGX,
      CyrixGXm,

      // IDT winchip family
      IDTC6,
      IDTC6Plus,

      MAXTYPE,
    };

    // CPU Feature set
    enum Features
    {
      FPU = 0,
      TSC,
      MMX,
      AMD3D,
      KNI,
      FDIVBUG,
      PSN,

      MAXFEATURES,
    };

    // Get the CPU Type
    Type GetType();

    // Get description
    const char *GetDesc();

    // Get detailed description
    const char *GetDetailedDesc();

    // Get serial number
    const char *GetSerial();

    // Get the clock speed in MHz
    U32 GetSpeed();

    // Test for a CPU feature
    Bool HasFeature(Features f);

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Memory
  //
  namespace Memory
  {

    // Physical memory in bytes
    U32  GetPhysical();

    // Virtual memory in bytes
    U32  GetVirtual();

    // Description
    const char *GetDesc();

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace Device
  //
  namespace Device
  {

    // Get number
    U32 GetNum();

    // Enumerate devices
    const char *Enum(U32 number);

  }


  /////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace OS
  //
  namespace OS
  {

    // OS Type
    enum Type
    {
      Win32s,
      Win9x,
      WinNT,
      Unknown
    };

    // Description string
    const char  *GetDesc();

    // OS Type
    Type GetType();

    // Version numbers
    U32 GetVersionMajor();
    U32 GetVersionMinor();
    U32 GetBuild();

    // Name of this computer
    const char *GetComputer();

    // Logged in user
    const char *GetUser();

  }

}

#endif