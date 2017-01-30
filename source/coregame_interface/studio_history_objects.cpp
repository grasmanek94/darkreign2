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

#include "studio_history_objects.h"
#include "mapobjctrl.h"
#include "gameobjctrl.h"
#include "movement_pathfollow.h"


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

    //
    // Data constructor
    //
    Objects::Data::Data(MapObj *o) : object(o)
    {
      type = object->MapType();
      matrix = object->WorldMatrix();
      zipped = object->GetFootInstance() ? TRUE : FALSE;

      UnitObj *unit = Promote::Object<UnitObjType, UnitObj>(object);

      if (unit)
      {
        team = unit->GetTeam();
      }
    }
    
    //
    // Constructor
    //
    Objects::Objects(Operation op, const MapObjList &list) : op(op), dataList(&Data::node)
    {
      // Setup data for each object in the list
      for (MapObjList::Iterator i(&list); *i; i++)
      {
        dataList.Append(new Data(**i));
      }
    }


    //
    // Constructor
    //
    Objects::Objects(Operation op, MapObj *object) : op(op), dataList(&Data::node)
    {
      ASSERT(object);

      // Setup data for the single object
      dataList.Append(new Data(object));
    }


    //
    // Destructor
    //
    Objects::~Objects()
    {
      dataList.DisposeAll();
    }


    //
    // Undo
    //
    // Undo operation
    //
    void Objects::Undo()
    {
      // Process each object
      for (NList<Data>::Iterator i(&dataList); *i; i++)
      {
        Data *d = *i;

        switch (op)
        {
          // Delete objects that were created
          case OP_CREATE:
            if (d->object.Alive())
            {
              GameObjCtrl::MarkForDeletion(d->object);
            }
            break;

          // Create objects that were deleted
          case OP_DELETE:
          {
            MapObj *m = MapObjCtrl::ObjectNewOnMap(d->type, d->matrix, 0, d->zipped);

            // Restore zipping
            if (d->zipped)
            {
              m->ToggleFootPrint(TRUE);
            }
  
            // Is this a unit
            UnitObj *u = Promote::Object<UnitObjType, UnitObj>(m);

            // Restore correct team
            if (u && d->team)
            {
              u->SetTeam(d->team);
            }

            // Save the old id
            U32 oldId = d->object.DirectId();

            // Now replace all references to the old object with the new object
            for (NList<Base>::Iterator items(&GetHistoryList()); *items; items++)
            {
              Objects *item = Promote<Objects>(*items);

              if (item)
              {
                for (NList<Data>::Iterator data(&item->dataList); *data; data++)
                {
                  if ((*data)->object.DirectId() == oldId)
                  {
                    (*data)->object = m;
                  }
                }
              }
            }

            break;
          }

          // Move objects back to original location
          case OP_MOVE:
            if (d->object.Alive())
            {
              d->object->SetSimCurrent(d->matrix);

              // Toggle claiming if necessary
              UnitObj *unitObj = Promote::Object<UnitObjType, UnitObj>(d->object);

              if (unitObj && unitObj->CanEverMove())
              {
                unitObj->GetDriver()->RemoveFromMapHook();
                unitObj->GetDriver()->AddToMapHook();
              }
            }
            break;

          // Restore zipped state
          case OP_ZIP:
            if (d->object.Alive())
            {
              d->object->ToggleFootPrint(d->zipped);
            }
            break;
        }
      }
    }


    //
    // Size
    //
    // Returns the size of this undo item
    //
    U32 Objects::Size()
    {
      return (sizeof(Objects) + (dataList.GetCount() * sizeof(Data)));
    }


    //
    // Name
    //
    // Returns the name of this history item
    //
    const char * Objects::Name()
    {
      switch (op)
      {
        case OP_CREATE: return ("Objects::Create");       
        case OP_DELETE: return ("Objects::Delete");
        case OP_MOVE:   return ("Objects::Move");        
        case OP_ZIP:    return ("Objects::Zip");

        default:        return ("Objects::Unknown");
      }
    }
  }
}
