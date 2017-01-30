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

#include "studio_brush_curve.h"
#include "studio_history_objects.h"
#include "mapobj.h"
#include "mapobjctrl.h"
#include "perfstats.h"
#include "main.h"
#include "resolver.h"
#include "render.h"
#include "common.h"
#include "viewer.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  /////////////////////////////////////////////////////////////////////////////
  //
  // Namespace Brush - Contains all available brushes
  //
  namespace Brush
  {

    // Camera name
    static const char *CAMERA_NAME = "Curve brush camera";

    // Object type names
    static U32 CAMERA_TYPE   = 0xFD47514E; // "studio.marker.camera"
    static U32 INTEREST_TYPE = 0x11BDF0B8; // "studio.marker.interest"


    //
    // Selection filter for marker objects
    //
    static MapObj * SelectionFilter(MapObj *obj)
    {
      return (Promote::Object<MarkerObjType, MarkerObj>(obj));
    }


    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Curve
    //

    //
    // Constructor
    //
    Curve::Curve(const char *name) 
    : Objects(name),
      prevLen(0.0F)
    {
      // Create interface vars
      varName = CreateString("name", "");
      varNameEdit = CreateString("nameedit", "");
      varLoop = CreateInteger("loop", FALSE);
      varTime = CreateFloat("time", 10.0F);
      varStrength = CreateFloat("strength", 100.0F);
      varStartTime = CreateFloat("starttime", 0.0F);
      varIndex = CreateInteger("index", 0);
      varFocus = CreateInteger("Focus", FALSE);
      varUseCamera = CreateInteger("usecamera", FALSE);
      varDrawLines = CreateInteger("drawlines", TRUE);
      varDrawNormals = CreateInteger("drawnormals", FALSE);
      varCurrTime = CreateFloat("currtime", 0.0F, 0.0F, 0.0F);
      varPlayMode = CreateInteger("playmode", FALSE);
      varFastMode = CreateInteger("fastmode", FALSE);

      // Create a camera
      Viewer::CreateCustomCamera(CAMERA_NAME, CameraSimulate, this);
    }


    //
    // Destructor
    //
    Curve::~Curve()
    {
      curveList.ReaperList<MarkerObj, SegmentListNode>::Clear();
      focusList.ReaperList<MarkerObj, SegmentListNode>::Clear();
      prevSlist.Clear();
    }


    //
    // Create a curve
    //
    void Curve::Create(const char *name)
    {
      if (Utils::Strlen(name) > 0)
      {
        if (BookmarkObj::CreateBookmark(name, BookmarkObj::CURVE))
        {
          if (config.Alive())
          {
            // Notify the config control that a bookmark was created
            config->SendNotify(config, Crc::CalcStr("Brush::Curve::Notify::Created"));
          }
        }
      }
    }


    //
    // Delete a curve
    //
    void Curve::Delete()
    {
      BookmarkObj *obj = BookmarkObj::FindBookmark(varName->GetStringValue());

      if (obj)
      {
        GameObjCtrl::MarkForDeletion(obj);

        if (config.Alive())
        {
          // Notify the config control that a region was deleted
          config->SendNotify(config, Crc::CalcStr("Brush::Curve::Notify::Deleted"));
        }
      }
    }


    //
    // Find an object's node in the list
    //
    Curve::SegmentListNode* Curve::FindNode(MarkerObj *obj, SegmentList **foundList)
    {
      for (U32 count = 0; count < 2; count++)
      {
        SegmentList *list = count ? &focusList : &curveList;

        for (SegmentList::Iterator i(list); *i; i++)
        {
          MapObj *obj2 = **i;

          if (obj2 == obj)
          {
            if (foundList)
            {
              *foundList = list;
            }
            return (*i);
          }
        }
      }
      return (NULL);
    }


    //
    // Get first SegmentListNode from a MapObjList
    //
    Curve::SegmentListNode* Curve::GetFirstNode(MapObjList &list, SegmentList **foundList)
    {
      for (MapObjList::Iterator i(&list); *i; i++)
      {
        MarkerObj *obj = Promote::Object<MarkerObjType, MarkerObj>(**i);
        if (obj)
        {
          SegmentListNode *node = FindNode(obj, foundList);
          if (node)
          {
            return (node);
          }
        }
      }
      return (NULL);
    }


    //
    // Add a node into the curve
    //
    void Curve::InsertNode(SegmentList &listIn, MarkerObj *node, MarkerObj *after, F32 strength, F32 time, Bool align)
    {
      MarkerObj *alignTo = NULL;

      // Find insertion point
      SegmentListNode *insertPoint = NULL;
      SegmentList *list = NULL;

      if (after)
      {
        insertPoint = FindNode(after, &list);
      }

      // Node not found in a list, use passed in list
      if (!list)
      {
        list = &listIn;
      }

      // Create new node
      SegmentListNode *newNode = new SegmentListNode;

      newNode->Setup(node);
      newNode->strength = strength;
      newNode->time = time;

      // Insert it
      if (insertPoint)
      {
        alignTo = *insertPoint;
        list->InsertAfter(&insertPoint->node, newNode);
      }
      else
      {
        if (list->GetCount())
        {
          alignTo = *(list->GetTail());
        }
        list->NList<SegmentListNode>::Append(newNode);
      }

      // Line up with last
      if (align && alignTo)
      {
        Matrix m = node->WorldMatrix();
        m.front = node->Position() - alignTo->Position();

        if (m.front.Magnitude() > 1e-4)
        {
          m.front.Normalize();
          node->SetSimCurrent( Quaternion( 2.0f * PI, m.front));
        }
      }
    }


    //
    // Iterate to a point on the curve
    //
    Bool Curve::Step(F32 &time, Matrix &matrix)
    {
      if (bookmarkPtr.Alive())
      {
        return (bookmarkPtr->Step(time, matrix));
      }
      else
      {
        return (FALSE);
      }
    }


    //
    // Render a segment list
    //
    void Curve::RenderSegmentList(SegmentList &list, Color c)
    {
      list.PurgeDead();
      list.UpdateTime(varLoop->GetIntegerValue());

      if (list.GetCount() > 1)
      {
        for (SegmentList::Iterator i(&list); *i; i++)
        {
          SegmentListNode *curr = *i;
          SegmentListNode *next = NULL;

          if (i.IsTail())
          {
            if (varLoop->GetIntegerValue())
            {
              // Join up to the first
              next = list.GetHead();
            }
          }
          else
          {
            // Grab the next
            next = curr->node.GetNext()->GetData();
          }

          // Rebuild the segment
          if (next)
          {
            Vector e0 = (*curr)->WorldMatrix().posit;
            Vector t0 = (*curr)->WorldMatrix().front * curr->strength;
            Vector e1 = (*next)->WorldMatrix().posit;
            Vector t1 = (*next)->WorldMatrix().front * next->strength;
            curr->segment.SetupHermite(e0, t0, e1, t1, curr->time);

            Render::Spline(curr->segment, c, varDrawNormals->GetIntegerValue());
          }
        }
      }
    }


    //
    // Upload
    //
    // Upload the curve to the bookmark object
    //
    void Curve::Upload(BookmarkObj &bookmark)
    {
      // Clear both items
      bookmark.DisposeCurves();

      // Transfer each node to a curve segment source list
      bookmark.InitCurveMemory();

      for (SegmentList::Iterator i(&curveList); *i; i++)
      {
        SegmentListNode *curr = *i;
        bookmark.GetCurveSrc().AddNode((*curr)->WorldMatrix(), curr->strength, curr->time);
      }
      bookmark.GetCurve().Build(bookmark.GetCurveSrc(), varLoop->GetIntegerValue());

      // Upload optional 
      if (focusList.GetCount())
      {
        // Transfer each node to a curve segment source list
        bookmark.InitFocusMemory();

        for (SegmentList::Iterator i(&focusList); *i; i++)
        {
          SegmentListNode *curr = *i;
          bookmark.GetFocusSrc().AddNode((*curr)->WorldMatrix(), curr->strength, curr->time);
        }
        bookmark.GetFocus().Build(bookmark.GetFocusSrc(), varLoop->GetIntegerValue());
      }
    }


    //
    // Download the curve from a bookmark object
    //
    void Curve::Download(BookmarkObj &bookmark)
    {
      if (cameraObjType.Alive() && interestObjType.Alive())
      {
        MapObjTypePtr prevType = objectType;

        // Delete all objects
        curveList.Clear();
        focusList.Clear();
        prevLen = 0.0F;

        // Setup loop flag
        varLoop->SetIntegerValue(bookmark.GetCurve().loop);

        // Download movement curve
        objectType = cameraObjType;
        Download(bookmark.GetCurveSrc(), curveList);

        if (bookmark.HasFocus())
        {
          objectType = interestObjType;        
          Download(bookmark.GetFocusSrc(), focusList);
        }

        // Restore object type
        objectType = prevType;
      }
    }


    //
    // Download helper
    //
    void Curve::Download(BookmarkObj::CurveSrc &src, SegmentList &list)
    {
      if (objectType.Alive())
      {
        for (NList<BookmarkObj::CurveSrcSegment>::Iterator i(&src.segmentList); *i; i++)
        {
          MarkerObj *newNode = Promote::Object<MarkerObjType, MarkerObj>(MapObjCtrl::ObjectNewOnMap(objectType, (*i)->m, 0, FALSE));
          if (newNode)
          {
            InsertNode(list, newNode, NULL, (*i)->strength, (*i)->time, FALSE);
          }
        }
        list.UpdateTime(varLoop->GetIntegerValue());
      }
      else
      {
        LOG_ERR(("FUBAR!"))
      }
    }


    //
    // Notification that the given interface var has changed
    //
    void Curve::NotifyVar(IFaceVar *var)
    {
      if (var == varUseCamera)
      {
        // Create or destroy the camera
        if (varUseCamera->GetIntegerValue())
        {
          Viewer::SetCurrent(CAMERA_NAME);
          varCurrTime->SetFloatValue(0.0F);
          varFastMode->SetIntegerValue(FALSE);
        }
        else
        {
          Viewer::SetCurrent("Default");
          varPlayMode->SetIntegerValue(FALSE);
          varFastMode->SetIntegerValue(FALSE);
        }
      }
      else 

      if (var == varFocus)
      {
        if (interestObjType.Alive() && cameraObjType.Alive())
        {
          // Setup object type
          objectType = varFocus->GetIntegerValue() ? interestObjType : cameraObjType;
        }
      }
      else

      if (var == varStrength || var == varTime)
      {
        for (U32 count = 0; count < 2; count++)
        {
          SegmentList *list = count ? &focusList : &curveList;

          // Apply time value to all selected nodes
          for (MapObjList::Iterator i(&data.sList); *i; i++)
          {
            MapObj *obj1 = **i;

            for (SegmentList::Iterator j(list); *j; j++)
            {
              MapObj *obj2 = **j;

              if (obj1 == obj2)
              {
                if (var == varStrength)
                {
                  (*j)->strength = varStrength->GetFloatValue();
                }
                else

                if (var == varTime)
                {
                  (*j)->time = varTime->GetFloatValue();
                }
              }
            }
          }
        }
      }

      // Pass notification down
      Objects::NotifyVar(var);
    }


    //
    // GetSelectionFilter
    //
    // Returns the object selection filter for this brush
    //
    Common::GameWindow::SelectFilter * Curve::GetSelectionFilter()
    {
      return (SelectionFilter);
    }


    //
    // Called when a brush event is generated
    //
    void Curve::Notification(U32 crc, ::Event *e)
    {
      switch (crc)
      {
        case 0x5AB0F116: // "System::Activated"
        {
          // Find the marker object
          cameraObjType = GameObjCtrl::FindType<MarkerObjType>(CAMERA_TYPE);
          interestObjType = GameObjCtrl::FindType<MarkerObjType>(INTEREST_TYPE);

          if (!cameraObjType.Alive() || !interestObjType.Alive())
          {
            LOG_ERR(("Could not find marker object type"));
          }
          else
          {
            // Set it up in the base brush
            objectType = varFocus->GetIntegerValue() ? interestObjType : cameraObjType;
          }

          // Clear selected objects
          data.sList.Clear();

          prevLen = 0.0F;

          // Always pass system events down
          break;
        }

        case 0x6589C573: // "System::Deactivated"
        {
          // Delete all markers
          curveList.Clear();
          focusList.Clear();
          break;
        }

        case 0x90486A58: // "System::PreEventPoll"
        {
          // Purge dead objects
          curveList.PurgeDead();
          curveList.UpdateTime(varLoop->GetIntegerValue());
          focusList.PurgeDead();
          focusList.UpdateTime(varLoop->GetIntegerValue());

          // Is selected list different?
          if (!data.sList.Identical(prevSlist))
          {
            // Set values from first selected marker object
            SegmentList *list;
            SegmentListNode *node = GetFirstNode(data.sList, &list);

            // Only update the vars if 1 object is selected
            if (data.sList.GetCount() == 1)
            {
              if (node)
              {
                varStrength->SetFloatValue(node->strength);
                varTime->SetFloatValue(node->time);
                varIndex->SetIntegerValue(node->index);
                varStartTime->SetFloatValue(node->startTime);

                // Set varUseFocus to list of currently selected object
                ASSERT(list)
                varFocus->SetIntegerValue((list == &focusList) ? TRUE : FALSE);
              }
            }

            prevSlist.Clear();
            prevSlist.Dup(data.sList);
          }

          // Update length if necessary
          F32 len;
          
          if (focusList.GetCount())
          {
            len = Min<F32>(curveList.totalTime, focusList.totalTime);
          }
          else
          {
            len = curveList.totalTime;
          }

          if (len != prevLen)
          {
            prevLen = len;
            varCurrTime->GetItem().SetFloatRange(0.0F, len);

            if (config.Alive())
            {
              // Notify the config control that a bookmark was created
              config->SendNotify(config, Crc::CalcStr("Brush::Curve::Notify::LengthChanged"));
            }
          }
          break;
        }

        case 0x985B3F49: // "Command::Select"
        {
          // Select all objects in current list
          SegmentList *list = NULL;
          SegmentListNode *node = GetFirstNode(data.sList, &list);

          if (!node)
          {
            list = varFocus->GetIntegerValue() ? &focusList : &curveList;
          }

          // Copy all objects from list
          if (list)
          {
            data.sList.Clear();
            for (SegmentList::Iterator i(list); *i; i++)
            {
              data.sList.Append(**i);
            }
          }
          break;       
        }

        case 0x9031A85E: // "Brush::Objects::ObjectCreated"
        {
          if (lastCreatedObj.Alive())
          {
            if (!bookmarkPtr.Alive())
            {
              CON_ERR(("Create a bookmark first!"))

              // Pass this event down
              break;
            }

            // Append it to the curve list
            MarkerObj *marker;

            if ((marker = Promote::Object<MarkerObjType, MarkerObj>(lastCreatedObj)) != NULL)
            {
              SegmentList *list;
              SegmentListNode *first = GetFirstNode(data.sList, &list);

              if (!first)
              {
                list = varFocus->GetIntegerValue() ? &focusList : &curveList;
              }

              InsertNode
              (
                *list,
                marker, 
                first ? first->GetData() : NULL, 
                varStrength->GetFloatValue(),
                varTime->GetFloatValue(),
                TRUE
              );
            }
            else
            {
              LOG_DIAG(("That wasnt a markerobj"));
            }
          }
          else
          {
            LOG_DIAG(("The last object is dead?"))
          }

          // Pass this event down
          break;
        }

        case 0x0B304867: // "Brush::Curve::Message::Create"
        {
          Create(varNameEdit->GetStringValue());
          return;
        }

        case 0x103CFFB6: // "Brush::Curve::Message::Delete"
        {
          Delete();
          return;
        }

        case 0x05637962: // "Brush::Region::Message::Apply"
        {
          Create(varName->GetStringValue());
          return;
        }

        case 0xD9A7891C: // "Brush::Curve::Message::Upload"
        {
          // copy settings back to bookmark
          if (bookmarkPtr.Alive())
          {
            Upload(*bookmarkPtr);
          }
          break;
        }

        case 0xCB56E72D: // "Brush::Curve::Message::Download"
        {
          // copy settings from bookmark
          BookmarkObj *obj = BookmarkObj::FindBookmark(varName->GetStringValue());

          if (obj && obj->GetType() == BookmarkObj::CURVE)
          {
            bookmarkPtr = obj;
            Download(*obj);
          }
          break;
        }

        case 0xBFF8BC7F: // "Brush::Curve::Message::Info"
        {
          // Print some info
          CON_MSG(("Curve: %.1f sec  Focus: %.1f sec", curveList.totalTime, focusList.totalTime))

          // Info for selected objects
          for (MapObjList::Iterator i(&data.sList); *i; i++)
          {
            MarkerObj *obj = Promote::Object<MarkerObjType, MarkerObj>(**i);
            SegmentListNode *node;

            if (obj && ((node = FindNode(obj)) != NULL))
            {
              CON_MSG(("Marker: index %3d  start time %6.1f", node->index, node->startTime))
            }
          }
          break;
        }

        case 0xC3C52EA3: // "System::PreDraw"
        {
          if (varDrawLines->GetIntegerValue())
          {
            // Don't draw camera curve while in camera view
            if (!varUseCamera->GetIntegerValue())
            {
              RenderSegmentList(curveList, Color(255L, 255L, 255L, 255L));
            }
            RenderSegmentList(focusList, Color(0L, 0L, 255L, 255L));
          }

          if (varUseCamera->GetIntegerValue())
          {
            if (bookmarkPtr.Alive() && bookmarkPtr->HasFocus())
            {
              Matrix m = Matrix::I;
              F32 time = varCurrTime->GetFloatValue();

              if (bookmarkPtr->GetFocus().Step(time, m.posit))
              {
                Common::Display::Mesh(0x92098DF3, m, Color(255L, 255L, 0L)); // "TerrainMarker"
              }
            }
          }
          break;
        }

        
        case 0x31ED300A: // "Brush::Curve::Message::Stop"
        {
          varPlayMode->SetIntegerValue(FALSE);
          varFastMode->SetIntegerValue(FALSE);
          break;
        }
      }

      // Not blocked at this level
      Objects::Notification(crc, e);
    }

    //
    // Camera simulation callback
    //
    Bool Curve::CameraSimulate(FamilyNode *node, F32 time, void *context)
    {
      ASSERT(context)

      Curve *brush = (Curve *)context;
      Vector v, t;

      if (!brush->bookmarkPtr.Alive())
      {
        LOG_DIAG(("No bookmark"))
        brush->varUseCamera->SetIntegerValue(0);
        return (FALSE);
      }

      F32 currTime = brush->varCurrTime->GetFloatValue();
      Matrix m;

      if (brush->varFastMode->GetIntegerValue())
      {
        currTime += (time * 3.0F);
      }
      else

      if (brush->varPlayMode->GetIntegerValue())
      {
        currTime += time;
      }

      if (brush->bookmarkPtr->Step(currTime, m))
      {
        node->SetWorldAll(m);
        brush->varCurrTime->SetFloatValue(currTime);
        return (TRUE);
      }
      /*
      else
      {
        brush->varUseCamera->SetIntegerValue(0);
        return (FALSE);
      }
      */
      return (TRUE);
    }
  }
}
