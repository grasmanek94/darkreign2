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

#include "vid_public.h"
#include "studio_brush_objects.h"
#include "studio_history_objects.h"
#include "studio_history_terrain.h"
#include "studio_attachlist.h"
#include "movement_pathfollow.h"
#include "footprint.h"
#include "mapobjctrl.h"
#include "unitobj.h"
#include "wallobj.h"
#include "input.h"
#include "gameobjctrl.h"
#include "worldctrl.h"
#include "team.h"
#include "common.h"
#include "terrain.h"
#include "main.h"
#include "iface.h"
#include "transportobj.h"


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
    // Class Objects - Brush for management of map objects
    //

    //
    // Constructor
    //
    Objects::Objects(const char *name) : Base(name), 
      moveFilter(0.6f, Matrix::I.posit),
      mode(OM_DEFAULT),
      objectDir(WorldCtrl::NORTH)
    {
      // Create interface vars
      varType = CreateString("type", "");
    }


    //
    // Destructor
    //
    Objects::~Objects()
    {
      // Clear movement list
      moveList.Clear();
    }


    //
    // HasProperty
    //
    // Does brush have the given property
    //
    Bool Objects::HasProperty(U32 property)
    {
      switch (property)
      {
        case 0x86405F56: // "DisplaySelected"
          return (TRUE);
      }

      return (Base::HasProperty(property));
    }

    
    //
    // NotifyVar
    //
    // Notification that the given interface var has changed
    //
    void Objects::NotifyVar(IFaceVar *var)
    {
      if (var == varType)
      {
        // Change the current object type
        objectType = GameObjCtrl::FindType<MapObjType>(var->GetStringValue());
      }

      // Pass notification down
      Base::NotifyVar(var);
    }


    //
    // Notification
    //
    // Called when a brush event is generated
    //
    void Objects::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0xD47619B1: // "Input::LeftMouseDown"
        {
          if 
          (
            // In default mode
            mode == OM_DEFAULT && 

            // Clicked on an object
            data.cInfo.mObject.mapObj && 

            // That is currently selected
            data.sList.Exists(data.cInfo.mObject.mapObj)
          )
          {
            // Clear any previously moved objects
            moveList.Clear();

            // Copy currently selected objects
            for (MapObjList::Iterator i(&data.sList); *i; i++)
            {
              CustomListNode *node = moveList.Append(**i);
              node->matrix = (**i)->WorldMatrix();            
              Matrix &m = node->matrix;

              // Change the y component to be the offset from the ground
              if (WorldCtrl::MetreOnMap(m.posit.x, m.posit.z))
              {
                m.posit.y = m.posit.y - TerrainData::FindFloorLower(m.posit.x, m.posit.z);
              }
              else
              {
                LOG_DIAG
                ((
                  "Object %s (%d) was off the map (%f,%f,%f)!!!!", 
                  (**i)->TypeName(), (**i)->Id(), m.posit.x, m.posit.y, m.posit.z
                ));
              }
            }

            // Save starting position
            moveStart = data.cInfo.mObject.mapObj->WorldMatrix().posit;

            // Clear the filters
            rotateSrc.ClearData();
            rotateDest.ClearData();
            moveFilter.SetMemory(Matrix::I.posit);
            moveDest.ClearData();

            // Start object movement
            ToggleMovement(TRUE);

            // Add history item
            History::Base::AddItem
            (
              new History::Objects(History::Objects::OP_MOVE, data.sList), TRUE
            );

            // Block this event
            return;
          }

          // Pass this event down
          break;
        }

        case 0xFA75C2F5: // "Input::MouseMove"
        {
          ASSERT(e);

          if (mode == OM_MOVE && data.cInfo.gameWindow)
          {
            // Work out rotations in radians
            F32 xRotate = Clamp<F32>(-PI, (F32)Input::MouseDelta().x * Main::ElapTimeFrame(), PI);
            F32 zRotate = Clamp<F32>(-PI, (F32)Input::MouseDelta().y * Main::ElapTimeFrame(), PI);
          
            // Are we rotating around x and z axis
            if (Common::Input::GetModifierKey(1))
            {
              rotateDest.Rotate(xRotate, Matrix::I.front);
              rotateDest.Rotate(zRotate, Matrix::I.right);
            }
            else

            // Are we rotating around y axis
            if (Common::Input::GetModifierKey(2))
            {
              rotateDest.Rotate(xRotate, Matrix::I.up);
            }
            else

            // Are we moving vertically
            if (Common::Input::GetModifierKey(3))
            {
              moveDest = moveFilter.Current();
              moveDest.y -= ((F32)Input::MouseDelta().y / 5.0f);
            }
            else
            {
              // Are we over the terrain
              if (data.cInfo.mTerrain.cell)
              {
                // Move the object in the xz-axis
                moveDest.x = data.cInfo.mTerrain.pos.x - moveStart.x;
                moveDest.z = data.cInfo.mTerrain.pos.z - moveStart.z;
              }
            }

            // Block this event
            return;
          }

          // Pass event down
          break;
        }

        case 0x90486A58: // "System::PreEventPoll"
        {
          if (mode == OM_MOVE)
          {
            rotateSrc = rotateSrc.Interpolate(rotateDest, 0.5f);
            RotateObjects(rotateSrc);

            moveFilter.Update(moveDest);
            MoveObjects(moveFilter.Current());
          }

          // Always pass system events down
          break;
        }

        case 0xC74F389F: // "Input::LeftMouseUp"
        {
          if (mode == OM_MOVE)
          {
            // Finish object movement
            ToggleMovement(FALSE);

            // Block this event
            return;
          }

          // Pass this event down
          break;
        }

        case 0x6780062A: // "System::LostCapture"
        {
          // Finish object movement
          ToggleMovement(FALSE);

          // Always pass system events down
          break;
        }

        case 0x27546BF9: // "Brush::LeftMouseClick"
        {
          ASSERT(e);
          ASSERT(data.cInfo.gameWindow);

          // Is there an object under mouse
          if (data.cInfo.mObject.mapObj)
          {
            // If some special key is down
            if (config.Alive() && Common::Input::GetModifierKey(1))
            {
              // Find the attachment control
              ICWindow *window = IFace::Find<ICWindow>("StudioTool::Attach", NULL, TRUE);
              AttachList *attachList = IFace::Find<AttachList>("AttachList", window, TRUE);
              IFace::Activate(window);

              // Get it to build the attachment list for this control
              attachList->BuildList(data.cInfo.mObject.mapObj);

              // Save the object which is under the cursor
              attachRoot = data.cInfo.mObject.mapObj;

              // Block this event
              return;
            }

            // Pass this event down
            break;
          }
          else
          {
            // Create a new object at the current location
            CreateNewObject();

            // Notify derived brushes of object creation
            Notify(0x9031A85E); // "Brush::Objects::ObjectCreated"
          }

          // Block this event
          return;
        }

        case 0xAC279542: // "Command::Link"
        {
          // Get the unit to link to
          UnitObj *dst = (data.cInfo.gameWindow && data.cInfo.mObject.unitObj) ? data.cInfo.mObject.unitObj : NULL;

          // Is the destination a wall
          WallObj *dstWall = dst ? Promote::Object<WallObjType, WallObj>(dst) : NULL;

          // Is the destination a telepad
          TransportObj *dstTransport = dst ? Promote::Object<TransportObjType, TransportObj>(dst) : NULL;

          for (MapObjList::Iterator i(&data.sList); *i; i++)
          {
            if ((*i)->Alive())
            {
              UnitObj *src = Promote::Object<UnitObjType, UnitObj>(**i);

              if (src)
              {
                WallObj *srcWall = Promote::Object<WallObjType, WallObj>(src);
                TransportObj *srcTransport = Promote::Object<TransportObjType, TransportObj>(src);

                if (dstWall && srcWall)
                {
                  WallObj::TestResult r = srcWall->ToggleLink(dstWall, FALSE, FALSE);

                  if (r == WallObj::TR_SUCCESS)
                  {
                    CON_MSG(("Success"));
                  }
                  else
                  {
                    CON_ERR(("Failed: [%s]", WallObj::ExplainResult(r)));
                  }
                }
                else if (srcTransport)
                {
                  srcTransport->SetTelepadLink(dstTransport);
                  if (dstTransport)
                  {
                    CON_MSG(("Telepad link %d[%s]->%d[%s]", src->Id(), src->TypeName(), dst->Id(), dst->TypeName()))
                  }
                  else
                  {
                    CON_MSG(("Cleard Telepad link %d[%s]", src->Id(), src->TypeName()))
                  }
                }
                else
                {
                  src->SetLinkedUnit(dst);
                  if (dst)
                  {
                    CON_MSG(("Linked %d[%s]->%d[%s]", src->Id(), src->TypeName(), dst->Id(), dst->TypeName()));
                  }
                  else
                  {
                    CON_MSG(("Cleared link for %d[%s]", src->Id(), src->TypeName()));
                  }
                }
              }
            }
          }
          break;
        }

        case 0xA214C1AE: // "Command::Attach"
        {
          // Find the attachment control
          ICWindow *window = IFace::Find<ICWindow>("StudioTool::Attach", NULL, TRUE);
          AttachList *attachList = IFace::Find<AttachList>("AttachList", window, TRUE);

          // Get the var which has the current attachment
          VarSys::VarItem *var = VarSys::FindVarItem(attachList->DynVarName("attachment"));

          // Craete the new object using the name in the attachment
          CreateNewObject(var->Str());
          return;
        }

        case 0x56CE53AB: // "Command::Upgrade"
        {
          for (MapObjList::Iterator i(&data.sList); *i; i++)
          {
            if ((*i)->Alive())
            {
              UnitObj *u = Promote::Object<UnitObjType, UnitObj>(**i);

              if (u && u->GetNextUpgrade())
              {
                // Create the upgrade
                MapObj *obj = u->UpgradeNow(TRUE);

                // Add history item
                if (obj)
                {
                  History::Base::AddItem(new History::Objects(History::Objects::OP_CREATE, obj));
                }
              }
            }
          }
          return;
        }

        case 0x631BE516: // "Command::NextDir"
          objectDir = WorldCtrl::SlideCompassDir(objectDir);
          CON_MSG(("Direction now %s", WorldCtrl::GetCompassDirName(objectDir)));
          break;

        case 0xB123296F: // "Command::PrevDir"
          objectDir = WorldCtrl::SlideCompassDir(objectDir, FALSE);
          CON_MSG(("Direction now %s", WorldCtrl::GetCompassDirName(objectDir)));
          break;

        case 0x14BFCAFA: // "Command::Up"
          SnapObjects(data.sList, data.cInfo.mObject.mapObj ? &data.cInfo.mObject.mapObj->WorldMatrix() : NULL);
          return;

        case 0x0EDB94AC: // "Command::Down"
          SnapObjects(data.sList);
          return;

        case 0x5B9666F9: // "Command::Left"
          SnapObjects(data.sList, NULL, SNAP_IDENTITY);
          return;

        case 0xACD276E5: // "Command::Right"
          SnapObjects(data.sList, data.cInfo.mObject.mapObj ? &data.cInfo.mObject.mapObj->WorldMatrix() : NULL, SNAP_SET_UP);
          return;

        case 0xED1538BC: // "Command::UpMod1"
          CheckZipping(data.sList);
          return;

        case 0x49354BEE: // "Command::DownMod1"
          ToggleZipping(data.sList, TRUE);
          return;

        case 0xE0561E65: // "Command::UpMod2"
        {
          // Unzip all objects
          ToggleZipping(data.sList, FALSE);

          // Add history item
          History::Base::AddItem(new History::Objects(History::Objects::OP_MOVE, data.sList), TRUE);

          // Align to camera matrix
          for (MapObjList::Iterator i(&data.sList); *i; i++)
          {
            (**i)->SetSimCurrent(Vid::CurCamera().WorldMatrix());
          }
          return;
        }
      
        case 0x008CF4B8: // "Command::Delete"
        {
          // Add history item
          History::Base::AddItem(new History::Objects(History::Objects::OP_DELETE, data.sList));

          // Mark each selected object for deletion
          for (MapObjList::Iterator i(&data.sList); *i; i++)
          {
            // List has been validated at this point
            GameObjCtrl::MarkForDeletion(**i);
          }

          // May as well clear the list while we're here
          data.sList.Clear();
          
          // Block this event
          return;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Get the object under the cursor
          MapObj * obj = ObjectUnderCursor();

          if (mode == OM_DEFAULT && obj)
          {
            // Select the object type
            varType->SetStringValue(obj->TypeName());
  
            // Get the closest object direction
            WorldCtrl::CompassDir d = WorldCtrl::GetCompassDirection
            (
              Vector(obj->WorldMatrix().front.x, 0.0F, obj->WorldMatrix().front.z)
            );

            // Has it changed
            if (d != objectDir)
            {
              objectDir = d;

              CON_MSG(("Direction now %s", WorldCtrl::GetCompassDirName(objectDir)));
            }
          }

          // Block this event
          return;
        }
      }

      // Not blocked at this level
      Base::Notification(crc, e);
    }


    //
    // CreateNewObject
    //
    // Create a new object (TRUE if created)
    //
    Bool Objects::CreateNewObject()
    {
      // Do we have an object type and clicked on the terrain
      if (objectType.Alive() && data.cInfo.mTerrain.cell)
      {
        Matrix m;
        UnitObj *unitObj;

        // Setup matrix using position and direction
        WorldCtrl::SetupWorldMatrix(m, data.cInfo.mTerrain.pos, objectDir);
  
        // Create a new object
        MapObj *obj = MapObjCtrl::ObjectNewOnMap(objectType, m, 0, FALSE);

        // Set the object's team
        if ((unitObj = Promote::Object<UnitObjType, UnitObj>(obj)) != NULL)
        {
          unitObj->SetTeam(data.team);
        }

        // Add history item
        History::Base::AddItem(new History::Objects(History::Objects::OP_CREATE, obj));

        // Setup reaper
        lastCreatedObj = obj;

        return (TRUE);
      }

      return (FALSE);
    }


    //
    // CreateNewObject
    //
    // Create a new object (TRUE if created)
    //
    Bool Objects::CreateNewObject(const char *point)
    {
      // Do we have an object type and an attach root
      if (objectType.Alive() && attachRoot.Alive())
      {
        UnitObj *unitObj;

        // Find the hard point
        MeshObj *hp = attachRoot->Mesh().FamilyNode::FindMeshObj(point, TRUE);

        // Hard point must not have anything else attached
        if (hp && !hp->HasAttachment())
        {
          // Create the object on the map
          MapObj *obj = MapObjCtrl::ObjectNewOnMap(objectType, hp->WorldMatrix(), 0, FALSE);

          // Set the object's team
          if ((unitObj = Promote::Object<UnitObjType, UnitObj>(obj)) != NULL)
          {
            unitObj->SetTeam(data.team);
          }

          // Attach to the hardpoint
          obj->Attach(attachRoot, *hp);

          // Add history item
          History::Base::AddItem(new History::Objects(History::Objects::OP_CREATE, obj));
        }

        return (TRUE);
      }

      return (FALSE);
    }



    //
    // CheckZipping
    //
    // Check zipping positions
    //
    void Objects::CheckZipping(const MapObjList &list)
    {
      // Don't allow checking while moving
      if (mode != OM_MOVE)
      {
        // Unzip all objects
        ToggleZipping(list, FALSE);

        // Add history for object movement
        History::Base::AddItem(new History::Objects(History::Objects::OP_MOVE, data.sList));

        // Process the objects
        for (MapObjList::Iterator i(&list); *i; i++)
        {
          MapObj *obj = **i;

          // See if type has a setup footprint
          FootPrint::Type *footType = obj->MapType()->GetFootPrintType();

          if (footType)
          {
            // Get current position
            Matrix m = obj->WorldMatrix();

            // Prepare a placement class
            FootPrint::Placement place(footType);

            // Adjust the location to be cell aligned
            place.AdjustLocation(m);

            // Set the new position
            obj->SetSimCurrent(m);
          }
        }
      }
    }


    //
    // ToggleZipping
    //
    // Toggle object zipping
    //
    void Objects::ToggleZipping(const MapObjList &list, Bool toggle)
    {
      // Don't allow zipping while moving
      if (mode != OM_MOVE)
      {
        History::Terrain *terrainHistory = NULL;

        if (toggle)
        {
          // Add history for terrain modifications
          terrainHistory = new History::Terrain();
          History::Base::AddItem(terrainHistory);

          // Add history for object movement
          History::Base::AddItem
          (
            new History::Objects(History::Objects::OP_MOVE, data.sList), TRUE
          );
        }

        // Add history item for zipping
        History::Base::AddItem
        (
          new History::Objects(History::Objects::OP_ZIP, data.sList), toggle
        );

        // Toggle the zipping
        for (MapObjList::Iterator i(&list); *i; i++)
        {
          MapObj *obj = **i;

          if (toggle)
          {
            // See if type has a setup footprint
            FootPrint::Type *footType = obj->MapType()->GetFootPrintType();

            if (footType)
            {
              // Get current position
              Matrix m = obj->WorldMatrix();

              // Prepare a placement class
              FootPrint::Placement place(footType);

              // Adjust the location to be cell aligned
              place.AdjustLocation(m);

              // Is this object allowed to zip at this location
              if (FootPrint::Placement::Acceptable(place.Check(m)))
              {
                terrainHistory->Add(place.GetMin(), place.GetMax());
              }             
            }
          }

          obj->ToggleFootPrint(toggle);
        }
      }
    }


    //
    // ToggleClaiming
    //
    // Toggle object claiming
    //
    void Objects::ToggleClaiming(const MapObjList &list, Bool toggle)
    {
      for (MapObjList::Iterator i(&list); *i; i++)
      {
        UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(**i);

        if (unitObj && !unitObj->GetParent() && unitObj->MapType()->GetMovementModel().hasDriver)
        {
          if (toggle)
          {
            unitObj->GetDriver()->RemoveFromMapHook();
          }
          else
          {
            unitObj->GetDriver()->AddToMapHook();
          }
        }
      }
    }


    //
    // ToggleMovement
    //
    // Toggle object movement
    //
    void Objects::ToggleMovement(Bool toggle)
    {
      if (toggle)
      {
        ToggleZipping(data.sList, FALSE);
        mode = OM_MOVE;
      }
      else
      {
        mode = OM_DEFAULT;
      }

      ToggleClaiming(data.sList, toggle);
    }


    //
    // MoveObjects
    //
    // Move objects on the custom move list
    //
    void Objects::MoveObjects(const Vector &offset)
    {
      // Ensure list is purged of dead objects
      moveList.PurgeDead();

      // Move each object to current offset
      for (CustomList::Iterator i(&moveList); *i; i++)
      {
        // Get the object
        MapObj *obj = **i;

        // Do not change objects with a parent
        if (!obj->GetParent())
        {
          // Use the current rotation
          Matrix m = obj->WorldMatrix();

          // Use the original position
          m.posit = (*i)->matrix.posit + offset;

          // Clamp position onto map
          obj->CheckWorldPosition(m.posit);

          UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(obj);

          if (unitObj && unitObj->CanEverMove())
          {
            // Use aligned height if this has a driver
            unitObj->GetDriver()->AlignObject(m.front, m);
          }
          else
          {
            // Add in the floor (removed from original matrix)
            m.posit.y += TerrainData::FindFloor(m.posit.x, m.posit.z);
          }

          // Set the new position
          obj->SetSimCurrent(m);
        }
      }
    }


    //
    // RotateObjects
    //
    // Rotate objects on the custom move list
    //     
    void Objects::RotateObjects(const Quaternion &q)
    {
      // Ensure list is purged of dead objects
      moveList.PurgeDead();

      // Setup rotation matrix
      Matrix r;
      r.ClearData();
      r.Set(q);

      // Move each object to current offset
      for (CustomList::Iterator i(&moveList); *i; i++)
      {
        // Get the original matrix
        Matrix m = (*i)->matrix;

        // Use the current position
        Vector curPos = (**i)->WorldMatrix().posit;

        // Move matrix for rotation
        m.posit.ClearData();

        // Add in the rotation
        m = m * r;

        // Restore the position
        m.posit = curPos;

        // Get the object
        MapObj *obj = **i;

        // Do not change objects with a parent
        if (!obj->GetParent())
        {
          // Set the new position
          (**i)->SetSimCurrent(m);
        }
      }
    }


    //
    // SnapObjects
    //
    // Change height of each object to terrain or object height
    //
    void Objects::SnapObjects(const MapObjList &list, const Matrix *matrix, U32 flags)
    {
      // Don't allow snapping while moving
      if (mode != OM_MOVE)
      {
        // Unzip all objects
        ToggleZipping(list, FALSE);

        // Add history item
        History::Base::AddItem(new History::Objects(History::Objects::OP_MOVE, list), TRUE);

        for (MapObjList::Iterator i(&list); *i; i++)
        {
          // Get the object
          MapObj *obj = **i;

          // Do not change objects with a parent
          if (!obj->GetParent())
          {        
            // Copy current matrix
            Matrix m = obj->WorldMatrix();

            // Get height and normal at this location
            Vector normal;
            F32 floor;

            floor = TerrainData::FindFloor(m.posit.x, m.posit.z, &normal);

            // Set orienation
            if (flags & SNAP_IDENTITY)
            {
              // Align with identity matrix
              m.front = Matrix::I.front;
              m.right = Matrix::I.right;
              m.up    = Matrix::I.up;
            }
            else

            if (flags & SNAP_SET_UP)
            {
              UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(obj);

              if (unitObj && unitObj->MapType()->GetMovementModel().hasDriver)
              {
                // For units that can move, use the object's alignment routine
                unitObj->GetDriver()->AlignObject(m.front, m);
              }
              else
              {
                // Align to terrain normal or the specified object's up vector
                m.SetFromUp(matrix && (&obj->WorldMatrix() != matrix) ? matrix->up : normal);
              }
            }
            else
            {
              // Set the new height
              m.posit.y = matrix && (&obj->WorldMatrix() != matrix) ? matrix->posit.y : floor;
            }

            // Set the new position
            obj->SetSimCurrent(m);
          }
        }
      }
    }
  }
}
