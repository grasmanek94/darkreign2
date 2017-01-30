///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_CURVE_H
#define __STUDIO_BRUSH_CURVE_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_brush_objects.h"
#include "markerobj.h"
#include "bookmarkobj.h"
#include "spline.h"


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
    // Class Curve - Brush for curve creation
    //
    class Curve : public Objects
    {
    protected:

      ///////////////////////////////////////////////////////////////////////////////
      //
      // List of objects in the current curve segment
      //
      class SegmentListNode : public Reaper<MarkerObj>
      {
      public:

        // Curve segment from this object to the next
        CubicSpline segment;

        // Strength at this node
        F32 strength;

        // Time across this segment
        F32 time;

        // Starting time
        F32 startTime;

        // Index
        U32 index;

        // List node
        NList<SegmentListNode>::Node node;
      };

      struct SegmentList : public ReaperList<MarkerObj, SegmentListNode>
      {
      public:

        // Total list length
        F32 totalTime;

        // Update list length
        void UpdateTime(Bool looping)
        {
          U32 index = 1;
          totalTime = 0;

          for (SegmentList::Iterator i(this); *i; i++)
          {
            SegmentListNode *curr = *i;

            if ((i.IsTail() && looping) || !i.IsTail())
            {
              curr->index = index;
              curr->startTime = totalTime;
              totalTime += curr->time;
            }
            else
            {
              // Last (or only) item in a non looping list
              curr->index = index;
              curr->startTime = totalTime;
            }

            index++;
          }
        }

        // Delete all objects
        void Clear()
        {
          for (Iterator i(this); *i; i++)
          {
            if ((*i)->Alive())
            {
              (**i)->MarkForDeletion();
            }
          }
          ReaperList<MarkerObj, SegmentListNode>::Clear();
        }
      };

      // Curve segment objects
      SegmentList curveList;
      SegmentList focusList;

      // Bookmark pointer
      BookmarkObjPtr bookmarkPtr;

      // Type reaper of object to create
      MarkerObjTypePtr cameraObjType;
      MarkerObjTypePtr interestObjType;

      // List of selected objects
      MapObjList prevSlist;

      // Vars
      IFaceVar *varName;
      IFaceVar *varNameEdit;
      IFaceVar *varLoop;
      IFaceVar *varTime;
      IFaceVar *varStrength;
      IFaceVar *varStartTime;
      IFaceVar *varIndex;
      IFaceVar *varFocus;
      IFaceVar *varUseCamera;
      IFaceVar *varDrawLines;
      IFaceVar *varDrawNormals;
      IFaceVar *varCurrTime;
      IFaceVar *varPlayMode;
      IFaceVar *varFastMode;

      // Length last time control was drawn
      F32 prevLen;

    protected:

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

      // Create a curve
      void Create(const char *name);

      // Delete a curve
      void Delete();

      // Find an object's node in the list
      SegmentListNode* FindNode(MarkerObj *obj, SegmentList **foundList = NULL);

      // Get first SegmentListNode from a MapObjList
      SegmentListNode* GetFirstNode(MapObjList &list, SegmentList **foundList = NULL);

      // Add a new curve node
      void InsertNode(SegmentList &list, MarkerObj *node, MarkerObj *after, F32 strength = 100.0F, F32 timeStep = 10.0F, Bool align = FALSE);

      // Render a segment list
      void RenderSegmentList(SegmentList &list, Color c);

      // Upload the curve to the bookmark object
      void Upload(BookmarkObj &bookmark);

      // Download the curve from a bookmark object
      void Download(BookmarkObj &bookmark);

      // Download helper
      void Download(BookmarkObj::CurveSrc &src, SegmentList &list);

      // Step along a curve
      Bool Step(F32 &time, Matrix &matrix);

      // Camera simulation callback
      static Bool CameraSimulate(FamilyNode *node, F32 time, void *context);

    public:

      // Constructor and destructor
      Curve(const char *name);
      ~Curve();

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);

      // Returns the object selection filter for this brush
      Common::GameWindow::SelectFilter * GetSelectionFilter();
    };
  }
}

#endif