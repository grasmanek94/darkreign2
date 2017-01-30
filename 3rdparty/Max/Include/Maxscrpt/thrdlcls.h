/*	
 *		thread_locals.h - thread locals for each interpreter thread in MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

/* thread locals and initial values */

	def_thread_local( CharStream*, current_stdout,			new WindowStream(_T("Script Output")));
	def_thread_local( BOOL,		force_listener_open,		TRUE);			// whether to force listener open on output to it

	def_thread_local( Value**,	current_frame,				NULL);			// current interpreter frame (for thunk evals)
	def_thread_local( Value**,	current_scan_frame,			NULL);			// current interpreter frame (for gc scanner) 
	def_thread_local( Value**,	current_locals_frame,		NULL);			// C++ local frame
	def_thread_local( Value*,	current_result,				NULL);			// C++ current Value* function result
	def_thread_local( long,		stack_limit,				1024000);		// max stack size to catch recurse loops, 1Mb to start
	def_thread_local( long,		stack_base,					(long)_alloca(sizeof(int)));	// current stack base
	def_thread_local( MSPlugin*, current_plugin,			NULL);			// current scripted plugin (for plugin thunk evals)
	def_thread_local( Struct*,	current_struct,				NULL);			// current struct (for struct member thunk evals)
	def_thread_local( Value*,	current_container,			NULL);			// current container for nested property access
	def_thread_local( int,		container_index,			0);				// current container index (if any)
	def_thread_local( Value*,	container_prop,				NULL);			// current container prop (if any)
	def_thread_local( Value*,	current_prop,				NULL);			// most recent prop access (if any)

	def_thread_local( Value*,	source_file,				NULL);			// current source file
	def_thread_local( int,		source_pos,					0);				// current pos in source file

	def_thread_local( BOOL,		needs_redraw,				0);
	def_thread_local( BOOL,		redraw_mode,				1);				// redraw on
	def_thread_local( BOOL,		pivot_mode,					0);				// pivot off
	def_thread_local( BOOL,		undo_mode,					1);				// undo on
	def_thread_local( Value*,	current_level,				&all_objects);	// $objects
	def_thread_local( BOOL,		use_time_context,			0);				// use MAX time slider
	def_thread_local( TimeValue, current_time,				0);
	def_thread_local( Value*,	current_coordsys,			n_default);
	def_thread_local( Value*,	center_mode,				n_default);

	def_thread_local( int,		rand_accum,					0);				// for our own rand()

	def_thread_local( HANDLE,	message_event,				NULL);			// listener_message synch event
