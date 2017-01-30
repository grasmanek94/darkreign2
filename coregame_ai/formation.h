///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Formations
//
// 02-APR-1999
//

#ifndef __FORMATION_H
#define __FORMATION_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Formation
//
namespace Formation
{

  // Formation slot
  struct Slot
  {
    // Locational offset
    F32 direction;
    F32 distance;

    // Orientational offset
    F32 orientation;

    // Default constructor
    Slot();

    // Initializing constructor
    Slot(F32 direction, F32 distance, F32 orientation);

    // Loading constructor
    Slot(FScope *fScope);

    // Compare function
    F32 Compare(Slot *slot);

    // Save and load state
    void SaveState(FScope *scope);
    void LoadState(FScope *scope);

  };

  // List of slots in a formation
  struct Slots
  {
    List<Slot> slots;
    
    // Default constructor
    Slots();

    // Loading constructor
    Slots(FScope *fScope);

    // Destructor
    ~Slots();

    // Add a slot to the slots
    void Add(Slot *slot);

  };

  // Initialization and Shutdown
  void Init();
  void Done();

  // Create a formation for a squad
  void Create(const Vector &location, F32 direction, SquadObj *squad, F32 range);

  // Find a named formation
  Slots * Find(U32 crc);

  // Apply the named formation to the given units with a direction and orientaiton
  void Apply(U32 crc, const Vector &location, F32 direction, const UnitObjList &units, U32 flags);

  // Process config
  void ProcessConfig(FScope *fScope);

};

#endif