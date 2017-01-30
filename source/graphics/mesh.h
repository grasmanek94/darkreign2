///////////////////////////////////////////////////////////////////////////////
//
// Copyright 1997-2000 Pandemic Studios, Dark Reign II
//
// mesh.h
//
// 07-JUL-1998
//
// Classes for vertex meshes with animation via hierarchical bones
//
// Meshes are loaded in any configuration, but are optimized and condensed 
// into a single 'skin' that gets deformed by weighted hierarchical 'bones'.
// Most optimization occurs during xsi import, but the hierarchy isn't pruned 
// of useless nodes until after all animation cycles are loaded and a god file
// is produced from the mesh viewer tool.

#ifndef __MESH_H
#define __MESH_H

#include "vid.h"
#include "bucketdesc.h"
#include "anim.h"
#include "nlist.h"
#include "ntree.h"
#include "meshmrm.h"
//----------------------------------------------------------------------------

// write message to meshviewer error dialog and to the standard log file
#define LOG_CON_DIAG(y)  { LOG_DIAG( y); if (!Mesh::Manager::readErrors) { CON_DIAG(("")); } CON_DIAG( y); Mesh::Manager::readErrors = TRUE; }
#define WARN_CON_DIAG(y) { LOG_WARN( y); if (!Mesh::Manager::readErrors) { CON_DIAG(("")); } CON_DIAG( y); Mesh::Manager::readErrors = TRUE; }
//----------------------------------------------------------------------------

#define MAXMESHPERGROUP       64
#define MAXBUCKYS             16
#define MAXKEYFRAMES      	  512
#define MAXMATERIALS          256

#define MAXREADMESH           (MAXMESHPERGROUP * 4)

#define DEFCYCLENAME          "default"
//----------------------------------------------------------------------------

extern Bool IsInTri( Vector &pos, Vector *v, Plane &plane);
//----------------------------------------------------------------------------

// legacy render support
//
struct VertGroup 
{
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

struct MeshData
{
  // basic mesh data
  // Array4 pads data to contain multiple of 4 entries
  //
#ifdef X__DO_XMM_BUILD
  Array4<Vector4, 4>  vertices;
  Array4<Vector4, 4>  normals;
#else
  Array4<Vector, 4>   vertices;
  Array4<Vector, 4>   normals;
#endif
  Array4<UVPair, 4>   uvs;
  Array4<Color, 4>    colors;

  Array<VertGroup>    groups;
  Array4<U16, 4>      indices;
//  Array<Plane>      planes;

  MeshData() {}
  MeshData( U32 vCount, U32 iCount, U32 nCount, U32 uCount, U32 cCount)
  {
	  // one material per mesh for now
    groups.Alloc( 1);
    indices.Alloc( iCount);
    vertices.Alloc( vCount);
    normals.Alloc( nCount);
    uvs.Alloc( uCount);
    colors.Alloc( cCount);
  }
  MeshData( MeshData & mdata)
  {
    groups.Setup( mdata.groups);
    indices.Setup( mdata.indices);
    vertices.Setup( mdata.vertices);
    normals.Setup( mdata.normals);
    uvs.Setup( mdata.uvs);
    colors.Setup( mdata.colors);
  }

  ~MeshData()
  {
    groups.Release();
    indices.Release();
    vertices.Release();
    normals.Release();
    uvs.Release();
    colors.Release();
  }

  void Swap( MeshData & mdata)
  {
    groups.Swap( mdata.groups);
    indices.Swap( mdata.indices);
    vertices.Swap( mdata.vertices);
    normals.Swap( mdata.normals);
    uvs.Swap( mdata.uvs);
    colors.Swap( mdata.colors);
  }

};
//----------------------------------------------------------------------------

class GodFile;
class MeshObj;
class MeshEnt;

// a single node in Mesh hierarchy
// Mesh is a vestigial class from before meshes were condensed into single skins.
// Besides the possiblity of local mesh data (not really used in DR2; only as a container
// shadow plane data) only a little state data is left in it. It could be pretty easily
// removed and its state data passed on to FamilyState.
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

  U32                     renderFlags;    // double sided
  
  U32 GetMem() const;

public:
  
  MeshData *              local;          // shadow planes are stored as local MeshData

  U32                     isNull        : 1;
  U32                     isTread       : 1;
  U32                     isControl     : 1;
  U32                     isShadowPlane : 1;

  void ClearData();

  Mesh()
  {
    ClearData();
  }
  virtual ~Mesh();

  void AllocLocal()
  {
    ReleaseLocal();
    local = new MeshData;
  }
  void AllocLocal( U32 vCount, U32 iCount, U32 nCount, U32 uCount, U32 cCount)
  {
    ReleaseLocal();
    local = new MeshData( vCount, iCount, nCount, uCount, cCount);
  }
  void SetupLocal( MeshData & mdata)
  {
    local = new MeshData( mdata);
  }
  void SwapLocal( MeshData & mdata)
  {
    local = new MeshData();
    local->Swap( mdata);
  }

  void ReleaseLocal();

#ifdef DOLOCALRENDER
  virtual void Render( const Array<FamilyState> & stateArray, U32 clipFlags = clipALL);
#endif

  U32 RenderFlags()
  {
    return renderFlags;
  }

  virtual void SetName( const char *_name);
  void Setup( const char *_name, U32 _index);

  // static mesh manager stuff      (in meshman.cpp)
  //
  // This should really become part of the namespace Mesh with the actual classes 
  // becoming something like Mesh::Root and Mesh::Ent
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
    static MeshRoot *           nullRoot;     // global null root
    static Bitmap *             envMap;
    static Vector               shadowVector; // limit shadow stretching

  public:
    // appearence globals
    static U32                  vertCount, vertCountMRM, objCountMRM;

    static Bool                 readErrors;     // were there xsi read errors?

    static MeshEnt *            curParent;     // for common selection between 
    static MeshEnt *            curEnt;        // DR2 and the mesh viewer
    static MeshEnt *            selEnt;
    static MeshObj *            curChild;
    static MeshObj *            selChild;

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
    static MeshRoot * MakeMesh( U32 vCount, U32 nCount, U32 uvCount, U32 fCount, Bitmap * texture);
    static MeshRoot * MakeGlobe( F32 radius, U32 bands, Bitmap * texture);

    static U32 Report();
    static U32 Report( MeshRoot & root, Bool all = FALSE);
    static U32 ReportList( const char * name = NULL);
  };

protected:
  void Save( GodFile *godFile);
  void Load( GodFile *godFile, MeshRoot &root, Mesh *_parent, Bool isPrimary);

  // static data save/load helpers
  static void SaveAnimation( GodFile *god, const Animation &animation);
  static void SaveAnimCycle( GodFile *god, const AnimList &animcycle);

  static void LoadAnimation( GodFile *god, Animation &animation, AnimType type);
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
  Array<Matrix>               baseMats;         // base pose mats FIXME

  ~MeshRead()
  {
    baseMats.Release();
  }
};
//----------------------------------------------------------------------------

// main mesh type definition
//
// MeshRoot condenses the vertex data of any group of hierarchically related meshes
// into a single 'skin' that can be easily mrm'ed and deformed by weighted animating
// bones at the same time.
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
  MeshRead *                  read;             // temp read data

  GameIdent                   fileName;
  NameString                  xsiName;

  Array<Matrix>               stateMats;        // root space to object space transforms
  Bounds                      bigBounds;        // bounding sphere for whole hierarchy
  Bounds                      fixedBounds;      // bounding sphere for default pose

  Array<FamilyState>          states;           // base state array

  AnimCycles                  animCycles;       // searchable tree of anim cycles
  F32                         texTimer;         // texture animation rate

  F32                         scale;            // for multiple scaled versions of the same file

  // shadowtypes
  enum
  {
    shadowOVAL,
    shadowGENERIC,
    shadowSEMILIVE,
    shadowLIVE,
  };
  U32                         shadowType  : 2;
  U32                         godLoad     : 1;    // was it loaded from a god file?
  U32                         chunkify    : 1;    // should chunks be auto produced?
  U32                         quickLight  : 1;    // light as if it has a single normal?
  U32                         noLight     : 1;    // full bright?
  U32                         shadowPlane : 1;    // has a shadow plane?
  U32                         isChunk     : 1;    // is this root an auto produced chunk?
  U32                         useMrm      : 1;
  U32                         envMap      : 1;    // display with enviroment mapping?
  U32                         hasAnim     : 1;    // does the mesh have anim cycles?
  U32                         hasTexAnim  : 1;    // does it have animating textures?
  U32                         hasTread    : 1;    // does it have treads?
  U32                         hasControl  : 1;    // does it have code controlled joints (CP-)

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
  Array4<UVPair, 4>           uvs2;
  Array<F32>                  vOffsets;           // tread offsets per state

  Array4<Color,  4>           colors;

  Array4<VertexI, 4>          vertex;             // experimental hardwareTL render

  // mrming mesh data
  //
  Array<FaceObj>              faces;
  Array<BucketLock>           buckys;
  Array<FaceGroup>            groups;
  Array<VertIndex>            vertToState;

  MRM *                       mrm;                // reworked mrm data
  F32                         mrmFactor;
  U32                         vertCount, faceCount;

  // does this root use multiweighted anim, envmapping, overlays
  //
  U32                         rootControlFlags;

  Array<Plane>                planes;             // plane data for rest state tris

  Array<MeshRoot *>           chunks;             // auto generated chunk meshes
  U32                         curChunk, chunkIndex;
  Color                       chunkColor;

  F32                         shadowRadius;
  U32                         shadowTime;     // semilive shadow
  Bitmap *                    shadowTexture;
  ShadowInfo                  shadowInfo;     // generic shadow

  F32                         treadPerMeter;     // convert meterPerSec to treadPerSec

  F32                         nearFadeFactor;     // camera collide fade behavior

  typedef void (MeshRoot::*ROOTRENDERPROCPTR)( Array<FaceGroup> & _buckys, U32 vertCount, const Array<FamilyState> & stateArray, Color color, U32 clipFlags, U32 _controlFlags);
  ROOTRENDERPROCPTR           renderProc;       // render function pointer

protected:
  void ClearData();

  // check if the mesh fits within the current temp mem space limits
  Bool Check( U32 maxBuckys = MAXBUCKYS, U32 maxVerts = 0xffffffff, U32 maxIndices = 0xffffffff);
  void Rebuild();

  void Setup();
  void SetupPlanes();
  void SetupPlane( U32 i);
  void SetupRenderProc();
  Bool SetupStates( const FamilyState *_states, U32 count, Matrix *mats = NULL);
  Bool SetupStates( const Array<AnimKey> _states);
  Bool SetupAnimCycle( AnimList &animList, const char *cycleName = DEFCYCLENAME);
  void CalcEnvMapTexCoords();
  void CalcBoundingSphere( Sphere & sphere);

  void SetMatricesView(  const Array<FamilyState> & stateArray, Matrix *tranys) const;
  void SetMatricesWorld( const Array<FamilyState> & stateArray, Matrix *tranys) const;
  void SetMatricesModel( const Array<FamilyState> & stateArray, Matrix *tranys) const;
  void SetMatricesWorldInverseZ( const Array<FamilyState> & stateArray, Matrix *tranys) const;

  // report memory usage
  //
  U32 GetMemGeometry() const;
  U32 GetMemAnim() const;
  U32 GetMemMRM() const;
  U32 GetMemChunks() const;

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

  // left as an inline because it always used in a loop (functions below)
  //
  void SetVert( Vector & dst, const Vector & src, const Matrix * tranys, const VertIndex & vindex, Bool doMultiWeight) const
  {
	  // transform by the zeroth matrix
	  tranys[vindex.index[0]].Transform( dst, src);
	  
	  // if we are doing multiple weights
//	  if ((Mesh::Manager::doMultiWeight) && (vindex.count > 1))
    if (doMultiWeight && (vindex.count > 1))
	  {
		  // scale by zeroth weight
		  dst *= vindex.weight[0];

		  // for each subsequent weight...
		  for (U32 j = 1; j < vindex.count; j++)
		  {
			  // transform by the Jth matrix
			  Vector vertJ;
			  tranys[vindex.index[j]].Transform( vertJ, src);

			  // scale by Jth weight
			  vertJ *= vindex.weight[j];

			  // accumulate value
			  dst += vertJ;
		  }
	  }
  }

  void SetVertex( Vertex & dst, const Vertex & src, const Matrix * tranys, const VertIndex & vindex, Bool doMultiWeight) const
  {
    SetVert( dst.vv, src.vv, tranys, vindex, doMultiWeight);

	  tranys[vindex.index[0]].Rotate( dst.nv, src.nv);
    dst.uv = src.uv;
  }

  void SetVertsView(  const Array<FamilyState> & stateArray, Matrix * tranys, Vector * verts, U32 vCount, Bool doMutiWeight) const;
  void SetVertsWorld( const Array<FamilyState> & stateArray, Matrix * tranys, Vector * verts, U32 vCount, Bool doMutiWeight) const;
  void SetVertsWorld( const Array<FamilyState> & stateArray, Vector * verts, U32 vCount, Bool doMutiWeight) const;
  void SetVertsIdentity( const Array<FamilyState> & stateArray, Vector * verts, U32 vCount, Bool doMultiWeight) const;

  void SetVertsWorldAnim(   const Array<FamilyState> & stateArray, Matrix * tranys, VertexTL * verts, U32 vCount, Bool doMutiWeight) const;

  inline void SetVertsWorldNoAnim( const Array<FamilyState> & stateArray, Vector * verts, U32 vCount) const
  {
    stateArray[0].WorldMatrix().Transform( verts, vertices.data, vCount);
  }

  void SetVertsWorldAnim(   const Array<FamilyState> & stateArray, Matrix * tranys, VertexI * verts, U32 vCount, Bool doMutiWeight) const;
  void SetVertsWorldNoAnim( const Array<FamilyState> & stateArray, VertexI * verts, U32 vCount) const;

public:

  void SetBucketDataZ( F32 z = 1);

  void ReleaseLocals();

  void SetNearFadeFactor( F32 nearF)
  {
    nearFadeFactor = nearF;
  }
  F32 GetNearFadeFactor()
  {
    return nearFadeFactor;
  }

  U32 GetMem() const;
  void SortFaces();

  U32 SetBlend( U32 mode, Bitmap * texture = NULL);
  
  MeshRoot()
  {
    ClearData();
    animCycles.SetNodeMember(&AnimList::cycleNode);
  }
  virtual ~MeshRoot();

  AnimList * FindAnimCycle(U32 cycleID) const
  {
    return animCycles.Find(cycleID); 
  }

  AnimList * FindAnimCycle(const char *cycle) const
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

  const MeshRoot *NextChunk();

  void Chunkify();

  virtual void SetWorldRecurse( const Matrix & world);
  virtual void SetWorldRecurseRender( const Matrix & world, FamilyState * stateArray);

  void Render( const Matrix &world, Color teamColor = 0xffffffff, U32 _controlFlags = controlDEF);
  void RenderLightSun( const Matrix &world, Color teamColor = 0xffffffff, U32 _controlFlags = controlDEF);

  void RenderColor( const Matrix &world, Color color, U32 _controlFlags = controlDEF);
  void RenderColorLight( const Matrix &world, Color color, U32 _controlFlags = controlDEF);
  void RenderColorLightSun( const Matrix &world, Color color, U32 _controlFlags = controlDEF);

  void Render( const Array<FamilyState> & stateArray, Color baseColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    (this->*renderProc)( groups, vertices.count, stateArray, baseColor, clipFlags, _controlFlags);
  }
  void RenderColor( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF)
  {
    (this->*renderProc)( groups, vertices.count, stateArray, color, clipFlags, _controlFlags);
  }

  // render with instance data
  void RenderLightAnimV(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderLightNoAnimV( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderColorAnimV(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderColorNoAnimV( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderEnvMapV(      Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  void RenderLightAnimV1(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderLightNoAnimV1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderColorAnimV1(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderColorNoAnimV1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderEnvMapV1(      Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  void RenderLightAnimVtl(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderLightNoAnimVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderColorAnimVtl(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderColorNoAnimVtl( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderEnvMapVtl(      Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  void RenderLightAnimVtl1(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderLightNoAnimVtl1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags);
  void RenderColorAnimVtl1(   Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderColorNoAnimVtl1( Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderEnvMapVtl1(      Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, Color baseColor, U32 clipFlags, U32 _controlFlags, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  // auto shadow texture generation
  void RenderShadowTexture( ShadowInfo & si, const Matrix ** lightA, U32 lCount, Array<FaceGroup> & _buckys, U32 vCount, const Array<FamilyState> & stateArray, U32 _controlFlags, Color color = 0x00000000, U32 blend = RS_BLEND_DEF);
  void RenderShadowTextureGeneric( Color color = 0xffffffff, U32 blend = RS_BLEND_DEF, U32 jitter = 8, Bool doBuild = FALSE);
  void RenderShadowVerts( const Vector & vect, Area<F32> & size, U32 type, Vector * vA, U32 vCount, const Array<FamilyState> & stateArray, U32 _controlFlags, Color color = 0x00000000, U32 blend = RS_BLEND_DEF);
  void RenderShadowTex( Bitmap & dstT, Area<F32> & size, const Vector * vA, U32 lCount, U32 vCount, const Color * cA, Array<FaceGroup> & _buckys, U32 blend = RS_BLEND_DEF, U32 jitter = 0, Bool fitOneWay = FALSE);

  // standard render functions
  void RenderLightAnim( Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderLightNoAnim( Color teamColor = 0xffffffff, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF);
  void RenderColorAnim( Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderColorNoAnim( Color color, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortNORMAL0);
  void RenderEnvMap( Color baseColor, U32 clipFlags = clipALL, U32 _controlFlags = controlDEF, Bitmap * tex = NULL, U32 blend = RS_BLEND_DEF, U16 sort = Vid::sortEFFECT0, Bool envmap = TRUE, Bool smooth = FALSE, F32 rotate = 0);

  // useful debug render
  void RenderWireframe( const Array<FamilyState> & stateArray, Color color, U32 blend = RS_BLEND_DEF, U32 clipFlags = clipALL);
  void RenderShadowPlane( U32 number, const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL);
  void RenderNormals( const Array<FamilyState> & stateArray, Color color, U32 clipFlags = clipALL);
  void RenderSelVerts( Bool show, const List<U16> * verts, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags = clipALL);
  void RenderSelFaces( Bool show, const List<U16> * faces, const Array<FamilyState> & stateArray, Color color0, Color color1, U32 clipFlags = clipALL);

  // collide with an multi-weighted animating mesh
  const FamilyNode * CollidePoly( const Array<FamilyState> & stateArray, const Vector &vStart, const Vector &vEnd, F32 &t) const;

  // setup
  void CompressStates();    // optimize the node hierarchy
  void CalcBoundingSphere();
  void CalcStatistics( U32 &_vertCount, U32 &_triCount) const;

  Bool MRMGen( List<U16> * verts = NULL);
  void MrmUpdate( Array<FaceGroup> & _groups, U32 vCountNew, U32 & vertCount, U32 & faceCount);
  inline void MrmUpdate( U32 vCountNew)
  {
    MrmUpdate( groups, vCountNew, vertCount, faceCount);
  }

  void MrmUpdate1( Array<FaceGroup> & _groups, U32 vCountNew, U32 & vertCount, U32 & faceCount);
  inline void MrmUpdate1( U32 vCountNew)
  {
    MrmUpdate1( groups, vCountNew, vertCount, faceCount);
  }

  // manipulate
  void SetColorGradient( Color color0, Color color1, F32 offset, F32 height, Bool alphaOnly = FALSE, Array<Color,4> * colorA = NULL);
  void TexAnim( Array<FaceGroup> * _buckys = NULL, Bool skipFrame0 = TRUE);
  void TexAnimStartRandom( Array<FaceGroup> * _buckys = NULL);

  // file
  Bool WriteXSI( const char *fileName);

  Bool Save( const char *fileName);
  Bool SaveScale( const char *fileName);    // append scale to name
  Bool Load( const char *fileName);
  Bool Save(GodFile *godFile);
  Bool Load(GodFile *godFile);

  // data fetch inlines
  //
  const Bounds & ObjectBounds() const
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

inline Bool FamilyState::IsTread() const
{
  return GetMeshFromRoot()->isTread;
}
inline Bool FamilyState::IsControl() const
{
  return GetMeshFromRoot()->isControl;
}

#endif    // __MESH_H
