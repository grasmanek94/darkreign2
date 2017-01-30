/*	
 *		MSPlugin.h - MAXScript scriptable plugins for MAX
 *
 *	 A typical scripted plugin is defined by 3 MAXScript classes:
 *
 *		MSPlugin			base mixin for all scripted MAX plug-in classes
 *							this class inherits from Value so it can live
 *							in the MAXScript garbage-collected heap.
 * 
 *		MSPluginGeomObject	scripted GeomObjects
 *		MSPluginModifier	scripted Modifiers
 *		MSPluginControl		scripted Controls
 *		MSPluginLight		scripted Lights
 *		MSPluginMtl			scripted Materials, etc...
 *
 *		MSGeomObjectXtnd
 *		MSModifierXtnd
 *		MSControlXtnd		... These are subclasses of the above MSPlugin classes that
 *							extend an existing class (specified in the extends: param).  
 *							Instances contain a delegate, a ref to an owned instance of 
 *							the class under extension and
 *							bounce all calls to it (delegation) then specialize UI calls 
 *							to allow UI replacement or extra rollouts for the delegate.  
 *
 *      MSPluginClass		MAXClass specialization whose instances represent scripted
 *							plugin classes, contains all the definition-level stuff
 *							and a pointer to the MSPluginDesc for the class.
 *							It is applyable for scripted instance creation.  It is also 
 *							kept in a special 
 *							hashtable to enable repeated re-definition in the scripter,
 *							the same value is updated on each re-definition.
 *							This class inherits also from MAXClass and so lives in
 *							the MAXScript heap. 
 *
 *		MSPluginDesc		ClassDesc2 specialization for MSPlugin classes.
 *							Unlike most ClassDescs, many instances of this are
 *						    created, one per scripted plugin.
 *							Instances contain a pointer to the corresponding
 *							MSPluginClass instance from which info for the implementing
 *							the standard ClassDesc2 interface is derived.
 *
 *			Copyright © Autodesk, Inc, 1998.  John Wainwright.
 *
 */

#ifndef _H_MSPLUGIN
#define _H_MSPLUGIN

class MSPlugin;
class MSPluginDesc;
class MSPluginClass;
class MSAutoMParamDlg;
class MSAutoEParamDlg;

#include <iparamm2.h>
#include <simpmod.h>

// plugin context predefined local indexes - MUST match order in Parser::plugin_def()
enum { 
	pl_this, pl_version, pl_delegate						// common
};

enum { 
	pl_extent = pl_delegate + 1, pl_min, pl_max, pl_center   // for SimpleMods
};

enum { 
	pl_mesh = pl_delegate + 1								// for SimpleObjects 
};

typedef RefTargetHandle (*creator_fn)(MSPluginClass* pic, BOOL loading);
extern void show_source_pos();

// parameter reference (used by subtex and submtl mechanism in scripted texmaps & mtls)
class ParamRef
{
public:
	BlockID	block_id;
	ParamID param_id;
	int		tabIndex;
	ParamRef(BlockID bid, ParamID pid, int index) { block_id = bid; param_id = pid; tabIndex = index; }
};

// ----------------------- MSPluginClass -----------------------
//	MAXClass specialization for scripted plug-in classes

visible_class (MSPluginClass)

class MSPluginClass : public MAXClass
{
public:
	Value*			class_name;		// UI-visible class name
	HINSTANCE		hInstance;		// owning module
	ClassDesc*		extend_cd;		// if extending, ClassDesc of class to extend
	creator_fn		obj_creator;	// obj maker for the appropriate MSPlugin subclass	
	HashTable*		local_scope;	// local name space	
	Value**			local_inits;	// local var init vals	
	int				local_count;	//   "    "  count	
	HashTable*		handlers;		// handler tables	
	Array*			rollouts;		// UI rollouts
	MouseTool*		create_tool;	// scripted creation tool if non-NULL
	Array*			pblock_defs;	// parameter block definition data from compiler (used to build the PB2 descs)
	Array*			loading_pblock_defs; // parameter block definition data for currently loading scene file (to permit old version schema migration)
	Tab<ParamBlockDesc2*> pbds;		// parameter block descriptors
	ReferenceTarget* alternate;		// any alternate UI object system-style during create mode
	Tab<ParamRef>	sub_texmaps;	// param references to any texmaps in pblocks in instances of this class in subobjno order
	Tab<ParamRef>	sub_mtls;		// param references to any mtls in pblocks in instances of this class in subobjno order
	int				version;		// plugin version (from version: param on def header)
	short			mpc_flags;		// flags	

	static HashTable* msp_classes;	// table of existing scripted plugin classes to enable redefinition
	static MSPlugin* creating;		// object currently being created if non-NULL

					MSPluginClass(Value* name, MAXSuperClass* supcls, creator_fn cfn);
				   ~MSPluginClass();

	// definition and redefinition
	static MSPluginClass* intern(Value* name, MAXSuperClass* supcls, creator_fn cfn);
	void			init(int local_count, Value** inits, HashTable* local_scope, HashTable* handlers, Array* pblock_defs, Array* rollouts, MouseTool* create_tool);

	// MAXScript required
	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MSPluginClass)) ? 1 : Value::is_kind_of(c); }
#	define			is_msplugin_class(v) ((v)->tag == class_tag(MSPluginClass))
	void			gc_trace();
	void			collect() { delete this; }

	// from Value 
	Value*			apply(Value** arglist, int count);		// object creation by applying class object

	// local
	void			SetClassID(Value* cidv);
	void			SetExtension(Value* cls);
	void			SetVersion(Value* ver) { version = ver->to_int(); }
	void			StopEditing(int stop_flags = 0);
	void			RestartEditing();
	ParamType2		lookup_type(Value* name);
	Value*			call_handler(Value* handler, Value** arg_list, int count, TimeValue t, BOOL disable_on_error=FALSE);

	// alternate UI
	void			install_alternate(ReferenceTarget* ref);
	// schema migration
	void			build_paramblk_descs();
	void			redefine(MSPlugin* val, HashTable* old_locals, Array* old_pblock_defs);
	Array*			find_pblock_def(Value* name, Array* pblock_defs);
	// scene I/O
	static void		save_class_defs(ISave* isave);
	static IOResult load_class_def(ILoad* iload);
	static void		post_load();

	// ClassDesc delegates
	RefTargetHandle	Create(BOOL loading);
	const TCHAR*	ClassName() { return class_name->to_string(); }
	SClass_ID		SuperClassID() { return sclass_id; }
	Class_ID		ClassID() { return class_id; }
	const TCHAR* 	Category() { return category->to_string(); }
	const TCHAR*	InternalName() { return name->to_string(); }
	HINSTANCE		HInstance() { return hInstance; }
};

// plugin class flags
#define MPC_TEMPORARY			0x0001	// no classID: temporary, cannot be saved in a scene file, shouldn't be wired in to the scene anywhere
#define MPC_REDEFINITION		0x0002	// class redefinition under way
#define MPC_MS_CREATING			0x0004	// currently creating new object
#define MPC_MS_WAS_CREATING		0x0008	// MAXScript was creating during a redefinition
#define MPC_EDITING_IN_CP		0x0010	// currently editing obj in command panel
#define MPC_EDITING_IN_MTLEDT	0x0020	// currently editing obj in material editor
#define MPC_EDITING_IN_EFX		0x0040	// currently editing obj in render effects/environment editor
#define MPC_REPLACE_UI			0x0080	// for extending classes, scripted UI completely replaces delegates UI
#define MPC_INVISIBLE			0x0100	// doesn't show up in create panel buttons, useful for controlling dummies, etc.
#define MPC_SILENTERRORS		0x0200  // don't report errors
#define MPC_MAX_CREATING		0x0400  // in default MAX creation mode
#define MPC_ABORT_CREATE		0x0800  // plugin should abort MAX create mode on next GetMouseCreateCallback
#define MPC_LEVEL_6				0x1000  // level 6 plugin; supports full, stand-alone creatability

// for accessing keyword params in pblock_defs
#define key_parm(_key)	_get_key_param(keys, n_##_key)
extern Value* _get_key_param(Array* keys, Value* key_name);

// ----------------------- MSPluginDesc -----------------------
//	ClassDescs for scripted classes, created dynamically for each scripted class

class MSPluginDesc : public ClassDesc2
{
public:
	MSPluginClass*	pc;			// my MAXScript-side plugin class
	MSPlugin*		plugin;		// object under creation, MSPlugin interface
	RefTargetHandle base_obj;	//   "      "       "     base object interface

					MSPluginDesc(MSPluginClass* ipc) { pc = ipc; }

	// from ClassDesc
	int 			IsPublic();
	void*			Create(BOOL loading = FALSE) { return pc->Create(loading); }
	const TCHAR*	ClassName() { return pc->ClassName(); }
	SClass_ID		SuperClassID() { return pc->SuperClassID(); }
	Class_ID		ClassID() { return pc->ClassID(); }
	const TCHAR* 	Category() { return pc->Category(); }
	int 			BeginCreate(Interface *i);
	int 			EndCreate(Interface *i);
	// from ClassDesc2
	const TCHAR*	InternalName() { return pc->InternalName(); }
	HINSTANCE		HInstance() { return pc->HInstance(); }

	// local 
	void			StartTool(IObjCreate *iob);   // start up scripted create tool
	void			StopTool(IObjCreate *iob);   // stop scripted create tool
};

// ----------------------- MSPluginPBAccessor -----------------------
//	paramblock accessor topass gets & sets to scripted handlers

class MSPluginPBAccessor : public PBAccessor
{
	BlockID			bid;
	MSPluginClass*	pc;
public:
	MSPluginPBAccessor(MSPluginClass* ipc, BlockID id) { pc = ipc; bid = id; }
	void Set(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t); // set from v
	void Get(PB2Value& v, ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t, Interval& valid); // set from v
	BOOL KeyFrameAtTime(ReferenceMaker* owner, ParamID id, int tabIndex, TimeValue t);
	Value* to_value(PB2Value& v, ParamDef& pd);
	void from_value(PB2Value& v, ParamDef& pd, Value* val);
};

// ----------------------- MSPlugin -----------------------
//	base mixin class for MAX-side scripted classes

#define MSP_LOADING		0x0001			// instance is currently being loaded from a scene
#define MSP_DISABLED	0x0002			// general disablement flag, used to diable plugin in case of handler errors, reset by redefinition
#define MSP_DELETED		0x0004			// I'm deleted in the MAX world

class MSPlugin : public Value
{
public:
	MSPluginClass*	pc;				// my class
	Value**			locals;			// local var array	
	short			flags;			// plugin flags	
	int				version;		// plugin version
	ReferenceTarget* ref;			// ReferenceTarget interface to me
	Tab<IParamBlock2*> pblocks;		// parameter blocks
	
					MSPlugin() { flags = 0; }
	virtual		   ~MSPlugin();
	void			init(MSPluginClass* pc);

	void			gc_trace();
	void			collect() { delete this; }

	void			DeleteThis();
	void			RefDeleted();

	// code management
	void			init_locals();
	Value*			call_handler(Value* handler, Value** arg_list, int count, TimeValue t, BOOL disable_on_error=FALSE);
	void			post_create(ReferenceTarget* me, BOOL loading);
	void			call_all_set_handlers();
	void			disable() { flags |= MSP_DISABLED; }
	void			enable() { flags &= ~MSP_DISABLED; }
	BOOL			enabled() { return !(flags & MSP_DISABLED); }

	// locals
	int				get_local_index(Value* prop);
	Value*			get_local(int index) { return locals[index]; }
	void			set_local(int index, Value* val) { locals[index] = val; }

	// block management
	IParamBlock2*	GetParamBlockByID(BlockID id);

	// UI 
	virtual HWND	AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0)=0;
	virtual void	DeleteRollupPage(HWND hRollup)=0;
	virtual IRollupWindow* GetRollupWindow()=0;

	// factored ReferenceTarget stuff
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message);
	RefTargetHandle clone_from(MSPlugin* obj, ReferenceTarget* obj_as_ref, RemapDir& remap);

	// delegate access 
	virtual ReferenceTarget* get_delegate()=0;

	// I/O
	IOResult		Save(ISave *isave);
    IOResult		Load(ILoad *iload);
	void			post_load();
};

// used for in-memory instance migration when a scripted plugin class is redefined
class MSPluginValueMigrator : public ValueMapper
{
	MSPluginClass*	pc;
	HashTable*		old_locals;
	Array*			old_pblock_defs;
public:

	MSPluginValueMigrator(MSPluginClass* pc, HashTable* old_locals, Array* old_pblock_defs)
	{
		this->pc = pc;
		this->old_locals = old_locals;
		this->old_pblock_defs = old_pblock_defs;
	}

	void map(Value* val) 
	{ 
		if (((MSPlugin*)val)->pc == pc) 
			pc->redefine((MSPlugin*)val, old_locals, old_pblock_defs); 
	}
};

#define MSPLUGIN_CHUNK	0x0010

// ----------------------- MSPluginObject ----------------------

//	template for MSPlugin classes derived from Object
template <class TYPE>
class MSPluginObject : public MSPlugin, public TYPE
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs

	void			DeleteThis();

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow* GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegate 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	IOResult		Save(ISave *isave) { return MSPlugin::Save(isave); }
    IOResult		Load(ILoad *iload) { return MSPlugin::Load(iload); }

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) { return 0; }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) { return 0; }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) { }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) { }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) { }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 	
	BOOL			HasUVW() { return 1; }
	void			SetGenUVW(BOOL sw) { }
	
	// From Object
	ObjectState		Eval(TimeValue time) { return ObjectState(this); }
	void			InitNodeName(TSTR& s) {s = GetObjectName();}
	Interval		ObjectValidity(TimeValue t) { return FOREVER; }
	int				CanConvertToType(Class_ID obtype) { return 0; }
	Object*			ConvertToType(TimeValue t, Class_ID obtype) { return NULL; }
	void			GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist) { }
	void			GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) { Object::GetDeformBBox(t, box, tm, useSel); }
	int				IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) { return 0; }

};

//	template for MSPlugin Xtnd classes derived from Object
template <class TYPE, class MS_SUPER>
class MSObjectXtnd : public MS_SUPER
{
public:
	TYPE*			delegate;		// my delegate

	void			DeleteThis();

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { }; // { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) return delegate->GetObjectName(); else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (TYPE*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
						{ return delegate->HitTest(t, inode, type, crossing, flags, p, vpt); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
						{ return delegate->Display(t, inode, vpt, flags); }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack();
	BOOL			HasUVW() { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) { delegate->SetGenUVW(sw); }
	
	// From Object
	ObjectState		Eval(TimeValue time);
	void			InitNodeName(TSTR& s) {s = GetObjectName();}
	Interval		ObjectValidity(TimeValue t) { return delegate->ObjectValidity(t); }
	int				CanConvertToType(Class_ID obtype) { return delegate->CanConvertToType(obtype); }
	Object*			ConvertToType(TimeValue t, Class_ID obtype) { return delegate->ConvertToType(t, obtype); }
	void			GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist) { delegate->GetCollapseTypes(clist, nlist); }
	void			GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) { delegate->GetDeformBBox(t, box, tm, useSel); }
	int				IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) { return delegate->IntersectRay(t, r, at, norm); }

};

// ----------------------- MSPluginGeomObject ----------------------
//  scripted GeomObject 

class MSPluginGeomObject : public MSPluginObject<GeomObject>
{
public:
					MSPluginGeomObject() { }
					MSPluginGeomObject(MSPluginClass* pc, BOOL loading);
				   ~MSPluginGeomObject() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From GeomObject
	int				IsRenderable() { return 0; }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) { return GeomObject::GetRenderMesh(t, inode, view, needDelete); }
};

class MSGeomObjectXtnd : public MSObjectXtnd<GeomObject, MSPluginGeomObject>
{
public:
					MSGeomObjectXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSGeomObjectXtnd() { DeleteAllRefsFromMe(); }

	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	// From GeomObject
	int				IsRenderable() { return delegate->IsRenderable(); }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) { return delegate->GetRenderMesh(t, inode, view, needDelete); }
};

// ----------------------- MSPluginHelper ----------------------
// scripted HelperObject

class MSPluginHelper : public MSPluginObject<HelperObject>
{
public:
					MSPluginHelper() { }
					MSPluginHelper(MSPluginClass* pc, BOOL loading);
				   ~MSPluginHelper() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From HelperObject
	int				UsesWireColor() { return HelperObject::UsesWireColor(); }   // TRUE if the object color is used for display
	BOOL			NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) { return HelperObject::NormalAlignVector(t, pt, norm); }
};

class MSHelperXtnd : public MSObjectXtnd<HelperObject, MSPluginHelper>
{
public:
					MSHelperXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSHelperXtnd() { DeleteAllRefsFromMe(); }

	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	// From HelperObject
	int				UsesWireColor() { return delegate->UsesWireColor(); }
	BOOL			NormalAlignVector(TimeValue t,Point3 &pt, Point3 &norm) { return delegate->NormalAlignVector(t, pt, norm); }
};

// ----------------------- MSPluginLight ----------------------
// scripted GenLight

class MSPluginLight : public MSPluginObject<GenLight>
{
public:
	NameTab			exclusionList;

					MSPluginLight() { }
					MSPluginLight(MSPluginClass* pc, BOOL loading);
				   ~MSPluginLight() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From LightObject
	RefResult		EvalLightState(TimeValue time, Interval& valid, LightState *ls) { return REF_SUCCEED; }
	ObjLightDesc *	CreateLightDesc(INode *n) { return LightObject::CreateLightDesc(n); }
	void			SetUseLight(int onOff) { }
	BOOL			GetUseLight(void) { return FALSE; }
	void			SetHotspot(TimeValue time, float f) { } 
	float			GetHotspot(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
	void			SetFallsize(TimeValue time, float f) { }
	float			GetFallsize(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
	void			SetAtten(TimeValue time, int which, float f) { }
	float			GetAtten(TimeValue t, int which, Interval& valid = Interval(0,0)) { return 0.0f; }
	void			SetTDist(TimeValue time, float f) { }
	float			GetTDist(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
	void			SetConeDisplay(int s, int notify=TRUE) { }
	BOOL			GetConeDisplay(void) { return FALSE; }
	int 			GetShadowMethod() {return LIGHTSHADOW_NONE;}
	void 			SetRGBColor(TimeValue t, Point3& rgb) { }
	Point3 			GetRGBColor(TimeValue t, Interval &valid = Interval(0,0)) {return Point3(0,0,0);}        
	void 			SetIntensity(TimeValue time, float f) { }
	float 			GetIntensity(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	void 			SetAspect(TimeValue t, float f) { }
	float			GetAspect(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}    
	void 			SetUseAtten(int s) { }
	BOOL 			GetUseAtten(void) {return FALSE;}
	void 			SetAttenDisplay(int s) { }
	BOOL 			GetAttenDisplay(void) {return FALSE;}      
	void 			Enable(int enab) { }
	void 			SetMapBias(TimeValue t, float f) { }
	float 			GetMapBias(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	void 			SetMapRange(TimeValue t, float f) { }
	float 			GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	void 			SetMapSize(TimeValue t, int f) { }
	int 			GetMapSize(TimeValue t, Interval& valid = Interval(0,0)) {return 0;}
	void 			SetRayBias(TimeValue t, float f) { }
	float 			GetRayBias(TimeValue t, Interval& valid = Interval(0,0)) {return 0.0f;}
	int 			GetUseGlobal() {return 0;}
	void 			SetUseGlobal(int a) { }
	int 			GetShadow() {return 0;}
	void 			SetShadow(int a) { }
	int 			GetShadowType() {return 0;}
	void 			SetShadowType(int a) { }
	int 			GetAbsMapBias() {return 0;}
	void 			SetAbsMapBias(int a) { }
	int 			GetOvershoot() {return 0;}
	void 			SetOvershoot(int a) { }
	int 			GetProjector() {return 0;}
	void 			SetProjector(int a) { }
	NameTab* 		GetExclList() { return &exclusionList; }
	BOOL 			Include() {return FALSE;}
	Texmap* 		GetProjMap() {return NULL;}
	void 			SetProjMap(Texmap* pmap) { }
	void 			UpdateTargDistance(TimeValue t, INode* inode) { }

	// From GenLight

	GenLight *		NewLight(int type) { return NULL; }
	int				Type() { return 0; }  // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT
	void			SetType(int tp) { } // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT      
	BOOL			IsSpot() { return FALSE; }
	BOOL			IsDir() { return FALSE; }
	void			SetSpotShape(int s) { }
	int				GetSpotShape(void) { return 0; }
	void 			SetHSVColor(TimeValue t, Point3& hsv) { }
	Point3 			GetHSVColor(TimeValue t, Interval &valid = Interval(0,0)) { return Point3(0.0f, 0.0f, 0.0f); }
	void 			SetContrast(TimeValue time, float f) { }
	float 			GetContrast(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }
	void 			SetUseAttenNear(int s) { }
	BOOL 			GetUseAttenNear(void) { return FALSE; }
	void 			SetAttenNearDisplay(int s) { }
	BOOL 			GetAttenNearDisplay(void) { return FALSE; }

	NameTab&		GetExclusionList() { return exclusionList; }
	void 			SetExclusionList(NameTab &list) { }

	BOOL 			SetHotSpotControl(Control *c) { return FALSE; }
	BOOL 			SetFalloffControl(Control *c) { return FALSE; }
	BOOL 			SetColorControl(Control *c) { return FALSE; }
	Control* 		GetHotSpotControl() { return NULL; }
	Control* 		GetFalloffControl() { return NULL; }
	Control* 		GetColorControl() { return NULL; }
	
	void 			SetAffectDiffuse(BOOL onOff) { }
	BOOL 			GetAffectDiffuse() { return FALSE; }
	void 			SetAffectSpecular(BOOL onOff) { }
	BOOL 			GetAffectSpecular() { return FALSE; }

	void 			SetDecayType(BOOL onOff) { }
	BOOL 			GetDecayType() { return FALSE; }
	void 			SetDecayRadius(TimeValue time, float f) {}
	float 			GetDecayRadius(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f;}
	void 			SetDiffuseSoft(TimeValue time, float f) {}
	float 			GetDiffuseSoft(TimeValue t, Interval& valid = Interval(0,0)) { return 0.0f; }

	void 			SetShadColor(TimeValue t, Point3& rgb) {}
	Point3 			GetShadColor(TimeValue t, Interval &valid = Interval(0,0)) { return Point3(0,0,0); }
	BOOL 			GetLightAffectsShadow() { return FALSE; }
	void 			SetLightAffectsShadow(BOOL b) {  }
	void			SetShadMult(TimeValue t, float m) {  }
	float			GetShadMult(TimeValue t, Interval &valid = Interval(0,0)) { return 1.0f; }

	Texmap* 		GetShadowProjMap() { return NULL;  }
	void 			SetShadowProjMap(Texmap* pmap) {}

	void 			SetAmbientOnly(BOOL onOff) {  }
	BOOL 			GetAmbientOnly() { return FALSE; }

	void			SetAtmosShadows(TimeValue t, int onOff) { }
	int				GetAtmosShadows(TimeValue t) { return 0; }
	void			SetAtmosOpacity(TimeValue t, float f) { }
	float			GetAtmosOpacity(TimeValue t, Interval& valid=FOREVER) { return 0.0f; }
	void			SetAtmosColAmt(TimeValue t, float f) { }
	float			GetAtmosColAmt(TimeValue t, Interval& valid=FOREVER) { return 0.0f; }

	void			SetUseShadowColorMap(TimeValue t, int onOff) { GenLight::SetUseShadowColorMap(t, onOff); }
	int				GetUseShadowColorMap(TimeValue t) { return GenLight::GetUseShadowColorMap(t); }

	void			SetShadowGenerator(ShadowType *s) { GenLight::SetShadowGenerator(s); };
	ShadowType*		GetShadowGenerator() { return GenLight::GetShadowGenerator(); } 

	void			SetEmitterEnable(TimeValue t, BOOL onOff) { GenLight::SetEmitterEnable(t, onOff); }
	BOOL			GetEmitterEnable(TimeValue t, Interval& valid = Interval(0,0)) { return GenLight::GetEmitterEnable(t, valid); }
	void			SetEmitterEnergy(TimeValue t, float energy) { GenLight::SetEmitterEnergy(t, energy); }
	float			GetEmitterEnergy(TimeValue t, Interval& valid = Interval(0,0)) { return GenLight::GetEmitterEnergy(t, valid); }
	void			SetEmitterDecayType(TimeValue t, int decay) { GenLight::SetEmitterDecayType(t, decay); }
	int				GetEmitterDecayType(TimeValue t, Interval& valid = Interval(0,0)) { return GenLight::GetEmitterDecayType(t, valid); }
	void			SetEmitterCausticPhotons(TimeValue t, int photons) { GenLight::SetEmitterCausticPhotons(t, photons); }
	int				GetEmitterCausticPhotons(TimeValue t, Interval& valid = Interval(0,0)) { return GenLight::GetEmitterCausticPhotons(t, valid); }
	void			SetEmitterGlobalIllumPhotons(TimeValue t, int photons) { GenLight::SetEmitterGlobalIllumPhotons(t, photons); }
	int				GetEmitterGlobalIllumPhotons(TimeValue t, Interval& valid = Interval(0,0)) { return GenLight::GetEmitterGlobalIllumPhotons(t, valid); }
};

class MSLightXtnd : public MSObjectXtnd<GenLight, MSPluginLight>
{
public:
					MSLightXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSLightXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

// ^\(\:b+\:a+\:b*\**\:b+\)\(.*\)\( {.*->\)
// \1\2\3\2

	// From LightObject

	RefResult		EvalLightState(TimeValue time, Interval& valid, LightState *ls) { return delegate->EvalLightState(time, valid, ls); }
	ObjLightDesc *	CreateLightDesc(INode *n) { return delegate->CreateLightDesc(n); }
	void			SetUseLight(int onOff) { delegate->SetUseLight(onOff); }
	BOOL			GetUseLight(void) { return delegate->GetUseLight(); }
	void			SetHotspot(TimeValue time, float f) { delegate->SetHotspot(time, f); } 
	float			GetHotspot(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetHotspot(t, valid); }
	void			SetFallsize(TimeValue time, float f) { delegate->SetFallsize(time, f); }
	float			GetFallsize(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetFallsize(t, valid); }
	void			SetAtten(TimeValue time, int which, float f) { delegate->SetAtten(time, which, f); }
	float			GetAtten(TimeValue t, int which, Interval& valid = Interval(0,0)) { return delegate->GetAtten(t, which, valid); }
	void			SetTDist(TimeValue time, float f) { delegate->SetTDist(time, f); }
	float			GetTDist(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetTDist(t, valid); }
	void			SetConeDisplay(int s, int notify=TRUE) { delegate->SetConeDisplay(s, notify); }
	BOOL			GetConeDisplay(void) { return delegate->GetConeDisplay(); }
	int 			GetShadowMethod() {return delegate->GetShadowMethod();}
	void 			SetRGBColor(TimeValue t, Point3& rgb) { delegate->SetRGBColor(t, rgb); }
	Point3 			GetRGBColor(TimeValue t, Interval &valid = Interval(0,0)) {return delegate->GetRGBColor(t, valid);}        
	void 			SetIntensity(TimeValue time, float f) { delegate->SetIntensity(time, f); }
	float 			GetIntensity(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetIntensity(t, valid); }
	void 			SetAspect(TimeValue t, float f) { delegate->SetAspect(t, f); }
	float			GetAspect(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetAspect(t, valid); }    
	void 			SetUseAtten(int s) { delegate->SetUseAtten(s); }
	BOOL 			GetUseAtten(void) { return delegate->GetUseAtten(); }
	void 			SetAttenDisplay(int s) { delegate->SetAttenDisplay(s); }
	BOOL 			GetAttenDisplay(void) { return delegate->GetAttenDisplay(); }      
	void 			Enable(int enab) { delegate->Enable(enab); }
	void 			SetMapBias(TimeValue t, float f) { delegate->SetMapBias(t, f); }
	float 			GetMapBias(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetMapBias(t, valid); }
	void 			SetMapRange(TimeValue t, float f) { delegate->SetMapRange(t, f); }
	float 			GetMapRange(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetMapRange(t, valid); }
	void 			SetMapSize(TimeValue t, int f) { delegate->SetMapSize(t, f); }
	int 			GetMapSize(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetMapSize(t, valid); }
	void 			SetRayBias(TimeValue t, float f) { delegate->SetRayBias(t, f); }
	float 			GetRayBias(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetRayBias(t, valid); }
	int 			GetAbsMapBias() { return delegate->GetAbsMapBias(); }
	void 			SetAbsMapBias(int a) { delegate->SetAbsMapBias(a); }
	int 			GetOvershoot() { return delegate->GetOvershoot(); }
	void 			SetOvershoot(int a) { delegate->SetOvershoot(a); }
	int 			GetProjector() { return delegate->GetProjector(); }
	void 			SetProjector(int a) { delegate->SetProjector(a); }
	NameTab* 		GetExclList() { return delegate->GetExclList(); }
	BOOL 			Include() { return delegate->Include(); }
	Texmap* 		GetProjMap() { return delegate->GetProjMap(); }
	void 			SetProjMap(Texmap* pmap) { delegate->SetProjMap(pmap); }
	void 			UpdateTargDistance(TimeValue t, INode* inode) { delegate->UpdateTargDistance(t, inode); }

	// From GenLight

	GenLight *		NewLight(int type) { return delegate->NewLight(type); }
	int				Type() { return delegate->Type(); }  // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT
	void			SetType(int tp) { delegate->SetType(tp); } // OMNI_LIGHT, TSPOT_LIGHT, DIR_LIGHT, FSPOT_LIGHT, TDIR_LIGHT      
	BOOL			IsSpot() { return delegate->IsSpot(); }
	BOOL			IsDir() { return delegate->IsDir(); }
	void			SetSpotShape(int s) { delegate->SetSpotShape(s); }
	int				GetSpotShape(void) { return delegate->GetSpotShape(); }
	void 			SetHSVColor(TimeValue t, Point3& hsv) { delegate->SetHSVColor(t, hsv); }
	Point3 			GetHSVColor(TimeValue t, Interval &valid = Interval(0,0)) { return delegate->GetHSVColor(t, valid); }
	void 			SetContrast(TimeValue time, float f) { delegate->SetContrast(time, f); }
	float 			GetContrast(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetContrast(t, valid); }
	void 			SetUseAttenNear(int s) { delegate->SetUseAttenNear(s); }
	BOOL 			GetUseAttenNear(void) { return delegate->GetUseAttenNear(); }
	void 			SetAttenNearDisplay(int s) { delegate->SetAttenNearDisplay(s); }
	BOOL 			GetAttenNearDisplay(void) { return delegate->GetAttenNearDisplay(); }

	NameTab&		GetExclusionList() { return delegate->GetExclusionList(); }
	void 			SetExclusionList(NameTab &list) { delegate->SetExclusionList(list); }

	BOOL 			SetHotSpotControl(Control *c) { return delegate->SetHotSpotControl(c); }
	BOOL 			SetFalloffControl(Control *c) { return delegate->SetFalloffControl(c); }
	BOOL 			SetColorControl(Control *c) { return delegate->SetColorControl(c); }
	Control* 		GetHotSpotControl() { return delegate->GetHotSpotControl(); }
	Control* 		GetFalloffControl() { return delegate->GetFalloffControl(); }
	Control* 		GetColorControl() { return delegate->GetColorControl(); }
	
	void 			SetAffectDiffuse(BOOL onOff) { delegate->SetAffectDiffuse(onOff); }
	BOOL 			GetAffectDiffuse() { return delegate->GetAffectDiffuse(); }
	void 			SetAffectSpecular(BOOL onOff) { delegate->SetAffectSpecular(onOff); }
	BOOL 			GetAffectSpecular() { return delegate->GetAffectSpecular(); }

	void 			SetDecayType(BOOL onOff) { delegate->SetDecayType(onOff); }
	BOOL 			GetDecayType() { return delegate->GetDecayType(); }
	void 			SetDecayRadius(TimeValue time, float f) { delegate->SetDecayRadius(time, f); }
	float 			GetDecayRadius(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetDecayRadius(t, valid);}
	void 			SetDiffuseSoft(TimeValue time, float f) { delegate->SetDiffuseSoft(time, f); }
	float 			GetDiffuseSoft(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetDiffuseSoft(t, valid); }

	int 			GetUseGlobal() { return delegate->GetUseGlobal(); }
	void 			SetUseGlobal(int a) { delegate->SetUseGlobal(a); }
	int 			GetShadow() { return delegate->GetShadow(); }
	void 			SetShadow(int a) { delegate->SetShadow(a); }
	int 			GetShadowType() { return delegate->GetShadowType(); }
	void 			SetShadowType(int a) { delegate->SetShadowType(a); }

	void 			SetShadColor(TimeValue t, Point3& rgb) { delegate->SetShadColor(t, rgb); }
	Point3 			GetShadColor(TimeValue t, Interval &valid = Interval(0,0)) { return delegate->GetShadColor(t, valid); }
	BOOL 			GetLightAffectsShadow() { return delegate->GetLightAffectsShadow(); }
	void 			SetLightAffectsShadow(BOOL b) { delegate->SetLightAffectsShadow(b); }
	void			SetShadMult(TimeValue t, float m) { delegate->SetShadMult(t, m); }
	float			GetShadMult(TimeValue t, Interval &valid = Interval(0,0)) { return delegate->GetShadMult(t, valid); }

	Texmap* 		GetShadowProjMap() { return delegate->GetShadowProjMap();  }
	void 			SetShadowProjMap(Texmap* pmap) { delegate->SetShadowProjMap(pmap); }

	void 			SetAmbientOnly(BOOL onOff) { delegate->SetAmbientOnly(onOff); }
	BOOL 			GetAmbientOnly() { return delegate->GetAmbientOnly(); }

	void			SetAtmosShadows(TimeValue t, int onOff) { delegate->SetAtmosShadows(t, onOff);}
	int				GetAtmosShadows(TimeValue t) { return delegate->GetAtmosShadows(t); }
	void			SetAtmosOpacity(TimeValue t, float f) { delegate->SetAtmosOpacity(t, f);}
	float			GetAtmosOpacity(TimeValue t, Interval& valid=FOREVER) { return delegate->GetAtmosOpacity(t); }
	void			SetAtmosColAmt(TimeValue t, float f) { delegate->SetAtmosColAmt(t, f);}
	float			GetAtmosColAmt(TimeValue t, Interval& valid=FOREVER) { return delegate->GetAtmosColAmt(t); }
	
	void			SetUseShadowColorMap(TimeValue t, int onOff) { delegate->SetUseShadowColorMap(t, onOff); }
	int				GetUseShadowColorMap(TimeValue t) { return delegate->GetUseShadowColorMap(t); }

	void			SetShadowGenerator(ShadowType *s) { delegate->SetShadowGenerator(s); }
	ShadowType*		GetShadowGenerator() { return delegate->GetShadowGenerator(); } 

	void			SetEmitterEnable(TimeValue t, BOOL onOff) { delegate->SetEmitterEnable(t, onOff); }
	BOOL			GetEmitterEnable(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetEmitterEnable(t, valid); }
	void			SetEmitterEnergy(TimeValue t, float energy) { delegate->SetEmitterEnergy(t, energy); }
	float			GetEmitterEnergy(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetEmitterEnergy(t, valid); }
	void			SetEmitterDecayType(TimeValue t, int decay) { delegate->SetEmitterDecayType(t, decay); }
	int				GetEmitterDecayType(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetEmitterDecayType(t, valid); }
	void			SetEmitterCausticPhotons(TimeValue t, int photons) { delegate->SetEmitterCausticPhotons(t, photons); }
	int				GetEmitterCausticPhotons(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetEmitterCausticPhotons(t, valid); }
	void			SetEmitterGlobalIllumPhotons(TimeValue t, int photons) { delegate->SetEmitterGlobalIllumPhotons(t, photons); }
	int				GetEmitterGlobalIllumPhotons(TimeValue t, Interval& valid = Interval(0,0)) { return delegate->GetEmitterGlobalIllumPhotons(t, valid); }
};

// ----------------------- MSPluginShape ----------------------
// scripted Shape

class MSPluginShape : public MSPluginObject<ShapeObject>
{
	ShapeHierarchy	sh;
public:
					MSPluginShape() { sh.New(); }
					MSPluginShape(MSPluginClass* pc, BOOL loading);
				   ~MSPluginShape() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From GeomObject
	int				IsRenderable() { return ShapeObject::IsRenderable(); }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) { return ShapeObject::GetRenderMesh(t, inode, view, needDelete); }

	// from ShapeObject
	void			InitNodeName(TSTR& s) { ShapeObject::InitNodeName(s); }
	SClass_ID		SuperClassID() { return ShapeObject::SuperClassID(); }

	int				IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) { return ShapeObject::IntersectRay(t, ray, at, norm); }
	int				NumberOfVertices(TimeValue t, int curve = -1) { return ShapeObject::NumberOfVertices(t, curve); }	// Informational only, curve = -1: total in all curves
	int				NumberOfCurves() { return 0; }                 // Number of curve polygons in the shape
	BOOL			CurveClosed(TimeValue t, int curve) { return FALSE; }     // Returns TRUE if the curve is closed
	Point3			InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) { return Point3 (0,0,0); }    // Interpolate from 0-1 on a curve
	Point3			TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) { return Point3 (0,0,0); }    // Get tangent at point on a curve
	float			LengthOfCurve(TimeValue t, int curve) { return 0.0f; }  // Get the length of a curve
	int				NumberOfPieces(TimeValue t, int curve) { return 0; }   // Number of sub-curves in a curve
	Point3			InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) { return Point3 (0,0,0); }  // Interpolate from 0-1 on a sub-curve
	Point3			TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) { return Point3 (0,0,0); }         // Get tangent on a sub-curve
	BOOL			CanMakeBezier() { return ShapeObject::CanMakeBezier(); }                  // Return TRUE if can turn into a bezier representation
	void			MakeBezier(TimeValue t, BezierShape &shape) { ShapeObject::MakeBezier(t, shape); }     // Create the bezier representation
	ShapeHierarchy& OrganizeCurves(TimeValue t, ShapeHierarchy *hier=NULL) { return sh; }       // Ready for lofting, extrusion, etc.
	void			MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE) { } // Create a PolyShape representation with optional fixed steps & optimization
	int				MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType) { return 0; }  // Generate mesh capping info for the shape
	int				MakeCap(TimeValue t, PatchCapInfo &capInfo) { return ShapeObject::MakeCap(t, capInfo); }	// Only implement if CanMakeBezier=TRUE -- Gen patch cap info

	
	MtlID			GetMatID(TimeValue t, int curve, int piece) { return ShapeObject::GetMatID(t, curve, piece); }
	BOOL			AttachShape(TimeValue t, INode *thisNode, INode *attachNode) { return ShapeObject::AttachShape(t, thisNode, attachNode); }	// Return TRUE if attached
	// UVW Mapping switch access
	BOOL			HasUVW() { return ShapeObject::HasUVW(); }
	void			SetGenUVW(BOOL sw) { ShapeObject::SetGenUVW(sw); }

	// These handle loading and saving the data in this class. Should be called
	// by derived class BEFORE it loads or saves any chunks
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return ShapeObject::Save(isave); }
    IOResult		Load(ILoad *iload) {  MSPlugin::Load(iload); return ShapeObject::Load(iload); }

	Class_ID		PreferredCollapseType() { return ShapeObject::PreferredCollapseType(); }
	BOOL			GetExtendedProperties(TimeValue t, TSTR &prop1Label, TSTR &prop1Data, TSTR &prop2Label, TSTR &prop2Data)
						{ return ShapeObject::GetExtendedProperties(t, prop1Label, prop1Data, prop2Label, prop2Data); }
	void			RescaleWorldUnits(float f) { ShapeObject::RescaleWorldUnits(f); }
};

class MSShapeXtnd : public MSObjectXtnd<ShapeObject, MSPluginShape>
{
public:
					MSShapeXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSShapeXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From GeomObject
	int				IsRenderable() { return delegate->IsRenderable(); }		
	Mesh*			GetRenderMesh(TimeValue t, INode *inode, View &view, BOOL& needDelete) { return delegate->GetRenderMesh(t, inode, view, needDelete); }

	// from ShapeObject
	void			InitNodeName(TSTR& s) { delegate->InitNodeName(s); }
	SClass_ID		SuperClassID() { return delegate->SuperClassID(); }

	int				IntersectRay(TimeValue t, Ray& ray, float& at, Point3& norm) { return delegate->IntersectRay(t, ray, at, norm); }
	int				NumberOfVertices(TimeValue t, int curve = -1) { return delegate->NumberOfVertices(t, curve); }	
	int				NumberOfCurves() { return delegate->NumberOfCurves(); }                 
	BOOL			CurveClosed(TimeValue t, int curve) { return delegate->CurveClosed(t, curve); }     
	Point3			InterpCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) { return delegate->InterpCurve3D(t, curve, param, ptype); }    
	Point3			TangentCurve3D(TimeValue t, int curve, float param, int ptype=PARAM_SIMPLE) { return delegate->TangentCurve3D(t, curve, param, ptype); }    
	float			LengthOfCurve(TimeValue t, int curve) { return delegate->LengthOfCurve(t, curve); }  
	int				NumberOfPieces(TimeValue t, int curve) { return delegate->NumberOfPieces(t, curve); }   
	Point3			InterpPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) { return delegate->InterpPiece3D(t, curve, piece, param, ptype); }  
	Point3			TangentPiece3D(TimeValue t, int curve, int piece, float param, int ptype=PARAM_SIMPLE) { return delegate->TangentPiece3D(t, curve, piece, param, ptype); }         
	BOOL			CanMakeBezier() { return delegate->CanMakeBezier(); }                  
	void			MakeBezier(TimeValue t, BezierShape &shape) { delegate->MakeBezier(t, shape); }     
	ShapeHierarchy& OrganizeCurves(TimeValue t, ShapeHierarchy *hier=NULL) { return delegate->OrganizeCurves(t, hier); }       
	void			MakePolyShape(TimeValue t, PolyShape &shape, int steps = PSHAPE_BUILTIN_STEPS, BOOL optimize = FALSE) { delegate->MakePolyShape(t, shape, steps, optimize); } 
	int				MakeCap(TimeValue t, MeshCapInfo &capInfo, int capType) { return delegate->MakeCap(t, capInfo, capType); }  
	int				MakeCap(TimeValue t, PatchCapInfo &capInfo) { return delegate->MakeCap(t, capInfo); }	

	
	MtlID			GetMatID(TimeValue t, int curve, int piece) { return delegate->GetMatID(t, curve, piece); }
	BOOL			AttachShape(TimeValue t, INode *thisNode, INode *attachNode) { return delegate->AttachShape(t, thisNode, attachNode); }	// Return TRUE if attached
	// UVW Mapping switch access
	BOOL			HasUVW() { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) { delegate->SetGenUVW(sw); }

	// These handle loading and saving the data in this class. Should be called
	// by derived class BEFORE it loads or saves any chunks
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return ShapeObject::Save(isave); }
    IOResult		Load(ILoad *iload) {  MSPlugin::Load(iload); return ShapeObject::Load(iload); }

	Class_ID		PreferredCollapseType() { return delegate->PreferredCollapseType(); }
	BOOL			GetExtendedProperties(TimeValue t, TSTR &prop1Label, TSTR &prop1Data, TSTR &prop2Label, TSTR &prop2Data)
						{ return delegate->GetExtendedProperties(t, prop1Label, prop1Data, prop2Label, prop2Data); }
	void			RescaleWorldUnits(float f) { delegate->RescaleWorldUnits(f); }

};

// ----------------------- MSPluginSimpleObject ----------------------
//	scriptable SimpleObject, mesh building and all

class MSPluginSimpleObject : public MSPlugin, public SimpleObject
{
public:
	IObjParam*		ip;			// ip for any currently open command panel dialogs

					MSPluginSimpleObject() { }
					MSPluginSimpleObject(MSPluginClass* pc, BOOL loading);
				   ~MSPluginSimpleObject() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis();

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow* GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegate 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						SimpleObject::NotifyRefChanged(changeInt, hTarget, partID, message); 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult		Save(ISave *isave) { return MSPlugin::Save(isave); }
    IOResult		Load(ILoad *iload) { return MSPlugin::Load(iload); }

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

	// From SimpleObject
	void			BuildMesh(TimeValue t);
	BOOL			OKtoDisplay(TimeValue t);
	void			InvalidateUI();
	CreateMouseCallBack* GetCreateMouseCallBack();
	BOOL			HasUVW();
	void			SetGenUVW(BOOL sw);
};

//	MSSimpleObjectXtnd
class MSSimpleObjectXtnd : public MSPluginSimpleObject
{
public:
	SimpleObject*	delegate;		// my delegate

					MSSimpleObjectXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSSimpleObjectXtnd() { DeleteAllRefsFromMe(); }

	void			DeleteThis();

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) return delegate->GetObjectName(); else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (SimpleObject*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
						{ return delegate->HitTest(t, inode, type, crossing, flags, p, vpt); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
						{ return delegate->Display(t, inode, vpt, flags); }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() { return delegate->GetCreateMouseCallBack(); } 
	BOOL			HasUVW() { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) { delegate->SetGenUVW(sw); }
	
	// From Object
	ObjectState		Eval(TimeValue time);
	void			InitNodeName(TSTR& s) {s = GetObjectName();}
	Interval		ObjectValidity(TimeValue t) { return delegate->ObjectValidity(t); }
	int				CanConvertToType(Class_ID obtype) { return delegate->CanConvertToType(obtype); }
	Object*			ConvertToType(TimeValue t, Class_ID obtype) { return delegate->ConvertToType(t, obtype); }
	void			GetCollapseTypes(Tab<Class_ID> &clist, Tab<TSTR*> &nlist) { delegate->GetCollapseTypes(clist, nlist); }
	void			GetDeformBBox(TimeValue t, Box3& box, Matrix3 *tm, BOOL useSel) { delegate->GetDeformBBox(t, box, tm, useSel); }
	int				IntersectRay(TimeValue t, Ray& r, float& at, Point3& norm) { return delegate->IntersectRay(t, r, at, norm); }

	void			BuildMesh(TimeValue t) { delegate->BuildMesh(t); }
	BOOL			OKtoDisplay(TimeValue t) { return delegate->OKtoDisplay(t); }
	void			InvalidateUI() { delegate->InvalidateUI(); }
	ParamDimension* GetParameterDim(int pbIndex) { return delegate->GetParameterDim(pbIndex); }
	TSTR			GetParameterName(int pbIndex) { return delegate->GetParameterName(pbIndex); }

};

// ----------------------- MSPluginModifier ----------------------
// scripted Modifier

class MSPluginModifier : public MSPlugin, public Modifier
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs

					MSPluginModifier() { }
					MSPluginModifier(MSPluginClass* pc, BOOL loading);
				   ~MSPluginModifier() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis() { MSPlugin::DeleteThis(); }

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow* GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegate 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	SClass_ID		SuperClassID() { return pc->sclass_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return Modifier::Save(isave); }
    IOResult		Load(ILoad *iload) { MSPlugin::Load(iload); return Modifier::Load(iload); }

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt) { return 0; }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) { return 0; }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) { }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) { }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) { }
	CreateMouseCallBack* GetCreateMouseCallBack() { return NULL; } 	
	BOOL			HasUVW() { return 1; }
	void			SetGenUVW(BOOL sw) { }

	// from Modifier
	Interval		LocalValidity(TimeValue t) { return Modifier::LocalValidity(t); }
	ChannelMask		ChannelsUsed() { return 0; }
	ChannelMask		ChannelsChanged() { return 0; }
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) { Modifier::NotifyInputChanged(changeInt, partID, message, mc); }

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) { }

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) { return Modifier::DependOnTopology(mc); }

	// this can return:
	//   DEFORM_OBJ_CLASS_ID -- not really a class, but so what
	//   MAPPABLE_OBJ_CLASS_ID -- ditto
	//   TRIOBJ_CLASS_ID
	//   BEZIER_PATCH_OBJ_CLASS_ID
	Class_ID		InputType() { return Class_ID(DEFORM_OBJ_CLASS_ID,0); }

	IOResult		SaveLocalData(ISave *isave, LocalModData *ld) { return Modifier::SaveLocalData(isave, ld); }  
	IOResult		LoadLocalData(ILoad *iload, LocalModData **pld) { return Modifier::LoadLocalData(iload, pld); }  

};

class MSModifierXtnd : public MSPluginModifier
{
public:
	Modifier*		delegate;		// my delegate

					MSModifierXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSModifierXtnd() { DeleteAllRefsFromMe(); }

					void DeleteThis() { MSPlugin::DeleteThis(); }

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	SClass_ID		SuperClassID() { return pc->sclass_id; }
	void			FreeCaches() { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) return delegate->GetObjectName(); else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (Modifier*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt)
						{ return delegate->HitTest(t, inode, type, crossing, flags, p, vpt); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags) 
						{ return delegate->Display(t, inode, vpt, flags); }		
	void			SetExtendedDisplay(int flags) { SetExtendedDisplay( flags); }      // for setting mode-dependent display attributes
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box) { delegate->GetWorldBoundBox(t, inode, vpt, box); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() { return delegate->GetCreateMouseCallBack(); } 
	BOOL			ChangeTopology() {return delegate->ChangeTopology();}

	void			Move( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ){ delegate->Move( t, partm, tmAxis, val, localOrigin ); }
	void			Rotate( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Quat& val, BOOL localOrigin=FALSE ){ delegate->Rotate( t, partm, tmAxis, val, localOrigin ); }
	void			Scale( TimeValue t, Matrix3& partm, Matrix3& tmAxis, Point3& val, BOOL localOrigin=FALSE ){ delegate->Scale( t, partm, tmAxis, val, localOrigin ); }
	void			TransformStart(TimeValue t) { delegate->TransformStart( t); }
	void			TransformHoldingStart(TimeValue t) { delegate->TransformHoldingStart( t); }
	void			TransformHoldingFinish(TimeValue t) { delegate->TransformHoldingFinish( t); }             
	void			TransformFinish(TimeValue t) { delegate->TransformFinish( t); }            
	void			TransformCancel(TimeValue t) { delegate->TransformCancel( t); }            
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc) { return delegate->HitTest( t, inode, type, crossing, flags, p, vpt, mc); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext* mc) { return delegate->Display( t, inode, vpt, flags, mc); };   // quick render in viewport, using current TM.         
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) { delegate->GetWorldBoundBox( t, inode, vpt, box, mc); }

	void			CloneSelSubComponents(TimeValue t) { delegate->CloneSelSubComponents( t); }
	void			AcceptCloneSelSubComponents(TimeValue t) { delegate->AcceptCloneSelSubComponents( t); }
	void			 SelectSubComponent(
					HitRecord *hitRec, BOOL selected, BOOL all, BOOL invert=FALSE) { delegate->SelectSubComponent(hitRec, selected, all, invert); }
	void			ClearSelection(int selLevel) { delegate->ClearSelection( selLevel); }
	void			SelectAll(int selLevel) { delegate->SelectAll( selLevel); }
	void			InvertSelection(int selLevel) { delegate->InvertSelection( selLevel); }
	int				SubObjectIndex(HitRecord *hitRec) {return  delegate->SubObjectIndex(hitRec);}               
	void			ActivateSubobjSel(int level, XFormModes& modes ) { delegate->ActivateSubobjSel( level, modes ); }
	BOOL			SupportsNamedSubSels() {return  delegate->SupportsNamedSubSels();}
	void			ActivateSubSelSet(TSTR &setName) { delegate->ActivateSubSelSet(setName); }
	void			NewSetFromCurSel(TSTR &setName) { delegate->NewSetFromCurSel(setName); }
	void			RemoveSubSelSet(TSTR &setName) { delegate->RemoveSubSelSet(setName); }
	void			SetupNamedSelDropDown() { delegate->SetupNamedSelDropDown(); }
	int				NumNamedSelSets() {return  delegate->NumNamedSelSets();}
	TSTR			GetNamedSelSetName(int i) {return  delegate->GetNamedSelSetName( i);}
	void			SetNamedSelSetName(int i,TSTR &newName) { delegate->SetNamedSelSetName( i, newName); }
	void			NewSetByOperator(TSTR &newName,Tab<int> &sets,int op) { delegate->NewSetByOperator(newName, sets, op); }
	void			GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) { delegate->GetSubObjectCenters(cb, t, node, mc); }
	void			GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) { delegate->GetSubObjectTMs( cb, t, node, mc); }                          
	BOOL			HasUVW () { return delegate->HasUVW(); }
	BOOL			HasUVW (int mapChannel) { return delegate->HasUVW (mapChannel); }
	void			SetGenUVW(BOOL sw) { delegate->SetGenUVW( sw);   }	// applies to mapChannel 1
	void			SetGenUVW (int mapChannel, BOOL sw) { delegate->SetGenUVW ( mapChannel, sw); }
	void			ShowEndResultChanged (BOOL showEndResult) { delegate->ShowEndResultChanged ( showEndResult);  }
	
	// from Modifier
	Interval		LocalValidity(TimeValue t) { return delegate->LocalValidity(t); }
	ChannelMask		ChannelsUsed() { return delegate->ChannelsUsed(); }
	ChannelMask		ChannelsChanged() { return delegate->ChannelsChanged(); }
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) { delegate->NotifyInputChanged(changeInt, partID, message, mc); }

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) { delegate->ModifyObject(t, mc, os, node); }

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) { return delegate->DependOnTopology(mc); }

	// this can return:
	//   DEFORM_OBJ_CLASS_ID -- not really a class, but so what
	//   MAPPABLE_OBJ_CLASS_ID -- ditto
	//   TRIOBJ_CLASS_ID
	//   BEZIER_PATCH_OBJ_CLASS_ID
	Class_ID		InputType() { return delegate->InputType(); }

	IOResult		SaveLocalData(ISave *isave, LocalModData *ld) { return delegate->SaveLocalData(isave, ld); }  
	IOResult		LoadLocalData(ILoad *iload, LocalModData **pld) { return delegate->LoadLocalData(iload, pld); }  
};

// ----------------------- MSPluginSimpleMod ----------------------
// scripted SimpleMod  (this one has full-implementation handler calls)

class MSPluginSimpleMod : public MSPlugin, public SimpleMod
{
public:
	IObjParam*		ip;					// ip for any currently open command panel dialogs
	Point3Value*	vec;				// cache for the Map parameter & local values
	Point3Value*	extent;
	Point3Value*	min;
	Point3Value*	max;
	Point3Value*	center;
	BOOL			busy;
	TimeValue		last_time;
	Point3			last_in, last_out;

	static CRITICAL_SECTION def_sync;	// thread synch for Map parameter cache
	static BOOL		setup_sync;
	
					MSPluginSimpleMod() { vec = extent = min = max = center = NULL; busy = FALSE; last_time = TIME_NegInfinity; }
					MSPluginSimpleMod(MSPluginClass* pc, BOOL loading);
				   ~MSPluginSimpleMod();

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	void			DeleteThis() { MSPlugin::DeleteThis(); }

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow* GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegate 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	SClass_ID		SuperClassID() { return pc->sclass_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count() + 2; }  
	Animatable*		SubAnim(int i);
	TSTR			SubAnimName(int i);
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						if (message == REFMSG_CHANGE) 
							last_time = TIME_NegInfinity;
						SimpleMod::NotifyRefChanged(changeInt, hTarget, partID, message);
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 2; }
	RefTargetHandle GetReference(int i);
	void			SetReference(int i, RefTargetHandle rtarg);
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return SimpleMod::Save(isave); }
    IOResult		Load(ILoad *iload) { MSPlugin::Load(iload); return SimpleMod::Load(iload); }

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);

	// Clients of SimpleMod need to implement this method
	Deformer&		GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat);
	void			InvalidateUI();
	Interval		GetValidity(TimeValue t);
	BOOL			GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis);
};

class MSSimpleModXtnd : public MSPluginSimpleMod
{
public:
	SimpleMod*		delegate;		// my delegate

					MSSimpleModXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSSimpleModXtnd() { DeleteAllRefsFromMe(); }

					void DeleteThis() { MSPlugin::DeleteThis(); }

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	SClass_ID		SuperClassID() { return pc->sclass_id; }
	void			FreeCaches() { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) return delegate->GetObjectName(); else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (SimpleMod*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From BaseObject
	TCHAR *			GetObjectName() { return pc->class_name->to_string(); }
	void			BeginEditParams( IObjParam  *ip, ULONG flags,Animatable *prev);
	void			EndEditParams( IObjParam *ip, ULONG flags,Animatable *next);
	int				HitTest(TimeValue t, INode* inode, int type, int crossing, int flags, IPoint2 *p, ViewExp *vpt, ModContext* mc)
						{ return delegate->HitTest(t, inode, type, crossing, flags, p, vpt, mc); }
	int				Display(TimeValue t, INode* inode, ViewExp *vpt, int flags, ModContext *mc) 
						{ return delegate->Display(t, inode, vpt, flags, mc); }		
	void			GetWorldBoundBox(TimeValue t,INode* inode, ViewExp *vpt, Box3& box, ModContext *mc) { delegate->GetWorldBoundBox(t, inode, vpt, box, mc); }
	void			GetLocalBoundBox(TimeValue t, INode *inode,ViewExp* vpt,  Box3& box ) { delegate->GetLocalBoundBox(t, inode, vpt,  box ); }
	void			Snap(TimeValue t, INode* inode, SnapInfo *snap, IPoint2 *p, ViewExp *vpt) { delegate->Snap(t, inode, snap, p, vpt); }
	CreateMouseCallBack* GetCreateMouseCallBack() { return delegate->GetCreateMouseCallBack(); } 
	BOOL			HasUVW() { return delegate->HasUVW(); }
	void			SetGenUVW(BOOL sw) { delegate->SetGenUVW(sw); }
		
	void			GetSubObjectCenters(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) { delegate->GetSubObjectCenters(cb, t, node, mc); }
	void			GetSubObjectTMs(SubObjAxisCallback *cb,TimeValue t,INode *node,ModContext *mc) { delegate->GetSubObjectTMs(cb, t, node, mc); }
	BOOL			ChangeTopology() { return delegate->ChangeTopology(); }
	
	// from Modifier
	Interval		LocalValidity(TimeValue t) { return delegate->LocalValidity(t); }
	ChannelMask		ChannelsUsed() { return delegate->ChannelsUsed(); }
	ChannelMask		ChannelsChanged() { return delegate->ChannelsChanged(); }
	// this is used to invalidate cache's in Edit Modifiers:
	void			NotifyInputChanged(Interval changeInt, PartID partID, RefMessage message, ModContext *mc) { delegate->NotifyInputChanged(changeInt, partID, message, mc); }

	// This is the method that is called when the modifier is needed to 
	// apply its effect to the object. Note that the INode* is always NULL
	// for object space modifiers.
	void			ModifyObject(TimeValue t, ModContext &mc, ObjectState* os, INode *node) { delegate->ModifyObject(t, mc, os, node); }

	// Modifiers that place a dependency on topology should return TRUE
	// for this method. An example would be a modifier that stores a selection
	// set base on vertex indices.
	BOOL			DependOnTopology(ModContext &mc) { return delegate->DependOnTopology(mc); }

	// this can return:
	//   DEFORM_OBJ_CLASS_ID -- not really a class, but so what
	//   MAPPABLE_OBJ_CLASS_ID -- ditto
	//   TRIOBJ_CLASS_ID
	//   BEZIER_PATCH_OBJ_CLASS_ID
	Class_ID		InputType() { return delegate->InputType(); }

	IOResult		SaveLocalData(ISave *isave, LocalModData *ld) { return delegate->SaveLocalData(isave, ld); }  
	IOResult		LoadLocalData(ILoad *iload, LocalModData **pld) { return delegate->LoadLocalData(iload, pld); } 

	// Clients of SimpleMod need to implement this method
	Deformer&		GetDeformer(TimeValue t,ModContext &mc,Matrix3& mat,Matrix3& invmat) { return delegate->GetDeformer(t, mc, mat, invmat); }
	void			InvalidateUI() { delegate->InvalidateUI(); }
	Interval		GetValidity(TimeValue t) { return delegate->GetValidity(t); }
	BOOL			GetModLimits(TimeValue t,float &zmin, float &zmax, int &axis) { return delegate->GetModLimits(t, zmin,  zmax,  axis); }

};

// ----------------------- MSPluginTexmap ----------------------
// scripted Texmap

class MSPluginTexmap : public MSPlugin, public Texmap
{
public:
	static MSAutoMParamDlg* masterMDlg;						// master dialog containing all scripted rollout
	static IMtlParams*		ip;

					MSPluginTexmap() { }
					MSPluginTexmap(MSPluginClass* pc, BOOL loading);
				   ~MSPluginTexmap() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow*  GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; } 


	// From Animatable
	void			DeleteThis();
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From MtlBase

	TSTR			GetFullName() { return MtlBase::GetFullName(); }
	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) { return MtlBase::BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) { return MtlBase::Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) { return MtlBase::LocalRequirements(subMtlNum); }
	BOOL			IsMultiMtl() { return MtlBase::IsMultiMtl(); }
	int				NumSubTexmaps();
	Texmap*			GetSubTexmap(int i);
//	int				MapSlotType(int i) { return MtlBase::MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m);
//	int				SubTexmapOn(int i) { return MtlBase::SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { MtlBase::DeactivateMapsInTree(); }     
	TSTR			GetSubTexmapSlotName(int i);
	TSTR			GetSubTexmapTVName(int i) { return GetSubTexmapSlotName(i); }
//	void			CopySubTexmap(HWND hwnd, int ifrom, int ito) { MtlBase::CopySubTexmap(hwnd, ifrom, ito); }     
	void			Update(TimeValue t, Interval& valid) { }
	void			Reset() { pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t);
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return MtlBase::Save(isave); }
    IOResult		Load(ILoad *iload) { MSPlugin::Load(iload); return MtlBase::Load(iload); }
	ULONG			GetGBufID() { return MtlBase::GetGBufID(); }
	void			SetGBufID(ULONG id) { MtlBase::SetGBufID(id); }
	void			EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) { MtlBase::EnumAuxFiles(nameEnum, flags); }   
	PStamp*			GetPStamp(int sz) { return MtlBase::GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return MtlBase::CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) { MtlBase::DiscardPStamp(sz); }      		

	// From Texmap

	// Evaluate the color of map for the context.
	AColor			EvalColor(ShadeContext& sc)  { return AColor (0,0,0); }
	
	// Evaluate the map for a "mono" channel.
	// this just permits a bit of optimization 
	float			EvalMono(ShadeContext& sc) { return Texmap::EvalMono(sc); }
	
	// For Bump mapping, need a perturbation to apply to a normal.
	// Leave it up to the Texmap to determine how to do this.
	Point3			EvalNormalPerturb(ShadeContext& sc) { return Point3(0,0,0); }

	// This query is made of maps plugged into the Reflection or 
	// Refraction slots:  Normally the view vector is replaced with
	// a reflected or refracted one before calling the map: if the 
	// plugged in map doesn't need this , it should return TRUE.
	BOOL			HandleOwnViewPerturb() { return Texmap::HandleOwnViewPerturb(); }

	BOOL			SupportTexDisplay() { return Texmap::SupportTexDisplay(); }
	void			ActivateTexDisplay(BOOL onoff) { Texmap::ActivateTexDisplay(onoff); }
	DWORD			GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {return Texmap::GetActiveTexHandle(t, thmaker);}

	void			GetUVTransform(Matrix3 &uvtrans) {Texmap::GetUVTransform(uvtrans); }
	int				GetTextureTiling() { return Texmap::GetTextureTiling(); }
	void			InitSlotType(int sType) { Texmap::InitSlotType(sType); }			   
	int				GetUVWSource() { return Texmap::GetUVWSource(); }
	int				GetMapChannel () { return Texmap::GetMapChannel (); }	// only relevant if above returns UVWSRC_EXPLICIT

	UVGen *			GetTheUVGen() { return Texmap::GetTheUVGen(); }  // maps with a UVGen should implement this
	XYZGen *		GetTheXYZGen() { return Texmap::GetTheXYZGen(); } // maps with a XYZGen should implement this

	// System function to set slot type for all subtexmaps in a tree.
	void			SetOutputLevel(TimeValue t, float v) { Texmap::SetOutputLevel(t, v); }

	// called prior to render: missing map names should be added to NameAccum.
	// return 1: success,   0:failure. 
	int				LoadMapFiles(TimeValue t) { return Texmap::LoadMapFiles(t); } 

	// render a 2-d bitmap version of map.
	void			RenderBitmap(TimeValue t, Bitmap *bm, float scale3D=1.0f, BOOL filter = FALSE) { Texmap::RenderBitmap(t, bm, scale3D, filter); }

	void			RefAdded(RefMakerHandle rm){ Texmap::RefAdded(rm); }

};


class MSTexmapXtnd : public MSPluginTexmap
{
public:
	Texmap*			delegate;		// my delegate

					MSTexmapXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSTexmapXtnd() { DeleteAllRefsFromMe(); }

	void			DeleteThis();

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) return pc->extend_cd->ClassName(); else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (Texmap*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From MtlBase

	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) { return delegate->BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) { return delegate->Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) { return delegate->LocalRequirements(subMtlNum); }
	BOOL			IsMultiMtl() { return delegate->IsMultiMtl(); }
	void			Update(TimeValue t, Interval& valid) { delegate->Update(t, valid); }
	void			Reset() { delegate->Reset(); pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t);
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
	IOResult		Save(ISave *isave) { return MSPlugin::Save(isave); } // return delegate->Save(isave); }
    IOResult		Load(ILoad *iload) { return MSPlugin::Load(iload); } // return delegate->Load(iload); }
	ULONG			GetGBufID() { return delegate->GetGBufID(); }
	void			SetGBufID(ULONG id) { delegate->SetGBufID(id); }
	void			EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) { delegate->EnumAuxFiles(nameEnum, flags); }   
	PStamp*			GetPStamp(int sz) { return delegate->GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return delegate->CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) { delegate->DiscardPStamp(sz); }      		

	int				NumSubTexmaps();
	Texmap*			GetSubTexmap(int i);
//	int				MapSlotType(int i) { return MtlBase::MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m);
//	int				SubTexmapOn(int i) { return MtlBase::SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { MtlBase::DeactivateMapsInTree(); }     
	TSTR			GetSubTexmapSlotName(int i);

	// From Texmap

	// Evaluate the color of map for the context.
	AColor			EvalColor(ShadeContext& sc)  { return delegate->EvalColor(sc); }
	
	// Evaluate the map for a "mono" channel.
	// this just permits a bit of optimization 
	float			EvalMono(ShadeContext& sc) { return delegate->EvalMono(sc); }
	
	// For Bump mapping, need a perturbation to apply to a normal.
	// Leave it up to the Texmap to determine how to do this.
	Point3			EvalNormalPerturb(ShadeContext& sc) { return Point3(0,0,0); }

	// This query is made of maps plugged into the Reflection or 
	// Refraction slots:  Normally the view vector is replaced with
	// a reflected or refracted one before calling the map: if the 
	// plugged in map doesn't need this , it should return TRUE.
	BOOL			HandleOwnViewPerturb() { return delegate->HandleOwnViewPerturb(); }

	BOOL			SupportTexDisplay() { return delegate->SupportTexDisplay(); }
	void			ActivateTexDisplay(BOOL onoff) { delegate->ActivateTexDisplay(onoff); }
	DWORD			GetActiveTexHandle(TimeValue t, TexHandleMaker& thmaker) {return delegate->GetActiveTexHandle(t, thmaker);}

	void			GetUVTransform(Matrix3 &uvtrans) {delegate->GetUVTransform(uvtrans); }
	int				GetTextureTiling() { return delegate->GetTextureTiling(); }
	void			InitSlotType(int sType) { delegate->InitSlotType(sType); }			   
	int				GetUVWSource() { return delegate->GetUVWSource(); }
	int				GetMapChannel () { return delegate->GetMapChannel (); }	// only relevant if above returns UVWSRC_EXPLICIT

	UVGen *			GetTheUVGen() { return delegate->GetTheUVGen(); }  // maps with a UVGen should implement this
	XYZGen *		GetTheXYZGen() { return delegate->GetTheXYZGen(); } // maps with a XYZGen should implement this

	// System function to set slot type for all subtexmaps in a tree.
	void			SetOutputLevel(TimeValue t, float v) { delegate->SetOutputLevel(t, v); }

	// called prior to render: missing map names should be added to NameAccum.
	// return 1: success,   0:failure. 
	int				LoadMapFiles(TimeValue t) { return delegate->LoadMapFiles(t); } 

	// render a 2-d bitmap version of map.
	void			RenderBitmap(TimeValue t, Bitmap *bm, float scale3D=1.0f, BOOL filter = FALSE) { delegate->RenderBitmap(t, bm, scale3D, filter); }

//	void			RefAdded(RefMakerHandle rm){ delegate->RefAdded(rm); }
};

// ----------------------- MSPluginMtl ----------------------
// scripted Mtl

class MSPluginMtl : public MSPlugin, public Mtl
{
public:
	static MSAutoMParamDlg* masterMDlg; // master dialog containing all scripted rollout
	static IMtlParams*		ip;

					MSPluginMtl() { }
					MSPluginMtl(MSPluginClass* pc, BOOL loading);
				   ~MSPluginMtl() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);

	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow*  GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; } 


	// From Animatable
	void			DeleteThis();
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) 
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From MtlBase

	TSTR			GetFullName() { return MtlBase::GetFullName(); }
	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) { return MtlBase::BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) { return MtlBase::Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) { return MtlBase::LocalRequirements(subMtlNum); }
	BOOL			IsMultiMtl() { return MtlBase::IsMultiMtl(); }
	int				NumSubTexmaps();
	Texmap*			GetSubTexmap(int i);
//	int				MapSlotType(int i) { return MtlBase::MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m);
//	int				SubTexmapOn(int i) { return MtlBase::SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { MtlBase::DeactivateMapsInTree(); }     
	TSTR			GetSubTexmapSlotName(int i);
	TSTR			GetSubTexmapTVName(int i) { return GetSubTexmapSlotName(i); }
//	void			CopySubTexmap(HWND hwnd, int ifrom, int ito) { MtlBase::CopySubTexmap(hwnd, ifrom, ito); }     
	void			Update(TimeValue t, Interval& valid) { }
	void			Reset() { pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t);
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return MtlBase::Save(isave); }
    IOResult		Load(ILoad *iload) { MSPlugin::Load(iload); return MtlBase::Load(iload); }
	ULONG			GetGBufID() { return MtlBase::GetGBufID(); }
	void			SetGBufID(ULONG id) { MtlBase::SetGBufID(id); }
	void			EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) { MtlBase::EnumAuxFiles(nameEnum, flags); }   
	PStamp*			GetPStamp(int sz) { return MtlBase::GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return MtlBase::CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) { MtlBase::DiscardPStamp(sz); }      		

	// From Mtl

	Texmap*			GetActiveTexmap() { return Mtl::GetActiveTexmap(); } 
	void			SetActiveTexmap( Texmap *txm) { Mtl::SetActiveTexmap(txm); } 
	void			RefDeleted() { Mtl::RefDeleted(); MSPlugin::RefDeleted(); } 
	void			RefAdded(RefMakerHandle rm) { Mtl::RefAdded(rm); }  
	Color			GetAmbient(int mtlNum=0, BOOL backFace=FALSE) { return Color(0,0,0); }
	Color			GetDiffuse(int mtlNum=0, BOOL backFace=FALSE) { return Color(0,0,0); }	    
	Color			GetSpecular(int mtlNum=0, BOOL backFace=FALSE) { return Color(0,0,0); }
	float			GetShininess(int mtlNum=0, BOOL backFace=FALSE) { return 0.0f; }
	float			GetShinStr(int mtlNum=0, BOOL backFace=FALSE) { return 0.0f; }		
	float			GetXParency(int mtlNum=0, BOOL backFace=FALSE) { return 0.0f; }
	BOOL			GetSelfIllumColorOn(int mtlNum=0, BOOL backFace=FALSE) { return Mtl::GetSelfIllumColorOn(mtlNum, backFace); } 
	float			GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE) { return Mtl::GetSelfIllum(mtlNum, backFace); } 
	Color			GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE) { return Mtl::GetSelfIllumColor(mtlNum, backFace); } 
	float			WireSize(int mtlNum=0, BOOL backFace=FALSE) { return Mtl::WireSize(mtlNum, backFace); } 
	void			SetAmbient(Color c, TimeValue t) { }		
	void			SetDiffuse(Color c, TimeValue t) { }		
	void			SetSpecular(Color c, TimeValue t) { }
	void			SetShininess(float v, TimeValue t) { }	
	void			Shade(ShadeContext& sc) { }
	int				NumSubMtls();
	Mtl*			GetSubMtl(int i);
	void			SetSubMtl(int i, Mtl *m);
	TSTR			GetSubMtlSlotName(int i);
	TSTR			GetSubMtlTVName(int i) { return GetSubMtlSlotName(i); } 					  
//	void			CopySubMtl(HWND hwnd, int ifrom, int ito) { Mtl::CopySubMtl(hwnd, ifrom, ito); }  
	float			GetDynamicsProperty(TimeValue t, int mtlNum, int propID) { return Mtl::GetDynamicsProperty(t, mtlNum, propID); } 
	void			SetDynamicsProperty(TimeValue t, int mtlNum, int propID, float value) { Mtl::SetDynamicsProperty(t, mtlNum, propID, value); } 
	float			EvalDisplacement(ShadeContext& sc) { return Mtl::EvalDisplacement(sc); } 
	Interval		DisplacementValidity(TimeValue t) { return Mtl::DisplacementValidity(t); } 

};


class MSMtlXtnd : public MSPluginMtl
{
public:
	Mtl*			delegate;		// my delegate

					MSMtlXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSMtlXtnd() { DeleteAllRefsFromMe(); }

	void			DeleteThis();

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) return pc->extend_cd->ClassName(); else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (Mtl*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// From MtlBase

	TSTR			GetFullName() { return delegate->GetFullName(); }
	int				BuildMaps(TimeValue t, RenderMapsContext &rmc) { return delegate->BuildMaps(t, rmc); }
	ULONG			Requirements(int subMtlNum) { return delegate->Requirements(subMtlNum); }
	ULONG			LocalRequirements(int subMtlNum) { return delegate->LocalRequirements(subMtlNum); }
	BOOL			IsMultiMtl() { return delegate->IsMultiMtl(); }
	int				NumSubTexmaps();
	Texmap*			GetSubTexmap(int i);
//	int				MapSlotType(int i) { return delegate->MapSlotType(i); }
	void			SetSubTexmap(int i, Texmap *m);
//	int				SubTexmapOn(int i) { return delegate->SubTexmapOn(i); }
//	void			DeactivateMapsInTree() { delegate->DeactivateMapsInTree(); }     
	TSTR			GetSubTexmapSlotName(int i);
//	TSTR			GetSubTexmapTVName(int i) { return delegate->GetSubTexmapTVName(i); }
//	void			CopySubTexmap(HWND hwnd, int ifrom, int ito) { delegate->CopySubTexmap(hwnd, ifrom, ito); }     	
	
	void			Update(TimeValue t, Interval& valid) { delegate->Update(t, valid); }
	void			Reset() { delegate->Reset(); pc->cd2->Reset(this, TRUE); }
	Interval		Validity(TimeValue t);
	ParamDlg*		CreateParamDlg(HWND hwMtlEdit, IMtlParams* imp);
	IOResult		Save(ISave *isave) { return MSPlugin::Save(isave); } // return delegate->Save(isave); }
    IOResult		Load(ILoad *iload) { return MSPlugin::Load(iload); } // return delegate->Load(iload); }
	ULONG			GetGBufID() { return delegate->GetGBufID(); }
	void			SetGBufID(ULONG id) { delegate->SetGBufID(id); }
	void			EnumAuxFiles(NameEnumCallback& nameEnum, DWORD flags) { delegate->EnumAuxFiles(nameEnum, flags); }   
	PStamp*			GetPStamp(int sz) { return delegate->GetPStamp(sz); }
	PStamp*			CreatePStamp(int sz) { return delegate->CreatePStamp(sz); }   		
	void			DiscardPStamp(int sz) { delegate->DiscardPStamp(sz); }      		

	// From Mtl

	BOOL			DontKeepOldMtl() { return TRUE; }
	Texmap*			GetActiveTexmap() { return delegate->GetActiveTexmap(); } 
	void			SetActiveTexmap( Texmap *txm) { delegate->SetActiveTexmap(txm); } 
//	void			RefDeleted() { delegate->RefDeleted(); } 
//	void			RefAdded(RefMakerHandle rm) { delegate->RefAdded(rm); }  
	Color			GetAmbient(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetAmbient(mtlNum, backFace); }
	Color			GetDiffuse(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetDiffuse(mtlNum, backFace); }	    
	Color			GetSpecular(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetSpecular(mtlNum, backFace); }
	float			GetShininess(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetShininess(mtlNum=0, backFace); }
	float			GetShinStr(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetShinStr(mtlNum=0, backFace); }		
	float			GetXParency(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetXParency(mtlNum=0, backFace); }
	BOOL			GetSelfIllumColorOn(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetSelfIllumColorOn(mtlNum, backFace); } 
	float			GetSelfIllum(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetSelfIllum(mtlNum, backFace); } 
	Color			GetSelfIllumColor(int mtlNum=0, BOOL backFace=FALSE) { return delegate->GetSelfIllumColor(mtlNum, backFace); } 
	float			WireSize(int mtlNum=0, BOOL backFace=FALSE) { return delegate->WireSize(mtlNum, backFace); } 
	void			SetAmbient(Color c, TimeValue t) { delegate->SetAmbient(c, t); }		
	void			SetDiffuse(Color c, TimeValue t) { delegate->SetDiffuse(c, t); }		
	void			SetSpecular(Color c, TimeValue t) { delegate->SetSpecular(c, t); }
	void			SetShininess(float v, TimeValue t) { delegate->SetShininess(v, t); }	
	void			Shade(ShadeContext& sc) { delegate->Shade(sc); }
	int				NumSubMtls();
	Mtl*			GetSubMtl(int i);
	void			SetSubMtl(int i, Mtl *m);
	TSTR			GetSubMtlSlotName(int i);
	TSTR			GetSubMtlTVName(int i) { return GetSubMtlSlotName(i); } 					  
//	void			CopySubMtl(HWND hwnd, int ifrom, int ito) { delegate->CopySubMtl(hwnd, ifrom, ito); }  
	float			GetDynamicsProperty(TimeValue t, int mtlNum, int propID) { return delegate->GetDynamicsProperty(t, mtlNum, propID); } 
	void			SetDynamicsProperty(TimeValue t, int mtlNum, int propID, float value) { delegate->SetDynamicsProperty(t, mtlNum, propID, value); } 
	float			EvalDisplacement(ShadeContext& sc) { return delegate->EvalDisplacement(sc); } 
	Interval		DisplacementValidity(TimeValue t) { return delegate->DisplacementValidity(t); } 
};

/* ------------- ParamDlg class for scripter material/texmap plug-ins --------------- */

class MSAutoMParamDlg : public IAutoMParamDlg 
{
	public:
		Tab<ParamDlg*>	secondaryDlgs;	// secondary ParamDlgs if this is a master
		ParamDlg*		delegateDlg;	// my delegate's dialog if any
		MSPlugin*		plugin;			// target plugin
		Rollout*		ro;				// rollout controlling dialog
		MtlBase*		mtl;			// material in the dialog
		IMtlParams*		ip;				// mtl interface
		TexDADMgr		texDadMgr;
		MtlDADMgr		mtlDadMgr;
		HWND			hwmedit;		// medit window

					MSAutoMParamDlg(HWND hMedit, IMtlParams *i, MtlBase* mtl, MSPlugin* plugin, Rollout* ro);
				   ~MSAutoMParamDlg();

		// from ParamDlg
		Class_ID	ClassID() { return mtl->ClassID(); }
		ReferenceTarget* GetThing() { return mtl; }
		void		SetThing(ReferenceTarget *m);
		void		DeleteThis();
		void		SetTime(TimeValue t);
		void		ReloadDialog();
		void		ActivateDlg(BOOL onOff);
		int			FindSubTexFromHWND(HWND hw);	
		int			FindSubMtlFromHWND(HWND hw);	

		void		InvalidateUI() { ReloadDialog(); }
		void		MtlChanged() { ip->MtlChanged(); }
		// secondary dialog list management
		int			NumDlgs() { return secondaryDlgs.Count(); }
		void		AddDlg(ParamDlg* dlg);
		ParamDlg*	GetDlg(int i);
		void		SetDlg(int i, ParamDlg* dlg);
		void		DeleteDlg(ParamDlg* dlg);

		// access to this dlg's parammap stuff
		IParamMap2* GetMap() { return NULL; }
};

// RK: Start
// ----------------------- MSPluginSpecialFX ----------------------

//	template for MSPlugin classes derived from SpecialFX
template <class TYPE>
class MSPluginSpecialFX : public TYPE, public MSPlugin
{
public:
	IRendParams		*ip;

	void			DeleteThis();
	
	// From MSPlugin
	HWND			AddRollupPage(HINSTANCE hInst, TCHAR *dlgTemplate, DLGPROC dlgProc, TCHAR *title, LPARAM param=0,DWORD flags=0);
	void			DeleteRollupPage(HWND hRollup);
	IRollupWindow*  GetRollupWindow();
	ReferenceTarget* get_delegate() { return NULL; }  // no delegate

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { } 		
	int				NumSubs() { return pblocks.Count(); }  
	Animatable*		SubAnim(int i) { return pblocks[i]; }
	TSTR			SubAnimName(int i) { return pblocks[i]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }

	// From ReferenceMaker
	RefResult		NotifyRefChanged( Interval changeInt,RefTargetHandle hTarget, PartID& partID, RefMessage message)
					{ 
						return ((MSPlugin*)this)->NotifyRefChanged(changeInt, hTarget, partID, message); 
					}

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count(); }
	RefTargetHandle GetReference(int i) { return pblocks[i]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i >= pblocks.Count()) pblocks.SetCount(i+1); pblocks[i] = (IParamBlock2*)rtarg; 
					}
	void			RefDeleted() { MSPlugin::RefDeleted(); }
	IOResult		Save(ISave *isave) { MSPlugin::Save(isave); return SpecialFX::Save(isave); }
    IOResult		Load(ILoad *iload) { MSPlugin::Load(iload); return SpecialFX::Load(iload); }

	// From SpecialFX
	TSTR			GetName() { return pc->class_name->to_string(); }
	BOOL			Active(TimeValue t) { return SpecialFX::Active(t); }
	void			Update(TimeValue t, Interval& valid) { SpecialFX::Update(t, valid); }
	int				NumGizmos() { return SpecialFX::NumGizmos(); }
	INode*			GetGizmo(int i) { return SpecialFX::GetGizmo(i); }
	void			DeleteGizmo(int i) { SpecialFX::DeleteGizmo(i); }
	void			AppendGizmo(INode *node) { SpecialFX::AppendGizmo(node); }
	BOOL			OKGizmo(INode *node) { return SpecialFX::OKGizmo(node); } 
	void			EditGizmo(INode *node) { SpecialFX::EditGizmo(node); } 
};

//	template for MSPlugin Xtnd classes derived from SpecialFX
template <class TYPE, class MS_SUPER>
class MSSpecialFXXtnd : public MS_SUPER
{
public:
	TYPE*			delegate;		// my delegate

	void			DeleteThis();

	// From MSPlugin
	ReferenceTarget* get_delegate() { return delegate; } 

	// From Animatable
	void			GetClassName(TSTR& s) { s = TSTR(pc->class_name->to_string()); }  
	Class_ID		ClassID() { return pc->class_id; }
	void			FreeCaches() { delegate->FreeCaches(); } 		
	int				NumSubs() { return pblocks.Count() + 1; }  
	Animatable*		SubAnim(int i) { if (i == 0) { return delegate; } else return pblocks[i-1]; }
	TSTR			SubAnimName(int i) { if (i == 0) { TSTR n; delegate->GetClassName(n); return n; } else return pblocks[i-1]->GetLocalName(); }
	int				NumParamBlocks() { return pblocks.Count(); }
	IParamBlock2*	GetParamBlock(int i) { return pblocks[i]; }
	IParamBlock2*	GetParamBlockByID(BlockID id) { return MSPlugin::GetParamBlockByID(id); }
	void*			GetInterface(ULONG id) { if (id == I_MAXSCRIPTPLUGIN) return (MSPlugin*)this; else return NULL; }
	
	// From ReferenceMaker
//	RefResult		NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID, RefMessage message) { return REF_SUCCEED; }

	// From ReferenceTarget
	int				NumRefs() { return pblocks.Count() + 1; }
	RefTargetHandle GetReference(int i) { if (i == 0) return delegate; else return pblocks[i-1]; }
	void			SetReference(int i, RefTargetHandle rtarg) 
					{ 
						if (i == 0) 
						{
							if (rtarg == NULL)
								delegate->FreeCaches();
							delegate = (TYPE*)rtarg; 
						}
						else { if (i - 1 >= pblocks.Count()) pblocks.SetCount(i); pblocks[i-1] = (IParamBlock2*)rtarg; }
					}

	// From SpecialFX
	BOOL			Active(TimeValue t) { return delegate->Active(t); }
	void			Update(TimeValue t, Interval& valid) { delegate->Update(t, valid); }
	int				NumGizmos() { return delegate->NumGizmos(); }
	INode*			GetGizmo(int i) { return delegate->GetGizmo(i); }
	void			DeleteGizmo(int i) { delegate->DeleteGizmo(i); }
	void			AppendGizmo(INode *node) { delegate->AppendGizmo(node); }
	BOOL			OKGizmo(INode *node) { return delegate->OKGizmo(node); } 
	void			EditGizmo(INode *node) { delegate->EditGizmo(node); } 
};

// ----------------------- MSPluginEffect ----------------------
// scripted Effect

class MSPluginEffect : public MSPluginSpecialFX<Effect>
{
public:
	MSAutoEParamDlg* masterFXDlg;						// master dialog containing all scripted rollout

					MSPluginEffect() { }
					MSPluginEffect(MSPluginClass* pc, BOOL loading);
				   ~MSPluginEffect() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// from Effect
	EffectParamDlg*	CreateParamDialog(IRendParams* imp);
	DWORD			GBufferChannelsRequired(TimeValue t);
	void			Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *cb );

	Effect*			to_effect() { return this; }
};

class MSEffectXtnd : public MSSpecialFXXtnd<Effect, MSPluginEffect>
{
public:
					MSEffectXtnd(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// from Effect
	EffectParamDlg*	CreateParamDialog(IRendParams* imp);
	DWORD			GBufferChannelsRequired(TimeValue t);
	void			Apply(TimeValue t, Bitmap *bm, RenderGlobalContext *gc, CheckAbortCallback *cb );

};

/* ------------- ParamDlg class for scripter effect plug-ins --------------- */

class MSAutoEParamDlg : public IAutoEParamDlg 
{
	public:
		Tab<SFXParamDlg*> secondaryDlgs; // secondary ParamDlgs if this is a master
		SFXParamDlg*	delegateDlg;	// my delegate's dialog if any
		MSPlugin*		plugin;			// target plugin
		Rollout*		ro;				// rollout controlling dialog
		SpecialFX*		sfx;			// effect/atmos in the dialog
		IRendParams*	ip;				// render interface

					MSAutoEParamDlg(IRendParams *i, SpecialFX* fx, MSPlugin* plugin, Rollout* ro);
				   ~MSAutoEParamDlg();

		// from ParamDlg
		Class_ID	ClassID() { return sfx->ClassID(); }
		ReferenceTarget* GetThing() { return sfx; }
		void		SetThing(ReferenceTarget *fx);
		void		DeleteThis();
		void		SetTime(TimeValue t);

		void		InvalidateUI(); 
		// secondary dialog list management
		int			NumDlgs() { return secondaryDlgs.Count(); }
		void		AddDlg(SFXParamDlg* dlg);
		SFXParamDlg* GetDlg(int i);
		void		SetDlg(int i, SFXParamDlg* dlg);
		void		DeleteDlg(SFXParamDlg* dlg);

		// access to this dlg's parammap stuff
		IParamMap2* GetMap() { return NULL; }
};

// RK: End 

// ----------------------- MSPluginAtmos ----------------------
// scripted Atmospheric

class MSPluginAtmos : public MSPluginSpecialFX<Atmospheric>
{
public:
	MSAutoEParamDlg* masterFXDlg;						// master dialog containing all scripted rollout

					MSPluginAtmos() { }
					MSPluginAtmos(MSPluginClass* pc, BOOL loading);
				   ~MSPluginAtmos() { DeleteAllRefsFromMe(); }

	static RefTargetHandle create(MSPluginClass* pc, BOOL loading);
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// from Atmospheric
	AtmosParamDlg *CreateParamDialog(IRendParams *ip);
	BOOL SetDlgThing(AtmosParamDlg* dlg);
	void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG=FALSE) { };

	Atmospheric* to_atmospheric() { return this; }
};

class MSAtmosXtnd : public MSSpecialFXXtnd<Atmospheric, MSPluginAtmos>
{
public:
					MSAtmosXtnd(MSPluginClass* pc, BOOL loading);
				   ~MSAtmosXtnd() { DeleteAllRefsFromMe(); }
	RefTargetHandle Clone(RemapDir& remap = NoRemap());

	// from Atmospheric
	AtmosParamDlg *CreateParamDialog(IRendParams *ip);
	BOOL SetDlgThing(AtmosParamDlg* dlg);
	void Shade(ShadeContext& sc,const Point3& p0,const Point3& p1,Color& color, Color& trans, BOOL isBG=FALSE) 
		{ delegate->Shade(sc, p0, p1, color, trans, isBG); }

};

// RK: End 
#endif
