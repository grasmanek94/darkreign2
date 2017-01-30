///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Team Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_teameditor.h"
#include "studio_regionlist.h"
#include "iface_util.h"
#include "ifvar.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "gameobjctrl.h"
#include "mapobj.h"

#include "objective.h"
#include "sides.h"
#include "team.h"
#include "regionobj.h"
#include "icdroplist.h"
#include "mods.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Relation
  //
  class Relation : public ICStatic
  {
    PROMOTE_LINK(Relation, ICStatic, 0xB8631CCB); // "Relation"

  public:

    // Team who has a relationship
    Team *who;

    // Team whom the relationship is with
    Team *with;

  public:

    // Constructor and Destructor
    Relation(Team *who, Team *with, IControl *parent);
    ~Relation();

  protected:

    // Redraw self
    void DrawSelf(PaintInfo &pi);

  };

  
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class TeamEditor
  //

  //
  // Constructor
  //
  TeamEditor::TeamEditor(IControl *parent) : 
    ICWindow(parent)
  {
    team = NULL;

    teamColor = new IFaceVar(this, CreateInteger("teamColor", Color(0.0f, 0.0f, 0.0f)));
    resourceStore = new IFaceVar(this, CreateInteger("resourceStore", 0));
    startRegion = new IFaceVar(this, CreateString("startRegion", ""));
    startYaw = new IFaceVar(this, CreateFloat("startYaw", 0.0F));
    defaultAI = new IFaceVar(this, CreateString("defaultAI", ""));
    filteringEnabled = new IFaceVar(this, CreateInteger("filteringEnabled", 0, 0, 1));
    defaultClient = new IFaceVar(this, CreateInteger("defaultClient", 0, 0, 1));
    availablePlay = new IFaceVar(this, CreateInteger("availablePlay", 0, 0, 1));
    hasStats = new IFaceVar(this, CreateInteger("hasStats", 0, 0, 1));
    permanentRadar = new IFaceVar(this, CreateInteger("permanentRadar", 0, 0, 1));
    side = new IFaceVar(this, CreateString("side", ""));
    sideFixed = new IFaceVar(this, CreateInteger("sideFixed", 0, 0, 1));
    requireAI = new IFaceVar(this, CreateInteger("requireAI", 0, 0, 1));
  }


  //
  // Destructor
  //
  TeamEditor::~TeamEditor()
  {
    delete teamColor;
    delete resourceStore;
    delete startRegion;
    delete startYaw;
    delete defaultAI;
    delete filteringEnabled;
    delete defaultClient;
    delete availablePlay;
    delete hasStats;
    delete permanentRadar;
    delete side;
    delete sideFixed;
    delete requireAI;
  }


  //
  // Setup
  //
  // Setup this control from one scope function
  //
  void TeamEditor::Setup(FScope *fScope)
  {
    // Pass on down
    ICWindow::Setup(fScope);
  }


  //
  // PostConfigure
  //
  // Checks that control was setup
  //
  void TeamEditor::PostConfigure()
  {
    ICWindow::PostConfigure();
  }


  //
  // Notify
  //
  // Notification that a local var has changed value
  //
  void TeamEditor::Notify(IFaceVar *var)
  {
    var;
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 TeamEditor::HandleEvent(Event &e)
  {
    if (e.type == IFace::EventID())
    {
      switch (e.subType)
      {
        case IFace::NOTIFY:
        {
          // Do specific handling
          switch (e.iface.p1)
          {
            case TeamEditorMsg::Download:
              Download();
              break;

            case TeamEditorMsg::PickStartRegion:
            {
              ICWindow *regionWindow = IFace::Find<ICWindow>("RegionPicker");
              RegionListBox *regionList = IFace::Find<RegionListBox>("RegionList", regionWindow);

              if (regionList)
              {
                regionList->SetPickRegion(startRegion);

                // Activate it
                IFace::Activate(regionWindow);
              }
              break;
            }

            case TeamEditorMsg::TestStartRegion:
            {
              team->MoveToStart();
              break;
            }

            case TeamEditorMsg::FilterPropertyClear:
            case TeamEditorMsg::FilterPropertyChange:
            {
              if (!filteringPropertyList.Alive())
              {
                break;
              }

              // Was it all items which changed
              if (!e.iface.p2)
              {
                if (e.iface.p3)
                {
                  // Select ALL items
                  for (U32 i = 1; i <= filteringTypeList->ItemCount(); i++)
                  {
                    filteringTypeList->SetSelectedItem(i, TRUE, FALSE);
                  }
                }
                else
                {
                  // Clear selection
                  filteringTypeList->ClearSelected(FALSE);
                }
              }
              else
              {
                // Check all of the items to see if they have the property specified
                for (NList<IControl>::Iterator c(&filteringTypeList->GetItems()); *c; c++)
                {
                  UnitObjType *type = GameObjCtrl::FindType<UnitObjType>((*c)->NameCrc());
                  ASSERT(type)

                  if (type->HasProperty(e.iface.p2))
                  {
                    // Do we want to set it or clear it ?
                    if (e.iface.p3)
                    {
                      // Force its selectiong
                      filteringTypeList->SetSelectedItem(*c, TRUE, FALSE);
                    }
                    else
                    {
                      // If its already selected then toggle selection
                      if ((*c)->GetControlState() & IControl::STATE_SELECTED)
                      {
                        filteringTypeList->SetSelectedItem(*c, FALSE, FALSE);
                      }
                    }
                  }
                }
              }
            }

            // Intentional Fallthrough

            case TeamEditorMsg::FilterTypeClear:
            case TeamEditorMsg::FilterTypeChange:
            {
              if (!filteringTypeList.Alive())
              {
                break;
              }

              // Type selection has changed, recompute the selected properties

              // Build the selected type list
              UnitObjTypeList types;
              BuildFilteringTypeList(types, FALSE);

              // We now have the list of types which are not selected

              // Clear the selected properties
              filteringPropertyList->ClearSelected(FALSE);

              // Enumerate the properties and see if each is successful
              for (NList<IControl>::Iterator c(&filteringPropertyList->GetItems()); *c; c++)
              {
                // Special case is the empty id for all items
                if (*(*c)->Name() == '\0')
                {
                  if (!types.GetCount())
                  {
                    filteringPropertyList->SetSelectedItem(*c, TRUE, FALSE);
                  }
                }
                else
                {
                  // Check to see is anything in the unselected list has this property
                  U32 id = (*c)->NameCrc();
                  Bool failed = FALSE;

                  for (UnitObjTypeList::Iterator t(&types); *t; t++)
                  {
                    if ((**t)->HasProperty(id))
                    {
                      failed = TRUE;
                      break;
                    }
                  }

                  if (!failed)
                  {
                    filteringPropertyList->SetSelectedItem(*c, TRUE, FALSE);
                  }
                }
              }

              types.Clear();

              break;
            }

            default : 
              ICWindow::HandleEvent(e);
              break;
          }

          return (TRUE);
        }
      }
    }

    return (ICWindow::HandleEvent(e));  
  }


  //
  // DrawSelf
  //
  // Control draws itself
  //
  void TeamEditor::DrawSelf(PaintInfo &pi)
  {
    ICWindow::DrawSelf(pi);
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool TeamEditor::Activate()
  {
    if (ICWindow::Activate())
    {
      ASSERT(team)

      ActivateVar(teamColor);
      ActivateVar(resourceStore);
      ActivateVar(startRegion);
      ActivateVar(startYaw);
      ActivateVar(defaultAI);
      ActivateVar(filteringEnabled);
      ActivateVar(defaultClient);
      ActivateVar(availablePlay);
      ActivateVar(hasStats);
      ActivateVar(permanentRadar);
      ActivateVar(side);
      ActivateVar(sideFixed);
      ActivateVar(requireAI);

      // Find all the important controls
      teamName = IFace::Find<ICStatic>("TeamName", this);

      defaultAIList = IFace::Find<ICDropList>("DefaultAIList", this, TRUE);
      filteringPropertyList = IFace::Find<ICListBox>("FilteringPropertyList", this, TRUE);
      filteringTypeList = IFace::Find<ICListBox>("FilteringTypeList", this, TRUE);
      objectivesList = IFace::Find<ICListBox>("ObjectivesList", this, TRUE);
      sideList = IFace::Find<ICDropList>("SideList", this, TRUE);

      // Upload to the control
      Upload();

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool TeamEditor::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      teamColor->Deactivate();
      resourceStore->Deactivate();
      startRegion->Deactivate();
      startYaw->Deactivate();
      defaultAI->Deactivate();
      filteringEnabled->Deactivate();
      defaultClient->Deactivate();
      availablePlay->Deactivate();
      hasStats->Deactivate();
      permanentRadar->Deactivate();
      sideFixed->Deactivate();
      requireAI->Deactivate();

      //team = NULL;

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // SetTeam
  //
  // Set the team to use
  //
  void TeamEditor::SetTeam(Team *t)
  {
    ASSERT(t)
    team = t;
  }


  //
  // Upload
  //
  void TeamEditor::Upload()
  {
    ASSERT(teamName.Alive())
    ASSERT(filteringPropertyList.Alive())
    ASSERT(filteringTypeList.Alive())
    ASSERT(objectivesList.Alive())
    ASSERT(sideList.Alive())

    // Team color
    teamColor->SetIntegerValue(team->GetColor());

    // Team resource
    resourceStore->SetIntegerValue(team->GetResourceStore());

    // Team name
    teamName->SetTextString(TRANSLATE((team->GetName())), TRUE);

    // Upload toggles
    defaultClient->SetIntegerValue(team->IsDefaultClient());
    availablePlay->SetIntegerValue(team->IsAvailablePlay());
    hasStats->SetIntegerValue(team->HasStats());
    permanentRadar->SetIntegerValue(team->GetPermanentRadar());
    sideFixed->SetIntegerValue(team->GetSideFixed());
    requireAI->SetIntegerValue(team->GetRequireAI());

    // Start Region
    RegionObj *region;
    region = team->GetStartRegion();

    if (region)
    {
      startRegion->SetStringValue(region->RegionName());
    }
    else
    {
      startRegion->SetStringValue("");
    }

    // Start angle
    startYaw->SetFloatValue(team->GetStartYaw());

    // Upload all of the available personalities
    ICListBox *listBox = defaultAIList->GetListBox();
    listBox->DeleteAllItems();

    listBox->AddTextItem("None", NULL);
    const NBinTree<Mods::Mod> *mods = Mods::GetMods(Mods::Types::Personality);
    if (mods)
    {
      for (NBinTree<Mods::Mod>::Iterator mod(mods); *mod; ++mod)
      {
        listBox->AddTextItem((*mod)->GetName().str, NULL);
      }
    }

    // Personality
    defaultAI->SetStringValue(team->GetPersonality());

    // Upload the enabled flag
    filteringEnabled->SetIntegerValue(team->GetTypeFiltering());

    // Select all of the items in this list which the team wants
    filteringTypeList->ClearSelected();
    for (UnitObjTypeList::Iterator type(&team->GetFilteredTypes()); *type; type++)
    {
      if ((*type)->Alive())
      {
        filteringTypeList->SetSelectedItem((**type)->GetName(), TRUE);
      }
    }

    // Add all the objectives to the list box
    objectivesList->DeleteAllItems();
    for (NBinTree<Objective::Type>::Iterator o(&Objective::GetTypes()); *o; o++)
    {
      objectivesList->AddTextItem((*o)->GetName(), NULL);
    }

    // Select all of the current objectives of this team
    for (Objective::ObjList::Iterator co(&team->GetCurrentObjectives()); *co; co++)
    {
      if ((*co)->Alive())
      {
        objectivesList->SetSelectedItem((**co)->GetType()->GetName());
      }
    }

    // Upload all of the available sides
    listBox = sideList->GetListBox();
    listBox->DeleteAllItems();
    for (NBinTree<Sides::Side>::Iterator sides(&Sides::GetSides()); *sides; sides++)
    {
      listBox->AddTextItem((*sides)->GetName().str, TRANSLATE(((*sides)->GetDescription().str)));
    }

    // Set the current side
    sideList->SetSelected(team->GetSide());
  }

  
  //
  // Download
  //
  void TeamEditor::Download()
  {
    ASSERT(filteringPropertyList.Alive())
    ASSERT(filteringTypeList.Alive())
    ASSERT(objectivesList.Alive())

    // Download the color
    team->SetColor(teamColor->GetIntegerValue());

    // Download toggles
    team->SetDefaultClient(defaultClient->GetIntegerValue());
    team->SetAvailablePlay(availablePlay->GetIntegerValue());
    team->SetHasStats(hasStats->GetIntegerValue());
    team->SetPermanentRadar(permanentRadar->GetIntegerValue());
    team->SetSideFixed(sideFixed->GetIntegerValue());
    team->SetRequireAI(requireAI->GetIntegerValue());

    // Download resource
    team->SetResourceStore(resourceStore->GetIntegerValue());

    // Download start region
    team->SetStartRegion(RegionObj::FindRegion(startRegion->GetStringValue()));

    // Download start angle
    team->SetStartYaw(startYaw->GetFloatValue());

    // Download the default personality
    team->SetPersonality(defaultAI->GetStringValue());

    // Download the filterering enabled flag
    team->SetTypeFiltering(filteringEnabled->GetIntegerValue());

    // Download the side name
    team->SetSide(side->GetStringValue());

    // Download the selected list
    UnitObjTypeList types;
    BuildFilteringTypeList(types);
    team->SetFilteredTypes(types);
    types.Clear();

    // Clear the teams objectives
    team->ClearObjectives();

    // Add all the objectives from the list box
    for (ICListBox::SelectedList::Iterator obj(&objectivesList->GetSelectedList()); *obj; obj++)
    {
      team->NewObjective((**obj)->Name());
    }
  }


  //
  // Build the list of selected filter types
  //
  void TeamEditor::BuildFilteringTypeList(UnitObjTypeList &types, Bool selected)
  {
    ASSERT(filteringTypeList.Alive())
    if (selected)
    {
      filteringTypeList->EnumSelected(GetFilteringTypesCallback, &types);
    }
    else
    {
      filteringTypeList->EnumNonSelected(GetFilteringTypesCallback, &types);
    }
  }


  //
  // Callback for building the selected filter types
  //
  void TeamEditor::GetFilteringTypesCallback(const char *key, const CH *, void *context)
  {
    UnitObjTypeList *types = (UnitObjTypeList *) context;

    // Using the key resolve the type
    UnitObjType *type = GameObjCtrl::FindType<UnitObjType>(key);

    if (type)
    {
      types->Append(type);
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Relation
  //


  //
  // Relation::Relation
  //
  Relation::Relation(Team *who, Team *with, IControl *parent)
  : ICStatic(parent),
    who(who),
    with(with)
  {
  }


  //
  // Relation::~Relation
  //
  Relation::~Relation()
  {
  }


  //
  // Relation::DrawSelf
  //
  void Relation::DrawSelf(PaintInfo &pi)
  {
    DrawCtrlBackground(pi, GetTexture());
    DrawCtrlFrame(pi);
  }

}
