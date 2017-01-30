/*		Functions.h - the Function family class - primitives, generics
 *
 *		Copyright (c) John Wainwright, 1996
 *		
 */

#ifndef _H_FUNCTION
#define _H_FUNCTION

/* --- function base class -- */

visible_class (Function)

class Function : public Value
{
public:
	TCHAR*		name;
	TCHAR*		struct_name;	// packaged in a struct if non-null

				   Function() { name = NULL; struct_name = NULL; }
	ScripterExport Function(TCHAR* name, TCHAR* struct_name=NULL);
	ScripterExport ~Function();

				classof_methods (Function, Value);
#	define		is_function(o) ((o)->_is_function())
	BOOL		_is_function() { return 1; }

	ScripterExport void sprin1(CharStream* s);
	ScripterExport void export_to_scripter();
};
 
/* ----------------------- Generics ------------------------- */

visible_class (Generic)

class Generic : public Function
{
public:
	value_vf	fn_ptr;

				Generic() { }
 ScripterExport Generic(TCHAR* name, value_vf fn, TCHAR* struct_name = NULL);
			    Generic(TCHAR* name) : Function(name) { }

				classof_methods (Generic, Function);
	BOOL		_is_function() { return 1; }
	ScripterExport void init(TCHAR* name, value_vf fn);
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count);
};

visible_class (MappedGeneric)

class MappedGeneric : public Generic
{
public:
				MappedGeneric() { }
 ScripterExport MappedGeneric(TCHAR* name, value_vf fn);
			    MappedGeneric(TCHAR* name) : Generic(name) { }

				classof_methods (MappedGeneric, Generic);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count);
};

visible_class (NodeGeneric)

class NodeGeneric : public MappedGeneric
{
public:
 ScripterExport NodeGeneric(TCHAR* name, value_vf fn);
			    NodeGeneric(TCHAR* name) : MappedGeneric(name) { }

				classof_methods (NodeGeneric, MappedGeneric);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count);
};

/* -------------------------- Primitives ------------------------------ */

#define LAZY_PRIMITIVE	0x0001

visible_class (Primitive)

class Primitive : public Function
{ 
public:
	short		flags;
	value_cf	fn_ptr;

			    Primitive() { }
 ScripterExport Primitive(TCHAR* name, value_cf fn, short init_flags=0);
 ScripterExport Primitive(TCHAR* name, TCHAR* struct, value_cf fn, short init_flags=0);
			    Primitive(TCHAR* name) : Function(name) { }

				classof_methods (Primitive, Function);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count);
};

visible_class (MappedPrimitive)

class MappedPrimitive : public Primitive
{ 
public:
 ScripterExport MappedPrimitive(TCHAR* name, value_cf fn);

				classof_methods (MappedPrimitive, Primitive);
	BOOL		_is_function() { return 1; }
	void		collect() { delete this; }

	ScripterExport Value* apply(Value** arglist, int count);
};

/* ----- */

visible_class (MAXScriptFunction)

class MAXScriptFunction : public Function
{
public:
	short		parameter_count;
	short		local_count;
	short		keyparm_count;
	short		flags;
	Value**		keyparms;
	Value*		body;
	HashTable*	local_scope;
	value_cf	c_callable_fn;

 ScripterExport MAXScriptFunction(TCHAR* name, int parm_count, int keyparm_count, Value** keyparms,
								  int local_count, Value* body, HashTable* local_scope, short flags = 0);
				~MAXScriptFunction();

				classof_methods (MAXScriptFunction, Function);
	BOOL		_is_function() { return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();
	void		sprin1(CharStream* s);

	Value*		apply(Value** arglist, int count);

	value_cf	get_c_callable_fn();
};

#define FN_MAPPED_FN	0x0001		// declared a collection-mapped function
#define FN_BODY_FN		0x0002		// a loop or other body function, don't trap exits here


// UserProp & UserGeneric instances represent dynamically-added, user-defined generics
//  on built-in classes.  They are kept in sorted tables in ValueMetaClass instances,
// suitable for bsearching.
class UserProp
{
public:
	Value*		prop;
	value_cf	getter;
	value_cf	setter;
				UserProp (Value* p, value_cf g, value_cf s) { prop = p; getter = g; setter = s; }
};

class UserGeneric
{
public:
	Value*		name;
	value_cf	fn;
				
				UserGeneric(Value* n, value_cf f) { name = n; fn = f; }
};

// UserGenericValue is the scripter-visible generic fn value that dispatches the
// UserGeneric 'methods' in a target object's class

visible_class (UserGenericValue)

class UserGenericValue : public Function
{
public:
	Value*		fn_name;
	Value*		old_fn;   // if non-NULL, the original global fn that this usergeneric replaced
				
 ScripterExport UserGenericValue(Value* name, Value* old_fn);

				classof_methods (UserGenericValue, Function);
	BOOL		_is_function() { return TRUE; }
	void		collect() { delete this; }
	void		gc_trace();

	Value*		apply(Value** arglist, int count);
};

#define	def_user_prop(_prop, _cls, _getter, _setter)		\
	_cls##_class.add_user_prop(#_prop, _getter, _setter)

#define	def_user_generic(_fn, _cls, _name)					\
	_cls##_class.add_user_generic(#_name, _fn)





#endif
