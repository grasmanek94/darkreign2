///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// Game-Play Engine
//
// 25-NOV-1998
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "footprint_private.h"
#include "stdload.h"
#include "console.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace FootPrint - Object footprinting system
//
namespace FootPrint
{
  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Type - Footprint data for each type
  //

  //
  // Constructor
  //
  Type::Type(MeshRoot &root, const char *typeName, MapObjType *type) : mapType(type)
  {  
    Point<S32> check;

    // Generate the file name
    Utils::Sprintf(name.str, MAX_FILEIDENT, "%s.fpd", typeName);

    // Load the required lower layer
    if (!layer[LAYER_LOWER].Load("SP-0", root, size))
    {
      ERR_CONFIG(("Unable to load lower shadow layer for footprint '%s'", Name()));
    }

    // Were any verts not aligned
    if (layer[LAYER_LOWER].GetWarnAlignment())
    {
      LOG_DIAG(("Footprint [%s] (%d,%d, SP-0) had vertex alignment errors", typeName, size.x, size.z));
    }

    // Load the optional upper layer
    if (layer[LAYER_UPPER].Load("SP-1", root, check))
    {
      // Ensure correct size
      if (size != check)
      {
        ERR_FATAL
        ((
          "Layers unmatched (%s) : Lower (%d,%d) Upper (%d,%d)", 
          Name(), size.x, size.z, check.x, check.z
        ));
      }

      // Were any verts not aligned
      if (layer[LAYER_UPPER].GetWarnAlignment())
      {
        LOG_DIAG(("Footprint [%s] (%d,%d, SP-1) had vertex alignment errors", typeName, size.x, size.z));
      }
    }

    // Allocate the cell array
    array = new Cell[size.x * size.z];

    // Load the configuration
    if (!Load())
    {
      //LOG_DIAG(("Unable to find configuration for %s", Name()));
    }
  }


  //
  // Destructor
  //
  Type::~Type()
  {
    delete [] array;
  }


  //
  // Load
  //
  // Load the configuration file
  //
  Bool Type::Load()
  {
    FScope *sScope, *fScope, *cellScope;
    PTree tree;

    // Read the file
    if (!tree.AddFile(Name()))
    {
      // Initialize default surface
      U8 impassable = MoveTable::SurfaceIndex("Impassable");

      for (S32 z = 0; z < size.z; z++)
      {
        for (S32 x = 0; x < size.x; x++)
        {
          // Get the footprint cell
          Cell &cell = GetCell(x, z);
          SetFlag(cell, SURFACE, TRUE);
          cell.surface = impassable;

          // Claim all cells
          SetFlag(cell, CLAIMLO, TRUE);
          SetFlag(cell, CLAIMHI, TRUE);
        }
      }
    
      return (FALSE);
    }

    // Get the main scope
    fScope = tree.GetGlobalScope()->GetFunction("ConfigureFootPrint");

    // Get the layer used for zipping
    Layer &layer = GetLayer(LAYER_LOWER);

    // Step through configuration
    while ((sScope = fScope->NextFunction()) != NULL)
    {
      switch (sScope->NameCrc())
      {
        case 0xA55CFF8C: // "SetupCell"
        {
          // Get the cell position
          S32 x = sScope->NextArgInteger();
          S32 z = sScope->NextArgInteger();

          // Step through configuration
          while ((cellScope = sScope->NextFunction()) != NULL)
          {
            // Is this cell within our bounds
            if (x >= 0 && x <= size.x && z >= 0 && z <= size.z)
            {
              // Get the footprint cell, or NULL if outside
              Cell *cell = (x < size.x && z < size.z) ? &GetCell(x, z) : NULL;

              // Get the layer cell
              Layer::Cell &layerCell = layer.GetCell(x, z);

              switch (cellScope->NameCrc())
              {               
                case 0x2769C38A: // "Hide"
                {
                  if (cell)
                  {
                    SetFlag(*cell, HIDE, cellScope->NextArgInteger());
                  }
                  break;
                }

                case 0x0F238F81: // "SetBase"
                {
                  if (cell)
                  {
                    SetFlag(*cell, SETBASE, cellScope->NextArgInteger());
                  }
                  break;
                }

                case 0xD374785A: // "ClaimLo"
                {
                  if (cell)
                  {
                    SetFlag(*cell, CLAIMLO, cellScope->NextArgInteger());
                  }
                  break;
                }

                case 0x8CD61441: // "ClaimHi"
                {
                  if (cell)
                  {
                    SetFlag(*cell, CLAIMHI, cellScope->NextArgInteger());
                  }
                  break;
                }

                case 0x6728DE39: // "Surface"
                {
                  if (cell)
                  {
                    // Get the surface name
                    const char *surfaceName = cellScope->NextArgString();

                    // Find the surface type
                    MoveTable::KeyInfo *info = MoveTable::FindSurfaceInfo(surfaceName);

                    if (info)
                    {
                      SetFlag(*cell, SURFACE, TRUE);
                      cell->surface = info->index;
                    }
                    else
                    {
                      LOG_WARN(("Surface %s not found for footprint %s (%d,%d)", surfaceName, Name(), x, z));
                    }
                  }
                  break;
                }

                case 0x268A0234: // "Second"
                {
                  if (cell)
                  {
                    SetFlag(*cell, SECOND, cellScope->NextArgInteger());
                  }
                  break;
                }

                case 0xFFAD789F: // "BlockLOS"
                {
                  if (cell)
                  {
                    SetFlag(*cell, BLOCKLOS, cellScope->NextArgInteger());
                  }
                  break;
                }

                case 0xCC5B039A: // "Zip"
                  layerCell.SetFlag(Layer::ZIP, cellScope->NextArgInteger());
                  break;

                case 0x0166A5F2: // "Dirs"
                {
                  if (cell)
                  {
                    cell->dirs = (U8)cellScope->NextArgInteger();
                  }
                  break;
                }
              }
            }
          }
          break;
        }
      }
    }

    return (TRUE);
  }


  //
  // Reset
  //
  // Reset default values in all cells
  //
  void Type::Reset()
  {
    // Step over all cells in current footprint
    for (S32 c = 0; c < size.x * size.z; c++)
    {
      array[c].Reset();
    }  
  }


  //
  // GetLayer
  //
  // Get a particular layer
  //
  Layer & Type::GetLayer(LayerIndex index)
  {
    ASSERT(index < LAYER_COUNT);
    return (layer[index]);
  }


  //
  // Export
  //
  // Export this footprint to the given file name
  //
  Bool Type::Export(const char *fileName)
  {
    PTree tree;
    FScope *root, *cellInfo;
  
    // Add the top level scope
    root = tree.GetGlobalScope()->AddFunction("ConfigureFootPrint");

    // Get the layer used for zipping
    Layer &layer = GetLayer(LAYER_LOWER);

    // Save out the grid
    for (S32 z = 0; z <= size.z; z++)
    {
      for (S32 x = 0; x <= size.x; x++)
      {
        // Write out cell info
        cellInfo = root->AddFunction("SetupCell");
        cellInfo->AddArgInteger(x);
        cellInfo->AddArgInteger(z);

        // Is this cell on the footprint
        if (x < size.x && z < size.z)
        {
          Cell &cell = GetCell(x, z);
          StdSave::TypeU32(cellInfo, "Hide", cell.GetFlag(HIDE));
          StdSave::TypeU32(cellInfo, "SetBase", cell.GetFlag(SETBASE));
          StdSave::TypeU32(cellInfo, "Second", cell.GetFlag(SECOND));
          StdSave::TypeU32(cellInfo, "Dirs", cell.dirs);
          StdSave::TypeU32(cellInfo, "ClaimLo", cell.GetFlag(CLAIMLO));
          StdSave::TypeU32(cellInfo, "ClaimHi", cell.GetFlag(CLAIMHI));
          StdSave::TypeU32(cellInfo, "BlockLOS", cell.GetFlag(BLOCKLOS));

          if (cell.GetFlag(SURFACE))
          {
            MoveTable::KeyInfo *info = MoveTable::FindSurfaceInfo(cell.surface);

            if (info)
            {
              StdSave::TypeString(cellInfo, "Surface", info->ident.str);
            }
          }
        }

        Layer::Cell &cell = layer.GetCell(x, z);
        StdSave::TypeU32(cellInfo, "Zip", cell.GetFlag(Layer::ZIP));
      }
    }
  
    // Save out to disk
    return (tree.WriteTreeText(fileName));
  }


  //
  // GetCell
  //
  // Returns the data for the given cell
  //
  Type::Cell & Type::GetCell(S32 x, S32 z)
  {
    ASSERT(x >= 0 && x < size.x);
    ASSERT(z >= 0 && z < size.z);

    return (array[z * size.x + x]);
  }


  //
  // GetVertexPosition
  //
  // Get the position of the given vertex
  //
  void Type::GetVertexPosition(Vector &pos, const Matrix &origin, S32 x, S32 z)
  {
    // Get the world position of the first shadow vertex
    origin.Transform(pos, GetLayer(LAYER_LOWER).GetCell(x, z).pos);
  }


  //
  // Toggle
  // 
  // Toggle a footprint for an existing object
  //
  void Type::Toggle(MapObj *obj, Bool toggle)
  {
    ASSERT(obj);

    // Does this object already have a footprint instance
    if (obj->GetFootInstance())
    {
      // Start a modification session
      TerrainData::SessionStart();

      // Delete any existing instance
      DeleteInstance(obj);

      // End the session
      TerrainData::SessionEnd();
    }

    // Are we placing the footprint
    if (toggle)
    {
      // Create a placement helper
      Placement place(this);

      // Copy the world matrix of the object
      Matrix m = obj->WorldMatrix();

      // Adjust the position
      place.AdjustLocation(m);
         
      // Is this object allowed to zip at this location
      Placement::Result r = place.Check(m);

      if (FootPrint::Placement::Acceptable(r))
      {
        // Move the object
        obj->SetSimCurrent(m);

        // Start a modification session
        TerrainData::SessionStart();

        // Zip the terrain instantly
        place.ThumpTerrain(obj->Position().y, 0.0F);

        // Do instance managment
        CreateInstance(obj, place);

        // End the session
        TerrainData::SessionEnd();
      }
      else
      {
        LOG_DIAG(("Result not acceptable for placement (%d/%d)", toggle, r));

        // Write informational message to console

        // "Mission::LoadError"
        CONSOLE(0xA1D5DDD2, ("Object %s (id:%d) was unable to zip", obj->TypeName(), obj->Id()))

        const char *reason = NULL;
        switch (r)
        {
          case Placement::PR_OFFMAP:   reason = "Object off map"; break;
          case Placement::PR_CLAIM:    reason = "Cell is currently claimed"; break;
          case Placement::PR_FOOTON:   reason = "Another footprint on footprint area"; break;
          case Placement::PR_FOOTOFF:  reason = "Another footprint on fringe area"; break;
        }

        if (reason)
        {
          // "Mission::LoadError"
          CONSOLE(0xA1D5DDD2, ("  Reason: %s", reason));
        }
      }
    }
  }
}
