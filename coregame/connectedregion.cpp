///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Connected Region
//

///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "connectedregion.h"
#include "blobs.h"
#include "movetable.h"
#include "icgridwindow.h"
#include "console.h"

#include "iface.h"
#include "iclistbox.h"


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace ConnectedRegion
//
namespace ConnectedRegion
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Constants
  //
  const U32 MaxRLERun = 65535;

  
  #pragma pack(push, 1)

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct RLCitem
  //
  struct RLCitem
  {
    Pixel value;
    U16 length;

    RLCitem()
    : value(0), 
      length(0)
    {
    }

    RLCitem(Pixel value, U16 length)
    : value(value),
      length(length)
    {
    }

  };

  #pragma pack(pop)


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct RLCrow
  //
  // This data structure holds a run length compressed row of a map
  //
  struct RLCrow
  {
    // How big is this row?
    U32 length;

    // Items in the row
    RLCitem *data;     
    
    // Constructor and Destructor
    RLCrow();
    ~RLCrow();

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RLCmap
  //
  // This data structure holds a run length compressed map
  //
  struct RLCmap
  {
    // How many rows in map?
    U32 rows;

    // The rows of the map
    RLCrow *data;

    // Number of regions
    U32 numRegions;

    // Region information
    Region *regions;

    // Constructor and Destructor
    RLCmap(U32 rows);
    ~RLCmap();

    // Compress a map into RLE form
    void Compress(Blobs::Image &image, U32 rows, U32 columns);

    // Get the value of a pixel in the image
    Pixel GetValue(U32 row, U32 column);

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal Data
  //
  static RLCmap *map[MoveTable::MAX_TRACTION_TYPES];
  static Bool initialized = FALSE;
  static ICGridWindowPtr grid;
  static U8 currentTraction;

  static VarInteger varRegion;
  static VarInteger varArea;
  static VarInteger varMinColumn;
  static VarInteger varMaxColumn;
  static VarInteger varMaxRow;
  static VarInteger varMinRow;
  static VarFloat varFill;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static void Generate();
  static void Delete();
  static void CmdHandler(U32 pathCrc);
  static Color CellCallBack(void *, U32 x, U32 z);
  static void EventCallBack(void *, U32 x, U32 y, U32 event);



  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RLCrow
  //

  //
  // Constructor
  //
  RLCrow::RLCrow()
  : length(0),
    data(NULL)
  {
  }


  //
  // Destructor
  //
  RLCrow::~RLCrow()
  {
    if (data)
    {
      delete [] data;
      data = NULL;
    }
  }


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class RLCmap
  //


  //
  // Constructor
  //
  RLCmap::RLCmap(U32 rows)
  : rows(rows),
    data(new RLCrow[rows]),
    numRegions(0),
    regions(NULL)
  {
  }


  //
  // Destructor
  //
  RLCmap::~RLCmap()
  {
    delete [] data;
    data = NULL;

    if (regions)
    {
      delete [] regions;
      regions = NULL;
    }
  }


  //
  // Compress
  //
  void RLCmap::Compress(Blobs::Image &image, U32 rows, U32 columns)
  {
    // Temporary storage for the current row
    RLCitem *currentRow = new RLCitem[columns];

    // Go through each row of uncompressed image and compress it
    for (U32 r = 0; r < rows; r++)
    {
      RLCitem run(0, 0);
      U32 rowLength = 0;
      U32 currentValue = 0;

      // Go through each pixel and build runs
      for (U32 c = 0; c < columns; c++)
      {
        // Find the next pixel
        currentValue = image(c, r);

        // See if this is the start of the first run
        if (!run.length)
        {
          // Set the new run's value
          ASSERT(currentValue < U16_MAX)
          run.value = U16(currentValue);

          // Say we have one pixel
          run.length = 1;
        } 

        // Otherwise, we have an ongoing run
        else
        {
          // See if the current value matches the ongoing run, and the
          // maximum run length has not been exceeded (we don't want to 
          // have run length values wrap around).
          if ((currentValue == run.value) && (run.length < MaxRLERun))
          {
            // Increment the run length
            run.length++;
          }

          // Otherwise, current value is different from run value, or
          // the maximum run length has been exceeded
          else
          {
            // Save the run
            currentRow[rowLength++] = run;

            // Set the new run's value
            ASSERT(currentValue < U16_MAX)
            run.value = U16(currentValue);

            // Say we have one pixel
            run.length = 1;

          }
        }
      }

      // Save the last run encountered in a row
      currentRow[rowLength++] = run;

      // Set the actual row length
      data[r].length = rowLength;
      data[r].data = new RLCitem[rowLength];

      // Copy the actual row from the temporary row
      Utils::Memcpy(data[r].data, currentRow, rowLength * sizeof (RLCitem));
    }

    delete [] currentRow;

    // Copy over the region information from the image
    numRegions = image.GetNumRegions();

    if (numRegions)
    {
      ASSERT(!regions)
      regions = new Region[numRegions];
      Utils::Memcpy(regions, image.GetRegions(), sizeof (Region) * numRegions);
    }
  }


  //
  // GetValue
  //
  Pixel RLCmap::GetValue(U32 row, U32 column)
  {
    U32 pos = 0;
    U32 length = data[row].length;

    ASSERT(length >= 1)

    RLCitem *item = data[row].data;
    Pixel value = item->value;
    U32 endColumn = (item++)->length;

    // Go along row by runs until our x position is within a run
    while (column >= endColumn && pos++ < length)
    {
      value = item->value;
      endColumn += (item++)->length;
    }

    if (column >= endColumn)
    {
      return (0);
    }
    
    return(value);
  }


  //
  // GetValue
  //
  Pixel GetValue(U8 traction, U32 x, U32 z)
  {
    ASSERT(traction < MoveTable::TractionCount())
    return (map[traction]->GetValue(z, x));
  }


  //
  // For the given pixel get the statistical information for that region
  //
  const Region & GetRegion(U8 traction, Pixel pixel)
  {
    ASSERT(traction < MoveTable::TractionCount())
    ASSERT(pixel <= map[traction]->numRegions)
    ASSERT(pixel >= 1)
    return (map[traction]->regions[pixel - 1]);
  }


  //
  // Get the number of regions available for the given traction type
  ///
  U32 GetNumRegions(U8 traction)
  {
    ASSERT(traction < MoveTable::TractionCount())
    return (map[traction]->numRegions);
  }



  //
  // Initialise cre data by loading or creating it on the fly
  //
  void Init()
  {
    ASSERT(!initialized)

    // Create the command scope
    VarSys::RegisterHandler("coregame.cre", CmdHandler);

    // Create commands
    VarSys::CreateCmd("coregame.cre.display");
    VarSys::CreateCmd("coregame.cre.hide");
    VarSys::CreateCmd("coregame.cre.traction");
    VarSys::CreateCmd("coregame.cre.recalc");
    VarSys::CreateCmd("coregame.cre.uploadtractions");
    VarSys::CreateCmd("coregame.cre.region");
//    VarSys::CreateCmd("coregame.cre.info");

    VarSys::CreateInteger("coregame.cre.info.region", 0, VarSys::DEFAULT, &varRegion);
    VarSys::CreateInteger("coregame.cre.info.area", 0, VarSys::DEFAULT, &varArea);
    VarSys::CreateInteger("coregame.cre.info.minColumn", 0, VarSys::DEFAULT, &varMinColumn);
    VarSys::CreateInteger("coregame.cre.info.maxColumn", 0, VarSys::DEFAULT, &varMaxColumn);
    VarSys::CreateInteger("coregame.cre.info.minRow", 0, VarSys::DEFAULT, &varMinRow);
    VarSys::CreateInteger("coregame.cre.info.maxRow", 0, VarSys::DEFAULT, &varMaxRow);
    VarSys::CreateFloat("coregame.cre.info.fill", 0.0f, VarSys::DEFAULT, &varFill);

    // Clear out the map
    for (U8 traction = 0; traction < MoveTable::MAX_TRACTION_TYPES; traction++)
    {
      map[traction] = NULL;
    }

    // Generate CRE
    Generate();

    initialized = TRUE;
  }

  //
  // Shutdown/cleanup routine
  //
  void Done()
  {
    ASSERT(initialized)

    Delete();

    initialized = FALSE;
  }


  //
  // Recalc CRE data
  //
  void Recalc()
  {
    ASSERT(initialized)

    Delete();
    Generate();
  }


  //
  // Generate a new CRE
  //
  void Generate()
  {
    //LOG_DIAG(("Starting CRE"))

    U32 mapColumns = WorldCtrl::CellMapX();
    U32 mapRows = WorldCtrl::CellMapZ();

    // Fire up blobs
    Blobs::Init();

    // Allocate room for the speed map
    Blobs::Image movementImage(mapRows, mapColumns);

    // For each effect type, go through and make a connectivity map
    for (U8 traction = 0; traction < MoveTable::TractionCount(); traction++) 
    {
      // Prepare the map by finding the effect of each tile's terrain type on each mobility effect.
      for (U32 x = 0; x < mapColumns; x++)
      {
        for (U32 z = 0; z < mapColumns; z++)
        {
          movementImage(x, z) = Pixel(PathSearch::CanMoveToCell(traction, x, z)) ? Blobs::TargetColor : Pixel(0);
        }
      }

      // Now that we have a movement image, compute the mobility-connected regions
      movementImage.FindConnectedRegions(mapRows, mapColumns);

      // Compress the map
      map[traction] = new RLCmap(mapRows);
      map[traction]->Compress(movementImage, mapRows, mapColumns);
    }

    // Finished with blobs for now
    Blobs::Done();

    //LOG_DIAG(("Finished CRE"))
  }


  //
  // Delete the current CRE
  //
  void Delete()
  {
    for (U8 traction = 0; traction < MoveTable::TractionCount(); traction++)
    {
      if (map[traction])
      {
        delete map[traction];
        map[traction] = NULL;
      }
    }
  }


  //
  // Cmd Handler
  //
  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0xF9D0649B: // "coregame.cre.display"
      {
        // Create the control
        if (grid.Alive())
        {
          grid->Deactivate();
        }
        else
        {
          U32 xc = 200 / WorldCtrl::CellMapX();
          U32 zc = 200 / WorldCtrl::CellMapZ();
          U32 c  = Max<U32>(1, Min<U32>(xc, zc));

          // Allocate the control
          grid = new ICGridWindow("Connected Regions", WorldCtrl::CellMapX(), WorldCtrl::CellMapZ(), c, c);

          // Setup the grid
          ICGrid &g = grid->Grid();
          g.SetCellCallBack(CellCallBack);
          //g.SetPostCallBack(PathPostCallBack);
          g.SetEventCallBack(EventCallBack);
          g.SetAxisFlip(FALSE, TRUE);

          // Setup the window
          grid->SetGeometry("Top", "Right", NULL);
          grid->Activate();
        }
        break;
      }

      case 0xF5D65937: // "coregame.cre.hide"
      {
        // Hide the control
        if (grid.Alive())
        {
          grid->Deactivate();
        }
        break;
      }

      case 0x9ECBA07A: // "coregame.cre.traction"
      {
        S32 index;
        const char *name;

        if (Console::GetArgInteger(1, index))
        {
          MoveTable::KeyInfo *keyInfo = MoveTable::FindTractionInfo(U8(index));
          if (keyInfo)
          {
            currentTraction = U8(index);
            CON_DIAG(("Setting CRE traction display to '%s'", keyInfo->ident.str))
          }
          else
          {
            CON_ERR(("Unknown traction index '%d'", index))
          }
        }
        else if (Console::GetArgString(1, name))
        {
          MoveTable::KeyInfo *keyInfo = MoveTable::FindTractionInfo(name);
          if (keyInfo)
          {
            currentTraction = U8(MoveTable::TractionIndex(name));
            CON_DIAG(("Setting CRE traction display to '%s'", keyInfo->ident.str))
          }
          else
          {
            CON_ERR(("Unknown traction index '%s'", name))
          }
        }
        else
        {
          CON_ERR(("coregame.cre.traction tractiontype"))
        }
        break;
      }

      case 0xE5A4CF67: // "coregame.cre.recalc"
      {
        Recalc();
        break;
      }

      case 0x294647B6: // "coregame.cre.uploadtractions"
      {
        const char *control;
        if (Console::GetArgString(1, control))
        {
          ICListBox *listBox = IFace::Find<ICListBox>(control, NULL, TRUE);
          ICListBox::Rebuild *rebuild = listBox->PreRebuild();
          listBox->DeleteAllItems();

          MoveTable::MoveBalanceTable &table = MoveTable::GetTable();

          for (NBinTree<MoveTable::MoveBalanceTable::KeyInfo>::Iterator t(&table.GetYTree()); *t; t++)
          {
            char buff[64];
            Utils::Sprintf(buff, 64, "[%d] %s", MoveTable::TractionIndex((*t)->ident.str), (*t)->ident.str);
            listBox->AddTextItem((*t)->ident.str, Utils::Ansi2Unicode(buff));
          }

          listBox->PostRebuild(&rebuild);
        }
        break;
      }

      case 0xD4B23E5A: // "coregame.cre.region"
      {
        S32 traction;
        S32 region;

        if (Console::GetArgInteger(1, traction) && Console::GetArgInteger(1, region))
        {
          if (U8(traction) < MoveTable::TractionCount())
          {
            if (U32(region) < map[traction]->numRegions)
            {
              const Region &r = GetRegion(U8(traction), Pixel(region));
              CON_DIAG(("Region %d for traction %d", region, traction))
              CON_DIAG(("- Number of cells         : %d", r.area))
              CON_DIAG(("- Column Extents          : %d - %d", r.minColumn, r.maxColumn))
              CON_DIAG(("- Row Extents             : %d - %d", r.maxRow, r.maxRow))
              CON_DIAG(("- Amount of Extents Filled: %f%%", r.fill * 100.0f))
            }
            else
            {
              CON_ERR(("Invalid region '%d' for traction '%d'", region, traction))
            }
          }
          else
          {
            CON_ERR(("Invalid traction index '%d', try coregame.tractiontypes", traction))
          }
        }
        else
        {
          CON_ERR(("coregame.cre.region traction region"))
        }
        
        break;
      }

      case 0xA3C8CCCC: // "coregame.cre.info"
      {
        MoveTable::MoveBalanceTable &table = MoveTable::GetTable();

        for (NBinTree<MoveTable::MoveBalanceTable::KeyInfo>::Iterator t(&table.GetYTree()); *t; t++)
        {
          U8 index = MoveTable::TractionIndex((*t)->ident.str);
          CON_DIAG(("Traction [%d] %s : Regions %d", index, (*t)->ident.str, map[index]->numRegions))
        }
        break;
      }
    }
  }


  //
  // CellCallBack
  //
  // Draws the surface types on the grid 
  //
  Color CellCallBack(void *, U32 x, U32 z)
  {
    Pixel pixel = GetValue(U8(currentTraction), x, z);

    if (pixel)
    {
      // We want every third bit to go into each of the RGB values starting at the top
      U32 red = 
        ((pixel & (0x1      )) <<  7) | // Put Bit  0 of Pixel into Bit 7 of Red
        ((pixel & (0x1 <<  3)) <<  3) | // Put Bit  3 of Pixel into Bit 6 of Red
        ((pixel & (0x1 <<  6)) >>  1) | // Put Bit  6 of Pixel into Bit 5 of Red
        ((pixel & (0x1 <<  9)) >>  5) | // Put Bit  9 of Pixel into Bit 4 of Red
        ((pixel & (0x1 << 12)) >>  9) | // Put Bit 12 of Pixel into Bit 3 of Red
        ((pixel & (0x1 << 15)) >> 13);  // Put Bit 15 of Pixel into Bit 2 of Red

      U32 green = 
        ((pixel & (0x1 <<  1)) <<  6) | // Put Bit  1 of Pixel into Bit 7 of Green
        ((pixel & (0x1 <<  4)) <<  2) | // Put Bit  4 of Pixel into Bit 6 of Green
        ((pixel & (0x1 <<  7)) >>  2) | // Put Bit  7 of Pixel into Bit 5 of Green
        ((pixel & (0x1 << 10)) >>  6) | // Put Bit 10 of Pixel into Bit 4 of Green
        ((pixel & (0x1 << 13)) >> 10);  // Put Bit 13 of Pixel into Bit 3 of Green

      U32 blue = 
        ((pixel & (0x1 <<  2)) <<  5) | // Put Bit  2 of Pixel into Bit 7 of Blue
        ((pixel & (0x1 <<  5)) <<  1) | // Put Bit  5 of Pixel into Bit 6 of Blue
        ((pixel & (0x1 <<  8)) >>  3) | // Put Bit  8 of Pixel into Bit 5 of Blue
        ((pixel & (0x1 << 11)) >>  7) | // Put Bit 11 of Pixel into Bit 4 of Blue
        ((pixel & (0x1 << 14)) >> 11);  // Put Bit 14 of Pixel into Bit 3 of Blue

      return (Color(red, green, blue));
    }
    else
    {
      return (Color(U32(0), U32(0), U32(0)));
    }
  }


  //
  // EventCallback
  //
  void EventCallBack(void *, U32 x, U32 z, U32 event)
  {
    if (event == 0x90E4DA5D) // "LeftClick"
    {
      Pixel pixel = GetValue(U8(currentTraction), x, z);
      if (pixel)
      {
        const Region &r = GetRegion(U8(currentTraction), pixel);

        // Set all of the vars
        varRegion = pixel;
        varArea = r.area;
        varMinColumn = r.minColumn;
        varMaxColumn = r.maxColumn;
        varMinRow = r.minRow;
        varMaxRow = r.maxRow;
        varFill = r.fill * 100.0f;
      }
      else
      {
        // Clear all of the vars
        varRegion = 0;
        varArea = 0;
        varMinColumn = 0;
        varMaxColumn = 0;
        varMinRow = 0;
        varMaxRow = 0;
        varFill = 0.0f;
      }
    }
  } 

}



