///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// vid.h
//
// 01-APR-1998        
//

#ifndef __VID_H
#define __VID_H

//-----------------------------------------------------------------------------

namespace Vid
{
  enum TranSortValues
  {
    sortDEBUG0       = 0,                    // debug
    sortFLARE0       = sortDEBUG0   + 4096,  // sprites & effects
    sortLIGHT0       = sortFLARE0   + 4096,
    sortFOG0         = sortLIGHT0   + 4096,
    sortFLAME0       = sortFOG0     + 4096,
    sortSMOKE0       = sortFLAME0   + 4096,
    sortEFFECT0      = sortSMOKE0   + 4096,  // mesheffects
    sortNORMAL0      = sortEFFECT0  + 4096,  // meshes
    sortSURFACE0     = sortNORMAL0  + 4096,  // groundwatersprites
    sortWATER0       = sortSURFACE0 + 4096,
    sortBRUSH0       = sortWATER0   + 4096,  // terrain editing brush
    sortGROUND0      = sortBRUSH0   + 4096,  // groundsprites
    sortTERRAIN0     = sortGROUND0  + 4096,
  };
};

#endif		// __VID_H

