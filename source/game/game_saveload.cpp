///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Save Game Management
//
// 14-JAN-2000
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "game_saveload.h"
#include "savegame.h"
#include "ifvar.h"
#include "iface.h"
#include "iclistbox.h"
#include "user.h"
#include "console.h"
#include "babel.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Game
//
namespace Game
{
  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class SaveLoad
  //

  //
  // Constructor
  //
  SaveLoad::SaveLoad(IControl *parent) : ICWindow(parent)
  {
    slotName = new IFaceVar(this, CreateString("slotName", ""));
    description = new IFaceVar(this, CreateString("description", ""));
  }


  //
  // Destructor
  //
  SaveLoad::~SaveLoad()
  {
    delete slotName;
    delete description;
  }


  //
  // Update
  //
  // Update slot information
  //
  void SaveLoad::Update()
  {
    // Find the slot list
    if (ICListBox *list = IFace::Find<ICListBox>("List", this))
    {
      // Set the names of the slots
      for (NList<IControl>::Iterator i(&list->GetItems()); *i; i++)
      {
        SaveGame::Info info;

        // Get the information for this slot
        SaveGame::GetInfo((*i)->Name(), info);

        // Set the text string of the control
        if (info.valid)
        {
          (*i)->SetTextString(Utils::Ansi2Unicode(info.description), TRUE);
        }
        else
        {
          (*i)->SetTextString(TRANSLATE(("#shell.win.options.saveload.empty")), TRUE);
        }
      }      
    }
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 SaveLoad::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case SaveLoadMsg::Select:
            {
              IFace::PostEvent
              (
                this, NULL, IFace::NOTIFY, SaveGame::SlotUsed(slotName->GetStringValue()) ? 
                  SaveLoadNotify::SelectedUsed : SaveLoadNotify::SelectedFree
              );
              return (TRUE);
            }

            case SaveLoadMsg::SaveRequest:
            {
              if (const Missions::Mission *mission = Missions::GetActive())
              {
                // Generate the description
                char buf[256];
                Utils::Sprintf(buf, 256, "%s %s", mission->GetName().str, Clock::GetDateAndTime());
                description->SetStringValue(buf);

                // Post response so the progress dialog has a cycle to display
                IFace::PostEvent
                (
                  this, NULL, IFace::NOTIFY, SaveGame::SlotUsed(slotName->GetStringValue()) ? 
                    SaveLoadNotify::SaveSlotUsed : SaveLoadNotify::SaveSlotFree
                );
              }
              return (TRUE);
            }

            case SaveLoadMsg::SaveCycle:
            {
              // Post response so the progress dialog has a cycle to display
              IFace::PostEvent(this, NULL, IFace::NOTIFY, SaveLoadNotify::SaveProceed);
              return (TRUE);
            }

            case SaveLoadMsg::Save:
            {
              if (const Missions::Mission *mission = Missions::GetActive())
              {
                if (SaveGame::Save(slotName->GetStringValue(), description->GetStringValue()))
                {
                  IFace::PostEvent(this, NULL, IFace::NOTIFY, SaveLoadNotify::SelectedUsed);
                }
              
                // Reload the slot information
                Update();

                // Notify the control that the save is done
                SendNotify(this, SaveLoadNotify::SaveEnd);
              }

              return (TRUE);
            }

            case SaveLoadMsg::LoadRequest:
            {
              // Is there an active non-system mission
              if (Missions::GetActive() && !Missions::GetActive()->IsSystem())
              {
                SendNotify(this, SaveLoadNotify::LoadConfirm);
              }
              else
              {
                SendNotify(this, SaveLoadNotify::LoadProceed);
              }

              return (TRUE);
            }

            case SaveLoadMsg::Load:
            {
              SaveGame::Load(slotName->GetStringValue());
              return (TRUE);
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
  // Activate this control
  //
  Bool SaveLoad::Activate()
  {
    if (ICWindow::Activate())
    {
      // Activate the vars
      slotName->Activate();

      // Load the slot information
      Update();

      // Is save game available
      if (Missions::GetActive() && !Missions::GetActive()->IsSystem())
      {
        SendNotify(this, SaveLoadNotify::SaveAvailable);
      }

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool SaveLoad::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      slotName->Deactivate();
      return (TRUE);
    }

    return (FALSE);
  }
}
