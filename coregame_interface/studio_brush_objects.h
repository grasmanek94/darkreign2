///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_BRUSH_OBJECTS_H
#define __STUDIO_BRUSH_OBJECTS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"
#include "lopassfilter.h"



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
    class Objects : public Base
    {
    protected:

      // Custom list node
      class CustomListNode : public Reaper<MapObj>
      {
      public:

        // Original position
        Matrix matrix;

        // List node
        NList<CustomListNode>::Node node;
      };

      // Custom list for object movement
      typedef ReaperList<MapObj, CustomListNode> CustomList;

      // Current object being placed
      MapObjTypePtr objectType;

      // Last object created
      MapObjPtr lastCreatedObj;

      // Brush mode
      enum ObjectMode { OM_DEFAULT, OM_MOVE } mode;

      // Snap mode
      enum
      { 
        // Set Up vector
        SNAP_SET_UP   = 0x1,

        // Set to identity
        SNAP_IDENTITY = 0x2
      };

      // Starting position for object movement
      Vector moveStart;

      // List of objects being moved
      CustomList moveList;

      // Destination vector for object movement
      Vector moveDest;

      // Filter for movement
      LoPassFilter<Vector> moveFilter;

      // Quaternion for object rotation
      Quaternion rotateSrc, rotateDest;

      // Direction to create new objects
      WorldCtrl::CompassDir objectDir;

      // Interface vars
      IFaceVar *varType;

      // Current object to attach to
      MapObjPtr attachRoot;

      // Called when a brush event is generated
      void Notification(U32 crc, ::Event *e);

      // Create a new object (TRUE if created)
      Bool CreateNewObject();

      // Create a new object and attach it to the attach root at the given hardpoint
      Bool CreateNewObject(const char *point);

      // Check zipping positions
      void CheckZipping(const MapObjList &list);

      // Toggle zipping
      void ToggleZipping(const MapObjList &list, Bool toggle);

      // Toggle claiming
      void ToggleClaiming(const MapObjList &list, Bool toggle);

      // Toggle object movement
      void ToggleMovement(Bool toggle);

      // Move objects on the custom move list
      void MoveObjects(const Vector &offset);

      // Rotate objects on the custom move list
      void RotateObjects(const Quaternion &q);

      // Change height of each object to terrain or object height
      void SnapObjects(const MapObjList &list, const Matrix *matrix  = NULL, U32 flags = 0);

    public:

      // Constructor and destructor
      Objects(const char *name);
      ~Objects();

      // Does brush have given property (required)
      Bool HasProperty(U32 property);

      // Interface var notification (required)
      void NotifyVar(IFaceVar *var);
    };
  }
}

#endif