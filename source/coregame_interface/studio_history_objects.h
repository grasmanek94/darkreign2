///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Dark Reign 2 Studio
//
// 11-FEB-1999
//

#ifndef __STUDIO_HISTORY_OBJECTS_H
#define __STUDIO_HISTORY_OBJECTS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_private.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Studio - Mission creation environment
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Namespace History - Undo/Redo functionality
  //
  namespace History
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Class Objects - History functions dealing with objects
    //
    class Objects : public Base
    {
      PROMOTE_LINK(Objects, Base, 0xEC92967E); // "Objects"

    public:

      // All possible operations
      enum Operation 
      { 
        OP_CREATE,
        OP_DELETE, 
        OP_MOVE,
        OP_ZIP
      };

    protected:

      // Custom list node
      struct Data
      {       
        // Object data
        MapObjPtr object;
        MapObjTypePtr type;
        Matrix matrix;
        Team *team;
        Bool zipped;

        // List node
        NList<Data>::Node node;

        // Constructor
        Data(MapObj *o);
      };

      // List for operations
      NList<Data> dataList;

      // The operation 
      Operation op;

      // Virtual methods
      void Undo();
      U32 Size();

    public:

      // Constructors and destructor
      Objects(Operation op, const MapObjList &list);
      Objects(Operation op, MapObj *object);
      ~Objects();

      // The name of this history item
      const char * Name();
    };
  }
}

#endif