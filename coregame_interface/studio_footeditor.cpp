///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// FootPrint Editor
//
// 24-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "studio_footeditor.h"
#include "icgrid.h"
#include "iface_util.h"
#include "ifvar.h"
#include "icslider.h"
#include "iface_types.h"
#include "babel.h"
#include "iface_messagebox.h"
#include "bitmap.h"
#include "gameobjctrl.h"
#include "mapobj.h"
#include "console.h"
#include "iface_priv.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Studio
//
namespace Studio
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class FootEditor
  //

  //
  // Constructor
  //
  FootEditor::FootEditor(IControl *parent) : ICWindow(parent),
    mode(EM_HIDE),
    defaultPath("."), 
    gridSize(224, 224), 
    gridOffset(5, 5),
    type(NULL),
    foot(NULL), 
    grid(NULL), 
    varType(NULL),
    reflectVarChanges(TRUE)
  {
    // Create interface vars
    varSurface = new IFaceVar(this, CreateString("surface", ""));
    varMode = new IFaceVar(this, CreateString("mode", "Hide"));
    varDirNorth = new IFaceVar(this, CreateInteger("DirNorth", 0, 0, 1));
    varDirEast = new IFaceVar(this, CreateInteger("DirEast", 0, 0, 1));
    varDirSouth = new IFaceVar(this, CreateInteger("DirSouth", 0, 0, 1));
    varDirWest = new IFaceVar(this, CreateInteger("DirWest", 0, 0, 1));
  }


  //
  // Destructor
  //
  FootEditor::~FootEditor()
  {
    if (varType)
    {
      delete varType;
    }

    delete varSurface;
    delete varMode;
    delete varDirNorth;
    delete varDirEast;
    delete varDirSouth;
    delete varDirWest;
  }


  //
  // Setup
  //
  // Setup this control from one scope function
  //
  void FootEditor::Setup(FScope *fScope)
  {
    switch (fScope->NameCrc())
    {
      case 0x22E56232: // "TypeVar"
        ConfigureVar(varType, fScope);
        break;

      case 0x2BA8A025: // "EditGridSize"
        gridSize.x = fScope->NextArgInteger();
        gridSize.y = fScope->NextArgInteger();
        break;

      case 0x87DBB97C: // "EditGridOffset"
        gridOffset.x = fScope->NextArgInteger();
        gridOffset.y = fScope->NextArgInteger();
        break;

      case 0xC1AFE857: // "EditPath"
        defaultPath = fScope->NextArgString();
        break;

      default:
        ICWindow::Setup(fScope);
        break;
    }
  }


  
  //
  // ModifySelectedCell
  //
  // Modify selected cell based on current values
  //
  void FootEditor::ModifySelectedCell()
  {
    // Has a grid been created
    if (grid)
    {     
      FootPrint::Type::Cell *typeCell;
      FootPrint::Layer::Cell *layerCell;

      const Point<U32> &p = grid->GetSelected();

      // Should we process this cell
      if (ValidCell(p.x, p.z, typeCell, layerCell))
      {
        switch (mode)
        {
          case EM_SURFACE:
          {
            ASSERT(typeCell)

            // Get the current surface info
            MoveTable::KeyInfo *info = MoveTable::FindSurfaceInfo(varSurface->GetStringValue());

            if (info)
            {
              typeCell->SetFlag(FootPrint::Type::SURFACE, TRUE);
              typeCell->surface = info->index;
            }
            else
            {
              typeCell->SetFlag(FootPrint::Type::SURFACE, FALSE);
            }
            break;
          }

          case EM_DIRECTIONS:
          {
            ASSERT(typeCell)

            typeCell->dirs = U8
            (
              (varDirNorth->GetIntegerValue()) |
              (varDirEast->GetIntegerValue() << 1) | 
              (varDirSouth->GetIntegerValue() << 2) | 
              (varDirWest->GetIntegerValue() << 3)
            );
            break;
          }
        }        

        // Update info
        UpdateSelectedInfo();
      }
    }
  }


  //
  // UpdateSelectedInfo
  //
  // Update the information for the selected cell
  //
  void FootEditor::UpdateSelectedInfo()
  { 
    // Don't update cell based on var changes
    reflectVarChanges = FALSE;

    // Has a grid been created
    if (grid)
    {     
      FootPrint::Type::Cell *typeCell;
      FootPrint::Layer::Cell *layerCell;

      const Point<U32> &p = grid->GetSelected();

      // Should we process this cell
      if (ValidCell(p.x, p.z, typeCell, layerCell))
      {
        switch (mode)
        {
          case EM_SURFACE:
          {
            MoveTable::KeyInfo *info = NULL;

            if (typeCell->GetFlag(FootPrint::Type::SURFACE))
            {
              info = MoveTable::FindSurfaceInfo(typeCell->surface);
            }         
            
            // Set or clear var
            varSurface->SetStringValue(info ? info->ident.str : "[No Surface]");
            break;
          }

          case EM_DIRECTIONS:
          {
            varDirNorth->SetIntegerValue(typeCell->dirs & 1);
            varDirEast->SetIntegerValue(typeCell->dirs & 2);
            varDirSouth->SetIntegerValue(typeCell->dirs & 4);
            varDirWest->SetIntegerValue(typeCell->dirs & 8);
            break;
          }
        }        
      }
    }

    // Revert to default
    reflectVarChanges = TRUE;
  }


  //
  // Notify
  //
  // Notification that a local var has changed value
  //
  void FootEditor::Notify(IFaceVar *var)
  {
    // Has the mode been changed
    if (var == varMode)
    {
      Bool flag = FALSE;

      switch (Crc::CalcStr(varMode->GetStringValue()))
      {
        case 0x2769C38A: // "Hide"
          mode = EM_HIDE;
          break;

        case 0x0F238F81: // "SetBase"
          mode = EM_SETBASE;
          break;

        case 0xD374785A: // "ClaimLo"
          mode = EM_CLAIMLO;
          break;

        case 0x8CD61441: // "ClaimHi"
          mode = EM_CLAIMHI;
          break;

        case 0x6728DE39: // "Surface"
          mode = EM_SURFACE;
          flag = TRUE;
          break;

        case 0x268A0234: // "Second"
          mode = EM_SECOND;
          break;

        case 0xFFAD789F: // "BlockLOS"
          mode = EM_BLOCKLOS;
          break;

        case 0xCC5B039A: // "Zip"
          mode = EM_ZIP;
          break;

        case 0xF35F8DA9: // "Directions"
          mode = EM_DIRECTIONS;
          flag = TRUE;
          break;
      }

      // Update info based on new mode
      UpdateSelectedInfo();

      // Change the display of the selected cell
      if (type)
      {
        ASSERT(grid)

        grid->SetDisplaySelected(flag);
      }
    }
    else
    
    // Type selection has changed
    if (var == varType)
    {
      EditType();
    }

    // Direction has changed
    if 
    (
      reflectVarChanges &&
      (
        var == varDirNorth || 
        var == varDirEast  || 
        var == varDirSouth || 
        var == varDirWest  ||
        var == varSurface
      )
    )
    {
      ModifySelectedCell();
    }
  }


  //
  // ValidCell
  //
  // Is the given cell valid for the current mode
  //
  Bool FootEditor::ValidCell
  (
    S32 x, S32 z, FootPrint::Type::Cell * &typeCell, FootPrint::Layer::Cell * &layerCell
  )
  {
    switch (mode)
    {
      case EM_HIDE:
      case EM_SETBASE:
      case EM_CLAIMLO:
      case EM_CLAIMHI:
      case EM_SURFACE:
      case EM_SECOND:
      case EM_DIRECTIONS:
      case EM_BLOCKLOS:
      {
        // Is this position on the foot
        if (foot && x > 0 && z > 0 && x <= foot->Size().x && z <= foot->Size().z)
        {
          // Setup the type cell
          typeCell = &foot->GetCell(x - 1, z - 1);
          return (TRUE);
        }
        break;
      }

      case EM_ZIP:
      {
        // Is this a vertex position
        if (foot && x > 0 && z > 0)
        {
          layerCell = &foot->GetLayer(FootPrint::Type::LAYER_LOWER).GetCell(x - 1, z - 1);
          return (TRUE);
        }
        break;
      }
    }

    return (FALSE);
  }


  //
  // ClearData
  //
  // Clear the current data
  //
  void FootEditor::ClearData()
  {
    // Do we already have a type set
    if (type)
    {
      ASSERT(foot && grid);

      // Clean up previous data
      grid->MarkForDeletion();
      foot = NULL;
      grid = NULL;
      type = NULL;
    }  
  }

  
  //
  // EditType
  //
  // Edit the currently selected type
  //
  void FootEditor::EditType()
  {
    // Clear current data
    ClearData();

    // Find the new type
    if ((type = GameObjCtrl::FindType<MapObjType>(varType->GetStringValue())) != NULL)
    {
      // Get the footprint type (required)
      if ((foot = type->GetFootPrintType()) == NULL)
      {
        type = NULL;
        return;
      }

      // Calculate the sizes
      U32 xs = foot->Size().x + 2;
      U32 zs = foot->Size().z + 2;
      U32 c = Max<U32>(1, Min(gridSize.x / xs, gridSize.y / zs));

      // Create the new foot grid
      grid = new ICGrid("FootGrid", xs, zs, c, c, this);

      // Now set it up
      grid->SetGeometry("Bottom", NULL);
      grid->SetPos(gridOffset.x, -gridOffset.y);
      grid->SetCellCallBack(CellCallBack);
      grid->SetEventCallBack(EventCallBack);
      grid->SetContext(this);
      grid->SetAxisFlip(FALSE, TRUE);

      if (IsActive())
      {
        grid->Activate();
      }

      // Update mode to update selected cell display
      Notify(varMode);
    }
  }


  //
  // Export
  //
  // Export the current footprint data
  //
  void FootEditor::Export()
  {
    ASSERT(foot && grid);

    // Point to the current default path
    const char *path = defaultPath.str;

    // Does this file exist in the file system
    FileSys::DataFile *file = FileSys::Open(foot->Name());

    // Use its location as the output path
    if (file)
    {
      path = file->Path();
      FileSys::Close(file);
    }

    // Generate the full file name
    PathString outName;
    Utils::Sprintf(outName.str, outName.GetSize(), "%s\\%s", path, foot->Name());

    // Export the footprint
    if (foot->Export(outName.str))
    {
      IFace::MsgBox
      (
        TRANSLATE(("Success")), 
        Utils::Ansi2Unicode(outName.str), 0, 
        new MBEvent("Cool", TRANSLATE(("Cool")))
      );
      
      // Update the file system records
      FileSys::BuildIndexes();
    }
    else
    {
      IFace::MsgBox
      (
        TRANSLATE(("Error")), 
        Utils::Ansi2Unicode(outName.str), 0, 
        new MBEvent("Woops", TRANSLATE(("Woops")))
      );
    }
  }


  //
  // CellCallBack
  //
  // Returns a single cell color based on the current footprint
  //
  Color FootEditor::CellCallBack(void *context, U32 x, U32 z)
  {
    ASSERT(context);

    // Setup default color values
    F32 r = 0.1F, g = 0.0F, b = 0.0F;

    // Apply a tiling effect to each cell
    F32 t = ((x & 1) ? 0.04F : 0.0F) + ((z & 1) ? 0.04F : 0.0F);

    // Context contains a pointer to the foot editor
    FootEditor *editor = (FootEditor*) context;

    FootPrint::Type::Cell *typeCell;
    FootPrint::Layer::Cell *layerCell;

    // Is this cell valid for the current mode
    if (editor->ValidCell(x, z, typeCell, layerCell))
    {
      switch (editor->mode)
      {
        case EM_HIDE:
          g = typeCell->GetFlag(FootPrint::Type::HIDE) ? 0.0F : 0.6F;
          break;

        case EM_SETBASE:
          g = typeCell->GetFlag(FootPrint::Type::SETBASE) ? 0.6F : 0.0F;
          break;

        case EM_CLAIMLO:
          g = typeCell->GetFlag(FootPrint::Type::CLAIMLO) ? 0.6F : 0.0F;
          break;

        case EM_CLAIMHI:
          g = typeCell->GetFlag(FootPrint::Type::CLAIMHI) ? 0.6F : 0.0F;
          break;

        case EM_SURFACE:
          if (MoveTable::SurfaceCount() && typeCell->GetFlag(FootPrint::Type::SURFACE))
          {
            b = 0.6F * (F32(typeCell->surface + 1) / F32(MoveTable::SurfaceCount()));
          }
          else
          {
            b = 0.0F;
          }
          break;

        case EM_SECOND:
          b = typeCell->GetFlag(FootPrint::Type::SECOND) ? 0.6F : 0.0F;
          break;

        case EM_BLOCKLOS:
          b = typeCell->GetFlag(FootPrint::Type::BLOCKLOS) ? 0.6F : 0.0F;
          break;

        case EM_ZIP:
          g = layerCell->GetFlag(FootPrint::Layer::ZIP) ? 0.6F : 0.0F;
          break;

        case EM_DIRECTIONS:
          b = typeCell->dirs * (1.0F / 15.0F);
          break;
      }
    }
    else
    {
      r = t = 0.0F;
    }

    return (Color(r + t, g + t, b + t));
  }


  //
  // EventCallBack
  //
  // Handles events from foot grids
  //
  void FootEditor::EventCallBack(void *context, U32 x, U32 z, U32 event)
  {
    ASSERT(context);

    // Context contains a pointer to the foot editor
    FootEditor *editor = (FootEditor*) context;
    FootPrint::Type::Cell *typeCell;
    FootPrint::Layer::Cell *layerCell;

    // Should we process this cell
    if (editor->ValidCell(x, z, typeCell, layerCell))
    {
      switch (event)
      {
        case 0x90E4DA5D: // "LeftClick"
        {
          // Selection has changed
          editor->UpdateSelectedInfo();

          switch (editor->mode)
          {
            case EM_HIDE:
              typeCell->SetFlag(FootPrint::Type::HIDE, !typeCell->GetFlag(FootPrint::Type::HIDE));
              break;

            case EM_SETBASE:
              typeCell->SetFlag(FootPrint::Type::SETBASE, !typeCell->GetFlag(FootPrint::Type::SETBASE));
              break;

            case EM_CLAIMLO:
              typeCell->SetFlag(FootPrint::Type::CLAIMLO, !typeCell->GetFlag(FootPrint::Type::CLAIMLO));
              break;

            case EM_CLAIMHI:
              typeCell->SetFlag(FootPrint::Type::CLAIMHI, !typeCell->GetFlag(FootPrint::Type::CLAIMHI));
              break;

            case EM_SECOND:
              typeCell->SetFlag(FootPrint::Type::SECOND, !typeCell->GetFlag(FootPrint::Type::SECOND));
              break;

            case EM_BLOCKLOS:
              typeCell->SetFlag(FootPrint::Type::BLOCKLOS, !typeCell->GetFlag(FootPrint::Type::BLOCKLOS));
              break;

            case EM_ZIP:
              layerCell->SetFlag(FootPrint::Layer::ZIP, !layerCell->GetFlag(FootPrint::Layer::ZIP));
              break;
          }
          break;
        }

        // Apply current settings
        case 0x173F5F78: // "RightClick"
          editor->ModifySelectedCell();
          break;
      }
    }
  }


  //
  // HandleEvent
  //
  // Pass any events to the registered handler
  //
  U32 FootEditor::HandleEvent(Event &e)
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
            case 0x65B6B742: // "Export"
              if (type) { Export(); }
              break;

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
  void FootEditor::DrawSelf(PaintInfo &pi)
  {
    ICWindow::DrawSelf(pi);

    PaintInfo p = pi;
    p.client.p0.x += gridOffset.x;
    p.client.p0.y += pi.client.Height() - gridOffset.y - gridSize.y;
    p.client.p1.x = p.client.p0.x + gridSize.x;
    p.client.p1.y = p.client.p0.y + gridSize.y;

    // Paint a background for the variable size grid
    IFace::RenderRectangle(p.client, Color(0.1F, 0.1F, 0.2F, IFace::data.alphaScale));
  }


  //
  // Activate
  //
  // Activate this control
  //
  Bool FootEditor::Activate()
  {
    if (ICWindow::Activate())
    {
      ActivateVar(varType);
      ActivateVar(varSurface);
      ActivateVar(varMode);
      ActivateVar(varDirNorth);
      ActivateVar(varDirEast);
      ActivateVar(varDirSouth);
      ActivateVar(varDirWest);

      // Fill the surface list
      ICListBox *listBox = IFace::Find<ICListBox>("ModeConfig.Surface.SurfaceList", this);

      if (listBox)
      {
        // Clear the list
        listBox->DeleteAllItems();

        // Get the movement table
        MoveTable::MoveBalanceTable &table = MoveTable::GetTable();

        // Use this item to apply no surface change
        listBox->AddTextItem("[No Surface]", NULL);

        // Iterate the surface tree
        for (NBinTree<MoveTable::MoveBalanceTable::KeyInfo>::Iterator s(&table.GetXTree()); *s; s++)
        {
          listBox->AddTextItem((*s)->ident.str, NULL);
        }              

        // Sort the list
        listBox->Sort(TRUE);
        listBox->SetSelectedItem(1);
      }

      return (TRUE);
    }

    return (FALSE);
  }


  //
  // Deactivate
  //
  // Deactivate this control
  //
  Bool FootEditor::Deactivate()
  {
    if (ICWindow::Deactivate())
    {
      varType->Deactivate();
      varSurface->Deactivate();
      varMode->Deactivate();
      varDirNorth->Deactivate();
      varDirEast->Deactivate();
      varDirSouth->Deactivate();
      varDirWest->Deactivate();
      return (TRUE);
    }

    return (FALSE);
  }
}
