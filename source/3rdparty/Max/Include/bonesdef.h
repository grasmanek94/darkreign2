#ifndef __BONESDEF__H
#define __BONESDEF__H

#include "mods.h"
#include "iparamm.h"
#include "shape.h"
#include "spline3d.h"
#include "splshape.h"
#include "linshape.h"

// This uses the linked-list class templates
#include "linklist.h"

extern void SpinnerOn(HWND hWnd,int SpinNum,int Winnum);
extern void SpinnerOff(HWND hWnd,int SpinNum,int Winnum);

#define PBLOCK_PARAM_REF		0
#define POINT1_REF				1
#define BONES_REF				2

#define BASE_TM_CHUNK           0x10
#define BONE_COUNT_CHUNK        0x20
#define BONE_DATATM_CHUNK       0x25
#define BONE_DATA_CHUNK         0x30
#define VERTEX_COUNT_CHUNK      0x40
#define VERTEX_DATA_CHUNK       0x50
#define BONE_NAME_CHUNK			0x160
#define BONE_BIND_CHUNK			0x200
#define BONE_SPLINE_CHUNK		0x210


#define DELTA_COUNT_CHUNK      0x400
#define DELTA_DATA_CHUNK       0x410

#define SELMOD 26

//--- Parameter map/block descriptors -------------------------------

#define PB_EFFECT               0
#define PB_LOCK_BONE			1
#define PB_ABSOLUTE_INFLUENCE   2
#define PB_FILTER_VERTICES      3
#define PB_FILTER_BONES         4
#define PB_FILTER_ENVELOPES     5
#define PB_DRAW_ENVELOPES       6
#define PB_DRAW_VERTICES        7
#define PB_REF_FRAME            8

#define PB_RADIUS               9

#define PB_PROJECT_THROUGH      10
#define PB_FALLOFF              11

#define PB_BONE_FALLOFF         12
#define PB_FEATHER              13
#define PB_DRAW_BONE_ENVELOPE   14

#define PB_ERADIUS				15
#define PB_ALWAYS_DEFORM		16
#define PB_PAINTSTR				17



#define BONE_LOCK_FLAG          1       
#define BONE_ABSOLUTE_FLAG      2
#define BONE_SPLINE_FLAG        4
#define BONE_SPLINECLOSED_FLAG  8
#define BONE_DRAW_ENVELOPE_FLAG 16
#define BONE_BONE_FLAG			32
#define BONE_DEAD_FLAG			64


#define BONE_FALLOFF_X3_FLAG    0
#define BONE_FALLOFF_X2_FLAG    1
#define BONE_FALLOFF_X_FLAG     2
#define BONE_FALLOFF_SINE_FLAG  3
#define BONE_FALLOFF_2X_FLAG    4
#define BONE_FALLOFF_3X_FLAG    5

//
#define CID_CREATECROSS CID_USER + 203
#define CID_CREATEPAINT CID_USER + 204

#define ID_LOCK				0x0100
#define ID_ABSOLUTE			0x0110
#define ID_DRAW_ENVELOPE	0x0120
#define ID_FALLOFF			0x0130
#define ID_COPY				0x0140
#define ID_PASTE			0x0150




// block IDs
//enum { bones_params,bones_paint, bones_filter, bones_advance };
//enum { bones_params};//,bones_paint,bones_filter,bones_advance};
// param param IDs
//enum { bones_effect, bones_radius, bones_lock, bones_absolute, bones_drawenvelopes, bones_falloff };
//enum { bones_paint_radius, bones_paint_feather};//, bones_paint_str };
//enum { bones_filter_vertices, bones_filter_bones, bones_filter_envelope, bones_filter_draw_all,bones_filter_color_verts };
//enum { bones_adv_always_deform, bones_adv_refframe, bones_adv_auto, bones_adv_inner, bones_adv_outer};



static Point2
ProjectPointF(GraphicsWindow *gw, Point3 fp) {
        IPoint3 out;
        gw->wTransPoint(&fp,&out);
        Point2 work;
        work.x = (float)out.x;
        work.y = (float)out.y;
        return work;
        }


class WeightRestore;

class BoneHitDataClass:public HitData
{
public:
        BoneHitDataClass(int v, int b, int e, int c, int ch)
                {
                VertexId = v;
                BoneId = b;
                EndPoint = e;
                CrossId = c;
                CrossHandleId = ch;
                }
        ~BoneHitDataClass() {}
int VertexId;
int BoneId;
int EndPoint;
int CrossId;
int CrossHandleId;
};


class VertexInfluenceListClass
{
public:
        int Bones;          
        float Influences;  
		float normalizedInfluences;
//extra data to hold spline stuff
        int SubCurveIds;
        int SubSegIds;
        float u;
        Point3 Tangents;
        Point3 OPoints;
};

class VertexListClass
        {
public:
        BOOL selected;
        BOOL modified;
        Point3 LocalPos;
//table of misc data    
        Tab<VertexInfluenceListClass> d;
        };

class CrossSectionClass
        {
public:
//        float Inner,Outer;
        float u;

		Control *InnerControl,*OuterControl;
		int RefInnerID,RefOuterID;
		BOOL innerSelected, outerSelected;
        };

class BoneDataClass
        {
public:
        INode *Node;
		Control *EndPoint1Control,*EndPoint2Control;

        Matrix3 tm;
        Matrix3 temptm;
        Tab<CrossSectionClass> CrossSectionList;
        BYTE flags;
        BYTE FalloffType;

		int BoneRefID;
		int RefEndPt1ID,RefEndPt2ID;
		BOOL end1Selected, end2Selected;
		TSTR name;
		Spline3D referenceSpline;
};

class CopyCrossClass
{
public:
	float inner, outer;
	float u;
};
class CopyClass
{
public:
	Point3 E1, E2;
	BOOL absolute,showEnvelope;
	int falloffType;
	Tab<CopyCrossClass> CList;
};

class VertexDistanceClass
{
public:
	float dist;
	float u;
    int SubCurveIds;
    int SubSegIds;
    Point3 Tangents;
    Point3 OPoints;

};

MakeLinkedList(BoneDataClass);

class CreateCrossSectionMode;
class CreatePaintMode;

class DumpHitDialog;

class BoneModData : public LocalModData {
	public:

//vertex info
        int VertexDataCount;
        float effect;

		Point3 hitPoint;
		BOOL isHit;

		Tab<VertexListClass*> VertexData;
		Matrix3 BaseTM,InverseBaseTM;

		ObjectState *hitState;
//		Mesh mesh;
//		PatchMesh patch;
		BOOL isMesh;
		BOOL isPatch;
		BOOL isSurf;
//selectio info
        BitArray selected;

		Tab<VertexDistanceClass> DistCache;
		int CurrentCachePiece;

		Tab<Matrix3> tmCacheToBoneSpace;
		Tab<Matrix3> tmCacheToObjectSpace;

		Tab<Point3> tempTableL1;
		Tab<Point3> tempTableL2;
		Tab<Point3> tempTableL1ObjectSpace;
		Tab<Point3> tempTableL2ObjectSpace;

		BOOL forceUpdate;
		Point3 localCenter;

		WeightRestore *wundo;
//6-18-99
		BOOL inputObjectIsNURBS;

//watje 9-7-99  198721 
        BOOL reevaluate;


		BoneModData()
			{
			VertexDataCount = 0;
			isMesh = FALSE;
			forceUpdate = FALSE;
			effect = -1.0f;
//watje 9-7-99  198721 
            reevaluate = FALSE;

			//mesh = NULL;
//			sel.ZeroCount();
			}
		~BoneModData()
			{
			for (int i=0;i<VertexDataCount;i++)
				{
				VertexData[i]->d.ZeroCount();
				if (VertexData[i] != NULL)
					delete (VertexData[i]);
				VertexData[i] = NULL;

				}


			VertexData.ZeroCount();
			VertexDataCount =0;


			}	
		LocalModData*	Clone()
			{
//			BoneModData* d = new BoneModData();
//			d->VertexDataCount = -1;


			return NULL;

			}	
};


class BonesDefMod : public Modifier {   
        public:


                IParamBlock *pblock_param;
                static HWND hParam;

                Control *p1Temp;


				int GetOpenID()
					{
					for (int i = 0; i < RefTable.Count();i++)
						{
						if (RefTable[i] == 0)
							{
							RefTable[i] = 1;
							return i+2;
							}
						}
					int u = 1;
					RefTable.Append(1,&u,1);
					return RefTable.Count()-1+2;
					}

				Tab<int> RefTable;



//                Matrix3 obtm;
//              Matrix3 iobtm;

//bones info
	            BoneDataClassList BoneData;

				BOOL inPaint;
				BOOL painting;
				BOOL reloadSplines;

				BOOL splineChanged;
				int whichSplineChanged;


//vertex info
//              int VertexDataCount;
//				Tab<VertexListClass*> VertexData;
//				BoneModData *bmd;


	            int OldVertexDataCount;
				Tab<VertexListClass*> OldVertexData;
				Matrix3 OldBaseTM,OldInverseBaseTM;

				BOOL forceRecomuteBaseNode;
				BOOL cacheValid;
				BOOL unlockVerts;
				BOOL unlockBone;
				void UnlockBone(BoneModData *bmd,TimeValue t, ObjectState *os);


				void RegisterClasses();
				static ICustToolbar *iParams;


                void ClearVertexSelections(BoneModData *bmd);
                void ClearBoneEndPointSelections();
                void ClearEnvelopeSelections();

				void DumpVertexList();
				void LockThisBone(int bid);
                void SelectFlexibleVerts(BoneModData *bmd);
                void BuildFalloff(BoneModData *bmd);

				int ConvertSelectedListToBoneID(int fsel);
				int ConvertSelectedBoneToListID(int fsel);

                
//selection space is   NOT RIGHT REDOC
//              vertices
//                      { bones
//                      Bone End
//                      Bone Start
//                      8 * Number of Ccross Sections
//                      }
//                      }

//edit modes to determine what is currently being edited since we only use one sub object
                int ModeEdit;
                int ModeBoneIndex;
                int ModeBoneEndPoint;
                int ModeBoneEnvelopeIndex;
                int ModeBoneEnvelopeSubType;
				int inPaintMode;

                Point3 Worldl1, Worldl2;

				void EnableButtons();
				void DisableButtons();

				CopyClass CopyBuffer;
				void CopyBone();
				void PasteBone();

                void AddCrossSection(int boneid, float u, float inner, float outer);
                void AddCrossSection(float u);

                float GetU(ViewExp *vpt,Point3 a, Point3 b, IPoint2 p);
				void GetCrossSectionRanges(float &inner, float &outer, int BoneID, int CrossID);
				void GetEndPoints(BoneModData *bmd, TimeValue t,Point3 &l1, Point3 &l2, int BoneID);
				void GetEndPointsLocal(BoneModData *bmd, TimeValue t,Point3 &l1, Point3 &l2, int BoneID);

				float ModifyU(TimeValue t, float LineU,  int BoneID, int sid);
				float ComputeInfluence(TimeValue t, float Influence, float u, int BoneID, int StartCross, int EndCross, int sid);

				void BuildCache(BoneModData *bmd, int BoneIndex, TimeValue t, ObjectState *os);
				void BuildCacheThread(BoneModData *bmd, int start, int end, int BoneIndex, TimeValue t, ObjectState *os, ShapeObject *pathOb, Matrix3 ntm );

				void UpdateTMCacheTable(BoneModData *bmd, TimeValue t, Interval& valid);


//distance stuff
                void RecurseDepth(float u1, float u2, float &fu,  ShapeObject *s,int Curve,int Piece, int &depth, Point3 fp);
                void PointToPiece(float &tempu, ShapeObject *s,int Curve,int Piece, int depth, Point3 fp);
                float SplineToPoint(Point3 p1, ShapeObject *s, float &u, Point3 &p, Point3 &t, int &cid, int &sid, Matrix3 tm);

                void RecurseDepth(float u1, float u2, float &fu,  Spline3D *s,int Curve,int Piece, int &depth, Point3 fp);
                void PointToPiece(float &tempu, Spline3D *s,int Curve,int Piece, int depth, Point3 fp);
                float SplineToPoint(Point3 p1, Spline3D *s, float &u, Point3 &p, Point3 &t, int &cid, int &sid, Matrix3 tm);

                
				float LineToPoint(Point3 p1, Point3 l1, Point3 l2, float &u);
//this one does no end caping test
                float LineToPoint(Point3 p1, Ray r, float &u);

                void ClosestPoint(float inc, float start, float end, 
                                                           Tab<float> *cl, Tab<int> *ci, ShapeObject *s, Point3 p1);
                void ClosestPoint(float inc, float start, float end, 
                                                           Tab<float> *cl, int ci, ShapeObject *s, Point3 p1);


//				void BuildEnvelopes(INode *bnode, INode *mnode, Point3 l1, Point3 l2, float &el1, float &el2);
				void BuildEnvelopes(INode *bnode, Object *obj, Point3 l1, Point3 l2, float &el1, float &el2);
                void BuildMajorAxis(INode *node, Point3 &t, Point3 &e);

                void RemoveBone();
                void RemoveBone(int bid);
                void RemoveCrossSection();
				void RemoveCrossSection(int bid, int eid);
				void RemoveCrossSectionNoNotify(int bid, int eid);

                void SetSelectedVertices(BoneModData *bmd, int BoneID, float amount);
                void IncrementVertices(BoneModData *bmd, int BoneID, Tab<float> amount, int flip);
                void SetVertex(BoneModData *bmd, int vertID, int BoneID, float amount);
				void SetVertices(BoneModData *bmd,int vertID, Tab<int> BoneIDList, Tab<float> amountList);

                void ComputeFalloff(float &u, int ftype);


				float RetrieveNormalizedWeight(BoneModData *bmd, int vid, int bid);
                void RecomputeAllBones(BoneModData *bmd, TimeValue t, ObjectState *os);
                void RecomputeBone(BoneModData *bmd, int BoneIndex,TimeValue t, ObjectState *os);

                Point3 VertexAnimation(TimeValue t, BoneModData * bmd, int vertex, int bone, Point3 p);

//dialog variables
                int LockBone;
                int AbsoluteInfluence;
                int FilterVertices, FilterBones,FilterEnvelopes,DrawEnvelopes;
                int DrawVertices;
                int RefFrame;
                int BoneFalloff;
				int AlwaysDeform;
//Paint controlls
                float Radius;
                float Feather;
				float paintStr;
                int ProjectThrough, Falloff;


                BOOL reset;
                BOOL BoneMoved;
                int absolute;

//                Matrix3 BaseTM,InverseBaseTM;



                static IObjParam *ip;
                static IParamMap *pmapParam;
                static BonesDefMod *editMod;
                static MoveModBoxCMode *moveMode;

                void StartCrossSectionMode(int type);
                static CreateCrossSectionMode *CrossSectionMode;

                void StartPaintMode();
                static CreatePaintMode *PaintMode;


                static ICustButton*   iCrossSectionButton;
                static ICustButton*   iPaintButton;
                static ICustButton*   iLock;
				static ICustButton*	  iEnvelope;
				static ICustButton*   iAbsolute;
				static ICustButton*   iFalloff;
				static ICustButton*   iCopy;
				static ICustButton*   iPaste;

                static int LastSelected;


                
                BonesDefMod();
                ~BonesDefMod();


                // From Animatable
                void DeleteThis() { 
					delete this; 
					}
                void GetClassName(TSTR& s) {s = GetString(IDS_RB_BONESDEFMOD);}  
                virtual Class_ID ClassID() { return Class_ID(9815843,87654);}           
                void BeginEditParams(IObjParam  *ip, ULONG flags,Animatable *prev);
                void EndEditParams(IObjParam *ip,ULONG flags,Animatable *next);         
                RefTargetHandle Clone(RemapDir& remap = NoRemap());
                TCHAR *GetObjectName() {return GetString(IDS_RB_BONESDEFMOD);}

                int SubNumToRefNum(int subNum);


                // From modifier
                ChannelMask ChannelsUsed()  {return PART_GEOM|PART_TOPO|PART_SELECT|PART_SUBSEL_TYPE;}
                ChannelMask ChannelsChanged() {return PART_GEOM;}
                Class_ID InputType() {return defObjectClassID;}
                void ModifyObject(TimeValue t, ModContext &mc, ObjectState *os, INode *node);
                Interval LocalValidity(TimeValue t);



                void LoadAxis(Point3 st, float dist, Matrix3 tm, Point3 *plist);
                void DrawCrossSection(Point3 a, Point3 Align, float length, Matrix3 tm, GraphicsWindow *gw);
                void DrawCrossSectionNoMarkers(Point3 a, Point3 Align, float length,GraphicsWindow *gw);
                void DrawEndCrossSection(Point3 a, Point3 align, float length,  Matrix3 tm, GraphicsWindow *gw);
                void GetCrossSection(Point3 a, Point3 Align, float length, Matrix3 tm,  Point3 *edge_p);
                void GetCrossSectionLocal(Point3 a, Point3 Align, float length, Point3 *edge_p);

                void DrawEnvelope(Tab<Point3> a, Tab<float> length, int count, Matrix3 tm, GraphicsWindow *gw);
                // From BaseObject
                int HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc);
                int Display(TimeValue t, INode* inode, ViewExp *vpt, int flagst, ModContext *mc);

                void GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc);               

                void Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE );
                CreateMouseCallBack* GetCreateMouseCallBack() {return NULL;} 
                void GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
                void GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc);
                void ActivateSubobjSel(int level, XFormModes& modes);
                void SelectSubComponent(HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE);
                void ClearSelection(int selLevel);
                void SelectAll(int selLevel);

                void InvertSelection(int selLevel);

				void UpdateEffectSpinner(BoneModData*bmd);

				BOOL updateP;
				void UpdateP(BoneModData* bmd);
				Tab<Point3> endPointDelta;
				void UpdateEndPointDelta();
					
				void SyncSelections();


				void EnableRadius(BOOL enable);
				void EnableEffect(BOOL enable);

				void UpdatePropInterface();

                int NumRefs(); 
/*				{
					int ct = 0;
					for (int i = 0; i<BoneData.Count();i++)
						{
						ct +=2 ;
						ct += BoneData[i].CrossSectionList.Count();
						}
					return 2+BoneData.Count()+ct;
//					ViewControls.Count();
					}
*/
                RefTargetHandle GetReference(int i);
                void SetReference(int i, RefTargetHandle rtarg);

                int NumSubs() {return 1;}
                Animatable* SubAnim(int i) {return GetReference(i);}
                TSTR SubAnimName(int i);

                RefResult NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, 
                   PartID& partID, RefMessage message);

                Matrix3 CompMatrix(TimeValue t,INode *inode,ModContext *mc);

                IOResult Load(ILoad *iload);
                IOResult Save(ISave *isave);
				
				IOResult SaveLocalData(ISave *isave, LocalModData *pld);
				IOResult LoadLocalData(ILoad *iload, LocalModData **pld);

				Matrix3 initialXRefTM;
				Matrix3 xRefTM;
				INode *bindNode;

				HoldWeights();
				AcceptWeights(BOOL accept);
				void NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc);


//watje 9-7-99  198721 
				void Reevaluate(BOOL eval);
//watje 10-13-99 212156
				BOOL DependOnTopology(ModContext &mc);
};

class LocalBoneData : public LocalModData {
public:
//vertex data count
//need individual vertex data
//sel sets
//need matrices
/*
	Mesh *mesh;
	PatchMesh *patch;
	BitArray faceSel;

	MeshTopoData(Mesh &mesh);
	MeshTopoData(PatchMesh &patch);
	MeshTopoData() { mesh=NULL; patch = NULL; }
	~MeshTopoData() { FreeCache(); }
	LocalModData *Clone();

	Mesh *GetMesh() {return mesh;}
	PatchMesh *GetPatch() {return patch;}
	void SetCache(Mesh &mesh);
	void SetCache(PatchMesh &patch);
	void FreeCache();

	BitArray &GetFaceSel() { return faceSel; }
	void SetFaceSel(BitArray &set, UnwrapMod *imod, TimeValue t);
*/

};



class CreateCrossSectionMouseProc : public MouseCallBack {
        private:
                BonesDefMod *mod;
                IObjParam *iObjParams;
                IPoint2 om;
                int type; // See above
				Point3 a,b;
				float SplineU;
        
        protected:
                HCURSOR GetTransformCursor();
                BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags );
                BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }           

        public:
                CreateCrossSectionMouseProc(BonesDefMod* bmod, IObjParam *i) { mod=bmod; iObjParams=i; }
                int proc( 
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m );
                void SetType(int type) { this->type = type; }
				void GetHit(float &ou) {ou = SplineU;}
        };





class CreateCrossSectionMode : public CommandMode {
        private:
                ChangeFGObject fgProc;
                CreateCrossSectionMouseProc eproc;
                BonesDefMod* mod;
                int type; // See above

        public:
                CreateCrossSectionMode(BonesDefMod* bmod, IObjParam *i) :
                        fgProc(bmod), eproc(bmod,i) {mod=bmod;}

                int Class() { return MODIFY_COMMAND; }
                int ID() { return CID_CREATECROSS; }
                MouseCallBack *MouseProc(int *numPoints) { *numPoints=1; return &eproc; }
                ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
                BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
                void EnterMode();
                void ExitMode();
                void SetType(int type) { this->type = type; eproc.SetType(type); }
        };



class CreatePaintMouseProc : public MouseCallBack {
        private:
                BonesDefMod *mod;
                IObjParam *iObjParams;
                IPoint2 om,lastPoint;
                Tab<float> FalloffList;

				Tab<IPoint2> mouseHitList;
				Tab<Object*> objList;
				BOOL BuildFalloffList( ViewExp *vpt, Tab<IPoint2> plist, BoneModData *bmd, Object *obj);

        
        protected:
                HCURSOR GetTransformCursor();
                BOOL HitTest( ViewExp *vpt, IPoint2 *p, int type, int flags, BoneModData *bmd, Object *obj );
                BOOL AnyHits( ViewExp *vpt ) { return vpt->NumSubObjHits(); }           

        public:
				BOOL first;
				float screenRadius;
				float prevScreenRadius;
                CreatePaintMouseProc(BonesDefMod* bmod, IObjParam *i) { mod=bmod; iObjParams=i; }
                int proc( 
                        HWND hwnd, 
                        int msg, 
                        int point, 
                        int flags, 
                        IPoint2 m );
        };



class CreatePaintMode : public CommandMode {
        private:
                ChangeFGObject fgProc;
                CreatePaintMouseProc eproc;
                BonesDefMod* mod;

        public:
                CreatePaintMode(BonesDefMod* bmod, IObjParam *i) :
                        fgProc(bmod), eproc(bmod,i) {mod=bmod;}

                int Class() { return MODIFY_COMMAND; }
                int ID() { return CID_CREATEPAINT; }
                MouseCallBack *MouseProc(int *numPoints) { *numPoints=2; return &eproc; }
                ChangeForegroundCallback *ChangeFGProc() { return &fgProc; }
                BOOL ChangeFG( CommandMode *oldMode ) { return oldMode->ChangeFGProc() != &fgProc; }
                void EnterMode();
                void ExitMode();
        };



class DumpHitDialog : public HitByNameDlgCallback {
public:
	BonesDefMod *eo;
	DumpHitDialog(BonesDefMod *e) {eo=e;};
	TCHAR *dialogTitle() {return _T(GetString(IDS_PW_SELECTBONES));};
	TCHAR *buttonText() {return _T(GetString(IDS_PW_SELECT));};
	BOOL singleSelect() {return FALSE;};
	BOOL useProc() {return TRUE;};
	int filter(INode *node);
	void proc(INodeTab &nodeTab);
	};


//---------------------------------------------------------------------
// Rightclick menu UI stuff


class BonesRightMenu : public RightClickMenu {
	private:
		BonesDefMod *ep;
	public:
		void Init(RightClickMenuManager* manager, HWND hWnd, IPoint2 m);
		void Selected(UINT id);
		void SetMod(BonesDefMod *ep) { this->ep = ep; }
	};

//BonesRightMenu rMenu;

class MapDlgProc : public ParamMapUserDlgProc {
	public:
		BonesDefMod *mod;		
		MapDlgProc(BonesDefMod *m) {mod = m;}		
		BOOL DlgProc(TimeValue t,IParamMap *map,HWND hWnd,UINT msg,WPARAM wParam,LPARAM lParam);		
		void DeleteThis() {
				delete this;
				}
//		void DoBitmapFit(HWND hWnd);
	};


class HitFaceClass
{
public:
Box2 b;
float zDepth;
BOOL inVolume;
BOOL isSelected;
};

class ZHashClass
{
public:
HitFaceClass *d;
ZHashClass *next;
};

class ZHashTableClass
{
ZHashClass *Table;
int TableSize;
float Zmin,Zmax;
/*
ClearTable()
	{
	return;
	};
AddEntry(HitFaceClass *p)
	{
	return;
	};
*/
};


class PasteRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		CopyClass ubuffer,rbuffer;
		CopyClass tbuffer;
		int whichBone;
		PasteRestore(BonesDefMod *c) 
			{
			mod = c;
			ubuffer.CList.ZeroCount();
			for (int i = 0; i < mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count();i++)
				{
				CopyCrossClass c;
				Interval v;
				c.u = mod->BoneData[mod->ModeBoneIndex].CrossSectionList[i].u;
				mod->BoneData[mod->ModeBoneIndex].CrossSectionList[i].InnerControl->GetValue(0,&c.inner,v);
				mod->BoneData[mod->ModeBoneIndex].CrossSectionList[i].OuterControl->GetValue(0,&c.outer,v);
				ubuffer.CList.Append(1,&c,1);
				}

//			ubuffer = mod->CopyBuffer;
			ubuffer.absolute = FALSE;
			if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
				ubuffer.absolute = TRUE;
			ubuffer.showEnvelope = FALSE;
			if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
				ubuffer.showEnvelope = TRUE;
			ubuffer.falloffType = mod->BoneData[mod->ModeBoneIndex].FalloffType;

			whichBone = mod->ModeBoneIndex;
			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rbuffer.CList.ZeroCount();
				for (int i = 0; i < mod->BoneData[mod->ModeBoneIndex].CrossSectionList.Count();i++)
					{
					CopyCrossClass c;
					Interval v;
					c.u = mod->BoneData[mod->ModeBoneIndex].CrossSectionList[i].u;
					mod->BoneData[mod->ModeBoneIndex].CrossSectionList[i].InnerControl->GetValue(0,&c.inner,v);
					mod->BoneData[mod->ModeBoneIndex].CrossSectionList[i].OuterControl->GetValue(0,&c.outer,v);
					rbuffer.CList.Append(1,&c,1);
					rbuffer.absolute = FALSE;
					if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_ABSOLUTE_FLAG)
						rbuffer.absolute = TRUE;
					rbuffer.showEnvelope = FALSE;
					if (mod->BoneData[mod->ModeBoneIndex].flags & BONE_DRAW_ENVELOPE_FLAG)
						rbuffer.showEnvelope = TRUE;
					rbuffer.falloffType = mod->BoneData[mod->ModeBoneIndex].FalloffType;

					}

//				rbuffer = mod->CopyBuffer;
				}
			tbuffer = mod->CopyBuffer;
			mod->CopyBuffer = ubuffer;
			mod->ModeBoneIndex = whichBone;

			mod->PasteBone();
//watje 9-7-99  198721 
			mod->Reevaluate(TRUE);

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			mod->CopyBuffer = tbuffer;

			}
		void Redo()
			{
			tbuffer = mod->CopyBuffer;
			mod->CopyBuffer = rbuffer;
			mod->ModeBoneIndex = whichBone;
			mod->PasteBone();
//watje 9-7-99  198721 
			mod->Reevaluate(TRUE);

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			mod->CopyBuffer = tbuffer;

			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_PASTE))); }
	};

class SelectionRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		int rModeBoneIndex;
		int rModeBoneEnvelopeIndex;
		int rModeBoneEndPoint;
		int rModeBoneEnvelopeSubType;
		BitArray rVertSel;

		int uModeBoneIndex;
		int uModeBoneEnvelopeIndex;
		int uModeBoneEndPoint;
		int uModeBoneEnvelopeSubType;
		BitArray uVertSel;

		SelectionRestore(BonesDefMod *c, BoneModData *md) 
			{
			mod = c;
			bmd = md;
			uModeBoneIndex = mod->ModeBoneIndex;
			uModeBoneEnvelopeIndex  = mod->ModeBoneEnvelopeIndex;
			uModeBoneEndPoint  = mod->ModeBoneEndPoint;
			uModeBoneEnvelopeIndex = mod->ModeBoneEnvelopeIndex;
			uModeBoneEnvelopeSubType = mod->ModeBoneEnvelopeSubType;
			uVertSel.SetSize(bmd->selected.GetSize());
			uVertSel = bmd->selected;
//			uVertSel;
			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				rModeBoneIndex = mod->ModeBoneIndex;
				rModeBoneEnvelopeIndex  = mod->ModeBoneEnvelopeIndex;
				rModeBoneEndPoint  = mod->ModeBoneEndPoint;
				rModeBoneEnvelopeIndex = mod->ModeBoneEnvelopeIndex;
				rModeBoneEnvelopeSubType = mod->ModeBoneEnvelopeSubType;
				rVertSel.SetSize(bmd->selected.GetSize());
				rVertSel = bmd->selected;
				}
			mod->ModeBoneIndex = uModeBoneIndex;
			mod->ModeBoneEnvelopeIndex = uModeBoneEnvelopeIndex ;
			mod->ModeBoneEndPoint = uModeBoneEndPoint  ;
			mod->ModeBoneEnvelopeIndex = uModeBoneEnvelopeIndex ;
			mod->ModeBoneEnvelopeSubType = uModeBoneEnvelopeSubType ;
			bmd->selected = uVertSel;
			if (bmd->selected.NumberSet() >0)
				mod->EnableEffect(TRUE);
			else mod->EnableEffect(FALSE);

			mod->SyncSelections();
			mod->UpdatePropInterface();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			mod->ModeBoneIndex = rModeBoneIndex;
			mod->ModeBoneEnvelopeIndex = rModeBoneEnvelopeIndex ;
			mod->ModeBoneEndPoint = rModeBoneEndPoint  ;
			mod->ModeBoneEnvelopeIndex = rModeBoneEnvelopeIndex ;
			mod->ModeBoneEnvelopeSubType = rModeBoneEnvelopeSubType ;
			bmd->selected = rVertSel;
			if (bmd->selected.NumberSet() >0)
				mod->EnableEffect(TRUE);
			else mod->EnableEffect(FALSE);

			mod->SyncSelections();
			mod->UpdatePropInterface();


			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }
	};

class DeleteBoneRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneDataClass undo,redo;
		int undoID, redoID;
		Tab<int> undoRefTable,redoRefTable;


		DeleteBoneRestore(BonesDefMod *c, int whichBone) 
			{
			mod = c;
 			undo = mod->BoneData[whichBone];
			undoID = whichBone;
			undoRefTable = mod->RefTable;
			}   		
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				redo = mod->BoneData[undoID];
				redoRefTable = mod->RefTable;

				}

			mod->BoneData[undoID].CrossSectionList = undo.CrossSectionList;
	        mod->BoneData[undoID].flags = undo.flags;
			mod->RefTable = undoRefTable;

//watje 9-7-99  198721 
			mod->Reevaluate(TRUE);
			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			mod->BoneData[undoID].CrossSectionList = redo.CrossSectionList;
	        mod->BoneData[undoID].flags = redo.flags;
			mod->RefTable = redoRefTable;



//watje 9-7-99  198721 
			mod->Reevaluate(TRUE);
			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_DELETEBONE))); }
	};


class WeightRestore : public RestoreObj {
	public:
		BonesDefMod *mod;
		BoneModData *bmd;
		Tab<VertexListClass*> undoVertexData;
		Tab<VertexListClass*> redoVertexData;

		WeightRestore(BonesDefMod *bmod, BoneModData *md) 
			{
			int c = md->VertexData.Count();
			undoVertexData.SetCount(md->VertexData.Count());
			for (int i=0; i<c; i++) {
				VertexListClass *vc;
				vc = new VertexListClass;
				undoVertexData[i] = vc;
				undoVertexData[i]->modified = md->VertexData[i]->modified;
				undoVertexData[i]->selected = md->VertexData[i]->selected;
				undoVertexData[i]->d = md->VertexData[i]->d;
				}

			mod = bmod;
			bmd = md;
			}   	
		~WeightRestore()
			{
			int c = undoVertexData.Count();
			for (int i=0; i<c; i++) 
				delete undoVertexData[i];
			c = redoVertexData.Count();
			for (i=0; i<c; i++) 
				delete redoVertexData[i];

			}
		void Restore(int isUndo) 
			{
			if (isUndo) 
				{
				int c = bmd->VertexData.Count();
				redoVertexData.SetCount(bmd->VertexData.Count());
				for (int i=0; i<c; i++) {
					VertexListClass *vc;
					vc = new VertexListClass;
					redoVertexData[i] = vc;
					redoVertexData[i]->modified = bmd->VertexData[i]->modified;
					redoVertexData[i]->selected = bmd->VertexData[i]->selected;
					redoVertexData[i]->d = bmd->VertexData[i]->d;
					}

				}

			int c = bmd->VertexData.Count();
			for (int i=0; i<c; i++) {
				bmd->VertexData[i]->modified = undoVertexData[i]->modified;
				bmd->VertexData[i]->selected = undoVertexData[i]->selected;
				bmd->VertexData[i]->d = undoVertexData[i]->d;
				}

			float ef;
			mod->pblock_param->GetValue(PB_EFFECT,0,ef,FOREVER);
			bmd->effect = ef;

			mod->SyncSelections();

			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}
		void Redo()
			{
			int c = bmd->VertexData.Count();
			for (int i=0; i<c; i++) {
				bmd->VertexData[i]->modified = redoVertexData[i]->modified;
				bmd->VertexData[i]->selected = redoVertexData[i]->selected;
				bmd->VertexData[i]->d = redoVertexData[i]->d;
				}

			float ef;
			mod->pblock_param->GetValue(PB_EFFECT,0,ef,FOREVER);
			bmd->effect = ef;
			mod->SyncSelections();


			mod->NotifyDependents(FOREVER, PART_ALL, REFMSG_CHANGE);
			}		
		void EndHold() 
			{ 
			mod->ClearAFlag(A_HELD);
			}
		TSTR Description() { return TSTR(_T(GetString(IDS_PW_SELECT))); }
	};


#endif
