/*	
 *		primitives.h - primitive function defs for MAXScript
 *
 *			Copyright © John Wainwright 1996
 *
 */

#include "buildver.h"

/* scripter-visible primitives */

	/* user input */

	def_visible_primitive( pick_object,				"pickObject");  
	def_visible_primitive( pick_point,				"pickPoint"); 
	def_visible_primitive( get_kb_value,			"getKBValue"); 
	def_visible_primitive( get_kb_point,			"getKBPoint"); 
	def_visible_primitive( get_kb_line,				"getKBLine"); 
	def_visible_primitive( get_kb_char,				"getKBChar"); 
	def_visible_primitive( redraw_views,			"redrawViews"); 
	def_visible_primitive( pick_offset_distance,	"pickOffsetDistance"); 
	def_visible_primitive( clear_listener,			"clearListener"); 
	def_visible_primitive( get_listener_sel,		"getListenerSel");
	def_visible_primitive( set_listener_sel,		"setListenerSel");
	def_visible_primitive( get_listener_sel_text,	"getListenerSelText");
	def_visible_primitive( set_listener_sel_text,	"setListenerSelText");

	/* command-line debugger */

	def_visible_primitive( dbg_break,		"break");
	def_visible_primitive( show_stack,		"stack");
	def_visible_primitive( show_locals,		"locals");
	def_visible_primitive( dbg_go,			"go");

	/* max file I/O */
	
	def_visible_primitive( load_max_file,	"loadMaxFile");
	def_visible_primitive( merge_max_file,	"mergeMaxFile");
	def_visible_primitive( get_max_file_object_names, "getMAXFileObjectNames");
	def_visible_primitive( save_max_file,	"saveMaxFile");
	def_visible_primitive( save_nodes,		"saveNodes");
	def_visible_primitive( reset_max_file,	"resetMaxFile");
	def_visible_primitive( hold_max_file,	"holdMaxFile");
	def_visible_primitive( fetch_max_file,	"fetchMaxFile");
	def_visible_primitive( import_file,		"importFile");
	def_visible_primitive( export_file,		"exportFile");
#ifdef DESIGN_VER 
	def_visible_primitive( link_file,		"linkFile");
#endif

	def_visible_primitive( load_material_library,	"loadMaterialLibrary");
	def_visible_primitive( save_material_library,	"saveMaterialLibrary");
	def_visible_primitive( get_medit_material,		"getMeditMaterial");
	def_visible_primitive( set_medit_material,		"setMeditMaterial");

	def_visible_primitive( quit_max,		"quitMax");

	/* max animation */
	
	def_visible_primitive( play_animation,	"playAnimation");
	def_visible_primitive( stop_animation,	"stopAnimation");

	/* text file I/O */

	def_visible_primitive( open_file,		"openfile");
	def_visible_primitive( create_file,		"createfile");
	def_visible_primitive( open_encrypted,	"openEncryptedFile");
	def_visible_primitive( file_in,			"filein");
	def_visible_primitive( format,			"format");
	def_visible_primitive( open_log,		"openlog");
	def_visible_primitive( close_log,		"closelog");
	def_visible_primitive( flush_log,		"flushlog");

	def_visible_primitive( get_open_filename, "getOpenFileName");
	def_visible_primitive( get_save_filename, "getSaveFileName");
	def_visible_primitive( get_save_path,     "getSavePath");
	def_visible_primitive( filename_from_path, "filenameFromPath");

	def_visible_primitive( get_files,			"getFiles");
	def_visible_primitive( get_directories,		"getDirectories");
	def_visible_primitive( delete_file,			"deleteFile");
	def_visible_primitive( rename_file,			"renameFile");
	def_visible_primitive( copy_file,			"copyFile");
	def_visible_primitive( make_dir,			"makeDir");
	def_visible_primitive( get_filename_path,	"getFilenamePath");
	def_visible_primitive( get_filename_file,	"getFilenameFile");
	def_visible_primitive( get_filename_type,	"getFilenameType");
	def_visible_primitive( get_file_mod_date,	"getFileModDate");
	def_visible_primitive( get_file_create_date,"getFileCreateDate");
	def_visible_primitive( get_file_attribute,	"getFileAttribute");
	def_visible_primitive( set_file_attribute,	"setFileAttribute");

	def_visible_primitive( edit_script,		"edit");
	def_visible_primitive( new_script,		"newScript");

	/* rendering */

	def_visible_primitive( render,			"render");

	/* noise functions */

	def_visible_primitive( noise3,			"noise3");
	def_visible_primitive( noise4,			"noise4");
	def_visible_primitive( turbulence,		"turbulence");
	def_visible_primitive( fractalNoise,	"fractalNoise");

	/* atmospherics */

	def_visible_primitive( addAtmospheric,		"addAtmospheric");
	def_visible_primitive( setAtmospheric,		"setAtmospheric");
	def_visible_primitive( getAtmospheric,		"getAtmospheric");
	def_visible_primitive( deleteAtmospheric,	"deleteAtmospheric");
	def_visible_primitive( editAtmospheric,		"editAtmospheric"); // RK: Added this

	/* effects */ // RK: Added this

	def_visible_primitive( addEffect,		"addEffect");
	def_visible_primitive( setEffect,		"setEffect");
	def_visible_primitive( getEffect,		"getEffect");
	def_visible_primitive( deleteEffect,	"deleteEffect");
	def_visible_primitive( editEffect,		"editEffect");

	/* trackview nodes */

	def_visible_primitive( newTrackViewNode,		"newTrackViewNode");
	def_visible_primitive( deleteTrackViewNode,		"deleteTrackViewNode");
	def_visible_primitive( addTrackViewController,	"addTrackViewController");
	def_visible_primitive( deleteTrackViewController, "deleteTrackViewController");

	/* CUI */

	def_struct_primitive( cui_getConfigFile,cui,	"getConfigFile");
	def_struct_primitive( cui_setConfigFile,cui,	"setConfigFile");
	def_struct_primitive( cui_getDir,		cui,	"getDir");
	def_struct_primitive( cui_saveConfig,	cui,	"saveConfig");
	def_struct_primitive( cui_saveConfigAs,	cui,	"saveConfigAs");
	def_struct_primitive( cui_loadConfig,	cui,	"loadConfig");

	/* macro scripts */
	
	def_struct_primitive( run_macro,	macros,	"run");
	def_struct_primitive( edit_macro,	macros,	"edit");
	def_struct_primitive( new_macro,	macros,	"new");
	def_struct_primitive( load_macros,	macros,	"load");

	/* track view window access */
	
	def_struct_primitive( tvw_open,				trackView,	"open");
	def_struct_primitive( tvw_zoomSelected,		trackView,	"zoomSelected");
	def_struct_primitive( tvw_close,			trackView,	"close");
	def_struct_primitive( tvw_numTrackViews,	trackView,	"numTrackViews");
	def_struct_primitive( tvw_getTrackViewName,	trackView,	"getTrackViewName");
	def_struct_primitive( tvw_setFilter,		trackView,	"setFilter");
	def_struct_primitive( tvw_clearFilter,		trackView,	"clearFilter");
	def_struct_primitive( tvw_pickTrackDlg,		trackView,	"pickTrackDlg");

	/* schematic view window access */
	
	def_struct_primitive( svw_open,				schematicView,	"open");
	def_struct_primitive( svw_zoomSelected,		schematicView,	"zoomSelected");
	def_struct_primitive( svw_close,			schematicView,	"close");
	def_struct_primitive( svw_numSchematicViews,	schematicView,	"numSchematicViews");

	/* modifier panel control */

	def_struct_primitive( modp_setCurrentObject, modPanel,	"setCurrentObject");
	def_struct_primitive( modp_getCurrentObject, modPanel,	"getCurrentObject");
	def_struct_primitive( modp_getModifierIndex, modPanel,	"getModifierIndex");
	def_struct_primitive( modp_addModToSelection, modPanel,	"addModToSelection");

	/* MAX map & xref path config */

	def_struct_primitive( mapPaths_add,			mapPaths,	"add");
	def_struct_primitive( mapPaths_count,		mapPaths,	"count");
	def_struct_primitive( mapPaths_get,			mapPaths,	"get");
	def_struct_primitive( mapPaths_delete,		mapPaths,	"delete");
	def_struct_primitive( xrefPaths_add,		xrefPaths,	"add");
	def_struct_primitive( xrefPaths_count,		xrefPaths,	"count");
	def_struct_primitive( xrefPaths_get,		xrefPaths,	"get");
	def_struct_primitive( xrefPaths_delete,		xrefPaths,	"delete");

	/* references */

//	def_struct_primitive( refs_dependsOn,		refs,		"dependsOn");
	def_struct_primitive( refs_dependents,		refs,		"dependents");

	/* tool modes */

	def_struct_primitive( toolMode_uniformScale,	toolMode,		"uniformScale");
	def_struct_primitive( toolMode_nonUniformScale,	toolMode,		"nonUniformScale");
	def_struct_primitive( toolMode_squashScale,		toolMode,		"squashScale");
	def_struct_primitive( toolMode_coordsys,		toolMode,		"coordsys");
	def_struct_primitive( toolMode_pivotCenter,		toolMode,		"pivotCenter");
	def_struct_primitive( toolMode_selectionCenter,	toolMode,		"selectionCenter");
	def_struct_primitive( toolMode_transformCenter,	toolMode,		"transformCenter");

	/* utils */

	def_visible_primitive( show_class,			"showClass");
	def_visible_generic  ( show_props,			"showProperties");
	def_visible_generic  ( get_props,			"getPropNames");
	def_visible_generic  ( create_instance,		"createInstance");
	def_visible_primitive( getproperty,			"getProperty");
	def_visible_primitive( setproperty,			"setProperty");
	def_visible_primitive( apropos,				"apropos");
	def_visible_generic  ( exprForMAXObject,	"exprForMAXObject");
	def_visible_generic  ( getSubAnim,			"getSubAnim");
	def_visible_generic  ( getSubAnimName,		"getSubAnimName");
	def_visible_generic  ( getSubAnimNames,		"getSubAnimNames");
	def_visible_primitive( show_source,			"showSource");
	def_visible_generic  ( getAppData,			"getAppData");
	def_visible_generic  ( setAppData,			"setAppData");
	def_visible_generic  ( deleteAppData,		"deleteAppData");
	def_visible_generic  ( clearAllAppData,		"clearAllAppData");
	def_visible_generic  ( addPluginRollouts,	"addPluginRollouts");
	def_visible_primitive( startObjectCreation,	"startObjectCreation");
//	def_visible_primitive( waitForToolStop,		"waitForToolStop");

	def_visible_primitive( progressStart,		"progressStart");
	def_visible_primitive( progressUpdate,		"progressUpdate");
	def_visible_primitive( progressEnd,			"progressEnd");
	def_visible_primitive( getProgressCancel,	"getProgressCancel");
	def_visible_primitive( setProgressCancel,	"setProgressCancel");

	def_visible_primitive( dos_command,		"DOSCommand");
	def_visible_primitive( sleep,			"sleep");
	def_visible_primitive( timeStamp,		"timeStamp");

	def_visible_primitive( open_utility,			"openUtility");
	def_visible_primitive( close_utility,			"closeUtility");
	def_visible_primitive( add_rollout,				"addRollout");
	def_visible_primitive( remove_rollout,			"removeRollout");
	def_visible_primitive( new_rollout_floater,		"newRolloutFloater");
	def_visible_primitive( close_rollout_floater,	"closeRolloutFloater");

	def_visible_primitive( message_box,				"messageBox");
	def_visible_primitive( query_box,				"queryBox");
	def_visible_primitive( yes_no_cancel_box,		"yesNoCancelBox");

	def_visible_primitive( set_open_scene_script,	"setOpenSceneScript");
	def_visible_primitive( set_save_scene_script,	"setSaveSceneScript");

	def_struct_primitive ( callbacks_addScript,			callbacks,	"addScript");
	def_struct_primitive ( callbacks_removeScript,		callbacks,	"removeScripts");
	def_struct_primitive ( callbacks_broadcastCallback,	callbacks,	"broadcastCallback");
	def_struct_primitive ( callbacks_show,				callbacks,	"show");
	def_struct_primitive ( pesistents_remove,			persistents, "remove");
	def_struct_primitive ( pesistents_removeAll,		persistents, "removeAll");
	def_struct_primitive ( pesistents_show,				persistents, "show");

	def_visible_primitive( gc,				 "gc");
	def_visible_primitive( freeSceneBitmaps, "freeSceneBitmaps");
	def_visible_primitive( enumerateFiles,	 "enumerateFiles");
	def_visible_primitive( setSaveRequired,	 "setSaveRequired");
	def_visible_primitive( getSaveRequired,	 "getSaveRequired");
	def_visible_primitive( clearUndoBuffer,	 "clearUndoBuffer");

	def_visible_primitive( getCurrentSelection,	"getCurrentSelection");
	def_visible_primitive( selectByName,		"selectByName");
	def_primitive        ( getDollarSel,		"getDollarSel");

	def_visible_primitive( animateVertex,		"animateVertex");

	def_visible_primitive( showTextureMap,		"showTextureMap");

	def_visible_primitive( setWaitCursor,		"setWaitCursor");
	def_visible_primitive( setArrowCursor,		"setArrowCursor");

	def_visible_primitive( registerTimeCallback, "registerTimeCallback");
	def_visible_primitive( unregisterTimeCallback, "unregisterTimeCallback");

#ifdef DESIGN_VER
	def_struct_primitive ( mtlBrowser_browseFrom,	mtlBrowser, "browseFrom");
#endif

	/* active viewport utils */

	def_visible_primitive( getActiveCamera,			"getActiveCamera");
	def_visible_primitive( getScreenScaleFactor,	"getScreenScaleFactor");
	def_visible_primitive( mapScreenToWorldRay,		"mapScreenToWorldRay");
	def_visible_primitive( mapScreenToView,			"mapScreenToView");
	def_visible_primitive( mapScreenToCP,			"mapScreenToCP");
	def_visible_primitive( getCPTM,					"getCPTM");
	def_visible_primitive( getViewTM,				"getViewTM");
	def_visible_primitive( getViewSize,				"getViewSize");
	def_visible_primitive( getViewFOV,				"getViewFOV");

	def_visible_primitive( encryptScript,			"encryptScript");
	def_visible_primitive( encryptFile,				"encryptFile");

	def_visible_primitive( deleteChangeHandler,		"deleteChangeHandler");
	def_visible_primitive( deleteAllChangeHandlers,	"deleteAllChangeHandlers");

	/* morph objects & keys */

	def_visible_primitive( getMKTime,			"getMKTime");
	def_visible_primitive( setMKTime,			"setMKTime");
	def_visible_primitive( getMKWeight,			"getMKWeight");
	def_visible_primitive( setMKWeight,			"setMKWeight");
	def_visible_primitive( getMKKey,			"getMKKey");
	def_visible_primitive( getMKKeyIndex,		"getMKKeyIndex");
	def_visible_primitive( getMKTargetNames,	"getMKTargetNames");
	def_visible_primitive( getMKTargetWeights,	"getMKTargetWeights");
	def_visible_primitive( createMorphObject,	"createMorphObject");
	def_visible_primitive( addMorphTarget,		"addMorphTarget");
	def_visible_primitive( setMorphTarget,		"setMorphTarget");
	def_visible_primitive( deleteMorphTarget,	"deleteMorphTarget");
	def_visible_primitive( setMorphTargetName,	"setMorphTargetName");

	/* OLE automation */

	def_visible_primitive( register_OLE_interface,	"registerOLEInterface");
	def_visible_primitive( create_OLE_object,		"createOLEObject");
	def_visible_primitive( release_OLE_object,		"releaseOLEObject");
	def_visible_primitive( release_all_OLE_objects,	"releaseAllOLEObjects");

/* evaluator primitives - emitted by parser */
	
	def_primitive( progn,					"progn");
	def_primitive( max_command,				"max");
	def_primitive( return,					"return");
	def_primitive( exit,					"exit");
	def_primitive( continue,				"continue");

	def_lazy_primitive( quote,				"quote");
	def_lazy_primitive( if,					"if");
	def_lazy_primitive( case,				"case");
	def_lazy_primitive( while,				"while");
	def_lazy_primitive( do,					"do");
	def_lazy_primitive( for,				"for");
	def_lazy_primitive( and,				"and");
	def_lazy_primitive( or,					"or");

	def_lazy_primitive( animate_context,	"animate_context");
	def_lazy_primitive( redraw_context,		"redraw_context");
	def_lazy_primitive( pivot_context,		"pivot_context");
	def_lazy_primitive( level_context,		"level_context");
	def_lazy_primitive( time_context,		"time_context");
	def_lazy_primitive( coordsys_context,	"coordsys_context");
	def_lazy_primitive( center_context,		"center_context");
	def_lazy_primitive( undo_context,		"undo_context");

	def_lazy_primitive( assign,				"=");
	def_lazy_primitive( op_assign,			"+=");

	def_mapped_generic( assign,				"=");
	def_mapped_generic( op_assign,			"+=");
	def_generic       ( not,				"not");

	def_lazy_primitive( try,				"try");
	def_primitive	  ( throw,				"throw");

	def_lazy_primitive( change_handler,		"change_handler");

	def_visible_lazy_primitive( swap,		"swap");
