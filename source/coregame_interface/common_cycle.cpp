///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Client/Editor Common Systems
//
// 19-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common.h"
#include "varsys.h"
#include "console.h"
#include "mapobjlist.h"
#include "mesh.h"
#include "promote.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Common - Code used in both the game and the studio
//
namespace Common
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Cycle - Cycle based information for client and studio
  //
  namespace Cycle
  {
    // System initialized flag
    static Bool initialized = FALSE;
    
    // List of selected map objects
    static MapObjList selected;

    // The object under the mouse
    static MapObjPtr mouseOver;
    static UnitObjPtr mouseOverUnit;

    // Var pointers
    static VarInteger varTriCount;
    static VarString varTypeName;


    //
    // Start
    //
    // Call at the start of a cycle
    //
    void Start()
    {
      // Reset cycle data
      mouseOver = NULL;
      mouseOverUnit = NULL;
    }

    
    //
    // End
    //
    // Call at the end of a cycle
    //
    void End()
    {
      U32 triCount = 0;
      Bool usedFocus = FALSE;     

      // Should we use the focus object for stats
      if (!selected.GetCount() && FocusObject())
      {
        selected.Append(FocusObject());
        usedFocus = TRUE;
      }

      // Total up values for all selected objects
      for (MapObjList::Iterator i(&selected); *i; i++)
      {
        // Get this object
        MapObj *obj = **i;

        // Add the values
        triCount += obj->Mesh().faceCount;
      }

      // Clear list if used focus object
      if (usedFocus)
      {
        selected.Clear();
      }

      // Transfer totals to vars
      varTriCount = triCount;

      // Update other vars
      if (mouseOverUnit.Alive())
      {
        // Promote to a unit
        varTypeName = mouseOverUnit->GetUpgradedUnit().GetDescKey();
      }
      else
      {
        varTypeName = mouseOver.Alive() ? mouseOver->GetDescKey() : "";
      }
    }


    //
    // ClearSelected
    //
    // Clear the current selected object list
    //
    void ClearSelected()
    {
      selected.Clear();
    }


    //
    // AddSelected
    //
    // Add an object to the selected list
    //
    void AddSelected(MapObj *obj)
    {
      selected.Append(obj);
    }

    
    //
    // SetMouseOverObject
    //
    // Set the object the mouse is over
    //
    void SetMouseOverObject(MapObj *obj)
    {
      mouseOver = obj;
      mouseOverUnit = obj ? Promote::Object<UnitObjType, UnitObj>(obj) : NULL;
    }


    //
    // SelectedList
    //
    // Get the list of selected objects
    //
    MapObjList & SelectedList()
    {
      return (selected);
    }


    //
    // FocusObject
    //
    // Get the object in focus
    //
    MapObj * FocusObject()
    {
      ASSERT(initialized);
      
      if (mouseOver.Alive())
      {
        return (mouseOver);
      }
      
      return (selected.GetFirst());
    }


    //
    // CmdHandler
    //
    // Handles var system events
    //
    static void CmdHandler(U32 crc)
    {
      ASSERT(initialized);

      switch (crc)
      {
        case 0x25767DFC: // "common.cycle.listselected"
        {
          CON_DIAG(("%d selected objects", selected.GetCount()))

          for (MapObjList::Iterator i(&selected); *i; i++)
          {
            CON_DIAG(("- %-20s (%d)", (**i)->TypeName(), (**i)->Id()))
          }
          break;
        }
      }
    }


    //
    // Init
    //
    // Initialize this sub-system
    //
    void Init()
    {
      ASSERT(!initialized);

      // Register command handler
      VarSys::RegisterHandler("common.cycle", CmdHandler);

      // Cycle commands
      VarSys::CreateCmd("common.cycle.listselected");

      // Cycle vars
      VarSys::CreateInteger("common.cycle.tricount", 0, VarSys::DEFAULT, &varTriCount);
      VarSys::CreateString("common.cycle.typename", "", VarSys::DEFAULT, &varTypeName);

      initialized = TRUE;
    }


    //
    // Done
    //
    // Shutdown this sub-system
    //
    void Done()
    {
      if (initialized)
      {
        // Clear the selected object list
        selected.Clear();
        mouseOver = FALSE;

        // Delete the cycle scope
        VarSys::DeleteItem("common.cycle");

        initialized = FALSE;
      }
    }    


    //
    // InitSim
    //
    // Initialize this sub-system
    //
    void InitSim()
    {
    }


    //
    // DoneSim
    //
    // Shutdown this sub-system
    //
    void DoneSim()
    {
    }    
  }
}
