/*	Collectable.h - Collectables include
 *
 *			Copyright (c) John Wainwright, 1996
 *
 */

#ifndef _H_COLLECTIBLE
#define _H_COLLECTIBLE

enum col_state {booting, pre_gc, in_mutator, in_mark, in_sweep, closing_down };

typedef struct free_mem free_mem;		// free mem linked list entry
struct free_mem
{
	free_mem*	next;
	free_mem*	prev;
	size_t		size;
};
		
/* collection flag bits ... */

enum gc_flags
{
	GC_IN_USE			= 0x0001,
	GC_GARBAGE			= 0x0002,
	GC_PERMANENT		= 0x0004,
	GC_IN_HEAP			= 0x0008,
	GC_NOT_NEW			= 0x0010,
	GC_STATIC			= 0x0020,
};

// free-list is kept in a number of separate size-related sub-lists, specifically
// for the high-bandwidth low size allocs.
// the heads of these are in the free_list static array in Collectable.
// each consecutive sub-list is for chunks one GC_ALLOC_MULTIPLE greater than the previous.
// the following defines determine the number of sub-lists.  

#define GC_NUM_SUBLISTS				128 
#define GC_LOW_SUBLIST				16   // <16, 16, 20, 24, 28, 32, ... 512, >512
#define GC_SUBLIST_INDEX_SHIFT		4    // log2(LOW_SUBLIST)

class Value;
class ValueMapper;

class Collectable
{
public:
	Collectable*	next;					// links (in whichever list instance is in)
	Collectable*	prev;
	static CRITICAL_SECTION list_update;	// for syncing allocation list updates
	short			flags;					// collection flags

	static Collectable* collectable_list;	// head of the collectable list
	static Collectable* permanent_list;		// head of the permanent list
	static free_mem* free_list[GC_NUM_SUBLISTS];				// head of the free list
	static size_t heap_allocated;			// running count of MAXScript heap usage
	static size_t heap_size;				// alloc'd heap size
	static col_state state;					// current collector state
	
	ScripterExport Collectable();
	ScripterExport ~Collectable();

	static ScripterExport void for_all_values(void (*map_fn)(Value* val), ValueMapper* mapper = NULL, ValueMetaClass* c = NULL);

	ScripterExport void* operator new (size_t);
	ScripterExport void operator delete (void *);
	
	static void	mark();
	static void	sweep();
	static void	setup(size_t);
	ScripterExport static void	gc();
	static void	coalesce_free_list();
	virtual void collect() = 0;				  // does the actual collecting, needs to be virtual to get right size to operator delete
	virtual void gc_trace() { mark_in_use(); } // the marking scanner, default is mark me in use
	static void close_down();
	static void drop_maxwrapper_refs();

	ScripterExport void	make_static() { make_permanent(); flags |= GC_STATIC; }
	ScripterExport void	make_permanent();
	ScripterExport void	make_collectable();

	int		is_marked()			{ return (flags & GC_IN_USE); }
	int		is_not_marked()		{ return !is_marked(); }
	int		is_garbage()		{ return is_not_marked(); }
	int		is_permanent()		{ return (flags & GC_PERMANENT); }
	void	mark_in_use()		{ flags |= GC_IN_USE; }
	void	unmark_in_use()		{ flags &= ~GC_IN_USE; }
};

// mapping object for Collectable::for_all_values()
class ValueMapper 
{
public:
	virtual void map(Value* val)=0;
};

ScripterExport void ms_free(void* p);
ScripterExport void* ms_malloc(size_t sz);
inline void ms_make_collectable(Collectable* v) { if (v != NULL && Collectable::state != closing_down) v->make_collectable(); }

#endif
