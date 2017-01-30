///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Interface System
//
// 4-DEC-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "common_stats.h"
#include "icbutton.h"
#include "icstatic.h"
#include "iface.h"
#include "iface_util.h"
#include "babel.h"
#include "team.h"
#include "player.h"
#include "font.h"
#include "gameobjctrl.h"

#include "missions.h"

#pragma warning(push, 3)
#include <fstream>
#include <ios>
#pragma warning(pop)

using std::ofstream;
using std::endl;


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Common
//
namespace Common
{

  #define LANGPREFIX "#game.stats."
  #define LANGCATEGORYPREFIX "#game.stats.categories."


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Stats::Container
  //
  struct Stats::Container : public IControl
  {
    // Constructor
    Container(IControl *parent) 
    : IControl(parent) 
    {
      geom.flags |= GEOM_PARENTWIDTH | GEOM_PARENTHEIGHT;
      SetName("[Container]");
    }

    friend Stats;
  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Stats::Button
  //
  struct Stats::Button : public ICButton
  {
    PROMOTE_LINK(Button, ICButton, 0x2876277B) // "Stats::Button"

    TeamType teamType;
    InformationType informationType;
    U32 teamId;

    // Constructor
    Button(TeamType teamType, InformationType informationType, U32 teamId, const CH *text, const Area<S32> &area, IControl *handler, IControl *parent, FScope *config)
    : teamType(teamType),
      informationType(informationType),
      teamId(teamId),
      ICButton(parent) 
    {
      SetPos(area.p0.x, area.p0.y);
      SetSize(area.Width(), area.Height());
      SetGeomSize(area.Width(), area.Height());
      SetTextFont("Button");
      SetTextString(text, TRUE);
      AddEventTranslation(ICButtonNotify::Pressed, StatsMsg::Query, handler);

      if (config)
      {
        config->InitIterators();
        Configure(config);
      }

      if (informationType == INFO_RESULT)
      {
        controlState |= STATE_DISABLED;
      }
    }

    void DrawSelf(PaintInfo &pi)
    {
      if (teamType == Stats::TEAM_ID)
      {
        DrawCtrlBackground(pi, GetTexture());
        DrawCtrlFrame(pi);
        IFace::RenderRectangle(pi.client, Team::Id2Team(teamId)->GetDimColor());
        DrawCtrlText(pi);
      }
      else
      {
        ICButton::DrawSelf(pi);
      }
    }

    friend Stats;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Stats::QueryHeading
  //
  struct Stats::QueryHeading : public ICStatic
  {

    QueryHeading(const CH *text, IControl *parent)
    : ICStatic(parent)
    {
      controlState |= STATE_HILITE;
      SetTextString(text, TRUE);
    }

    void DrawSelf(PaintInfo &pi)
    {
      DrawCtrlBackground(pi);

      // Add Title
      U32 vCentre = (pi.client.Height() - pi.font->Height()) / 2;
      pi.font->Draw
      (
        pi.client.p0.x + 5, 
        pi.client.p0.y + vCentre,
        GetTextString(),
        Utils::Strlen(GetTextString()),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Draw seperator line
      IFace::RenderRectangle
      (
        ClipRect(
          pi.client.p0.x,
          pi.client.p0.y,
          pi.client.p1.x,
          pi.client.p0.y + 1),
        pi.colors->fg[ColorIndex()], 
        NULL,
        pi.alphaScale
      );
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Stats::QueryResults
  //
  struct Stats::QueryResults : public ICStatic
  {
    struct Column
    {
      U32 value;
      Color color;
    };

    const char *title;
    U32 numColumns;
    Column *columns;

    QueryResults(IControl *parent, const char *title, U32 numColumns, Column *columns)
    : ICStatic(parent),
      title(title),
      numColumns(numColumns),
      columns(columns)
    {
    }

    ~QueryResults()
    {
      delete columns;
    }

    void DrawSelf(PaintInfo &pi)
    {
      DrawCtrlBackground(pi);

      U32 vCentre = (pi.client.Height() - pi.font->Height()) / 2;

      // Add Category Name
      const CH *t;
      GameObjType *type = GameObjCtrl::FindType<GameObjType>(title);
      if (type)
      {
        t = type->GetDesc();
      }
      else
      {
        char buff[256];
        Utils::Sprintf(buff, 256, "%s%s", LANGCATEGORYPREFIX, title);
        t = TRANSLATE((buff));
      }
      pi.font->Draw
      (
        pi.client.p0.x + 5, 
        pi.client.p0.y + vCentre,
        t,
        Utils::Strlen(t),
        pi.colors->fg[ColorIndex()],
        &pi.client
      );

      // Offset
      U32 offset = 150;
      U32 width = 48;

      // Draw each of the columns
      for (U32 c = 0; c < numColumns; ++c)
      {
        // Draw background color
        IFace::RenderRectangle
        (
          ClipRect(
            pi.client.p0.x + offset + 1,
            pi.client.p0.y + 1,
            pi.client.p0.x + offset + width,
            pi.client.p1.y),
          columns[c].color, 
          NULL,
          pi.alphaScale
        );

        // Draw text
        CH buff[32];
        Utils::Sprintf(buff, 32, L"%d", columns[c].value);
        U32 length = Utils::Strlen(buff);
        U32 hCentre = (width - pi.font->Width(buff, length)) / 2;
        pi.font->Draw
        (
          pi.client.p0.x + offset + hCentre, 
          pi.client.p0.y + vCentre,
          buff,
          length,
          pi.colors->fg[ColorIndex()],
          &pi.client
        );

        offset += width;
      }
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Stats::QuerySet
  //
  struct Stats::QuerySet
  {
    NBinTree<QuerySet>::Node node;

    const char *title;
    U32 value[Game::MAX_TEAMS + 1];

    QuerySet(const char *title)
    : title(title)
    {
      Utils::Memset(value, 0x00, sizeof (value));
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Stats::QueryGroup
  //
  struct Stats::QueryGroup
  {
    char *title;
    NBinTree<QuerySet> querySets;
    NList<QueryGroup>::Node node;

    QueryGroup(const char *title)
    : title(Utils::Strdup(title)),
      querySets(&QuerySet::node)
    {
    }

    ~QueryGroup()
    {
      delete title;
      querySets.DisposeAll();
    }

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Stats
  //

  // Info headings
  const char * Stats::infoHeadings[Stats::INFO_MAX] =
  {
    LANGPREFIX "teams",
    LANGPREFIX "result",
    LANGPREFIX "resource",
    LANGPREFIX "constructed",
    LANGPREFIX "lost",
    LANGPREFIX "destroyed"
  };


  //
  // Constructor
  //
  Stats::Stats(IControl *parent) 
  : IControl(parent),
    itemConfig(NULL),
    titleConfig(NULL),
    teamConfig(NULL)
  {
    // Create container control
    container = new Container(this);

    // Initialize all the spacings to be something sensible
    totalColumnWeighting = 0.0f;
    for (int i = INFO_ALL; i < INFO_MAX; i++)
    {
      columnWeightings[i] = 1.0f;
      totalColumnWeighting += 1.0f;
    }
    columnGap = 5;
    rowHeight = 30;
    rowGap = 5;
    totalGap = 5;
  }


  //
  // Destructor
  //
  Stats::~Stats()
  {
    if (itemConfig)
    {
      delete itemConfig;
    }
    if (titleConfig)
    {
      delete titleConfig;
    }
    if (teamConfig)
    {
      delete teamConfig;
    }
  }

  
  //
  // Setup this control from one scope function
  //
  void Stats::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0xA7E28F0D: // "ColumnWeightings"
      {
        totalColumnWeighting = 0.0f;
        for (int i = INFO_ALL; i < INFO_MAX; i++)
        {
          F32 val = StdLoad::TypeF32(fScope);
          columnWeightings[i] = val;
          totalColumnWeighting += val;
        }
        break;
      }

      case 0x6BC0CF12: // "QueryCtrl"
        queryCtrlName = StdLoad::TypeString(fScope);
        break;

      case 0x235DECB4: // "QueryList"
        queryListName = StdLoad::TypeString(fScope);
        break;

      case 0xF4E44C64: // "ItemConfig"
        itemConfig = fScope->Dup();
        break;

      case 0xDA4ED14C: // "TitleConfig"
        titleConfig = fScope->Dup();
        break;

      case 0xC73417F9: // "TeamConfig"
        teamConfig = fScope->Dup();
        break;

      case 0xDEF41550: // "ColumnGap"
        columnGap = StdLoad::TypeU32(fScope);
        break;

      case 0x89E7524A: // "RowGap"
        rowGap = StdLoad::TypeU32(fScope);
        break;

      case 0xFC197CFE: // "TotalGap"
        totalGap = StdLoad::TypeU32(fScope);
        break;

      case 0x7EB404A2: // "RowHeight"
        rowHeight = StdLoad::TypeU32(fScope);
        break;

      default:
        IControl::Setup(fScope);
        break;
    }
  }


  //
  // Event handling function
  //
  U32 Stats::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          switch (e.iface.p1)
          {
            case StatsMsg::Save:
              Save();
              break;

            case StatsMsg::Query:
            {
              if (queryCtrl.Alive() && queryList.Alive())
              {
                // This is a query from a stats button (we hope)
                Button *button = IFace::Promote<Button>(static_cast<IControl *>(e.iface.from));

                if (button && button->informationType != INFO_RESULT)
                {
                  // Process the query and create the results
                  Query(button->teamType, button->informationType, button->teamId, queryList);

                  // Set the text of the control to reflect the query
                  const CH *team = NULL;
                  switch (button->teamType)
                  {
                    case TEAM_ALL:
                      team = TRANSLATE(("#game.stats.all.teams"));
                      break;

                    case TEAM_ID:
                    {
                      Team *t = Team::Id2Team(button->teamId);
                      team = t->IsAvailablePlay() && t->GetTeamLeader() ? 
                              t->GetTeamLeader()->GetDisplayName() : 
                              Utils::Ansi2Unicode(t->GetName());
                      break;
                    }

                    case TEAM_TOTAL:
                      team = TRANSLATE(("#game.stats.total"));
                      break;
                  }
                  ASSERT(team)

                  const CH *info = NULL;
                  switch (button->informationType)
                  {
                    case INFO_ALL:
                      info = TRANSLATE(("#game.stats.all.info"));
                      break;

                    default:
                      info = TRANSLATE((infoHeadings[button->informationType]));
                      break;
                  }
                  ASSERT(info)
                  
                  CH buff[256];
                  Utils::Sprintf(buff, 256, L"%s - %s", team, info);

                  queryCtrl->SetTextString(buff, TRUE);

                  // Activate the query
                  queryCtrl->Activate();

                  // Raise the window
                  queryCtrl->SetZPos(0);
                }
              }

              // Handled
              return (TRUE);
            }
          }
        }
      }
    }
    return (IControl::HandleEvent(e));
  }


  //
  // Activate
  //
  Bool Stats::Activate()
  {
    if (IControl::Activate())
    {
      queryCtrl = IFace::FindByName(queryCtrlName.str, this);
      IControl *ctrl = IFace::FindByName(queryListName.str, this);
      if (ctrl)
      {
        queryList = IFace::Promote<ICListBox>(ctrl);
      }

      U32 x = columnGap;
      F32 total = F32(GetSize().x - columnGap * INFO_MAX);

      int i;

      // Compute the column position information
      for (i = INFO_ALL; i < INFO_MAX; i++)
      {
        columnPositions[i].offset = x;
        columnPositions[i].size = U32(total * columnWeightings[i] / totalColumnWeighting);
        x += columnPositions[i].size + columnGap;
      }

      // Empty the container
      container->DisposeChildren();

      U32 y = rowGap;

      // Lets build up the stats grid

      // Add the information headers
      for (i = INFO_ALL; i < INFO_MAX; i++)
      {
        (new Button
          (
            TEAM_ALL, 
            static_cast<InformationType>(i), 
            0,
            TRANSLATE((infoHeadings[i])), 
            Area<S32>
            (
              columnPositions[i].offset, y,
              columnPositions[i].offset + columnPositions[i].size, y + rowHeight
            ), 
            this,
            container,
            titleConfig
          )
        )->Activate();
      }

      // Add the teams
      y += rowHeight;
      y += rowGap;
      
      for (U32 teamId = 0; teamId < Game::MAX_TEAMS; teamId++)
      {
        Team *team = Team::Id2Team(teamId);
        if (team && team->HasStats())
        {
          // Add the team title
          (new Button
            (
              TEAM_ID, 
              INFO_ALL, 
              teamId,
              team->IsAvailablePlay() && team->GetTeamLeader() ? 
                team->GetTeamLeader()->GetDisplayName() : 
                Utils::Ansi2Unicode(team->GetName()), 
              Area<S32>
              (
                columnPositions[INFO_ALL].offset, y,
                columnPositions[INFO_ALL].offset + columnPositions[INFO_ALL].size, y + rowHeight
              ), 
              this,
              container,
              teamConfig
            )
          )->Activate();

          // Add all of the stats
          for (i = INFO_RESULT; i < INFO_MAX; i++)
          {
            (new Button
              (
                TEAM_ID,
                static_cast<InformationType>(i), 
                teamId,
                Query(TEAM_ID, static_cast<InformationType>(i), teamId),
                Area<S32>
                (
                  columnPositions[i].offset, y,
                  columnPositions[i].offset + columnPositions[i].size, y + rowHeight
                ), 
                this,
                container,
                itemConfig
              )
            )->Activate();
          }

          y += rowHeight;
          y += rowGap;
        }
      }

      // Add the totals
      y += totalGap;

      // Add total title
      (new Button
        (
          TEAM_TOTAL,
          INFO_ALL, 
          0,
          TRANSLATE(("#game.stats.total")),
          Area<S32>
          (
            columnPositions[INFO_ALL].offset, y,
            columnPositions[INFO_ALL].offset + columnPositions[INFO_ALL].size, y + rowHeight
          ), 
          this,
          container,
          itemConfig
        )
      )->Activate();

      // Add all of the stats
      for (i = INFO_RESOURCE; i < INFO_MAX; i++)
      {
        (new Button
          (
            TEAM_TOTAL,
            static_cast<InformationType>(i), 
            0,
            Query(TEAM_TOTAL, static_cast<InformationType>(i), 0),
            Area<S32>
            (
              columnPositions[i].offset, y,
              columnPositions[i].offset + columnPositions[i].size, y + rowHeight
            ), 
            this,
            container,
            itemConfig
          )
        )->Activate();
      }

      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }


  //
  // Deactivate
  //
  Bool Stats::Deactivate()
  {
    if (IControl::Deactivate())
    {
      return (TRUE);
    }
    else
    {
      return (FALSE);
    }
  }



  //
  // Query
  //
  U32 Stats::Query(InformationType informationType, U32 teamId)
  {
    Team *team = Team::Id2Team(teamId);

    if (team)
    {
      switch (informationType)
      {
        case INFO_RESOURCE:
          return (team->GetEndGame().resources.GetAmount(0xA05DF8D0)); // "resource.collected"
          break;

        case INFO_CONSTRUCTED:
          return (team->GetEndGame().constructed.GetTotal());
          break;

        case INFO_LOST:
          return (team->GetEndGame().losses.GetTotal());
          break;

        case INFO_DESTROYED:
          return 
          (
            team->GetEndGame().kills[Relation::ENEMY]->GetTotal() +
            team->GetEndGame().kills[Relation::ALLY]->GetTotal() +
            team->GetEndGame().kills[Relation::NEUTRAL]->GetTotal() +
            team->GetEndGame().killsSelf.GetTotal() +
            team->GetEndGame().killsMisc.GetTotal()
          );
          break;

        default:
          ERR_FATAL(("Unknown information type %d", informationType))
      }
    }
    else
    {
      return (0);
    }
  }


  //
  // QueryStatsCategory
  //
  void Stats::QueryStatsCategory(NBinTree<QuerySet> &querySets, const ::Stats::Category &category, U32 teamId)
  {
    ::Stats::Category::Iterator iter;
    const char *name;
    U32 num;
    U32 total;
    F32 percentage;

    category.ResetIterator(iter);
    while (category.Iterate(iter, name, num, total, percentage))
    {
      U32 crc = Crc::CalcStr(name);
      QuerySet *q = querySets.Find(crc);
      if (!q)
      {
        q = new QuerySet(name);
        querySets.Add(crc, q);
      }
      q->value[teamId] = num;
    }
  }


  //
  // Query
  //
  void Stats::Query(InformationType informationType, U32 teamId, NList<QueryGroup> &queryGroups)
  {
    Team *team = Team::Id2Team(teamId);

    if (team)
    {
      switch (informationType)
      {
        case INFO_RESULT:
          break;

        case INFO_RESOURCE:
          QueryStatsCategory(FindCreateGroup(queryGroups, infoHeadings[informationType])->querySets, team->GetEndGame().resources, teamId);
          break;

        case INFO_CONSTRUCTED:
          QueryStatsCategory(FindCreateGroup(queryGroups, infoHeadings[informationType])->querySets, team->GetEndGame().constructed, teamId);
          break;

        case INFO_LOST:
          QueryStatsCategory(FindCreateGroup(queryGroups, infoHeadings[informationType])->querySets, team->GetEndGame().losses, teamId);
          break;

        case INFO_DESTROYED:
        {
          char buff[128];
          for (U32 r = 0; r < Relation::MAX; r++)
          {
            Utils::Sprintf(buff, 128, "%s%s", infoHeadings[informationType], Relation(Relation::Type(r)).GetName());
            QueryStatsCategory(FindCreateGroup(queryGroups, buff)->querySets, *team->GetEndGame().kills[r], teamId);
          }

          Utils::Sprintf(buff, 128, "%sSelf", infoHeadings[informationType]);
          QueryStatsCategory(FindCreateGroup(queryGroups, buff)->querySets, team->GetEndGame().killsSelf, teamId);

          Utils::Sprintf(buff, 128, "%sMisc", infoHeadings[informationType]);
          QueryStatsCategory(FindCreateGroup(queryGroups, buff)->querySets, team->GetEndGame().killsMisc, teamId);
          break;
        }

        default:
          ERR_FATAL(("Unknown information type %d", informationType))
      }
    }
  }


  //
  // Query
  //
  const CH * Stats::Query(TeamType teamType, InformationType informationType, U32 teamId)
  {
    switch (teamType)
    {
      case TEAM_ALL:
        ERR_FATAL(("Cannot query TEAM_ALL"))

      case TEAM_ID:
      {
        Team *team = Team::Id2Team(teamId);
        ASSERT(team)
        switch (informationType)
        {
          case INFO_ALL:
            return (L"");

          case INFO_RESULT:
            return (TRANSLATE((Team::EndGame::resultKey[team->GetEndGame().result])));
            break;

          default:
            return (TRANSLATE(("{1:d}", 1, Query(informationType, teamId))));
        }
        break;
      }

      case TEAM_TOTAL:
        switch (informationType)
        {
          case INFO_RESULT:
            return (L"N/A");

          default:
          {
            U32 sum = 0;
            for (teamId = 0; teamId < Game::MAX_TEAMS; teamId++)
            {
              sum += Query(informationType, teamId);
            }
            return (TRANSLATE(("{1:d}", 1, sum)));
            break;
          }
        }
        break;

      default:
        ERR_FATAL(("Unknown TeamType %d", teamType))
    }
  }

  
  //
  // Query
  //
  void Stats::Query(TeamType teamType, InformationType informationType, U32 teamId, ICListBox *listBox)
  {
    ASSERT(listBox)

    switch (teamType)
    {
      case TEAM_ALL:
      case TEAM_TOTAL:
      {
        Bool teams[Game::MAX_TEAMS + 1];
        NList<QueryGroup> queryGroups(&QueryGroup::node);

        for (teamId = 0; teamId < Game::MAX_TEAMS; teamId++)
        {
          Team *team = Team::Id2Team(teamId);
          teams[teamId] = (team && team->HasStats() && (teamType == TEAM_ALL)) ? TRUE : FALSE;
          if (team)
          {
            if (informationType == INFO_ALL)
            {
              // If the information type is all, add all of the information types in order
              for (int i = INFO_RESOURCE; i < INFO_MAX; i++)
              {
                Query(static_cast<InformationType>(i), teamId, queryGroups);
              }
            }
            else
            {
              Query(informationType, teamId, queryGroups);
            }
          }
        }

        ComputeTotals(queryGroups);
        teams[Game::MAX_TEAMS] = TRUE;

        PrintResults(queryGroups, teams, listBox);
        queryGroups.DisposeAll();
        break;
      }

      case TEAM_ID:
      {
        NList<QueryGroup> queryGroups(&QueryGroup::node);
        if (informationType == INFO_ALL)
        {
          // If the information type is all, add all of the information types in order
          for (int i = INFO_RESOURCE; i < INFO_MAX; i++)
          {
            Query(static_cast<InformationType>(i), teamId, queryGroups);
          }
        }
        else
        {
          Query(informationType, teamId, queryGroups);
        }

        Bool teams[Game::MAX_TEAMS + 1];
        Utils::Memset(teams, 0x00, sizeof(teams));
        teams[teamId] = TRUE;

        PrintResults(queryGroups, teams, listBox);
        queryGroups.DisposeAll();
        break;
      }

      default:
        ERR_FATAL(("Unknown TeamType %d", teamType))
    }
  }


  //
  // Save
  //
  void Stats::Save()
  {
    // Collect all of the stats
    Bool teams[Game::MAX_TEAMS + 1];
    NList<QueryGroup> queryGroups(&QueryGroup::node);

    for (U32 teamId = 0; teamId < Game::MAX_TEAMS; teamId++)
    {
      Team *team = Team::Id2Team(teamId);
      teams[teamId] = (team && team->HasStats()) ? TRUE : FALSE;
      if (team)
      {
        // If the information type is all, add all of the information types in order
        for (int i = INFO_RESOURCE; i < INFO_MAX; i++)
        {
          Query(static_cast<InformationType>(i), teamId, queryGroups);
        }
      }
    }

    ComputeTotals(queryGroups);
    teams[Game::MAX_TEAMS] = TRUE;

    U32 t;

    if (!File::Exists("stats", "."))
    {
      Dir::Make("stats");
    }

    char file[256] = "stats\\";
    {
      Utils::Strcat(file, Clock::GetDateAndTime());

      // Replace ':' with '.'
      char *ptr = file;
      while (ptr && *ptr)
      {
        ptr = Utils::Strchr(ptr, ':');
        if (ptr)
        {
          *(ptr++) = '.';
        }
      }

      // Append ".html"
      Utils::Strcat(file, ".html");
    }

    // Open a stats file and start writting
    ofstream out(file);

    out << "<html>" << endl;
    out << "<head><title>DR2 Stats</title></head>" << endl;
    out << "<basefont face=\"arial\" color=#ffffff>" << endl;
    out << "<body bgcolor=#000000>" << endl;

    out << "<h1>Dark Reign 2 Match Statistics</h1>" << endl;

    out << "<table>" << endl;
    out << "<tr><td><b>Completed:</b></td><td>" << 
      Clock::Date::GetVerbose() << "</td></tr>" << endl;
    out << "<tr><td><b>Duration:</b></td><td>" << 
      U32(GameTime::SimTotalTime() / 60.0f) << "min " <<
      fmod(GameTime::SimTotalTime(), 60.0f) << "sec" << "</td></tr>" << endl;
    out << "<tr><td><b>Map:</b></td><td>" << 
      (Missions::GetActive() ? Missions::GetActive()->GetName().str : "???")
      << "</td></tr>" << endl;
    //out << "<tr><td><b>RuleSet:</b></td><td>" << "</td></tr>" << endl;
    out << "</table>" << endl;

    out << "<br>" << endl;

    out << "<table cellpadding=2 cellspacing=1 bgcolor=#303030>" << endl;

    U32 numCols = 2;

    // Write team headings
    out << "<tr><td>Teams</td>";
    for (t = 0; t < Game::MAX_TEAMS; ++t)
    {
      if (teams[t])
      {
        numCols++;

        Team *team = Team::Id2Team(t);
        ASSERT(team)

        out << "<td align=center width=100 bgcolor=" << team->GetDimColor() << ">";

        for (List<Player>::Iterator p(&team->GetPlayers()); *p; ++p)
        {
          out << Utils::Unicode2Ansi((*p)->GetDisplayName()) << "<br>";
        }

        out << "</td>" << endl;
      }
    }
    out << "<td align=center width=100 bgcolor=#303030>Total</td></tr>" << endl;

    // Write results
    out << "<tr><td>Result</td>";
    for (t = 0; t < Game::MAX_TEAMS; ++t)
    {
      if (teams[t])
      {
        Team *team = Team::Id2Team(t);
        ASSERT(team)

        out << "<td align=center bgcolor=" << team->GetDimColor() << ">" << 
          Utils::Unicode2Ansi(TRANSLATE((Team::EndGame::resultName[team->GetEndGame().result]))) << "</td>" << endl;
      }
    }
    out << "</tr>" << endl;

    // Write the query groups
    for (NList<QueryGroup>::Iterator g(&queryGroups); *g; ++g)
    {
      // Write group title
      out << "<tr><td colspan=" << std::dec << numCols << " bgcolor=#DFDFDF><font color=#000000>" << 
        Utils::Unicode2Ansi(TRANSLATE(((*g)->title))) << "</font></td></tr>" << endl;

      // Write the items in the query set
      for (NBinTree<QuerySet>::Iterator q(&(*g)->querySets); *q; ++q)
      {
        // Work out the name
        const CH *title;
        GameObjType *type = GameObjCtrl::FindType<GameObjType>((*q)->title);
        if (type)
        {
          title = type->GetDesc();
        }
        else
        {
          char buff[256];
          Utils::Sprintf(buff, 256, "%s%s", LANGCATEGORYPREFIX, (*q)->title);
          title = TRANSLATE((buff));
        }

        out << "<tr><td>" << Utils::Unicode2Ansi(title) << "</td>";

        for (t = 0; t <= Game::MAX_TEAMS; ++t)
        {
          if (teams[t])
          {
            Team *team = Team::Id2Team(t);

            out << "<td align=right bgcolor=" << 
              ((t < Game::MAX_TEAMS) ? team->GetDimColor() : 
              Color(U32(0x30), U32(0x30), U32(0x30))) << ">" << 
                std::dec << (*q)->value[t] << "</td>" << endl;
          }
        }
        out << "</tr>" << endl;
      }
    }

    out << "</table>" << endl;

    out << "</body>" << endl;
    
    out << "</html>" << endl;

    // Close the file
    out.close();

    queryGroups.DisposeAll();
  }


  //
  // Compute totals
  //
  void Stats::ComputeTotals(NList<QueryGroup> &queryGroups)
  {
    for (NList<QueryGroup>::Iterator g(&queryGroups); *g; ++g)
    {
      for (NBinTree<QuerySet>::Iterator q(&(*g)->querySets); *q; ++q)
      {
        for (U32 t = 0; t < Game::MAX_TEAMS; ++t)
        {
          (*q)->value[Game::MAX_TEAMS] += (*q)->value[t];
        }
      }
    }
  }

  
  //
  // Add a query set to the listbox
  //
  void Stats::PrintResults(NList<QueryGroup> &queryGroups, Bool *teams, ICListBox *listBox)
  {
    ASSERT(listBox)
    listBox->DeleteAllItems();

    U32 numColumns = 0;

    // Compute the number of columns
    for (int t = 0; t <= Game::MAX_TEAMS; ++t)
    {
      if (teams[t])
      {
        numColumns++;
      }
    }

    ASSERT(numColumns)

    // Add the query groups
    for (NList<QueryGroup>::Iterator g(&queryGroups); *g; ++g)
    {
      // Add title to listbox
      listBox->AddItem("Title", new QueryHeading(TRANSLATE(((*g)->title)), listBox));

      // Add the items in the query set to the list box
      for (NBinTree<QuerySet>::Iterator q(&(*g)->querySets); *q; ++q)
      {
        QueryResults::Column *columns = new QueryResults::Column[numColumns];
        U32 c = 0;
        for (t = 0; t <= Game::MAX_TEAMS; ++t)
        {
          if (teams[t])
          {
            columns[c].value = (*q)->value[t];
            columns[c].color = (t < Game::MAX_TEAMS) ? Team::Id2Team(t)->GetDimColor() : Color(0.0f, 0.0f, 0.0f, 0.0f);
            c++;
          }
        }
        listBox->AddItem("Result", new QueryResults(listBox, (*q)->title, numColumns, columns));
      }
    }
  }


  //
  // FindCreate a query group
  //
  Stats::QueryGroup * Stats::FindCreateGroup(NList<QueryGroup> &queryGroups, const char *name)
  {
    U32 crc = Crc::CalcStr(name);

    QueryGroup *g = NULL;

    for (NList<QueryGroup>::Iterator q(&queryGroups); *q; ++q)
    {
      if (Crc::CalcStr((*q)->title) == crc)
      {
        g = *q;
        break;
      }
    }

    if (!g)
    {
      queryGroups.Append(g = new QueryGroup(name));
    }
    return (g);
  }

}
