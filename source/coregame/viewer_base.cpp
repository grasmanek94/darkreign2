///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Camera system
//
// 16-FEB-1999
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "viewer.h"
#include "viewer_private.h"
#include "common.h"
#include "team.h"
#include "sight.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Viewer
//
namespace Viewer
{

  //
  // Base::Base
  //
  Base::Base(const char *name) : ident(name)
  {
    m.ClearData();
    orientation.ClearData();
    velocity.ClearData();
  }


  //
  // Notification
  //
  Bool Base::Notify(ViewNotify notify, S32 p1, S32 p2)
  {
    switch (notify)
    {
      case VN_ACTIVATE:
      {
        dx = 0;
        dy = 0;
        dz = 0;
        mode = 0;
        // fall through
      }

      case VN_DEACTIVATE:
      case VN_LOSTCAPTURE:
      {
        // Reset flags
        rScroll = FALSE;
        mScroll = FALSE;

        // Clear last lock object
        lastLockObj = NULL;

        return (TRUE);
      }

      case VN_RBUTTON:
      {
        // Entering/leaving right scroll mode
        if (p1)
        {
          rScroll = TRUE;
        }
        else
        {
          rScroll = FALSE;
        }
        return (TRUE);
      }

      case VN_MBUTTON:
      {
        // Entering/leaving mid scroll mode
        if (p1)
        {
          mScroll = TRUE;
        }
        else
        {
          mScroll = FALSE;
        }
        return (TRUE);
      }

      case VN_MOUSEMOVE:
      {
        dx += p1;
        dy += p2;
        return (TRUE);
      }

      case VN_MOUSEAXIS:
      {
        dz += p1;
        return (TRUE);
      }
    }
    return (FALSE);
  }


  //
  // Set camera transform
  //
  void Base::Set(const Vector &pos, const Quaternion &quat, U32)
  {
    // Store orienation
    orientation = quat;

    // Update matrix and quaternion
    m.Set(pos, orientation);

    // Clear movement variables
    velocity.ClearData();
  }


  //
  // Generate quaternion and vector for looking at a position
  //
  Bool Base::GenerateLookAt(F32, F32, Vector &, Quaternion &, F32, F32)
  {
    return (FALSE);
  }


  //
  // Find the next map object
  //
  MapObj *Base::FindObject(MapObjPtr &object, FindObjectMode mode)
  {
    MapObjList &selectedList = Common::Cycle::SelectedList();
    MapObjListNode *node = NULL;

    selectedList.PurgeDead();

    // If current object is not on selected list, then use first mode
    if (object.Alive() && (mode == FO_NEXT || mode == FO_PREV))
    {
      node = selectedList.Find(object);
    }

    // Use node to iterate over list
    if (node)
    {
      if (mode == FO_NEXT)
      {
        NList<MapObjListNode>::Node *next = node->node.GetNext();

        // Advance to next node in selected list
        if (next)
        {
          ASSERT(next->GetData())
          return (*(next->GetData()));
        }
        else
        {
          // Otherwise use first
          return (selectedList.GetFirst());
        }
      }
      else

      if (mode == FO_PREV)
      {
        NList<MapObjListNode>::Node *prev = node->node.GetPrev();

        // Advance to previous node in selected list
        if (prev)
        {
          ASSERT(prev->GetData())

          return(*(prev->GetData()));
        }
        else
        {
          // Otherwise use last
          return(selectedList.GetLast());
        }
      }
    }

    // Otherwise choose first object
    MapObj *focusObj = Common::Cycle::FocusObject();

    // Use focus object if available
    if (focusObj && CanTrack(focusObj))
    {
      return (focusObj);
    }
    else

    // ... else the first selected object if it is available
    if (selectedList.GetCount() && CanTrack(selectedList.GetFirst()))
    {
      return (selectedList.GetFirst());
    }
    else

    // If last object is still alive then resort to it
    if ((mode == FO_LASTUNIT) && object.Alive())
    {
      return (object);
    }

    return (NULL);
  }


  //
  // Are we allowed to look at/track this object
  //
  Bool Base::CanTrack(MapObj *obj)
  {
    // Make sure we are allowed to track this unit
    if (Team::GetDisplayTeam())
    {
      // Is it visible to our display team
      if (!obj->GetVisible(Team::GetDisplayTeam()))
      {
        return (FALSE);
      }
    }
    return (TRUE);
  }


  //
  // Lock on previous target
  //
  void Base::LockPrevTarget()
  {
    lastLockObj = FindObject(lastLockObj, FO_PREV);

    if (lastLockObj.Alive())
    {
      LookAt(lastLockObj->Origin().x, lastLockObj->Origin().z);
    }
  }


  //
  // Lock on next target
  //
  void Base::LockNextTarget()
  {
    lastLockObj = FindObject(lastLockObj, FO_NEXT);

    if (lastLockObj.Alive())
    {
      LookAt(lastLockObj->Origin().x, lastLockObj->Origin().z);
    }
  }


  //
  // Build a quaternion from p1->p2
  //
  void Base::MakeQuaternion(const Vector &p1, const Vector &p2, Quaternion &q)
  {
    Matrix mat = Matrix::I;
    Vector v = p2 - p1;
    v.Normalize();
    mat.SetFromFront(v);
    q.Set(mat);
  }
}
