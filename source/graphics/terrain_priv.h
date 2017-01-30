///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// terrain_priv.h     DR2 terrain system
//
// 04-MAY-1998
//

#ifndef _TERRAINPRIV_H
#define _TERRAINPRIV_H

#include "terrain.h"
#include "effects_utils.h"
//----------------------------------------------------------------------------

namespace Terrain
{
  extern U32             sysInit;
  extern U32             mapInit;

  extern S32             fogFactorsS32[];
  extern F32             fogFactorsF32[];
  extern U16             clusterI[96];

  extern Material *      waterMaterial;
  extern Material *      mirrorMaterial;

  extern F32             offMapHeight;

  extern HeightField     heightField;
  extern HeightField     randomField;

  extern F32             terrAverageHeight;
  extern F32             terrMinHeight;
  extern F32             terrMaxHeight;

  extern F32             invMeterHeight;
  extern F32             invMeterWidth;

  extern U32             meterPerClus, clusWidth, clusHeight;
  extern F32             clusPerMeter;

  extern F32             moveSpeed;

  extern Bitmap *        shadowTex;

  struct WaterRegion
  {
    Area<F32> rect, mRect;
    F32 height;
    Camera cam;
  };
  extern Array<WaterRegion> waterList;
  extern U32             waterCount;
  extern WaterRegion     * bigWater;

  extern void         (* renderFunc)();
  extern void         (* renderMirrorMaskFunc)( WaterRegion * waterR);

  namespace Sky
  {
    void Render();

    void Load( const char * meshName, const char * skyTexName = NULL, const char * cloudTexName = NULL);

    F32  GetLevel();
    void SetLevel( F32 l);

    Bitmap * EnvTexture();
    Bitmap * CloudTexture();

    extern Vector windVector;
  }

  enum ImportFlags
  {
    IMPORTNONE        = 0,
    IMPORTSAVE        = 1,
    IMPORTFIT         = 2,
  };

  Bool Init();
  void Done();
  Bool Setup( U32 wid, U32 hgt, U32 csize, F32 offsetX = 0.0f, F32 offsetZ = 0.0f, F32 startHeight = 10.0f);

  Bool SetTexClamp( Bool clamp = TRUE);
  Bool SetWater( Bool on = TRUE);
  Bool SetShroud( Bool on = TRUE);

  Bool WaterInView( F32 & height);
  Bool GetVisibleClusterRect( Area<S32> & rect);
  Bool GetVisibleWaterRect( Area<S32> & rect);

  // HeightField inlines
  //
  inline U32 MeterWidth()
  {
    return heightField.meterWidth;
  }
  inline U32 MeterHeight()
  {
    return heightField.meterHeight;
  }
  inline F32 InvMeterWidth()
  {
    return invMeterWidth;
  }
  inline F32 InvMeterHeight()
  {
    return invMeterHeight;
  }
  inline U32 CellWidth()
  {
    return heightField.cellWidth;
  }
  inline U32 CellPitch()
  {
    return heightField.cellPitch;
  }
  inline U32 CellHeight()
  {
    return heightField.cellHeight;
  }
  inline U32 CellMax()
  {
    return heightField.cellMax;
  }
  inline U32 MeterPerCell()
  {
    return heightField.meterPerCell;
  }
  inline F32 CellPerMeter()
  {
    return heightField.cellPerMeter;
  }
  inline U32 MeterPerCluster()
  {
    return meterPerClus;
  }
  inline F32 ClusterPerMeter()
  {
    return clusPerMeter;
  }
  inline U32 ClusterWidth()
  {
    return clusWidth;
  }
  inline U32 ClusterHeight()
  {
    return clusHeight;
  }

  // HeightField::Cell inlines
  //
  inline Bool CellCoords( F32 x, F32 z, S32 & cx, S32 & cz)
  {
    return heightField.CellCoords( x, z, cx, cz);
  }
  inline U32 CellOffset( F32 x, F32 z)
  {
    return heightField.CellOffset( x,  z);
  }
  inline F32 GetCellHeight( U32 offset)
  {
    return heightField.GetCellHeight( offset);
  }
  inline F32 GetCellHeight( F32 x, F32 z)
  {
    return heightField.GetCellHeight( x, z);
  }
  inline F32 SetCellHeight( U32 offset, F32 h)
  {
    return heightField.SetCellHeight( offset, h);
  }
  inline F32 SetCellHeight( F32 x, F32 z, F32 h)
  {
    return heightField.SetCellHeight( x, z, h);
  }
  inline Bool GetCellVisible( U32 offset)
  {
    return heightField.GetCellVisible( offset);
  }
  inline Bool SetCellVisible( U32 offset, Bool vis)
  {
    return heightField.SetCellVisible( offset, vis);
  }
  inline Bool GetCellVisible( F32 x, F32 z)
  {
    return heightField.GetCellVisible( x, z);
  }
  inline Bool SetCellVisible( F32 x, F32 z, Bool vis)
  {
    return heightField.SetCellVisible( x, z, vis);
  }
  inline U8 GetCellFog( U32 offset)
  {
    return heightField.GetCellFog( offset);
  }
  inline U8 SetCellFog( U32 offset, U8 fog)
  {
    return heightField.SetCellFog( offset, fog);
  }
  inline U8 GetCellFog( F32 x, F32 z)
  {
    return heightField.GetCellFog( x, z);
  }
  inline U8 SetCellFog( F32 x, F32 z, U8 fog)
  {
    return heightField.SetCellFog( x, z, fog);
  }
  inline Color GetCellColor( U32 offset)
  {
    return heightField.GetCellColor( offset);
  }
  inline Color SetCellColor( U32 offset, Color color)
  {
    return heightField.SetCellColor( offset, color);
  }
  inline Color GetCellColor( F32 x, F32 z)
  {
    return heightField.GetCellColor( x, z);
  }
  inline Color SetCellColor( F32 x, F32 z, Color color)
  {
    return heightField.SetCellColor( x, z, color);
  }

  inline void GetPos( Cell & cell, Vector & pos)
  {
    heightField.GetPos( cell, pos);
  }
  inline Cell * GetCell( S32 cx, S32 cz)
  {
    return heightField.GetCell( cx, cz);
  }
  inline void CellCoords( Cell & cell, S32 & cx, S32 & cz)
  {
    heightField.CellCoords( cell, cx, cz);
  }
  inline Bool CellOnMap(S32 cellX, S32 cellZ)
  {
    return heightField.CellOnMap( cellX, cellZ);
  }
  inline Bool MeterOnMap(F32 x, F32 z)
  {
    return heightField.MeterOnMap( x, z);
  }
  inline F32 OffsetX()
  {
    return heightField.offsetX;
  }
  inline F32 OffsetZ()
  {
    return heightField.offsetZ;
  }

  inline void CellHeights( U32 cx, U32 cz, F32 * heights)
  {
    ASSERT(CellOnMap(cx, cz));

    heightField.CellHeights(cz * CellPitch() + cx, heights);
  }

  Bool FindWater(F32 x, F32 z, F32 * height = NULL, F32 * depth = NULL);
  F32  FindFloor( F32 x, F32 z, Vector * surfNormal = NULL);
  F32  FindFloorWithWater( F32 x, F32 z, Vector * surfNormal = NULL);

  // doesn't check for valid cell coords
  //
  F32  GetHeight( S32 cx, S32 cz);
  Bool GetWater( S32 cx, S32 cz, F32 * height = NULL);
  F32  GetHeightWithWater( S32 cx, S32 cz);
  F32  GetHeightWater( S32 cx, S32 cz);

  Cluster & GetCluster(F32 x, F32 z);
  Cluster & GetCluster(U32 cx, U32 cz);
  void GetClusterOffsets(F32 x, F32 z, U32 & cx, U32 & cz);
  U32 ClusterOffset( F32 x, F32 z);
  void CalcClusSphere( S32 x, S32 z);
  void CalcCellRect( const Area<S32> & rect);
  void Recalc();
  void SetupPerf();
	void CalcLighting();

	U32	GetMem();
  U32 Report( Bool all = FALSE);
  void BuildNormals();

  Bool ImportBitmap( char * buffer, U32 bwid, U32 bhgt, F32 scale, Area<S32> & rect, U32 csize = CELLSIZE, U32 flags = 0);
  void Paste( Area<S32> & dstRect, HeightField & buf, Area<S32> & bufRect, F32 scale, U32 flags = HeightField::EDITHEIGHTS, F32 atHeight = 0);

  void SetBaseTexName( const char * name);
  S32 AddTexture( const char * name);

  Bool Load( BlockFile & bfile);
	Bool Save( BlockFile & bfile);
  void PurgeTextures();
 
  U32 FindNormal( Vector & normal);
  U32 FindNormalQuick( U32 offset);

  void Simulate( F32 timestep);
  void SimulateViewer( F32 timestep);

  typedef F32 (*FINDFLOORPROCPTR)( F32 x, F32 z, Vector * surfNormal = NULL);

  Bool Intersect( Vector & pos, Vector front, F32 stepScale = 1.0f, const FINDFLOORPROCPTR findFloorProc = FindFloor, F32 range = F32_MAX);
  Bool ScreenToTerrain( S32 sx, S32 sy, Vector &pos, FINDFLOORPROCPTR findFloorProc = FindFloor);

  inline Bool IntersectWithWater( Vector & pos, Vector front, F32 stepScale = 1.0f)
  {
    return Intersect( pos, front, stepScale, FindFloorWithWater);
  }
  inline Bool ScreenToTerrainWithWater( S32 sx, S32 sy, Vector &pos)
  {
    return ScreenToTerrain( sx, sy, pos, FindFloorWithWater);
  }

  typedef F32 (*GETHEIGHTPROCPTR)( S32 cx, S32 cz);

  Bucket * RenderGroundSprite( const Vector & origin, F32 radx, F32 rady, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1),
    const GETHEIGHTPROCPTR getHeightProc = GetHeight, U16 sorting = Vid::sortGROUND0 );
  Bucket * RenderGroundSprite( const Vector & origin, F32 radx, F32 rady, U32 clipFlags, F32 viewz, const Bitmap *texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1),
    const GETHEIGHTPROCPTR getHeightProc = GetHeight, U16 sorting = Vid::sortGROUND0);
  Bucket * RenderGroundSprite( const Vector & origin, F32 radx, F32 rady, const Vector &front, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1),
    const GETHEIGHTPROCPTR getHeightProc = GetHeight, U16 sorting = Vid::sortGROUND0);

  inline Bucket * RenderGroundSpriteWithWater( const Vector & origin, F32 radx, F32 rady, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return RenderGroundSprite( origin, radx, rady, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  } 
  inline Bucket * RenderGroundSpriteWithWater( const Vector & origin, F32 radx, F32 rady, U32 clipFlags, F32 viewz, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return RenderGroundSprite( origin, radx, rady, clipFlags, viewz, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }
  inline Bucket * RenderGroundSpriteWithWater( const Vector & origin, F32 radx, F32 rady, const Vector & front, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortSURFACE0)
  {
    return RenderGroundSprite( origin, radx, rady, front, texture, color, blend, uv0, uv1, uv2, GetHeightWithWater, sorting);
  }

  Bucket * RenderWaterSprite( const Vector & origin, F32 radius, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0);
  Bucket * RenderWaterSprite( const Vector & origin, F32 radius, U32 clipFlags, F32 viewz, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0);
  Bucket * RenderWaterSprite( const Vector & origin, F32 radius, const Vector & front, const Bitmap * texture, Color color, U32 blend = RS_BLEND_DEF, 
    UVPair uv0 = UVPair(0,0), UVPair uv1 = UVPair(1,0), UVPair uv2 = UVPair(1,1), U16 sorting = Vid::sortGROUND0);

  inline Bucket * RenderShadow( const ShadowInfo & si, const GETHEIGHTPROCPTR getHeightProc) // = GetHeight)
  {
    return RenderGroundSprite( si.p, si.radx, si.radx, si.clipFlags, si.z, si.texture, si.color, si.blend, si.uv0, si.uv1, si.uv2, getHeightProc, Vid::sortSURFACE0);
  }
  inline Bucket * RenderLight( const ShadowInfo & si, const GETHEIGHTPROCPTR getHeightProc = GetHeight)
  {
    return RenderShadow( si, getHeightProc);
  }
  Bool BoundsTestShadow( MeshEnt & ent, const GETHEIGHTPROCPTR getHeightProc = GetHeight, const FINDFLOORPROCPTR findFloorProc = FindFloor);
  Bucket * RenderShadow( MeshEnt & ent, const GETHEIGHTPROCPTR getHeightProc = GetHeight);
  inline Bool BoundsTestShadowWithWater( MeshEnt & ent)
  {
    return BoundsTestShadow( ent, GetHeightWithWater, FindFloorWithWater);
  }
  inline Bucket * RenderShadowWithWater( MeshEnt & ent)
  {
    return RenderShadow( ent, GetHeightWithWater);
  }

  Bool BoundsTestShadow( ShadowInfo & si, const Matrix & world, F32 radius, U32 shadowType = MeshRoot::shadowOVAL, U32 translucency = 255, const GETHEIGHTPROCPTR getHeightProc = GetHeight, const FINDFLOORPROCPTR findFloorProc = FindFloor); 
  Bucket * RenderShadow( const Vid::Light::Obj & light, const Matrix & world, F32 radius, U32 shadowType = MeshRoot::shadowOVAL, U32 translucency = 255, const GETHEIGHTPROCPTR getHeightProc = GetHeight, const FINDFLOORPROCPTR findFloorProc = FindFloor);
  Bucket * RenderShadow( const Matrix & world, F32 radius, U32 shadowType = MeshRoot::shadowOVAL, U32 translucency = 255, const GETHEIGHTPROCPTR getHeightProc = GetHeight, const FINDFLOORPROCPTR findFloorProc = FindFloor);

  inline Bool BoundsTestShadowWithWater( ShadowInfo & si, const Matrix & world, F32 radius, U32 shadowType = MeshRoot::shadowOVAL, U32 translucency = 255) 
  {
    return BoundsTestShadow( si, world, radius, shadowType, translucency, GetHeightWithWater, FindFloorWithWater); 
  }
  inline Bucket * RenderShadowWithWater( const Matrix & world, F32 radius, U32 shadowType = MeshRoot::shadowOVAL, U32 translucency = 255)
  {
    return RenderShadow( world, radius, shadowType, translucency, GetHeightWithWater, FindFloorWithWater);
  }

  Bool BoundsTestLight( ShadowInfo & si, const Vid::Light::Obj & light, const GETHEIGHTPROCPTR getHeightProc = GetHeight, const FINDFLOORPROCPTR findFloorProc = FindFloor); 
  Bucket * RenderLight( const Vid::Light::Obj & light, const GETHEIGHTPROCPTR getHeightProc = GetHeight, const FINDFLOORPROCPTR findFloorProc = FindFloor);

  inline Bool BoundsTestLightWithWater( ShadowInfo & si, const Vid::Light::Obj & light) 
  {
    return BoundsTestLight( si, light, GetHeightWithWater, FindFloorWithWater);
  }
  inline Bucket * RenderLightWithWater( const Vid::Light::Obj & light)
  {
    return RenderLight( light, GetHeightWithWater, FindFloorWithWater);
  }

  inline void Render()
  {
    (*renderFunc)();
  }
  void RenderVtl();
  void RenderV();
  void RenderQuick();

  inline void RenderMirrorMask( WaterRegion * waterR)
  {
    (*renderMirrorMaskFunc)( waterR);
  }
  void RenderMirrorMaskVtl( WaterRegion * waterR);
  void RenderMirrorMaskV( WaterRegion * waterR);
  void RenderMirrorMaskQuick( WaterRegion * waterR);

  void RenderCellRect( Area<S32> & rect, Color color, Bool dofill = FALSE, Bool doFade = FALSE, Bool doTile = TRUE, Bitmap *tex = NULL, F32 rotation = 0);
  void RenderCell( Cell & cell, Color color, Bitmap * tex = NULL);
  void RenderIsometric( Bool doColor = FALSE, Bool doOverlay = FALSE);  

#ifdef DOTERRAINLOD

  void SetClusLod( U32 offset, Bool flag);
  void ClusSetParents( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, U32 level = 0)
  Bool ChekTri( F32 py, U32 o0, U32 o1, U32 o2, U32 level = 0);
  Bool DrawTri( U32 o0, U32 o1, U32 o2, U32 i0, U32 i1, U32 i2, Vector * verts, U16 ** indexList, U32 level = 0);
  void RenderClusterLOD( Cluster & clus, S32 x, S32 z, U32 cellOffset, U32 clipFlags = clipALL);
  void RenderClusterLODQuick( Cluster & clus, S32 x, S32 z, U32 cellOffset, U32 clipFlags = clipALL);

#endif

  void RenderTerrainMap( const char * filename, U32 size = 1024, Bool doColor = TRUE, Bool doOverlay = TRUE);

  extern UVPair           overlayUVList[256];

  extern Bool             waterInView;
  extern Area<S32>        waterRect;
  extern Vector           waterNorms[25];
  extern F32              waterWave0, waterWave1;
  extern Bool             waterLayer2;
  extern Bool             waterIsAlpha;
  extern UVPair           waterUVList0[25];
  extern UVPair           waterUVList1[25];
  extern F32              lowWaterHeight;
  extern U32              lowWaterCount;
  extern Bool             lowWaterFirst;

  extern UVPair           cellUVList[4 * 4 * 3];
  extern UVPair           overlayUVList[256];

  extern Vector           normList[NORMALCOUNT];  
	extern Color            normLights[NORMALCOUNT];  

  extern U32              renderFlags;
  extern Bitmap           * waterTex, * defTex, * editTex;
  extern Bitmap           * texList[TEXTURECOUNT];
  extern Bitmap           * overlayList[TEXTURECOUNT];

  extern U32              texCount, overlayCount;

  extern U32              meterPerClus, cellPerClus; // in one dimension
  extern Cluster          * clusList;

  extern S32              shroudRate1;
  extern Color            waterColorMirror;

  struct Overlay
  {
    U32             style, blend;
    Point<S32>      size;
    Array<UVPair>   uvs;

    void GetName( GameIdent & buff)
    {
      GameIdent blendBuff;
      Effects::Blend::GetString( blend, blendBuff);
      sprintf( buff.str, "%2d x %2d x %3d  %s", size.x, size.z, style, blendBuff.str);
    }

    void Swap( Overlay & overlay)
    {
      blend = overlay.blend;
      style = overlay.style;
      size  = overlay.size;
      uvs.Swap( overlay.uvs);
    }

    void Release()
    {
      uvs.Release();
    }
  };

  extern Array<Overlay>   overlays;

  S32  AddOverlayTexture( const char * name);
  S32  AddOverlay( Point<S32> _size, U32 _style, U32 _blend = RS_BLEND_MODULATE);
  void ApplyOverlay( S32 cx, S32 cz, U32 overlayIndex, U32 textureIndex);
  void RemoveOverlay( S32 cx, S32 cz, Point<S32> size);

  void SetRenderFunc();

}
//----------------------------------------------------------------------------

#endif