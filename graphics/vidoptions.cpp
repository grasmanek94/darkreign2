///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vidoptions.cpp
//
// 01-APR-1998
//

#include "vid_private.h"
#include "vid_cmd.h"
#include "iface.h"
#include "iface_types.h"
#include "iface_priv.h"
#include "iface_messagebox.h"
#include "icwindow.h"
#include "iclistbox.h"
#include "fontsys.h"
#include "input.h"
#include "iface_priv.h"
//-----------------------------------------------------------------------------

#define TESTTIME    CLOCKS_PER_SEC * 3
//-----------------------------------------------------------------------------

namespace Vid
{

  //
  // Static data
  //
  NList<Options> Options::allWindows(&Options::node);


  //
  // Constructor
  //
  Options::Options(IControl *parent) : ICWindow(parent)
  {
    // Create vars
    VarSys::CreateString(DynVarName("cardName"), "", VarSys::DEFAULT, &drvName);
    VarSys::CreateFloat(DynVarName("vidMem"), 0.0F, VarSys::DEFAULT, &vidMem);

    allWindows.Append(this);
  }
  //-----------------------------------------------------------------------------

  //
  // Destructor
  //
  Options::~Options()
  {
    allWindows.Unlink(this);
  }
  //-----------------------------------------------------------------------------

  void Options::OnModeChange()
  {
    DriverDD * ddd = &Vid::ddDrivers[Vid::curDD];

    for (NList<Options>::Iterator i(&allWindows); *i; i++)
    {
      Options * ctrl = *i;
      ICListBox * driverBox = (ICListBox *) ctrl->Find(0x8A931010); // "DriverList"
      if (driverBox && ctrl->IsActive())
      {
        S32 sel = driverBox->GetSelectedIndex() - 1;
        if (sel < 0)
        {
          sel = 0;
        }
        ddd = &Vid::ddDrivers[sel];
      }

      // Setup card name and memory
      ctrl->drvName = ddd->ident.szDriver;
      ctrl->vidMem  = F32(ddd->desc.dwVidMemTotal) / (1024.0F * 1024.0F);

      ctrl->SendNotify(ctrl, CurMode().tripleBuf ? 0xF13BA087 : 0x46A402AB); // "Vid::Option::NoTripleBuf"
      ctrl->SendNotify(ctrl, Var::Dialog::mipmap ? 0x0D95AA79 : 0xBA0A0855); // "Vid::Option::Trilinear" : "Vid::Option::NoTrilinear"

      ctrl->SendNotify(ctrl, ddd->tex32     ? 0xF765541C : 0xCB1516E9); // "Vid::Option::32bit" : "Vid::Option::No32bit"
      ctrl->SendNotify(ctrl, ddd->gamma && Vid::isStatus.fullScreen && CurDD().gamma ? 0x01C7674A : 0x3DB725BF); // "Vid::Option::Gamma" : "Vid::Option::NoGamma"

      ctrl->SendNotify(ctrl, ddd->texMulti  ? 0xF7C32628 : 0x614B22FC); // "Vid::Option::Multitex" : "Vid::Option::NoMultitex"
//      ctrl->SendNotify(ctrl, ddd->hardTL    ? 0xE96DFBBC : 0x7811F848); // "Vid::Option::NoHardTL"
      ctrl->SendNotify(ctrl, ddd->antiAlias ? 0x551E05F5 : 0xE281A7D9); // "Vid::Option::NoAntiAlias"
      ctrl->SendNotify(ctrl, ddd->windowed  ? 0x9B1A112D : 0x0D9215F9); // "Vid::Option::NoWindowed"

      ctrl->SendNotify(ctrl, 0x7811F848); // "Vid::Option::NoHardTL"
    }
  }
  //-----------------------------------------------------------------------------

  void Options::FillDrivers()
  {
    ICListBox * driverBox = (ICListBox *) Find(0x8A931010); // "DriverList"
    if (driverBox)
    {
      // Rebuild the list
      driverBox->DeleteAllItems();

      U32 i;
      for (i = 0; i < Vid::numDDs; i++)
      {
//        driverBox->AddTextItem(Vid::ddDrivers[i].name.str, NULL);
        driverBox->AddTextItem(Vid::ddDrivers[i].ident.szDescription, NULL);
      }
    }
  }
  //-----------------------------------------------------------------------------

  void Options::FillModes()
  {
    ICListBox * driverBox = (ICListBox *) Find(0x8A931010); // "DriverList"
    ICListBox * modeBox   = (ICListBox *) Find(0xE1E86F9F); // "ModeList"
    if (driverBox && modeBox)
    {
      // Rebuild the list
      modeBox->DeleteAllItems();

      S32 sel = driverBox->GetSelectedIndex() - 1;
      if (sel < 0)
      {
        sel = 0;
      }
      DriverDD & ddd = Vid::ddDrivers[sel];

      for (U32 i = 0; i < ddd.numModes; i++)
      {
        modeBox->AddTextItem(ddd.vidModes[i].name.str, NULL);
      }
      if (ddd.windowed)
      {
        modeBox->AddTextItem(ddd.vidModes[VIDMODEWINDOW].name.str, NULL);
      }
    }
    OnModeChange();
  }
  //-----------------------------------------------------------------------------

  U32 Options::HandleEvent(Event &e)
  {
    static U32 startMode, theMode, startDD, cycledDDs;
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
            case IControlNotify::Activating:
              Var::Dialog::inDialog = TRUE;
              break;
            case IControlNotify::Activated:
            {
              setmode = FALSE;
              for (U32 i = 0; i < 4; i++)
              {
                savePerfs[i] = Var::perfs[i];
              }
              saveMultiTex  = Vid::renderState.status.texMulti;
              saveMipmap    = Var::varMipmap;
              saveMipfilter = Var::varMipfilter;
              saveMovie     = Var::varMovie;
              saveMirror    = Var::varMirror;
              saveGamma     = Var::varGamma;
              saveWeather   = Var::varWeather;

              Command::SetupDialog();

              ICListBox * driverBox = (ICListBox *) Find(0x8A931010); // "DriverList"
              ICListBox * modeBox   = (ICListBox *) Find(0xE1E86F9F); // "ModeList"

              saveDriver = Vid::curDD + 1;
              saveMode   = Vid::curMode + 1;
              if (modeBox)
              {
                saveMode = Vid::curMode == VIDMODEWINDOW ? modeBox->ItemCount() : Vid::curMode + 1;
              }

              FillDrivers();
              if (driverBox)
              {
                driverBox->SetSelectedItem( saveDriver);
              }

              FillModes();
              if (modeBox)
              {
                modeBox->SetSelectedItem( saveMode);
              }
              break;
            }
            case IControlNotify::Deactivated:
            {
              Var::Dialog::inDialog = FALSE;
              if (!setmode)
              {
                for (U32 i = 0; i < 4; i++)
                {
                  Var::perfs[i] = savePerfs[i];
                }
                Vid::renderState.status.texMulti = saveMultiTex;
                Var::varMipmap    = saveMipmap;
                Var::varMipfilter = saveMipfilter;
                Var::varMovie     = saveMovie;
                Var::varMirror    = saveMirror;
                Var::varGamma     = saveGamma;
                Var::varWeather   = saveWeather;
              }
              break;
            }
            case 0xF2F6914B: // "FillDrivers"
            {
              FillDrivers();
              // Handled
              return (TRUE);
            }
            case 0xD6FA23BD: // "FillModes"
            {
              FillModes();
              // Handled
              return (TRUE);
            }
            
            case 0xC7C383A3: // "SetMode"
            {
              setmode = TRUE;

              ICListBox * driverBox = (ICListBox *) Find(Crc::CalcStr("DriverList"));
              ICListBox * modeBox = (ICListBox *) Find(Crc::CalcStr("ModeList"));

              Command::SetPerfs();

              if (!driverBox || !modeBox)
              {
                break;
              }

              S32 driverSel = driverBox->GetSelectedIndex();
              S32 modeSel = modeBox->GetSelectedIndex();

              Var::varMipmap    = Var::Dialog::mipmap;
              Var::varMipfilter = Var::Dialog::mipfilter;
              Var::varMirror    = Var::Dialog::mirror;
              Var::varWeather   = Var::Dialog::weather;
              Var::varGamma     = (S32) Var::Dialog::gamma;

              if (driverSel        == (S32) saveDriver
                && ((!*Var::Dialog::winMode && modeSel == (S32) saveMode)
                  ||( *Var::Dialog::winMode && *Var::Dialog::winWid == viewRect.Width() && *Var::Dialog::winHgt == viewRect.Height() && CurDD().curMode == VIDMODEWINDOW))
                && Var::Dialog::texButts.d     == Var::varTexNoSwap
                && S32(Var::Dialog::texReduce) == Var::varTexReduce
                && Var::Dialog::tex32     == Var::varTex32
                && Var::Dialog::tripleBuf == Var::varTripleBuf
                && Var::Dialog::movie     == Var::varMovie
                && Var::Dialog::multiTex  == Var::varMultiTex)
              {
                // nothing requiring mode change was altered
                //
                break;
              }

              Var::varTexNoSwap = Var::Dialog::texButts.d;
              Var::varTexReduce = Var::Dialog::texReduce;
              Var::varTripleBuf = Var::Dialog::tripleBuf;
              Var::varTex32     = Var::Dialog::tex32;
              Var::varMovie     = Var::Dialog::movie;
              Var::varMultiTex = Var::Dialog::multiTex;

              Settings::viewRect.SetSize( *Var::Dialog::winWid, *Var::Dialog::winHgt);

              U32 lastDD = Vid::curDD;
              Vid::curDD = driverSel > 0 ? driverSel - 1 : 0;
              if (lastDD != Vid::curDD) 
              {

                Vid::InitDD( TRUE);
              }

              if (Vid::CurDD().windowed && modeSel == (S32) modeBox->ItemCount())
              {
                Vid::SetModeForce( VIDMODEWINDOW);
              }
              else if (Vid::CurDD().windowed && *Var::Dialog::winMode)
              {
                Vid::SetMode( VIDMODEWINDOW, *Var::Dialog::winWid, *Var::Dialog::winHgt, TRUE);
              }
              else
              {
                Vid::SetModeForce( modeSel > 0 ? modeSel - 1 : 0);
              }

              driverBox->SetSelectedItem( driverSel);
              modeBox->SetSelectedItem( modeSel);
              break;
            }
            case 0xA3A50304: // "TestStart"

              if (!Input::KeyState(Input::CTRLDOWN) || !Input::KeyState(Input::SHIFTDOWN))
              {
                break;
              }

              startDD   = Vid::curDD;
              startMode = Vid::curMode;
              theMode   = Vid::curMode == VIDMODEWINDOW ? 0 : Vid::curMode + 1;
              timer = 0;
              cycledDDs = Vid::numDDs > 1 ? FALSE : TRUE;

              LOG_DIAG( ("") );
              LOG_DIAG( ("Vid::Options: starting mode test") );
              LOG_DIAG( ("") );

              // fall through
            case 0x338BCFAC: // "Test"
            {
              if (!IsActive())
              {
                break;
              }
              ICListBox * driverBox = (ICListBox *) Find(Crc::CalcStr("DriverList"));
              ICListBox * modeBox   = (ICListBox *) Find(Crc::CalcStr("ModeList"));

              ASSERT( driverBox && modeBox);

              if (timer > Clock::Time::Ms())
              {
                // time's not up
                IFace::PostEvent( this, NULL, IFace::NOTIFY, id);
                break;
              }
              if (theMode == startMode && Vid::curDD == startDD)
              {
                // done
                SetModeForce( startMode);

                driverBox->SetSelectedItem( startDD + 1);
                modeBox->SetSelectedItem( startMode == VIDMODEWINDOW ? modeBox->ItemCount() : startMode + 1);
                break;
              }

              if (theMode < Vid::CurDD().numModes && theMode != VIDMODEWINDOW)
              {
                // cycling modes in this driver

                driverBox->SetSelectedItem( Vid::curDD + 1);
                modeBox->SetSelectedItem( theMode == VIDMODEWINDOW ? modeBox->ItemCount() : theMode + 1);

                SetModeForce( theMode);

                // keep going
                timer = Clock::Time::Ms() + TESTTIME;
                IFace::PostEvent( this, NULL, IFace::NOTIFY, Crc::CalcStr("Test"));

                theMode++;
              }
              else if (theMode != VIDMODEWINDOW && startMode != VIDMODEWINDOW && Vid::CurDD().windowed)
              {
                // do windowed 

                theMode = VIDMODEWINDOW;

                driverBox->SetSelectedItem( Vid::curDD + 1);
                modeBox->SetSelectedItem( theMode == VIDMODEWINDOW ? modeBox->ItemCount() : theMode + 1);

                SetModeForce( theMode);

                // keep going
                timer = Clock::Time::Ms() + TESTTIME;
                IFace::PostEvent( this, NULL, IFace::NOTIFY, Crc::CalcStr("Test"));
              }
              else if (!cycledDDs)
              {
                // new dd driver

                Vid::curDD = Vid::curDD >= Vid::numDDs - 1 ? 0 : Vid::curDD + 1;
                Vid::InitDD( TRUE);
                FillModes();

                cycledDDs = Vid::curDD == startDD ? TRUE : FALSE;
                theMode   = cycledDDs && startMode == VIDMODEWINDOW ? VIDMODEWINDOW : 0;

                driverBox->SetSelectedItem( Vid::curDD + 1);
                modeBox->SetSelectedItem( theMode == VIDMODEWINDOW ? modeBox->ItemCount() : theMode + 1);

                SetModeForce( theMode);

                // keep going
                timer = Clock::Time::Ms() + TESTTIME;
                IFace::PostEvent( this, NULL, IFace::NOTIFY, Crc::CalcStr("Test"));

                theMode++;
              }
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