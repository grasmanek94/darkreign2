/*	
 *		MAXMaterials.h - MAX material & map wrapper classes
 *
 *			Copyright © John Wainwright 1996
 *
 */

#ifndef _H_MAXMATERIALS
#define _H_MAXMATERIALS

#include "Max.h"
#include "MaxObj.h"
//#include "sceneapi.h"

#if 0 // HEY!! add material library access
		virtual int LoadMaterialLib(const TCHAR *name)=0;
		virtual int SaveMaterialLib(const TCHAR *name)=0;
		virtual MtlBaseLib& GetMaterialLibrary()=0;
		static Class_ID mtlBaseLibClassID(MTLBASE_LIB_CLASS_ID,0);
#endif

/* -------------------------- MAXMaterial -------------------------- */

visible_class (MAXMaterial)

class MAXMaterial : public MAXWrapper
{
public:
	Mtl*		mat;				/* the MAX-side material	*/

				MAXMaterial(Mtl* imat);
	static ScripterExport MAXMaterial* intern(Mtl* imat);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXMaterial)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name();

#include "defimpfn.h"
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	def_property( name );
	def_property( effectsChannel );

	Mtl*		to_mtl() { check_for_deletion(); return mat; }
	MtlBase*	to_mtlbase() { check_for_deletion(); return mat; }
};

/* ---------------------- MAXMultiMaterial ----------------------- */

visible_class (MAXMultiMaterial)

class MAXMultiMaterial : public MAXWrapper
{
public:
	MultiMtl*	mat;				/* the MAX-side material	*/

				MAXMultiMaterial(MultiMtl* imat);
	static ScripterExport MAXMultiMaterial* intern(MultiMtl* imat);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXMultiMaterial)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name();

	def_generic (get, "get");
	def_generic (put, "put");
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	ScripterExport Value* map(node_map& m);

	def_property( name );
	def_property( numsubs );
	def_property( count );

	Mtl*		to_mtl() { return mat; }
	MtlBase*	to_mtlbase() { check_for_deletion(); return mat; }
};

/* ---------------------- Material Library ----------------------- */

applyable_class (MAXMaterialLibrary)

class MAXMaterialLibrary : public MAXWrapper
{
public:
	MtlBaseLib	new_lib;
	MtlBaseLib&	lib;

				MAXMaterialLibrary(MtlBaseLib& ilib);
				MAXMaterialLibrary(MtlBaseLib* ilib);
				MAXMaterialLibrary();
	static ScripterExport Value* intern(MtlBaseLib& ilib);
	static ScripterExport Value* intern(MtlBaseLib* ilib);

				classof_methods (MAXMaterialLibrary, MAXWrapper);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MtlBase*	get_mtlbase(int index);
	MtlBase*	find_mtlbase(TCHAR* name);
	TCHAR*		class_name();

	/* operations */
	
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	/* built-in property accessors */

	def_property ( count );
};

/* ------------------------- MAXTexture ------------------------ */

visible_class (MAXTexture)

class MAXTexture : public MAXWrapper
{
public:
	Texmap*	map;				/* the MAX-side map	*/

				MAXTexture(Texmap* imap);
	static ScripterExport MAXTexture* intern(Texmap* imap);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXTexture)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name();

#include "defimpfn.h"
#	include "texmapro.h"
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	def_property( name );

	Texmap*		to_texmap() { check_for_deletion(); return map; }
	MtlBase*	to_mtlbase() { check_for_deletion(); return map; }
};

/* -------------------------- MAXMtlBase -------------------------- */

// a generic wrapper for MtlBase classes such as UVGen, XYZGen, TexOut, etc.

visible_class (MAXMtlBase)

class MAXMtlBase : public MAXWrapper
{
public:
	MtlBase*	mtl;				/* the MAX-side mtlbase	*/

				MAXMtlBase(MtlBase* imtl);
	static ScripterExport MAXMtlBase* intern(MtlBase* imtl);

	static Value* make(MAXClass* cls, Value** arg_list, int count);

	BOOL		is_kind_of(ValueMetaClass* c) { return (c == class_tag(MAXMtlBase)) ? 1 : MAXWrapper::is_kind_of(c); }
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	TCHAR*		class_name() { return _T("MapSupportClass"); }
	Value*		copy_vf(Value** arg_list, int count) { return MAXWrapper::copy_no_undo(arg_list, count); }

	MtlBase*	to_mtlbase() { check_for_deletion(); return mtl; }
};

/* ------------------ MEdit materials virtual array -------------------- */

visible_class (MAXMeditMaterials)

class MAXMeditMaterials : public Value
{
public:
				MAXMeditMaterials() { tag = &MAXMeditMaterials_class; }

				classof_methods (MAXMeditMaterials, Value);
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);
	MtlBase*	get_mtl(int index);
	MtlBase*	find_mtl(TCHAR* name);
	static SceneAPI	*sceneapi;
	static void setup();

	// operations
	ScripterExport Value* map(node_map& m);

#include "defimpfn.h"
#	include "arraypro.h"

	// built-in property accessors
	def_property ( count );
};

extern ScripterExport MAXMeditMaterials medit_materials;

#endif
