/*	
 *		MeshSub.h - edit mesh sub-object classes & functions
 *					also includes MeshDelta and MNMesh scripter classes
 *
 *	exposes the new-with-R3 MeshDelta, MapDelta & MeshSel tools for
 *  working with meshes.  Also provides access to MNMesh tools and 
 *  mesh sub-objects as direct properties on nodes.
 *
 *			Copyright © Autodesk, Inc., 1998
 *				John Wainwright
 */

#ifndef _H_MESHSUB
#define _H_MESHSUB

// mesh selection types
#define MSEL_ALL		1		// whole mesh selected 
#define MSEL_CUR		2		// current selection 
#define MSEL_EXP		3		// explicit selection (in vsel) 
#define MSEL_SINGLE		4		// explicit single index  

/* -------------- base class for mesh sub-object selections ------------------- */

class MeshSelection : public Value
{
public:
	MAXWrapper*	owner;			// owner node or modifier if any
	BYTE		sel_type;		// selection type
	BitArray	vsel;			// stand-alone selection if any
	DWORD		index;			// single vert index 			

	void		gc_trace();

	virtual MeshSelection* new_sel(MAXWrapper* own, BYTE stype, DWORD indx = 0) = 0;

	// utility functions to be specialized
	virtual BitArray* get_sel() = 0;	// my element selection
	virtual BitArray* get_owner_sel() = 0;	// owner's element selection
	virtual BitArray get_sel_vertices(Mesh* m) = 0;	// vertexes involved in my element selection
	virtual GenericNamedSelSetList& get_named_sel_set_list() = 0;
	virtual int		num_elements(Mesh* m) = 0;
	virtual BOOL	is_same_selection(Value* s) = 0;
	virtual void	delete_sel(Mesh& m, MeshDelta& md, BitArray &sel) = 0;

	// utility functions
			DWORD	get_sel_index(BitArray* vs, int n);  // index for n'th item vertex in BitArray
			void	update_sel();
			void	sprin1(TCHAR* type, CharStream* s);

	// operations
#include "defimpfn.h"
#	include "arraypro.h"
	def_generic ( move,			"move");
	def_generic ( scale,		"scale");
	def_generic ( rotate,		"rotate");
	def_generic ( delete,		"delete");
	def_generic ( select,		"select");
	def_generic ( deselect,		"deselect");
	def_generic ( selectmore,	"selectMore");

	ScripterExport Value* map(node_map& m);

	// built-in property accessors
	def_property ( count );
	def_property ( index );
	def_property ( selSetNames );
};

/* ---------------- mesh vertex selection --------------------- */

visible_class (VertSelectionValue)

class VertSelectionValue : public MeshSelection
{
public:
	ScripterExport VertSelectionValue(MAXWrapper* own, BYTE stype, DWORD indx = 0);
	
	MeshSelection* new_sel(MAXWrapper* own, BYTE stype, DWORD indx = 0);

				classof_methods (VertSelectionValue, Value);
#	define		is_vertselection(v) ((v)->tag == class_tag(VertSelectionValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// specialized utility functions
	BitArray*	get_sel();
	BitArray*	get_owner_sel() { return (owner == NULL) ? NULL : owner->get_vertsel(); }
	BitArray	get_sel_vertices(Mesh* m);
	GenericNamedSelSetList& get_named_sel_set_list() { return owner->get_named_vertsel_set(); }
	int			num_elements(Mesh* m) { return m->getNumVerts(); } 
	BOOL		is_same_selection(Value* s) { return is_vertselection(s); }
	void		delete_sel(Mesh& m, MeshDelta& md, BitArray &sel) { md.DeleteVertSet(m, sel); }

	// operations
	def_generic ( put,			"put");

	// built-in property accessors
	def_property ( pos );
};

/* ---------------- mesh face selection --------------------- */

visible_class (FaceSelectionValue)

class FaceSelectionValue : public MeshSelection
{
public:
	ScripterExport FaceSelectionValue(MAXWrapper* own, BYTE stype, DWORD indx = 0);
	
	MeshSelection* new_sel(MAXWrapper* own, BYTE stype, DWORD indx = 0);

				classof_methods (FaceSelectionValue, Value);
#	define		is_faceselection(v) ((v)->tag == class_tag(FaceSelectionValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// specialized utility functions
	BitArray*	get_sel();
	BitArray*	get_owner_sel() { return (owner == NULL) ? NULL : owner->get_facesel(); }
	BitArray	get_sel_vertices(Mesh* m);
	GenericNamedSelSetList& get_named_sel_set_list() { return owner->get_named_facesel_set(); }
	int			num_elements(Mesh* m) { return m->getNumFaces(); } 
	BOOL		is_same_selection(Value* s) { return is_faceselection(s); }
	void		delete_sel(Mesh& m, MeshDelta& md, BitArray &sel) { md.DeleteFaceSet(m, sel); }

	// operations
	def_generic ( put,			"put");

	// built-in property accessors
};

/* ---------------- edge face selection --------------------- */

visible_class (EdgeSelectionValue)

class EdgeSelectionValue : public MeshSelection
{
public:
	ScripterExport EdgeSelectionValue(MAXWrapper* own, BYTE stype, DWORD indx = 0);
	
	MeshSelection* new_sel(MAXWrapper* own, BYTE stype, DWORD indx = 0);

				classof_methods (EdgeSelectionValue, Value);
#	define		is_edgeselection(v) ((v)->tag == class_tag(EdgeSelectionValue))
	void		collect() { delete this; }
	ScripterExport void sprin1(CharStream* s);

	// specialized utility functions
	BitArray*	get_sel();
	BitArray*	get_owner_sel() { return (owner == NULL) ? NULL : owner->get_edgesel(); }
	BitArray	get_sel_vertices(Mesh* m);
	GenericNamedSelSetList& get_named_sel_set_list() { return owner->get_named_edgesel_set(); }
	int			num_elements(Mesh* m) { return m->getNumFaces() * 3; } 
	BOOL		is_same_selection(Value* s) { return is_edgeselection(s); }
	void		delete_sel(Mesh& m, MeshDelta& md, BitArray &sel) { md.DeleteEdgeSet(m, sel); }

	// operations

	// built-in property accessors
};

#endif
