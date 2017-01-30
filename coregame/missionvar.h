///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Mission Vars
//

#ifndef __MISSIONVAR_H
#define __MISSIONVAR_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace MissionVar
//
namespace MissionVar
{

  // Char to dereference mission vars
  #define MISSION_VARCHAR '~'


  // Init
  void Init();

  // Done
  void Done();

  // Save the mission vars
  void Save(FScope *fScope);

  // Load the mission vars
  void Load(FScope *fScope);

}


#endif