///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid_cmd.h
//

#ifndef __VIDCMD_H
#define __VIDCMD_H

#include "icwindow.h"
//-----------------------------------------------------------------------------

namespace Vid
{
  namespace Command
  {
    void Init();
    void Done();

    void Reset();
    void Setup();   // post Vid::Init
    void SetupDialog();
    void SetPerfs();

    void OnModeChange();

    Bool CheckCustom();

    extern Color nightLightColor;
  };
  //-----------------------------------------------------------------------------

  class Options : public ICWindow
  {
    PROMOTE_LINK(Options, ICWindow, 0x9C586C0C) // "Vid::Options"

  public:

    // Name of the driver
    VarString drvName;

    // Video memory (MB)
    VarFloat vidMem;

    NList<Options>::Node node;

    static NList<Options> allWindows;

  protected:

    void FillDrivers();
    void FillModes();

    U32 saveDriver;
    U32 saveMode;

    Bool setmode;
    F32  savePerfs[4];

    U32 saveMultiTex;
    U32 saveMipmap;
    U32 saveMipfilter;
    U32 saveMovie;
    U32 saveMirror;
    U32 saveGamma;
    U32 saveWeather;

  public:

    // Constructor 
    Options(IControl *parent);
    ~Options();

    // Event handler
    U32 HandleEvent(Event &e);

    static void OnModeChange();
  };
  //-----------------------------------------------------------------------------

  class Graphics : public ICWindow
  {
    PROMOTE_LINK(Graphics, ICWindow, 0x1D786456); // "Vid::Graphics"

  public:

    // Constructor 
    Graphics(IControl *parent);
    ~Graphics();

    // Event handler
    U32 HandleEvent(Event &e);

    void _OnModeChange();

    static void OnModeChange();
  };
  //-----------------------------------------------------------------------------
};

#endif
