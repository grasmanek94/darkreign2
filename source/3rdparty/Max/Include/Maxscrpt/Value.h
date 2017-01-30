/*		Value.h - metaclass system  MAXScript values
 *
 *	All MAXScript-specific C++ objects are subclasses of a single root class, Value, 
 *  and allocated & automatically freed in a specially maintained heap.  There is also
 *  a metaclass system to provide a runtime type calculus for the scripter.  Value subclasses
 *  are divided into those that are scripter-visible, (ie, may wind up as objects that the
 *  scripter may pass around or store in variables, etc.), and those that are entirely
 *  internal to the scripter operation (such as thunks, etc.).  The scripter-visible
 *  classes (the majority) are represented in the metasystem by instances of separate
 *  metaclasses.  The metaclasses are all subclasses of ValueMetaClass, the metaclass of
 *  a class X is named XClass and its sole instance is X_class. The class instances are
 *  made visible in globals (usually) named X.  
 *
 *  Each Value instance has a tag word that either contains a pointer to the instance's
 *  class instance (in the case of scripter-visible classes) or the reserved value INTERNAL_CLASS_TAG.
 *  This value is used in performing runtimne type tests and for yielding results to classOf 
 *  methods.
 *
 *  The metaclass, its instance and some of the class calculus methods are usually defined via
 *  a bunch of macros defined here (see visible_class, visible_class_instance, etc.)
 *
 *  Some of the classes are can be instanced directly as literals in a script, such as strings, 
 *  Point3s, arrays, etc.  Some others are instantiable directly by applying the class value
 *  to a set of initializing parameters, ie, using the class as a function in a function call,
 *	for example, ray, quat, interval, etc.  These are defined via a variant macro: applyable_class().
 *  A special case of this is provided in the MAXWrapper subsytem for creatable MAX objects, such as
 *  boxes, lights, camera, etc..  These are represnted by instances of the class MAXClass, and again, thses
 *  instances are exposed in globals to be applied to creation paramters.  These instances
 *  contain a lot of property metadata and are defined in MAX_classes.cpp.  See MAXObject.h for more
 *  info.
 *	
 *		Copyright (c) John Wainwright, 1996
 *
 */

#ifndef _H_VALUE
#define _H_VALUE

#include "Colctble.h"
#include "Max.h"
#include "STDMAT.H"
#include "Surf_api.h"
#include "Collect.h"

class Name;
#include "defextfn.h"
#	include "corename.h"

// forward declarations...
class PathName;
class Undefined;
class UserProp;
class UserGeneric;

extern ScripterExport Undefined undefined;
class ValueMetaClass;
typedef struct node_map node_map;

// the root MAXScript class
class Value : public Collectable
{
private:
    ScripterExport static Matrix3  s_error_matrix;
	ScripterExport static Box2  s_error_box2;
public:
	ValueMetaClass* tag;		// runtime type tag; filled in by subclasses

	ScripterExport virtual BOOL	is_kind_of(ValueMetaClass* c);
	ScripterExport virtual ValueMetaClass* local_base_class(); // local base class in this class's plug-in
	virtual Value*	eval() { check_interrupts(); return this; }
	virtual Value*	eval_no_wrapper() { check_interrupts(); return this; }
	virtual Value*  apply(Value** arglist, int count) { throw TypeError (_T("Call needs function or class"), this); return this; }
	virtual void	export_to_scripter() { }

	virtual Value*  map(node_map& m) { unimplemented(_T("map"), this) ; return this; }
	virtual Value*	map_path(PathName* path, node_map& m) { unimplemented(_T("map_path"), this) ; return this; }
	virtual Value*	find_first(BOOL (*test_fn)(INode* node, int level, void* arg), void* test_arg) { unimplemented(_T("find_first"), this) ; return this; }
	virtual Value*	get_path(PathName* path) { unimplemented(_T("get"), this) ; return this; }

	ScripterExport virtual void	sprin1(CharStream* stream);
	ScripterExport virtual void	sprint(CharStream* stream);

	virtual void	prin1() { sprin1(thread_local(current_stdout)); }
	virtual void	print() { sprint(thread_local(current_stdout)); }

	/* include all the protocol declarations */

#include "defabsfn.h"
#	include "mathpro.h"
#	include "vectpro.h"
#	include "matpro.h"
#	include "quatpro.h"
#	include "arraypro.h"
#	include "streampr.h"
#	include "strngpro.h"
#	include "timepro.h"
#	include "colorpro.h"
#	include "nodepro.h"
#	include "ctrlrpro.h"
#	include "prims.h"
#	include "biprops.h"
#	include "bitmapro.h"
#	include "texmapro.h"
#	include "atmspro.h"
#	include "nurbspro.h"
#	include "ctbmapro.h"

// MXSAgni specific -- START --
#	include "bmatpro.h"
#	include	"boxpro.h"
#	include "phyblpro.h"
#	include "phymcpro.h"
#	include	"bipedpro.h"
#	include "notespro.h"
#	include "xrefspro.h"
// MXSAgni specific -- END --

	ScripterExport virtual Class_ID get_max_class_id() { return Class_ID(0, 0); }
	ScripterExport virtual Value* delete_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_T("delete"), this, Value*); }	  
	ScripterExport virtual Value* clearSelection_vf(Value** arglist, int arg_count) { ABSTRACT_FUNCTION(_T("clearSelection"), this, Value*); }	  

#undef def_generic
#define def_generic(fn, name) ScripterExport virtual Value* fn##_vf(Value** arglist, int arg_count);
#	include "kernlpro.h"
	
	virtual float		to_float() { ABSTRACT_CONVERTER(float, Float); }
	virtual TCHAR*		to_string() { ABSTRACT_CONVERTER(TCHAR*, String); }
	virtual int			to_int() { ABSTRACT_CONVERTER(int, Integer); }
	virtual BOOL		to_bool() { ABSTRACT_CONVERTER(BOOL, Boolean); }
	virtual BitArray&	to_bitarray() { throw ConversionError (this, _T("BitArray")); return *(BitArray*)NULL; }
	virtual Point3		to_point3() { ABSTRACT_CONVERTER(Point3, Point3); }
	virtual Point2		to_point2() { ABSTRACT_CONVERTER(Point2, Point2); }
	virtual AColor		to_acolor() { throw ConversionError (this, _T("Color")); return AColor(0,0,0); }
	virtual COLORREF	to_colorref() { throw ConversionError (this, _T("Color")); return RGB(0,0,0); }
	virtual INode*		to_node() { ABSTRACT_CONVERTER(INode*, <node>); }
	virtual Ray			to_ray() { throw ConversionError (this, _T("Ray")); return Ray(); }
	virtual Interval	to_interval() { throw ConversionError (this, _T("Interval")); return Interval();  }
	virtual Quat		to_quat() { throw ConversionError (this, _T("Quaternion")); return Quat();  }
	virtual AngAxis		to_angaxis() { throw ConversionError (this, _T("AngleAxis")); return AngAxis();  }
	virtual Matrix3&	to_matrix3() { throw ConversionError (this, _T("Matrix")); return s_error_matrix;  }
	virtual float*		to_eulerangles() { ABSTRACT_CONVERTER(float*, Float); }
	virtual Mtl*		to_mtl() { ABSTRACT_CONVERTER(Mtl*, Material); }
	virtual Texmap*		to_texmap() { ABSTRACT_CONVERTER(Texmap*, TextureMap); }
	virtual MtlBase*	to_mtlbase() { ABSTRACT_CONVERTER(MtlBase*, MtlBase); }
	virtual Modifier*	to_modifier() { ABSTRACT_CONVERTER(Modifier*, Modifier); }
	virtual TimeValue	to_timevalue() { ABSTRACT_CONVERTER(TimeValue, Time); }
	virtual Control*	to_controller() { ABSTRACT_CONVERTER(Control*, Controller); }
	virtual Atmospheric* to_atmospheric() { ABSTRACT_CONVERTER(Atmospheric*, Atmospheric); }
	virtual Effect*		to_effect() { ABSTRACT_CONVERTER(Effect*, Effect); }						// RK: Added this
	virtual ShadowType*	to_shadowtype() { ABSTRACT_CONVERTER(ShadowType*, ShadowType); }			// RK: Added this
	virtual FilterKernel*	to_filter() { ABSTRACT_CONVERTER(FilterKernel*, FilterKernel); }		// RK: Added this
	virtual INode*		to_rootnode() { ABSTRACT_CONVERTER(INode*, <root>); }						// RK: Added this
	virtual ITrackViewNode* to_trackviewnode() { ABSTRACT_CONVERTER(ITrackViewNode*, TrackViewNode); }
	virtual NURBSIndependentPoint* to_nurbsindependentpoint() { throw ConversionError (this, _T("NURBSIndependentPoint")); return (NURBSIndependentPoint*)0;  }
	virtual NURBSPoint*	to_nurbspoint() { throw ConversionError (this, _T("NURBSPoint")); return (NURBSPoint*)0;  }
	virtual NURBSObject* to_nurbsobject() { throw ConversionError (this, _T("NURBSObject")); return (NURBSObject*)0;  }
	virtual NURBSControlVertex* to_nurbscontrolvertex() { throw ConversionError (this, _T("NURBSControlVertex")); return (NURBSControlVertex*)0;  }
	virtual NURBSCurve* to_nurbscurve() { throw ConversionError (this, _T("NURBSCurve")); return (NURBSCurve*)0;  }
	virtual NURBSCVCurve* to_nurbscvcurve() { throw ConversionError (this, _T("NURBSCVCurve")); return (NURBSCVCurve*)0;  }
	virtual NURBSSurface* to_nurbssurface() { throw ConversionError (this, _T("NURBSSurface")); return (NURBSSurface*)0;  }
	virtual NURBSTexturePoint* to_nurbstexturepoint() { throw ConversionError (this, _T("NURBSTexturePoint")); return (NURBSTexturePoint*)0;  }
	virtual NURBSSet*	to_nurbsset() { throw ConversionError (this, _T("NURBSSet")); return (NURBSSet*)0;  }
	virtual ReferenceTarget* to_reftarg() { ABSTRACT_CONVERTER(ReferenceTarget*, MaxObject); }
	virtual Mesh*		to_mesh() { ABSTRACT_CONVERTER(Mesh*, Mesh); }

// MXSAgni specific -- START --
	virtual Box2&	to_box2() { throw ConversionError (this, _T("Box2")); return s_error_box2;  }
// MXSAgni specific -- END --
	virtual NURBSTextureSurface* to_nurbstexturesurface() { throw ConversionError (this, _T("NURBSTextureSurface")); return (NURBSTextureSurface*)0;  }
	virtual NURBSDisplay* to_nurbsdisplay() { throw ConversionError (this, _T("NURBSDisplay")); return (NURBSDisplay*)0;  }
	virtual TessApprox*	 to_tessapprox() { throw ConversionError (this, _T("TessApprox")); return (TessApprox*)0;  }

	virtual Value*	widen_to(Value* arg, Value** arg_list) { ABSTRACT_WIDENER(arg); }
	virtual BOOL	comparable(Value* arg) { return (tag == arg->tag); }
	virtual BOOL	is_const() { return FALSE; }

	ScripterExport virtual Value*	get_property(Value** arg_list, int count);
	ScripterExport virtual Value*	set_property(Value** arg_list, int count);
	ScripterExport Value*	_get_property(Value* prop);
	ScripterExport Value*	_set_property(Value* prop, Value* val);
	virtual Value*	get_container_property(Value* prop, Value* cur_prop) { throw AccessorError (cur_prop, prop); }
	virtual Value*	set_container_property(Value* prop, Value* val, Value* cur_prop) { throw AccessorError (cur_prop, prop); }

#ifdef USE_PROPERTY_PATH_THUNKS
	virtual Value*	get_property_path(Value** arg_list, int count);
	virtual Value*	set_property_path(Value** arg_list, int count);
#endif

	// polymorphic default type predicates - abstracted over by is_x(v) macros as needed
	virtual BOOL	_is_collection() { return FALSE; }
	virtual BOOL	_is_charstream() { return FALSE; }
	virtual BOOL	_is_rolloutcontrol() { return FALSE; }
	virtual BOOL	_is_rolloutthunk() { return FALSE; }
	virtual BOOL	_is_function()	 { return FALSE; }
	virtual BOOL	_is_selection()	 { return FALSE; }
	virtual BOOL	_is_thunk()		{ return FALSE; }

	// yield selection set iterator if you can
	virtual SelectionIterator* selection_iterator() { throw RuntimeError (_T("Operation requires a selection (Array or BitArray)")); return NULL; }

	// scene persistence functions
	ScripterExport virtual IOResult Save(ISave* isave);
	// the Load fn is a static method on loadbale classes, see SceneIO.cpp & .h and each loadable class

	// called during MAX exit to have all MAXScript-side refs dropped (main implementation in MAXWrapper)
	virtual void drop_MAX_refs() { }
};

/* ---------- the base class for all metaclasses  ---------- */

class MetaClassClass;
extern MetaClassClass value_metaclass;  // the metaclass class

class ValueMetaClass : public Value
{
public:
	TCHAR*			name;
	UserProp*		user_props;		// additional, user defined property accessors
	short			uprop_count;
	UserGeneric*	user_gens;		//     "        "      "    generic fns
	short			ugen_count;

					ValueMetaClass() { }
	ScripterExport	ValueMetaClass(TCHAR* iname);

	ScripterExport BOOL	is_kind_of(ValueMetaClass* c);
#	define		is_valueclass(o) ((o)->tag == (ValueMetaClass*)&value_metaclass)
	ScripterExport void sprin1(CharStream* s);
	ScripterExport void	export_to_scripter();
	ScripterExport void add_user_prop(TCHAR* prop, value_cf getter, value_cf setter);
	ScripterExport void add_user_generic(TCHAR* name, value_cf fn);
	ScripterExport UserGeneric* find_user_gen(Value* name);
	ScripterExport UserProp* find_user_prop(Value* prop);
};

#define classof_methods(_cls, _super)					\
	Value* classOf_vf(Value** arg_list, int count)		\
	{													\
		check_arg_count(classOf, 1, count + 1);			\
		return &_cls##_class;							\
	}													\
	Value* superClassOf_vf(Value** arg_list, int count)	\
	{													\
		check_arg_count(superClassOf, 1, count + 1);	\
		return &_super##_class;							\
	}													\
	Value* isKindOf_vf(Value** arg_list, int count)		\
	{													\
		check_arg_count(isKindOf, 2, count + 1);		\
		return (arg_list[0] == &_cls##_class) ?			\
			&true_value :								\
			_super##_class.isKindOf_vf(arg_list, count); \
	}													\
	BOOL is_kind_of(ValueMetaClass* c)					\
	{													\
		return (c == &_cls##_class) ? 1					\
					: _super::is_kind_of(c);			\
	}

#define visible_class(_cls)												\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(TCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() { delete this; }							\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_s(_cls, _super)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(TCHAR* name) : ValueMetaClass (name) { }	\
		void		collect() { delete this; }							\
		Value*		classOf_vf(Value** arg_list, int count)				\
		{																\
			check_arg_count(classOf, 1, count + 1);						\
			return &_super##_class;										\
		}																\
		Value*		superClassOf_vf(Value** arg_list, int count)		\
		{																\
			return _super##_class.classOf_vf(NULL, 0);					\
		}																\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class(_cls)											\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(TCHAR* name) : ValueMetaClass (name) { }\
		void		collect() { delete this; }							\
		ScripterExport Value* apply(Value** arglist, int count);		\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define applyable_class_s(_cls, _super)									\
	class _cls##Class : public ValueMetaClass							\
	{																	\
	public:																\
					_cls##Class(TCHAR* name) : ValueMetaClass (name) { }\
		Value*		classOf_vf(Value** arg_list, int count)				\
		{																\
			check_arg_count(classOf, 1, count + 1);						\
			return &_super##_class;										\
		}																\
		Value*		superClassOf_vf(Value** arg_list, int count)		\
		{																\
			return _super##_class.classOf_vf(NULL, 0);					\
		}																\
		void		collect() { delete this; }							\
		ScripterExport Value* apply(Value** arglist, int count);		\
	};																	\
	extern ScripterExport _cls##Class _cls##_class;

#define visible_class_instance(_cls, _name)				\
	ScripterExport _cls##Class _cls##_class (_T(_name));

#define class_tag(_cls)				&_cls##_class

#define INTERNAL_CLASS_TAG				((ValueMetaClass*)0L)
#define INTERNAL_INDEX_THUNK_TAG		((ValueMetaClass*)1L)
#define INTERNAL_PROP_THUNK_TAG			((ValueMetaClass*)2L)
#define INTERNAL_LOCAL_THUNK_TAG		((ValueMetaClass*)3L)
#define INTERNAL_FREE_THUNK_TAG			((ValueMetaClass*)4L)
#define INTERNAL_RO_LOCAL_THUNK_TAG		((ValueMetaClass*)5L)
#define INTERNAL_CODE_TAG				((ValueMetaClass*)6L)
#define INTERNAL_SOURCEWRAPPER_TAG		((ValueMetaClass*)7L)
#define INTERNAL_PIPE_TAG				((ValueMetaClass*)8L)
#define INTERNAL_TOOL_LOCAL_THUNK_TAG	((ValueMetaClass*)9L)
#define INTERNAL_GLOBAL_THUNK_TAG		((ValueMetaClass*)10L)
#define INTERNAL_CONST_GLOBAL_THUNK_TAG ((ValueMetaClass*)11L)
#define INTERNAL_SYS_GLOBAL_THUNK_TAG	((ValueMetaClass*)12L)
#define INTERNAL_PLUGIN_LOCAL_THUNK_TAG	((ValueMetaClass*)13L)
#define INTERNAL_PLUGIN_PARAM_THUNK_TAG	((ValueMetaClass*)14L)
#define INTERNAL_RCMENU_LOCAL_THUNK_TAG	((ValueMetaClass*)15L)
#define INTERNAL_STRUCT_MEM_THUNK_TAG	((ValueMetaClass*)16L)
#define INTERNAL_MSPLUGIN_TAG			((ValueMetaClass*)17L)
#define INTERNAL_STRUCT_TAG				((ValueMetaClass*)18L)
#define INTERNAL_MAKER_TAG				((ValueMetaClass*)19L)
#define INTERNAL_CODEBLOCK_LOCAL_TAG	((ValueMetaClass*)20L)
#define INTERNAL_CODEBLOCK_TAG			((ValueMetaClass*)21L)

#define INTERNAL_TAGS					((ValueMetaClass*)100L)  // must be higher than all internal tags

visible_class (Value)

/* ---------- the distinguished value subclasses ---------- */

visible_class (Boolean)

class Boolean;
class ValueLoader;
extern ScripterExport Boolean true_value;
extern ScripterExport Boolean false_value;

class Boolean : public Value
{
public:
			Boolean() { tag = &Boolean_class; }
			classof_methods (Boolean, Value);
	void	collect() { delete this; }
	void	sprin1(CharStream* s);

#	define	is_bool(o) ((o)->tag == &Boolean_class)
	Value*	not_vf(Value**arg_list, int count);
	BOOL	to_bool() { return this == &true_value; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

/* ----- */

visible_class (Undefined)

class Undefined : public Value
{
public:
			Undefined() { tag = &Undefined_class; }
			classof_methods (Undefined, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
	Value*  copy_vf(Value** arg_list, int count) { return this; }

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
	Mtl*	to_mtl() { return NULL; }		// undefined is a NULL material
};

extern ScripterExport Undefined undefined;
extern ScripterExport Undefined dontCollect;

/* ----- */

visible_class (Ok)

class Ok : public Value
{
public:
			Ok() { tag = &Ok_class; }
			classof_methods (Ok, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

extern ScripterExport Ok ok;

/* ----- */

visible_class (Empty)

class Empty : public Value
{
public:
			Empty() { tag = &Empty_class; }
			classof_methods (Empty, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);
};

extern ScripterExport Empty empty;

/* ----- */

visible_class (Unsupplied)

class Unsupplied : public Value
{
public:
			Unsupplied() { tag = &Unsupplied_class; }
			classof_methods (Unsupplied, Value);
	void	collect() { delete this; }
	ScripterExport void	sprin1(CharStream* s);

	// scene I/O 
	IOResult Save(ISave* isave);
	static Value* Load(ILoad* iload, USHORT chunkID, ValueLoader* vload);
};

extern ScripterExport Unsupplied unsupplied;

#endif
