//////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound Configuration Control
//
// 27-MAY-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icsoundconfig.h"
#include "iface.h"
#include "icwindow.h"
#include "iface_types.h"
#include "iclistbox.h"
#include "ifvar.h"
#include "sound.h"
#include "random.h"


///////////////////////////////////////////////////////////////////////////////
//
// Class ICSoundConfig - Sound configuration control
//


//
// Constructor
//
ICSoundConfig::ICSoundConfig(IControl *parent) : ICWindow(parent)
{
  // The current provider name
  providerIndex = new IFaceVar(this, CreateInteger("providerIndex", 0));
  message2D = new IFaceVar(this, CreateString("message2D", ""));
  message3D = new IFaceVar(this, CreateString("message3D", ""));
  lastError = new IFaceVar(this, CreateString("lastError", ""));
  digitalVolume = new IFaceVar(this, CreateFloat("digitalVolume", 0.0F, 0.0F, 1.0F));
  reservedVolume = new IFaceVar(this, CreateFloat("reservedVolume", 0.0F, 0.0F, 1.0F));
  redbookVolume = new IFaceVar(this, CreateFloat("redbookVolume", 0.0F, 0.0F, 1.0F));
  redbookEnabled = new IFaceVar(this, CreateInteger("redbookEnabled", 0));
}


//
// Destructor
//
ICSoundConfig::~ICSoundConfig()
{
  delete providerIndex;
  delete message2D;
  delete message3D;
  delete lastError;
  delete digitalVolume;
  delete reservedVolume;
  delete redbookVolume;
  delete redbookEnabled;
}


//
// UpdateState
//
// Update state information
//
void ICSoundConfig::UpdateState()
{
  char buf[512];

  // Do we have 2D sound
  Utils::Sprintf(buf, 512, "2D  : %s", Sound::Digital::GetServiceName2D());
  message2D->SetStringValue(buf);

  Utils::Sprintf(buf, 512, "3D  : %s", Sound::Digital::GetServiceName3D());
  message3D->SetStringValue(buf);

  if (listBox.Alive())
  {
    if (Sound::Digital::Claimed3D())
    {
      listBox->SetSelectedItem(Sound::Digital::GetServiceIndex3D() + 1);
    }
    else
    {
      SendNotify(listBox, ICListBoxMsg::ClearSelection);
    }
  }
}


//
// Notify
//
// Notification that a local var has changed value
//
void ICSoundConfig::Notify(IFaceVar *var)
{
  if (var == digitalVolume)
  {
    Sound::Digital::SetVolume(digitalVolume->GetFloatValue());
  }
  else

  if (var == reservedVolume)
  {
    Sound::Digital::Reserved::SetVolume(reservedVolume->GetFloatValue());
  }
  else

  if (var == redbookVolume)
  {
    Sound::Redbook::SetVolume(redbookVolume->GetFloatValue());
  }
  else

  if (var == redbookEnabled)
  {
    // Are we turning audio on
    if (redbookEnabled->GetIntegerValue())
    {
      // Set the enabled status
      Sound::Redbook::SetEnabled(TRUE);

      // Ignore if already claimed
      if (!Sound::Redbook::Claimed())
      {
        // Attempt to claim
        if (Sound::Redbook::Claim())
        {
          // Grab the volume from the driver
          redbookVolume->SetFloatValue(Sound::Redbook::Volume());
          
          // Start a random track
          Sound::Redbook::Play(Random::nonSync.Integer(Sound::Redbook::TrackCount()));
        }
        else
        {
          // Automatically pop the button up
          redbookEnabled->SetIntegerValue(FALSE);
        }
      }
    }
    else
    {
      Sound::Redbook::SetEnabled(FALSE);
      Sound::Redbook::Release();
    }
  }
}


//
// HandleEvent
//
// Pass any events to the registered handler
//
U32 ICSoundConfig::HandleEvent(Event &e)
{
  if (e.type == IFace::EventID())
  {
    switch (e.subType)
    {
      case IFace::NOTIFY:
      {
        switch (e.iface.p1)
        {
          case 0x0F9DCBFF: // "SoundConfig::Message::Open"
          {
            if (providerIndex->GetIntegerValue() > 0)
            {
              // Set the new provider preference
              Sound::Digital::SetProviderPreference(providerIndex->GetIntegerValue() - 1);

              // Change the digital 3D provider
              if (Sound::Digital::Change3DProvider())
              {
                lastError->SetStringValue("Go big n spin!");
              }
              else
              {
                lastError->SetStringValue(Sound::LastError());
              }

              // Update control state
              UpdateState();
            }

            return (TRUE);
          }

          case 0x02D5D57F: // "SoundConfig::Message::Release"
          {
            // Release the digital 3D provider
            Sound::Digital::Change3DProvider(FALSE);
            
            // Clear last error
            lastError->SetStringValue("");

            // Update control state
            UpdateState();

            return (TRUE);
          }

          case IControlNotify::Activated:
          {
            // Update the current state
            UpdateState();
            break;           
          }
        }
      }
    }
  }

  return (ICWindow::HandleEvent(e));  
}


//
// Activate
//
// Control is being activated
//
Bool ICSoundConfig::Activate()
{
  if (ICWindow::Activate())
  {
    // Get the current volumes
    digitalVolume->SetFloatValue(Sound::Digital::Volume());
    reservedVolume->SetFloatValue(Sound::Digital::Reserved::Volume());
    redbookVolume->SetFloatValue(Sound::Redbook::Volume());
    redbookEnabled->SetIntegerValue(Sound::Redbook::GetEnabled());
    
    providerIndex->Activate();
    message2D->Activate();
    message3D->Activate();
    lastError->Activate();
    digitalVolume->Activate();
    reservedVolume->Activate();
    redbookVolume->Activate();
    redbookEnabled->Activate();

    // Find the provider list box
    listBox = IFace::Find<ICListBox>("Providers", this);

    if (listBox.Alive())
    {
      // Clear current items
      listBox->DeleteAllItems();

      Bool first = TRUE;
      const char *name;

      while (Sound::Digital::Next3DProvider(name, first))
      {
        listBox->AddTextItem(name, NULL);
      }
    }

    // Clear last error
    lastError->SetStringValue("");

    UpdateState();

    return (TRUE);
  }

  return (FALSE);
}
