///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-1999 Pandemic Studios, Dark Reign II
//
// mesh.h
//
// 07-JUL-1998
//

#ifndef __MESH_H
#define __MESH_H

#include "vid.h"
#include "nlist.h"
#include "ntree.h"
#include "array.h"
#include "varsys.h"
#include "anim.h"
#include "meshmrm.h"
#include "filter.h"
//----------------------------------------------------------------------------

#define LOG_CON_DIAG(y)  { LOG_DIAG( y); if (!Mesh::Manager::readErrors) { CON_DIAG(("")); } CON_DIAG( y); Mesh::Manager::readErrors = TRUE; }
#define WARN_CON_DIAG(y) { LOG_WARN( y); if (!Mesh::Manager::readErrors) { CON_DIAG(("")); } CON_DIAG( y); Mesh::Manager::readErrors = TRUE; }
//----------------------------------------------------------------------------

#define MAXMESHPERGROUP       64
#define MAXBUCKYS             16
#define MAXKEYFRAMES      	  512
#define MAXMATERIALS          256

#define MAXREADINDICES        (MAXINDICES * 3)
#define MAXREADTRIS           (MAXTRIS  * 3)
#define MAXREADVERTS          (MAXVERTS * 3)
#define MAXREADMESH           (MAXMESHPERGROUP * 3)
#define MAXREADBUCKYS         32

#define DEFCYCLENAME          "default"
//----------------------------------------------------------------------------

extern Bool IsInTri( Vector &pos, Vector *v, Plane &plane);
//----------------------------------------------------------------------------

// legacy render support
//
class VertGroup 
{
public:
  U16                     vertCount;          // number of verts for this group
  U16                     indexCount;         // number of indices 
  U16                     planeIndex;         // index of first plane equation
  U16                     stateIndex;         // index of the state
    
  Material *              material;
  Bitmap *                texture;

  void ClearData()
  {
    Utils::Memset( this, 0, sizeof( *this));
  }
  VertGroup()
  {
    ClearData();
  }
};
//----------------------------------------------------------------------------

// vertex to state matrix indexing with multiwieghting
//
#define MAXWEIGHTS  2
struct VertIndex
{
  U32 count;
  U16 index[MAXWEIGHTS];
  F32 weight[MAXWEIGHTS];
};
//----------------------------------------------------------------------------

class GodFile;
class MeshObj;
class MeshEnt;

// a single node in the heirarchy
//
class Mesh : public FamilyNode
{
public:
  class Manager;
protected:
  friend Manager;
  friend class MeshObj;
  friend class MeshRoot;
  friend struct MeshConfig;

  U32                     renderFlags;

public:
  U32                     isNull    : 1;
  U32                     isTread   : 1;
  U32                     isControl : 1;

  // basic mesh data
  // Array4 pads data to contain multiple of 4 entries
  //
#ifdef X__DO_XMM_BUILD
  Array4<Vector4, 4>          localVertices;
  Array4<Vector4, 4>          localNormals;
#else
  Array4<Vector, 4>           localVertices;
  Array4<Vector, 4>           localNormals;
#endif
  Array4<UVPair, 4>           localUvs;
  Array4<Color, 4>            localColors;

  // old pipe mesh data
  //
  Array<VertGroup>            localGroups;
  Array4<U16, 4>              localIndices;
//  Array<Plane>                localPlanes;

  void ClearData();

  Mesh()
  {
    ClearData();
  }
  virtual ~Mesh();

#ifdef DOLOCALRENDER
  virtual void Render( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
#endif

  U32 RenderFlags()
  {
    return renderFlags;
  }

  virtual void SetName( const char *_name);
  void Setup( const char *_name, U32 _index);

  virtual U32 GetMem();

  // static mesh manager stuff      (in meshman.cpp)
  //
  class Manager
  {
//  private:
  public:
    friend Mesh;
    friend class MeshRoot;
    friend class MeshObj;
    friend class MeshEnt;

    static NBinTree<MeshRoot>   rootTree;     // searchable tree of root MeshGroups
    static NList<MeshEnt>       entList;      // all MeshEnt's
    static Bool                 sysInit;
    static MeshRoot *           nullRoot;
    static Bitmap *             envMap;
  public:
    static VarFloat             scaleFactor;    // load controls
    static VarFloat             vertexThresh;   // optimize thresholds
    static VarFloat             normalThresh;
    static VarFloat             tcoordThresh;
    static VarInteger           doLoadGod;      // load god over xsi
    static VarInteger           doBasePose;     // ignore animation transforms
    static VarInteger           doMrmGen;       // generate mrm data
    static VarInteger           doFrogPose;     // special pose for mrmgen
    static VarInteger           doGenericMat;   // use generic 70% grey material
    static VarInteger           doOptimize;     // optimize mesh
    static VarFloat             mrmMergeThresh;
    static VarFloat             mrmNormalCrease;
    static VarInteger           mrmMultiNormals;
    static VarInteger           mipMin;         // mip minimum pixel size (8)
    static VarInteger           mipCount;       // number of mip levels   (4)
    static VarInteger           terrMipCount;   // for terrain            (4)

    static VarFloat             mrmFactor;      // runtime controls
    static VarFloat             mrmDist;
    static VarInteger           mrmAuto;        // do auto mrm adjustment
    static VarFloat             mrmAutoFactor;
    static VarInteger           mrmLow;
    static VarInteger           mrmHigh;

    static VarFloat             mrmAutoConstant1;
    static VarFloat             mrmAutoConstant2;
    static VarInteger           mrmAutoCutoff;
    static F32                  mrmCameraMove;

    static VarInteger           doMRM;          // do multi-res meshes
    static VarInteger           doMultiWeight;  // do multi-weighted anims
    static VarInteger           doInterpolate;  // interpolate between sim frames

    // appearence globals
    static U32                  vertCount, vertCountMRM, objCountMRM;

    static VarInteger           teamColor;      
    static VarInteger           baseColor;      
    static VarInteger           envAlpha;      
    static VarInteger           shadowAlpha;

    static VarInteger           showBounds;     // draw bounds
    static VarInteger           showNormals;    // draw normals
    static VarInteger           showHardPoints; // draw hardpoints
    static VarInteger           showOrigin;     // draw mesh origins
    static VarInteger           showShadows;    // draw shadows
    static VarInteger           showEnvMap;     // diplay with environment mapping
    static VarInteger           showOverlay;    // diplay with overlay texture
    static VarInteger           showMesh;       // show the mesh
    static VarInteger           showSP0;        // diplay shadow plane 0
    static VarInteger           showSP1;        // diplay shadow plane 1
    static VarInteger           render2;        // use alternate render func
    static VarInteger           entFaces;       // use instance based faces data
    static VarInteger           animOverlay;    // do anim overlay

    static VarFloat             animBlendTime;
    static VarFloat             animBlendRate;

    static VarInteger           lightQuick;
    static VarInteger           lightSingle;

    static VarString            godFilePath;    // where to save god files
    static VarString            gfgFilePath;    // where to save gfg files
    static Bool                 readErrors;     // were there xsi read errors?

    static MeshEnt *            curParent;
    static MeshEnt *            curEnt;
    static MeshEnt *            selEnt;
    static MeshObj *            curChild;
    static MeshObj *            selChild;

    // temp storage
    static TempBucket *         tempBuckets;
  
  protected:
    static MeshRoot  *FindExists( const char *meshName);
    static Bool       SetupRoot( MeshRoot &root, const char *rootName = NULL);

    static void CmdInit();
    static void CmdDone();
    static void CmdHandler(U32 pathCrc);

  public:
    static Bool Init();
    static void Done();
    static Bool InitIFace();
    static void DoneIFace();

    static void OnModeChange();

    static Bool InitResources();
    static void DisposeAll();
    static void ResetCounts();

    static void      MakeName( BuffString &buff, const char *meshName, F32 scale = 1.0f); 
    static MeshRoot *Find( const char *meshName); 
    static MeshRoot *FindRead( const char *meshName, const char *fileName = NULL); 
    static MeshRoot *FindRead( const char *meshName, F32 scale, Bool mrmGen, const char *fileName = NULL);
    static AnimList *ReadAnimCycle( const char *fileName, MeshRoot &mesh, const char *cycleName);

    // apply scale to name
    static MeshRoot *FindLoadScale( const char *godFileName); 

    // load name raw
    static MeshRoot *FindLoad( const char *godFileName); 

    // construction functions that can return NULL
    // can also use MeshEnt contructors directly
    static MeshEnt *Create( const MeshRoot *mesh);
    static MeshEnt *Create( const char *meshName);

    static void RenderList();
    static void SimulateList( F32 dt);
    static void UpdateMRMFactor();
    static MeshEnt* PickAtScreenPos(S32 x, S32 y, MeshObj **child = NULL);

    // restore all meshes to max vertex count
    static void FullResList();
    static void SetupPerf();

    static void SetupRenderProcList();
  };

protected:
  void Save( GodFile *godFile);
  void Load( GodFile *godFile, MeshRoot &root, Mesh *_parent, Bool isPrimary);

  // static data save/load helpers
  static void SaveAnimation( GodFile *god, const Animation &animation);
  static void SaveAnimCycle( GodFile *god, const AnimList &animcycle);

  static void LoadAnimation( GodFile *god, Animation &animation);
  static void LoadAnimCycle( GodFile *god, AnimList &animcycle);

  static void SaveMRMFace( GodFile *god, const MRM::Face &face);
  static void SaveMRMVertex( GodFile *god, const MRM::Vertex &vertex);
  static void SaveMRM( GodFile *god, const MeshRoot &root);

  static void LoadMRMFace( GodFile *god, MRM::Face &face);
  static void LoadMRMVertex( GodFile *god, MRM::Vertex &vertex);
  static MRM *LoadMRM( GodFile *god, const MeshRoot &root);

};
//----------------------------------------------------------------------------

// temporary read memory for xsi files
//
struct MeshRead
{
//  Array<FamilyNode>           baseMats;         // base pose mats FIXME
  Array<Matrix>               baseMats;         // base pose mats FIXME

  ~MeshRead()
  {
    baseMats.Release();
  }
};
//----------------------------------------------------------------------------

// main mesh type definition
//
class MeshRoot : public Mesh
{
//protected:
public:
  friend Mesh::Manager;
  friend Mesh;
  friend class MeshObj;
  friend class MeshEnt;
  friend struct MeshConfig;

  NBinTree<MeshRoot>::Node    treeNode;         // node for MeshMan tree
  MeshRead *                  read;

  GameIdent                   fileName;
  NameString                  xsiName;

  Array<Matrix>               stateMats;        // root space to object space transforms
  Bounds                      bigBounds;        // bounding sphere for whole hierarchy

  Array<FamilyState>          states;           // base state array

  AnimCycles                  animCycles;       // searchable tree of anim cycles
  U32                         texTimer;         // texture animation rate

  F32                         scale;            // for multiple scaled versions of the same file

  U32                         godLoad     : 1;
  U32                         chunkify    : 1;
  U32                         quickLight  : 1;
  U32                         shadowPlane : 1;
  U32                         isChunk     : 1;
  U32                         useMrm      : 1;
  U32                         envMap      : 1;
  U32                         hasAnim     : 1;
  U32                         hasTexAnim  : 1;
  U32                         hasTread    : 1;
  U32                         hasWeapon   : 1;

  // basic mesh data
  // Array4 pads data to contain multiple of 4 entries
  //
#ifdef X__DO_XMM_BUILD
  Array4<Vector4, 4>          vertices;
  Array4<Vector4, 4>          normals;
#else
  Array4<Vector, 4>           vertices;
  Array4<Vector, 4>           normals;
#endif
  Array4<UVPair, 4>           uvs;
  Array4<Color,  4>           colors;

  // mrm modified count values
  U32                         vertCount;
  U32                         normCount;
  U32                         textCount;
  U32                         faceCount;

  // mrming mesh data
  //
  Array<FaceObj>              faces;
  Array<BucketLock>           buckys;
  Array<FaceGroup>            groups;
  Array<VertIndex>            vertToState;

  MRM *                       mrm;
  F32                         mrmFactor;

  // does this root use multiweighted anim, envmapping, overlays
  //
  U32                         rootControlFlags;

#if 0
//#ifdef DOOLDPIPE
  // old pipe mesh data
  //
  Array<VertGroup>            groups;
  Array4<U16, 4>              indices;
#endif

  Array<Plane>                planes;

  Array<MeshRoot *>           chunks;
  U32                         curChunk, chunkIndex;
  Color                       chunkColor;

  F32                         shadowRadius;

  typedef void (MeshRoot::*ROOTRENDERPROCPTR)( FaceGroup * _buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags);
  ROOTRENDERPROCPTR           renderProc, renderColorProc;

protected:
  void ClearData();

  void Setup();
  void SetupPlanes();
  void SetupPlane( U32 i);
  void SetupRenderProc();
  Bool SetupStates( const FamilyState *_states, U32 count, Matrix *mats = NULL);
  Bool SetupStates( const Array<AnimKey> _states);
  Bool SetupAnimCycle( AnimList &animList, const char *cycleName = DEFCYCLENAME);

public:

  void SortFaces();

  MeshRoot()
  {
    ClearData();
    animCycles.SetNodeMember(&AnimList::cycleNode);
  }
  virtual ~MeshRoot();

  AnimList *FindAnimCycle(U32 cycleID) const
  {
    return animCycles.Find(cycleID); 
  }

  AnimList *FindAnimCycle(const char *cycle) const
  {
    return (FindAnimCycle(Crc::CalcStr(cycle)));
  }

  Bool SetAnimCycle( const char *cycleName = "default")
  {
    return SetAnimCycle( Crc::CalcStr( cycleName)); 
  }

  const Mesh * Get( const NodeIdent &ident) const
  {
    ASSERT( ident.index < states.count);

    return states[ident.index].GetMeshFromRoot();
  }

  Bool SetAnimCycle( U32 cycleID);

  const Mesh * FindIdent( NodeIdent &ident)  const;
  const Mesh * FindIdent( const char *_name, U32 &index) const
  {
    NodeIdent ident;
    ident = _name;
    const Mesh *mesh = FindIdent( ident);
    index = ident.index;
    return mesh;
  }

  void Chunkify();
  const MeshRoot *NextChunk();

  virtual void SetWorldRecurse( const Matrix & world);
  virtual void SetWorldRecurseRender( const Matrix & world, FamilyState * stateArray);

  void SetMatricesView(  const Array<FamilyState> & stateArray, Matrix *tranys) const;
  void SetMatricesWorld( const Array<FamilyState> & stateArray, Matrix *tranys) const;
  void SetMatricesModel( const Array<FamilyState> & stateArray, Matrix *tranys) const;

  // left as an inline because it always used in a loop (functions below)
  //
  void SetVert( const Matrix *tranys, U32 i, Vector &vert, Bool doMultiWeight) const
  {
	  // set a single vert with multi-weighting
	  VertIndex &vindex = vertToState[i];
	  
	  // get the vertex in local coordinates
	  Vector &v = vertices[i];

	  // transform by the zeroth matrix
	  tranys[vindex.index[0]].Transform(vert, v);
	  
	  // if we are doing multiple weights
//	  if ((Mesh::Manager::doMultiWeight) && (vindex.count > 1))
    if (doMultiWeight && (vindex.count > 1))
	  {
		  // scale by zeroth weight
		  vert *= vindex.weight[0];

		  // for each subsequent weight...
		  for (U32 j = 1; j < vindex.count; j++)
		  {
			  // transform by the Jth matrix
			  Vector vertJ;
			  tranys[vindex.index[j]].Transform(vertJ, v);

			  // scale by Jth weight
			  vertJ *= vindex.weight[j];

			  // accumulate value
			  vert += vertJ;
		  }
	  }
  }

  void SetVertsView(  const Array<FamilyState> & stateArray, Matrix *tranys, Vector *verts, Bool doMutiWeight) const;
  void SetVertsWorld( const Array<FamilyState> & stateArray, Matrix *tranys, Vector *verts, Bool doMutiWeight) const;
  void SetVertsWorld( const Array<FamilyState> & stateArray, Vector *verts, Bool doMutiWeight) const;
  void SetVertsModel( const Array<FamilyState> & stateArray, Matrix *tranys, Vector *verts, Bool doMutiWeight) const;

  void Render( const Matrix &world, Color teamColor = 0xffffffff, U32 _controlFlags = controlDEF);
  void RenderLightSun( const Matrix &world, Color teamColor = 0xffffffff, U32 _controlFlags = controlDEF);

  void RenderColor( const Matrix &world, Color color, U32 _controlFlags = controlDEF);
  void RenderColorLight( const Matrix &world, Color color, U32 _controlFlags = controlDEF);
  void RenderColorLightSun( const Matrix &world, Color color, U32 _controlFlags = controlDEF);

  void Render( const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    (this->*renderProc)( groups.data, groups.count, stateArray, teamColor, clipFlags, _controlFlags);
  }
  void RenderColor( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    (this->*renderColorProc)( groups.data, groups.count, stateArray, color, clipFlags, _controlFlags);
  }

  void RenderLightAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightNoAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightNoAnimColorsI(  FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);

  void RenderColorAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorNoAnimI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorNoAnimColorsI( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);

  void RenderLightAnimI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightNoAnimI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightAnimColorsI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightNoAnimColorsI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);

  void RenderColorAnimI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorNoAnimI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorAnimColorsI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorNoAnimColorsI_2( FaceGroup *_buckys, U32 buckyCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);

  void RenderLightAnim( const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderLightAnimI_2( groups.data, groups.count, stateArray, teamColor, clipFlags, _controlFlags);
  }
  void RenderLightNoAnim(  const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderLightNoAnimI_2( groups.data, groups.count, stateArray, teamColor, clipFlags, _controlFlags);
  }
  void RenderLightAnimColors( const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderLightAnimColorsI_2( groups.data, groups.count, stateArray, teamColor, clipFlags, _controlFlags);
  }
  void RenderLightNoAnimColors(  const Array<FamilyState> & stateArray, Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderLightNoAnimColorsI_2( groups.data, groups.count, stateArray, teamColor, clipFlags, _controlFlags);
  }

  void RenderColorAnim( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderColorAnimI_2( groups.data, groups.count, stateArray, color, clipFlags, _controlFlags);
  }
  void RenderColorNoAnim( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderColorNoAnimI_2( groups.data, groups.count, stateArray, color, clipFlags, _controlFlags);
  }
  void RenderColorAnimColors( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderColorAnimColorsI_2( groups.data, groups.count, stateArray, color, clipFlags, _controlFlags);
  }
  void RenderColorNoAnimColors( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    RenderColorNoAnimColorsI_2( groups.data, groups.count, stateArray, color, clipFlags, _controlFlags);
  }

  void RenderShadowPlane( U32 number, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL);

  void RenderNormals( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL);
  void RenderSelVerts( Bool show, const List<U16> * verts, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags = clipALL);
  void RenderSelFaces( Bool show, const List<U16> * faces, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags = clipALL);

#if 0
  // old render functions
  void RenderLightOldPipe( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderLightProjectOldPipe( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderLightProjectIndexed( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderLight( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderLightIndexed( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderOldPipe( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderDo( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
  void RenderIndexed( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
#endif

  void CalcBoundingSphere( Sphere &sphere);
  void CalcBoundingSphere( Bool checkAnims = FALSE);

  const FamilyNode * CollidePoly( const Array<FamilyState> & stateArray, const Vector &vStart, const Vector &vEnd, F32 &t) const;

  void CalcStatistics( U32 &_vertCount, U32 &_triCount) const;

  Bool MRMGen( List<U16> * verts = NULL);
  void MRMUpdateFaces( MRM::Vertex *vertex, Direction direction, Array<FaceGroup> & _groups);
  U32  MRMSetVertCount( U32 r, Array<FaceGroup> & _groups);

  void CompressStates();

  Bool WriteXSI( const char *fileName);

  Bool Save( const char *fileName);
  Bool SaveScale( const char *fileName);    // append scale to name
  Bool Load( const char *fileName);
  Bool Save(GodFile *godFile);
  Bool Load(GodFile *godFile);

  virtual U32 GetMem() const;
  U32 GetMemGeometry() const;
  U32 GetMemAnim() const;
  U32 GetMemMRM() const;
  U32 GetMemChunks() const;
  U32 ReportMem() const;
  U32 Report() const;

  const Bounds &ObjectBounds() const
  {
    return bigBounds;
  }

  U32 VertCount() const
  {
    return vertices.count;
  }
  U32 NormCount() const
  {
    return normals.count;
  }
  U32 UVCount() const
  {
    return uvs.count;
  }
  U32 ColorCount() const
  {
    return colors.count;
  }
  Vector *Vertices() const
  {
    return &vertices[0];
  }
  Vector *Normals() const
  {
    return &normals[0];
  }
  UVPair *UVs() const
  {
    return &uvs[0];
  }
  Color *Colors() const
  {
    return colors.count ? &colors[0] : NULL;
  }

  Color &VertColor( U32 index) const
  {
    return colors[index];
  }

  Vector &VertVector( U32 index) const
  {
    return vertices[index];
  }
  Vector &VertNormal( U32 index) const
  {
    return normals[index];
  }
  UVPair &VertUV( U32 index) const
  {
    return uvs[index];
  }

  F32 &VertVectorX( U32 index) const
  {
    return VertVector( index).x;
  }
  F32 &VertVectorY( U32 index) const
  {
    return VertVector( index).y;
  }
  F32 &VertVectorZ( U32 index) const
  {
    return VertVector( index).z;
  }

  F32 &VertNormalX( U32 index) const
  {
    return VertNormal( index).x;
  }
  F32 &VertNormalY( U32 index) const
  {
    return VertNormal( index).y;
  }
  F32 &VertNormalZ( U32 index) const
  {
    return VertNormal( index).z;
  }

  F32 &VertU( U32 index) const
  {
    return VertUV( index).u;
  }
  F32 &VertV( U32 index) const
  {
    return VertUV( index).v;
  }
};
//----------------------------------------------------------------------------

#endif    // __MESH_H


#if 0
void MeshRoot::CalcBoundingBox( Vector &min, Vector &max)
{
  min.Set(  FLT_MAX,  FLT_MAX,  FLT_MAX);
	max.Set( -FLT_MAX, -FLT_MAX, -FLT_MAX);

	// get all vertices in world coordinates
  Vector *verts = (Vector *) Vid::tempVertices;
	SetVertsWorld( states, verts);

  // calc bounding boxes
  U32 i;
	for (i = 0; i < vertices.count; i++)
	{
    // get the vertex position
	  Vector &v = verts[i];

		// update the bounding box
		if (v.x < min.x)
		{
			min.x = v.x;
		}
		if (v.y < min.y)
		{
			min.y = v.y;
		}
		if (v.z < min.z)
		{
			min.z = v.z;
		}
		if (v.x > max.x)
		{
			max.x = v.x;
		}
		if (v.y > max.y)
		{
			max.y = v.y;
		}
		if (v.z > max.z)
		{
			max.z = v.z;
		}
  }
}
//----------------------------------------------------------------------------
#endif
