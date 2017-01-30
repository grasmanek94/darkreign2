///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Facility Shortcut Control
//
// 17-SEP-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "client_facility.h"
#include "input.h"
#include "common.h"
#include "iface.h"
#include "iface_util.h"
#include "iface_priv.h"
#include "taskctrl.h"
#include "tasks_unitconstructor.h"
#include "team.h"
#include "client_private.h"
#include "viewer.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Client
//
namespace Client
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FacilityIcon - A single facility icon
  //

  //
  // Constructor
  //
  FacilityIcon::FacilityIcon(IControl *parent, UnitObj *facility) : 
    IControl(parent),
    facility(facility),
    textureOverlay(NULL),
    pointCount(0, 0),
    areaProgress(0, 0, 0, 0),
    areaHealth(0, 0, 0, 0),
    alphas(0.4F, 0.6F)
  {
    // Control needs to be polled every cycle
    SetPollInterval(-1);

    // Set the icon image
    FScope *fScope = facility->UnitType()->GetDisplayConfig();

    // Read image if its there
    if (fScope)
    {
      FScope *sScope = fScope->GetFunction("Image");

      if (sScope)
      {
        TextureInfo t;
        IFace::FScopeToTextureInfo(sScope, t);
        SetImage(&t);
      }
    }

    // Set the name using the type identifier
    SetName(facility->TypeName());
  }


  //
  // Destructor
  //
  FacilityIcon::~FacilityIcon()
  {
    if (textureOverlay)
    {
      delete textureOverlay;
    }
  }
 
  
  //
  // Setup
  //
  // Configure control from an FScope
  //
  void FacilityIcon::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x4CF287CE: // "TextureOverlay"
      {
        if (textureOverlay)
        {
          delete textureOverlay;
        }
        textureOverlay = new TextureInfo;
        IFace::FScopeToTextureInfo(fScope, *textureOverlay);
        break;
      }

      case 0x8494CA0F: // "PointCount"
        StdLoad::TypePoint(fScope, pointCount);
        break;

      case 0xC84F46D4: // "AreaProgress"
        StdLoad::TypeArea(fScope, areaProgress);
        break;

      case 0x76FD5325: // "AreaHealth"
        StdLoad::TypeArea(fScope, areaHealth);
        break;

      case 0x4C7418A3: // "BarAlphas"
        StdLoad::TypePoint(fScope, alphas);
        break;

      // Pass it to the previous level in the hierarchy
      default:
        IControl::Setup(fScope);
    }
  }


  //
  // Poll
  //
  // Cycle based processing
  //
  void FacilityIcon::Poll()
  {
    if (facility.Alive())
    {
      Task::RetrievedData data;

      // Get the progress
      if (facility->Retrieve(TaskRetrieve::Progress, data))
      {
        progress = data.f1;
        progressType = data.u1;
      }
      else
      {
        progress = 0.0F;
      }

      // Get the count
      count = (facility->Retrieve(TaskRetrieve::Count, data)) ? data.u1 : 0;
    }
  }


  //
  // DrawSelf
  //
  // Draw this custom control
  //
  void FacilityIcon::DrawSelf(PaintInfo &pi)
  {
    // Draw background
    DrawCtrlBackground(pi, GetTexture());

    if (facility.Alive())
    {
      // Is this a construction facility
      if (Tasks::UnitConstructor *task = TaskCtrl::PromoteIdle<Tasks::UnitConstructor>(facility))
      {
        // Is there an offmap object available
        if (OffMapObj *offMap = task->GetOffMapObject())
        {
          IFace::RenderRectangle
          (
            pi.client,
            Color::Std[Color::YELLOW],
            NULL,
            data.clientAlpha.GetValue() * IFace::data.alphaScale * 0.5F
          );
        }
      }
    }

    // Draw the overlay
    if (textureOverlay)
    {
      IFace::RenderRectangle
      (
        pi.client, IFace::data.cgTexture->bg[ColorIndex()], textureOverlay, pi.alphaScale
      );
    }

    if (facility.Alive())
    {
      // Display the health
      F32 pct;
      Color color;
      facility->GetHealthInfo(color, pct);
      IFace::RenderRectangle(areaHealth + pi.client.p0, color, NULL, IFace::data.alphaScale);
    
      // Display progress bar
      if (progress > 0.0F)
      {
        // Get the configured bar color
        color = HUD::GetColorEntry(progressType);

        ClipRect clip = areaProgress + pi.client.p0;
        IFace::RenderRectangle(clip, color, NULL, alphas.x * IFace::data.alphaScale);
        clip.p1.x = clip.p0.x + S32(progress * F32(clip.p1.x - clip.p0.x));
        IFace::RenderRectangle(clip, color, NULL, alphas.y * IFace::data.alphaScale);
      }

      // Display count
      if (pi.font && count)
      {
        IFace::RenderS32
        (
          count, pi.font, pi.colors->fg[ColorIndex()], 
          pointCount.x, pointCount.y, &pi.client, 
          IFace::data.alphaScale
        );
      }
    }
  }


  //
  // ProcessOperation
  //
  // Process the given operation
  //
  void FacilityIcon::ProcessOperation(U32 event)
  {
    switch (event)
    {
      case 0xCCEDD540: // "Primary"
      {
        // Jump to facility if already selected
        if (data.cInfo.oneUnit.GetPointer() == facility)
        {
          Viewer::GetCurrent()->LookAt(facility->Position().x, facility->Position().z);
        }
        else
        {
          // Just select the facility
          Events::SelectUnit(facility);
        }
        break;
      }
    }
  }


  //
  // HandleEvent
  //
  // Event handler
  //
  U32 FacilityIcon::HandleEvent(Event &e)
  {
    // Input events
    if (e.type == ::Input::EventID())
    {    
      // Process event 
      switch (e.subType)
      {
        case ::Input::MOUSEBUTTONDOWN:
        case ::Input::MOUSEBUTTONDBLCLKUP:
        {
          if (facility.Alive())
          {
            // Send events to parent
            if (e.input.code == ::Input::LeftButtonCode())
            {
              if (Common::Input::GetModifierKey(3))
              {
                ProcessOperation(0x7C13A54E); // "PrimaryShift"
              }
              else
              {
                ProcessOperation(0xCCEDD540); // "Primary"
              }
            }
            else 
          
            if (e.input.code == ::Input::RightButtonCode())
            {
              if (Common::Input::GetModifierKey(3))
              {
                ProcessOperation(0xBAAF3F34); // "SecondaryShift"
              }
              else
              {
                ProcessOperation(0xBD34689F); // "Secondary"
              }
            }
          }

          return (TRUE);
        }
      }
    }
    else

    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::DISPLAYTIP:
        {
          // Set the tool tip
          SetTipText(facility->GetUpgradedUnit().GetDesc(), FALSE);

          // Pass down to the icontrol display function
          break;
        }
      }
    }

    // Allow parent to process this event
    return IControl::HandleEvent(e);
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool FacilityIcon::Activate()
  {
    if (IControl::Activate())
    {
      Poll();
      return (TRUE);
    }

    return (FALSE);
  }


  //
  // GetFacility
  //
  // Returns the facilty for this icon, or NULL if none
  //
  UnitObj * FacilityIcon::GetFacility()
  {
    return (facility.Alive() ? facility.GetData() : NULL);
  }



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Facility - Facility shortcut control
  //

  //
  // Constructor
  //
  Facility::Facility(IControl *parent) : IconWindow(parent)
  {
    // Control needs to be polled every cycle
    SetPollInterval(-1);
  }


  //
  // Destructor
  //
  Facility::~Facility()
  {
  }


  //
  // Poll
  //
  // Cycle based processing
  //
  void Facility::Poll()
  {
    Bool modified = FALSE;

    // Check each icon
    for (List<IControl>::Iterator i(&icons); *i;)
    {
      // Promote to a facility icon
      FacilityIcon *icon = IFace::Promote<FacilityIcon>(i++);

      // Facility has died, or changed team
      if (icon && (!icon->GetFacility() || icon->GetFacility()->GetTeam() != Team::GetDisplayTeam()))
      {
        DeleteIcon(icon);
        modified = TRUE;
      }
    }

    // Were any icons deleted
    if (modified)
    {
      ArrangeIcons();
    }
  }


  //
  // GetIcon
  //
  // Get the icon for a particular object
  //
  FacilityIcon * Facility::GetIcon(UnitObj *facility)
  {
    ASSERT(facility)

    // Check each icon
    for (List<IControl>::Iterator i(&icons); *i; i++)
    {
      // Promote to a facility icon
      FacilityIcon *icon = IFace::Promote<FacilityIcon>(*i);

      // Is this the one we're after
      if (icon && icon->GetFacility() == facility)
      {
        return (icon);
      }     
    }   

    return (NULL);
  }


  //
  // Add
  //
  // Add a facility to the bar
  //
  void Facility::Add(UnitObj *facility)
  {
    // Ignore if already have this facility
    if (!GetIcon(facility))
    {
      //LOG_DIAG(("Add %s - %d", facility->TypeName(), facility->Id()));

      AddIcon(new FacilityIcon(this, facility));
      ArrangeIcons();
    }
  }
}
