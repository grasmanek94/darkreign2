///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// terrain.cpp     DR2 terrain system
//
// 04-MAY-1998
//

#include "random.h"
#include "console.h"
#include "terrain.h"
#include "statistics.h"
//----------------------------------------------------------------------------

#define DOVERTCOLORS
#define DOBLACKTERRAIN      // outside map bounds
#define DOMIPMAPS
//#define DOQUICKNORMALS

const F32 MAXTEXCOORD         = 1.0f;

const char *BASETEXTURENAME   = "terrainbase.pic";
const char *WATERTEXTURENAME  = "water.tga";
const char *GRIDTEXTURENAME   = "terrgrid.tga";
const char *EDITTEXTURENAME   = "terredit.tga";
const char *SHADOWTEXTURENAME = "shadow.pic";
//----------------------------------------------------------------------------

namespace Terrain
{
  // externals
  //
  U32             sysInit;

  F32             fogFactors[8] = { 1.0f, 0.875f, 0.75f, 0.625f, 0.5f, 0.375f, 0.25f, 0.0f };

  HeightField     heightField;

  F32             farPlane;

  F32             terrAverageHeight;
  F32             terrMinHeight;
  F32             terrMaxHeight;

  // show shroud
  //
  VarInteger      shroud;       

  F32             moveSpeed;

  // locals
  //
  U32             clusWidth, clusHeight, clusCount;
  U32             meterPerClus, cellPerClus; // in one dimension
  U32             cellPerClusShift;
  F32             clusPerMeter;
  Cluster         *clusList;

  Bitmap          *waterTex, *defTex, *editTex, *shadowTex;
  Bitmap          *texList[TEXTURECOUNT];
  U32             texCount;
  VarInteger      mipCount;       

  // cell normal array
  //
	U32             normCount;						
	Vector          normList[NORMALCOUNT];  

  // calculated normal lighting array
  //
	Color           normLights[NORMALCOUNT];  

  // reflect terrain off map?
  //
  VarInteger      offmap;       
  Color           offMapColor;

  // show water
  //
  VarInteger      water;       

  // terrain shading intensity
  //
  VarFloat        shadefactor;

  // default terrain texture
  //
  VarString       baseTexName;

  // meshviewer animation
  //
  F32             movePos;

  // water animation
  //
  VarFloat        waterSpeed;
  F32             waterPos0, waterPos1;

  UVPair          baseUVList0[4] =
  {
    UVPair( 0.0f, 0.0f),
    UVPair( 0.0f, MAXTEXCOORD),
    UVPair( MAXTEXCOORD, MAXTEXCOORD),
    UVPair( MAXTEXCOORD, 0.0f),
  };
  UVPair          baseUVList1[4] =
  {
    UVPair( 0.5f, 0.0f),
    UVPair( 0.5f, 1.0f),
    UVPair( 1.5f, 1.0f),
    UVPair( 1.5f, 0.0f),
  };
  UVPair          waterUVList0[4];
  UVPair          waterUVList1[4];

  // cell uv array
  // 4 pairs * 4 rotations * 3 flips 
  //
  UVPair cellUVList[4 * 4 * 3] =
  {
                       // no flip 
     UVPair( 0.0f, 0.0f),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, 0.0f),

     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, 0.0f),     
     UVPair( 0.0f, 0.0f),

     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, 0.0f),     
     UVPair( 0.0f, 0.0f),
     UVPair( 0.0f, MAXTEXCOORD),

     UVPair( MAXTEXCOORD, 0.0f),     
     UVPair( 0.0f, 0.0f),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
                     // u flip
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),

     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),     
     UVPair( MAXTEXCOORD, 0.0f),

     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),     
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),

     UVPair( 0.0f, 0.0f),     
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),
     UVPair( 0.0f, MAXTEXCOORD),
                      // v flip
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),

     UVPair( 0.0f, 0.0f),
     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),     
     UVPair( 0.0f, MAXTEXCOORD),

     UVPair( MAXTEXCOORD, 0.0f),
     UVPair( MAXTEXCOORD, MAXTEXCOORD),     
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),

     UVPair( MAXTEXCOORD, MAXTEXCOORD),     
     UVPair( 0.0f, MAXTEXCOORD),
     UVPair( 0.0f, 0.0f),
     UVPair( MAXTEXCOORD, 0.0f),
  };

  namespace Sky
  {
    void Init();
    void Done();
    void Simulate( F32 timestep);
    void Render();
    U32 GetMem();
  }

  U32 endVar;     // just for GetMem to calc sizeof Terrain namespace
  //----------------------------------------------------------------------------

  // function prototypes
  //
	void CalcLighting();

  void BuildNormals();
  void Recalc();
  void CalcSpheres();
	void CalcNormals();
	void CalcNormalsQuick();

  Bool ChekTri( F32 py, U32 o0, U32 o1, U32 o2, U32 level = 0);
  Bool DrawTri( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, Vector *verts, U16 **indexList, U32 level = 0);

  void RenderCluster( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, U32 clipFlags = clipALL);
  void RenderClusterColor( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, Color color, U32 clipFlags = clipALL);
  void RenderClusterMRM( Cluster &clus, S32 x, S32 z, U32 cellOffset, U32 clipFlags = clipALL);
  void RenderClusterColorMRM( Cluster &clus, S32 x, S32 z, U32 cellOffset, Color color, U32 clipFlags = clipALL);
  void RenderCellNormal( Cell &cell, F32 x, F32 z);

  void RenderIsometric();  

  void RenderCell( Cell &c0, Cell &c1, Cell &c2, Cell &c3, S32 x0, S32 z0, S32 x1, S32 z1, Color color, U32 clipFlags);
  void RenderCell( Cell **cc, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, UVPair *uvList, U32 clipFlags);
   //----------------------------------------------------------------------------

  void CmdHandler(U32 pathCrc)
  {
    switch (pathCrc)
    {
      case 0x9C14C472: // "terrain.shadefactor"
      {
        // rebuild the terrain normal list
        Terrain::BuildNormals();
        break;
      }
      case 0x34A5BC9B: // "terrain.shadefactorinc"
      {
  #define SHADEINC      0.01f

        F32 shade = *shadefactor + SHADEINC;
        if (shade > 1.0f)
        {
          shade = 0.0f;
        }
        shadefactor = shade;
        break;
      }
      case 0x0CF9BADE: // "terrain.toggle.shroud"
      {
        shroud = !*shroud;
        break;
      }
      case 0x662A9CB3: // "terrain.toggle.water"
      {
        water = !*water;
        break;
      }
      case 0x7E6F4AE8: // "terrain.toggle.offmap"
      {
        offmap = !*offmap;
        break;
      }
    }
  }
  //----------------------------------------------------------------------------

  // clear terrain's values
  // 
  Bool Init()
  {
    // do vars first
    //
    VarSys::RegisterHandler("terrain", CmdHandler);
    VarSys::RegisterHandler("terrain.show", CmdHandler);
    VarSys::RegisterHandler("terrain.toggle", CmdHandler);

    VarSys::CreateInteger("terrain.mipcount", 5, VarSys::DEFAULT, &mipCount);

    VarSys::CreateFloat("terrain.shadefactor", 0.0, VarSys::NOTIFY, &shadefactor);
    VarSys::SetFloatRange("terrain.shadefactor", 0.0f, 1.0f);
    VarSys::CreateFloat("terrain.waterspeed", 0.01f, VarSys::DEFAULT, &waterSpeed);

    VarSys::CreateString("terrain.basetex", BASETEXTURENAME, VarSys::NOTIFY, &baseTexName);

    VarSys::CreateInteger("terrain.show.offmap", 0, VarSys::DEFAULT, &offmap);
    VarSys::CreateInteger("terrain.show.shroud", 1, VarSys::DEFAULT, &shroud);
    VarSys::CreateInteger("terrain.show.water", 1, VarSys::DEFAULT, &water);

    VarSys::CreateCmd("terrain.toggle.shroud");
    VarSys::CreateCmd("terrain.toggle.water");
    VarSys::CreateCmd("terrain.toggle.offmap");

    VarSys::CreateCmd("terrain.shadefactorinc");

    Sky::Init();

    heightField.ClearData();

    cellPerClusShift = CELLPERCLUSTERSHIFT;
    clusList = NULL;

    waterTex = NULL;
    defTex   = NULL;
    editTex  = NULL;
    texCount = 0;

    offMapColor.Set(U32(0), U32(0), U32(0), U32(255));

	  normCount = 0;
    BuildNormals();

    waterPos0 = waterPos1 = 0.0f;

    moveSpeed = 0.0f; 
    movePos = 0.0f;

    return sysInit = TRUE;
  }
  //----------------------------------------------------------------------------

  // release terrain memory
  //
  void Release()
  {
    heightField.Release();

    if (clusList)
    {
      delete [] clusList;
    }
    clusList = NULL;
  }
  //----------------------------------------------------------------------------

  // close the terrain system
  //
  void Done()
  {
    Sky::Done();

    // Destroy terrain scope
    VarSys::DeleteItem("terrain.toggle");
    VarSys::DeleteItem("terrain.show");
    VarSys::DeleteItem("terrain");

    // Delete the command scope
    VarSys::DeleteItem("terrain");

    Release();

    sysInit = FALSE;
  }
  //----------------------------------------------------------------------------

  F32 FindFloor( F32 x, F32 z, Vector *surfNormal) // = NULL)
  {
    return heightField.FindFloor(x, z, surfNormal);
  }
  //----------------------------------------------------------------------------

  F32 FindFloorWithWater( F32 x, F32 z, Vector *surfNormal) // = NULL)
  {
    // Get the cluster this position is within
    Cluster &clus = GetCluster(x, z);

    F32 h1 = heightField.FindFloor(x, z, surfNormal);

    if (clus.status.water && clus.waterHeight > h1)
    {
      if (surfNormal)
      {
        *surfNormal = normList[0];
      }
      return clus.waterHeight;
    }
    return h1;
  }
  //----------------------------------------------------------------------------

  // doesn't check for valid cell coords
  //
  F32 GetHeight( S32 cx, S32 cz)
  {
    return heightField.cellList[ cz * heightField.cellPitch + cx].height;
  }
  //----------------------------------------------------------------------------

  Bool GetWater( S32 cx, S32 cz, F32 *height) // = NULL
  {
    cx >>= cellPerClusShift;
    cz >>= cellPerClusShift;

    Cluster &clus = clusList[ cz * clusWidth + cx];

    if (clus.status.water)
    {
      if (height)
      {
        *height = clus.waterHeight;
      }

      return (TRUE);
    }
    return FALSE;
  }
  //----------------------------------------------------------------------------

  F32  GetHeightWithWater( S32 cx, S32 cz)
  {
    F32 h0, h1 = GetHeight( cx, cz);
    if (GetWater( cx, cz, &h0) && h0 > h1)
    {
      return h0;
    }
    return h1;
  }
  //----------------------------------------------------------------------------

  // perform a heightfield to terrain paste operation
  // recalc data important to terrain
  //
  void Paste( Rect &dstRect, HeightField &buf, Rect &bufRect, F32 scale, U32 flags, F32 atHeight) // = EDITHEIGHTS
  {
    heightField.Paste( dstRect, buf, bufRect, scale, flags, atHeight);

    if (!(flags & HeightField::EDITHEIGHTS))
    {
      // not editing heights: nothing else to do
      return;
    }
    // editing heights: recalc cell normals and cluster bounding spheres

    // adjacent cells are affected: increase rect bounds by 1
    Rect drect = dstRect;
    drect.l -= 1;
    drect.r += 1;
    drect.t -= 1;
    drect.b += 1;

    CalcCellRect( dstRect);
  }
  //----------------------------------------------------------------------------

  //
  // GetClusterOffsets
  //
  // Returns the offset of the cluster containing the given metre position
  //
  void GetClusterOffsets(F32 x, F32 z, U32 &cx, U32 &cz)
  {
    ASSERT(MeterOnMap(x, z));

    x -= OffsetX();
    z -= OffsetZ();

    // Get the cluster this position is within
    cx = U32(x * clusPerMeter);
    cz = U32(z * clusPerMeter);
  }
  //----------------------------------------------------------------------------

  //
  // GetCluster
  //
  // Returns the cluster containing the given metre position
  //
  Cluster & GetCluster(F32 x, F32 z)
  {
    ASSERT(MeterOnMap(x, z));

    x -= OffsetX();
    z -= OffsetZ();

    // Get the cluster this position is within
    return (clusList[U32(z * clusPerMeter) * clusWidth + U32(x * clusPerMeter)]);
  }
  //----------------------------------------------------------------------------

  //
  // GetCluster
  //
  // Returns the cluster at the given cluster position
  //
  Cluster & GetCluster(U32 cx, U32 cz)
  {
    ASSERT(cx < clusWidth)
    ASSERT(cz < clusHeight)

    return (clusList[cz * clusWidth + cx]);
  }
  //----------------------------------------------------------------------------

  //
  // FindWater
  //
  // Find the water height at this location (FALSE if no water)
  //
  Bool FindWater(F32 x, F32 z, F32 *height, F32 *depth)
  {
    // Get the cluster this position is within
    Cluster &clus = GetCluster(x, z);

    if (clus.status.water)
    {
      if (height)
      {
        *height = clus.waterHeight;
      }

      if (depth)
      {     
        *depth = clus.waterHeight - FindFloor(x, z);
      }

      return (TRUE);
    }

    return (FALSE);
  }
  //----------------------------------------------------------------------------

  // recalculate normals and affected clusters' bounding spheres
  //
  void CalcCellRect( const Rect &rect)
  {
    Rect drect = rect;

    // clip rectangle to leave edge cell normals alone
    // CalcCellNormal expects to access cell - 1, cell + 1, cell - cellWidth, cell + cellWidth
    if (drect.l <= 0)
    {
      drect.l = 1;
    }
    if (drect.r >= (S32) (heightField.cellWidth - 1))
    {
      drect.r = heightField.cellWidth - 2;
    }
    if (drect.t <= 0)
    {
      drect.t = 1;
    }
    if (drect.b >= (S32) (heightField.cellHeight - 1))
    {
      drect.b = heightField.cellHeight - 2;
    }

    S32 z, x;  // cells
    for (z = drect.t; z < drect.b; z++)
    {
      x = rect.l;
      U32 offset = z * heightField.cellPitch + x;
      for (x = drect.l; x < drect.r; x++, offset++)
      {
        Cell &cell = heightField.cellList[ offset];
  #define DODYNAMICNORMALS2
  #ifdef DODYNAMICNORMALS1
        cell.normal = (U8) heightField.CalcCellNormalQuick( offset);
  #endif
  #ifdef DODYNAMICNORMALS2
        cell.normal = (U8) heightField.CalcCellNormal( offset);
  #endif

        if (cell.height < terrMinHeight)
        {
          terrMinHeight = cell.height ;
        }
        if (cell.height  > terrMaxHeight)
        {
          terrMaxHeight = cell.height ;
        }
      }
    }

    // calc cluster bounding rect
    drect = rect;
    drect.l >>= cellPerClusShift;
    drect.r >>= cellPerClusShift;
    drect.t >>= cellPerClusShift;
    drect.b >>= cellPerClusShift;

    if (drect.l < 0)
    {
      drect.l = 0;
    }
    if (drect.t < 0)
    {
      drect.t = 0;
    }

    if (drect.r < (S32) (clusWidth - 1))
    {
      drect.r += 1;
    }
    else if (drect.r > (S32) clusWidth)
    {
      drect.r = (S32) clusWidth;
    }
    if (drect.b < (S32) (clusHeight - 1))
    {
      drect.b += 1;
    }
    else if (drect.b > (S32) clusHeight)
    {
      drect.b = (S32) clusHeight;
    }

    for (z = drect.t; z < drect.b; z++)
    {
      x = drect.l;
      U32 offset = z * clusWidth + x;
      for ( ; x < drect.r; x++, offset++)
      {
        CalcClusSphere( x, z);
      }
    }
  }
  //----------------------------------------------------------------------------

  void ClusSetParents( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, U32 level = 0)
  {
    U32 o3 = (o1 + o2) >> 1;
    U32 i3 = (i1 + i2) >> 1;

    if (level < 3)
    {
      level++;

      ClusSetParents( o3, o2, o0, i3, i2, i0, level);
      ClusSetParents( o3, o0, o1, i3, i0, i1, level);
    }
    Cell &c3 = heightField.cellList[o3];
    Cell &c0 = heightField.cellList[o0];

    if ( (o0 / heightField.cellPitch) % cellPerClus == 0
     &&  (o0 % heightField.cellPitch) % cellPerClus == 0 )
    {
      // cluster corners
      return;
    }

    c3.parent1 = &c0;

    S32 o4 = 2 * S32(o3) - S32(o0);

    S32 clusx = (o0 % heightField.cellPitch) / cellPerClus;
    Bool hit = FALSE;
    if (o4 < 0 || o4 > S32(heightField.cellMax))
    {
      hit = TRUE;
    }
    else if (clusx == 0 && (i3 == 5 || i3 == 10 || i3 == 15))
    {
      hit = TRUE;
    }
    else if (clusx == S32(clusWidth - 1) && (i3 == 9 || i3 == 14 || i3 == 19))
    {
      hit = TRUE;
    }
    if (!hit)
    {
      Cell &c4 = heightField.cellList[o4];
      c3.parent2 = &c4;
    }
  }
  //----------------------------------------------------------------------------

  void SetBaseTexName( const char *name)
  {
    baseTexName = name;
  }
  //----------------------------------------------------------------------------

  // allocate and initialize a terrain 
  //
  Bool Setup( U32 wid, U32 hgt, U32 csize, F32 offsetX, F32 offsetZ, F32 startHeight) // = 0.0f, 0.0f, 10.0f
  {
    // release anything already open
    Release();

    memset( texList, 0, sizeof( Bitmap *) * TEXTURECOUNT);
    texCount = 0;
    AddTexture( *baseTexName);

    cellPerClus = 1 << cellPerClusShift;

    // make sure that wid and hgt contain an integer number of clusters
    wid = wid / cellPerClus * cellPerClus;
    hgt = hgt / cellPerClus * cellPerClus;

    // create the heightfield
    heightField.Setup( wid, hgt, csize, offsetX, offsetZ, startHeight);

    clusWidth    = heightField.cellWidth  / cellPerClus;
    clusHeight   = heightField.cellHeight / cellPerClus;
    meterPerClus = heightField.meterPerCell * cellPerClus;
    clusPerMeter = 1.0f / (F32) meterPerClus;
    clusCount    = clusWidth * clusHeight;

    // create the cluster list
    // clusters are initialized through their constructors
    clusList = new Cluster[clusCount + 1];
    if (!clusList)
    {
      return FALSE;
    }
    Cluster *c0, *ce0 = clusList + clusCount;
    U32 o0 = 0;
    for (c0 = clusList; c0 < ce0; c0 += clusWidth, o0 += heightField.cellPitch * cellPerClus)
    {
      Cluster *c, *ce = c0 + clusWidth;
      U32 o = o0;
      for (c = c0; c < ce; c++, o += cellPerClus)
      {
        c->offset = o;

        U32 o1 = o  + 4 * CellPitch();
        U32 o2 = o  + 4;
        U32 o3 = o1 + 4;

        ClusSetParents( o,  o1, o2,  0, 20,  4);
        ClusSetParents( o3, o2, o1, 24,  4, 20);

        Cluster *l = c - 1;
        if (l >= c0)
        {
          if (c + 1 == ce)
          {
            c->status.right = TRUE;
          }
        }
        else
        {
          c->status.left = TRUE;
        }
      }
    }

    terrAverageHeight = terrMinHeight = terrMaxHeight = heightField.cellList[0].height;

    // initialize the cell clusters' bounding spheres
    Recalc();

    waterTex  = Bitmap::Manager::FindCreate( WATERTEXTURENAME,  *mipCount);
    defTex    = Bitmap::Manager::FindCreate( GRIDTEXTURENAME,   *mipCount);
    editTex   = Bitmap::Manager::FindCreate( EDITTEXTURENAME,   *mipCount);
    shadowTex = Bitmap::Manager::FindCreate( SHADOWTEXTURENAME, 0);

    Vid::CurCamera().SetFarPlane( 440.0f);

    sysInit = TRUE;

    return TRUE;
  }
  //----------------------------------------------------------------------------

  // return the offset of the cluster at location 'x, z' in meters
  //
  U32 ClusterOffset( F32 x, F32 z)
  {
    x -= OffsetX();
    z -= OffsetZ();

    S32 cx = (S32) x;       // FIXME
    S32 cz = (S32) z;
    if (cx < 0)
    {
      cx = 0;
    }
    else if (cx >= (S32) heightField.meterWidth)
    {
      cx = (S32) heightField.meterWidth - 1;
    }
    if (cz < 0)
    {
      cz = 0;
    }
    else if (cz >= (S32) heightField.meterHeight)
    {
      cz = (S32) heightField.meterWidth - 1;
    }
    cx = (S32) ((F32) cx * clusPerMeter);
    cz = (S32) ((F32) cz * clusPerMeter);

	  return (U32) (cz * clusWidth + cx);
  }
  //----------------------------------------------------------------------------

  void RenderCluster( Cluster &clus, S32 x, S32 z, U32 cellOffset)
  {
    Sphere sphere = clus.sphere;
    Vector o = sphere.Origin();
    o.x = (F32)x + (F32)meterPerClus * 0.5f;
    o.z = (F32)z + (F32)meterPerClus * 0.5f;
    sphere.SetOrigin( o);

    U32 clipFlags = Vid::CurCamera().SphereTest( sphere);
    if (clipFlags == clipOUTSIDE)
    {
      // cluster is completely outside the view frustrum
      return;
    }
    DxLight::Manager::SetActiveList( sphere);

    RenderCluster( clus, x, z, cellOffset, 1, 1, clipFlags);
  }
  //----------------------------------------------------------------------------

  void SetClusMrm( U32 offset, Bool flag)
  {
    U32 wid = CellPitch();
    Cell *c0 = &heightField.cellList[offset];

    if (flag)
    {
      (c0 + 1)->Activate();
      (c0 + 3)->Activate();
      (c0 + wid)->Activate();
      (c0 + wid + 4)->Activate();
      (c0 + 3 * wid)->Activate();
      (c0 + 3 * wid + 4)->Activate();
      (c0 + 4 * wid + 1)->Activate();
      (c0 + 4 * wid + 3)->Activate();
    }
    else
    {
      Cell *c, *ce, *ce0 = c0 + wid * cellPerClus;

      for (c = c0 + 1, ce = c0 + cellPerClus; c < ce; c++)
      {
        c->flags &= ~Cell::cellMRM;
      }
      for (c0 += wid; c0 < ce0; c0 += wid)
      {
        for (c = c0, ce = c0 + cellPerClus; c <= ce; c++)
        {
          c->flags &= ~Cell::cellMRM;
        }
      }
      for (c = c0 + 1, ce = c0 + cellPerClus; c < ce; c++)
      {
        c->flags &= ~Cell::cellMRM;
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw the terrain
  //
  void Render()
  {
  //#define DODRAWSUNARROW
  #ifdef DODRAWSUNARROW
    MeshRoot *arrowMesh = Mesh::Manager::FindLoad( "cone.god");

    Matrix m =  Sky::sun->WorldMatrix();
    m.posit = Vid::CurCamera().WorldMatrix().Position();
    m.posit += Vid::CurCamera().WorldMatrix().Front() * 10.0f;
    arrowMesh->Render( m, Color( 0l, 222l, 0l, 122l));  
  #endif

    Mesh::Manager::terrMrmDist = Mesh::Manager::terrMrmDistFactor * Vid::CurCamera().FarPlane();

    DxLight::Manager::ResetDarken();

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);

    if (Mesh::Manager::lightQuick)
    {
      // recalc lighting values for normal list
	    CalcLighting();
    }

    // returns vRect of meters that might be visible
	  Rect vRect;
    Vid::CurCamera().GetVisibleRect( vRect);

    vRect.l -= (S32) OffsetX();
    vRect.r -= (S32) OffsetX();
    vRect.t -= (S32) OffsetZ();
    vRect.b -= (S32) OffsetZ();

    // convert it to cluster coords 
    vRect.l = (S32) ((F32) vRect.l * clusPerMeter);
    vRect.r = (S32) ((F32) vRect.r * clusPerMeter);
    vRect.t = (S32) ((F32) vRect.t * clusPerMeter);
    vRect.b = (S32) ((F32) vRect.b * clusPerMeter);
  
    // increase bounds by one along each edge to be sure everything gets drawn
    vRect.l--;
    vRect.r++;
    vRect.t--;
    vRect.b++;

    S32 l = vRect.l;
    S32 t = vRect.t;
    if (l < 0)
    {
      l = 0;
    }
    if (t < 0)
    {
      t = 0;
    }

    F32 invClusWid = 1.0f / clusWidth;
    F32 invClusHgt = 1.0f / clusHeight;

  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    Cluster *mrmClusList[1000];
    U32 mrmClusCount = 0;

    Rect cRect;
//    if (!offmap)
    {
      // clip to the actual terrain rectangle
      if (vRect.l < 0)
      {
        vRect.l = 0;
      }
      if (vRect.r > (S32) clusWidth)
      {
        vRect.r = clusWidth;
      }
      if (vRect.t < 0)
      {
        vRect.t = 0;
      }
      if (vRect.b > (S32) clusHeight)
      {
        vRect.b = clusHeight;
      }

      if (Mesh::Manager::terrMrm)
      {
        cRect = vRect;
        cRect.l -= 1;
        cRect.r += 1;
        cRect.t -= 1;
        cRect.b += 1;

        // clip to the actual terrain rectangle
        if (cRect.l < 0)
        {
          cRect.l = 0;
        }
        if (cRect.r > (S32) clusWidth)
        {
          cRect.r = clusWidth;
        }
        if (cRect.t < 0)
        {
          cRect.t = 0;
        }
        if (cRect.b > (S32) clusHeight)
        {
          cRect.b = clusHeight;
        }

        S32 wid = cRect.r - cRect.l;
        Cluster *c0  = &clusList[cRect.t * clusWidth + cRect.l];
        Cluster *ce0 = &clusList[cRect.b * clusWidth + cRect.l];
        for ( ; c0 < ce0; c0 += clusWidth)
        {
          Cluster *c  = c0;
          Cluster *ce = c0 + wid;
          for ( ; c < ce; c++)
          {
            SetClusMrm( c->offset, FALSE);
          }
        }
      }

      // re-convert to meters
      vRect.t *= meterPerClus;
      vRect.b *= meterPerClus;
      vRect.l *= meterPerClus;
      vRect.r *= meterPerClus;

      S32 cellStrideX = 1;
      S32 cellStrideZ = 1;

      S32 z, x;
      for (z = vRect.t; z < vRect.b; z += meterPerClus)
      {
        for (x = vRect.l; x < vRect.r; x += meterPerClus)
        {
          F32 coz = (F32) z * clusPerMeter;
          F32 cox = (F32) x * clusPerMeter;

          U16 cw = Utils::FP::SetTruncMode();

          S32 clusOffz = Utils::FastFtoL( coz);
          S32 clusOffx = Utils::FastFtoL( cox);

          Utils::FP::RestoreMode(cw);

          Cluster &clus = clusList[clusOffz * clusWidth + clusOffx];

          U32 clipFlags = Vid::CurCamera().SphereTest( clus.sphere, &clus.viewOrigin);
          if (clipFlags == clipOUTSIDE)
          {
            // cluster is completely outside the view frustrum
            continue;
          }
          U32 off = clusOffz * cellPerClus * heightField.cellPitch + clusOffx * cellPerClus;

   //      Vid::SetModelViewVector( sphere.Origin());
  //        if (!DxLight::Manager::quickLight)
  //        {
            DxLight::Manager::SetActiveList( clus.sphere);
            DxLight::Manager::SetupLightsModelSpace();
  //        }

          clus.offset = off;
          clus.x = x + (S32) OffsetX();
          clus.z = z + (S32) OffsetZ();
          clus.clipFlags = clipFlags;

          if (Mesh::Manager::terrMrm && clus.viewOrigin.z >= Mesh::Manager::terrMrmDist)
          {
            mrmClusList[mrmClusCount] = &clus;
            mrmClusCount++;
            ASSERT( mrmClusCount < 1000);
          }
          else
          {
            SetClusMrm( off, TRUE);
            RenderCluster( clus, clus.x, clus.z, off, 1, 1, clipFlags);
          }
        }
      }
    }
#if 0
    else
    {
      cRect = vRect;

      // reflect terrain to outside map
      // re-convert to meters
      vRect.t *= meterPerClus;
      vRect.b *= meterPerClus;
      vRect.l *= meterPerClus;
      vRect.r *= meterPerClus;
      S32 z, x;
      for (z = vRect.t; z < vRect.b; z += meterPerClus)
      {
        for (x = vRect.l; x < vRect.r; x += meterPerClus)
        {
          S32 reflect = 0;
          S32 cellStrideX = 1;
          S32 cellStrideZ = 1;

          F32 coz = (F32) z * clusPerMeter;
          F32 cox = (F32) x * clusPerMeter;

          U16 cw = Utils::FP::SetTruncMode();

          S32 clusOffz = Utils::FastFtoL( coz);
          S32 clusOffx = Utils::FastFtoL( cox);

          Bool renderNormal = TRUE;

          if (clusOffz < 0)
          {
            clusOffz += 1;
            clusOffz = (-clusOffz) % clusHeight;
            coz += 1.0f;
            S32 v = Utils::FastFtoL(coz * invClusHgt + 0.0001f);
            if (!(v & 0x01))
            {
              reflect = heightField.cellWidth;
              cellStrideZ = -1;
            }
            else
            {
              clusOffz = clusHeight - 1 - clusOffz;
            }
            renderNormal = FALSE;
          }
          else if (clusOffz >= (S32) clusHeight)
          {
            S32 v = Utils::FastFtoL(coz * invClusHgt + 0.0001f);
            clusOffz %= clusHeight;
            if ((v & 0x01))
            {
              reflect = heightField.cellWidth;
              clusOffz = clusHeight - 1 - clusOffz;
              cellStrideZ = -1;
            }
            renderNormal = FALSE;
          }
          if (clusOffx < 0)
          {
            clusOffx += 1;
            clusOffx = (-clusOffx) % clusWidth;
            cox += 1.0f;
            S32 v = Utils::FastFtoL(cox * invClusWid + 0.0001f);
            if (!(v & 0x01))
            {
              reflect = cellStrideZ < 0 ? heightField.cellWidth + 1 : 1;
              cellStrideX = -1;
            }
            else 
            {
              clusOffx = clusWidth - 1 - clusOffx;
            }
            renderNormal = FALSE;
          }
          else if (clusOffx >= (S32) clusWidth)
          {
            clusOffx %= clusWidth;
            S32 v = Utils::FastFtoL(cox * invClusWid + 0.0001f);
            if ((v & 0x01))
            {
              reflect = cellStrideZ < 0 ? heightField.cellWidth + 1 : 1;
              clusOffx = clusWidth - 1 - clusOffx;
              cellStrideX = -1;
            }
            renderNormal = FALSE;
          }

          Utils::FP::RestoreMode(cw);

          Cluster &clus = clusList[clusOffz * clusWidth + clusOffx];

          Sphere sphere = clus.sphere;
          Vector o = sphere.Origin();
          o.x = (F32)x + (F32)meterPerClus * 0.5f;
          o.z = (F32)z + (F32)meterPerClus * 0.5f;
          sphere.SetOrigin( o);

          U32 clipFlags = Vid::CurCamera().SphereTest( sphere, &clus.viewOrigin);
          if (clipFlags == clipOUTSIDE)
          {
            // cluster is completely outside the view frustrum
            continue;
          }
          U32 off = clusOffz * clusWidth * cellPerClus * cellPerClus + clusOffx * cellPerClus + reflect;

          if (cellStrideX < 0)
          {
            if (cellStrideZ < 0)
            {
              off += cellPerClus * clusWidth * (cellPerClus - 1) + cellPerClus - 1;
            }
            else
            {
              off += cellPerClus - 1;
            }
          }
          else if (cellStrideZ < 0)
          {
            off += cellPerClus * clusWidth * (cellPerClus - 1);
          }

          clus.offset = off;
          clus.x = x;
          clus.z = z;
          clus.clipFlags = clipFlags;

    #ifdef DOBLACKTERRAIN
          if (renderNormal)
          {
  //          if (!DxLight::Manager::quickLight)
  //          {  //        Vid::SetModelViewVector( sphere.Origin());
              DxLight::Manager::SetActiveList( sphere);
              DxLight::Manager::SetupLightsModelSpace();
  //          }
            RenderCluster( clus, x, z, off, cellStrideX, cellStrideZ, clipFlags);
          }
          else
          {
            RenderClusterColor( clus, x, z, off, cellStrideX, cellStrideZ, offMapColor, clipFlags);
          }
    #else
    //      Vid::SetModelViewVector( sphere.Origin());
          DxLight::Manager::SetActiveList( sphere);
          DxLight::Manager::SetupLightsModelSpace();

          if (Mesh::Manager::terrMrm && clus.viewOrigin.z >= Mesh::Manager::terrMrmDist)
          {
            SetClusMrm( off, FALSE);
            mrmClusList[mrmClusCount] = &clus;
            mrmClusCount++;
            ASSERT( mrmClusCount < 1000);
          }
          else
          {
            RenderCluster( clus, x, z, off, cellStrideX, cellStrideZ, clipFlags);
          }
    #endif
        }
      }
    }
#endif

    if (Mesh::Manager::terrMrm)
    {
      Cluster **c  = &mrmClusList[0];
      Cluster **ce = &mrmClusList[mrmClusCount];

      for ( ; c < ce; c++)
      {
        Cluster &clus = **c;

        F32 py = (F32)fabs(Vid::CurCamera().RHW( clus.viewOrigin.z) * Vid::CurCamera().ProjConstY());

        U32 o0 = clus.offset;
        U32 o1 = o0 + 4 * CellPitch();
        U32 o2 = o0 + 4;
        U32 o3 = o1 + 4;

        ChekTri( py, o0, o1, o2);
        ChekTri( py, o3, o2, o1);
      }

      for (c = ce - 1; c >= mrmClusList; c--)
      {
        Cluster &clus = **c;
        RenderClusterMRM( clus, clus.x, clus.z, clus.offset, clus.clipFlags);
      }
    }

  #ifdef DOSTATISTICS
    Statistics::terrainTris = Statistics::tempTris;
  #endif
  }
  //----------------------------------------------------------------------------

  Bool ChekTri( F32 py, U32 o0, U32 o1, U32 o2, U32 level) // = 0
  {
    U32 o3 = S32((o1 + o2) >> 1);

    if (level < 3)
    {
      level++;

      ChekTri( py, o3, o0, o1, level);
      ChekTri( py, o3, o2, o0, level);
    }
    Cell &c1 = heightField.cellList[o1];
    Cell &c2 = heightField.cellList[o2];
    Cell &c3 = heightField.cellList[o3];

    if ((F32) fabs( c3.height * 2.0f - c1.height - c2.height) * py > Mesh::Manager::terrMrmThresh)
    {
      c3.Activate();

      return TRUE;
    }
    return FALSE;
  }
  //----------------------------------------------------------------------------

  Bool DrawTri( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, Vector *verts, U16 **indexList, U32 level) // = 0
  {
    Bool ret = FALSE;

    if (level < 4)
    {
      S32 o3 = S32((o1 + o2) >> 1);
      S32 i3 = S32((i1 + i2) >> 1);
      level++;

      ret |= DrawTri( o3, o0, o1, i3, i0, i1, verts, indexList, level);
      ret |= DrawTri( o3, o2, o0, i3, i2, i0, verts, indexList, level);
    }

    Cell &c0 = heightField.cellList[o0];

    if ((!ret && (c0.flags & Cell::cellMRM)))
    {
      Plane plane;
      plane.Set( verts[i0], verts[i1], verts[i2]);

      // backcull
  	  if (plane.Evalue(Vid::model_view_vector) > 0.0f)
      {
        U16 *il = *indexList;

        il[0] = U16(i0);
        il[1] = U16(i1);
        il[2] = U16(i2);

        (*indexList) += 3;
      }
      return TRUE;
    }
    return ret;
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterMRM( Cluster &clus, S32 x, S32 z, U32 cellOffset, U32 clipFlags) // = clipALL
  {
    Camera &cam = Vid::CurCamera();

//    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);

    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    Cell *c0 = &heightField.cellList[cellOffset];

    Vector verts[25];
    Vector norms[25];
    Color colors[25];
    F32     fogs[25];

    U32 meterStride = MeterPerCell();
    U32 cellStride  = CellPitch();

    VertexTL *tempvertmem = Vid::tempVertices;

    VertexTL *dvv = tempvertmem;
    Vector *dv = verts;
    Vector *dn = norms;
    Color  *dc = colors;
    F32    *df = fogs;
    F32 u, v;
    for (z0 = z, v = 0.0f; z0 <= zend; z0 += meterStride, c0 += cellStride, v += 1.0f)
    {
      Cell *c = c0;
      for (x0 = x, u = 0.0f; x0 <= xend; x0 += meterStride, dv++, dn++, dc++, df++, c += 1, dvv++, u += 1.0f)
      {
        if (shroud)
        {
          *df = fogFactors[c->GetFog()];
        }
        else
        {
          *df = 1.0f;
        }

        dv->x = (F32) x0;
        dv->z = (F32) z0;
        dv->y = c->height;

        *dn = normList[c->normal];

        *dc = c->color;

        dvv->u = u;
        dvv->v = v;
      }
    }

    // reset loop variables
    c0 = &heightField.cellList[cellOffset];

    // submit cells
    U32 vcount = dv - verts;
    cam.LightSetHomogeneousVerts( tempvertmem, verts, norms, colors, fogs, vcount);

    U32 o0 = cellOffset;
    U32 o1 = o0 + 4 * CellPitch();
    U32 o2 = o0 + 4;
    U32 o3 = o1 + 4;

    U16 indexlist[96];
    U16 *il = indexlist;
    DrawTri( o0, o1, o2,  0, 20,  4, verts, &il);
    DrawTri( o3, o2, o1, 24,  4, 20, verts, &il);

    Cell &c = heightField.cellList[o0];
    Bitmap *tex = texList[c.texture];
    Vid::SetBucketTexture( tex);

    Vid::CurCamera().ClipToBucket( tempvertmem, vcount, indexlist, il - indexlist);

	  if (clus.status.water && *water)
    {
      // one quad covers the whole cluster with water
      Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
      Vid::SetBucketTexture( waterTex);
      Vid::SetTranslucent( TRUE);

      Vid::SetTranBucketZ( farPlane);
      RenderCell( NULL, x, z, xend, zend, clus.waterHeight, waterUVList0, clipFlags);

      Vid::SetTranBucketZ( farPlane + 100.0f);
      RenderCell( NULL, x, z, xend, zend, clus.waterHeight, waterUVList1, clipFlags);
	  }
  }
  //----------------------------------------------------------------------------


  // draw a single cluster
  //
  void RenderCluster( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, U32 clipFlags) // = clipALL
  {
    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

    Camera &cam = Vid::CurCamera();

//    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF | RS_TEXCLAMP);

    // build list of vertex positions, normals, and colors
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

  #ifdef DOCLUSTERCELLS
    Cell *c = clus.cells;

    Vector verts[25];
    Vector norms[25];
    Color colors[25];
    F32     fogs[25];

    Vector *dv = verts;
    Vector *dn = norms;
    Color  *dc = colors;
    F32    *df = fogs;
    for (z0 = z; z0 <= zend; z0 += meterStrideZ)
    {
      for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, dn++, dc++, df++, c += 1)
      {
        if (Common::Display::shroud)
        {
          *df = fogFactors[c->GetFog()];
        }
        else
        {
          *df = 1.0f;
        }

        dv->x = (F32) x0;
        dv->z = (F32) z0;
        dv->y = c->height;

        *dn = normList[c->normal];

        *dc = c->color;
      }
    }

    // reset loop variables
    c = clus.cells;

    // submit cells
    VertexTL *tempvertmem = Vid::tempVertices;

    U32 vcount = dv - verts;

    U32 indexer[] = { 1, 0, 5, 6, 1 };
    S32 flip = 1;
    if (clipFlags == clipNONE)
    {
      cam.LightProjectVerts( tempvertmem, verts, norms, colors, fogs, vcount);

      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, flip = abs(flip - 1))
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += 1, flip = abs(flip - 1))
        {
          if (!c->GetVisible())
          {
            continue;
          }

          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
          U16 iv[4];
          iv[0] = (U16)(vcount + indexer[flip + 0]);
          iv[1] = (U16)(vcount + indexer[flip + 1]);
          iv[2] = (U16)(vcount + indexer[flip + 2]);
          iv[3] = (U16)(vcount + indexer[flip + 3]);

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }

          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex);

          VertexTL * vertmem;
          U16 *      indexmem;
          if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
          {
            LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
            return;
          }

          UVPair *uvList = &cellUVList[c->uv];

          U32 i;
          for (i = 0; i < 4; i++)
          {
            VertexTL &dv = vertmem[i];
            const VertexTL &sv = tempvertmem[iv[i]];

            dv.vv  = sv.vv;
            dv.rhw = sv.rhw;
            dv.diffuse  = sv.diffuse;
            dv.specular = sv.specular;

            dv.uv  = uvList[i];
          }
          Utils::Memcpy( indexmem, Vid::rectIndices, 12);

          Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
          Statistics::tempTris += 2;
  #endif
        }
      }
    }
    else
    {
      cam.LightSetHomogeneousVerts( tempvertmem, verts, norms, colors, fogs, vcount);

      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, flip = abs(flip - 1))
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += 1, flip = abs(flip - 1))
        {
          if (!c->GetVisible())
          {
            continue;
          }
          U16 iv[4];
          iv[0] = (U16)(vcount + indexer[flip + 0]);
          iv[1] = (U16)(vcount + indexer[flip + 1]);
          iv[2] = (U16)(vcount + indexer[flip + 2]);
          iv[3] = (U16)(vcount + indexer[flip + 3]);

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex);

          VertexTL vertmem[4];
          UVPair *uvList = &cellUVList[c->uv];

          U32 i;
          for (i = 0; i < 4; i++)
          {
            VertexTL &dv = vertmem[i];
            VertexTL &sv = tempvertmem[iv[i]];
          
            dv.vv  = sv.vv;
            dv.rhw = sv.rhw;
            dv.diffuse = sv.diffuse;
            dv.specular = sv.specular;

            dv.uv  = uvList[i];
          }

          Vid::CurCamera().ClipToBucket( vertmem, 4, Vid::rectIndices, 6);
        }
      }
    }

  #else

    Cell *c0 = &heightField.cellList[cellOffset];

    Vector verts[25];
    Vector norms[25];
    Color colors[25];
    F32     fogs[25];

    Vector *dv = verts;
    Vector *dn = norms;
    Color  *dc = colors;
    F32    *df = fogs;
    for (z0 = z; z0 <= zend; z0 += meterStrideZ, c0 += cellStrideWidth)
    {
      Cell *c = c0;
      for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, dn++, dc++, df++, c += cellStrideX)
      {
        if (shroud)
        {
          *df = fogFactors[c->GetFog()];
        }
        else
        {
          *df = 1.0f;
        }

        dv->x = (F32) x0;
        dv->z = (F32) z0;
        dv->y = c->height;

        *dn = normList[c->normal];

        *dc = c->color;
      }
    }

    // reset loop variables
    c0 = &heightField.cellList[cellOffset];

    // submit cells
    VertexTL *tempvertmem = Vid::tempVertices;

    U32 vcount = dv - verts;

  #ifdef DOFLIPTRI
    U32 indexer[] = { 1, 0, 5, 6, 1 };
    S32 flip = 1;
  #endif

    if (clipFlags == clipNONE)
    {
      cam.LightProjectVerts( tempvertmem, verts, norms, colors, fogs, vcount);

  #ifdef DOFLIPTRI
      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth, flip = abs(flip - 1))
      {
        Cell *c = c0;
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX, flip = abs(flip - 1))
  #else
      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
      {
        Cell *c = c0;
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
  #endif
        {
          if (!c->GetVisible())
          {
            continue;
          }

          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
          U16 iv[4];

  #ifdef DOFLIPTRI
          iv[0] = (U16)(vcount + indexer[flip + 0]);
          iv[1] = (U16)(vcount + indexer[flip + 1]);
          iv[2] = (U16)(vcount + indexer[flip + 2]);
          iv[3] = (U16)(vcount + indexer[flip + 3]);
  #else
          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[3] = (U16)(vcount + 1);
  #endif

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }

          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex);

          VertexTL * vertmem;
          U16 *      indexmem;
          if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
          {
            LOG_WARN( ("Terrain::RenderCluster: can't lock bucket!") );
            return;
          }

          UVPair *uvList = &cellUVList[c->uv];

          U32 i;
          for (i = 0; i < 4; i++)
          {
            VertexTL &dv = vertmem[i];
            const VertexTL &sv = tempvertmem[iv[i]];

            dv.vv  = sv.vv;
            dv.rhw = sv.rhw;
            dv.diffuse  = sv.diffuse;
            dv.specular = sv.specular;

            dv.uv  = uvList[i];
          }
          Utils::Memcpy( indexmem, Vid::rectIndices, 12);

          Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
          Statistics::tempTris += 2;
  #endif
        }
      }
    }
    else
    {
      cam.LightSetHomogeneousVerts( tempvertmem, verts, norms, colors, fogs, vcount);

  #ifdef DOFLIPTRI
      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth, flip = abs(flip - 1))
      {
        Cell *c = c0;
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX, flip = abs(flip - 1))
  #else
      for (z0 = z, vcount = 0; z0 < zend; z0 += meterStrideZ, vcount++, c0 += cellStrideWidth)
      {
        Cell *c = c0;
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, c += cellStrideX)
  #endif
        {
          if (!c->GetVisible())
          {
            continue;
          }
          U16 iv[4];

  #ifdef DOFLIPTRI
          iv[0] = (U16)(vcount + indexer[flip + 0]);
          iv[1] = (U16)(vcount + indexer[flip + 1]);
          iv[2] = (U16)(vcount + indexer[flip + 2]);
          iv[3] = (U16)(vcount + indexer[flip + 3]);
  #else
          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[3] = (U16)(vcount + 1);
  #endif

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[3]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          Bitmap *tex = texList[c->texture];
          Vid::SetBucketTexture( tex);

          VertexTL vertmem[4];
          UVPair *uvList = &cellUVList[c->uv];

          U32 i;
          for (i = 0; i < 4; i++)
          {
            VertexTL &dv = vertmem[i];
            VertexTL &sv = tempvertmem[iv[i]];
          
            dv.vv  = sv.vv;
            dv.rhw = sv.rhw;
            dv.diffuse = sv.diffuse;
            dv.specular = sv.specular;

            dv.uv  = uvList[i];
          }

          Vid::CurCamera().ClipToBucket( vertmem, 4, Vid::rectIndices, 6);
        }
      }
    }
  #endif

	  if (clus.status.water && *water)
    {
      // one quad covers the whole cluster with water
      Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
      Vid::SetBucketTexture( waterTex);
      Vid::SetTranslucent( TRUE);

      Vid::SetTranBucketZ( farPlane);
  //    Vid::SetBucketFlags( bucketFlags | RS_SRC_ONE | RS_DST_ONE | RS_TEX_MODULATE | RS_NOFOG);
      RenderCell( NULL, x, z, xend, zend, clus.waterHeight, waterUVList0, clipFlags);

      Vid::SetTranBucketZ( farPlane + 100.0f);
  //    Vid::SetBucketFlags( bucketFlags | RS_SRC_DSTCOLOR | RS_DST_SRCCOLOR | RS_TEX_MODULATE);
      RenderCell( NULL, x, z, xend, zend, clus.waterHeight, waterUVList1, clipFlags);
	  }

  #if 0
    if (Mesh::Manager::doBounds)
    {
      clus.sphere.Render();
    }
  #endif
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterColor( Cluster &clus, S32 x, S32 z, U32 cellOffset, S32 cellStrideX, S32 cellStrideZ, Color color, U32 clipFlags) // = 0
  {
    clus;

    Camera &cam = Vid::CurCamera();

    S32 meterStrideX    = abs( cellStrideX) * heightField.meterPerCell;
    S32 meterStrideZ    = abs( cellStrideZ) * heightField.meterPerCell;
    S32 cellStrideWidth = cellStrideZ * heightField.cellPitch;

//    Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
    Vid::SetBucketTexture(defTex);

    // fill the vertex memory
    S32 x0, xend = x + meterPerClus;
    S32 z0, zend = z + meterPerClus;

    Vector verts[25];

  #ifdef DOCLUSTERCELLS

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96))
      {
        return;
      }

      Cell *c = clus.cells;
      VertexTL *dv = vertmem;
      Vector *sv = verts;
      F32 vv = 0.0f;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, vv += 1.0f)
      {
        F32 uu = 0.0f;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, sv++, uu += 1.0f, c += 1)
        {
          sv->x = (F32) x0;
          sv->z = (F32) z0;
          sv->y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

          cam.TransformProjectFromModelSpace( *dv, *sv);

          dv->SetFog();
        }
      }

      U16 *ii = indexmem;
      U32 vcount = 0;
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++)
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++)
        {
          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
          ii[0] = (U16)(vcount);
          ii[1] = (U16)(vcount + 5);
          ii[2] = (U16)(vcount + 6);
          ii[5] = (U16)(vcount + 1);

          Plane planes[2];
          planes[0].Set( verts[ii[0]], verts[ii[1]], verts[ii[2]]);
          planes[1].Set( verts[ii[0]], verts[ii[2]], verts[ii[5]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          // set indices
          ii[3] = ii[0];
          ii[4] = ii[2];

          ii += 6;

  #ifdef DOSTATISTICS
          Statistics::tempTris += 2;
  #endif
        }
      }

      Vid::UnlockIndexedPrimitiveMem( dv - vertmem, ii - indexmem);
    }
    else
    {
      VertexTL *vertmem = Vid::tempVertices;
      U16 *indexmem = Vid::tempIndices;

      Cell *c = clus.cells;
      VertexTL *dv = vertmem;
      Vector *sv = verts;
      F32 vv = 0.0f;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, vv += 1.0f)
      {
        F32 uu = 0.0f;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, sv++, uu += 1.0f, c += 1)
        {
          sv->x = (F32) x0;
          sv->z = (F32) z0;
          sv->y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

          cam.SetHomogeneousFromModelSpace(*dv, *sv);
        }
      }

      U16 *ii = indexmem;
      U32 vcount = 0;
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++)
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++)
        {
          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
          ii[0] = (U16)(vcount);
          ii[1] = (U16)(vcount + 5);
          ii[2] = (U16)(vcount + 6);
          ii[5] = (U16)(vcount + 1);

          Plane planes[2];
          planes[0].Set( verts[ii[0]], verts[ii[1]], verts[ii[2]]);
          planes[1].Set( verts[ii[0]], verts[ii[2]], verts[ii[5]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          // set indices
          ii[3] = ii[0];
          ii[4] = ii[2];

          ii += 6;
        }
      }
      cam.ClipToBucket( vertmem, dv - vertmem, indexmem, ii - indexmem);
    }

  #else

  #ifdef DOFLIPTRI
    U32 indexer[] = { 1, 0, 5, 6, 1 };
    S32 flip = 1;
  #endif

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 25, &indexmem, 96))
      {
        return;
      }

      Cell *c0 = &heightField.cellList[cellOffset];
      VertexTL *dv = vertmem;
      Vector *sv = verts;
      F32 vv = 0.0f;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, vv += 1.0f, c0 += cellStrideWidth)
      {
        Cell *c = c0;

        F32 uu = 0.0f;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, sv++, uu += 1.0f, c += cellStrideX)
        {
          sv->x = (F32) x0;
          sv->z = (F32) z0;
          sv->y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

          cam.TransformProjectFromModelSpace( *dv, *sv);

          dv->SetFog();
        }
      }

      U16 *iv = indexmem;
      U32 vcount = 0;
  #ifdef DOFLIPTRI
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++, flip = abs(flip - 1))
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, flip = abs(flip - 1))
  #else
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++)
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++)
  #endif
        {
          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //

  #ifdef DOFLIPTRI
          iv[0] = (U16)(vcount + indexer[flip + 0]);
          iv[1] = (U16)(vcount + indexer[flip + 1]);
          iv[2] = (U16)(vcount + indexer[flip + 2]);
          iv[5] = (U16)(vcount + indexer[flip + 3]);
  #else
          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[5] = (U16)(vcount + 1);
  #endif

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[5]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          // set indices
          iv[3] = iv[0];
          iv[4] = iv[2];

          iv += 6;

  #ifdef DOSTATISTICS
          Statistics::tempTris += 2;
  #endif
        }
      }

      Vid::UnlockIndexedPrimitiveMem( dv - vertmem, iv - indexmem);
    }
    else
    {
      VertexTL *vertmem = Vid::tempVertices;
      U16 *indexmem = Vid::tempIndices;

      Cell *c0 = &heightField.cellList[cellOffset];
      VertexTL *dv = vertmem;
      Vector *sv = verts;
      F32 vv = 0.0f;
      for (z0 = z; z0 <= zend; z0 += meterStrideZ, vv += 1.0f, c0 += cellStrideWidth)
      {
        Cell *c = c0;

        F32 uu = 0.0f;
        for (x0 = x; x0 <= xend; x0 += meterStrideX, dv++, sv++, uu += 1.0f, c += cellStrideX)
        {
          sv->x = (F32) x0;
          sv->z = (F32) z0;
          sv->y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

          cam.SetHomogeneousFromModelSpace(*dv, *sv);
        }
      }

      U16 *iv = indexmem;
      U32 vcount = 0;
  #ifdef DOFLIPTRI
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++, flip = abs(flip - 1))
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++, flip = abs(flip - 1))
  #else
      for (z0 = z; z0 < zend; z0 += meterStrideZ, vcount++)
      {
        for (x0 = x; x0 < xend; x0 += meterStrideX, vcount++)
  #endif
        {
          // 0*\--*3
          //  | \ |
          // 1*--\*2
          //
  #ifdef DOFLIPTRI
          iv[0] = (U16)(vcount + indexer[flip + 0]);
          iv[1] = (U16)(vcount + indexer[flip + 1]);
          iv[2] = (U16)(vcount + indexer[flip + 2]);
          iv[5] = (U16)(vcount + indexer[flip + 3]);
  #else
          iv[0] = (U16)(vcount);
          iv[1] = (U16)(vcount + 5);
          iv[2] = (U16)(vcount + 6);
          iv[5] = (U16)(vcount + 1);
  #endif

          Plane planes[2];
          planes[0].Set( verts[iv[0]], verts[iv[1]], verts[iv[2]]);
          planes[1].Set( verts[iv[0]], verts[iv[2]], verts[iv[5]]);

          // backcull
	  	    if (planes[0].Evalue(Vid::model_view_vector) <= 0.0f
	  	     && planes[1].Evalue(Vid::model_view_vector) <= 0.0f)
          {
            continue;
          }
          // set indices
          iv[3] = iv[0];
          iv[4] = iv[2];

          iv += 6;
        }
      }
      cam.ClipToBucket( vertmem, dv - vertmem, indexmem, iv - indexmem);
    }
  #endif
  }
  //----------------------------------------------------------------------------

  // draw a rectangle of cells; used by the edit brush
  //
  void RenderCellRect( Rect &rect, Color color, Bool dofill, Bitmap *tex) // = FALSE, = NULL)
  {
    ASSERT( rect.l >= 0 && rect.r <= (S32) heightField.cellWidth 
         && rect.t >= 0 && rect.b <= (S32) heightField.cellHeight);

    if (!tex)
    {
      // use the default edit texture
      tex = editTex;
    }

    Camera &cam = Vid::CurCamera();

    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetTranBucketZ( 100.0f);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( NULL);

    S32 wid = rect.Width();
    S32 hgt = rect.Height();

  #define MAXCELLS      ((MAXBRUSHX + 1) * (MAXBRUSHZ + 1))
    ASSERT( (wid + 1) * (hgt + 1) <= MAXVERTS && (wid) * (hgt) * 6 <= MAXINDICES);

    S32 meterStride = heightField.meterPerCell;
    S32 x = rect.l * meterStride + (S32)OffsetX();
    S32 z = rect.t * meterStride + (S32)OffsetZ();
    S32 xend = x + wid * meterStride;
    S32 zend = z + hgt * meterStride;
    Cell *c0;
    S32 x0, x1, z0, z1;
    S32 cellOffset = rect.t * heightField.cellPitch + rect.l;

    wid += 1;
    hgt += 1;

    if (Mesh::Manager::doNormals)
    {
      Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_DEF);
  
      S32 offset = cellOffset;

      for (z0 = z; z0 <= zend; z0 += meterStride)
      {
        c0 = &heightField.cellList[offset];
        for (x0 = x; x0 <= xend; x0 += meterStride)
        {
          if (x0 >= 0 && x0 < (S32) heightField.meterWidth && z0 >= 0 && z0 < (S32) heightField.meterHeight)
          {
            RenderCellNormal( *c0, (F32) x0, (F32) z0);
          }

          c0 += 1;
        }
        offset += heightField.cellPitch;
      }

      Vid::SetBucketFlags( DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF);
    }

    Vid::SetBucketTexture( tex);
    if (dofill)
    {
      Cell *c0 = &heightField.cellList[cellOffset];

      VertexTL *vertmem = Vid::tempVertices;
      U16 *indexmem = Vid::tempIndices;

      U16 *iv = indexmem;
      VertexTL *dv = vertmem;
      F32 vv = 0.0f;
      U32 vcount = 0;

  #ifdef DOFLIPTRI
      U32 indexer[] = { 1, 0, 5, 6, 1 };
      indexer[2] = wid;
      indexer[3] = wid + 1;
      S32 flip = z % 2 ? (x % 2 ? 1 : 0) : (x % 2 ? 0 : 1);

      Bool xFlip = wid % 2 ? TRUE : FALSE;
  #endif

      for (z0 = z; z0 < zend; z0 += meterStride, vv += 1.0f, vcount++)
      {
        Cell *c = c0;
        F32 uu = 0.0f;

  #ifdef DOFLIPTRI
        for (x0 = x; x0 < xend; x0 += meterStride, dv++, uu += 1.0f, c += 1, vcount++, flip = abs(flip - 1))
  #else
        for (x0 = x; x0 < xend; x0 += meterStride, dv++, uu += 1.0f, c += 1, vcount++)
  #endif
        {
          dv->vv.x = (F32) x0;
          dv->vv.z = (F32) z0;
          dv->vv.y = c->height;
          dv->diffuse  = color;
          dv->specular = 0xff000000;
          dv->uv.u = uu;
          dv->uv.v = vv;

          cam.SetHomogeneousFromModelSpace(*dv);

          dv->vv.z *= Mesh::Manager::zBias;

          if (x0 >= 0 && x0 < (S32) heightField.meterWidth && z0 >= 0 && z0 < (S32) heightField.meterHeight)
          {
            // 0*\--*3
            //  | \ |
            // 1*--\*2
            //
  #ifdef DOFLIPTRI
            iv[0] = (U16)(vcount + indexer[flip + 0]);
            iv[1] = (U16)(vcount + indexer[flip + 1]);
            iv[2] = (U16)(vcount + indexer[flip + 2]);
            iv[5] = (U16)(vcount + indexer[flip + 3]);
  #else
            iv[0] = (U16)(vcount);
            iv[1] = (U16)(vcount + wid);
            iv[2] = (U16)(vcount + wid + 1);
            iv[5] = (U16)(vcount + 1);
  #endif
            iv[3] = iv[0];
            iv[4] = iv[2];

            iv += 6;
          }
        }
  #ifdef DOFLIPTRI
        if (xFlip)
        {
          flip = abs(flip - 1);
        }
  #endif

        dv->vv.x = (F32) x0;
        dv->vv.z = (F32) z0;
        dv->vv.y = c->height;
        dv->diffuse  = color;
        dv->specular = 0xff000000;
        dv->uv.u = uu;
        dv->uv.v = vv;

        cam.SetHomogeneousFromModelSpace(*dv);

        dv->vv.z *= Mesh::Manager::zBias;

        dv++;
      
        c0 += heightField.cellPitch;
      }

      Cell *c = c0;
      F32 uu = 0.0f;
      for (x0 = x; x0 <= xend; x0 += meterStride, dv++, uu += 1.0f, c += 1)
      {
        dv->vv.x = (F32) x0;
        dv->vv.z = (F32) z0;
        dv->vv.y = c->height;
        dv->diffuse  = color;
        dv->specular = 0xff000000;
        dv->uv.u = uu;
        dv->uv.v = vv;

        cam.SetHomogeneousFromModelSpace(*dv);

        dv->vv.z *= Mesh::Manager::zBias;
      }

      cam.ClipToBucket( vertmem, dv - vertmem, indexmem, iv - indexmem);
    }
    else
    {
      // top cell strip
      c0 = &heightField.cellList[cellOffset];
      Cell *c1 = c0 + heightField.cellPitch;
      Cell *c2 = c1 + 1;
      Cell *c3 = c2 - heightField.cellPitch;

      z0 = z;
      z1 = z + meterStride;
      for (x0 = x, x1 = x + meterStride; x0 < xend; x0 += meterStride, x1 += meterStride)
      {
        RenderCell( *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += 1;
        c1 += 1;
        c2 += 1;
        c3 += 1;
      }

      // bottom cell strip
      z0 = zend - meterStride;
      z1 = zend;
      c0 = &heightField.cellList[cellOffset + (hgt - 1) * heightField.cellPitch];
      c1 = c0 + heightField.cellPitch;
      c2 = c1 + 1;
      c3 = c2 - heightField.cellPitch;
      for (x0 = x, x1 = x + meterStride; x0 < xend; x0 += meterStride, x1 += meterStride)
      {
        RenderCell( *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += 1;
        c1 += 1;
        c2 += 1;
        c3 += 1;
      }

      // left cell strip
      x0 = x;
      x1 = x + meterStride;
      z += meterStride;
      zend -= meterStride;
      c0 = &heightField.cellList[cellOffset + heightField.cellPitch];
      c1 = c0 + heightField.cellPitch;
      c2 = c1 + 1;
      c3 = c2 - heightField.cellPitch;
      for (z0 = z, z1 = z + meterStride; z0 < zend; z0 += meterStride, z1 += meterStride)
      {
        RenderCell( *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += heightField.cellPitch;
        c1 += heightField.cellPitch;
        c2 += heightField.cellPitch;
        c3 += heightField.cellPitch;
      }

      // right cell strip
      x0 = xend - meterStride;
      x1 = xend;
      c0 = &heightField.cellList[cellOffset + heightField.cellPitch + wid - 1];
      c1 = c0 + heightField.cellPitch;
      c2 = c1 + 1;
      c3 = c2 - heightField.cellPitch;
      for (z0 = z, z1 = z + meterStride; z0 < zend; z0 += meterStride, z1 += meterStride)
      {
        RenderCell( *c0, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);

        c0 += heightField.cellPitch;
        c1 += heightField.cellPitch;
        c2 += heightField.cellPitch;
        c3 += heightField.cellPitch;
      }
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cell
  //
  void RenderCell( Cell &c0, Cell &c1, Cell &c2, Cell &c3, S32 x0, S32 z0, S32 x1, S32 z1, Color color, U32 clipFlags)
  {
    Camera &cam = Vid::CurCamera();

    UVPair *uvList = &cellUVList[c0.uv];

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
      {
        return;
      }

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.z = (F32) z0;
      vertmem[0].vv.y = c0.height;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.z = (F32) z1;
      vertmem[1].vv.y = c1.height;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.z = (F32) z1;
      vertmem[2].vv.y = c2.height;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.z = (F32) z0;
      vertmem[3].vv.y = c3.height;

      U32 i;
      for (i = 0; i < 4; i++)
      {
        VertexTL &dv = vertmem[i];

        dv.uv = uvList[i];
        dv.diffuse = color;
        dv.specular = 0xff000000;

        cam.TransformProjectFromModelSpace( dv);
      }

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
      Statistics::tempTris += 2;
  #endif
    }
    else
    {
      VertexTL vertmem[4];

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.z = (F32) z0;
      vertmem[0].vv.y = c0.height;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.z = (F32) z1;
      vertmem[1].vv.y = c1.height;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.z = (F32) z1;
      vertmem[2].vv.y = c2.height;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.z = (F32) z0;
      vertmem[3].vv.y = c3.height;

      U32 i;
      for (i = 0; i < 4; i++)
      {
        VertexTL &dv = vertmem[i];

        dv.uv = uvList[i];
        dv.diffuse = color;
        dv.specular = 0xff000000;

        cam.SetHomogeneousFromModelSpace( dv);
      }

      cam.ClipToBucket( vertmem, 4, Vid::rectIndices, 6);
    }
  }
  //----------------------------------------------------------------------------

  // used by water; skip backface culling
  //
  void RenderCell( Cell **cc, S32 x0, S32 z0, S32 x1, S32 z1, F32 y, UVPair *uvList, U32 clipFlags)
  {
    cc;

    Camera &cam = Vid::CurCamera();

    if (clipFlags == clipNONE)
    {
      VertexTL *vertmem;
      U16 *indexmem;

      if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
      {
        return;
      }

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.z = (F32) z0;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.z = (F32) z1;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.z = (F32) z1;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.z = (F32) z0;

      Vector norms[4];
      U32 i;
      for (i = 0; i < 4; i++)
      {
        norms[i] = normList[0];
        vertmem[i].vv.y = y;
        vertmem[i].uv = uvList[i];
      }
      cam.LightProjectVerts( vertmem, norms, 4);

      Utils::Memcpy( indexmem, Vid::rectIndices, 12);

      Vid::UnlockIndexedPrimitiveMem( 4, 6);

  #ifdef DOSTATISTICS
      Statistics::tempTris += 2;
  #endif
    }
    else
    {
      VertexTL vertmem[4];

      vertmem[0].vv.x = (F32) x0;
      vertmem[0].vv.z = (F32) z0;

      vertmem[1].vv.x = (F32) x0;
      vertmem[1].vv.z = (F32) z1;

      vertmem[2].vv.x = (F32) x1;
      vertmem[2].vv.z = (F32) z1;

      vertmem[3].vv.x = (F32) x1;
      vertmem[3].vv.z = (F32) z0;

      Vector norms[4];
      U32 i;
      for (i = 0; i < 4; i++)
      {
        norms[i] = normList[0];
        vertmem[i].vv.y = y;
        vertmem[i].uv = uvList[i];
      }
      cam.LightSetHomogeneousVerts( vertmem, norms, 4);

      cam.ClipToBucket( vertmem, 4, Vid::rectIndices, 6);
    }
  }
  //----------------------------------------------------------------------------

  // draw a single cell; used for current 'click' location display
  //
  void RenderCell( Cell &cell, Color color, Bitmap *tex) // = NULL
  {
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_BLEND_DEF,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetTranBucketZ( 100.0f);
    Vid::SetBucketMaterial( Vid::defMaterial);
    Vid::SetBucketTexture( tex ? tex : texList[cell.texture]);

    U32 offset = (U32) (&cell - heightField.cellList);

    U32 z0 = (offset / heightField.cellPitch) + (S32)OffsetZ();
    U32 x0 = (offset - z0 * heightField.cellPitch) * heightField.meterPerCell + (S32)OffsetX();
    z0 *= heightField.meterPerCell;
    U32 z1 = z0 + heightField.meterPerCell;
    U32 x1 = x0 + heightField.meterPerCell;

    Cell *c1 = &cell + heightField.cellPitch;
    Cell *c2 = c1 + 1;
    Cell *c3 = c2 - heightField.cellPitch;

    RenderCell( cell, *c1, *c2, *c3, x0, z0, x1, z1, color, clipALL);
  }
  //----------------------------------------------------------------------------

  // draw a cell's normals; used by editing brush
  //
  void RenderCellNormal( Cell &cell, F32 x, F32 z)
  {
    static Color color = RGBA_MAKE( 0, 255, 255, 255);

    Camera &cam = Vid::CurCamera();

    F32 h = cell.height;
    Vector norm = normList[cell.normal];
    norm *= 2.0f;

#if 1
    // lock vertex memory
	  VertexTL *vertmem;
	  U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 3, &indexmem, 3))
	  {
      return;
    }

	  vertmem[0].vv.x = x;
	  vertmem[0].vv.y = h;
	  vertmem[0].vv.z = z;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;

    cam.TransformProjectFromModelSpace( vertmem[0]);

    vertmem[0].vv.z *= Mesh::Manager::zBias;

	  vertmem[1].vv.x = x + norm.x;
	  vertmem[1].vv.y = h + norm.y;
	  vertmem[1].vv.z = z + norm.z;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;

    cam.TransformProjectFromModelSpace( vertmem[1]);

    vertmem[1].vv.z *= Mesh::Manager::zBias;

	  vertmem[2] = vertmem[0];
    vertmem[0].vv.x -= 1.0f;
    vertmem[2].vv.x += 1.0f;

    Utils::Memcpy( indexmem, Vid::rectIndices, 6);

    // submit the polygons
    Vid::UnlockIndexedPrimitiveMem( 3, 3);
#else
    Vector normX;
    normX.x =  norm.y;
    normX.y = -norm.x;
    normX.z =  norm.z;
    normX  *=  0.1f;

    Vector normZ;
    normZ.x =  norm.x;
    normZ.y = -norm.z;
    normZ.z =  norm.y;
    normZ  *=  0.1f;

    VertexL vertmem[5];

	  vertmem[0].vv.x = x + norm.x;
	  vertmem[0].vv.y = h + norm.y;
	  vertmem[0].vv.z = z + norm.z;
	  vertmem[0].diffuse  = color;
	  vertmem[0].specular = 0xff000000;

//	  vertmem[1].vv.x = x + NORMDRAWWID;
	  vertmem[1].vv.x = x;
	  vertmem[1].vv.y = h;
	  vertmem[1].vv.z = z;
	  vertmem[1].diffuse  = color;
	  vertmem[1].specular = 0xff000000;
    vertmem[1].vv += normX;

//	  vertmem[2].vv.x = x - NORMDRAWWID;
    vertmem[2].vv.x = x;
	  vertmem[2].vv.y = h;
	  vertmem[2].vv.z = z;
	  vertmem[2].diffuse  = color;
	  vertmem[2].specular = 0xff000000;
    vertmem[2].vv -= normX;

	  vertmem[3].vv.x = x;
	  vertmem[3].vv.y = h;
//	  vertmem[3].vv.z = z + NORMDRAWWID;
    vertmem[3].vv.z = z;
	  vertmem[3].diffuse  = color;
	  vertmem[3].specular = 0xff000000;
    vertmem[3].vv += normZ;

	  vertmem[4].vv.x = x;
	  vertmem[4].vv.y = h;
//	  vertmem[4].vv.z = z - NORMDRAWWID;
    vertmem[4].vv.z = z;
	  vertmem[4].diffuse  = color;
	  vertmem[4].specular = 0xff000000;
    vertmem[4].vv -= normZ;

    static U16 normalIndexMem[] = { 0, 1, 2, 0, 3, 4 };

//    F32 zbias = *Mesh::Manager::zBias;
//    Mesh::Manager::zBias = *Mesh::Manager::zBias * 0.9f;

//    Vid::CurCamera().ProjectClip( NULL, vertmem, 5, normalIndexMem, 6);
    Vid::CurCamera().ProjectClipBias( NULL, vertmem, 5, normalIndexMem, 6);

//    Mesh::Manager::zBias = zbias;
#endif
  }
  //----------------------------------------------------------------------------

  // recalculate a cluster's bounding sphere
  // FIXME : for occlusion
  //
  void CalcClusSphere( S32 x, S32 z)
  {
    Cluster &clus = clusList[ z * clusWidth + x];
    Vector o;

    U32 offset, cellOffset = z * cellPerClus * heightField.cellPitch + x * cellPerClus;
    Vector points[289];   // 17 * 17 (cells per cluster)
    x *= meterPerClus;
    z *= meterPerClus;
    x += (S32) OffsetX();
    z += (S32) OffsetZ();

    clus.offset = cellOffset;

    U32 halfMeters = meterPerClus >> 1;
    o.y = 0.0;
    o.x =  (F32) (x + halfMeters);
    o.z =  (F32) (z + halfMeters);

    S32 zend = z + meterPerClus;
    S32 xend = x + meterPerClus;
    S32 ix;
    Vector *p;

    Bool waterVisible = FALSE;

    for (p = points; z <= zend; z += heightField.meterPerCell, cellOffset += heightField.cellPitch)
    {
      offset = cellOffset;
  
      for (ix = x; ix <= xend; ix += heightField.meterPerCell, offset++, p++)
      {
        p->x = (F32) ix;
        p->z = (F32) z;
        p->y = heightField.cellList[offset].height;

        o.y += p->y;

        if (p->y < clus.waterHeight)
        {
          waterVisible = TRUE;
        }
      }
    }

    // Turn off water if not visible anymore
    if (!waterVisible)
    {
      clus.status.water = FALSE;
    }

  #if 1
    U16 indexlist[] = {
      0, 5, 6, 0, 6, 1,
      1, 6, 7, 1, 7, 2,
      2, 7, 8, 2, 8, 3, 
      3, 8, 9, 3, 0, 4,

      5, 10, 11, 5, 11, 6,
      6, 11, 12, 6, 12, 7,
      7, 12, 13, 7, 13, 8,
      8, 13, 14, 8, 14, 9,

      10, 15, 16, 10, 16, 11,
      11, 16, 17, 11, 17, 12,
      12, 17, 18, 12, 18, 13,
      13, 18, 19, 13, 19, 14,

      15, 20, 21, 15, 21, 16,
      16, 21, 22, 16, 22, 17,
      17, 22, 23, 17, 23, 18,
      18, 23, 24, 18, 24, 19,
    };

    clus.sphere.Calculate_Dimensions (
      points, p - points, 
      indexlist, 72, &Matrix::I);

  #else

    U32 count = (p - points);
    o.y /= (F32) count;
    clus.sphere.SetOrigin( o);

    clus.sphere.radius2 = 0.0;
    U32 i; 
    for (i = 0; i < count; i++)
    {
      Vector dv = o - points[i];
      F32 mag2 = dv.Dot( dv);
      if (mag2 > clus.sphere.radius2)
      {
        clus.sphere.radius2 = mag2;
      }
    }
    clus.sphere.radius = (F32) sqrt( clus.sphere.radius2);
  #endif
  }
  //----------------------------------------------------------------------------

  // recalculate all the terrain's clusters' bounding spheres
  //
  void CalcSpheres()
  {
    U32 cz, cx;
    for (cz = 0; cz < clusHeight; cz++)
    {
		  for (cx = 0; cx < clusWidth; cx++)
		  {
        CalcClusSphere( cx, cz);
      }
    }
  }
  //----------------------------------------------------------------------------

  // recalculate all the terrain's volitile data
  //
  void Recalc()
  {
    CalcSpheres();
    CalcNormals();
  }
  //----------------------------------------------------------------------------

  // import a buffer of greyscale values as a heightfield
  //
  Bool ImportBitmap( char *buffer, U32 bwid, U32 bhgt, F32 scale, Rect &rect, U32 csize, U32 flags) // = CELLSIZE, = 0
  {
    U32 wid = rect.Width();
    U32 hgt = rect.Height();

    if (!(flags & IMPORTSAVE))
    {
      if (!Setup( wid, hgt, csize))
      {
        return FALSE;
      }
    }

    heightField.ImportBitmap( buffer, bwid, bhgt, scale, rect);

    Recalc();

    return TRUE;
  }
  //----------------------------------------------------------------------------
  const char *TERRAINKEY = "terraindata";
  static U32 saveVersion = 10;

  // save Dark Reign 2 terrain data
  //
  Bool Save( BlockFile &bfile)
  {
    bfile.OpenBlock(TERRAINKEY);

    // terrain save version number
    bfile.WriteToBlock( &saveVersion, sizeof( saveVersion));

    // offset
    bfile.WriteToBlock( &heightField.offsetX, sizeof( heightField.offsetX));
    bfile.WriteToBlock( &heightField.offsetZ, sizeof( heightField.offsetZ));

    // normals
    bfile.WriteToBlock( &normCount, sizeof( normCount));
    bfile.WriteToBlock( &normList, sizeof( Vector) * normCount);

    // find unused textures
    U8 mapper[256];
    memset( mapper, 0x0, 256);
    U32 i;
    // cells
    for (i = 0; i <= heightField.cellMax; i++)
    {
      Cell &cell = heightField.cellList[i];

      if (cell.texture >= texCount)
      {
        cell.texture = 0;
      }
      mapper[cell.texture] = 1;
    }

    // textures
    bfile.WriteToBlock( &texCount, sizeof( texCount));
    for (i = 0; i < texCount; i++)
    {
      // replace unused textures with the default texture
      // next load/save will eliminate them
      const char *name = mapper[i] ? texList[i]->GetName() : defTex->GetName();
      U32 slen = strlen( name) + 1;
      bfile.WriteToBlock( &slen, sizeof( slen));
      bfile.WriteToBlock( name, slen);
    }

    // cells
    for (i = 0; i <= heightField.cellMax; i++)
    {
      Cell &cell = heightField.cellList[i];

      if (cell.texture >= texCount)
      {
        cell.texture = 0;
        LOG_DIAG( ("Save: cell texture out of list") ); 
      }

      bfile.WriteToBlock( &cell.height, sizeof( cell.height));
      bfile.WriteToBlock( &cell.normal, sizeof( cell.normal));
      bfile.WriteToBlock( &cell.uv, sizeof( cell.uv));
      bfile.WriteToBlock( &cell.texture, sizeof( cell.texture));
      bfile.WriteToBlock( &cell.flags, sizeof( cell.flags));

      bfile.WriteToBlock( &cell.color, sizeof( cell.color));
    }
    U32 dummy = 0;
    //F32 lt = *Client::lightTime;
    F32 lt = 0.25f;

    bfile.WriteToBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
    bfile.WriteToBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
    bfile.WriteToBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
    bfile.WriteToBlock( &lt, sizeof( lt));
    bfile.WriteToBlock( &Vid::renderState.fogDepth, sizeof( Vid::renderState.fogDepth));
    bfile.WriteToBlock( &dummy, sizeof( dummy));
    bfile.WriteToBlock( &dummy, sizeof( dummy));
    F32 level = Sky::GetLevel();
    bfile.WriteToBlock( &level, sizeof( F32));
    bfile.WriteToBlock( &dummy, sizeof( dummy));

    farPlane = Vid::CurCamera().FarPlane();

    bfile.WriteToBlock( &farPlane, sizeof( farPlane));    // version 3
    bfile.WriteToBlock( &Vid::renderState.fogR, sizeof( F32));    // version 4
    bfile.WriteToBlock( &Vid::renderState.fogG, sizeof( F32));    // version 4
    bfile.WriteToBlock( &Vid::renderState.fogB, sizeof( F32));    // version 4

    for (i = 0; i < clusCount; i++)
    {
      Cluster &clus = clusList[i];
      bfile.WriteToBlock( &clus.status, sizeof( clus.status));
      bfile.WriteToBlock( &clus.waterHeight, sizeof( F32));
    }

    bfile.CloseBlock();

	  return TRUE;
  }
  //----------------------------------------------------------------------------

  // load Dark Reign 2 terrain data
  //
  Bool Load( BlockFile &bfile)
  {
    bfile.OpenBlock(TERRAINKEY);

    // terrain version number
    U32 version;
    bfile.ReadFromBlock( &version, sizeof( version));

    if (version < 6)
    {
      ERR_FATAL(("Map version (%d) is now unsupported", version));
    }

    if (version > 9)
    {
      bfile.ReadFromBlock( &heightField.offsetX, sizeof( heightField.offsetX));
      bfile.ReadFromBlock( &heightField.offsetZ, sizeof( heightField.offsetZ));
    }

    // normals
    bfile.ReadFromBlock( &normCount, sizeof( normCount));
    bfile.ReadFromBlock( &normList, sizeof( Vector) * normCount);

    // textures
    U32 tcount;
    bfile.ReadFromBlock( &tcount, sizeof( tcount));

    U32 i;
    U8 mapper[256];
    for (i = 0; i < tcount; i++)
    {
      char name[44];
      U32 slen;
      bfile.ReadFromBlock( &slen, sizeof( slen));
      bfile.ReadFromBlock( name, slen);
      mapper[i] = (U8) AddTexture( name);
    }

    // cells

    F32 height;
    U8 normal, uv;
    U8 texture, flags;
    Color color;

#if 0
    U32 count = 0;
    for (i = 0; i <= heightField.cellMax; i++)
    {
      Cell &cell = heightField.cellList[i];

      if (version >= 9 || i == 0 || (i % heightField.cellWidth) != 0)
      {
        bfile.ReadFromBlock( &height, sizeof( height));
        bfile.ReadFromBlock( &normal, sizeof( normal));
        bfile.ReadFromBlock( &uv, sizeof( uv));
        bfile.ReadFromBlock( &texture, sizeof( texture));
        bfile.ReadFromBlock( &flags, sizeof( flags));
        bfile.ReadFromBlock( &color, sizeof( color));

        cell.texture = mapper[texture];
        cell.height = height;
        cell.normal = normal;
        cell.uv     = uv;
        cell.color  = color;

        count++;
      }
      else
      {
        Cell *oldCell = &cell - heightField.cellWidth;

        cell.texture = oldCell->texture;
        cell.height  = oldCell->height;
        cell.normal  = oldCell->normal;
        cell.uv      = oldCell->uv;
        cell.color   = oldCell->color;
      }

      if (cell.texture >= texCount)
      {
        ASSERT( cell.texture < texCount);
      }
    }
#else

    if (version >= 9)
    {
      U32 x, z;
      for (z = 0; z < heightField.cellHeight; z++)
      {
        for (x = 0; x < heightField.cellPitch; x++)
        {
          Cell &cell = heightField.cellList[z * heightField.cellPitch + x];

          bfile.ReadFromBlock( &height, sizeof( height));
          bfile.ReadFromBlock( &normal, sizeof( normal));
          bfile.ReadFromBlock( &uv, sizeof( uv));
          bfile.ReadFromBlock( &texture, sizeof( texture));
          bfile.ReadFromBlock( &flags, sizeof( flags));
          bfile.ReadFromBlock( &color, sizeof( color));

          cell.texture = mapper[texture];
          cell.height = height;
          cell.normal = normal;
          cell.uv     = uv;
          cell.color  = color;

          if (cell.texture >= texCount)
          {
            ASSERT( cell.texture < texCount);
          }
        }
      }
    }
    else
    {
      U32 x, z;
      for (z = 0; z < heightField.cellHeight; z++)
      {
        for (x = 0; x < heightField.cellWidth; x++)
        {
          Cell &cell = heightField.cellList[z * heightField.cellPitch + x];

          bfile.ReadFromBlock( &height, sizeof( height));
          bfile.ReadFromBlock( &normal, sizeof( normal));
          bfile.ReadFromBlock( &uv, sizeof( uv));
          bfile.ReadFromBlock( &texture, sizeof( texture));
          bfile.ReadFromBlock( &flags, sizeof( flags));
          bfile.ReadFromBlock( &color, sizeof( color));

          cell.texture = mapper[texture];
          cell.height = height;
          cell.normal = normal;
          cell.uv     = uv;
          cell.color  = color;

          if (cell.texture >= texCount)
          {
            ASSERT( cell.texture < texCount);
          }
        }
      }
      for (z = 0; z < heightField.cellHeight; z++)
      {
        Cell &cell0 = heightField.cellList[z * heightField.cellPitch];
        Cell &cell1 = heightField.cellList[z * heightField.cellPitch + heightField.cellWidth];

        cell1.texture = cell0.texture;
        cell1.height  = cell0.height; 
        cell1.normal  = cell0.normal; 
        cell1.uv      = cell0.uv;     
        cell1.color   = cell0.color;  
      }
    }
#endif

    U32 dummy = 0;
    F32 lt;
    F32 depth, level;

    bfile.ReadFromBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
    bfile.ReadFromBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
    bfile.ReadFromBlock( &lt, sizeof( lt)); // Spacer to maintain compatability
    bfile.ReadFromBlock( &lt, sizeof( lt));
    bfile.ReadFromBlock( &depth, sizeof( depth));
    bfile.ReadFromBlock( &dummy, sizeof( dummy));
    bfile.ReadFromBlock( &dummy, sizeof( dummy));

    Vid::SetFogDepth( depth);

    bfile.ReadFromBlock( &level, sizeof( level));
    Sky::SetLevel( level);

    bfile.ReadFromBlock( &dummy, sizeof( dummy));

    bfile.ReadFromBlock( &farPlane, sizeof( farPlane));
    if (farPlane < 10.0f)
    {
      farPlane = 10.0f;
    }
    Vid::CurCamera().SetFarPlane( farPlane);

    F32 r, g, b;
    bfile.ReadFromBlock( &r, sizeof( F32));
    bfile.ReadFromBlock( &g, sizeof( F32));
    bfile.ReadFromBlock( &b, sizeof( F32));
    Vid::SetFogColor( r, g, b);

    if (version == 7)
    {
      // Damnit, checked in code without the flag, design might have saved maps :(
      for (i = 0; i < clusCount; i++)
      {
        Cluster &clus = clusList[i];
        bfile.ReadFromBlock( &clus.waterHeight, sizeof( F32));
      }
    }
    else

    if (version > 7)
    {
      for (i = 0; i < clusCount; i++)
      {
        Cluster &clus = clusList[i];
        bfile.ReadFromBlock( &clus.status, sizeof( clus.status));
        bfile.ReadFromBlock( &clus.waterHeight, sizeof( F32));
      }
    }

    bfile.CloseBlock();
  
    Recalc();

  #ifdef DOCLUSTERCELLS
    Cluster *clus = clusList;
    Cell *c00 = heightField.cellList;
    U32 z, x;
    for (z = 0; z < clusHeight; z++, c00 += 4 * heightField.cellPitch)
    {
      Cell *c0 = c00;
      for (x = 0; x < clusWidth; x++, clus++, c0 += 4)
      {
        Cell *c1 = c0;
        U32 z1, x1, count = 0;
        for (z1 = 0; z1 <= 4; z1++, c1 += heightField.cellPitch)
        {
          Cell *c = c1;
          for (x1 = 0; x1 <= 4; x1++, c++, count++)
          {
            clus->cells[count] = *c;
          }
        }
      }
    }
  #endif

    return TRUE;
  }
  //----------------------------------------------------------------------------

  // do any terrain animation
  //
  void Simulate( F32 timestep)
  {
    Sky::Simulate( timestep);

    F32 wVel0 = waterPos0 + waterSpeed * timestep;
	  F32 wVel1 = waterPos1 - waterSpeed * timestep;

	  wVel0 = (F32) fmod (wVel0, 1.0f);
	  wVel1 = (F32) fmod (wVel1, 1.0f);

	  waterPos0 = wVel0;
	  waterPos1 = wVel1;

    U32 i;
    for (i = 0; i < 4; i++)
    {
      waterUVList0[i].u = baseUVList0[i].u + wVel0;
      waterUVList0[i].v = baseUVList0[i].v + wVel0;

      waterUVList1[i].u = baseUVList1[i].u + wVel1;
      waterUVList1[i].v = baseUVList1[i].v + wVel1;
    }
  }
  //----------------------------------------------------------------------------

  // do any terrain animation
  //
  void SimulateViewer( F32 timestep)
  {
    Sky::Simulate( timestep);

    F32 wVel0 = waterPos0 + waterSpeed * timestep;
	  F32 wVel1 = waterPos1 - waterSpeed * timestep;

	  wVel0 = (F32) fmod (wVel0, 1.0f);
	  wVel1 = (F32) fmod (wVel1, 1.0f);

	  waterPos0 = wVel0;
	  waterPos1 = wVel1;

    U32 i;
    for (i = 0; i < 4; i++)
    {
      waterUVList0[i].u = baseUVList0[i].u + wVel0;
      waterUVList0[i].v = baseUVList0[i].v + wVel0;

      waterUVList1[i].u = baseUVList1[i].u + wVel1;
      waterUVList1[i].v = baseUVList1[i].v + wVel1;
    }

    F32 move = movePos - moveSpeed / MeterPerCell() * timestep;
    move = (F32) fmod( move, 1.0f);
    movePos = move;

    for (i = 0; i < 4; i++)
    {
      cellUVList[i].v = baseUVList0[i].v + move;
    }
  }
  //----------------------------------------------------------------------------

  // return the amount of memory used by a terrain !!!! FIXME
  //
  U32 GetMem()
  {
	  U32 memory = (heightField.cellMax + 1 + heightField.cellPitch) * sizeof(Cell);
    memory += sizeof(HeightField);
	  memory += clusCount * sizeof(Cluster);
    memory += (char *)&endVar - (char *)&sysInit;

//    memory += Sky::GetMem();    // FIXME

	  return memory;
  }
  //----------------------------------------------------------------------------

  // recalculate the lighting values for the standard normals via sunlight 
  //
  void CalcLighting()
  {
    DxLight::Manager::sun->Light( normLights, normList, normCount);
  }
  //----------------------------------------------------------------------------

  //const U32 NORMCOUNTX      = 9;   // number of normals around the x axis
  const S32 NORMCOUNTX      = 17;     // number of normals around the x axis
  const S32 NORMHALFX       = NORMCOUNTX / 2;
  const S32 NORMQUARTX      = NORMCOUNTX / 4;
  const F32 NORMINTERVALX   = 1.0f / (F32) NORMHALFX;
  const F32 DANGX           = (F32) PIBY2 * NORMINTERVALX;

  const S32 NORMCOUNTY      = 8;  // 8 intervals around the y axis
  const F32 DANGY           = (F32) PI2 / (F32) NORMCOUNTY;

  // for quick normal recalculation
  static F32 invdx;
  static U32 normMapper[NORMCOUNTX][NORMCOUNTX][NORMCOUNTX][NORMCOUNTX];

  // build the standard terrain normal list
  //
  void BuildNormals()
  {
    invdx = 1.0f / heightField.meterPerCell;

    normCount = 0;
    // start with a default strait up normal
    normList[ normCount] = Matrix::I.Up();
    normCount++;

    // loop from strait up to strait front
    F32 ax, ay; 
    F32 dscale = NORMINTERVALX * shadefactor;
    F32 scale = 1.0f - dscale;
    for (ax = DANGX; ax <= PIBY2; ax += DANGX, scale -= dscale)
    {
      // loop around the circle
      Quaternion qx;
      qx.Set( ax, Matrix::I.Right());
      for (ay = 0.0f; ay < (F32) PI2; ay += DANGY, normCount++)
      {
        Quaternion qy = qx; 
        qy.Rotate( ay, Matrix::I.Up());

        Matrix mat;
        mat.ClearData();
        mat.Set( qy);

        normList[ normCount] = mat.Up() * scale;
      }
    }

  #ifdef DOQUICKNORMALS
    U32 i0, i1, i2, i3, index[4], n;
    for (i0 = 0; i0 < NORMCOUNTX; i0++)
    {
      index[0] = 0;
      if (i0 < NORMHALFX)
      {
        index[0] = 1 + (NORMHALFX - 1 - i0) * NORMCOUNTY;
      }
      else if (i0 > NORMHALFX)
      {
        index[0] = 5 + (i0 - NORMHALFX - 1) * NORMCOUNTY;
      }
      for (i1 = 0; i1 < NORMCOUNTX; i1++)
      {
        index[1] = 0;
        if (i1 < NORMHALFX)
        {
  //        index[1] = 8 + (NORMHALFX - 1 - i1) * NORMCOUNTY;
          index[1] = 2 + (NORMHALFX - 1 - i1) * NORMCOUNTY;
        }
        else if (i1 > NORMHALFX)
        {
  //        index[1] = 4 + (i1 - NORMHALFX - 1) * NORMCOUNTY;
          index[1] = 6 + (i1 - NORMHALFX - 1) * NORMCOUNTY;
        }
        for (i2 = 0; i2 < NORMCOUNTX; i2++)
        {
          index[2] = 0;
          if (i2 < NORMHALFX)
          {
  //          index[2] = 7 + (NORMHALFX - 1 - i2) * NORMCOUNTY;
            index[2] = 3 + (NORMHALFX - 1 - i2) * NORMCOUNTY;
          }
          else if (i2 > NORMHALFX)
          {
  //          index[2] = 3 + (i2 - NORMHALFX - 1) * NORMCOUNTY;
            index[2] = 7 + (i2 - NORMHALFX - 1) * NORMCOUNTY;
          }
          for (i3 = 0; i3 < NORMCOUNTX; i3++)
          {
            index[3] = 0;
            if (i3 < NORMHALFX)
            {
  //            index[3] = 6 + (NORMHALFX - 1 - i3) * NORMCOUNTY;
              index[3] = 4 + (NORMHALFX - 1 - i3) * NORMCOUNTY;
            }
            else if (i3 > NORMHALFX)
            {
  //            index[3] = 2 + (i3 - NORMHALFX - 1) * NORMCOUNTY;
              index[3] = 8 + (i3 - NORMHALFX - 1) * NORMCOUNTY;
            }

            Vector norm = { 0.0f, 0.0f, 0.0f };
            for (n = 0; n < 4; n++)
            {
              norm += normList[index[n]];
            }
            norm.Normalize();

            F32 maxdiff = F32_MAX;
            U32 i, normIndex = 0;
            for (i = 0; i < (U32) normCount1; i++)
            {
              F32 diff = (F32) fabs( norm.x - normList[i].x);
              diff +=    (F32) fabs( norm.y - normList[i].y);
              diff +=    (F32) fabs( norm.z - normList[i].z);

              if (diff < maxdiff)
              {
                maxdiff = diff;
                normIndex = i;
              }
            }
            normMapper[i0][i1][i2][i3] = normIndex;
          }
        }
      }
    }
  #endif
  }
  //----------------------------------------------------------------------------

  // find the appropriate index into the standard Terrain normal list
  //
  U32 FindNormal( Vector &normal)
  {
    Vector *norms = normList;
    U32 count = normCount;

    // compare it to all the normals in the standard list and find the best match
    F32 maxdiff = F32_MAX;
    U32 i, normIndex = 0;
    for (i = 0; i < count; i++)
    {
      F32 diff = (F32) fabs( normal.x - norms[i].x);
      diff +=    (F32) fabs( normal.y - norms[i].y);
      diff +=    (F32) fabs( normal.z - norms[i].z);

      if (diff < maxdiff)
      {
        maxdiff = diff;
        normIndex = i;
      }
    }
    return normIndex;
  }
  //----------------------------------------------------------------------------

  // set the cell's normal index into the standard Terrain normal list
  // via quick algorithim
  // ! this function assumes that the cell is not on the bounding edge of the heightfield
  // ! i.e. cx > 0 && cx < cellwidth - 1 && cz > 0 && cz < cellheight - 1
  //
  U32 FindNormalQuick( U32 offset)
  {
    F32 slope, h0, h[4][2];
    S32 index[4];

    h0 = heightField.cellList[offset].height;
    h[0][0] = heightField.cellList[offset - heightField.cellPitch].height; 
    h[0][1] = heightField.cellList[offset + heightField.cellPitch].height; 
    h[1][0] = heightField.cellList[offset - heightField.cellPitch - 1].height; 
    h[1][1] = heightField.cellList[offset + heightField.cellPitch + 1].height; 
    h[2][0] = heightField.cellList[offset - 1].height; 
    h[2][1] = heightField.cellList[offset + 1].height; 
    h[3][0] = heightField.cellList[offset + heightField.cellPitch - 1].height; 
    h[3][1] = heightField.cellList[offset - heightField.cellPitch + 1].height; 

    U32 i;
    for (i = 0; i < 4; i++)
    {
      slope = (h[i][0] - h0) * invdx;
      if (slope >= 0.0f)
      {
        if (slope <= 1.0f) 
        {
          index[i] = (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] = NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      else
      {
        if (slope >= -1.0f) 
        {
          index[i] = (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] = -NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      slope = (h0 - h[i][1]) * invdx;
      if (slope >= 0.0f)
      {
        if (slope <= 1.0f) 
        {
          index[i] += (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] += NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      else
      {
        if (slope >= -1.0f) 
        {
          index[i] += (U32) (slope * NORMQUARTX);
        }
        else
        {
          index[i] += -NORMHALFX - (U32) (1.0f / slope * NORMQUARTX);
        }
      }
      index[i] >>= 1;
      index[i] += NORMHALFX;
    }

    return normMapper[index[0]][index[1]][index[2]][index[3]];
  }
  //----------------------------------------------------------------------------

  // recalculate all the cells' normal indices in terrain
  //
  void CalcNormals()
  {
    terrAverageHeight = 0.0f;
    terrMinHeight =  1000000.0f;
    terrMaxHeight = -1000000.0f;

    U32 offset, cx, cz;
    for (offset = 0, cz = 0; cz < heightField.cellHeight; cz++)
    {
      for (cx = 0; cx < heightField.cellPitch; cx++, offset++)
      {
        // edge cells get the default normal
        U32 normIndex = 0;
        if (cx > 0 && cx < heightField.cellPitch  - 1
         && cz > 0 && cz < heightField.cellHeight - 1)
        {
          Vector norm;
          heightField.CalcCellNormal( offset, norm);

          F32 maxdiff = F32_MAX;
          U32 i;
          for (i = 0; i < normCount; i++)
          {
            F32 diff = (F32) fabs( norm.x - normList[i].x);
            diff +=    (F32) fabs( norm.y - normList[i].y);
            diff +=    (F32) fabs( norm.z - normList[i].z);

            if (diff < maxdiff)
            {
              maxdiff = diff;
              normIndex = i;
            }
          }
        }
        heightField.cellList[offset].normal = (U8) normIndex;

        F32 h = heightField.cellList[offset].height; 

        terrAverageHeight += h;

        if (h < terrMinHeight)
        {
          terrMinHeight = h;
        }
        if (h > terrMaxHeight)
        {
          terrMaxHeight = h;
        }
      }
    }
    if (terrMaxHeight < terrMinHeight + 100.0f)
    {
      terrMaxHeight = terrMinHeight + 100.0f;
    }
    terrAverageHeight /= (F32) (heightField.cellMax + 1);
  }
  //----------------------------------------------------------------------------

  // recalculate all the cells' normal indices in terrain
  //
  void CalcNormalsQuick()
  {
  #ifdef DOQUICKNORMALS
    U32 offset, cx, cz;
    for (offset = 0, cz = 0; cz < heightField.cellHeight; cz++)
    {
      for (cx = 0; cx < heightField.cellPitch; cx++, offset++)
      {
        // edge cells get the default normal
        U32 normIndex = 0;
        if (cx > 0 && cx < heightField.cellPitch  - 1
         && cz > 0 && cz < heightField.cellHeight - 1)
        {
          normIndex = FindNormalQuick( offset);
        }
        heightField.cellList[offset].normal = (U8) normIndex;
      }
    }
  #endif
  }
  //----------------------------------------------------------------------------

  U16 AddTexture( const char *name)
  {
    U16 i;
    for (i = 0; i < texCount; i++)
    {
      if (texList[i] && !strcmp( texList[i]->GetName(), name))
      {
        return i;
      }
    }
    texList[texCount] = Bitmap::Manager::FindCreate( name, *mipCount);

    if (!texList[texCount])
    {
      LOG_DIAG( ("Can't find terrain texture %s", name) );

      texList[texCount] = defTex;
    }

    texCount++;

    return i;
  }
  //----------------------------------------------------------------------------

  void RenderCellIsometric( Cell &c0, Cell &c1, Cell &c2, Cell &c3, S32 x0, S32 z0, S32 x1, S32 z1, U32 clipFlags) // = clipALL)
  {							
    clipFlags;

	  VertexTL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
    {
      return;
    }
    Camera &camera = Vid::CurCamera();

    UVPair *uvList = &cellUVList[c0.uv];

    vertmem[0].vv.x = (F32) x0;
    vertmem[0].vv.z = (F32) z0;
    vertmem[0].vv.y = c0.height;
    vertmem[0].uv = uvList[0];
    vertmem[0].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[0]);

    vertmem[1].vv.x = (F32) x0;
    vertmem[1].vv.z = (F32) z1;
    vertmem[1].vv.y = c1.height;
    vertmem[1].uv = uvList[1];
    vertmem[1].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[1]);

    vertmem[2].vv.x = (F32) x1;
    vertmem[2].vv.z = (F32) z1;
    vertmem[2].vv.y = c2.height;
    vertmem[2].uv = uvList[2];
    vertmem[2].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[2]);

    vertmem[3].vv.x = (F32) x1;
    vertmem[3].vv.z = (F32) z0;
    vertmem[3].vv.y = c3.height;
    vertmem[3].uv = uvList[3];
    vertmem[3].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[3]);

  #ifdef DOVERTCOLORS
    Color c = normLights[c0.normal];
    vertmem[0].diffuse.Modulate( c0.color, 
      F32(c.r) * (1.0f / 255.0f),
      F32(c.g) * (1.0f / 255.0f),
      F32(c.b) * (1.0f / 255.0f));

    c = normLights[c1.normal];
    vertmem[1].diffuse.Modulate( c1.color, 
      (F32)c.r * (1.0f / 255.0f),
      (F32)c.g * (1.0f / 255.0f),
      (F32)c.b * (1.0f / 255.0f));

    c = normLights[c2.normal];
    vertmem[2].diffuse.Modulate( c2.color, 
      (F32)c.r * (1.0f / 255.0f),
      (F32)c.g * (1.0f / 255.0f),
      (F32)c.b * (1.0f / 255.0f));

    c = normLights[c1.normal];
    vertmem[3].diffuse.Modulate( c3.color, 
      (F32)c.r * (1.0f / 255.0f),
      (F32)c.g * (1.0f / 255.0f),
      (F32)c.b * (1.0f / 255.0f));

  #else
    vertmem[0].diffuse = normLights[c0.normal];
    vertmem[1].diffuse = normLights[c1.normal];
    vertmem[2].diffuse = normLights[c2.normal];
    vertmem[3].diffuse = normLights[c3.normal];
  #endif

    Utils::Memcpy( indexmem, Vid::rectIndices, 12);

    Vid::UnlockIndexedPrimitiveMem( 4, 6);
  }
  //----------------------------------------------------------------------------

  void RenderCellIsometric( S32 x0, S32 z0, S32 x1, S32 z1, F32 y, UVPair *uvList, U32 clipFlags) // = clipALL
  {	
    clipFlags;

	  VertexTL *vertmem;
    U16 *indexmem;
    if (!Vid::LockIndexedPrimitiveMem( (void **)&vertmem, 4, &indexmem, 6))
    {
      return;
    }
    Camera &camera = Vid::CurCamera();

    vertmem[0].vv.x = (F32) x0;
    vertmem[0].vv.z = (F32) z0;
    vertmem[0].vv.y = y;
    vertmem[0].uv = uvList[0];
    vertmem[0].diffuse = normLights[0];
    vertmem[0].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[0]);

    vertmem[1].vv.x = (F32) x0;
    vertmem[1].vv.z = (F32) z1;
    vertmem[1].vv.y = y;
    vertmem[1].uv = uvList[1];
    vertmem[1].diffuse = normLights[0];
    vertmem[1].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[1]);

    vertmem[2].vv.x = (F32) x1;
    vertmem[2].vv.z = (F32) z1;
    vertmem[2].vv.y = y;
    vertmem[2].uv = uvList[2];
    vertmem[2].diffuse = normLights[0];
    vertmem[2].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[2]);

    vertmem[3].vv.x = (F32) x1;
    vertmem[3].vv.z = (F32) z0;
    vertmem[3].vv.y = y;
    vertmem[3].uv = uvList[3];
    vertmem[3].diffuse = normLights[0];
    vertmem[3].specular = 0xff000000;
    camera.TransformProjectIsoFromWorldSpace( vertmem[3]);

    Utils::Memcpy( indexmem, Vid::rectIndices, 12);

    Vid::UnlockIndexedPrimitiveMem( 4, 6);
  }
  //----------------------------------------------------------------------------

  // draw a single cluster
  //
  void RenderClusterIsometric( Cluster &clus, S32 x, S32 z, U32 cellOffset, U32 cellStride, U32 clipFlags) // = 1, = clipALL
  {
    clipFlags;

    DxLight::Manager::SetActiveList( clus.sphere);

    U32 meterStride     = cellStride * heightField.meterPerCell;
    U32 cellStrideWidth = cellStride * heightField.cellPitch;

    // fill the vertex memory
    S32 x0, x1, xend = x + meterPerClus;
    S32 z0, z1, zend = z + meterPerClus;
    U32 offset = cellOffset;
    for (z0 = z, z1 = z + meterStride; z0 < zend; z0 += meterStride, z1 += meterStride)
    {
      Cell *c0 = &heightField.cellList[offset];
      Cell *c1 = c0 + heightField.cellPitch;
      Cell *c2 = c1 + 1;
      Cell *c3 = c0 + 1;
      for (x0 = x, x1 = x + meterStride; x0 < xend; x0 += meterStride, x1 += meterStride)
      {
        Bitmap *tex = texList[c0->texture];
        ASSERT(tex)

        Vid::SetBucketTexture(tex);

        RenderCellIsometric( *c0, *c1, *c2, *c3, x0, z0, x1, z1, clipFlags);

        c0 += cellStride;
        c1 += cellStride;
        c2 += cellStride;
        c3 += cellStride;
      }
      offset += cellStrideWidth;
    }

	  if (clus.status.water && *water)
	  {
      // one quad covers the whole cluster with water
	    Vid::SetBucketTexture(waterTex);

  //	  Vector origin = clus.sphere.Origin();
  //	  Vid::CurCamera().Transform( origin, origin);
	    Vid::SetTranBucketZ( 100.0f);

      RenderCellIsometric( x, z, xend, zend, clus.waterHeight, waterUVList0, clipFlags);
	  }
  }
  //----------------------------------------------------------------------------

  void RenderIsometric()
  {
    Vid::SetBucketPrimitiveDesc(
      PT_TRIANGLELIST,
      FVF_TLVERTEX,
      DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | RS_BLEND_DEF,
      TRUE);

    Vid::SetWorldTransform( Matrix::I);
    Vid::SetBucketMaterial( Vid::defMaterial);

    CalcLighting();

    // returns vRect of meters that might be visible
	  Rect vRect;
    Vid::CurCamera().GetVisibleRect( vRect);

    vRect.l -= (S32) OffsetX();
    vRect.r -= (S32) OffsetX();
    vRect.t -= (S32) OffsetZ();
    vRect.b -= (S32) OffsetZ();

    // convert it to cluster coords 
    vRect.l = (S32) ((F32) vRect.l * clusPerMeter);
    vRect.r = (S32) ((F32) vRect.r * clusPerMeter);
    vRect.t = (S32) ((F32) vRect.t * clusPerMeter);
    vRect.b = (S32) ((F32) vRect.b * clusPerMeter);
  
    // increase bounds by one along each edge to be sure everything gets drawn
    vRect.l--;
    vRect.r++;
    vRect.t--;
    vRect.b++;

    // clip to the actual terrain rectangle
    if (vRect.l < 0)
    {
      vRect.l = 0;
    }
    if (vRect.r > (S32) clusWidth)
    {
      vRect.r = clusWidth;
    }
    if (vRect.t < 0)
    {
      vRect.t = 0;
    }
    if (vRect.b > (S32) clusHeight)
    {
      vRect.b = clusHeight;
    }
    // calc the first cluster's and cell's offsets
    U32 clusOffset, clusOffsetStart = vRect.t * clusWidth + vRect.l;
    U32 cellOffset, cellOffsetStart = vRect.t * clusWidth * cellPerClus * cellPerClus + vRect.l * cellPerClus;

    // re-convert to meters
    vRect.t *= meterPerClus;
    vRect.b *= meterPerClus;
    vRect.l *= meterPerClus;
    vRect.r *= meterPerClus;

    vRect.t += (S32) OffsetZ();
    vRect.b += (S32) OffsetZ();
    vRect.l += (S32) OffsetX();
    vRect.r += (S32) OffsetX();

    U32 cellClusWidth = heightField.cellPitch * cellPerClus;
    while (vRect.t < vRect.b)
    {
      clusOffset = clusOffsetStart;
      cellOffset = cellOffsetStart;
      S32 x;
      for (x = vRect.l; x < vRect.r; x += meterPerClus, clusOffset++, cellOffset += cellPerClus)
      {
        Cluster &clus = clusList[ clusOffset];

        U32 clipFlags = Vid::CurCamera().SphereTest( clus.sphere);
        if (clipFlags == clipOUTSIDE)
        {
          // cluster is completely outside the view frustrum
          continue;
        }
  //      DxLight::Manager::SetActiveList( Sky::sun);
  //      DxLight::Manager::SetupLightsModelSpace();

        RenderClusterIsometric( clus, x, vRect.t, cellOffset, 1, clipFlags);
      }
      vRect.t += meterPerClus;
      clusOffsetStart += clusWidth;
      cellOffsetStart += cellClusWidth; 
    }
  }
  //----------------------------------------------------------------------------

  void RenderShadow( const Vector &pos, F32 radius, GETHEIGHTPROCPTR getHeightProc, FINDFLOORPROCPTR findFloorProc) // = GetHeight, FindFloor 
  {
    RenderShadow( *DxLight::Manager::sun, pos, radius, getHeightProc, findFloorProc);
  }
  //----------------------------------------------------------------------------

  void RenderShadow( const DxLight &light, const Vector &pos, F32 radius, GETHEIGHTPROCPTR getHeightProc, FINDFLOORPROCPTR findFloorProc) // = GetHeight, FindFloor 
  {
    Vector p = pos;
    p.y += 0.3f;
    Sphere s( p, radius);

    // get the light to pos vector
    Vector v;
  //  if (light.Type() == lightDIRECTION)
    {
      v = light.WorldMatrix().Front();
    }
  //  else
  //  {
  //    /v = pos - light.WorldMatrix().Position();
  //    v.Normalize();
  //    if (v.y < 0.01f)
  //    {
  //      v.y = 0.01f;
  //    }
  //  }

    // a little more downward tilt
    v.y -= 0.2f;

    // does the shadow hit the terrain?
    if (!Intersect( s.posit, v, 1.0f, findFloorProc))
    {
      return;
    }

    Vector viewpos;
    U32 clipFlags = Vid::CurCamera().SphereTest( s, &viewpos);
    if (clipFlags == clipOUTSIDE)
    {
      return;
    }

    // calculate stretched radius
    F32 stretch = -1.2f / v.y;
    if (stretch > 3.0f)
    {
      stretch = 3.0f;
    }
    F32 rad = s.radius * stretch;

    // calculate delta radius
    F32 dr = (rad - s.radius);

    // shift shadow by delta radius
    v.y = 0.0f;
    v.Normalize();
    s.posit += v * dr;

    // calculate u stretch
    stretch = rad / s.radius - 1.0f;
    s.radius = rad;

    F32 vx = (F32) fabs( v.x);
    F32 vz = (F32) fabs( v.z);

  #if 1
    UVPair uv0( 0.0f - stretch * vz, 0.0f - stretch * vx);
    UVPair uv1( 1.0f + stretch * vz, 0.0f - stretch * vx);
    UVPair uv2( 1.0f + stretch * vz, 1.0f + stretch * vx);
  #else
    UVPair uv0( 0.0f - stretch, 0.0f);
    UVPair uv1( 1.0f + stretch, 0.0f);
    UVPair uv2( 1.0f + stretch, 1.0f);
  #endif

    RenderGroundSprite( s, shadowTex, DxLight::Manager::shadowColor, clipFlags, viewpos.z, uv0, uv1, uv2, getHeightProc);

  #ifdef DOSTATISTICS
    Statistics::shadowTris = Statistics::shadowTris + Statistics::tempTris;
  #endif
  }
  //----------------------------------------------------------------------------

  // draw a ground-hugging sprite; rotate the texture first
  //
  void RenderGroundSprite( const Sphere &sphere, Bitmap *texture, Color color, const Vector &front, UVPair uv0, UVPair uv1, UVPair uv2, GETHEIGHTPROCPTR getHeightProc) // =  uv0(0,0), uv1(1,0), uv2(1,1), GetHeight
  {
  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    Vector f = front;
    f.y =0.0f;
    f.Normalize();
    Matrix m;
    m.ClearData();
    m.SetFromFront( f);

    F32 du = (F32) fabs(uv2.u + uv0.u) * 0.5f;
    F32 dv = (F32) fabs(uv2.v + uv0.v) * 0.5f;
    uv0.u -= du;
    uv0.v -= dv;
    uv1.u -= du;
    uv1.v -= dv;
    uv2.u -= du;
    uv2.v -= dv;

    m.Rotate( uv0);
    m.Rotate( uv1);
    m.Rotate( uv2);

    uv0.u += du;
    uv0.v += dv;
    uv1.u += du;
    uv1.v += dv;
    uv2.u += du;
    uv2.v += dv;

    RenderGroundSprite( sphere, texture, color, uv0, uv1, uv2, getHeightProc);
  }
  //----------------------------------------------------------------------------

  // draw a ground-hugging sprite
  //
  void RenderGroundSprite( const Sphere &sphere, Bitmap *texture, Color color, UVPair uv0, UVPair uv1, UVPair uv2, GETHEIGHTPROCPTR getHeightProc) // =  uv0(0,0), uv1(1,0), uv2(1,1), GetHeight
  {
  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    Vector viewpos;
    U32 clipFlags = Vid::CurCamera().SphereTest( sphere, &viewpos);
    if (clipFlags == clipOUTSIDE)
    {
      return;
    }
    RenderGroundSprite( sphere, texture, color, clipFlags, viewpos.z, uv0, uv1, uv2, getHeightProc);
  }
  //----------------------------------------------------------------------------

#if 0

  // draw a ground-hugging sprite
  //
  void RenderGroundSprite( const Sphere &sphere, Bitmap *texture, Color color, U32 clipFlags, F32 viewz, UVPair uv0, UVPair uv1, UVPair uv2, GETHEIGHTPROCPTR getHeightProc) // =  uv0(0,0), uv1(1,0), uv2(1,1), GetHeight
  {
  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    clipFlags;

 	  // set the primitive description
	  Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | RS_BLEND_DEF, TRUE);

	  // set the world transform matrix
	  Vid::SetWorldTransform(Matrix::I);

    // set material, texture, and force translucency
	  Vid::SetBucketMaterialTexture( NULL, NULL, FALSE);
//	  Vid::SetBucketMaterialTexture( NULL, texture, TRUE);
	  Vid::SetTranBucketZ(viewz);

	  // sprite corners in world coordinates
	  F32 meterX0 = sphere.Origin().x - sphere.Radius();
	  F32 meterZ0 = sphere.Origin().z - sphere.Radius();
    F32 meterX1 = sphere.Origin().x + sphere.Radius();
	  F32 meterZ1 = sphere.Origin().z + sphere.Radius();

	  // get corners of the grid covered by the sprite
    // get grid smaller than sprite
    // round up to grid point
    U16 cw  = Utils::FP::SetRoundUpMode();
	  U32 cellX0 = Utils::FastFtoL( meterX0 * CellPerMeter());
	  U32 cellZ0 = Utils::FastFtoL( meterZ0 * CellPerMeter());

    // round down to grid point
    Utils::FP::SetRoundDownMode();
	  U32 cellX1 = Utils::FastFtoL( meterX1 * CellPerMeter());
	  U32 cellZ1 = Utils::FastFtoL( meterZ1 * CellPerMeter());

    // restore rounding mode
    Utils::FP::RestoreMode(cw);

	  // dimensions of sprite in grid squares
	  U32 sizex = (cellX1 - cellX0 + 3);

  #ifdef DEBUG
	  U32 sizez = (cellZ1 - cellZ0 + 3);
    // verify buffer size
    ASSERT( sizex * sizez + 1 <= MAXVERTS && sizex * sizez * 6 <= MAXINDICES);
  #endif

	  // lock primitive memory
    const VertexL *vertmem = (VertexL *) Vid::tempVertices;
    const U16 *indexmem = Vid::tempIndices;

    VertexL *pvert = (VertexL *) Vid::tempVertices;
    U16 *pindex = Vid::tempIndices;

    // delta meters per grid
	  F32 dm = (F32) MeterPerCell();

	  F32 dx12 = meterX0 - meterX0;
	  F32 dx02 = meterX0 - meterX1;
    F32 dz12 = meterZ0 - meterZ1;
	  F32 dz02 = meterZ0 - meterZ1;
	  F32 dx = dx12 * dz02 - dx02 * dz12;
	  if (dx == 0.0f)
    {
      dx = F32_EPSILON;
    }
	  dx = 1.0f / dx;
	  F32 dz = -dx;

    // delta texture coords
	  F32 du12 = (F32) (uv1.u - uv2.u);
	  F32 du02 = (F32) (uv0.u - uv2.u);
	  F32 dudx = (du12 * dz02 - du02 * dz12) * dx;
	  F32 dudz = (du12 * dx02 - du02 * dx12) * dz;

	  F32 dv12 = (F32) (uv1.v - uv2.v);
	  F32 dv02 = (F32) (uv0.v - uv2.v);
	  F32 dvdx = (dv12 * dz02 - dv02 * dz12) * dx;
	  F32 dvdz = (dv12 * dx02 - dv02 * dx12) * dz;

	  // texture coordinate delta per grid
	  F32 dudxdm = dudx * dm;
	  F32 dvdxdm = dvdx * dm;
	  F32 dudzdm = dudz * dm;
	  F32 dvdzdm = dvdz * dm;

    F32 meterCellX0 = cellX0 * dm;
    F32 meterCellZ0 = cellZ0 * dm;
    F32 meterCellX1 = cellX1 * dm;

    // southwest sprite vert
	  pvert->vv.x     = meterX0;
	  pvert->vv.y     = FindFloor(meterX0, meterZ0);
	  pvert->vv.z     = meterZ0;
	  pvert->diffuse  = color;
	  pvert->specular = 0xff000000;
	  pvert->u        = uv0.u;
	  pvert->v        = uv0.v;
	  pvert++;

    // deltas
    dx = meterX1 - meterX0;
    dz = meterZ1 - meterZ0;

    // edge deltas
    F32 dx0 = (cellX0 * dm - meterX0);
    F32 dz0 = (cellZ0 * dm - meterZ0);
    F32 dx1 = (meterX1 - cellX1 * dm);

  	// south edge cell verts
    F32 xx, u = uv0.u + dx0 * dudx, v = uv0.v + dx0 * dvdx;
	  for (xx = meterCellX0; xx <= meterCellX1; xx += dm, u += dudxdm, v += dvdxdm)
  	{
	    pvert->vv.x     = xx;
	    pvert->vv.y     = FindFloor( xx, meterZ0);
	    pvert->vv.z     = meterZ0;
	    pvert->diffuse  = color;
	    pvert->specular = 0xff000000;
	    pvert->u        = u;
	    pvert->v        = v;
	    pvert++;
    }

    u = uv0.u + dx * dudx;
    v = uv0.v + dx * dvdx;

	  // southeast corner
	  pvert->vv.x     = meterX1;
	  pvert->vv.y     = FindFloor(meterX1, meterZ0);
	  pvert->vv.z     = meterZ0;
	  pvert->diffuse  = color;
	  pvert->specular = 0xff000000;
	  pvert->u        = u;
	  pvert->v        = v;
	  pvert++;

    // generate vertices on grid cell points
    u = uv0.u + dz0 * dudz;
    v = uv0.v + dz0 * dvdz;

    U32 cellX, cellZ;
	  for (cellZ = cellZ0; cellZ <= cellZ1; cellZ++, meterCellZ0 += dm, u += dudzdm, v += dvdzdm)
	  {
      // west vert for this row
      xx = meterX0;

			pvert->vv.x     = xx;
      pvert->vv.y     = FindFloor( xx, meterCellZ0);
			pvert->vv.z     = meterCellZ0;
			pvert->diffuse  = color;
			pvert->specular = 0xff000000;
			pvert->u        = u;
			pvert->v        = v;
			pvert++;

      xx += dx0;

      // middle verts for this row
      F32 uu = u + dx0 * dudx;
      F32 vv = v + dx0 * dvdx;
		  for (cellX = cellX0; cellX <= cellX1; cellX++, xx += dm, uu += dudxdm, vv += dvdxdm)
		  {
			  pvert->vv.x     = xx;
        pvert->vv.y     = GetHeight( cellX, cellZ);
			  pvert->vv.z     = meterCellZ0;
			  pvert->diffuse  = color;
			  pvert->specular = 0xff000000;
			  pvert->u        = uu;
			  pvert->v        = vv;
			  pvert++;
		  }

      uu = uv0.u + dx * dudx + (meterCellZ0 - meterZ0) * dudz;
      vv = uv0.v + dx * dvdx + (meterCellZ0 - meterZ0) * dvdz;

  	  // east vert for this row
	    pvert->vv.x     = meterX1;
	    pvert->vv.y     = FindFloor(meterX1, meterCellZ0);
	    pvert->vv.z     = meterCellZ0;
	    pvert->diffuse  = color;
	    pvert->specular = 0xff000000;
	    pvert->u        = uu;
	    pvert->v        = vv;
	    pvert++;
    }

    u = uv0.u + dz * dudz;
    v = uv1.v + dz * dvdz;

	  // northwest corner
	  pvert->vv.x     = meterX0;
	  pvert->vv.y     = FindFloor(meterX0, meterZ1);
	  pvert->vv.z     = meterZ1;
	  pvert->diffuse  = color;
	  pvert->specular = 0xff000000;
	  pvert->u        = u;
	  pvert->v        = v;
	  pvert++;

    u += dx0 * dudx;
    v += dx0 * dvdx;

  	// north edge cell vert
	  for (xx = meterCellX0; xx <= meterCellX1; xx += dm, u += dudxdm, v += dvdxdm)
  	{
	    pvert->vv.x     = xx;
	    pvert->vv.y     = FindFloor( xx, meterZ1);
	    pvert->vv.z     = meterZ1;
	    pvert->diffuse  = color;
	    pvert->specular = 0xff000000;
	    pvert->u        = u;
	    pvert->v        = v;
	    pvert++;
    }

    u = uv0.u + dx * dudx + dz * dudz;
    v = uv0.v + dx * dvdx + dz * dvdz;

	  // northeast corner
	  pvert->vv.x     = meterX1;
	  pvert->vv.y     = FindFloor(meterX1, meterZ1);
	  pvert->vv.z     = meterZ1;
	  pvert->diffuse  = color;
	  pvert->specular = 0xff000000;
	  pvert->u        = u;
	  pvert->v        = v;
	  pvert++;

	  // generate indices

    cw  = Utils::FP::SetRoundDownMode();
	  cellX0 = Utils::FastFtoL( meterX0 * CellPerMeter());
	  cellZ0 = Utils::FastFtoL( meterZ0 * CellPerMeter());

    // round up to grid point
    Utils::FP::SetRoundUpMode();
	  cellX1 = Utils::FastFtoL( meterX1 * CellPerMeter());
	  cellZ1 = Utils::FastFtoL( meterZ1 * CellPerMeter());

    // restore rounding mode
    Utils::FP::RestoreMode(cw);

	  U32 offset = 0;

  #ifdef DOFLIPTRI
    U32 indexer[] = { 1, 0, 5, 6, 1 };
    indexer[2] = sizex;
    indexer[3] = sizex + 1;
    S32 flip = cellZ0 % 2 ? (cellX0 % 2 ? 1 : 0) : (cellX0 % 2 ? 0 : 1);
    Bool xFlip = sizex % 2 ? TRUE : FALSE;
  #else
    U32 sizex1 = sizex + 1;
  #endif

	  for (cellZ = cellZ0; cellZ < cellZ1; cellZ++)
	  {
  #ifdef DOFLIPTRI
		  for (cellX = cellX0; cellX < cellX1; cellX++, flip = abs(flip - 1))
  #else
		  for (cellX = cellX0; cellX < cellX1; cellX++)
  #endif
		  {
        if (cellX < 0 || cellX >= CellWidth() || cellZ < 0 || cellZ >= CellHeight())
        {
          // cell off map
          offset++;
          continue;
        }
  #ifdef DOFLIPTRI

			  // upper left triangle
			  *pindex++ = (U16)(offset + indexer[flip + 0]);
			  *pindex++ = (U16)(offset + indexer[flip + 1]);
			  *pindex++ = (U16)(offset + indexer[flip + 2]);

			  // lower right triangle
			  *pindex++ = (U16)(offset + indexer[flip + 0]);
			  *pindex++ = (U16)(offset + indexer[flip + 2]);
			  *pindex++ = (U16)(offset + indexer[flip + 3]);
  #else
			  // upper left triangle
			  *pindex++ = (U16) offset;
			  *pindex++ = (U16)(offset + sizex);
			  *pindex++ = (U16)(offset + sizex1);

			  // lower right triangle
			  *pindex++ = (U16) offset;
			  *pindex++ = (U16)(offset + sizex1);
			  *pindex++ = (U16)(offset + 1);

  #endif

			  // advance the base offset
			  offset++;
      }
		  // advance the base offset
		  offset++;

  #ifdef DOFLIPTRI
      if (xFlip)
      {
        flip = abs(flip - 1);
      }
  #endif
    }

    // submit for projection and clipping
  #if 0
    if (clipFlags == clipNONE)
    {
      Vid::CurCamera().ProjectNoClip( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem);
    }
    else
  #endif
    {
  //    Vid::CurCamera().ProjectClip( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem);
      Vid::CurCamera().ProjectClipBias( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem, *Mesh::Manager::zBias);
    }
  }
  //----------------------------------------------------------------------------

#else

  // draw a ground-hugging sprite
  //
  void RenderGroundSprite( const Sphere &sphere, Bitmap *texture, Color color, U32 clipFlags, F32 viewz, UVPair uv0, UVPair uv1, UVPair uv2, GETHEIGHTPROCPTR getHeightProc) // =  uv0(0,0), uv1(1,0), uv2(1,1), GetHeight
  {
  #ifdef DOSTATISTICS
    Statistics::tempTris = 0;
  #endif

    clipFlags;

    if (texture)
    {
      uv0.u += texture->UVShiftWidth();
      uv0.v += texture->UVShiftHeight();
      uv1.u += texture->UVShiftWidth();
      uv1.v += texture->UVShiftHeight();
      uv2.u += texture->UVShiftWidth();
      uv2.v += texture->UVShiftHeight();
    }

 	  // set the primitive description
	  Vid::SetBucketPrimitiveDesc(PT_TRIANGLELIST, FVF_TLVERTEX,
		  DP_DONOTUPDATEEXTENTS | DP_DONOTLIGHT | DP_DONOTCLIP | RS_TEXCLAMP | RS_NOFILTER | RS_BLEND_DEF, TRUE);

	  // set the world transform matrix
	  Vid::SetWorldTransform(Matrix::I);

    // set material, texture, and force translucency
	  Vid::SetBucketMaterialTexture( NULL, texture, TRUE);
	  Vid::SetTranBucketZ(viewz);

	  // sprite corners in world coordinates
	  F32 meterX0 = sphere.Origin().x - sphere.Radius() - OffsetX();
	  F32 meterZ0 = sphere.Origin().z - sphere.Radius() - OffsetZ();
    F32 meterX1 = sphere.Origin().x + sphere.Radius() - OffsetX();
	  F32 meterZ1 = sphere.Origin().z + sphere.Radius() - OffsetZ();

	  // get corners of the grid covered by the sprite
    // get grid larger than sprite; adjust texture coords
    // round down to grid point
    U16 cw  = Utils::FP::SetRoundDownMode();
	  S32 cellX0 = Utils::FastFtoL( meterX0 * CellPerMeter());
	  S32 cellZ0 = Utils::FastFtoL( meterZ0 * CellPerMeter());

    // round up to grid point
    Utils::FP::SetRoundUpMode();
	  S32 cellX1 = Utils::FastFtoL( meterX1 * CellPerMeter());
	  S32 cellZ1 = Utils::FastFtoL( meterZ1 * CellPerMeter());

    // restore rounding mode
    Utils::FP::RestoreMode(cw);

	  // dimensions of sprite in grid squares
	  S32 sizex = (cellX1 - cellX0 + 1);

  #ifdef DEBUG
	  S32 sizez = (cellZ1 - cellZ0 + 1);
    // verify buffer size
    ASSERT( sizex * sizez + 1 <= MAXVERTS && sizex * sizez * 6 <= MAXINDICES);
  #endif

	  // lock primitive memory
    const VertexL *vertmem = (VertexL *) Vid::tempVertices;
    const U16 *indexmem = Vid::tempIndices;

    VertexL *pvert = (VertexL *) Vid::tempVertices;
    U16 *pindex = Vid::tempIndices;

    // delta meters per grid
	  F32 dm = (F32) MeterPerCell();

	  F32 dx12 = meterX0 - meterX0;
	  F32 dx02 = meterX0 - meterX1;
    F32 dz12 = meterZ0 - meterZ1;
	  F32 dz02 = meterZ0 - meterZ1;
	  F32 dx = dx12 * dz02 - dx02 * dz12;
	  if (dx == 0.0f)
    {
      dx = F32_EPSILON;
    }
	  dx = 1.0f / dx;
	  F32 dz = -dx;

    // delta texture coords
	  F32 du12 = (F32) (uv1.u - uv2.u);
	  F32 du02 = (F32) (uv0.u - uv2.u);
	  F32 dudx = (du12 * dz02 - du02 * dz12) * dx;
	  F32 dudz = (du12 * dx02 - du02 * dx12) * dz;

	  F32 dv12 = (F32) (uv1.v - uv2.v);
	  F32 dv02 = (F32) (uv0.v - uv2.v);
	  F32 dvdx = (dv12 * dz02 - dv02 * dz12) * dx;
	  F32 dvdz = (dv12 * dx02 - dv02 * dx12) * dz;

	  // texture coordinates of southwest grid point
    dx = (cellX0 * dm - meterX0);
    dz = (cellZ0 * dm - meterZ0);
    F32 u = uv0.u + dx * dudx + dz * dudz;
	  F32 v = uv0.v + dx * dvdx + dz * dvdz;

	  // texture coordinate delta per grid
	  dudx *= dm;
	  dvdx *= dm;
	  dudz *= dm;
	  dvdz *= dm;

	  // sprite grid corners in world coordinates
	  meterX0 = cellX0 * dm + OffsetX();
	  meterZ0 = cellZ0 * dm + OffsetZ();
    meterX1 = cellX1 * dm + OffsetX();
	  meterZ1 = cellZ1 * dm + OffsetZ();

    // generate vertices on grid cell points
    S32 cellX, cellZ;
	  for (cellZ = cellZ0; cellZ <= cellZ1; cellZ++, meterZ0 += dm, u += dudz, v += dvdz)
	  {
      F32 xx = meterX0;
      F32 uu = u;
      F32 vv = v;

		  for (cellX = cellX0; cellX <= cellX1; cellX++, xx += dm, uu += dudx, vv += dvdx)
		  {
			  pvert->vv.x     = xx;
        pvert->vv.y     = (*getHeightProc)( cellX, cellZ);
			  pvert->vv.z     = meterZ0;
			  pvert->diffuse  = color;
			  pvert->specular = RGBA_MAKE(0x00, 0x00, 0x00, 0xFF);
			  pvert->u        = uu;
			  pvert->v        = vv;
			  pvert++;
		  }
    }

	  // generate indices
	  U32 offset = 0;

  #ifdef DOFLIPTRI
    U32 indexer[] = { 1, 0, 5, 6, 1 };
    indexer[2] = sizex;
    indexer[3] = sizex + 1;
    S32 flip = cellZ0 % 2 ? (cellX0 % 2 ? 1 : 0) : (cellX0 % 2 ? 0 : 1);
    Bool xFlip = sizex % 2 ? TRUE : FALSE;
  #else
    U32 sizex1 = sizex + 1;
  #endif

	  for (cellZ = cellZ0; cellZ < cellZ1; cellZ++)
	  {
  #ifdef DOFLIPTRI
		  for (cellX = cellX0; cellX < cellX1; cellX++, flip = abs(flip - 1))
  #else
		  for (cellX = cellX0; cellX < cellX1; cellX++)
  #endif
		  {
        if (cellX < 0 || cellX >= (S32)CellWidth() || cellZ < 0 || cellZ >= (S32)CellHeight())
        {
          // cell off map
          offset++;
          continue;
        }
  #ifdef DOFLIPTRI

			  // upper left triangle
			  *pindex++ = (U16)(offset + indexer[flip + 0]);
			  *pindex++ = (U16)(offset + indexer[flip + 1]);
			  *pindex++ = (U16)(offset + indexer[flip + 2]);

			  // lower right triangle
			  *pindex++ = (U16)(offset + indexer[flip + 0]);
			  *pindex++ = (U16)(offset + indexer[flip + 2]);
			  *pindex++ = (U16)(offset + indexer[flip + 3]);
  #else
			  // upper left triangle
			  *pindex++ = (U16) offset;
			  *pindex++ = (U16)(offset + sizex);
			  *pindex++ = (U16)(offset + sizex1);

			  // lower right triangle
			  *pindex++ = (U16) offset;
			  *pindex++ = (U16)(offset + sizex1);
			  *pindex++ = (U16)(offset + 1);

  #endif

			  // advance the base offset
			  offset++;
      }
		  // advance the base offset
		  offset++;

  #ifdef DOFLIPTRI
      if (xFlip)
      {
        flip = abs(flip - 1);
      }
  #endif
    }

    // submit for projection and clipping
  #if 0
    if (clipFlags == clipNONE)
    {
      Vid::CurCamera().ProjectNoClip( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem);
    }
    else
  #endif
    {
  //    Vid::CurCamera().ProjectClip( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem);
      Vid::CurCamera().ProjectClipBias( NULL, vertmem, pvert - vertmem, indexmem, pindex - indexmem, *Mesh::Manager::zBias);
    }
  }
  //----------------------------------------------------------------------------
#endif

  Bool IntersectClip( Vector &pos, Vector front, F32 stepScale) // = 1.0f
  {
    front *= stepScale;

    if (front.x == 0.0f && front.y == 0.0f && front.z == 0.0f)
    {
      ASSERT( front.x != 0.0f || front.y != 0.0f || front.z != 0.0f);

      return FALSE;
    }

    F32 wid = MeterWidth()  - 0.1f;
    F32 hgt = MeterHeight() - 0.1f;

    pos.x -= OffsetX();
    pos.z -= OffsetZ();

    // check for starting point off of map
    if (pos.x < 0.0f)
    {
      if (front.x <= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 x = 1.0f / front.x * -pos.x;
      pos.z += front.z * x;
      pos.y += front.y * x;
      pos.x = 0.0f;
      if (pos.z < 0.0f)
      {
        if (front.z <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * -pos.z;
        pos.x += front.x * z;
        if (pos.x > wid)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = 0.0f;
      }
      else if (pos.z > hgt)
      {
        if (front.z >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * (hgt - pos.z);
        pos.x += front.x * z;
        if (pos.x > wid)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = hgt;
      }
    }
    if (pos.x >= wid)
    {
      if (front.x >= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 x = 1.0f / front.x * (wid - pos.x);
      pos.z += front.z * x;
      pos.y += front.y * x;
      pos.x = (F32) wid;
      if (pos.z < 0.0f)
      {
        if (front.z <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * -pos.z;
        pos.x += front.x * z;
        if (pos.x < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = 0.0f;
      }
      else if (pos.z > hgt)
      {
        if (front.z >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 z = 1.0f / front.z * (hgt - pos.z);
        pos.x += front.x * z;
        if (pos.x < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * z;
        pos.z = hgt;
      }
    }
    if (pos.z < 0.0f)
    {
      if (front.z <= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 z = 1.0f / front.z * -pos.z;
      pos.x += front.x * z;
      pos.y += front.y * z;
      pos.z = 0.0f;
      if (pos.x < 0.0f)
      {
        if (front.x <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * -pos.x;
        pos.z += front.z * x;
        if (pos.z > hgt)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = 0.0f;
      }
      else if (pos.x > wid)
      {
        if (front.x >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * (wid - pos.x);
        pos.z += front.z * x;
        if (pos.z > hgt)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = wid;
      }
    }
    if (pos.z >= hgt)
    {
      if (front.z >= 0.0f)
      {
        // pos -> front doesn't intersect map
        return FALSE;
      }
      F32 z = 1.0f / front.z * (hgt - pos.z);
      pos.x += front.x * z;
      pos.y += front.y * z;
      pos.z = (F32) hgt;
      if (pos.x < 0.0f)
      {
        if (front.x <= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * -pos.x;
        pos.z += front.z * x;
        if (pos.z < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = 0.0f;
      }
      else if (pos.x > wid)
      {
        if (front.x >= 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        F32 x = 1.0f / front.x * (wid - pos.x);
        pos.z += front.z * x;
        if (pos.z < 0.0f)
        {
          // pos -> front doesn't intersect map
          return FALSE;
        }
        pos.y += front.y * x;
        pos.x = wid;
      }
    }
    pos.x += OffsetX();
    pos.z += OffsetZ();

    return TRUE;
  }

  // returns the intersection point of the vector 'front' with the terrain in 'pos'
  // returns NULL if it reaches the edge of the terrain without an intersection
  // checks and corrects for starting pos off map
  //
  Bool Intersect( Vector &pos, Vector front, F32 stepScale, FINDFLOORPROCPTR findFloorProc) // = 1.0f, FindFloor
  {
    if (!IntersectClip( pos, front, stepScale))
    {
      return FALSE;
    }
    Vector startPos = pos;

    front *= stepScale;

    // check if pos is already below the terrain
    F32 y = (*findFloorProc)( pos.x, pos.z);
    if (pos.y < y)
    {
      return FALSE;
    }

    U32 counter; 
    for (counter = 0; counter < 10000; counter++)
    {
      if (pos.y <= y)
      {
        // FindFloor returns F32_MAX if pos is outside terrain
        if (y < F32_MAX)
        {
          pos.y = y;

#if 0 
          if (pos.x < OffsetX() || pos.x >= MeterWidth() + OffsetX() || pos.z < OffsetZ() || pos.z >= MeterHeight() + OffsetZ())
          {
            ERR_FATAL(("Invalid position (%.2f, %.2f)", pos.x, pos.z));
          }
#endif

          return TRUE;
        }
        else
        {
          return FALSE;
        }
      }
      pos += front;

      y = (*findFloorProc)( pos.x, pos.z);
    }
    LOG_ERR( ("Terrain::Intersect: overflow! start: %4.2f %4.2f %4.2f ; end: %4.2f %4.2f %4.2f ; step %4.2f %4.2f %4.2f", 
      startPos.x, startPos.y, startPos.z, pos.x, pos.y, pos.z, front.x, front.y, front.z) );

    return FALSE;
  }
  //----------------------------------------------------------------------------

  // calculate the world position corresponding to the screen position 'sx, sy'
  // return it in 'pos'
  //
  Bool ScreenToTerrain( S32 sx, S32 sy, Vector &pos, FINDFLOORPROCPTR findFloorProc) // = FindFloor)
  {
    Vid::CurCamera().ScreenToWorld( pos, sx, sy);
    Vector front = pos - Vid::CurCamera().WorldMatrix().Position();

    return Intersect( pos, front, 1.0f, findFloorProc);
  }
  //----------------------------------------------------------------------------
}
//----------------------------------------------------------------------------