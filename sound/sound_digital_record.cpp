///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Sound System
//
// 8-OCT-1997
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "sound_private.h"


namespace Sound 
{ 
  namespace Digital
  {
    ///////////////////////////////////////////////////////////////////////////////
    //
    // Struct Record - Data maintained for each known sound effect
    //

    //
    // Static data
    // 
    NBinTree<Record> Record::tree(&Record::node);


    //
    // Constructor
    //
    Record::Record(const char *name)
    {
      RegisterConstruction(dTrack);

      // Save the crc of the name
      nameCrc = Crc::CalcStr(name);

      // Add to the record tree
      tree.Add(nameCrc, this);

      // Get fast find file instance
      if ((fastFind = FileSys::GetFastFind(name)) != NULL)
      {
        valid = TRUE;
      }
      else
      {
        LOG_WARN(("Sound data file not found [%s]", name));
        valid = FALSE;
      }

      // Setup default values
      freq = 0;
      lastUse = 0;
    }


    //
    // Destructor
    //
    Record::~Record()
    {
      if (fastFind)
      {
        delete fastFind;
      }

      RegisterDestruction(dTrack);
    }


    //
    // Name
    //
    // File name of this record
    //
    const char * Record::Name()
    {
      return (fastFind ? fastFind->Name() : "[Invalid]");
    }


    //
    // Request
    //
    // Get the record for a file
    //
    Record * Record::Request(const char *name)
    {
      ASSERT(name);

      // Do we already have a record for this file
      if (Record *record = tree.Find(Crc::CalcStr(name)))
      {
        return (record);
      }

      // Allocate a new record
      return (new Record(name));
    }

    
    //
    // ReleaseAll
    //
    // Release all current records
    //
    void Record::ReleaseAll()
    {
      tree.DisposeAll();
    }
  }
}

