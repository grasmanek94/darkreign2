///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// Blobs
//


#ifndef __BLOBS_H
#define __BLOBS_H


///////////////////////////////////////////////////////////////////////////////
//
// NameSpace Blobs
//
namespace Blobs
{

  ///////////////////////////////////////////////////////////////////////////////
  //
  // Definitions
  //

  // A blob pixel
  typedef U16 Pixel;

  // Maximum number of regions
  const Pixel MaxRegions = 65534;

  // Value to use to make a pixel get tested for connected regions
  const Pixel TargetColor = MaxRegions + 1;


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Struct Region
  //
  struct Region
  {
    // Area of the region in pixels
    U32 area;

    // Row bondaries of the blob
    U32 minRow, maxRow;

    // Column boundaries of the blob
    U32 minColumn, maxColumn;

    // Filled percent of the bounding rectangle
    F32 fill;

  };


  ///////////////////////////////////////////////////////////////////////////////
  //
  // Class Image
  //
  class Image
  {
  private:

    // Number of rows
    U32 rows;

    // Number of columns
    U32 columns;

    // The pixel data for the image
    Pixel **data;

    // Number of regions
    U32 numRegions;

    // Region statistics
    Region *regions;

  public:

    struct ColoringPoint;

  public:

    // Constructor and Destructor
    Image(U32 rows, U32 columns);
    ~Image();

    // Find the connect regions in the image
    void FindConnectedRegions(U32 rows, U32 columns);

  public:

    // () operator
    Pixel & operator()(U32 c, U32 r) const
    {
      return (data[c][r]);
    }

    // GetPixel
    Pixel & GetPixel(U32 c, U32 r) const
    {
      return (data[c][r]);
    }

    // For the given pixel get the statistical information for that region
    const Region & GetRegion(Pixel pixel) const
    {
      ASSERT(pixel < numRegions)
      return (regions[pixel]);
    }

    // Get the number of regions found
    U32 GetNumRegions() const
    {
      return (numRegions);
    }

    // Get the regions
    const Region * GetRegions() const
    {
      return (regions);
    }

  private:

    void UpdateRegion(U32 r, U32 c, U32 region);
    void ComputeBlobData(U32 region);
    void AddPointToGrowList(ColoringPoint *&growingList, U32 row, U32 column, Pixel crayon);
    void Color(U32 row, U32 column, Pixel crayon, U32 rows, U32 colunns);
    void Uncolor(U32 row, U32 column, Pixel crayon, U32 rows, U32 columns);
    void BlankRegion(U32 region);
    void PrintRegion(U32 region) const;

  };

  // Initialization and Shutdown
  void Init(void);
  void Done(void);

}

#endif
