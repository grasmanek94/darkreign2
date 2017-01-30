///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 4-DEC-1998
//


#ifndef __COMMON_STATS_H
#define __COMMON_STATS_H


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "icontrol.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Editor
//
namespace Common
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // NameSpace StatsMsg
  //
  namespace StatsMsg
  {
    const U32 Query = 0x1C09607C; // "Common::Stats::Message::Query"
    const U32 Save  = 0x6CD7ACE6; // "Common::Stats::Message::Save"
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Stats
  //
  class Stats : public IControl
  {
    PROMOTE_LINK(TeamStatusListBox, IControl, 0xC01F9142); // "Stats"

  private:

    // Control container
    struct Container;

    // Stat button
    struct Button;

    // Query heading
    struct QueryHeading;

    // Query results
    struct QueryResults;

    // Query set
    struct QuerySet;

    // Query group
    struct QueryGroup;

    enum TeamType
    {
      TEAM_ALL,
      TEAM_ID,
      TEAM_TOTAL
    };

    enum InformationType
    {
      INFO_ALL,
      INFO_RESULT,
      INFO_RESOURCE,
      INFO_CONSTRUCTED,
      INFO_LOST,
      INFO_DESTROYED,
      INFO_MAX
    };

    struct Position
    {
      U32 offset;
      U32 size;
    };

    // Column weightings
    F32 columnWeightings[INFO_MAX];

    // Total column weighting
    F32 totalColumnWeighting;

    // Positions of columns
    Position columnPositions[INFO_MAX];

    // Gap between columns
    U32 columnGap;

    // Height of rows
    U32 rowHeight;

    // Gap between rows
    U32 rowGap;

    // Gap for total row
    U32 totalGap;

    // Container
    Container *container;

    // Name of the query control
    IControlName queryCtrlName;

    // Query control
    IControlPtr queryCtrl;

    // Name of the query listbox
    IControlName queryListName;

    // Query listbox
    ICListBoxPtr queryList;

    // Title config
    FScope *titleConfig;

    // Team config
    FScope *teamConfig;

    // Item config
    FScope *itemConfig;

    // Info headings
    static const char *infoHeadings[INFO_MAX];

  public:

    Stats(IControl *parent);
    ~Stats();

    // Setup this control from one scope function
    void Setup(FScope *fScope);

    // Event handling function
    U32 HandleEvent(Event &e);

    // Activate
    Bool Activate();

    // Deactivate
    Bool Deactivate();

  private:

    // Query
    U32 Query(InformationType informationType, U32 teamId);

    // Query Stats Category
    void QueryStatsCategory(NBinTree<QuerySet> &querySets, const ::Stats::Category &category, U32 teamId);

    // Query
    void Query(InformationType informationType, U32 teamId, NList<QueryGroup> &queryGroups);

    // Query
    void Query(TeamType teamType, InformationType informationType, U32 teamId, ICListBox *listBox);

    // Query
    const CH * Query(TeamType teamType, InformationType informationType, U32 teamId);

    // Save
    void Save();

    // Compute totals
    static void ComputeTotals(NList<QueryGroup> &queryGroups);

    // Add a query set to a listbox
    static void PrintResults(NList<QueryGroup> &queryGroups, Bool *teams, ICListBox *listBox);

    // FindCreate a query group
    static QueryGroup * FindCreateGroup(NList<QueryGroup> &queryGroups, const char *name);

  public:

    friend Button;
    friend QueryGroup;

  };
}

#endif
