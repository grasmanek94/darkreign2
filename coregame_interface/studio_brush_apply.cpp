///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//

#include "studio_brush_apply.h"
#include "studio_history.h"
#include "input.h"
#include "common.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Apply - Base for all apply-type brushes
    //

    //
    // Constructor
    //
    Apply::Apply(const char *name) : Base(name)
    {
      captureThreshold[MOUSE_LEFT].Set(0, 0);
      captureApply = CM_OFF;
      nextApplyTime = 0;

      // Create interface vars
      varApplyRate = CreateInteger("applyRate", 20, 20, 1000);
    }


    //
    // Destructor
    //
    Apply::~Apply()
    {
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Apply::HasProperty(U32 property)
    {
      return (Base::HasProperty(property));
    }


    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Apply::NotifyVar(IFaceVar *var)
    {
      // Pass notification down
      Base::NotifyVar(var);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Apply::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0xFEA6C563: // "Brush::LeftMouseCaptureStart"
        {
          if (Common::Input::GetModifierKey(2))
          {
            // Resizing this brush
            captureResize = CM_ON;

            // Save the current mouse position
//            SaveMousePosition(e);

            // Now send a notify so brush can filter the resize out
            Notify("Brush::ResizeStart");
          }
          else
          {
            // Applying this brush
            captureApply = CM_ON;

            // Start of application, so clear continuous undo item
            History::Base::ClearContinuousItem();
          }

          // Block this event
          return;
        }

        case 0xFA75C2F5: // "Input::MouseMove"
        {
          ASSERT(e);

          // Are we resizing this brush
          if (captureResize == CM_ON)
          {
            // Get mouse deltas
            S32 dx = Input::MouseDelta().x >> 3;
            S32 dy = Input::MouseDelta().y >> 3;

            // Should we only use the largest delta
            if (!Common::Input::GetModifierKey(3))
            {
              // Use the largest delta
              dx = dy = ((abs(dy) > abs(dx)) ? dy : dx);
            }

            // Now modify this brush
            ModifySize(dx, dy);

            // Block this event
            return;
          }

          // Pass event down
          break;
        }

        case 0x252BA28C: // "Brush::LeftMouseCaptureEnd"
        {
          if (captureResize == CM_ON)
          {
            RestoreMousePosition();
            captureResize = CM_OFF;
          }
          else
          {
            captureApply = CM_OFF;
          }
          
          // Block this event
          return;
        }

        case 0x6780062A: // "System::LostCapture"
        {
          if (captureResize == CM_ON)
          {
            RestoreMousePosition();
            captureResize = CM_OFF;
          }
          else
          {
            captureApply = CM_OFF;
          }

          // Always pass system events down
          break;
        }

        case 0x8C6EE5DF: // "System::PostEventPoll"
        {
          // Are we currently applying this brush
          if (captureApply == CM_ON && Clock::Time::Ms() >= nextApplyTime)
          {
            // Set next time this brush will be applied
            nextApplyTime = Clock::Time::Ms() + varApplyRate->GetIntegerValue();

            // Generate apply event
            Notify("Brush::Apply");
          }

          // Always pass system events down
          break;
        }

        case 0x4F1528D7: // "Command::Increment"
        {
          // Increase brush size
          ModifySize(1, 1);

          // Block this event
          return;
        }

        case 0xCE78B6A5: // "Command::Decrement"
        {
          // Decrease brush size
          ModifySize(-1, -1);

          // Block this event
          return;
        }
      }

      // Not blocked at this level
      Base::Notification(crc, e);
    }


    //
    // ModifySize
    //
    // Resize this brush using the given deltas
    //
    void Apply::ModifySize(S32, S32)
    {
    }
  }
}
