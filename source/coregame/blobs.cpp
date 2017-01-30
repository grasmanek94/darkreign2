///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Blobs
//


///////////////////////////////////////////////////////////////////////////////
//
// Includes
//
#include "blobs.h"


///////////////////////////////////////////////////////////////////////////////
//
// Namespace Blob
//
namespace Blobs
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Image::ColoringPoint
  //
  // A point stored in the queue for coloring blobs
  //
  struct Image::ColoringPoint
  {
    U32 row;
    U32 column;
    ColoringPoint *next;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Internal data
  //

  // Coloring point list is a list of recently deleted points
  static Image::ColoringPoint *pointHead;
  static U32 pointCount;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Prototypes
  //
  static Image::ColoringPoint * NewColoringPoint();
  static void DelColoringPoint(Image::ColoringPoint *n);


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Image
  //


  //
  // Constructor
  //
  Image::Image(U32 rows, U32 columns)
  : rows(rows), 
    columns(columns),
    numRegions(0)
  {
    data = new Pixel*[rows];
    for (U32 r = 0; r < rows; r++)
    {
      data[r] = new Pixel[columns];
    }

    regions = new Region[MaxRegions];
  }


  //
  // Destructor
  //
  Image::~Image()
  {
    for (U32 r = 0; r < rows; r++)
    {
      delete [] data[r];
    }
    delete [] data;
    delete [] regions;
  }


  //
  // FindConnectedRegions
  //
  void Image::FindConnectedRegions(U32 rows, U32 columns)
  {
    // Which color we are using now
    Pixel crayon = 1;

    // Reset number of blobs
    numRegions = 0;

    for (U32 i = 0; i < rows; i++)
    {
	    for (U32 j = 0; j < columns; j++)
      {
        // See if we've found a new blob.  If so, color it & record data
        if (GetPixel(i, j) == TargetColor)
        {
          BlankRegion(numRegions);
          Color(i, j, crayon, rows, columns);
          ComputeBlobData(crayon - 1);

          // Record the blob number if needed
          numRegions++;
          crayon++;

          if (crayon > MaxRegions)
          {
            crayon = 1;
            LOG_WARN(("Too many darned regions!"))
          }
        }
	    }
	  } 
  }


  //
  // UpdateRegion
  //
  void Image::UpdateRegion(U32 r, U32 c, U32 reg)
  {
    Region &region = regions[reg];

    // Add one pixel to the area of the blob
    region.area++;

    // Modify bounding rectangle
    region.minColumn = Min(c, region.minColumn);
    region.maxColumn = Max(c, region.maxColumn);
    region.minRow = Min(r, region.minRow);
    region.maxRow = Max(r, region.maxRow);
  }


  //
  // Compute the more advanced measures of a blob 
  // that require the whole blob to be found first
  //
  void Image::ComputeBlobData(U32 region)
  {
    // No data need be computed for an empty blob
    if (regions[region].area == 0)
    {
      return;
    }

    // Find the area of the bounding box
    F32 rectArea = F32(
      (1 + regions[region].maxRow - regions[region].minRow) * 
      (1 + regions[region].maxColumn - regions[region].minColumn));

    // Find the percentage of the bounding rectangle which is actually full
    ASSERT(rectArea >= 0.0f) 
    regions[region].fill = ((F32) regions[region].area) / rectArea;
  }


  //
  // Recursively color in the blobs found in the reduced image
  //
  void Image::AddPointToGrowList(ColoringPoint *&growingList, U32 row, U32 column, Pixel crayon)
  {
    ColoringPoint *newPoint;

    // Apply our "virtual crayon" to pt
    GetPixel(row, column) = crayon;

    // Make a new coloring point out of our starting place
    newPoint = NewColoringPoint();

    // Set it up
    newPoint->row = row;
    newPoint->column = column;

    // Add it to the list
    newPoint->next = growingList;
    growingList = newPoint;
  }


  //
  // Recursively color in the blobs found in the reduced image
  //
  void Image::Color(U32 row, U32 column, Pixel crayon, U32 rows, U32 columns)
  {
    // List of nodes to color
    ColoringPoint *growingList = NULL;

    // Current point to color
    ColoringPoint *currentPoint;

    // Make a new coloring point out of our starting place
    AddPointToGrowList(growingList, row, column, crayon);

    // While points unprocessed...
    while (growingList)
    {
      // Pop next point from list
      currentPoint = growingList;

      // Remove it from list
      growingList = growingList->next;

      // Update the stats
      UpdateRegion(currentPoint->row, currentPoint->column, crayon - 1);
    
      // Check to see if each of the adjacent points needs to be color'd
      if (currentPoint->column > 0)
      {
        if (GetPixel(currentPoint->row, currentPoint->column - 1) == TargetColor)
        {
          AddPointToGrowList(growingList, currentPoint->row, currentPoint->column - 1, crayon);
        }
      }

      if (currentPoint->row > 0)
      {
        if (GetPixel(currentPoint->row - 1, currentPoint->column) == TargetColor)
        {
          AddPointToGrowList(growingList, currentPoint->row - 1, currentPoint->column, crayon);
        }
      }

      if (currentPoint->column < columns - 1)
      {
        if (GetPixel(currentPoint->row, currentPoint->column + 1) == TargetColor)
        {
          AddPointToGrowList(growingList, currentPoint->row, currentPoint->column + 1, crayon);
        }
      }

      if (currentPoint->row < rows - 1)
      {
        if (GetPixel(currentPoint->row + 1, currentPoint->column) == TargetColor)
        {
          AddPointToGrowList(growingList, currentPoint->row + 1, currentPoint->column, crayon);
        }
      }

      // Release the memory from the current point
      DelColoringPoint(currentPoint);
    }
  }


  //
  // Recursively color in the blobs found in the reduced image
  //
  void Image::Uncolor(U32 row, U32 column, Pixel crayon, U32 rows, U32 columns)
  {
    // List of nodes to color
    ColoringPoint *growingList;

    // Current point to color
    ColoringPoint *currentPoint;

    // Init the list of points
    growingList = NULL;

    // Make a new coloring point out of our starting place
    AddPointToGrowList(growingList, row, column, 0);

    // While points unprocessed...
    while (growingList)
    {
      // Pop next point from list
      currentPoint = growingList;

      // Remove it from list
      growingList = growingList->next;

      // Check to see if each of the adjacent points needs to be color'd
      if (currentPoint->column > 0)
      {
        if (GetPixel(currentPoint->row, currentPoint->column - 1) == crayon)
        {
          AddPointToGrowList(growingList, currentPoint->row, currentPoint->column - 1, 0);
        }
      }

      if (currentPoint->row > 0)
      {
        if (GetPixel(currentPoint->row - 1, currentPoint->column) == crayon)
        {
          AddPointToGrowList(growingList, currentPoint->row - 1, currentPoint->column, 0);
        }
      }

      if (currentPoint->column < columns - 1)
      {
        if (GetPixel(currentPoint->row, currentPoint->column + 1) == crayon)
        {
          AddPointToGrowList(growingList, currentPoint->row, currentPoint->column + 1, 0);
        }
      }

      if (currentPoint->row < rows - 1)
      {
        if (GetPixel(currentPoint->row + 1, currentPoint->column) == crayon)
        {
          AddPointToGrowList(growingList, currentPoint->row + 1, currentPoint->column, 0);
        }
      }

      // Release the memory from the current point
      DelColoringPoint(currentPoint);
    }
  }


  //
  // Initialize the data record for a region
  //
  void Image::BlankRegion(U32 region)
  {
    regions[region].area = 0;
    regions[region].minColumn = U32_MAX;
    regions[region].maxColumn = 0;
    regions[region].minRow = U32_MAX;
    regions[region].maxRow = 0;
  }


  //
  // Print out the information for a given region
  //
  void Image::PrintRegion(U32 region) const
  {
    ASSERT(region < numRegions)
    Region &r = regions[region];
    
    LOG_DIAG(("Region: %d", region))
    LOG_DIAG((" - Area: %d", r.area))
    LOG_DIAG((" - Columns: %d, %d", r.minColumn, r.maxColumn))
    LOG_DIAG((" - Rows: %d, %d", r.minRow, r.maxRow))
    LOG_DIAG((" - Fill %: [%f]", r.fill))
  }


  //
  // Initialization
  //
  void Init()
  {
    pointHead = NULL;
    pointCount = 0;
  }


  //
  // Shutdown
  //
  void Done()
  {
    Image::ColoringPoint *n;

    while (pointHead)
    {
      n = pointHead;
      pointHead = pointHead->next;
      delete n;
      pointCount--;
    }

    ASSERT(!pointCount)
  }


  //
  // Get a new colouring point
  //
  Image::ColoringPoint * NewColoringPoint()
  {
    Image::ColoringPoint *n;

    // is there any unused nodes?
    if (pointHead)
    {
      // reuse old node
      n = pointHead;
      pointHead = pointHead->next;
    }
    else
    {
      // new node
      n = new Image::ColoringPoint;
      pointCount++;
    }

    return (n);
  }


  //
  // Discard a colouring point
  //
  void DelColoringPoint(Image::ColoringPoint *n)
  {
    n->next = pointHead;
    pointHead = n;
  }

}

