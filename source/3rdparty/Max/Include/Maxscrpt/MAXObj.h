/*	
 *		MAXObject.h - MAX object wrapper classes in MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_MAXOBJECT
#define _H_MAXOBJECT

#include "Max.h"
#include "SimpObj.h"
#include "Pathname.h"
#include "istdplug.h"
#include "tvnode.h"

extern ScripterExport Interface* MAXScript_interface;
class MAXControl;

#define MESH_READ_ACCESS	0  // mesh access modes
#define MESH_WRITE_ACCESS	1
#define MESH_BASE_OBJ		2

/* --------------------- base wrapper class ------------------------ */

/* subclasses of MAXWrapper act as MAXScript-side proxies for MAX-side objects,
 * such as nodes, modifiers, materials, etc.  The prime purpose of MAXWrapper is
 * to maintain MAX referenced to the MAX-side objects it wraps & process delete notify messages
 *
 * subclasses should all do a MakeRefByID for each ref on creation & check for 
 * any ref deletion on the MAX side by using the check_for_deletion() macro.
 */

extern ScripterExport TCHAR* get_deleted_obj_err_message();

#define check_for_deletion()  if (ref_deleted) throw RuntimeError (get_deleted_obj_err_message(), class_name());
#define deletion_check(val)  if ((val)->ref_deleted) throw RuntimeError (get_deleted_obj_err_message(), (val)->class_name());

visible_class (MAXWrapper)

class MAXWrapper : public Value, public ReferenceTarget // ReferenceMaker
{
public:
	Tab<RefTargetHandle> refs;		/* reference array		*/
	short		ref_deleted;		 

	ScripterExport			MAXWrapper();
	ScripterExport			~MAXWrapper();
	
	ScripterExport Value*		classOf_vf(Value** arg_list, int count);
	ScripterExport Value*		superClassOf_vf(Value** arg_list, int count);
	ScripterExport Value*		isKindOf_vf(Value** arg_list, int count);
	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXWrapper)) ? 1 : Value::is_kind_of(c); }
	void		collect() { delete this; }
	virtual	TCHAR* class_name() = 0;
	virtual ReferenceTarget* get_max_object() { return GetReference(0); }
	ScripterExport Value*		copy_no_undo(Value** arg_list, int count);

	ScripterExport void make_ref(int ref_no, ReferenceTarget* ref);
	void		drop_MAX_refs() { DeleteAllRefsFromMe(); }

    // Animatable
	Class_ID ClassID() { return Class_ID(MAXSCRIPT_WRAPPER_CLASS_ID, 0); }
	SClass_ID SuperClassID() { return MAXSCRIPT_WRAPPER_CLASS_ID; }		

	// ReferenceMaker
	int			NumRefs() { return refs.Count(); }
	RefTargetHandle GetReference(int i) { return refs[i]; }
	ScripterExport void		SetReference(int i, RefTargetHandle rtarg);
	ScripterExport RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);

	// generic property accessors (operate on ref(0))
	ScripterExport Value*		get_property(Value** arg_list, int count);
	ScripterExport Value*		set_property(Value** arg_list, int count);

	// direct property access, when no wrapper exists, but we have the MAX-side ref
	static ScripterExport Value* get_property(ReferenceTarget* ref, Value* prop, Value* owner);
	static ScripterExport Value* set_property(ReferenceTarget* ref, Value* prop, Value* val);

//#include "defimpfn.h"
// RK: 06/01/99 Redefining these macros with ScripterExport directive so that
//				.dlx plugins can have classes derived from MAXWrapper
// RK: Start
#ifdef def_generic
#	undef def_generic
#	undef def_property
#	undef def_2_prop_path
#	undef def_nested_prop
#endif
#ifdef def_prop_getter
#	undef def_prop_getter
#	undef def_prop_setter
#endif

#define def_generic(fn, name)	\
			ScripterExport Value* fn##_vf(Value** arglist, int arg_count)

#define def_property(p)										\
			ScripterExport Value*	get_##p(Value** arg_list, int count);	\
			ScripterExport Value*	set_##p(Value** arg_list, int count)
#define def_prop_getter(p)									\
			ScripterExport Value*	get_##p(Value** arg_list, int count)
#define def_prop_setter(p)									\
			ScripterExport Value*	set_##p(Value** arg_list, int count)
#define def_2_prop_path(p1, p2)										\
			ScripterExport Value*	get_##p1##_##p2(Value** arg_list, int count);	\
			ScripterExport Value*	set_##p1##_##p2(Value** arg_list, int count)
#define def_nested_prop(p1)											\
			ScripterExport Value*	get_nested_##p1(Value** arg_list, int count);	\
			ScripterExport Value*	set_nested_##p1(Value** arg_list, int count)
// RK: End
			def_generic (numrefs, "numRefs");	// peek & poke for MAXScript...
			def_generic (getref, "getRef");
			def_generic (setref, "getRef");
			def_generic (numsubs, "numSubs");
			def_generic (getsub, "getSub");
			def_generic (setsub, "getSub");

			def_generic (show_props, "showProperties");
			def_generic (get_props,  "getPropNames");
			def_generic (copy,		 "copy");
			def_generic (isDeleted,	 "isDeleted");
			def_generic (exprForMAXObject,	"exprForMAXObject");
//			def_generic (dependsOn,	 "dependsOn");

			// the standard transform subanim properties
			def_property	( pos );
			def_2_prop_path	( pos, controller );
			def_2_prop_path	( pos, isAnimated );
			def_2_prop_path	( pos, keys );
			def_2_prop_path	( pos, track );
			def_2_prop_path	( pos, x );
			def_2_prop_path	( pos, y );
			def_2_prop_path	( pos, z );
			def_property	( rotation );
			def_2_prop_path	( rotation, angle );
			def_2_prop_path	( rotation, x_rotation );
			def_2_prop_path	( rotation, y_rotation );
			def_2_prop_path	( rotation, z_rotation );
			def_2_prop_path	( rotation, axis );
			def_2_prop_path	( rotation, controller );
			def_2_prop_path	( rotation, isAnimated );
			def_2_prop_path	( rotation, keys );
			def_2_prop_path	( rotation, track );
			def_property	( scale );
			def_2_prop_path	( scale, controller );
			def_2_prop_path	( scale, isAnimated );
			def_2_prop_path	( scale, keys );
			def_2_prop_path	( scale, track );
			def_2_prop_path	( scale, x );
			def_2_prop_path	( scale, y );
			def_2_prop_path	( scale, z );
			def_property	( controller );
			def_property	( transform );
			def_property	( isAnimated );
			def_property	( numsubs );

			def_nested_prop	( angle );
			def_nested_prop	( x_rotation );
			def_nested_prop	( y_rotation );
			def_nested_prop	( z_rotation );
			def_nested_prop	( axis );
			def_nested_prop	( controller );
			def_nested_prop	( isAnimated );
			def_nested_prop	( keys );
			def_nested_prop	( x );
			def_nested_prop	( y );
			def_nested_prop	( z );

	// utility methods for the above subanim property accessors, implemented by 
	// those that have standard transform subsanims
	virtual Control* get_max_pos_controller(ParamDimension** pdim) { return NULL; }
	virtual Control* get_max_scale_controller(ParamDimension** pdim) { return NULL; }
	virtual Control* get_max_rotation_controller(ParamDimension** pdim) { return NULL; }
	virtual Control* get_max_tm_controller(ParamDimension** pdim) { return NULL; }
	virtual Control* get_max_controller(ParamDimension** pdim) { return NULL; }
	ScripterExport virtual Control* get_max_prop_controller(Value* prop, ParamDimension** pdim);
	virtual BOOL	 set_max_pos_controller(MAXControl* c) { return FALSE; }
	virtual BOOL	 set_max_scale_controller(MAXControl* c) { return FALSE; }
	virtual BOOL	 set_max_rotation_controller(MAXControl* c) { return FALSE; }
	virtual BOOL	 set_max_tm_controller(MAXControl* c) { return FALSE; }
	virtual BOOL	 set_max_controller(MAXControl* c) { return FALSE; }
	ScripterExport virtual BOOL	 set_max_prop_controller(Value* prop, MAXControl* c);

	static ScripterExport Control* get_max_prop_controller(ReferenceTarget* ref, Value* prop, ParamDimension** pdim);
	static ScripterExport BOOL	   set_max_prop_controller(ReferenceTarget* ref, Value* prop, MAXControl* c);

	ScripterExport Value*		get_container_property(Value* prop, Value* cur_prop);
	ScripterExport Value*		set_container_property(Value* prop, Value* val, Value* cur_prop);

	// coordsystem mappers, default is no mapping
	virtual void	object_to_current_coordsys(Point3& p, int mode=0) { }
	virtual void	object_from_current_coordsys(Point3& p, int mode=0) { }
	virtual void	world_to_current_coordsys(Point3& p, int mode=0) { }
	virtual void	world_from_current_coordsys(Point3& p, int mode=0) { }
	virtual void	world_to_current_coordsys(Quat& q) { }
	virtual void	world_from_current_coordsys(Quat& q) { }

	// map to & from controller value coordsys, eg, parent for nodes, modcontext for modifiers, etc.
	virtual void	ctrl_to_current_coordsys(Point3& p, int mode=0) { }
	virtual void	ctrl_from_current_coordsys(Point3& p, int mode=0) { }
	virtual void	ctrl_to_current_coordsys(Quat& q) { }
	virtual void	ctrl_from_current_coordsys(Quat& q) { }
	virtual void	ctrl_to_current_coordsys(ScaleValue& s) { }
	virtual void	ctrl_from_current_coordsys(ScaleValue& s) { }

	virtual Matrix3& local_tm() { return idTM; }
	virtual Matrix3  local_tm_inv() { return Inverse(local_tm()); }

	// recursive time functions
	#undef def_time_fn
	#define def_time_fn(_fn) ScripterExport Value* _fn##_vf(Value** arglist, int arg_count)
	#include "time_fns.h"			// once for the generics
	#undef def_time_fn
	#define def_time_fn(_fn) ScripterExport Value* _fn(Animatable* anim, Value** arglist, int arg_count)
	#include "time_fns.h"			// and once again for the factored animatable form

	// mesh & mesh-sub-object access setup
	ScripterExport virtual Mesh* set_up_mesh_access(int access, ReferenceTarget** owner = NULL);
	ScripterExport virtual Mesh* set_up_mesh_face_access(int index, int access, ReferenceTarget** owner = NULL);
	ScripterExport virtual Mesh* set_up_mesh_vertex_access(int index, int access, ReferenceTarget** owner = NULL);
	ScripterExport virtual BitArray* get_vertsel();
	ScripterExport virtual BitArray* get_facesel();
	ScripterExport virtual BitArray* get_edgesel();
	ScripterExport virtual GenericNamedSelSetList& get_named_vertsel_set();
	ScripterExport virtual GenericNamedSelSetList& get_named_facesel_set();
	ScripterExport virtual GenericNamedSelSetList& get_named_edgesel_set();
	virtual void update_sel() { NotifyDependents(FOREVER, PART_SELECT, REFMSG_CHANGE); }

	// R2.5 stuff

	def_property   ( category );
	def_prop_getter( classID );

	def_generic ( get,				 "get" );				    // <mw>[i] => subAnim[i]
	def_generic ( getSubAnim,	 	 "getSubAnim" );			// equivalent 
	def_generic ( getSubAnimName,	 "getSubAnimName" );
	def_generic ( getSubAnimNames,	 "getSubAnimNames" );
	def_generic ( getAppData,		 "getAppData");
	def_generic ( setAppData,		 "setAppData");
	def_generic ( deleteAppData,	 "deleteAppData");
	def_generic ( clearAllAppData,	 "clearAllAppData");
	def_generic ( addPluginRollouts, "addPluginRollouts");

	ReferenceTarget* to_reftarg() { return get_max_object(); }

	// scene I/O 
	ScripterExport IOResult Save(ISave* isave);
	static ScripterExport Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

// coordsys mappers defines..
#define NO_TRANSLATE	0x0001
#define NO_SCALE		0x0002

#define ctrl_to_current_coordsys_rotate(p)			ctrl_to_current_coordsys(p, NO_TRANSLATE + NO_SCALE)
#define ctrl_from_current_coordsys_rotate(p)		ctrl_from_current_coordsys(p, NO_TRANSLATE + NO_SCALE)
#define object_to_current_coordsys_rotate(p)		object_to_current_coordsys(p, NO_TRANSLATE + NO_SCALE)
#define object_from_current_coordsys_rotate(p)		object_from_current_coordsys(p, NO_TRANSLATE + NO_SCALE)
#define world_to_current_coordsys_rotate(p)			world_to_current_coordsys(p, NO_TRANSLATE + NO_SCALE)
#define world_from_current_coordsys_rotate(p)		world_from_current_coordsys(p, NO_TRANSLATE + NO_SCALE)
#define object_to_current_coordsys_scaleRotate(p)	object_to_current_coordsys(p, NO_TRANSLATE)
#define object_from_current_coordsys_scaleRotate(p)	object_from_current_coordsys(p, NO_TRANSLATE)

/* ---------------------- MAXClass ----------------------- */
 
/* this is the class that provides a runtime representation for
 * the classes of MAX objects.  It is essentially the MAXScript 
 * equivalent of the ClassDesc instance with extra metadata
 * about creation parameters, ec. */

typedef struct			// parameter descriptor struct
{
	Value*		name;			// parameter name
	TCHAR		desc_type;		// descriptor type... paramblock/fn/etc.
	TCHAR		flags;
	union
	{
		struct					// paramblock entry descriptor
		{
			int		parm_id;	
		};
		struct					// fn entry descriptor
		{
			max_getter_cf getter;
			max_setter_cf setter;
		};
		struct					// subanim entry descriptor
		{
			int		subanim_num;	
		};
		struct					// paramblockn - paramblock at given refno
		{
			int		refno;
			int		pbn_id;
		};
	};
	ParamType	type;			// common type code & initval
	union
	{
		float		fval;
		int			ival;
		BOOL		bval;
		struct		{float x, y, z;};
		struct		{float r, g, b;};
		struct		{float h, s, v;};
		struct		{int btn_min, btn_max, btn_val;};
	} init_val;
} parm_desc;

#define PD_NO_INIT			0x01	// flags this property as reqiring no init
#define PD_LOCAL_DUP		0x02	// this is a duplicate local translation
#define PD_HAS_LOCAL_DUPS	0x04	// this has duplicate local translations
#define PD_SHOWN			0x08	// temp flag to mark already shown props in showProps()

class MAXClass;
class MAXSuperClass;

typedef struct					// entries in the superclass table
{
	SClass_ID		key;
	MAXSuperClass*	mx_superclass;
} superclass_table;

typedef Value* (*maker_fn)(MAXClass* cls, ReferenceTarget* obj, Value** arglist, int count);

visible_class (MAXSuperClass)

class MAXSuperClass : public Value
{
public:
	Value*		name;
	SClass_ID	sclass_id;
	Value*		superclass;
	maker_fn	maker;
	short		n_parms;
	short		flags;
	parm_desc*	parms;

				MAXSuperClass(TCHAR* cname, SClass_ID sid, Value* superClass, maker_fn maker, ...);
			   ~MAXSuperClass();
	void		complete_init();

	static superclass_table* superclasses;		// lookup table of all superclasses by SClass_ID
	static short n_superclasses;
	static BOOL superclass_table_dirty;
	static MAXSuperClass* lookup_superclass(SClass_ID sid);

	Value*		classOf_vf(Value** arg_list, int count);
	Value*		superClassOf_vf(Value** arg_list, int count);
	Value*		isKindOf_vf(Value** arg_list, int count);
	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXWrapper)) ? 1 : Value::is_kind_of(c); }
	void		collect() { delete this; }
	void		gc_trace();
	ScripterExport void sprin1(CharStream* s);
	void		export_to_scripter();

	void		apply_keyword_parms(ReferenceTarget* obj, Value** key_arg_list, int count);

				def_generic (get_props, "getPropNames");

	def_property( categories );
};

#define SC_SUPERCLASS_INITED	0x0001

typedef struct
{
	Class_ID	cid;
	SClass_ID	scid;
} class_key;

typedef struct				// entries in the class table
{
	class_key	key;
	MAXClass*	mx_class;
} class_table;

enum metadata_flags
{
	md_use_getref  = 0x0001,		/* class flags */
	md_use_getref0 = 0x0002,
	md_use_getref1 = 0x0004,
	md_no_create   = 0x0008,
	md_no_access   = 0x0010,
	md_direct_index = 0x0020,
	md_new_plugin  = 0x0040,
	md_auto_parms  = 0x0080,
	md_name_clash  = 0x0100,
};

enum metadata_flag
{
//	end			= 0,				/* metadata vararg tags */
	getters     = 1,
	setters,
	accessors,
	path_getters,
	path_setters,
	path_accessors,
	fns,
	paramblock,
	subanims,
	paramblockn,
	subanimparamblock
};

enum
{
	TYPE_POINT3_ANGLE	= TYPE_USER + 128,		/* MAXScript special paramblock types... */
	TYPE_POINT3_PCNT,

};


visible_class (MAXClass)

class MAXClass : public Value
{
public:
	Value*			name;
	Class_ID		class_id;
	SClass_ID		sclass_id;
	ClassDesc2*		cd2;				// for now, pointer to ClassDesc2 if this class is PB2-based
	short			md_flags;
	int				n_parms;
	parm_desc*		parms;
	MAXSuperClass*	superclass;
	short			paramblock_ref_no;
	Value*			category;

	static class_table*	classes;		// lookup table of all classes by Class_ID
	static short	n_classes;
	static BOOL		class_table_dirty;
	static MAXClass* lookup_class(Class_ID* cid, SClass_ID scid);

								   MAXClass() { }
					ScripterExport MAXClass(TCHAR* cname, Class_ID cid, SClass_ID sid, MAXSuperClass* sclass, short cflags, ...);
			        ScripterExport ~MAXClass();
	ClassDesc*		complete_init();

	static void		setup();
	Value*			classOf_vf(Value** arg_list, int count);
	Value*			superClassOf_vf(Value** arg_list, int count);
	Value*			isKindOf_vf(Value** arg_list, int count);
	BOOL			is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXWrapper)) ? 1 : Value::is_kind_of(c); }
	void			collect() { delete this; }
	void			gc_trace();
	ScripterExport void sprin1(CharStream* s);
	void			export_to_scripter();

	ScripterExport Value* apply(Value** arglist, int count);		// object creation by applying class object

	static ScripterExport Value* make_wrapper_for(ReferenceTarget* ref);
	static ScripterExport Value* make_node_for(ReferenceTarget* ref);

	parm_desc*		get_parm_desc(ReferenceTarget* ref, Value* key);
	BOOL			build_parm_descs(ReferenceTarget* ref);
	void			apply_keyword_parms(ReferenceTarget* ref, Value** key_arg_list, int count);
	void			initialize_object(ReferenceTarget* ref);
	Value*			get_max_property(ReferenceTarget* ref, parm_desc* pd, TimeValue t, Interval& valid);
	void			set_max_property(ReferenceTarget* ref, parm_desc* pd, TimeValue t, Value* val);

	// PB2-related
	ParamDef*		get_parm_def(Value* prop, ParamBlockDesc2*& pbd, int& tabIndex, ReferenceTarget* ref = NULL);
	void			set_max_property(ReferenceTarget* ref, ParamDef* pd, int tabIndex, ParamBlockDesc2* pbd, TimeValue t, Value* val);
	Value*			get_max_property(ReferenceTarget* ref, ParamDef* pd, int tabIndex, ParamBlockDesc2* pbd, TimeValue t, Interval& valid);
	static Value*	get_pb2_property(IParamBlock2* pb, ParamDef* pd, int tabIndex, TimeValue t, Interval& valid);
	static void		set_pb2_property(IParamBlock2* pb, ParamDef* pd, int tabIndex, TimeValue t, Value* val);

					def_generic (get_props,		   "getPropNames");
					def_generic (create_instance,  "createInstance");

	Class_ID		get_max_class_id() { return class_id; }

	def_property   ( category );
	def_prop_getter( classID );

	Value*			Category();
};

/* MAX object makers... */

Value* make_max_node(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_object(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_light(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_camera(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_modifier(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_material(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_texturemap(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_mtlbase(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_system(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_helper(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_controller(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_atmospheric(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);
Value* make_max_effect(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count); // RK: Added this
Value* make_max_filter(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count); // RK: Added this
Value* make_max_shadow(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count); // LE: Added this
Value* no_max_maker(MAXClass* cls, ReferenceTarget* obj, Value** arg_list, int count);

/* ------------------  MAXWrapper instance cache -----------------*/
// MAXWrapper instance cache - it's critical we don't wind up generating a million
// MAXWrappers with References to the same MAX-side object as this KILLS the collector
// during the DeleteAllRefsFromMe() which seems to do a linear search through its refs

#define MAXWRAPPER_CACHE_SIZE 2048			// must be power of 2
extern ScripterExport MAXWrapper* maxwrapper_cache[];

inline BOOL
_maxwrapper_cache_get(long index, MAXWrapper*** pw)
{
    // compute cache index - fold halfwords & fold to cache size, get cache entry
	index = (LOWORD(index) ^ HIWORD(index)) % MAXWRAPPER_CACHE_SIZE - 1;
	*pw = &maxwrapper_cache[abs(index)];
	// we match initially if entry contains a non-deleted MAXWrapper
	MAXWrapper* w = **pw;
	return (w && !w->ref_deleted);
}

// a macro for implementing MAXWrapper interning member functions
#define maxwrapper_cache_get(_mw_class, _mw_ref, _mw_var, _ce_var)									\
	(_maxwrapper_cache_get((long)(_mw_ref) ^ (long)class_tag(_mw_class), (MAXWrapper***)&_ce_var) &&  \
     (_mw_var = *_ce_var)->tag == class_tag(_mw_class))

/* ------------------------ MAXNode class  ------------------------ */

/* this class is the MAXScript wrapper for object hierarchy 
 * INodes. */

visible_class (MAXNode)

class MAXNode : public MAXWrapper
{
public:
	INode	*node;
	
	ScripterExport MAXNode(INode* init_node);
	static ScripterExport Value* intern(INode* init_node);

	Value*		classOf_vf(Value** arg_list, int count);
	Value*		superClassOf_vf(Value** arg_list, int count);
	Value*		isKindOf_vf(Value** arg_list, int count);
	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXNode)) ? 1 : MAXWrapper::is_kind_of(c); }
#	define is_node(v) ((v)->tag == class_tag(MAXNode))
	void		collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
	TCHAR*		class_name();
	ScripterExport ReferenceTarget* get_max_object();

	/* include all the protocol declarations */

#include "defimpfn.h"
#	include "nodepro.h"

	def_generic( distance,  "distance" );  // shortcut distance fn for nodes
	def_generic( eq,		"=");
	def_generic( ne,		"!=");
	def_generic( coerce,	"coerce");
	
	/* declare built-in property accessors */

	def_property	( dir );
	def_property	( target );
	def_property	( mat );
	def_property	( modifiers );
	def_property	( name );
	def_property	( max );
	def_property	( min );
	def_property	( center );
	def_property	( transform );
	def_property	( parent );
	def_property	( children );
	def_property	( pivot );
	def_2_prop_path	( pivot, x );
	def_2_prop_path	( pivot, y );
	def_2_prop_path	( pivot, z );
	def_property	( objectoffsetpos );
	def_property	( objectoffsetrot );
	def_property	( objectoffsetscale );
	def_property	( objecttransform );

	def_nested_prop	( angle );
	def_nested_prop	( axis );

	/* mesh props */

	def_property	( numverts );
	def_property	( numtverts );
	def_property	( numfaces );
	def_property	( numcpvverts );
	def_property	( mesh );
	/* LE added these */
	def_property	( displacementMapping );
	def_property	( subdivisionDisplacement );
	def_property	( splitMesh );
	/* end LE */

	/* mesh sub-objects (implemented in MeshSub.cpp) */

	def_property	( vertices );
	def_property	( selectedVertices );
	def_property	( faces );
	def_property	( selectedFaces );
	def_property	( edges );
	def_property	( selectedEdges );

	/* NURBS sub-objects (implemented in NurbsSub.cpp) */

	def_property		  ( selectedCurveCVs );
	def_property		  ( selectedCurves );
	def_property		  ( selectedImports );
	def_property		  ( selectedPoints );
	def_property		  ( selectedSurfaces );
	def_property		  ( selectedSurfCVs );
	def_property		  ( curveCVs );
	def_property		  ( curves );
	def_property		  ( imports );
	def_property		  ( surfaces );
	def_property		  ( surfCVs );
	def_property		  ( points );

	/* spline shape props */

	def_property	( numsplines );

	/* general property & substructure access */
	
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);
	Value*		find_modifier(Object* obj, Value* prop);
	Value*		get_property_via_metadata(Value* prop);
	Control*	get_max_prop_controller(Value* prop, ParamDimension** pdim);
	BOOL		set_max_prop_controller(Value* prop, MAXControl* c);

	/* children mappers */
	
	ScripterExport Value* map(node_map& m);
	ScripterExport Value* map_path(PathName* path, node_map& m);
	ScripterExport Value* find_first(BOOL (*test_fn)(INode* node, int level, void* arg), void* test_arg);
	ScripterExport Value* get_path(PathName* path);

	INode*		to_node() { check_for_deletion(); return node; }

	//  coordsystem mappers
	void		object_to_current_coordsys(Point3& p, int mode=0);
	void		object_from_current_coordsys(Point3& p, int mode=0);
	void		world_to_current_coordsys(Point3& p, int mode=0);
	void		world_from_current_coordsys(Point3& p, int mode=0);
	void		world_to_current_coordsys(Quat& q);
	void		world_from_current_coordsys(Quat& q);
	void		world_to_current_coordsys(ScaleValue& s);
	void		world_from_current_coordsys(ScaleValue& s);

	// standard transorm controller access
	Control*	get_max_pos_controller(ParamDimension** pdim);
	Control*	get_max_scale_controller(ParamDimension** pdim);
	Control*	get_max_rotation_controller(ParamDimension** pdim);
	Control*	get_max_tm_controller(ParamDimension** pdim);
	Control*	get_max_controller(ParamDimension** pdim);
	BOOL		set_max_pos_controller(MAXControl* c);
	BOOL		set_max_scale_controller(MAXControl* c);
	BOOL		set_max_rotation_controller(MAXControl* c);
	BOOL		set_max_tm_controller(MAXControl* c);
	BOOL		set_max_controller(MAXControl* c);

	void		ctrl_to_current_coordsys(Point3& p, int mode=0);
	void		ctrl_from_current_coordsys(Point3& p, int mode=0);
	void		ctrl_to_current_coordsys(Quat& q);
	void		ctrl_from_current_coordsys(Quat& q);
	void		ctrl_to_current_coordsys(ScaleValue& s);
	void		ctrl_from_current_coordsys(ScaleValue& s);

	// recursive time functions
	#undef def_time_fn
	#define def_time_fn(_fn) Value* _fn##_vf(Value** arglist, int arg_count)
	#include "time_fns.h"

	// mesh access setup
	Mesh*		set_up_mesh_access(int access, ReferenceTarget** owner = NULL);
	Mesh*		set_up_mesh_face_access(int index, int access, ReferenceTarget** owner = NULL);
	Mesh*		set_up_mesh_vertex_access(int index, int access, ReferenceTarget** owner = NULL);
	BitArray*	get_vertsel();
	BitArray*	get_facesel();
	BitArray*	get_edgesel();
	GenericNamedSelSetList& get_named_vertsel_set();
	GenericNamedSelSetList& get_named_facesel_set();
	GenericNamedSelSetList& get_named_edgesel_set();
	void		update_sel();

	// NURBS access setup
	Object*		set_up_nurbs_access(NURBSSubObjectLevel level, BitArray& sel);
	Object*		set_up_nurbs_access(NURBSSubObjectLevel level);

};

#define INODE_CLASS_ID		Class_ID(BASENODE_CLASS_ID, 0)	/* INode class ID for MAXScript metadata table indexing */
#define INODE_SUPERCLASS_ID BASENODE_CLASS_ID

/* ---------------------- MAXNodeChildrenArray ----------------------- */

visible_class (MAXNodeChildrenArray)

class MAXNodeChildrenArray : public MAXWrapper, public Collection
{
public:
	INode*		parent;				/* parent node */

				MAXNodeChildrenArray(INode* parent);
	static ScripterExport Value* intern(INode* parent);

				classof_methods (MAXNodeChildrenArray, MAXWrapper);
	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name();

	// operations
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"

	// built-in property accessors
	def_property ( count );
	def_property ( center );
	def_property ( min );
	def_property ( max );

};

/* ---------------------- MAXObject ----------------------- */

// generic wrapper for MAX reftarg objects (as opposed to nodes); ie, things inside nodes, mod stacks, etc.

visible_class (MAXObject)

class MAXObject : public MAXWrapper
{
public:
	Object* obj;				// the MAX-side object

	ScripterExport MAXObject(Object* o);
	static ScripterExport Value* intern(Object* o);

	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name();

	BOOL CanTransferReference(int i) { return FALSE; }  // prevent mod applications from making us refer to a derived object
	
	def_property	( mesh );
};

/* ---------------------- MAXPB2ArrayParam ----------------------- */

// a virtual array wrapper for ParamBlock2 Tab<> array parameters
// allows indexed access to Tab<> paramaters held in ParamBlock2's

visible_class (MAXPB2ArrayParam)

class MAXPB2ArrayParam : public MAXWrapper, public Collection
{
public:
	IParamBlock2*		pblock;				// owning paramblock
	ParamDef*			pdef;				// Tab<> parameter def

				MAXPB2ArrayParam(IParamBlock2* pb, ParamDef* pd);
	static ScripterExport Value* intern(IParamBlock2* pb, ParamDef* pd);

				classof_methods (MAXPB2ArrayParam, MAXWrapper);
//	BOOL		_is_collection() { return 1; }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("Array Parameter"); }

	// operations
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"

	// built-in property accessors
	def_property ( count );

	// nested props on indexed elements (notably <param>[i].controller)
	ScripterExport Value*		get_container_property(Value* prop, Value* cur_prop);
	ScripterExport Value*		set_container_property(Value* prop, Value* val, Value* cur_prop);

	// NULL get/showprops
	Value* show_props_vf(Value** arg_list, int count) { return &undefined; }
	Value* get_props_vf(Value** arg_list, int count) { return &undefined; }
};

/* ---------------------- MAXModifier ----------------------- */

/* generic wrapper for MAX modifiers. */

visible_class (MAXModifier)

class MAXModifier : public MAXWrapper
{
public:
	Modifier*	mod;			// the MAX-side modifier

	ScripterExport MAXModifier(Modifier* imod);
	static ScripterExport Value* intern(Modifier* imod);

#	define is_modifier(v) ((v)->tag == class_tag(MAXModifier))
	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXModifier)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name();

	def_property	( name );
	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	Modifier*	to_modifier() { check_for_deletion(); return mod; }
};

visible_class (MAXModifierArray)

class MAXModifierArray : public MAXWrapper
{
public:
	INode*		node;			/* the noed containing the modifiers */

				MAXModifierArray(INode* node);
	static ScripterExport Value* intern(INode* node);

				classof_methods (MAXModifierArray, MAXWrapper);
	void		collect() { delete this; }
	BOOL		_is_collection() { return 1; }
	ScripterExport void		sprin1(CharStream* s);
	Modifier*	get_modifier(int index);
	Modifier*	find_modifier(TCHAR* name);
	TCHAR*		class_name();

	// operations
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"

	// built-in property accessors 
	def_property ( count );

};

/* ---------------------- MAXControl ----------------------- */

/* generic wrapper for MAX controls. */

visible_class (MAXControl)

class MAXControl : public MAXWrapper
{
public:
	Control*		 controller;			/* the MAX-side controller					*/
	ParamDimension*  dim;					/* dimension from originating animatable	*/
	short			 flags;

	ScripterExport MAXControl(Control* icont, ParamDimension* idim);
	ScripterExport MAXControl(Control* icont, int flags = 0);
	static ScripterExport Value* intern(Control* icont, ParamDimension* idim);
	static ScripterExport Value* intern(Control* icont, int flags = 0);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXControl)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name();
#	define is_controller(v) ((v)->tag == class_tag(MAXControl))

	// the non-recursive controller ops... (recursive's are implemented as MAXWrapper methods)
	#include "defimpfn.h"
	use_generic			 ( copy,	"copy");
	def_visible_generic  ( supportsTimeOperations,	"supportsTimeOperations");
	def_visible_generic  ( getTimeRange,			"getTimeRange");
	def_visible_generic  ( deleteKey,				"deleteKey");
	def_visible_generic  ( selectKey,				"selectKey");
	def_visible_generic  ( deselectKey,				"deselectKey");
	def_visible_generic  ( isKeySelected,			"isKeySelected");
	def_visible_generic  ( moveKey,					"moveKey");
	def_visible_generic  ( numKeys,					"numKeys");
	def_visible_generic  ( getKey,					"getKey");
	def_visible_generic  ( getKeyTime,				"getKeyTime");
	def_visible_generic  ( getKeyIndex,				"getKeyIndex");
	def_visible_generic  ( numSelKeys,				"numSelKeys");
	def_visible_generic  ( numEaseCurves,			"numEaseCurves");
	def_visible_generic  ( applyEaseCurve,			"applyEaseCurve");
	def_visible_generic  ( addMultiplierCurve,		"addMultiplierCurve");
	def_visible_generic  ( deleteMultiplierCurve,	"deleteMultiplierCurve");
	def_visible_generic  ( numMultiplierCurves,		"numMultiplierCurves");
	def_visible_generic  ( getMultiplierValue,		"getMultiplierValue");
	def_visible_generic  ( getBeforeORT,			"getBeforeORT");
	def_visible_generic  ( getAfterORT,				"getAfterORT");

	// built-in property accessors 
	def_property ( value );
	def_property ( keys );

	Control*	to_controller() { check_for_deletion(); return controller; }

	// trap reduceKeys() here to supply controllers dim
	def_visible_generic  ( reduceKeys,				"reduceKeys");

};

#define MAX_CTRL_NO_DIM		0x01		// indicates no explicit dimension yet assigned
#define MAX_CTRL_NEW		0x02		// new controller, copy(old) when first assigned

/* ---------------------- MAXSubAnim ----------------------- */

/* generic wrapper for MAX subanims. */

visible_class (MAXSubAnim)

class MAXSubAnim : public MAXWrapper
{
public:
	ReferenceTarget* ref;					/* parent object */
	int				 subanim_num;			/* the subanim index */

	ScripterExport MAXSubAnim(ReferenceTarget* ref, int index);
	static ScripterExport Value* intern(ReferenceTarget* ref, int index);

				classof_methods(MAXSubAnim, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("SubAnim"); }
	ScripterExport ReferenceTarget* get_max_object();

	// standard transform controller access
	Control*	get_max_pos_controller(ParamDimension** pdim);
	Control*	get_max_scale_controller(ParamDimension** pdim);
	Control*	get_max_rotation_controller(ParamDimension** pdim);
	Control*	get_max_tm_controller(ParamDimension** pdim);
	Control*	get_max_controller(ParamDimension** pdim);
	BOOL		set_max_pos_controller(MAXControl* c);
	BOOL		set_max_scale_controller(MAXControl* c);
	BOOL		set_max_rotation_controller(MAXControl* c);
	BOOL		set_max_tm_controller(MAXControl* c);
	BOOL		set_max_controller(MAXControl* c);

	def_property( controller );
	def_property( isAnimated );
	def_property( keys );
	def_property( value );
	def_property( object );

	// these converters all attempt to bounce off the subanim object
	Value*		subanim_obj();

	INode*		to_node()		{ return subanim_obj() != this ? subanim_obj()->to_node() : Value::to_node(); }
	Mtl*		to_mtl()		{ return subanim_obj() != this ? subanim_obj()->to_mtl() : Value::to_mtl(); }
	Texmap*		to_texmap()		{ return subanim_obj() != this ? subanim_obj()->to_texmap() : Value::to_texmap(); }
	Modifier*	to_modifier()	{ return subanim_obj() != this ? subanim_obj()->to_modifier() : Value::to_modifier(); }
	Control*	to_controller() { return subanim_obj() != this ? subanim_obj()->to_controller() : Value::to_controller() ; }
	Atmospheric* to_atmospheric() { return subanim_obj() != this ? subanim_obj()->to_atmospheric() : Value::to_atmospheric(); }
	Effect*		to_effect() { return subanim_obj() != this ? subanim_obj()->to_effect() : Value::to_effect(); }
	ITrackViewNode* to_trackviewnode() { return subanim_obj() != this ? subanim_obj()->to_trackviewnode() : Value::to_trackviewnode(); }
};

/* ---------------------- MAXRefTarg ----------------------- */
// generic wrapper for MAX ReferenceTargets.

visible_class (MAXRefTarg)

class MAXRefTarg : public MAXWrapper
{
public:
	ReferenceTarget* ref;					/* the object */

	ScripterExport MAXRefTarg(ReferenceTarget* ref);
	static ScripterExport Value* intern(ReferenceTarget* ref);

				classof_methods(MAXRefTarg, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("MAXRefTarg"); }

};

/* ---------------------- MAXTVNode ----------------------- */

// generic wrapper for MAX TrackView Nodes. 

visible_class (MAXTVNode)

class MAXTVNode : public MAXWrapper
{
public:
	ITrackViewNode*	 parent;		// parent node
	int				 index;			// the TVNode index

	ScripterExport MAXTVNode(ITrackViewNode* parent, int index);
	static ScripterExport Value* intern(ITrackViewNode* parent, int index);

				classof_methods (MAXTVNode, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("TrackViewNode"); }
	ReferenceTarget* get_max_object();

	Value*		get_property(Value** arg_list, int count);
	Value*		set_property(Value** arg_list, int count);

	Control* get_max_prop_controller(Value* prop, ParamDimension** pdim);
	BOOL	 set_max_prop_controller(Value* prop, MAXControl* c);
	Control* get_max_pos_controller(ParamDimension** pdim) { return get_max_prop_controller(n_position, pdim); }
	Control* get_max_scale_controller(ParamDimension** pdim) { return get_max_prop_controller(n_scale, pdim); }
	Control* get_max_rotation_controller(ParamDimension** pdim) { return get_max_prop_controller(n_rotation, pdim); }

	def_generic  ( get_props,  "getPropNames" );
	def_property ( name );

	RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);

	ITrackViewNode* to_trackviewnode() { check_for_deletion(); return (parent == NULL) ? MAXScript_interface->GetTrackViewRootNode() : parent->GetNode(index); }
};

/* ---------------------- MAXMeshClass ----------------------- */
// the sole instance of the MAXMeshClass represents the Nodes made from the core tri-mesh object class TriObject.
// reflecting its special status in MAX, it is defined by a separate class in MAXScript,
// allowing access to all the things you want to get at when procedurally working 
// with meshes.
 
visible_class (MAXMeshClass)

class MAXMeshClass : public Value
{
public:
				MAXMeshClass();

				classof_methods (MAXMeshClass, Value);
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	void		export_to_scripter();

	ScripterExport Value* apply(Value** arglist, int count);		// object creation by applying class object
	Class_ID	get_max_class_id() { return triObjectClassID; } 
};

/* ---------------------- MeshValue ----------------------- */
// A wrapper for the MAX SDK Mesh objects, typically used in Object plugins for creating scene
// object meshes

applyable_class (MeshValue)

class MeshValue : public MAXWrapper
{
public:
	Object*		obj;		// MAX object if indirect ref to mesh (ref 0)
	Mesh*		mesh;		// direct mesh ref
	BOOL		owned;		// if direct ref owned by this value (& so s/b deleted on collect)

	ScripterExport MeshValue(Mesh* imesh, BOOL owned = FALSE);
	ScripterExport MeshValue(Object* obj);
	static ScripterExport Value* intern(Object* obj);
			   ~MeshValue();

				classof_methods (MeshValue, MAXWrapper);
	void		collect() { delete this; }
	void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("TriMesh"); }
	ReferenceTarget* get_max_object() { return obj; }
	void		SetReference(int i, RefTargetHandle rtarg);
	RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, PartID& partID,  RefMessage message);

	void		setup_mesh(int access = MESH_READ_ACCESS);

	// mesh protocol (same as for MAXNode)
	def_generic( setvert,			"setvert");
	def_generic( getvert,			"getvert");
	def_generic( settvert,			"settvert");
	def_generic( gettvert,			"gettvert");
	def_generic( setvertcolor,		"setvertcolor");
	def_generic( getvertcolor,		"getvertcolor");
	def_generic( setnumverts,		"setnumverts");
	def_generic( getnumverts,		"getnumverts");
	def_generic( setnumtverts,		"setnumtverts");
	def_generic( getnumtverts,		"getnumtverts");
	def_generic( setnumcpvverts,	"setnumcpvverts");
	def_generic( getnumcpvverts,	"getnumcpvverts");
	def_generic( setnumfaces,		"setnumfaces");
	def_generic( getnumfaces,		"getnumfaces");
	def_generic( buildtvfaces,		"buildTVFaces");
	def_generic( buildvcfaces,		"buildVCFaces");
	def_generic( defaultvcfaces,	"defaultVCFaces");
	def_generic( getnormal,			"getnormal");
	def_generic( setnormal,			"setnormal");
	def_generic( setface,			"setface");
	def_generic( getface,			"getface");
	def_generic( settvface,			"setTVFace");
	def_generic( gettvface,			"getTVFace");
	def_generic( setvcface,			"setVCFace");
	def_generic( getvcface,			"getVCFace");
	def_generic( getfacenormal,		"getfacenormal");
	def_generic( setfacenormal,		"setfacenormal");
	def_generic( setfacematid,		"setfaceMatID");
	def_generic( getfacematid,		"getfaceMatID");
	def_generic( setfacesmoothgroup, "setfaceSmoothGroup");
	def_generic( getfacesmoothgroup, "getfaceSmoothGroup");
	def_generic( setedgevis,		"setedgevis");
	def_generic( getedgevis,		"getedgevis");
	def_generic( detachVerts,		"detachVerts");
	def_generic( detachFaces,		"detachFaces");
	def_generic( extrudeface,		"extrudeface");
	def_generic( deletevert,		"deletevert");
	def_generic( deleteface,		"deleteface");
	def_generic( collapseface,		"collapseface");
	def_generic( setMesh,			"setMesh");
	def_generic( update,			"update");
	def_generic( getVertSelection,	"getVertSelection");
	def_generic( setVertSelection,	"setVertSelection"); 
	def_generic( getFaceSelection,  "getFaceSelection");  
	def_generic( setFaceSelection,	"setFaceSelection");  
	def_generic( getEdgeSelection,	"getEdgeSelection");  
	def_generic( setEdgeSelection,	"setEdgeSelection"); 

	def_generic( copy,				"copy");  
	def_generic( delete,			"delete");  
	use_generic( plus,				"+" );		// mesh boolean ops
	use_generic( minus,				"-" );
	use_generic( times,				"*" );

	def_property( numverts );
	def_property( numtverts );
	def_property( numfaces );
	def_property( numcpvverts );
	def_property( mesh );

	Mesh*		to_mesh() { setup_mesh(); return mesh; }
};

/* ---------------------- MAXAtmospheric ----------------------- */

visible_class (MAXAtmospheric)

class MAXAtmospheric : public MAXWrapper
{
public:
	Atmospheric*	atmos;			// the MAX-side Atmospheric

	ScripterExport MAXAtmospheric(Atmospheric* iatmos);
	static ScripterExport Value* intern(Atmospheric* iatmos);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXAtmospheric)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("Atmospheric"); }

#include "defimpfn.h"
	#include "atmspro.h"

	def_property	( name );
	def_prop_getter	( numGizmos );

	Atmospheric*	to_atmospheric() { check_for_deletion(); return atmos; }
};

// RK: Start
/* ---------------------- MAXEffect ----------------------- */

visible_class (MAXEffect)

class MAXEffect : public MAXWrapper
{
public:
	Effect*	effect;			// the MAX-side Effect

	ScripterExport MAXEffect(Effect* ieffect);
	static ScripterExport Value* intern(Effect* ieffect);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXEffect)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void		sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("Effect"); }

#include "defimpfn.h"
	#include "atmspro.h"

	def_property	( name );
	def_prop_getter	( numGizmos );

	Effect*	to_effect() { check_for_deletion(); return effect; }
};

// RK: End
/* ---------------------- ChangeHandler ----------------------- */

/* maintains notification references to a bunch of objects & a scitped function
 * to execute if one changes - runtime object for the 'on object changed ....' construct */
class ChangeHandler;

visible_class (ChangeHandler)

class CHTimeChangeCallback : public TimeChangeCallback
{
public:
	static BOOL registered;					// if active
	static Tab<ChangeHandler*> handlers;	// active handlers
	static BOOL notified;					// set if any delayed-handle events have been notified
	public:
		void TimeChanged(TimeValue t);
};

class CHRedrawViewsCallback : public RedrawViewsCallback
{
public:
	static BOOL registered;					// if active
	static Tab<ChangeHandler*> handlers;	// active handlers
	static BOOL notified;					// set if any delayed-handle events have been notified
	public:
		void proc(Interface *ip);
};

class ChangeHandler : public MAXWrapper
{
public:
	Value*		fn;					// function to run
	Value*		attribute;			// controlling change atribute
	Value*		id;					// optional handler ID for group deleting
	Value*		handle_at;			// delayed handling event code (#redraw, #timeChange)
	short		parameter_count;	// # formal params for fn
	short		flags;
	Tab<ReferenceTarget*> targets;	// table of targets who've notified delayed-handling handler

	static CHRedrawViewsCallback	chrvcb;		// redrawviews callback
	static CHTimeChangeCallback		chtccb;		// timechange callback
	static Tab<ChangeHandler*>		handlers;	// immediate-mode active handlers

				ChangeHandler(Value* ifn, Value* attrib, Value* id, Value* handle_at);

				classof_methods (ChangeHandler, Value);
	void		collect() { delete this; }
	void		gc_trace();
	TCHAR*		class_name() { return _T("<ChangeHandler>"); }
	void		handle_event(ReferenceTarget* target);
	void		call_handler(ReferenceTarget* target);

	RefResult	NotifyRefChanged(Interval changeInt, RefTargetHandle hTarget, 
								 PartID& partID,  RefMessage message);		
};

#define CH_BROKEN		0x0001		// fn had a runtime error, disabled
#define CH_BUSY			0x0002		// fn is running, ignore recursive events
#define CH_NOTIFIED		0x0004		// change notified, handle at next specied handleAt: event

/* ---------------------- Node mapping stuff  ----------------------- */

/* node mapping argument structures */

struct node_map
{
	value_vf	vfn_ptr;							// virtual fn to map
	value_cf	cfn_ptr;							// or, c fn to map (one or the other must be null)
	Value**		arg_list;							// args to pass on...
	int			count;
	BOOL		(*selector)(INode*, int, void*);	// set selector fn
	void*		sel_arg;							// arg for the selector
	int			get_index;							// index if we are doing an indexed get
	int			get_count;							// local running traverse count during get
	Value**		get_result_p;						// ptr to result holder for indexed get
	Array*		collection;							// append map results here if non-null (used in for ... collect)
	short		flags;								// control flags
};

#define NM_INVERT	0x0001							// invert map order, map parents last on the way out of the recursion
#define NM_SELECT	0x0002							// applying a select, adjust clear flag 
#define NM_GET	    0x0004							// doing a get, return get_index'th item 

typedef struct
{
	BOOL		(*tester)(INode*, int, void*);		// node test fn
	void*		test_arg;							// tester arg
	BOOL		(*selector)(INode*, int, void*);	// set selector fn
	void*		sel_arg;							// arg for the selector
} node_find;

typedef struct
{
	PathName*	path;								// path to the node
	BOOL		(*selector)(INode*, int, void*);	// set selector fn
	void*		sel_arg;							// arg for the selector
} node_get;

class ConvertToRestore : public RestoreObj 
{
	BOOL onlyForRedo;
public:		   
	ConvertToRestore(BOOL onlyForRedo = FALSE) { this->onlyForRedo = onlyForRedo; }			
	void Restore(int isUndo);
	void Redo();
	TSTR Description() {return TSTR(_T("convertTo()"));}
};

// G-bufer channel names & IDs
typedef struct { TCHAR* name; int code; } gbuff_chan_code;
extern gbuff_chan_code gbuff_chan_codes[];

// various externs
extern void for_all_nodes(INode* root, node_map* m);
extern Value* find_first_node(INode* root, node_find* f);
extern Value* get_node(INode* root, node_get* g);
extern void for_all_path_nodes(INode* root, PathName* path, node_map* m);
extern BOOL all_objects_selector(INode* node, int level, void* selector_arg);
extern int max_name_compare(TCHAR* max_name, TCHAR* other);
extern int max_name_match(TCHAR* max_name, TCHAR* pattern);
extern Value* get_subanim_property(Animatable* anim, Value* prop);
extern Value* get_subanim_property(Animatable* anim, int i);
extern Value* set_subanim_property(Animatable* anim, Value* prop, Value* val);
extern Value* set_subanim_property(Animatable* anim, int i, Value* val);
extern BOOL set_subanim_controller(Animatable* anim, Value* prop, Control* newc, MAXControl* newcv);
extern Control* find_subanim_controller(Animatable* anim, Value* prop, ParamDimension** dim);
extern void deselect_if_motion_panel_open();

#endif
